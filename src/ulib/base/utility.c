/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    utility.c
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/utility.h>

#include <sched.h>

#ifdef HAVE_SYSEXITS_H
#  include <sysexits.h>
#else
#  include <ulib/base/replace/sysexits.h>
#endif

#ifdef HAVE_FNMATCH
#  include <fnmatch.h>
#endif

#ifdef ENABLE_THREAD
#  if defined(__NetBSD__) || defined(__UNIKERNEL__)
#     include <lwp.h>
#  endif
#  ifdef HAVE_SYS_SYSCALL_H
#     include <sys/syscall.h>
#  endif
#endif

#ifndef _MSWINDOWS_
#  include <pwd.h>
#  if defined(U_LINUX) && defined(HAVE_LIBCAP)
#     include <sys/prctl.h>
#     include <sys/capability.h>
#     ifdef SECBIT_KEEP_CAPS
#        define U_PR_SET_KEEPCAPS SECBIT_KEEP_CAPS
#     else
#        define U_PR_SET_KEEPCAPS PR_SET_KEEPCAPS
#     endif
#  elif defined(__APPLE__)
#     include <mach/mach.h>
#     include <mach/mach_port.h>
#     include <mach/mach_traps.h>
#  endif
#endif

__pure unsigned long u_hex2int(const char* restrict s, const char* restrict e)
{
   /* handle up to 16 digits */

   uint32_t len = e-s;
   unsigned long val = 0UL;

   U_INTERNAL_TRACE("u_hex2int(%p,%p)", s, e)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_POINTER(e)

#ifndef U_COVERITY_FALSE_POSITIVE /* Control flow issues (MISSING_BREAK) */
   switch (len)
      {
      case 16: val = (val << 4) | u__hexc2int(s[len-16]);
      case 15: val = (val << 4) | u__hexc2int(s[len-15]);
      case 14: val = (val << 4) | u__hexc2int(s[len-14]);
      case 13: val = (val << 4) | u__hexc2int(s[len-13]);
      case 12: val = (val << 4) | u__hexc2int(s[len-12]);
      case 11: val = (val << 4) | u__hexc2int(s[len-11]);
      case 10: val = (val << 4) | u__hexc2int(s[len-10]);
      case  9: val = (val << 4) | u__hexc2int(s[len- 9]);
      case  8: val = (val << 4) | u__hexc2int(s[len- 8]);
      case  7: val = (val << 4) | u__hexc2int(s[len- 7]);
      case  6: val = (val << 4) | u__hexc2int(s[len- 6]);
      case  5: val = (val << 4) | u__hexc2int(s[len- 5]);
      case  4: val = (val << 4) | u__hexc2int(s[len- 4]);
      case  3: val = (val << 4) | u__hexc2int(s[len- 3]);
      case  2: val = (val << 4) | u__hexc2int(s[len- 2]);
      case  1: val = (val << 4) | u__hexc2int(s[len- 1]);
      }
#endif

   U_INTERNAL_PRINT("val = %lu", val)

   return val;
}

__pure unsigned long u_strtoul(const char* restrict s, const char* restrict e)
{
   /* handle up to 10 digits */

   uint32_t len = e-s;
   unsigned long val = 0UL;

   U_INTERNAL_TRACE("u_strtoul(%p,%p)", s, e)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_POINTER(e)

#ifndef U_COVERITY_FALSE_POSITIVE /* Control flow issues (MISSING_BREAK) */
   switch (len)
      {
      case 10: val += (s[len-10] - '0') * 1000000000UL;
      case  9: val += (s[len- 9] - '0') * 100000000UL;
      case  8: val += (s[len- 8] - '0') * 10000000UL;
      case  7: val += (s[len- 7] - '0') * 1000000UL;
      case  6: val += (s[len- 6] - '0') * 100000UL;
      case  5: val += (s[len- 5] - '0') * 10000UL;
      case  4: val += (s[len- 4] - '0') * 1000UL;
      case  3: val += (s[len- 3] - '0') * 100UL;
      case  2: val += (s[len- 2] - '0') * 10UL;
      case  1: val += (s[len- 1] - '0');
      }
#endif

   U_INTERNAL_PRINT("val = %lu", val)

   return val;
}

__pure uint64_t u_strtoull(const char* restrict s, const char* restrict e)
{
   uint32_t len = e-s;
   uint64_t val = 0UL;

   U_INTERNAL_TRACE("u_strtoul(%p,%p)", s, e)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_POINTER(e)

#ifndef U_COVERITY_FALSE_POSITIVE /* Control flow issues (MISSING_BREAK) */
   switch (len)
      {
      case 20: val += (s[len-20] - '0') * 10000000000000000000ULL;
      case 19: val += (s[len-19] - '0') * 1000000000000000000ULL;
      case 18: val += (s[len-18] - '0') * 100000000000000000ULL;
      case 17: val += (s[len-17] - '0') * 10000000000000000ULL;
      case 16: val += (s[len-16] - '0') * 1000000000000000ULL;
      case 15: val += (s[len-15] - '0') * 100000000000000ULL;
      case 14: val += (s[len-14] - '0') * 10000000000000ULL;
      case 13: val += (s[len-13] - '0') * 1000000000000ULL;
      case 12: val += (s[len-12] - '0') * 100000000000ULL;
      case 11: val += (s[len-11] - '0') * 10000000000ULL;
      case 10: val += (s[len-10] - '0') * 1000000000ULL;
      case  9: val += (s[len- 9] - '0') * 100000000ULL;
      case  8: val += (s[len- 8] - '0') * 10000000ULL;
      case  7: val += (s[len- 7] - '0') * 1000000ULL;
      case  6: val += (s[len- 6] - '0') * 100000ULL;
      case  5: val += (s[len- 5] - '0') * 10000ULL;
      case  4: val += (s[len- 4] - '0') * 1000ULL;
      case  3: val += (s[len- 3] - '0') * 100ULL;
      case  2: val += (s[len- 2] - '0') * 10ULL;
      case  1: val += (s[len- 1] - '0');
      }
#endif

   U_INTERNAL_PRINT("val = %llu", val)

   return val;
}

__pure long u_strtol(const char* restrict s, const char* restrict e)
{
   int sign = 1;

   U_INTERNAL_TRACE("u_strtol(%p,%p)", s, e)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_POINTER(e)

// while (u__isspace(*s)) ++s;

   if (*s == '-')
      {
      ++s;

      sign = -1;
      }
   else
      {
      if (*s == '+' ||
          *s == '0')
         {
         ++s;
         }
      }

   return (sign * u_strtoul(s, e));
}

__pure int64_t u_strtoll(const char* restrict s, const char* restrict e)
{
   int sign = 1;

   U_INTERNAL_TRACE("u_strtoll(%p,%p)", s, e)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_POINTER(e)

// while (u__isspace(*s)) ++s;

   if (*s == '-')
      {
      ++s;

      sign = -1;
      }
   else
      {
      if (*s == '+' ||
          *s == '0')
         {
         ++s;
         }
      }

   return (sign * u_strtoull(s, e));
}

/* To avoid libc locale overhead */

__pure int u__strncasecmp(const char* restrict s1, const char* restrict s2, size_t n)
{
   U_INTERNAL_TRACE("u__strncasecmp(%p,%p,%lu)", s1, s2, n)

   U_INTERNAL_ASSERT_MAJOR(n, 0)
   U_INTERNAL_ASSERT_POINTER(s1)
   U_INTERNAL_ASSERT_POINTER(s2)

   while (n)
      {
      char c1 = u__tolower(*s1),
           c2 = u__tolower(*s2);

      if (c1 == c2)
         {
         if (c1)
            {
            --n;
            ++s1;
            ++s2;

            continue;
            }

         return 0;
         }

      return (c1 - c2);
      }

   return 0;
}

uint32_t u_gettid(void)
{
#ifndef ENABLE_THREAD
   return U_NOT_FOUND;
#else
   uint32_t tid =
# ifdef _MSWINDOWS_
   GetCurrentThreadId();
# elif defined(HAVE_PTHREAD_GETTHREADID_NP)
   pthread_getthreadid_np();
# elif defined(U_LINUX)
   syscall(SYS_gettid);
# elif defined(__sun)
   pthread_self();
# elif defined(__APPLE__)
   mach_thread_self();
   mach_port_deallocate(mach_task_self(), tid);
# elif defined(__NetBSD__) || defined(__UNIKERNEL__)
   _lwp_self();
# elif defined(__FreeBSD__)
   thr_self(&tid);
# elif defined(__DragonFly__)
   lwp_gettid();
# endif
   return tid;
#endif
}

/* Security functions */

#ifndef _MSWINDOWS_
static uid_t real_uid      = (uid_t)-1;
static gid_t real_gid      = (gid_t)-1;
static uid_t effective_uid = (uid_t)-1;
static gid_t effective_gid = (gid_t)-1;
#endif

void u_init_security(void)
{
   /**
    * Run this at the beginning of the program to initialize this code and
    * to drop privileges before someone uses them to shoot us in the foot
    */
#ifndef _MSWINDOWS_
   int leffective_uid;

   U_INTERNAL_TRACE("u_init_security()")

   alarm(0); /* can be inherited from parent process */

         real_uid = getuid();
   leffective_uid = geteuid();

   /* sanity check */

   if (leffective_uid != (int)real_uid &&
       leffective_uid != 0)
      {
      U_WARNING("Setuid but not to root (uid=%ld, euid=%d), dropping setuid privileges now", (long) real_uid, leffective_uid);

      u_never_need_root();
      }
   else
      {
      effective_uid = leffective_uid;
      }

   real_gid      = getgid();
   effective_gid = getegid();

   u_dont_need_root();
   u_dont_need_group();
#endif
}

/* Temporarily gain root privileges */

void u_need_root(bool necessary)
{
   U_INTERNAL_TRACE("u_need_root(%d)", necessary)

#ifndef _MSWINDOWS_
   U_INTERNAL_PRINT("(_euid_=%d, uid=%d), current=%d", effective_uid, real_uid, geteuid())

   if (effective_uid)
      {
      if (necessary) U_ERROR("Require root privilege but not setuid root");
                     U_DEBUG("Require root privilege but not setuid root");

      return;
      }

   if (real_uid == (uid_t)(-1)) U_ERROR("u_init_security() not called");

   if (geteuid() == 0) return; /* nothing to do */

   if (seteuid(effective_uid) == -1 ||
       geteuid()              !=  0)
      {
      if (necessary) U_ERROR("Did not get root privilege");
                     U_DEBUG("Did not get root privilege");
      }
#endif
}

/* Temporarily drop root privileges */

void u_dont_need_root(void)
{
   U_INTERNAL_TRACE("u_dont_need_root()")

#ifndef _MSWINDOWS_
   U_INTERNAL_PRINT("(_euid_=%d, uid=%d), current=%d", effective_uid, real_uid, geteuid())

   if (effective_uid) return;

   if (real_uid == (uid_t)(-1)) U_ERROR("u_init_security() not called");

   if (geteuid() != 0) return; /* nothing to do */

   if (seteuid(real_uid) == -1 ||
       geteuid() != real_uid)
      {
      U_ERROR("Did not drop root privilege");
      }
#endif
}

/* Permanently drop root privileges */

void u_never_need_root(void)
{
#ifndef _MSWINDOWS_
#  if defined(U_LINUX) && defined(HAVE_LIBCAP)
/*
cap_list[] = {
   {"chown",               CAP_CHOWN},
   {"dac_override",        CAP_DAC_OVERRIDE},
   {"dac_read_search",     CAP_DAC_READ_SEARCH},
   {"fowner",              CAP_FOWNER},
   {"fsetid",              CAP_FSETID},
   {"kill",                CAP_KILL},
   {"setgid",              CAP_SETGID},
   {"setuid",              CAP_SETUID},
   {"setpcap",             CAP_SETPCAP},
   {"linux_immutable",     CAP_LINUX_IMMUTABLE},
   {"net_bind_service",    CAP_NET_BIND_SERVICE},
   {"net_broadcast",       CAP_NET_BROADCAST},
   {"net_admin",           CAP_NET_ADMIN},
   {"net_raw",             CAP_NET_RAW},
   {"ipc_lock",            CAP_IPC_LOCK},
   {"ipc_owner",           CAP_IPC_OWNER},
   {"sys_module",          CAP_SYS_MODULE},
   {"sys_rawio",           CAP_SYS_RAWIO},
   {"sys_chroot",          CAP_SYS_CHROOT},
   {"sys_ptrace",          CAP_SYS_PTRACE},
   {"sys_pacct",           CAP_SYS_PACCT},
   {"sys_admin",           CAP_SYS_ADMIN},
   {"sys_boot",            CAP_SYS_BOOT},
   {"sys_nice",            CAP_SYS_NICE},
   {"sys_resource",        CAP_SYS_RESOURCE},
   {"sys_time",            CAP_SYS_TIME},
   {"sys_tty_config",      CAP_SYS_TTY_CONFIG},
   {"mknod",               CAP_MKNOD},
#ifdef CAP_LEASE
   {"lease",               CAP_LEASE},
#endif
#ifdef CAP_AUDIT_WRITE
   {"audit_write",         CAP_AUDIT_WRITE},
#endif
#ifdef CAP_AUDIT_CONTROL
   {"audit_control",       CAP_AUDIT_CONTROL},
#endif
#ifdef CAP_SETFCAP
   {"setfcap",             CAP_SETFCAP},
#endif
#ifdef CAP_MAC_OVERRIDE
   {"mac_override",        CAP_MAC_OVERRIDE},
#endif
#ifdef CAP_MAC_ADMIN
   {"mac_admin",           CAP_MAC_ADMIN},
#endif
#ifdef CAP_SYSLOG
   {"syslog",              CAP_SYSLOG},
#endif
#ifdef CAP_WAKE_ALARM
   {"wake_alarm",          CAP_WAKE_ALARM},
#endif
   {0, -1}
   };
*/

# ifdef DEBUG
   cap_value_t minimal_cap_values[] = { CAP_SETUID, CAP_SETGID, CAP_SETPCAP, CAP_SYS_PTRACE };
# else
   cap_value_t minimal_cap_values[] = { CAP_SETUID, CAP_SETGID, CAP_SETPCAP };
# endif

   cap_t caps = cap_init();

   if (caps == 0) U_ERROR("cap_init() failed");

   (void) cap_clear(caps);

   (void) cap_set_flag(caps, CAP_EFFECTIVE,   3, minimal_cap_values, CAP_SET);
   (void) cap_set_flag(caps, CAP_PERMITTED,   3, minimal_cap_values, CAP_SET);
   (void) cap_set_flag(caps, CAP_INHERITABLE, 3, minimal_cap_values, CAP_SET);

   if (cap_set_proc(caps) < 0) U_ERROR("cap_set_proc() failed");

   (void) cap_free(caps);

   if (prctl(U_PR_SET_KEEPCAPS, 1, 0, 0, 0) < 0) U_ERROR("prctl() failed");
# endif

   U_INTERNAL_TRACE("u_never_need_root()")

   U_INTERNAL_PRINT("(_euid_=%d, uid=%d)", effective_uid, real_uid)

   if (real_uid == (uid_t)(-1)) U_ERROR("u_init_security() not called");

   if (geteuid() == 0) (void) setuid(real_uid);

   if (geteuid() != real_uid ||
       getuid()  != real_uid)
      {
      U_ERROR("Did not drop root privilege");
      }

   effective_uid = real_uid;
#endif
}

/* Temporarily gain group privileges */

void u_need_group(bool necessary)
{
   U_INTERNAL_TRACE("u_need_group(%d)", necessary)

#ifndef _MSWINDOWS_
   U_INTERNAL_PRINT("(egid_=%d, gid=%d)", effective_gid, real_gid)

   if (real_gid == (gid_t)(-1)) U_ERROR("u_init_security() not called");

   if (getegid() == effective_gid) return; /* nothing to do */

    if (setegid(effective_gid) == -1 ||
        getegid() != effective_gid)
      {
      if (necessary) U_ERROR("Did not get group privilege");
                     U_DEBUG("Did not get group privilege");
      }
#endif
}

/* Temporarily drop group privileges */

void u_dont_need_group(void)
{
   U_INTERNAL_TRACE("u_dont_need_group()")

#ifndef _MSWINDOWS_
   U_INTERNAL_PRINT("(egid_=%d, gid=%d)", effective_gid, real_gid)

   if (real_gid == (gid_t)(-1)) U_ERROR("u_init_security() not called");

   if (getegid() != effective_gid) return; /* nothing to do */

    if (setegid(real_gid) == -1 ||
        getegid() != real_gid)
      {
      U_ERROR("Did not drop group privilege");
      }
#endif
}

/* Permanently drop group privileges */

void u_never_need_group(void)
{
   U_INTERNAL_TRACE("u_never_need_group()")

#ifndef _MSWINDOWS_
   U_INTERNAL_PRINT("(egid_=%d, gid=%d)", effective_gid, real_gid)

   if (real_gid == (gid_t)(-1)) U_ERROR("u_init_security() not called");

   if (getegid() != effective_gid) (void) setgid(real_gid);

   if (getegid() != real_gid ||
       getgid()  != real_gid)
      {
      U_ERROR("Did not drop group privilege");
      }

    effective_gid = real_gid;
#endif
}

/* Change the current working directory to the `user` user's home dir, and downgrade security to that user account */

bool u_runAsUser(const char* restrict user, bool change_dir)
{
#ifdef _MSWINDOWS_
   return false;
#else
   struct passwd* restrict pw;

   U_INTERNAL_TRACE("u_runAsUser(%s,%d)", user, change_dir)

   U_INTERNAL_ASSERT_POINTER(user)

   if (!(pw = getpwnam(user)) ||
       setgid(pw->pw_gid)     ||
       setuid(pw->pw_uid))
      {
      return false;
      }

   (void) u__strncpy(u_user_name, user, (u_user_name_len = u__strlen(user, __PRETTY_FUNCTION__))); /* change user name */

   if (change_dir &&
       pw->pw_dir &&
       pw->pw_dir[0])
      {
      (void) chdir(pw->pw_dir);

      u_getcwd(); /* get current working directory */

      U_INTERNAL_ASSERT_EQUALS(strcmp(pw->pw_dir,u_cwd), 0)
      }

   return true;
#endif
}

char* u_getPathRelativ(const char* restrict path, uint32_t* restrict ptr_path_len)
{
   U_INTERNAL_TRACE("u_getPathRelativ(%s,%u)", path, *ptr_path_len)

   U_INTERNAL_ASSERT_POINTER(path)
   U_INTERNAL_ASSERT_POINTER(u_cwd)

   U_INTERNAL_PRINT("u_cwd = %s", u_cwd)

   if (path[0] == u_cwd[0])
      {
      uint32_t path_len = *ptr_path_len;

      while (path[path_len-1] == '/') --path_len;

      if (path_len >= u_cwd_len &&
          memcmp(path, u_cwd, u_cwd_len) == 0)
         {
         if (path_len == u_cwd_len)
            {
            path     = ".";
            path_len = 1;
            }
         else if (     u_cwd_len  == 1 ||
                  path[u_cwd_len] == '/')
            {
            uint32_t len = u_cwd_len + (u_cwd_len > 1);

            path     += len;
            path_len -= len;

            while (path[0] == '/')
               {
               ++path;
               --path_len;
               }
            }
         }

      *ptr_path_len = path_len;

      U_INTERNAL_PRINT("path(%u) = %.*s", path_len, path_len, path)
      }

   if (path[0] == '.' &&
       path[1] == '/')
      {
      path          += 2;
      *ptr_path_len -= 2;

      while (path[0] == '/')
         {
         ++path;
         --(*ptr_path_len);
         }

      U_INTERNAL_PRINT("path(%u) = %.*s", *ptr_path_len, *ptr_path_len, path)
      }

   return (char*)path;
}

/* find sequence of U_LF2 or U_CRLF2 */

__pure uint32_t u_findEndHeader(const char* restrict str, uint32_t n)
{
   const char* restrict p;
   const char* restrict end = str + n;
   const char* restrict ptr = str;

   uint32_t pos, endHeader = U_NOT_FOUND;

   U_INTERNAL_TRACE("u_findEndHeader(%.*s,%u)", U_min(n,128), str, n)

   U_INTERNAL_ASSERT_POINTER(str)

   while (ptr < end)
      {
      p = (const char* restrict) memchr(ptr, '\n', end - ptr);

      if (p == 0) break;

      if (u_get_unalignedp32(p-1) == U_MULTICHAR_CONSTANT32('\r','\n','\r','\n'))
         {
         pos = p - str + 3;

         if (pos <= n)
            {
            endHeader = pos;

         /* U_line_terminator_len = 2; */
            }

         break;
         }

      /* \n\n (U_LF2) */

      if (p[1] == '\n')
         {
         U_INTERNAL_ASSERT_EQUALS(u_get_unalignedp16(p), U_MULTICHAR_CONSTANT16('\n','\n'))

         pos = p - str + 2;

         if (pos <= n)
            {
            endHeader = pos;

         /* U_line_terminator_len = 1; */
            }

         break;
         }

      ptr = p + 1;
      }

   return endHeader;
}

/* Determine the width of the terminal we're running on */

__pure int u_getScreenWidth(void)
{
#ifdef TIOCGWINSZ
   struct winsize wsz;
#endif

   U_INTERNAL_TRACE("u_getScreenWidth()")

   /* If there's a way to get the terminal size using POSIX tcgetattr(), somebody please tell me */

#ifdef TIOCGWINSZ
   if (ioctl(STDERR_FILENO, TIOCGWINSZ, &wsz) != -1)  /* most likely ENOTTY */
      {
      U_INTERNAL_PRINT("wsz.ws_col = %d", wsz.ws_col)

      return wsz.ws_col;
      }
#endif

   return 0;
}

/**
 * Calculate the download rate and trim it as appropriate for the speed. Appropriate means that
 * if rate is greater than 1K/s, kilobytes are used, and if rate is greater than 1MB/s, megabytes are used.
 * UNITS is zero for B/s, one for KB/s, two for MB/s, and three for GB/s
 */

double u_calcRate(uint64_t bytes, uint32_t msecs, int* restrict units)
{
   int i;
   double rate = (double)1000. * bytes / (double)msecs;

   U_INTERNAL_TRACE("u_calcRate(%u,%u,%p)", bytes, msecs, units)

   U_INTERNAL_ASSERT_POINTER(units)
   U_INTERNAL_ASSERT_MAJOR(bytes, 0)
   U_INTERNAL_ASSERT_MAJOR(msecs, 0)

   for (i = 0; rate > 1024. && u_short_units[i+1]; ++i) rate /= 1024.;

   *units = i;

   U_INTERNAL_PRINT("rate = %7.2f%s", rate, u_short_units[i])

   return rate;
}

uint32_t u_printSize(char* restrict buffer, uint64_t bytes)
{
   int units;
   double size;
   uint32_t len;

   U_INTERNAL_TRACE("u_printSize(%p,%llu)", buffer, bytes)

   if (bytes == 0)
      {
      u_put_unalignedp32(buffer,   U_MULTICHAR_CONSTANT32('0',' ','B','y'));
      u_put_unalignedp16(buffer+4, U_MULTICHAR_CONSTANT16('t','e'));

      len = 6;
      }
   else
      {
      size = u_calcRate(bytes, 1000, &units);

      len = (units ? sprintf(buffer, "%5.2f %s", size, u_short_units[units])
                   : sprintf(buffer, "%7.0f Bytes", size));
      }

   buffer[len] = '\0';

   return len;
}

uint32_t u_memory_dump(char* restrict bp, unsigned char* restrict cp, uint32_t n)
{
   char text[16];
   unsigned char c;
   unsigned int offset = 0;
   char* restrict start_buffer = bp;
   int i, j, line, remain, _remain = 16;
   bool prev_is_zero = false, print_nothing = false;

   static char bufzero[16];

   U_INTERNAL_TRACE("u_memory_dump(%p,%p,%u)", bp, cp, n)

   line   = n / 16;
   remain = n % 16;

   for (i = 0; i < line; ++i, offset += 16)
      {
      if (memcmp(cp, bufzero, sizeof(bufzero))) prev_is_zero = print_nothing = false;
      else
         {
         if (prev_is_zero == false) prev_is_zero = true;
         else
            {
            if (print_nothing == false)
               {
               print_nothing = true;

               u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16('*','\n'));

               bp += 2;
               }

            cp += 16;

            continue;
            }
         }
iteration:
      (void) sprintf(bp, "%07X|", offset);

      U_INTERNAL_ASSERT_EQUALS(strlen(bp), 8)

      bp += 8;

      for (j = 0; j < 16; ++j)
         {
         if (j < _remain)
            {
            c = *cp++;

            u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16("0123456789abcdef"[((c >> 4) & 0x0F)],
                                                          "0123456789abcdef"[( c       & 0x0F)]));

            bp += 2;

            text[j] = (u__isprint(c) ? c : '.');
            }
         else
            {
            u_put_unalignedp16(bp, U_MULTICHAR_CONSTANT16(' ',' '));

            bp += 2;

            text[j] = ' ';
            }

         *bp++ = (j == 7 ? ':' : ' ');
         }

      *bp++ = '|';

      for (j = 0; j < 16; ++j) *bp++ = text[j];

      *bp++ = '\n';
      }

   if (remain &&
       remain != _remain)
      {
      _remain = remain;

      goto iteration;
      }

   if (print_nothing)
      {
      (void) sprintf(bp, "%07X\n", offset);

      U_INTERNAL_ASSERT_EQUALS(strlen(bp), 8)

      bp += 8;
      }

   return (bp - start_buffer);
}

char* u_memoryDump(char* restrict bp, unsigned char* restrict cp, uint32_t n)
{
   uint32_t written;

   U_INTERNAL_TRACE("u_memoryDump(%p,%p,%u)", bp, cp, n)

   written = u_memory_dump(bp, cp, n);

   U_INTERNAL_ASSERT_MINOR(written, 4096)

   bp[written] = '\0';

   return bp;
}

/* get the number of the processors including offline CPUs */

static inline const char* nexttoken(const char* q, int sep)
{
   if (q) q = strchr(q, sep);
   if (q) ++q;

   return q;
}

/**
 * When parsing bitmask lists, only allow numbers, separated by one
 * of the allowed next characters.
 *
 * The parameter 'sret' is the return from a sscanf "%u%c".  It is
 * -1 if the sscanf input string was empty.  It is 0 if the first
 * character in the sscanf input string was not a decimal number.
 * It is 1 if the unsigned number matching the "%u" was the end of the
 * input string.  It is 2 if one or more additional characters followed
 * the matched unsigned number.  If it is 2, then 'nextc' is the first
 * character following the number.  The parameter 'ok_next_chars'
 * is the nul-terminated list of allowed next characters.
 *
 * The mask term just scanned was ok if and only if either the numbers
 * matching the %u were all of the input or if the next character in
 * the input past the numbers was one of the allowed next characters
 */

static inline bool scan_was_ok(int sret, char nextc, const char* ok_next_chars) { return (sret == 1 || (sret == 2 && strchr(ok_next_chars, nextc))); }

int u_get_num_cpu(void)
{
   U_INTERNAL_TRACE("u_get_num_cpu()")

   if (u_num_cpu == -1)
      {
#  ifdef _SC_NPROCESSORS_ONLN
      u_num_cpu = sysconf(_SC_NPROCESSORS_ONLN);
#  elif defined(_SC_NPROCESSORS_CONF)
      u_num_cpu = sysconf(_SC_NPROCESSORS_CONF);
#  else
      FILE* fp = fopen("/sys/devices/system/cpu/present", "r");

      if (fp)
         {
         char buf[128];
         const char* q;

         char nextc;          /* char after sscanf %u match */
         unsigned int a;      /* begin of range */
         unsigned int b;      /* end of range */
         unsigned int s;      /* stride */

         if (fgets(buf, sizeof(buf), fp))
            {
            const char* p;

            q = buf;

            buf[u__strlen(buf, __PRETTY_FUNCTION__) - 1] = '\0';

            /**
             * Parses a comma-separated list of numbers and ranges
             * of numbers,with optional ':%u' strides modifying ranges.
             *
             * Some examples of input lists and their equivalent simple list:
             *
             *  Input           Equivalent to
             *   0-3             0,1,2,3
             *   0-7:2           0,2,4,6
             *   1,3,5-7         1,3,5,6,7
             *   0-3:2,8-15:4    0,2,8,12
             */

            while (p = q, q = nexttoken(q, ','), p)
               {
               const char* c1;
               const char* c2;
               int sret = sscanf(p, "%u%c", &a, &nextc);

               if (scan_was_ok(sret, nextc, ",-") == false) break;

               b  = a;
               s  = 1;
               c1 = nexttoken(p, '-');
               c2 = nexttoken(p, ',');

               if (c1 != 0 && (c2 == 0 || c1 < c2))
                  {
                  sret = sscanf(c1, "%u%c", &b, &nextc);

                  if (scan_was_ok(sret, nextc, ",:") == false) break;

                  c1 = nexttoken(c1, ':');

                  if (c1 != 0 && (c2 == 0 || c1 < c2))
                     {
                     sret = sscanf(c1, "%u%c", &s, &nextc);

                     if (scan_was_ok(sret, nextc, ",") == false) break;
                     }
                  }

               if (!(a <= b)) break;

               while (a <= b)
                  {
                  u_num_cpu = a + 1; /* Number of highest set bit +1 is the number of the CPUs */

                  a += s;
                  }
               }
            }

         (void) fclose(fp);
         }
#endif
      }

   return u_num_cpu;
}

/* Pin the process to a particular core */

void u_bind2cpu(cpu_set_t* cpuset, int n)
{
   U_INTERNAL_TRACE("u_bind2cpu(%p,%d)", cpuset, n)

   /**
    * CPU mask of CPUs available to this process,
    * conceptually, each bit represents a logical CPU, ie:
    *
    * mask = 3  (11b):   cpu0, 1
    * mask = 13 (1101b): cpu0, 2, 3
    */

#if !defined(U_SERVER_CAPTIVE_PORTAL) && defined(HAVE_SCHED_GETAFFINITY)
   CPU_SET(n, cpuset);

   (void) sched_setaffinity(u_pid, sizeof(cpu_set_t), cpuset);

   CPU_ZERO(cpuset);

   (void) sched_getaffinity(u_pid, sizeof(cpu_set_t), cpuset);

   U_INTERNAL_PRINT("cpuset = %ld", CPUSET_BITS(cpuset)[0])
#endif
}

void u_switch_to_realtime_priority(void)
{
#if !defined(U_SERVER_CAPTIVE_PORTAL) && defined(_POSIX_PRIORITY_SCHEDULING) && (_POSIX_PRIORITY_SCHEDULING > 0) && (defined(HAVE_SCHED_H) || defined(HAVE_SYS_SCHED_H))
   struct sched_param sp;

   U_INTERNAL_TRACE("u_switch_to_realtime_priority()")

/* sched_getscheduler(u_pid); // SCHED_FIFO | SCHED_RR | SCHED_OTHER */

   (void) sched_getparam(u_pid, &sp);

   sp.sched_priority = sched_get_priority_max(SCHED_FIFO);

   U_INTERNAL_PRINT("sp.sched_priority = %d", sp.sched_priority)

   /*
   struct rlimit rlim_old, rlim_new = { sp.sched_priority, sp.sched_priority };

   (void) prlimit(u_pid, RLIMIT_RTPRIO, &rlim_new, &rlim_old);

   U_INTERNAL_PRINT("Previous RLIMIT_RTPRIO limits: soft=%lld; hard=%lld\n", (long long)rlim_old.rlim_cur, (long long)rlim_old.rlim_max);

   (void) prlimit(u_pid, RLIMIT_RTPRIO, 0, &rlim_old);

   U_INTERNAL_PRINT("New RLIMIT_RTPRIO limits: soft=%lld; hard=%lld\n", (long long)rlim_old.rlim_cur, (long long)rlim_old.rlim_max);
   */

   if (sched_setscheduler(u_pid, SCHED_FIFO, &sp) == -1) U_WARNING("Cannot set posix realtime scheduling policy");
#endif
}

void u_get_memusage(unsigned long* vsz, unsigned long* rss)
{
   FILE* fp = fopen("/proc/self/stat", "r");

   U_INTERNAL_TRACE("u_get_memusage(%p,%p)", vsz, rss)

   if (fp)
      {
      /**
       * The fields, in order, with their proper scanf(3) format specifiers, are:
       * -----------------------------------------------------------------------------------------------------------------------------
       * pid %d          The process ID.
       * comm %s         The filename of the executable, in parentheses.  This is visible whether or not the executable is swapped out.
       * state %c        R is running, S is sleeping, D is waiting, Z is zombie, T is traced or stopped (on a signal), and W is paging.
       * ppid %d         The PID of the parent.
       * pgrp %d         The process group ID of the process.
       * session %d      The session ID of the process.
       * tty_nr %d       The controlling terminal of the process.
       * tpgid %d        The ID of the foreground process group of the controlling terminal of the process.
       * flags %u        The kernel flags word of the process.
       * minflt %lu      The number of minor faults the process has made which have not required loading a memory page from disk.
       * cminflt %lu     The number of minor faults that the process's waited-for children have made.
       * majflt %lu      The number of major faults the process has made which have required loading a memory page from disk.
       * cmajflt %lu     The number of major faults that the process's waited-for children have made.
       * utime %lu       Amount of time that this process has been scheduled in   user mode, measured in clock ticks.
       * stime %lu       Amount of time that this process has been scheduled in kernel mode, measured in clock ticks.
       * cutime %ld      Amount of time that this process's waited-for children have been scheduled in   user mode, measured in clock ticks.
       * cstime %ld      Amount of time that this process's waited-for children have been scheduled in kernel mode, measured in clock ticks.
       * priority %ld    For processes running a real-time scheduling policy , this is the negated scheduling priority, minus one. 
       * nice %ld        The nice value (see setpriority(2)), a value in the range 19 (low priority) to -20 (high priority).
       * num_threads %ld Number of threads in this process.
       * itrealvalue %ld The time in jiffies before the next SIGALRM is sent to the process due to an interval timer.
       * starttime %llu  The time in jiffies the process started after system boot.
       * -----------------------------------------------------------------------------------------------------------------------------
       * vsize %lu Virtual memory size in bytes.
       * rss %ld   Resident Set Size: number of pages the process has in real memory.
       *           This is just the pages which count toward text, data, or stack space.
       *           This does not include pages which have not been demand-loaded in, or which are swapped out
       * -----------------------------------------------------------------------------------------------------------------------------
       */

      (void) fscanf(fp, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lu %lu", vsz, rss);

      (void) fclose(fp);

      *rss *= PAGESIZE;
      }
}

uint32_t u_get_uptime(void)
{
   FILE* fp = fopen("/proc/uptime", "r");

   U_INTERNAL_TRACE("u_get_uptime()")

   if (fp)
      {
      uint32_t sec;

      (void) fscanf(fp, "%u", &sec);

      (void) fclose(fp);

      return sec;
      }

   return 0;
}

__pure bool u_rmatch(const char* restrict haystack, uint32_t haystack_len, const char* restrict needle, uint32_t needle_len)
{
   U_INTERNAL_TRACE("u_rmatch(%.*s,%u,%.*s,%u)", U_min(haystack_len,128), haystack, haystack_len,
                                                 U_min(  needle_len,128),   needle,   needle_len)

   U_INTERNAL_ASSERT_POINTER(needle)
   U_INTERNAL_ASSERT_POINTER(haystack)
   U_INTERNAL_ASSERT_MAJOR(haystack_len,0)

   if (haystack_len >= needle_len)
      {
      /* see if substring characters match at end */

      const char* restrict nn = needle   + needle_len   - 1;
      const char* restrict hh = haystack + haystack_len - 1;

      while (*nn-- == *hh--)
         {
         if (nn >= needle) continue;

         return true; /* we got all the way to the start of the substring so we must've won */
         }
      }

   return false;
}

/**
 * Search a string for any of a set of characters.
 * Locates the first occurrence in the string s of any of the characters in the string accept
 */
 
__pure const char* u__strpbrk(const char* restrict s, uint32_t slen, const char* restrict _accept)
{
   const char* restrict c;
   const char* restrict end = s + slen;

   U_INTERNAL_TRACE("u__strpbrk(%.*s,%u,%s)", U_min(slen,128), s, slen, _accept)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_MAJOR(slen, 0)
   U_INTERNAL_ASSERT_POINTER(_accept)

   while (s < end)
      {
      for (c = _accept; *c; ++c)
         {
         if (*s == *c) return s;
         }

      ++s;
      }

   return 0;
}

/* Search a string for a terminator of a group of delimitator {} [] () <%%>...*/

__pure const char* u_strpend(const char* restrict s, uint32_t slen,
                             const char* restrict group_delimitor, uint32_t group_delimitor_len, char skip_line_comment)
{
   char c;
   int level = 1;
   const char* restrict end = s + slen;
   uint32_t i, n = group_delimitor_len / 2;

   U_INTERNAL_TRACE("u_strpend(%.*s,%u,%s,%u,%d)", U_min(slen,128), s, slen, group_delimitor, group_delimitor_len, skip_line_comment)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_MAJOR(slen,0)
   U_INTERNAL_ASSERT_POINTER(group_delimitor)
   U_INTERNAL_ASSERT_EQUALS(s[0], group_delimitor[n-1])
   U_INTERNAL_ASSERT_EQUALS(group_delimitor_len & 1, 0)

   while (s < end)
      {
loop: c = *++s;

      if (u__isspace(c)) continue;

      if (c == skip_line_comment)
         {
         /* skip line comment */

         s = (const char* restrict) memchr(s, '\n', end - s);

         if (s == 0) break;
         }
      else if (c == group_delimitor[0] &&
               *(s-1) != '\\')
         {
         U_INTERNAL_PRINT("c = %c level = %d s = %.*s", c, level, 10, s)

         for (i = 1; i < n; ++i)
            {
            U_INTERNAL_PRINT("s[%d] = %c group_delimitor[%d] = %c", i, s[i], i, group_delimitor[i])

            if (s[i] != group_delimitor[i]) goto loop;
            }

         ++level;
         }
      else if (c == group_delimitor[n] &&
               *(s-1) != '\\')
         {
         U_INTERNAL_PRINT("c = %c level = %d s = %.*s", c, level, 10, s)

         for (i = 1; i < n; ++i)
            {
            U_INTERNAL_PRINT("s[%d] = %c group_delimitor[%d] = %c", i, s[i], n+i, group_delimitor[n+i])

            if (s[i] != group_delimitor[n+i]) goto loop;
            }

         if (--level == 0) return s;
         }

      U_INTERNAL_PRINT("level = %d s = %.*s", level, 10, s)
      }

   return 0;
}

/* check if string a start with string b */

__pure bool u_startsWith(const char* restrict a, uint32_t n1, const char* restrict b, uint32_t n2)
{
   int32_t diff = n1 - n2;

   U_INTERNAL_TRACE("u_startsWith(%.*s,%u,%.*s,%u)", U_min(n1,128), a, n1, U_min(n2,128), b, n2)

   if (diff >= 0 &&
       (memcmp(a, b, n2) == 0))
      {
      return true;
      }

   return false;
}

/* check if string a terminate with string b */

__pure bool u_endsWith(const char* restrict a, uint32_t n1, const char* restrict b, uint32_t n2)
{
   int32_t diff = n1 - n2;

   U_INTERNAL_TRACE("u_endsWith(%.*s,%u,%.*s,%u)", U_min(n1,128), a, n1, U_min(n2,128), b, n2)

   if (diff >= 0 &&
       (memcmp(a+diff, b, n2) == 0))
      {
      return true;
      }

   return false;
}

__pure bool u_isNumber(const char* restrict s, uint32_t n)
{
   int vdigit[]             = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };
   const char* restrict end = s + n;

   U_INTERNAL_TRACE("u_isNumber(%.*s,%u)", U_min(n,128), s, n)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_MAJOR(n,0)

   if (*s == '+' ||
       *s == '-')
      {
      ++s;
      }

   while (s < end &&
          ((*(const unsigned char* restrict)s) >> 4) == 0x03 &&
          vdigit[(*(const unsigned char* restrict)s)  & 0x0f])
      {
      U_INTERNAL_PRINT("*s = %c, *s >> 4 = %c ", *s, (*(char* restrict)s) >> 4)

      ++s;
      }

   return (s == end);
}

/* find first char not quoted */

__pure const char* u_find_char(const char* restrict s, const char* restrict end, char c)
{
   uint32_t i;

   U_INTERNAL_TRACE("u_find_char(%.*s,%p,%d)", U_min(end-s,128), s, end, c)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_POINTER(end)
   U_INTERNAL_ASSERT_EQUALS(s[-1],c)

loop:
   s = (const char* restrict) memchr(s, c, end - s);

   if (s == 0) s = end;
   else
      {
      if (*(s-1) == '\\')
         {
         for (i = 2; (*(s-i) == '\\'); ++i) {}

         if ((i & 1) == 0)
            {
            ++s;

            goto loop;
            }
         }
      }

   return s;
}

/* skip string delimiter or white space and line comment */

__pure const char* u_skip(const char* restrict s, const char* restrict end, const char* restrict delim, char line_comment)
{
   U_INTERNAL_TRACE("u_skip(%.*s,%p,%s,%d)", U_min(end-s,128), s, end, delim, line_comment)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_POINTER(end)

   if (delim)
      {
      /* skip string delimiter */

      while (s < end &&
             strchr(delim, *s))
         {
         ++s;
         }
      }
   else
      {
skipws:
      while (s < end &&
             u__isspace(*s))
         {
         ++s;
         }

      if (line_comment)
         {
         if (*s == line_comment)
            {
            /* skip line comment */

            s = (const char* restrict) memchr(s, '\n', end - s);

            if (s) goto skipws;

            return end;
            }
         }
      }

   return s;
}

/* delimit token */

const char* u_delimit_token(const char* restrict s, const char** restrict pold, const char* restrict end, const char* restrict delim, char skip_line_comment)
{
   char c;

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_POINTER(end)
   U_INTERNAL_ASSERT_POINTER(pold)

   U_INTERNAL_TRACE("u_delimit_token(%.*s,%p,%p,%s,%d)", U_min(end-s,128), s, pold, end, delim, skip_line_comment)

   U_INTERNAL_PRINT("s = %p", s)

   s = u_skip(s, end, delim, skip_line_comment);

   U_INTERNAL_PRINT("s = %p", s)

   if (s == end)
      {
      *pold = 0;

      return ++end;
      }

   *pold =  s;
       c = *s++;

   /* NB: we don't search for delimiter in block text... */

   if (u__isquote(c))
      {
      s = u_find_char(s, end, c);

      U_INTERNAL_PRINT("s = %p", s)

      if (delim)
         {
         if (++s < end) goto next; /* NB: goto next char (skip '"')... */

         /* NB: we consider this block text with '"' as token... */

         s = end;
         }
      else
         {
         /* NB: we consider this block text without '"' as token... */

         ++(*pold);
         }

      goto end;
      }

   if (delim)
      {
next: s = (const char* restrict) (s < end ? u__strpbrk(s, end - s, delim) : 0);

      if (s == 0) return end;
      }
   else
      {
      /* find next white space */

      while (s < end &&
             u__isspace(*s) == false)
         {
         ++s;
         }
      }

end:
   U_INTERNAL_PRINT("s = %p end = %p result = \"%.*s\"", s, end, (s-(*pold)), *pold)

   return s;
}

uint32_t u_split(char* restrict s, uint32_t n, char** restrict argv, const char* restrict delim)
{
   const char* restrict p;
   char* restrict end  = s + n;
   char** restrict ptr = argv;

   U_INTERNAL_TRACE("u_split(%.*s,%u,%p,%s)", U_min(n,128), s, n, argv, delim)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_MAJOR(n,0)
   U_INTERNAL_ASSERT_POINTER(argv)
   U_INTERNAL_ASSERT_EQUALS(u_isBinary((const unsigned char*)s,n),false)

   while (s < end)
      {
      s = (char* restrict) u_delimit_token(s, (const char** restrict)&p, end, delim, 0);

      U_INTERNAL_PRINT("s = %.*s", 20, s)

      if (s <= end)
         {
         *argv++ = (char* restrict) p;

         *s++ = '\0';

         U_INTERNAL_PRINT("u_split() = %s", p)
         }
      }

   *argv = 0;

   n = (argv - ptr);

   return n;
}

/**
 * Match STRING against the filename pattern MASK, returning true if it matches, false if not, inversion if flags contain FNM_INVERT
 *
 * '?' matches any single character
 * '*' matches any string, including the empty string
 */

__pure bool u_dosmatch(const char* restrict s, uint32_t n1, const char* restrict mask, uint32_t n2, int flags)
{
   bool result;
   const char* restrict cp = 0;
   const char* restrict mp = 0;
   unsigned char c1 = 0, c2 = 0;

   const char* restrict end_s    =    s + n1;
   const char* restrict end_mask = mask + n2;

   U_INTERNAL_TRACE("u_dosmatch(%.*s,%u,%.*s,%u,%d)", U_min(n1,128), s, n1, n2, mask, n2, flags)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_MAJOR(n1, 0)
   U_INTERNAL_ASSERT_MAJOR(n2, 0)
   U_INTERNAL_ASSERT_POINTER(mask)

   if (flags & FNM_IGNORECASE)
      {
      while (s < end_s)
         {
         c2 = u__tolower(*mask);

         if (c2 == '*') break;

         c1 = u__tolower(*s);

         if (c2 != c1 &&
             c2 != '?')
            {
            return ((flags & FNM_INVERT) != 0);
            }

         ++s;
         ++mask;
         }

      U_INTERNAL_PRINT("c1 = %c c2 = %c", c1, c2)

      while (true)
         {
         if (s >= end_s)
            {
            if (mask == 0) return false;

            while (*mask == '*') ++mask;

            result = (mask >= end_mask);

            return ((flags & FNM_INVERT) != 0 ? (result == false) : result);
            }

         c2 = (mask ? u__tolower(*mask) : 0);

         if (c2 == '*')
            {
            if (++mask >= end_mask) return ((flags & FNM_INVERT) == 0);

            cp = s+1;
            mp = mask;

            continue;
            }

         c1 = u__tolower(*s);

         U_INTERNAL_PRINT("c1 = %c c2 = %c", c1, c2)

         if (c2 == c1 ||
             c2 == '?')
            {
            ++s;
            ++mask;

            continue;
            }

         s    = (cp ? cp++ : end_s);
         mask = mp;
         }
      }
   else
      {
      while (s < end_s)
         {
         c2 = *mask;

         if (c2 == '*') break;

         c1 = *s;

         if (c2 != c1 &&
             c2 != '?')
            {
            return ((flags & FNM_INVERT) != 0);
            }

         ++s;
         ++mask;
         }

      U_INTERNAL_PRINT("c1 = %c c2 = %c", c1, c2)

      while (true)
         {
         if (s >= end_s)
            {
            while (*mask == '*') ++mask;

            result = (mask >= end_mask);

            return ((flags & FNM_INVERT) != 0 ? (result == false) : result);
            }

         c2 = *mask;

         if (c2 == '*')
            {
            if (++mask >= end_mask) return ((flags & FNM_INVERT) == 0);

            cp = s + 1;
            mp = mask;

            continue;
            }

         c1 = *s;

         U_INTERNAL_PRINT("c1 = %c c2 = %c", c1, c2)

         if (c2 == c1 ||
             c2 == '?')
            {
            ++s;
            ++mask;

            continue;
            }

         s    = cp++;
         mask = mp;
         }
      }
}

__pure bool u_dosmatch_ext(const char* restrict s, uint32_t n1, const char* restrict mask, uint32_t n2, int flags)
{
   U_INTERNAL_TRACE("u_dosmatch_ext(%.*s,%u,%.*s,%u,%d)", U_min(n1,128), s, n1, n2, mask, n2, flags)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_MAJOR(n1, 0)
   U_INTERNAL_ASSERT_MAJOR(n2, 0)
   U_INTERNAL_ASSERT_POINTER(mask)

   while (n2)
      {
      U_INTERNAL_PRINT("switch: s[0] = %c n1 = %u mask[0] = %c n2 = %u", s[0], n1, mask[0], n2)

      switch (mask[0])
         {
         case '*':
            {
            while (mask[1] == '*')
               {
               ++mask;
               --n2;
               }

            if (n2 == 1) return ((flags & FNM_INVERT) == 0); /* match */

            while (n1)
               {
               if (u_dosmatch_ext(s, n1, mask+1, n2-1, flags & ~FNM_INVERT)) return ((flags & FNM_INVERT) == 0); /* match */

               ++s;
               --n1;
               }

            return ((flags & FNM_INVERT) != 0); /* no match */
            }
         break;

         case '?':
            {
            if (n1 == 0) return ((flags & FNM_INVERT) != 0); /* no match */

            ++s;
            --n1;
            }
         break;

         case '[':
            {
            bool match = false,
                  bnot = (mask[1] == '^');

            if (bnot)
               {
               mask += 2;
                 n2 -= 2;
               }
            else
               {
               ++mask;
               --n2;
               }

            U_INTERNAL_PRINT("s[0] = %c n1 = %u mask[0] = %c n2 = %u", s[0], n1, mask[0], n2)

            while (true)
               {
               if (mask[0] == '\\')
                  {
                  ++mask;
                  --n2;

                  if (mask[0] == s[0]) match = true;
                  }
               else
                  {
                  if (mask[0] == ']') break;

                  if (n2 == 0)
                     {
                     --mask;
                     ++n2;

                     break;
                     }

                  if (n2 >= 3 && 
                      mask[1] == '-')
                     {
                     int start = mask[0],
                           end = mask[2],
                             c =    s[0];

                     if (start > end)
                        {
                        int t = start;
                        start = end;
                          end = t;
                        }

                     if (flags & FNM_IGNORECASE)
                        {
                        start = u__tolower((unsigned char)start);
                          end = u__tolower((unsigned char)end);
                            c = u__tolower((unsigned char)c);
                        }

                     mask += 2;
                       n2 -= 2;

                     if (c >= start &&
                         c <= end)
                        {
                        match = true;
                        }
                     }
                  else
                     {
                     if ((flags & FNM_IGNORECASE) == 0)
                        {
                        if (mask[0] == s[0]) match = true;
                        }
                     else
                        {
                        if (u__tolower((unsigned char)mask[0]) == u__tolower((unsigned char)s[0])) match = true;
                        }
                     }
                  }

               ++mask;
               --n2;
               }

            U_INTERNAL_PRINT("match = %d bnot = %d", match, bnot)

            if (match == false || bnot) return ((flags & FNM_INVERT) != 0); /* no match */

            U_INTERNAL_PRINT("s[0] = %c n1 = %u mask[0] = %c n2 = %u", s[0], n1, mask[0], n2)

            ++s;
            --n1;
            }
         break;

         case '\\':
            {
            if (n2 >= 2)
               {
               ++mask;
               --n2;
               }
            }

         /* fall through */

         default:
            {
            U_INTERNAL_PRINT("default: s[0] = %c n1 = %u mask[0] = %c n2 = %u", s[0], n1, mask[0], n2)

            if ((flags & FNM_IGNORECASE) == 0)
               {
               if (mask[0] != s[0]) return ((flags & FNM_INVERT) != 0); /* no match */
               }
            else
               {
               if (u__tolower((unsigned char)mask[0]) != u__tolower((unsigned char)s[0])) return ((flags & FNM_INVERT) != 0); /* no match */
               }

            ++s;
            --n1;
            }
         break;
         }

      ++mask;
      --n2;

      if (n1 == 0)
         {
         while (*mask == '*')
            {
            ++mask;
            --n2;
            }

         break;
         }
      }

   U_INTERNAL_PRINT("n1 = %u n2 = %u", n1, n2)

   if (n2 == 0 &&
       n1 == 0)
      {
      return ((flags & FNM_INVERT) == 0);
      }

   return ((flags & FNM_INVERT) != 0);
}

__pure bool u_match_with_OR(bPFpcupcud pfn_match, const char* restrict s, uint32_t n1, const char* restrict pattern, uint32_t n2, int flags)
{
   const char* restrict p_or;
   const char* restrict end = pattern + n2;

   U_INTERNAL_TRACE("u_match_with_OR(%p,%.*s,%u,%.*s,%u,%d)", pfn_match, U_min(n1,128), s, n1, n2, pattern, n2, flags)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_MAJOR(n1, 0)
   U_INTERNAL_ASSERT_MAJOR(n2, 0)
   U_INTERNAL_ASSERT_POINTER(pattern)

   while (true)
      {
      p_or = (const char* restrict) memchr(pattern, '|', n2);

      if (p_or == 0) return pfn_match(s, n1, pattern, n2, flags);

      if (pfn_match(s, n1, pattern, (p_or - pattern), (flags & ~FNM_INVERT))) return ((flags & FNM_INVERT) == 0);

      pattern = p_or + 1;
         n2   = end - pattern;
      }
}

/**
 * Verifies that the passed string is actually an e-mail address
 *
 * see: http://www.remote.org/jochen/mail/info/chars.html
 */

#define RFC822_SPECIALS "()<>@,;:\\\"[]"

__pure bool u_validate_email_address(const char* restrict address, uint32_t address_len)
{
   int count;
   const char* restrict c;
   const char* restrict end;
   const char* restrict domain;

   U_INTERNAL_TRACE("u_validate_email_address(%.*s,%u)", U_min(address_len,128), address, address_len)

   if (address_len < 3) return false;

   /* first we validate the name portion (name@domain) */

   for (c = address, end = address + address_len; c < end; ++c)
      {
      U_INTERNAL_PRINT("c = %c", *c)

      if (*c == '\"' &&
          (c == address || *(c - 1) == '.' || *(c - 1) == '\"'))
         {
         while (++c < end)
            {
            U_INTERNAL_PRINT("c = %c", *c)

            if (*c == '\"') break;

            if (*c == '\\' && (*++c == ' ')) continue;

            if (*c <= ' ' || *c >= 127) return false;
            }

         if (c++ >= end) return false;

         U_INTERNAL_PRINT("c = %c", *c)

         if (*c == '@') break;

         if (*c != '.') return false;

         continue;
         }

      if (*c == '@') break;

      if (*c <= ' ' ||
          *c >= 127) return false;

      if (strchr(RFC822_SPECIALS, *c)) return false;
      }

   if (c == address || *(c - 1) == '.') return false;

   /* next we validate the domain portion (name@domain) */

   if ((domain = ++c) >= end) return false;

   count = 0;

   do {
      U_INTERNAL_PRINT("c = %c", *c)

      if (*c == '.')
         {
         if (c == domain || *(c - 1) == '.') return false;

         ++count;
         }

      if (*c <= ' ' ||
          *c >= 127) return false;

      if (strchr(RFC822_SPECIALS, *c)) return false;
      }
   while (++c < end);

   return (count >= 1);
}

/* Perform 'natural order' comparisons of strings */

__pure int u_strnatcmp(char const* restrict a, char const* restrict b)
{
   char ca, cb;
   int ai = 0, bi = 0;

   U_INTERNAL_TRACE("u_strnatcmp(%s,%s)", a, b)

   U_INTERNAL_ASSERT_POINTER(a)
   U_INTERNAL_ASSERT_POINTER(b)

   while (true)
      {
      ca = a[ai];
      cb = b[bi];

      /* skip over leading spaces or zeros */

      while (u__isspace(ca) || ca == '0') ca = a[++ai];
      while (u__isspace(cb) || cb == '0') cb = b[++bi];

      /* process run of digits */

      if (u__isdigit(ca) &&
          u__isdigit(cb))
         {
         int bias = 0;

         /**
          * The longest run of digits (stripping off leading zeros) wins. That aside, the greatest value
          * wins, but we can't know that it will until we've scanned both numbers to know that they have the
          * same magnitude, so we remember it in BIAS
          */

         while (true)
            {
            if (!u__isdigit(ca) &&
                !u__isdigit(cb))
               {
               goto done_number;
               }

            else if (!u__isdigit(ca)) return -1;
            else if (!u__isdigit(cb)) return  1;
            else if (ca < cb)
               {
               if (!bias) bias = -1;
               }
            else if (ca > cb)
               {
               if (!bias) bias = 1;
               }
            else if (!ca &&
                     !cb)
               {
               return bias;
               }

            ca = a[++ai];
            cb = b[++bi];
            }

done_number:
         if (bias) return bias;
         }

      if (!ca &&
          !cb)
         {
         /* The strings compare the same. Perhaps the caller will want to call strcmp to break the tie */

         return 0;
         }

      /*
      if (fold_case)
         {
         ca = u__toupper(ca);
         cb = u__toupper(cb);
         }
      */

      if      (ca < cb) return -1;
      else if (ca > cb) return  1;

      ++ai;
      ++bi;
      }
}

#ifdef _MSWINDOWS_
#  define PATH_LIST_SEP ';'
#else
#  define PATH_LIST_SEP ':'
#endif

/**
 * Given a string containing units of information separated by colons, return the next one pointed to by (p_index),
 * or NULL if there are no more. Advance (p_index) to the character after the colon
 */

static inline char* extract_colon_unit(char* restrict pzDir, const char* restrict string, uint32_t string_len, uint32_t* restrict p_index)
{
         char* restrict pzDest = pzDir;
   const char* restrict pzSrc  = string + *p_index;

   U_INTERNAL_TRACE("extract_colon_unit(%s,%.*s,%u,%p)", pzDir, string_len, string, string_len, p_index)

   if ((string == 0) ||
       (*p_index >= string_len))
      {
      return 0;
      }

   while (*pzSrc == PATH_LIST_SEP) pzSrc++;

   while (true)
      {
      char ch = (*pzDest = *pzSrc);

      if (ch == '\0') break;

      if (ch == PATH_LIST_SEP)
         {
         *pzDest = '\0';

         break;
         }

      pzDest++;
      pzSrc++;
      }

   if (*pzDir == '\0') return 0;

   *p_index = (pzSrc - string);

   return pzDir;
}

/* Turn STRING (a pathname) into an absolute pathname, assuming that DOT_PATH contains the symbolic location of '.' */

static inline void make_absolute(char* restrict result, const char* restrict dot_path, const char* restrict string)
{
   int result_len;

   U_INTERNAL_TRACE("make_absolute(%p,%s,%s)", result, dot_path, string)

   U_INTERNAL_ASSERT_POINTER(dot_path)

   if (dot_path[0])
      {
      u__strcpy(result, dot_path);

      result_len = u__strlen(result, __PRETTY_FUNCTION__);

      if (result[result_len - 1] != PATH_SEPARATOR)
         {
         result[result_len++] = PATH_SEPARATOR;
         result[result_len]   = '\0';
         }
      }
   else
      {
      result[0] = '.';
      result[1] = PATH_SEPARATOR;
      result[2] = '\0';

      result_len = 2;
      }

   u__strcpy(result + result_len, string);
}

/**
 * find a FILE MODE along PATH
 *
 * pathfind looks for a a file with name FILENAME and MODE access along colon
 * delimited PATH, and build the full pathname as a string, or NULL if not found
 */

#ifdef _MSWINDOWS_
#  define U_PATH_DEFAULT "C:\\msys\\1.0\\bin;C:\\MinGW\\bin;C:\\windows;C:\\windows\\system;C:\\windows\\system32"

static const char* u_check_for_suffix_exe(const char* restrict program)
{
   static char program_w32[MAX_FILENAME_LEN + 1];

   int len = u__strlen(program, __PRETTY_FUNCTION__);

   U_INTERNAL_TRACE("u_check_for_suffix_exe(%s)", program)

   if (u_endsWith(program, len, U_CONSTANT_TO_PARAM(".exe")) == false)
      {
      u__memcpy(program_w32, program, len, __PRETTY_FUNCTION__);

      u_put_unalignedp32(program_w32+len, U_MULTICHAR_CONSTANT32('.','e','x','e'));

      program = program_w32;

      U_INTERNAL_PRINT("program = %s", program)
      }

   return program;
}
#else
#  define U_PATH_DEFAULT "/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin"
#endif

bool u_pathfind(char* restrict result, const char* restrict path, uint32_t path_len, const char* restrict filename, int mode)
{
   uint32_t p_index = 0;
   char zPath[PATH_MAX + 1];

   U_INTERNAL_TRACE("u_pathfind(%p,%.*s,%u,%s,%d)", result, path_len, path, path_len, filename, mode)

   if (path_len == 0)
      {
      path = getenv("PATH");

      if (path) path_len = u__strlen(path, __PRETTY_FUNCTION__);
      else
         {
         path     = U_PATH_DEFAULT;
         path_len = U_CONSTANT_SIZE(U_PATH_DEFAULT);
         }

      U_INTERNAL_PRINT("path(%u) = %.*s", path_len, path_len, path)
      }

#ifdef _MSWINDOWS_
   if (mode & X_OK) filename = u_check_for_suffix_exe(filename);
#endif

   /* FOR each non-null entry in the colon-separated path, DO ... */

   zPath[0] = '\0';

   while (true)
      {
      char* restrict colon_unit = extract_colon_unit(zPath, path, path_len, &p_index);

      /* IF no more entries, THEN quit */

      if (colon_unit == 0) break;

      make_absolute(result, colon_unit, filename);

      /* Make sure we can access it in the way we want */

      if (access(result, mode) >= 0)
         {
         /* We can, so normalize the name and return it below */

         (void) u_canonicalize_pathname(result);

         return true;
         }
      }

   return false;
}

/**
 * Canonicalize path, and build a new path. The new path differs from original in that:
 *
 * Multiple    '/'                     are collapsed to a single '/'
 * Trailing    '/'                     are removed
 * Leading     './'  and trailing '/.' are removed
 * Non-leading '../' and trailing '..' are handled by removing portions of the path
 */

bool u_canonicalize_pathname(char* restrict path)
{
   int len;
   char* restrict p;
   char* restrict s;
   char* restrict src;
   char* restrict dst;
   bool is_modified = false;
   char* restrict lpath = path;

   U_INTERNAL_TRACE("u_canonicalize_pathname(%s)", path)

#ifdef _MSWINDOWS_
   if (u__isalpha(path[0]) &&
                  path[1] == ':')
      {
      lpath += 2; /* Skip over the disk name in MSDOS pathnames */
      }
#endif

   /* Collapse multiple slashes */

   for (p = lpath; *p; ++p)
      {
      if (u_get_unalignedp16(p) == U_MULTICHAR_CONSTANT16('/','/'))
         {
         s = p+1;

         while (*(++s) == '/') {}

         is_modified = true;

         for (src = s, dst = p+1; (*dst = *src); ++src, ++dst) {} /* u__strcpy(p + 1, s); */

         U_INTERNAL_PRINT("path = %s", path)
         }
      }

   /* Collapse "/./" -> "/" */

   p = lpath;

   while (*p)
      {
      if (p[0] == '/' &&
          u_get_unalignedp16(p+1) == U_MULTICHAR_CONSTANT16('.','/'))
         {
         is_modified = true;

         for (src = p+2, dst = p; (*dst = *src); ++src, ++dst) {} /* u__strcpy(p, p + 2); */

         U_INTERNAL_PRINT("path = %s", path)
         }
      else
         {
         ++p;
         }
      }

   /* Remove trailing slashes */

   p = lpath + u__strlen(lpath, __PRETTY_FUNCTION__) - 1;

   if ( p > lpath &&
       *p == '/')
      {
      is_modified = true;

      do { *p-- = '\0'; } while (p > lpath && *p == '/');
      }

   /* Remove leading "./" */

   if (u_get_unalignedp16(lpath) == U_MULTICHAR_CONSTANT16('.','/'))
      {
      if (lpath[2] == 0)
         {
         lpath[1] = 0;

         return true;
         }

      is_modified = true;

      for (src = lpath+2, dst = lpath; (*dst = *src); ++src, ++dst) {} /* u__strcpy(lpath, lpath + 2); */

      U_INTERNAL_PRINT("path = %s", path)
      }

   /* Remove trailing "/" or "/." */

   len = u__strlen(lpath, __PRETTY_FUNCTION__);

   if (len < 2) goto end;

   if (lpath[len-1] == '/')
      {
      lpath[len-1] = 0;

      is_modified = true;
      }
   else
      {
      if (u_get_unalignedp16(lpath+len-2) == U_MULTICHAR_CONSTANT16('/','.'))
         {
         if (len == 2)
            {
            lpath[1] = 0;

            return true;
            }

         is_modified = true;

         lpath[len-2] = 0;
         }
      }

   /* Collapse "/.." with the previous part of path */

   p = lpath;

   while (p[0] &&
          p[1] &&
          p[2])
      {
      if ((p[0] != '/'  ||
           p[1] != '.'  ||
           p[2] != '.') ||
          (p[3] != '/'  &&
           p[3] != 0))
         {
         ++p;

         continue;
         }

      /* search for the previous token */

      s = p-1;

      while (s >= lpath && *s != '/') --s;

      ++s;

      /* If the previous token is "..", we cannot collapse it */

      if (u_get_unalignedp16(s) == U_MULTICHAR_CONSTANT16('.','.') &&
          (s + 2) == p)
         {
         p += 3;

         continue;
         }

      if (p[3] != '\0')
         {
         /*      "/../foo" -> "/foo" */
         /* "token/../foo" ->  "foo" */

         is_modified = true;

         for (src = p+4, dst = s + (s == lpath && *s == '/'); (*dst = *src); ++src, ++dst) {} /* u__strcpy(s + (s == lpath && *s == '/'), p + 4); */

         U_INTERNAL_PRINT("path = %s", path)

         p = s - (s > lpath);

         continue;
         }

      /* trailing ".." */

      is_modified = true;

      if (s == lpath)
         {
         /* "token/.." -> "." */

         if (lpath[0] != '/') lpath[0] = '.';

         lpath[1] = 0;
         }
      else
         {
         /* "foo/token/.." -> "foo" */

         if (s == (lpath + 1)) s[ 0] = '\0';
         else                  s[-1] = '\0';
         }

      break;
      }

end:
   U_INTERNAL_PRINT("path = %s", path)

   return is_modified;
}

/* Prepare command for call to exec() */

int u_splitCommand(char* restrict s, uint32_t n, char** restrict argv, char* restrict pathbuf, uint32_t pathbuf_size)
{
   char c;
   uint32_t i = 0;
   bool bpath = false;
   int result = u_split(s, n, argv+1, 0);

   U_INTERNAL_TRACE("u_splitCommand(%.*s,%u,%p,%p,%u)", U_min(n,128), s, n, argv, pathbuf, pathbuf_size)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_MAJOR(n,0)
   U_INTERNAL_ASSERT_MAJOR(pathbuf_size,0)

   /* check if command have path separator */

   while ((c = argv[1][i++]))
      {
      if (IS_DIR_SEPARATOR(c))
         {
         bpath = true;

         break;
         }
      }

   if (bpath)
      {
      argv[0] = argv[1];
      argv[1] = (char* restrict) u_basename(argv[0]);

      pathbuf[0] = '\0';
      }
   else
      {
      argv[0] = pathbuf;

#  ifdef _MSWINDOWS_
      argv[1] = (char* restrict) u_check_for_suffix_exe(argv[1]);
#  endif

      if (u_pathfind(pathbuf, 0, 0, argv[1], R_OK | X_OK) == false) return -1;

      U_INTERNAL_ASSERT_MINOR(u__strlen(pathbuf, __PRETTY_FUNCTION__), pathbuf_size)
      }

   return result;
}

/**
 * It uses George Marsaglia's MWC algorithm to produce an unsigned integer.
 *
 * see http://www.bobwheeler.com/statistics/Password/MarsagliaPost.txt
 */

uint32_t u_get_num_random(uint32_t range)
{
   uint32_t result;

   U_INTERNAL_TRACE("u_get_num_random(%u)", range)

   U_INTERNAL_ASSERT_MAJOR(u_m_w, 0)
   U_INTERNAL_ASSERT_MAJOR(u_m_z, 0)

   u_m_z = 36969 * (u_m_z & 65535) + (u_m_z >> 16);
   u_m_w = 18000 * (u_m_w & 65535) + (u_m_w >> 16);

   result = (u_m_z << 16) + u_m_w;

   if (range)
      {
      result = ((result % range) + 1);

      U_INTERNAL_ASSERT(result <= range)
      }

   return result;
}

/* Produce a uniform random sample from the open interval (0, 1). The method will not return either end point */

double u_get_uniform(void)
{
   uint32_t u;

   U_INTERNAL_TRACE("u_get_uniform()")

   /* 0 <= u < 2^32 */

   u = u_get_num_random(0);

   /* The magic number below is 1/(2^32 + 2). The result is strictly between 0 and 1 */

   return (u + 1.0) * 2.328306435454494e-10;
}

/**
 * Function fnmatch() as specified in POSIX 1003.2-1992, section B.6. Compares a filename or pathname to a pattern
 */

static const char* restrict end_p;
static const char* restrict end_s;

static inline int rangematch(const char* restrict pattern, char test, int flags, char** restrict newp)
{
   char c;
   int negate, ok;

   U_INTERNAL_TRACE("rangematch(%.*s,%c)", end_p - pattern, pattern, test)

   /**
    * A bracket expression starting with an unquoted circumflex
    * character produces unspecified results (IEEE 1003.2-1992, 3.13.2).
    * This implementation treats it like '!', for consistency with the
    * regular expression syntax. J.T. Conklin (conklin@ngai.kaleida.com)
    */

   if ((negate = (*pattern == '!' || *pattern == '^')) != 0) ++pattern;

   if (flags & FNM_CASEFOLD) test = u__tolower((unsigned char)test);

   /**
    * A right bracket shall lose its special meaning and represent itself in a bracket expression if it occurs first in the list. -- POSIX.2 2.8.3.2
    */

   ok = 0;
   c  = *pattern++;

   do {
      char c2;

      if (c == '\\' && !(flags & FNM_NOESCAPE)) c = *pattern++;

      U_INTERNAL_PRINT("c = %c test = %c", c, test)

      if (pattern > end_p) return -1; /* if (c == EOS) return (RANGE_ERROR); */

      if (c == '/' && (flags & FNM_PATHNAME)) return 0;

      if (flags & FNM_CASEFOLD) c = u__tolower((unsigned char)c);

      if (      * pattern     == '-' &&
          (c2 = *(pattern+1)) != ']' &&
                 (pattern+1)  != end_p)
         {
         pattern += 2;

         if (c2 == '\\' && !(flags & FNM_NOESCAPE)) c2 = *pattern++;

         if (pattern > end_p) return -1; /* if (c2 == EOS) return (RANGE_ERROR); */

         if (flags & FNM_CASEFOLD) c2 = u__tolower((unsigned char)c2);

         if (c    <= test &&
             test <= c2)
            {
            ok = 1;
            }
         }
      else if (c == test)
         {
         ok = 1;
         }
      }
   while ((c = *pattern++) != ']');

   *newp = (char* restrict) pattern;

   return (ok != negate);
}

__pure static int kfnmatch(const char* restrict pattern, const char* restrict string, int flags, int nesting)
{
   char c, test;
   char* restrict newp;
   const char* restrict stringstart;

   U_INTERNAL_TRACE("kfnmatch(%.*s,%.*s,%d,%d)", end_p - pattern, pattern, end_s - string, string, flags, nesting)

   if (nesting == 20) return 1;

   for (stringstart = string;;)
      {
      c = *pattern++;

      if (pattern > end_p)
         {
         if ((flags & FNM_LEADING_DIR) && *string == '/') return 0;

         return (string != end_s);
         }

      switch (c)
         {
         case '?':
            {
            if (string == end_s) return 1;

            if (*string == '/' && (flags & FNM_PATHNAME)) return 1;

            if (*string == '.' && (flags & FNM_PERIOD) &&
                (string == stringstart || ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
               {
               return 1;
               }

            ++string;
            }
         break;

         case '*':
            {
            c = *pattern;

            /* Collapse multiple stars */

            while (c == '*') c = *++pattern;

            if (*string == '.' && (flags & FNM_PERIOD) &&
                (string == stringstart || ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
               {
               return 1;
               }

            /* Optimize for pattern with * at end or before / */

            if (pattern == end_p) /* if (c == EOS) */
               {
               if (flags & FNM_PATHNAME) return ((flags & FNM_LEADING_DIR) || memchr(string, '/', end_s - string) == 0 ? 0 : 1);

               return 0;
               }

            if (c == '/' && flags & FNM_PATHNAME)
               {
               if ((string = (const char* restrict)memchr(string, '/', end_s - string)) == 0) return 1;

               break;
               }

            /* General case, use recursion */

            while (string < end_s)
               {
               test = *string;

               if (!kfnmatch(pattern, string, flags & ~FNM_PERIOD, nesting + 1)) return 0;

               if (test == '/' && flags & FNM_PATHNAME) break;

               ++string;
               }

            return 1;
            }

         case '[':
            {
            if (string == end_s) return 1;

            if (*string == '/' && (flags & FNM_PATHNAME)) return 1;

            if (*string == '.' && (flags & FNM_PERIOD) &&
                (string == stringstart || ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
               {
               return 1;
               }

            switch (rangematch(pattern, (char)*string,  flags, (char** restrict)&newp))
               {
               case -1: goto norm;
               case  1: pattern = newp; break;
               case  0: return 1;
               }

            ++string;
            }
         break;

         case '\\':
            {
            if (!(flags & FNM_NOESCAPE))
               {
               c = *pattern++;

               if (pattern > end_p) /* if ((c = *pattern++) == EOS) */
                  {
                  c = '\\';

                  --pattern;
                  }
               }
            }

         /* FALLTHROUGH */

         default:
            {
norm:       if (c == *string)
               {
               }
            else if ((flags & FNM_CASEFOLD) && (u__tolower((unsigned char)c) == u__tolower((unsigned char)*string)))
               {
               }
            else
               {
               return 1;
               }

            string++;
            }
         break;
         }
      }

   /* NOTREACHED */
}

#define __FNM_FLAGS (FNM_PATHNAME | FNM_NOESCAPE | FNM_PERIOD | FNM_LEADING_DIR | FNM_CASEFOLD | FNM_INVERT)

bool u_fnmatch(const char* restrict string, uint32_t n1, const char* restrict pattern, uint32_t n2, int flags)
{
   int result;

   U_INTERNAL_TRACE("u_fnmatch(%.*s,%u,%.*s,%u,%d)", U_min(n1,128), string, n1, n2, pattern, n2, flags)

   U_INTERNAL_ASSERT_MAJOR(n1, 0)
   U_INTERNAL_ASSERT_MAJOR(n2, 0)
   U_INTERNAL_ASSERT_POINTER(string)
   U_INTERNAL_ASSERT_POINTER(pattern)
   U_INTERNAL_ASSERT_EQUALS(flags & ~__FNM_FLAGS, 0)

   end_s = string  + n1;
   end_p = pattern + n2;

   result = kfnmatch(pattern, string, flags, 0);

   return ((flags & FNM_INVERT) != 0 ? (result != 0) : (result == 0));
}

/* buffer type identification - Assumed an ISO-1 character set */

#define U_LOOP_STRING( exec_code ) unsigned char c = *s; while (n--) { exec_code ; c = *(++s); }

/**
 * #if !defined(GCOV)
 *
 * (Duff's device) This is INCREDIBLY ugly, but fast. We break the string up into 8 byte units. On the first
 * time through the loop we get the "leftover bytes" (strlen % 8). On every other iteration, we perform 8 BODY's
 * so we handle all 8 bytes. Essentially, this saves us 7 cmp & branch instructions. If this routine is heavily
 * used enough, it's worth the ugly coding
 *
 * #undef  U_LOOP_STRING
 * #define U_LOOP_STRING( exec_code ) { \
 * unsigned char c; \
 * uint32_t U_LOOP_CNT = (n + 8 - 1) >> 3; \
 * switch (n & (8 - 1)) { \
 * case 0: \
 * do {    { c = *s++; exec_code; } \
 * case 7: { c = *s++; exec_code; } \
 * case 6: { c = *s++; exec_code; } \
 * case 5: { c = *s++; exec_code; } \
 * case 4: { c = *s++; exec_code; } \
 * case 3: { c = *s++; exec_code; } \
 * case 2: { c = *s++; exec_code; } \
 * case 1: { c = *s++; exec_code; } \
 * } while (--U_LOOP_CNT); } }
 *
 * #endif
 */

__pure bool u_isName(const char* restrict s, uint32_t n)
{
   U_LOOP_STRING( if (u__isname(c) == false) return false )

   U_INTERNAL_TRACE("u_isName(%.*s,%u)", U_min(n,128), s, n)

   return true;
}

__pure bool u_isDigit(const char* restrict s, uint32_t n)
{
   U_LOOP_STRING( if (u__isdigit(c) == false) return false )

   U_INTERNAL_TRACE("u_isDigit(%.*s,%u)", U_min(n,128), s, n)

   return true;
}

__pure bool u_isWhiteSpace(const char* restrict s, uint32_t n)
{
   U_LOOP_STRING( if (u__isspace(c) == false) return false )

   U_INTERNAL_TRACE("u_isWhiteSpace(%.*s,%u)", U_min(n,128), s, n)

   return true;
}

__pure bool u_isText(const unsigned char* restrict s, uint32_t n)
{
   U_LOOP_STRING( if (u__istext(c) == false) return false )

   U_INTERNAL_TRACE("u_isText(%.*s,%u)", U_min(n,128), s, n)

   return true;
}

__pure bool u_isBase64(const char* restrict s, uint32_t n)
{
   /* u__isalnum(c) || (c == '+') || (c == '/') || (c == '=') */

   U_LOOP_STRING( if (u__isbase64(c) == false) return false )

   U_INTERNAL_TRACE("u_isBase64(%.*s,%u)", U_min(n,128), s, n)

   return true;
}

__pure bool u_isBase64Url(const char* restrict s, uint32_t n)
{
   /* u__isalnum(c) || (c == '+') || (c == '/') || (c == '=') */

   U_LOOP_STRING( if (u__isb64url(c) == false) return false )

   U_INTERNAL_TRACE("u_isBase64Url(%.*s,%u)", U_min(n,128), s, n)

   return true;
}

__pure bool u_isPrintable(const char* restrict s, uint32_t n, bool bline)
{
   U_LOOP_STRING( if (u__isprint(c) == false && (bline == false || u__islterm(c) == false)) return false )

   U_INTERNAL_TRACE("u_isPrintable(%.*s,%u,%d)", U_min(n,128), s, n, bline)

   return true;
}

/* The NT file naming convention specifies that all characters greater than ASCII 31 to be used except for the following: "/:<>*?\| */

__pure bool u_isFileName(const char* restrict s, uint32_t n)
{
   U_LOOP_STRING( if (u__isvalidchar(c) == false || u__isfnameinvalid(c)) return false )

   U_INTERNAL_TRACE("u_isFileName(%.*s,%u)", U_min(n,128), s, n)

   return true;
}

__pure bool u_isUrlEncoded(const char* restrict s, uint32_t n, bool bquery)
{
   bool benc = false;
   unsigned char c = *s;

   U_INTERNAL_TRACE("u_isUrlEncoded(%.*s,%u,%d)", U_min(n,128), s, n, bquery)

   U_INTERNAL_ASSERT_MAJOR(n, 0)

   while (n--)
      {
      U_INTERNAL_PRINT("c = %c n = %u", c, n)

      if (bquery         == false ||
          u__isurlqry(c) == false) /* URL: char FROM query '&' (38 0x26) | '=' (61 0x3D) | '#' (35 0x23) */
         {
         if (c == '%')
            {
            if (n >= 2)
               {
               if (u__isxdigit(s[1]) &&
                   u__isxdigit(s[2]))
                  {
                  benc = true;
                  }

               s += 2;
               n -= 2;
               }
            }
         else if (c == '+') benc = true;
         else if (u__is2urlenc(c)) return false;
         }

      c = *(++s);
      }

   return benc;
}

__pure bool u_isUrlEncodeNeeded(const char* restrict s, uint32_t n)
{
   unsigned char c = *s;

   U_INTERNAL_TRACE("u_isUrlEncodeNeeded(%.*s,%u)", U_min(n,128), s, n)

   while (n--)
      {
      if (u__is2urlenc(c)             &&
          (c != '%'                   ||
           u__isxdigit(s[1]) == false ||
           u__isxdigit(s[2]) == false))
         {
         return true;
         }

      c = *(++s);
      }

   return false;
}

__pure bool u_isIPv4Addr(const char* restrict s, uint32_t n)
{
   U_INTERNAL_TRACE("u_isIPv4Addr(%.*s,%u)", U_min(n,128), s, n)

   /* u__isdigit(c) || (c == '.') */

   if (n >= U_CONSTANT_SIZE("8.8.8.8")         &&
       n <= U_CONSTANT_SIZE("255.255.255.255") &&
       u__isdigit(s[0])                        &&
       u__isdigit(s[--n]))
      {
      uint32_t count = 0;

      while (n--)
         {
         unsigned char c = *(++s);

         U_INTERNAL_PRINT("c = %c n = %u count = %u", c, n, count)

         if (c == '.')
            {
            c = *(++s);

            --n;
            ++count;
            }

         if (u__isdigit(c) == false) return false;
         }

      if (count == 3) return true;
      }

   return false;
}

__pure bool u_isIPv6Addr(const char* restrict s, uint32_t n)
{
   /* u__isxdigit(c) || (c == '.') || (c == ':') */

   uint32_t count1 = 0,
            count2 = 0;
   unsigned char c = *s;

   U_INTERNAL_TRACE("u_isIPv6Addr(%.*s,%u)", U_min(n,128), s, n)

   while (n--)
      {
           if (c == '.') ++count1;
      else if (c == ':') ++count2;
      else if (u__isxdigit(c) == false) return false;

      c = *(++s);
      }

   if (count1 != 3 &&
       count2 <  5)
      {
      return false;
      }

   return true;
}

__pure bool u_isHostName(const char* restrict ptr, uint32_t len)
{
   int ch;
   const char* restrict end = ptr + len;

   U_INTERNAL_TRACE("u_isHostName(%.*s,%u)", U_min(len,128), ptr, len)

   U_INTERNAL_ASSERT_POINTER(ptr)

   if (ptr[ 0] == '[' &&
       end[-1] == ']')
      {
      if (u_isIPv6Addr(ptr+1, len-1)) return true;

      return false;
      }

   ch = *(unsigned char*)ptr;

   /**
    * Host names may contain only alphanumeric characters, minus signs ("-"), and periods (".").
    * They must begin with an alphabetic character and end with an alphanumeric character
    *
    * Several well known Internet and technology companies have DNS records that use the underscore:
    *
    * see http://domainkeys.sourceforge.net/underscore.html
    */

   if (u__isalpha(ch))
      {
      if (u__isalnum(end[-1]) == false) return false;

      while (++ptr < end)
         {
         ch = *(unsigned char*)ptr;

         if (u__ishname(ch) == false) return false;
         }

      return true;
      }

   while (ptr < end)
      {
      ch = *(unsigned char*)ptr;

      if (u__isipv4(ch) == false &&
          u__isipv6(ch) == false)
         {
         return false;
         }

      ++ptr;
      }

   return true;
}

__pure const char* u_isUrlScheme(const char* restrict url, uint32_t len)
{
   const char* restrict ptr;
   const char* restrict first_slash;

   U_INTERNAL_TRACE("u_isUrlScheme(%.*s,%u)", U_min(len,128), url, len)

   U_INTERNAL_ASSERT_POINTER(url)

   first_slash = (const char* restrict) memchr(url, '/', len);

   if (first_slash     == 0   ||
       first_slash     == url ||     /* Input with no slash at all or slash first can't be URL */
       first_slash[-1] != ':' ||
       first_slash[ 1] != '/' ||     /* Character before must be : and next must be / */
       first_slash     == (url + 1)) /* There must be something before the :// */
      {
      return 0;
      }

   /* Check all characters up to first slash - 1. Only alphanum is allowed */

   ptr = url;

   --first_slash;

   U_INTERNAL_ASSERT_EQUALS(first_slash[0], ':')

   while (ptr < first_slash)
      {
      int ch = *(unsigned char*)ptr;

      /**
       * The set of valid URL schemes, as per STD66 (RFC3986) is '[A-Za-z][A-Za-z0-9+.-]*'.
       * But use sightly looser check of '[A-Za-z0-9][A-Za-z0-9+.-]*' because earlier version
       * of check used '[A-Za-z0-9]+' so not to break any remote helpers
       */

      if (u__isalnum(ch) == false &&
          (ptr != url && u__ispecial(ch)) == false)
         {
         return 0;
         }

      ++ptr;
      }

   U_INTERNAL_ASSERT_EQUALS(ptr[1], '/')
   U_INTERNAL_ASSERT_EQUALS(ptr[2], '/')

   return ptr+3;
}

__pure bool u_isURL(const char* restrict url, uint32_t len)
{
   const char* restrict ptr;

   U_INTERNAL_TRACE("u_isURL(%.*s,%u)", U_min(len,128), url, len)

   /* proto://hostname[:port]/[path]?[query] */

   ptr = (const char* restrict) u_isUrlScheme(url, len);

   if (ptr)
      {
      int ch;
      const char* restrict end;
      const char* restrict tmp;

      len -= (ptr - url);

      tmp  = ptr;
      end  = ptr + len;

      while (tmp < end)
         {
         ch = *(unsigned char*)tmp;

         if (ch == '/' ||
             ch == '?')
            {
            len = (end = tmp) - ptr;

            break;
            }

         ++tmp;
         }

      U_INTERNAL_PRINT("ptr = %.*s", U_min(len,128), ptr)

      tmp = (const char* restrict) memrchr(ptr, ':', len);

      if (        tmp &&
          ((end - tmp) <= 5)) /* NB: port number: 0-65536 */
         {
         len = tmp - ptr;

         while (++tmp < end)
            {
            ch = *(unsigned char*)tmp;

            U_INTERNAL_PRINT("ch = %c", ch)

            if (u__isdigit(ch) == 0) return false;
            }
         }

      if (u_isHostName(ptr, len)) return true;
      }

   return false;
}

__pure bool u_isHTML(const char* restrict ptr)
{
   /* NB: we check for <(h(1-6|tml)|!DOCTYPE) */

   U_INTERNAL_TRACE("u_isHTML(%.*s)", 12, ptr)

   switch (u_get_unalignedp32(ptr))
      {
      case U_MULTICHAR_CONSTANT32('<','h','1','>'):
      case U_MULTICHAR_CONSTANT32('<','H','1','>'):
      case U_MULTICHAR_CONSTANT32('<','h','2','>'):
      case U_MULTICHAR_CONSTANT32('<','H','2','>'):
      case U_MULTICHAR_CONSTANT32('<','h','3','>'):
      case U_MULTICHAR_CONSTANT32('<','H','3','>'):
      case U_MULTICHAR_CONSTANT32('<','h','4','>'):
      case U_MULTICHAR_CONSTANT32('<','H','4','>'):
      case U_MULTICHAR_CONSTANT32('<','h','5','>'):
      case U_MULTICHAR_CONSTANT32('<','H','5','>'):
      case U_MULTICHAR_CONSTANT32('<','h','6','>'):
      case U_MULTICHAR_CONSTANT32('<','H','6','>'):
      case U_MULTICHAR_CONSTANT32('<','h','t','m'):
      case U_MULTICHAR_CONSTANT32('<','H','T','M'):
      case U_MULTICHAR_CONSTANT32('<','!','d','o'):
      case U_MULTICHAR_CONSTANT32('<','!','D','O'): return true;
      default:                                      return false;
      }
}

__pure bool u_isMacAddr(const char* restrict p, uint32_t len)
{
   uint32_t c;

   U_INTERNAL_TRACE("u_isMacAddr(%.*s,%u)", U_min(len,128), p, len)

   /* windows-style: 01-23-45-67-89-ab, 01:23:45:67:89:ab */

   if (len == (6 * 3) - 1)
      {
      for (c = 0; c < len; ++c)
         {
         if ((c % 3) == 2)
            {
            if (p[c] != ':' &&
                p[c] != '-') return false;
            }
         else
            {
            if (!u__isxdigit(p[c])) return false;
            }
         }

      return true;
      }

   /* cisco-style: 0123.4567.89ab */

   if (len == (3 * 5) - 1)
      {
      for (c = 0; c < len; ++c)
         {
         if ((c % 5) == 4)
            {
            if (p[c] != '.') return false;
            }
         else
            {
            if (!u__isxdigit(p[c])) return false;
            }
         }

      return true;
      }

   return false;
}

/************************************************************************
 * From rfc2044: encoding of the Unicode values on UTF-8:               *
 *                                                                      *
 * UCS-4 range (hex.)    UTF-8 octet sequence (binary)                  *
 * 0000 0000-0000 007F   0xxxxxxx                                       *
 * 0000 0080-0000 07FF   110xxxxx 10xxxxxx                              *
 * 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx                     *
 ************************************************************************/

const unsigned char u_validate_utf8[] = {
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 00..1f */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 20..3f */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 40..5f */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 60..7f */
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, /* 80..9f */
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, /* a0..bf */
   8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* c0..df */
   0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, /* e0..ef */
   0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, /* f0..ff */
   0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, /* s0..s0 */
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, /* s1..s2 */
   1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, /* s3..s4 */
   1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, /* s5..s6 */
   1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* s7..s8 */
};

__pure bool u_isUTF8(const unsigned char* restrict buf, uint32_t len)
{
   /*
   bool result = false;
   uint32_t j, following;
   */
   uint32_t code = 0, state = 0;
   const unsigned char* restrict end = buf + len;

   U_INTERNAL_TRACE("u_isUTF8(%.*s,%u)", U_min(len,128), buf, len)

   U_INTERNAL_ASSERT_POINTER(buf)
   U_INTERNAL_ASSERT_MAJOR(len, 0)

   while (buf < end)
      {
      /**
       * UTF is a string of 1, 2, 3 or 4 bytes. The valid strings are as follows (in "bit format"):
       *
       *    0xxxxxxx                             valid 1-byte
       *    110xxxxx 10xxxxxx                    valid 2-byte
       *    1110xxxx 10xxxxxx 10xxxxxx           valid 3-byte
       *    11110xxx 10xxxxxx 10xxxxxx 10xxxxxx  valid 4-byte
       *    ........
       */

      unsigned char c = *buf++;

      /**
       * Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
       *
       * see http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details
       */

      uint32_t type = u_validate_utf8[c];

      code = (state != 0 ? (c & 0x3fu)    | (code << 6)
                         : (0xff >> type) & (uint32_t)(c));

      state = u_validate_utf8[256 + state * 16 + type];

      U_INTERNAL_PRINT("c = %u type = %u code = %u state = %u", c, type, code, state)

      if (state == 1) return false;

      /**
       * if ((c & 0x80) == 0) // 0xxxxxxx is plain ASCII
       *    {
       *    // Even if the whole file is valid UTF-8 sequences, still reject it if it uses weird control characters
       *
       *    if ((u_cttab(c) & 0x0200) == 0) return false;
       *    }
       * else if ((c & 0x40) == 0) return false; // 10xxxxxx never 1st byte
       * else
       *    {
       *    // 11xxxxxx begins UTF-8
       *   
       *    if      ((c & 0x20) == 0) following = 1; // 110xxxxx
       *    else if ((c & 0x10) == 0) following = 2; // 1110xxxx
       *    else if ((c & 0x08) == 0) following = 3; // 11110xxx
       *    else if ((c & 0x04) == 0) following = 4; // 111110xx
       *    else if ((c & 0x02) == 0) following = 5; // 1111110x
       *    else                      return false;
       *   
       *    for (j = 0; j < following; j++)
       *       {
       *       if (buf >= end) return result;
       *   
       *       c = *buf++;
       *   
       *       if ((c & 0x80) == 0 ||
       *           (c & 0x40))
       *          {
       *          return false;
       *          }
       *       }
       *   
       *    result = true;
       *    }
       *
       * return result;
       */
      }

   return (state == 0);
}

__pure int u_isUTF16(const unsigned char* restrict buf, uint32_t len)
{
   uint32_t be, i;

   U_INTERNAL_TRACE("u_isUTF16(%.*s,%u)", U_min(len,128), buf, len)

   if (len < 2) return 0;

   if      (u_get_unalignedp16(buf) == U_MULTICHAR_CONSTANT16(0xff,0xfe)) be = 0;
   else if (u_get_unalignedp16(buf) == U_MULTICHAR_CONSTANT16(0xfe,0xff)) be = 1;
   else
      {
      return 0;
      }

   for (i = 2; i + 1 < len; i += 2)
      {
      uint32_t c = (be ? buf[i+1] + 256 * buf[i]
                       : buf[i]   + 256 * buf[i+1]);

      if ( c == 0xfffe ||
          (c  < 128 && ((u_cttab(c) & 0x0200) == 0)))
         {
         return 0;
         }
      }

   return (1 + be);
}

/**
 * From RFC 3986
 *
 * #define U_URI_UNRESERVED  0 // ALPHA (%41-%5A and %61-%7A) DIGIT (%30-%39) '-' '.' '_' '~'
 * #define U_URI_PCT_ENCODED 1
 * #define U_URI_GEN_DELIMS  2 // ':' '/' '?' '#' '[' ']' '@'
 * #define U_URI_SUB_DELIMS  4 // '!' '$' '&' '\'' '(' ')' '*' '+' ',' ';' '='
 *
 * unsigned int u_uri_encoded_char_mask = (U_URI_PCT_ENCODED | U_URI_GEN_DELIMS | U_URI_SUB_DELIMS);
 *
 * const unsigned char u_uri_encoded_char[256] = {
 *   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x00 - 0x0f
 *   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0x10 - 0x1f
 *   1, 4, 1, 2, 4, 1, 4, 4, 4, 4, 4, 4, 4, 0, 0, 2,  //  !"#$%&'()*+,-./
 *   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 1, 4, 1, 2,  // 0123456789:;<=>?
 *   2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // @ABCDEFGHIJKLMNO
 *   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 1, 0,  // PQRSTUVWXYZ[\]^_
 *   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // `abcdefghijklmno
 *   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1,  // pqrstuvwxyz{|}~
 *   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 *   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 *   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 *   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 *   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 *   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 *   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 *   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
 * };
 */

#define U__S  0x00000001 /* character space    ' ' (32 0x20) */
#define U__E  0x00000002 /* character used in printf format  */
#define U__H  0x00000004 /* character         '+' (43 0x2B) */
#define U__V  0x00000008 /* character         ',' (44 0x2C) */
#define U__O  0x00000010 /* character minus   '-' (45 0x2D) */
#define U__N  0x00000020 /* character point   '.' (46 0x2E) */
#define U__G  0x00000040 /* character         ':' (58 0x3A) */
#define U__Q  0x00000080 /* character underbar '_' (95 0x5F) */
#define U__B  0x00000100 /* character tab      \t  (09 0x09) */
#define U__R  0x00000200 /* carriage return or new line (a \r or \n) */
#define U__W  0x00000400 /* WhiteSpace */
#define U__C  0x00000800 /* Control character */
#define U__D  0x00001000 /* Digit */
#define U__L  0x00002000 /* Lowercase */
#define U__I  0x00004000 /* Punctuation */
#define U__U  0x00008000 /* Uppercase */
#define U__Z  0x00010000 /* Octal */
#define U__F  0x00020000 /* character never appears in plain ASCII text */
#define U__T  0x00040000 /* character      appears in plain ASCII text */
#define U__X  0x00080000 /* Hexadecimal */
#define U__A  0x00100000 /* BASE64 encoded: '+' (43 0x2B) | '/' (47 0x2F) | '=' (61 0x3D) */
#define U__M  0x00200000 /* HTTP request/response (COPY, DELETE, GET, HEAD|HTTP, OPTIONS, POST/PUT/PATCH) */
#define U__Y  0x00400000 /* HTTP header (Accept...,Host,Range,Cookie,Referer,X-Real-IP,User-Agent,Connection,Content-...) */
#define U__K  0x00800000 /* string quote:    '"' (34 0x22) | ''' (39 0x27) */
#define U__J  0x01000000 /* HTML special:    '&' (38 0x26) | '<' (60 0x3C) | '>' (62 0x3E) */
#define U__UE 0x02000000 /* TO   URL encode: ' ' (32 0x20) | ... */
#define U__UQ 0x04000000 /* FROM URL query:  '&' (38 0x26) | '=' (61 0x3D) | '#' (35 0x23) */
#define U__UF 0x08000000 /* filename invalid char: '"' '*' ':' '<' '>' '?' '\' '|' */
#define U__XM 0x10000000 /* char >= (32 0x20) */
#define U__XE 0x20000000 /* char '}' | ']' */

#define LU    (U__L | U__U)
#define LX    (U__L | U__X)
#define UX    (U__U | U__X)
#define ITK   (U__I | U__T | U__K | U__UF |               U__XM)
#define LT    (U__L | U__T |                              U__XM)
#define UT    (U__U | U__T |                              U__XM)
#define ITF   (U__I | U__T |                              U__XM) 
#define ITA   (U__I | U__T | U__A |                       U__XM)
#define ITQ   (U__I | U__T | U__Q |                       U__XM)
#define LTE   (U__L | U__T |               U__E |         U__XM)
#define LTY   (U__L | U__T | U__Y |        U__E |         U__XM)
#define UXT   (U__U | U__X | U__T |        U__E |         U__XM)
#define DT    (U__D | U__T |               U__E |         U__XM)
#define ITN   (U__I | U__T | U__N |        U__E |         U__XM)
#define ITO   (U__I | U__T | U__O |        U__E |         U__XM)
#define DTZ   (U__D | U__T | U__Z |        U__E |         U__XM)
#define UTE   (U__U | U__T |               U__E |         U__XM)
#define UTY   (U__U | U__T | U__Y |        U__E |         U__XM)
#define UTM   (U__U | U__T | U__M |        U__E |         U__XM)
#define LTM   (U__L | U__T | U__M |        U__E |         U__XM)
#define LXT   (U__L | U__X | U__T |        U__E |         U__XM)
#define LTMY  (U__L | U__T | U__M | U__Y | U__E |         U__XM)
#define UTMY  (U__U | U__T | U__M | U__Y | U__E |         U__XM)
#define UXTM  (U__U | U__X | U__T | U__M | U__E |         U__XM)
#define UXTY  (U__U | U__X | U__T | U__Y | U__E |         U__XM)
#define LXTM  (U__L | U__X | U__T | U__M | U__E |         U__XM)
#define LXTY  (U__L | U__X | U__T | U__Y | U__E |         U__XM)
#define LXTMY (U__L | U__X | U__T | U__M | U__E | U__Y  | U__XM)
#define UXTMY (U__U | U__X | U__T | U__M | U__E | U__Y  | U__XM)
#define IT    (U__I |        U__T |        U__E | U__UF | U__XM)

#define FUE   (U__F |                       U__UE | U__XM)
#define IF    (U__I | U__F |                U__UE)
#define CF    (U__C | U__F |                U__UE)
#define CT    (U__C | U__T |                U__UE)
#define WF    (U__W | U__F |                U__UE)
#define ITUE  (U__I | U__T |        U__UF | U__UE | U__XM)
#define CWT   (U__C | U__W | U__T |         U__UE)
#define CWF   (U__C | U__W | U__F |         U__UE)
#define ITG   (U__I | U__T | U__G | U__UF | U__UE | U__XM)
#define ITJ   (U__I | U__T | U__J | U__UF |         U__XM)
#define CWBT  (U__C | U__W | U__B | U__T  | U__UE)
#define CWRT  (U__C | U__W | U__R | U__T  | U__UE)
#define ITUQ  (U__I | U__T |                U__UE | U__XM)
#define SWT   (U__S | U__W | U__T |         U__UE | U__XM | U__E)
#define ITAH  (U__I | U__T | U__A | U__H  | U__UE | U__XM | U__E)
#define ITAU  (U__I | U__T | U__A |         U__UE | U__XM | U__UQ)
#define ITJU  (U__I | U__T | U__J |         U__UE | U__XM | U__UQ)
#define ITVF  (U__I | U__T | U__V |                 U__XM)
#define ITKF  (U__I | U__T | U__K |                 U__XM | U__E)

#define ITUEF  (U__I | U__T | U__UE | U__XM)
#define ITUEFX (U__I | U__T | U__UE | U__XM | U__XE)
#define ITUEFQ (U__I | U__T | U__UE | U__XM | U__UQ | U__E)

const unsigned int u__ct_tab[256] = {
/*                         BEL  BS    HT    LF        FF    CR                 */
CF, CF, CF, CF, CF, CF, CF, CT, CT, CWBT, CWRT, CWF, CWT, CWRT, CF, CF,/* 0x00 */
/*                                              ESC                            */
CF, CF, CF, CF, CF, CF, CF, CF, CF,   CF,   CF,  CT,  CF,  CF,  CF, CF,/* 0x10 */

/* ' ' '!' '"'  '#'   '$'  '%'  '&' '\'' '('  ')' '*' '+'   ','  '-'   '.'  '/'         */
  SWT, ITF,ITK,ITUEFQ,ITF,ITUQ,ITJU,ITKF,ITF,ITF, IT,ITAH,ITVF, ITO,  ITN, ITA,  /* 0x20 */
/* '0' '1' '2'  '3'   '4'  '5'  '6' '7'  '8'  '9' ':' ';'   '<'  '='   '>'  '?'         */
  DTZ, DTZ,DTZ, DTZ, DTZ,  DTZ,DTZ, DTZ, DT,  DT,ITG,ITUEF,ITJ, ITAU, ITJ,ITUE,  /* 0x30 */
/* '@' 'A' 'B'  'C'   'D'  'E'  'F' 'G'  'H'  'I' 'J' 'K'   'L'  'M'   'N'  'O'         */
  ITF,UXTY,UXT,UXTMY,UXTM, UXT,UXT, UTM,UTMY, UTY,UTE,UT,  UTE, UTE,  UTE, UTM,  /* 0x40 */
/* 'P' 'Q' 'R'  'S'   'T'  'U'  'V' 'W'  'X'  'Y' 'Z' '['   '\'  ']'   '^'  '_'         */
  UTM,UTE,UTY, UTY,  UTE, UTY, UTE,UTE, UTY, UTE, UT,ITUEF,ITUE,ITUEFX,ITUEF,ITQ,/* 0x50 */
/* '`' 'a' 'b'  'c'   'd'  'e'  'f' 'g'  'h'  'i' 'j' 'k'   'l'  'm'   'n'  'o'         */
ITUEF,LXTY,LXT,LXTMY,LXTM,LXT, LXT,LTM,LTMY, LTY,LTE, LT,  LTE,  LT,  LTE, LTM,  /* 0x60 */
/* 'p' 'q' 'r'  's'   't'  'u'  'v' 'w'  'x'  'y' 'z' '{'   '|'  '}'   '~'              */
  LTM,LTE,LTY, LTY,   LT, LTY, LTE,LTE, LTY,  LT, LT,ITUEF,ITUE,ITUEFX,ITF, CF,  /* 0x70 */

FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, /* 0x80 */
FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, /* 0x90 */
FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, /* 0xa0 */
FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, /* 0xb0 */
FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, /* 0xc0 */
FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, /* 0xd0 */
FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, /* 0xe0 */
FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE, FUE  /* 0xf0 */

/**
 * ISO-1 character set
 *
 * C,  C,  C,  C,  C, CT,  C,  C,  C,  C,  C,  C,  C,  C,  C,  C,
 * C,  C,  C,  C,  C,  C,  C,  C,  C,  C,  C,  C,  C,  C,  C,  C,
 * W,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,
 * I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,
 * U,  U,  U,  U,  U,  U,  U,  U,  U,  U,  U,  U,  U,  U,  U,  U,
 * U,  U,  U,  U,  U,  U,  U,  I,  U,  U,  U,  U,  U,  U,  U, LU,
 * L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,
 * L,  L,  L,  L,  L,  L,  L,  I,  L,  L,  L,  L,  L,  L,  L,  L 
 */
};

#undef U__S
#undef U__H
#undef U__V
#undef U__O
#undef U__N
#undef U__G
#undef U__Q
#undef U__B
#undef U__R
#undef U__W
#undef U__C
#undef U__D
#undef U__L
#undef U__I
#undef U__U
#undef U__Z
#undef U__F
#undef U__T
#undef U__X
#undef U__A
#undef U__M
#undef U__Y
#undef U__K
#undef U__J
#undef U__UE
#undef U__UQ
#undef U__UF
#undef U__XM
#undef U__XE

#undef CF
#undef CT
#undef DT
#undef LU
#undef LX
#undef LT
#undef LTE
#undef UT
#undef UTE
#undef UX
#undef WF
#undef IF
#undef IT
#undef FUE
#undef CWT
#undef CWF
#undef DTZ
#undef ITA
#undef ITF
#undef ITG
#undef ITK
#undef ITJ
#undef ITN
#undef ITO
#undef ITQ
#undef LTM
#undef LTY
#undef LXT
#undef UXT
#undef SWT
#undef UTY
#undef UTM
#undef ITVF
#undef ITKF
#undef ITUE
#undef ITAH
#undef CWRT
#undef LTMY
#undef LXTM
#undef LXTY
#undef UXTM
#undef UTMY
#undef UXTY
#undef ITAU
#undef ITJU
#undef ITUQ
#undef ITUEF
#undef ITUEFX
#undef ITUEFQ
#undef LXTMY
#undef UXTMY

/* Table for converting to lower-case */

const unsigned char u__ct_tol[256] = {
   '\0',   '\01',  '\02',  '\03',  '\04',  '\05',  '\06',  '\07',
   '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
   '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
   '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
   ' ',    '!',    '\"',   '#',    '$',    '%',    '&',    '\'',
   '(',    ')',    '*',    '+',    ',',    '-',    '.',    '/',
   '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
   '8',    '9',    ':',    ';',    '<',    '=',    '>',    '?',
   '@',    'a',    'b',    'c',    'd',    'e',    'f',    'g',
   'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
   'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
   'x',    'y',    'z',    '[',    '\\',   ']',    '^',    '_',
   '`',    'a',    'b',    'c',    'd',    'e',    'f',    'g',
   'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',
   'p',    'q',    'r',    's',    't',    'u',    'v',    'w',
   'x',    'y',    'z',    '{',    '|',    '}',    '~',    '\177',

   /* ISO-1 */

   0x80,   0x81,   0x82,   0x83,   0x84,   0x85,   0x86,   0x87,
   0x88,   0x89,   0x8a,   0x8b,   0x8c,   0x8d,   0x8e,   0x8f,
   0x90,   0x91,   0x92,   0x93,   0x94,   0x95,   0x96,   0x97,
   0x98,   0x99,   0x9a,   0x9b,   0x9c,   0x9d,   0x9e,   0x9f,
   0xa0,   0xa1,   0xa2,   0xa3,   0xa4,   0xa5,   0xa6,   0xa7,
   0xa8,   0xa9,   0xaa,   0xab,   0xac,   0xad,   0xae,   0xaf,
   0xb0,   0xb1,   0xb2,   0xb3,   0xb4,   0xb5,   0xb6,   0xb7,
   0xb8,   0xb9,   0xba,   0xbb,   0xbc,   0xbd,   0xbe,   0xbf,
   0xe0,   0xe1,   0xe2,   0xe3,   0xe4,   0xe5,   0xe6,   0xe7,
   0xe8,   0xe9,   0xea,   0xeb,   0xec,   0xed,   0xee,   0xef,
   0xf0,   0xf1,   0xf2,   0xf3,   0xf4,   0xf5,   0xf6,   0xd7,
   0xf8,   0xf9,   0xfa,   0xfb,   0xfc,   0xfd,   0xfe,   0xdf,
   0xe0,   0xe1,   0xe2,   0xe3,   0xe4,   0xe5,   0xe6,   0xe7,
   0xe8,   0xe9,   0xea,   0xeb,   0xec,   0xed,   0xee,   0xef,
   0xf0,   0xf1,   0xf2,   0xf3,   0xf4,   0xf5,   0xf6,   0xf7,
   0xf8,   0xf9,   0xfa,   0xfb,   0xfc,   0xfd,   0xfe,   0xff
};

/* Table for converting to upper-case */

const unsigned char u__ct_tou[256] = {
   '\0',   '\01',  '\02',  '\03',  '\04',  '\05',  '\06',  '\07',
   '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
   '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
   '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
   ' ',    '!',    '\"',   '#',    '$',    '%',    '&',    '\'',
   '(',    ')',    '*',    '+',    ',',    '-',    '.',    '/',
   '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7',
   '8',    '9',    ':',    ';',    '<',    '=',    '>',    '?',
   '@',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
   'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
   'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
   'X',    'Y',    'Z',    '[',    '\\',   ']',    '^',    '_',
   '`',    'A',    'B',    'C',    'D',    'E',    'F',    'G',
   'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',
   'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',
   'X',    'Y',    'Z',    '{',    '|',    '}',    '~',    '\177',

   /* ISO-1 */

   0x80,   0x81,   0x82,   0x83,   0x84,   0x85,   0x86,   0x87,
   0x88,   0x89,   0x8a,   0x8b,   0x8c,   0x8d,   0x8e,   0x8f,
   0x90,   0x91,   0x92,   0x93,   0x94,   0x95,   0x96,   0x97,
   0x98,   0x99,   0x9a,   0x9b,   0x9c,   0x9d,   0x9e,   0x9f,
   0xa0,   0xa1,   0xa2,   0xa3,   0xa4,   0xa5,   0xa6,   0xa7,
   0xa8,   0xa9,   0xaa,   0xab,   0xac,   0xad,   0xae,   0xaf,
   0xb0,   0xb1,   0xb2,   0xb3,   0xb4,   0xb5,   0xb6,   0xb7,
   0xb8,   0xb9,   0xba,   0xbb,   0xbc,   0xbd,   0xbe,   0xbf,
   0xc0,   0xc1,   0xc2,   0xc3,   0xc4,   0xc5,   0xc6,   0xc7,
   0xc8,   0xc9,   0xca,   0xcb,   0xcc,   0xcd,   0xce,   0xcf,
   0xd0,   0xd1,   0xd2,   0xd3,   0xd4,   0xd5,   0xd6,   0xd7,
   0xd8,   0xd9,   0xda,   0xdb,   0xdc,   0xdd,   0xde,   0xdf,
   0xc0,   0xc1,   0xc2,   0xc3,   0xc4,   0xc5,   0xc6,   0xc7,
   0xc8,   0xc9,   0xca,   0xcb,   0xcc,   0xcd,   0xce,   0xcf,
   0xd0,   0xd1,   0xd2,   0xd3,   0xd4,   0xd5,   0xd6,   0xf7,
   0xd8,   0xd9,   0xda,   0xdb,   0xdc,   0xdd,   0xde,   0xff
};

/* Table for calculate hex digit => unsigned int */

const unsigned char u__ct_hex2int[112] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x00 - 0x0f */
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x10 - 0x1f */
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*  !"#$%&'()*+,-./ */
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, /* 0123456789:;<=>? */
   0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* @ABCDEFGHIJKLMNO */
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* PQRSTUVWXYZ[\]^_ */
   0,10,11,12,13,14,15, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* `abcdefghijklmno */
/**
 * 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // pqrstuvwxyz{|}~
 * 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 * 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 * 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 * 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 * 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 * 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 * 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 * 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 */
};

/* MIME TYPE */

typedef struct mimeentry {
   const char* restrict type;
   const char* restrict name;
   uint32_t name_len;
} mimeentry;

#define MIME_ENTRY(name,type) { type, name+1, U_CONSTANT_SIZE(name)-1 }

/**
 * Complete list of MIME types
 *
 *.3dm  x-world/x-3dmf
 *.3dmf x-world/x-3dmf
 *
 *.a     application/octet-stream
 *.aab   application/x-authorware-bin
 *.aam   application/x-authorware-map
 *.aas   application/x-authorware-seg
 *.abc   text/vnd.abc
 *.acgi  text/html
 *.afl   video/animaflex
 *.ai    application/postscript
 *.aif   audio/aiff
 *.aif   audio/x-aiff
 *.aifc  audio/aiff
 *.aifc  audio/x-aiff
 *.aiff  audio/aiff
 *.aiff  audio/x-aiff
 *.aim   application/x-aim
 *.aip   text/x-audiosoft-intra
 *.ani   application/x-navi-animation
 *.aos   application/x-nokia-9000-communicator-add-on-software
 *.aps   application/mime
 *.arc   application/octet-stream
 *.arj   application/arj
 *.arj   application/octet-stream
 *.art   image/x-jg
 *.asf   video/x-ms-asf
 *.asm   text/x-asm
 *.asp   text/asp
 *.asx   application/x-mplayer2
 *.asx   video/x-ms-asf
 *.asx   video/x-ms-asf-plugin
 *.au    audio/basic
 *.au    audio/x-au
 *.avi   application/x-troff-msvideo
 *.avi   video/avi
 *.avi   video/msvideo
 *.avi   video/x-msvideo
 *.avs   video/avs-video
 *
 *.bcpio application/x-bcpio
 *.bin   application/mac-binary
 *.bin   application/macbinary
 *.bin   application/octet-stream
 *.bin   application/x-binary
 *.bin   application/x-macbinary
 *.bm    image/bmp
 *.bmp   image/bmp
 *.bmp   image/x-windows-bmp
 *.boo   application/book
 *.book  application/book
 *.boz   application/x-bzip2
 *.bsh   application/x-bsh
 *.bz    application/x-bzip
 *.bz2   application/x-bzip2
 *
 *.c     text/plain
 *.c     text/x-c
 *.c++   text/plain
 *.cat   application/vnd.ms-pki.seccat
 *.cc    text/plain
 *.cc    text/x-c
 *.ccad  application/clariscad
 *.cco   application/x-cocoa
 *.cdf   application/cdf
 *.cdf   application/x-cdf
 *.cdf   application/x-netcdf
 *.cer   application/pkix-cert
 *.cer   application/x-x509-ca-cert
 *.cha   application/x-chat
 *.chat  application/x-chat
 *.class application/java
 *.class application/java-byte-code
 *.class application/x-java-class
 *.com   application/octet-stream
 *.com   text/plain
 *.conf  text/plain
 *.cpio  application/x-cpio
 *.cpp   text/x-c
 *.cpt   application/mac-compactpro
 *.cpt   application/x-compactpro
 *.cpt   application/x-cpt
 *.crl   application/pkcs-crl
 *.crl   application/pkix-crl
 *.crt   application/pkix-cert
 *.crt   application/x-x509-ca-cert
 *.crt   application/x-x509-user-cert
 *.csh   application/x-csh
 *.csh   text/x-script.csh
 *.css   application/x-pointplus
 *.css   text/css
 *.cxx   text/plain
 *
 *.dcr   application/x-director
 *.deepv application/x-deepv
 *.def   text/plain
 *.der   application/x-x509-ca-cert
 *.dif   video/x-dv
 *.dir   application/x-director
 *.dl    video/dl
 *.dl    video/x-dl
 *.doc   application/msword
 *.dot   application/msword
 *.dp    application/commonground
 *.drw   application/drafting
 *.dump  application/octet-stream
 *.dv    video/x-dv
 *.dvi   application/x-dvi
 *.dwf   drawing/x-dwf (old)
 *.dwf   model/vnd.dwf
 *.dwg   application/acad
 *.dwg   image/vnd.dwg
 *.dwg   image/x-dwg
 *.dxf   application/dxf
 *.dxf   image/vnd.dwg
 *.dxf   image/x-dwg
 *.dxr   application/x-director
 *
 *.el    text/x-script.elisp
 *.elc   application/x-bytecode.elisp (compiled elisp)
 *.elc   application/x-elc
 *.env   application/x-envoy
 *.eps   application/postscript
 *.es    application/x-esrehber
 *.etx   text/x-setext
 *.evy   application/envoy
 *.evy   application/x-envoy
 *.exe   application/octet-stream
 *
 *.f     text/plain
 *.f     text/x-fortran
 *.f77   text/x-fortran
 *.f90   text/plain
 *.f90   text/x-fortran
 *.fdf   application/vnd.fdf
 *.fif   application/fractals
 *.fif   image/fif
 *.fli   video/fli
 *.fli   video/x-fli
 *.flo   image/florian
 *.flx   text/vnd.fmi.flexstor
 *.fmf   video/x-atomic3d-feature
 *.for   text/plain
 *.for   text/x-fortran
 *.fpx   image/vnd.fpx
 *.fpx   image/vnd.net-fpx
 *.frl   application/freeloader
 *.funk  audio/make
 *
 *.g     text/plain
 *.g3    image/g3fax
 *.gif   image/gif
 *.gl    video/gl
 *.gl    video/x-gl
 *.gsd   audio/x-gsm
 *.gsm   audio/x-gsm
 *.gsp   application/x-gsp
 *.gss   application/x-gss
 *.gtar  application/x-gtar
 *.gz    application/x-compressed
 *.gz    application/x-gzip
 *.gzip  application/x-gzip
 *.gzip  multipart/x-gzip
 *
 *.h     text/plain
 *.h     text/x-h
 *.hdf   application/x-hdf
 *.help  application/x-helpfile
 *.hgl   application/vnd.hp-hpgl
 *.hh    text/plain
 *.hh    text/x-h
 *.hlb   text/x-script
 *.hlp   application/hlp
 *.hlp   application/x-helpfile
 *.hlp   application/x-winhelp
 *.hpg   application/vnd.hp-hpgl
 *.hpgl  application/vnd.hp-hpgl
 *.hqx   application/binhex
 *.hqx   application/binhex4
 *.hqx   application/mac-binhex
 *.hqx   application/mac-binhex40
 *.hqx   application/x-binhex40
 *.hqx   application/x-mac-binhex40
 *.hta   application/hta
 *.htc   text/x-component
 *.htm   text/html
 *.html  text/html
 *.htmls text/html
 *.htt   text/webviewhtml
 *.htx   text/html
 *
 *.ice   x-conference/x-cooltalk
 *.ico   image/x-icon
 *.idc   text/plain
 *.ief   image/ief
 *.iefs  image/ief
 *.iges  application/iges
 *.iges  model/iges
 *.igs   application/iges
 *.igs   model/iges
 *.ima   application/x-ima
 *.imap  application/x-httpd-imap
 *.inf   application/inf
 *.ins   application/x-internett-signup
 *.ip    application/x-ip2
 *.isu   video/x-isvideo
 *.it    audio/it
 *.iv    application/x-inventor
 *.ivr   i-world/i-vrml
 *.ivy   application/x-livescreen
 *
 *.jam   audio/x-jam
 *.jav   text/plain
 *.jav   text/x-java-source
 *.java  text/plain
 *.java  text/x-java-source
 *.jcm   application/x-java-commerce
 *.jfif  image/jpeg
 *.jfif  image/pjpeg
 *.jfif-tbnl image/jpeg
 *.jpe   image/jpeg
 *.jpe   image/pjpeg
 *.jpeg  image/jpeg
 *.jpeg  image/pjpeg
 *.jpg   image/jpeg
 *.jpg   image/pjpeg
 *.jps   image/x-jps
 *.js    application/x-javascript
 *.js    application/javascript
 *.js    application/ecmascript
 *.js    text/javascript
 *.js    text/ecmascript
 *.jut   image/jutvision
 *
 *.kar   audio/midi
 *.kar   music/x-karaoke
 *.ksh   application/x-ksh
 *.ksh   text/x-script.ksh
 *
 *.la    audio/nspaudio
 *.la    audio/x-nspaudio
 *.lam   audio/x-liveaudio
 *.latex application/x-latex
 *.lha   application/lha
 *.lha   application/octet-stream
 *.lha   application/x-lha
 *.lhx   application/octet-stream
 *.list  text/plain
 *.lma   audio/nspaudio
 *.lma   audio/x-nspaudio
 *.log   text/plain
 *.lsp   application/x-lisp
 *.lsp   text/x-script.lisp
 *.lst   text/plain
 *.lsx   text/x-la-asf
 *.ltx   application/x-latex
 *.lzh   application/octet-stream
 *.lzh   application/x-lzh
 *.lzx   application/lzx
 *.lzx   application/octet-stream
 *.lzx   application/x-lzx
 *
 *.m     text/plain
 *.m     text/x-m
 *.m1v   video/mpeg
 *.m2a   audio/mpeg
 *.m2v   video/mpeg
 *.m3u   audio/x-mpequrl
 *.man   application/x-troff-man
 *.map   application/x-navimap
 *.mar   text/plain
 *.mbd   application/mbedlet
 *.mc$   application/x-magic-cap-package-1.0
 *.mcd   application/mcad
 *.mcd   application/x-mathcad
 *.mcf   image/vasa
 *.mcf   text/mcf
 *.mcp   application/netmc
 *.me    application/x-troff-me
 *.mht   message/rfc822
 *.mhtml message/rfc822
 *.mid   application/x-midi
 *.mid   audio/midi
 *.mid   audio/x-mid
 *.mid   audio/x-midi
 *.mid   music/crescendo
 *.mid   x-music/x-midi
 *.midi  application/x-midi
 *.midi  audio/midi
 *.midi  audio/x-mid
 *.midi  audio/x-midi
 *.midi  music/crescendo
 *.midi  x-music/x-midi
 *.mif   application/x-frame
 *.mif   application/x-mif
 *.mime  message/rfc822
 *.mime  www/mime
 *.mjf   audio/x-vnd.audioexplosion.mjuicemediafile
 *.mjpg  video/x-motion-jpeg
 *.mm    application/base64
 *.mm    application/x-meme
 *.mme   application/base64
 *.mod   audio/mod
 *.mod   audio/x-mod
 *.moov  video/quicktime
 *.mov   video/quicktime
 *.movie video/x-sgi-movie
 *.mp2   audio/mpeg
 *.mp2   audio/x-mpeg
 *.mp2   video/mpeg
 *.mp2   video/x-mpeg
 *.mp2   video/x-mpeq2a
 *.mp3   audio/mpeg3
 *.mp3   audio/x-mpeg-3
 *.mp3   video/mpeg
 *.mp3   video/x-mpeg
 *.mpa   audio/mpeg
 *.mpa   video/mpeg
 *.mpc   application/x-project
 *.mpe   video/mpeg
 *.mpeg  video/mpeg
 *.mpg   audio/mpeg
 *.mpg   video/mpeg
 *.mpga  audio/mpeg
 *.mpp   application/vnd.ms-project
 *.mpt   application/x-project
 *.mpv   application/x-project
 *.mpx   application/x-project
 *.mrc   application/marc
 *.ms    application/x-troff-ms
 *.mv    video/x-sgi-movie
 *.my    audio/make
 *.mzz   application/x-vnd.audioexplosion.mzz
 *
 *.nap   image/naplps
 *.naplps image/naplps
 *.nc    application/x-netcdf
 *.ncm   application/vnd.nokia.configuration-message
 *.nif   image/x-niff
 *.niff  image/x-niff
 *.nix   application/x-mix-transfer
 *.nsc   application/x-conference
 *.nvd   application/x-navidoc
 *
 *.o     application/octet-stream
 *.oda   application/oda
 *.omc   application/x-omc
 *.omcd  application/x-omcdatamaker
 *.omcr  application/x-omcregerator
 *
 *.p     text/x-pascal
 *.p10   application/pkcs10
 *.p10   application/x-pkcs10
 *.p12   application/pkcs-12
 *.p12   application/x-pkcs12
 *.p7a   application/x-pkcs7-signature
 *.p7c   application/pkcs7-mime
 *.p7c   application/x-pkcs7-mime
 *.p7m   application/pkcs7-mime
 *.p7m   application/x-pkcs7-mime
 *.p7r   application/x-pkcs7-certreqresp
 *.p7s   application/pkcs7-signature
 *.part  application/pro_eng
 *.pas   text/pascal
 *.pbm   image/x-portable-bitmap
 *.pcl   application/vnd.hp-pcl
 *.pcl   application/x-pcl
 *.pct   image/x-pict
 *.pcx   image/x-pcx
 *.pdb   chemical/x-pdb
 *.pdf   application/pdf
 *.pfunk audio/make
 *.pfunk audio/make.my.funk
 *.pgm   image/x-portable-graymap
 *.pgm   image/x-portable-greymap
 *.pic   image/pict
 *.pict  image/pict
 *.pkg   application/x-newton-compatible-pkg
 *.pko   application/vnd.ms-pki.pko
 *.pl    text/plain
 *.pl    text/x-script.perl
 *.plx   application/x-pixclscript
 *.pm    image/x-xpixmap
 *.pm    text/x-script.perl-module
 *.pm4   application/x-pagemaker
 *.pm5   application/x-pagemaker
 *.png   image/png
 *.pnm   application/x-portable-anymap
 *.pnm   image/x-portable-anymap
 *.pot   application/mspowerpoint
 *.pot   application/vnd.ms-powerpoint
 *.pov   model/x-pov
 *.ppa   application/vnd.ms-powerpoint
 *.ppm   image/x-portable-pixmap
 *.pps   application/mspowerpoint
 *.pps   application/vnd.ms-powerpoint
 *.ppt   application/mspowerpoint
 *.ppt   application/powerpoint
 *.ppt   application/vnd.ms-powerpoint
 *.ppt   application/x-mspowerpoint
 *.ppz   application/mspowerpoint
 *.pre   application/x-freelance
 *.prt   application/pro_eng
 *.ps    application/postscript
 *.psd   application/octet-stream
 *.pvu   paleovu/x-pv
 *.pwz   application/vnd.ms-powerpoint
 *.py    text/x-script.phyton
 *.pyc   applicaiton/x-bytecode.python
 *
 *.qcp   audio/vnd.qcelp
 *.qd3   x-world/x-3dmf
 *.qd3d  x-world/x-3dmf
 *.qif   image/x-quicktime
 *.qt    video/quicktime
 *.qtc   video/x-qtc
 *.qti   image/x-quicktime
 *.qtif  image/x-quicktime
 *
 *.ra    audio/x-pn-realaudio
 *.ra    audio/x-pn-realaudio-plugin
 *.ra    audio/x-realaudio
 *.ram   audio/x-pn-realaudio
 *.ras   application/x-cmu-raster
 *.ras   image/cmu-raster
 *.ras   image/x-cmu-raster
 *.rast  image/cmu-raster
 *.rexx  text/x-script.rexx
 *.rf    image/vnd.rn-realflash
 *.rgb   image/x-rgb
 *.rm    application/vnd.rn-realmedia
 *.rm    audio/x-pn-realaudio
 *.rmi   audio/mid
 *.rmm   audio/x-pn-realaudio
 *.rmp   audio/x-pn-realaudio
 *.rmp   audio/x-pn-realaudio-plugin
 *.rng   application/ringing-tones
 *.rng   application/vnd.nokia.ringing-tone
 *.rnx   application/vnd.rn-realplayer
 *.roff  application/x-troff
 *.rp    image/vnd.rn-realpix
 *.rpm   audio/x-pn-realaudio-plugin
 *.rt    text/richtext
 *.rt    text/vnd.rn-realtext
 *.rtf   application/rtf
 *.rtf   application/x-rtf
 *.rtf   text/richtext
 *.rtx   application/rtf
 *.rtx   text/richtext
 *.rv    video/vnd.rn-realvideo
 *
 *.s     text/x-asm
 *.s3m   audio/s3m
 *.saveme application/octet-stream
 *.sbk   application/x-tbook
 *.scm   application/x-lotusscreencam
 *.scm   text/x-script.guile
 *.scm   text/x-script.scheme
 *.scm   video/x-scm
 *.sdml  text/plain
 *.sdp   application/sdp
 *.sdp   application/x-sdp
 *.sdr   application/sounder
 *.sea   application/sea
 *.sea   application/x-sea
 *.set   application/set
 *.sgm   text/sgml
 *.sgm   text/x-sgml
 *.sgml  text/sgml
 *.sgml  text/x-sgml
 *.sh    application/x-bsh
 *.sh    application/x-sh
 *.sh    application/x-shar
 *.sh    text/x-script.sh
 *.shar  application/x-bsh
 *.shar  application/x-shar
 *.shtml text/html
 *.shtml text/x-server-parsed-html
 *.sid   audio/x-psid
 *.sit   application/x-sit
 *.sit   application/x-stuffit
 *.skd   application/x-koan
 *.skm   application/x-koan
 *.skp   application/x-koan
 *.skt   application/x-koan
 *.sl    application/x-seelogo
 *.smi   application/smil
 *.smil  application/smil
 *.snd   audio/basic
 *.snd   audio/x-adpcm
 *.sol   application/solids
 *.spc   application/x-pkcs7-certificates
 *.spc   text/x-speech
 *.spl   application/futuresplash
 *.spr   application/x-sprite
 *.sprite application/x-sprite
 *.src   application/x-wais-source
 *.ssi   text/x-server-parsed-html
 *.ssm   application/streamingmedia
 *.sst   application/vnd.ms-pki.certstore
 *.step  application/step
 *.stl   application/sla
 *.stl   application/vnd.ms-pki.stl
 *.stl   application/x-navistyle
 *.stp   application/step
 *.sv4cpio application/x-sv4cpio
 *.sv4crc application/x-sv4crc
 *.svf   image/vnd.dwg
 *.svf   image/x-dwg
 *.svr   application/x-world
 *.svr   x-world/x-svr
 *.swf   application/x-shockwave-flash
 *
 *.t     application/x-troff
 *.talk  text/x-speech
 *.tar   application/x-tar
 *.tbk   application/toolbook
 *.tbk   application/x-tbook
 *.tcl   application/x-tcl
 *.tcl   text/x-script.tcl
 *.tcsh  text/x-script.tcsh
 *.tex   application/x-tex
 *.texi  application/x-texinfo
 *.texinfo application/x-texinfo
 *.text  application/plain
 *.text  text/plain
 *.tgz   application/gnutar
 *.tgz   application/x-compressed
 *.tif   image/tiff
 *.tif   image/x-tiff
 *.tiff  image/tiff
 *.tiff  image/x-tiff
 *.tr    application/x-troff
 *.tsi   audio/tsp-audio
 *.tsp   application/dsptype
 *.tsp   audio/tsplayer
 *.tsv   text/tab-separated-values
 *.turbot image/florian
 *.txt   text/plain
 *
 *.uil   text/x-uil
 *.uni   text/uri-list
 *.unis  text/uri-list
 *.unv   application/i-deas
 *.uri   text/uri-list
 *.uris  text/uri-list
 *.ustar application/x-ustar
 *.ustar multipart/x-ustar
 *.uu    application/octet-stream
 *.uu    text/x-uuencode
 *.uue   text/x-uuencode
 *
 *.vcd   application/x-cdlink
 *.vcs   text/x-vcalendar
 *.vda   application/vda
 *.vdo   video/vdo
 *.vew   application/groupwise
 *.viv   video/vivo
 *.viv   video/vnd.vivo
 *.vivo  video/vivo
 *.vivo  video/vnd.vivo
 *.vmd   application/vocaltec-media-desc
 *.vmf   application/vocaltec-media-file
 *.voc   audio/voc
 *.voc   audio/x-voc
 *.vos   video/vosaic
 *.vox   audio/voxware
 *.vqe   audio/x-twinvq-plugin
 *.vqf   audio/x-twinvq
 *.vql   audio/x-twinvq-plugin
 *.vrml  application/x-vrml
 *.vrml  model/vrml
 *.vrml  x-world/x-vrml
 *.vrt   x-world/x-vrt
 *.vsd   application/x-visio
 *.vst   application/x-visio
 *.vsw   application/x-visio
 *
 *.w60   application/wordperfect6.0
 *.w61   application/wordperfect6.1
 *.w6w   application/msword
 *.wav   audio/wav
 *.wav   audio/x-wav
 *.wb1   application/x-qpro
 *.wbmp  image/vnd.wap.wbmp
 *.web   application/vnd.xara
 *.wiz   application/msword
 *.wk1   application/x-123
 *.wmf   windows/metafile
 *.wml   text/vnd.wap.wml
 *.wmlc  application/vnd.wap.wmlc
 *.wmls  text/vnd.wap.wmlscript
 *.wmlsc application/vnd.wap.wmlscriptc
 *.word  application/msword
 *.wp    application/wordperfect
 *.wp5   application/wordperfect
 *.wp5   application/wordperfect6.0
 *.wp6   application/wordperfect
 *.wpd   application/wordperfect
 *.wpd   application/x-wpwin
 *.wq1   application/x-lotus
 *.wri   application/mswrite
 *.wri   application/x-wri
 *.wrl   application/x-world
 *.wrl   model/vrml
 *.wrl   x-world/x-vrml
 *.wrz   model/vrml
 *.wrz   x-world/x-vrml
 *.wsc   text/scriplet
 *.wsrc  application/x-wais-source
 *.wtk   application/x-wintalk
 *
 *.xbm   image/x-xbitmap
 *.xbm   image/x-xbm
 *.xbm   image/xbm
 *.xdr   video/x-amt-demorun
 *.xgz   xgl/drawing
 *.xif   image/vnd.xiff
 *.xl    application/excel
 *.xla   application/excel
 *.xla   application/x-excel
 *.xla   application/x-msexcel
 *.xlb   application/excel
 *.xlb   application/vnd.ms-excel
 *.xlb   application/x-excel
 *.xlc   application/excel
 *.xlc   application/vnd.ms-excel
 *.xlc   application/x-excel
 *.xld   application/excel
 *.xld   application/x-excel
 *.xlk   application/excel
 *.xlk   application/x-excel
 *.xll   application/excel
 *.xll   application/vnd.ms-excel
 *.xll   application/x-excel
 *.xlm   application/excel
 *.xlm   application/vnd.ms-excel
 *.xlm   application/x-excel
 *.xls   application/excel
 *.xls   application/vnd.ms-excel
 *.xls   application/x-excel
 *.xls   application/x-msexcel
 *.xlt   application/excel
 *.xlt   application/x-excel
 *.xlv   application/excel
 *.xlv   application/x-excel
 *.xlw   application/excel
 *.xlw   application/vnd.ms-excel
 *.xlw   application/x-excel
 *.xlw   application/x-msexcel
 *.xm    audio/xm
 *.xml   application/xml
 *.xml   text/xml
 *.xmz   xgl/movie
 *.xpix  application/x-vnd.ls-xpix
 *.xpm   image/x-xpixmap
 *.xpm   image/xpm
 *.x-png image/png
 *.xsr   video/x-amt-showrun
 *.xwd   image/x-xwd
 *.xwd   image/x-xwindowdump
 *.xyz   chemical/x-pdb
 *
 *.z     application/x-compress
 *.z     application/x-compressed
 *.zip   application/x-compressed
 *.zip   application/x-zip-compressed
 *.zip   application/zip
 *.zip   multipart/x-zip
 *.zoo   application/octet-stream
 *.zsh   text/x-script.zsh
 */

static struct mimeentry mimetab_a[] = {
   MIME_ENTRY( "ai",       "application/postscript" ),
   MIME_ENTRY( "arj",      "application/x-arj-compressed" ),
   MIME_ENTRY( "asx",      "video/x-ms-asf" ),
   MIME_ENTRY( "atom",     "application/atom+xml" ),
   MIME_ENTRY( "avi",      "video/x-msvideo" ),
   MIME_ENTRY( "appcache", "text/cache-manifest" ),

   /*
   MIME_ENTRY( "aif",  "audio/x-aiff" ), (aifc, aiff)
   MIME_ENTRY( "api",  "application/postscript" ),
   MIME_ENTRY( "asc",  "text/plain" ),
   MIME_ENTRY( "au",   "audio/basic" ),
   */

   { 0, 0, 0 }
};

static struct mimeentry mimetab_b[] = {
   MIME_ENTRY( "bash", "application/x-bsh" ),
   MIME_ENTRY( "bat",  "application/x-msdos-program" ),
   MIME_ENTRY( "bmp",  "image/x-ms-bmp" ),
   MIME_ENTRY( "bild", "image/jpeg" ),
   MIME_ENTRY( "boz",  "application/x-bzip2" ),

   /*
   MIME_ENTRY( "bin",   "application/x-bcpio" ),
   MIME_ENTRY( "bcpio", "application/octet-stream" ),
   */

   { 0, 0, 0 }
};

static struct mimeentry mimetab_c[] = {

   /**
    * The certificate being downloaded represents a Certificate Authority.
    * When it is downloaded the user will be shown a sequence of dialogs that
    * will guide them through the process of accepting the Certificate Authority
    * and deciding if they wish to trust sites certified by the CA. If a certificate
    * chain is being imported then the first certificate in the chain must be the CA
    * certificate, and any subsequent certificates will be added as untrusted CA
    * certificates to the local database
    */
   MIME_ENTRY( "crt", "application/x-x509-ca-cert" ),
   MIME_ENTRY( "cer", "application/x-x509-ca-cert" ),
   MIME_ENTRY( "crx", "application/x-chrome-extension crx" ),

   /*
   MIME_ENTRY( "c++",     "text/x-c++src" ),
   MIME_ENTRY( "c4d",     "application/vnd.clonk.c4group" ),
   MIME_ENTRY( "cac",     "chemical/x-cache" ),
   MIME_ENTRY( "cascii",  "chemical/x-cactvs-binary" ),
   MIME_ENTRY( "cct",     "application/x-director" ),
   MIME_ENTRY( "cdf",     "application/x-netcdf" ),
   MIME_ENTRY( "cef",     "chemical/x-cxf" ),
   MIME_ENTRY( "cls",     "text/x-tex" ),
   MIME_ENTRY( "cpio",    "application/x-cpio" ),
   MIME_ENTRY( "cpt",     "application/mac-compactpro" ),
   MIME_ENTRY( "csm",     "chemical/x-csml" ),
   MIME_ENTRY( "csh",     "application/x-csh" ),
   MIME_ENTRY( "cdf",     "application/x-netcdf" ),
   MIME_ENTRY( "c",       "text/x-c" ),
   */

   { 0, 0, 0 }
};

static struct mimeentry mimetab_d[] = {
   MIME_ENTRY( "der", "application/x-x509-ca-cert" ),
   MIME_ENTRY( "doc", "application/msword" ),
   MIME_ENTRY( "dtd", "application/xml" ),

   /*
   MIME_ENTRY( "dcr", "application/x-director" ),
   MIME_ENTRY( "deb", "application/x-debian-package" ),
   MIME_ENTRY( "dir", "application/x-director" ),
   MIME_ENTRY( "dll", "application/octet-stream" ),
   MIME_ENTRY( "dms", "application/octet-stream" ),
   MIME_ENTRY( "dvi", "application/x-dvi" ),
   MIME_ENTRY( "dxr", "application/x-director" ),
   */

   { 0, 0, 0 }
};

static struct mimeentry mimetab_e[] = {
   MIME_ENTRY( "eps", "application/postscript" ),
   MIME_ENTRY( "eot", "application/vnd.ms-fontobject" ),

   /*
   MIME_ENTRY( "etx", "text/x-setext" ),
   MIME_ENTRY( "ez",  "application/andrew-inset" ),
   MIME_ENTRY( "exe", "application/octet-stream" ),
   */

   { 0, 0, 0 }
};

/*
static struct mimeentry mimetab_f[] = {
   { 0, 0, 0 }
};
*/

static struct mimeentry mimetab_g[] = {
   MIME_ENTRY( "gtar", "application/x-gtar" ),

   { 0, 0, 0 }
};

static struct mimeentry mimetab_h[] = {
   MIME_ENTRY( "htc", "text/x-component" ),

   /*
   MIME_ENTRY( "hdf", "application/x-hdf" ),
   MIME_ENTRY( "hqx", "application/mac-binhex40" ),
   */

   { 0, 0, 0 }
};

/*
static struct mimeentry mimetab_i[] = {
   MIME_ENTRY( "ice", "x-conference/x-cooltalk" ),
   MIME_ENTRY( "ief", "image/ief" ),
   MIME_ENTRY( "ig",  "model/iges" ), (igs, iges) 
   { 0, 0, 0 }
};
*/

static struct mimeentry mimetab_j[] = {
   MIME_ENTRY( "json", "application/json" ),
   MIME_ENTRY( "jp",   "image/jpeg" ), /* (jpg, jpe, jpeg) */
   { 0, 0, 0 }
};

/*
static struct mimeentry mimetab_k[] = {
   MIME_ENTRY( "kar", "audio/midi" ),
   { 0, 0, 0 }
};

static struct mimeentry mimetab_l[] = {
   MIME_ENTRY( "latex", "application/x-latex" ),
   MIME_ENTRY( "lh",    "application/octet-stream" ), (lha, lhz)
   { 0, 0, 0 }
};
*/

static struct mimeentry mimetab_m[] = {
   MIME_ENTRY( "mng",      "video/x-mng" ),
   MIME_ENTRY( "mp4",      "video/mp4" ),
   MIME_ENTRY( "m4a",      "audio/mp4" ),
   MIME_ENTRY( "mp",       "video/mpeg" ), /* (mp2, mp3, mpg, mpe, mpeg, mpga) */
   MIME_ENTRY( "md5",      "text/plain" ),
   MIME_ENTRY( "mov",      "video/quicktime" ),
   MIME_ENTRY( "mf",       "text/cache-manifest" ),
   MIME_ENTRY( "manifest", "text/cache-manifest" ),

   /*
   MIME_ENTRY( "m3u",  "audio/x-mpegurl" ),
   MIME_ENTRY( "man",  "application/x-troff-man" ),
   MIME_ENTRY( "me",   "application/x-troff-me" ),
   MIME_ENTRY( "mesh", "model/mesh" ),
   MIME_ENTRY( "msh",  "model/mesh" ),
   MIME_ENTRY( "mif",  "application/vnd.mif" ),
   MIME_ENTRY( "mi",   "audio/midi" ), (mid, midi)
   MIME_ENTRY( "movie","video/x-sgi-movie" ),
   MIME_ENTRY( "ms",   "application/x-troff-ms" ),
   */

   { 0, 0, 0 }
};

/*
static struct mimeentry mimetab_n[] = {
   MIME_ENTRY( "nc", "application/x-netcdf" ),
   { 0, 0, 0 }
};
*/

static struct mimeentry mimetab_o[] = {
   MIME_ENTRY( "ogv", "video/ogg" ),
   MIME_ENTRY( "og",  "audio/ogg" ), /* (oga, ogg) */
   MIME_ENTRY( "otf", "font/opentype" ),

   /*
   MIME_ENTRY( "oda",  "application/oda" ),
   */

   { 0, 0, 0 }
};

static struct mimeentry mimetab_p[] = {
   MIME_ENTRY( "pdf", "application/pdf" ),
   MIME_ENTRY( "pem", "application/x-x509-ca-cert" ),
   MIME_ENTRY( "pbm", "image/x-portable-bitmap" ),
   MIME_ENTRY( "pgm", "image/x-portable-graymap" ),
   MIME_ENTRY( "pnm", "image/x-portable-anymap" ),
   MIME_ENTRY( "ppm", "image/x-portable-pixmap" ),
   MIME_ENTRY( "ps",  "application/postscript" ),
   MIME_ENTRY( "p7b", "application/x-pkcs7-certificates" ),
   MIME_ENTRY( "p7c", "application/x-pkcs7-mime" ),
   MIME_ENTRY( "p12", "application/x-pkcs12" ),
   MIME_ENTRY( "ppt", "application/vnd.ms-powerpoint" ),

   /*
   MIME_ENTRY( "pac", "application/x-ns-proxy-autoconfig" ),
   MIME_ENTRY( "pdb", "chemical/x-pdb" ),
   MIME_ENTRY( "pgn", "application/x-chess-pgn" ),
   */

   { 0, 0, 0 }
};

static struct mimeentry mimetab_q[] = {
   MIME_ENTRY( "qt", "video/quicktime" ),
   { 0, 0, 0 }
};

static struct mimeentry mimetab_r[] = {
   MIME_ENTRY( "rar",  "application/x-rar-compressed" ),
   MIME_ENTRY( "rtf",  "text/rtf" ),
   MIME_ENTRY( "ru",   "text/x-ruby" ),
   MIME_ENTRY( "rtx",  "text/richtext" ),
   MIME_ENTRY( "rdf",  "application/rdf+xml" ),
   MIME_ENTRY( "roff", "application/x-troff" ),
   MIME_ENTRY( "rss",  "application/rss+xml" ),

   /*
   MIME_ENTRY( "ra",  "audio/x-realaudio" ),
   MIME_ENTRY( "ras", "image/x-cmu-raster" ),
   MIME_ENTRY( "rgb", "image/x-rgb" ),
   MIME_ENTRY( "rpm", "audio/x-pn-realaudio-plugin" ),
   MIME_ENTRY( "ram", "audio/x-pn-realaudio" ),
   MIME_ENTRY( "rm",  "audio/x-pn-realaudio" ),
   */

   { 0, 0, 0 }
};

static struct mimeentry mimetab_s[] = {
   MIME_ENTRY( "svg",        "image/svg+xml" ), /* (svg, svgz) */
   MIME_ENTRY( "swf",        "application/x-shockwave-flash" ),
   MIME_ENTRY( "sgm",        "text/sgml" ), /* (sgml) */
   MIME_ENTRY( "sh",         "application/x-sh" ),
   MIME_ENTRY( "safariextz", "application/octet-stream" ),

   /*
   MIME_ENTRY( "snd",     "audio/basic" ),
   MIME_ENTRY( "shar",    "application/x-shar" ),
   MIME_ENTRY( "sit",     "application/x-stuffit" ),
   MIME_ENTRY( "silo",    "model/mesh" ),
   MIME_ENTRY( "sig",     "application/pgp-signature" ),
   MIME_ENTRY( "spl",     "application/x-futuresplash" ),
   MIME_ENTRY( "src",     "application/x-wais-source" ),
   MIME_ENTRY( "smi",     "application/smil" ), (smil)
   MIME_ENTRY( "sk",      "application/x-koan" ), (skd, skm, skp, skt)
   MIME_ENTRY( "sv4cpio", "application/x-sv4cpio" ),
   MIME_ENTRY( "sv4crc",  "application/x-sv4crc" ),
   */

   { 0, 0, 0 }
};

static struct mimeentry mimetab_t[] = {
   MIME_ENTRY( "tar",  "application/x-tar" ),
   MIME_ENTRY( "tgz",  "application/x-tar-gz" ),
   MIME_ENTRY( "tif",  "image/tiff" ), /* (tiff) */
   MIME_ENTRY( "ttf",  "font/truetype" ),
   MIME_ENTRY( "ttl",  "text/turtle" ),

   /*
   MIME_ENTRY( "texi",     "application/x-texinfo" ), (texinfo)
   MIME_ENTRY( "tex",      "application/x-tex" ),
   MIME_ENTRY( "tr",       "application/x-troff" ),
   MIME_ENTRY( "tcl",      "application/x-tcl" ),
   MIME_ENTRY( "tsv",      "text/tab-separated-values" ),
   MIME_ENTRY( "torrent",  "application/x-bittorrent" ),
   */

   { 0, 0, 0 }
};

/*
static struct mimeentry mimetab_u[] = {
   MIME_ENTRY( "untar",  "application/x-ustar" ),
   { 0, 0, 0 }
};

static struct mimeentry mimetab_v[] = {
   MIME_ENTRY( "vbxml", "application/vnd.wap.wbxml" ),
   MIME_ENTRY( "vcd",   "application/x-cdlink" ),
   MIME_ENTRY( "vmrl",  "model/vrml" ),
   { 0, 0, 0 }
};
*/

static struct mimeentry mimetab_w[] = {
   MIME_ENTRY( "wav",  "audio/x-wav" ),
   MIME_ENTRY( "wmv",  "video/x-ms-wmv" ),
   MIME_ENTRY( "webm", "video/webm" ),
   MIME_ENTRY( "woff", "application/x-font-woff" ),
   MIME_ENTRY( "webp", "image/webp" ),

   /*
   MIME_ENTRY( "wbmp",  "image/vnd.wap.wbmp" ),
   MIME_ENTRY( "wml",   "text/vnd.wap.wml" ),
   MIME_ENTRY( "wmls",  "text/vnd.wap.wmlscript" ),
   MIME_ENTRY( "wmlc",  "application/vnd.wap.wmlc" ),
   MIME_ENTRY( "wmlsc", "application/vnd.wap.wmlscriptc" ),
   MIME_ENTRY( "wrl",   "model/vrml" ),
   */

   { 0, 0, 0 }
};

static struct mimeentry mimetab_x[] = {
   MIME_ENTRY( "xml", "text/xml" ),
   MIME_ENTRY( "xsl", "text/xml" ),
   MIME_ENTRY( "xls", "application/vnd.ms-excel" ),
   MIME_ENTRY( "xpm", "image/x-xpixmap" ),
   MIME_ENTRY( "xbm", "image/x-xbitmap" ),
   MIME_ENTRY( "xpi", "application/x-xpinstall" ),
   MIME_ENTRY( "xrdf", "application/rdf+xml" ),

   /*
   MIME_ENTRY( "xyz", "chemical/x-pdb" ),
   MIME_ENTRY( "xwd", "image/x-xwindowdump" ),
   */

   { 0, 0, 0 }
};

static struct mimeentry mimetab_z[] = {
   MIME_ENTRY( "zip", "application/zip" ),
   { 0, 0, 0 }
};

static struct mimeentry mimetab_null[] = {
   { 0, 0, 0 }
};

const char* u_get_mimetype(const char* restrict suffix, int* pmime_index)
{
   static const int dispatch_table[] = {
      0,/* 'a' */
      (char*)&&case_b-(char*)&&case_a,/* 'b' */
      (char*)&&case_c-(char*)&&case_a,/* 'c' */
      (char*)&&case_d-(char*)&&case_a,/* 'd' */
      (char*)&&case_e-(char*)&&case_a,/* 'e' */
      (char*)&&cdefault-(char*)&&case_a,/* 'f' */
      (char*)&&case_g-(char*)&&case_a,/* 'g' */
      (char*)&&case_h-(char*)&&case_a,/* 'h' */
      (char*)&&cdefault-(char*)&&case_a,/* 'i' */
      (char*)&&case_j-(char*)&&case_a,/* 'j' */
      (char*)&&cdefault-(char*)&&case_a,/* 'k' */
      (char*)&&cdefault-(char*)&&case_a,/* 'l' */
      (char*)&&case_m-(char*)&&case_a,/* 'm' */
      (char*)&&cdefault-(char*)&&case_a,/* 'n' */
      (char*)&&case_o-(char*)&&case_a,/* 'o' */
      (char*)&&case_p-(char*)&&case_a,/* 'p' */
      (char*)&&case_q-(char*)&&case_a,/* 'q' */
      (char*)&&case_r-(char*)&&case_a,/* 'r' */
      (char*)&&case_s-(char*)&&case_a,/* 's' */
      (char*)&&case_t-(char*)&&case_a,/* 't' */
      (char*)&&cdefault-(char*)&&case_a,/* 'u' */
      (char*)&&cdefault-(char*)&&case_a,/* 'v' */
      (char*)&&case_w-(char*)&&case_a,/* 'w' */
      (char*)&&case_x-(char*)&&case_a,/* 'x' */
      (char*)&&cdefault-(char*)&&case_a,/* 'y' */
      (char*)&&case_z-(char*)&&case_a /* 'z' */
   };

   uint32_t i;
   struct mimeentry* ptr;

   U_INTERNAL_TRACE("u_get_mimetype(%s,%p)", suffix, pmime_index)

   U_INTERNAL_ASSERT_POINTER(suffix)

   i = u_get_unalignedp32(suffix);

   switch (i)
      {
      case U_MULTICHAR_CONSTANT32('c','s','s',0):
         {
         if (pmime_index) *pmime_index = U_css;

         return "text/css";
         }
      case U_MULTICHAR_CONSTANT32('f','l','v',0):
         {
         if (pmime_index) *pmime_index = U_flv;

         return "video/x-flv";
         }
      case U_MULTICHAR_CONSTANT32('g','i','f',0):
         {
         if (pmime_index) *pmime_index = U_gif;

         return "image/gif";
         }
      case U_MULTICHAR_CONSTANT32('i','c','o',0):
         {
         if (pmime_index) *pmime_index = U_ico;

         return "image/x-icon";
         }
      case U_MULTICHAR_CONSTANT32('p','n','g',0):
         {
         if (pmime_index) *pmime_index = U_png;

         return "image/png";
         }
      case U_MULTICHAR_CONSTANT32('j','p','g',0):
         {
         if (pmime_index) *pmime_index = U_jpg;

         return "image/jpg";
         }
      case U_MULTICHAR_CONSTANT32('s','h','t','m'):
      case U_MULTICHAR_CONSTANT32('h','t','m','l'):
         {
         if (pmime_index) *pmime_index = (i == U_MULTICHAR_CONSTANT32('h','t','m','l') ? U_html : U_ssi);

         return U_CTYPE_HTML;
         }
      case U_MULTICHAR_CONSTANT32('t','x','t',0):
      case U_MULTICHAR_CONSTANT32('u','s','p',0):
      case U_MULTICHAR_CONSTANT32('c','s','p',0):
      case U_MULTICHAR_CONSTANT32('c','g','i',0):
      case U_MULTICHAR_CONSTANT32('p','h','p',0):
         {
         if (pmime_index)
            {
            switch (i)
               {
               case U_MULTICHAR_CONSTANT32('t','x','t',0): *pmime_index = U_txt; break;
               case U_MULTICHAR_CONSTANT32('u','s','p',0): *pmime_index = U_usp; break;
               case U_MULTICHAR_CONSTANT32('c','s','p',0): *pmime_index = U_csp; break;
               case U_MULTICHAR_CONSTANT32('c','g','i',0): *pmime_index = U_cgi; break;
               case U_MULTICHAR_CONSTANT32('p','h','p',0): *pmime_index = U_php; return "application/x-httpd-php";
               }
            }

         return U_CTYPE_TEXT_WITH_CHARSET;
         }
      }

   switch (u_get_unalignedp16(suffix))
      {
      case U_MULTICHAR_CONSTANT16('g','z'):
         {
         if (pmime_index) *pmime_index = U_gz;

         return "application/x-gzip";
         }
      case U_MULTICHAR_CONSTANT16('p','l'):
         {
         if (pmime_index) *pmime_index = U_perl;

         return "text/x-script.perl";
         }
      case U_MULTICHAR_CONSTANT16('r','b'):
         {
         if (pmime_index) *pmime_index = U_ruby;

         return "text/x-ruby";
         }
      case U_MULTICHAR_CONSTANT16('p','y'):
         {
         if (pmime_index) *pmime_index = U_python;

         return "text/x-script.phyton";
         }
      }

   i = *suffix;

   if (i < 'a' || i > 'z') goto cdefault;

   goto *((char*)&&case_a + dispatch_table[i-'a']);

case_a: ptr = mimetab_a; goto loop;
case_b: ptr = mimetab_b; goto loop;
case_c: ptr = mimetab_c; goto loop;
case_d: ptr = mimetab_d; goto loop;
case_e: ptr = mimetab_e; goto loop;

case_g: ptr = mimetab_g; goto loop;
case_h: ptr = mimetab_h; goto loop;

case_j: ptr = mimetab_j; goto loop;

case_m: ptr = mimetab_m; goto loop;

case_o: ptr = mimetab_o; goto loop;
case_p: ptr = mimetab_p; goto loop;
case_q: ptr = mimetab_q; goto loop;
case_r: ptr = mimetab_r; goto loop;
case_s: ptr = mimetab_s; goto loop;
case_t: ptr = mimetab_t; goto loop;

case_w: ptr = mimetab_w; goto loop;
case_x: ptr = mimetab_x; goto loop;
case_z: ptr = mimetab_z; goto loop;

cdefault: ptr = mimetab_null;

loop:
   while (ptr->name)
      {
      U_INTERNAL_PRINT("mimetab = %p (%s,%u,%s)", ptr, ptr->name, ptr->name_len, ptr->type)

      for (i = 0; i < ptr->name_len; ++i)
         {
         if (suffix[i+1] != ptr->name[i])
            {
            ++ptr;

            goto loop;
            }
         }

      if (pmime_index) *pmime_index = U_know;

      return ptr->type;
      }

   if (u_get_unalignedp16(suffix) == U_MULTICHAR_CONSTANT16('j','s')) // NB: must be here because of conflit with .json
      {
      if (pmime_index) *pmime_index = U_js;

      return "application/javascript"; // RFC 4329 (2006) now recommends the use of application/javascript
      }

   if (pmime_index) *pmime_index = U_unknow;

   return 0;
}
