// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    trace.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================
 
/*
#define DEBUG_DEBUG
*/

#include <ulib/base/trace.h>
#include <ulib/base/utility.h>

#include <ulib/debug/trace.h>
#include <ulib/debug/crono.h>

UCrono* UTrace::time_syscall_read_or_write;

U_NO_EXPORT void UTrace::set(int level)
{
   U_INTERNAL_TRACE("UTrace::set(%d)", level)

   u_trace_check_if_interrupt(); // check for context manage signal event

   int skip = (u_trace_num_tab == 0);

   struct iovec iov[4] = { { (caddr_t)u_trace_tab, u_trace_num_tab++ },
                           { (caddr_t)U_CONSTANT_TO_PARAM("{Call   ") },
                           { (caddr_t)buffer_trace, buffer_trace_len },
                           { (caddr_t)"\n", 1 } };

   u_trace_writev(iov + skip, 4 - skip);

   if ((level & 0x00000100) != 0) u_trace_mask_level = this; // [0-9]+256...
}

UTrace::UTrace(int level, uint32_t len, const char* name)
{
   U_INTERNAL_TRACE("UTrace::UTrace(%d,%u,%s)", level, len, name)

   U_INTERNAL_ASSERT_EQUALS(sizeof(buffer_trace), 1017)

   buffer_trace_len   =
   buffer_syscall_len = 0;

   active[0] = u_trace_check_if_active(level);

   if (active[0])
      {
      u__memcpy(buffer_trace, name, buffer_trace_len = len, __PRETTY_FUNCTION__);

      set(level);
      }
}

UTrace::UTrace(int level, const char* format, uint32_t fmt_size, ...)
{
   U_INTERNAL_TRACE("UTrace::UTrace(%d,%.*s,%u)", level, fmt_size, format, fmt_size)

   U_INTERNAL_ASSERT_EQUALS(sizeof(buffer_trace), 1017)

   buffer_trace_len   =
   buffer_syscall_len = 0;

   active[0] = u_trace_check_if_active(level);

   if (active[0])
      {
      va_list argp;
      va_start(argp, fmt_size);

      buffer_trace_len = u__vsnprintf(buffer_trace, sizeof(buffer_trace), format, fmt_size, argp);

      va_end(argp);

      set(level);
      }
}

UTrace::~UTrace()
{
   U_INTERNAL_TRACE("UTrace::~UTrace()");

   if (active[0])
      {
      int skip = (u_trace_num_tab == 1);

      struct iovec iov[4] = { { (caddr_t)u_trace_tab, --u_trace_num_tab },
                              { (caddr_t)U_CONSTANT_TO_PARAM("}Return ") },
                              { (caddr_t)buffer_trace, buffer_trace_len },
                              { (caddr_t)"\n", 1 } };

      u_trace_writev(iov + skip, 4 - skip);

      if (u_trace_mask_level == this) u_trace_mask_level = 0;
      }

   if (u_trace_signal) u_trace_handlerSignal();
}

// ------------------------------------------------------------------------------------------------------------------------
// NB: what trace_return() do is to append to 'buffer_trace' of object the return value of method called.
//     After that 'buffer_trace' is printed in the destructor of object (see above)...
// ------------------------------------------------------------------------------------------------------------------------

void UTrace::trace_return(const char* format, uint32_t fmt_size, ...)
{
   U_INTERNAL_TRACE("UTrace::trace_return(%.*s,%u)", fmt_size, format, fmt_size)

   U_INTERNAL_ASSERT_EQUALS(sizeof(buffer_trace), 1017)

   if (active[0] &&
       (sizeof(buffer_trace) - buffer_trace_len) > 32)
      {
      char* ptr = buffer_trace + buffer_trace_len;

      u__memcpy(ptr, " = ", U_CONSTANT_SIZE(" = "), __PRETTY_FUNCTION__);
                ptr +=      U_CONSTANT_SIZE(" = ");

      buffer_trace_len += 3;

      va_list argp;
      va_start(argp, fmt_size);

      buffer_trace_len += u__vsnprintf(ptr, sizeof(buffer_trace) - buffer_trace_len, format, fmt_size, argp);

      va_end(argp);

      U_INTERNAL_PRINT("buffer_trace = %.*s", buffer_trace_len, buffer_trace)
      }
}
 
void UTrace::trace_syscall(const char* format, uint32_t fmt_size, ...)
{
   U_INTERNAL_TRACE("UTrace::trace_syscall(%.*s,%u)", fmt_size, format, fmt_size)

   va_list argp;
   va_start(argp, fmt_size);

   buffer_syscall_len = u__vsnprintf(buffer_syscall, sizeof(buffer_trace), format, fmt_size, argp);

   va_end(argp);

   U_INTERNAL_PRINT("buffer_syscall=%s", buffer_syscall)

   /*
   if (active[0])
      {
      flag_syscall_read_or_write[0] = (strncmp(format, U_CONSTANT_TO_PARAM("::read("))  == 0 ||
                                       strncmp(format, U_CONSTANT_TO_PARAM("::write(")) == 0 ||
                                       strncmp(format, U_CONSTANT_TO_PARAM("::send("))  == 0 ||
                                       strncmp(format, U_CONSTANT_TO_PARAM("::recv("))  == 0);

      if (flag_syscall_read_or_write[0])
         {
         if (time_syscall_read_or_write == 0) time_syscall_read_or_write = new UCrono;

         time_syscall_read_or_write->start();
         }
      }
   */

#ifdef _MSWINDOWS_
   SetLastError(0);
#endif
   errno = u_errno = 0;
}

void UTrace::trace_sysreturn(bool error, const char* format, uint32_t fmt_size, ...)
{
   U_INTERNAL_TRACE("UTrace::trace_sysreturn(%d,%.*s,%u)", error, fmt_size, format, fmt_size)

   U_INTERNAL_ASSERT_EQUALS(sizeof(buffer_syscall), 1017)

#ifdef _MSWINDOWS_
   if (format         &&
       active[0]      &&
       error == false &&
       format[1] == 'd') // int (BOOL for mingw)
      {
      va_list argp;
      va_start(argp, fmt_size);

      error = (va_arg(argp, int)                 == 0 &&
               strstr(buffer_syscall, "::fcntl") == 0);

      va_end(argp);
      }

   if (error      &&
       errno == 0 &&
       strstr(buffer_syscall, "::getenv") == 0)
      {
      errno = - GetLastError();

      if (errno == 0) error = false;
      }
#endif

   if (error ||
       active[0])
      {
      if (format) // check for system call with return void (Es: free())
         {
         char* ptr = buffer_syscall + buffer_syscall_len;

         u__memcpy(ptr, " = ", U_CONSTANT_SIZE(" = "), __PRETTY_FUNCTION__);
                   ptr +=      U_CONSTANT_SIZE(" = ");

         buffer_syscall_len += 3;

         U_INTERNAL_ASSERT_MAJOR(sizeof(buffer_syscall) - buffer_syscall_len, 16)

         va_list argp;
         va_start(argp, fmt_size);

         buffer_syscall_len += u__vsnprintf(ptr, sizeof(buffer_syscall) - buffer_syscall_len, format, fmt_size, argp);

         va_end(argp);

         U_INTERNAL_PRINT("buffer_syscall = %.*s", buffer_syscall_len, buffer_syscall)

         if (error)
            {
            if (errno)
               {
               char msg_sys_error[sizeof(buffer_syscall)];

               buffer_syscall_len += u__snprintf(msg_sys_error, sizeof(buffer_syscall), U_CONSTANT_TO_PARAM("%R"), 0); // NB: the last argument (0) is necessary...

               U_INTERNAL_ASSERT_MINOR(buffer_syscall_len, sizeof(buffer_syscall))

               (void) strcat(buffer_syscall, msg_sys_error);

               u_errno = errno;
               }

            if (errno != EAGAIN                            &&
                errno != EINPROGRESS                       &&
#           ifdef USE_LIBPCRE
                strstr(buffer_syscall, "::pcre_exec") == 0 &&
#           endif
                strstr(buffer_syscall, "::getenv")    == 0)
               {
               U_WARNING("%s", buffer_syscall);
               }
            }
         }

      if (active[0])
         {
         /*
         if (error == false &&
             flag_syscall_read_or_write[0])
            {
            va_list argp;
            va_start(argp, fmt_size);

            ssize_t bytes_read_or_write = va_arg(argp, ssize_t);

            va_end(argp);

            if (bytes_read_or_write > (ssize_t)(10 * 1024))
               {
                             time_syscall_read_or_write->stop();
               long dltime = time_syscall_read_or_write->getTimeElapsed();

               if (dltime > 0)
                  {
                  int units;
                  char msg[sizeof(buffer_syscall)];
                  double rate = u_calcRate(bytes_read_or_write, dltime, &units);

                  buffer_syscall_len += u__snprintf(msg, sizeof(buffer_syscall), U_CONSTANT_TO_PARAM(" (%.2f %s/s)"), rate, u_short_units[units]);

                  U_INTERNAL_ASSERT_MINOR(buffer_syscall_len, sizeof(buffer_syscall))

                  (void) strcat(buffer_syscall, msg);
                  }
               }
            }
         */

         struct iovec iov[3] = { { (caddr_t)u_trace_tab,    u_trace_num_tab },
                                 { (caddr_t)buffer_syscall, buffer_syscall_len },
                                 { (caddr_t)"\n", 1 } };

         u_trace_writev(iov, 3);
         }
      }
}
