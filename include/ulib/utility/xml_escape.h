// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    xml_escape.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_XML_ESCAPE_H
#define ULIB_XML_ESCAPE_H 1

#include <ulib/base/coder/xml.h>

#include <ulib/string.h>

struct U_EXPORT UXMLEscape {

   static void encode(const char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UXMLEscape::encode(%.*S,%u,%p)", n, s, n, &buffer)

      U_ASSERT(buffer.capacity() >= n)

      buffer.rep->_length = u_xml_encode((const unsigned char*)s, n, (unsigned char*)buffer.data());

      U_INTERNAL_DUMP("buffer(%u) = %#V", buffer.size(), buffer.rep)

      U_INTERNAL_ASSERT(buffer.invariant())
      }

   static void encode(const UString& s, UString& buffer) { encode(U_STRING_TO_PARAM(s), buffer); }

   static void encode_add(const char* input, uint32_t len, UString& buffer)
      {
      U_TRACE(0, "UXMLEscape::encode_add(%.*S,%u,%p)", len, input, len, &buffer)

      U_ASSERT(buffer.space() >= len)

      buffer.rep->_length += u_xml_encode((const unsigned char*)input, len, (unsigned char*)buffer.pend());

      U_INTERNAL_DUMP("buffer(%u) = %#V", buffer.size(), buffer.rep)

      U_INTERNAL_ASSERT(buffer.invariant())
      }

   static void encode_add(const UString& input, UString& buffer) { encode_add(input.data(), input.size(), buffer); }

   static void decode(const char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UXMLEscape::decode(%.*S,%u,%p)", n, s, n, &buffer)

      U_ASSERT(buffer.capacity() >= n)

      buffer.rep->_length = u_xml_decode(s, n, (unsigned char*)buffer.data());

      U_INTERNAL_DUMP("buffer(%u) = %#V", buffer.size(), buffer.rep)

      U_INTERNAL_ASSERT(buffer.invariant())
      }

   static void decode(const UString& s, UString& buffer) { decode(U_STRING_TO_PARAM(s), buffer); }
};
#endif
