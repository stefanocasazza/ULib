// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    dynamic.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/dynamic/dynamic.h>

UString* UDynamic::plugin_dir;

HINSTANCE UDynamic::dload(const char* pathname)
{
   U_TRACE(0, "UDynamic::load(%S)", pathname)

#ifdef _MSWINDOWS_
   HINSTANCE handle = ::LoadLibrary(pathname);
#else
   /**
    * Perform lazy binding
    * --------------------------------------------------------------------
    * Only resolve symbols as the code that references them is executed.
    * If the symbol is never referenced, then it is never resolved.
    * Lazy binding is only performed for function references; references
    * to variables are always immediately bound when the library is loaded
    * --------------------------------------------------------------------
    */

   HINSTANCE handle = U_SYSCALL(dlopen, "%S,%d", pathname, RTLD_LAZY); // RTLD_NOW
#endif

   if (handle == U_NULLPTR)
      {
#  if defined(_MSWINDOWS_)
      const char* err = "load failed";
#  else
      const char* err = ::dlerror();
#  endif

      U_WARNING("UDynamic::load(%S) failed: %.*S", pathname, 256, err);

      return U_NULLPTR;
      }

#ifndef _MSWINDOWS_
   (void) ::dlerror(); /* Clear any existing error */
#endif

   return handle;
}

HINSTANCE UDynamic::dload(const char* name, uint32_t name_len)
{
   U_TRACE(0, "UDynamic::dload(%.*S,%u)", name_len, name, name_len)

   if (plugin_dir == U_NULLPTR) U_NEW_STRING(plugin_dir, UString(U_STRING_FROM_CONSTANT(U_LIBEXECDIR)))

   U_INTERNAL_ASSERT(plugin_dir->isNullTerminated())

   char buffer[U_PATH_MAX+1];

   (void) u__snprintf(buffer, U_PATH_MAX, U_CONSTANT_TO_PARAM(U_FMT_LIBPATH), U_PATH_CONV(plugin_dir->data()), name_len, name);

   return dload(buffer);
}

void UDynamic::dclose(HINSTANCE handle)
{
   U_TRACE(0, "UDynamic::dclose(%p)", handle)

   U_INTERNAL_ASSERT_POINTER(handle)

#ifdef _MSWINDOWS_
   ::FreeLibrary(handle);
#else
   (void) U_SYSCALL(dlclose, "%p", handle);
#endif
}

void* UDynamic::lookup(HINSTANCE handle, const char* sym)
{
   U_TRACE(0, "UDynamic::lookup(%p,%S)", handle, sym)

   U_INTERNAL_ASSERT_POINTER(handle)

#if defined(_MSWINDOWS_)
   void* addr = (void*) ::GetProcAddress(handle, sym);
#else
   void* addr = U_SYSCALL(dlsym, "%p,%S", handle, sym);
#endif

   if (addr == U_NULLPTR)
      {
#  if defined(_MSWINDOWS_)
      const char* err = "symbol missing";
#  else
      const char* err = ::dlerror();
#  endif

      U_WARNING("UDynamic::lookup(%p,%S) failed: %.*S", handle, sym, 256, err);

      return U_NULLPTR;
      }

   return addr;
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UDynamic::dump(bool reset) const
{
   *UObjectIO::os << "handle                      " << (void*)handle;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
