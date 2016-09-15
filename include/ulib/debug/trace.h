// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    trace.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_TRACE_H
#define ULIB_TRACE_H 1

#include <ulib/base/trace.h>

#include <ulib/debug/error_simulation.h>

#define U_MANAGE_RETURN_VALUE(type,format) \
   type trace_return_type(type ret) { \
   void* ptr_value = USimulationError::checkForMatch(buffer_trace); \
   if (ptr_value) \
      ret = *(type*)ptr_value; \
   trace_return(format,U_CONSTANT_SIZE(format),ret); \
   return ret; }

#define U_MANAGE_SYSRETURN_VALUE(type,format,error) \
   type trace_sysreturn_type(type ret) { \
   void* ptr_value = USimulationError::checkForMatch(buffer_syscall); \
   if (ptr_value) \
      ret = *(type*)ptr_value; \
   trace_sysreturn((error),format,U_CONSTANT_SIZE(format),ret); \
   return ret; }

#ifdef USE_LIBTDB
#  include <tdb.h>
typedef TDB_DATA tdbdata_t;
#endif

typedef                 DIR* pdir_t;
typedef                void* pvoid_t;
typedef                FILE* pfile_t;
typedef const          void* pcvoid_t;
typedef                char* pchar_t;
typedef const          char* pcchar_t;
typedef       unsigned char* puchar_t;
typedef const unsigned char* pcuchar_t;

// typedef int (*x11error_t)  (void*, void*);
// typedef int (*x11IOerror_t)(void*);

class UCrono;

class U_EXPORT UTrace {
public:

   // Initialization and termination methods

   UTrace(int level, uint32_t len, const char* name);
   UTrace(int level, const char* format, uint32_t fmt_size, ...);

   ~UTrace();

   char                     active[1];
   char flag_syscall_read_or_write[1];

   // trace return from generic call

   void trace_return(const char* format, uint32_t fmt_size, ...);

   // manage return from generic call for tipology of value (of return)...

   U_MANAGE_RETURN_VALUE(bool,               "%b")
   U_MANAGE_RETURN_VALUE(char,               "%C")
   U_MANAGE_RETURN_VALUE(int,                "%d")
   U_MANAGE_RETURN_VALUE(unsigned int,       "%u")
   U_MANAGE_RETURN_VALUE(long,               "%ld")
   U_MANAGE_RETURN_VALUE(unsigned long,      "%lu")
   U_MANAGE_RETURN_VALUE(long long,          "%lld")
   U_MANAGE_RETURN_VALUE(unsigned long long, "%llu")
   U_MANAGE_RETURN_VALUE(float,              "%f")
   U_MANAGE_RETURN_VALUE(double,             "%g")
   U_MANAGE_RETURN_VALUE(long double,        "%LG")
   U_MANAGE_RETURN_VALUE(void*,              "%p")
   U_MANAGE_RETURN_VALUE(const void*,        "%p")
   U_MANAGE_RETURN_VALUE(char*,              "%S")
   U_MANAGE_RETURN_VALUE(const char*,        "%S")
   U_MANAGE_RETURN_VALUE(void**,             "%p")
   U_MANAGE_RETURN_VALUE(char**,             "%p")
#ifdef USE_LIBTDB
   U_MANAGE_RETURN_VALUE(tdbdata_t,          "%J")
#endif

   // trace call and return from system call

   void trace_syscall(              const char* format, uint32_t fmt_size, ...);
   void trace_sysreturn(bool error, const char* format, uint32_t fmt_size, ...);

   // manage return from system call for tipology of value (of return)...

   U_MANAGE_SYSRETURN_VALUE(int,                "%d",   ret == -1)
   U_MANAGE_SYSRETURN_VALUE(unsigned int,       "%u",   ret == 0U)
   U_MANAGE_SYSRETURN_VALUE(long,               "%ld",  ret == -1L)
   U_MANAGE_SYSRETURN_VALUE(long long,          "%lld", ret == -1LL)
   U_MANAGE_SYSRETURN_VALUE(unsigned long,      "%lu",  ret == 0UL)
   U_MANAGE_SYSRETURN_VALUE(unsigned long long, "%llu", ret == 0ULL)
   U_MANAGE_SYSRETURN_VALUE(float,              "%f",   ret == 0.0)
   U_MANAGE_SYSRETURN_VALUE(double,             "%g",   ret == 0.0)
   U_MANAGE_SYSRETURN_VALUE(pvoid_t,            "%p",   ret == 0 ||
                                                        ret == (void*)-1)
   U_MANAGE_SYSRETURN_VALUE(pcvoid_t,           "%p",   ret == 0 ||
                                                        ret == (const void*)-1)
   U_MANAGE_SYSRETURN_VALUE(pchar_t,            "%S",   ret == 0)
   U_MANAGE_SYSRETURN_VALUE(pcchar_t,           "%S",   ret == 0)
   U_MANAGE_SYSRETURN_VALUE(puchar_t,           "%S",   ret == 0)
   U_MANAGE_SYSRETURN_VALUE(pcuchar_t,          "%S",   ret == 0)
   U_MANAGE_SYSRETURN_VALUE(pdir_t,             "%p",   ret == 0)
   U_MANAGE_SYSRETURN_VALUE(pfile_t,            "%p",   ret == 0)
   U_MANAGE_SYSRETURN_VALUE(sighandler_t,       "%p",   ret == (sighandler_t)SIG_ERR)
#ifdef USE_LIBTDB
   U_MANAGE_SYSRETURN_VALUE(tdbdata_t,          "%J",   false)
#endif

   void resume()  {                           u_trace_suspend = status; }
   void suspend() { status = u_trace_suspend; u_trace_suspend = 1; }

private:
   char buffer_trace[1017], buffer_syscall[1017];
   uint32_t buffer_trace_len, buffer_syscall_len;
   int status;

   static UCrono* time_syscall_read_or_write;

   void set(int level) U_NO_EXPORT;
};

#endif
