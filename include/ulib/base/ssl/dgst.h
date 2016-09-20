/* ===================================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    dgst.h - Interface definition for Hash functions required by the OpenSSL library
//
// = AUTHOR
//    Stefano Casazza
//
// =================================================================================== */

#ifndef ULIB_DGST_H
#define ULIB_DGST_H 1

#include <ulib/base/base.h>

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/engine.h>

#if !defined(HAVE_OPENSSL_97) && !defined(HAVE_OPENSSL_98)
#  warning "WARNING: I must to disable some function with this version of openssl... be aware"
#  define HMAC_CTX_init(ctx)
#  define HMAC_Init_ex(ctx,key,len,md,impl)
#  define EVP_MD_CTX_init(ctx)
#  define EVP_SignInit_ex(a,b,c)
#  define EVP_MD_CTX_cleanup(ctx)
#  define EVP_VerifyInit_ex(a,b,c)
#  define HMAC_CTX_cleanup(ctx) HMAC_cleanup(ctx)
#endif

#define U_MAX_HASH_SIZE       256 /* Max size of any expected hash algorithms (oversized) */
#define U_MAX_HASH_BLOCK_SIZE  64 /* Max size of blocks used - MD5 and SHA1 are both 64 bytes */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Enumeration of Hash (Digest) types
 *
 * The hash types known to openssl

typedef enum {
   U_HASH_MD2       = 0,
   U_HASH_MD5       = 1,
   U_HASH_SHA       = 2,
   U_HASH_SHA1      = 3,
   U_HASH_SHA224    = 4,
   U_HASH_SHA256    = 5,
   U_HASH_SHA384    = 6,
   U_HASH_SHA512    = 7,
   U_HASH_MDC2      = 8,
   U_HASH_RIPEMD160 = 9
} UHashType;
*/

extern U_EXPORT UHashType              u_hashType;                   /* What type of hash is this? */
extern U_EXPORT EVP_PKEY* restrict     u_pkey;                       /* private key to sign the digest */
extern U_EXPORT const EVP_MD* restrict u_md;                         /* Digest instance */
extern U_EXPORT unsigned char          u_mdValue[U_MAX_HASH_SIZE];   /* Final output */
extern U_EXPORT int                    u_mdLen;                      /* Length of digest */

extern U_EXPORT const char* restrict   u_hmac_key;    /* The loaded key */
extern U_EXPORT uint32_t               u_hmac_keylen; /* The loaded key length */

#if OPENSSL_VERSION_NUMBER < 0x10100000L
extern U_EXPORT HMAC_CTX    u_hctx;  /* Context for HMAC */
extern U_EXPORT EVP_MD_CTX  u_mdctx; /* Context for digest */
#else
extern U_EXPORT HMAC_CTX*   u_hctx;  /* Context for HMAC */
extern U_EXPORT EVP_MD_CTX* u_mdctx; /* Context for digest */
#endif

U_EXPORT void u_dgst_algoritm(int alg);
U_EXPORT int  u_dgst_get_algoritm(const char* restrict alg) __pure;

U_EXPORT void u_dgst_init(int alg, const char* restrict key, uint32_t keylen);

/**
 * \brief Hash some data.
 *
 * Take length bytes of data from the data buffer and update the hash
 * that already exists. This function may (and normally will) be called
 * many times for large blocks of data.
 *
 * @param data    The buffer containing the data to be hashed.
 * @param length  The number of bytes to be read from data
 */

static inline void u_dgst_hash(unsigned char* restrict data, uint32_t length)
{
   U_INTERNAL_TRACE("u_dgst_hash(%.*s,%u)", U_min(length,128), data, length)

   if (u_hmac_keylen)
      {
#  if OPENSSL_VERSION_NUMBER < 0x10100000L
      HMAC_Update(&u_hctx, data, length);
#  else
      HMAC_Update( u_hctx, data, length);
#  endif
      }
   else
      {
#  if OPENSSL_VERSION_NUMBER < 0x10100000L
      (void) EVP_DigestUpdate(&u_mdctx, data, length);
#  else
      (void) EVP_DigestUpdate( u_mdctx, data, length);
#  endif
      }
}

/**
 * \brief Finish up a Digest operation and read the result.
 *
 * This call tells the CryptoHash object that the input is complete and
 * to finalise the Digest. The output of the digest is write into the 
 * hash buffer if not null
 *
 * @param hash The buffer the hash should be read into.
 * @returns    The number of bytes copied into the hash buffer
 */

U_EXPORT int u_dgst_finish(unsigned char* restrict hash, int base64); /* Finish and get hash */

/**
 * The EVP signature routines are a high level interface to digital signatures
 */

U_EXPORT void u_dgst_sign_init(  int alg, ENGINE* restrict impl);
U_EXPORT void u_dgst_verify_init(int alg, ENGINE* restrict impl);

static inline void u_dgst_sign_hash(unsigned char* restrict data, uint32_t length)
{
   U_INTERNAL_TRACE("u_dgst_sign_hash(%.*s,%u)", U_min(length,128), data, length)

#if OPENSSL_VERSION_NUMBER < 0x10100000L
   (void) EVP_SignUpdate(&u_mdctx, data, length);
#else
   (void) EVP_SignUpdate( u_mdctx, data, length);
#endif
}

static inline void u_dgst_verify_hash(unsigned char* restrict data, uint32_t length)
{
   U_INTERNAL_TRACE("u_dgst_verify_hash(%.*s,%u)", U_min(length,128), data, length)

#if OPENSSL_VERSION_NUMBER < 0x10100000L
   (void) EVP_VerifyUpdate(&u_mdctx, data, length);
#else
   (void) EVP_VerifyUpdate( u_mdctx, data, length);
#endif
}

int u_dgst_sign_finish(  unsigned char* restrict sig, int base64); /* Finish and get signature */
int u_dgst_verify_finish(unsigned char* restrict sigbuf, uint32_t siglen);

#ifdef __cplusplus
}
#endif

#endif
