/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    hash.c
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================*/

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/hash.h>
#include <ulib/base/utility.h>

/**
 * Quick 4byte hashing function
 *
 * References: http://mia.ece.uic.edu/cgi-bin/lxr/http/ident?v=ipband-0.7.2;i=makehash
 * -----------------------------------------------------------------------------------------------------------------
 * Performs a one to one mapping between input integer and output integers, in other words, two different
 * input integers i, j will ALWAYS result in two different output hash(i) and hash(j). This hash function
 * is designed so that a changing just one bit in input 'i' will potentially affect the every bit in hash(i),
 * and the correlation between succesive hashes is (hopefully) extremely small (if not zero). It also can be
 * used as a quick, dirty, portable and open source random number generator that generates randomness on all 32 bits
 * -----------------------------------------------------------------------------------------------------------------
 */

__pure uint32_t u_random(uint32_t a)
{
   /**
    * Random sequence table - source of random numbers.
    * The random() routine 'amplifies' this 2**8 long sequence into a 2**32 long sequence
    */

   static const unsigned char rseq[259] = {
    79, 181,  35, 147,  68, 177,  63, 134, 103,   0,  34,  88,  69, 221, 231,  13,
    91,  49, 220,  90,  58, 112,  72, 145,   7,   4,  93, 176, 129, 192,   5, 132,
    86, 142,  21, 148,  37, 139,  39, 169, 143, 224, 251,  64, 223,   1,   9, 152,
    51,  66,  98, 155, 180, 109, 149, 135, 229, 137, 215,  42,  62, 115, 246, 242,
   118, 160,  94, 249, 123, 144, 122, 213, 252, 171,  60, 167, 253, 198,  77,   2,
   154, 174, 168,  52,  27,  92, 226, 233, 205,  10, 208, 247, 209, 113, 211, 106,
   163, 116,  65, 196,  73, 201,  23,  15,  31, 140, 189,  53, 207,  83,  87, 202,
   101, 173,  28,  46,   6, 255, 237,  47, 227,  36, 218,  70, 114,  22, 100,  96,
   182, 117,  43, 228, 210,  19, 191, 108, 128,  89,  97, 153, 212, 203,  99, 236,
   238, 141,   3,  95,  29, 232,   8,  75,  57,  25, 159,  24, 131, 162,  67, 119,
    74,  30, 138, 214, 240,  12, 187, 127, 133,  18,  81, 222, 188, 239,  82, 199,
   186, 166, 197, 230, 126, 161, 200,  40,  59, 165, 136, 234, 250,  44, 170, 157,
   190, 150, 105,  84,  55, 204,  56, 244, 219, 151, 178, 195, 194, 110, 184,  14,
    48, 146, 235, 216, 120, 175, 254,  50, 102, 107,  41, 130,  54,  26, 248, 225,
   111, 124,  33, 193,  76, 121, 125, 158, 185, 245,  16, 206,  71,  45,  20, 179,
    32,  38, 241,  80,  85, 243,  11, 217,  61,  17,  78, 172, 156, 183, 104, 164,
    79, 181,  35 };

   int i = 0;
   union uucflag u = { { 0 } };
   unsigned char* restrict c = (unsigned char* restrict) &a;

   U_INTERNAL_TRACE("u_random(%u)", a)

   for (; i < 4; ++i)
      {
      u.c[3] = rseq[c[0]] + rseq[c[1] + 1] + rseq[c[2] + 2] + rseq[c[3] + 3];
      u.c[2] = rseq[c[1]] + rseq[c[2] + 1] + rseq[c[3] + 2];
      u.c[1] = rseq[c[2]] + rseq[c[3] + 1];
      u.c[0] = rseq[c[3]];

      a = u.u;
      }

   return a;
}

#ifdef HAVE_ARCH64
__pure uint32_t u_random64(uint64_t ptr)
{
   uint32_t a = (uint32_t)((ptr & 0xffffffff00000000) >> 32L),
            b = (uint32_t)((ptr & 0x00000000ffffffff));

   U_INTERNAL_TRACE("u_random64(%llu)", ptr)

   return u_random(a) + u_random(b);
}
#endif

/* the famous DJB hash function for strings */

__pure uint32_t u_cdb_hash(const unsigned char* restrict t, uint32_t tlen, int flags)
{
   uint32_t h;

   U_INTERNAL_TRACE("u_cdb_hash(%.*s,%u,%d)", U_min(tlen,128), t, tlen, flags)

#ifdef USE_HARDWARE_CRC32
   if (flags == -1) return u_crc32(t, tlen);
#endif
   h = 5381;

   if (flags <= 0) while (tlen--) h = ((h << 5) + h) ^            *t++;
   else            while (tlen--) h = ((h << 5) + h) ^ u__tolower(*t++);

   return h;
}

__pure uint32_t u_hash_ignore_case(const unsigned char* restrict data, uint32_t len)
{
   union uucflag u;
   uint32_t tmp, hash = len, rem = len & 3;

   U_INTERNAL_TRACE("u_hash_ignore_case(%.*s,%u,%d)", U_min(len,128), data, len)

   len >>= 2;

   /* Main loop */

   for (; len > 0; --len)
      {
      u.u    = u_get_unalignedp32(data);
      u.c[0] = u__tolower(u.c[0]);
      u.c[1] = u__tolower(u.c[1]);
      u.c[2] = u__tolower(u.c[2]);
      u.c[3] = u__tolower(u.c[3]);

      hash  +=  u_get_unalignedp16(&u.lo);
      tmp    = (u_get_unalignedp16(&u.hi) << 11) ^ hash;
      hash   = (hash << 16) ^ tmp;
      data  += 2 * sizeof(uint16_t);
      hash  += hash >> 11;
      }

   /* Handle end cases */

   switch (rem)
      {
      case 3:
         {
         u.lo   = u_get_unalignedp16(data);
         u.c[0] = u__tolower(u.c[0]);
         u.c[1] = u__tolower(u.c[1]);

         hash += u_get_unalignedp16(&u.lo);
         hash ^= hash << 16;
         hash ^= ((signed char)u__tolower(data[sizeof(uint16_t)])) << 18;
         hash += hash >> 11;
         }
      break;

      case 2:
         {
         u.lo   = u_get_unalignedp16(data);
         u.c[0] = u__tolower(u.c[0]);
         u.c[1] = u__tolower(u.c[1]);

         hash += u_get_unalignedp16(&u.lo);
         hash ^= hash << 11;
         hash += hash >> 17;
         }
      break;

      case 1:
         {
         hash += (signed char)u__tolower(*data);
         hash ^= hash << 10;
         hash += hash >> 1;
         }
      }

   /* Force "avalanching" of final 127 bits */

   hash ^= hash <<  3;
   hash += hash >>  5;
   hash ^= hash <<  4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >>  6;

   return hash;
}

/* MurmurHash3 was written by Austin Appleby

#define getblock32(p,i) u_get_unalignedp32(p+i)
#define getblock64(p,i) u_get_unalignedp64(p+i)

//#if !defined(USE_HARDWARE_CRC32) && !defined(HAVE_ARCH64)
static inline uint32_t rotl32(uint32_t x, int8_t r) { return (x << r) | (x >> (32 - r)); }

static inline uint32_t fmix32(uint32_t h)
{
   h ^= h >> 16;
   h *= 0x85ebca6b;
   h ^= h >> 13;
   h *= 0xc2b2ae35;
   h ^= h >> 16;

   return h;
}

__pure uint32_t murmurhash3_x86_32(const unsigned char* restrict bp, uint32_t len)
{
   U_INTERNAL_TRACE("murmurhash3_x86_32(%.*s,%u,%d)", U_min(len,128), bp, len)

   uint8_t* tail;
   int i, nblocks;
   uint32_t* blocks;
   uint32_t  c1, c2, k1, h1 = u_seed_hash; // seed

   if (len > 16)
      {
      const uint32_t c3 = 0x38b34ae5,
                     c4 = 0xa1e38b93;

      uint32_t h2, h3, h4, k2, k3, k4;

      c1 = 0x239b961b;
      c2 = 0xab0e9789;

      nblocks = len / 16;

      tail   = (uint8_t*)(bp + nblocks*16);
      blocks = (uint32_t*)tail;

      h2 = h3 = h4 = h1; // seed

      // body

      for (i = -nblocks; i; ++i)
         {
         k1 = getblock32(blocks,i*4);
         k2 = getblock32(blocks,i*4+1);
         k3 = getblock32(blocks,i*4+2);
         k4 = getblock32(blocks,i*4+3);

         k1 *= c1; k1 = rotl32(k1,15); k1 *= c2; h1 ^= k1;
         h1 = rotl32(h1,19); h1 += h2; h1 = h1*5+0x561ccd1b;
         k2 *= c2; k2 = rotl32(k2,16); k2 *= c3; h2 ^= k2;
         h2 = rotl32(h2,17); h2 += h3; h2 = h2*5+0x0bcaa747;
         k3 *= c3; k3 = rotl32(k3,17); k3 *= c4; h3 ^= k3;
         h3 = rotl32(h3,15); h3 += h4; h3 = h3*5+0x96cd1c35;
         k4 *= c4; k4 = rotl32(k4,18); k4 *= c1; h4 ^= k4;
         h4 = rotl32(h4,13); h4 += h1; h4 = h4*5+0x32ac3b17;
         }

      // tail

      k1 = k2 = k3 = k4 = 0;

      switch (len & 15)
         {
         case 15: k4 ^= (uint32_t)tail[14] << 16;
         case 14: k4 ^= (uint32_t)tail[13] << 8;
         case 13: k4 ^= (uint32_t)tail[12];
                  k4 *= c4; k4 = rotl32(k4,18); k4 *= c1; h4 ^= k4;

         case 12: k3 ^= (uint32_t)tail[11] << 24;
         case 11: k3 ^= (uint32_t)tail[10] << 16;
         case 10: k3 ^= (uint32_t)tail[ 9] << 8;
         case  9: k3 ^= (uint32_t)tail[ 8] << 0;
                  k3 *= c3; k3 = rotl32(k3,17); k3 *= c4; h3 ^= k3;

         case  8: k2 ^= (uint32_t)tail[ 7] << 24;
         case  7: k2 ^= (uint32_t)tail[ 6] << 16;
         case  6: k2 ^= (uint32_t)tail[ 5] << 8;
         case  5: k2 ^= (uint32_t)tail[ 4] << 0;
                  k2 *= c2; k2 = rotl32(k2,16); k2 *= c3; h2 ^= k2;

         case  4: k1 ^= (uint32_t)tail[ 3] << 24;
         case  3: k1 ^= (uint32_t)tail[ 2] << 16;
         case  2: k1 ^= (uint32_t)tail[ 1] << 8;
         case  1: k1 ^= (uint32_t)tail[ 0] << 0;
                  k1 *= c1; k1 = rotl32(k1,15); k1 *= c2; h1 ^= k1;
         }

      // finalization

      h1 ^= (uint32_t)len; h2 ^= (uint32_t)len;
      h3 ^= (uint32_t)len; h4 ^= (uint32_t)len;

      h1 += h2; h1 += h3; h1 += h4;
      h2 += h1; h3 += h1; h4 += h1;

      h1 = fmix32(h1);
      h2 = fmix32(h2);
      h3 = fmix32(h3);
      h4 = fmix32(h4);

      h1 += h2; h1 += h3; h1 += h4;
      h2 += h1; h3 += h1; h4 += h1;

      U_INTERNAL_PRINT("h4 = %u", h4)

      U_INTERNAL_ASSERT_MAJOR(h4, 0)

      return h4;
      }

   c1 = 0xcc9e2d51;
   c2 = 0x1b873593;

   nblocks = len / 4;

   tail   = (uint8_t*)(bp + nblocks*4);
   blocks = (uint32_t*)tail;

   // body

   i = -nblocks;

   for (; i; ++i)
      {
      k1  = getblock32(blocks,i);

      k1 *= c1;
      k1  = rotl32(k1,15);
      k1 *= c2;

      h1 ^= k1;
      h1  = rotl32(h1,13);
      h1  = h1*5+0xe6546b64;
      }

   // tail

   k1 = 0;

   switch (len & 3)
      {
      case 3: k1 ^= u__tolower(tail[2]) << 16;
      case 2: k1 ^= u__tolower(tail[1]) <<  8;
      case 1: k1 ^= u__tolower(tail[0]);
              k1 *= c1; k1 = rotl32(k1,15); k1 *= c2; h1 ^= k1;
      }

   // finalization

   h1 ^= len;
   h1  = fmix32(h1);

   U_INTERNAL_PRINT("h1 = %u", h1)

   U_INTERNAL_ASSERT_MAJOR(h1, 0)

   return h1;
}
//#endif

//#if !defined(USE_HARDWARE_CRC32) && defined(HAVE_ARCH64)
static inline uint64_t rotl64(uint64_t x, int8_t r) { return (x << r) | (x >> (64 - r)); }

static inline uint64_t fmix64(uint64_t k)
{
   k ^= k >> 33;
   k *= 0xff51afd7ed558ccdULL;
   k ^= k >> 33;
   k *= 0xc4ceb9fe1a85ec53ULL;
   k ^= k >> 33;

   return k;
}

__pure uint32_t murmurhash3_x86_64(const unsigned char* restrict bp, uint32_t len)
{
   U_INTERNAL_TRACE("murmurhash3_x86_64(%.*s,%u,%d)", U_min(len,128), bp, len)

   int i;
   uint64_t h1, h2, k1, k2;
   const int nblocks = len / 16;
   const uint64_t c1 = 0x87c37b91114253d5ULL,
                  c2 = 0x4cf5ad432745937fULL;

   const uint8_t* tail    = (const uint8_t*)(bp + nblocks*16);
   const uint64_t* blocks = (const uint64_t*)bp;

   h1 = h2 = u_seed_hash; // seed

   // body

   for (i = 0; i < nblocks; ++i)
      {
      k1 = getblock64(blocks,i*2+0);
      k2 = getblock64(blocks,i*2+1);

      k1 *= c1; k1 = rotl64(k1,31); k1 *= c2; h1 ^= k1;
      h1 = rotl64(h1,27); h1 += h2; h1 = h1*5+0x52dce729;
      k2 *= c2; k2 = rotl64(k2,33); k2 *= c1; h2 ^= k2;
      h2 = rotl64(h2,31); h2 += h1; h2 = h2*5+0x38495ab5;
      }

   // tail

   k1 = k2 = 0;

   switch (len & 15)
      {
      case 15: k2 ^= (uint64_t)tail[14] << 48;
      case 14: k2 ^= (uint64_t)tail[13] << 40;
      case 13: k2 ^= (uint64_t)tail[12] << 32;
      case 12: k2 ^= (uint64_t)tail[11] << 24;
      case 11: k2 ^= (uint64_t)tail[10] << 16;
      case 10: k2 ^= (uint64_t)tail[ 9] << 8;
      case  9: k2 ^= (uint64_t)tail[ 8];
               k2 *= c2; k2 = rotl64(k2,33); k2 *= c1; h2 ^= k2;

      case  8: k1 ^= (uint64_t)tail[ 7] << 56;
      case  7: k1 ^= (uint64_t)tail[ 6] << 48;
      case  6: k1 ^= (uint64_t)tail[ 5] << 40;
      case  5: k1 ^= (uint64_t)tail[ 4] << 32;
      case  4: k1 ^= (uint64_t)tail[ 3] << 24;
      case  3: k1 ^= (uint64_t)tail[ 2] << 16;
      case  2: k1 ^= (uint64_t)tail[ 1] << 8;
      case  1: k1 ^= (uint64_t)tail[ 0];
               k1 *= c1; k1 = rotl64(k1,31); k1 *= c2; h1 ^= k1;
      }

   // finalization

   h1 ^= (uint64_t)len;
   h2 ^= (uint64_t)len;
   h1 += h2;
   h2 += h1;
   h1 = fmix64(h1);
   h2 = fmix64(h2);
   h1 += h2;
   h2 += h1;

   U_INTERNAL_PRINT("h2 = %llu %u", h2, (uint32_t)h2)

   U_INTERNAL_ASSERT_MAJOR(h2, 0)

   return (uint32_t)h2;
}
//#endif
*/
