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

template <class T> class UPlugIn;

template <> class U_EXPORT UPlugIn<void*> {
public:
   static void* create(const char* name, uint32_t name_len);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UPlugIn<void*>)
};

template <class T> class U_EXPORT UPlugIn<T*> : public UPlugIn<void*> {
public:
   static T* create(const char* name, uint32_t name_len) { return (T*) UPlugIn<void*>::create(name, name_len); }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UPlugIn<T*>)
};

#endif
