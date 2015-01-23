/* sem.c */

#include <ulib/base/base.h>

#include <unistd.h>
#include <errno.h>

typedef int sem_t;

extern U_EXPORT int sem_wait(sem_t* sem);
extern U_EXPORT int sem_post(sem_t* sem);
extern U_EXPORT int sem_destroy(sem_t* sem);
extern U_EXPORT int sem_init(sem_t* sem, int pshared, unsigned value);
extern U_EXPORT int sem_getvalue(sem_t* sem, int* sval);
extern U_EXPORT int sem_timedwait(sem_t* sem, const struct timespec* abs_timeout);

int sem_wait(sem_t* sem)                                          { errno = ENOSYS; return -1; }
int sem_post(sem_t* sem)                                          { errno = ENOSYS; return -1; }
int sem_destroy(sem_t* sem)                                       { errno = ENOSYS; return -1; } 
int sem_init(sem_t* sem, int pshared, unsigned value)             { errno = ENOSYS; return -1; }
int sem_getvalue(sem_t* sem, int* sval)                           { errno = ENOSYS; return -1; }
int sem_timedwait(sem_t* sem, const struct timespec* abs_timeout) { errno = ENOSYS; return -1; }
