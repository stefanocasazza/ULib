// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    hexdump.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/hexdump.h>

void UHexDump::encode(const char* s, uint32_t n, UString& buffer)
{
   U_TRACE(0, "UHexDump::encode(%.*S,%u,%.*S)", n, s, n, U_STRING_TO_TRACE(buffer))

   uint32_t pos = u_hexdump_encode((const unsigned char*)s, n, (unsigned char*)buffer.data());

   buffer.size_adjust(pos);
}

void UHexDump::decode(const char* s, uint32_t n, UString& buffer)
{
   U_TRACE(0, "UHexDump::decode(%.*S,%u,%.*S)", n, s, n, U_STRING_TO_TRACE(buffer))

   U_ASSERT(buffer.capacity() >= n)

   uint32_t pos = u_hexdump_decode(s, n, (unsigned char*)buffer.data());

   buffer.size_adjust(pos);
}
