// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    compress.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/internal/common.h>
#include <ulib/base/lzo/minilzo.h>
#include <ulib/utility/compress.h>

bool UCompress::flag_init;

/* Work-memory needed for compression. Allocate memory in units
 * of `lzo_align_t' (instead of `char') to make sure it is properly aligned
 */

#define HEAP_ALLOC(var,size) \
      lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem,LZO1X_1_MEM_COMPRESS);

void UCompress::init()
{
   U_TRACE(0, "UCompress::init()")

   U_INTERNAL_ASSERT_EQUALS(flag_init, false)

#ifdef DEBUG
   int r = lzo_init();

   U_INTERNAL_ASSERT_EQUALS(r,LZO_E_OK)
#else
   (void) lzo_init();
#endif

// if (r != LZO_E_OK) U_ERROR("lzo_init() failed");

   flag_init = true;
}

uint32_t UCompress::compress(const char* src, uint32_t src_len, char* dst)
{
   U_TRACE(0, "UCompress::compress(%.*S,%u,%p)", src_len, src, src_len, dst)

   // Step 1: initialize the LZO library

   if (flag_init == false) init();

   U_INTERNAL_ASSERT(flag_init)

   // Step 2: compress from `src' to `dst' with LZO1X-1

   lzo_uint out_len;

#ifdef DEBUG
   int r = lzo1x_1_compress((lzo_byte*)src, (lzo_uint)src_len, (lzo_byte*)dst, &out_len, wrkmem);

   U_INTERNAL_ASSERT_EQUALS(r,LZO_E_OK)
#else
    (void) lzo1x_1_compress((lzo_byte*)src, (lzo_uint)src_len, (lzo_byte*)dst, &out_len, wrkmem);
#endif

   U_INTERNAL_DUMP("compressed %u bytes into %u bytes (%u%%)", src_len, out_len,
                     100 - (out_len * 100 / src_len))

   U_RETURN(out_len);
}

uint32_t UCompress::decompress(const char* src, uint32_t src_len, char* dst)
{
   U_TRACE(0, "UCompress::decompress(%.*S,%u,%p)", src_len, src, src_len, dst)

   // Step 1: initialize the LZO library

   if (flag_init == false) init();

   U_INTERNAL_ASSERT(flag_init)

   // Step 2: decompress from `src' to `dst' with LZO1X-1

   lzo_uint out_len;

#ifdef DEBUG
   int r = lzo1x_decompress((lzo_byte*)src, (lzo_uint)src_len, (lzo_byte*)dst, &out_len, NULL);

   U_INTERNAL_ASSERT_EQUALS(r,LZO_E_OK)
#else
    (void) lzo1x_decompress((lzo_byte*)src, (lzo_uint)src_len, (lzo_byte*)dst, &out_len, NULL);
#endif

   U_INTERNAL_DUMP("decompressed %u bytes back into %u bytes", src_len, out_len)

   U_RETURN(out_len);
}
