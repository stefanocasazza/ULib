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

   static void encode(const char* s, uint32_t n, UString& buffer, bool json = false)
      {
      U_TRACE(0, "UEscape::encode(%.*S,%u,%p,%b)", n, s, n, &buffer, json)

      U_ASSERT(buffer.uniq())
      U_ASSERT(buffer.capacity() >= n)

      uint32_t sz = buffer.size();

      buffer.rep->_length = sz + u_escape_encode((const unsigned char*)s, n, buffer.c_pointer(sz), buffer.space(), json);

      U_INTERNAL_DUMP("buffer(%u) = %#V", buffer.size(), buffer.rep)
      }

   static void encode(const UString& s, UString& buffer, bool json = false) { encode(U_STRING_TO_PARAM(s), buffer, json); }

   static void decode(const char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UEscape::decode(%.*S,%u,%p)", n, s, n, &buffer)

      U_ASSERT(buffer.uniq())

      buffer.rep->_length = u_escape_decode(s, n, (unsigned char*)buffer.data());

      U_INTERNAL_DUMP("buffer(%u) = %#V", buffer.size(), buffer.rep)
      }

   static void decode(const UString& s, UString& buffer) { decode(U_STRING_TO_PARAM(s), buffer); }
};

#endif
