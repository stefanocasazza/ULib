/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    url.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_CODER_URL_H
#define ULIB_CODER_URL_H 1

#include <ulib/base/base.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Encode-Decode url into a buffer */

U_EXPORT uint32_t u_url_encode(const unsigned char* restrict s, uint32_t n, unsigned char* restrict result);
U_EXPORT uint32_t u_url_decode(const          char* restrict s, uint32_t n, unsigned char* restrict result);

#ifdef __cplusplus
}
#endif

#endif
