/* inflate.c - code for handling deflation */

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/base.h>
#include <ulib/base/zip/zipentry.h>
#include <ulib/base/zip/pushback.h>
#include <ulib/base/zip/compress.h>

static z_stream zs;

int init_compression(void)
{
   U_INTERNAL_TRACE("init_compression()")

   (void) memset(&zs, 0, sizeof(z_stream));

   /**
    * Why -MAX_WBITS? zlib has an undocumented feature, where if the windowbits
    * parameter is negative, it omits the zlib header, which seems to kill
    * any other zip/unzip program. This caused me SO much pain...
    */

   if (zlib_deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 9, Z_DEFAULT_STRATEGY) != Z_OK)
      {
      U_INTERNAL_TRACE("Error initializing deflation!")

      return 1;
      }

   return 0;
}

int compress_file(int in_fd, int out_fd, struct zipentry* ze)
{
   int rtval = 0;
   unsigned wramt;
   Bytef  in_buff[RDSZ],
         out_buff[RDSZ];

   U_INTERNAL_TRACE("compress_file()")

   zs.avail_in  = 0;
   zs.next_in   = in_buff;
   zs.next_out  = out_buff;
   zs.avail_out = (uInt)RDSZ;

   ze->crc = crc32(0L, 0, 0);

   while (1)
      {
      /* If deflate is out of input, fill the input buffer for it */

      if (zs.avail_in == 0 &&
          zs.avail_out > 0)
         {
         if ((rtval = read(in_fd, in_buff, RDSZ)) == 0) break;

         if (rtval == -1)
            {
            perror("read");

            return 1;
            }

         /* compute the CRC while we're at it */
         ze->crc = crc32(ze->crc, in_buff, rtval);
         }

      if (ze->compressed)
         {
         zs.avail_in = rtval;

         /* deflate the data */
         if (zlib_deflate(&zs, 0) != Z_OK)
            {
            U_INTERNAL_TRACE("Error deflating! %s:%d", __FILE__, __LINE__)

            return 1;
            }

         zs.next_in = in_buff;
         }
      else
         {
         zs.avail_in   = 0;
         zs.avail_out -= rtval;
         }

      /* If the output buffer is full, dump it to disk */
      if (zs.avail_out == 0)
         {
         if (write(out_fd, (ze->compressed ? out_buff : in_buff), RDSZ) != RDSZ)
            {
            perror("write");

            return 1;
            }

         /* clear the output buffer */
         zs.next_out  = out_buff;
         zs.avail_out = (uInt)RDSZ;
         }
      }

   /* If we have any data waiting in the buffer after we're done with the file we can flush it */
   if (zs.avail_out < RDSZ)
      {
      wramt = RDSZ - zs.avail_out;

      if (write(out_fd, (ze->compressed ? out_buff : in_buff), wramt) != (int)wramt)
         {
         perror("write");

         return 1;
         }

      /* clear the output buffer */
      zs.next_out  = out_buff;
      zs.avail_out = (uInt)RDSZ;
      }

   /* finish deflation. This purges zlib's internal data buffers */
   while (ze->compressed &&
          zlib_deflate(&zs, Z_FINISH) == Z_OK)
      {
      wramt = RDSZ - zs.avail_out;

      if (write(out_fd, out_buff, wramt) != (int)wramt)
         {
         perror("write");

         return 1;
         }

      zs.next_out = out_buff;
      zs.avail_out = (uInt)RDSZ;
      }

   /* If there's any data left in the buffer, write it out */
   if (zs.avail_out != RDSZ)
      {
      wramt = RDSZ - zs.avail_out;

      if (write(out_fd, out_buff, wramt) != (int)wramt)
         {
         perror("write");

         return 1;
         }
      }

   /* update fastzip's entry information */
   if (ze->compressed)
      {
      ze->usize = (ub4)zs.total_in;
      ze->csize = (ub4)zs.total_out;
      }

   /* Reset the deflation for the next time around */
   if (zlib_deflateReset(&zs) != Z_OK)
      {
      U_INTERNAL_TRACE("Error resetting deflation")

      return 1;
      }

   return 0;
}

int end_compression(void)
{
   int rtval;

   U_INTERNAL_TRACE("end_compression()")

   /* Oddly enough, zlib always returns Z_DATA_ERROR if you specify no zlib header. Go fig */

   if ((rtval = zlib_deflateEnd(&zs)) != Z_OK &&
        rtval                         != Z_DATA_ERROR)
      {
      U_INTERNAL_TRACE("Error calling deflateEnd: (%d) %s", rtval, zs.msg)

      return 1;
      }

   return 0;
}

int init_inflation(void)
{
   U_INTERNAL_TRACE("init_inflation()")

   (void) memset(&zs, 0, sizeof(z_stream));

   if (zlib_inflateInit2(&zs, -MAX_WBITS) != Z_OK)
      {
      U_INTERNAL_TRACE("Error initializing deflation!");

      return 1;
      }

   return 0;
}

int inflate_file(pb_file* pbf, int out_fd, struct zipentry* ze)
{
   int rdamt;
   ub4 crc = 0;
   Bytef  in_buff[RDSZ];
   Bytef out_buff[RDSZ];

   U_INTERNAL_TRACE("inflate_file(%p,%d,%p)", pbf, out_fd, ze)

   zs.avail_in = 0;

   crc = crc32(crc, 0, 0); /* initialize crc */

   for (;;) /* loop until we've consumed all the compressed data */
      {
      int rtval;

      if (zs.avail_in == 0)
         {
         if ((rdamt = pb_read(pbf, in_buff, RDSZ)) == 0) break;

         if (rdamt < 0)
            {
            perror("read");

            return 1;
            }

         U_INTERNAL_TRACE("%d bytes read", rdamt)

         zs.next_in  = in_buff;
         zs.avail_in = rdamt;
         }

      zs.next_out  = out_buff;
      zs.avail_out = RDSZ;

      if ((rtval = zlib_inflate(&zs, 0)) != Z_OK)
         {
         if (rtval == Z_STREAM_END)
            {
            U_INTERNAL_TRACE("end of stream");

            if (zs.avail_out != RDSZ)
               {
               crc = crc32(crc, out_buff, (RDSZ - zs.avail_out));

               if (out_fd >= 0 && write(out_fd, out_buff, (RDSZ - zs.avail_out)) != (int)(RDSZ - zs.avail_out))
                  {
                  perror("write");

                  return 1;
                  }
               }

            break;
            }
         else
            {
            U_INTERNAL_TRACE("Error inflating file! (%d)", rtval)

            return 1;
            }
         }
      else
         {
         if (zs.avail_out != RDSZ)
            {
            crc = crc32(crc, out_buff, (RDSZ - zs.avail_out));

            if (out_fd >= 0 && write(out_fd, out_buff, (RDSZ - zs.avail_out)) != (int)(RDSZ - zs.avail_out))
               {
               perror("write");

               return 1;
               }

            zs.next_out  = out_buff;
            zs.avail_out = RDSZ;
            }
         }
      }

   U_INTERNAL_TRACE("done inflating - %d bytes left over, CRC is %x", zs.avail_in, crc)

   if (zs.avail_in) pb_push(pbf, zs.next_in, zs.avail_in);

   ze->crc = crc;

   /* update fastzip's entry information */
   ze->csize = (ub4)zs.total_in;
   ze->usize = (ub4)zs.total_out;

   zlib_inflateReset(&zs);

   return 0;
}

int inflate_buffer(pb_file* pbf, unsigned* inlen, char** out_buff, unsigned* outlen, struct zipentry* ze)
{
   int flag;
   Bytef* ptr;
   unsigned size, nsize;

   U_INTERNAL_TRACE("inflate_buffer(%p,%u,%p,%u,%p)", pbf, *inlen, out_buff, *outlen, ze)

   zs.next_in = pbf->next;

   if (*inlen)
      {
      flag        = 0;
      zs.avail_in = *inlen;
      ptr         = *(Bytef**)out_buff;
      size        = *outlen;
      }
   else
      {
      flag        = 1;
      zs.avail_in = pbf->buff_amt;
      size        = zs.avail_in * 2;
      ptr         = (Bytef*) malloc(size);
      }

   zs.next_out  = ptr;
   zs.avail_out = size;

   for (;;) /* loop until we've consumed all the compressed data */
      {
      int rtval = zlib_inflate(&zs, 0);

      U_INTERNAL_TRACE("zs.avail_in  = %u zs.next_in  = %p zs.avail_out = %u zs.next_out = %p",
                        zs.avail_in,  zs.next_in, zs.avail_out, zs.next_out)

      if (rtval != Z_OK)
         {
         U_INTERNAL_TRACE("rtval = %d", rtval)

         if (rtval == Z_STREAM_END)
            {
            U_INTERNAL_TRACE("end of stream: size = %u zs.avail_out = %u", size, zs.avail_out)

            break;
            }
         else
            {
            U_INTERNAL_TRACE("Error inflating file (%d)", rtval)

            return 1;
            }
         }
      else
         {
         U_INTERNAL_TRACE("zs.avail_in = %u zs.avail_out = %u", zs.avail_in, zs.avail_out)

         U_INTERNAL_ASSERT(zs.avail_out == 0)

         if (zs.avail_in == 0) break; /* zlib 1.1.4 */

         nsize = size * 2;

         U_INTERNAL_TRACE("size = %u nsize = %u", size, nsize)

         ptr          = (Bytef*) realloc(ptr, nsize);
         zs.next_out  = ptr + size;
         zs.avail_out =       size;
         size         = nsize;
         }
      }

   U_INTERNAL_TRACE("zs.total_in = %u zs.total_out = %u", zs.total_in, zs.total_out)

   ze->crc = crc32(ze->crc, 0, 0); /* initialize crc */
   ze->crc = crc32(ze->crc, ptr, zs.total_out);

   U_INTERNAL_TRACE("done inflating - %d bytes left over, CRC is %x", zs.avail_in, ze->crc)

   /* update fastzip's entry information */
   ze->csize = (ub4)zs.total_in;
   ze->usize = (ub4)zs.total_out;

   if (flag)
      {
      *inlen    = (ub4)zs.total_in;
      *outlen   = (ub4)zs.total_out;
      *out_buff = (char*)ptr;
      }

   zlib_inflateReset(&zs);

   return 0;
}
