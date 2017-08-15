// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    plugin.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/dynamic/plugin.h>

void* UPlugIn<void*>::create(const char* name, uint32_t name_len)
{
   U_TRACE(0, "UPlugIn<void*>::create(%.*S,%u)", name_len, name, name_len)

   HINSTANCE handle = UDynamic::dload(name, name_len);

   if (handle)
      {
      char sym[128];

      (void) u__snprintf(sym, sizeof(sym), U_CONSTANT_TO_PARAM("u_creat_%.*s"), name_len, name);

      pvPF creator = (pvPF) UDynamic::lookup(handle, sym);

      if (creator)
         {
         void* obj = creator();

         U_INTERNAL_DUMP("obj = %p", obj)

         return obj;
         }
      }

   return U_NULLPTR;
}
