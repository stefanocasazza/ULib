// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    plugin.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_PLUGIN_H
#define ULIB_PLUGIN_H 1

#include <ulib/dynamic/dynamic.h>

// macros that help when defining the specific function required to support creation of plugin object

#define U_PLUGIN_TO_PARAM(mod) (mod).getName(),(mod).getNameLen()
#define U_PLUGIN_TO_TRACE(mod) (mod).getNameLen(),(mod).getName()

#define U_CREAT_FUNC(name, obj) extern "C" { void* u_creat_##name(); U_EXPORT void* u_creat_##name() { return new obj(); } }

enum EnvironmentType {
   U_CGI     = 0x001,
   U_PHP     = 0x002,
   U_RAKE    = 0x004,
   U_SHELL   = 0x008,
   U_WSCGI   = 0x010,
   U_GENERIC = 0x020
};

class UHTTP;

template <class T> class UPlugIn;

template <> class U_EXPORT UPlugIn<void*> : public UDynamic {
public:

   UPlugIn()
      {
      U_TRACE_NO_PARAM(0, "UPlugIn<void*>::UPlugIn()")

      obj      = 0;
      next     = 0;
      name     = 0;
      name_len = 0;
      }

   ~UPlugIn();

   // SERVICES

   const char* getName() const    { return name; }
   uint32_t    getNameLen() const { return name_len; }

   static bool empty()
      {
      U_TRACE_NO_PARAM(0, "UPlugIn<void*>::empty()")

      if (first == 0) U_RETURN(true);

      U_RETURN(false);
      }

   static UPlugIn<void*>* getObjWrapper(void* obj) __pure;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   void* obj;
   const char* name;
   UPlugIn<void*>* next;
   uint32_t name_len;

   static UPlugIn<void*>* first;

   static void  clear();
   static void* create(const char* name, uint32_t name_len);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UPlugIn<void*>)

   friend class UHTTP;
   friend class UServer_Base;
};

template <class T> class U_EXPORT UPlugIn<T*> : public UPlugIn<void*> {
public:

   UPlugIn()
      {
      U_TRACE_REGISTER_OBJECT(0, UPlugIn<T*>, "", 0)
      }

   ~UPlugIn()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UPlugIn<T*>)
      }

   // PlugIn operations

   static T* create(const char* name, uint32_t name_len) { return (T*) UPlugIn<void*>::create(name, name_len); }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return UPlugIn<void*>::dump(reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UPlugIn<T*>)
};

#endif
