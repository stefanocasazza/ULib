/** ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    hash.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_BASE_HASH_H
#define ULIB_BASE_HASH_H 1

#include <ulib/base/xxhash/xxhash.h>

#undef GCC_VERSION
#include <ulib/base/base.h>

#define U_hash_size(n) (1U<<(n))
#define U_hash_mask(n) (U_hash_size(n)-1U)

/* get next prime number for container */

#define U_GET_NEXT_PRIME_NUMBER(sz) ((sz) <=         53U ?         97U : \
                                     (sz) <=         97U ?        193U : \
                                     (sz) <=        193U ?        389U : \
                                     (sz) <=        389U ?        769U : \
                                     (sz) <=        769U ?       1543U : \
                                     (sz) <=       1543U ?       3079U : \
                                     (sz) <=       3079U ?       6151U : \
                                     (sz) <=       6151U ?      12289U : \
                                     (sz) <=      12289U ?      24593U : \
                                     (sz) <=      24593U ?      49157U : \
                                     (sz) <=      49157U ?      98317U : \
                                     (sz) <=      98317U ?     196613U : \
                                     (sz) <=     196613U ?     393241U : \
                                     (sz) <=     393241U ?     786433U : \
                                     (sz) <=     786433U ?    1572869U : \
                                     (sz) <=    1572869U ?    3145739U : \
                                     (sz) <=    3145739U ?    6291469U : \
                                     (sz) <=    6291469U ?   12582917U : \
                                     (sz) <=   12582917U ?   25165843U : \
                                     (sz) <=   25165843U ?   50331653U : \
                                     (sz) <=   50331653U ?  100663319U : \
                                     (sz) <=  100663319U ?  201326611U : \
                                     (sz) <=  201326611U ?  402653189U : \
                                     (sz) <=  402653189U ?  805306457U : \
                                     (sz) <=  805306457U ?  805306457U : \
                                     (sz) <= 1610612741U ? 3221225473U : 4294967291U)

#ifdef __cplusplus
extern "C" {
#endif

U_EXPORT uint32_t u_random(  uint32_t val) __pure; /* quick 4byte hashing function */
#ifdef HAVE_ARCH64
U_EXPORT uint32_t u_random64(uint64_t ptr) __pure;
#endif
static inline double u_randomd(uint32_t a) { return (1.0/4294967296.0) * u_random(a); } /* Map hash value into number 0.0 <= n < 1.0 */

/* hash variable-length key into 32-bit value */

U_EXPORT uint32_t murmurhash3_x86_32_ignore_case(unsigned char* restrict t, uint32_t tlen) __pure;

U_EXPORT      uint32_t u_cdb_hash(        unsigned char* restrict t, uint32_t tlen, int flags) __pure;
static inline uint32_t u_hash_ignore_case(unsigned char* restrict t, uint32_t tlen) { return murmurhash3_x86_32_ignore_case(t, tlen); }

#ifdef USE_HARDWARE_CRC32
U_EXPORT uint32_t u_crc32(unsigned char* restrict t, uint32_t tlen) __pure;

static inline uint32_t u_hash(unsigned char* restrict t, uint32_t tlen) { return u_crc32(t, tlen); }
#else
# ifndef HAVE_ARCH64
U_EXPORT uint32_t u_hash(unsigned char* restrict t, uint32_t tlen) __pure;
# else
U_EXPORT uint32_t murmurhash3_x86_64(unsigned char* restrict t, uint32_t tlen) __pure;

static inline uint32_t u_hash(    unsigned char* restrict t, uint32_t tlen) { return (uint32_t)XXH64(t, tlen, u_seed_hash); }
static inline uint32_t u_hash_old(unsigned char* restrict t, uint32_t tlen) { return murmurhash3_x86_64(t, tlen); }
# endif
#endif

#ifdef __cplusplus
}
#endif

#endif
