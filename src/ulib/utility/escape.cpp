// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    escape.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/escape.h>

void UEscape::encode(const char* s, uint32_t n, UString& buffer, bool json)
{
   U_TRACE(0, "UEscape::encode(%.*S,%u,%.*S,%b)", n, s, n, U_STRING_TO_TRACE(buffer), json)

   U_ASSERT(buffer.capacity() >= n)

   uint32_t sz  = buffer.size(),
            pos = u_escape_encode((const unsigned char*)s, n, buffer.c_pointer(sz), buffer.space(), json);

   buffer.size_adjust(sz + pos);
}

bool UEscape::decode(const char* s, uint32_t n, UString& buffer)
{
   U_TRACE(0, "UEscape::decode(%.*S,%u,%.*S)", n, s, n, U_STRING_TO_TRACE(buffer))

   uint32_t pos = u_escape_decode(s, n, (unsigned char*)buffer.data());

   buffer.size_adjust(pos);

   if (pos > 0) U_RETURN(true);

   U_RETURN(false);
}
