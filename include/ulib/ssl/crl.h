// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    crl.h - interface to a X509_CRL_CRL structure
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_CRL_H
#define ULIB_CRL_H 1

#include <ulib/date.h>
#include <ulib/string.h>

#include <openssl/x509.h>
#include <openssl/asn1.h>

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#  define X509_CRL_get_lastUpdate X509_CRL_get0_lastUpdate
#  define X509_CRL_get_nextUpdate X509_CRL_get0_nextUpdate
#endif

class UCertificate;

/**
 * This class provides all the services the openssl X509_CRL structure supports. A CRL is a list of certificate revoked.
 * This class contains a openssl X509_CRL structure and basically acts as a wrapper to functions that act on that structure
 */

class U_EXPORT UCrl {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   /**
    * Constructs this object takes <i>X509_CRL</i> as type
    */

   UCrl(X509_CRL* _crl = 0) : crl(_crl)
      {
      U_TRACE_REGISTER_OBJECT(0, UCrl, "%p", _crl)
      }

   /**
    * Constructs this object from the a encoded string.
    * The <i>type</i> specifies the type of encoding the string is in, e.g. DER or PEM
    *
    * @param encoding a string of bytes
    * @param type the CRL's encoding type
    */

   static X509_CRL* readCRL(const UString& x, const char* format = 0);

   UCrl(const UString& x, const char* format = 0)
      {
      U_TRACE_REGISTER_OBJECT(0, UCrl, "%V,%S", x.rep, format)

      crl = readCRL(x, format);
      }

   /**
    * Deletes this object
    */

   void clear()
      {
      U_TRACE_NO_PARAM(1, "UCrl::clear()")

      U_INTERNAL_ASSERT_POINTER(crl)

      U_SYSCALL_VOID(X509_CRL_free, "%p", crl);

      crl = 0;
      }

   ~UCrl()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UCrl)

      if (crl) clear();
      }

   bool isValid() const
      {
      U_TRACE_NO_PARAM(0, "UCrl::isValid()")

      U_RETURN(crl != 0);
      }

   void set(X509_CRL* pcrl)
      {
      U_TRACE(0, "UCrl::set(%p)", pcrl)

      if (isValid()) clear();

      if (pcrl) crl = X509_CRL_dup(pcrl);
      }

   bool set(const UString& x, const char* format = 0)
      {
      U_TRACE(0, "UCrl::set(%V,%S)", x.rep, format)

      set(readCRL(x, format));

      U_RETURN(crl != 0);
      }

   X509_CRL* getCrl() const { return crl; }

   bool isUpToDate() const;

   static unsigned getRevokedSerials(X509_CRL* crl, long* revoked, unsigned sz);
          unsigned getRevokedSerials(               long* revoked, unsigned sz) { return getRevokedSerials(crl, revoked, sz); }

   long hashCode() const
      {
      U_TRACE_NO_PARAM(0, "UCrl::hashCode()")

      U_INTERNAL_ASSERT_POINTER(crl)

      long hash = X509_NAME_hash(X509_CRL_get_issuer(crl));

      U_RETURN(hash);
      }

   static long getNumber(X509_CRL* crl);
          long getNumber() const          { return getNumber(crl); }

   /**
    * Returns the file name from CApath
    */

   static UString getFileName(X509_CRL* crl);

   UString getFileName() const { return getFileName(crl); }

   /**
    * Returns the <i>versionNumber</i> of this CRL
    */

   long getVersionNumber() const
      {
      U_TRACE_NO_PARAM(1, "UCrl::getVersionNumber()")

      U_INTERNAL_ASSERT_POINTER(crl)

      long version = U_SYSCALL(X509_CRL_get_version, "%p", crl);

      U_RETURN(version);
      }

   /**
    * Returns the last update of this CRL
    */

   const char* getLastUpdate() const __pure
      {
      U_TRACE_NO_PARAM(0, "UCrl::getLastUpdate()")

      U_INTERNAL_ASSERT_POINTER(crl)

      const ASN1_UTCTIME* utctime = X509_CRL_get_lastUpdate(crl);

      U_RETURN((const char*)utctime->data);
      }

   static time_t getIssueTime(X509_CRL* crl);
          time_t getIssueTime() const           { return getIssueTime(crl); }

   /**
    * Returns the next update for this CRL
    */

   const char* getNextUpdate() const __pure
      {
      U_TRACE_NO_PARAM(0, "UCrl::getNextUpdate()")

      U_INTERNAL_ASSERT_POINTER(crl)

      const ASN1_UTCTIME* utctime = X509_CRL_get_nextUpdate(crl);

      U_RETURN((const char*)utctime->data);
      }

   /**
    * Returns the issuer for this CRL
    */

   static UString getIssuer(X509_CRL* crl, bool ldap = false);

   UString getIssuer(bool ldap = false) const { return getIssuer(crl, ldap); }

   UString getIssuerForLDAP() const { return getIssuer(true); }

   /**
    * Indicate if the crl has been issued by a CA
    *
    * @param  ca the certificate we want to know if it has issued the crl
    * @return true if ca has issued the crl, else false
    */

   bool isIssued(UCertificate& ca) const;

   /**
    * Returns either the DER or PEM or BASE64 encoding of the crl depending on the value of format
    */

   UString getEncoded(const char* format = "PEM") const;

   static bool isEqual(X509_CRL* a, X509_CRL* b)
      {
      U_TRACE(0, "UCrl::isEqual(%p,%p)", a, b)

      int rc = U_SYSCALL(X509_CRL_cmp, "%p,%p", a, b);

      U_RETURN(rc == 0);
      }

   bool operator==(const UCrl& c) const { return  isEqual(crl, c.crl); }
   bool operator!=(const UCrl& c) const { return !isEqual(crl, c.crl); }

   // STREAM

   UString print() const;

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT ostream& operator<<(ostream& os, const UCrl& c);

#  ifdef DEBUG
   const char* dump(bool reset) const;
#  endif
#endif

protected:
   X509_CRL* crl;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UCrl)
};

#endif
