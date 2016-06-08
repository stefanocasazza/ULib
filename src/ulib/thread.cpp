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

#ifndef _MSWINDOWS_
#  ifndef HAVE_PTHREAD_CANCEL
#     ifdef SIGCANCEL
#        define U_SIG_THREAD_CANCEL SIGCANCEL
#     else
#        define U_SIG_THREAD_CANCEL SIGQUIT
#     endif
#  endif
#endif

#include <time.h>

#ifndef HAVE_NANOSLEEP
extern "C" { int nanosleep (const struct timespec* requested_time,
                                  struct timespec* remaining); }
#endif

UThread* UThread::first;

void UThread::close()
{
   U_TRACE_NO_PARAM(0, "UThread::close()")

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

      (void) WaitForSingleObject((HANDLE)_tid, INFINITE);

      (void) U_SYSCALL(CloseHandle, "%p", cancellation);
      (void) U_SYSCALL(CloseHandle, "%p", (HANDLE)_tid);

      _endthreadex(0);
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
}

void UThread::yield()
{
   U_TRACE_NO_PARAM(1, "UThread::yield()")

   // Yields the current thread's CPU time slice to allow another thread to begin immediate execution

   U_INTERNAL_DUMP("cancel = %d", cancel)

#ifdef HAVE_PTHREAD_CANCEL
   U_SYSCALL_VOID_NO_PARAM(pthread_testcancel);
#elif !defined(_MSWINDOWS_) && !defined(__UNIKERNEL__)
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

#if !defined(HAVE_PTHREAD_CANCEL) && !defined(_MSWINDOWS_) && !defined(__UNIKERNEL__)
   if (old) (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_SETMASK, &old, 0);
#endif
}

#ifdef _MSWINDOWS_
void UThread::wait(CRITICAL_SECTION* pmutex, CONDITION_VARIABLE* pcond)
{
   U_TRACE(0, "UThread::wait(%p,%p)", pmutex, pcond)

#if _WIN32_WINNT >= 0x0600
   SleepConditionVariableCS(pcond, pmutex, INFINITE); // block until we are signalled from other...
#endif
}

void UThread::signal(CONDITION_VARIABLE* pcond)
{
   U_TRACE(0, "UThread::signal(%p)", pcond)

#if _WIN32_WINNT >= 0x0600
   WakeConditionVariable(pcond); // signal to waiting thread...
#endif
}

void UThread::signalAll(CONDITION_VARIABLE* pcond)
{
   U_TRACE(0, "UThread::signalAll(%p)", pcond)

#if _WIN32_WINNT >= 0x0600
   WakeAllConditionVariable(pcond); // signal to waiting thread...
#endif
}
#else
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

   static pthread_cond_t  mcond = PTHREAD_COND_INITIALIZER;
   static pthread_mutex_t mlock = PTHREAD_MUTEX_INITIALIZER;

   // Mutexes are used to ensure the exclusive access, where as condition variables are used to synchronize threads based on the events.
   // We need Mutexes to ensure that condition variables dont end up in an infinite wait. One thing to remember is Mutex operation of lock
   // and unlock are guaranteed to be atomic, but the condition variables need not be. i.e The thread can get scheduled out while the condition variable wait is half way

   lock(&mlock);

   U_INTERNAL_DUMP("suspendCount = %d", suspendCount)

   if (signo == U_SIGSTOP &&
       suspendCount++ == 0)
      {
      U_INTERNAL_DUMP("SUSPEND: start(%2D)")

      // A thread can wake up from pthread_cond_wait() at any name, not necessarily only when it is signalled.
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

   unlock(&mlock);
}

void UThread::maskSignal()
{
   U_TRACE_NO_PARAM(1, "UThread::maskSignal()")

   sigset_t mask;

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

   sigInstall(U_SIGSTOP);
   sigInstall(U_SIGCONT);
#endif
}

bool UThread::initRwLock(pthread_rwlock_t* prwlock)
{
   U_TRACE(1, "UThread::initRwLock(%p)", prwlock)

   pthread_rwlockattr_t rwlockattr;

   if (U_SYSCALL(pthread_rwlockattr_init,       "%p",    &rwlockattr)                         != 0 ||
#     if defined(LINUX) || defined(__LINUX__) || defined(__linux__)
       U_SYSCALL(pthread_rwlockattr_setpshared, "%p,%d", &rwlockattr, PTHREAD_PROCESS_SHARED) != 0 ||
#     endif
       U_SYSCALL(pthread_rwlock_init,           "%p,%p", prwlock, &rwlockattr)                != 0)
      {
      U_RETURN(false);
      }

   U_RETURN(true);
}

bool UThread::initIPC(pthread_mutex_t* pmutex, pthread_cond_t* pcond)
{
   U_TRACE(0, "UThread::initIPC(%p,%p)", pmutex, pcond)

   if (pmutex) /* initialize mutex */
      {
      pthread_mutexattr_t mutexattr;

      if (U_SYSCALL(pthread_mutexattr_init,       "%p",    &mutexattr)                         != 0 ||
#        if defined(LINUX) || defined(__LINUX__) || defined(__linux__)
          U_SYSCALL(pthread_mutexattr_setrobust,  "%p,%d", &mutexattr, PTHREAD_MUTEX_ROBUST)   != 0 ||
          U_SYSCALL(pthread_mutexattr_setpshared, "%p,%d", &mutexattr, PTHREAD_PROCESS_SHARED) != 0 ||
#        endif
          U_SYSCALL(pthread_mutex_init,           "%p,%p", pmutex, &mutexattr)                 != 0)
         {
         U_RETURN(false);
         }
      }

   if (pcond) /* initialize condition variable */
      {
      pthread_condattr_t condattr;

      if (U_SYSCALL(pthread_condattr_init,       "%p",    &condattr)                         != 0 ||
#        if defined(LINUX) || defined(__LINUX__) || defined(__linux__)
          U_SYSCALL(pthread_condattr_setpshared, "%p,%d", &condattr, PTHREAD_PROCESS_SHARED) != 0 ||
#        endif
          U_SYSCALL(pthread_cond_init,           "%p,%p", pcond, &condattr)                  != 0)
         {
         U_RETURN(false);
         }
      }

   U_RETURN(true);
}

void UThread::doIPC(pthread_mutex_t* plock, pthread_cond_t* pcond, vPF function, bool wait)
{
   U_TRACE(0, "UThread::doIPC(%p,%p,%p,%b)", plock, pcond, function, wait)

   lock(plock);

   if (wait) (void) U_SYSCALL(pthread_cond_wait, "%p,%p", pcond, plock); // block until we are signalled from other...

   function(); // ...than call function

   unlock(plock);

   if (wait == false) (void) U_SYSCALL(pthread_cond_signal, "%p", pcond); // signal to waiting thread...
}
#endif

bool UThread::start(uint32_t timeoutMS)
{
   U_TRACE(1, "UThread::start(%u)", timeoutMS)

   U_INTERNAL_ASSERT_EQUALS(tid, 0)

   next  = first;
   first = this;

   U_INTERNAL_DUMP("first = %p next = %p", first, next)

#ifdef _MSWINDOWS_
   HANDLE process = GetCurrentProcess();

   (void) DuplicateHandle(process, GetCurrentThread(), process, (LPHANDLE)&tid, 0, FALSE, DUPLICATE_SAME_ACCESS);

   cancellation = CreateEvent(NULL, TRUE, FALSE, NULL);

   // This starts a free standing procedure as a thread.
   // That thread instantiates the class and calls its main method
   void* NO_SECURITY_ATTRIBUTES = NULL;
   const unsigned CREATE_IN_RUN_STATE    = 0;
   const unsigned USE_DEFAULT_STACK_SIZE = 0;

   if (_beginthreadex(NO_SECURITY_ATTRIBUTES, USE_DEFAULT_STACK_SIZE, execHandler, this, CREATE_IN_RUN_STATE, (unsigned int*)&id) == 0)
      {
      int m_thread_start_error;
      errno_t m_return_value = _get_errno(&m_thread_start_error);

      if (m_return_value == 0)
         {
         U_WARNING("Create Thread Fail, error %d", m_thread_start_error);
         }
      else
         {
         U_WARNING("Create Thread Fail, get_errno fail, returned %d", m_return_value);
         }

      CloseHandle(cancellation);
                cancellation = 0;

      U_RETURN(false);
      }

   setCancel(cancelInitial);

   SetThreadPriority((HANDLE)tid, THREAD_PRIORITY_NORMAL);
#else
   bool result;
   pthread_attr_t attr;
   
   (void) U_SYSCALL(pthread_attr_init,           "%p",    &attr);
   (void) U_SYSCALL(pthread_attr_setdetachstate, "%p,%d", &attr, detachstate);

   result = (U_SYSCALL(pthread_create, "%p,%p,%p,%p", &tid, &attr, (pvPFpv)execHandler, this) == 0);

   (void) pthread_attr_destroy(&attr);

   if (result)
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

   switch (mode)
      {
      case cancelDeferred:
      case cancelImmediate:
         {
#     ifdef HAVE_PTHREAD_CANCEL
         int old;

         (void) U_SYSCALL(pthread_setcancelstate, "%d,%p",                           PTHREAD_CANCEL_ENABLE,        &old);
         (void) U_SYSCALL(pthread_setcanceltype,  "%d,%p", (mode == cancelDeferred ? PTHREAD_CANCEL_DEFERRED
                                                                                   : PTHREAD_CANCEL_ASYNCHRONOUS), &old);
#     endif
         }
      break;

      case cancelInitial:
      case cancelDisabled:
         {
#     ifdef HAVE_PTHREAD_CANCEL
         int old;

         (void) U_SYSCALL(pthread_setcancelstate, "%d,%p", PTHREAD_CANCEL_DISABLE, &old);
#     endif
         }
      break;
      }

   cancel = mode;

   U_INTERNAL_DUMP("cancel = %d", cancel)
}

int UThread::enterCancel()
{
   U_TRACE_NO_PARAM(1, "UThread::enterCancel()")

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

// U_ASSERT(isCurrentThread(tid))

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

// THREAD POOL

UThreadPool::UThreadPool(uint32_t size) : UThread(PTHREAD_CREATE_DETACHED), pool(size)
{
   U_TRACE_REGISTER_OBJECT(0, UThreadPool, "%u", size)

   UThread* th;

   active = true;

#if defined(_MSWINDOWS_)
   // Task queue mutex
   InitializeCriticalSection(&tasks_mutex);
# if _WIN32_WINNT >= 0x0600
   // Condition variable
   InitializeConditionVariable(&condition);
   InitializeConditionVariable(&condition_task_finished);
# endif
   // This starts a free standing procedure as a thread.
   // That thread instantiates the class and calls its main method
   void* NO_SECURITY_ATTRIBUTES = NULL;
   const unsigned CREATE_IN_RUN_STATE    = 0;
   const unsigned USE_DEFAULT_STACK_SIZE = 0;

   for (uint32_t i = 0; i < size; ++i)
      {
      U_NEW(UThread, th, UThread(UThread::detachstate));

      if (_beginthreadex(NO_SECURITY_ATTRIBUTES, USE_DEFAULT_STACK_SIZE, execHandler, this, CREATE_IN_RUN_STATE, (unsigned int*)&id) == 0)
         {
         int m_thread_start_error;
         errno_t m_return_value = _get_errno(&m_thread_start_error);

         if (m_return_value == 0)
            {
            U_WARNING("Create Thread Fail, error %d", m_thread_start_error);
            }
         else
            {
            U_WARNING("Create Thread Fail, get_errno fail, returned %d", m_return_value);
            }

         delete th;

         continue;
         }

      pool.push_back(th);
      }
#else
   pthread_attr_t attr;

   // Task queue mutex
   tasks_mutex = PTHREAD_MUTEX_INITIALIZER;
   // Condition variable
   condition               =
   condition_task_finished = PTHREAD_COND_INITIALIZER;

   (void) pthread_attr_init(&attr);
   (void) pthread_attr_setdetachstate(&attr, UThread::detachstate);

   for (uint32_t i = 0; i < size; ++i)
      {
      U_NEW(UThread, th, UThread(UThread::detachstate));

      if (pthread_create(&(th->tid), &attr, (pvPFpv)execHandler, this))
         {
         delete th;

         continue;
         }

      pool.push(th);
      }

   (void) pthread_attr_destroy(&attr);
#endif

   UTimeVal::nanosleep(200); // wait for UThreadPool init completion
}

UThreadPool::~UThreadPool()
{
   U_TRACE_UNREGISTER_OBJECT(0, UThread)

   active = false;

   lock(&tasks_mutex);

   signalAll(&condition);

   queue.clear();

   unlock(&tasks_mutex);

#ifdef _MSWINDOWS_
   DeleteCriticalSection(&tasks_mutex);
#endif
}

// define method VIRTUAL of class UThread

void UThreadPool::run()
{
   U_TRACE_NO_PARAM(0, "UThreadPool::run()")

   UThread* current_task;

   do {
      // We need to put pthread_cond_wait in a loop for two reasons:
      //
      // 1. There can be spurious wakeups (due to signal/ENITR)
      //
      // 2. When tasks_mutex is released for waiting, another thread can be waken up from a signal/broadcast and that
      //    thread can miss up the condition. So when the current thread wakes up the condition may no longer be actually true!

      lock(&tasks_mutex);

      while (queue._length == 0 && active)
         {
         // Wait until there is a task in the queue

         wait(&tasks_mutex, &condition); // Unlock tasks_mutex while wait, then lock it back when signaled
         }

      if (active == false) // Destructor ordered on abort
         {
         unlock(&tasks_mutex);

         return;
         }

      // If we got here, we successfully acquired the lock and the queue<Task> is not empty

      current_task = queue.pop();

      unlock(&tasks_mutex);

      current_task->run(); // execute the task

      delete current_task;

      signal(&condition_task_finished);
      }
   while (active);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UThread::dump(bool reset) const
{
   *UObjectIO::os << "id             " << id           << '\n'
                  << "tid            " << (void*)tid   << '\n'
                  << "cancel         " << cancel       << '\n'
                  << "detachstate    " << detachstate  << '\n'
                  << "next  (UThread " << (void*)next  << ")\n"
                  << "first (UThread " << (void*)first << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UThreadPool::dump(bool reset) const
{
   *UObjectIO::os << "active         " << active        << '\n'
                  << "pool  (UVector " << (void*)&pool  << ")\n"
                  << "queue (UVector " << (void*)&queue << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
