// fallocate64.c

#include <ulib/base/base.h>

extern U_EXPORT int fallocate64(int fd, int mode, off_t offset, off_t len);

/* Reserve storage for the data of the file associated with fd */

int fallocate64(int fd, int mode, off_t offset, off_t len) { return fallocate(fd, mode, offset, len); }
