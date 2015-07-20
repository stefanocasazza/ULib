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

#include <ulib/internal/common.h>

#define U_SIGSTOP (SIGRTMIN+5)
#define U_SIGCONT (SIGRTMIN+6)

#ifdef _MSWINDOWS_
#undef signal
#undef sleep
#endif

class UNotifier;

class U_EXPORT UThread {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   enum Cancel {
      cancelInitial,    /* used internally, do not use */
      cancelDeferred,   /* exit thread on cancellation pointsuch as yield */
      cancelImmediate,  /* exit befor cancellation */
      cancelDisabled    /* ignore cancellation */
   };

   // COSTRUTTORI

            UThread(bool suspendEnable = false, bool joinEnable = true);
   virtual ~UThread();

   // SERVICES

   static pid_t getTID();

   static void sleep(time_t timeoutMS);

   // Inter Process Communication

   static void lock(pthread_mutex_t* mutex)
      {
      U_TRACE(1, "UThread::lock(%p)", mutex)

      (void) U_SYSCALL(pthread_mutex_lock, "%p", mutex);
      }

   static void unlock(pthread_mutex_t* mutex)
      {
      U_TRACE(1, "UThread::unlock(%p)", mutex)

      (void) U_SYSCALL(pthread_mutex_unlock, "%p", mutex);
      }

   static bool initRwLock(pthread_rwlock_t* rwlock)
      {
      U_TRACE(1, "UThread::initRwLock(%p)", rwlock)

      pthread_rwlockattr_t rwlockattr;

      if (U_SYSCALL(pthread_rwlockattr_init,       "%p",    &rwlockattr)                         != 0 ||
          U_SYSCALL(pthread_rwlockattr_setpshared, "%p,%d", &rwlockattr, PTHREAD_PROCESS_SHARED) != 0 ||
          U_SYSCALL(pthread_rwlock_init,           "%p,%p", rwlock, &rwlockattr)                 != 0)
         {
         U_RETURN(false);
         }

      U_RETURN(true);
      }

   static bool initIPC(pthread_mutex_t* mutex, pthread_cond_t* cond);
   static void   doIPC(pthread_mutex_t* mutex, pthread_cond_t* cond, vPF function, bool wait);

   /**
    * All threads execute by deriving the run method of UThread.
    * This method is called after initial to begin normal operation of the
    * thread. If the method terminates, then the thread will also terminate
    */

   virtual void run()
      {
      U_TRACE(0, "UThread::run()")
      } 

   /**
    * When a new thread is created, it does not begin immediate execution.
    * This is because the derived class virtual tables are not properly loaded
    * at the time the C++ object is created within the constructor itself, at
    * least in some compiler/system combinations. It can be started directly
    * after the constructor completes by calling the start() method
    *
    * @return false if execution fails
    */

   void stop();
   bool start(uint32_t timeoutMS = 0);

   /**
    * Start a new thread as "detached". This is an alternative
    * start() method that resolves some issues with later glibc
    * implementations which incorrectly implement self-detach
    *
    * @return false if execution fails
    */

   bool detach();

   /**
    * Yields the current thread's CPU time slice to allow another thread to begin immediate execution
    */

   void yield();

   /**
    * Suspends execution of the selected thread. Pthreads do not normally
    * support suspendable threads, so the behavior is simulated with signals
    */

   void suspend();

   /**
    * Resumes execution of the selected thread
    */

   void resume();

   /**
    * Check if this thread is detached
    *
    * @return true if the thread is detached
    */

   bool isDetached() const;

   /** 
    * Each time a thread receives a signal, it stores the signal number locally
    */

   void signal(int signo);

   // Cancellation

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

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UThread* next;

   static UThread* first;
   static pthread_cond_t cond;
   static pthread_mutex_t _lock;

   void close(); // close current thread, free all
   void sigInstall(int signo);

   static void   lock() {   lock(&_lock); }
   static void unlock() { unlock(&_lock); }

   static void sigHandler(int signo);
   static void execHandler(UThread* th);

   static void threadCleanup(UThread* th)
      {
      U_TRACE(0, "UThread::threadCleanup(%p)", th)

      th->close();
      }


   // A special global function, getThread(), is provided to identify the thread object that represents the current
   // execution context you are running under. This is sometimes needed to deliver signals to the correct thread

   static UThread* getThread() __pure;

private:
   class UThreadImpl* priv; // private data

   friend class UNotifier;
   friend class UThreadImpl;

#ifdef U_COMPILER_DELETE_MEMBERS
   UThread(const UThread&) = delete;
   UThread& operator=(const UThread&) = delete;
#else
   UThread(const UThread&)            {}
   UThread& operator=(const UThread&) { return *this; }
#endif
};

#endif
