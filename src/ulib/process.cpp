// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    process.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/process.h>
#include <ulib/utility/interrupt.h>

#include <ulib/base/utility.h>

#ifdef _MSWINDOWS_
HANDLE               UProcess::hFile[6];
HANDLE               UProcess::hChildIn;
HANDLE               UProcess::hChildOut;
HANDLE               UProcess::hChildErr;
STARTUPINFO          UProcess::aStartupInfo;
PROCESS_INFORMATION  UProcess::aProcessInformation;
#elif defined(HAVE_POSIX_SPAWN)
#  include <spawn.h>
#endif

int UProcess::filedes[6];

// services for EXEC

void UProcess::kill(pid_t pid, int sig)
{
   U_TRACE(1, "UProcess::kill(%d,%d)", pid, sig)

   (void) U_SYSCALL(kill, "%d,%d", pid, sig);
}

void UProcess::nice(int inc)
{
   U_TRACE(1, "UProcess::nice(%d)", inc)

   (void) U_SYSCALL(nice, "%d", inc);
}

void UProcess::setProcessGroup(pid_t pid, pid_t pgid)
{
   U_TRACE(1, "UProcess::setProcessGroup(%d,%d)", pid, pgid)

#ifndef _MSWINDOWS_
   (void) U_SYSCALL(setpgid, "%d,%d", pid, pgid);
#endif
}

bool UProcess::fork()
{
   U_TRACE_NO_PARAM(1, "UProcess::fork()")

   U_CHECK_MEMORY

   _pid = U_FORK();

   if (child()) u_setPid();

   running = (_pid != -1);

   U_INTERNAL_DUMP("%P running = %b", running)

   U_RETURN(running);
}

// inlining failed in call to 'UProcess::setStdInOutErr(bool, bool, bool)': call is unlikely and code size would grow

U_NO_EXPORT void UProcess::setStdInOutErr(bool fd_stdin, bool fd_stdout, bool fd_stderr)
{
   U_TRACE(1, "UProcess::setStdInOutErr(%b,%b,%b)", fd_stdin, fd_stdout, fd_stderr) // problem with sanitize address

#ifdef _MSWINDOWS_
   HANDLE hProcess = GetCurrentProcess();
#endif

   // check if we write to STDIN

   if (fd_stdin)
      {
#  ifdef _MSWINDOWS_
      if (hFile[1]) // Created parent-output pipe...
         {
         hChildIn = hFile[0];

         // Duplicating as inheritable child-input pipe
         // --------------------------------------------------------------------------------------------------------------------------------------
         // (void) U_SYSCALL(DuplicateHandle, "%p,%p,%p,%p,%lu,%b,%lu", hProcess, hFile[0], hProcess, &hChildIn, 0, TRUE, DUPLICATE_SAME_ACCESS);
         // (void) U_SYSCALL(    CloseHandle, "%p",                               hFile[0]);
         // --------------------------------------------------------------------------------------------------------------------------------------
         }
      else
         {
         hChildIn = (HANDLE)_get_osfhandle(filedes[0]);
         }
#  else
      U_INTERNAL_ASSERT_MAJOR(filedes[0], STDERR_FILENO)

#     ifndef HAVE_DUP3
      (void) U_SYSCALL(dup2, "%d,%d",    filedes[0], STDIN_FILENO);
#     else
      (void) U_SYSCALL(dup3, "%d,%d,%d", filedes[0], STDIN_FILENO, O_CLOEXEC);
#     endif

      U_INTERNAL_ASSERT_EQUALS(::fcntl(STDIN_FILENO,F_GETFD,FD_CLOEXEC), 0)
#  endif
      }

   // check if we read from STDOUT

   if (fd_stdout)
      {
#  ifdef _MSWINDOWS_
      if (hFile[2]) // Created parent-input pipe...
         {
         hChildOut = hFile[3];

         // Duplicating as inheritable child-output pipe
         // --------------------------------------------------------------------------------------------------------------------------------------
         // (void) U_SYSCALL(DuplicateHandle, "%p,%p,%p,%p,%lu,%b,%lu", hProcess, hFile[3], hProcess, &hChildOut, 0, TRUE, DUPLICATE_SAME_ACCESS);
         // (void) U_SYSCALL(    CloseHandle, "%p",                               hFile[3]);
         // --------------------------------------------------------------------------------------------------------------------------------------
         }
      else
         {
         hChildOut = (HANDLE)_get_osfhandle(filedes[3]);
         }
#  else
      U_INTERNAL_ASSERT_MAJOR(filedes[3], STDOUT_FILENO)

#     ifndef HAVE_DUP3
      (void) U_SYSCALL(dup2, "%d,%d",    filedes[3], STDOUT_FILENO);
#     else
      (void) U_SYSCALL(dup3, "%d,%d,%d", filedes[3], STDOUT_FILENO, O_CLOEXEC);
#     endif

      U_INTERNAL_ASSERT_EQUALS(::fcntl(STDOUT_FILENO,F_GETFD,FD_CLOEXEC), 0)
#  endif
      }

   // check if we read from STDERR

   if (fd_stderr)
      {
#  ifdef _MSWINDOWS_
      if (hFile[4]) // Created parent-input pipe...
         {
         hChildErr = hFile[5];

         // Duplicating as inheritable child-output pipe
         // --------------------------------------------------------------------------------------------------------------------------------------
         // (void) U_SYSCALL(DuplicateHandle, "%p,%p,%p,%p,%lu,%b,%lu", hProcess, hFile[5], hProcess, &hChildErr, 0, TRUE, DUPLICATE_SAME_ACCESS);
         // (void) U_SYSCALL(    CloseHandle, "%p",                               hFile[5]);
         // --------------------------------------------------------------------------------------------------------------------------------------
         }
      else
         {
         hChildErr = (HANDLE)_get_osfhandle(filedes[5]);
         }
#  else
      U_INTERNAL_ASSERT(filedes[5] >= STDIN_FILENO)

#     ifndef HAVE_DUP3
      (void) U_SYSCALL(dup2, "%d,%d",    filedes[5], STDERR_FILENO);
#     else
      (void) U_SYSCALL(dup3, "%d,%d,%d", filedes[5], STDERR_FILENO, O_CLOEXEC);
#     endif

      U_INTERNAL_ASSERT_EQUALS(::fcntl(STDERR_FILENO,F_GETFD,FD_CLOEXEC), 0)
#  endif
      }

#ifdef _MSWINDOWS_
   CloseHandle(hProcess);
#else
   if (fd_stdin)
      {
      U_INTERNAL_DUMP("filedes[0,1] = { %d, %d }", filedes[0], filedes[1])

                                      (void) U_SYSCALL(close, "%d", filedes[0]);
      if (filedes[1] > STDERR_FILENO) (void) U_SYSCALL(close, "%d", filedes[1]);
      }

   if (fd_stdout)
      {
      U_INTERNAL_DUMP("filedes[2,3] = { %d, %d }", filedes[2], filedes[3])

                                      (void) U_SYSCALL(close, "%d", filedes[3]);
      if (filedes[2] > STDERR_FILENO) (void) U_SYSCALL(close, "%d", filedes[2]);
      }

   if (fd_stderr)
      {
      U_INTERNAL_DUMP("filedes[4,5] = { %d, %d }", filedes[4], filedes[5])

                                      (void) U_SYSCALL(close, "%d", filedes[5]);
      if (filedes[4] > STDERR_FILENO) (void) U_SYSCALL(close, "%d", filedes[4]);
      }
#endif
}

#ifdef _MSWINDOWS_
void UProcess::pipe(int fdp)
{
   U_TRACE(1, "UProcess::pipe(%d)", fdp)

   U_INTERNAL_ASSERT_RANGE(STDIN_FILENO, fdp, STDERR_FILENO)

   int fdn = (fdp * 2); // filedes[fdn] is for READING, filedes[fdn+1] is for WRITING

   if (U_SYSCALL(CreatePipe, "%p,%p,%p,%lu", hFile+fdn, hFile+fdn+1, &sec_none, 0))
      {
      U_INTERNAL_DUMP("hFile[%d,%d] = { %p, %p }", fdn, fdn+1, hFile[fdn], hFile[fdn+1])
      }
   else
      {
      U_INTERNAL_DUMP("$R", "CreatePipe()")
      }
}
#else
void UProcess::pipe(int fdp)
{
   U_TRACE(1, "UProcess::pipe(%d)", fdp)

   // pipe() creates a pair of file descriptors, pointing to a pipe inode, and places them in the array pointed to by fds.

   U_INTERNAL_ASSERT_RANGE(STDIN_FILENO, fdp, STDERR_FILENO)

   int* fds = filedes + (fdp * 2); // fds[0] is for READING, fds[1] is for WRITING

#  ifndef HAVE_PIPE2
   (void) U_SYSCALL(pipe, "%p",     fds);
#  else
   (void) U_SYSCALL(pipe2, "%p,%d", fds, O_CLOEXEC);
#  endif

   U_INTERNAL_DUMP("filedes[%d,%d] = { %d, %d }", (fdp * 2), (fdp * 2) + 1, fds[0], fds[1])
}
#endif

#ifdef _MSWINDOWS_
static inline BOOL is_console(HANDLE h) { return h != INVALID_HANDLE_VALUE && ((ULONG_PTR)h & 3) == 3; }

pid_t UProcess::execute(const char* pathname, char* argv[], char* envp[], bool fd_stdin, bool fd_stdout, bool fd_stderr)
{
   U_TRACE(1, "UProcess::execute(%S,%p,%p,%b,%b,%b)", pathname, argv, envp, fd_stdin, fd_stdout, fd_stderr)

   U_INTERNAL_ASSERT_POINTER(argv)
   U_DUMP_EXEC(argv, envp)
   U_INTERNAL_ASSERT_EQUALS(strcmp(argv[0],u_basename(pathname)), 0)

   (void) U_SYSCALL(memset, "%p,%d,%lu",        &aStartupInfo, 0, sizeof(STARTUPINFO));
   (void) U_SYSCALL(memset, "%p,%d,%lu", &aProcessInformation, 0, sizeof(PROCESS_INFORMATION));

   /*
   typedef struct _STARTUPINFO {
   DWORD cb;            // Size of the structure, in bytes
   LPTSTR lpReserved;
   LPTSTR lpDesktop;
   LPTSTR lpTitle;
   DWORD dwX;
   DWORD dwY;
   DWORD dwXSize;
   DWORD dwYSize;
   DWORD dwXCountChars;
   DWORD dwYCountChars;
   DWORD dwFillAttribute;
   DWORD dwFlags;
   WORD wShowWindow;
   WORD cbReserved2;
   LPBYTE lpReserved2;
   HANDLE hStdInput;
   HANDLE hStdOutput;
   HANDLE hStdError;
   } STARTUPINFO, *LPSTARTUPINFO;
   */

   aStartupInfo.cb          = sizeof(STARTUPINFO);
   aStartupInfo.dwFlags     = STARTF_USESHOWWINDOW;
   aStartupInfo.wShowWindow = SW_SHOWNORMAL;

   // STARTF_USESTDHANDLES - Sets the standard input, standard output, and standard error handles for the process
   // to the handles specified in the hStdInput, hStdOutput, and hStdError members of the STARTUPINFO structure.
   // For this to work properly, the handles must be inheritable and the CreateProcess function's fInheritHandles
   // parameter must be set to TRUE. If this value is not specified, the hStdInput, hStdOutput, and hStdError
   // members of the STARTUPINFO structure are ignored

   if (fd_stdin || fd_stdout || fd_stderr)
      {
      aStartupInfo.dwFlags |= STARTF_USESTDHANDLES; // Tell the new process to use our std handles

      setStdInOutErr(fd_stdin, fd_stdout, fd_stderr);
      }

   aStartupInfo.hStdInput  = (fd_stdin  ? hChildIn  : GetStdHandle(STD_INPUT_HANDLE));
   aStartupInfo.hStdOutput = (fd_stdout ? hChildOut : GetStdHandle(STD_OUTPUT_HANDLE));
   aStartupInfo.hStdError  = (fd_stderr ? hChildErr : GetStdHandle(STD_ERROR_HANDLE));

   U_INTERNAL_DUMP("hStdInput(%b) = %p hStdOutput(%b) = %p hStdError(%b) = %p", is_console(aStartupInfo.hStdInput),  aStartupInfo.hStdInput,
                                                                                is_console(aStartupInfo.hStdOutput), aStartupInfo.hStdOutput,
                                                                                is_console(aStartupInfo.hStdError),  aStartupInfo.hStdError)

   char* w32_shell = (strncmp(argv[0], U_CONSTANT_TO_PARAM("sh.exe") == 0) ? (char*)pathname : 0);

   U_INTERNAL_DUMP("w32_shell = %S", w32_shell)

   int index = 0;
   char buffer1[4096];
   char* w32_cmd = (char*)pathname;

   if (argv[1])
      {
      bool flag;
      w32_cmd = buffer1;
      int len = u__strlen(pathname, __PRETTY_FUNCTION__);

      if (len)
         {
         index = len;

         U_MEMCPY(w32_cmd, pathname, len);
         }

      for (int i = 1; argv[i]; ++i)
         {
         w32_cmd[index++] = ' ';

         len = u__strlen(argv[i], __PRETTY_FUNCTION__);

         if (len)
            {
            U_INTERNAL_ASSERT_MINOR(index+len+3, 4096)

            flag = (strchr(argv[i], ' ') != 0);

            if (flag) w32_cmd[index++] = '"';

            U_MEMCPY(w32_cmd+index, argv[i], len);

            index += len;

            if (flag) w32_cmd[index++] = '"';
            }
         }

      w32_cmd[index] = '\0';
      }

   U_INTERNAL_DUMP("w32_cmd(%d) = %.*S", index, index, w32_cmd)

   char* w32_envp = 0;
   char buffer2[32000] = { '\0' };

   if (envp)
      {
      index    = 0;
      w32_envp = buffer2;

      for (int len, i = 0; envp[i]; ++i, ++index)
         {
         len = u__strlen(envp[i], __PRETTY_FUNCTION__);

         if (len)
            {
            U_INTERNAL_ASSERT_MINOR(index+len+1, 32000)

            U_MEMCPY(w32_envp+index, envp[i], len);

            index += len;

            w32_envp[index] = '\0';
            }
         }

      w32_envp[index+1] = '\0';

      U_INTERNAL_DUMP("w32_envp(%d) = %#.*S", index, index+1, w32_envp)
      }

   pid_t pid;
   BOOL fRet = U_SYSCALL(CreateProcessA, "%S,%S,%p,%p,%b,%lu,%p,%p,%p,%p",
                             w32_shell,               // No module name (use command line)
                             w32_cmd,                 // Command line
                             &sec_none,               // Default process security attributes
                             &sec_none,               // Default  thread security attributes
                             sec_none.bInheritHandle, // inherit handles from the parent
                             DETACHED_PROCESS,        // the new process does not inherit its parent's console
                             w32_envp,                // if NULL use the same environment as the parent
                             0,                       // Launch in the current directory
                             &aStartupInfo,           // Startup Information
                             &aProcessInformation);   // Process information stored upon return

   if (fRet)
      {
      /*
      typedef struct _PROCESS_INFORMATION {
      HANDLE hProcess;
      HANDLE hThread;
      DWORD dwProcessId;
      DWORD dwThreadId;
      } PROCESS_INFORMATION;
      */

      U_INTERNAL_DUMP("dwProcessId = %p hProcess = %p hThread = %p", aProcessInformation.dwProcessId,
                                                                     aProcessInformation.hProcess,
                                                                     aProcessInformation.hThread)

      u_hProcess = aProcessInformation.hProcess;

      pid = (pid_t) aProcessInformation.dwProcessId;

      (void) U_SYSCALL(CloseHandle, "%p", aProcessInformation.hThread);
      }
   else
      {
      U_INTERNAL_DUMP("$R", "CreateProcess()")

      pid = -1;
      }

   /* associate handle to filedes */

   if (fd_stdin && hFile[1])
      {
      filedes[0] = _open_osfhandle((long)hChildIn, (_O_RDONLY | O_BINARY));
      filedes[1] = _open_osfhandle((long)hFile[1], (_O_WRONLY | O_BINARY));

      U_INTERNAL_DUMP("filedes[0,1] = { %d, %d }", filedes[0], filedes[1])
      }

   if (fd_stdout && hFile[2])
      {
      filedes[2] = _open_osfhandle((long)hFile[2],  (_O_RDONLY | O_BINARY));
      filedes[3] = _open_osfhandle((long)hChildOut, (_O_WRONLY | O_BINARY));

      U_INTERNAL_DUMP("filedes[2,3] = { %d, %d }", filedes[2], filedes[3])
      }

   if (fd_stderr && hFile[4])
      {
      filedes[4] = _open_osfhandle((long)hFile[4],  (_O_RDONLY | O_BINARY));
      filedes[5] = _open_osfhandle((long)hChildErr, (_O_WRONLY | O_BINARY));

      U_INTERNAL_DUMP("filedes[4,5] = { %d, %d }", filedes[4], filedes[5])
      }

   hChildIn = hChildOut = hChildErr = 0;

   (void) U_SYSCALL(memset, "%p,%d,%lu", hFile, 0, sizeof(hFile));

   U_RETURN(pid);
}
#else
pid_t UProcess::execute(const char* pathname, char* argv[], char* envp[], bool fd_stdin, bool fd_stdout, bool fd_stderr)
{
   U_TRACE(1, "UProcess::execute(%S,%p,%p,%b,%b,%b)", pathname, argv, envp, fd_stdin, fd_stdout, fd_stderr)

   U_INTERNAL_ASSERT_POINTER(argv)
   U_DUMP_EXEC(argv, envp)
   U_INTERNAL_ASSERT_EQUALS(strcmp(u_basename(pathname), argv[0]), 0)

   pid_t pid;

#  ifdef HAVE_POSIX_SPAWN
   posix_spawn_file_actions_t action;

   (void) U_SYSCALL(posix_spawn_file_actions_init, "%p", &action);

   // check if we write to STDIN

   if (fd_stdin)
      {
      U_INTERNAL_ASSERT_MAJOR(filedes[0], STDERR_FILENO)

      (void) U_SYSCALL(posix_spawn_file_actions_adddup2, "%p,%d,%d", &action, filedes[0], STDIN_FILENO);

                                      (void) U_SYSCALL(posix_spawn_file_actions_addclose, "%p,%d", &action, filedes[0]);
      if (filedes[1] > STDERR_FILENO) (void) U_SYSCALL(posix_spawn_file_actions_addclose, "%p,%d", &action, filedes[1]);
      }

   // check if we read from STDOUT

   if (fd_stdout)
      {
      U_INTERNAL_ASSERT_MAJOR(filedes[3], STDOUT_FILENO)

      (void) U_SYSCALL(posix_spawn_file_actions_adddup2, "%p,%d,%d", &action, filedes[3], STDOUT_FILENO);

                                      (void) U_SYSCALL(posix_spawn_file_actions_addclose, "%p,%d", &action, filedes[3]);
      if (filedes[2] > STDERR_FILENO) (void) U_SYSCALL(posix_spawn_file_actions_addclose, "%p,%d", &action, filedes[2]);
      }

   // check if we read from STDERR

   if (fd_stderr)
      {
      U_INTERNAL_ASSERT(filedes[5] >= STDIN_FILENO)

      (void) U_SYSCALL(posix_spawn_file_actions_adddup2, "%p,%d,%d", &action, filedes[5], STDERR_FILENO);

                                      (void) U_SYSCALL(posix_spawn_file_actions_addclose, "%p,%d", &action, filedes[5]);
      if (filedes[4] > STDERR_FILENO) (void) U_SYSCALL(posix_spawn_file_actions_addclose, "%p,%d", &action, filedes[4]);
      }

   (void) U_SYSCALL(posix_spawn, "%p,%S,%p,%p,%p,%p", &pid, pathname, &action, 0, argv, envp);

   (void) U_SYSCALL(posix_spawn_file_actions_destroy, "%p", &action);
#  else
   pid = U_VFORK();

   if (pid == 0) // child
      {
      setStdInOutErr(fd_stdin, fd_stdout, fd_stderr);

      U_EXEC(pathname, argv, envp);
      }

   // parent

   if (u_exec_failed) U_RETURN(-1);
#  endif

   U_RETURN(pid);
   }
#endif

/**
 * The wait() system call suspends execution of the calling process until one of its children terminates.
 * The call wait(&status) is equivalent to:
 *
 *    waitpid(-1, &status, 0);
 *
 * The waitpid() system call suspends execution of the calling process until a child specified by pid argument has changed state.
 * By default, waitpid() waits only for terminated children, but this behavior is modifiable via the options argument, as described below.
 *
 * The value of pid can be:
 *
 * < -1 meaning wait for any child process whose process group ID is equal to the absolute value of pid.
 *   -1 meaning wait for any child process.
 *    0 meaning wait for any child process whose process group ID is equal to that of the calling process.
 * >  0 meaning wait for the child whose process ID is equal to the value of pid.
 *
 * The value of options is an OR of zero or more of the following constants:
 *
 * WNOHANG(1)    return immediately if no child has exited.
 * WUNTRACED(2)  also return if a child has stopped (but not traced via ptrace(2)).
 *               Status for traced children which have stopped is provided even if this option is not specified.
 * WCONTINUED(8) (since Linux 2.6.10) also return if a stopped child has been resumed by delivery of SIGCONT
 */

int UProcess::waitpid(pid_t pid, int* _status, int options)
{
   U_TRACE(1, "UProcess::waitpid(%d,%p,%d)", pid, _status, options)

   int result;

loop:
   result = U_SYSCALL(waitpid, "%d,%p,%d", pid, _status, options);

   if (result == -1 &&
        errno == EINTR)
      {
      UInterrupt::checkForEventSignalPending();

      if (UInterrupt::isSysCallToRestart()) goto loop;
      }

   // errno == ECHILD: if the process specified in pid does not exist or is not a child of the calling process...

#if DEBUG
   if (_status) U_INTERNAL_DUMP("status = %d", *_status)
#endif

   U_RETURN(result);
}

void UProcess::wait()
{
   U_TRACE_NO_PARAM(0, "UProcess::wait()")

   U_CHECK_MEMORY

   if (running)
      {
      waitpid(_pid, &status, 0);

      running = false;
      }

#ifdef DEBUG
   char buffer[128];

   (void) exitInfo(buffer, status);

   U_INTERNAL_DUMP("status = %d, %S", status, buffer)
#endif
}

int UProcess::waitAll(int timeoutMS)
{
   U_TRACE(1, "UProcess::waitAll(%d)", timeoutMS)

   if (timeoutMS) UInterrupt::setAlarm(timeoutMS);

   wait();

#ifdef DEBUG
   char buffer[128];
#endif
   int result = (status ? U_FAILED_ALL    // (status != 0) -> failed
                        : U_FAILED_NONE); // (status == 0) -> success

   if (timeoutMS) UInterrupt::setAlarm(timeoutMS);

   while (UProcess::waitpid(-1, &status, 0) > 0)
      {
      if ((status == 0 && result == U_FAILED_ALL) || // (status == 0) -> success
          (status != 0 && result == U_FAILED_NONE))  // (status != 0) -> failed
         {
         result = U_FAILED_SOME;
         }

      U_DUMP("result = %b status = %d, %S", result, status, exitInfo(buffer))
      }

   if (timeoutMS) UInterrupt::resetAlarm();

   U_RETURN(result);
}

char* UProcess::exitInfo(char* buffer, int _status)
{
   U_TRACE(0, "UProcess::exitInfo(%p,%d)", buffer, _status)

   uint32_t n = 0;

   if (WIFEXITED(_status))
      {
      n = u__snprintf(buffer, 128, U_CONSTANT_TO_PARAM("Exit %d"), WEXITSTATUS(_status));
      }
   else if (WIFSIGNALED(_status))
      {
#  ifndef WCOREDUMP
#  define WCOREDUMP(status) ((status) & 0200) // settimo bit
#  endif
      n = u__snprintf(buffer, 128, U_CONSTANT_TO_PARAM("Signal %Y%s"), WTERMSIG(_status), (WCOREDUMP(_status) ? " - core dumped" : ""));
      }
   else if (WIFSTOPPED(_status))
      {
      n = u__snprintf(buffer, 128, U_CONSTANT_TO_PARAM("Signal %Y"), WSTOPSIG(_status));
      }
#  ifdef __clang__
#  undef WIFCONTINUED // to avoid warning: equality comparison with extraneous parentheses...
#  endif
#  ifndef WIFCONTINUED
#  define WIFCONTINUED(status) status == 0xffff
#  endif
   else if (WIFCONTINUED(_status))
      {
      U_MEMCPY(buffer, "SIGCONT", (n = U_CONSTANT_SIZE("SIGCONT")));
      }

   U_INTERNAL_ASSERT_MINOR(n, 128)

   buffer[n] = '\0';

   U_RETURN(buffer);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UProcess::dump(bool reset) const
{
   *UObjectIO::os << "pid             " << _pid    << '\n'
                  << "status          " << status  << '\n' 
                  << "running         " << running;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
