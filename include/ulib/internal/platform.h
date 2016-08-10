/* platform.h */

#ifndef ULIB_PLATFORM_H
#define ULIB_PLATFORM_H

#ifndef _REENTRANT
#define _REENTRANT 1
#endif

#ifndef __PTH__
#  ifndef _THREADSAFE
#  define _THREADSAFE 1
#  endif
#  ifndef _POSIX_PTHREAD_SEMANTICS
#  define _POSIX_PTHREAD_SEMANTICS
#  endif
#endif

/* see if targeting legacy Microsoft windows platform */

#if defined(LINUX) || defined(__LINUX__) || defined(__linux__) || defined(__linux)
#  define U_LINUX
#elif defined(_MSC_VER) || defined(WIN32) || defined(_WIN32)
#  define _MSWINDOWS_
#  if defined(_MSC_VER)
#     define NOMINMAX
#  endif
#  if defined(_M_X64) || defined(_M_ARM)
#     define _MSCONDITIONALS_
#     ifndef _WIN32_WINNT
#     define _WIN32_WINNT 0x0600
#     endif
#  endif
#  ifdef  _MSC_VER
#     pragma warning(disable: 4251)
#     pragma warning(disable: 4996)
#     pragma warning(disable: 4355)
#     pragma warning(disable: 4290)
#     pragma warning(disable: 4291)
#  endif
#  if defined(__BORLANDC__) && !defined(__MT__)
#     error Please enable multithreading
#  endif
#  if defined(_MSC_VER) && !defined(_MT)
#     error Please enable multithreading (Project -> Settings -> C/C++ -> Code Generation -> Use Runtime Library)
#  endif
/* Require for compiling with critical sections */
#  ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x0600
#  endif
/* Make sure we're consistent with _WIN32_WINNT */
#  ifndef WINVER
#  define WINVER _WIN32_WINNT
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#  endif
#endif

#ifdef __GNUC__
#  if    __GNUC__ > 1
#     if __GNUC__ < 3
#        ifndef _GNU_SOURCE
#        define _GNU_SOURCE
#        endif
#     else
#        define __PRINTF(x,y) __attribute__ ((format (printf, x, y)))
#        define __SCANF(x, y) __attribute__ ((format (scanf, x, y)))
#        define __MALLOC      __attribute__ ((malloc))
#     endif
#     if !defined(__STRICT_ANSI__) && !defined(__PEDANTIC__)
#        define DYNAMIC_LOCAL_ARRAYS
#        ifndef __cplusplus
#           include <stdbool.h> /* C99 only */
#        endif
#     endif
#  endif
#  ifdef HAVE_BUILTIN_CPU_INIT
#     define U_CPU_SUPPORT(x,func) __attribute__ ((target (#x))) func
#  else
#     define U_CPU_SUPPORT(x,func) func_##x
#  endif
/* GCC have printf type attribute check */
#  define GCC_VERSION_NUM (__GNUC__       * 10000 + \
                           __GNUC_MINOR__ *   100 + \
                           __GNUC_PATCHLEVEL__)
#  if GCC_VERSION_NUM > 29600 && GCC_VERSION_NUM != 30303 /* Test for GCC == 3.3.3 (SuSE Linux) */
#    if defined(U_LINUX) || defined(_MSWINDOWS_)
#     define __pure                       __attribute__((pure))
#    endif
#     define LIKELY(x)                    __builtin_expect(!!(x), 1)
#     define UNLIKELY(x)                  __builtin_expect(!!(x), 0)
#     define __noreturn                   __attribute__((noreturn))
#     define PRINTF_ATTRIBUTE(a,b)        __attribute__((__format__(printf, a, b)))
#     define PREFETCH_ATTRIBUTE(addr,rw)  __builtin_prefetch(addr, rw, 1);
#  else
#     define __pure
#     define __noreturn
#     define LIKELY(x) (x)
#     define UNLIKELY(x) (x)
#     define PRINTF_ATTRIBUTE(a,b)
#     define PREFETCH_ATTRIBUTE(addr,rw)
/**
 * Mark functions as cold. gcc will assume any path leading to a call to them will be unlikely.
 * gcc also has a __attribute__((__hot__)) to move hot functions into a special section
 */
#     if GCC_VERSION_NUM < 40300
#        define __hot
#        define __cold
#     else
#        define __hot  __attribute__((hot))
#        define __cold __attribute__((cold))
#     endif
#  endif
/**
 * ---------------------------------------------------------
 * C++0x features supported in GCC:
 * ---------------------------------------------------------
 * g++ -E -dM -std=c++98 -x c++ /dev/null > std1 &&
 * g++ -E -dM -std=c++0x -x c++ /dev/null > std2 &&
 * diff -u std1 std2 | grep '[+|-]^*#define' && rm std1 std2
 * ---------------------------------------------------------
 * Output with 4.8.0:
 * ---------------------------------------------------------
 * +#define __GXX_EXPERIMENTAL_CXX0X__ 1
 * -#define __cplusplus 199711L
 * +#define __cplusplus 201103L
 * +#define __GNUC_STDC_INLINE__ 1
 * -#define __GNUC_GNU_INLINE__ 1
 * ---------------------------------------------------------
 */
#  if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#    if (__GNUC__ * 100 + __GNUC_MINOR__) >= 405
       /* C++11 features supported in GCC 4.5: */
#      define U_COMPILER_DECLTYPE
#      define U_COMPILER_STATIC_ASSERT
#      define U_COMPILER_VARIADIC_MACROS
#      define U_COMPILER_ATOMICS
#      define U_COMPILER_AUTO_FUNCTION
#      define U_COMPILER_AUTO_TYPE
#      define U_COMPILER_CLASS_ENUM
#      define U_COMPILER_DEFAULT_MEMBERS
#      define U_COMPILER_DELETE_MEMBERS
#      define U_COMPILER_EXTERN_TEMPLATES
#      define U_COMPILER_INITIALIZER_LISTS
#      define U_COMPILER_UNICODE_STRINGS
#      define U_COMPILER_VARIADIC_TEMPLATES
#      define U_COMPILER_LAMBDA
#      define U_COMPILER_RAW_STRINGS
#    endif
#    if (__GNUC__ * 100 + __GNUC_MINOR__) >= 406
       /* C++11 features supported in GCC 4.6: */
#      define U_COMPILER_CONSTEXPR
#      define U_COMPILER_NULLPTR
#      define U_COMPILER_RVALUE_REFS
#      define U_COMPILER_UNRESTRICTED_UNIONS
#      define U_COMPILER_RANGE_FOR
#    endif
#    if (__GNUC__ * 100 + __GNUC_MINOR__) >= 407
       /* GCC 4.6.x has problems dealing with noexcept expressions,
        * so turn the feature on for 4.7 and above, only */
#      define U_COMPILER_NOEXCEPT
       /* C++11 features supported in GCC 4.7: */
#      define U_COMPILER_NONSTATIC_MEMBER_INIT
#      define U_COMPILER_DELEGATING_CONSTRUCTORS
#      define U_COMPILER_EXPLICIT_OVERRIDES
#      define U_COMPILER_TEMPLATE_ALIAS
#      define U_COMPILER_UDL
#    endif
#    if (__GNUC__ * 100 + __GNUC_MINOR__) >= 408
#      define U_COMPILER_ATTRIBUTES
#      define U_COMPILER_ALIGNAS
#      define U_COMPILER_ALIGNOF
#      define U_COMPILER_INHERITING_CONSTRUCTORS
#      define U_COMPILER_THREAD_LOCAL
#    endif
#  endif
#else
#  define GCC_VERSION_NUM 0
#  define PRINTF_ATTRIBUTE(a,b)
#  define __pure
#  define __hot            
#  define __cold           
#  define LIKELY(x)   (x)
#  define UNLIKELY(x) (x)
#  define U_CPU_SUPPORT(x,func) func_##x
#  define __noreturn
#  define PREFETCH_ATTRIBUTE(addr,rw)
#  if !defined(FLEX_SCANNER) && !defined(__FreeBSD__) && !defined(MACOSX) /* _POSIX_SOURCE too restrictive */
/**
 * Header to request specific standard support. Before including it, one
 * of the following symbols must be defined (1003.1-1988 isn't supported):
 *
 * SUV_POSIX1990  for 1003.1-1990
 * SUV_POSIX1993  for 1003.1b-1993 - real-time
 * SUV_POSIX1996  for 1003.1-1996
 * SUV_SUS1       for Single UNIX Specification, v. 1 (UNIX 95)
 * SUV_SUS2       for Single UNIX Specification, v. 2 (UNIX 98)
 * SUV_SUS3       for Single UNIX Specification, v. 3
 */
#  undef  SUV_POSIX1990
#  undef  SUV_POSIX1993
#  undef  SUV_POSIX1996
#  undef  SUV_SUS1
#  define SUV_SUS2 1
#  undef  SUV_SUS3
#  include <ulib/internal/suvreq.h>
#  endif
#endif /* __GNUC__ */

#ifndef __MALLOC
#define __MALLOC
#define __SCANF(x, y)
#define __PRINTF(x, y)
#endif

#ifndef DEBUG
#  ifndef NDEBUG
#  define NDEBUG
#  endif
#endif
#ifdef DEBUG
#  ifdef NDEBUG
#  undef NDEBUG
#  endif
#endif

/**
 * Shared library support
 *
 * Visibility is available for GCC newer than 3.4. See: http://gcc.gnu.org/bugzilla/show_bug.cgi?id=9283
 *
 * The U_NO_EXPORT macro marks the symbol of the given variable to be hidden. A hidden symbol is stripped
 * during the linking step, so it can't be used from outside the resulting library, which is similar to
 * static. However, static limits the visibility to the current compilation unit. Hidden symbols can still
 * be used in multiple compilation units.
 *
 * \code
 * int U_EXPORT bar;
 * int U_NO_EXPORT foo;
 * \end
 */

#ifdef _MSWINDOWS_
#  define U_EXPORT __declspec(dllexport)
#  define U_NO_EXPORT
#elif defined(HAVE_GNUC_VISIBILTY)
#  define U_EXPORT    __attribute__ ((visibility("default")))
#  define U_NO_EXPORT __attribute__ ((visibility("hidden")))
#else
#  define U_EXPORT
#  define U_NO_EXPORT
#endif

/**
 * Note that IS_ABSOLUTE_PATH accepts d:foo as well, although it is only semi-absolute. This is because the users of IS_ABSOLUTE_PATH
 * want to know whether to prepend the current working directory to a file name, which should not be done with a name like d:foo
 */

#ifdef _MSWINDOWS_
#  define PATH_SEPARATOR        '\\'
#  define IS_DIR_SEPARATOR(c)   ((c) == '/' || (c) == '\\')
#  define IS_ABSOLUTE_PATH(f)   (IS_DIR_SEPARATOR((f)[0]) || (((f)[0]) && ((f)[1] == ':')))
#  define U_PATH_CONV(s)        u_slashify(s, '/', '\\')
#  define U_PATH_SHELL          "sh.exe"
#  define U_LIB_SUFFIX          "dll"
#else
#  define PATH_SEPARATOR '/'
#  ifdef __clang__
#     define U_COMPILER_RANGE_FOR
#     define U_COMPILER_RVALUE_REFS
#     define U_COMPILER_DELETE_MEMBERS
#     define IS_DIR_SEPARATOR(c)  (c) == '/' /* to avoid warning: equality comparison with extraneous parentheses... */
#  else
#     define IS_DIR_SEPARATOR(c) ((c) == '/')
#  endif
#  define IS_ABSOLUTE_PATH(f)   IS_DIR_SEPARATOR((f)[0])
#  define U_PATH_CONV(s)        s
#  define U_PATH_SHELL          "/bin/sh"
#  define U_LIB_SUFFIX          "so"
/* unix is binary by default */
#  ifndef O_BINARY
#  define O_BINARY 0
#  endif
#  ifndef O_TEXT
#  define O_TEXT 0
#  endif
#endif

#ifdef _MSWINDOWS_
#  ifndef ENABLE_LFS
#     define __NO_MINGW_LFS
#  endif
#  ifdef  FD_SETSIZE
#  undef  FD_SETSIZE
#  endif
#  define FD_SETSIZE 1024 /* larger than default (64) */
#  define HAVE_GMTIME_R 1 /* to avoid replace gmtime_r */
#  define _POSIX_THREAD_SAFE_FUNCTIONS 1 /* for localtime_r and gmtime_r */
/* #undef HAVE_GETOPT_LONG // within WINE don't work */
#  define HAVE_WORKING_SOCKET_OPTION_SO_RCVTIMEO 1
#  undef  HAVE_NANOSLEEP
#  include <ulib/base/win32/system.h>
#else
#  ifndef CONFIG_MMAP_ALLOW_UNINITIALIZED
#  define CONFIG_MMAP_ALLOW_UNINITIALIZED
#  endif
#  include <sys/mman.h>
#  if defined(MAP_ANON) && !defined(MAP_ANONYMOUS)
#     define MAP_ANONYMOUS MAP_ANON /* don't use a file */
#  endif
#  include <stdio.h>
#  include <stdint.h>
#  include <unistd.h>
#  include <sys/uio.h>
#  include <sys/stat.h>
#  ifdef __PTH__
#     include <pth.h>
#     include <sys/wait.h>
typedef int fd_t;
typedef int socket_t;
#     define INVALID_SOCKET -1
#     define INVALID_HANDLE_VALUE -1
#     include <signal.h>
#     define pthread_t       pth_t
#     define pthread_cond_t  pth_cond_t
#     define pthread_mutex_t pth_mutex_t
inline void pthread_exit(void* p) { pth_exit(p); };
inline void pthread_kill(pthread_t tid, int sig) { pth_raise(tid, sig); };
inline int  pthread_mutex_init(pthread_mutex_t* mutex, void* x) { return pth_mutex_init(mutex) != 0; };
inline void pthread_mutex_destroy(pthread_mutex_t* mutex) {};
inline void pthread_mutex_lock(pthread_mutex_t* mutex) { pth_mutex_acquire(mutex, 0, NULL); };
inline void pthread_mutex_unlock(pthread_mutex_t* mutex) { pth_mutex_release(mutex); };
inline void pthread_cond_signal(pthread_cond_t* cond) { pth_cond_notify(cond, FALSE); };
inline void pthread_cond_broadcast(pthread_cond_t* cond) { pth_cond_notify(cond, TRUE); };
inline int  pthread_sigmask(int how, const sigset_t* set, sigset_t* oset) { return pth_sigmask(how, set, oset);};
inline void pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex) { pth_cond_await(cond, mutex, NULL); };
#  else
#     include <pthread.h>
typedef int fd_t;
typedef int socket_t;
#     define INVALID_SOCKET -1
#     define INVALID_HANDLE_VALUE -1
#     include <signal.h>
#  endif
#endif

#undef getchar
#undef putchar

#if !defined(_GNU_SOURCE) || defined(__OSX__) || defined(__NetBSD__) || defined(__UNIKERNEL__)
typedef void (*sighandler_t)(int); /* Convenient typedef for signal handlers */
#endif
typedef unsigned long timeout_t; /* Typedef for millisecond timer values */

#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#ifndef U_LINUX
#  define U_MAP_ANON           MAP_ANONYMOUS
#  define MAP_HUGETLB          0
#  define U_MAP_ANON_HUGE      0
#  define U_MAP_ANON_HUGE_ADDR (void*)(0x0UL)
#else
#  ifdef MAP_UNINITIALIZED /* (since Linux 2.6.33) */
#     define U_MAP_ANON (MAP_ANONYMOUS | MAP_UNINITIALIZED)
#  else
#     define U_MAP_ANON  MAP_ANONYMOUS
#  endif
#  ifndef MAP_HUGETLB /* (since Linux 2.6.32) */
#  define MAP_HUGETLB 0x40000 /* arch specific */
#  endif
#  ifdef __ia64__ /* Only ia64 requires this */
#     define U_MAP_ANON_HUGE      (MAP_HUGETLB | MAP_FIXED)
#     define U_MAP_ANON_HUGE_ADDR (void*)(0x8000000000000000UL)
#  else
#     define U_MAP_ANON_HUGE      MAP_HUGETLB 
#     define U_MAP_ANON_HUGE_ADDR (void*)(0x0UL)
#  endif
#endif

#endif
