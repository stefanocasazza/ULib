/** ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    escape.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_CODER_ESCAPE_H
#define ULIB_CODER_ESCAPE_H 1

#include <ulib/base/base.h>

#ifdef __cplusplus
extern "C" {
#endif

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

U_EXPORT uint32_t      u_sprintc(char* restrict buffer, unsigned char c);
U_EXPORT unsigned char u_escape_decode_ptr(const char** restrict s);

U_EXPORT uint32_t u_escape_encode(const unsigned char* restrict s, uint32_t n,          char* restrict result, uint32_t max_length, bool json);
U_EXPORT uint32_t u_escape_decode(const          char* restrict s, uint32_t n, unsigned char* restrict result);

#ifdef __cplusplus
}
#endif

#endif
