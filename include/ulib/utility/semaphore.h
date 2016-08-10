// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    semaphore.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_SEMAPHORE_H
#define ULIB_SEMAPHORE_H 1

#include <ulib/internal/common.h>

#ifdef HAVE_SEMAPHORE_H
#  include <semaphore.h>
#elif !defined(HAVE_SEM_INIT)
typedef int sem_t;
#  ifdef HAVE_OLD_IOSTREAM
extern "C" { int sem_getvalue(sem_t* sem, int* sval); }
#  endif
#endif

class UTimeVal;
class UServer_Base;

/**
 * A data race occurs when two threads access the same variable concurrently and at least one of the accesses is write
 *
 * A semaphore is generally used as a synchronization object between multiple threads or to protect a limited and finite
 * resource such as a memory or thread pool. The semaphore has a counter which only permits access by one or more threads
 * when the value of the semaphore is non-zero. Each access reduces the current value of the semaphore by 1. One or more
 * threads can wait on a semaphore until it is no longer 0, and hence the semaphore can be used as a simple thread
 * synchronization object to enable one thread to pause others until the thread is ready or has provided data for them.
 * Semaphores are typically used as a counter for protecting or limiting concurrent access to a given resource, such as
 * to permitting at most "x" number of threads to use resource "y", for example
 */

class U_EXPORT USemaphore {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   USemaphore()
      {
      U_TRACE_REGISTER_OBJECT(0, USemaphore, "", 0)

      next = 0;
      psem = 0;
      }

   /**
    * The initial value of the semaphore can be specified. An initial value is often used when used to lock a finite
    * resource or to specify the maximum number of thread instances that can access a specified resource.
    *
    * @param resource specify initial resource count or 1 default
    */

   void init(sem_t* ptr = 0, int resource = 1);

   /**
    * Destroying a semaphore also removes any system resources associated with it. If a semaphore has threads currently
    * waiting on it, those threads will all continue when a semaphore is destroyed
    */

   ~USemaphore();

   /**
    * Wait is used to keep a thread held until the semaphore counter is greater than 0. If the current thread is held, then
    * another thread must increment the semaphore. Once the thread is accepted, the semaphore is automatically decremented,
    * and the thread continues execution.
    *
    * @return false if timed out
    * @param  timeout period in milliseconds to wait
    */

   bool wait(time_t timeoutMS);

   /**
    * Posting to a semaphore increments its current value and releases the first thread waiting for the semaphore
    * if it is currently at 0. Interestingly, there is no support to increment a semaphore by any value greater than 1
    * to release multiple waiting threads in either pthread. Hence, if one wants to release a semaphore to enable multiple
    * threads to execute, one must perform multiple post operations
    */

   void   lock();
   void unlock() { post(); }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool) const;
#endif

protected:
   USemaphore* next;
#if defined(__MACOSX__) || defined(__APPLE__)
   sem_t* psem;
   char name[24];
#elif defined(_MSWINDOWS_) || (defined(HAVE_SEM_INIT) && (!defined(U_LINUX) || LINUX_VERSION_CODE > KERNEL_VERSION(2,6,7)))
   sem_t* psem;
#else
   int psem;
#endif

   static USemaphore* first;

   void post();

   static bool checkForDeadLock(UTimeVal& time); // NB: check if process has restarted and it had a lock active...

#if !defined(__MACOSX__) && !defined(__APPLE__) && defined(HAVE_SEM_GETVALUE) && (!defined(U_LINUX) || LINUX_VERSION_CODE > KERNEL_VERSION(2,6,7))
   int getValue() { int value = -1; (void) sem_getvalue(psem, &value); return value; }
#else
   int getValue() { return -1; }
#endif
   
private:
   U_DISALLOW_COPY_AND_ASSIGN(USemaphore)

   friend class UServer_Base;
};

#endif
