/* ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    cgzio.c
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/coder/gzio.h>

/**
 * Synopsis: Compress and Decompresses the source buffer into the destination buffer
 *
 * #define Z_OK              0
 * #define Z_STREAM_END      1
 * #define Z_NEED_DICT       2
 * #define Z_ERRNO         (-1)
 * #define Z_STREAM_ERROR  (-2)
 * #define Z_DATA_ERROR    (-3)
 * #define Z_MEM_ERROR     (-4)
 * #define Z_BUF_ERROR     (-5)
 * #define Z_VERSION_ERROR (-6)
 */

#ifdef DEBUG_DEBUG
static const char* get_error_string(int err)
{
   const char* p;

   switch (err)
      {
      case Z_ERRNO:           p = "Error occured while reading file";                                          break;  
      case Z_STREAM_ERROR:    p = "The stream state was inconsistent (e.g., next_in or next_out was NULL)";    break;  
      case Z_DATA_ERROR:      p = "The deflate data was invalid or incomplete";                                break;  
      case Z_MEM_ERROR:       p = "Memory could not be allocated for processing";                              break;  
      case Z_BUF_ERROR:       p = "Ran out of output buffer for writing compressed bytes";                     break;  
      case Z_VERSION_ERROR:   p = "The version of zlib.h and the version of the library linked do not match";  break;  
      default:                p = "Unknown error code.";                                                       break;
      }

   return p;
}
#endif

#ifdef U_ZLIB_DEFLATE_WORKSPACESIZE
static int workspacesize = zlib_deflate_workspacesize();
static char workspace[workspacesize];
#endif

uint32_t u_gz_deflate(const char* restrict input, uint32_t len, char* restrict result, bool bheader)
{
   int err;
   z_stream stream;

   /**
    * stream.zalloc    = // Set zalloc, zfree, and opaque to Z_NULL so  
    * stream.zfree     = // that when we call deflateInit2 they will be  
    * stream.opaque    = // updated to use default allocation functions.  
    * stream.total_out = // Total number of output bytes produced so far  
    * stream.next_in   = // Pointer to input bytes  
    * stream.avail_in  = // Number of input bytes left to process
    */

   U_INTERNAL_TRACE("u_gz_deflate(%p,%u,%p,%d)", input, len, result, bheader)

   U_INTERNAL_ASSERT_POINTER(input)

   /**
    * Before we can begin compressing (aka "deflating") data using the zlib 
    * functions, we must initialize zlib. Normally this is done by calling the 
    * deflateInit() function; in this case, however, we'll use deflateInit2() so 
    * that the compressed data will have gzip headers. This will make it easy to 
    * decompress the data later using a tool like gunzip, WinZip, etc.
    *
    * deflateInit2() accepts many parameters, the first of which is a C struct of 
    * type "z_stream" defined in zlib.h. The properties of this struct are used to 
    * control how the compression algorithms work. z_stream is also used to 
    * maintain pointers to the "input" and "output" byte buffers (next_in/out) as 
    * well as information about how many bytes have been processed, how many are 
    * left to process, etc.
    */

   (void) memset(&stream, 0, sizeof(z_stream));

#ifdef U_ZLIB_DEFLATE_WORKSPACESIZE
   (void) memset((void*)workspace, 0, workspacesize);

   stream.workspace = workspace; /* Set the workspace */
#endif

   /**
    * Initialize the zlib deflation (i.e. compression) internals with deflateInit2().
    *
    * The parameters are as follows:
    *
    * z_streamp strm - Pointer to a zstream struct 
    * int level      - Compression level. Must be Z_DEFAULT_COMPRESSION, or between 0 and 9:
    *                  1 gives best speed, 9 gives best compression, 0 gives no compression. 
    *
    * int method     - Compression method. Only method supported is "Z_DEFLATED". 
    * int windowBits - Base two logarithm of the maximum window size (the size of 
    *                  the history buffer). It should be in the range 8..15.
    *                  windowBits can also be -8..-15 for raw deflate. In this case,
    *                  -windowBits determines the window size. deflate() will then
    *                  generate raw deflate data with no zlib header or trailer, and
    *                  will not compute an adler32 check value.
    *                  Add 16 to windowBits to write a simple gzip header and trailer
    *                  around the compressed data instead of a zlib wrapper. The gzip
    *                  header will have no file name, no extra data, no comment, no
    *                  modification time (set to zero), no header crc, and the
    *                  operating system will be set to 255 (unknown). 
    * int memLevel   - Amount of memory allocated for internal compression state. 
    *                  1 uses minimum memory but is slow and reduces compression 
    *                  ratio; 9 uses maximum memory for optimal speed. Default value is 8. 
    * int strategy   - Used to tune the compression algorithm. Use the value 
    *                  Z_DEFAULT_STRATEGY for normal data, Z_FILTERED for data 
    *                  produced by a filter (or predictor), or Z_HUFFMAN_ONLY to 
    *                  force Huffman encoding only (no string match)
    */

   err = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, (bheader ? MAX_WBITS+16 : -MAX_WBITS), MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);

   if (err != Z_OK)
      {
      U_INTERNAL_PRINT("deflateInit2() = (%d, %s)", err, get_error_string(err))

      return 0;
      }

   stream.next_in  = (unsigned char*)input;
   stream.avail_in = len;

   /**
    * The zlib documentation states that destination buffer size must be
    * at least 0.1% larger than avail_in plus 12 bytes
    */

   do {
      /* Store location where next byte should be put in next_out */

      U_INTERNAL_PRINT("stream.total_out = %lu", stream.total_out)

      stream.next_out = (unsigned char*)result + stream.total_out;

      /**
       * Calculate the amount of remaining free space in the output buffer  
       * by subtracting the number of bytes that have been written so far  
       * from the buffer's total capacity
       */

      stream.avail_out = 0xffff0000;

      /**
       * deflate() compresses as much data as possible, and stops/returns when 
       * the input buffer becomes empty or the output buffer becomes full. If 
       * deflate() returns Z_OK, it means that there are more bytes left to 
       * compress in the input buffer but the output buffer is full; the output 
       * buffer should be expanded and deflate should be called again (i.e., the 
       * loop should continue to rune). If deflate() returns Z_STREAM_END, the 
       * end of the input stream was reached (i.e.g, all of the data has been 
       * compressed) and the loop should stop
       */

      err = zlib_deflate(&stream, Z_FINISH);
      }
   while (err == Z_OK);

   /* Check for zlib error and convert code to usable error message if appropriate */

   if (err != Z_STREAM_END)  
      {
      U_INTERNAL_PRINT("zlib_deflate() = (%d, %s)", err, get_error_string(err))

      return 0;
      }

   U_INTERNAL_ASSERT(stream.avail_out)

   err = zlib_deflateEnd(&stream);

   if (err != Z_OK)
      {
      U_INTERNAL_PRINT("zlib_deflateEnd() = (%d, %s)", err, get_error_string(err))

      return 0;
      }

   U_INTERNAL_PRINT("stream.total_in = %lu len = %u", stream.total_in, len)

   U_INTERNAL_ASSERT_EQUALS(stream.total_in, len)

#ifdef DEBUG
   if (bheader)
      {
      uint32_t   size_original;
      uint32_t* psize_original = (uint32_t*)(result + stream.total_out - 4);

      U_INTERNAL_ASSERT_EQUALS(memcmp(result, U_CONSTANT_TO_PARAM(GZIP_MAGIC)), 0)

#  if __BYTE_ORDER == __LITTLE_ENDIAN
      size_original = *psize_original;

      U_INTERNAL_PRINT("size original = %u (LE)",            *psize_original)
#  else
      size_original = u_invert32(*psize_original);

      U_INTERNAL_PRINT("size original = %u (BE)", u_invert32(*psize_original))
#  endif

      U_INTERNAL_ASSERT_EQUALS(len, size_original)
      }
#endif

   return stream.total_out;
}

/* gzip flag byte */

#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7: reserved */

uint32_t u_gz_inflate(const char* restrict input, uint32_t len, char* restrict result)
{
   int err;
   z_stream stream;

   U_INTERNAL_TRACE("u_gz_inflate(%p,%u,%p)", input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   (void) memset(&stream, 0, sizeof(z_stream));

#ifdef U_ZLIB_DEFLATE_WORKSPACESIZE
   (void) memset((void*)workspace, 0, workspacesize);

   stream.workspace = workspace;
#endif

   err = inflateInit2(&stream, -MAX_WBITS);

   if (err != Z_OK)
      {
      U_INTERNAL_PRINT("inflateInit2() = (%d, %s)", err, get_error_string(err))

      return 0;
      }

   if (u_get_unalignedp16(input) == U_MULTICHAR_CONSTANT16('\x1F','\x8B'))
      {
      int header_size;
      const char* restrict ptr   = input + U_CONSTANT_SIZE(GZIP_MAGIC);
            char _flags, _method = *ptr++; /* method */

      if (_method != Z_DEFLATED) /* 8 */
         {
         U_WARNING("u_gz_inflate(): unknown method %d -- not supported", _method);

         return 0;
         }

      _flags = *ptr++; /* compression flags */

      if ((_flags & ENCRYPTED) != 0)
         {
         U_WARNING("u_gz_inflate(): file is encrypted -- not supported");

         return 0;
         }

      if ((_flags & CONTINUATION) != 0)
         {
         U_WARNING("u_gz_inflate(): file is a a multi-part gzip file -- not supported");

         return 0;
         }

      if ((_flags & RESERVED) != 0)
         {
         U_WARNING("u_gz_inflate(): has flags 0x%x -- not supported", _flags);

         return 0;
         }

      /**
       * timestamp:   4
       * extra flags: 1
       * OS type:     1
       */

      ptr += 4 + 1 + 1;

      if ((_flags & EXTRA_FIELD) != 0)
         {
         unsigned _len  =  (unsigned)*ptr++;
                  _len |= ((unsigned)*ptr++) << 8;

         input += _len;
         }

      if ((_flags & ORIG_NAME) != 0) while (*ptr++ != '\0') {} /* Discard file    name if any */
      if ((_flags & COMMENT)   != 0) while (*ptr++ != '\0') {} /* Discard file comment if any */

      header_size = (ptr - input);

      len  -= header_size + 8; /* crc + size original */
      input = ptr;

      U_INTERNAL_PRINT("header_size = %d size original = %u *ptr = '%o'", header_size, *(uint32_t*)(ptr + len + 4), *(unsigned char*)ptr)
      }

   stream.next_in  = (unsigned char*)input;
   stream.avail_in = len;

   do {
      /* Set up output buffer */

      stream.next_out  = (unsigned char*)result + stream.total_out;
      stream.avail_out = 0xffff0000;

      err = zlib_inflate(&stream, Z_SYNC_FLUSH);

      switch (err)
         {
         case Z_OK:
            {
#        if !defined(ZLIB_VERNUM) || ZLIB_VERNUM < 0x1200
            /**
             * zlib < 1.2.0 workaround: push a dummy byte at the end of the stream when inflating (see zlib ChangeLog)
             * The zlib code effectively READ the dummy byte, this imply that the pointer MUST point to a valid data region.
             * The dummy byte is not always needed, only if inflate return Z_OK instead of Z_STREAM_END
             */
            unsigned char dummy = 0; /* dummy byte */

            stream.next_in  = &dummy;
            stream.avail_in = 1;

            err = inflate(&stream, Z_SYNC_FLUSH);

            U_INTERNAL_PRINT("inflate() = (%d, %s)", err, get_error_string(err))
#        endif
            }
         break;

         case Z_STREAM_END:
            {
            err = zlib_inflateEnd(&stream);

            if (err != Z_OK)
               {
               U_INTERNAL_PRINT("zlib_inflateEnd() = (%d, %s)", err, get_error_string(err))

               return 0;
               }

            U_INTERNAL_PRINT("stream.total_in = %lu len = %u", stream.total_in, len)

            U_INTERNAL_ASSERT_EQUALS(stream.total_in, len)

            return stream.total_out;
            }

         default:
            {
            U_INTERNAL_PRINT("zlib_inflate() = (%d, %s)", err, get_error_string(err))

            return 0;
            }
         }
      }
   while (stream.avail_out == 0);

   return 0;
}
