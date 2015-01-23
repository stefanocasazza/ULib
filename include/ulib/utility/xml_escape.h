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
      U_TRACE(0, "UXMLEscape::encode(%.*S,%u,%.*S)", n, s, n, U_STRING_TO_TRACE(buffer))

      U_ASSERT(buffer.capacity() >= n)

      uint32_t pos = u_xml_encode((const unsigned char*)s, n, (unsigned char*)buffer.data());

      buffer.size_adjust(pos);
      }

   static void encode(const UString& s, UString& buffer) { encode(U_STRING_TO_PARAM(s), buffer); }

   static bool decode(const char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UXMLEscape::decode(%.*S,%u,%.*S)", n, s, n, U_STRING_TO_TRACE(buffer))

      U_ASSERT(buffer.capacity() >= n)

      uint32_t pos = u_xml_decode(s, n, (unsigned char*) buffer.data());

      buffer.size_adjust(pos);

      if (pos > 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool decode(const UString& s, UString& buffer) { return decode(U_STRING_TO_PARAM(s), buffer); }
};

#endif
