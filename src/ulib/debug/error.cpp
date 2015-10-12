// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    error.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/interrupt.h>
#include <ulib/debug/error.h>

#ifndef _MSWINDOWS_
#  include <sys/wait.h>
#endif

vPF UError::callerDataDump;

#ifdef HAVE_EXECINFO_H
#  include <execinfo.h>
#  ifndef __GXX_ABI_VERSION
#  define __GXX_ABI_VERSION 100
#  endif
#  include <cxxabi.h>
#  ifdef HAVE_DLFCN_H
#  include <dlfcn.h>
#   if defined(LINUX) || defined(__LINUX__) || defined(__linux__)
static uint32_t execute_addr2line(char* buffer, uint32_t buffer_size, const char* image, void* addr)
{
   ssize_t len;
   int pipefd[2];
   uint32_t output_len;

   (void) pipe(pipefd);

   pid_t pid = fork();

   if (pid == 0)
      {
      char buf[32];
      int fd_stderr = open("/tmp/addr2line.err", O_CREAT | O_WRONLY, 0666);

      (void) close(pipefd[0]);

#    ifndef HAVE_DUP3
      (void) dup2(pipefd[1], STDOUT_FILENO);
      (void) dup2(fd_stderr, STDERR_FILENO);
#    else
      (void) dup3(pipefd[1], STDOUT_FILENO, O_CLOEXEC);
      (void) dup3(fd_stderr, STDERR_FILENO, O_CLOEXEC);
#    endif

      // Invokes addr2line utility to determine the function name
      // and the line information from an address in the code segment

      (void) snprintf(buf, sizeof(buf), "%p", addr);

      (void) execlp("addr2line", "addr2line", buf, "-f", "-C", "-e", image, (char*)0);

      abort();
      }

   (void) close(pipefd[1]);

   output_len = 0;

loop:
   len = read(pipefd[0], buffer + output_len, buffer_size - output_len);

   if (len > 0)
      {
      output_len += len;

      goto loop;
      }

   buffer[output_len] = 0;

   (void) close(pipefd[0]);

   (void) waitpid(pid, 0, 0);

   return output_len;
}
#   endif
#  endif
#endif

void UError::stackDump()
{
   U_INTERNAL_TRACE("UError::stackDump()")

   char name[128];

   (void) u__snprintf(name, sizeof(name), "stack.%N.%P", 0);

   int fd = open(name, O_CREAT | O_WRONLY | O_APPEND | O_BINARY, 0666);

   if (fd == -1) return;

   // readlink() places the contents of the symbolic link path in the buffer buf, which has size bufsiz.
   // does not append a null byte to buf. It will truncate the contents (to a length of bufsiz characters),
   // in case the buffer is too small to hold all of the contents

   char name_buf[1024] = { 0 };

#if defined(U_GDB_STACK_DUMP_ENABLE) && !defined(_MSWINDOWS_)
   int n = readlink("/proc/self/exe", name_buf, sizeof(name_buf) - 1);

   if (n > 0)
      {
      struct timespec req = { 2, 0 };

      name_buf[n] = 0;

      pid_t pid = fork();

      if (pid == 0)
         {
         char buf[32];
         int fd_err = open("/tmp/gbd.err", O_CREAT | O_WRONLY, 0666);

         (void) u__snprintf(buf, sizeof(buf), "--pid=%P", 0);

         (void) dup2(fd, STDOUT_FILENO);
#     ifdef U_COVERITY_FALSE_POSITIVE
         if (fd > 0)
#     endif
         (void) dup2(fd_err, STDERR_FILENO);

         (void) execlp("gdb", "gdb", "--nx", "--batch", "-ex", "thread apply all bt full", buf, name_buf, (char*)0); // thread apply all bt full 20

         abort();
         }

      (void) nanosleep(&req, 0);

      if (waitpid(pid, 0, WNOHANG) < 0)
         {
         (void) kill(u_pid, SIGCONT);

         (void) nanosleep(&req, 0);

         if (waitpid(pid, 0, WNOHANG) < 0) (void) kill(pid, SIGKILL);
         }

      if (lseek(fd, U_SEEK_BEGIN, SEEK_END) > 650)
         {
         (void) close(fd);

         return;
         }
      }
#endif

#ifdef HAVE_EXECINFO_H
   void* array[256];
   (void) memset(array, 0, sizeof(void*) * 256);

   // If you're using Linux, the standard C library includes a function called backtrace,
   // which populates an array with frames' return addresses, and another function called
   // backtrace_symbols, which will take the addresses from backtrace and look up the
   // corresponding function names. These are documented in the GNU C Library manual.

   int trace_size = backtrace(array, 256); /* use -rdynamic flag when compiling */

   if (trace_size <= 2) abort();

# if !defined(LINUX) && !defined(__LINUX__) && !defined(__linux__)
   // This function is similar to backtrace_symbols() but it writes the result immediately
   // to a file and can therefore also be used in situations where malloc() is not usable anymore

   backtrace_symbols_fd(array, trace_size, fd);
# else
   uint32_t output_len;
   char* functionNameEnd;
   char buffer[128 * 1024];

   FILE* f = fdopen(fd, "w");

   (void) fwrite(buffer, u__snprintf(buffer, sizeof(buffer), "%9D: %N (pid %P) === STACK TRACE ===\n", 0), 1, f);

# ifdef HAVE_DLFCN_H
   Dl_info dlinf;
# else
   int status;
   char* realname;
   char* lastparen;
   char* firstparen;
# endif

   char** strings = backtrace_symbols(array, trace_size);

   // -------------------------------------
   // NB: we start the loop from 3 to avoid
   // -------------------------------------
   // UError::stackDump()
   // u_debug_at_exit()
   // u__printf()
   // -------------------------------------

   for (int i = 3; i < trace_size; ++i)
      {
      (void) fprintf(f, "#%d %s\n", i-2, strings[i]);

#   ifdef HAVE_DLFCN_H
      if (dladdr(array[i], &dlinf)          == 0 ||
          strcmp(name_buf, dlinf.dli_fname) == 0)
         {
         output_len = execute_addr2line(buffer, sizeof(buffer), name_buf, array[i]);
         }
      else
         {
         output_len = execute_addr2line(buffer, sizeof(buffer), dlinf.dli_fname, (void*)((char*)array[i] - (char*)dlinf.dli_fbase));
         }

      // The source code below prints line numbers for all local functions.
      // If a function from another library is called, you might see a couple of ??:0 instead of file names.

      if (output_len &&
          buffer[0] != '?')
         {
         functionNameEnd = strchr(buffer, '\n');

         if (functionNameEnd)
            {
            *functionNameEnd++ = 0;

            (void) fprintf(f, "[%s]%s%.*s", buffer, functionNameEnd[0] == '\n' ? "" : "\n", (int)strlen(functionNameEnd), functionNameEnd);
            }
         }

      (void) fwrite(U_CONSTANT_TO_PARAM("--------------------------------------------------------------------\n"), 1, f);
#    else
      firstparen = strchr(strings[i], '('); // extract the identifier from strings[i]. It's inside of parens
      lastparen  = strchr(strings[i], '+');

      if (firstparen &&
          lastparen  &&
          firstparen < lastparen)
         {
         *lastparen = '\0';

         realname = abi::__cxa_demangle(firstparen + 1, 0, 0, &status);

         if (realname)
            {
            (void) fprintf(f, "%s\n", realname);

            free(realname);
            }
         }
#    endif
      }

   (void) fclose(f);
# endif
#endif
}
