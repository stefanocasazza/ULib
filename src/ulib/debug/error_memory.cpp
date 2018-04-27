// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    error_memory.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/base/utility.h>
#include <ulib/internal/common.h>

const char* UMemoryError::getErrorType(const void* pobj) const
{
   if (invariant()) return "";

   // (ABW) Array Beyond Write | (FMR) Free Memory Read

   char buffer[4096];
   uint32_t n = snprintf(buffer, sizeof(buffer), "[pobj = %p _this = %p - %s]", pobj, _this, (_this ? "ABW" : "FMR"));

#ifdef U_STDCPP_ENABLE
   if (UObjectDB::fd > 0)
      {
      uint32_t l = UObjectDB::dumpObject(buffer+n+1, sizeof(buffer)-n-1, (void*)pobj);

      if (l)
         {
         buffer[n  +1] =
         buffer[n+l+1] = '\n';

         buffer[(n += l+2)] = '\0';
         }
      }
#endif

   char* pbuffer = (char*) malloc(n);

   U_INTERNAL_ASSERT_POINTER_MSG(pbuffer, "cannot allocate memory, exiting...")

   (void) memcpy(buffer, pbuffer, n+1);

   return pbuffer;
}
