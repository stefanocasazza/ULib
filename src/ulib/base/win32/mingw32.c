/* mingw32.c - This code is designed to be built with the mingw compiler */

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/base.h>
#include <ulib/base/utility.h>

/* We need MS-defined signal and raise here */

#include <tlhelp32.h>
#include <ws2tcpip.h>
#include <mmsystem.h>
#include <sys/timeb.h>

#undef raise
#undef signal
#undef select
#undef rename
#undef unlink

/* void* _alloca(size_t size); */

HANDLE u_hProcess;
SECURITY_ATTRIBUTES sec_none;
SECURITY_DESCRIPTOR sec_descr;

static SYSTEM_INFO system_info;
static OSVERSIONINFOEX version;

/*
struct passwd {
   char* pw_name;   // username
   char* pw_passwd; // user password
   uid_t pw_uid;    // user ID
   gid_t pw_gid;    // group ID
   char* pw_gecos;  // real name
   char* pw_dir;    // home directory
   char* pw_shell;  // shell program
};
*/

static char passwd_any_name[256];
static struct passwd passwd_any = { passwd_any_name, (char*)"", 0, 0, (char*)"", (char*)"/root", (char*)"" };

/* Just pretend that everyone is a superuser. NT will let us know if we don't really have permission to do something */

uid_t getuid(void)      { return 0; }
uid_t geteuid(void)     { return 0; }
uid_t getegid(void)     { return 0; }
int   setuid(uid_t uid) { return (uid == 0 ? 0 : -1); }
int   setgid(gid_t gid) { return (gid == 0 ? 0 : -1); }

struct passwd* getpwuid(uid_t uid)
{
   U_INTERNAL_TRACE("getpwuid(%d)", uid)

   if (passwd_any_name[0] == '\0')
      {
      /* Obtain only logon id here, uid part is moved to getuid */

      char name[256];
      DWORD length = sizeof(name);

      u__strcpy(passwd_any_name, GetUserName(name, &length) ? name : "unknown");
      }

   return &passwd_any;
}

struct passwd* getpwnam(const char* name)
{
   U_INTERNAL_TRACE("getpwnam(%s)", name)

   return &passwd_any;
}

int inet_aton(const char* src, struct in_addr* addr)
{
   U_INTERNAL_TRACE("inet_aton(%s,%p)", src, addr)

   *(long unsigned int*)addr = (long unsigned int)inet_addr(src);

   return 1;
}

/*
*/
#if _WIN32_WINNT < 0x0600
const CHAR* inet_ntop(INT af, PVOID src, LPSTR dst, size_t size)
{
   U_INTERNAL_TRACE("inet_ntop(%d,%p,%s,%d)", af, src, dst, size)

   if (af == AF_INET)
      {
      struct sockaddr_in in;

      (void) memset(&in, 0, sizeof(in));

      in.sin_family = AF_INET;

      u__memcpy(&in.sin_addr, src, sizeof(struct in_addr), __PRETTY_FUNCTION__);

      getnameinfo((struct sockaddr*)&in, sizeof(struct sockaddr_in), dst, size, 0, 0, NI_NUMERICHOST);

      return dst;
      }

   if (af == AF_INET6)
      {
      struct sockaddr_in6 in;

      (void) memset(&in, 0, sizeof(in));

      in.sin6_family = AF_INET6;

      u__memcpy(&in.sin6_addr, src, sizeof(struct in_addr6), __PRETTY_FUNCTION__);

      getnameinfo((struct sockaddr*)&in, sizeof(struct sockaddr_in6), dst, size, NULL, 0, NI_NUMERICHOST);

      return dst;
      }

   return 0;
}
#endif

#define isWindow9x()  (version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
#define isWindowNT()  (version.dwPlatformId == VER_PLATFORM_WIN32_NT)
#define granularity() (system_info.dwAllocationGranularity)

void u_init_ulib_mingw(void)
{
   U_INTERNAL_TRACE("u_init_ulib_mingw()", 0)

   /* Figure out on which OS we're running */

   version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO); /* Request simple version info */

   GetVersionEx((LPOSVERSIONINFO)&version);

   U_INTERNAL_PRINT("dwPlatformId = %d dwMajorVersion = %d dwMinorVersion = %d",
                     version.dwPlatformId, version.dwMajorVersion, version.dwMinorVersion)

   GetSystemInfo(&system_info);

   U_INTERNAL_PRINT("system_info.dwAllocationGranularity = %d", system_info.dwAllocationGranularity)

/* (void) InitializeSecurityDescriptor(&sec_descr, SECURITY_DESCRIPTOR_REVISION); */

   sec_none.nLength              = sizeof(SECURITY_ATTRIBUTES);
   sec_none.bInheritHandle       = TRUE; /* Set the bInheritHandle flag so pipe handles are inherited */
   sec_none.lpSecurityDescriptor = 0;    /* &sec_descr; */
}

char* realpath(const char* name, char* resolved_path)
{
   U_INTERNAL_TRACE("realpath(%s,%s)", name, resolved_path)

   if (name == 0)
      {
      errno = EINVAL;

      return NULL;
      }

   if (name[0] == '\0')
      {
      errno = ENOENT;

      return NULL;
      }

   /* Make sure we can access it in the way we want */

   if (access(u_slashify(name, '/', '\\'), F_OK))
      {
      errno = ENOENT;

      return NULL;
      }

   if (strncmp(name, U_CONSTANT_TO_PARAM(".")) == 0) (void) u__strncpy(resolved_path, u_cwd, u_cwd_len);
   else
      {
      /* We can, so normalize the name and return it below */

      (void) u__strcpy(resolved_path, name);

      (void) u_canonicalize_pathname(resolved_path);
      }

   return resolved_path;
}

char* u_slashify(const char* src, char slash_from, char slash_to)
{
   static char u_slashify_buffer[PATH_MAX];

   char* dst = u_slashify_buffer;

   U_INTERNAL_TRACE("u_slashify(%s,%c,%c)", src, slash_from, slash_to)

   /**
    * Skip over the disk name in MSDOS pathnames
    *
    * if (u__isalpha(src[0]) && src[1] == ':') src += 2;
    */

   while (*src)
      {
      if (*src == slash_from) *dst++ = slash_to;
      else                    *dst++ = *src;

      ++src;
      }

   *dst = 0;

   return u_slashify_buffer;
}

int rename_w32(const char* oldpath, const char* newpath)
{
   int ok, oldatts, newatts;

   U_INTERNAL_TRACE("rename_w32(%s,%s)", oldpath, newpath)

   oldpath = U_PATH_CONV(oldpath);
   oldatts = GetFileAttributes(oldpath);

   if (oldatts == -1) return -1;

   oldpath = strdup(oldpath);
   newpath = U_PATH_CONV(newpath);
   newatts = GetFileAttributes(newpath);

   if (newatts != -1 &&
       newatts & FILE_ATTRIBUTE_READONLY)
      {
      /* Destination file exists and is read only, change that or else the rename won't work */

      (void) SetFileAttributesA(newpath, newatts & ~FILE_ATTRIBUTE_READONLY);
      }

   /**
    * NT4 (and up) provides a way to rename/move a file with similar semantics to what's usually done on UNIX
    * if there's an existing file with <newpath> it is removed before the file is renamed/moved.
    * The MOVEFILE_COPY_ALLOWED is specified to allow such a rename across drives
    */

   ok = ((isWindowNT() ? MoveFileExA(oldpath, newpath, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) :
                         MoveFile   (oldpath, newpath)) != 0);

   U_INTERNAL_PRINT("isWindowNT() = %d ok = %d", isWindowNT(), ok)

   if (ok == FALSE)
      {
      switch (GetLastError())
         {
         case ERROR_FILE_EXISTS:
         case ERROR_ALREADY_EXISTS:
            {
            /* try win95 hack */

            int i;
            for (i = 0; i < 2; ++i)
               {
               if (!DeleteFileA(newpath) &&
                   GetLastError() != ERROR_FILE_NOT_FOUND)
                  {
                  break;
                  }
               else if (MoveFile(oldpath, newpath))
                  {
                  ok = TRUE;

                  break;
                  }
               }
            }
         }
      }

   free((void*)oldpath);

   if (ok == FALSE) return -1;

   (void) SetFileAttributes(newpath, oldatts); /* Reset R/O attributes if neccessary */

   return 0;
}

int unlink_w32(const char* path)
{
   int ret, attrs = GetFileAttributes(path);

   U_INTERNAL_TRACE("unlink_w32(%s)", path)

   if (attrs == -1) return -1;

   /* Allow us to delete even if read-only */

   if (attrs & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM))
      {
      (void) SetFileAttributesA(path, attrs & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM));

      ret = unlink(path);

      if (ret == -1) (void) SetFileAttributesA(path, attrs);
      }
   else
      {
      ret = unlink(path);
      }

   U_INTERNAL_PRINT("ret = %d", ret)

   return ret;
}

int mkstemp(char* tmpl)
{
   int ret, iLen = u__strlen(tmpl, __PRETTY_FUNCTION__);

   U_INTERNAL_TRACE("mkstemp(%s)", tmpl)

   if (iLen >= 6)
      {
      char* pChr = tmpl + iLen - 6;

      srand(u_now->tv_sec);

      if (strncmp(pChr, "XXXXXX", 6) == 0)
         {
         int iChr;

         for (iChr = 0; iChr < 6; ++iChr)
            {
            int iRnd  = rand() / 528.5;

            *(pChr++) = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[iRnd > 0 ? iRnd - 1 : 0];
            }
         }
      else
         {
         errno = EINVAL;

         return -1;
         }
      }
   else
      {
      errno = EINVAL;

      return -1;
      }

   ret = _open(tmpl, _O_CREAT | _O_EXCL | _O_BINARY, _S_IREAD | _S_IWRITE);

   U_INTERNAL_PRINT("ret = %d", ret)

   return ret;
}

/**
 * INODE FOR WINDOWS - IT IS NOT STABLE FOR FILES ON NETWORK DRIVES (NFS)
 * -------------------------------------------------------------------------------------------------------------------------
 * Basically, the implementation of stat uses as inode number the FileIndex from the  BY_HANDLE_FILE_INFORMATION structure
 * returned by the Win32 API function  GetFileInformationByHandle. The FileIndex is a 64-bit number that indicates the
 * position of the file in the Master File Table (MFT). On Windows XP one can also obtain this number by using the command
 * fsutil usn readdata <path>. It is stable between successive starts of the system, provided the MFT does not overflow and
 * therefore has to be rebuilt. On WinNT systems (NT, 2K, XP) the FileIndex is also returned for directories, on Win9x
 * (95, 98, ME) it returns zero for directories.  For directories on 9x and network files, the stat uses a hashed value of
 * the full path of the file.
 * ***************************************************
 * IT IS NOT STABLE FOR FILES ON NETWORK DRIVES (NFS),
 * successive calls to GetFileInformationByHandle return different values.
 * ***********************************************************************
 * The FileIndex consists of two parts: the low 48 bits are the file record number and contain the actual index in the MFT;
 * the high 16 bits are the socalled sequence number: each time an entry in the MFT is reused for another file, the sequence
 * number is increased by one. This behavior of the sequence number can be observed by creating a file, printing its
 * fileindex, deleting it, creating a new file and printing its fileindex; the fileindex of the newest file is equal to that
 * of the first file, with the sequence number, in the left most part of the fileindex, increased by one. So the file
 * reference number appears to be the equivalent of the Unix inode
 * -------------------------------------------------------------------------------------------------------------------------
 */
 
#define HIDWORD(l)         ((DWORD)((UINT64)(l) >> 32))
#define LODWORD(w)         ((DWORD)((UINT64)(w) & 0xffffffff))
#define MAKEDWORDLONG(a,b) ((DWORDLONG)(((DWORD)(a))|(((DWORDLONG)((DWORD)(b)))<<32)))

uint64_t u_get_inode(int fd)
{
   uint64_t ino64;
   BY_HANDLE_FILE_INFORMATION FileInformation;
   HANDLE hFile = (HANDLE) _get_osfhandle(fd); /* obtain handle from file descriptor "fd" */

   U_INTERNAL_TRACE("u_get_inode(%d)", fd)

   U_INTERNAL_ASSERT_DIFFERS(fd, -1)

   U_INTERNAL_PRINT("hFile = %ld", hFile)

   ZeroMemory(&FileInformation, sizeof(FileInformation));

   if (!GetFileInformationByHandle(hFile, &FileInformation)) return 0;

   ino64  = (uint64_t) MAKEDWORDLONG(FileInformation.nFileIndexLow, FileInformation.nFileIndexHigh);
   ino64 &= ((~(0ULL)) >> 16); /* remove sequence number */

   U_INTERNAL_PRINT("ino64 = %lu", ino64)

   return ino64;
}

/*--------------------------------------------------------------------*/
/* Not implemented                                                    */
/*--------------------------------------------------------------------*/

pid_t fork(void)
{
   U_INTERNAL_TRACE("fork()")

   errno = ENOSYS;

   return -1;
}

/*
pid_t vfork(void)
{
   U_INTERNAL_TRACE("vfork()")

   errno = ENOSYS;

   return -1;
}
*/

int nice(int inc)
{
   U_INTERNAL_TRACE("nice(%d)", inc)

   errno = ENOSYS;

   return -1;
}

pid_t getppid(void)
{
   U_INTERNAL_TRACE("getppid()")

   errno = ENOSYS;

   return -1;
}

int setrlimit(int resource, const struct rlimit* rlim)
{
   U_INTERNAL_TRACE("setrlimit(%d,%p)", resource, rlim)

   errno = ENOSYS;

   return -1;
}

int socketpair(int d, int type, int protocol, int sv[2])
{
   U_INTERNAL_TRACE("socketpair(%d,%d,%d,%p)", d, type, protocol, sv)

   errno = ENOSYS;

   return -1;
}

int setpgrp(void)
{
   U_INTERNAL_TRACE("setpgrp()")

   errno = ENOSYS;

   return -1;
}

int setpgid(pid_t pid, pid_t pgid)
{
   U_INTERNAL_TRACE("setpgid(%d,%d)", pid, pgid)

   errno = ENOSYS;

   return -1;
}

/*--------------------------------------------------------------------*/
/* Signal support                                                     */
/*--------------------------------------------------------------------*/

int sigsuspend(const sigset_t* mask)
{
   U_INTERNAL_TRACE("sigsuspend(%p)", mask)

   errno = ENOSYS;

   return -1;
}

#define sigmask(nsig) (1U << nsig)

/* Signal block mask: bit set to 1 means blocked */
static sigset_t signal_block_mask = 0;

/* Signal pending mask: bit set to 1 means sig is pending */
static sigset_t signal_pending_mask = 0;

/* Signal handlers. Initial value = (0 -> SIG_DFL) */
static sighandler_t signal_handlers[NSIG] = { 0 };

sighandler_t signal_w32(int nsig, sighandler_t handler)
{
   sighandler_t old_handler;

   U_INTERNAL_TRACE("signal_w32(%d,%p)", nsig, handler)

   /**
    * We delegate some signals to the system function.
    * The SIGILL, SIGSEGV, and SIGTERM signals are not generated under Windows NT.
    * They are included for ANSI compatibility. Thus you can set signal handlers for these signals with signal,
    * and you can also explicitly generate these signals by calling raise().
    *
    * SIGINT is not supported for any Win32 application, including Windows 98/Me and Windows NT/2000/XP.
    * When a CTRL+C interrupt occurs, Win32 operating systems generate a new thread to specifically handle that interrupt.
    * This can cause a single-thread application such as UNIX, to become multithreaded, resulting in unexpected behavior
    */

   switch (nsig)
      {
      case SIGINT:    /*  2 Interactive attention */
      case SIGILL:    /*  4 Illegal instruction */
      case SIGFPE:    /*  8 Floating point error */
      case SIGSEGV:   /* 11 Core Invalid memory reference */
      case SIGTERM:   /* 15 Termination request */
      case SIGABRT:   /* 22 Abnormal termination (abort) */
   /* case SIGBREAK:     21 Control-break */
         {
         old_handler = signal(nsig, handler);
         }
      break;

      default:
         {
         if (nsig < 0 || nsig >= NSIG)
            {
            errno = EINVAL;

            return NULL;
            }

         /* Store handler ptr */

         old_handler = signal_handlers[nsig];

         signal_handlers[nsig] = handler;
         }
      }

   U_INTERNAL_PRINT("old_handler = %p", old_handler)

   return old_handler;
}

int raise_w32(int nsig)
{
   U_INTERNAL_TRACE("raise_w32(%d)", nsig)

   /* We delegate some raises to the system routine */

   switch (nsig)
      {
      case 0: return 0; /* Test the existence of process */

      case SIGINT:    /*  2 Interactive attention */
      case SIGILL:    /*  4 Illegal instruction */
      case SIGFPE:    /*  8 Floating point error */
      case SIGSEGV:   /* 11 Segmentation violation */
      case SIGTERM:   /* 15 Termination request */
      case SIGABRT:   /* 22 Abnormal termination (abort) */
   /* case SIGBREAK:     21 Control-break */
         {
         return raise(nsig);
         }
      break;

      default:
         {
         if (nsig < 1 || nsig >= NSIG)
            {
            errno = EINVAL;

            return -1;
            }

         /* If the signal is blocked, remember to issue later */

         if (signal_block_mask & sigmask(nsig))
            {
            signal_pending_mask |= sigmask(nsig);

            return 0;
            }

         if (signal_handlers[nsig] == SIG_IGN) return 0;

         if (signal_handlers[nsig] != SIG_DFL)
            {
            signal_handlers[nsig](nsig);

            return 0;
            }

         /* By default, signal terminates the calling program with exit code 3, regardless of the value of sig */

         exit(3);
         }
      break;
      }

   /* Other signals are ignored by default */

   U_INTERNAL_PRINT("ret = %d", 0)

   return 0;
}

int sigaction(int signum, const struct sigaction* act, struct sigaction* oldact)
{
   int ret;
   struct sigaction sa;

   U_INTERNAL_TRACE("sigaction(%d,%p,%p)", signum, act, oldact)

   if (signum == 0) return 0;

   if (oldact == 0) oldact = &sa;

   if (act == 0)
      {
      oldact->sa_handler = signal_w32(signum, SIG_IGN);

      signal_w32(signum, oldact->sa_handler);
      }
   else
      {
      oldact->sa_handler = signal_w32(signum, act->sa_handler);
      }

   ret = (oldact->sa_handler == SIG_ERR ? -1 : 0);

   U_INTERNAL_PRINT("ret = %d", ret)

   return ret;
}

/**
 * If SET is not NULL, modify the current set of blocked signals
 * according to HOW, which may be SIG_BLOCK, SIG_UNBLOCK or SIG_SETMASK.
 * If OSET is not NULL, store the old set of blocked signals in *OSET
 */

int sigprocmask(int how, const sigset_t* set, sigset_t* oldset)
{
   U_INTERNAL_TRACE("sigprocmask(%d,%p,%p)", how, set, oldset)

   if (oldset) *oldset = signal_block_mask;

   if (set)
      {
      sigset_t newmask = signal_block_mask;

      switch (how)
         {
         case SIG_BLOCK:   newmask |=  *set; break; /* add set to current mask */
         case SIG_UNBLOCK: newmask &= ~*set; break; /* remove set from current mask */
         case SIG_SETMASK: newmask  =  *set; break; /* just set it */

         default:
            {
            errno = EINVAL;

            return -1;
            }
         }

      if (signal_block_mask != newmask)
         {
         signal_block_mask = newmask;

         if (signal_pending_mask)
            {
            unsigned i;

            for (i = 0; i < NSIG; ++i)
               {
               if (signal_pending_mask & sigmask(i))
                  {
                  signal_pending_mask &= ~sigmask(i);

                  raise_w32(i);
                  }
               }
            }
         }
      }

   U_INTERNAL_PRINT("ret = %d", 0)

   return 0;
}

int sigpending(sigset_t* set)
{
   U_INTERNAL_TRACE("sigpending(%p)", set)

   if (set == 0)
      {
      errno = EFAULT;

      return -1;
      }

   *set = signal_pending_mask;

   U_INTERNAL_PRINT("ret = %d", 0)

   return 0;
}

static int handle_kill_result(HANDLE h)
{
   BOOL bclose;

   U_INTERNAL_TRACE("handle_kill_result(%d)", h)

   if      (GetLastError() == ERROR_ACCESS_DENIED) errno = EPERM;
   else if (GetLastError() == ERROR_NO_MORE_FILES) errno = ESRCH;
   else                                            errno = EINVAL;

   bclose = CloseHandle(h);

   U_INTERNAL_PRINT("bclose = %b", bclose)

   U_INTERNAL_ASSERT(bclose)

   return -1;
}

/**
 * Send signal sig to process number pid. If pid is zero,
 * send sig to all processes in the current process's process group.
 * If pid is < -1, send sig to all processes in process group - pid
 */

int kill(pid_t pid, int sig)
{
   BOOL bclose;
   DWORD thread_id;
   HANDLE h, h_thread;
   PROCESSENTRY32 pe32;

   U_INTERNAL_TRACE("kill(%ld,%d)", pid, sig)

   if (pid <= 0 || sig < 0)
      {
      errno = EINVAL;

      return -1;
      }

   if (pid == getpid()) return raise_w32(sig);

   if (sig == 0) /* we just wanted to know if the process exists */
      {
      h = CreateToolhelp32Snapshot(0, pid);

      if (h == INVALID_HANDLE_VALUE)
         {
         errno = ESRCH;

         return -1;
         }

      pe32.dwSize = sizeof(PROCESSENTRY32);

      if (!Process32First(h, &pe32)) return handle_kill_result(h);

      return 0;
      }

   h = OpenProcess(sig == 0 ? PROCESS_QUERY_INFORMATION|PROCESS_VM_READ : PROCESS_ALL_ACCESS, FALSE, (DWORD)pid);

   if (h == 0)
      {
      errno = ESRCH;

      return -1;
      }

   switch (sig)
      {
      case SIGINT:
         if (!GenerateConsoleCtrlEvent(CTRL_C_EVENT, (DWORD)pid)) return handle_kill_result(h);
      break;

      case SIGQUIT:
         if (!GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, (DWORD)pid)) return handle_kill_result(h);
      break;

      /* Kill the process associated with process handle */

      case SIGABRT:
      case SIGKILL:
         if (!TerminateProcess(h, sig)) return handle_kill_result(h);
      break;

      default:
         {
         h_thread = CreateRemoteThread(h, NULL, 0, (LPTHREAD_START_ROUTINE)(GetProcAddress(GetModuleHandleA("KERNEL32.DLL"), "ExitProcess")),
                                       0, 0, &thread_id);

         if (h_thread) WaitForSingleObject(h_thread, 5);
         else          return handle_kill_result(h);
         }
      break;
      }

   bclose = CloseHandle(h);

   U_INTERNAL_PRINT("bclose = %b", bclose)

   U_INTERNAL_ASSERT(bclose)

   return 0;
}

#if (_WIN32_WINNT < 0x0500)
WINBASEAPI BOOL WINAPI GetFileSizeEx(HANDLE,PLARGE_INTEGER);
#endif

/*
int truncate(const char* fname, off_t length)
{
   BOOL bclose;
   HANDLE hFile;
   LARGE_INTEGER fileSize;

   U_INTERNAL_TRACE("truncate(%s,%ld)", fname, length)

   if (length < 0)
      {
      errno = EINVAL;

      return -1;
      }

   hFile = CreateFileA(fname, GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);

   if (hFile == INVALID_HANDLE_VALUE)
      {
      errno = EACCES;

      return -1;
      }

   if (GetFileSizeEx(hFile, &fileSize) == 0)
      {
      bclose = CloseHandle(hFile);

      U_INTERNAL_PRINT("bclose = %b", bclose)

      U_INTERNAL_ASSERT(bclose)

      errno = EACCES;

      return -1;
      }

   if (fileSize.QuadPart < length)
      {
      bclose = CloseHandle(hFile);

      U_INTERNAL_PRINT("bclose = %b", bclose)

      U_INTERNAL_ASSERT(bclose)

      errno = EINVAL;

      return -1;
      }

   fileSize.QuadPart = length;

   if (SetFilePointerEx(hFile, fileSize, 0, FILE_BEGIN) == 0 || SetEndOfFile(hFile) == 0)
      {
      bclose = CloseHandle(hFile);

      U_INTERNAL_PRINT("bclose = %b", bclose)

      U_INTERNAL_ASSERT(bclose)

      errno = EACCES;

      return -1;
      }

   bclose = CloseHandle(hFile);

   U_INTERNAL_PRINT("bclose = %b", bclose)

   U_INTERNAL_ASSERT(bclose)

   return 0;
}

int fsync(int fd)
{
   HANDLE h = (HANDLE) _get_osfhandle(fd);

   U_INTERNAL_TRACE("fsync(%d)", fd)

   if (fd < 0) return -1;

   U_INTERNAL_PRINT("h = %p", h)

   if (h == INVALID_HANDLE_VALUE) return -1;

   if (!FlushFileBuffers(h)) return -1;

   return 0;
}
*/

/**
 * Map addresses starting near ADDR and extending for LEN bytes.
 * From OFFSET into the file FD describes according to PROT and FLAGS.
 * If ADDR is nonzero, it is the desired mapping address. If the MAP_FIXED
 * bit is set in FLAGS, the mapping will be at ADDR exactly (which must be
 * page-aligned); otherwise the system chooses a convenient nearby address.
 * The return value is the actual mapping address chosen or MAP_FAILED for
 * errors (in which case `errno' is set). A successful `mmap' call deallocates
 * any previous mapping for the affected region
 */

#ifndef SECTION_MAP_EXECUTE_EXPLICIT
/* not defined in the February 2003 version of the Platform SDK */
#define SECTION_MAP_EXECUTE_EXPLICIT 0x0020
#endif

#ifndef FILE_MAP_EXECUTE
/* not defined in the February 2003 version of the Platform SDK */ 
#define FILE_MAP_EXECUTE SECTION_MAP_EXECUTE_EXPLICIT
#endif

static inline int mapProtFlags(int flags, DWORD* dwAccess)
{
   U_INTERNAL_TRACE("mapProtFlags(%d,%p)", flags, dwAccess)

   if ((flags & PROT_READ) == PROT_READ)
      {
      if ((flags & PROT_WRITE) == PROT_WRITE)
         {
         *dwAccess = FILE_MAP_WRITE;

         if ((flags & PROT_EXEC) == PROT_EXEC) return PAGE_EXECUTE_READWRITE;

         return PAGE_READWRITE;
         }

      if ((flags & PROT_EXEC) == PROT_EXEC)
         {
         *dwAccess = FILE_MAP_EXECUTE;

         return PAGE_EXECUTE_READ;
         }

      *dwAccess = FILE_MAP_READ;

      return PAGE_READONLY;
      }

   if ((flags & PROT_WRITE) == PROT_WRITE)
      {
      *dwAccess = FILE_MAP_COPY;

      return PAGE_WRITECOPY;
      }   

   if ((flags & PROT_EXEC) == PROT_EXEC)
      {
      *dwAccess = FILE_MAP_EXECUTE;

      return PAGE_EXECUTE_READ;
      }

   *dwAccess = 0;

   return 0;   
}

struct mmapInfos {
   void* start; /* ptr returned by MapViewOfFile */
   HANDLE hMap; /* handle returned by CreateFileMapping */

   /* the duplicated handle fd
   HANDLE hdupFile;
   */
};

static CRITICAL_SECTION cs;
static struct mmapInfos* g_mmapInfos;
static int g_curMMapInfos, g_maxMMapInfos;

void* mmap(void* start, size_t length, int prot, int flags, int fd, off_t offset)
{
   BOOL bclose;
   HANDLE hFile;
   int mmlen;
   caddr_t gran_addr;
   struct mmapInfos* mmi;
   DWORD dwAccess, flProtect;
   off_t gran_offset, filelen;

   U_INTERNAL_TRACE("mmap(%p,%ld,%d,%d,%d,%ld)", start, length, prot, flags, fd, offset)

   flProtect = mapProtFlags(prot, &dwAccess);

   if (flProtect == 0)
      {
      errno = EINVAL;

      return MAP_FAILED;
      }

   if (g_maxMMapInfos == 0)
      {
      InitializeCriticalSection(&cs);

      g_mmapInfos = (struct mmapInfos*) calloc((g_maxMMapInfos = 10), sizeof(struct mmapInfos));
      }

   EnterCriticalSection(&cs);

   for (g_curMMapInfos = 0; g_curMMapInfos < g_maxMMapInfos; ++g_curMMapInfos) if (g_mmapInfos[g_curMMapInfos].start == 0) break;

   U_INTERNAL_PRINT("g_curMMapInfos = %d g_maxMMapInfos = %d", g_curMMapInfos, g_maxMMapInfos)

   if (g_curMMapInfos == g_maxMMapInfos)
      {
      int i;

      g_maxMMapInfos += 10;
      g_mmapInfos     = (struct mmapInfos*) realloc(g_mmapInfos, g_maxMMapInfos * sizeof(struct mmapInfos));

      for (i = g_maxMMapInfos; i < g_maxMMapInfos; ++i) g_mmapInfos[i].start = 0;
      }

   mmi           = &(g_mmapInfos[g_curMMapInfos]);
/* mmi->hdupFile = INVALID_HANDLE_VALUE; */

   LeaveCriticalSection(&cs);

   if (flags & MAP_PRIVATE)
      {
                        dwAccess  = FILE_MAP_COPY;
      if (isWindow9x()) flProtect = PAGE_WRITECOPY;
      }

   U_INTERNAL_PRINT("Addr   before: %p",  start)
   U_INTERNAL_PRINT("Offset before: %ld", offset)

   if (flags & MAP_FIXED)
      {
      gran_addr   = (caddr_t) start;
      gran_offset = offset;
      }
   else
      {
      gran_addr   = (caddr_t) (((DWORD) start / granularity()) * granularity());
      gran_offset = offset & ~(granularity() - 1);
      }

   U_INTERNAL_PRINT("Addr    after: %p",  gran_addr)
   U_INTERNAL_PRINT("Offset  after: %ld", gran_offset)

   if (fd == -1)
      {
      U_INTERNAL_ASSERT_EQUALS(offset,0)
      U_INTERNAL_ASSERT(flags & MAP_ANONYMOUS)

      hFile = INVALID_HANDLE_VALUE;

      /* Map always in multipliers of `granularity'-sized chunks */

      mmlen = ((length + granularity()) / granularity()) * granularity();
      }
   else
      {
      hFile = (HANDLE) _get_osfhandle(fd);

      U_INTERNAL_PRINT("hFile = %p", hFile)

      if (hFile == INVALID_HANDLE_VALUE) return MAP_FAILED;

      /*
      if (!DuplicateHandle(GetCurrentProcess(), hFile, GetCurrentProcess(), &mmi->hdupFile, 0, FALSE, DUPLICATE_SAME_ACCESS)) return MAP_FAILED;

      U_INTERNAL_PRINT("mmi->hdupFile = %p", mmi->hdupFile)
      */

      filelen = _filelength(fd);

      U_INTERNAL_PRINT("filelen = %ld", filelen)

      mmlen = (filelen < gran_offset + length ? filelen - gran_offset : length);
      }

   U_INTERNAL_PRINT("mmlen = %d", mmlen)

   mmi->hMap = CreateFileMapping(hFile, &sec_none, flProtect, 0, mmlen, NULL);

   U_INTERNAL_PRINT("mmi->hMap = %p", mmi->hMap)

   if (mmi->hMap == 0)
      {
      /*
      bclose = CloseHandle(mmi->hdupFile);

      U_INTERNAL_PRINT("bclose = %b", bclose)

      U_INTERNAL_ASSERT(bclose)
      */

      errno = EACCES;

      return MAP_FAILED;
      }

   mmi->start = MapViewOfFileEx(mmi->hMap, dwAccess,
                              /* HIDWORD(gran_offset), */ 0,
                              /* LODWORD(gran_offset), */ gran_offset,
                              (SIZE_T)mmlen, (LPVOID)gran_addr);

   U_INTERNAL_PRINT("mmi->start = %p", mmi->start)

   if (mmi->start == 0 &&
       (flags & MAP_FIXED))
      {
      U_INTERNAL_PRINT("Starting address: %p", (LPVOID) gran_addr)

      mmi->start = MapViewOfFileEx(mmi->hMap, dwAccess,
                                 /* HIDWORD(gran_offset), */ 0,
                                 /* LODWORD(gran_offset), */ gran_offset,
                                 (SIZE_T)mmlen, (LPVOID)NULL);

      U_INTERNAL_PRINT("mmi->start = %p", mmi->start)
      }

   if (mmi->start == 0)
      {
      DWORD dwLastErr = GetLastError();

      bclose = CloseHandle(mmi->hMap);

      U_INTERNAL_ASSERT(bclose)

      U_INTERNAL_PRINT("bclose = %b", bclose)

      /*
      bclose = CloseHandle(mmi->hdupFile);

      U_INTERNAL_PRINT("bclose = %b", bclose)

      U_INTERNAL_ASSERT(bclose)
      */

      errno = (dwLastErr == ERROR_MAPPED_ALIGNMENT ? EINVAL : EACCES);

      return MAP_FAILED;
      }

   return mmi->start;
}

/**
 * Deallocate any mapping for the region starting at ADDR and extending LEN bytes.
 *
 * Returns 0 if successful, -1 for errors (and sets errno)
 */

int munmap(void* start, size_t length)
{
   BOOL bclose;

   U_INTERNAL_TRACE("munmap(%p,%ld)", start, length)

   U_INTERNAL_ASSERT_POINTER(g_mmapInfos)

   U_INTERNAL_PRINT("g_curMMapInfos = %d g_maxMMapInfos = %d g_mmapInfos[g_curMMapInfos].start = %p",
                     g_curMMapInfos,     g_maxMMapInfos,     g_mmapInfos[g_curMMapInfos].start)

   if (g_mmapInfos[g_curMMapInfos].start != start)
      {
      for (g_curMMapInfos = 0; g_curMMapInfos < g_maxMMapInfos; ++g_curMMapInfos)
         {
         U_INTERNAL_PRINT("g_mmapInfos[%d].start = %p", g_curMMapInfos, g_mmapInfos[g_curMMapInfos].start)

         if (g_mmapInfos[g_curMMapInfos].start == start) break;
         }

      if (g_curMMapInfos == g_maxMMapInfos)
         {
         errno = EINVAL;

         return -1;
         }
      }

   U_INTERNAL_ASSERT_EQUALS(g_mmapInfos[g_curMMapInfos].start, start)

   UnmapViewOfFile(start);

   U_INTERNAL_PRINT("g_mmapInfos[%d].hMap     = %p", g_curMMapInfos, g_mmapInfos[g_curMMapInfos].hMap)
/* U_INTERNAL_PRINT("g_mmapInfos[%d].hdupFile = %p", g_curMMapInfos, g_mmapInfos[g_curMMapInfos].hdupFile) */

   bclose = CloseHandle(g_mmapInfos[g_curMMapInfos].hMap);

   U_INTERNAL_PRINT("bclose = %b", bclose)

   U_INTERNAL_ASSERT(bclose)

   /*
   bclose = CloseHandle(g_mmapInfos[g_curMMapInfos].hdupFile);

   U_INTERNAL_PRINT("bclose = %b", bclose)

   U_INTERNAL_ASSERT(bclose)
   */

/* EnterCriticalSection(&cs); */

   g_mmapInfos[g_curMMapInfos].hMap     = 0;
   g_mmapInfos[g_curMMapInfos].start    = 0;
/* g_mmapInfos[g_curMMapInfos].hdupFile = 0; */

/* LeaveCriticalSection(&cs); */

   SetLastError(0);

   return 0;
}

/**
 * Synchronize the region starting at ADDR and extending LEN bytes with the file it maps.
 * Filesystem operations on a file being mapped are unpredictable before this is done
 */

int msync(void* start, size_t length, int flags)
{
   int ret = (FlushViewOfFile(start, length) ? 0 : -1);

   U_INTERNAL_TRACE("msync(%p,%ld,%d)", start, length, flags)

   U_INTERNAL_PRINT("ret = %d", ret)

   return ret;
}

/**
 * implemented in MINGW Runtime
 *
 * int gettimeofday(struct timeval* tv, void* tz)
 * {
 * U_INTERNAL_TRACE("gettimeofday(%p,%p)", tv, tz)
 *
 * struct _timeb theTime;
 *
 * _ftime(&theTime);
 *
 * tv->tv_sec  = theTime.time;
 * tv->tv_usec = theTime.millitm * 1000;
 *
 * U_INTERNAL_PRINT("ret = %d", 0)
 *
 * return 0;
 * }
 */

static int is_fh_socket(HANDLE fh)
{
   char sockbuf[80];
   int result = TRUE, optlen = sizeof(sockbuf);
   int retval = getsockopt((SOCKET)fh, SOL_SOCKET, SO_TYPE, sockbuf, &optlen);

   U_INTERNAL_TRACE("is_fh_socket(%p)", fh)

   U_INTERNAL_PRINT("retval = %d", retval)

   U_INTERNAL_ASSERT_DIFFERS(fh, INVALID_HANDLE_VALUE)

   if (retval == SOCKET_ERROR)
      {
      int iRet = WSAGetLastError();

      U_INTERNAL_PRINT("iRet = %d", iRet)

      if (iRet == WSAENOTSOCK ||
          iRet == WSAEBADF    ||
          iRet == WSANOTINITIALISED)
         {
         result = FALSE;
         }
      }

   U_INTERNAL_PRINT("ret = %d", result)

   return result;
}

static int is_fd_socket(int fd)
{
   SOCKET h;
   int result;

   U_INTERNAL_TRACE("is_fd_socket(%d)", fd)

   result = is_fh_socket((HANDLE)h);

   if (result == FALSE)
      {
      SOCKET h = (SOCKET) _get_osfhandle(fd);

      U_INTERNAL_PRINT("h = %p", h)

      if ((HANDLE)h == INVALID_HANDLE_VALUE) result = FALSE;
      }

next:
   U_INTERNAL_PRINT("ret = %d", result)

   return result;
}

ssize_t writev(int fd, const struct iovec* iov, int count)
{
   int i;
   char* buf;
   char* ptr;
   ssize_t result;
   size_t length = 0;

   U_INTERNAL_ASSERT_DIFFERS(is_fd_socket(fd), TRUE)

   /* Determine the total length of all the buffers in <iov> */

   for (i = 0; i < count; ++i) length += iov[i].iov_len;

   ptr = buf = (char*) malloc(length);

   for (i = 0; i < count; ++i)
      {
      if (iov[i].iov_len)
         {
         u__memcpy(ptr, iov[i].iov_base, iov[i].iov_len, __PRETTY_FUNCTION__);

         ptr += iov[i].iov_len;
         }
      }

   result = write(fd, buf, length);

   if (result > 0) goto next;

   if (WriteFile((HANDLE)_get_osfhandle(fd), buf, length, (DWORD*)&result, 0) == FALSE) result = -1; 

next:
   free(buf);

   U_INTERNAL_PRINT("result = %d", result)

   return result;
}

/**
 * Create a one-way communication channel (__pipe). If successful, two file descriptors are stored in PIPEDES;
 * bytes written on PIPEDES[1] can be read from PIPEDES[0]. Returns 0 if successful, -1 if not
 */

int pipe(int filedes[2])
{
   int ret = _pipe(filedes, 0, _O_BINARY | _O_NOINHERIT);

   U_INTERNAL_TRACE("pipe(%d,%d)", filedes[0], filedes[1])

   U_INTERNAL_PRINT("ret = %d", ret)

   return ret;
}

/**
 * Wait for a child matching PID to die.
 * If PID is greater than 0, match any process whose process ID is PID.
 * If PID is (pid_t) -1, match any process.
 * If PID is (pid_t)  0, match any process with the same process group as the current process.
 * If PID is less than -1, match any process whose process group is the absolute value of PID.
 * If the WNOHANG   bit is set in OPTIONS, and that child is not already dead, return (pid_t) 0.
 * If the WUNTRACED bit is set in OPTIONS, return status for stopped children; otherwise don't.
 * If successful, return PID and store the dead child's status in STAT_LOC. Return (pid_t) -1 for errors
 */

pid_t waitpid(pid_t pid, int* stat_loc, int options)
{
   BOOL ok;
   BOOL bclose;
   DWORD status;
   HANDLE hProcess;

   U_INTERNAL_TRACE("waitpid(%ld,%p,%d)", pid, stat_loc, options)

   if (pid == 0 || (options & ~(WNOHANG | WUNTRACED)) != 0)
      {
      errno = EINVAL;

      return (pid_t) -1;
      }

   if ((pid == -1) ||
       (pid == -2))
      {
      errno = ECHILD;

      return (pid_t) -1;
      }

   ok = FALSE;

   if (u_hProcess)
      {
      U_INTERNAL_PRINT("u_hProcess = %ld", u_hProcess)

      hProcess = u_hProcess;
                 u_hProcess = 0;
      }
   else
      {
      hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)pid);
      }

   if (hProcess)
      {
      DWORD rc = WaitForSingleObject(hProcess, INFINITE);

      U_INTERNAL_PRINT("hProcess = %ld WaitForSingleObject() = %ld", hProcess, rc)

      if (rc != WAIT_FAILED)
         {
         ok = GetExitCodeProcess(hProcess, (LPDWORD)&status);

         U_INTERNAL_PRINT("GetExitCodeProcess() = %d status = %d", ok, status)

         if (stat_loc) *stat_loc = status;
         }

      bclose = CloseHandle(hProcess);

      U_INTERNAL_PRINT("bclose = %b", bclose)

      U_INTERNAL_ASSERT(bclose)
      }

   return (ok ? pid : (pid_t) -1);
}

HANDLE is_pipe(int fd)
{
   HANDLE h = (HANDLE) _get_osfhandle(fd);

   U_INTERNAL_TRACE("is_pipe(%d)", fd)

   if (h != INVALID_HANDLE_VALUE        &&
       GetFileType(h) == FILE_TYPE_PIPE && /* The specified file is a socket, a named pipe, or an anonymous pipe */
       is_fh_socket(h)  == FALSE)
      {
      return h;
      }

   return INVALID_HANDLE_VALUE;
}

/*
typedef struct fd_set {
   u_int  fd_count;
   SOCKET fd_array[FD_SETSIZE];
} fd_set;
*/

#ifdef DEBUG
static int extract_file_fd(fd_set* set, fd_set* fileset)
{
   U_INTERNAL_TRACE("extract_file_fd(%p,%p)", set, fileset)

   if (set)
      {
      u_int i, idx;

      fileset->fd_count = 0;

      U_INTERNAL_PRINT("set->fd_count = %d", set->fd_count)

      for (idx = 0; idx < set->fd_count; ++idx)
         {
         int fd = set->fd_array[idx];

         if (is_fd_socket(fd) == FALSE)
            {
            for (i = 0; i < fileset->fd_count; ++i)
               {
               if (fileset->fd_array[i] == fd) break;
               }

            if (i == fileset->fd_count)
               {
               U_INTERNAL_ASSERT_MINOR(fileset->fd_count, FD_SETSIZE)

               fileset->fd_array[i] = fd;

               fileset->fd_count++;
               }
            }
         }

      U_INTERNAL_PRINT("fileset->fd_count = %d", fileset->fd_count)

      return fileset->fd_count;
      }

   return 0;
}
#endif

static fd_set* fdset_fd2sock(fd_set* set, fd_set* fileset)
{
   U_INTERNAL_TRACE("fdset_fd2sock(%p,%p)", set, fileset)

   if (set)
      {
      SOCKET h;
      u_int idx;

      fileset->fd_count = set->fd_count;

      U_INTERNAL_PRINT("set->fd_count = %d", set->fd_count)

      for (idx = 0; idx < set->fd_count; ++idx)
         {
         int fd = set->fd_array[idx];

         h = (SOCKET) _get_osfhandle(fd);

      // U_INTERNAL_ASSERT_DIFFERS((HANDLE)h, INVALID_HANDLE_VALUE)
      // U_INTERNAL_ASSERT_EQUALS(is_fh_socket((HANDLE)h), TRUE)

         U_INTERNAL_PRINT("fd = %d h = %p", fd, h)

         fileset->fd_array[idx] = ((HANDLE)h == INVALID_HANDLE_VALUE ? fd : h);
         }

      return fileset;
      }

   return (fd_set*)0;
}

static void fdset_sock2fd(fd_set* fileset, fd_set* set)
{
   U_INTERNAL_TRACE("fdset_sock2fd(%p,%p)", fileset, set)

   if (set)
      {
      int fd;
      u_int i, idx;
      SOCKET h1, h2 = (SOCKET)INVALID_HANDLE_VALUE;

      U_INTERNAL_PRINT("fileset->fd_count = %d", fileset->fd_count)

      for (idx = 0; idx < fileset->fd_count; ++idx)
         {
         h1 = fileset->fd_array[idx];

         U_INTERNAL_PRINT("h1 = %p", h1)

         U_INTERNAL_ASSERT_DIFFERS((HANDLE)h1, INVALID_HANDLE_VALUE)
         U_INTERNAL_ASSERT_EQUALS(is_fh_socket((HANDLE)h1), TRUE)

         for (i = 0; i < set->fd_count; ++i)
            {
            fd = set->fd_array[i];
            h2 = (SOCKET) _get_osfhandle(fd);

            U_INTERNAL_PRINT("fd = %d h2 = %p", fd, h2)

         // U_INTERNAL_ASSERT_DIFFERS((HANDLE)h2, INVALID_HANDLE_VALUE)
         // U_INTERNAL_ASSERT_EQUALS(is_fh_socket((HANDLE)h2), TRUE)

            if (h1 == h2 ||
                (HANDLE)h2 == INVALID_HANDLE_VALUE)
               {
               fileset->fd_array[idx] = fd;

               break;
               }
            }

      // U_INTERNAL_ASSERT_EQUALS(h1, h2)
         }

      *set = *fileset;

      U_INTERNAL_PRINT("set->fd_count = %d", set->fd_count)

      U_INTERNAL_ASSERT_EQUALS(set->fd_count, fileset->fd_count)
      }
}

/**
 * Microsoft Windows does not have a unified IO system, so it doesn't support select() on files, devices, or pipes...
 * Microsoft provides the Berkeley select() call and an asynchronous select function that sends a WIN32 message when
 * the select condition exists... WSAAsyncSelect()
 */

int select_w32(int nfds, fd_set* rd, fd_set* wr, fd_set* ex, struct timeval* timeout)
{
   int r;
   fd_set sock_rd, sock_wr, sock_ex;
#ifdef DEBUG
   fd_set file_rd, file_wr, file_ex;
#endif

   U_INTERNAL_TRACE("select_w32(%d,%p,%p,%p,%p)", nfds, rd, wr, ex, timeout)

   if (nfds == 0 &&
       timeout)
      {
      Sleep(timeout->tv_sec * 1000 + timeout->tv_usec / 1000);

      return 0;
      }

   /*
   file_nfds  = extract_file_fd(rd, &file_rd);
   file_nfds += extract_file_fd(wr, &file_wr);
   file_nfds += extract_file_fd(ex, &file_ex);

   if (file_nfds)
      {
      // assume normal files are always readable/writable fake read/write fd_set and return value

      if (rd) *rd = file_rd;
      if (wr) *wr = file_wr;
      if (ex) *ex = file_ex;

      return file_nfds;
      }

   U_INTERNAL_ASSERT_EQUALS(extract_file_fd(rd, &file_rd) +
                            extract_file_fd(wr, &file_wr) +
                            extract_file_fd(ex, &file_ex), 0)
   */

   /* nfds argument is ignored and included only for the sake of compatibility */

   r = select(nfds, fdset_fd2sock(rd, &sock_rd), fdset_fd2sock(wr, &sock_wr), fdset_fd2sock(ex, &sock_ex), timeout);

   U_INTERNAL_PRINT("r = %d", r)

   if (r > 0)
      {
      fdset_sock2fd(&sock_rd, rd);
      fdset_sock2fd(&sock_wr, wr);
      fdset_sock2fd(&sock_ex, ex);
      }

   return r;
}

/*------------------------------------------*/
/*            Async timers                  */
/*------------------------------------------*/

/**
 * setitimer() does not exist on native MS Windows, so we emulate in both cases by using multimedia timers.
 * We emulate two timers, one for SIGALRM, another for SIGPROF. Minimum timer resolution on Win32 systems varies,
 * and is greater than or equal than 1 ms. The resolution is always wrapped not to attempt to get below the system
 * defined limit
 */

/* Last itimerval, as set by call to setitimer */
static struct itimerval itv;

/* Timer ID as returned by MM */
static MMRESULT tid = 0;

/* Timer precision, denominator of one fraction: for 100 ms interval, request 10 ms precision */
#define TIMER_PREC 10

/* Divide time in ms specified by IT by DENOM. Return 1 ms if division results in zero */

static UINT setitimer_helper_period(UINT denom)
{
   static TIMECAPS time_caps;

   UINT res;
   const struct timeval* tv = (itv.it_value.tv_sec == 0 && itv.it_value.tv_usec == 0) ? &itv.it_interval : &itv.it_value;

   U_INTERNAL_TRACE("setitimer_helper_period(%d)", denom)

   if (time_caps.wPeriodMin == 0) timeGetDevCaps(&time_caps, sizeof(time_caps));

   /* Zero means stop timer */

   if (tv->tv_sec == 0 && tv->tv_usec == 0) return 0;

   /* Convert to ms and divide by denom */

   res = (tv->tv_sec * 1000 + (tv->tv_usec + 500) / 1000) / denom;

   /* Converge to minimum timer resolution */

   if (res < time_caps.wPeriodMin) res = time_caps.wPeriodMin;

   U_INTERNAL_PRINT("ret = %d", res)

   return res;
}

static void CALLBACK setitimer_helper_proc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
   U_INTERNAL_TRACE("setitimer_helper_proc(%d,%d,%d,%d,%d)", uID, uMsg, dwUser, dw1, dw2)

   /* Just raise the signal indicated by the dwUser parameter */

   (void) raise_w32(dwUser);
}

int setitimer(int which, const struct itimerval* value, struct itimerval* oldvalue)
{
   int ret;
   UINT delay;

   U_INTERNAL_TRACE("setitimer(%d,%p,%p)", which, value, oldvalue)

   if (which != ITIMER_REAL)
      {
      errno = ENOSYS;

      return -1;
      }

   /* Check if we will wrap */

   if (itv.it_value.tv_sec >= (long) (UINT_MAX / 1000))
      {
      errno = EINVAL;

      return -1;
      }

   /* First stop the old timer */

   if (tid)
      {
      timeKillEvent(tid);

      timeEndPeriod(setitimer_helper_period(TIMER_PREC));

      tid = 0;
      }

   /* Return old itimerval if requested */

   if (oldvalue) *oldvalue = itv;

   if (value == NULL)
      {
      errno = EFAULT;

      return -1;
      }

   itv = *value;

   /* Determine if to start new timer */

   delay = setitimer_helper_period(1);

   if (delay)
      {
      UINT resolution = setitimer_helper_period(TIMER_PREC),
           event_type = (itv.it_value.tv_sec == 0 && itv.it_value.tv_usec == 0 ? TIME_ONESHOT : TIME_PERIODIC);

      timeBeginPeriod(resolution);

      tid = timeSetEvent(delay, resolution, setitimer_helper_proc, SIGALRM, event_type);
      }

   ret = (!delay || tid);

   U_INTERNAL_PRINT("ret = %d", ret)

   return ret;
}

unsigned int alarm(unsigned int seconds)
{
   unsigned int ret;
   struct itimerval newt, oldt;

   U_INTERNAL_TRACE("alarm(%u)", seconds)

   newt.it_value.tv_sec = seconds;
   newt.it_value.tv_usec = 0;
   newt.it_interval.tv_sec = 0;
   newt.it_interval.tv_usec = 0;

   setitimer(ITIMER_REAL, &newt, &oldt);

   /* Never return zero if there was a timer outstanding */

   ret = oldt.it_value.tv_sec + (oldt.it_value.tv_usec > 0 ? 1 : 0);

   U_INTERNAL_PRINT("ret = %d", ret)

   return ret;
}

struct w32_error_table_entry {
   DWORD value;
   const char* name;
/* const char* msg; */
};

struct w32_error_table_entry w32_error_table[] = {
#  include "./winerror.str"
};

/*
  Values are 32 bit values layed out as follows:

   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
  +---+-+-+-----------------------+-------------------------------+
  |Sev|C|R|     Facility          |               Code            |
  +---+-+-+-----------------------+-------------------------------+

  where

      Sev - is the severity code

          00 - Success
          01 - Informational
          10 - Warning
          11 - Error

      C - is the Customer code flag

      R - is a reserved bit

      Facility - is the facility code

      Code - is the facility's status code
*/

#include <lmerr.h>

#ifndef FACILITY_MSMQ
#define FACILITY_MSMQ 0x0E
#endif

const char* getSysError_w32(unsigned* len)
{
   static char buffer[1024];

   unsigned int i, lenMsg;
/* const char* msg  = "Unknown error"; */
   const char* name = "???";

   DWORD ret;              /* Temp space to hold a return value */
   LPTSTR pBuffer;         /* Buffer to hold the textual error description */
   HINSTANCE hInst = NULL; /* Instance handle for DLL */

   DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | /* The function will allocate space for pBuffer */
                   FORMAT_MESSAGE_MAX_WIDTH_MASK  |
                   FORMAT_MESSAGE_IGNORE_INSERTS;   /* No inserts */

   U_INTERNAL_TRACE("getSysError_w32(%p)", len)

   for (i = 0; i < U_NUM_ELEMENTS(w32_error_table); ++i)
      {
      if ((int)w32_error_table[i].value == errno)
         {
         name = w32_error_table[i].name;
      /* msg  = w32_error_table[i].msg; */

         break;
         }
      }

   if (HRESULT_FACILITY(errno) == FACILITY_MSMQ)
      {
      /**
       * MSMQ errors only (see winerror.h for facility info)
       * Load the MSMQ library containing the error message strings
       */

      hInst = LoadLibrary( TEXT("mqutil.dll") );
      }
   else if (errno >= NERR_BASE &&
            errno <= MAX_NERR)
      {
      /**
       * Could be a network error
       * Load the library containing network messages
       */

      hInst = LoadLibrary( TEXT("netmsg.dll") );
      }

   dwFlags |= (hInst == NULL ? FORMAT_MESSAGE_FROM_SYSTEM    /* System wide message */
                             : FORMAT_MESSAGE_FROM_HMODULE); /* Message definition is in a module */

   ret = FormatMessage(
         dwFlags,
         hInst,                                       /* Handle to the DLL */
         errno,                                       /* Message identifier */
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   /* Default language */
         (LPTSTR)&pBuffer,                            /* Buffer that will hold the text string */
         256,                                         /* Allocate at least this many chars for pBuffer */
         NULL                                         /* No insert values */
        );

   if (ret == 0) lenMsg = 0;
   else
      {
      lenMsg = u__strlen(pBuffer, __PRETTY_FUNCTION__);

      U_INTERNAL_ASSERT_MINOR(lenMsg, sizeof(buffer))

      if (lenMsg < 5) lenMsg = 0;

      else if (pBuffer[lenMsg-2] == '.' &&
               pBuffer[lenMsg-1] == ' ')
         {
         lenMsg -= 2;
         }

      if (pBuffer[lenMsg-1] == '\n') --lenMsg;
      }

   (void) snprintf(buffer, sizeof(buffer), "%s (%d, %.*s)", name, errno, lenMsg, pBuffer);

   /* Free the buffer */

   LocalFree(pBuffer);

   *len = u__strlen(buffer, __PRETTY_FUNCTION__);

   U_INTERNAL_PRINT("ret = %s", buffer)

   return buffer;
}

/* Perform file control operations on FD */

typedef struct {
   long osfhnd;    /* underlying OS file HANDLE */
   char osfile;    /* attributes of file (e.g., open in text mode?) */
   char pipech;    /* one char buffer for handles opened on pipes */
#ifdef MSVCRT_THREADS
   int lockinitflag;
   CRITICAL_SECTION lock;
#  if _MSC_VER >= 1400
   char textmode;
   char pipech2[2];
#  endif
#endif
} ioinfo;

EXTERN_C _CRTIMP ioinfo* __pioinfo[];

#define IOINFO_L2E         5
#define IOINFO_ARRAY_ELTS (1 << IOINFO_L2E)

#define pioinfo_extra 0
#define _pioinfo(i)   ((ioinfo*)((char*)(__pioinfo[i >> IOINFO_L2E]) + (i & (IOINFO_ARRAY_ELTS - 1)) * (sizeof(ioinfo) + pioinfo_extra)))

#define _osfhnd(i)    (_pioinfo(i)->osfhnd)
#define _osfile(i)    (_pioinfo(i)->osfile)
#define _pipech(i)    (_pioinfo(i)->pipech)

#define FOPEN        0x01  /* file handle open */
#define FNOINHERIT   0x10  /* file handle opened O_NOINHERIT */
#define FAPPEND      0x20  /* file handle opened O_APPEND */
#define FDEV         0x40  /* file handle refers to device */
#define FTEXT        0x80  /* file handle is in text mode */

int fcntl_w32(int fd, int cmd, void* arg)
{
   int res = -1;
   SOCKET h = (SOCKET) _get_osfhandle(fd);

   U_INTERNAL_TRACE("fcntl_w32(%d,%d,%p)", fd, cmd, arg)

   if ((HANDLE)h == INVALID_HANDLE_VALUE) return -1;

   if (is_fh_socket((HANDLE)h) == TRUE)
      {
      unsigned long mode = (unsigned long) arg;

      if (cmd == F_SETFL)
         {
         /**
          * Set the socket I/O mode: In this case FIONBIO enables or disables
          * the blocking mode for the socket based on the numerical value of iMode.
          *
          * If iMode  = 0,     blocking mode is enabled
          * If iMode != 0, non-blocking mode is enabled
          */

         u_long iMode = (mode & O_NONBLOCK ? 1 : 0);

         res = ioctlsocket(h, FIONBIO, &iMode);
         }  
      else
         {
         char outBuffer[32];
         DWORD cbBytesReturned;

         /**
          * int WSAIoctl(
          * in  SOCKET s,                                               // A descriptor identifying a socket
          * in  DWORD dwIoControlCode,                                  // The control code of operation to perform
          * in  LPVOID lpvInBuffer,                                     // A pointer to the input buffer
          * in  DWORD cbInBuffer,                                       // The size, in bytes, of the input buffer
          * out LPVOID lpvOutBuffer,                                    // A pointer to the output buffer
          * in  DWORD cbOutBuffer,                                      // The size, in bytes, of the output buffer
          * out LPDWORD lpcbBytesReturned,                              // A pointer to actual number of bytes of output
          * in  LPWSAOVERLAPPED lpOverlapped,                           // A pointer to a WSAOVERLAPPED structure (ignored for non-overlapped sockets)
          * in  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) // A pointer to the completion routine called when
          *                                                             // the operation has been completed (ignored for non-overlapped sockets)
          */

         res = WSAIoctl(h, cmd, &mode, sizeof(unsigned long), outBuffer, sizeof(outBuffer), &cbBytesReturned, 0, 0);
         }
      }
   else
      {
      switch (cmd)
         {
         /* Duplicate file descriptor */
         case F_DUPFD: res = dup(fd); break;

         /* Get file descriptor flags */
         case F_GETFD: res = (_osfile(fd) & FNOINHERIT ? FD_CLOEXEC : 0); break;

         case F_SETFD: /* Set file descriptor flags */
            {
            int newflags = (int) arg;

            if (newflags & FD_CLOEXEC)
               {
               newflags &= ~FD_CLOEXEC;
               newflags |=  FNOINHERIT;
               }

            _osfile(fd) = newflags;

            res = 0;
            }
         break;

         case F_GETFL: /* Get file status flags and file access modes */
            {
            int osflags;

            res     = 0;
            osflags = _osfile(fd);

            if      (osflags & FOPEN)        res |= O_RDWR;
            else if (osflags & FAPPEND)      res |= O_APPEND;
            else if (osflags & FNOINHERIT)   res |= O_NOINHERIT;
            else if (osflags & FTEXT)        res |= O_TEXT;
            }
         break;

         case F_SETFL: /* Set file status flags */
            {
            int osflags = 0;

            if       ((int) arg & O_APPEND) osflags |= FAPPEND;
            else if  ((int) arg & O_TEXT)   osflags |= FTEXT;

            _osfile(fd) = osflags;

            res = 0;
            }
         break;

         case F_SETLK:  /* Set record locking information */
         case F_SETLKW: /* Set record locking information; wait if blocked */
            {
            BOOL result;
            struct flock* l = (struct flock*) arg;
            off_t l_len = (l->l_len ? l->l_len : ULONG_MAX);

            if (isWindowNT())
               {
               OVERLAPPED theOvInfo;

               (void) memset(&theOvInfo, 0, sizeof(OVERLAPPED));

               theOvInfo.Offset = l->l_start;

               if (l->l_type == F_UNLCK) result = UnlockFileEx((HANDLE)h,                          0, l_len, 0, &theOvInfo);
               else                      result =   LockFileEx((HANDLE)h, LOCKFILE_EXCLUSIVE_LOCK, 0, l_len, 0, &theOvInfo);
               }
            else
               {
               if (l->l_type == F_UNLCK) result = UnlockFile((HANDLE)h, l->l_start, 0, l_len, 0);
               else                      result =   LockFile((HANDLE)h, l->l_start, 0, l_len, 0);
               }

            if (result == TRUE) res = 0;
            }
         break;

         case F_GETLK:  /* Get record locking information */
         case F_GETOWN: /* Get process or process group ID to receive SIGURG signals */
         case F_SETOWN: /* Set process or process group ID to receive SIGURG signals */
            {
            errno = ENOSYS;
            }
         break;

         default: errno = EINVAL; break;
         }
      }

   U_INTERNAL_PRINT("res = %d", res)

   return res;
}

#define _set_osfhnd(fh,osfh)  (void)(_osfhnd(fh) = osfh)
#define _set_osflags(fh,flags)      (_osfile(fh) = (flags))

int w32_open_osfhandle(long osfhandle, int flags)
{
   int fh;
   HANDLE hF;
   BOOL bclose;
   char fileflags; /* _osfile flags */

   U_INTERNAL_TRACE("w32_open_osfhandle(%ld,%d)", osfhandle, flags)

   /* copy relevant flags from second parameter */

   fileflags = FDEV;

   if (flags & O_APPEND)      fileflags |= FAPPEND;
   if (flags & O_TEXT)        fileflags |= FTEXT;
   if (flags & O_NOINHERIT)   fileflags |= FNOINHERIT;

   /* attempt to allocate a C Runtime file handle */

   hF = CreateFile("NUL", 0, 0, ((void *)0), OPEN_ALWAYS, 0, ((void*)0));

   fh = _open_osfhandle((long)hF, 0);

   bclose = CloseHandle(hF);

   U_INTERNAL_PRINT("bclose = %b", bclose)

   U_INTERNAL_ASSERT(bclose)

   /* the file is open. now, set the info in _osfhnd array */

   _set_osfhnd(fh, osfhandle);

   fileflags |= FOPEN;           /* mark as open */

   _set_osflags(fh, fileflags);  /* set osfile entry */

   return fh;                    /* return handle */
}
