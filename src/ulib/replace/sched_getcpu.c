/* sched_getcpu.c */

#include <ulib/base/base.h>

extern U_EXPORT int sched_getcpu(void);
       U_EXPORT int sched_getcpu(void) { errno = ENOSYS; return -1; }
