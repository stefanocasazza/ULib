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

#include <ulib/string.h>
#include <ulib/dynamic/plugin.h>

UPlugIn<void*>* UPlugIn<void*>::first;

UPlugIn<void*>::~UPlugIn()
{
   U_TRACE_NO_PARAM(0, "UPlugIn<void*>::~UPlugIn()")

   if (name) U_SYSCALL_VOID(free, "%p", (void*)name);
   if (next) delete next;
}

void* UPlugIn<void*>::create(const char* _name, uint32_t _name_len)
{
   U_TRACE(0, "UPlugIn<void*>::create(%.*S,%u)", _name_len, _name, _name_len)

   UPlugIn<void*>* item;

   U_NEW(UPlugIn<void*>, item, UPlugIn<void*>);

   item->next = first;
   first      = item;

   item->name     = strndup(_name, _name_len);
   item->name_len =                _name_len;

   if (item->UDynamic::load(_name, _name_len))
      {
      char buffer[128];

      (void) u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("u_creat_%.*s"), _name_len, _name);

      pvPF creator = (pvPF) item->operator[](buffer);

      if (creator)
         {
         item->obj = creator();

         U_INTERNAL_DUMP("obj = %p", item->obj)
         }
      }

   U_RETURN(item->obj);
}

void UPlugIn<void*>::clear()
{
   U_TRACE_NO_PARAM(0, "UPlugIn<void*>::clear()")

   if (first)
      {
      delete first;
             first = 0;
      }
}

__pure UPlugIn<void*>* UPlugIn<void*>::getObjWrapper(void* _obj)
{
   U_TRACE(0, "UPlugIn<void*>::getObjWrapper(%p)", _obj)

   for (UPlugIn<void*>* item = first; item; item = item->next)
      {
      if (item->obj == _obj) U_RETURN_POINTER(item, UPlugIn<void*>);
      }

   U_RETURN_POINTER(0, UPlugIn<void*>);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UPlugIn<void*>::dump(bool reset) const
{
   *UObjectIO::os << "obj      " << obj          << '\n'
                  << "first    " << (void*)first << '\n'
                  << "next     " << (void*)next  << '\n'
                  << "name     ";

   char buffer[32];

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%#.*S"), name_len, name));

   *UObjectIO::os << '\n'
                  << "name_len " << name_len;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
