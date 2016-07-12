/* ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    des3.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_CODER_DES3_H
#define ULIB_CODER_DES3_H 1

#include <ulib/base/base.h>

#ifdef __cplusplus
extern "C" {
#endif

U_EXPORT void u_des_init(void);
U_EXPORT void u_des_reset(void);
U_EXPORT void u_des_key(const char* restrict str);
U_EXPORT long u_des_encode(const unsigned char* restrict inp, long len, unsigned char* restrict out);
U_EXPORT long u_des_decode(const unsigned char* restrict inp, long len, unsigned char* restrict out);

U_EXPORT void u_des3_init(void);
U_EXPORT void u_des3_reset(void);
U_EXPORT void u_des3_key(const char* restrict str);
U_EXPORT long u_des3_encode(const unsigned char* restrict inp, long len, unsigned char* restrict out);
U_EXPORT long u_des3_decode(const unsigned char* restrict inp, long len, unsigned char* restrict out);

#ifdef __cplusplus
}
#endif

#endif
