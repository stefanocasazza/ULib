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
   U_TRACE(0, "UBase64::encode(%.*S,%u,%.*S)", n, s, n, U_STRING_TO_TRACE(buffer))

#ifdef DEBUG
   uint32_t length = ((n + 2) / 3) * 4, num_lines = (u_base64_max_columns ? length / u_base64_max_columns + 1 : 0);

   U_INTERNAL_DUMP("buffer.capacity() = %u length = %u num_lines = %u", buffer.capacity(), length, num_lines)

   U_ASSERT(buffer.capacity() >= length + num_lines + 1)
#endif

   uint32_t pos = u_base64_encode((const unsigned char*)s, n, (unsigned char*)buffer.data());

   buffer.size_adjust(pos);
}

bool UBase64::decode(const char* s, uint32_t n, UString& buffer)
{
   U_TRACE(0, "UBase64::decode(%.*S,%u,%.*S)", n, s, n, U_STRING_TO_TRACE(buffer))

   uint32_t pos = u_base64_decode(s, n, (unsigned char*)buffer.data());

   buffer.size_adjust(pos);

   U_INTERNAL_DUMP("u_base64_errors = %u buffer(%u) = %#.*S", u_base64_errors, buffer.size(), U_STRING_TO_TRACE(buffer))

   if (pos > 0 &&
       u_base64_errors == 0)
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

void UBase64::encodeUrl(const char* s, uint32_t n, UString& buffer)
{
   U_TRACE(0, "UBase64::encodeUrl(%.*S,%u,%.*S)", n, s, n, U_STRING_TO_TRACE(buffer))

#ifdef DEBUG
   uint32_t length = ((n + 2) / 3) * 4, num_lines = (u_base64_max_columns ? length / u_base64_max_columns + 1 : 0);

   U_INTERNAL_DUMP("buffer.capacity() = %u length = %u num_lines = %u", buffer.capacity(), length, num_lines)

   U_ASSERT(buffer.capacity() >= length + num_lines + 1)
#endif

   uint32_t pos = u_base64url_encode((const unsigned char*)s, n, (unsigned char*)buffer.data());

   buffer.size_adjust(pos);
}

bool UBase64::decodeUrl(const char* s, uint32_t n, UString& buffer)
{
   U_TRACE(0, "UBase64::decodeUrl(%.*S,%u,%.*S)", n, s, n, U_STRING_TO_TRACE(buffer))

   uint32_t pos = u_base64url_decode(s, n, (unsigned char*)buffer.data());

   buffer.size_adjust(pos);

   U_INTERNAL_DUMP("u_base64_errors = %u buffer(%u) = %#.*S", u_base64_errors, buffer.size(), U_STRING_TO_TRACE(buffer))

   if (pos > 0 &&
       u_base64_errors == 0)
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UBase64::decodeAll(const char* s, uint32_t n, UString& buffer)
{
   U_TRACE(0, "UBase64::decodeAll(%.*S,%u,%.*S)", n, s, n, U_STRING_TO_TRACE(buffer))

   uint32_t pos = u_base64all_decode(s, n, (unsigned char*)buffer.data());

   buffer.size_adjust(pos);

   U_INTERNAL_DUMP("u_base64_errors = %u buffer(%u) = %#.*S", u_base64_errors, buffer.size(), U_STRING_TO_TRACE(buffer))

   if (pos > 0 &&
       u_base64_errors == 0)
      {
      U_RETURN(true);
      }

   U_INTERNAL_DUMP("buffer(%u) = %#.*S", buffer.size(), U_STRING_TO_TRACE(buffer))

   U_RETURN(false);
}
