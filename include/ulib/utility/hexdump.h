// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    hexdump.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_HEXDUMP_H
#define ULIB_HEXDUMP_H 1

#include <ulib/base/coder/hexdump.h>

#include <ulib/string.h>

struct U_EXPORT UHexDump {

   static void encode(const char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UHexDump::encode(%.*S,%u,%p)", n, s, n, &buffer)

      U_ASSERT(buffer.uniq())

      buffer.rep->_length = u_hexdump_encode((const unsigned char*)s, n, (unsigned char*)buffer.data());
      }

   static void encode(const UString& s, UString& buffer) { encode(U_STRING_TO_PARAM(s), buffer); }

   static void decode(const char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UHexDump::decode(%.*S,%u,%V)", n, s, n, buffer.rep)

      U_ASSERT(buffer.uniq())
      U_ASSERT(buffer.capacity() >= n)

      buffer.rep->_length = u_hexdump_decode(s, n, (unsigned char*)buffer.data());

      U_INTERNAL_DUMP("buffer(%u) = %#V", buffer.size(), buffer.rep)
      }

   static void decode(const UString& s, UString& buffer) { decode(U_STRING_TO_PARAM(s), buffer); }
};

#endif
