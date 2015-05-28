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

      U_ASSERT(buffer.uniq())
      U_ASSERT(buffer.capacity() >= n)

      buffer.rep->_length = u_xml_encode((const unsigned char*)s, n, (unsigned char*)buffer.data());
      }

   static void encode(const UString& s, UString& buffer) { encode(U_STRING_TO_PARAM(s), buffer); }

   static void decode(const char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UXMLEscape::decode(%.*S,%u,%p)", n, s, n, &buffer)

      U_ASSERT(buffer.uniq())
      U_ASSERT(buffer.capacity() >= n)

      buffer.rep->_length = u_xml_decode(s, n, (unsigned char*) buffer.data());

      U_INTERNAL_DUMP("buffer(%u) = %#V", buffer.size(), buffer.rep)
      }

   static void decode(const UString& s, UString& buffer) { decode(U_STRING_TO_PARAM(s), buffer); }
};

#endif
