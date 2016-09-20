/* ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    dgst.c
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#include <ulib/base/utility.h>
#include <ulib/base/ssl/dgst.h>
#include <ulib/base/coder/base64.h>
#include <ulib/base/coder/hexdump.h>

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#  ifndef OPENSSL_NO_SHA0
#     define OPENSSL_NO_SHA0
#  endif
#endif

UHashType              u_hashType;                 /* What type of hash is this? */
EVP_PKEY* restrict     u_pkey;                     /* private key to sign the digest */
const EVP_MD* restrict u_md;                       /* Digest instance */
unsigned char          u_mdValue[U_MAX_HASH_SIZE]; /* Final output */
int                    u_mdLen;                    /* Length of digest */

const char* restrict   u_hmac_key;    /* The loaded key */
uint32_t               u_hmac_keylen; /* The loaded key length */

#if OPENSSL_VERSION_NUMBER < 0x10100000L
HMAC_CTX    u_hctx;  /* Context for HMAC */
EVP_MD_CTX  u_mdctx; /* Context for digest */
#else
HMAC_CTX*   u_hctx;  /* Context for HMAC */
EVP_MD_CTX* u_mdctx; /* Context for digest */
#endif

__pure int u_dgst_get_algoritm(const char* restrict alg)
{
   int result = -1;

   U_INTERNAL_TRACE("u_dgst_get_algoritm(%s)", alg)

   if (u_get_unalignedp16(alg) == U_MULTICHAR_CONSTANT16('m','d'))
      {
           if (alg[2] == '5') result = U_HASH_MD5;
#  ifndef OPENSSL_NO_MD2
      else if (alg[2] == '2') result = U_HASH_MD2;
#  endif
#  ifndef OPENSSL_NO_MDC2
      else if (u_get_unalignedp16(alg+2) == U_MULTICHAR_CONSTANT16('c','2')) result = U_HASH_MDC2;
#  endif
      }
   else if (u_get_unalignedp16(alg) == U_MULTICHAR_CONSTANT16('s','h'))
      {
           if (u_get_unalignedp16(alg+2) == U_MULTICHAR_CONSTANT16('a','1')) result = U_HASH_SHA1;
#  ifdef HAVE_OPENSSL_98
      else if (u_get_unalignedp32(alg+2) == U_MULTICHAR_CONSTANT32('a','2','2','4')) result = U_HASH_SHA224;
      else if (u_get_unalignedp32(alg+2) == U_MULTICHAR_CONSTANT32('a','2','5','6')) result = U_HASH_SHA256;
      else if (u_get_unalignedp32(alg+2) == U_MULTICHAR_CONSTANT32('a','3','8','4')) result = U_HASH_SHA384;
      else if (u_get_unalignedp32(alg+2) == U_MULTICHAR_CONSTANT32('a','5','1','2')) result = U_HASH_SHA512;
#  endif
#  ifndef OPENSSL_NO_SHA0
      else if (alg[2] == 'a') result = U_HASH_SHA;
#  endif
      }
#ifndef OPENSSL_NO_RMD160
   else if (u_get_unalignedp64(alg) == U_MULTICHAR_CONSTANT64('r','i','p','e','m','d','1','6'))
      {
      U_INTERNAL_ASSERT_EQUALS(alg[8], '0')

      result = U_HASH_RIPEMD160;
      }
#endif

   return result;
}

void u_dgst_algoritm(int alg)
{
   U_INTERNAL_TRACE("u_dgst_algoritm(%d)", alg)

   switch (alg)
      {
      case U_HASH_MD5:       u_md = EVP_md5();        break;
#  ifndef OPENSSL_NO_MD2
      case U_HASH_MD2:       u_md = EVP_md2();        break;
#  endif
#  ifndef OPENSSL_NO_SHA0
      case U_HASH_SHA:       u_md = EVP_sha();        break;
#  endif
      case U_HASH_SHA1:      u_md = EVP_sha1();       break;
#  ifdef HAVE_OPENSSL_98
      case U_HASH_SHA224:    u_md = EVP_sha224();     break;
      case U_HASH_SHA256:    u_md = EVP_sha256();     break;
      case U_HASH_SHA384:    u_md = EVP_sha384();     break;
      case U_HASH_SHA512:    u_md = EVP_sha512();     break;
#  endif
#  if !defined(OPENSSL_NO_MDC2) && !defined(NO_MDC2)
      case U_HASH_MDC2:      u_md = EVP_mdc2();       break;
#  endif
#  ifndef OPENSSL_NO_RMD160
      case U_HASH_RIPEMD160: u_md = EVP_ripemd160();  break;
#  endif
      default:               u_md = 0;
      }

   if (u_md == 0) U_ERROR("Loading digest algorithm '%d' failed", alg);

   u_hashType = (UHashType)alg;
}

void u_dgst_init(int alg, const char* restrict _key, uint32_t keylen)
{
   U_INTERNAL_TRACE("u_dgst_init(%d,%.*s,%u)", alg, keylen, _key, keylen)

   u_dgst_algoritm(alg);

   if (keylen)
      {
      u_hmac_key    = _key;
      u_hmac_keylen = keylen;

#  if OPENSSL_VERSION_NUMBER < 0x10100000L
      HMAC_CTX_cleanup(&u_hctx);
      HMAC_CTX_init(&u_hctx);

      HMAC_Init_ex(&u_hctx, _key, keylen, u_md, 0);
#  else
      if  (u_hctx) (void) HMAC_CTX_reset(u_hctx);
      else u_hctx = HMAC_CTX_new();

      HMAC_Init_ex(u_hctx, _key, keylen, u_md, 0);
#  endif
      }
   else
      {
#  if OPENSSL_VERSION_NUMBER < 0x10100000L
      EVP_MD_CTX_cleanup(&u_mdctx);

      (void) EVP_DigestInit(&u_mdctx, u_md);
#  else
      if  (u_mdctx) EVP_MD_CTX_reset(u_mdctx);
      else u_mdctx = EVP_MD_CTX_new();

      (void) EVP_DigestInit_ex(u_mdctx, u_md, 0);
#  endif
      }
}

static int u_finish(unsigned char* restrict ptr, int base64)
{
   U_INTERNAL_TRACE("u_finish(%p,%d)", ptr, base64)

   U_INTERNAL_ASSERT_POINTER(ptr)
   U_INTERNAL_ASSERT_MINOR(u_mdLen, U_MAX_HASH_SIZE)

   if (base64 ==  0) return u_hexdump_encode(u_mdValue, u_mdLen, ptr);
   if (base64 ==  1) return u_base64_encode( u_mdValue, u_mdLen, ptr);
   if (base64 == -1)          u__memcpy(ptr, u_mdValue, u_mdLen, __PRETTY_FUNCTION__);

   return u_mdLen;
}

int u_dgst_finish(unsigned char* restrict hash, int base64) /* Finish and get hash */
{
   U_INTERNAL_TRACE("u_dgst_finish(%p,%d)", hash, base64)

   /* Finish up and copy out hash, returning the length */

   if (u_hmac_keylen)
      {
#  if OPENSSL_VERSION_NUMBER < 0x10100000L
      HMAC_Final(&u_hctx, u_mdValue, (unsigned int*)&u_mdLen);
#  else
      HMAC_Final( u_hctx, u_mdValue, (unsigned int*)&u_mdLen);
#  endif
      }
   else
      {
#  if OPENSSL_VERSION_NUMBER < 0x10100000L
      (void) EVP_DigestFinal(&u_mdctx, u_mdValue, (unsigned int*)&u_mdLen);
#  else
      (void) EVP_DigestFinal( u_mdctx, u_mdValue, (unsigned int*)&u_mdLen);
#  endif
      }

   if (hash) return u_finish(hash, base64);

   return u_mdLen;
}

/* The EVP signature routines are a high level interface to digital signatures */

void u_dgst_sign_init(int alg, ENGINE* impl)
{
   U_INTERNAL_TRACE("u_dgst_sign_init(%d,%p)", alg, impl)

   u_dgst_algoritm(alg);

#if OPENSSL_VERSION_NUMBER < 0x10100000L
   EVP_MD_CTX_cleanup(&u_mdctx);
   EVP_MD_CTX_init(&u_mdctx);

   EVP_SignInit_ex(&u_mdctx, u_md, impl);
#else
   if  (u_mdctx) EVP_MD_CTX_reset(u_mdctx);
   else u_mdctx = EVP_MD_CTX_new();

   (void) EVP_DigestInit_ex(u_mdctx, u_md, impl);
#endif
}

void u_dgst_verify_init(int alg, ENGINE* restrict impl)
{
   U_INTERNAL_TRACE("u_dgst_verify_init(%d,%p)", alg, impl)

   u_dgst_algoritm(alg);

#if OPENSSL_VERSION_NUMBER < 0x10100000L
   EVP_MD_CTX_cleanup(&u_mdctx);
   EVP_MD_CTX_init(&u_mdctx);

   EVP_VerifyInit_ex(&u_mdctx, u_md, impl);
#else
   if  (u_mdctx) EVP_MD_CTX_reset(u_mdctx);
   else u_mdctx = EVP_MD_CTX_new();

   (void) EVP_DigestInit_ex(u_mdctx, u_md, impl);
#endif
}

int u_dgst_sign_finish(unsigned char* restrict sig, int base64) /* Finish and get signature */
{
   U_INTERNAL_TRACE("u_dgst_sign_finish(%p,%d)", sig, base64)

   U_INTERNAL_ASSERT_POINTER(u_pkey)

#if OPENSSL_VERSION_NUMBER < 0x10100000L
   (void) EVP_SignFinal(&u_mdctx, u_mdValue, (unsigned int*)&u_mdLen, u_pkey);
#else
   (void) EVP_SignFinal( u_mdctx, u_mdValue, (unsigned int*)&u_mdLen, u_pkey);
#endif

   if (sig) return u_finish(sig, base64);

   return u_mdLen;
}

int u_dgst_verify_finish(unsigned char* restrict sigbuf, uint32_t siglen)
{
   U_INTERNAL_TRACE("u_dgst_verify_finish(%p,%u)", sigbuf, siglen)

   U_INTERNAL_ASSERT_POINTER(u_pkey)

#if OPENSSL_VERSION_NUMBER < 0x10100000L
   return EVP_VerifyFinal(&u_mdctx, sigbuf, siglen, u_pkey);
#else
   return EVP_VerifyFinal( u_mdctx, sigbuf, siglen, u_pkey);
#endif
}
