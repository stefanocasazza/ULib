// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    common.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_DEBUG_COMMON_H
#define ULIB_DEBUG_COMMON_H 1

#include <ulib/internal/common.h>
#include <ulib/internal/error.h>

#ifdef DEBUG
U_EXPORT void  u_debug_exit(int exit_value);
U_EXPORT pid_t u_debug_fork( pid_t pid, int trace_active);
U_EXPORT pid_t u_debug_vfork(pid_t pid, int trace_active);
U_EXPORT void  u_debug_exec(const char* pathname, char* const argv[], char* const envp[], int trace_active) __noreturn;

// u_debug_set_memlimit() uses setrlimit() to restrict dynamic memory allocation.
// The argument to set_memlimit() is the limit in megabytes (a floating-point number)

U_EXPORT void u_debug_set_memlimit(float size);
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifdef DEBUG
U_EXPORT void u_debug_init(void);
#endif
U_EXPORT void u_debug_at_exit(void);
#ifdef __cplusplus
}
#endif

#endif
