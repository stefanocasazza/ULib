/* pushback.c - code for a pushback buffer to handle file I/O */

#include <ulib/base/utility.h>
#include <ulib/base/zip/zipentry.h>
#include <ulib/base/zip/pushback.h>

inline void pb_init(pb_file* pbf, int fd, ub1* data)
{
   U_INTERNAL_TRACE("pb_init(%p,%d,%p)", pbf, fd, data)

   if (data)
      {
      pbf->fd       = -1;
      pbf->next     = data;
      pbf->buff_amt = fd;
      }
   else
      {
      pbf->fd       = fd;
      pbf->next     = pbf->pb_buff;
      pbf->buff_amt = 0;
      }
}

int pb_push(pb_file* pbf, void* buff, int amt)
{
   int in_amt, wrap = 0, n;

   U_INTERNAL_TRACE("pb_push(%p,%p,%d)", pbf, buff, amt)

   U_INTERNAL_TRACE("%d bytes being pushed back to the buffer", amt)

   if (pbf->fd == -1)
      {
      /* update the buff_amt field */

      pbf->next     -= amt;
      pbf->buff_amt += amt;

      U_INTERNAL_TRACE("%d bytes remaining in buffer", pbf->buff_amt)

      return amt;
      }

   /* determine how much we can take */

   if ((int)(RDSZ - pbf->buff_amt) < amt) in_amt = RDSZ - pbf->buff_amt;
   else                                   in_amt = amt;

   if (in_amt == 0) return 0;

   /* figure out if we need to wrap around, and if so, by how much */

   if (((pbf->pb_buff + RDSZ) - pbf->next) < in_amt)
      {
      wrap = in_amt - ((pbf->pb_buff + RDSZ) - pbf->next);
      }

   /* write everything up til the end of the buffer */

   n = in_amt - wrap;

   if (n > 0) u__memcpy(pbf->next, buff, n, __PRETTY_FUNCTION__);

   /* finish writing what's wrapped around */

   if (wrap > 0) u__memcpy(pbf->pb_buff, ((char*)buff + n), wrap, __PRETTY_FUNCTION__);

   /* update the buff_amt field */

   pbf->buff_amt += in_amt;

   U_INTERNAL_TRACE("%d bytes we can't accept", (amt - in_amt))

   return in_amt;
}

int pb_read(pb_file* pbf, void* buff, int amt)
{
   void* bp = buff;
   int tmp, wrap = 0, out_amt = 0, n;

   U_INTERNAL_TRACE("pb_read(%p,%p,%d)", pbf, buff, amt)

   U_INTERNAL_TRACE("%d bytes requested from us", amt)

   if (pbf->fd == -1)
      {  
      if (amt > (int)pbf->buff_amt) amt = pbf->buff_amt;

      if (amt > 0)
         {
         u__memcpy(buff, pbf->next, amt, __PRETTY_FUNCTION__);

         /* update the buff_amt field */

         pbf->next     += amt;
         pbf->buff_amt -= amt;
         }

      U_INTERNAL_TRACE("%d bytes remaining in buffer", pbf->buff_amt)

      return amt;
      }

   while (out_amt < amt)
      {
      /* if our push-back buffer contains some data */

      if (pbf->buff_amt > 0)
         {
         U_INTERNAL_TRACE("giving data from buffer")

         /* calculate how much we can actually give the caller */

         if ((amt - out_amt) < (int)pbf->buff_amt) tmp = (amt - out_amt);
         else                                      tmp = pbf->buff_amt;

         /* Determine if we're going to need to wrap around the buffer */

         if (tmp > ((pbf->pb_buff + RDSZ) - pbf->next))
            {
            wrap = tmp - ((pbf->pb_buff + RDSZ) - pbf->next);
            }

         n = tmp - wrap;

         if (n > 0) u__memcpy(bp, pbf->next, n, __PRETTY_FUNCTION__);

         bp = &(((char*)bp)[n]);

         /* If we need to wrap, read from the start of the buffer */

         if (wrap > 0)
            {
            u__memcpy(bp, pbf->pb_buff, wrap, __PRETTY_FUNCTION__);

            bp = &(((char *)bp)[wrap]);
            }

         /* update the buff_amt field */

         pbf->buff_amt -= tmp;
         pbf->next     += tmp;

         U_INTERNAL_TRACE("%d bytes remaining in buffer", pbf->buff_amt)

         /**
          * if the buffer is empty, reset the next header to the front of the
          * buffer so subsequent pushbacks/reads won't have to wrap
          */

         if (pbf->buff_amt == 0) pbf->next = pbf->pb_buff;

         out_amt += tmp;
         }
      else
         {
         U_INTERNAL_TRACE("Reading from file...")

         /* The pushback buffer was empty, so we just need to read from the file */

         U_INTERNAL_ASSERT_DIFFERS(pbf->fd,-1)

         tmp = read(pbf->fd, bp, (amt - out_amt));

         if (tmp == 0) break;

         out_amt += tmp;

         bp = &(((char*)bp)[tmp]);
         }
      }

   U_INTERNAL_TRACE("managed to read %d bytes", out_amt)

   return out_amt;
}
