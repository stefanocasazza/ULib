/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    base.c
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */
 
/*
#define DEBUG_DEBUG
*/

#include <ulib/base/hash.h>
#include <ulib/base/error.h>
#include <ulib/base/utility.h>
#include <ulib/internal/chttp.h>
#include <ulib/base/coder/escape.h>

#ifdef DEBUG
#  define U_INTERNAL_ERROR(assertion,format,args...) \
      if ((bool)(assertion) == false) { \
         u_internal_print(true, \
         "  pid: %.*s\n" \
         " file: %s\n" \
         " line: %d\n" \
         " function: %s\n" \
         " assertion: \"(%s)\" \n" \
         "-------------------------------------\n" \
         format "\n", \
         u_pid_str_len, u_pid_str, \
         __FILE__, \
         __LINE__, \
         __PRETTY_FUNCTION__, \
         #assertion , ##args); \
      }

void u_debug_init(void);
void u_debug_at_exit(void);
#else
#  define U_INTERNAL_ERROR(assertion,format,args...)
/*
#  undef  U_INTERNAL_TRACE
#  define U_INTERNAL_TRACE(format,args...) u_internal_print(false, format"\n" , ##args);
#  undef  U_INTERNAL_PRINT
#  define U_INTERNAL_PRINT(format,args...) U_INTERNAL_TRACE(format,args)
*/
#endif

#ifdef HAVE_ENDIAN_H
#  include <endian.h>
#elif defined(HAVE_SYS_ENDIAN_H)
#  include <sys/endian.h>
#endif
#ifdef HAVE_SYS_SYSCALL_H
#  include <sys/syscall.h>
#endif
#ifndef _MSWINDOWS_
#  include <pwd.h>
#  include <sys/uio.h>
#  include <sys/utsname.h>
#endif
/* For TIOCGWINSZ and friends: */
#ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif
#ifdef HAVE_TERMIOS_H
#  include <termios.h>
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86) /* Test for Intel/AMD architecture */
#  if defined(_MSC_VER)
#     include <intrin.h> /* __cpuid Visual Studio */
#  elif defined(__GNUC__) && defined(HAVE_CPUID_H)
#     include <cpuid.h> /* __get_cpuid GCC / LLVM (Clang) */
#  endif
#endif

int u_num_cpu = -1;
uint32_t u_m_w = 521288629;
uint32_t u_m_z = 362436069;
uint32_t u_seed_hash = 0xdeadbeef;
struct timeval* u_now = &u_timeval;
const char* restrict u_tmpdir = "/tmp";
const char* u_short_units[] = { "B", "KB", "MB", "GB", "TB", 0 };
const char* u_day_name[7]    = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
const char* u_month_name[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

/* conversion table number to string */

const char* u_ctn2s = "00010203040506070809"
                      "10111213141516171819"
                      "20212223242526272829"
                      "30313233343536373839"
                      "40414243444546474849"
                      "50515253545556575859"
                      "60616263646566676869"
                      "70717273747576777879"
                      "80818283848586878889"
                      "90919293949596979899";

/* Startup */

bool     u_is_tty;
bool     u_ulib_init;
pid_t    u_pid;
uint32_t u_pid_str_len;
uint32_t u_progname_len;

      char* restrict u_pid_str;
const char* restrict u_progpath;
const char* restrict u_progname;

/* Current working directory */

char*    u_cwd;
uint32_t u_cwd_len;

/* Location info */

uint32_t             u_num_line;
const char* restrict u_name_file;
const char* restrict u_name_function;

/* Internal buffer */

char*    u_buffer;
char*    u_err_buffer;
uint32_t u_buffer_len; /* signal that is busy if != 0 */

/* Time services */

bool   u_daylight;
void*  u_pthread_time; /* pthread clock */
time_t u_start_time;
int    u_now_adjust;   /* GMT based time */
struct tm u_strftime_tm;

struct timeval u_timeval;

/*
#ifdef HAVE_CLOCK_GETTIME
struct timeval u_start_clock;
#endif
*/

/* Services */

int u_errno; /* An errno value */
int u_flag_exit;
int u_flag_test;
bool u_recursion;
bool u_fork_called;
bool u_exec_failed;
uint32_t u_flag_sse; /* detect SSE2, SSSE3, SSE4.2 */
char u_user_name[32];
char u_hostname[HOST_NAME_MAX+1];
int32_t u_printf_string_max_length;
uint32_t u_hostname_len, u_user_name_len;
struct uclientimage_info u_clientimage_info;

static char* num2str32(uint32_t num, char* restrict cp)
{
   uint32_t a, b, c, d1, d2, d3, d4;

   U_INTERNAL_TRACE("num2str32(%u,%p)", num, cp)

   if (num < 10000)
      {
      d1 = (num / 100) << 1;
      d2 = (num % 100) << 1;

      if (num >= 1000) *cp++ = u_ctn2s[d1];
      if (num >=  100) *cp++ = u_ctn2s[d1+1];
      if (num >=   10) *cp++ = u_ctn2s[d2];

      *cp++ = u_ctn2s[d2+1];

      return cp;
      }

   if (num < 100000000) /* num = bbbbcccc */
      {
      b  = num / 10000;
      c  = num % 10000;
      d1 = (b / 100) << 1;
      d2 = (b % 100) << 1;
      d3 = (c / 100);
      d4 = (c % 100);

      if (num >= 10000000) *cp++ = u_ctn2s[d1];
      if (num >=  1000000) *cp++ = u_ctn2s[d1+1];
      if (num >=   100000) *cp++ = u_ctn2s[d2];

      *cp++ = u_ctn2s[d2+1];

      U_NUM2STR16(cp,   d3);
      U_NUM2STR16(cp+2, d4);

      return (cp + 4);
      }

   /* num = aabbbbcccc in decimal */

   a = num  / 100000000; /* 1 to 42 */
       num %= 100000000;

   if (a < 10) *cp++ = '0' + (char)a;
   else
      {
      U_NUM2STR16(cp, a);

      cp += 2;
      }

   b = num / 10000; /* 0 to 9999 */
   c = num % 10000; /* 0 to 9999 */

   d1 = (b / 100);
   d2 = (b % 100);

   d3 = (c / 100);
   d4 = (c % 100);

   U_NUM2STR16(cp,   d1);
   U_NUM2STR16(cp+2, d2);
   U_NUM2STR16(cp+4, d3);
   U_NUM2STR16(cp+6, d4);

   return (cp + 8);
}

static char* num2str64(uint64_t num, char* restrict cp)
{
   uint32_t a, i, j, v0, v1, b0, b1, c0, c1, d1, d2, d3, d4, d5, d6, d7, d8;

   U_INTERNAL_TRACE("num2str64(%llu,%p)", num, cp)

   if (num <= UINT_MAX) return u_num2str32((uint32_t)num, cp);

   if (num < 10000000000000000)
      {
      v0 = (uint32_t)(num / 100000000);
      v1 = (uint32_t)(num % 100000000);

      b0 = v0 / 10000;
      c0 = v0 % 10000;

      d1 = (b0 / 100) << 1;
      d2 = (b0 % 100) << 1;

      d3 = (c0 / 100) << 1;
      d4 = (c0 % 100) << 1;

      b1 = v1 / 10000;
      c1 = v1 % 10000;

      d5 = (b1 / 100);
      d6 = (b1 % 100);

      d7 = (c1 / 100);
      d8 = (c1 % 100);

      if (num >= 1000000000000000) *cp++ = u_ctn2s[d1];
      if (num >=  100000000000000) *cp++ = u_ctn2s[d1+1];
      if (num >=   10000000000000) *cp++ = u_ctn2s[d2];
      if (num >=    1000000000000) *cp++ = u_ctn2s[d2+1];
      if (num >=     100000000000) *cp++ = u_ctn2s[d3];
      if (num >=      10000000000) *cp++ = u_ctn2s[d3+1];
      if (num >=       1000000000) *cp++ = u_ctn2s[d4];
      if (num >=        100000000) *cp++ = u_ctn2s[d4+1];

      U_NUM2STR16(cp,   d5);
      U_NUM2STR16(cp+2, d6);
      U_NUM2STR16(cp+4, d7);
      U_NUM2STR16(cp+6, d8);

      return (cp + 8);
      }

   a = (uint32_t)(num  / 10000000000000000); /* 1 to 1844 */
                  num %= 10000000000000000;

        if (a <  10) *cp++ = '0' + (char)a;
   else if (a < 100)
      {
      U_NUM2STR16(cp, a);

      cp += 2;
      }
   else if (a < 1000)
      {
      *cp++ = '0' + (char)(a / 100);

      i = (a % 100);

      U_NUM2STR16(cp, i);

      cp += 2;
      }
   else
      {
      i = (a / 100);
      j = (a % 100);

      U_NUM2STR16(cp,   i);
      U_NUM2STR16(cp+2, j);

      cp += 4;
      }

   v0 = (uint32_t)(num / 100000000);
   v1 = (uint32_t)(num % 100000000);

   b0 = v0 / 10000;
   c0 = v0 % 10000;

   d1 = (b0 / 100);
   d2 = (b0 % 100);

   d3 = (c0 / 100);
   d4 = (c0 % 100);

   b1 = v1 / 10000;
   c1 = v1 % 10000;

   d5 = (b1 / 100);
   d6 = (b1 % 100);

   d7 = (c1 / 100);
   d8 = (c1 % 100);

   U_NUM2STR16(cp,    d1);
   U_NUM2STR16(cp+2,  d2);
   U_NUM2STR16(cp+4,  d3);
   U_NUM2STR16(cp+6,  d4);
   U_NUM2STR16(cp+8,  d5);
   U_NUM2STR16(cp+10, d6);
   U_NUM2STR16(cp+12, d7);
   U_NUM2STR16(cp+14, d8);

   return (cp + 16);
}

static char* dtoa(double num, char* restrict cp) { return cp + sprintf(cp, "%g", num); }

pcPFdpc   u_dbl2str  = dtoa;
pcPFu32pc u_num2str32 = num2str32;
pcPFu64pc u_num2str64 = num2str64;

/**
 * "FATAL: kernel too old"
 *
 * Even if you recompile the code with -static compiler command-line option to avoid any dependency on the dynamic Glibc library,
 * you could still encounter the error in question, and your code will exit with Segmentation Fault error.
 *
 * This kernel version check is done by DL_SYSDEP_OSCHECK macro in Glibc's sysdeps/unix/sysv/linux/dl-osinfo.h
 * It calls _dl_discover_osversion to get current kernel's version.
 *
 * The fix (or hack) is to add the following function in your code and compile your code with -static compiler command-line option.
 *
 * int _dl_discover_osversion() { return 0xffffff; }
 */

__pure const char* u_basename(const char* restrict path)
{
   const char* restrict base;

   U_INTERNAL_TRACE("u_basename(%s)", path)

#ifdef _MSWINDOWS_
   if (u__isalpha(path[0]) && path[1] == ':') path += 2; /* Skip over the disk name in MSDOS pathnames */
#endif

   for (base = path; *path; ++path) if (IS_DIR_SEPARATOR(*path)) base = path + 1;

   return base;
}

__pure const char* u_getsuffix(const char* restrict path, uint32_t len)
{
   const char* restrict ptr;

   U_INTERNAL_TRACE("u_getsuffix(%.*s,%u)", U_min(len,128), path, len)

   U_INTERNAL_ASSERT_POINTER(path)

   // NB: we can have something like 'www.sito1.com/tmp'...

   ptr = (const char*) memrchr(path, '.', len);

   return (ptr && memrchr(ptr+1, '/', len - (ptr+1 - path)) == 0 ? ptr : 0);
}

void u_setPid(void)
{
   static char buffer[10];

   pid_t pid_copy;

   U_INTERNAL_TRACE("u_setPid()")

   u_pid     = getpid();
   u_pid_str = buffer + sizeof(buffer);

   pid_copy = u_pid;

   while (pid_copy >= 10)
      {
      *--u_pid_str = (pid_copy % 10) + '0';

      pid_copy /= 10;
      }

   U_INTERNAL_ASSERT_MINOR(pid_copy, 10)

   *--u_pid_str = pid_copy + '0';

   u_pid_str_len = buffer + sizeof(buffer) - u_pid_str;
}

bool u_is_overlap(const char* restrict dst, const char* restrict src, size_t n)
{
   U_INTERNAL_TRACE("u_is_overlap(%p,%p,%lu)", dst, src, n)

   U_INTERNAL_ASSERT_MAJOR(n, 0)

        if (src < dst) return ((src + n - 1) >= dst);
   else if (dst < src) return ((dst + n - 1) >= src);

   /* They start at same place. Since we know neither of them has zero length, they must overlap */

   U_INTERNAL_ASSERT_EQUALS(dst, src)

   return true;
}

#ifdef DEBUG
size_t u__strlen(const char* restrict s, const char* called_by_function)
{
   U_INTERNAL_TRACE("u__strlen(%s,%s)", s, called_by_function)

   U_INTERNAL_ASSERT_POINTER(called_by_function)
   U_INTERNAL_ASSERT_POINTER_MSG(s,called_by_function)

   return strlen(s);
}

void u__strcpy(char* restrict dest, const char* restrict src)
{
   size_t n = u__strlen(src, __PRETTY_FUNCTION__);

   U_INTERNAL_TRACE("u__strcpy(%p,%p,%lu)", dest, src, n)

   U_INTERNAL_ASSERT_MAJOR(n, 0)
   U_INTERNAL_ASSERT_POINTER(src)
   U_INTERNAL_ASSERT_POINTER(dest)
   U_INTERNAL_ASSERT_EQUALS(u_is_overlap(dest,src,n), false)

   (void) strcpy(dest, src);
}

char* u__strncpy(char* restrict dest, const char* restrict src, size_t n)
{
   U_INTERNAL_TRACE("u__strncpy(%p,%p,%lu)", dest, src, n)

   U_INTERNAL_ASSERT_MAJOR(n, 0)
   U_INTERNAL_ASSERT_POINTER(src)
   U_INTERNAL_ASSERT_POINTER(dest)
   U_INTERNAL_ASSERT_EQUALS(u_is_overlap(dest,src,n), false)

   (void) strncpy(dest, src, n);

   return dest;
}

void u__memcpy(void* restrict dst, const void* restrict src, size_t n, const char* called_by_function)
{
   U_INTERNAL_TRACE("u__memcpy(%p,%p,%lu,%s)", dst, src, n, called_by_function)

   U_INTERNAL_ASSERT_POINTER(src)
   U_INTERNAL_ASSERT_POINTER(dst)
   U_INTERNAL_ASSERT_POINTER(called_by_function)

   if (n == 0) U_WARNING("*** Zero copy in memcpy *** - %s", called_by_function);

   if (u_is_overlap((const char* restrict)dst, (const char* restrict)src, n))
      {
      U_WARNING("*** Source and Destination OVERLAP in memcpy *** - %s", called_by_function);

      (void) apex_memmove(dst, src, n);
      }

   (void) apex_memcpy(dst, src, n);
}
#endif

void u_init_ulib_username(void)
{
   struct passwd* restrict pw;

   U_INTERNAL_TRACE("u_init_ulib_username()")

   pw = getpwuid(getuid());

   if (pw == 0) u__memcpy(u_user_name, "root", (u_user_name_len = 4), __PRETTY_FUNCTION__);
   else
      {
      u_user_name_len = u__strlen(pw->pw_name, __PRETTY_FUNCTION__);

      U_INTERNAL_ASSERT_MAJOR(u_user_name_len,0)

      u__memcpy(u_user_name, pw->pw_name, u_user_name_len, __PRETTY_FUNCTION__);
      }
}

void u_init_ulib_hostname(void)
{
   U_INTERNAL_TRACE("u_init_ulib_hostname()")

   u_hostname[0]  = 0;
   u_hostname_len = 0;

   if (gethostname(u_hostname, sizeof(u_hostname)) != 0)
      {
      char* restrict tmp = getenv("HOSTNAME"); /* bash setting... */

      if ( tmp &&
          *tmp)
         {
         u_hostname_len = u__strlen(tmp, __PRETTY_FUNCTION__);

         if (u_hostname_len > HOST_NAME_MAX) u_hostname_len = HOST_NAME_MAX;

         u__memcpy(u_hostname, tmp, u_hostname_len, __PRETTY_FUNCTION__);

         u_hostname[u_hostname_len] = 0;
         }
      else
         {
#     ifndef _MSWINDOWS_
         FILE* node = (FILE*) fopen("/proc/sys/kernel/hostname", "r");

         if (node)
            {
            (void) fscanf(node, "%255s", u_hostname);

            (void) fclose(node);

            u_hostname_len = u__strlen(u_hostname, __PRETTY_FUNCTION__);
            }
         else
            {
            struct utsname buf;

            if (uname(&buf) == 0)
               {
               u_hostname_len = u__strlen(buf.nodename, __PRETTY_FUNCTION__);

               u__memcpy(u_hostname, buf.nodename, u_hostname_len, __PRETTY_FUNCTION__);

               u_hostname[u_hostname_len] = 0;
               }
            }
#     endif
         }
      }

   if (u_hostname_len == 0)
      {
      u_hostname_len = u__strlen(u_hostname, __PRETTY_FUNCTION__);

      if (u_hostname_len == 0) u__strncpy(u_hostname, "localhost", (u_hostname_len = 9) + 1);
      }
}

void u_getcwd(void) /* get current working directory */
{
   unsigned newsize = 256;

   U_INTERNAL_TRACE("u_getcwd()")

loop:
   if (u_cwd) free(u_cwd);

   u_cwd = (char*) malloc(newsize);

   if (getcwd(u_cwd, newsize) == 0 &&
       errno == ERANGE)
      {
      newsize += 256;

      U_WARNING("Current working directory need a bigger buffer (%u bytes), doing reallocation", newsize);

      goto loop;
      }

#ifdef _MSWINDOWS_
   u__strcpy(u_cwd, u_slashify(u_cwd, PATH_SEPARATOR, '/'));
#endif

   u_cwd_len = u__strlen(u_cwd, __PRETTY_FUNCTION__);

   U_INTERNAL_ASSERT_MAJOR(u_cwd_len, 0)
   U_INTERNAL_ASSERT_MINOR(u_cwd_len, newsize)
}

static const char* u_months[12]    = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };
static const char* u_months_it[12] = { "gen", "feb", "mar", "apr", "mag", "giu", "lug", "ago", "set", "ott", "nov", "dic" };

__pure unsigned u_getMonth(const char* buf)
{
   unsigned i;

   U_INTERNAL_TRACE("u_getMonth(%s)", buf)

   for (i = 0; i < 12; ++i)
      {
      const char* ptr = u_months[i];

      if ((ptr[0] == u__tolower(buf[0])) &&
          (ptr[1] == u__tolower(buf[1])) &&
          (ptr[2] == u__tolower(buf[2])))
         {
         return i+1;
         }

      ptr = u_months_it[i];

      if ((ptr[0] == u__tolower(buf[0])) &&
          (ptr[1] == u__tolower(buf[1])) &&
          (ptr[2] == u__tolower(buf[2])))
         {
         return i+1;
         }
      }

   return 0;
}

void u_initRandom(void)
{
   U_INTERNAL_TRACE("u_initRandom()")

   U_INTERNAL_ASSERT_POINTER(u_now)
   U_INTERNAL_ASSERT_MAJOR(u_now->tv_usec, 0)

   /**
    * The "hash seed" is a feature to perturb the results to avoid "algorithmic complexity attacks"
    *
    * http://lwn.net/Articles/474365
    */

   u_seed_hash = u_random((uint32_t)u_pid ^ (uint32_t)u_now->tv_usec);

#ifdef SYS_getrandom
   if (syscall(SYS_getrandom, &u_seed_hash, sizeof(u_seed_hash), 0) == sizeof(u_seed_hash)) u_seed_hash |= 1;
   else
#endif
   {
#ifndef U_COVERITY_FALSE_POSITIVE /* RESOURCE_LEAK */
   int         fd = open("/dev/urandom", O_CLOEXEC | O_RDONLY);
   if (fd < 0) fd = open("/dev/random",  O_CLOEXEC | O_RDONLY);
   if (fd > 0)
      {
      if (read(fd, &u_seed_hash, sizeof(u_seed_hash)) == sizeof(u_seed_hash)) u_seed_hash |= 1;

      (void) close(fd);
      }
#endif
   }

   /* seed the random generator */

   u_set_seed_random(u_seed_hash >> 16, u_seed_hash % 4294967296);
}

bool u_setStartTime(void)
{
   struct tm tm;
   time_t t, lnow;
   const char* compilation_date;

   U_INTERNAL_TRACE("u_setStartTime()")

   U_INTERNAL_ASSERT_POINTER(u_now)

   /**
    * calculate number of seconds between UTC to current time zone
    *
    *         time() returns the time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds.
    * gettimeofday() gives the number of seconds and microseconds since the Epoch (see time(2)). The tz argument is a struct timezone:
    *
    * struct timezone {
    *    int tz_minuteswest;  // minutes west of Greenwich
    *    int tz_dsttime;      // type of DST correction
    * };
    */

   (void) gettimeofday(u_now, 0);

   /* initialize time conversion information */

   tzset();

   /**
    * The localtime() function converts the calendar time to broken-time representation, expressed relative
    * to the user's specified timezone. The function acts as if it called tzset(3) and sets the external
    * variables tzname with information about the current timezone, timezone with the difference between
    * Coordinated Universal Time (UTC) and local standard time in seconds, and daylight to a non-zero value
    * if daylight savings time rules apply during some part of the year. The return value points to a
    * statically allocated struct which might be overwritten by subsequent calls to any of the date and time
    * functions. The localtime_r() function does the same, but stores the data in a user-supplied struct. It
    * need not set tzname, timezone, and daylight
    *
    * This variable (daylight) has a nonzero value if Daylight Saving Time rules apply. A nonzero value does
    * not necessarily mean that Daylight Saving Time is now in effect; it means only that Daylight Saving Time
    * is sometimes in effect.
    *
    * This variable (timezone) contains the difference between UTC and the latest local standard time, in seconds
    * west of UTC. For example, in the U.S. Eastern time zone, the value is 5*60*60. Unlike the tm_gmtoff member
    * of the broken-down time structure, this value is not adjusted for daylight saving, and its sign is reversed.
    * In GNU programs it is better to use tm_gmtoff, since it contains the correct offset even when it is not the latest one
    */

   (void) localtime_r(&(u_now->tv_sec), &u_strftime_tm);

#ifdef TM_HAVE_TM_GMTOFF
   u_daylight = (daylight && (timezone != -u_strftime_tm.tm_gmtoff));
#endif

   /**
    * The timegm() function converts the broken-down time representation,
    * expressed in Coordinated Universal Time (UTC) to calendar time
    */

   lnow = timegm(&u_strftime_tm);

   u_now_adjust = (lnow - u_now->tv_sec);

   U_INTERNAL_PRINT("u_now_adjust = %d timezone = %ld daylight = %d u_daylight = %d tzname[2] = { %s, %s }",
                     u_now_adjust,     timezone,      daylight,     u_daylight,     tzname[0], tzname[1])

   U_INTERNAL_ASSERT(u_now_adjust <= ((daylight ? 3600 : 0) - timezone))

   /* NB: check if current date is OK (>= compilation_date) */

   compilation_date = __DATE__; /* Dec  6 2012 */

/* (void) memset(&tm, 0, sizeof(struct tm)); */

   tm.tm_min   = 0;
   tm.tm_hour  = 0;
   tm.tm_mday  =       atoi(compilation_date+4);
   tm.tm_mon   = u_getMonth(compilation_date)   -    1; /* tm relative format month - range from 0-11 */
   tm.tm_year  =       atoi(compilation_date+7) - 1900; /* tm relative format year  - is number of years since 1900 */
   tm.tm_sec   = 1;
   tm.tm_wday  = 0; /* day of the week */
   tm.tm_yday  = 0; /* day in the year */
   tm.tm_isdst = -1;

   t = mktime(&tm); /* NB: The timelocal() function is equivalent to the POSIX standard function mktime(3) */

   U_INTERNAL_PRINT("lnow = %ld t = %ld u_now = { %lu, %lu }", lnow, t, u_now->tv_sec, u_now->tv_usec)

   if (lnow >= t ||
       (t - lnow) < U_ONE_DAY_IN_SECOND)
      {
      if (u_start_time == 0)
         {
         u_start_time = lnow; /* u_now->tv_sec + u_now_adjust */

         u_initRandom();
         }

      /**
       * The mktime() function modifies the fields of the tm structure as follows: tm_wday and tm_yday are set to values
       * determined from the contents of the other fields; if structure members are outside their valid interval, they will
       * be normalized (so that, for example, 40 October is changed into 9 November); tm_isdst is set (regardless of its
       * initial value) to a positive value or to 0, respectively, to indicate whether DST is or is not in effect at the
       * specified time.  Calling mktime() also sets the external variable tzname with information about the current timezone
       */

#  ifndef TM_HAVE_TM_GMTOFF
      u_daylight = (tm.tm_isdst != 0);
#  endif

/*
#  ifdef HAVE_CLOCK_GETTIME
      {
      struct timespec ts;

      (void) clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);

      u_start_clock.tv_sec  = u_now->tv_sec  -  ts.tv_sec;
      u_start_clock.tv_usec = u_now->tv_usec - (ts.tv_nsec / 1000L);

      U_INTERNAL_PRINT("u_now = { %lu, %lu } u_start_clock = { %ld, %ld }", u_now->tv_sec, u_now->tv_usec, u_start_clock.tv_sec, u_start_clock.tv_usec)
      }
#  endif
*/

      U_DEBUG("System date update: u_now_adjust = %d timezone = %ld daylight = %d u_daylight = %d tzname[2] = { %s, %s }",
                                   u_now_adjust,     timezone,      daylight,     u_daylight,     tzname[0], tzname[1])

      return true;
      }

   return false;
}

void u_init_ulib(char** restrict argv)
{
   U_INTERNAL_TRACE("u_init_ulib(%p)", argv)

   u_setPid();

#if defined(U_STATIC_ONLY)
   if (argv == 0)
      {
      u_progpath     =
      u_progname     =                 "ULib";
      u_progname_len = U_CONSTANT_SIZE("ULib");

      if (u_now == 0)
         {
         u_now = &u_timeval;
         }
      }
   else
#endif
   {
   u_progpath = *argv;
   u_progname = u_basename(u_progpath);

   U_INTERNAL_ASSERT_POINTER(u_progname)

   u_progname_len = u__strlen(u_progname, __PRETTY_FUNCTION__);

   U_INTERNAL_ASSERT_MAJOR(u_progname_len, 0)
   }

   U_INTERNAL_ASSERT_EQUALS(u_ulib_init, false)

   u_ulib_init = true;

#ifdef _MSWINDOWS_
   u_init_ulib_mingw();
#endif

#if (defined(_M_X64) || defined(__x86_64__) || defined(__i386) || defined(_M_IX86)) && (defined(_MSC_VER) || (defined(__GNUC__) && defined(HAVE_CPUID_H)))
   { /*  Compiler and architecture test (Intel/AMD Architecture)! Visual Studio and GCC / LLVM (Clang) */
# if defined(_MSC_VER)
   int cpuid[4] = {-1}; /* Visual Studio */
   __cpuid(cpuid, 1);
# else
   unsigned int cpuid[4] = {0,0,0,0}; /* GCC / LLVM (Clang) */
   __get_cpuid(1, &cpuid[0], &cpuid[1], &cpuid[2], &cpuid[3]);
# endif
# ifndef bit_SSE2
# define bit_SSE2 (1 << 26) /* Taken from GCC <cpuid.h> ... just more visual & descriptive! */
# endif
# ifndef bit_SSSE3
# define bit_SSSE3 (1 << 9)
# endif
# ifndef bit_SSE4_2
# define bit_SSE4_2 (1 << 20)
# endif
# if defined(_M_X64) || defined(__x86_64__)  /* 64-bit */
        if ((cpuid[2] & bit_SSE4_2) != 0) u_flag_sse = 42; /* detect SSE4.2, available on Core i and newer processors, they include "fast unaligned" memory access */
# else /* 32-bit */
        if ((cpuid[2] & bit_SSE4_2) != 0) u_flag_sse = 42; /* detect SSE4.2, available on Core i and newer processors, they include "fast unaligned" memory access */
   else if ((cpuid[2] & bit_SSSE3)  != 0) u_flag_sse =  3; /* detect SSSE3,  available on Core/Core 2 and newer */
   else if ((cpuid[3] & bit_SSE2)   != 0) u_flag_sse =  2; /* this is for very, very old computers with SSE2 only! eg. old Pentium 4! */
# endif
   }
#endif

#ifdef USE_HARDWARE_CRC32
   {
   uint32_t h = 0xABAD1DEA;

   U_INTERNAL_ASSERT_EQUALS(u_flag_sse, 42)

# if __x86_64__
   h = (uint32_t)__builtin_ia32_crc32di(h, U_MULTICHAR_CONSTANT64('1','2','3','4','5','6','7','8'));
# else
   h =           __builtin_ia32_crc32si(h, U_MULTICHAR_CONSTANT32('/','o','p','t'));
#endif

   U_INTERNAL_ERROR(h, "hardware crc32 failed (h = %u). Exiting...", h);
   }
#endif

#if !defined(__MINGW32__) && defined(HAVE_ARCH64) && defined(U_APEX_ENABLE)
   apex_memmove_dispatcher();
#endif

   u_getcwd(); /* get current working directory */

#if !defined(_MSWINDOWS_) && defined(DEBUG)
   {
   const char* restrict pwd = getenv("PWD"); /* check for bash setting */

   if (pwd &&
       strncmp(u_cwd, pwd, u_cwd_len) != 0)
      {
      U_WARNING("Current working directory from environment (PWD): %s differ from system getcwd(): %.*s", pwd, u_cwd_len, u_cwd);
      }
   }
#endif

   u_is_tty = isatty(STDERR_FILENO);

#ifdef HAVE_ATEXIT
   (void) atexit(u_exit); /* initialize AT EXIT */
#endif

#ifdef DEBUG
   u_debug_init();
#endif

   (void) u_setStartTime();

   U_DEBUG("u_flag_sse = %u", u_flag_sse)
}

/**
 * Places characters into the array pointed to by s as controlled by the string
 * pointed to by format. If the total number of resulting characters including
 * the terminating null character is not more than maxsize, returns the number
 * of characters placed into the array pointed to by s (not including the
 * terminating null character); otherwise zero is returned and the contents of
 * the array indeterminate
 */

uint32_t u_strftime1(char* restrict buffer, uint32_t maxsize, const char* restrict format, uint32_t fmt_size)
{
   static const int   day_name_len[7]  = { 6, 6, 7, 9, 8, 6, 8 };
   static const int month_name_len[12] = { 7, 8, 5, 5, 3, 4, 4, 6, 9, 7, 8, 8 };

   /**
    * %% A single character %
    * %A The full name for the day of the week
    * %B The full name of the month
    * %H The hour (on a 24-hour clock), formatted with two digits
    * %I The hour (on a 12-hour clock), formatted with two digits
    * %M The minute, formatted with two digits
    * %S The second, formatted with two digits
    * %T The time in 24-hour notation (%H:%M:%S) (SU)
    * %U The week number, formatted with two digits (from 0 to 53; week number 1 is taken as beginning with the first Sunday in a year). See also %W
    * %W Another version of the week number: like %U, but counting week 1 as beginning with the first Monday in a year
    * %X A string representing the full time of day (hours, minutes, and seconds), in a format like 13:13:13
    * %Y The full year, formatted with four digits to include the century
    * %Z Defined by ANSI C as eliciting the time zone if available
    * %a An abbreviation for the day of the week
    * %b An abbreviation for the month name
    * %c A string representing the complete date and time, in the form Mon Apr 01 13:13:13 1992
    * %d The day of the month, formatted with two digits
    * %e Like %d, the day of the month as a decimal number, but a leading zero is replaced by a space
    * %h Equivalent to %b (SU)
    * %j The count of days in the year, formatted with three digits (from 1 to 366)
    * %m The month number, formatted with two digits
    * %p Either AM or PM as appropriate
    * %w A single digit representing the day of the week: Sunday is day 0
    * %x A string representing the complete date, in a format like Mon Apr 01 1992
    * %y The last two digits of the year
    * %z The +hhmm or -hhmm numeric timezone (that is, the hour and minute offset from UTC)
    */

   static const int dispatch_table[] = {
      (char*)&&case_A-(char*)&&cdefault,/* 'A' */
      (char*)&&case_B-(char*)&&cdefault,/* 'B' */
      0,/* 'C' */
      0,/* 'D' */
      0,/* 'E' */
      0,/* 'F' */
      0,/* 'G' */
      (char*)&&case_H-(char*)&&cdefault,/* 'H' */
      (char*)&&case_I-(char*)&&cdefault,/* 'I' */
      0,/* 'J' */
      0,/* 'K' */
      0,/* 'L' */
      (char*)&&case_M-(char*)&&cdefault,/* 'M' */
      0,/* 'N' */
      0,/* 'O' */
      0,/* 'P' */
      0,/* 'Q' */
      0,/* 'R' */
      (char*)&&case_S-(char*)&&cdefault,/* 'S' */
      (char*)&&case_T-(char*)&&cdefault,/* 'T' */
      (char*)&&case_U-(char*)&&cdefault,/* 'U' */
      0,/* 'V' */
      (char*)&&case_W-(char*)&&cdefault,/* 'W' */
      (char*)&&case_T-(char*)&&cdefault,/* 'X' */
      (char*)&&case_Y-(char*)&&cdefault,/* 'Y' */
      (char*)&&case_Z-(char*)&&cdefault,/* 'Z' */
      0,/* '[' */
      0,/* '\' */
      0,/* ']' */
      0,/* '^' */
      0,/* '_' */
      0,/* '`' */
      (char*)&&case_a-(char*)&&cdefault,/* 'a' */
      (char*)&&case_b-(char*)&&cdefault,/* 'b' */
      (char*)&&case_c-(char*)&&cdefault,/* 'c' */
      (char*)&&case_d-(char*)&&cdefault,/* 'd' */
      (char*)&&case_e-(char*)&&cdefault,/* 'e' */
      0,/* 'f' */
      0,/* 'g' */
      (char*)&&case_b-(char*)&&cdefault,/* 'h' */
      0,/* 'i' */
      (char*)&&case_j-(char*)&&cdefault,/* 'j' */
      0,/* 'k' */
      0,/* 'l' */
      (char*)&&case_m-(char*)&&cdefault,/* 'm' */
      0,/* 'n' */
      0,/* 'o' */
      (char*)&&case_p-(char*)&&cdefault,/* 'p' */
      0,/* 'q' */
      0,/* 'r' */
      0,/* 's' */
      0,/* 't' */
      0,/* 'u' */
      0,/* 'v' */
      (char*)&&case_w-(char*)&&cdefault,/* 'w' */
      (char*)&&case_x-(char*)&&cdefault,/* 'x' */
      (char*)&&case_y-(char*)&&cdefault,/* 'y' */
      (char*)&&case_z-(char*)&&cdefault /* 'z' */
   };

   char ch;    /* character from format */
   int n, val; /* handy integer (short term usage) */

   uint32_t ret;
   const char* restrict fp;
   char* restrict bp = buffer;
   const char* restrict fmark = format; /* for remembering a place in format */

#ifdef DEBUG
   uint32_t format_size_save = fmt_size;
#endif

   U_INTERNAL_TRACE("u_strftime1(%p,%u,%.*s,%u)", buffer, maxsize, fmt_size, format, fmt_size)

   U_INTERNAL_ASSERT_POINTER(format)
   U_INTERNAL_ASSERT_MAJOR(maxsize, 0)
   U_INTERNAL_ASSERT_MAJOR(fmt_size, 0)

   do {
      U_INTERNAL_ERROR((bp-buffer) <= maxsize, "BUFFER OVERFLOW at u_strftime1() ret = %lu maxsize = %u format = \"%.*s\"", (bp-buffer), maxsize, format_size_save, format);

      /* Scan the format for conversions ('%' character) */

      fp = (const char* restrict) memchr(fmark, '%', fmt_size);

      U_INTERNAL_PRINT("fp = %p fmark = %p fmt_size = %u", fp, fmark, fmt_size)

      if (fp == 0)
         {
         U_INTERNAL_ASSERT_MAJOR(fmt_size, 0)

         u__memcpy(bp, fmark, fmt_size, __PRETTY_FUNCTION__);
                   bp +=      fmt_size;

         break;
         }

      if ((n = (fp - fmark)))
         {
         u__memcpy(bp, fmark, n, __PRETTY_FUNCTION__);
                   bp +=      n;
             fmt_size -=      n;
         }

      /* skip over '%' */

      ch = fp[1];

      fmark = fp+2;

      fmt_size -= 2;

      if (u__isalpha(ch) == false)
         {
cdefault:
         *bp++ = '%'; /* "%%" prints % */

         if (ch != '%') /* "%?" prints %?, unless ? is 0: pretend it was %c with argument ch */
            {
            if (ch == '\0') break;

            *bp++ = ch;
            }

         continue;
         }

      U_INTERNAL_PRINT("dispatch_table[%d] = %p &&cdefault = %p", ch-'A', dispatch_table[ch-'A'], &&cdefault)

      goto *((char*)&&cdefault + dispatch_table[ch-'A']);

case_A: /* %A The full name for the day of the week */
      (void) u__memcpy(bp, u_day_name[u_strftime_tm.tm_wday], day_name_len[u_strftime_tm.tm_wday], __PRETTY_FUNCTION__);

      bp += day_name_len[u_strftime_tm.tm_wday];

      continue;

case_B: /* %B The full name of the month */
      (void) u__memcpy(bp, u_month_name[u_strftime_tm.tm_mon], month_name_len[u_strftime_tm.tm_mon], __PRETTY_FUNCTION__);

      bp += month_name_len[u_strftime_tm.tm_mon];

      continue;

case_H: /* %H The hour (on a 24-hour clock), formatted with two digits */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-2))

   /* if ((bp-buffer) >= (maxsize-2)) return 0; */

      U_NUM2STR16(bp, u_strftime_tm.tm_hour);

      bp += 2;

      continue;

case_I: /* %I The hour (on a 12-hour clock), formatted with two digits */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-2))

   /* if ((bp-buffer) >= (maxsize-2)) return 0; */

      if (u_strftime_tm.tm_hour == 0 ||
          u_strftime_tm.tm_hour == 12)
         {
         u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16('1', '2'));
         }
      else
         {
         val = u_strftime_tm.tm_hour % 12;

         u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16('0' + (val >= 10 ? (val / 10) : 0),
                                                       '0' + (val  % 10)));
         }

      bp += 2;

      continue;

case_M: /* %M The minute, formatted with two digits */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-2))

   /* if ((bp-buffer) >= (maxsize-2)) return 0; */

      U_NUM2STR16(bp, u_strftime_tm.tm_min);

      bp += 2;

      continue;

case_S: /* %S The second, formatted with two digits */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-2))

   /* if ((bp-buffer) >= (maxsize-2)) return 0; */

      U_NUM2STR16(bp, u_strftime_tm.tm_sec);

      bp += 2;

      continue;

case_T: /* %X A string representing the full time of day (hours, minutes, and seconds), in a format like 13:13:13 - %T The time in 24-hour notation (%H:%M:%S) (SU) */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-8))

   /* if ((bp-buffer) >= (maxsize-8)) return 0; */

      U_NUM2STR64(bp, ':', u_strftime_tm.tm_hour, u_strftime_tm.tm_min, u_strftime_tm.tm_sec);

      bp += 8;

      continue;

case_U: /* %U The week number, formatted with two digits (from 0 to 53; week number 1 is taken as beginning with the first Sunday in a year). See also %W */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-2))

   /* if ((bp-buffer) >= (maxsize-2)) return 0; */

      U_NUM2STR16(bp, (u_strftime_tm.tm_yday + 7 - u_strftime_tm.tm_wday) / 7);

      bp += 2;

      continue;

case_W: /* %W Another version of the week number: like %U, but counting week 1 as beginning with the first Monday in a year */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-2))

   /* if ((bp-buffer) >= (maxsize-2)) return 0; */

      U_NUM2STR16(bp, (u_strftime_tm.tm_yday + ((8-u_strftime_tm.tm_wday) % 7)) / 7);

      bp += 2;

      continue;

case_Y: /* %Y The full year, formatted with four digits to include the century */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-4))

   /* if ((bp-buffer) >= (maxsize-4)) return 0; */

      (void) sprintf(bp, "%.4d", 1900 + u_strftime_tm.tm_year);

      U_INTERNAL_ASSERT_EQUALS(strlen(bp), 4)

      bp += 4;

      continue;

case_Z: /* %Z Defined by ANSI C as eliciting the time zone if available */
      n = u__strlen(tzname[u_daylight], __PRETTY_FUNCTION__);

      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-n))

   /* if ((bp-buffer) >= (maxsize-n)) return 0; */

      (void) u__memcpy(bp, tzname[u_daylight], n, __PRETTY_FUNCTION__);

      bp += n;

      continue;

case_a: /* %a An abbreviation for the day of the week */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-3))

   /* if ((bp-buffer) >= (maxsize-3)) return 0; */

      u_put_unalignedp32(bp, U_MULTICHAR_CONSTANT32(u_day_name[u_strftime_tm.tm_wday][0],
                                                    u_day_name[u_strftime_tm.tm_wday][1],
                                                    u_day_name[u_strftime_tm.tm_wday][2],' '));

      bp += 3;

      continue;

case_b: /* %b An abbreviation for the month name - %h Equivalent to %b (SU) */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-3))

   /* if ((bp-buffer) >= (maxsize-3)) return 0; */

      u_put_unalignedp32(bp, U_MULTICHAR_CONSTANT32(u_month_name[u_strftime_tm.tm_mon][0],
                                                    u_month_name[u_strftime_tm.tm_mon][1],
                                                    u_month_name[u_strftime_tm.tm_mon][2],' '));

      bp += 3;

      continue;

case_c: /* %c A string representing the complete date and time, in the form Mon Apr 01 13:13:13 1992 */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-24))

   // if ((bp-buffer) >= (maxsize-24)) return 0;

      u_put_unalignedp32(bp, U_MULTICHAR_CONSTANT32(u_day_name[u_strftime_tm.tm_wday][0],
                                                    u_day_name[u_strftime_tm.tm_wday][1],
                                                    u_day_name[u_strftime_tm.tm_wday][2],' '));

      u_put_unalignedp32(bp+4, U_MULTICHAR_CONSTANT32(u_month_name[u_strftime_tm.tm_mon][0],
                                                      u_month_name[u_strftime_tm.tm_mon][1],
                                                      u_month_name[u_strftime_tm.tm_mon][2],' '));

      (void) sprintf(bp+8, "%.2d %2.2d:%2.2d:%2.2d %.4d", u_strftime_tm.tm_mday, u_strftime_tm.tm_hour,
                                                          u_strftime_tm.tm_min,  u_strftime_tm.tm_sec,
                                                          1900 + u_strftime_tm.tm_year);

      U_INTERNAL_ASSERT_EQUALS(strlen(bp), 8+17)

      bp += 8+17;

      continue;

case_d: /* %d The day of the month, formatted with two digits */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-2))

   /* if ((bp-buffer) >= (maxsize-2)) return 0; */

      U_NUM2STR16(bp, u_strftime_tm.tm_mday);

      bp += 2;

      continue;

case_e: /* %e Like %d, the day of the month as a decimal number, but a leading zero is replaced by a space */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-2))

   /* if ((bp-buffer) >= (maxsize-2)) return 0; */

      val = (u_strftime_tm.tm_mday >= 10 ? (u_strftime_tm.tm_mday / 10) : 0);

      u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16(val ? '0' + val : ' ',
                                                          '0' + (u_strftime_tm.tm_mday % 10)));

      bp += 2;

      continue;

case_j: /* %j The count of days in the year, formatted with three digits (from 1 to 366) */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-3))

   /* if ((bp-buffer) >= (maxsize-3)) return 0; */

      (void) sprintf(bp, "%.3d", u_strftime_tm.tm_yday+1);

      U_INTERNAL_ASSERT_EQUALS(strlen(bp), 3)

      bp += 3;

      continue;

case_m: /* %m The month number, formatted with two digits */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-2))

   /* if ((bp-buffer) >= (maxsize-2)) return 0; */

      U_NUM2STR16(bp, u_strftime_tm.tm_mon+1);

      bp += 2;

      continue;

case_p: /* %p Either AM or PM as appropriate */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-2))

   /* if ((bp-buffer) >= (maxsize-2)) return 0; */

      u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16(u_strftime_tm.tm_hour < 12 ? 'A' : 'P','M'));

      bp += 2;

      continue;

case_w: /* %w A single digit representing the day of the week: Sunday is day 0 */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-1))

   /* if ((bp-buffer) >= (maxsize-1)) return 0; */

      *bp++ = '0' + (u_strftime_tm.tm_wday % 10);

      continue;

case_x: /* %x A string representing the complete date, in a format like Mon Apr 01 1992 */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-15))

   /* if ((bp-buffer) >= (maxsize-15)) return 0; */

      u_put_unalignedp32(bp, U_MULTICHAR_CONSTANT32(u_day_name[u_strftime_tm.tm_wday][0],
                                                    u_day_name[u_strftime_tm.tm_wday][1],
                                                    u_day_name[u_strftime_tm.tm_wday][2],' '));

      u_put_unalignedp32(bp+4, U_MULTICHAR_CONSTANT32(u_month_name[u_strftime_tm.tm_mon][0],
                                                      u_month_name[u_strftime_tm.tm_mon][1],
                                                      u_month_name[u_strftime_tm.tm_mon][2],' '));

      (void) sprintf(bp+8, "%.2d %.4d", u_strftime_tm.tm_mday, 1900 + u_strftime_tm.tm_year);

      U_INTERNAL_ASSERT_EQUALS(strlen(bp), 8+7)

      bp += 8+7;

      continue;

case_y: /* %y The last two digits of the year */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-2))

   /* if ((bp-buffer) >= (maxsize-2)) return 0; */

      /**
       * The year could be greater than 100, so we need the value modulo 100.
       * The year could be negative, so we need to correct for a possible negative remainder
       */

      U_NUM2STR16(bp, ((u_strftime_tm.tm_year % 100) + 100) % 100);

      bp += 2;

      continue;

case_z: /* %z The +hhmm or -hhmm numeric timezone (that is, the hour and minute offset from UTC) */
      U_INTERNAL_ASSERT((bp-buffer) <= (maxsize-5))

   /* if ((bp-buffer) >= (maxsize-5)) return 0; */

      val = (u_now_adjust / 3600);

      if (val > 0)
         {
         *bp++ = '+';

         U_NUM2STR16(bp, val);
         }
      else
         {
         *bp++ = '-';

         U_NUM2STR16(bp, -val);
         }

      U_NUM2STR16(bp+2, u_now_adjust % 3600);

      bp += 4;
      }
   while (fmt_size);

   ret = (bp-buffer);

   U_INTERNAL_PRINT("ret = %u maxsize = %u", ret, maxsize)

   if (ret < maxsize) *bp = '\0';

   U_INTERNAL_ERROR(ret <= maxsize, "BUFFER OVERFLOW at u_strftime1() ret = %u maxsize = %u format = \"%.*s\"", ret, maxsize, format_size_save, format);

   return ret;
}

#ifdef DEBUG
#  include <ulib/base/trace.h>

static bool u_askForContinue(void)
{
   U_INTERNAL_TRACE("u_askForContinue()")

   if (u_is_tty &&
       isatty(STDIN_FILENO))
      {
      char ch[2];

      // NB: we use U_MESSAGE here, but we are already inside u__printf()...

      int u_flag_exit_save = u_flag_exit;
                             u_flag_exit = 0;

      U_MESSAGE("Press '%Wc%W' to continue, '%W%s%W' to exit: %W", GREEN, YELLOW, RED, "Enter", YELLOW, RESET);

      u_flag_exit = u_flag_exit_save;

      if (read(STDIN_FILENO, ch, 1) == 1 &&
          ch[0] == 'c'                   &&
          read(STDIN_FILENO, ch, 1) == 1) /* get 'return' key */
         { 
         return true;
         }
      }

   return false;
}
#endif

void u_internal_print(bool abrt, const char* restrict format, ...)
{
   uint32_t bytes_written;
   char u_internal_buf[16 * 1024];

   va_list argp;
   va_start(argp, format);

   (void) vsnprintf(u_internal_buf, sizeof(u_internal_buf), format, argp);

   va_end(argp);

#ifdef DEBUG
   if (abrt) u_printError();
#endif

   bytes_written = strlen(u_internal_buf);

   (void) write(STDERR_FILENO, u_internal_buf, bytes_written);

   if (abrt)
      {
#  ifdef DEBUG
      if (u_trace_fd > STDERR_FILENO) /* NB: write error on trace file, check stderr to avoid duplication of message on terminal */
         {
         struct iovec iov[1] = { { (caddr_t)u_internal_buf, bytes_written } };

         u_trace_writev(iov, 1);
         }

      if (u_askForContinue() == false)
#  endif
      {
      u_flag_exit = -2; // abort...

#  ifdef DEBUG
      u_debug_at_exit();
#  endif
      }
      }
}

/**
 * Encode escape sequences into a buffer, the following are recognized:
 * --------------------------------------------------------------------
 *  \a  BEL                 (\007  7  7)
 *  \b  BS  backspace       (\010  8  8) 
 *  \t  HT  horizontal tab  (\011  9  9)
 *  \n  LF  newline         (\012 10  A) 
 *  \v  VT  vertical tab    (\013 11  B)
 *  \f  FF  formfeed        (\014 12  C) 
 *  \r  CR  carriage return (\015 13  D)
 *  \e  ESC character       (\033 27 1B)
 *
 *  \DDD number formed of 1-3 octal digits
 * --------------------------------------------------------------------
 */

#define U_CTL_ENTRY(name) {name, U_CONSTANT_SIZE(name)}

uint32_t u_sprintcrtl(char* restrict out, unsigned char c)
{
   struct control_info {
      const char* restrict name;
      uint32_t len;
   };

   static const struct control_info control_table[32] = {
    U_CTL_ENTRY("\\000"), U_CTL_ENTRY("\\001"), U_CTL_ENTRY("\\002"), U_CTL_ENTRY("\\003"), U_CTL_ENTRY("\\004"), U_CTL_ENTRY("\\005"), U_CTL_ENTRY("\\006"),
    U_CTL_ENTRY("\\007"), U_CTL_ENTRY("\\b"  ), U_CTL_ENTRY("\\t"  ), U_CTL_ENTRY("\\n"  ), U_CTL_ENTRY("\\013"), U_CTL_ENTRY("\\f"  ), U_CTL_ENTRY("\\r"  ),
    U_CTL_ENTRY("\\016"), U_CTL_ENTRY("\\017"), U_CTL_ENTRY("\\020"), U_CTL_ENTRY("\\021"), U_CTL_ENTRY("\\022"), U_CTL_ENTRY("\\023"), U_CTL_ENTRY("\\024"),
    U_CTL_ENTRY("\\025"), U_CTL_ENTRY("\\026"), U_CTL_ENTRY("\\027"), U_CTL_ENTRY("\\030"), U_CTL_ENTRY("\\031"), U_CTL_ENTRY("\\032"), U_CTL_ENTRY("\\033"),
    U_CTL_ENTRY("\\034"), U_CTL_ENTRY("\\035"), U_CTL_ENTRY("\\036"), U_CTL_ENTRY("\\037")
   };

   U_INTERNAL_TRACE("u_sprintcrtl(%p,%d)", out, c)

   u__memcpy(out, control_table[c].name, control_table[c].len, __PRETTY_FUNCTION__);

   return control_table[c].len;
}

#undef U_CTL_ENTRY

uint32_t u_sprintc(char* restrict out, unsigned char c)
{
   U_INTERNAL_TRACE("u_sprintc(%p,%d)", out, c)

   if (c < 32) return u_sprintcrtl(out, c);

   if (c == '"' || /* 0x22 */
       c == '\\')  /* 0x5C */
      {
      *out++ = '\\';
      *out   = c;

      return 2;
      }

   if (c > 126)
      {
      char* restrict cp;

      /* \DDD number formed of 1-3 octal digits */

      *out++ = '\\';

      cp = out + 3;

      do {
         *--cp = (c & 7) + '0';

         c >>= 3;
         }
      while (c);

      while (--cp >= out) *cp = '0';

      return 4;
      }

   *out = c;

   return 1;
}

#if !defined(_MSWINDOWS_) && !defined(__UNIKERNEL__)
static const char* tab_color[] = { U_RESET_STR,
   U_BLACK_STR,       U_RED_STR,           U_GREEN_STR,       U_YELLOW_STR,
   U_BLUE_STR,        U_MAGENTA_STR,       U_CYAN_STR,        U_WHITE_STR,
   U_BRIGHTBLACK_STR, U_BRIGHTRED_STR,     U_BRIGHTGREEN_STR, U_BRIGHTYELLOW_STR,
   U_BRIGHTBLUE_STR,  U_BRIGHTMAGENTA_STR, U_BRIGHTCYAN_STR,  U_BRIGHTWHITE_STR };
#endif

/**
 * Print with format extension: bBCDHMNOPQrRSUvVYwW
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

uint32_t u__vsnprintf(char* restrict buffer, uint32_t buffer_size, const char* restrict format, uint32_t fmt_size, va_list argp)
{
   static const int dispatch_table[] = {
      (char*)&&case_space-(char*)&&cdefault, /* ' ' */
      0,/* '!' */
      0,/* '"' */
      (char*)&&case_alt-(char*)&&cdefault,/* '#' */
      0,/* '$' */
      0,/* '%' */
      0,/* '&' */
      (char*)&&case_quote-(char*)&&cdefault, /* '\'' */
      0,/* '(' */
      0,/* ')' */
      (char*)&&case_asterisk-(char*)&&cdefault,/* '*' */
      (char*)&&case_plus-(char*)&&cdefault,    /* '+' */
      0,/* ',' */
      (char*)&&case_minus-(char*)&&cdefault, /* '-' */
      (char*)&&case_period-(char*)&&cdefault,/* '.' */
      0,/* '/' */
      (char*)&&case_zero-(char*)&&cdefault,  /* '0' */
      (char*)&&case_digit-(char*)&&cdefault, /* '1' */
      (char*)&&case_digit-(char*)&&cdefault, /* '2' */
      (char*)&&case_digit-(char*)&&cdefault, /* '3' */
      (char*)&&case_digit-(char*)&&cdefault, /* '4' */
      (char*)&&case_digit-(char*)&&cdefault, /* '5' */
      (char*)&&case_digit-(char*)&&cdefault, /* '6' */
      (char*)&&case_digit-(char*)&&cdefault, /* '7' */
      (char*)&&case_digit-(char*)&&cdefault, /* '8' */
      (char*)&&case_digit-(char*)&&cdefault, /* '9' */
      0,/* ':' */
      0,/* ';' */
      0,/* '<' */
      0,/* '=' */
      0,/* '>' */
      0,/* '?' */
      0,/* '@' */
      (char*)&&case_float-(char*)&&cdefault,/* 'A' */
      (char*)&&case_B-(char*)&&cdefault,/* 'B' */
      (char*)&&case_C-(char*)&&cdefault,/* 'C' */
      (char*)&&case_D-(char*)&&cdefault,/* 'D' */
      (char*)&&case_float-(char*)&&cdefault,/* 'E' */
      (char*)&&case_float-(char*)&&cdefault,/* 'F' */
      (char*)&&case_float-(char*)&&cdefault,/* 'G' */
      (char*)&&case_H-(char*)&&cdefault,/* 'H' */
      (char*)&&case_I-(char*)&&cdefault,/* 'I' */
      (char*)&&case_J-(char*)&&cdefault,/* 'J' */
      0,/* 'K' */
      (char*)&&case_L-(char*)&&cdefault,/* 'L' */
      (char*)&&case_M-(char*)&&cdefault,/* 'M' */
      (char*)&&case_N-(char*)&&cdefault,/* 'N' */
      (char*)&&case_str-(char*)&&cdefault,/* 'O' */
      (char*)&&case_P-(char*)&&cdefault,/* 'P' */
      (char*)&&case_Q-(char*)&&cdefault,/* 'Q' */
      (char*)&&case_R-(char*)&&cdefault,/* 'R' */
      (char*)&&case_str-(char*)&&cdefault,/* 'S' */
      (char*)&&case_T-(char*)&&cdefault,/* 'T' */
      (char*)&&case_U-(char*)&&cdefault,/* 'U' */
      (char*)&&case_V-(char*)&&cdefault,/* 'V' */
      (char*)&&case_W-(char*)&&cdefault,/* 'W' */
      (char*)&&case_X-(char*)&&cdefault,/* 'X' */
      (char*)&&case_Y-(char*)&&cdefault,/* 'Y' */
      0,/* 'Z' */
      0,/* '[' */
      0,/* '\' */
      0,/* ']' */
      0,/* '^' */
      0,/* '_' */
      0,/* '`' */
      (char*)&&case_float-(char*)&&cdefault,/* 'a' */
      (char*)&&case_b-(char*)&&cdefault,/* 'b' */
      (char*)&&case_c-(char*)&&cdefault,/* 'c' */
      (char*)&&case_d-(char*)&&cdefault,/* 'd' */
      (char*)&&case_float-(char*)&&cdefault,/* 'e' */
      (char*)&&case_float-(char*)&&cdefault,/* 'f' */
      (char*)&&case_float-(char*)&&cdefault,/* 'g' */
      (char*)&&case_h-(char*)&&cdefault,/* 'h' */
      (char*)&&case_d-(char*)&&cdefault,/* 'i' */
      (char*)&&case_j-(char*)&&cdefault,/* 'j' */
      0,/* 'k' */
      (char*)&&case_l-(char*)&&cdefault,/* 'l' */
      0,/* 'm' */
      0,/* 'n' */
      (char*)&&case_o-(char*)&&cdefault,/* 'o' */
      (char*)&&case_p-(char*)&&cdefault,/* 'p' */
      (char*)&&case_q-(char*)&&cdefault,/* 'q' */
      (char*)&&case_r-(char*)&&cdefault,/* 'r' */
      (char*)&&case_str-(char*)&&cdefault,/* 's' */
      0,/* 't' */
      (char*)&&case_u-(char*)&&cdefault,/* 'u' */
      (char*)&&case_v-(char*)&&cdefault,/* 'v' */
      (char*)&&case_w-(char*)&&cdefault,/* 'w' */
      (char*)&&case_X-(char*)&&cdefault,/* 'x' */
      0,/* 'y' */
      0 /* 'z' */
   };

   int pads;    /* extra padding size */
   int dpad;    /* extra 0 padding needed for integers */
   int bpad;    /* extra blank padding needed */
   int size;    /* size of converted field or string */
   int width;   /* width from format (%8d), or 0 */
   int prec;    /* precision from format (%.3d), or -1 */
   int dprec;   /* a copy of prec if [diouxX], 0 otherwise */
   int fieldsz; /* field size expanded by sign, dpad etc */

   char ch;                     /* character from format */
   time_t t;
   char sign;                   /* sign prefix (' ', '+', '-', or \0) */
   unsigned char c;
   uint64_t argument;           /* integer arguments %[diIouxX] */
   int i, n, remaining;         /* handy integer (short term usage) */
   struct U_DATA udata;
   unsigned char buf[32];       /* space for %[cdiouxX] or fmt for float/double */
   struct ustringrep* pstr;
   uint32_t ret, len, maxlen;
   unsigned char* restrict cp;  /* handy char pointer (short term usage) */
   enum { OCT, DEC, HEX } base; /* base for [diIouxX] conversion */

   /* Flags used during conversion */

   int flags;

#  define LONGINT           0x001 /* long integer */
#  define LLONGINT          0x002 /* long long integer */
#  define LONGDBL           0x004 /* long double */
#  define SHORTINT          0x008 /* short integer */
#  define ALT               0x010 /* alternate form */
#  define LADJUST           0x020 /* left adjustment */
#  define ZEROPAD           0x040 /* zero (as opposed to blank) */
#  define HEXPREFIX         0x080 /* add 0x or 0X prefix */
#  define THOUSANDS_GROUPED 0x100 /* For decimal conversion (i,d,u,f,F,g,G) the output is to be grouped with thousands */

   /* To extend shorts properly, we need both signed and unsigned argument extraction methods */

#  define VA_ARG(type) va_arg(argp, type)

#  define SARG() (flags & LLONGINT ?                      VA_ARG(int64_t) : \
                  flags &  LONGINT ? (int64_t)            VA_ARG(long) : \
                  flags & SHORTINT ? (int64_t)(int16_t)   VA_ARG(int)  : \
                                     (int64_t)            VA_ARG(int))
#  define UARG() (flags & LLONGINT ?                      VA_ARG(uint64_t) : \
                  flags &  LONGINT ? (uint64_t)           VA_ARG(unsigned long) : \
                  flags & SHORTINT ? (uint64_t)(uint16_t) VA_ARG(int) : \
                                     (uint64_t)           VA_ARG(unsigned int))

   const char* restrict fp;
   char* restrict bp = buffer;
   const char* restrict fmark = format; /* for remembering a place in format */

#ifdef DEBUG
   uint32_t format_size_save = fmt_size;
#endif

   U_INTERNAL_TRACE("u__vsnprintf(%p,%u,%.*s,%u)", buffer, buffer_size, fmt_size, format, fmt_size)

   U_INTERNAL_ERROR(buffer_size, "ZERO BUFFER SIZE at u__vsnprintf() format = \"%.*s\"", format_size_save, format);

   U_INTERNAL_ASSERT_POINTER(format)
   U_INTERNAL_ASSERT_MAJOR(fmt_size, 0)

   do {
      U_INTERNAL_ERROR((bp-buffer) <= buffer_size, "BUFFER OVERFLOW at u__vsnprintf() ret = %lu buffer_size = %u format = \"%.*s\"", (bp-buffer), buffer_size, format_size_save, format);

      /* Scan the format for conversions ('%' character) */

      fp = (const char* restrict) memchr(fmark, '%', fmt_size);

      U_INTERNAL_PRINT("fp = %p fmark = %p fmt_size = %u", fp, fmark, fmt_size)

      if (fp == 0)
         {
         U_INTERNAL_ASSERT_MAJOR(fmt_size, 0)

         u__memcpy(bp, fmark, fmt_size, __PRETTY_FUNCTION__);
                   bp +=      fmt_size;

         break;
         }

      if ((n = (fp - fmark)))
         {
         u__memcpy(bp, fmark, n, __PRETTY_FUNCTION__);
                   bp +=      n;
             fmt_size -=      n;
         }

      /* init var */

      sign  = 0;
      prec  = -1;
      width =
      flags =
      dprec = 0;

      /* skip over '%' */

      --fmt_size;

      fmark = fp+1;

rflag:
      ch = *fmark++;

      --fmt_size;

reswitch:
      U_INTERNAL_PRINT("prec = %d sign = %d width = %d u__isprintf(%c) = %u", prec, sign, width, ch, u__isprintf(ch))

      if (UNLIKELY(u__isprintf(ch) == false))
         {
cdefault:
         *bp++ = '%'; /* "%%" prints % */

         if (ch != '%') /* "%?" prints %?, unless ? is 0: pretend it was %c with argument ch */
            {
            if (ch == '\0') break;

            *bp++ = ch;
            }

         continue;
         }

      U_INTERNAL_PRINT("dispatch_table[%d] = %p &&cdefault = %p", ch-'A', dispatch_table[ch-' '], &&cdefault)

      goto *((char*)&&cdefault + dispatch_table[ch-' ']);

case_space: /* If the space and + flags both appear, the space flag will be ignored */
      if (sign == 0) sign = ' ';

      goto rflag;

case_alt: /* field flag characters: # */
      flags |= ALT;

      goto rflag;

case_quote: /* For decimal conversion (i,d,u,f,F,g,G) the output is to be grouped with thousands */
      flags |= THOUSANDS_GROUPED;

      goto rflag;

case_asterisk: /* A negative field width argument is taken as a - flag followed by a positive field width. They don't exclude field widths read from args */
      if ((width = VA_ARG(int)) >= 0) goto rflag;

      width = -width;

case_minus: /* field flag characters: - */
      flags |= LADJUST;
      flags &= ~ZEROPAD; /* '-' disables '0' */

      goto rflag;

case_plus: /* field flag characters: + */
      sign = '+';

      goto rflag;

case_period: /* The field precision '.' */
      fp = fmark;

      if ((ch = *fmark++) != '*')
         {
         for (prec = 0; u__isdigit(ch); ch = *fmark++) prec = (prec*10) + (ch-'0');
         }
      else
         {
         prec = VA_ARG(int);

         ch = *fmark++;

         if (u__tolower(ch) == 's')
            {
            U_INTERNAL_ASSERT_EQUALS(fmark - fp, 2)

            fmt_size -= 2;

            goto case_str;
            }
         }

      fmt_size -= (fmark - fp);

      goto reswitch;

case_zero: /* Note that 0 is taken as a flag, not as the beginning of a field width */
      if ((flags & LADJUST) == 0) flags |= ZEROPAD; /* '-' disables '0' */

      goto rflag;

case_digit: /* field width: [1-9] - An optional decimal digit string (with nonzero first digit) specifying a minimum field width */
      U_INTERNAL_ASSERT_EQUALS(width, 0)

      fp = fmark;

      do {
         width = width*10 + (ch-'0');

         ch = *fmark++;
         }
      while (u__isdigit(ch));

      fmt_size -= (fmark - fp);

      goto reswitch;

case_float:
      if (ch == 'g'  &&
          prec == -1 &&
          width == 0 &&
          (flags & (LADJUST | ZEROPAD | THOUSANDS_GROUPED | LONGDBL)) == 0)
         {
         double dbl = VA_ARG(double);

         len = u_dtoa(dbl, bp) - bp;
         }
      else
         {
         buf[0] = '%';

         cp = buf + 1;

         if ((flags & ALT)     != 0)           *cp++ = '#';
         if ((flags & ZEROPAD) != 0)           *cp++ = '0';
         if ((flags & LADJUST) != 0)           *cp++ = '-';
         if ((flags & THOUSANDS_GROUPED) != 0) *cp++ = '\'';
         if (sign)                             *cp++ = sign;

         u_put_unalignedp32(cp, U_MULTICHAR_CONSTANT32('*','.','*','L')); /* width, prec */

         u_put_unalignedp16(cp + ((flags & LONGDBL) == 0 ? 3 : 4), U_MULTICHAR_CONSTANT16(ch,'\0'));

         if ((flags & LONGDBL) != 0)
            {
            long double ldbl = VA_ARG(long double);

            len = sprintf(bp, (const char* restrict)buf, width, prec, ldbl);
            }
         else
            {
            double dbl = VA_ARG(double);

            len = sprintf(bp, (const char* restrict)buf, width, prec, dbl);
            }

         U_INTERNAL_ASSERT_EQUALS(len, strlen(bp))
         }

      bp += len;

      continue;

case_str:
      /* s: print                     string */
      /* S: print formatted           string */
      /* O: print formatted temporary string + plus free(string) */

      cp = VA_ARG(unsigned char* restrict);

      U_INTERNAL_PRINT("cp = %p ch = %c prec = %d width = %d", cp, ch, prec, width)

      if (cp == 0)
         {
         if (prec == 0)
            {
            if (ch == 'S') goto empty;
            }
         else if (prec == -1)
            {
            u_put_unalignedp32(bp,   U_MULTICHAR_CONSTANT32('(','n','u','l'));
            u_put_unalignedp16(bp+4, U_MULTICHAR_CONSTANT16('l',')'));

            bp += 6;
            }

         continue;
         }

      if (ch != 's')
         {
case_ustring_V:
         U_INTERNAL_PRINT("prec = %d u_printf_string_max_length = %d", prec, u_printf_string_max_length)

         if (prec == 0)
            {
empty:      u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16('"','"'));

            bp += 2;

            continue;
            }

         len = prec;

#     if defined(DEBUG) && defined(U_STDCPP_ENABLE)
         if (ch == 'O') maxlen = 4096; 
         else
#     endif
         maxlen = (u_printf_string_max_length > 0 ? u_printf_string_max_length : 128);

         if (prec < 0 || /* NB: no precision specified... */
             prec > (int)maxlen)
            {
            prec = maxlen;
            }

         U_INTERNAL_ASSERT_EQUALS(sign, 0)

         n         = 0;
         remaining = buffer_size - (bp-buffer);

         if ((flags & ALT) != 0) /* NB: # -> force print of all binary string (compatibly with buffer size)... */
            {
            remaining -= (prec * 2);

            if (u__isprint(*cp) == false &&
                u__isspace(*cp) == false)
               {
               sign = 1; /* we want to print buffer as exadecimal... */
               }
            }

         *bp++ = '"';

         while (true)
            {
            c = cp[n];

            if (sign)
               {
               u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16("0123456789abcdef"[(c >> 4) & 0x0F],
                                                             "0123456789abcdef"[(c     ) & 0x0F]));

               bp        += 2;
               remaining -= 2;
               }
            else
               {
               if (c == '\0') break;

               i = u_sprintc(bp, c);

               bp        += i;
               remaining -= i;
               }

            if (++n == prec ||
                remaining <= 60)
               {
               if (sign ||
                   len == U_NOT_FOUND) /* NB: no precision specified... */
                  {
                  /* NB: print something that have the meaning of 'to be continued'... */

                  u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16('.','.'));

                   bp  += 2;
                  *bp++ = '.';
                  }

               break;
               }
            }

         *bp++ = '"';

#     if defined(DEBUG) && defined(U_STDCPP_ENABLE)
         if (ch == 'O') free(cp);
#     endif

         continue;
         }

case_ustring_v:
      U_INTERNAL_ASSERT_EQUALS(sign, 0)

      size = (prec < 0 ? (int)u__strlen((const char*)cp, __PRETTY_FUNCTION__) : prec);

      U_INTERNAL_ERROR(size <= (int)(buffer_size - (bp-buffer)),
                       "WE ARE GOING TO OVERFLOW BUFFER at u__vsnprintf() size = %u remaining = %ld cp = %.20s buffer_size = %u format = \"%.*s\"",
                       size, (buffer_size - (bp-buffer)), cp, buffer_size, format_size_save, format);

      /* if a width from format is specified, the 0 flag for padding will be ignored... */

      if (width >= 0) flags &= ~ZEROPAD;

      goto next;

case_B: /* extension: print bit conversion of int */
#  ifdef DEBUG
      i  = sizeof(int);
      n  = VA_ARG(int);
      cp = (unsigned char* restrict)&n;

#  if __BYTE_ORDER != __LITTLE_ENDIAN
      cp += sizeof(int);
#  endif

      *bp++ = '<';

      while (true)
         {
#     if __BYTE_ORDER == __LITTLE_ENDIAN
         c = *cp++;
#     else
         c = *--cp;
#     endif

         if (c)
            {
            int j;

#        if __BYTE_ORDER == __LITTLE_ENDIAN
            for (j = 0; j <= 7; ++j)
#        else
            for (j = 7; j >= 0; --j)
#        endif
               {
               *bp++ = '0' + u_test_bit(j,c);
               }
            }
         else
            {
            u_put_unalignedp64(bp, U_MULTICHAR_CONSTANT64('0','0','0','0','0','0','0','0'));

            bp += 8;
            }

         if (--i == 0) break;

         *bp++ = ' ';
         }

      *bp++ = '>';
#  endif

      continue;

case_C: /* extension: print formatted char */
      c = VA_ARG(int);

      *bp++ = '\'';

      if (u__isquote(c) == false) bp += u_sprintc(bp, c);
      else
         {
         if (c == '"') *bp++ = '"';
         else
            {
            u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16('\\','\''));

            bp += 2;
            }
         }

      *bp++ = '\'';

      continue;

case_D: /* extension: print date and time in various format */
      if ((flags & ALT) != 0) t = VA_ARG(time_t); /* flag '#' => var-argument */
      else
         {
         U_gettimeofday /* NB: optimization if it is enough a time resolution of one second... */

                         t  = u_now->tv_sec;
         if (width != 8) t += u_now_adjust;
         }

      /**
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
       */

      bp +=
          (width ==  0 ? u_strftime2(bp, 36, U_CONSTANT_TO_PARAM("%d/%m/%y"), t)            :
           width <=  2 ? u_strftime2(bp, 36, U_CONSTANT_TO_PARAM("%T"), t)                  :
           width ==  3 ? u_strftime2(bp, 36, U_CONSTANT_TO_PARAM("%d/%m/%Y %T"), t)         :
           width ==  4 ? u_strftime2(bp, 36, U_CONSTANT_TO_PARAM("%d%m%y_%H%M%S"), t)       :
           width ==  5 ? u_strftime2(bp, 36, U_CONSTANT_TO_PARAM("%a, %d %b %Y %T %Z"), t)  :
           width ==  6 ? u_strftime2(bp, 36, U_CONSTANT_TO_PARAM("%Y/%m/%d"), t)            :
           width ==  7 ? u_strftime2(bp, 36, U_CONSTANT_TO_PARAM("%Y/%m/%d %T"), t)         :
           width ==  8 ? u_strftime2(bp, 36, U_CONSTANT_TO_PARAM("%a, %d %b %Y %T GMT"), t) :
           width ==  9 ? u_strftime2(bp, 36, U_CONSTANT_TO_PARAM("%d/%m/%y %T"), t)         :
                         u_strftime2(bp, 36, U_CONSTANT_TO_PARAM("%d/%b/%Y:%T %z"), t));

      if (width == 2) /* check for days */
         {
         if ((flags & ALT) != 0 &&
             t > U_ONE_DAY_IN_SECOND)
            {
            u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16(' ','+'));

            bp = u_num2str32(t / U_ONE_DAY_IN_SECOND, bp+2);

            u_put_unalignedp32(bp, U_MULTICHAR_CONSTANT32(' ','d','a','y'));

            bp[4] = 's';

            bp += 5;
            }
         }
      else if (width == 4) /* _millisec */
         {
         static long old_ms;
                long     ms = u_now->tv_usec / 1000L;

         if (old_ms >= ms) ms = (old_ms >= 999 ? 0 : old_ms+1);
             old_ms  = ms;

         bp += sprintf(bp, "_%03ld", ms);
         }

      continue;

case_H: /* extension: print host name */
      U_INTERNAL_ERROR(u_hostname_len, "HOSTNAME NULL at u__vsnprintf() - CHECK THE PARAMETERS - format = \"%.*s\"", format_size_save, format);

      u__memcpy(bp, u_hostname, u_hostname_len, __PRETTY_FUNCTION__);

      bp += u_hostname_len;

      continue;

case_I: /* extension: print off_t */
#  if SIZEOF_OFF_T == 8 && defined(ENABLE_LFS)
      flags |= LLONGINT;
#  endif

      goto case_d;

case_J: /* extension: print U_DATA */
      udata = VA_ARG(struct U_DATA);

      cp   = udata.dptr;
      prec = udata.dsize;

      goto case_ustring_V;

case_L: /* field length modifier */
      flags |= LONGDBL;

      goto rflag;

case_M: /* extension: print memory dump */
#  ifdef DEBUG
      cp = VA_ARG(unsigned char* restrict);
      n  = VA_ARG(int);

      len = u_memory_dump(bp, cp, n);

      bp += len;
#  endif

      continue;

case_N: /* extension: print program name */
      u__memcpy(bp, u_progname, u_progname_len, __PRETTY_FUNCTION__);

      bp += u_progname_len;

      continue;

case_P: /* extension: print process pid */
      u__memcpy(bp, u_pid_str, u_pid_str_len, __PRETTY_FUNCTION__);

      bp += u_pid_str_len;

      continue;

case_Q: /* extension: call exit() or abort() (var-argument is the arg passed to exit) */
      u_flag_exit = VA_ARG(int);

      continue;

case_R: /* extension: print msg - u_getSysError() */
      cp = VA_ARG(unsigned char* restrict);

      U_INTERNAL_PRINT("cp = %s", cp)

      if (cp)
         {
         len = u__strlen((const char* restrict)cp, __PRETTY_FUNCTION__);

         u__memcpy(bp, cp, len, __PRETTY_FUNCTION__);

         bp += len;
         }

      if ((flags & ALT) == 0)
         {
         u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16(' ','-'));

          bp  += 2;
         *bp++ = ' ';
         }

      if (errno == 0) errno = u_errno;

#  ifdef _MSWINDOWS_
      if (errno < 0)
         {
         errno = - errno;

         cp = getSysError_w32((uint32_t*)&len);

         u__memcpy(bp, cp, len, __PRETTY_FUNCTION__);

         bp += len;

         u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16(' ','-'));

          bp  += 2;
         *bp++ = ' ';

         MAP_WIN32_ERROR_TO_POSIX
         }
#  endif

      u_getSysError((uint32_t*)&len);

      u__memcpy(bp, u_err_buffer, len, __PRETTY_FUNCTION__);

      bp += len;

      continue;

case_T: /* extension: print time_t */
#  if SIZEOF_TIME_T == 8
      flags |= LLONGINT;
#  endif

      goto case_d;

case_U: /* extension: print user name */
      U_INTERNAL_ERROR(u_user_name_len, "USER NAME NULL at u__vsnprintf() - CHECK THE PARAMETERS - format = \"%.*s\"", format_size_save, format);

      u__memcpy(bp, u_user_name, u_user_name_len, __PRETTY_FUNCTION__);

      bp += u_user_name_len;

      continue;

case_V: /* extension: print ustring */
      pstr = VA_ARG(struct ustringrep*);

      U_INTERNAL_ASSERT_POINTER(pstr)

      cp   = (unsigned char* restrict) pstr->str;
      prec =                           pstr->_length;

      goto case_ustring_V;

case_W: /* extension: print COLOR (ANSI ESCAPE STR) */
      n = VA_ARG(int);

#  if !defined(_MSWINDOWS_) && !defined(__UNIKERNEL__)
      if (u_is_tty)
         {
         U_INTERNAL_ERROR(n <= BRIGHTWHITE, "INVALID COLOR(%d) at u__vsnprintf() - CHECK THE PARAMETERS - format = \"%.*s\"", n, format_size_save, format);

         len = sizeof(U_RESET_STR) - (n == RESET);

         u__memcpy(bp, tab_color[n], len, __PRETTY_FUNCTION__);

         bp += len;
         }
#  endif

      continue;

case_X:
      base     = HEX;
      argument = UARG();

      /* leading 0x/X only if non-zero */

      if (argument != 0 &&
          (flags & ALT) != 0)
         {
         flags |= HEXPREFIX;
         }

nosign:
      sign = 0;

number: /* uint32_t conversions */
      if ((dprec = prec) >= 0) flags &= ~ZEROPAD; /* ... diouXx conversions ... if a precision is specified, the 0 flag will be ignored */

      if (prec     == 0 &&
          argument == 0)
         {
         size = 0; /* The result of converting a zero value with an explicit precision of zero is no characters */
         }
      else
         {
         if (base == OCT)
            {
            cp = buf + sizeof(buf);

            do { *--cp = (argument & 7) + '0'; } while (argument >>= 3);

            /* handle octal leading 0 */

            if (*cp != '0' &&
                (flags & ALT) != 0)
               {
               *--cp = '0';
               }

            size = (ptrdiff_t)(buf + sizeof(buf) - cp);
            }
         else if (base == HEX)
            {
            const unsigned char* restrict xdigs = (const unsigned char* restrict)(ch == 'X' ? "0123456789ABCDEF"
                                                                                            : "0123456789abcdef"); /* digits for [xX] conversion */

            cp = buf + sizeof(buf);

            do { *--cp = xdigs[argument & 15]; } while (argument /= 16);

            size = (ptrdiff_t)(buf + sizeof(buf) - cp);
            }
         else
            {
            U_INTERNAL_ASSERT_EQUALS(base, DEC)

            U_INTERNAL_PRINT("dprec = %d flags = %d", dprec, flags)

            if (LIKELY((flags & THOUSANDS_GROUPED) == 0))
               {
               if (width     ||
                   dprec > 0 ||
                   (flags & (LADJUST | ZEROPAD)) != 0)
                  {
                  size = u_num2str64(argument, (char* restrict)(cp = buf)) - (char* restrict)buf;

                  goto next;
                  }

               if (sign) *bp++ = sign;

               bp = u_num2str64(argument, bp);

               continue;
               }

            n = 1;

            cp = buf + sizeof(buf);

            while (argument >= 10) /* NB: many numbers are 1 digit */
               {
               *--cp = (unsigned char)(argument % 10) + '0';

               argument /= 10;

               if ((n++ % 3) == 0) *--cp = ',';
               }

            *--cp = argument + '0';

            size = (ptrdiff_t)(buf + sizeof(buf) - cp);
            }
         }

      goto next;

case_Y: /* extension: print u_getSysSignal(signo) */
      u_getSysSignal(VA_ARG(int), (uint32_t*)&len);

      u__memcpy(bp, u_err_buffer, len, __PRETTY_FUNCTION__);

      bp += len;

      continue;

case_b: /* extension: print bool */
      n = VA_ARG(int);

      if (n) u_put_unalignedp32(bp, U_MULTICHAR_CONSTANT32('t','r','u','e'));
      else
         {
         *bp++ = 'f';

         u_put_unalignedp32(bp, U_MULTICHAR_CONSTANT32('a','l','s','e'));
         }

      bp += 4;

      continue;

case_c: /* field conversion specifier */
      *(cp = buf) = VA_ARG(int);

      U_INTERNAL_ASSERT_EQUALS(sign, 0)

      size = 1;

      goto next;

case_d:
      argument = SARG();

      if ((int64_t)argument < 0)
         {
         sign     = '-';
         argument = -argument;
         }

      base = DEC;

      goto number;

case_h: /* field length modifier: h hh */
      flags |= SHORTINT;

      goto rflag;

case_j: /* field length modifier - A following integer conversion corresponds to an intmax_t or uintmax_t argument */
/*case_z:  field length modifier - A following integer conversion corresponds to a    size_t or   ssize_t argument */
/*case_t:  field length modifier - A following integer conversion corresponds to a ptrdiff_t              argument */
      goto rflag;

case_l: /* field length modifier: l ll */
      flags |= (flags & LONGINT ? LLONGINT : LONGINT);

      goto rflag;

/**
 * "%n" format specifier, which writes the number of characters written to an address that is passed
 *      as an argument on the stack. It is the format specifier of choice for those performing format string attacks
 *
 * case_n:
 *    if (flags & LLONGINT)
 *       {
 *       long long* p = VA_ARG(long long*);
 *
 *       U_INTERNAL_ERROR(p, "NULL pointer at u__vsnprintf() - CHECK THE PARAMETERS - format = \"%.*s\"", format_size_save, format);
 * 
 *       *p = (bp-buffer);
 *       }
 *    else if (flags &  LONGINT)
 *       {
 *       long* p = VA_ARG(long*);
 *
 *       U_INTERNAL_ERROR(p, "NULL pointer at u__vsnprintf() - CHECK THE PARAMETERS - format = \"%.*s\"", format_size_save, format);
 *
 *       *p = (bp-buffer);
 *       }
 *    else if (flags & SHORTINT)
 *       {
 *       short* p = VA_ARG(short*);
 *
 *       U_INTERNAL_ERROR(p, "NULL pointer at u__vsnprintf() - CHECK THE PARAMETERS - format = \"%.*s\"", format_size_save, format);
 *
 *       *p = (bp-buffer);
 *       }
 *    else
 *       {
 *       int* p = VA_ARG(int*);
 *
 *       U_INTERNAL_ERROR(p, "NULL pointer at u__vsnprintf() - CHECK THE PARAMETERS - format = \"%.*s\"", format_size_save, format);
 *
 *       *p = (bp-buffer);
 *       }
 * 
 *    continue; // no output
 */

case_o:
      base     = OCT;
      argument = UARG();

      goto nosign;

case_p: /* The argument shall be a pointer to void. The value of the pointer is converted to a sequence of printable characters, in an implementation-defined manner */
#  if defined(HAVE_ARCH64) && defined(U_LINUX)
      argument = (long) VA_ARG(const char* restrict);
#  else
      argument = (long) VA_ARG(const char* restrict) & 0x00000000ffffffffLL;
#  endif

      if (argument == 0)
         {
         *bp++ = '(';

         u_put_unalignedp32(bp, U_MULTICHAR_CONSTANT32('n','i','l',')'));

         bp += 4;

         continue;
         }

      ch     = 'x';
      base   = HEX;
      flags |= HEXPREFIX;

      goto nosign;

case_q: /* field length modifier: quad. This is a synonym for ll */
      flags |= LLONGINT;

      goto rflag;

case_r: /* extension: print u_getExitStatus(exit_value) */
      u_getExitStatus(VA_ARG(int), (uint32_t*)&len);

      u__memcpy(bp, u_err_buffer, len, __PRETTY_FUNCTION__);

      bp += len;

      continue;

case_u:
      base     = DEC;
      argument = UARG();

      goto nosign;

case_v: /* extension: print ustring */
      pstr = VA_ARG(struct ustringrep*);

      U_INTERNAL_ASSERT_POINTER(pstr)

      cp   = (unsigned char* restrict) pstr->str;
      prec =                           pstr->_length;

      goto case_ustring_v;

case_w: /* extension: print current working directory */
      U_INTERNAL_ERROR(u_cwd_len, "CURRENT WORKING DIRECTORY NULL at u__vsnprintf() - CHECK THE PARAMETERS - format = \"%.*s\"", format_size_save, format);

      u__memcpy(bp, u_cwd, u_cwd_len, __PRETTY_FUNCTION__);

      bp += u_cwd_len;

      continue;

      /**
       * char sign   - sign prefix (' ', '+', '-', or \0)
       * int size    - size of converted field or string
       * int width   - width from format (%8d), or 0
       * int fieldsz - field size expanded by sign, dpad etc
       * int pads    - extra padding size
       * int dpad    - extra 0 padding needed for integers
       * int bpad    - extra blank padding needed
       * int prec    - precision from format (%.3d), or -1
       * int dprec   - a copy of prec if [diouxX], 0 otherwise
       * ---------------------------------------------------------------------------------
       * All reasonable formats wind up here. At this point, `cp' points to
       * a string which (if not flags & LADJUST) should be padded out to
       * 'width' places. If flags & ZEROPAD, it should first be prefixed by any
       * sign or other prefix (%010d = "-000123456"); otherwise, it should be
       * blank padded before the prefix is emitted (%10d = "   -123456"). After
       * any left-hand padding and prefixing, emit zeroes required by a decimal
       * [diouxX] precision, then print the string proper, then emit zeroes
       * required by any leftover floating precision; finally, if LADJUST, pad with blanks
       * ---------------------------------------------------------------------------------
       */
next:
      U_INTERNAL_PRINT("size = %d width = %d prec = %d dprec = %d sign = %c", size, width, prec, dprec, sign)

      dpad = dprec - size; /* compute actual size, so we know how much to pad */

      if (dpad < 0) dpad = 0;

      fieldsz = size + dpad;

      pads = width - fieldsz;

      if (pads < 0) pads = 0;

      U_INTERNAL_PRINT("fieldsz = %d pads = %d dpad = %d", fieldsz, pads, dpad)

      /* check ret */

      ret = (bp-buffer) + (width > fieldsz ? width : fieldsz);

      U_INTERNAL_ERROR(ret <= buffer_size,
                       "BUFFER OVERFLOW at u__vsnprintf() ret = %u buffer_size = %u format = \"%.*s\"", ret, buffer_size, format_size_save, format);

      /* right-adjusting blank padding */

      bpad = 0;

      if (pads &&
          (flags & (LADJUST | ZEROPAD)) == 0)
         {
         for (bpad = pads; pads; --pads) *bp++ = ' ';
         }

      /* prefix */

      if (sign)
         {
              if (bpad) --bp;
         else if (pads) --pads;

         *bp++ = sign;
         }
      else if ((flags & HEXPREFIX) != 0)
         {
              if (bpad)   bp -= 2;
         else if (pads) pads -= 2;

         u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16('0',ch));

         bp += 2;
         }

      /* right-adjusting zero padding */

      if ((flags & (LADJUST | ZEROPAD)) == ZEROPAD)
         {
         for (; pads; --pads) *bp++ = '0';
         }

      /* leading zeroes from decimal precision */

      for (; dpad; --dpad) *bp++ = '0';

      /* the string or number proper */

      if (size)
         {
         U_INTERNAL_ASSERT_POINTER(cp)

         u__memcpy(bp, cp, size, __PRETTY_FUNCTION__);

         bp += size;
         }

      /* left-adjusting padding (always blank) */

      if ((flags & LADJUST) != 0)
         {
         for (; pads; --pads) *bp++ = ' ';
         }
      }
   while (fmt_size);

   ret = (bp-buffer);

   U_INTERNAL_PRINT("ret = %u buffer_size = %u", ret, buffer_size)

   if (ret < buffer_size) *bp = '\0';

   U_INTERNAL_ERROR(ret <= buffer_size, "BUFFER OVERFLOW at u__vsnprintf() ret = %u buffer_size = %u format = \"%.*s\"", ret, buffer_size, format_size_save, format);

   return ret;
}

uint32_t u__snprintf(char* restrict buffer, uint32_t buffer_size, const char* restrict format, uint32_t fmt_size, ...)
{
   uint32_t bytes_written;

   va_list argp;
   va_start(argp, fmt_size);

   U_INTERNAL_TRACE("u__snprintf(%p,%u,%.*s,%u)", buffer, buffer_size, fmt_size, format, fmt_size)

   bytes_written = u__vsnprintf(buffer, buffer_size, format, fmt_size, argp);

   va_end(argp);

   return bytes_written;
}

void u__printf(int fd, const char* format, uint32_t fmt_size, ...)
{
   char buffer[8196];
   uint32_t bytes_written;

   va_list argp;
   va_start(argp, fmt_size);

   U_INTERNAL_TRACE("u__printf(%d,%.*s,%u)", fd, fmt_size, format, fmt_size)

   bytes_written = u__vsnprintf(buffer, sizeof(buffer)-1, format, fmt_size, argp);

   va_end(argp);

   buffer[bytes_written++] = '\n';

#ifdef DEBUG
   if (u_flag_exit < 0) u_printError();
#endif

   (void) write(fd, buffer, bytes_written);

   if (u_flag_exit)
      {
#  ifdef DEBUG
      if (u_trace_fd > STDERR_FILENO) /* NB: write error on trace file, check stderr to avoid duplication message on terminal */
         {
         /* check if warning due to syscall */

         if (u_flag_exit != 2 || errno == 0)
            {
            struct iovec iov[1] = { { (caddr_t)buffer, bytes_written } };

            u_trace_writev(iov, 1);
            }
         }
#  endif

      /* check if warning */

      if (u_flag_exit == 2)
         {
         u_flag_exit = 0;

         return;
         }

#  ifdef DEBUG
      if (u_flag_exit < 0)
         {
         if (u_flag_test > 0) /* check if to force continue - test */
            {
            --u_flag_test;

            u_flag_exit = 0;

            return;
            }

         if (u_askForContinue()) return;
         }

      u_debug_at_exit();
#  endif

      exit(u_flag_exit);
      }
}

/* AT EXIT */

vPF u_fns[32];
int u_fns_index;

void u_atexit(vPF function)
{
   int i;

   U_INTERNAL_TRACE("u_atexit(%p)", function)

   U_INTERNAL_ASSERT_POINTER(function)

   for (i = u_fns_index - 1; i >= 0; --i)
      {
      if (u_fns[i] == function) return;
      }

   u_fns[u_fns_index++] = function;
}

void u_unatexit(vPF function)
{
   int i;

   U_INTERNAL_TRACE("u_unatexit(%p)", function)

   U_INTERNAL_ASSERT_POINTER(function)

   for (i = u_fns_index - 1; i >= 0; --i)
      {
      if (u_fns[i] == function)
         {
         u_fns[i] = 0;

         break;
         }
      }
}

void u_exit(void)
{
   int i;

   U_INTERNAL_TRACE("u_exit()")

   U_INTERNAL_PRINT("u_fns_index = %d", u_fns_index)

   for (i = u_fns_index - 1; i >= 0; --i)
      {
      if (u_fns[i])
         {
         U_INTERNAL_PRINT("u_fns[%d] = %p", i, u_fns[i])

         u_fns[i]();
         }
      }
}

#define U_HTTP_ENTRY(n,x) U_http_method_list[n].name =                 #x, \
                          U_http_method_list[n].len  = U_CONSTANT_SIZE(#x)

void u_init_http_method_list(void)
{
   U_INTERNAL_TRACE("u_init_http_method_list()")

   if (U_http_method_list[0].len == 0)
      {
      /* request methods */
      U_HTTP_ENTRY(0,GET);
      U_HTTP_ENTRY(1,HEAD);
      U_HTTP_ENTRY(2,POST);
      U_HTTP_ENTRY(3,PUT);
      U_HTTP_ENTRY(4,DELETE);
      U_HTTP_ENTRY(5,OPTIONS);
      /* pathological */
      U_HTTP_ENTRY(6,TRACE);
      U_HTTP_ENTRY(7,CONNECT);
      /* webdav */
      U_HTTP_ENTRY(8,COPY);
      U_HTTP_ENTRY(9,MOVE);
      U_HTTP_ENTRY(10,LOCK);
      U_HTTP_ENTRY(11,UNLOCK);
      U_HTTP_ENTRY(12,MKCOL);
      U_HTTP_ENTRY(13,SEARCH);
      U_HTTP_ENTRY(14,PROPFIND);
      U_HTTP_ENTRY(15,PROPPATCH);
      /* rfc-5789 */
      U_HTTP_ENTRY(16,PATCH);
      U_HTTP_ENTRY(17,PURGE);
      /* subversion */
      U_HTTP_ENTRY(18,MERGE);
      U_HTTP_ENTRY(19,REPORT);
      U_HTTP_ENTRY(20,CHECKOUT);
      U_HTTP_ENTRY(21,MKACTIVITY);
      /* upnp */
      U_HTTP_ENTRY(22,NOTIFY);
      U_HTTP_ENTRY(23,MSEARCH);
      U_HTTP_ENTRY(24,SUBSCRIBE);
      U_HTTP_ENTRY(25,UNSUBSCRIBE);
      }
}

#undef U_HTTP_ENTRY

/*
#if defined(U_ALL_C) && !defined(DEBUG)
# undef  U_INTERNAL_TRACE
# define U_INTERNAL_TRACE(format,args...)
# undef  U_INTERNAL_PRINT
# define U_INTERNAL_PRINT(format,args...)
#endif
*/
