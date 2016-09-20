// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    pkcs10.h - interface to a X509_REQ structure
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_PKCS10_H
#define ULIB_PKCS10_H 1

#include <ulib/date.h>
#include <ulib/string.h>

#include <openssl/x509.h>

/**
 * This class provides all the services the openssl X509_REQ structure supports.
 * This class contains a openssl X509_REQ structure and basically acts as a wrapper to functions that act on
 * that structure. A UPKCS10 holds all the information needed to create an X509_REQ identity certificate.
 * It can be sent to a CA who generates and signs the identity certficate. It provides get methods to extract
 * specific fields from the X509_REQ, such as subject, publicKey, versionNumber, signatureAlgorithm and signature
 */

class U_EXPORT UPKCS10 {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   /**
    * Constructs this object takes <i>X509_REQ</i> as type.
    */ 

   UPKCS10(X509_REQ* _request = 0) : request(_request)
      {
      U_TRACE_REGISTER_OBJECT(0, UPKCS10, "%p", _request)
      }

   /**
    * Constructs this object from the a encoded string.
    * The <i>type</i> specifies the type of encoding the string is in, e.g. DER or PEM.
    *
    * @param encoding a string of bytes
    * @param type the PKCS10's encoding type
    */

   static X509_REQ* readPKCS10(const UString& x, const char* format = 0);

   UPKCS10(const UString& x, const char* format = 0)
      {
      U_TRACE_REGISTER_OBJECT(0, UPKCS10, "%V,%S", x.rep, format)

      request = readPKCS10(x, format);
      }

   /**
    * Deletes this object
    */

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UPKCS10::clear()")

      U_INTERNAL_ASSERT_POINTER(request)

      U_SYSCALL_VOID(X509_REQ_free, "%p", request);

      request = 0;
      }

   ~UPKCS10()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UPKCS10)

      if (request) clear();
      }

   bool isValid() const
      {
      U_TRACE_NO_PARAM(0, "UPKCS10::isValid()")

      U_RETURN(request != 0);
      }

   /**
    * Returns <i>subject</i> of this certificateRequest
    */

   static UString getSubject(X509_REQ* request);

   UString getSubject() const { return getSubject(request); }

   /**
    * Returns the <i>versionNumber</i> of this certificateRequest
    */

   long getVersionNumber() const
      {
      U_TRACE_NO_PARAM(1, "UPKCS10::getVersionNumber()")

      U_INTERNAL_ASSERT_POINTER(request)

      long version = U_SYSCALL(X509_REQ_get_version, "%p", request);

      U_RETURN(version);
      }

   /**
    * Returns <i>publicKey</i> of this certificateRequest
    */

   EVP_PKEY* getSubjectPublicKey() const
      {
      U_TRACE_NO_PARAM(1, "UPKCS10::getSubjectPublicKey()")

      U_INTERNAL_ASSERT_POINTER(request)

      EVP_PKEY* evp = (EVP_PKEY*) U_SYSCALL(X509_REQ_get_pubkey, "%p", request);

      U_RETURN_POINTER(evp, EVP_PKEY);
      }

   /**
    * Returns true if the signature of the request verifies
    */

   bool verify(EVP_PKEY* publicKey) const
      {
      U_TRACE(0, "UPKCS10::verify(%p)", publicKey)

      U_INTERNAL_ASSERT_POINTER(request)

      if (U_SYSCALL(X509_REQ_verify, "%p,%p", request, publicKey) <= 0) U_RETURN(false);

      U_RETURN(true);
      }

   /**
    * Returns either the DER or PEM encoding of the CertificateRequest (depending on the value of format)
    * that can be sent to a CA for signing
    */

   UString getEncoded(const char* format = "PEM") const;

   // STREAM

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT ostream& operator<<(ostream& os, const UPKCS10& c);

#  ifdef DEBUG
   const char* dump(bool reset) const;
#  endif
#endif

protected:
   X509_REQ* request;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UPKCS10)
};

#endif
