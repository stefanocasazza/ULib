// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    thread.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_THREAD_H
#define ULIB_THREAD_H

#include <ulib/timeval.h>

#ifdef _MSWINDOWS_
#  undef sleep
#  undef signal
#  define PTHREAD_CREATE_DETACHED 1
#else
#  define U_SIGSTOP (SIGRTMIN+5)
#  define U_SIGCONT (SIGRTMIN+6)
#endif

class UNotifier;

class U_EXPORT UThread {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // COSTRUTTORI

   UThread(int _detachstate);

   virtual ~UThread()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UThread)

      if (tid) close();
      }

   // SERVICES

#ifdef _MSWINDOWS_
   static DWORD getTID();
#else
   static pthread_t getTID();
#endif

   // Inter Process Communication

   static void lock(pthread_mutex_t* pmutex)
      {
      U_TRACE(1, "UThread::lock(%p)", pmutex)

#  ifndef _MSWINDOWS_
      (void) U_SYSCALL(pthread_mutex_lock, "%p", pmutex);
#  endif
      }

   static void unlock(pthread_mutex_t* pmutex)
      {
      U_TRACE(1, "UThread::unlock(%p)", pmutex)

#  ifndef _MSWINDOWS_
      (void) U_SYSCALL(pthread_mutex_unlock, "%p", pmutex);
#  endif
      }

#ifndef _MSWINDOWS_
   static bool initRwLock(pthread_rwlock_t* prwlock)
      {
      U_TRACE(1, "UThread::initRwLock(%p)", prwlock)

      pthread_rwlockattr_t rwlockattr;

      if (U_SYSCALL(pthread_rwlockattr_init,       "%p",    &rwlockattr)                         != 0 ||
          U_SYSCALL(pthread_rwlockattr_setpshared, "%p,%d", &rwlockattr, PTHREAD_PROCESS_SHARED) != 0 ||
          U_SYSCALL(pthread_rwlock_init,           "%p,%p", prwlock, &rwlockattr)                != 0)
         {
         U_RETURN(false);
         }

      U_RETURN(true);
      }

   static bool initIPC(pthread_mutex_t* mutex, pthread_cond_t* cond);
   static void   doIPC(pthread_mutex_t* mutex, pthread_cond_t* cond, vPF function, bool wait);
#endif

   /**
    * When a new thread is created, it does not begin immediate execution. This is because the derived class virtual tables are not properly loaded
    * at the time the C++ object is created within the constructor itself, at least in some compiler/system combinations. It can be started directly
    * after the constructor completes by calling the start() method
    *
    * @return false if execution fails
    */

   bool start(uint32_t timeoutMS = 0);

   /**
    * All threads execute by deriving the run method of UThread. This method is called after initial to begin normal operation of the
    * thread. If the method terminates, then the thread will also terminate
    */

   virtual void run()
      {
      U_TRACE(0, "UThread::run()")
      }

   /**
    * Check if this thread is detached
    *
    * @return true if the thread is detached
    */

   bool isDetached()
      {
      U_TRACE(0, "UThread::isDetached()")

      U_INTERNAL_DUMP("detachstate = %d", detachstate)

      if (detachstate == PTHREAD_CREATE_DETACHED) U_RETURN(true);

      U_RETURN(false);
      }

          void     sleep(time_t timeoutMS);
   static void nanosleep(time_t timeoutMS)
      {
      U_TRACE(0, "UThread::nanosleep(%ld)", timeoutMS)

      UThread* th = getThread();

      if (th)
         {
         th->sleep(timeoutMS);

         return;
         }

      UTimeVal(timeoutMS / 1000L, (timeoutMS % 1000L) * 1000L).nanosleep();
      }

   /**
    * Yields the current thread's CPU time slice to allow another thread to begin immediate execution
    */

   void yield();

   /**
    * Suspends execution of the selected thread. Pthreads do not normally support suspendable threads, so the behavior is simulated with signals.
    * You can't kill or stop just one thread from another process. You can send a signal to a particular thread, but the stop/abort action that
    * is taken by the signal affects the whole process. In the earlier implementation of Linux threads, it was possible to stop a single thread
    * with SIGSTOP, but this behaviour has now been fixed to conform to the Posix standard (so it stops all threads in the process)
    */

   void resume()
      {
      U_TRACE(0, "UThread::resume()")

#  ifndef _MSWINDOWS_
      resume(tid);
#   ifndef HAVE_PTHREAD_SUSPEND
      yield(); // give the signal a time to kick in
#   endif
#  endif
      }
      
   void suspend()
      {
      U_TRACE(0, "UThread::suspend()")

#  ifndef _MSWINDOWS_
      suspend(tid);
#   ifndef HAVE_PTHREAD_SUSPEND
      yield(); // give the signal a time to kick in
#   endif
#  endif
      }

   // Cancellation

   enum Cancel {
      cancelInitial,    /* used internally, do not use */
      cancelDeferred,   /* exit thread on cancellation pointsuch as yield */
      cancelImmediate,  /* exit befor cancellation */
      cancelDisabled,   /* ignore cancellation */
      cancelManual
   };

   void setCancel(int mode);

   /**
    * This is used to help build wrapper functions in libraries
    * around system calls that should behave as cancellation points but don't
    *
    * @return saved cancel type
    */

   int enterCancel();

   /**
    * This is used to restore a cancel block
    *
    * @param cancel type that was saved
    */

   void exitCancel(int cancel);

   // A special global function, getThread(), is provided to identify the thread object that represents the current
   // execution context you are running under. This is sometimes needed to deliver signals to the correct thread

   static UThread* getThread() __pure
      {
      U_TRACE(1, "UThread::getThread()")

      U_INTERNAL_DUMP("first = %p", first)

#  ifdef _MSWINDOWS_
      DWORD _tid = GetCurrentThreadId();
#  else
      pthread_t _tid = (pthread_t) U_SYSCALL_NO_PARAM(pthread_self);
#  endif

      for (UThread* obj = first; obj; obj = obj->next)
         {
#     ifdef _MSWINDOWS_
         if (_tid == obj->tid) U_RETURN_POINTER(obj, UThread);
#     else
         if (pthread_equal(_tid, obj->tid)) U_RETURN_POINTER(obj, UThread);
#     endif
         }

      U_RETURN_POINTER(0, UThread);
      }

   static bool isCurrentThread(pthread_t _tid)
      {
      U_TRACE(1, "UThread::isCurrentThread(%p)", _tid)

      U_INTERNAL_ASSERT_POINTER(_tid)

#  ifdef _MSWINDOWS_
      if (GetCurrentThreadId() == _tid) U_RETURN(true);
#  else
      if (pthread_equal((pthread_t)U_SYSCALL_NO_PARAM(pthread_self), _tid)) U_RETURN(true);
#  endif

      U_RETURN(false);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UThread* next;
   int detachstate, cancel;
#ifdef _MSWINDOWS_
   DWORD tid;
   HANDLE cancellation;
#else
   pthread_t tid;
   pthread_attr_t attr;
   int suspendCount;

   static pthread_mutex_t mlock;
#endif

   static UThread* first;

   void close();

#ifdef _MSWINDOWS_
   static void   lock() {}
   static void unlock() {}

   static void  resume(pthread_t _tid) {}
   static void suspend(pthread_t _tid) {}

   static unsigned __stdcall execHandler(void* th);
#else
   void sigInstall(int signo);
   void manageSignal(int signo);

   static void   lock() {   lock(&mlock); }
   static void unlock() { unlock(&mlock); }

   static void execHandler(UThread* th);

   static void sigHandler(int signo)
      {
      U_TRACE(0, "UThread::sigHandler(%d)", signo)

      UThread* th = getThread();

      if (th) th->manageSignal(signo);
      }

   static bool isDetached(pthread_attr_t* pattr)
      {
      U_TRACE(1, "UThread::isDetached(%p)", pattr)

      int state;

      (void) U_SYSCALL(pthread_attr_getdetachstate, "%p,%p", pattr, &state);

      if (state == PTHREAD_CREATE_DETACHED) U_RETURN(true);

      U_RETURN(false);
      }

   static void stop(pthread_t _tid, pthread_attr_t* pattr)
      {
      U_TRACE(1, "UThread::stop(%p,%p)", _tid, pattr)

#   ifdef HAVE_PTHREAD_CANCEL
      (void) U_SYSCALL(pthread_cancel, "%p", _tid);
#   endif

      if (isDetached(pattr) == false) (void) U_SYSCALL(pthread_join, "%p,%p", _tid, 0);
#   ifdef HAVE_PTHREAD_YIELD
      else (void) U_SYSCALL_NO_PARAM(pthread_yield);
#   endif
      }

   static void suspend(pthread_t _tid)
      {
      U_TRACE(1, "UThread::suspend(%p)", _tid)

      U_ASSERT_EQUALS(isCurrentThread(_tid), false)

#   ifdef HAVE_PTHREAD_SUSPEND
      (void) U_SYSCALL(pthread_suspend, "%p", _tid);
#   else
      (void) U_SYSCALL(pthread_kill, "%p,%d", _tid, U_SIGSTOP);
#   endif
      }

   static void resume(pthread_t _tid)
      {
      U_TRACE(1, "UThread::resume(%p)", _tid)

      U_ASSERT_EQUALS(isCurrentThread(_tid), false)

#   ifdef HAVE_PTHREAD_SUSPEND
      (void) U_SYSCALL(pthread_resume, "%p", _tid);
#   else
      (void) U_SYSCALL(pthread_kill, "%p,%d", _tid, U_SIGCONT);
#   endif
      }

   static void threadCleanup(void* th)
      {
      U_TRACE(0, "UThread::threadCleanup(%p)", th)

      U_INTERNAL_ASSERT_POINTER(th)

      U_INTERNAL_DUMP("th->tid = %p", ((UThread*)th)->tid)

      if (((UThread*)th)->tid) ((UThread*)th)->close();
      }
#endif

private:
   friend class UNotifier;

#ifdef U_COMPILER_DELETE_MEMBERS
   UThread(const UThread&) = delete;
   UThread& operator=(const UThread&) = delete;
#else
   UThread(const UThread&)            {}
   UThread& operator=(const UThread&) { return *this; }
#endif
};

class U_EXPORT UThreadPool {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR
};

#endif
