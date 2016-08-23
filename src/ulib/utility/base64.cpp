// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    base64.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/base64.h>

void UBase64::encode(const char* s, uint32_t n, UString& buffer)
{
   U_TRACE(0, "UBase64::encode(%.*S,%u,%V)", n, s, n, buffer.rep)

#ifdef DEBUG
   uint32_t length = ((n + 2) / 3) * 4, num_lines = (u_base64_max_columns ? length / u_base64_max_columns + 1 : 0);

   U_INTERNAL_DUMP("buffer.capacity() = %u length = %u num_lines = %u", buffer.capacity(), length, num_lines)

   U_ASSERT(buffer.uniq())
   U_ASSERT(buffer.capacity() >= length + num_lines + 1)
#endif

   buffer.rep->_length = u_base64_encode((const unsigned char*)s, n, (unsigned char*)buffer.data());
}

void UBase64::encodeUrl(const char* s, uint32_t n, UString& buffer)
{
   U_TRACE(0, "UBase64::encodeUrl(%.*S,%u,%V)", n, s, n, buffer.rep)

#ifdef DEBUG
   uint32_t length = ((n + 2) / 3) * 4;

   U_INTERNAL_DUMP("buffer.capacity() = %u length = %u", buffer.capacity(), length)

   U_ASSERT(buffer.uniq())
   U_ASSERT(buffer.capacity() >= length + 1)
#endif

   buffer.rep->_length = u_base64url_encode((const unsigned char*)s, n, (unsigned char*)buffer.data());
}
