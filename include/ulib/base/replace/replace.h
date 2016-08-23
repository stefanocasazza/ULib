/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    replace.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_REPLACE_H
#define ULIB_REPLACE_H 1

#ifndef HAVE_SCHED_GETCPU
#  ifdef __cplusplus
extern "C" {
#  endif
int sched_getcpu(void);
#  ifdef __cplusplus
}
#  endif
#endif

#ifndef HAVE_NANOSLEEP
#  ifdef __cplusplus
extern "C" {
#  endif
int nanosleep(const struct timespec* req, struct timespec* rem);
#  ifdef __cplusplus
}
#  endif
#endif

#if !defined(HAVE_SENDFILE) && !defined(HAVE_MACOSX_SENDFILE)
#  ifdef __cplusplus
extern "C" {
#  endif
ssize_t sendfile(int out_fd, int in_fd, off_t* poffset, size_t count);
#  ifdef __cplusplus
}
#  endif
#endif

#if !defined(HAVE_MREMAP) && !defined(__UNIKERNEL__)
#  ifdef __cplusplus
extern "C" {
#  endif
void* mremap(void* old_address, size_t old_size , size_t new_size, int flags);
#  ifdef __cplusplus
}
#  endif
#endif

#ifndef HAVE_STRNDUP
#  ifdef __cplusplus
extern "C" {
#  endif
char* strndup(const char* s, size_t n);
#  ifdef __cplusplus
}
#  endif
#endif

#ifndef HAVE_STRPTIME
#  ifdef __cplusplus
extern "C" {
#  endif
char* strptime(const char* buf, const char* fmt, struct tm* tm);
#  ifdef __cplusplus
}
#  endif
#endif

#ifndef HAVE_MKDTEMP
#  ifdef __cplusplus
extern "C" {
#  endif
char* mkdtemp(char* template_name);
#  ifdef __cplusplus
}
#  endif
#endif

#ifndef HAVE_MEMMEM
#  ifdef __cplusplus
extern "C" {
#  endif
void* memmem(const void* haystack, size_t haystacklen, const void* needle, size_t needlelen);
#  ifdef __cplusplus
}
#  endif
#endif

#ifndef HAVE_MEMRCHR
#  ifdef __cplusplus
extern "C" {
#  endif
void* memrchr(const void* s, int c, size_t count);
#  ifdef __cplusplus
}
#  endif
#endif

#ifndef HAVE_GMTIME_R
struct tm;
#  ifdef __cplusplus
extern "C" {
#  endif
struct tm* gmtime_r(const time_t* timep, struct tm* result);
#  ifdef __cplusplus
}
#  endif
#endif

#ifndef HAVE_TIMEGM
#  ifdef __cplusplus
extern "C" {
#  endif
time_t timegm(struct tm* tm);
#  ifdef __cplusplus
}
#  endif
#endif

#ifndef HAVE_DAEMON
#  ifdef __cplusplus
extern "C" {
#  endif
int daemon(int nochdir, int noclose);
#  ifdef __cplusplus
}
#  endif
#endif

#ifndef HAVE_PREAD
#  ifdef __cplusplus
extern "C" {
#  endif
ssize_t pread(int fd, void *buf, size_t count, off_t offset);
#  ifdef __cplusplus
}
#  endif
#endif

#ifndef HAVE_PREAD_PWRITE
#  ifdef __cplusplus
extern "C" {
#  endif
ssize_t pwrite(int fd, const void* buf, size_t count, off_t offset);
#  ifdef __cplusplus
}
#  endif
#endif

#if defined(HAVE_SEM_INIT) && !defined(HAVE_SEM_TIMEDWAIT)
#  include <semaphore.h>
#  ifdef __cplusplus
extern "C" {
#  endif
int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);
#  ifdef __cplusplus
}
#  endif
#endif

#ifndef HAVE_FNMATCH
#  ifdef __cplusplus
extern "C" {
#  endif
/* Bits set in the FLAGS argument to 'fnmatch'  */
#define FNM_PATHNAME    (1 << 0) /* No wildcard can ever match '/' */
#define FNM_NOESCAPE    (1 << 1) /* Backslashes don't quote special chars */
#define FNM_PERIOD      (1 << 2) /* Leading '.' is matched only explicitly */
#define FNM_LEADING_DIR (1 << 3) /* Ignore '/...' after a match */
#define FNM_CASEFOLD    (1 << 4) /* Compare without regard to case */

#define FNM_NOMATCH 1 /* Value returned by `fnmatch' if STRING does not match PATTERN */
#define FNM_NOSYS (-1)

int fnmatch(const char*, const char*, int);
#  ifdef __cplusplus
}
#  endif
#endif

#ifndef HAVE_FALLOCATE
#  ifdef __cplusplus
extern "C" {
#  endif
int fallocate(int fd, int mode, off_t offset, off_t len);
#  ifdef __cplusplus
}
#  endif
#endif

#endif

