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
      U_TRACE(0, "UQuotedPrintable::encode(%.*S,%u,%p)", n, s, n, &buffer)

      U_ASSERT(buffer.uniq())
      U_ASSERT(buffer.capacity() >= n)

      buffer.rep->_length = u_quoted_printable_encode((const unsigned char*)s, n, (unsigned char*)buffer.data());

      U_INTERNAL_DUMP("buffer(%u) = %#V", buffer.size(), buffer.rep)
      }

   static void encode(const UString& s, UString& buffer) { encode(U_STRING_TO_PARAM(s), buffer); }

   static void decode(const char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UQuotedPrintable::decode(%.*S,%u,%p)", n, s, n, &buffer)

      U_ASSERT(buffer.uniq())
      U_ASSERT(buffer.capacity() >= n)

      buffer.rep->_length = u_quoted_printable_decode(s, n, (unsigned char*)buffer.data());

      U_INTERNAL_DUMP("buffer(%u) = %#V", buffer.size(), buffer.rep)
      }

   static void decode(const UString& s, UString& buffer) { decode(U_STRING_TO_PARAM(s), buffer); }
};

#endif
