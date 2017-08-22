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

class ULog;
class URDB;

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

      sem = 0ULL;
      }

   ~ULock()
      {
      U_TRACE_UNREGISTER_OBJECT(0, ULock)

      (void) reset();
      }

   // SERVICES

   void lock()
      {
      U_TRACE_NO_PARAM(0, "ULock::lock()")

      U_CHECK_MEMORY

      if (isLocked() == false)
         {
         setLocked();

         getPointerToSemaphore()->lock();

         U_ASSERT(isLocked())
         }
      }

   void unlock()
      {
      U_TRACE_NO_PARAM(0, "ULock::unlock()")

      U_CHECK_MEMORY

      if (isLocked())
         {
         getPointerToSemaphore()->unlock();

         setUnLocked();

         U_ASSERT_EQUALS(isLocked(), false)
         }
      }

   void init(sem_t* ptr);
   bool lock(time_t timeout);

   // ATOMIC COUNTER

   static void atomicIncrement(long* pvalue, long offset)
      {
      U_TRACE(0, "ULock::atomicIncrement(%p,%ld)", pvalue, offset)

#  if defined(HAVE_GCC_ATOMICS) && defined(ENABLE_THREAD)
      (void) __sync_add_and_fetch(pvalue, offset);
#  else
      *pvalue += offset;
#  endif
      }

   static void atomicDecrement(long* pvalue, long offset)
      {
      U_TRACE(0, "ULock::atomicDecrement(%p,%ld)", pvalue, offset)

#  if defined(HAVE_GCC_ATOMICS) && defined(ENABLE_THREAD)
      (void) __sync_sub_and_fetch(pvalue, offset);
#  else
      *pvalue -= offset;
#  endif
      }

   static void atomicIncrement(sig_atomic_t& value) { atomicIncrement((long*)&value, 1L); }
   static void atomicDecrement(sig_atomic_t& value) { atomicDecrement((long*)&value, 1L); }

   // SPIN LOCK

   static bool spinlock(char* plock, uint32_t cnt)
      {
      U_TRACE(0+256, "ULock::spinlock(%p,%u)", plock, cnt)

      do {
         if (spinLockAcquire(plock)) U_RETURN(true);
         }
      while (cnt--);

      U_RETURN(false);
      }

   // STREAM

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   uint64_t sem;

   bool reset()
      {
      U_TRACE_NO_PARAM(0, "ULock::reset()")

      if (sem)
         {
         unlock();

         delete getPointerToSemaphore();

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   USemaphore* getPointerToSemaphore()
      {
      U_TRACE_NO_PARAM(0, "ULock::getPointerToSemaphore()")

      USemaphore* psem = (USemaphore*)u_getPayload(sem);

      U_INTERNAL_ASSERT_POINTER(psem)

      U_RETURN_POINTER(psem, USemaphore);
      }

   // manage lock recursivity...

   bool isLocked()
      {
      U_TRACE_NO_PARAM(0, "ULock::isLocked()")

      U_INTERNAL_DUMP("u_getTag(%#llx) = %u", sem, u_getTag(sem))

      if (u_getTag(sem) == U_TRUE_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   void setLocked()
      {
      U_TRACE_NO_PARAM(0, "ULock::setLocked()")

      u_setTag(U_TRUE_VALUE, &sem);
      }

   void setUnLocked()
      {
      U_TRACE_NO_PARAM(0, "ULock::setUnLocked()")

      u_setTag(U_FALSE_VALUE, &sem);
      }

   // SPIN LOCK

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

   friend class ULog;
   friend class URDB;
};

#endif
