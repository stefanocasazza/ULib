// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    process.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_PROCESS_H
#define ULIB_PROCESS_H 1

#include <ulib/internal/common.h>

#ifdef HAVE_SYSEXITS_H
#  include <sysexits.h>
#else
#  include <ulib/base/replace/sysexits.h>
#endif

#if defined(__CYGWIN__) || defined(__APPLE__) || !defined(_MSWINDOWS_)
#  include <sys/wait.h>
#  ifndef _MSWINDOWS_
#     include <sys/socket.h>
#  endif
#endif

#define U_FAILED_NONE  0
#define U_FAILED_SOME  2
#define U_FAILED_ALL   3

class UCommand;
class UServer_Base;
class UNoCatPlugIn;

class U_EXPORT UProcess {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UProcess()
      {
      U_TRACE_REGISTER_OBJECT(0, UProcess, "", 0)

      _pid    = (pid_t)-1;
      status  = 0;
      running = false;
      }

   ~UProcess()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UProcess)
      }

   // FORK

   bool fork();

   bool  child() const { return (_pid == 0); }
   bool parent() const { return (_pid >  0); }

   pid_t  pid() const  { return  _pid; }
   pid_t ppid() const
      {
      U_TRACE_NO_PARAM(0, "UProcess::ppid()")

      pid_t _ppid = U_SYSCALL_NO_PARAM(getppid);

      U_RETURN(_ppid);
      }

   // WAIT

   void wait();
   int  waitAll(int timeoutMS = 0);

   static uint32_t removeZombies()
      {
      U_TRACE_NO_PARAM(1, "UProcess::removeZombies()")

      uint32_t n = 0;

#  ifndef _MSWINDOWS_
      while (U_SYSCALL(waitpid, "%d,%p,%d", -1, 0, WNOHANG) > 0) ++n;
#  endif

      U_RETURN(n);
      }

   static int waitpid(pid_t pid = -1, int* status = 0, int options = WNOHANG);

   // STATUS CHILD

   static char* exitInfo(char* buffer, int status);

          char* exitInfo(char* buffer) const { return exitInfo(buffer, status); }

   static int exitValue(int _status)
      {
      U_TRACE(0, "UProcess::exitValue(%d)", _status)

      int exit_value = (  WIFEXITED(_status) ? WEXITSTATUS(_status)       :
                        WIFSIGNALED(_status) ? - (WTERMSIG(_status) << 8) : -1);

      U_RETURN(exit_value);
      }

   int exitValue() const { return exitValue(status); }

   // Inter Process Comunication (IPC)

   static int filedes[6]; // 3 serie di pipe to manage file descriptors for child I/O redirection...

   static void pipe(int fdp);

   // services for EXEC

   static void nice(int inc);
   static void kill(pid_t pid, int sig);
   static void setProcessGroup(pid_t pid = 0, pid_t pgid = 0);

   // exec with internal vfork() with management of file descriptors for child I/O redirection...

   static pid_t execute(const char* pathname, char* argv[], char* envp[], bool fd_stdin, bool fd_stdout, bool fd_stderr);

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool) const;
#endif

protected:
   pid_t _pid;
   int status;
   bool running;

#ifdef _MSWINDOWS_
   static STARTUPINFO aStartupInfo;
   static PROCESS_INFORMATION aProcessInformation;
   static HANDLE hFile[6], hChildIn, hChildOut, hChildErr;
#endif

   static void setStdInOutErr(bool fd_stdin, bool fd_stdout, bool fd_stderr) U_NO_EXPORT;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UProcess)

   friend class UCommand;
   friend class UServer_Base;
   friend class UNoCatPlugIn;
};

#endif
