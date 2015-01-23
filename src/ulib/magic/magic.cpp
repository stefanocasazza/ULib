// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    magic.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/magic/magic.h>

magic_t UMagic::magic;

bool UMagic::init(int flags)
{
   U_TRACE(1, "UMagic::init(%d)", flags)

   U_INTERNAL_ASSERT_EQUALS(magic, 0)

   magic = (magic_t) U_SYSCALL(magic_open, "%d", flags);

   bool ok = (magic && U_SYSCALL(magic_load, "%p", magic, 0) != -1);

   U_DUMP("ok = %b status = %.*S", ok, 512, getError())

   U_RETURN(ok);
}

UString UMagic::getType(const char* buffer, uint32_t buffer_len)
{
   U_TRACE(1, "UMagic::getType(%.*S,%u)", buffer_len, buffer, buffer_len)

   U_INTERNAL_ASSERT_POINTER(magic)

   UString str;
   const char* result = (const char*) U_SYSCALL(magic_buffer, "%p,%p,%u", magic, buffer, buffer_len);

   if (result) (void) str.assign(result);
   else        (void) str.assign(U_CONSTANT_TO_PARAM("application/octet-stream"));

   U_RETURN_STRING(str);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UMagic::dump(bool reset) const
{
   *UObjectIO::os << "magic " << magic;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
