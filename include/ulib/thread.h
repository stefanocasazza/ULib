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
#include <ulib/container/vector.h>

#ifdef U_LINUX
#  define U_SIGSTOP (SIGRTMIN+5)
#  define U_SIGCONT (SIGRTMIN+6)
#elif defined(_MSWINDOWS_)
#  undef sleep
#  undef signal
#  define PTHREAD_CREATE_DETACHED 1
#else
#  define U_SIGSTOP SIGSTOP
#  define U_SIGCONT SIGCONT
#endif

class UNotifier;
class UThreadPool;
class UServer_Base;

class U_EXPORT UThread {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UThread(int _detachstate = 0)
      {
      U_TRACE_REGISTER_OBJECT(0, UThread, "%d", _detachstate)

      next         = 0;
      detachstate  = _detachstate;
      cancel       = 0;
      id           = 0;
      tid          = 0;
#  ifdef _MSWINDOWS_
      cancellation = 0;
#  else
      suspendCount = 0;
#  endif
      }

   virtual ~UThread()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UThread)

      if (tid)
         {
#     ifndef _MSWINDOWS_
         if (isDetached()) suspend();
#     endif

         close();
         }
      }

   // SERVICES

#ifdef _MSWINDOWS_
   static void lock(CRITICAL_SECTION* pmutex)
      {
      U_TRACE(0, "UThread::lock(%p)", pmutex)

      EnterCriticalSection(pmutex);
      }

   static void unlock(CRITICAL_SECTION* pmutex)
      {
      U_TRACE(0, "UThread::unlock(%p)", pmutex)

      LeaveCriticalSection(pmutex);
      }

   static void signal(   CONDITION_VARIABLE* pcond);
   static void signalAll(CONDITION_VARIABLE* pcond);
   static void wait(CRITICAL_SECTION* pmutex, CONDITION_VARIABLE* pcond);
#else
   static void lock(pthread_mutex_t* pmutex)
      {
      U_TRACE(1, "UThread::lock(%p)", pmutex)

      (void) U_SYSCALL(pthread_mutex_lock, "%p", pmutex);
      }

   static void unlock(pthread_mutex_t* pmutex)
      {
      U_TRACE(1, "UThread::unlock(%p)", pmutex)

      (void) U_SYSCALL(pthread_mutex_unlock, "%p", pmutex);
      }

   static void wait(pthread_mutex_t* pmutex, pthread_cond_t* pcond)
      {
      U_TRACE(0, "UThread::wait(%p,%p)", pmutex, pcond)

      (void) U_SYSCALL(pthread_cond_wait, "%p,%p", pcond, pmutex); // block until we are signalled from other...
      }

   static void signal(pthread_cond_t* pcond)
      {
      U_TRACE(0, "UThread::signal(%p)", pcond)

      (void) U_SYSCALL(pthread_cond_signal, "%p", pcond); // signal to waiting thread...
      }

   static void signalAll(pthread_cond_t* pcond)
      {
      U_TRACE(0, "UThread::signalAll(%p)", pcond)

      (void) U_SYSCALL(pthread_cond_broadcast, "%p", pcond); // signal to waiting thread...
      }

   static bool initRwLock(pthread_rwlock_t* prwlock);

   // Inter Process Communication

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
      U_TRACE_NO_PARAM(0, "UThread::run()")
      }

   /**
    * Check if this thread is detached
    *
    * @return true if the thread is detached
    */

   bool isDetached()
      {
      U_TRACE_NO_PARAM(0, "UThread::isDetached()")

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

      UTimeVal::nanosleep(timeoutMS);
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

#ifdef _MSWINDOWS_
   void  resume() {}
   void suspend() {}
#else
   void resume()
      {
      U_TRACE_NO_PARAM(0, "UThread::resume()")

      U_ASSERT_EQUALS(isCurrentThread(tid), false)

#   ifdef HAVE_PTHREAD_SUSPEND
      (void) U_SYSCALL(pthread_resume, "%p", tid);
#   else
      (void) U_SYSCALL(pthread_kill, "%p,%d", tid, U_SIGCONT);

      yield(); // give the signal a time to kick in
#   endif
      }
      
   void suspend()
      {
      U_TRACE_NO_PARAM(0, "UThread::suspend()")

      U_ASSERT_EQUALS(isCurrentThread(tid), false)

#   ifdef HAVE_PTHREAD_SUSPEND
      (void) U_SYSCALL(pthread_suspend, "%p", tid);
#   else
      (void) U_SYSCALL(pthread_kill, "%p,%d", tid, U_SIGSTOP);

      yield(); // give the signal a time to kick in
#   endif
      }
#endif

   // Cancellation

   enum Cancel {
      cancelInitial,   /* used internally, do not use */
      cancelDeferred,  /* exit thread on cancellation pointsuch as yield */
      cancelImmediate, /* exit befor cancellation */
      cancelDisabled,  /* ignore cancellation */
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
      U_TRACE_NO_PARAM(1, "UThread::getThread()")

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
   pid_t id;
#ifdef _MSWINDOWS_
   DWORD tid;
   HANDLE cancellation;
#else
   pthread_t tid;
   int suspendCount;
#endif

   static UThread* first;

   void close();

   void threadStart()
      {
      U_TRACE_NO_PARAM(0, "UThread::threadStart()")

      U_INTERNAL_DUMP("tid = %p id = %u", tid, id)

      setCancel(cancelDeferred);

      run();

      U_INTERNAL_DUMP("tid = %p id = %u", tid, id)

      if (tid) close();
      }

#ifdef _MSWINDOWS_
   static unsigned __stdcall execHandler(void* th)
      {
      U_TRACE(0, "UThread::::execHandler(%p)", th)

      U_INTERNAL_ASSERT_POINTER(th)

      ((UThread*)th)->threadStart();

      U_RETURN(0);
      }
#else
   void maskSignal();
   void sigInstall(int signo);
   void manageSignal(int signo);

   static void sigHandler(int signo)
      {
      U_TRACE(0, "UThread::sigHandler(%d)", signo)

      UThread* th = getThread();

      if (th) th->manageSignal(signo);
      }

   static void execHandler(UThread* th)
      {
      U_TRACE(0, "UThread::execHandler(%p)", th)

      U_INTERNAL_ASSERT_POINTER(th)

      th->id = u_gettid();

      th->maskSignal();

      th->threadStart();
      }

   static void threadCleanup(UThread* th)
      {
      U_TRACE(0, "UThread::threadCleanup(%p)", th)

      U_INTERNAL_ASSERT_POINTER(th)

      U_INTERNAL_DUMP("th->tid = %p th->id = %u", th->tid, th->id)

      if (th->tid) th->close();
      }

   static void stop(pthread_t _tid, pthread_attr_t* pattr)
      {
      U_TRACE(1, "UThread::stop(%p,%p)", _tid, pattr)

      int state;

#   ifdef HAVE_PTHREAD_CANCEL
      (void) U_SYSCALL(pthread_cancel, "%p", _tid);
#   endif

      (void) U_SYSCALL(pthread_attr_getdetachstate, "%p,%p", pattr, &state);

      if (state != PTHREAD_CREATE_DETACHED) (void) U_SYSCALL(pthread_join, "%p,%p", _tid, 0);
#   ifdef HAVE_PTHREAD_YIELD
      else (void) U_SYSCALL_NO_PARAM(pthread_yield);
#   endif
      }
#endif

private:
   U_DISALLOW_ASSIGN(UThread)

   friend class UNotifier;
   friend class UThreadPool;
   friend class UServer_Base;
};

// UThreadPool class manages all the UThreadPool related activities. This includes keeping track of idle threads and snchronizations between all threads.
// Using UThreadPool is advantageous only when the work to be done is really time consuming. (at least 1 or 2 seconds)

class U_EXPORT UThreadPool : public UThread {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

    UThreadPool(uint32_t size);
   ~UThreadPool();

   // SERVICES

   void addTask(UThread* task)
      {
      U_TRACE(0, "UThreadPool::addTask(%p)", task)

      U_INTERNAL_ASSERT(active)

      lock(&tasks_mutex);

      queue.push(task);

      unlock(&tasks_mutex);

      signal(&condition); // Waking up the threads so they will know there is a job to do
      }

   // This function gives the user the ability to send 10 tasks to the thread pool then to wait till
   // all the tasks completed, and give the next 10 which are dependand on the result of the previous ones

   void waitForWorkToBeFinished()
      {
      U_TRACE_NO_PARAM(0, "UThreadPool::waitForWorkToBeFinished()")

      lock(&tasks_mutex);

      while (queue._length != 0) wait(&tasks_mutex, &condition_task_finished);

      unlock(&tasks_mutex);
      }

   // define method VIRTUAL of class UThread

   virtual void run() U_DECL_OVERRIDE;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UVector<UThread*> pool;  // Thread pool storage
   UVector<UThread*> queue; // Queue to keep track of incoming tasks
   bool active;

#ifdef _MSWINDOWS_
   CRITICAL_SECTION tasks_mutex; // Task queue mutex
   CONDITION_VARIABLE condition, condition_task_finished; // Condition variable
#else
   pthread_mutex_t tasks_mutex; // Task queue mutex
   pthread_cond_t condition, condition_task_finished; // Condition variable
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UThreadPool)
};
#endif
