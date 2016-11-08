// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    debug_common.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/debug/common.h>
#include <ulib/internal/error.h>

#ifdef USE_LIBSSL
#  include <openssl/err.h>
#endif

#ifdef _MSWINDOWS_
#  include <process.h>
#else
#  include <pwd.h>
#  include <sys/resource.h>
#endif

#ifdef HAVE_SYSEXITS_H
#  include <sysexits.h>
#else
#  include <ulib/base/replace/sysexits.h>
#endif

#ifdef DEBUG
#  ifndef PACKAGE_NAME
#  define PACKAGE_NAME ULib
#  endif
#  ifndef PACKAGE_STRING
#  define PACKAGE_STRING "ULib 1.4.2"
#  endif

extern "C" void U_EXPORT u_debug_at_exit(void)
{
   U_INTERNAL_TRACE("u_debug_at_exit()")

   // NB: if there are errors in this code maybe recursion occurs...

   if (u_recursion == false)
      {
      u_recursion = true;

      UError::stackDump();

      if (UError::callerDataDump) UError::callerDataDump();

#  ifdef USE_LIBSSL
      ERR_print_errors_fp(stderr);
#  endif

      char* cmd_on_exit = getenv("EXEC_ON_EXIT");

      if ( cmd_on_exit &&
          *cmd_on_exit)
         {
         char command[U_PATH_MAX];
         uint32_t len = u__snprintf(command, sizeof(command), U_CONSTANT_TO_PARAM("%s %s %P %N"), cmd_on_exit, u_progpath);

         U_INTERNAL_PRINT("command = %.*s", len, command)

         if (u_is_tty) U_MESSAGE("EXEC_ON_EXIT<on>: COMMAND=%.*S", len, command);

         (void) system(command);
         }

#  ifdef U_STDCPP_ENABLE
      UObjectDB::close();
#  endif

      u_trace_close();

      U_WRITE_MEM_POOL_INFO_TO("mempool.%N.%P", 0);

      U_INTERNAL_PRINT("u_flag_exit = %d", u_flag_exit)

      if (u_flag_exit == -2) abort(); // some assertion false - core dumped...
      }
}

static void print_info(void)
{
   // print program mode and info for ULib...

   U_MESSAGE("DEBUG MODE%W (pid %W%P%W) - " PACKAGE_STRING " " PLATFORM_VAR " (" __DATE__ ")%W", YELLOW, BRIGHTCYAN, YELLOW, RESET);

   char* memusage = getenv("UMEMUSAGE");

   U_MESSAGE("UMEMUSAGE%W<%W%s%W>%W", YELLOW, (memusage ? GREEN : RED), (memusage ? "on" : "off"), YELLOW, RESET);
}

extern "C" void U_EXPORT u_debug_init(void)
{
   U_INTERNAL_TRACE("u_debug_init()")

   print_info();

   USimulationError::init();

#ifdef U_STDCPP_ENABLE
   UObjectDB::init(true, true);
#endif

   // we go to check if there are previous creation of global
   // objects that can have forced the initialization of trace file...

   u_trace_check_init();
}

// set_memlimit() uses setrlimit() to restrict dynamic memory allocation.
// The argument to set_memlimit() is the limit in megabytes (a floating-point number)

void U_EXPORT u_debug_set_memlimit(float size)
{
   U_TRACE(1, "u_debug_set_memlimit(%f)", size)

   struct rlimit r;
   r.rlim_cur = (rlim_t)(size * 1048576);

   // Heap size, seems to be common
   (void) U_SYSCALL(setrlimit, "%d,%p", RLIMIT_DATA, &r);

   // Size of stack segment
   (void) U_SYSCALL(setrlimit, "%d,%p", RLIMIT_STACK, &r);

#ifdef RLIMIT_RSS
   // Resident set size. This affects swapping; processes that are exceeding their
   // resident set size will be more likely to have physical memory taken from them
   (void) U_SYSCALL(setrlimit, "%d,%p", RLIMIT_RSS, &r);
#endif

#ifdef RLIMIT_VMEM
   // Mapped memory (brk + mmap)
   (void) U_SYSCALL(setrlimit, "%d,%p", RLIMIT_VMEM, &r);
#endif

#ifdef RLIMIT_AS
   // Address space limit
   (void) U_SYSCALL(setrlimit, "%d,%p", RLIMIT_AS, &r);
#endif
}

pid_t U_EXPORT u_debug_fork(pid_t _pid, int trace_active)
{
   U_INTERNAL_TRACE("u_debug_fork(%d,%d)", _pid, trace_active)

   u_fork_called = true;

   if (_pid == 0) // child
      {
      u_setPid();

      print_info(); // print program mode and info for ULib...

      u_trace_initFork();

#  ifdef U_STDCPP_ENABLE
      if (UObjectDB::fd > 0) UObjectDB::initFork();
#  endif
      }

   if (trace_active)
      {
      char buffer[32];

      u_trace_write(buffer, sprintf(buffer, "::fork() = %d", _pid));
      }

   return _pid;
}

pid_t U_EXPORT u_debug_vfork(pid_t _pid, int trace_active)
{
   U_INTERNAL_TRACE("u_debug_vfork(%d,%d)", _pid, trace_active)

   // NB: same address space...  Parent process execution stopped until the child calls exec() or exit()

   if (trace_active)
      {
      static char buffer[32];

      if         (_pid == 0) u_trace_write(U_CONSTANT_TO_PARAM("..........CHILD..........")); // child
      else // if (_pid  > 0)
         {
         if (u_exec_failed == false)
            {
            struct iovec iov[1] = { { (caddr_t)"\n", 1 } };

            u_trace_writev(iov, 1);
            }

         u_trace_write(U_CONSTANT_TO_PARAM("..........PARENT.........")); // parent
         }

      u_trace_write(buffer, sprintf(buffer, "::vfork() = %d", _pid));
      }

   u_fork_called = false;

   return _pid;
}

void U_EXPORT u_debug_exit(int exit_value)
{
   U_INTERNAL_TRACE("u_debug_exit(%d)", exit_value)

   char buffer[32];

   u_trace_write(buffer, sprintf(buffer, "::exit(%d)", exit_value));
}

__noreturn void U_EXPORT u_debug_exec(const char* pathname, char* const argv[], char* const envp[], int trace_active)
{
   U_INTERNAL_TRACE("u_debug_exec(%s,%p,%p,%d)", pathname, argv, envp, trace_active)

   char buffer[1024];
   bool flag_trace_active = false;
   struct iovec iov[3] = { { (caddr_t)u_trace_tab, u_trace_num_tab },
                           { (caddr_t)buffer,                    0 },
                           { (caddr_t)"\n",                      1 } };

   iov[1].iov_len = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("::execve(%S,%p,%p)"), pathname, argv, envp);

   if (trace_active)
      {
      flag_trace_active = true;

      u_trace_writev(iov, 2);
      }

   if (u_fork_called)
      {
#  ifdef U_STDCPP_ENABLE
      if (UObjectDB::fd > 0) UObjectDB::close();
#  endif

      if (u_trace_fd > 0) u_trace_close();
      }

   u_exec_failed = false;

   (void) ::execve(pathname, argv, envp);

   u_exec_failed = true;

   if (flag_trace_active == false)
      {
      char buf[64];
      uint32_t bytes_written = u__snprintf(buf, sizeof(buf), U_CONSTANT_TO_PARAM("%W%N%W: %WWARNING: %W"),BRIGHTCYAN,RESET,YELLOW,RESET);

      (void) write(STDERR_FILENO, buf, bytes_written);
      (void) write(STDERR_FILENO, buffer, iov[1].iov_len);
      }

   iov[1].iov_len = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM(" = -1%R"), 0); // NB: the last argument (0) is necessary...

   if (flag_trace_active == false)
      {
      (void) write(STDERR_FILENO, buffer,          iov[1].iov_len);
      (void) write(STDERR_FILENO, iov[2].iov_base, iov[2].iov_len);
      }
   else
      {
      if (u_fork_called) u_trace_init(false);

      u_trace_writev(iov+1, 2);

      iov[1].iov_len = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("::_exit(%d)"), EX_UNAVAILABLE);

      u_trace_writev(iov, 3);

      if (u_fork_called) u_trace_close();
      }

   ::_exit(EX_UNAVAILABLE);
}
#endif
