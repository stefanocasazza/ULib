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

   sem = U_NEW(USemaphore);

   sem->init(ptr_lock);

   if ((spinlock = ptr_spinlock)) *spinlock = 0;
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

void ULock::lock(time_t timeout)
{
   U_TRACE(0, "ULock::lock(%ld)", timeout)

   U_CHECK_MEMORY

   if (locked == 0)
      {
      if (spinlock)
         {
      // uint32_t cnt = 100;

      // do {
            if (spinLockAcquire(spinlock))
               {
               locked = -1;

               return;
               }
      //    }
      // while (cnt--);
         }

      if (sem &&
          sem->wait(timeout))
         {
         locked = 1;
         }
      }
}

void ULock::unlock()
{
   U_TRACE(0, "ULock::unlock()")

   U_CHECK_MEMORY

   if (locked)
      {
      if (locked == -1)
         {
         U_INTERNAL_ASSERT_POINTER(spinlock)

         spinLockRelease(spinlock);
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
   *UObjectIO::os << "locked          " << locked          << '\n'
                  << "spinlock        " << (void*)spinlock << '\n'
                  << "sem (USemaphore " << (void*)sem      << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
