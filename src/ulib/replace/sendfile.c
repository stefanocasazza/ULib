/* sendfile.c */

#include <ulib/base/base.h>

#include <errno.h>
#ifndef _MSWINDOWS_
#  include <sys/socket.h>
#  include <sys/uio.h>
#endif

/**
 * This call copies data between one file descriptor and another. Either or both of these file descriptors may refer to a socket (but see below).
 * in_fd should be a file descriptor opened for reading and out_fd should be a descriptor opened for writing. offset is a pointer to a variable
 * holding the input file pointer position from which sendfile() will start reading data. When sendfile() returns, this variable will be set to
 * the offset of the byte following the last byte that was read. Because this copying is done within the kernel, sendfile() does not need to spend
 * time transferring data to and from user space.
 *
 * @param offset offset in input to start writing; updated on return to reflect the number of bytes sent.
 * @param count  is the number of bytes to copy between file descriptors.
 *
 * sendfile() returns the number of bytes sent, if transmission succeeded. If there was an error, it returns -1 with errno set. It should never return 0
 *
 * Could also use sendfilev() on Solaris >= 8:
 *
 * http://docs.sun.com/db/doc/816-0217/6m6nhtaps?a=view
 */

#ifdef __OSX__
/* int sendfile(int fd, int s, off_t offset, off_t* len, struct sf_hdtr* hdtr, int flags); */
#else
extern U_EXPORT ssize_t sendfile(int ofd, int ifd, off_t* offset, size_t count);
       U_EXPORT ssize_t sendfile(int ofd, int ifd, off_t* offset, size_t count)
{
   char* p;
   char buf[262144]; /* we're not recursive */
   size_t n = count;
   ssize_t r_in, r_out, wanted;

   count = 0;

   while (n > 0)
      {
      wanted = (n > sizeof(buf) ? sizeof(buf) : n);

      r_in = (offset ? pread(ifd, buf, (size_t) wanted, *offset)
                     :  read(ifd, buf, (size_t) wanted));

      if (r_in <= 0)
         {
         if (count) goto end;

         return -1;
         }

      n -= r_in;
      p  = buf;

      /* We now have r_in bytes waiting to go out, starting at p. Keep going until they're all written out */

      while (r_in > 0)
         {
         r_out = send(ofd, p, (size_t) r_in, 0);

         if (r_out <= 0)
            {
            if (count) goto end;

            return -1;
            }

         r_in  -= r_out;
         p     += r_out;
         count += r_out;
         }
      }

end:
   if (offset) *offset += count;

   return count;
}
#endif
