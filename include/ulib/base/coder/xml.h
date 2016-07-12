/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    xml.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_XML_H
#define ULIB_XML_H 1

#include <ulib/base/base.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Encode-Decode xml into a buffer */

U_EXPORT uint32_t u_xml_encode(const unsigned char* restrict s, uint32_t n, unsigned char* restrict result);
U_EXPORT uint32_t u_xml_decode(const          char* restrict s, uint32_t n, unsigned char* restrict result);

#ifdef __cplusplus
}
#endif

#endif
