// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//   command.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/command.h>
#include <ulib/process.h>
#include <ulib/timeval.h>
#include <ulib/notifier.h>
#include <ulib/file_config.h>
#include <ulib/utility/services.h>
#include <ulib/utility/string_ext.h>

#if defined(__OSX__) || defined(__NetBSD__) || defined(__UNIKERNEL__)
extern char** environ;
#endif

int   UCommand::status;
int   UCommand::exit_value;
int   UCommand::timeoutMS = -1;
pid_t UCommand::pid;

void UCommand::freeCommand()
{
   U_TRACE_NO_PARAM(0, "UCommand::freeCommand()")

   if (pathcmd)
      {
      U_SYSCALL_VOID(free, "%p", pathcmd);

      pathcmd = 0;
      }

   if (argv_exec)
      {
      U_INTERNAL_ASSERT_MAJOR(ncmd, 0)

      // NB: we need to consider that u_splitCommand() start by 1 and null terminator...

      UMemoryPool::_free(argv_exec, 1+ncmd+1 + U_ADD_ARGS, sizeof(char*));

      argv_exec = 0;
      }
}

void UCommand::freeEnvironment()
{
   U_TRACE_NO_PARAM(0, "UCommand::freeEnvironment()")

   if (envp_exec)
      {
      U_INTERNAL_ASSERT_MAJOR(nenv, 0)

      freeEnvironment(envp_exec, nenv);

      envp_exec = 0;
      }
}

void UCommand::reset(const UString* penv)
{
   U_TRACE(0, "UCommand::reset(%p)", penv)

   freeEnvironment();

   envp = 0;
   nenv = 0;

   if (penv)
      {
      environment = *penv;
                  // penv->clear(); NB: we can need it (environment) after for variable translation...
      }
   else
      {
      freeCommand();

      ncmd = nfile = 0;
      }
}

void UCommand::setCommand()
{
   U_TRACE_NO_PARAM(0, "UCommand::setCommand()")

   U_INTERNAL_ASSERT(command)

   command.duplicate();

   freeCommand();

   char* argv[U_MAX_ARGS];
   char buffer[U_PATH_MAX+1];

   argv[0] = argv[1] = 0;

   ncmd = u_splitCommand(U_STRING_TO_PARAM(command), argv, buffer, sizeof(buffer));

   U_INTERNAL_DUMP("ncmd = %d", ncmd)

   if (ncmd != -1)
      {
      U_INTERNAL_ASSERT_RANGE(1,ncmd,U_MAX_ARGS)

      if (buffer[0]) argv[0] = pathcmd = U_SYSCALL(strdup, "%S", buffer);

      U_INTERNAL_DUMP("pathcmd = %S", pathcmd)

      // NB: allocation and copy list arguments

      uint32_t n = 1+ncmd+1;

      argv_exec = (char**) UMemoryPool::_malloc(n + U_ADD_ARGS, sizeof(char*)); // U_ADD_ARGS => space for addArgument()...

      U_MEMCPY(argv_exec, argv, n * sizeof(char*)); // NB: copy also null terminator...
      }
   else
      {
      U_WARNING("setCommand() failed, command %V not found", command.rep);
      }

   U_DUMP_ATTRS(argv_exec)
}

int32_t UCommand::setEnvironment(const UString& env, char**& _envp)
{
   U_TRACE(0, "UCommand::setEnvironment(%V,%p)", env.rep, _envp)

   char* argp[U_MAX_ARGS];

   int32_t _nenv = u_split(U_STRING_TO_PARAM(env), argp, 0);

   U_INTERNAL_DUMP("_nenv = %d", _nenv)

   U_INTERNAL_ASSERT_RANGE(1, _nenv, U_MAX_ARGS)

   // NB: allocation and copy list arguments

   uint32_t n = _nenv + 1; // NB: consider also null terminator...

   _envp = (char**) UMemoryPool::_malloc(n, sizeof(char*)); // NB: consider also null terminator...

   U_MEMCPY(_envp, argp, n * sizeof(char*)); // NB: copy also null terminator...

   U_INTERNAL_DUMP("_envp = %p", _envp)

   U_DUMP_ATTRS(_envp)

   U_RETURN(_nenv);
}

U_NO_EXPORT void UCommand::setEnvironment(const UString& env)
{
   U_TRACE(0, "UCommand::setEnvironment(%V)", env.rep)

   freeEnvironment();

   env.duplicate();

   nenv = setEnvironment(env, envp);
}

void UCommand::setEnvironment(const UString* penv)
{
   U_TRACE(0, "UCommand::setEnvironment(%p)", penv)

   U_INTERNAL_ASSERT_POINTER(penv)
   U_INTERNAL_ASSERT_POINTER(argv_exec)

   environment = *penv;

   if ((flag_expand = penv->find('$')) == U_NOT_FOUND) setEnvironment(environment);

   U_INTERNAL_DUMP("flag_expand = %u", flag_expand)
}

void UCommand::setFileArgument()
{
   U_TRACE_NO_PARAM(0, "UCommand::setFileArgument()")

   U_INTERNAL_ASSERT_POINTER(argv_exec)

   U_INTERNAL_DUMP("ncmd = %d", ncmd)

   for (int32_t i = ncmd; i >= 2; --i)
      {
      U_INTERNAL_DUMP("argv_exec[%d] = %S", i, argv_exec[i])

      if (u_get_unalignedp32(argv_exec[i]) == U_MULTICHAR_CONSTANT32('$','F','I','L'))
         {
         nfile = i;

         break;
         }
      }

   U_INTERNAL_DUMP("nfile = %d", nfile)
}

void UCommand::setNumArgument(int32_t n, bool bfree)
{
   U_TRACE(1, "UCommand::setNumArgument(%d,%b)", n, bfree)

   U_INTERNAL_ASSERT_POINTER(argv_exec)

   // check if we need to free strndup() malloc... (see URPCGenericMethod::execute())

   if (bfree &&
       ncmd > n)
      {
      while (ncmd != n)
         {
         U_SYSCALL_VOID(free, "%p", argv_exec[ncmd]);

         --ncmd;
         }
      }

   argv_exec[(ncmd = n) + 1] = 0;
}

U_NO_EXPORT void UCommand::setStdInOutErr(int fd_stdin, bool flag_stdin, bool flag_stdout, int fd_stderr)
{
   U_TRACE(0, "UCommand::setStdInOutErr(%d,%b,%b,%d)", fd_stdin, flag_stdin, flag_stdout, fd_stderr)

   if (flag_stdin)
      {
      if (fd_stdin == -1)
         {
         UProcess::pipe(STDIN_FILENO); // UProcess::filedes[0] is for READING,
                                       // UProcess::filedes[1] is for WRITING
#     ifdef _MSWINDOWS_
         // Ensure the write handle to the pipe for STDIN is not inherited
         (void) U_SYSCALL(SetHandleInformation, "%p,%ld,%ld", UProcess::hFile[1], HANDLE_FLAG_INHERIT, 0);
#     endif
         }
      else
         {
         UProcess::filedes[0] = fd_stdin;
         }
      }

   if (flag_stdout)
      {
      UProcess::pipe(STDOUT_FILENO);   // UProcess::filedes[2] is for READING,
                                       // UProcess::filedes[3] is for WRITING
#  ifdef _MSWINDOWS_
      // Ensure the read handle to the pipe for STDOUT is not inherited
      (void) U_SYSCALL(SetHandleInformation, "%p,%ld,%ld", UProcess::hFile[2], HANDLE_FLAG_INHERIT, 0);
#  endif
      }

   if (fd_stderr != -1)
      {
      UProcess::filedes[5] = fd_stderr;
      }
}

U_NO_EXPORT void UCommand::execute(bool flag_stdin, bool flag_stdout, bool flag_stderr)
{
   U_TRACE(0, "UCommand::execute(%b,%b,%b)", flag_stdin, flag_stdout, flag_stderr)

   U_INTERNAL_ASSERT_POINTER(argv_exec)
   U_INTERNAL_ASSERT_POINTER(argv_exec[0])

   if (flag_expand != U_NOT_FOUND)
      {
      // NB: it must remain in this way (I don't understand why...)

      environment = UStringExt::expandEnvironmentVar(environment, &environment);

      setEnvironment(environment);
      }

#ifdef DEBUG
   char* _end  = (char*) command.pend();
   char* begin =         command.data();

   U_INTERNAL_DUMP("begin = %p end = %p argv_exec[1] = %p %S", begin, _end, argv_exec[1], argv_exec[1])

# ifndef _MSWINDOWS_
   U_INTERNAL_ASSERT_RANGE(begin, argv_exec[1], _end)
# endif

   int32_t i;

   // NB: we cannot check argv a cause of addArgument()...

   for (i = 0; argv_exec[1+i]; ++i) {}

   U_INTERNAL_ASSERT_EQUALS(i,ncmd)

   if (envp            &&
       envp != environ &&
       flag_expand == U_NOT_FOUND)
      {
      _end  = (char*) environment.pend();
      begin =         environment.data();

      U_INTERNAL_DUMP("begin = %p _end = %p envp[0] = %p %S", begin, _end, envp[0], envp[0])

      for (i = 0; envp[i]; ++i)
         {
         U_INTERNAL_ASSERT_RANGE(begin,envp[i],_end)
         }

      U_INTERNAL_ASSERT_EQUALS(i, nenv)
      }
#endif

   exit_value = status = -1;
   pid        = UProcess::execute(U_PATH_CONV(argv_exec[0]), argv_exec+1, envp, flag_stdin, flag_stdout, flag_stderr);
}

U_NO_EXPORT bool UCommand::postCommand(UString* input, UString* output)
{
   U_TRACE(1, "UCommand::postCommand(%p,%p)", input, output)

   U_INTERNAL_DUMP("pid = %d", pid)

   if (input)
      {
                    UFile::close(UProcess::filedes[0]); // UProcess::filedes[0] for READING...
                                 UProcess::filedes[0] = 0;
      if (pid <= 0) UFile::close(UProcess::filedes[1]); // UProcess::filedes[1] for WRITING...
      }

   if (output)
      {
                    UFile::close(UProcess::filedes[3]); // UProcess::filedes[3] for WRITING...
                                 UProcess::filedes[3] = 0;
      if (pid <= 0) UFile::close(UProcess::filedes[2]); // UProcess::filedes[2] for READING...
      }

   exit_value = status = -1;

   if (pid <= 0)
      {
      UProcess::filedes[1] = UProcess::filedes[2] = 0;

      U_RETURN(false);
      }

   if (input &&
       input != (void*)-1) // special value...
      {
      U_INTERNAL_ASSERT(*input)

      (void) UNotifier::write(UProcess::filedes[1], input->data(), input->size());

      UFile::close(UProcess::filedes[1]);
                   UProcess::filedes[1] = 0;
      }

   exit_value = status = 0;

   if (output &&
       output != (void*)-1) // special value...
      {
      bool kill_command = (UNotifier::waitForRead(UProcess::filedes[2], timeoutMS) <= 0);

#  ifdef _MSWINDOWS_
      if (kill_command && timeoutMS == -1) kill_command = false; // NB: we don't have select() on pipe...
#  endif

      if (kill_command == false) UServices::readEOF(UProcess::filedes[2], *output);

      UFile::close(UProcess::filedes[2]);
                   UProcess::filedes[2] = 0;

      if (kill_command)
         {
         // Timeout execeded

         UProcess::kill(pid, SIGTERM);

         UTimeVal::nanosleep(1L);

         UProcess::kill(pid, SIGKILL);
         }

      bool result = wait();

      if (kill_command)
         {
         exit_value = -EAGAIN;

         U_RETURN(false);
         }

      U_RETURN(result);
      }

   U_RETURN(true);
}

U_NO_EXPORT bool UCommand::wait()
{
   U_TRACE_NO_PARAM(0, "UCommand::wait()")

   UProcess::waitpid(pid, &status, 0);

   exit_value = UProcess::exitValue(status);

   U_RETURN(exit_value == 0);
}

bool UCommand::setMsgError(const char* cmd, bool only_if_error)
{
   U_TRACE(0, "UCommand::setMsgError(%S,%b)", cmd, only_if_error)

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   U_INTERNAL_DUMP("pid = %d exit_value = %d status = %d", pid, exit_value, status)

   // NB: '<' is reserved for xml...

   if (isStarted() == false)
      {
      u_buffer_len = u__snprintf(u_buffer, U_MAX_SIZE_PREALLOCATE, U_CONSTANT_TO_PARAM("command %S didn't start %R"),  cmd, 0); // NB: the last argument (0) is necessary...

      U_RETURN(true);
      }

   if (isTimeout() &&
       timeoutMS > 0)
      {
      u_buffer_len = u__snprintf(u_buffer, U_MAX_SIZE_PREALLOCATE, U_CONSTANT_TO_PARAM("command %S (pid %u) excedeed time (%d secs) for execution"), cmd, pid, timeoutMS / 1000);

      U_RETURN(true);
      }

   if (only_if_error == false)
      {
      char buffer[128];

      u_buffer_len = u__snprintf(u_buffer, U_MAX_SIZE_PREALLOCATE, U_CONSTANT_TO_PARAM("command %S started (pid %u) and ended with status: %d (%d, %s)"),
                                                                    cmd, pid, status, exit_value, UProcess::exitInfo(buffer, status));
      }

   U_RETURN(false);
}

// public method...

bool UCommand::executeWithFileArgument(UString* output, UFile* file)
{
   U_TRACE(0, "UCommand::executeWithFileArgument(%p,%p)", output, file)

   int fd_stdin = -1;

   if (getNumFileArgument() == -1)
      {
      if (file->open()) fd_stdin = file->getFd();
      }
   else
      {
      setFileArgument(file->getPathRelativ());
      }

   bool result = execute(0, output, fd_stdin, -1);

   if (result == false) printMsgError();

   if (file->isOpen()) file->close();

   U_RETURN(result);
}

bool UCommand::executeAndWait(UString* input, int fd_stdin, int fd_stderr)
{
   U_TRACE(0, "UCommand::executeAndWait(%p,%d,%d)", input, fd_stdin, fd_stderr)

   if (execute(input, 0, fd_stdin, fd_stderr) &&
       wait())
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UCommand::execute(UString* input, UString* output, int fd_stdin, int fd_stderr)
{
   U_TRACE(0, "UCommand::execute(%p,%p,%d,%d)", input, output, fd_stdin, fd_stderr)

   bool flag_stdin  = (input ? true : fd_stdin != -1),
        flag_stdout = (output != 0),
        flag_stderr = (fd_stderr != -1);

   setStdInOutErr(fd_stdin, flag_stdin, flag_stdout, fd_stderr);

   execute(flag_stdin, flag_stdout, flag_stderr);

   if (postCommand(input, output)) U_RETURN(true);

   U_RETURN(false);
}

// Return output of command

void UCommand::outputCommandWithDialog(const UString& cmd, char** penv, UString* output, int fd_stdin, int fd_stderr, bool dialog)
{
   U_TRACE(1, "UCommand::outputCommandWithDialog(%V,%p,%p,%d,%d,%b)", cmd.rep, penv, output, fd_stdin, fd_stderr, dialog)

   U_INTERNAL_ASSERT_POINTER(output)

   UCommand tmp(cmd, penv);
   bool flag_stdin = (fd_stdin  != -1), flag_stdout, flag_stderr;

   if (dialog)
      {
      // we must have already called UProcess::pipe(STDOUT_FILENO)...

      U_INTERNAL_ASSERT_MAJOR(UProcess::filedes[2],STDOUT_FILENO)
      U_INTERNAL_ASSERT_MAJOR(UProcess::filedes[3],STDOUT_FILENO)

      flag_stdout = false;
      flag_stderr = false;
      }
   else
      {
      flag_stdout = true;
      flag_stderr = (fd_stderr != -1);
      }

   setStdInOutErr(fd_stdin, flag_stdin, flag_stdout, fd_stderr);

   tmp.execute(flag_stdin, flag_stdout, flag_stderr);

   if (postCommand(0, output) == false || exit_value) (void) setMsgError(cmd.data(), false);
}

UString UCommand::outputCommand(const UString& cmd, char** penv, int fd_stdin, int fd_stderr)
{
   U_TRACE(0, "UCommand::outputCommand(%V,%p,%d,%d)", cmd.rep, penv, fd_stdin, fd_stderr)

   UString output(U_CAPACITY);

   outputCommandWithDialog(cmd, penv, &output, fd_stdin, fd_stderr, false);

   if (exit_value) printMsgError();

   U_RETURN_STRING(output);
}

UCommand* UCommand::loadConfigCommand(UFileConfig* cfg)
{
   U_TRACE(0, "UCommand::loadConfigCommand(%p)", cfg)

   U_INTERNAL_ASSERT_POINTER(cfg)

   UCommand* cmd   = 0;
   UString command = cfg->at(U_CONSTANT_TO_PARAM("COMMAND"));

   if (command)
      {
      if (U_ENDS_WITH(command, ".sh"))
         {
         UString buffer(U_CONSTANT_SIZE(U_PATH_SHELL) + 1 + command.size());

         buffer.snprintf(U_CONSTANT_TO_PARAM("%s %v"), U_PATH_SHELL, command.rep);

         command = buffer;
         }

      U_NEW(UCommand, cmd, UCommand(command));

      if (cmd->checkForExecute() == false)
         {
         U_WARNING("loadConfigCommand() failed, command %V not executable", cmd->command.rep);

         delete cmd;

         U_RETURN_POINTER(0,UCommand);
         }

      UString environment = cfg->at(U_CONSTANT_TO_PARAM("ENVIRONMENT"));

      if (environment)
         {
         environment = UStringExt::prepareForEnvironmentVar(environment); 

         cmd->setEnvironment(&environment);
         }
      }

   U_RETURN_POINTER(cmd,UCommand);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UCommand::dump(bool _reset) const
{
   *UObjectIO::os << "pid                     " << pid                  << '\n'
                  << "ncmd                    " << ncmd                 << '\n'
                  << "nfile                   " << nfile                << '\n'
                  << "exit_value              " << exit_value           << '\n'
                  << "command     (UString    " << (void*)&command      << ")\n"
                  << "environment (UString    " << (void*)&environment  << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
