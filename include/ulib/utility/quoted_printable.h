// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    quoted_printable.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_QUOTED_PRINTABLE_H
#define ULIB_QUOTED_PRINTABLE_H 1

#include <ulib/base/coder/quoted_printable.h>

#include <ulib/string.h>

struct U_EXPORT UQuotedPrintable {

   static void encode(const char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UQuotedPrintable::encode(%.*S,%u,%.*S)", n, s, n, U_STRING_TO_TRACE(buffer))

      U_ASSERT(buffer.capacity() >= n)

      uint32_t pos = u_quoted_printable_encode((const unsigned char*)s, n, (unsigned char*)buffer.data());

      buffer.size_adjust(pos);
      }

   static void encode(const UString& s, UString& buffer) { encode(U_STRING_TO_PARAM(s), buffer); }

   static bool decode(const char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UQuotedPrintable::decode(%.*S,%u,%.*S)", n, s, n, U_STRING_TO_TRACE(buffer))

      U_ASSERT(buffer.capacity() >= n)

      uint32_t pos = u_quoted_printable_decode(s, n, (unsigned char*)buffer.data());

      buffer.size_adjust(pos);

      if (pos > 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool decode(const UString& s, UString& buffer) { return decode(U_STRING_TO_PARAM(s), buffer); }
};

#endif
