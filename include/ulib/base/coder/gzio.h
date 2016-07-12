/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    gzio.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_GZIO_H
#define ULIB_GZIO_H 1

#include <ulib/base/base.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Synopsis: Compress and Decompresses the source buffer into the destination buffer
 *
 * #define GZIP_MAGIC "\037\213" Magic header for gzip files, 1F 8B
 */

U_EXPORT uint32_t u_gz_deflate(const char* restrict input, uint32_t len, char* restrict result, bool bheader);
U_EXPORT uint32_t u_gz_inflate(const char* restrict input, uint32_t len, char* restrict result);

#ifdef __cplusplus
}
#endif

#endif
