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

#include <ulib/string.h>
#include <ulib/dynamic/dynamic.h>

UString* UDynamic::plugin_dir;

#ifdef _MSWINDOWS_
#  define U_FMT_LIBPATH "%s/%.*s." U_LIB_SUFFIX
#else
#  define U_FMT_LIBPATH "%s/%.*s." U_LIB_SUFFIX
#endif

bool UDynamic::load(const char* pathname)
{
   U_TRACE(0, "UDynamic::load(%S)", pathname)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_EQUALS(handle, 0)

#ifdef _MSWINDOWS_
   handle = ::LoadLibrary(pathname);
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

   handle = U_SYSCALL(dlopen, "%S,%d", pathname, RTLD_LAZY); // RTLD_NOW
#endif

   if (handle == 0)
      {
#  if defined(_MSWINDOWS_)
      err = "load failed";
#  else
      err = ::dlerror();
#  endif

      U_WARNING("UDynamic::load(%S) failed: %.*S", pathname, 256, err);

      U_RETURN(false);
      }

#ifndef _MSWINDOWS_
   (void) ::dlerror(); /* Clear any existing error */
#endif

   U_RETURN(true);
}

void* UDynamic::operator[](const char* _sym)
{
   U_TRACE(0, "UDynamic::operator[](%S)", _sym)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(handle)

#if defined(_MSWINDOWS_)
   addr = (void*) ::GetProcAddress(handle, _sym);
#else
   addr = U_SYSCALL(dlsym, "%p,%S", handle, _sym);
#endif

   if (addr == 0)
      {
#  if defined(_MSWINDOWS_)
      err = "symbol missing";
#  else
      err = ::dlerror();
#  endif

      U_WARNING("UDynamic::operator[](%S) failed: %.*S", _sym, 256, err);
      }

   U_RETURN(addr);
}

void UDynamic::close()
{
   U_TRACE_NO_PARAM(0, "UDynamic::close()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(handle)

#if defined(_MSWINDOWS_)
   ::FreeLibrary(handle);
#else
   (void) U_SYSCALL(dlclose, "%p", handle);
#endif

   err    = 0;
   addr   = 0;
   handle = 0;
}

void UDynamic::setPluginDirectory(const UString& dir)
{
   U_TRACE(0, "UDynamic::setPluginDirectory(%V)", dir.rep)

   U_INTERNAL_ASSERT(dir.isNullTerminated())

   if (plugin_dir == 0) U_NEW(UString, plugin_dir, UString(dir));
   else
      {
      U_INTERNAL_DUMP("plugin_dir = %V", plugin_dir->rep)

      if (*plugin_dir != dir) *plugin_dir = dir;
      }
}

void UDynamic::clear()
{
   U_TRACE_NO_PARAM(0, "UDynamic::clear()")

   if (plugin_dir)
      {
      delete plugin_dir;
             plugin_dir = 0;
      }
}

bool UDynamic::load(const char* _name, uint32_t _name_len)
{
   U_TRACE(0, "UDynamic::load(%.*S,%u)", _name_len, _name, _name_len)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_EQUALS(handle, 0)

   if (plugin_dir == 0) U_NEW(UString, plugin_dir, UString(U_STRING_FROM_CONSTANT(U_LIBEXECDIR)));

   U_INTERNAL_ASSERT(plugin_dir->isNullTerminated())

   char buffer[U_PATH_MAX];

   (void) u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM(U_FMT_LIBPATH), U_PATH_CONV(plugin_dir->data()), _name_len, _name);

   bool result = load(buffer);

   U_RETURN(result);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UDynamic::dump(bool reset) const
{
   *UObjectIO::os << "err           " << err        << '\n'
                  << "addr          " << addr       << '\n'
                  << "handle        " << (void*)handle;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
