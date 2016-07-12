/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    hexdump.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_CODER_HEXDUMP_H
#define ULIB_CODER_HEXDUMP_H 1

#include <ulib/base/base.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Encode-Decode hexdump into a buffer */

U_EXPORT uint32_t u_hexdump_encode(const unsigned char* restrict s, uint32_t n, unsigned char* restrict result);
U_EXPORT uint32_t u_hexdump_decode(const          char* restrict s, uint32_t n, unsigned char* restrict result);

#ifdef __cplusplus
}
#endif

#endif
