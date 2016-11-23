/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    base.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_BASE_H
#define ULIB_BASE_H 1

/* Manage the header files to include */

#ifdef HAVE_CONFIG_H
#  include <ulib/internal/config.h>
#  include <ulib/base/replace/replace.h>
#else
#  define restrict __restrict
#  include <ulib/internal/platform.h>
#  ifndef U_LIBEXECDIR
#  define U_LIBEXECDIR "/usr/libexec/ulib"
#  endif
#endif

#ifdef U_LINUX
# ifdef HAVE__USR_SRC_LINUX_INCLUDE_GENERATED_UAPI_LINUX_VERSION_H
#  include "/usr/src/linux/include/generated/uapi/linux/version.h"
# else
#  include <linux/version.h>
# endif
# ifndef LINUX_VERSION_CODE
#  error "You need to use at least 2.0 Linux kernel."
# endif
# ifdef U_APEX_ENABLE
#  include <ulib/base/apex/apex_memmove.h>
# endif
#endif

#ifdef USE_LIBSSL
#  include <openssl/opensslv.h>
#  if   (OPENSSL_VERSION_NUMBER < 0x00905100L)
#     error "Must use OpenSSL 0.9.6 or later, Aborting..."
#  elif (OPENSSL_VERSION_NUMBER > 0x00908000L)
#     define HAVE_OPENSSL_98 1
#  elif (OPENSSL_VERSION_NUMBER > 0x00907000L)
#     define HAVE_OPENSSL_97 1
#  endif
#endif

#ifdef __clang__
#  ifdef  NULL
#  undef  NULL
#  endif
#  define NULL 0
#  include <wchar.h>
#  ifdef ENABLE_THREAD
/*typedef pthread_t        __gthread_t;*/
/*typedef pthread_key_t    __gthread_key_t;*/
/*typedef pthread_once_t   __gthread_once_t;*/
  typedef pthread_mutex_t  __gthread_mutex_t;
/*typedef pthread_mutex_t  __gthread_recursive_mutex_t;*/
/*typedef pthread_cond_t   __gthread_cond_t;*/
/*typedef struct timespec  __gthread_time_t;*/
#  endif
#endif

#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <limits.h>

/* Defs */
#include <ulib/base/color.h>
#include <ulib/base/macro.h>

#ifndef LINUX_VERSION_CODE
#define LINUX_VERSION_CODE 0
#endif
#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif
#ifdef __clang__
#  define U_DUMP_KERNEL_VERSION(x)
#else
#  define U_DO_PRAGMA(x) _Pragma (#x)
#  define U_DUMP_KERNEL_VERSION(x) U_DO_PRAGMA(message (#x " = " U_STRINGIFY(x)))
#endif

#ifdef USE_LIBZ /* check for crc32 */
#  include <zlib.h>
#  define zlib_deflateInit2(strm, level, method, windowBits, memLevel, strategy) \
               deflateInit2_((strm),(level),(method),(windowBits),(memLevel),(strategy),ZLIB_VERSION,sizeof(z_stream))
#  define zlib_inflateInit2(strm, windowBits) \
               inflateInit2_((strm),(windowBits),ZLIB_VERSION,sizeof(z_stream))
#  ifndef U_ZLIB_DEFLATE_WORKSPACESIZE
#     ifndef zlib_inflate
#     define zlib_inflate(a,b)   inflate(a,b)
#     endif
#     ifndef zlib_deflate
#     define zlib_deflate(a,b)   deflate(a,b)
#     endif
#     ifndef zlib_inflateEnd
#     define zlib_inflateEnd(a)  inflateEnd(a)
#     endif
#     ifndef zlib_deflateEnd
#     define zlib_deflateEnd(a)  deflateEnd(a)
#     endif
#     ifndef zlib_inflateReset
#     define zlib_inflateReset(strm) inflateReset(strm)
#     endif
#     ifndef zlib_deflateReset
#     define zlib_deflateReset(strm) deflateReset(strm)
#     endif
#  endif
#endif

#define U_BUFFER_SIZE 8192

#ifdef __cplusplus
extern "C" {
#endif

typedef int   (*iPF)     (void);
typedef bool  (*bPF)     (void);
typedef void  (*vPF)     (void);
typedef void* (*pvPF)    (void);
typedef bool  (*bPFi)    (int);
typedef void  (*vPFi)    (int);
typedef void  (*vPFpv)   (void*);
typedef bool  (*bPFpv)   (void*);
typedef int   (*iPFpv)   (void*);
typedef bool  (*bPFpc)   (const char*);
typedef void  (*vPFpc)   (const char*);
typedef void* (*pvPFpv)  (void*);
typedef bool  (*bPFpcu)  (const char*,uint32_t);
typedef void  (*vPFpvu)  (void*,uint32_t);
typedef int   (*iPFpvpv) (void*,void*);
typedef bool  (*bPFpvpv) (void*,void*);
typedef bool  (*bPFpcpc) (const char*,const char*);
typedef bool  (*bPFpcpv) (const char*,const void*);
typedef void  (*vPFpvpc) (void*,char*);
typedef void  (*vPFpvpv) (void*,void*);

typedef char* (*pcPFdpc)   (double,char*);
typedef char* (*pcPFu32pc) (uint32_t,char*);
typedef char* (*pcPFu64pc) (uint64_t,char*);

typedef void* (*pvPFpvpb)  (void*,bool*);
typedef int   (*qcompare)  (const void*,const void*);
typedef void  (*vPFpvpcpc) (void*,char*,char*);
typedef void* (*pvPFpvpvs) (void*,const void*,size_t);

typedef struct U_DATA {
   unsigned char* dptr;
   size_t dsize;
} U_DATA;

/**
 * #define U_SUBSTR_INC_REF // NB: be aware that in this way we don't capture the event 'DEAD OF SOURCE STRING WITH CHILD ALIVE'...
 */

/* String representation */

typedef struct ustringrep {
#ifdef DEBUG
   const void* _this;
#endif
#if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
   struct ustringrep* parent; /* manage substring for increment reference of source string */
# ifdef DEBUG
   int32_t child; /* manage substring for capture event 'DEAD OF SOURCE STRING WITH CHILD ALIVE'... */
# endif
#endif
   uint32_t _length, _capacity, references;
   const char* str;
} ustringrep;

typedef struct ustring { struct ustringrep* rep; } ustring;

/* Internal buffer */

extern U_EXPORT char* u_buffer;
extern U_EXPORT char* u_err_buffer;
extern U_EXPORT uint32_t u_buffer_len; /* assert that u_buffer is busy if u_buffer_len != 0 */

/* Startup */

extern U_EXPORT pid_t u_pid;
extern U_EXPORT uint32_t u_pid_str_len;
extern U_EXPORT uint32_t u_progname_len;
extern U_EXPORT bool u_is_tty, u_ulib_init;

extern U_EXPORT       char* restrict u_pid_str;
extern U_EXPORT const char* restrict u_progpath;
extern U_EXPORT const char* restrict u_progname;

U_EXPORT void u_init_ulib(char** restrict argv);

/* At Exit */

extern U_EXPORT vPF u_fns[32];
extern U_EXPORT int u_fns_index;

U_EXPORT void u_exit(void);
U_EXPORT void u_atexit(vPF function);
U_EXPORT void u_unatexit(vPF function);

/* Current working directory */

extern U_EXPORT char* u_cwd;
extern U_EXPORT uint32_t u_cwd_len;

U_EXPORT void u_getcwd(void);

/* Time services */

extern U_EXPORT bool u_daylight;
extern U_EXPORT int u_now_adjust;   /* GMT based time */
extern U_EXPORT time_t u_start_time;
extern U_EXPORT void* u_pthread_time; /* pthread clock */

extern U_EXPORT struct timeval* u_now;
extern U_EXPORT struct tm u_strftime_tm;
extern U_EXPORT struct timeval u_timeval;
extern U_EXPORT const char* u_day_name[7];    /* "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" */
extern U_EXPORT const char* u_month_name[12]; /* "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" */

U_EXPORT bool     u_setStartTime(void);
U_EXPORT unsigned u_getMonth(const char* buf) __pure;

/* Services */

extern U_EXPORT int u_errno; /* An errno value */
extern U_EXPORT int u_num_cpu;
extern U_EXPORT int u_flag_exit;
extern U_EXPORT int u_flag_test;
extern U_EXPORT bool u_recursion;
extern U_EXPORT bool u_fork_called;
extern U_EXPORT bool u_exec_failed;
extern U_EXPORT char u_user_name[32];
extern U_EXPORT uint32_t u_flag_sse; /* detect SSE2, SSSE3, SSE4.2 */
extern U_EXPORT uint32_t u_m_w, u_m_z;
extern U_EXPORT const char* u_short_units[6]; /* { "B", "KB", "MB", "GB", "TB", 0 } */
extern U_EXPORT const char* restrict u_tmpdir;
extern U_EXPORT char u_hostname[HOST_NAME_MAX+1];
extern U_EXPORT uint32_t u_hostname_len, u_user_name_len, u_seed_hash;

U_EXPORT void u_setPid(void);
U_EXPORT void u_initRandom(void);
U_EXPORT void u_init_ulib_username(void);
U_EXPORT void u_init_ulib_hostname(void);
U_EXPORT void u_init_http_method_list(void);

U_EXPORT const char* u_basename(const char* restrict path) __pure;
U_EXPORT const char* u_getsuffix(const char* restrict path, uint32_t len) __pure;
U_EXPORT bool u_is_overlap(const char* restrict dst, const char* restrict src, size_t n);

/* Location info */

extern U_EXPORT uint32_t u_num_line;
extern U_EXPORT const char* restrict u_name_file;
extern U_EXPORT const char* restrict u_name_function;

/* MIME type identification */

static inline bool u_is_gz(int mime_index)     { return (mime_index == U_gz); }
static inline bool u_is_js(int mime_index)     { return (mime_index == U_js); }
static inline bool u_is_css(int mime_index)    { return (mime_index == U_css); }
static inline bool u_is_gif(int mime_index)    { return (mime_index == U_gif); }
static inline bool u_is_jpg(int mime_index)    { return (mime_index == U_jpg); }
static inline bool u_is_png(int mime_index)    { return (mime_index == U_png); }
static inline bool u_is_flv(int mime_index)    { return (mime_index == U_flv); }
static inline bool u_is_html(int mime_index)   { return (mime_index == U_html); }

static inline bool u_is_usp(int mime_index)    { return (mime_index == U_usp); }
static inline bool u_is_csp(int mime_index)    { return (mime_index == U_csp); }
static inline bool u_is_cgi(int mime_index)    { return (mime_index == U_cgi); }
static inline bool u_is_ssi(int mime_index)    { return (mime_index == U_ssi); }
static inline bool u_is_php(int mime_index)    { return (mime_index == U_php); }
static inline bool u_is_ruby(int mime_index)   { return (mime_index == U_ruby); }
static inline bool u_is_python(int mime_index) { return (mime_index == U_python); }

static inline bool u_is_know(int mime_index)   { return (mime_index == U_know); }
static inline bool u_is_unknow(int mime_index) { return (mime_index == U_unknow); }

static inline bool u_is_img(int mime_index)    { return (mime_index == U_png ||
                                                         mime_index == U_gif ||
                                                         mime_index == U_jpg ||
                                                         mime_index == U_ico); }

static inline bool u_is_cacheable(int mime_index) { return (u_is_js(mime_index)  ||
                                                            u_is_css(mime_index) ||
                                                            u_is_img(mime_index) ||
                                                            u_is_ssi(mime_index) ||
                                                            u_is_html(mime_index)); }

/**
 * Print with format extension: bBCDHMNOPQrRSvVUYwW
 * ----------------------------------------------------------------------------
 * '%b': print bool ("true" or "false")
 * '%B': print bit conversion of integer
 * '%C': print formatted char
 * '%H': print name host
 * '%M': print memory dump
 * '%N': print name program
 * '%P': print pid process
 * '%Q': sign for call to exit() or abort() (var-argument is param to exit)
 * '%r': print u_getExitStatus(exit_value)
 * '%R': print var-argument (msg) "-" u_getSysError()
 * '%O': print formatted temporary string + free(string)
 * '%S': print formatted string
 * '%v': print ustring
 * '%V': print ustring
 * '%J': print U_DATA
 * '%U': print name login user
 * '%Y': print u_getSysSignal(signo)
 * '%w': print current working directory
 * '%W': print COLOR (index to ANSI ESCAPE STR)
 * ----------------------------------------------------------------------------
 * '%D': print date and time in various format:
 * ----------------------------------------------------------------------------
 *             0  => format: %d/%m/%y
 * with flag  '1' => format:          %T (=> "%H:%M:%S)
 * with flag  '2' => format:          %T (=> "%H:%M:%S) +n days
 * with flag  '3' => format: %d/%m/%Y %T
 * with flag  '4' => format: %d%m%y_%H%M%S_millisec (for file name, backup, etc...)
 * with flag  '5' => format: %a, %d %b %Y %T %Z
 * with flag  '6' => format: %Y/%m/%d
 * with flag  '7' => format: %Y/%m/%d %T
 * with flag  '8' => format: %a, %d %b %Y %T GMT
 * with flag  '9' => format: %d/%m/%y %T
 * with flag '10' => format: %d/%b/%Y:%T %z
 * with flag  '#' => var-argument
 * ----------------------------------------------------------------------------
 */

extern U_EXPORT int32_t u_printf_string_max_length;

/* NB: we use u__printf(), u__snprintf(), u__vsnprintf(), ... cause of conflit with /usr/include/unicode/urename.h */

U_EXPORT void u__printf(int fd,           const char* restrict format, uint32_t fmt_size, ...);
U_EXPORT void u_internal_print(bool abrt, const char* restrict format,                    ...) PRINTF_ATTRIBUTE(2,3);

U_EXPORT uint32_t u_sprintc(   char* restrict buffer, unsigned char c);
U_EXPORT uint32_t u_sprintcrtl(char* restrict buffer, unsigned char c);
U_EXPORT uint32_t u__snprintf( char* restrict buffer, uint32_t buffer_size, const char* restrict format, uint32_t fmt_size, ...);
U_EXPORT uint32_t u__vsnprintf(char* restrict buffer, uint32_t buffer_size, const char* restrict format, uint32_t fmt_size, va_list argp);

#ifdef DEBUG
/* NB: u_strlen() and u_memcpy() conflit with /usr/include/unicode/urename.h */
U_EXPORT size_t u__strlen(const char* restrict s,    const char* function);
U_EXPORT void   u__strcpy(      char* restrict dest, const char* restrict src);
U_EXPORT void   u__memcpy(      void* restrict dest, const void* restrict src, size_t n, const char* function);
U_EXPORT char*  u__strncpy(     char* restrict dest, const char* restrict src, size_t n);
#else
#  define u__strlen(s,func)                 strlen((s))
#  define u__strcpy(dest,src)        (void) strcpy((dest),(src))
#  define u__strncpy(dest,src,n)            strncpy((dest),(src),(n))
#  ifdef U_APEX_ENABLE
#     define u__memcpy(dest,src,n,func) (void) apex_memcpy((dest),(src),(n))
#  else
#     define u__memcpy(dest,src,n,func) (void)      memcpy((dest),(src),(n))
#  endif
#endif
#ifndef U_APEX_ENABLE
#  define apex_memcpy( dest,src,n) memcpy( (dest),(src),(n))
#  define apex_memmove(dest,src,n) memmove((dest),(src),(n))
#endif

#if (defined(U_LINUX) || defined(_MSWINDOWS_)) && !defined(__suseconds_t_defined)
typedef long suseconds_t;
#endif

#define U_SECOND 1000000L

static inline void u_adjtime(void* tv_sec, void* tv_usec)
{
   long r = *((suseconds_t*)tv_usec) / U_SECOND;

   U_INTERNAL_TRACE("u_adjtime(%p,%p)", tv_sec, tv_usec)

   // NB: r can be negativ...

   if (r)
      {
      *((long*)tv_sec)         += r;
      *((suseconds_t*)tv_usec) %= U_SECOND;
      }

   U_INTERNAL_ASSERT_MINOR(*((suseconds_t*)tv_usec), U_SECOND)

   if (*((suseconds_t*)tv_usec) < 0) { *((suseconds_t*)tv_usec) += U_SECOND; --(*((long*)tv_sec)); }

   U_INTERNAL_ASSERT_RANGE(0, *((suseconds_t*)tv_usec), U_SECOND)
}

static inline void u_gettimenow(void)
{
   U_INTERNAL_TRACE("u_gettimenow()")

   (void) gettimeofday(u_now, 0);
}

static inline void u_gettimeofday(struct timeval* tv)
{
   U_INTERNAL_TRACE("u_gettimeofday(%p)", tv)

   (void) gettimeofday(tv, 0);
}

/**
#ifdef HAVE_CLOCK_GETTIME
extern U_EXPORT struct timeval u_start_clock;

 * struct timespec {
 *    time_t tv_sec;  //     seconds
 *    long   tv_nsec; // nanoseconds
 * };

static inline void u_gettimenow(void)
{
   struct timespec ts;

   U_INTERNAL_TRACE("u_gettimenow()")

   (void) clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);

   u_now->tv_sec  = u_start_clock.tv_sec  +  ts.tv_sec;
   u_now->tv_usec = u_start_clock.tv_usec + (ts.tv_nsec / 1000L);

   u_adjtime(&(u_now->tv_sec), &(u_now->tv_usec));

   U_INTERNAL_PRINT("u_now = { %lu, %lu }", u_now->tv_sec, u_now->tv_usec)
}
#endif
*/

U_EXPORT      uint32_t u_strftime1(char* restrict buffer, uint32_t buffer_size, const char* restrict fmt, uint32_t fmt_size);
static inline uint32_t u_strftime2(char* restrict buffer, uint32_t buffer_size, const char* restrict fmt, uint32_t fmt_size, time_t when)
{
   U_INTERNAL_TRACE("u_strftime2(%.*s,%u,%.*s,%u,%ld)", buffer_size, buffer, buffer_size, fmt_size, fmt, fmt_size, when)

   U_INTERNAL_ASSERT_POINTER(fmt)
   U_INTERNAL_ASSERT_MAJOR(buffer_size, 0)

   (void) memset(&u_strftime_tm, 0, sizeof(struct tm));

   (void) gmtime_r(&when, &u_strftime_tm);

   return u_strftime1(buffer, buffer_size, fmt, fmt_size);
}

/* conversion number to string */

extern U_EXPORT const char* u_ctn2s;

extern U_EXPORT pcPFdpc   u_dbl2str;
extern U_EXPORT pcPFu32pc u_num2str32;
extern U_EXPORT pcPFu64pc u_num2str64;

static inline char* u_num2str32s(int32_t num, char* restrict cp)
{
   U_INTERNAL_TRACE("u_num2str32s(%u,%p)", num, cp)

   if (num < 0)
      {
      num = -num;

      *cp++ = '-';
      }

   return u_num2str32(num, cp);
}

static inline char* u_num2str64s(int64_t num, char* restrict cp)
{
   U_INTERNAL_TRACE("u_num2str64s(%lld,%p)", num, cp)

   if (num < 0)
      {
      num = -num;

      *cp++ = '-';
      }

   return u_num2str64(num, cp);
}

static inline char* u_dtoa(double num, char* restrict cp)
{
   U_INTERNAL_TRACE("u_dtoa(%g,%p)", num, cp)

   if (num < 0)
      {
      num = -num;

      *cp++ = '-';
      }

   return u_dbl2str(num, cp);
}

#ifdef __cplusplus
}
#endif

#endif
