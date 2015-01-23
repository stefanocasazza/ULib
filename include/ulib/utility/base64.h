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
};

#endif
