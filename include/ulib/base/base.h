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

/* Manage file to include */

#ifdef HAVE_CONFIG_H
#  include <ulib/internal/config.h>
#  include <ulib/base/replace/replace.h>
#else
#  include <ulib/internal/platform.h>
#  ifndef U_LIBEXECDIR
#  define U_LIBEXECDIR "/usr/libexec/ulib"
#  endif
#endif

#if defined(LINUX) || defined(__LINUX__) || defined(__linux__)
#ifdef HAVE__USR_SRC_LINUX_INCLUDE_GENERATED_UAPI_LINUX_VERSION_H
#  include "/usr/src/linux/include/generated/uapi/linux/version.h"
#  else
#  include <linux/version.h>
#  endif
#  ifndef LINUX_VERSION_CODE
#     error "You need to use at least 2.0 Linux kernel."
#  endif
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

typedef int      (*iPF)     (void);
typedef int      (*iPFpv)   (void*);
typedef unsigned (*uPFpv)   (void*);
typedef int      (*iPFpvpv) (      void*,      void*);
typedef int      (*qcompare)(const void*,const void*);
typedef bool     (*bPFi)    (int);
typedef bool     (*bPF)     (void);
typedef bool     (*bPFpv)   (void*);
typedef bool     (*bPFpvpv) (void*,void*);
typedef bool     (*bPFpc)   (const char*);
typedef bool     (*bPFpcu)  (const char*, uint32_t);
typedef bool     (*bPFpcpc) (const char*,const char*);
typedef bool     (*bPFpcpv) (const char*,const void*);
typedef void     (*vPF)     (void);
typedef void     (*vPFi)    (int);
typedef void     (*vPFpv)   (void*);
typedef void     (*vPFpc)   (const char*);
typedef void     (*vPFpvpc) (void*,char*);
typedef void     (*vPFpvpv) (void*,void*);
typedef void     (*vPFpvu)  (void*,uint32_t);
typedef void*    (*pvPF)    (void);
typedef void*    (*pvPFpv)  (void*);
typedef void*    (*pvPFpvpb)(void*,bool*);

typedef struct U_DATA {
   unsigned char* dptr;
   size_t dsize;
} U_DATA;

/**
 * #define U_SUBSTR_INC_REF // NB: in this way we don't capture the event 'DEAD OF SOURCE STRING WITH CHILD ALIVE'...
 */

typedef struct ustringrep {
#ifdef DEBUG
   const void* _this;
#endif
#if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
   struct ustringrep* parent; /* manage substring for increment reference of source string */
#  ifdef DEBUG
   int32_t child; /* manage substring for capture event 'DEAD OF SOURCE STRING WITH CHILD ALIVE'... */
#  endif
#endif
   uint32_t _length, _capacity, references;
   const char* str;
} ustringrep;

/* String representation */
extern U_EXPORT struct ustringrep u_empty_string_rep_storage;

/* Internal buffer */
extern U_EXPORT char* u_buffer;
extern U_EXPORT char* u_err_buffer;
extern U_EXPORT uint32_t u_buffer_len; /* assert that is busy if != 0 */

/* Startup */
extern U_EXPORT pid_t u_pid;
extern U_EXPORT bool u_is_tty;
extern U_EXPORT uint32_t u_pid_str_len;
extern U_EXPORT uint32_t u_progname_len;
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

extern U_EXPORT const char* u_months[];    /* "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" */
extern U_EXPORT const char* u_months_it[]; /* "gen", "feb", "mar", "apr", "mag", "giu", "lug", "ago", "set", "ott", "nov", "dic" */

U_EXPORT bool     u_setStartTime(void);
U_EXPORT int      u_getMonth(const char* buf) __pure;
U_EXPORT uint32_t u_strftime1(char* restrict buffer, uint32_t buffer_size, const char* restrict fmt);
U_EXPORT uint32_t u_strftime2(char* restrict buffer, uint32_t buffer_size, const char* restrict fmt, time_t when);

/* Services */
extern U_EXPORT int u_errno; /* An errno value */
extern U_EXPORT int u_flag_exit;
extern U_EXPORT int u_flag_test;
extern U_EXPORT bool u_recursion;
extern U_EXPORT bool u_fork_called;
extern U_EXPORT bool u_exec_failed;
extern U_EXPORT char u_user_name[32];
extern U_EXPORT const char* restrict u_tmpdir;
extern U_EXPORT char u_hostname[HOST_NAME_MAX+1];
extern U_EXPORT const int MultiplyDeBruijnBitPosition2[32];
extern U_EXPORT uint32_t u_hostname_len, u_user_name_len, u_seed_hash;

extern U_EXPORT const unsigned char u_alphabet[];  /* "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" */
extern U_EXPORT const unsigned char u_hex_upper[]; /* "0123456789ABCDEF" */
extern U_EXPORT const unsigned char u_hex_lower[]; /* "0123456789abcdef" */

/* conversion number to string */
extern U_EXPORT const char u_ctn2s[201];

U_EXPORT uint32_t u_num2str32( char* restrict cp, uint32_t num);
U_EXPORT uint32_t u_num2str32s(char* restrict cp,  int32_t num);
U_EXPORT uint32_t u_num2str64( char* restrict cp, uint64_t num);
U_EXPORT uint32_t u_num2str64s(char* restrict cp,  int64_t num);

/* Location info */
extern U_EXPORT uint32_t u_num_line;
extern U_EXPORT const char* restrict u_name_file;
extern U_EXPORT const char* restrict u_name_function;

U_EXPORT void u_setPid(void);
U_EXPORT void u_init_ulib_username(void);
U_EXPORT void u_init_ulib_hostname(void);
U_EXPORT void u_init_http_method_list(void);

U_EXPORT const char* u_basename(const char* restrict path) __pure;
U_EXPORT const char* u_getsuffix(const char* restrict path, uint32_t len) __pure;

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

static inline bool u_is_know(int mime_index)   { return (mime_index == U_know); }
static inline bool u_is_unknow(int mime_index) { return (mime_index == U_unknow); }

static inline bool u_is_img(int mime_index)    { return (mime_index == U_png ||
                                                         mime_index == U_gif ||
                                                         mime_index == U_jpg ||
                                                         mime_index == U_ico); }

/**
 * ----------------------------------------------------------------------------
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

/* NB: u_printf(), u_vsnprintf and u_snprintf conflit with /usr/include/unicode/urename.h */

U_EXPORT void u__printf(int fd,           const char* restrict format, ...);
U_EXPORT void u_internal_print(bool abrt, const char* restrict format, ...);

U_EXPORT uint32_t u_sprintc(   char* restrict buffer, unsigned char c);
U_EXPORT uint32_t u__snprintf( char* restrict buffer, uint32_t buffer_size, const char* restrict format, ...);
U_EXPORT uint32_t u__vsnprintf(char* restrict buffer, uint32_t buffer_size, const char* restrict format, va_list argp);

#ifdef __cplusplus
}
#endif

#endif
