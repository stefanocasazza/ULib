/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    error.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_BASE_ERROR_H
#define ULIB_BASE_ERROR_H 1

#include <ulib/base/base.h>

#ifdef __cplusplus
extern "C" {
#endif
U_EXPORT void u_printError(void);
U_EXPORT void u_getSysError(                  uint32_t* restrict len); /* map  errno number     to an message string */
U_EXPORT void u_getSysSignal(int signo,       uint32_t* restrict len); /* map signal number     to an message string */
U_EXPORT void u_getExitStatus(int exit_value, uint32_t* restrict len); /* map exit status codes to an message string */
#ifdef __cplusplus
}
#endif

#endif
