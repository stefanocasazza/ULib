/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    base_error.c
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/error.h>

#ifdef HAVE_SYSEXITS_H
#  include <sysexits.h>
#else
#  include <ulib/base/replace/sysexits.h>
#endif

#define U_STR_ERROR "\n" \
" ----------------------------------- \n" \
"| *****  ****   ****    ***   ****  |\n" \
"| *      *   *  *   *  *   *  *   * |\n" \
"| *****  ****   ****   *   *  ****  |\n" \
"| *      *  *   *  *   *   *  *  *  |\n" \
"| *****  *   *  *   *   ***   *   * |\n" \
" ----------------------------------- \n"

void u_printError(void)
{
   U_INTERNAL_TRACE("u_printError()")

#ifndef _MSWINDOWS_
   if (u_is_tty) (void) write(STDERR_FILENO, U_CONSTANT_TO_PARAM(U_RED_STR));
#endif
                 (void) write(STDERR_FILENO, U_CONSTANT_TO_PARAM(U_STR_ERROR));
#ifndef _MSWINDOWS_
   if (u_is_tty) (void) write(STDERR_FILENO, U_CONSTANT_TO_PARAM(U_RESET_STR));
#endif
}

/**
 * Maps errno number to an error message string, the contents of which are implementation defined.
 * The returned string is only guaranteed to be valid only until the next call to function
 */

void u_getSysError(uint32_t* restrict len)
{
   /**
    * Translation table for errno values. See intro(2) in most UNIX systems Programmers Reference Manuals.
    * Note that this table is generally only accessed when it is used at runtime to initialize errno name and
    * message tables that are indexed by errno value.
    * Not all of these errnos will exist on all systems. This table is the only thing that should have to be
    * updated as new error numbers are introduced. It's sort of ugly, but at least its portable
    */

   struct error_info
      {
      int                  value;   /* The numeric value from <errno.h> */
      const char* restrict name;    /* The equivalent symbolic value */
#  ifndef HAVE_STRERROR
      const char* restrict msg;     /* Short message about this value */
#  endif
      };

#ifdef HAVE_STRERROR
#  define U_ERR_ENTRY(name,msg) {name, #name}
#else
#  define U_ERR_ENTRY(name,msg) {name, #name, msg}
#endif

   static const struct error_info error_table[] = {
#if defined(EPERM)
   U_ERR_ENTRY(EPERM, "Not owner"),
#endif
#if defined(ENOENT)
   U_ERR_ENTRY(ENOENT, "No such file or directory"),
#endif
#if defined(ESRCH)
   U_ERR_ENTRY(ESRCH, "No such process"),
#endif
#if defined(EINTR)
   U_ERR_ENTRY(EINTR, "Interrupted system call"),
#endif
#if defined(EIO)
   U_ERR_ENTRY(EIO, "I/O error"),
#endif
#if defined(ENXIO)
   U_ERR_ENTRY(ENXIO, "No such device or address"),
#endif
#if defined(E2BIG)
   U_ERR_ENTRY(E2BIG, "Argument list too long"),
#endif
#if defined(ENOEXEC)
   U_ERR_ENTRY(ENOEXEC, "Exec format error"),
#endif
#if defined(EBADF)
   U_ERR_ENTRY(EBADF, "Bad file number"),
#endif
#if defined(ECHILD)
   U_ERR_ENTRY(ECHILD, "No child processes"),
#endif
#if defined(EWOULDBLOCK)   /* Put before EAGAIN, sometimes aliased */
   U_ERR_ENTRY(EWOULDBLOCK, "Operation would block"),
#endif
#if defined(EAGAIN)
   U_ERR_ENTRY(EAGAIN, "No more processes"),
#endif
#if defined(ENOMEM)
   U_ERR_ENTRY(ENOMEM, "Cannot allocate memory"),
#endif
#if defined(EACCES)
   U_ERR_ENTRY(EACCES, "Permission denied"),
#endif
#if defined(EFAULT)
   U_ERR_ENTRY(EFAULT, "Bad address"),
#endif
#if defined(ENOTBLK)
   U_ERR_ENTRY(ENOTBLK, "Block device required"),
#endif
#if defined(EBUSY)
   U_ERR_ENTRY(EBUSY, "Device busy"),
#endif
#if defined(EEXIST)
   U_ERR_ENTRY(EEXIST, "File exists"),
#endif
#if defined(EXDEV)
   U_ERR_ENTRY(EXDEV, "Cross-device link"),
#endif
#if defined(ENODEV)
   U_ERR_ENTRY(ENODEV, "No such device"),
#endif
#if defined(ENOTDIR)
   U_ERR_ENTRY(ENOTDIR, "Not a directory"),
#endif
#if defined(EISDIR)
   U_ERR_ENTRY(EISDIR, "Is a directory"),
#endif
#if defined(EINVAL)
   U_ERR_ENTRY(EINVAL, "Invalid argument"),
#endif
#if defined(ENFILE)
   U_ERR_ENTRY(ENFILE, "File table overflow"),
#endif
#if defined(EMFILE)
   U_ERR_ENTRY(EMFILE, "Too many open files"),
#endif
#if defined(ENOTTY)
   U_ERR_ENTRY(ENOTTY, "Not a typewriter"),
#endif
#if defined(ETXTBSY)
   U_ERR_ENTRY(ETXTBSY, "Text file busy"),
#endif
#if defined(EFBIG)
   U_ERR_ENTRY(EFBIG, "File too large"),
#endif
#if defined(ENOSPC)
   U_ERR_ENTRY(ENOSPC, "No space left on device"),
#endif
#if defined(ESPIPE)
   U_ERR_ENTRY(ESPIPE, "Illegal seek"),
#endif
#if defined(EROFS)
   U_ERR_ENTRY(EROFS, "Read-only file system"),
#endif
#if defined(EMLINK)
   U_ERR_ENTRY(EMLINK, "Too many links"),
#endif
#if defined(EPIPE)
   U_ERR_ENTRY(EPIPE, "Broken pipe"),
#endif
#if defined(EDOM)
   U_ERR_ENTRY(EDOM, "Math argument out of domain of func"),
#endif
#if defined(ERANGE)
   U_ERR_ENTRY(ERANGE, "Math result not representable"),
#endif
#if defined(ENOMSG)
   U_ERR_ENTRY(ENOMSG, "No message of desired type"),
#endif
#if defined(EIDRM)
   U_ERR_ENTRY(EIDRM, "Identifier removed"),
#endif
#if defined(ECHRNG)
   U_ERR_ENTRY(ECHRNG, "Channel number out of range"),
#endif
#if defined(EL2NSYNC)
   U_ERR_ENTRY(EL2NSYNC, "Level 2 not synchronized"),
#endif
#if defined(EL3HLT)
   U_ERR_ENTRY(EL3HLT, "Level 3 halted"),
#endif
#if defined(EL3RST)
   U_ERR_ENTRY(EL3RST, "Level 3 reset"),
#endif
#if defined(ELNRNG)
   U_ERR_ENTRY(ELNRNG, "Link number out of range"),
#endif
#if defined(EUNATCH)
   U_ERR_ENTRY(EUNATCH, "Protocol driver not attached"),
#endif
#if defined(ENOCSI)
   U_ERR_ENTRY(ENOCSI, "No CSI structure available"),
#endif
#if defined(EL2HLT)
   U_ERR_ENTRY(EL2HLT, "Level 2 halted"),
#endif
#if defined(EDEADLK)
   U_ERR_ENTRY(EDEADLK, "Deadlock condition"),
#endif
#if defined(ENOLCK)
   U_ERR_ENTRY(ENOLCK, "No record locks available"),
#endif
#if defined(EBADE)
   U_ERR_ENTRY(EBADE, "Invalid exchange"),
#endif
#if defined(EBADR)
   U_ERR_ENTRY(EBADR, "Invalid request descriptor"),
#endif
#if defined(EXFULL)
   U_ERR_ENTRY(EXFULL, "Exchange full"),
#endif
#if defined(ENOANO)
   U_ERR_ENTRY(ENOANO, "No anode"),
#endif
#if defined(EBADRQC)
   U_ERR_ENTRY(EBADRQC, "Invalid request code"),
#endif
#if defined(EBADSLT)
   U_ERR_ENTRY(EBADSLT, "Invalid slot"),
#endif
#if defined(EDEADLOCK)
   U_ERR_ENTRY(EDEADLOCK, "File locking deadlock error"),
#endif
#if defined(EBFONT)
   U_ERR_ENTRY(EBFONT, "Bad font file format"),
#endif
#if defined(ENOSTR)
   U_ERR_ENTRY(ENOSTR, "Device not a stream"),
#endif
#if defined(ENODATA)
   U_ERR_ENTRY(ENODATA, "No data available"),
#endif
#if defined(ETIME)
   U_ERR_ENTRY(ETIME, "Timer expired"),
#endif
#if defined(ENOSR)
   U_ERR_ENTRY(ENOSR, "Out of streams resources"),
#endif
#if defined(ENONET)
   U_ERR_ENTRY(ENONET, "Machine is not on the network"),
#endif
#if defined(ENOPKG)
   U_ERR_ENTRY(ENOPKG, "Package not installed"),
#endif
#if defined(EREMOTE)
   U_ERR_ENTRY(EREMOTE, "Object is remote"),
#endif
#if defined(ENOLINK)
   U_ERR_ENTRY(ENOLINK, "Link has been severed"),
#endif
#if defined(EADV)
   U_ERR_ENTRY(EADV, "Advertise error"),
#endif
#if defined(ESRMNT)
   U_ERR_ENTRY(ESRMNT, "Srmount error"),
#endif
#if defined(ECOMM)
   U_ERR_ENTRY(ECOMM, "Communication error on send"),
#endif
#if defined(EPROTO)
   U_ERR_ENTRY(EPROTO, "Protocol error"),
#endif
#if defined(EMULTIHOP)
   U_ERR_ENTRY(EMULTIHOP, "Multihop attempted"),
#endif
#if defined(EDOTDOT)
   U_ERR_ENTRY(EDOTDOT, "RFS specific error"),
#endif
#if defined(EBADMSG)
   U_ERR_ENTRY(EBADMSG, "Not a data message"),
#endif
#if defined(ENAMETOOLONG)
   U_ERR_ENTRY(ENAMETOOLONG, "File name too long"),
#endif
#if defined(EOVERFLOW)
   U_ERR_ENTRY(EOVERFLOW, "Value too large for defined data type"),
#endif
#if defined(ENOTUNIQ)
   U_ERR_ENTRY(ENOTUNIQ, "Name not unique on network"),
#endif
#if defined(EBADFD)
   U_ERR_ENTRY(EBADFD, "File descriptor in bad state"),
#endif
#if defined(EREMCHG)
   U_ERR_ENTRY(EREMCHG, "Remote address changed"),
#endif
#if defined(ELIBACC)
   U_ERR_ENTRY(ELIBACC, "Cannot access a needed shared library"),
#endif
#if defined(ELIBBAD)
   U_ERR_ENTRY(ELIBBAD, "Accessing a corrupted shared library"),
#endif
#if defined(ELIBSCN)
   U_ERR_ENTRY(ELIBSCN, ".lib section in a.out corrupted"),
#endif
#if defined(ELIBMAX)
   U_ERR_ENTRY(ELIBMAX, "Attempting to link in too many shared libraries"),
#endif
#if defined(ELIBEXEC)
   U_ERR_ENTRY(ELIBEXEC, "Cannot exec a shared library directly"),
#endif
#if defined(EILSEQ)
   U_ERR_ENTRY(EILSEQ, "Illegal byte sequence"),
#endif
#if defined(ENOSYS)
   U_ERR_ENTRY(ENOSYS, "Operation not applicable"),
#endif
#if defined(ELOOP)
   U_ERR_ENTRY(ELOOP, "Too many symbolic links encountered"),
#endif
#if defined(ERESTART)
   U_ERR_ENTRY(ERESTART, "Interrupted system call should be restarted"),
#endif
#if defined(ESTRPIPE)
   U_ERR_ENTRY(ESTRPIPE, "Streams pipe error"),
#endif
#if defined(ENOTEMPTY)
   U_ERR_ENTRY(ENOTEMPTY, "Directory not empty"),
#endif
#if defined(EUSERS)
   U_ERR_ENTRY(EUSERS, "Too many users"),
#endif
#if defined(ENOTSOCK)
   U_ERR_ENTRY(ENOTSOCK, "Socket operation on non-socket"),
#endif
#if defined(EDESTADDRREQ)
   U_ERR_ENTRY(EDESTADDRREQ, "Destination address required"),
#endif
#if defined(EMSGSIZE)
   U_ERR_ENTRY(EMSGSIZE, "Message too long"),
#endif
#if defined(EPROTOTYPE)
   U_ERR_ENTRY(EPROTOTYPE, "Protocol wrong type for socket"),
#endif
#if defined(ENOPROTOOPT)
   U_ERR_ENTRY(ENOPROTOOPT, "Protocol not available"),
#endif
#if defined(EPROTONOSUPPORT)
   U_ERR_ENTRY(EPROTONOSUPPORT, "Protocol not supported"),
#endif
#if defined(ESOCKTNOSUPPORT)
   U_ERR_ENTRY(ESOCKTNOSUPPORT, "Socket type not supported"),
#endif
#if defined(EOPNOTSUPP)
   U_ERR_ENTRY(EOPNOTSUPP, "Operation not supported on transport endpoint"),
#endif
#if defined(EPFNOSUPPORT)
   U_ERR_ENTRY(EPFNOSUPPORT, "Protocol family not supported"),
#endif
#if defined(EAFNOSUPPORT)
   U_ERR_ENTRY(EAFNOSUPPORT, "Address family not supported by protocol"),
#endif
#if defined(EADDRINUSE)
   U_ERR_ENTRY(EADDRINUSE, "Address already in use"),
#endif
#if defined(EADDRNOTAVAIL)
   U_ERR_ENTRY(EADDRNOTAVAIL, "Cannot assign requested address"),
#endif
#if defined(ENETDOWN)
   U_ERR_ENTRY(ENETDOWN, "Network is down"),
#endif
#if defined(ENETUNREACH)
   U_ERR_ENTRY(ENETUNREACH, "Network is unreachable"),
#endif
#if defined(ENETRESET)
   U_ERR_ENTRY(ENETRESET, "Network dropped connection because of reset"),
#endif
#if defined(ECONNABORTED)
   U_ERR_ENTRY(ECONNABORTED, "Software caused connection abort"),
#endif
#if defined(ECONNRESET)
   U_ERR_ENTRY(ECONNRESET, "Connection reset by peer"),
#endif
#if defined(ENOBUFS)
   U_ERR_ENTRY(ENOBUFS, "No buffer space available"),
#endif
#if defined(EISCONN)
   U_ERR_ENTRY(EISCONN, "Transport endpoint is already connected"),
#endif
#if defined(ENOTCONN)
   U_ERR_ENTRY(ENOTCONN, "Transport endpoint is not connected"),
#endif
#if defined(ESHUTDOWN)
   U_ERR_ENTRY(ESHUTDOWN, "Cannot send after transport endpoint shutdown"),
#endif
#if defined(ETOOMANYREFS)
   U_ERR_ENTRY(ETOOMANYREFS, "Too many references: cannot splice"),
#endif
#if defined(ETIMEDOUT)
   U_ERR_ENTRY(ETIMEDOUT, "Connection timed out"),
#endif
#if defined(ECONNREFUSED)
   U_ERR_ENTRY(ECONNREFUSED, "Connection refused"),
#endif
#if defined(EHOSTDOWN)
   U_ERR_ENTRY(EHOSTDOWN, "Host is down"),
#endif
#if defined(EHOSTUNREACH)
   U_ERR_ENTRY(EHOSTUNREACH, "No route to host"),
#endif
#if defined(EALREADY)
   U_ERR_ENTRY(EALREADY, "Operation already in progress"),
#endif
#if defined(EINPROGRESS)
   U_ERR_ENTRY(EINPROGRESS, "Operation now in progress"),
#endif
#if defined(ESTALE)
   U_ERR_ENTRY(ESTALE, "Stale NFS file handle"),
#endif
#if defined(EUCLEAN)
   U_ERR_ENTRY(EUCLEAN, "Structure needs cleaning"),
#endif
#if defined(ENOTNAM)
   U_ERR_ENTRY(ENOTNAM, "Not a XENIX named type file"),
#endif
#if defined(ENAVAIL)
   U_ERR_ENTRY(ENAVAIL, "No XENIX semaphores available"),
#endif
#if defined(EISNAM)
   U_ERR_ENTRY(EISNAM, "Is a named type file"),
#endif
#if defined(EREMOTEIO)
   U_ERR_ENTRY(EREMOTEIO, "Remote I/O error"),
#endif
#if defined(EDQUOT)
   U_ERR_ENTRY(EDQUOT, "Quota exceeded"),
#endif
#if defined(ENOMEDIUM)
   U_ERR_ENTRY(ENOMEDIUM, "No medium found"),
#endif
#if defined(EMEDIUMTYPE)
   U_ERR_ENTRY(EMEDIUMTYPE, "Wrong medium type"),
#endif
#if defined(ENMFILE)
   U_ERR_ENTRY(ENMFILE, "No more files"),
#endif
#if defined(ENOSHARE)
   U_ERR_ENTRY(ENOSHARE, "No such host or network path"),
#endif
#if defined(EPROCLIM)
   U_ERR_ENTRY(EPROCLIM, "Too many processes")
#endif
};

#ifdef EVMSERR
   /* This is not in the table, because the numeric value of EVMSERR (32767) lies outside the range of sys_errlist[] */
   if (errno == EVMSERR) return "EVMSERR (32767, VMS-specific error)";
#endif

   const char* restrict msg  = "Unknown error";
   const char* restrict name = "???";

   U_INTERNAL_TRACE("u_getSysError(%p)", len)

   if ((errno > 0) &&
       (errno < (int)U_NUM_ELEMENTS(error_table))) /* check if in range */
      {
#  ifdef HAVE_STRERROR
      msg = strerror(errno);
#  endif

      if (error_table[errno].value == errno)
         {
#     ifndef HAVE_STRERROR
         msg  = error_table[errno].msg;
#     endif
         name = error_table[errno].name;
         }
      else
         {
         unsigned int i;

         for (i = 0; i < U_NUM_ELEMENTS(error_table); ++i)
            {
            if (error_table[i].value == errno)
               {
#           ifndef HAVE_STRERROR
               msg  = error_table[i].msg;
#           endif
               name = error_table[i].name;

               break;
               }
            }
         }
      }

   U_INTERNAL_ASSERT_POINTER(u_err_buffer)

   *len = snprintf(u_err_buffer, 256, "%s (%d, %s)", name, errno, msg);
}

/**
 * Maps an signal number to an signal message string, the contents of which are implementation defined.
 * The returned string is only guaranteed to be valid only until the next call to strsignal
 */

void u_getSysSignal(int signo, uint32_t* restrict len)
{
   /**
    * Translation table for signal values.  Note that this table is generally only accessed when
    * it is used at runtime to initialize signal name and message tables that are indexed by signal value.
    * Not all of these signals will exist on all systems.  This table is the only thing that should have to
    * be updated as new signal numbers are introduced. It's sort of ugly, but at least its portable
    */

   struct signal_info {
      int                  value;   /* The numeric value from <signal.h> */
      const char* restrict name;    /* The equivalent symbolic value */
#  ifndef HAVE_STRSIGNAL
      const char* restrict msg;     /* Short message about this value */
#  endif
   };

#undef U_ERR_ENTRY

#ifdef HAVE_STRSIGNAL
#  define U_SIG_ENTRY(name,msg) {name, #name}
#else
#  define U_SIG_ENTRY(name,msg) {name, #name, msg}
#endif

   static const struct signal_info signal_table[] = {
#if defined(SIGHUP)
   U_SIG_ENTRY(SIGHUP, "Hangup"),
#endif
#if defined(SIGINT)
   U_SIG_ENTRY(SIGINT, "Interrupt"),
#endif
#if defined(SIGQUIT)
   U_SIG_ENTRY(SIGQUIT, "Quit"),
#endif
#if defined(SIGILL)
   U_SIG_ENTRY(SIGILL, "Illegal instruction"),
#endif
#if defined(SIGTRAP)
   U_SIG_ENTRY(SIGTRAP, "Trace/breakpoint trap"),
#endif
   /* Put SIGIOT before SIGABRT, so that if SIGIOT == SIGABRT then SIGABRT overrides SIGIOT. SIGABRT is in ANSI and POSIX.1, and SIGIOT isn't */
#if defined(SIGIOT)
   U_SIG_ENTRY(SIGIOT, "IOT trap"),
#endif
#if defined(SIGABRT)
   U_SIG_ENTRY(SIGABRT, "Aborted"),
#endif
#if defined(SIGEMT)
   U_SIG_ENTRY(SIGEMT, "Emulation trap"),
#endif
#if defined(SIGFPE)
   U_SIG_ENTRY(SIGFPE, "Arithmetic exception"),
#endif
#if defined(SIGKILL)
   U_SIG_ENTRY(SIGKILL, "Killed"),
#endif
#if defined(SIGBUS)
   U_SIG_ENTRY(SIGBUS, "Bus error"),
#endif
#if defined(SIGSEGV)
   U_SIG_ENTRY(SIGSEGV, "Segmentation fault"),
#endif
#if defined(SIGSYS)
   U_SIG_ENTRY(SIGSYS, "Bad system call"),
#endif
#if defined(SIGPIPE)
   U_SIG_ENTRY(SIGPIPE, "Broken pipe"),
#endif
#if defined(SIGALRM)
   U_SIG_ENTRY(SIGALRM, "Alarm clock"),
#endif
#if defined(SIGTERM)
   U_SIG_ENTRY(SIGTERM, "Terminated"),
#endif
#if defined(SIGUSR1)
   U_SIG_ENTRY(SIGUSR1, "User defined signal 1"),
#endif
#if defined(SIGUSR2)
   U_SIG_ENTRY(SIGUSR2, "User defined signal 2"),
#endif
   /* Put SIGCLD before SIGCHLD, so that if SIGCLD == SIGCHLD then SIGCHLD overrides SIGCLD. SIGCHLD is in POXIX.1 */
#if defined(SIGCLD)
   U_SIG_ENTRY(SIGCLD, "Child status changed"),
#endif
#if defined(SIGCHLD)
   U_SIG_ENTRY(SIGCHLD, "Child status changed"),
#endif
#if defined(SIGPWR)
   U_SIG_ENTRY(SIGPWR, "Power fail/restart"),
#endif
#if defined(SIGWINCH)
   U_SIG_ENTRY(SIGWINCH, "Window size changed"),
#endif
#if defined(SIGURG)
   U_SIG_ENTRY(SIGURG, "Urgent I/O condition"),
#endif
#if defined(SIGIO)
   /* I/O pending has also been suggested, but is misleading since the signal only happens when the process has asked for it, not everytime I/O is pending */
   U_SIG_ENTRY(SIGIO, "I/O possible"),
#endif
#if defined(SIGPOLL)
   U_SIG_ENTRY(SIGPOLL, "Pollable event occurred"),
#endif
#if defined(SIGSTKFLT)
   U_SIG_ENTRY(SIGSTKFLT, "Stack fault"),
#endif
#if defined(SIGSTOP)
   U_SIG_ENTRY(SIGSTOP, "Stopped (signal)"),
#endif
#if defined(SIGTSTP)
   U_SIG_ENTRY(SIGTSTP, "Stopped (user)"),
#endif
#if defined(SIGCONT)
   U_SIG_ENTRY(SIGCONT, "Continued"),
#endif
#if defined(SIGTTIN)
   U_SIG_ENTRY(SIGTTIN, "Stopped (tty input)"),
#endif
#if defined(SIGTTOU)
   U_SIG_ENTRY(SIGTTOU, "Stopped (tty output)"),
#endif
#if defined(SIGVTALRM)
   U_SIG_ENTRY(SIGVTALRM, "Virtual timer expired"),
#endif
#if defined(SIGPROF)
   U_SIG_ENTRY(SIGPROF, "Profiling timer expired"),
#endif
#if defined(SIGXCPU)
   U_SIG_ENTRY(SIGXCPU, "CPU time limit exceeded"),
#endif
#if defined(SIGXFSZ)
   U_SIG_ENTRY(SIGXFSZ, "File size limit exceeded"),
#endif
#if defined(SIGWIND)
   U_SIG_ENTRY(SIGWIND, "SIGWIND"),
#endif
#if defined(SIGPHONE)
   U_SIG_ENTRY(SIGPHONE, "SIGPHONE"),
#endif
#if defined(SIGLOST)
   U_SIG_ENTRY(SIGLOST, "Resource lost"),
#endif
#if defined(SIGWAITING)
   U_SIG_ENTRY(SIGWAITING, "Process's LWPs are blocked"),
#endif
#if defined(SIGLWP)
   U_SIG_ENTRY(SIGLWP, "Signal LWP"),
#endif
#if defined(SIGDANGER)
   U_SIG_ENTRY(SIGDANGER, "Swap space dangerously low"),
#endif
#if defined(SIGGRANT)
   U_SIG_ENTRY(SIGGRANT, "Monitor mode granted"),
#endif
#if defined(SIGRETRACT)
   U_SIG_ENTRY(SIGRETRACT, "Need to relinguish monitor mode"),
#endif
#if defined(SIGMSG)
   U_SIG_ENTRY(SIGMSG, "Monitor mode data available"),
#endif
#if defined(SIGSOUND)
   U_SIG_ENTRY(SIGSOUND, "Sound completed"),
#endif
#if defined(SIGSAK)
   U_SIG_ENTRY(SIGSAK, "Secure attention")
#endif
};

   const char* restrict msg = "Unknown signal";
   const char* restrict name = "???";

   U_INTERNAL_TRACE("u_getSysSignal(%d,%p)", signo, len)

   if ((signo > 0) &&
       (signo < (int)U_NUM_ELEMENTS(signal_table))) /* check if in range */
      {
#  ifdef HAVE_STRSIGNAL
      msg = strsignal(signo);
#  endif

      if (signal_table[signo].value == signo)
         {
#     ifndef HAVE_STRSIGNAL
         msg  = signal_table[signo].msg;
#     endif
         name = signal_table[signo].name;
         }
      else
         {
         unsigned int i;

         for (i = 0; i < U_NUM_ELEMENTS(signal_table); ++i)
            {
            if (signal_table[i].value == signo)
               {
#           ifndef HAVE_STRSIGNAL
               msg  = signal_table[i].msg;
#           endif
               name = signal_table[i].name;

               break;
               }
            }
         }
      }

   U_INTERNAL_ASSERT_POINTER(u_err_buffer)

   *len = snprintf(u_err_buffer, 256, "%s (%d, %s)", name, signo, msg);
}

#undef U_SIG_ENTRY

#define U_EXIT_ENTRY(name,msg) {name, #name, msg}

void u_getExitStatus(int exitno, uint32_t* restrict len)
{
   /* Translation table for exit status codes for system programs */

   struct exit_value_info {
      int value;                 /* The numeric value from <sysexits.h> */
      const char* restrict name; /* The equivalent symbolic value */
      const char* restrict msg;  /* Short message about this value */
   };

   static const struct exit_value_info exit_value_table[] = {
#if defined(EX_USAGE)
   U_EXIT_ENTRY(EX_USAGE, "command line usage error"),
#endif
#if defined(EX_DATAERR)
   U_EXIT_ENTRY(EX_DATAERR, "data format error"),
#endif
#if defined(EX_NOINPUT)
   U_EXIT_ENTRY(EX_NOINPUT, "cannot open input"),
#endif
#if defined(EX_NOUSER)
   U_EXIT_ENTRY(EX_NOUSER, "addresse unknown"),
#endif
#if defined(EX_NOHOST)
   U_EXIT_ENTRY(EX_NOHOST, "host name unknown"),
#endif
#if defined(EX_UNAVAILABLE)
   U_EXIT_ENTRY(EX_UNAVAILABLE, "service unavailable"),
#endif
#if defined(EX_SOFTWARE)
   U_EXIT_ENTRY(EX_SOFTWARE, "internal software error"),
#endif
#if defined(EX_OSERR)
   U_EXIT_ENTRY(EX_OSERR, "system error (e.g., can't fork)"),
#endif
#if defined(EX_OSFILE)
   U_EXIT_ENTRY(EX_OSFILE, "critical OS file missing"),
#endif
#if defined(EX_CANTCREAT)
   U_EXIT_ENTRY(EX_CANTCREAT, "can't create (user) output file"),
#endif
#if defined(EX_IOERR)
   U_EXIT_ENTRY(EX_IOERR, "input/output error"),
#endif
#if defined(EX_TEMPFAIL)
   U_EXIT_ENTRY(EX_TEMPFAIL, "temp failure; user is invited to retry"),
#endif
#if defined(EX_PROTOCOL)
   U_EXIT_ENTRY(EX_PROTOCOL, "remote error in protocol"),
#endif
#if defined(EX_NOPERM)
   U_EXIT_ENTRY(EX_NOPERM, "permission denied"),
#endif
#if defined(EX_CONFIG)
   U_EXIT_ENTRY(EX_CONFIG, "configuration error")
#endif
};

#undef U_EXIT_ENTRY

   const char* restrict msg;
   const char* restrict name;

   U_INTERNAL_TRACE("u_getExitStatus(%d,%p)", exitno, len)

   if (exitno == 0)
      {
      msg  = "successful termination";
      name = "EX_OK";
      }
   else
      {
      msg  = "Unknown value";
      name = "???";
      }

   if ((exitno >= EX__BASE) &&
       (exitno <= EX__MAX)) // check if in range
      {
      int _index = exitno - EX__BASE;

      if (exit_value_table[_index].value == exitno)
         {
         msg  = exit_value_table[_index].msg;
         name = exit_value_table[_index].name;
         }
      else
         {
         unsigned int i;

         for (i = 0; i < U_NUM_ELEMENTS(exit_value_table); ++i)
            {
            if (exit_value_table[i].value == exitno)
               {
               msg  = exit_value_table[i].msg;
               name = exit_value_table[i].name;

               break;
               }
            }
         }
      }

   U_INTERNAL_ASSERT_POINTER(u_err_buffer)

   *len = snprintf(u_err_buffer, 256, "%s (%d, %s)", name, exitno, msg);
}
