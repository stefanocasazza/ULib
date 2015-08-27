// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    thread.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/thread.h>

#include <time.h>

#ifdef HAVE_SYS_SYSCALL_H
#  include <sys/syscall.h>
#endif

#ifndef HAVE_PTHREAD_CANCEL
#  ifdef SIGCANCEL
#     define U_SIG_THREAD_CANCEL SIGCANCEL
#  else
#     define U_SIG_THREAD_CANCEL SIGQUIT
#  endif
#endif

#ifndef HAVE_NANOSLEEP
extern "C" { int nanosleep (const struct timespec* requested_time,
                                  struct timespec* remaining); }
#endif

UThread* UThread::first;

#ifndef _MSWINDOWS_
pthread_mutex_t UThread::mlock = PTHREAD_MUTEX_INITIALIZER;
#endif

UThread::UThread(int _detachstate)
{
   U_TRACE_REGISTER_OBJECT(0, UThread, "%d", _detachstate)

   next         = 0;
   tid          = 0;
   detachstate  = _detachstate;
   cancel       = 0;

#ifdef _MSWINDOWS_
   HANDLE process = GetCurrentProcess();

   DuplicateHandle(process, GetCurrentThread(), process, (LPHANDLE)&tid, 0, FALSE, DUPLICATE_SAME_ACCESS);

   cancellation = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
   suspendCount = 0;

   (void) U_SYSCALL(pthread_attr_init,           "%p",    &attr);
   (void) U_SYSCALL(pthread_attr_setdetachstate, "%p,%d", &attr, _detachstate);
#endif
}

void UThread::close()
{
   U_TRACE(0, "UThread::close()")

#ifdef _MSWINDOWS_
   DWORD _tid = tid;
#else
   pthread_t _tid = tid; 
#endif

   tid = 0;

   U_INTERNAL_DUMP("tid = %p first = %p next = %p", _tid, first, next)

   U_INTERNAL_ASSERT_POINTER(first)

   UThread* obj;
   UThread** ptr = &first;

   while ((obj = *ptr))
      {
      U_INTERNAL_ASSERT_POINTER(obj)

#  ifdef _MSWINDOWS_
      if (tid == obj->tid)
#  else
      if (pthread_equal(tid, obj->tid))
#  endif
         {
         U_INTERNAL_ASSERT_EQUALS(this, obj)
         U_INTERNAL_ASSERT_EQUALS(next, obj->next)

         *ptr = next;
                next = 0;

         break;
         }

      ptr = &(*ptr)->next;
      }

   if (_tid)
      {
#  ifdef _MSWINDOWS_ // wait for real w32 thread to cleanup
      switch (cancel)
         {
         case cancelImmediate: TerminateThread((HANDLE)_tid, 0); break;

         default: SetEvent(cancellation);
         }

      (void) ::WaitForSingleObject((HANDLE)_tid, INFINITE);

      (void) U_SYSCALL(CloseHandle, "%p", cancellation);
      (void) U_SYSCALL(CloseHandle, "%p", (HANDLE)_tid);

      ExitThread(0);
#  else
#   ifdef HAVE_PTHREAD_CANCEL
      (void) U_SYSCALL(pthread_cancel, "%p", _tid);
#   endif

      if (detachstate == PTHREAD_CREATE_JOINABLE) (void) U_SYSCALL(pthread_join, "%p,%p", _tid, 0);
#   ifdef HAVE_PTHREAD_YIELD
      else
         {
         (void) U_SYSCALL_NO_PARAM(pthread_yield);
         }
#   endif
#  endif
      }

#ifndef _MSWINDOWS_
   (void) pthread_attr_destroy(&attr);
#endif
}

#ifdef _MSWINDOWS_
DWORD UThread::getTID()
{
   U_TRACE(0, "UThread::getTID()")

   DWORD _tid = GetCurrentThreadId();

   U_RETURN(_tid);
}
#else
pthread_t UThread::getTID()
{
   U_TRACE(0, "UThread::getTID()")

   pthread_t _tid = syscall(SYS_gettid);

   U_RETURN(_tid);
}
#endif

void UThread::yield()
{
   U_TRACE(1, "UThread::yield()")

   // Yields the current thread's CPU time slice to allow another thread to begin immediate execution

   U_INTERNAL_DUMP("cancel = %d", cancel)

#ifdef HAVE_PTHREAD_CANCEL
   U_SYSCALL_VOID_NO_PARAM(pthread_testcancel);
#elif !defined(_MSWINDOWS_)
   sigset_t old = 0;

   if (cancel != cancelInitial &&
       cancel != cancelDisabled)
      {
      sigset_t scancel;

#  ifdef sigemptyset
                       sigemptyset(&scancel);
#  else
      (void) U_SYSCALL(sigemptyset, "%p", &scancel);
#  endif

#  ifdef sigaddset
                       sigaddset(&scancel, U_SIG_THREAD_CANCEL);
#  else
      (void) U_SYSCALL(sigaddset, "%p,%d", &scancel, U_SIG_THREAD_CANCEL);
#  endif

      (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_UNBLOCK, &scancel, &old);
      }
#endif

#ifdef HAVE_PTHREAD_YIELD
   (void) U_SYSCALL_NO_PARAM(pthread_yield);
#endif

#if !defined(HAVE_PTHREAD_CANCEL) && !defined(_MSWINDOWS_)
   if (old) (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_SETMASK, &old, 0);
#endif
}

#ifndef _MSWINDOWS_
void UThread::sigInstall(int signo)
{
   U_TRACE(1, "UThread::sigInstall(%d)", signo)

   struct sigaction sa;

#ifdef sigemptyset
                    sigemptyset(&sa.sa_mask);
#else
   (void) U_SYSCALL(sigemptyset, "%p", &sa.sa_mask);
#endif

#ifdef SA_RESTART
   sa.sa_flags  = SA_RESTART;
#else
   s.sa_flags   = 0;
#endif

#ifdef SA_INTERRUPT
   sa.sa_flags |= SA_INTERRUPT;
#endif
   sa.sa_handler = sigHandler;

   (void) U_SYSCALL(sigaction, "%d,%p,%p", signo,  &sa, 0);
}

void UThread::manageSignal(int signo)
{
   U_TRACE(1, "UThread::manageSignal(%d)", signo)

   // Mutexes are used to ensure the exclusive access, where as condition variables are used to synchronize threads based on the events.
   // We need Mutexes to ensure that condition variables dont end up in an infinite wait. One thing to remember is Mutex operation of lock
   // and unlock are guaranteed to be atomic, but the condition variables need not be. i.e The thread can get scheduled out while the condition variable wait is half way

   static pthread_cond_t  mcond = PTHREAD_COND_INITIALIZER;

   U_INTERNAL_DUMP("suspendCount = %d", suspendCount)

   (void) U_SYSCALL(pthread_mutex_lock, "%p", &mlock);

   if (signo == U_SIGSTOP &&
       suspendCount++ == 0)
      {
      U_INTERNAL_DUMP("SUSPEND: start(%2D)")

      // NB: A thread can wake up from pthread_cond_wait() at any name, not necessarily only when it is signalled.
      // This means that you need to pair pthread_cond_wait() with some shared state that encodes the condition that the thread is really waiting for

      do {
         (void) U_SYSCALL(pthread_cond_wait, "%p,%p", &mcond, &mlock); // NB: pthread_cond_wait() requires that the mutex be locked already when you call it
         }
      while (suspendCount == 1);

      U_INTERNAL_DUMP("SUSPEND: end(%2D)")
      }
   else if (signo == U_SIGCONT  &&
            suspendCount    > 0 &&
            suspendCount-- == 1)
      {
      (void) U_SYSCALL(pthread_cond_signal, "%p", &mcond);
      }

   (void) U_SYSCALL(pthread_mutex_unlock, "%p", &mlock);
}
#endif

#ifdef _MSWINDOWS_
unsigned __stdcall UThread::execHandler(void* th)
{
   U_TRACE(0, "UThread::::execHandler(%p)", th)

   U_INTERNAL_DUMP("th->tid = %p", ((UThread*)th)->tid)

   ((UThread*)th)->setCancel(cancelDeferred);

   ((UThread*)th)->run();

   ((UThread*)th)->close();

   U_RETURN(0);
}
#else
void UThread::execHandler(UThread* th)
{
   U_TRACE(1, "UThread::execHandler(%p)", th)

   sigset_t mask;

   th->tid = (pthread_t) U_SYSCALL_NO_PARAM(pthread_self);

   U_INTERNAL_DUMP("th->tid = %p", th->tid)

#ifdef sigemptyset
   sigemptyset(&mask);
#else
   (void) U_SYSCALL(sigemptyset, "%p", &mask);
#endif

#ifdef sigaddset
// sigaddset(&mask, SIGHUP);
   sigaddset(&mask, SIGINT);
   sigaddset(&mask, SIGABRT);
   sigaddset(&mask, SIGPIPE);
   sigaddset(&mask, SIGALRM);
#else
// (void) U_SYSCALL(sigaddset, "%p,%d", &mask, SIGHUP);
   (void) U_SYSCALL(sigaddset, "%p,%d", &mask, SIGINT);
   (void) U_SYSCALL(sigaddset, "%p,%d", &mask, SIGABRT);
   (void) U_SYSCALL(sigaddset, "%p,%d", &mask, SIGPIPE);
   (void) U_SYSCALL(sigaddset, "%p,%d", &mask, SIGALRM);
#endif

   (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_BLOCK, &mask, 0);

#ifndef HAVE_PTHREAD_SUSPEND
# ifdef sigemptyset
                    sigemptyset(&mask);
# else
   (void) U_SYSCALL(sigemptyset, "%p", &mask);
# endif

# ifdef sigaddset
                    sigaddset(&mask, U_SIGSTOP);
                    sigaddset(&mask, U_SIGCONT);
# else
   (void) U_SYSCALL(sigaddset, "%p,%d", &mask, U_SIGSTOP);
   (void) U_SYSCALL(sigaddset, "%p,%d", &mask, U_SIGCONT);
# endif

   (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_UNBLOCK, &mask, 0);

   th->sigInstall(U_SIGSTOP);
   th->sigInstall(U_SIGCONT);
#endif

   pthread_cleanup_push(threadCleanup, th);

   th->setCancel(cancelDeferred);

   th->run();

   pthread_cleanup_pop(0);

   U_INTERNAL_DUMP("th->tid = %p", th->tid)

   if (th->tid) th->close();
}
#endif

bool UThread::start(uint32_t timeoutMS)
{
   U_TRACE(1, "UThread::start(%u)", timeoutMS)

   U_INTERNAL_ASSERT_EQUALS(tid, 0)

#if defined(DEBUG) && !defined(_MSWINDOWS_)
   if (u_plock == 0)
      {
      static pthread_mutex_t plock = PTHREAD_MUTEX_INITIALIZER;

      u_plock = &plock;
      }
#endif

   next  = first;
   first = this;

   U_INTERNAL_DUMP("first = %p next = %p", first, next)

#ifdef _MSWINDOWS_
   (void) _beginthreadex(NULL, 0, execHandler, this, CREATE_SUSPENDED, (unsigned*)&tid);

   if (tid == 0)
       {
       CloseHandle(cancellation);
                   cancellation = 0;

       U_RETURN(false);
       }

   setCancel(cancelInitial);

   SetThreadPriority((HANDLE)tid, THREAD_PRIORITY_NORMAL);

   ResumeThread((HANDLE)tid);
#else
   if (U_SYSCALL(pthread_create, "%p,%p,%p,%p", &tid, &attr, (pvPFpv)execHandler, this) == 0)
      {
      if (timeoutMS)
         {
         // give at the thread the time to initialize...

         struct timespec ts = { 0L, 1000000L };

         ts.tv_nsec *= (long)timeoutMS;

         (void) U_SYSCALL(nanosleep, "%p,%p", &ts, 0);
         }

      U_RETURN(true);
      }
#endif

   U_RETURN(false);
}

void UThread::setCancel(int mode)
{
   U_TRACE(1, "UThread::setCancel(%d)", mode)

   int old;

   switch (mode)
      {
      case cancelImmediate:
      case cancelDeferred:
         {
#     ifdef HAVE_PTHREAD_CANCEL
         (void) U_SYSCALL(pthread_setcancelstate, "%d,%p",                           PTHREAD_CANCEL_ENABLE,        &old);
         (void) U_SYSCALL(pthread_setcanceltype,  "%d,%p", (mode == cancelDeferred ? PTHREAD_CANCEL_DEFERRED
                                                                                   : PTHREAD_CANCEL_ASYNCHRONOUS), &old);
#     endif
         }
      break;

      case cancelInitial:
      case cancelDisabled:
#  ifdef HAVE_PTHREAD_CANCEL
         (void) U_SYSCALL(pthread_setcancelstate, "%d,%p", PTHREAD_CANCEL_DISABLE, &old);
#  endif
      break;
      }

   cancel = mode;

   U_INTERNAL_DUMP("cancel = %d", cancel)
}

int UThread::enterCancel()
{
   U_TRACE(1, "UThread::enterCancel()")

   U_INTERNAL_DUMP("cancel = %d", cancel)

   int old = cancel;

   if (old != cancelDisabled &&
       old != cancelImmediate)
      {
      setCancel(cancelImmediate);

#  ifdef HAVE_PTHREAD_CANCEL
      U_SYSCALL_VOID_NO_PARAM(pthread_testcancel);
#  elif defined(_MSWINDOWS_)
      yield();
#  endif
      }

   U_RETURN(old);
}

void UThread::exitCancel(int old)
{
   U_TRACE(1, "UThread::exitCancel(%d)", old)

   U_INTERNAL_DUMP("cancel = %d", cancel)

   if (old != cancel)
      {
#  ifdef HAVE_PTHREAD_CANCEL
      U_SYSCALL_VOID_NO_PARAM(pthread_testcancel);
#  endif

      setCancel(old);
      }
}

void UThread::sleep(time_t timeoutMS)
{
   U_TRACE(1, "UThread::sleep(%ld)", timeoutMS)

   struct timespec ts = { timeoutMS / 1000L, (timeoutMS % 1000L) * 1000000L };

   U_INTERNAL_ASSERT(ts.tv_sec >= 0L)
   U_INTERNAL_ASSERT_RANGE(0L, ts.tv_nsec, 999999999L)

#ifdef HAVE_PTHREAD_CANCEL
   int old = enterCancel();
#endif

#ifdef _MSWINDOWS_
   switch (cancel)
      {
      case cancelInitial:
      case cancelDisabled: SleepEx(timeoutMS, FALSE); break;

      default:
         {
         if (WaitForSingleObject(cancellation, timeoutMS) == WAIT_OBJECT_0)
            {
            if (cancel != cancelManual) close();
            }
         }
      }
#endif

#ifdef HAVE_PTHREAD_DELAY
   (void) pthread_delay(&ts);
#elif defined(HAVE_PTHREAD_DELAY_NP)
   (void) pthread_delay_np(&ts);
#else
   U_INTERNAL_DUMP("Call   nanosleep(%2D)")

   (void) U_SYSCALL(nanosleep, "%p,%p", &ts, 0);

   U_INTERNAL_DUMP("Return nanosleep(%2D)")
#endif

#ifdef HAVE_PTHREAD_CANCEL
   exitCancel(old);
#endif
}

// Inter Process Communication

#ifndef _MSWINDOWS_
bool UThread::initIPC(pthread_mutex_t* pmutex, pthread_cond_t* pcond)
{
   U_TRACE(0, "UThread::initIPC(%p,%p)", pmutex, pcond)

   if (pmutex) /* initialize mutex */
      {
      pthread_mutexattr_t mutexattr;

      if (U_SYSCALL(pthread_mutexattr_init,       "%p",    &mutexattr)                         != 0 ||
          U_SYSCALL(pthread_mutexattr_setrobust,  "%p,%d", &mutexattr, PTHREAD_MUTEX_ROBUST)   != 0 ||
          U_SYSCALL(pthread_mutexattr_setpshared, "%p,%d", &mutexattr, PTHREAD_PROCESS_SHARED) != 0 ||
          U_SYSCALL(pthread_mutex_init,           "%p,%p", pmutex,   &mutexattr)               != 0)
         {
         U_RETURN(false);
         }
      }

   if (pcond) /* initialize condition variable */
      {
      pthread_condattr_t condattr;

      if (U_SYSCALL(pthread_condattr_init,       "%p",    &condattr)                         != 0 ||
          U_SYSCALL(pthread_condattr_setpshared, "%p,%d", &condattr, PTHREAD_PROCESS_SHARED) != 0 ||
          U_SYSCALL(pthread_cond_init,           "%p,%p", pcond, &condattr)                  != 0)
         {
         U_RETURN(false);
         }
      }

   U_RETURN(true);
}

void UThread::doIPC(pthread_mutex_t* pmutex, pthread_cond_t* pcond, vPF function, bool wait)
{
   U_TRACE(0, "UThread::doIPC(%p,%p,%p,%b)", pmutex, pcond, function, wait)

   lock(pmutex);

   if (wait) (void) U_SYSCALL(pthread_cond_wait, "%p,%p", pcond, pmutex); // block until we are signalled from other...

   function(); // ...than call function

   unlock(pmutex);

   if (wait == false) (void) U_SYSCALL(pthread_cond_signal, "%p", pcond); // signal to waiting thread...
}
#endif

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UThread::dump(bool reset) const
{
   *UObjectIO::os << "tid            " << tid          << '\n'
                  << "cancel         " << cancel       << '\n'
                  << "detachstate    " << detachstate << '\n'
                  << "next  (UThread " << (void*)next  << ")\n"
                  << "first (UThread " << (void*)first << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
