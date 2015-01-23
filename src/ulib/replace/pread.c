/* pread.c */

#include <ulib/base/base.h>

#include <errno.h>
#include <unistd.h>

extern U_EXPORT ssize_t pread( int fd,       void* buf, size_t count, off_t offset);
extern U_EXPORT ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset);

/*
 * The pread() function is similar to read(). One difference is that
 * pread() has an additional parameter [off] which is the offset to
 * position in the file before reading. The function also differs in that
 * the file position is unchanged by the function (i.e. the file position
 * is the same before and after a call to pread).
 */

U_EXPORT ssize_t pread(int fd, void* buf, size_t count, off_t offset)
{
   off_t cur_pos;
   ssize_t num_read;

   U_INTERNAL_TRACE("pread(%d,%p,%d,%ld)", fd, buf, count, offset)

   if (((cur_pos = lseek(fd,      0, SEEK_CUR)) == (off_t)-1) ||
                   lseek(fd, offset, SEEK_SET)  == (off_t)-1) return -1;

   num_read = read(fd, buf, count);

   U_INTERNAL_PRINT("num_read = %d", num_read)

   if (lseek(fd, cur_pos, SEEK_SET) == (off_t)-1) return -1;

   return num_read;
}

/* Write COUNT of BUF to FD at given position OFFSET without changing
 * the file position. Return the number written, or -1
 */

U_EXPORT ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset)
{
   off_t cur_pos;
   ssize_t num_write;

   U_INTERNAL_TRACE("pwrite(%d,%p,%d,%ld)", fd, buf, count, offset)

   if (((cur_pos = lseek(fd,      0, SEEK_CUR)) == (off_t)-1) ||
                   lseek(fd, offset, SEEK_SET)  == (off_t)-1) return -1;

   num_write = write(fd, buf, count);

   U_INTERNAL_PRINT("num_write = %d", num_write)

   if (lseek(fd, cur_pos, SEEK_SET) == (off_t)-1) return -1;

   return num_write;
}
