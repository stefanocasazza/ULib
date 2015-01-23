#include <sys/types.h>
#include "seek.h"

#define CUR 1 /* sigh */

seek_pos seek_cur(int fd)
{ return lseek(fd,(off_t) 0,CUR); }
