/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    base64.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_CODER_BASE64_H
#define ULIB_CODER_BASE64_H 1

#include <ulib/base/base.h>

#ifdef __cplusplus
extern "C" {
#endif

#define U_OPENSSL_BASE64_MAX_COLUMN 64

extern U_EXPORT int u_base64_errors;
extern U_EXPORT int u_base64_max_columns;

/* Encode-Decode base64 into a buffer */

U_EXPORT uint32_t u_base64_encode(   const unsigned char* restrict s, uint32_t n, unsigned char* restrict result);
U_EXPORT uint32_t u_base64_decode(   const          char* restrict s, uint32_t n, unsigned char* restrict result);

U_EXPORT uint32_t u_base64url_encode(const unsigned char* restrict s, uint32_t n, unsigned char* restrict result);
U_EXPORT uint32_t u_base64url_decode(const          char* restrict s, uint32_t n, unsigned char* restrict result);
U_EXPORT uint32_t u_base64all_decode(const          char* restrict s, uint32_t n, unsigned char* restrict result);

#ifdef __cplusplus
}
#endif

#endif
