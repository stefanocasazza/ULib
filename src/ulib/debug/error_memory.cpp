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

   uint32_t n;
   char buffer[4096];

   (void) snprintf(buffer, sizeof(buffer), "[pobj = %p _this = %p - %s]", pobj, _this, (_this ? "ABW" : "FMR"));

   n = strlen(buffer)+1; // null-terminator

#ifdef U_STDCPP_ENABLE
   if (UObjectDB::fd > 0)
      {
      uint32_t l = UObjectDB::dumpObject(&buffer[n], sizeof(buffer)-n, (void*)pobj);

      if (l)
         {
         buffer[n-1] = 
         buffer[n+l] = '\n';

         n += l+1;

         buffer[n] = '\0';
         }
      }
#endif

#ifdef USE_LIBMIMALLOC
   char* pbuffer = (char*) mi_malloc(n);
#else
   char* pbuffer = (char*) malloc(n);
#endif

   U_INTERNAL_ASSERT_POINTER_MSG(pbuffer, "cannot allocate memory, exiting...")

   (void) memcpy(pbuffer, buffer, n);

   U_INTERNAL_ASSERT_EQUALS(strlen(pbuffer), n-1)

   return pbuffer;
}
