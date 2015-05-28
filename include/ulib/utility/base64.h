// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    base64.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_BASE64_H
#define ULIB_BASE64_H 1

#include <ulib/base/coder/base64.h>

#include <ulib/string.h>

struct U_EXPORT UBase64 {

   static void encode(const char* s, uint32_t n, UString& buffer);
   static void encode(const UString& s,          UString& buffer) { encode(U_STRING_TO_PARAM(s), buffer); }

   static void decode(const char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UBase64::decode(%.*S,%u,%p)", n, s, n, &buffer)

      U_ASSERT(buffer.uniq())

      buffer.rep->_length = u_base64_decode(s, n, (unsigned char*)buffer.data());

      U_INTERNAL_DUMP("u_base64_errors = %u buffer(%u) = %#V", u_base64_errors, buffer.size(), buffer.rep)
      }

   static void decode(const UString& s, UString& buffer) { decode(U_STRING_TO_PARAM(s), buffer); }

   static void encodeUrl(const char* s, uint32_t n, UString& buffer);
   static void encodeUrl(const UString& s,          UString& buffer) { encodeUrl(U_STRING_TO_PARAM(s), buffer); }

   static void decodeUrl(const char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UBase64::decodeUrl(%.*S,%u,%p)", n, s, n, &buffer)

      U_ASSERT(buffer.uniq())

      buffer.rep->_length = u_base64url_decode(s, n, (unsigned char*)buffer.data());

      U_INTERNAL_DUMP("u_base64_errors = %u buffer(%u) = %#V", u_base64_errors, buffer.size(), buffer.rep)
      }

   static void decodeUrl(const UString& s, UString& buffer) { decodeUrl(U_STRING_TO_PARAM(s), buffer); }

   static void decodeAll(const char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UBase64::decodeAll(%.*S,%u,%p)", n, s, n, &buffer)

      U_ASSERT(buffer.uniq())

      buffer.rep->_length = u_base64all_decode(s, n, (unsigned char*)buffer.data());

      U_INTERNAL_DUMP("u_base64_errors = %u buffer(%u) = %#V", u_base64_errors, buffer.size(), buffer.rep)
      }

   static void decodeAll(const UString& s, UString& buffer) { decodeAll(U_STRING_TO_PARAM(s), buffer); }
};

#endif
