/* ============================================================================
//
// = LIBRARY
//    ULib - c library
//
// = FILENAME
//    trace.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#ifndef ULIB_BASE_TRACE_H
#define ULIB_BASE_TRACE_H 1

#include <ulib/base/utility.h>

/*
 * All programs need some form of logging built in to them, so we can observe what they're doing.
 * This is especially important when things go wrong
 */

#ifdef __cplusplus
extern "C" {
#endif

extern U_EXPORT int      u_trace_fd;
extern U_EXPORT int      u_trace_signal;
extern U_EXPORT int      u_trace_suspend;  /* on-off */
extern U_EXPORT uint32_t u_trace_num_tab;
extern U_EXPORT char     u_trace_tab[256]; /* 256 max indent */
extern U_EXPORT void*    u_trace_mask_level;

U_EXPORT void u_trace_lock(void);
U_EXPORT void u_trace_close(void);
U_EXPORT void u_trace_unlock(void);
U_EXPORT void u_trace_initFork(void);
U_EXPORT void u_trace_check_init(void);
U_EXPORT void u_trace_init(int bsignal);
U_EXPORT void u_trace_handlerSignal(void);
U_EXPORT void u_trace_check_if_interrupt(void); /* check for context manage signal event - interrupt */
U_EXPORT int  u_trace_check_if_active(int level);
U_EXPORT void u_trace_write(const char* restrict t, uint32_t tlen);
U_EXPORT void u_trace_writev(const struct iovec* restrict iov, int n);
U_EXPORT void u_trace_dump(const char* restrict format, uint32_t fmt_size, ...);

#ifdef __cplusplus
}
#endif

#endif
