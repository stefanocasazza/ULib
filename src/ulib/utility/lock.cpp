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

void ULock::init(sem_t* ptr)
{
   U_TRACE(0, "ULock::init(%p)", ptr)

   U_CHECK_MEMORY

   if (ptr == U_NULLPTR)
      {
      if (reset()) sem = 0ULL;
      }
   else
      {
      USemaphore* psem;

      U_NEW(USemaphore, psem, USemaphore);

      sem = u_getValue(0, psem);

      getPointerToSemaphore()->init(ptr);
      }

   U_ASSERT_EQUALS(isLocked(), false)
}

bool ULock::lock(time_t timeout)
{
   U_TRACE(0, "ULock::lock(%ld)", timeout)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(sem)

   if (isLocked()) U_RETURN(true);

   if (getPointerToSemaphore()->wait(timeout))
      {
      setLocked();

      U_RETURN(true);  
      }

   U_RETURN(false);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* ULock::dump(bool reset) const
{
   *UObjectIO::os << "sem " << sem;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
