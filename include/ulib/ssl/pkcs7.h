// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    pkcs7.h - interface to a PKCS7
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_PKCS7_H
#define U_PKCS7_H 1

#include <ulib/string.h>

#include <openssl/pkcs7.h>
#include <openssl/x509v3.h>

/**
 * This class provides services for a PKCS7 structure. (general syntax for data that may have cryptography applied to it,
 * such as digital signatures and digital envelopes). This class contains a openssl PKCS7 structure and basically acts as
 * a wrapper to functions that act on that structure
 */

class UMimePKCS7;
class UCertificate;

template <class T> class UVector;

class U_EXPORT UPKCS7 {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

private:
   void set(PKCS7* p7, BIO* data)
      {
      U_TRACE(0, "UPKCS7::set(%p,%p)", p7, data)

      pkcs7  = p7;
      indata = data;
      }

public:
   /**
    * Constructs this object takes <i>PKCS7</i> as type, data is the signed data if the content is not present in pkcs7
    * (that is it is detached)
    */ 

   UPKCS7(PKCS7* p7 = 0, BIO* data = 0)
      {
      U_TRACE_REGISTER_OBJECT(0, UPKCS7, "%p,%p", p7, data)

      set(p7, data);
      }

   /**
    * Constructs this object from the a encoded string.
    * The <i>type</i> specifies the type of encoding the string is in, e.g. DER or PEM.
    * 
    * @param encoding a string of bytes
    * @param type the PKCS7's encoding type
    */

   static PKCS7* readPKCS7(const UString& x, const char* format = 0);

   UPKCS7(const UString& x, const char* format = 0)
      {
      U_TRACE_REGISTER_OBJECT(0, UPKCS7, "%V,%S", x.rep, format)

      indata = 0;
      pkcs7  = readPKCS7(x, format);
      }

   /**
    * Deletes this object
    */

   void clear()
      {
      U_TRACE_NO_PARAM(1, "UPKCS7::clear()")

      U_INTERNAL_ASSERT_POINTER(pkcs7)

      U_SYSCALL_VOID(PKCS7_free, "%p", pkcs7);

      pkcs7 = 0;
      }

   ~UPKCS7();

   bool isValid() const
      {
      U_TRACE_NO_PARAM(0, "UPKCS7::isValid()")

      U_RETURN(pkcs7 != 0);
      }

   PKCS7* getPKCS7() const { return pkcs7; }

   bool isDetached() const
      {
      U_TRACE_NO_PARAM(1, "UPKCS7::isDetached()")

      bool result = (U_SYSCALL(PKCS7_ctrl, "%p,%d,%ld,%s", pkcs7, PKCS7_OP_GET_DETACHED_SIGNATURE, 0, 0) != 0);

      U_RETURN(result);
      }

   /* if it isn't a signed-only message */

   bool isMessageEncrypted() const
      {
      U_TRACE_NO_PARAM(0, "UPKCS7::isMessageEncrypted()")

      bool result = (PKCS7_type_is_enveloped(pkcs7) != 0);

      U_RETURN(result);
      }

   bool isMessageSignedAndEncrypted() const
      {
      U_TRACE_NO_PARAM(0, "UPKCS7::isMessageSignedAndEncrypted()")

      bool result = (PKCS7_type_is_signedAndEnveloped(pkcs7) != 0);

      U_RETURN(result);
      }

   /**
    * Returns either the DER or PEM or BASE64 encoding of the pkcs7 depending on the value of format
    */

   UString getEncoded(const char* format = "PEM") const;

   /**
    * Returns bool value to indicate the correctness of the signed data.
    * PKCS7_NOVERIFY Verify only the signature, not the certificate chain.
    * This is probably not what you want, because the signature could be easily forged
    */

   bool verify(int flags = PKCS7_NOVERIFY) const;

   /**
    * Retrieves the signer's certificates from p7, it does not check their validity or whether any signatures are valid.
    * The flags parameter have the same meanings as in PKCS7_verify()
    */

   unsigned getSignerCertificates(UVector<UCertificate*>& vec, int flags = 0) const;

   /**
    * Returns the signed content from p7
    */

   UString getContent(bool* valid_content = 0) const;

   /**
    * creates and returns a PKCS#7 signedData structure
    *
    * @param data     is the data to be signed
    * @param signcert is the certificate to sign with
    * @param pkey     is the corresponsding private key
    * @param passwd   is the corresponsding password for the private key
    * @param certs    is an optional additional set of certificates to include in the PKCS#7 structure
    *                 (for example any intermediate CAs in the chain)
    * @param flags    is an optional set of flags (PKCS7_TEXT, PKCS7_NOCERTS, PKCS7_DETACHED, PKCS7_BINARY, ...)
    */

   static PKCS7* sign(const UString& data,
                      const UString& signcert,
                      const UString& pkey,
                      const UString& passwd,
                      const UString& certs, int flags = PKCS7_BINARY);

   /**
    * convert PKCS#7 structure to S/MIME format
    *
    * @param pkcs7 is the appropriate PKCS7 structure
    */

   static UString writeMIME(PKCS7* pkcs7);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   BIO* indata; // indata is the signed data if the content is not present in pkcs7 (that is it is detached)
   PKCS7* pkcs7;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UPKCS7)

   friend class UMimePKCS7;
};

#endif
