// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    lock.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_LOCK_H
#define ULIB_LOCK_H 1

#include <ulib/utility/semaphore.h>

class U_EXPORT ULock {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   ULock()
      {
      U_TRACE_REGISTER_OBJECT(0, ULock, "")

      plock  = 0;
      psem   = 0;
      locked = 0;
      }

   ~ULock()
      {
      U_TRACE_UNREGISTER_OBJECT(0, ULock)

      destroy();
      }

   // SERVICES

   void   unlock();
   bool spinlock(uint32_t cnt);
   bool     lock(time_t timeout);

   void destroy();
   void init(sem_t* ptr_lock, char* ptr_spinlock = 0);

   void lock()
      {
      U_TRACE_NO_PARAM(0, "ULock::lock()")

      U_CHECK_MEMORY

      if (psem &&
          locked == 0)
         {
         psem->lock();

         locked = 1;
         }
      }

   // ATOMIC COUNTER

   static long atomicIncrement(long* pvalue, long offset)
      {
      U_TRACE(0, "ULock::atomicIncrement(%ld,%ld)", *pvalue, offset)

#  if defined(HAVE_GCC_ATOMICS) && defined(ENABLE_THREAD)
      return __sync_add_and_fetch(pvalue, offset);
#  else
      return (*pvalue += offset);
#  endif
      }

   static long atomicDecrement(long* pvalue, long offset)
      {
      U_TRACE(0, "ULock::atomicDecrement(%ld,%ld)", *pvalue, offset)

#  if defined(HAVE_GCC_ATOMICS) && defined(ENABLE_THREAD)
      return __sync_sub_and_fetch(pvalue, offset);
#  else
      return (*pvalue -= offset);
#  endif
      }

   // STREAM

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   char* plock;
   USemaphore* psem;
   int locked; // manage lock recursivity...

   static bool spinLockAcquire(char* ptr)
      {
      U_TRACE(0, "ULock::spinLockAcquire(%p)", ptr)

      U_INTERNAL_ASSERT_POINTER(ptr)

      // if not locked by another already, then we acquired it...

#  if defined(HAVE_GCC_ATOMICS) && defined(ENABLE_THREAD)
      if (__sync_lock_test_and_set(ptr, 1) == 0) U_RETURN(true);
#  else
      if (*ptr == 0)
         {
         *ptr = 1;

         U_RETURN(true);
         }
#  endif

      U_RETURN(false);
      }

   static void spinLockRelease(char* ptr)
      {
      U_TRACE(0, "ULock::spinLockRelease(%p)", ptr)

      U_INTERNAL_ASSERT_POINTER(ptr)

#  if defined(HAVE_GCC_ATOMICS) && defined(ENABLE_THREAD)
      /**
       * In theory __sync_lock_release should be used to release the lock.
       * Unfortunately, it does not work properly alone. The workaround is
       * that more conservative __sync_lock_test_and_set is used instead
       */

      (void) __sync_lock_test_and_set(ptr, 0);
#  else
      *ptr = 0;
#  endif
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(ULock)
};

#endif
