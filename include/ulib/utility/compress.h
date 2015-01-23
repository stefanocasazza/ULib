// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    compress.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_COMPRESS_H
#define ULIB_COMPRESS_H 1

#include <ulib/internal/common.h>

struct U_EXPORT UCompress {

   static void init();
   static bool flag_init;

   static uint32_t   compress(const char* src, uint32_t src_len, char* dst);
   static uint32_t decompress(const char* src, uint32_t src_len, char* dst);

   // We want to compress the data block at `src' with length `src_len' to
   // the block at `dst'. Because the input block may be incompressible,
   // we must provide a little more output space in case that compression
   // is not possible.

   static uint32_t space(uint32_t src_len)
      {
      U_TRACE(0, "UCompress::space(%u)", src_len)

      U_RETURN(src_len + src_len / 64 + 16 + 3);
      }
};

#endif
