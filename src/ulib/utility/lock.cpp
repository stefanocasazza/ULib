// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    lock.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/lock.h>

void ULock::init(sem_t* ptr_lock, char* ptr_spinlock)
{
   U_TRACE(0, "ULock::init(%p,%p)", ptr_lock, ptr_spinlock)

   U_CHECK_MEMORY

   if (ptr_lock)
      {
      sem = U_NEW(USemaphore);

      sem->init(ptr_lock);
      }

   if ((plock = ptr_spinlock)) *plock = 0;
}

void ULock::destroy()
{
   U_TRACE(0, "ULock::destroy()")

   U_CHECK_MEMORY

   unlock();

   if (sem)
      {
      delete sem;
             sem = 0;
      }
}

bool ULock::spinlock(uint32_t cnt)
{
   U_TRACE(0+256, "ULock::spinlock(%u)", cnt)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(plock)

   if (locked) U_RETURN(true);

   do {
      if (spinLockAcquire(plock))
         {
         locked = -1;

         U_RETURN(true);
         }
      }
   while (cnt--);

   U_RETURN(false);
}

bool ULock::lock(time_t timeout)
{
   U_TRACE(0, "ULock::lock(%ld)", timeout)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(sem)

   if (locked) U_RETURN(true);

   if (sem->wait(timeout))
      {
      locked = 1;

      U_RETURN(true);  
      }

   U_RETURN(false);
}

void ULock::unlock()
{
   U_TRACE(0, "ULock::unlock()")

   U_CHECK_MEMORY

   if (locked)
      {
      if (locked == -1)
         {
         U_INTERNAL_ASSERT_POINTER(plock)

         spinLockRelease(plock);
         }
      else
         {
         U_INTERNAL_ASSERT_POINTER(sem)
         U_INTERNAL_ASSERT_EQUALS(locked, 1)

         sem->unlock();
         }

      locked = 0;
      }
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* ULock::dump(bool reset) const
{
   *UObjectIO::os << "plock           " << (void*)plock << '\n'
                  << "locked          " << locked       << '\n'
                  << "sem (USemaphore " << (void*)sem   << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
