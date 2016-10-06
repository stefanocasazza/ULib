// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    pkcs7.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/ssl/pkcs7.h>
#include <ulib/utility/base64.h>
#include <ulib/ssl/certificate.h>
#include <ulib/utility/services.h>
#include <ulib/utility/string_ext.h>

#include <openssl/pem.h>

UPKCS7::~UPKCS7()
{
   U_TRACE_UNREGISTER_OBJECT(0, UPKCS7)

   if (pkcs7) clear();

   if (indata) (void) BIO_free(indata);
}

PKCS7* UPKCS7::readPKCS7(const UString& x, const char* format)
{
   U_TRACE(1, "UPKCS7::readPKCS7(%V,%S)", x.rep, format)

   BIO* in;
   UString tmp   = x;
   PKCS7* _pkcs7 = 0;

   if (format == 0) format = (x.isBinary() ? "DER" : "PEM");

   if (strncmp(format, U_CONSTANT_TO_PARAM("PEM")) == 0 &&
       strncmp(x.data(), U_CONSTANT_TO_PARAM("-----BEGIN PKCS7-----")) != 0)
      {
      unsigned length = x.size();

      UString buffer(length);

      UBase64::decode(x.data(), length, buffer);

      if (buffer &&
          u_base64_errors == 0)
         {
         tmp    = buffer;
         format = "DER";
         }
      }

   in = (BIO*) U_SYSCALL(BIO_new_mem_buf, "%p,%d", U_STRING_TO_PARAM(tmp));

   _pkcs7 = (PKCS7*) (strncmp(format, U_CONSTANT_TO_PARAM("PEM")) == 0 ? U_SYSCALL(PEM_read_bio_PKCS7, "%p,%p,%p,%p", in, 0, 0, 0)
                                                                       : U_SYSCALL(d2i_PKCS7_bio,      "%p,%p",       in, 0));

   (void) U_SYSCALL(BIO_free, "%p", in);

   U_RETURN_POINTER(_pkcs7, PKCS7);
}

UString UPKCS7::getContent(bool* valid_content) const
{
   U_TRACE(1, "UPKCS7::getContent(%p)", valid_content)

   U_INTERNAL_ASSERT_POINTER(pkcs7)

#if defined(_MSWINDOWS_) || (defined(DEBUG) && defined(__clang__))
#undef  NULL
#define NULL 0
#endif

   U_INTERNAL_DUMP("type = %p PKCS7_type_is_signed() = %d PKCS7_get_detached() = %d",
                     OBJ_obj2nid(pkcs7->type), PKCS7_type_is_signed(pkcs7), PKCS7_get_detached(pkcs7))

#if defined(DEBUG) && !defined(_MSWINDOWS_)
   // dump signatures on data

   STACK_OF(PKCS7_SIGNER_INFO)* sinfos = PKCS7_get_signer_info(pkcs7);

   U_INTERNAL_DUMP("sinfos = %p sk_PKCS7_SIGNER_INFO_num() = %d", sinfos, sk_PKCS7_SIGNER_INFO_num(sinfos))
#endif

   BIO* p7b = (BIO*) U_SYSCALL(PKCS7_dataInit, "%p,%p", pkcs7, indata);

   if (p7b)
      {
      if (valid_content) *valid_content = true;

      // We now have to 'read' from p7b to calculate digests etc...

      char buf[4096];
      BIO* out = (BIO*) U_SYSCALL(BIO_new, "%p", BIO_s_mem());

      while (true)
         {
         int i = BIO_read(p7b, buf, sizeof(buf));

         if (i <= 0) break;

         BIO_write(out, buf, i);
         }

      UString content = UStringExt::BIOtoString(out);

      if (indata) U_SYSCALL_VOID(BIO_pop,      "%p", p7b);
                  U_SYSCALL_VOID(BIO_free_all, "%p", p7b);

      U_RETURN_STRING(content);
      }

   if (valid_content) *valid_content = false;

   return UString::getStringNull();
}

UString UPKCS7::getEncoded(const char* format) const
{
   U_TRACE(1, "UPKCS7::getEncoded(%S)", format)

   U_INTERNAL_ASSERT_POINTER(pkcs7)

   if (strncmp(format, U_CONSTANT_TO_PARAM("DER"))    == 0 ||
       strncmp(format, U_CONSTANT_TO_PARAM("BASE64")) == 0)
      {
      unsigned len = U_SYSCALL(i2d_PKCS7, "%p,%p", pkcs7, 0);

      UString encoding(len);

      unsigned char* temp = (unsigned char*) encoding.data();

      (void) U_SYSCALL(i2d_PKCS7, "%p,%p", pkcs7, &temp);

      encoding.size_adjust(len);

      if (strncmp(format, U_CONSTANT_TO_PARAM("BASE64")) == 0)
         {
         UString x(len * 3 + 32U);

         UBase64::encode(encoding, x);

         U_RETURN_STRING(x);
         }

      U_RETURN_STRING(encoding);
      }
   else if (strncmp(format, U_CONSTANT_TO_PARAM("PEM")) == 0)
      {
      BIO* bio = (BIO*) U_SYSCALL(BIO_new, "%p", BIO_s_mem());

      (void) U_SYSCALL(PEM_write_bio_PKCS7, "%p,%p", bio, pkcs7);

      UString encoding = UStringExt::BIOtoString(bio);

      U_RETURN_STRING(encoding);
      }

   return UString::getStringNull();
}

bool UPKCS7::verify(int flags) const
{
   U_TRACE(1, "UPKCS7::verify(%d)", flags)

   U_INTERNAL_ASSERT_POINTER(pkcs7)

   BIO* out              = 0;
   STACK_OF(X509)* other = 0;

   /**
    * PKCS7_verify() verifies a PKCS#7 signedData structure. pkcs7 is the PKCS7 structure to verify.
    * other is a set of certificates in which to search for the signer's certificate. store is a trusted
    * certficate store (used for chain verification). indata is the signed data if the content is not present
    * in pkcs7 (that is it is detached). The content is written to out if it is not NULL.
    * If PKCS7_NOVERIFY is set the signer's certificates are not chain verified.
    * --------------------------------------------------------------------------------------------------------
    * VERIFY PROCESS: Normally the verify process proceeds as follows:
    * --------------------------------------------------------------------------------------------------------
    * Initially some sanity checks are performed on p7. The type of p7 must be signedData. There must be at
    * least one signature on the data and if the content is detached indata cannot be NULL.
    * An attempt is made to locate all the signer's certificates, first looking in the certs parameter
    * (if it is not NULL) and then looking in any certificates contained in the p7 structure itself. If any
    * signer's certificates cannot be located the operation fails. Each signer's certificate is chain verified
    * using the smimesign purpose and the supplied trusted certificate store. Any internal certificates in the
    * message are used as untrusted CAs. If any chain verify fails an error code is returned. Finally the signed
    * content is read (and written to out is it is not NULL) and the signature's checked. If all signature's
    * verify correctly then the function is successful. Any of the following flags (ored together) can be passed
    * in the flags parameter to change the default verify behaviour.
    * Only the flag PKCS7_NOINTERN is meaningful to PKCS7_get0_signers().
    * --------------------------------------------------------------------------------------------------------
    * - If PKCS7_NOINTERN is set the certificates in the message itself are not searched when locating the signer's
    *   certificate. This means that all the signers certificates must be in the certs parameter.
    * - If the PKCS7_TEXT flag is set MIME headers for type text/plain are deleted from the content. If the content
    *   is not of type text/plain then an error is returned.
    * - If PKCS7_NOVERIFY is set the signer's certificates are not chain verified.
    * - If PKCS7_NOCHAIN is set then the certificates contained in the message are not used as untrusted CAs.
    *   This means that the whole verify chain (apart from the signer's certificate) must be
    *   contained in the trusted store.
    * - If PKCS7_NOSIGS is set then the signatures on the data are not checked
    *  --------------------------------------------------------------------------------------------------------
    */

   int result = U_SYSCALL(PKCS7_verify, "%p,%p,%p,%p,%p,%d", pkcs7, other, UServices::store, indata, out, flags);

   if (other) U_SYSCALL_VOID(sk_X509_pop_free, "%p,%p", other, X509_free);

   U_RETURN(result == 1);
}

unsigned UPKCS7::getSignerCertificates(UVector<UCertificate*>& vec, int flags) const
{
   U_TRACE(1, "UPKCS7::getSignerCertificates(%p,%d)", &vec, flags)

   U_INTERNAL_ASSERT_POINTER(pkcs7)

   STACK_OF(X509)* signers = (STACK_OF(X509)*) U_SYSCALL(PKCS7_get0_signers, "%p,%p,%d", pkcs7, 0, flags);

   if (signers)
      {
      unsigned l = vec.size();
      int n = U_SYSCALL(sk_X509_num, "%p", signers);

      for (int i = 0; i < n; ++i)
         {
         UCertificate* cert;
         X509* x = sk_X509_value(signers, i);

         U_NEW(UCertificate, cert, UCertificate(x));

         vec.push_back(cert);
         }

      U_SYSCALL_VOID(sk_X509_free, "%p", signers);

      unsigned result = vec.size() - l;

      U_RETURN(result);
      }

   U_RETURN(0);
}

/**
 * creates and returns a PKCS#7 signedData structure
 *
 * data     is the data to be signed
 * signcert is the certificate to sign with
 * pkey     is the corresponsding private key
 * passwd   is the corresponsding password for the private key
 * certs    is an optional additional set of certificates to include in the PKCS#7 structure
 *          (for example any intermediate CAs in the chain)
 * flags    is an optional set of flags (PKCS7_TEXT, PKCS7_NOCERTS, PKCS7_DETACHED, PKCS7_BINARY, ...)
 */

PKCS7* UPKCS7::sign(const UString& data, const UString& signcert, const UString& pkey, const UString& passwd, const UString& certs, int flags)
{
   U_TRACE(1, "UPKCS7::sign(%V,%V,%V,%V,%V,%d)", data.rep, signcert.rep, pkey.rep, passwd.rep, certs.rep, flags)

   U_INTERNAL_ASSERT(passwd.isNullTerminated())

   UCertificate signer(signcert);

   STACK_OF(X509)* other = UCertificate::loadCerts(certs);

   EVP_PKEY* key = UServices::loadKey(pkey, "PEM", true, passwd.data(), 0);

   BIO* in = (BIO*) U_SYSCALL(BIO_new_mem_buf, "%p,%d", U_STRING_TO_PARAM(data));

   PKCS7* p7 = PKCS7_sign(signer.getX509(), key, other, in, flags);

   (void) U_SYSCALL(BIO_free, "%p", in);

   U_SYSCALL_VOID(EVP_PKEY_free, "%p", key);

   if (other) U_SYSCALL_VOID(sk_X509_pop_free, "%p,%p", other, X509_free);

   U_RETURN_POINTER(p7, PKCS7);
}

/* convert PKCS#7 structure to S/MIME format */

UString UPKCS7::writeMIME(PKCS7* _pkcs7)
{
   U_TRACE(1, "UPKCS7::writeMIME(%p)", _pkcs7)

   U_INTERNAL_ASSERT_POINTER(_pkcs7)

   UString result;

   BIO* out = (BIO*) U_SYSCALL(BIO_new, "%p", BIO_s_mem());

   if (U_SYSCALL(SMIME_write_PKCS7, "%p,%p,%p,%d", out, _pkcs7, 0, 0))
      {
      result = UStringExt::BIOtoString(out);
      }
   else
      {
      (void) U_SYSCALL(BIO_free, "%p", out);
      }

   U_RETURN_STRING(result);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UPKCS7::dump(bool reset) const
{
   *UObjectIO::os << "pkcs7    " << (void*)pkcs7 << '\n'
                  << "indata   " << (void*)indata;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
