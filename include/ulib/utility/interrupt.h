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
   static sigset_t* mask_interrupt; // SIGALRM | SIGUSR[1|2] | SIGCHLD
   static sigset_t mask_wait_for_signal;
   static sig_atomic_t flag_wait_for_signal;
   static bool exit_loop_wait_event_for_signal;

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

   static RETSIGTYPE handlerInterrupt(int signo)
      {
      U_TRACE(0, "UInterrupt::handlerInterrupt(%d)", signo)

      U_MESSAGE("(pid %P) program interrupt - %Y", signo);

      if (signo == SIGBUS || //  7
          signo == SIGSEGV)  // 11 
         {
#     ifdef DEBUG
         u_debug_at_exit(); 
#     endif
         }

   // U_EXIT(-1);

      sendOurselves(signo);
      }

   static RETSIGTYPE handlerSignal(int signo)
      {
      U_TRACE(0, "[SIGNAL] UInterrupt::handlerSignal(%d)", signo)

      flag_wait_for_signal = false;
      }

   static void setHandlerForSignal(int signo, sighandler_t function)
      {
      U_TRACE(1, "UInterrupt::setHandlerForSignal(%d,%p)", signo, function)

      act.sa_handler = function;

      (void) U_SYSCALL(sigaction, "%d,%p,%p", signo, &act, U_NULLPTR);
      }

   static void setNoZombies()
      {
      U_TRACE_NO_PARAM(1, "UInterrupt::setNoZombies()")

      act.sa_flags   = SA_NOCLDWAIT;
      act.sa_handler = (sighandler_t)SIG_DFL;

      (void) U_SYSCALL(sigaction, "%d,%p,%p", SIGCHLD, &act, U_NULLPTR);

      act.sa_flags = 0;
      }

   static void waitForSignal(int signo)
      {
      U_TRACE(1, "UInterrupt::waitForSignal(%d)", signo)

      flag_wait_for_signal = true;

      setHandlerForSignal(signo, (sighandler_t)handlerSignal);

      while (flag_wait_for_signal == true) (void) U_SYSCALL(sigsuspend, "%p", &mask_wait_for_signal);
      }

   static void sendOurselves(int signo) // Send ourselves the signal: see http://www.cons.org/cracauer/sigint.html
      {
      U_TRACE(0, "UInterrupt::sendOurselves(%d)", signo)

      setHandlerForSignal(signo, (sighandler_t)SIG_DFL);

      u_exit();

      (void) U_SYSCALL(kill, "%d,%d", u_pid, signo);
      }

   static bool sendSignal(int signo, pid_t pid)
      {
      U_TRACE(1, "UInterrupt::sendSignal(%d,%d)", signo, pid)

      if (U_SYSCALL(kill, "%d,%d", pid, signo) != -1) U_RETURN(true);

      U_RETURN(false);
      }

   static bool enable(sigset_t* mask)
      {
      U_TRACE(1, "UInterrupt::enable(%p)", mask)

      if (!mask)
         {
         if (!mask_interrupt) setMaskInterrupt(U_NULLPTR, 0);

         mask = mask_interrupt;
         }

      if (U_SYSCALL(sigprocmask, "%d,%p,%p", SIG_BLOCK, mask, U_NULLPTR) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool disable(sigset_t* mask, sigset_t* mask_old)
      {
      U_TRACE(1, "UInterrupt::disable(%p,%p)", mask, mask_old)

      if (!mask)
         {
         if (!mask_interrupt) setMaskInterrupt(U_NULLPTR, 0);

         mask = mask_interrupt;
         }

      if (U_SYSCALL(sigprocmask, "%d,%p,%p", SIG_BLOCK, mask, mask_old) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   // manage sync signal
  
   static bool flag_alarm;
   static struct itimerval timerval;

   static RETSIGTYPE handlerAlarm(int signo)
      {
      U_TRACE(0, "[SIGALRM] UInterrupt::handlerAlarm(%d)", signo)

      U_INTERNAL_DUMP("timerval.it_value = { %ld %6ld } timerval.it_interval = { %ld %6ld }",
                       timerval.it_value.tv_sec, timerval.it_value.tv_usec, timerval.it_interval.tv_sec, timerval.it_interval.tv_usec)

      if (signo) flag_alarm = (signo == SIGALRM);
      else  
         {
         timerval.it_value.tv_sec  =
         timerval.it_value.tv_usec = 0;

         (void) U_SYSCALL(setitimer, "%d,%p,%p", ITIMER_REAL, &timerval, U_NULLPTR);
         }
      }

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

   static RETSIGTYPE handlerSegv(int signo) { handlerSegvWithInfo(signo, U_NULLPTR, U_NULLPTR); } 

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

   static void erase(int signo)
      {
      U_TRACE(1, "UInterrupt::erase(%d)", signo)

      U_INTERNAL_ASSERT_RANGE(1, signo, NSIG)
      U_INTERNAL_ASSERT_POINTER(handler_signal[signo])

      handler_signal[signo] = U_NULLPTR;

      (void) U_SYSCALL(sigaction, "%d,%p,%p", signo, old + signo, U_NULLPTR);
      }

   static void insert(int signo, sighandler_t handler)
      {
      U_TRACE(1, "UInterrupt::insert(%d,%p)", signo, handler)

      U_INTERNAL_ASSERT_RANGE(1, signo, NSIG)

      handler_signal[signo] = handler;

      act.sa_handler = handlerEventSignal;

      (void) U_SYSCALL(sigaction, "%d,%p,%p", signo, &act, old + signo);
      }

   static RETSIGTYPE handlerEventSignal(int signo)
      {
#  ifdef DEBUG
      u_trace_unlock();
#  endif

      U_TRACE(0, "[SIGNAL] UInterrupt::handlerEventSignal(%d)", signo)

      ++event_signal[signo];

      event_signal_pending = (event_signal_pending ? NSIG : signo);
      }
};

#endif
