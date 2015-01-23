// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    escape.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_ESCAPE_H
#define ULIB_ESCAPE_H 1

#include <ulib/base/coder/escape.h>

#include <ulib/string.h>

/**
 * ---------------------------------------------------------------------------
 * Encode-Decode escape sequences into a buffer, the following are recognized:
 * ---------------------------------------------------------------------------
 * \0  NUL
 * \r  CR  carriage return    (\015 13  D)
 * \n  LF  newline            (\012 10  A)
 * \t  HT  horizontal tab     (\011  9  9)
 * \b  BS  backspace          (\010  8  8)
 * \f  FF  formfeed           (\014 12  C)
 * \v  VT  vertical tab       (\013 11  B)
 * \a  BEL                    (\007  7  7)
 * \e  ESC character          (\033 27 1B)
 *
 * \u    four-hex-digits (unicode char)
 * \^C   C = any letter (Control code)
 * \xDD  number formed of 1-2 hex   digits
 * \DDD  number formed of 1-3 octal digits
 * ---------------------------------------------------------------------------
 */

struct U_EXPORT UEscape {

   static void encode(const UString& s,          UString& buffer, bool json = false) { encode(U_STRING_TO_PARAM(s), buffer, json); }
   static void encode(const char* s, uint32_t n, UString& buffer, bool json = false);

   static bool decode(const UString& s,          UString& buffer) { return decode(U_STRING_TO_PARAM(s), buffer); }
   static bool decode(const char* s, uint32_t n, UString& buffer);
};

#endif
