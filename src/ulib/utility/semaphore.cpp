// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    semaphore.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>
#include <ulib/timeval.h>
#include <ulib/utility/interrupt.h>
#include <ulib/utility/semaphore.h>

#if defined(U_LINUX) && !defined(__clang__)
U_DUMP_KERNEL_VERSION(LINUX_VERSION_CODE)
#endif

USemaphore* USemaphore::first;

/**
 * The initial value of the semaphore can be specified. An initial value is often used when used to lock a finite
 * resource or to specify the maximum number of thread instances that can access a specified resource.
 *
 * @param resource specify initial resource count or 1 default
 */

void USemaphore::init(sem_t* ptr, int resource)
{
   U_TRACE(1, "USemaphore::init(%p,%d)", ptr, resource)

   U_INTERNAL_ASSERT_EQUALS(psem, 0)

#if defined(__MACOSX__) || defined(__APPLE__)
   (void) u__snprintf(name, sizeof(name), U_CONSTANT_TO_PARAM("/sem%X"), ptr);

   psem = (sem_t*) U_SYSCALL(sem_open, "%S,%d,%d,%u", name, O_CREAT, 0644, 1);

   if (psem == SEM_FAILED)
      {
      U_ERROR_SYSCALL("USemaphore::init() failed");
      }
#elif defined(HAVE_SEM_INIT) && (!defined(U_LINUX) || LINUX_VERSION_CODE > KERNEL_VERSION(2,6,7))
   U_INTERNAL_ASSERT_POINTER(ptr)

   // initialize semaphore object sem to value, share it with other processes

   if (U_SYSCALL(sem_init, "%p,%d,%u", psem = ptr, 1, resource) == -1) // 1 -> semaphore is shared between processes
      {
      U_ERROR("USemaphore::init(%p,%u) failed", ptr, resource);
      }
#elif defined(_MSWINDOWS_)
   psem = (sem_t*) ::CreateSemaphore((LPSECURITY_ATTRIBUTES)NULL, (LONG)resource, 1000000, (LPCTSTR)NULL);
#else
   psem = UFile::mkTemp();

   if (psem == -1) U_ERROR("USemaphore::init(%p,%u) failed", ptr, resource);
#endif

#if !defined(__MACOSX__) && !defined(__APPLE__) && defined(HAVE_SEM_GETVALUE)
   next  = first;
           first = this;

# ifdef DEBUG
   U_INTERNAL_DUMP("first = %p next = %p", first, next)

   U_INTERNAL_ASSERT_DIFFERS(first, next)

   int _value = getValue();

   if (_value != resource) U_ERROR("USemaphore::init(%p,%u) failed - value = %d", ptr, resource, _value);
# endif
#endif
}

/**
 * Destroying a semaphore also removes any system resources associated with it. If a semaphore has threads currently
 * waiting on it, those threads will all continue when a semaphore is destroyed
 */

USemaphore::~USemaphore()
{
   U_TRACE_UNREGISTER_OBJECT(0, USemaphore)

   U_INTERNAL_ASSERT_POINTER(psem)

#if defined(__MACOSX__) || defined(__APPLE__)
   (void) U_SYSCALL(sem_close,  "%p", psem);
   (void) U_SYSCALL(sem_unlink, "%S", name);
#elif defined(HAVE_SEM_INIT) && (!defined(U_LINUX) || LINUX_VERSION_CODE > KERNEL_VERSION(2,6,7))
   (void) sem_destroy(psem); // Free resources associated with semaphore object sem
#elif defined(_MSWINDOWS_)
   (void) ::CloseHandle((HANDLE)psem);
#else
   UFile::close(psem);
#endif
}

/**
 * Posting to a semaphore increments its current value and releases the first thread waiting for the semaphore
 * if it is currently at 0. Interestingly, there is no support to increment a semaphore by any value greater than 1
 * to release multiple waiting threads in either pthread or the win32 API. Hence, if one wants to release
 * a semaphore to enable multiple threads to execute, one must perform multiple post operations
 */

void USemaphore::post()
{
   U_TRACE_NO_PARAM(1, "USemaphore::post()")

   U_INTERNAL_ASSERT_POINTER(psem)

   U_INTERNAL_DUMP("value = %d", getValue())

#if defined(__MACOSX__) || defined(__APPLE__)
   (void) U_SYSCALL(sem_post, "%p", psem); // unlock a semaphore
#elif defined(HAVE_SEM_INIT) && (!defined(U_LINUX) || LINUX_VERSION_CODE > KERNEL_VERSION(2,6,7))
   (void) U_SYSCALL(sem_post, "%p", psem); // unlock a semaphore
#elif defined(_MSWINDOWS_)
   ::ReleaseSemaphore((HANDLE)psem, 1, (LPLONG)NULL);
#else
   (void) UFile::unlock(psem);
#endif

   U_INTERNAL_DUMP("value = %d", getValue())
}

// NB: check if process has restarted and it had a lock armed (DEADLOCK)...

bool USemaphore::checkForDeadLock(UTimeVal& time)
{
   U_TRACE(1, "USemaphore::checkForDeadLock(%p)", &time)

#if !defined(__MACOSX__) && !defined(__APPLE__) && defined(HAVE_SEM_GETVALUE)
   bool sleeped = false;

   for (USemaphore* item = first; item; item = item->next)
      {
      if (item->getValue() <= 0)
         {
         sleeped = true;

         time.nanosleep();

         if (item->getValue() <= 0)
            {
            time.nanosleep();

            if (item->getValue() <= 0) item->post(); // unlock the semaphore
            }
         }
      }

   U_RETURN(sleeped);
#else
   U_RETURN(false);
#endif
}

/**
 * Wait is used to keep a thread held until the semaphore counter is greater than 0. If the current thread is held, then
 * another thread must increment the semaphore. Once the thread is accepted, the semaphore is automatically decremented,
 * and the thread continues execution.
 *
 * @return false if timed out
 * @param timeout period in milliseconds to wait
 */

bool USemaphore::wait(time_t timeoutMS)
{
   U_TRACE(1, "USemaphore::wait(%ld)", timeoutMS) // problem with sanitize address

   U_INTERNAL_ASSERT_POINTER(psem)
   U_INTERNAL_ASSERT_MAJOR(timeoutMS, 0)

   U_INTERNAL_DUMP("value = %d", getValue())

#if defined(__MACOSX__) || defined(__APPLE__) || \
   (defined(HAVE_SEM_INIT) && (!defined(U_LINUX) || LINUX_VERSION_CODE > KERNEL_VERSION(2,6,7)))

   // Wait for sem being posted

   U_INTERNAL_ASSERT(u_now->tv_sec > 1260183779) // 07/12/2009

   struct timespec abs_timeout = { u_now->tv_sec + timeoutMS / 1000L, 0 };

   U_INTERNAL_DUMP("abs_timeout = { %d, %d }", abs_timeout.tv_sec, abs_timeout.tv_nsec)

   int rc = U_SYSCALL(sem_timedwait, "%p,%p", psem, &abs_timeout);

   U_INTERNAL_DUMP("value = %d", getValue())

   if (rc == 0) U_RETURN(true);
#elif defined(_MSWINDOWS_)
   if (::WaitForSingleObject((HANDLE)psem, timeoutMS) == WAIT_OBJECT_0) U_RETURN(true);
#else
   if (UFile::lock(psem)) U_RETURN(true);
#endif

   U_RETURN(false);
}

void USemaphore::lock()
{
   U_TRACE_NO_PARAM(1, "USemaphore::lock()")

   U_INTERNAL_ASSERT_POINTER(psem)

   U_INTERNAL_DUMP("value = %d", getValue())

#if defined(__MACOSX__) || defined(__APPLE__)
   (void) U_SYSCALL(sem_wait, "%p", psem);
#elif defined(HAVE_SEM_INIT) && (!defined(U_LINUX) || LINUX_VERSION_CODE > KERNEL_VERSION(2,6,7))
   /**
    * sem_wait() decrements (locks) the semaphore pointed to by sem. If the semaphore's value is greater than zero,
    * then the decrement proceeds, and the function returns, immediately.  * If the semaphore currently has the value
    * zero, then the call blocks until either it becomes possible to perform the decrement (i.e., the semaphore value
    * rises above zero), or a * signal handler interrupts the call
    */

wait:
   int rc = U_SYSCALL(sem_wait, "%p", psem);

   if (rc == -1)
      {
      U_INTERNAL_DUMP("errno = %d", errno)

      if (errno == EINTR)
         {
         UInterrupt::checkForEventSignalPending();

         goto wait;
         }
      }

   U_INTERNAL_ASSERT_EQUALS(rc, 0)
#elif defined(_MSWINDOWS_)
   (void) ::WaitForSingleObject((HANDLE)psem, INFINITE);
#else
   (void) UFile::lock(psem);
#endif

   U_INTERNAL_DUMP("value = %d", getValue())
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* USemaphore::dump(bool reset) const
{
   *UObjectIO::os << "next " << (void*)next  << '\n'
                  << "psem " << (void*)psem;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
