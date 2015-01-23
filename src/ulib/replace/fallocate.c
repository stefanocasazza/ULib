/* fallocate.c */

#include <ulib/base/base.h>

#include <unistd.h>
#include <stdio.h>

/**
 * fallocate() allows the caller to directly manipulate the allocated disk space for the file
 * referred to by fd for the byte range starting at offset and continuing for len bytes
 */

extern U_EXPORT int fallocate(int fd, int mode, off_t offset, off_t len);
       U_EXPORT int fallocate(int fd, int mode, off_t offset, off_t len)
{
   if (ftruncate(fd, offset+len) == -1) return (-1);

   return 0;
}
