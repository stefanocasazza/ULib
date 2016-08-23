/**
 * system.h - windows system-dependent definitions
 *
 * Any include of <windows.h> should be through
 * this file, which wraps it in various other handling
 */

#ifndef W32_SYSTEM_H
#define W32_SYSTEM_H 1

/**
 * libstdc++-v3 _really_ dislikes min & max defined as macros
 * As of gcc 3.3.1, it defines NOMINMAX itself, so test first, to avoid a redefinition error
 *
 * #ifndef NOMINMAX
 * #define NOMINMAX
 * #endif
 */

/**
 * Require at least Internet Explorer 3, in order to have access
 * to sufficient Windows Common Controls features from <commctrl.h>
 *
 * #define _WIN32_IE 0x0300
 */

#include <_mingw.h>
#include <_mingw_off_t.h>

#include <w32api.h>
#include <windows.h>
#include <winsock2.h>

#ifndef _CRTIMP
#define _CRTIMP __declspec(dllimport)
#endif

#include <time.h>
/*
#if !HAVE_GMTIME_R && !defined(gmtime_r)
static inline struct tm* gmtime_r(const time_t* clock, struct tm* result)
{
   struct tm* ptr = gmtime(clock);

   if (ptr)
      {
      *result = *ptr;

      return result;
      }

   return 0;
}
#endif

#if !HAVE_LOCALTIME_R && !defined(localtime_r)
static inline struct tm* localtime_r(const time_t* clock, struct tm* result)
{
   struct tm* ptr = localtime(clock);

   if (ptr)
      {
      *result = *ptr;

      return result;
      }

   return 0;
}
#endif
*/

#include <stdio.h>
#include <sys/stat.h>

#ifdef _MSC_VER
typedef char* caddr_t;
typedef   signed __int8    int8_t;
typedef unsigned __int8   uint8_t;
typedef   signed __int16  int16_t;
typedef unsigned __int16 uint16_t;
typedef   signed __int32  int32_t;
typedef unsigned __int32 uint32_t;
typedef   signed __int64  int64_t;
typedef unsigned __int64 uint64_t;
#  define  snprintf  _snprintf
#  define vsnprintf _vsnprintf
#else
#  include <sys/types.h>
#  include <stdint.h>
#  include <unistd.h>
#endif

/* #define _UWIN */
#include <io.h>
#include <signal.h>
#define sighandler_t __p_sig_fn_t

/* gcc c++11 support on mingw requires pthread support library */
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 7)) && defined(__MINGW_WINPTHREAD__)
#  include <pthread.h>   /* gnu libstdc++ now requires a win pthread */
#  undef _MSCONDITIONALS_
#else
#  define _MSTHREADS_
typedef DWORD            pthread_t;
typedef CRITICAL_SECTION pthread_mutex_t;
#endif
typedef HANDLE fd_t;
typedef SOCKET socket_t;
#ifdef _MSC_VER
typedef struct timespec {
    time_t tv_sec;
    long   tv_nsec;
} timespec_t;
#endif
#ifdef __cplusplus
extern "C" {
#endif
#ifdef _MSTHREADS_
   inline void      pthread_exit(void* p) { _endthreadex((DWORD)0); }
   inline pthread_t pthread_self(void)    { return (pthread_t)GetCurrentThreadId(); }

   inline int  pthread_mutex_init(pthread_mutex_t* mutex, void* x) { InitializeCriticalSection(mutex); return 0; }
   inline void pthread_mutex_destroy(pthread_mutex_t* mutex)       {     DeleteCriticalSection(mutex); }
   inline void pthread_mutex_lock(pthread_mutex_t* mutex)          {      EnterCriticalSection(mutex); }
   inline void pthread_mutex_unlock(pthread_mutex_t* mutex)        {      LeaveCriticalSection(mutex); }
#endif
   /*
   inline char* strdup(  const char* s)                            { return _strdup(s); }
   inline int   stricmp( const char* s1, const char* s2)           { return _stricmp(s1, s2); }
   inline int   strnicmp(const char* s1, const char* s2, size_t l) { return _strnicmp(s1, s2, l); }
   */
#ifdef __cplusplus
}
#endif

#if LITTLEENDIAN == 0x0001
#  define __LITTLE_ENDIAN 1234
#  ifndef __BYTE_ORDER
#     define __BYTE_ORDER  __LITTLE_ENDIAN
#  endif
#elif BIGENDIAN == 0x0001
#  define __BIG_ENDIAN  1234
#  define __BYTE_ORDER  __BIG_ENDIAN
#endif

#define F_WRLCK   1 /* Write lock */
#define F_UNLCK   2 /* Remove lock */
#define F_GETFL   3 /* Get file status flags */
#define F_SETFL   4 /* Set file status flags */
#define F_DUPFD   0 /* Duplicate file descriptor */
#define F_GETFD   1 /* Get file descriptor flags */
#define F_SETFD   2 /* Set file descriptor flags */
#define F_GETLK   5 /* Get record locking info */
#define F_SETLK   6 /* Set record locking info (non-blocking) */
#define F_SETLKW  7 /* Set record locking info (blocking) */
#define F_SETOWN  8 /* Get owner of socket (receiver of SIGIO) */
#define F_GETOWN  9 /* Set owner of socket (receiver of SIGIO) */

#define FD_CLOEXEC      1 /* actually anything with low bit set goes */
#define O_NONBLOCK      04000
#define O_CLOEXEC       02000000 /* Set close_on_exec */
#define S_ISSOCK(mode)  0

typedef int pid_t;

struct flock {
   short l_type;
   short l_whence;
   off_t l_start;
   off_t l_len;    /* len == 0 means until end of file */
   long  l_sysid;
   pid_t l_pid;
   long  l_pad[4]; /* reserve area */
};

/* POSIX macros for evaluating exit statuses */

#ifndef WIFSTOPPED
#define WIFSTOPPED(w) (((w) & 0xff) == 0x7f)
#endif
#ifndef WIFSIGNALED
#define WIFSIGNALED(w) (((w) & 0xff) != 0x7f && ((w) & 0xff) != 0)
#endif
#ifndef WIFEXITED
#define WIFEXITED(w) (((w) & 0xff) == 0)
#endif
#ifndef WSTOPSIG
#define WSTOPSIG(w) (((w) >> 8) & 0xff)
#endif
#ifndef WTERMSIG
#define WTERMSIG(w) ((w) & 0x7f)
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(w) (((w) >> 8) & 0xff)
#endif

/* Protection bits */

#ifndef S_IRUSR
#define S_IRUSR  0400  /* Read by owner */
#define S_IWUSR  0200  /* Write by owner */
#define S_IXUSR  0100  /* Execute by owner */
/* Read, write, and execute by owner */
#define S_IRWXU  (S_IRUSR|S_IWUSR|S_IXUSR)
#endif

#ifndef S_IREAD
#define S_IREAD  S_IRUSR
#define S_IWRITE S_IWUSR
#define S_IEXEC  S_IXUSR
#endif

#ifndef S_IRGRP
#define S_IRGRP  (S_IRUSR >> 3) /* Read by group */
#define S_IWGRP  (S_IWUSR >> 3) /* Write by group */
#define S_IXGRP  (S_IXUSR >> 3) /* Execute by group */
/* Read, write, and execute by group.  */
#define S_IRWXG  (S_IRWXU >> 3)

#define S_IROTH  (S_IRGRP >> 3) /* Read by others */
#define S_IWOTH  (S_IWGRP >> 3) /* Write by others */
#define S_IXOTH  (S_IXGRP >> 3) /* Execute by others */
/* Read, write, and execute by others. */
#define S_IRWXO  (S_IRWXG >> 3)
#endif

#define mkdir(path,mode) mkdir(path)
/* win32 doesn't have symbolic link, so just #define it to stat */
#define lstat(x,y) stat(x, y)
/* win32 doesn't have the same kind of sleep as UNIX, so make a version that behaves the same */
#define sleep(seconds) (Sleep(seconds * 1000))
/* Make all changes done to FD actually appear on disk */
#define fsync(fd) _commit(fd)
/* Determines whether a file descriptor is associated with a character device */
#define isatty(fd) _isatty(fd)

/*** Environment Variables ***/

#define setenv(__pname,__pvalue,___overwrite) ({ int result; \
 if (___overwrite == 0 && getenv (__pname)) result = 0; \
 else                                       result = SetEnvironmentVariable (__pname,__pvalue); \
 result; })

#define unsetenv(pname) SetEnvironmentVariable(pname, NULL)

/**
 * These depend upon the type of sigset_t, which right now is always a long...
 * They're in the POSIX namespace, but are not ANSI
 */
#define sigset_t long
#define sigemptyset(what)   (*(what) = 0)
#define sigaddset(what,sig) (*(what) |= (1<<(sig)))

#define SIGHUP     1
#define SIGQUIT    3 /* Quit (POSIX) */
#define SIGKILL    9 /* Kill, unblockable (POSIX) */
#define SIGBUS    10 /* ,7,10 */
#define SIGPIPE   13
#define SIGALRM   14
#define SIGUSR1   16 /* ,30,10 */
#define SIGUSR2   17 /* ,31,12 */
#define SIGCHLD   20 /* ,17,18 */
#define SIGWINCH  28

#ifndef SIG_SETMASK
#define SIG_SETMASK   0    /* set mask with sigprocmask() */
#endif
#ifndef SIG_BLOCK
#define SIG_BLOCK     1    /* set of signals to block */
#endif
#ifndef SIG_UNBLOCK 
#define SIG_UNBLOCK   2    /* set of signals to, well, unblock */
#endif
#ifndef SA_NOCLDWAIT
#define SA_NOCLDWAIT  2
#endif

#define SHUT_RD       0
#define SHUT_WR       1
#define WNOHANG       1    /* Don't block waiting */
#define WUNTRACED     2    /* Report status of stopped children */
#define ITIMER_REAL   1
#define ITIMER_PROF   2
#define RLIMIT_DATA   2    /* max data size */
#define RLIMIT_STACK  3    /* max stack size */
#define MS_SYNC       4    /* Synchronous memory sync */
#define MS_ASYNC      1    /* Sync memory asynchronously */
#define MS_INVALIDATE 2    /* Invalidate the caches  */
#define PROT_NONE     0x0  /* page can not be accessed */
#define PROT_READ     0x1  /* page can be read */
#define PROT_WRITE    0x2  /* Page can be written */
#define PROT_EXEC     0x4  /* page can be executed */
#define MAP_FIXED     0x10 /* Interpret addr exactly */
#define MAP_PRIVATE   0x02 /* Changes are private */
#define MAP_SHARED    0x01 /* Share changes */
#define MAP_ANONYMOUS 0x20 /* Don't use a file */
#define MAP_ANON      MAP_ANONYMOUS
#define MAP_FAILED    ((void*)-1)
/* These are Linux-specific.  */
#define MAP_LOCKED    0x02000  /* Lock the mapping */
#define MAP_NORESERVE 0x04000  /* Don't check for reservations */
#define MAP_POPULATE  0x08000  /* Populate (prefault) pagetables */

#define ENOTSUP       48 /* This is the value in Solaris */
#define EBADRQC       54 /* "Invalid request code" */
#define ENODATA       61 /* "No data (for no delay io)" */
#define ENONET        64 /* "Machine is not on the network" */
#define ENOLINK       67 /* "The link has been severed" */
#define ECOMM         70 /* "Communication error on send" */
#define ENOTUNIQ      80 /* "Given log. name not unique" */
#define ENMFILE       89 /* "No more files" */
#define EMSGSIZE      90 /* Message too long */
#define ENOMEDIUM    135 /* "no medium" */
#define ENOSHARE     136 /* "No such host or network path" */
#define EOVERFLOW    139 /* "Value too large for defined data type" */

#ifndef ETIMEDOUT
#define ETIMEDOUT    110 /* Connection timed out */
#endif

#define EINPROGRESS  115 /* Operation now in progress */

struct sigaction {          /* Structure describing the action to be taken when a signal arrives */
   sighandler_t sa_handler; /* Signal handler */
   sigset_t     sa_mask;    /* Additional set of signals to be blocked */
   int          sa_flags;   /* Special flags */
};

typedef int    rlim_t;
typedef char* caddr_t;

struct passwd {
   char* pw_name;    /* user name */
   char* pw_passwd;  /* user password */
   uid_t pw_uid;     /* user id */
   gid_t pw_gid;     /* group id */
   char* pw_gecos;   /* real name */
   char* pw_dir;     /* home directory */
   char* pw_shell;   /* shell program */
};

struct itimerval {
   struct timeval it_interval; /* next value */
   struct timeval it_value;    /* current value */
};

struct iovec {
   void*  iov_base; /* Pointer to data */
   size_t iov_len;  /* Length of data  */
};

struct rlimit {
   rlim_t rlim_cur;   /* Soft limit */
   rlim_t rlim_max;   /* Hard limit (ceiling for rlim_cur) */
};

#ifdef __cplusplus
extern "C" {
#endif
U_EXPORT pid_t  fork(void);
U_EXPORT pid_t vfork(void);
U_EXPORT uid_t getuid(void);
U_EXPORT uid_t geteuid(void);
U_EXPORT uid_t getegid(void);
U_EXPORT pid_t getppid(void);

U_EXPORT int nice(int inc);
U_EXPORT int setpgrp(void);
U_EXPORT int setuid(uid_t uid);
U_EXPORT int setgid(gid_t gid);
U_EXPORT int mkstemp(char* name);
U_EXPORT int pipe(int filedes[2]);
U_EXPORT int kill(pid_t pid, int sig);
U_EXPORT int sigpending(sigset_t* set);
U_EXPORT int setpgid(pid_t pid, pid_t pgid);
U_EXPORT int sigsuspend(const sigset_t* mask);
U_EXPORT int munmap(void* start, size_t length);
U_EXPORT int msync(void* start, size_t length, int flags);
U_EXPORT int inet_aton(const char *cp, struct in_addr *addr);
U_EXPORT int setrlimit(int resource, const struct rlimit* rlim);
U_EXPORT int socketpair(int d, int type, int protocol, int sv[2]);
U_EXPORT int sigprocmask(int how, const sigset_t* set, sigset_t* oldset);
U_EXPORT int sigaction(int signum, const struct sigaction* act, struct sigaction* oldact);
U_EXPORT int setitimer(int which, const struct itimerval* value, struct itimerval* ovalue);

U_EXPORT struct passwd* getpwuid(uid_t uid);
U_EXPORT struct passwd* getpwnam(const char* name);

U_EXPORT pid_t waitpid(pid_t pid, int* status, int options);
U_EXPORT char* realpath(const char* path, char* resolved_path);
U_EXPORT void* mmap(void* start, size_t length, int prot, int flags, int fd, off_t offset);

U_EXPORT unsigned int alarm(unsigned int seconds);
U_EXPORT ssize_t writev(int fd, const struct iovec* vector, int count);

/**
 * implemented in MINGW Runtime
 *
 * U_EXPORT int gettimeofday(struct timeval* tv, void* tz);
 * U_EXPORT int truncate(const char* fname, off_t distance);
 */

#if _WIN32_WINNT < 0x0600
U_EXPORT const CHAR* inet_ntop(INT, PVOID, LPSTR, size_t);
#endif

U_EXPORT int  raise_w32(int nsig);
U_EXPORT int unlink_w32(const char* path);
U_EXPORT int  fcntl_w32(int fd, int cmd, void* arg);
U_EXPORT int rename_w32(const char* oldpath, const char* newpath);
U_EXPORT int select_w32(int nfds, fd_set* rd, fd_set* wr, fd_set* ex, struct timeval* timeout);

U_EXPORT HANDLE is_pipe(int fd);
U_EXPORT void u_init_ulib_mingw(void);
U_EXPORT uint64_t u_get_inode(int fd); /* INODE FOR WINDOWS - It is not stable for files on network drives (NFS) */
U_EXPORT const char* getSysError_w32(unsigned* len);
U_EXPORT int w32_open_osfhandle(long osfhandle, int flags);
U_EXPORT sighandler_t signal_w32(int nsig, sighandler_t handler);
U_EXPORT char* u_slashify(const char* src, char slash_from, char slash_to);

extern U_EXPORT HANDLE u_hProcess;
extern U_EXPORT SECURITY_ATTRIBUTES sec_none;
extern U_EXPORT SECURITY_DESCRIPTOR sec_descr;
#ifdef __cplusplus
}
#endif

#define raise(a) raise_w32(a)
#define unlink(a) unlink_w32(a)
#define signal(a,b) signal_w32(a,b)
#define rename(a,b) rename_w32(a,b)
#define fcntl(a,b,c) fcntl_w32(a,b,(void*)(c))

#define select(a,b,c,d,e) select_w32(a,b,c,d,e)

/* Perform a mapping of Win32 error numbers into POSIX errno's */

#ifndef ELOOP
#define ELOOP            40 /* Too many symbolic links encountered */
#endif
#ifndef ENOSR
#define ENOSR            63 /* Out of streams resources */
#endif
#ifndef EUSERS
#define EUSERS           87 /* Too many users */
#endif
#ifndef ENOTSOCK
#define ENOTSOCK         88 /* Socket operation on non-socket */
#endif
#ifndef EDESTADDRREQ
#define EDESTADDRREQ     89 /* Destination address required */
#endif
#ifndef EPROTOTYPE
#define EPROTOTYPE       91 /* Protocol wrong type for socket */
#endif
#ifndef ENOPROTOOPT
#define ENOPROTOOPT      92 /* Protocol not available */
#endif
#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT  93 /* Protocol not supported */
#endif
#ifndef ESOCKTNOSUPPORT
#define ESOCKTNOSUPPORT  94 /* Socket type not supported */
#endif
#ifndef EOPNOTSUPP
#define EOPNOTSUPP       95 /* Operation not supported on transport endpoint */
#endif
#ifndef EPFNOSUPPORT
#define EPFNOSUPPORT     96 /* Protocol family not supported */
#endif
#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT     97 /* Address family not supported by protocol */
#endif
#ifndef EADDRINUSE
#define EADDRINUSE       98 /* Address already in use */
#endif
#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL    99 /* Cannot assign requested address */
#endif
#ifndef ENETDOWN
#define ENETDOWN        100 /* Network is down */
#endif
#ifndef ENETUNREACH
#define ENETUNREACH     101 /* Network is unreachable */
#endif
#ifndef ENETRESET
#define ENETRESET       102 /* Network dropped connection because of reset */
#endif
#ifndef ECONNABORTED
#define ECONNABORTED    103 /* Software caused connection abort */
#endif
#ifndef ECONNRESET
#define ECONNRESET      104 /* Connection reset by peer */
#endif
#ifndef ENOBUFS
#define ENOBUFS         105 /* No buffer space available */
#endif
#ifndef EISCONN
#define EISCONN         106 /* Transport endpoint is already connected */
#endif
#ifndef ENOTCONN
#define ENOTCONN        107 /* Transport endpoint is not connected */
#endif
#ifndef ESHUTDOWN
#define ESHUTDOWN       108 /* Cannot send after transport endpoint shutdown */
#endif
#ifndef ETOOMANYREFS
#define ETOOMANYREFS    109 /* Too many references: cannot splice */
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED    111 /* Connection refused */
#endif
#ifndef EHOSTDOWN
#define EHOSTDOWN       112 /* Host is down */
#endif
#ifndef EHOSTUNREACH
#define EHOSTUNREACH    113 /* No route to host */
#endif
#ifndef EALREADY
#define EALREADY        114 /* Operation already in progress */
#endif
#ifndef ESTALE
#define ESTALE          116 /* Stale NFS file handle */
#endif
#ifndef EREMOTE
#define EREMOTE         121 /* Remote I/O error */
#endif
#ifndef EDQUOT
#define EDQUOT          122 /* Quota exceeded */
#endif

#define MAP_WIN32_ERROR_TO_POSIX \
  switch (errno) {\
  case ERROR_BAD_PATHNAME:\
  case ERROR_INVALID_NAME:\
  case ERROR_FILE_NOT_FOUND:\
  case ERROR_PATH_NOT_FOUND:\
  errno = ENOENT; break;\
  case ERROR_OUTOFMEMORY:\
  case ERROR_BUFFER_OVERFLOW:\
  case ERROR_NOT_ENOUGH_MEMORY:\
  errno = ENOMEM; break;\
  case ERROR_ACCESS_DENIED:\
  case ERROR_LOCK_VIOLATION:\
  case ERROR_SHARING_VIOLATION:\
  errno = EACCES; break;\
  case ERROR_FILE_EXISTS:\
  case ERROR_ALREADY_EXISTS:\
  errno = EEXIST; break;\
  case WSAEWOULDBLOCK:\
  case ERROR_MORE_DATA:\
  case ERROR_OPEN_FILES:\
  case ERROR_NO_PROC_SLOTS:\
  case ERROR_DEVICE_IN_USE:\
  case ERROR_MAX_THRDS_REACHED:\
  case ERROR_ACTIVE_CONNECTIONS:\
  errno = EAGAIN; break;\
  case WSAEINVAL:\
  case ERROR_NO_TOKEN:\
  case ERROR_BAD_PIPE:\
  case ERROR_BAD_USERNAME:\
  case ERROR_INVALID_DATA:\
  case ERROR_NEGATIVE_SEEK:\
  case ERROR_THREAD_1_INACTIVE:\
  case ERROR_INVALID_PARAMETER:\
  case ERROR_FILENAME_EXCED_RANGE:\
  case ERROR_INVALID_SIGNAL_NUMBER:\
  case ERROR_META_EXPANSION_TOO_LONG:\
  errno = EINVAL; break;\
  case ERROR_CRC:\
  case ERROR_IO_DEVICE:\
  case ERROR_OPEN_FAILED:\
  case ERROR_SIGNAL_REFUSED:\
  case ERROR_NO_SIGNAL_SENT:\
  errno = EIO; break;\
  case ERROR_NO_DATA:\
  case ERROR_BROKEN_PIPE:\
  errno = EPIPE; break;\
  case ERROR_BUSY:\
  case ERROR_PIPE_BUSY:\
  case ERROR_SIGNAL_PENDING:\
  case ERROR_PIPE_CONNECTED:\
  case ERROR_CHILD_NOT_COMPLETE:\
  errno = EBUSY; break;\
  case ERROR_BAD_UNIT:\
  case ERROR_BAD_DEVICE:\
  case ERROR_INVALID_DRIVE:\
  errno = ENODEV; break;\
  case ERROR_DISK_FULL:\
  case ERROR_EOM_OVERFLOW:\
  case ERROR_END_OF_MEDIA:\
  case ERROR_HANDLE_DISK_FULL:\
  case ERROR_NO_DATA_DETECTED:\
  errno = ENOSPC; break;\
  case ERROR_NOT_OWNER:\
  case ERROR_CANNOT_MAKE:\
  errno = EPERM; break;\
  case WSAEFAULT:\
  case ERROR_NOACCESS:\
  case ERROR_INVALID_ADDRESS:\
  case ERROR_PROCESS_ABORTED:\
  errno = EFAULT; break;\
  case ERROR_NOT_SUPPORTED:\
  case ERROR_CALL_NOT_IMPLEMENTED:\
  errno = ENOSYS; break;\
  case ERROR_PIPE_LISTENING:\
  case ERROR_PIPE_NOT_CONNECTED:\
  errno = ECOMM; break;\
  case ERROR_BAD_NETPATH:\
  case ERROR_BAD_NET_NAME:\
  errno = ENOSHARE; break;\
  case ERROR_SETMARK_DETECTED:\
  case ERROR_BEGINNING_OF_MEDIA:\
  errno = ESPIPE; break;\
  case ERROR_TOO_MANY_OPEN_FILES:\
  errno = EMFILE; break;\
  case ERROR_INVALID_HANDLE:\
  errno = EBADF; break;\
  case ERROR_NOT_SAME_DEVICE:\
  errno = EXDEV; break;\
  case ERROR_WRITE_PROTECT:\
  errno = EROFS; break;\
  case ERROR_SHARING_BUFFER_EXCEEDED:\
  errno = ENOLCK; break;\
  case ERROR_NO_MORE_SEARCH_HANDLES:\
  errno = ENFILE; break;\
  case ERROR_WAIT_NO_CHILDREN:\
  errno = ECHILD; break;\
  case WSAENOTEMPTY:\
  case ERROR_DIR_NOT_EMPTY:\
  errno = ENOTEMPTY; break;\
  case ERROR_DIRECTORY:\
  errno = ENOTDIR; break;\
  case WSAEINTR:\
  case ERROR_INVALID_AT_INTERRUPT_TIME:\
  errno = EINTR; break;\
  case ERROR_FILE_INVALID:\
  errno = ENXIO; break;\
  case ERROR_INVALID_FUNCTION:\
  errno = EBADRQC; break;\
  case ERROR_NO_MORE_FILES:\
  errno = ENMFILE; break;\
  case ERROR_HANDLE_EOF:\
  errno = ENODATA; break;\
  case ERROR_REM_NOT_LIST:\
  errno = ENONET; break;\
  case ERROR_DUP_NAME:\
  errno = ENOTUNIQ; break;\
  case ERROR_NOT_CONNECTED:\
  errno = ENOLINK; break;\
  case ERROR_POSSIBLE_DEADLOCK:\
  errno = EDEADLOCK; break;\
  case ERROR_NOT_READY:\
  errno = ENOMEDIUM; break;\
  case WSAEINPROGRESS:\
  errno = EINPROGRESS; break;\
  case WSAETIMEDOUT:\
  errno = ETIMEDOUT; break;\
  case WSAEMSGSIZE:\
  errno = EMSGSIZE; break;\
  case WSAENAMETOOLONG:\
  errno = ENAMETOOLONG; break;\
  case WSAEALREADY:\
  errno = EALREADY; break;\
  case WSAENOTSOCK:\
  errno = ENOTSOCK; break;\
  case WSAEDESTADDRREQ:\
  errno = EDESTADDRREQ; break;\
  case WSAEPROTOTYPE:\
  errno = EPROTOTYPE; break;\
  case WSAENOPROTOOPT:\
  errno = ENOPROTOOPT; break;\
  case WSAEPROTONOSUPPORT:\
  errno = EPROTONOSUPPORT; break;\
  case WSAESOCKTNOSUPPORT:\
  errno = ESOCKTNOSUPPORT; break;\
  case WSAEOPNOTSUPP:\
  errno = EOPNOTSUPP; break;\
  case WSAEPFNOSUPPORT:\
  errno = EPFNOSUPPORT; break;\
  case WSAEAFNOSUPPORT:\
  errno = EAFNOSUPPORT; break;\
  case WSAEADDRINUSE:\
  errno = EADDRINUSE; break;\
  case WSAEADDRNOTAVAIL:\
  errno = EADDRNOTAVAIL; break;\
  case WSAENETDOWN:\
  errno = ENETDOWN; break;\
  case WSAENETUNREACH:\
  errno = ENETUNREACH; break;\
  case WSAENETRESET:\
  errno = ENETRESET; break;\
  case WSAECONNABORTED:\
  errno = ECONNABORTED; break;\
  case WSAECONNRESET:\
  errno = ECONNRESET; break;\
  case WSAENOBUFS:\
  errno = ENOBUFS; break;\
  case WSAEISCONN:\
  errno = EISCONN; break;\
  case WSAENOTCONN:\
  errno = ENOTCONN; break;\
  case WSAESHUTDOWN:\
  errno = ESHUTDOWN; break;\
  case WSAETOOMANYREFS:\
  errno = ETOOMANYREFS; break;\
  case WSAECONNREFUSED:\
  errno = ECONNREFUSED; break;\
  case WSAELOOP:\
  errno = ELOOP; break;\
  case WSAEHOSTDOWN:\
  errno = EHOSTDOWN; break;\
  case WSAEHOSTUNREACH:\
  errno = EHOSTUNREACH; break;\
  case WSAEPROCLIM:\
  errno = ENOSR; break;\
  case WSAEUSERS:\
  errno = EUSERS; break;\
  case WSAEDQUOT:\
  errno = EDQUOT; break;\
  case WSAESTALE:\
  errno = ESTALE; break;\
  case WSAEREMOTE:\
  errno = EREMOTE; break;\
  }

#endif
