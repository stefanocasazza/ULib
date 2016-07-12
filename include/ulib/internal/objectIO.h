// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    objectIO.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_OBJECTIO_H
#define ULIB_OBJECTIO_H 1

#include <ulib/base/base.h>

// manage representation object => string

class UStringRep;

struct U_EXPORT UObjectIO {

   static ostrstream* os;
   static istrstream* is;

   static bool     busy_output;
   static char*    buffer_output;
   static uint32_t buffer_output_sz, buffer_output_len;

   static void output();
   static void  init(      char* t, uint32_t sz);
   static void input(const char* t, uint32_t tlen);

   static UStringRep* create(bool bcopy);
};

template <class T> inline void UString2Object(const char* t, uint32_t tlen, T& object)
{
   U_INTERNAL_TRACE("UString2Object(%.*s,%u,%p)", tlen, t, tlen, &object)

#ifdef U_STDCPP_ENABLE
   UObjectIO::input(t, tlen);

   U_INTERNAL_ASSERT_POINTER(UObjectIO::is)

   *UObjectIO::is >> object;

# ifdef HAVE_OLD_IOSTREAM
   delete UObjectIO::is;
# else
   UObjectIO::is->~istrstream();
# endif
#endif
}

template <class T> inline char* UObject2String(T& object)
{
   U_INTERNAL_TRACE("UObject2String(%p)", &object)

#ifndef U_STDCPP_ENABLE
   return 0;
#else
   U_INTERNAL_ASSERT_POINTER(UObjectIO::os)

   *UObjectIO::os << object;

   UObjectIO::output();

   return UObjectIO::buffer_output;
#endif
}

template <class T> inline UStringRep* UObject2StringRep(T& object, bool bcopy)
{
   U_INTERNAL_TRACE("UObject2StringRep(%p,%b)", &object, bcopy)

#ifndef U_STDCPP_ENABLE
   return 0;
#else
   U_INTERNAL_ASSERT_POINTER(UObjectIO::os)

   *UObjectIO::os << object;

   return UObjectIO::create(bcopy);
#endif
}

template <class T> inline uint32_t UObject2String(T& object, char* _buffer, uint32_t buffer_size)
{
   U_INTERNAL_TRACE("UObject2String(%p,%p,%u)", &object, _buffer, buffer_size)

#ifndef U_STDCPP_ENABLE
   return 0;
#else
   uint32_t len;

   ostrstream _os(_buffer, buffer_size);

   _os << object;

   len = _os.pcount();

   U_INTERNAL_PRINT("_os.pcount() = %u", len)

   U_INTERNAL_ASSERT_MINOR(len, buffer_size)

   return len;
#endif
}

#ifdef U_OBJECT_TO_TRACE
#undef U_OBJECT_TO_TRACE
#endif

#ifndef U_STDCPP_ENABLE
#  define U_OBJECT_TO_TRACE(obj) "not available"
#else
template <class T> inline char* U_OBJECT_TO_TRACE(T& object)
{
   U_INTERNAL_TRACE("U_OBJECT_TO_TRACE(%p)", &object)

#ifdef DEBUG
   int status = u_trace_suspend;
                u_trace_suspend = 1;
#endif

   char* str = UObject2String<T>(object);

   str = strndup(str, UObjectIO::buffer_output_len);

#ifdef DEBUG
   u_trace_suspend = status;
#endif

   return str;
}
#endif

#endif
