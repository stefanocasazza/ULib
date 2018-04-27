// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    dynamic.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_DYNAMIC_H
#define ULIB_DYNAMIC_H 1

#include <ulib/string.h>

#if defined(HAVE_DLFCN_H)
extern "C" {
#  include <dlfcn.h>
}
#  ifndef RTLD_GLOBAL
#  define RTLD_GLOBAL 0
#  endif
#endif

#ifdef HAVE_SHL_LOAD
#  include <dl.h>
#endif

#ifdef _MSWINDOWS_
#  define U_FMT_LIBPATH "%s/%.*s." U_LIB_SUFFIX
#else
typedef void* HINSTANCE;
#  define U_FMT_LIBPATH "%s/%.*s." U_LIB_SUFFIX
#endif

enum DynamicPageType {
   U_DPAGE_INIT    = -1,
   U_DPAGE_RESET   = -2,
   U_DPAGE_DESTROY = -3,
   U_DPAGE_SIGHUP  = -4,
   U_DPAGE_FORK    = -5
};

/**
 * @class UDynamic
 * @short  Dynamic class file loader
 *
 * This class is used to load object files. On elf based systems this is typically done with dlopen
 */

class UOrmSession;
class UServer_Base;

template <class T> class UPlugIn;

class U_EXPORT UDynamic {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UDynamic()
      {
      U_TRACE_NO_PARAM(0, "UDynamic::UDynamic()")

      handle = U_NULLPTR;
      }

   ~UDynamic()
      {
      U_TRACE_NO_PARAM(0, "UDynamic::~UDynamic()")

      if (handle) close();
      }

   // SERVICE

   static void setPluginDirectory(const UString& dir)
      {
      U_TRACE(0, "UDynamic::setPluginDirectory(%V)", dir.rep)

      U_INTERNAL_ASSERT(dir.isNullTerminated())

      if (plugin_dir == U_NULLPTR) U_NEW_STRING(plugin_dir, UString(dir))
      else
         {
         U_INTERNAL_DUMP("plugin_dir = %V", plugin_dir->rep)

         if (*plugin_dir != dir) *plugin_dir = dir;
         }
      }

   /**
    * Load a object file
    *
    * @param name of object to load
    */

   bool load(const char* pathname)
      {
      U_TRACE(0, "UDynamic::load(%S)", pathname)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_EQUALS(handle, U_NULLPTR)

      handle = dload(pathname);

      if (handle) U_RETURN(true);

      U_RETURN(false);
      }

   bool load(const char* name, uint32_t name_len)
      {
      U_TRACE(0, "UDynamic::load(%.*S,%u)", name_len, name, name_len)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_EQUALS(handle, U_NULLPTR)

      handle = dload(name, name_len);

      if (handle) U_RETURN(true);

      U_RETURN(false);
      }

   /**
    * Detach a DSO object from running memory
    */

   void close()
      {
      U_TRACE_NO_PARAM(0, "UDynamic::close()")

      U_CHECK_MEMORY

      dclose(handle);
             handle = U_NULLPTR;
      }

   /**
    * Lookup a symbol in the loaded file
    */

   void* operator[](const char* sym)
      {
      U_TRACE(0, "UDynamic::operator[](%S)", sym)

      U_CHECK_MEMORY

      return lookup(handle, sym);
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   HINSTANCE handle;

   static void clear()
      {
      U_TRACE_NO_PARAM(0, "UDynamic::clear()")

      if (plugin_dir)
         {
         U_DELETE(plugin_dir)

         plugin_dir = U_NULLPTR;
         }
      }

   static UString* plugin_dir;

   static void dclose(HINSTANCE handle);
   static HINSTANCE dload(const char* pathname);
   static void* lookup(HINSTANCE handle, const char* sym);
   static HINSTANCE dload(const char* name, uint32_t name_len);

private:
   U_DISALLOW_ASSIGN(UDynamic)

   friend class UOrmSession;
   friend class UServer_Base;

   template <class T> friend class UPlugIn;
};

#endif
