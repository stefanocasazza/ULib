// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    services.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_SERVICES_H
#define ULIB_SERVICES_H 1

#include <ulib/string.h>
#include <ulib/utility/hexdump.h>

#ifdef USE_LIBUUID
#  include <uuid/uuid.h>
#endif

#ifdef USE_LIBSSL
#  include <openssl/pem.h>
#  include <openssl/engine.h>
#  include <openssl/x509_vfy.h>
#  include <ulib/base/ssl/dgst.h>
typedef int (*verify_cb)(int,X509_STORE_CTX*); /* error callback */
#  ifndef X509_V_FLAG_CRL_CHECK
#  define X509_V_FLAG_CRL_CHECK     0x4
#  endif
#  ifndef X509_V_FLAG_CRL_CHECK_ALL
#  define X509_V_FLAG_CRL_CHECK_ALL 0x8
#  endif
#  define U_STORE_FLAGS (X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL)
#endif

#ifndef FNM_LEADING_DIR
#define FNM_LEADING_DIR FNM_PERIOD
#endif
#ifndef FNM_CASEFOLD
#define FNM_CASEFOLD FNM_IGNORECASE
#endif

struct U_EXPORT UServices {

   static bool isSetuidRoot();               // UID handling: check if we are setuid-root
   static void closeStdInputOutput();        // move stdin and stdout to /dev/null
   static int  getDevNull(const char* file); // return open(/dev/null)

   static int  askToLDAP(UString* pinput, UHashMap<UString>* ptable, const char* fmt, va_list argp);

   /**
    * Read data from fd
    *
    * @param timeoutMS specified the timeout value, in milliseconds. A negative value indicates no timeout, i.e. an infinite wait
    */

   static bool read(int fd, UString& buffer, uint32_t count = U_SINGLE_READ, int timeoutMS = -1); // read while not received almost count data

   // read while received data

   static void readEOF(int fd, UString& buffer)
      {
      U_TRACE(0, "UServices::readEOF(%d,%p)", fd, &buffer)

      while (UServices::read(fd, buffer, U_SINGLE_READ, -1)) {}
      }

   // ------------------------------------------------------------
   // DOS or wildcard regexpr - multiple patterns separated by '|'
   // ------------------------------------------------------------
   // '?' matches any single character
   // '*' matches any string, including the empty string
   // ------------------------------------------------------------

   static bool dosMatch(const char* s, uint32_t len, const char* mask, uint32_t size, int flags = 0)
      {
      U_TRACE(0, "UServices::dosMatch(%.*S,%u,%.*S,%u,%d)", len, s, len, size, mask, size, flags)

      if (u_dosmatch(s, len, mask, size, flags)) U_RETURN(true);

      U_RETURN(false);
      }

   static bool dosMatchExt(const char* s, uint32_t len, const char* mask, uint32_t size, int flags = 0)
      {
      U_TRACE(0, "UServices::dosMatchExt(%.*S,%u,%.*S,%u,%d)", len, s, len, size, mask, size, flags)

      if (u_dosmatch_ext(s, len, mask, size, flags)) U_RETURN(true);

      U_RETURN(false);
      }

   static bool dosMatchWithOR(const char* s, uint32_t len, const char* mask, uint32_t size, int flags = 0)
      {
      U_TRACE(0, "UServices::dosMatchWithOR(%.*S,%u,%.*S,%u,%d)", len, s, len, size, mask, size, flags)

      if (u_dosmatch_with_OR(s, len, mask, size, flags)) U_RETURN(true);

      U_RETURN(false);
      }

   static bool dosMatchExtWithOR(const char* s, uint32_t len, const char* mask, uint32_t size, int flags = 0)
      {
      U_TRACE(0, "UServices::dosMatchExtWithOR(%.*S,%u,%.*S,%u,%d)", len, s, len, size, mask, size, flags)

      if (u_dosmatch_ext_with_OR(s, len, mask, size, flags)) U_RETURN(true);

      U_RETURN(false);
      }

   static bool dosMatch(const UString& s, const char* mask, uint32_t size, int flags = 0) { return dosMatch(U_STRING_TO_PARAM(s),              mask, size, flags); }
   static bool dosMatch(const UString& s, const UString& mask,             int flags = 0) { return dosMatch(U_STRING_TO_PARAM(s), U_STRING_TO_PARAM(mask), flags); }

   static bool dosMatchExt(const UString& s, const char* mask, uint32_t size, int flags = 0) { return dosMatchExt(U_STRING_TO_PARAM(s),              mask, size, flags); }
   static bool dosMatchExt(const UString& s, const UString& mask,             int flags = 0) { return dosMatchExt(U_STRING_TO_PARAM(s), U_STRING_TO_PARAM(mask), flags); }

   static bool dosMatchWithOR(const UString& s, const char* mask, uint32_t size, int flags = 0)
      { return dosMatchWithOR(U_STRING_TO_PARAM(s), mask, size, flags); }

   static bool dosMatchWithOR(const UString& s, const UString& mask, int flags = 0) { return dosMatchWithOR(U_STRING_TO_PARAM(s), U_STRING_TO_PARAM(mask), flags); }

   static bool dosMatchExtWithOR(const UString& s, const char* mask, uint32_t size, int flags = 0)
      { return dosMatchExtWithOR(U_STRING_TO_PARAM(s), mask, size, flags); }

   static bool dosMatchExtWithOR(const UString& s, const UString& mask, int flags = 0) { return dosMatchExtWithOR(U_STRING_TO_PARAM(s), U_STRING_TO_PARAM(mask), flags); }

   static bool fnmatch(const UString& s, const UString& mask, int flags = FNM_PATHNAME | FNM_CASEFOLD)
      {
      U_TRACE(0, "UServices::fnmatch(%V,%V,%d)", s.rep, mask.rep, flags)

      if (u_fnmatch(U_STRING_TO_PARAM(s), U_STRING_TO_PARAM(mask), flags)) U_RETURN(true);

      U_RETURN(false);
      }

   // manage session cookies and hashing password...

   static unsigned char key[16];

   static void generateKey(unsigned char* pkey, unsigned char* hexdump)
      {
      U_TRACE(1, "UServices::generateKey(%p,%p)", pkey, hexdump)

      U_INTERNAL_ASSERT_POINTER(pkey)

#  ifdef USE_LIBUUID
      U_SYSCALL_VOID(uuid_generate, "%p", pkey);
#  else
      *(uint64_t*) pkey                     = getUniqUID();
      *(uint64_t*)(pkey + sizeof(uint64_t)) = getUniqUID();
#  endif

      if (hexdump) (void) u_hexdump_encode(pkey, 16, hexdump);
      }

   static UString generateToken(const UString& data, time_t expire);
   static bool    getTokenData(       UString& data, const UString& value, time_t& expire);

   // creat a new unique UUID value - 16 bytes (128 bits) long
   // return from the binary representation a 36-byte string (plus tailing '\0') of the form 1b4e28ba-2fa1-11d2-883f-0016d3cca427

   static UString  getUUID();
   static uint64_t getUniqUID(); // creat a new unique UUID value - 8 bytes (64 bits) long

   static void generateDigest(int alg, uint32_t keylen, unsigned char* data, uint32_t size, UString& output, int base64 = 0);
   static void generateDigest(int alg, uint32_t keylen, const UString& data,                UString& output, int base64 = 0)
      { generateDigest(alg, keylen, (unsigned char*)U_STRING_TO_PARAM(data), output, base64); }

   static UString generateCode(uint32_t len = 6)
      {
      U_TRACE(0, "UServices::generateCode(%u)", len)

      UString code(len);
      char* ptr = code.data();

      for (uint32_t i = 0; i < len; ++i, ++ptr) *ptr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[u_get_num_random(64 - 3)];

      code.size_adjust(len);

      U_RETURN_STRING(code);
      }

#ifdef USE_LIBSSL
   static UString createToken(int alg = U_HASH_SHA256);

   static void generateDigest(int alg, unsigned char* data, uint32_t size)
      {
      U_TRACE(0, "UServices::generateDigest(%d,%.*S,%u)", alg, size, data, size)

      u_dgst_init(alg, 0, 0);

      u_dgst_hash(data, size);

      (void) u_dgst_finish(0, 0);

      U_INTERNAL_DUMP("u_mdLen = %d", u_mdLen)
      }

   static void generateDigest(int alg, const UString& data) { generateDigest(alg, (unsigned char*)U_STRING_TO_PARAM(data)); }

   /**
    * setup OPENSSL standard certificate directory. The X509_STORE holds the tables etc for verification stuff.
    * A X509_STORE_CTX is used while validating a single certificate. The X509_STORE has X509_LOOKUPs for looking
    * up certs. The X509_STORE then calls a function to actually verify the certificate chain
    */

   static UString* CApath;
   static X509_STORE* store;

   static void setCApath(const char* _CApath);

   /* When something goes wrong, this is why */

   static int verify_error;
   static int verify_depth;            /* how far to go looking up certs */
   static X509* verify_current_cert;   /* current certificate */

   /**
    * The passwd_cb() function must write the password into the provided buffer buf which is of size size.
    * The actual length of the password must be returned to the calling function. rwflag indicates whether the
    * callback is used for reading/decryption (rwflag=0) or writing/encryption (rwflag=1).
    *
    * See man SSL_CTX_set_default_passwd_cb(3) for more information
    */

   static int passwd_cb(char* buf, int size, int rwflag, void* restrict password);

   static void setVerifyStatus(long result);

   static void setVerifyStatus() { setVerifyStatus(verify_error); }

   static void setVerifyCallback(verify_cb func)
      {
      U_TRACE(0, "UServices::setVerifyCallback(%p)", func)

      U_INTERNAL_ASSERT_POINTER(func)
      U_INTERNAL_ASSERT_POINTER(store)

      X509_STORE_set_verify_cb_func(store, func);
      }

   static void releaseEngine(ENGINE* e, bool bkey);
   static int X509Callback(int ok, X509_STORE_CTX* ctx);
   static UString getFileName(long hash, bool crl = false);
   static ENGINE* loadEngine(const char* id, unsigned int flags);
   static bool setupOpenSSLStore(const char* CAfile = 0, const char* CApath = 0, int store_flags = U_STORE_FLAGS);
   static EVP_PKEY* loadKey(const UString& x, const char* format, bool _private = true, const char* password = 0, ENGINE* e = 0);

   /**
    * data   is the data to be signed
    * pkey   is the corresponsding private key
    * passwd is the corresponsding password for the private key
    */

   static bool    verifySignature(  int alg, const UString& data, const UString& signature, const UString& pkey,                                    ENGINE* e = 0);
   static UString getSignatureValue(int alg, const UString& data,                           const UString& pkey, const UString& passwd, int base64, ENGINE* e = 0);
#endif
};

#endif
