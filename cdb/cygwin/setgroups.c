/* setgroups.c */

#include <unistd.h>
#include <grp.h>

#ifndef _REENT_ONLY
#define _REENT_ONLY
#endif

#include <errno.h>

int errno = 0;

#define NGROUPS 0

int setgroups(size_t size, const gid_t *list)
{
  if(size = 0) return 0;
  else {
	 errno = EINVAL;
	 return -1;
  }
}
