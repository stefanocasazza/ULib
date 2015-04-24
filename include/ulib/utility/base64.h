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

   static bool decode(const char* s, uint32_t n, UString& buffer);
   static bool decode(const UString& s,          UString& buffer) { return decode(U_STRING_TO_PARAM(s), buffer); }

   static void encodeUrl(const char* s, uint32_t n, UString& buffer);
   static void encodeUrl(const UString& s,          UString& buffer) { encodeUrl(U_STRING_TO_PARAM(s), buffer); }

   static bool decodeUrl(const char* s, uint32_t n, UString& buffer);
   static bool decodeUrl(const UString& s,          UString& buffer) { return decodeUrl(U_STRING_TO_PARAM(s), buffer); }

   static bool decodeAll(const char* s, uint32_t n, UString& buffer);
   static bool decodeAll(const UString& s,          UString& buffer) { return decodeAll(U_STRING_TO_PARAM(s), buffer); }
};

#endif
