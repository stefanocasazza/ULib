// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    interrupt.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_INTERRUPT_H
#define ULIB_INTERRUPT_H 1

#include <ulib/internal/common.h>

#include <setjmp.h>

#ifndef HAVE_SIGINFO_T
typedef uint64_t siginfo_t;
#endif

struct U_EXPORT UInterrupt {

   static struct sigaction act;

   /*
   static const char*    CLD_list[];
   static const char*   POLL_list[];
   static const char*   TRAP_list[];
   static const char* origin_list[];

   static const char* ILL_errlist[];
   static const char* FPE_errlist[];
   */
   static const char*  BUS_errlist[];
   static const char* SEGV_errlist[];

   static void init();
   static void setMaskInterrupt(sigset_t* mask, int signo);

   static RETSIGTYPE handlerInterrupt(int signo);

   static sigset_t* mask_interrupt; // SIGALRM | SIGUSR[1|2] | SIGCHLD
   static sig_atomic_t flag_wait_for_signal;
   static bool exit_loop_wait_event_for_signal;

   static RETSIGTYPE handlerSignal(int signo);

   static void setHandlerForSignal(int signo, sighandler_t function)
      {
      U_TRACE(1, "UInterrupt::setHandlerForSignal(%d,%p)", signo, function)

      act.sa_handler = function;

      (void) U_SYSCALL(sigaction, "%d,%p,%p", signo, &act, 0);
      }

   static void setNoZombies()
      {
      U_TRACE_NO_PARAM(1, "UInterrupt::setNoZombies()")

      act.sa_flags   = SA_NOCLDWAIT;
      act.sa_handler = (sighandler_t)SIG_DFL;

      (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGCHLD, &act, 0);

      act.sa_flags = 0;
      }

   static void waitForSignal(int signo);
   static void sendOurselves(int signo);
   static bool sendSignal(   int signo, pid_t pid)
      {
      U_TRACE(1, "UInterrupt::sendSignal(%d,%d)", signo, pid)

      if (U_SYSCALL(kill, "%d,%d", pid, signo) != -1) U_RETURN(true);

      U_RETURN(false);
      }

   static bool  enable(sigset_t* mask);
   static bool disable(sigset_t* mask, sigset_t* mask_old);

   // manage sync signal
  
   static bool flag_alarm;
   static struct itimerval timerval;
   static RETSIGTYPE handlerAlarm(int signo);

   static void setAlarm(int timeoutMS)
      {
      U_TRACE(0, "UInterrupt::setAlarm(%d)", timeoutMS)

      timerval.it_value.tv_sec  = (timeoutMS / 1000);
      timerval.it_value.tv_usec = (timeoutMS % 1000) * 1000;

      handlerAlarm(0);
      }

   static void resetAlarm() { setAlarm(0); }

   // manage SIGSEGV signal

   static jmp_buf jbuf;
   static void setHandlerForSegv(vPF func, bPF fault);

   static void getSignalInfo(int signo, siginfo_t* info);

   static RETSIGTYPE handlerSegv(int signo) { handlerSegvWithInfo(signo, 0, 0); } 

   static RETSIGTYPE handlerSegvWithInfo(     int signo, siginfo_t* info, void*);
   static RETSIGTYPE handlerInterruptWithInfo(int signo, siginfo_t* info, void*) __noreturn;

   // manage async signal

   static bool syscall_restart; // NB: notify to make certain system calls restartable across signals...
   static int event_signal_pending;
   static struct sigaction old[NSIG];
   static sig_atomic_t event_signal[NSIG];
   static sighandler_t handler_signal[NSIG];

   static void callHandlerSignal();
   static void checkForEventSignalPending()
      {
      U_TRACE_NO_PARAM(0, "UInterrupt::checkForEventSignalPending()")

      U_INTERNAL_DUMP("errno = %d u_errno = %d UInterrupt::event_signal_pending = %d UInterrupt::syscall_restart = %b UInterrupt::flag_alarm = %b",
                       errno,     u_errno,     UInterrupt::event_signal_pending,     UInterrupt::syscall_restart,     UInterrupt::flag_alarm)

      if (event_signal_pending) callHandlerSignal();
      }

   static bool isSysCallToRestart()
      {
      U_TRACE_NO_PARAM(0, "UInterrupt::isSysCallToRestart()")

      U_INTERNAL_DUMP("errno = %d u_errno = %d UInterrupt::event_signal_pending = %d UInterrupt::syscall_restart = %b UInterrupt::flag_alarm = %b",
                       errno,     u_errno,     UInterrupt::event_signal_pending,     UInterrupt::syscall_restart,     UInterrupt::flag_alarm)

      if (syscall_restart &&
          flag_alarm == false)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static void  erase(int signo);
   static void insert(int signo, sighandler_t handler);

   static RETSIGTYPE handlerEventSignal(int signo);
};

#endif
