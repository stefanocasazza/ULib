// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    interrupt.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/base/utility.h>
#include <ulib/utility/interrupt.h>

/*
const char* UInterrupt::ILL_errlist[] = {
   "ILL_ILLOPC",  "illegal opcode",          // ILL_ILLOPC 1
   "ILL_ILLOPN",  "illegal operand",         // ILL_ILLOPN 2
   "ILL_ILLADR",  "illegal addressing mode", // ILL_ILLADR 3
   "ILL_ILLTRP",  "illegal trap",            // ILL_ILLTRP 4
   "ILL_PRVOPC",  "privileged opcode",       // ILL_PRVOPC 5
   "ILL_PRVREG",  "privileged register",     // ILL_PRVREG 6
   "ILL_COPROC",  "coprocessor error",       // ILL_COPROC 7
   "ILL_BADSTK",  "internal stack error"     // ILL_BADSTK 8
};

const char* UInterrupt::FPE_errlist[] = {
   "FPE_INTOVF",  "integer overflow",                 // FPE_INTOVF 2
   "FPE_FLTDIV",  "floating point divide by zero",    // FPE_FLTDIV 3
   "FPE_FLTOVF",  "floating point overflow",          // FPE_FLTOVF 4
   "FPE_FLTUND",  "floating point underflow",         // FPE_FLTUND 5
   "FPE_FLTRES",  "floating point inexact result",    // FPE_FLTRES 6
   "FPE_FLTINV",  "floating point invalid operation", // FPE_FLTINV 7
   "FPE_FLTSUB",  "subscript out of range"            // FPE_FLTSUB 8
};

const char* UInterrupt::CLD_list[] = {
   "CLD_EXITED",     "child has exited",              // CLD_EXITED     1
   "CLD_KILLED",     "child was killed",              // CLD_KILLED     2
   "CLD_DUMPED",     "child terminated abnormally",   // CLD_DUMPED     3
   "CLD_TRAPPED",    "traced child has trapped",      // CLD_TRAPPED    4
   "CLD_STOPPED",    "child has stopped",             // CLD_STOPPED    5
   "CLD_CONTINUED",  "stopped child has continued",   // CLD_CONTINUED  6
};

const char* UInterrupt::POLL_list[] = {
   "POLL_IN", "data input available",           // POLL_IN  1
   "POLL_OUT", "output buffers available",      // POLL_OUT 2
   "POLL_MSG", "input message available",       // POLL_MSG 3
   "POLL_ERR", "i/o error",                     // POLL_ERR 4
   "POLL_PRI", "high priority input available", // POLL_PRI 5
   "POLL_HUP", "device disconnected"            // POLL_HUP 6
};

const char* UInterrupt::TRAP_list[] = {
   "TRAP_BRKPT", "process breakpoint", // TRAP_BRKPT 1
   "TRAP_TRACE", "process trace trap", // TRAP_TRACE 2
};

const char* UInterrupt::origin_list[] = {
   "SI_USER",     "kill(), sigsend() or raise()",  // SI_USER      0
   "SI_QUEUE",    "sigqueue()",                    // SI_QUEUE    -1
   "SI_TIMER",    "timer expired",                 // SI_TIMER    -2
   "SI_MESGQ",    "mesq state changed",            // SI_MESGQ    -3
   "SI_ASYNCIO",  "AIO completed",                 // SI_ASYNCIO  -4
   "SI_SIGIO",    "queued SIGIO",                  // SI_SIGIO    -5
   "SI_KERNEL",   "kernel"                         // SI_KERNEL 0x80
};
*/

const char* UInterrupt::SEGV_errlist[] = {
   "", "",
   "SEGV_MAPERR", "Address not mapped to object",           // SEGV_MAPERR 1
   "SEGV_ACCERR", "Invalid permissions for mapped object"   // SEGV_ACCERR 2
};

const char* UInterrupt::BUS_errlist[] = {
   "", "",
   "BUS_ADRALN", "Invalid address alignment",      // BUS_ADRALN 1
   "BUS_ADRERR", "Non-existant physical address",  // BUS_ADRERR 2
   "BUS_OBJERR", "Object specific hardware error"  // BUS_OBJERR 3
};

bool              UInterrupt::flag_alarm;
bool              UInterrupt::syscall_restart; // NB: notify to make certain system calls restartable across signals...
bool              UInterrupt::exit_loop_wait_event_for_signal;
jmp_buf           UInterrupt::jbuf;
sigset_t*         UInterrupt::mask_interrupt;
sig_atomic_t      UInterrupt::flag_wait_for_signal;
struct sigaction  UInterrupt::act;
struct itimerval  UInterrupt::timerval;
struct sigaction  UInterrupt::old[NSIG];

int               UInterrupt::event_signal_pending;
sig_atomic_t      UInterrupt::event_signal[NSIG];
sighandler_t      UInterrupt::handler_signal[NSIG];

void UInterrupt::insert(int signo, sighandler_t handler)
{
   U_TRACE(1, "UInterrupt::insert(%d,%p)", signo, handler)

   U_INTERNAL_ASSERT_RANGE(1, signo, NSIG)

   handler_signal[signo] = handler;

   act.sa_handler = handlerEventSignal;

   (void) U_SYSCALL(sigaction, "%d,%p,%p", signo, &act, old + signo);
}

void UInterrupt::erase(int signo)
{
   U_TRACE(1, "UInterrupt::erase(%d)", signo)

   U_INTERNAL_ASSERT_RANGE(1, signo, NSIG)
   U_INTERNAL_ASSERT_POINTER(handler_signal[signo])

   handler_signal[signo] = 0;

   (void) U_SYSCALL(sigaction, "%d,%p,%p", signo, old + signo, 0);
}

bool UInterrupt::disable(sigset_t* mask, sigset_t* mask_old)
{
   U_TRACE(1, "UInterrupt::disable(%p,%p)", mask, mask_old)

   if (!mask)
      {
      if (!mask_interrupt) setMaskInterrupt(0, 0);

      mask = mask_interrupt;
      }

   if (U_SYSCALL(sigprocmask, "%d,%p,%p", SIG_BLOCK, mask, mask_old) == 0) U_RETURN(true);

   U_RETURN(false);
}

bool UInterrupt::enable(sigset_t* mask)
{
   U_TRACE(1, "UInterrupt::enable(%p)", mask)

   if (!mask)
      {
      if (!mask_interrupt) setMaskInterrupt(0, 0);

      mask = mask_interrupt;
      }

   if (U_SYSCALL(sigprocmask, "%d,%p,%p", SIG_BLOCK, mask, 0) == 0) U_RETURN(true);

   U_RETURN(false);
}

RETSIGTYPE UInterrupt::handlerSignal(int signo)
{
   U_TRACE(0, "[SIGNAL] UInterrupt::handlerSignal(%d)", signo)

   flag_wait_for_signal = false;
}

// Send ourselves the signal: see http://www.cons.org/cracauer/sigint.html

void UInterrupt::sendOurselves(int signo)
{
   U_TRACE(0, "UInterrupt::sendOurselves(%d)", signo)

   setHandlerForSignal(signo, (sighandler_t)SIG_DFL);

   u_exit();

   (void) U_SYSCALL(kill, "%d,%d", u_pid, signo);
}

RETSIGTYPE UInterrupt::handlerInterrupt(int signo)
{
   U_TRACE(0, "UInterrupt::handlerInterrupt(%d)", signo)

   U_MESSAGE("(pid %P) program interrupt - %Y", signo);

   if (signo == SIGBUS || //  7
       signo == SIGSEGV)  // 11 
      {
#  ifdef DEBUG
      u_debug_at_exit(); 
#  endif
      }

// U_EXIT(-1);

   sendOurselves(signo);
}

RETSIGTYPE UInterrupt::handlerEventSignal(int signo)
{
#ifdef DEBUG
   u_trace_unlock();
#endif

   U_TRACE(0, "[SIGNAL] UInterrupt::handlerEventSignal(%d)", signo)

   ++event_signal[signo];

   event_signal_pending = (event_signal_pending ? NSIG : signo);
}

void UInterrupt::waitForSignal(int signo)
{
   U_TRACE(1, "UInterrupt::waitForSignal(%d)", signo)

   static sigset_t mask_wait_for_signal;

   flag_wait_for_signal = true;

   setHandlerForSignal(signo, (sighandler_t)handlerSignal);

   while (flag_wait_for_signal == true)
      {
      (void) U_SYSCALL(sigsuspend, "%p", &mask_wait_for_signal);
      }
}

void UInterrupt::callHandlerSignal()
{
   U_TRACE_NO_PARAM(0, "UInterrupt::callHandlerSignal()")

   int i;

loop:
   U_INTERNAL_DUMP("event_signal_pending = %d", event_signal_pending)

   i = event_signal_pending;
       event_signal_pending = 0;

   if (i < NSIG)
      {
      U_INTERNAL_ASSERT_POINTER(handler_signal[i])

      handler_signal[i](event_signal[i]);

      event_signal[i] = 0;
      }
   else
      {
      for (i = 1; i < NSIG; ++i)
         {
         if (event_signal[i])
            {
            U_INTERNAL_ASSERT_POINTER(handler_signal[i])

            handler_signal[i](event_signal[i]);

            event_signal[i] = 0;
            }
         }
      }

   // NB: it can happen that in manage the signal the calling function produce another signal because
   // the interval is too short (< 10ms) in this case the parameter to the calling function is zero...

   if (event_signal_pending) goto loop;
}

void UInterrupt::setMaskInterrupt(sigset_t* mask, int signo)
{
   U_TRACE(1, "UInterrupt::setMaskInterrupt(%p,%d)", mask, signo)

   if (mask)
      {
#  ifdef sigemptyset
      sigemptyset(mask);
#  else
      (void) U_SYSCALL(sigemptyset, "%p", mask);
#  endif

#  ifdef sigaddset
      sigaddset(mask, signo);
#  else
      (void) U_SYSCALL(sigaddset, "%p,%d", mask, signo);
#  endif
      }
   else
      {
      U_INTERNAL_ASSERT_EQUALS(mask_interrupt,0)

      mask_interrupt = new sigset_t;

#  ifdef sigemptyset
      sigemptyset(mask_interrupt);
#  else
      (void) U_SYSCALL(sigemptyset, "%p", mask_interrupt);
#  endif

#  ifdef sigaddset
      sigaddset(mask_interrupt, SIGUSR1); // 10
      sigaddset(mask_interrupt, SIGUSR2); // 12
      sigaddset(mask_interrupt, SIGALRM); // 14
      sigaddset(mask_interrupt, SIGCHLD); // 17
#  else
      (void) U_SYSCALL(sigaddset, "%p,%d", mask_interrupt, SIGUSR1); // 10
      (void) U_SYSCALL(sigaddset, "%p,%d", mask_interrupt, SIGUSR2); // 12
      (void) U_SYSCALL(sigaddset, "%p,%d", mask_interrupt, SIGALRM); // 14
      (void) U_SYSCALL(sigaddset, "%p,%d", mask_interrupt, SIGCHLD); // 17
#  endif
      }
}

void UInterrupt::init()
{
   U_TRACE_NO_PARAM(1, "UInterrupt::init()")

#ifdef HAVE_SIGINFO_T
   act.sa_flags     = SA_SIGINFO;
   act.sa_sigaction = handlerInterruptWithInfo;

   (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGBUS,  &act, 0); // 7
   (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGSEGV, &act, 0); // 11
   /*
   (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGCHLD, &act, 0);
   (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGILL,  &act, 0);
   (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGFPE,  &act, 0);
   (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGPOLL, &act, 0);
   (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGTRAP, &act, 0);
   */
#endif

   act.sa_flags   = 0;
   act.sa_handler = handlerInterrupt;

// (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGHUP,  &act, 0);
   (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGINT,  &act, 0); // 2
// (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGQUIT, &act, 0);
   (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGTERM, &act, 0); // 15
#ifndef _MSWINDOWS_
// (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGPIPE, &act, 0); // 13
#endif

#ifndef HAVE_SIGINFO_T
   (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGSEGV, &act, 0); // 11
#  ifndef _MSWINDOWS_
   (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGBUS,  &act, 0); // 7
#  endif
#endif

#ifndef _MSWINDOWS_
   sigset_t mask_sigpipe;

   setMaskInterrupt(&mask_sigpipe, SIGPIPE);
            disable(&mask_sigpipe, 0);
#endif

   syscall_restart = true; // NB: notify to make certain system calls restartable across signals...

   setHandlerForSignal(SIGALRM, (sighandler_t)handlerAlarm);
}

RETSIGTYPE UInterrupt::handlerAlarm(int signo)
{
   U_TRACE(0, "[SIGALRM] UInterrupt::handlerAlarm(%d)", signo)

   if ((flag_alarm = (signo == SIGALRM)))
      {
      timerval.it_value.tv_sec  =
      timerval.it_value.tv_usec = 0;
      }

   U_INTERNAL_DUMP("timerval.it_value = { %ld %6ld }", timerval.it_value.tv_sec, timerval.it_value.tv_usec)

   (void) U_SYSCALL(setitimer, "%d,%p,%p", ITIMER_REAL, &timerval, 0);
}

void UInterrupt::setHandlerForSegv(vPF func, bPF fault)
{
   U_TRACE(0+256, "UInterrupt::setHandlerForSegv(%p,%p)", func, fault)

#ifndef HAVE_SIGINFO_T
   act.sa_handler   = handlerSegv;
#else
   act.sa_flags     = SA_SIGINFO;
   act.sa_sigaction = handlerSegvWithInfo;
#endif

   (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGSEGV, &act, old + SIGSEGV); // 11

retry:
   if (setjmp(jbuf) == 0) func();
   else
      {
      U_WARNING("%.*s", u_buffer_len, u_buffer);
                        u_buffer_len = 0;

      if (fault()) goto retry;
      }

   (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGSEGV, old + SIGSEGV, 0);
}

RETSIGTYPE UInterrupt::handlerSegvWithInfo(int signo, siginfo_t* info, void* context)
{
   if (u_recursion == false) // NB: maybe recursion occurs...
      {
#  ifdef DEBUG
      u_flag_test = 0;

      U_TRACE(0, "[SIGSEGV] UInterrupt::handlerSegvWithInfo(%d,%p,%p)", signo, info, context)
#  endif

      getSignalInfo(signo, info);

      /**
       * When you do something that causes the kernel to send you a SIGSEGV:
       *
       * If you don't have a signal handler, the kernel kills the process and that's that
       *
       * If you do have a signal handler
       *  Your handler gets called
       *  ==> The kernel restarts the offending operation <==
       *
       * So if you don't do anything abut it, it will just loop continuously.
       * If you do catch SIGSEGV and you don't exit, thereby interfering with
       * the normal flow, you must:
       *
       * fix things such that the offending operation doesn't restart or
       * fix the memory layout such that what was offending will be ok on the next run
       */

      longjmp(jbuf, 1);
      }

   ::abort();
}

void UInterrupt::getSignalInfo(int signo, siginfo_t* info)
{
   U_TRACE(0, "UInterrupt::getSignalInfo(%d,%p)", signo, info)

/*
#if defined(__CYGWIN__)
    union sigval {
      int   sival_int; // Integer signal value
      void* sival_ptr; // Pointer signal value
    }
   
   siginfo_t {
      int          si_signo; // Signal number
      int          si_code;  // Cause of the signal
      union sigval si_value; // Signal value
    }
#else
    siginfo_t {
       int      si_signo;  // Signal number
       int      si_errno;  // An errno value
       int      si_code;   // Signal code
       pid_t    si_pid;    // Sending process ID
       uid_t    si_uid;    // Real user ID of sending process
       int      si_status; // Exit value or signal
       clock_t  si_utime;  // User time consumed
       clock_t  si_stime;  // System time consumed
       sigval_t si_value;  // Signal value
       int      si_int;    // POSIX.1b signal
       void*    si_ptr;    // POSIX.1b signal
       void*    si_addr;   // Memory location which caused fault
       int      si_band;   // Band event
       int      si_fd;     // File descriptor
    }
#endif
*/

#ifndef SI_FROMKERNEL
#  ifdef _MSWINDOWS_
#     define SI_FROMKERNEL(siptr) false 
#  else
#     define SI_FROMKERNEL(siptr) ((siptr)->si_code > 0)
#  endif
#endif

#ifndef _MSWINDOWS_
#  ifdef HAVE_MEMBER_SI_ADDR
#     define U_MSG_FROMKERNEL \
      "program interrupt by the kernel - %Y%W\n" \
           "----------------------------------------" \
           "----------------------------------------------------\n" \
           " pid: %W%P%W\n" \
           " address: %W%p - %s (%d, %s)%W\n" \
           " rss usage: %W%.2f MBytes%W\n" \
           " address space usage: %W%.2f MBytes%W\n" \
           "----------------------------------------" \
           "----------------------------------------------------"
#  else
#     define U_MSG_FROMKERNEL \
      "program interrupt by the kernel - %Y%W\n" \
           "----------------------------------------" \
           "----------------------------------------------------\n" \
           " pid: %W%P%W\n" \
           " desc: %W %s (%d, %s)%W\n" \
           " rss usage: %W%.2f MBytes%W\n" \
           " address space usage: %W%.2f MBytes%W\n" \
           "----------------------------------------" \
           "----------------------------------------------------"
#  endif
#endif

   if (info == 0 ||
       SI_FROMKERNEL(info) == false)
      {
      u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("program interrupt by kill(), sigsend() or raise() - %Y"), signo);
      }
#ifndef _MSWINDOWS_
   else
      {
      unsigned long vsz, rss;
      int index = info->si_code * 2;
      const char** errlist = (signo == SIGBUS ? BUS_errlist : SEGV_errlist);

      u_get_memusage(&vsz, &rss);

      u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM(U_MSG_FROMKERNEL),
           signo, YELLOW,
            CYAN, YELLOW,
#        ifdef HAVE_MEMBER_SI_ADDR
           CYAN, info->si_addr, errlist[index], info->si_code, errlist[index+1], YELLOW,
#        else
           CYAN,                errlist[index], info->si_code, errlist[index+1], YELLOW,
#        endif
           CYAN, (double)vsz / (1024.0 * 1024.0), YELLOW,
           CYAN, (double)rss / (1024.0 * 1024.0), YELLOW);
      }
#  endif
}

__noreturn RETSIGTYPE UInterrupt::handlerInterruptWithInfo(int signo, siginfo_t* info, void* context)
{
   if (u_recursion == false) // NB: maybe recursion occurs...
      {
#  ifdef DEBUG
      u_flag_test = 0;

      U_TRACE(0, "UInterrupt::handlerInterruptWithInfo(%d,%p,%p)", signo, info, context)
#  endif

      getSignalInfo(signo, info);

      U_ABORT("%.*s", u_buffer_len, u_buffer);
      }

   ::abort();
}
