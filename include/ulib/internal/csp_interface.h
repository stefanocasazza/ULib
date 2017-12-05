/* csp_interface.h */

#ifndef HAVE__BOOL
#  define true  1
#  define false 0
typedef _Bool bool;
#endif

#ifndef _SYS_SYSMACROS_H_OUTER
#define _SYS_SYSMACROS_H_OUTER 1
#endif

#define U_CSP_INTERFACE
#include <ulib/base/base.h>
#include <ulib/internal/chttp.h>

#ifdef USE_LIBV8
char* runv8(const char* jssrc); /* compiles and executes javascript and returns the script return value as string */
#endif

char*        get_reply(void);
unsigned int get_reply_capacity(void);
void         set_reply_capacity(unsigned int n);
