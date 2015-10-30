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
#include <dl.h>
#endif

#ifndef _MSWINDOWS_
typedef void* HINSTANCE;
#endif

/**
 * @class UDynamic
 * @short  Dynamic class file loader
 *
 * This class is used to load object files. On elf based systems this is typically done with dlopen
 */

class UString;
class UOrmSession;
class UServer_Base;

class U_EXPORT UDynamic {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // COSTRUTTORI

   UDynamic()
      {
      U_TRACE_NO_PARAM(0, "UDynamic::UDynamic()")

      err    = "none";
      addr   = 0;
      handle = 0;
      }

   ~UDynamic()
      {
      U_TRACE_NO_PARAM(0, "UDynamic::~UDynamic()")
      }

   /**
    * Load a object file
    *
    * @param name of object to load
    */

   bool load(const char* pathname);
   bool load(const char* name, uint32_t name_len);

   /**
    * Detach a DSO object from running memory
    */

   void close();

   /**
    * Lookup a symbol in the loaded file
    */

   void* operator[](const char* sym);

   /**
    * Retrieve error indicator associated with DSO failure
    */

   const char* getError() const { return err; }

   static void* getAddressOfFunction(const char* name);

   static void setPluginDirectory(const UString& dir);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   void* addr;
   const char* err;
   HINSTANCE handle;

   static void clear();

   static UDynamic* pmain;
   static UString* plugin_dir;

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UDynamic(const UDynamic&) = delete;
   UDynamic& operator=(const UDynamic&) = delete;
#else
   UDynamic(const UDynamic&)            {}
   UDynamic& operator=(const UDynamic&) { return *this; }
#endif

   friend class UOrmSession;
   friend class UServer_Base;
};

#endif
