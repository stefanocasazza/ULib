// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    command.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_COMMAND_H
#define ULIB_COMMAND_H 1

#include <ulib/string.h>

class UFile;
class UHTTP;
class UDialog;
class UFileConfig;
class UServer_Base;
class UProxyPlugIn;
class UNoCatPlugIn;

#define U_ADD_ARGS  100 
#define U_MAX_ARGS 8192 

class U_EXPORT UCommand {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   void zero()
      {
      envp = 0;
      pathcmd = 0;
      ncmd = nenv = nfile = 0;
      flag_expand = U_NOT_FOUND;
      argv_exec = envp_exec = 0;
      }

   void reset(const UString* penv);

   UCommand()
      {
      U_TRACE_REGISTER_OBJECT(0, UCommand, "", 0)

      zero();
      }

   UCommand(const UString& cmd, char** penv = 0) : command(cmd)
      {
      U_TRACE_REGISTER_OBJECT(0, UCommand, "%V,%p", cmd.rep, penv)

      zero();
      setCommand();
      setEnvironment(penv);
      }

   UCommand(const UString& cmd, const UString* penv) : command(cmd)
      {
      U_TRACE_REGISTER_OBJECT(0, UCommand, "%V,%p", cmd.rep, penv)

      zero();
      setCommand();
      setEnvironment(penv);
      }

   ~UCommand()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UCommand)

      reset(0);
      }

   // MANAGE GENERIC ENVIRONMENT

   void setEnvironment(char** penv)
      {
      U_TRACE(0, "UCommand::setEnvironment(%p)", penv)

      envp        = penv;
      flag_expand = U_NOT_FOUND;
      }

   void setEnvironment(const UString* env);

   // MANAGE GENERIC ARGUMENT

   void delArgument()
      {
      U_TRACE_NO_PARAM(0, "UCommand::delArgument()")

      U_INTERNAL_ASSERT_POINTER(argv_exec)

      argv_exec[ncmd--] = 0;

      U_INTERNAL_DUMP("ncmd = %d", ncmd)

      U_INTERNAL_ASSERT_RANGE(1,ncmd,U_ADD_ARGS)
      }

   void addArgument(const char* argument)
      {
      U_TRACE(0, "UCommand::addArgument(%S)", argument)

      U_INTERNAL_ASSERT_POINTER(argv_exec)
      U_INTERNAL_ASSERT(u_isText((const unsigned char*)argument, u__strlen(argument, __PRETTY_FUNCTION__)))

      argv_exec[++ncmd] = (char*) argument;
      argv_exec[ncmd+1] = 0;

      U_INTERNAL_DUMP("ncmd = %d", ncmd)

      U_INTERNAL_ASSERT_RANGE(1,ncmd,U_ADD_ARGS)
      }

   void setArgument(int n, const char* argument)
      {
      U_TRACE(0, "UCommand::setArgument(%d,%S)", n, argument)

      U_INTERNAL_ASSERT_RANGE(2,n,ncmd)
      U_INTERNAL_ASSERT_POINTER(argv_exec)
      U_INTERNAL_ASSERT(u_isText((const unsigned char*)argument, u__strlen(argument, __PRETTY_FUNCTION__)))

      argv_exec[n] = (char*) argument;
      }

   void setLastArgument(const char* argument)
      {
      U_TRACE(0, "UCommand::setLastArgument(%S)", argument)

      U_INTERNAL_ASSERT_POINTER(argv_exec)
      U_INTERNAL_ASSERT_EQUALS(argv_exec[ncmd+1],0)
      U_INTERNAL_ASSERT(u_isText((const unsigned char*)argument, u__strlen(argument, __PRETTY_FUNCTION__)))

      argv_exec[ncmd] = (char*) argument;
      }

   char* getArgument(int n) const __pure
      {
      U_TRACE(0, "UCommand::getArgument(%d)", n)

      char* arg = (argv_exec ? argv_exec[n] : 0);

      U_INTERNAL_ASSERT(arg == 0 || u_isText((const unsigned char*)arg, u__strlen(arg, __PRETTY_FUNCTION__)))

      U_RETURN(arg);
      }

   void setNumArgument(int32_t n = 1, bool bfree = false);

   // MANAGE FILE ARGUMENT

   void setFileArgument();
   void setFileArgument(const char* pathfile)
      {
      U_TRACE(0, "UCommand::setFileArgument(%S)", pathfile)

      U_INTERNAL_ASSERT_POINTER(argv_exec)
      U_INTERNAL_ASSERT(u_isText((const unsigned char*)pathfile, u__strlen(pathfile, __PRETTY_FUNCTION__)))

      U_INTERNAL_DUMP("ncmd = %d", ncmd)

      U_INTERNAL_ASSERT_RANGE(2,nfile,ncmd)

      argv_exec[nfile] = (char*) pathfile;
      }

   int32_t getNumArgument() const       { return ncmd; }
   int32_t getNumFileArgument() const   { return nfile; }

   UString getStringCommand()           { return command; }
   UString getStringEnvironment()       { return environment; }

   void setCommand(const UString& cmd)
      {
      U_TRACE(0, "UCommand::setCommand(%V)", cmd.rep)

      command = cmd;

      setCommand();
      }

   void set(const UString& cmd, char** penv = 0)
      {
      U_TRACE(0, "UCommand::set(%V,%p)", cmd.rep, penv)

      setCommand(cmd);
      setEnvironment(penv);
      }

   void set(const UString& cmd, const UString* penv)
      {
      U_TRACE(0, "UCommand::set(%V,%p)", cmd.rep, penv)

      setCommand(cmd);
      setEnvironment(penv);
      }

   bool isShellScript() const __pure
      {
      U_TRACE_NO_PARAM(0, "UCommand::isShellScript()")

      U_INTERNAL_ASSERT(command)

      if (strncmp(command.data(), U_CONSTANT_TO_PARAM(U_PATH_SHELL)) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   char* getCommand() const __pure { return getArgument(isShellScript() ? 2 : 0); }

   // SERVICES

   static pid_t pid;
   static int timeoutMS; // specified the timeout value, in milliseconds for read output. A negative value indicates no timeout, i.e. an infinite wait
   static int exit_value, status;

   static UCommand* loadConfigCommand(UFileConfig* cfg);

   static void setTimeout(int seconds) { timeoutMS = (seconds * 1000); }

   static int32_t setEnvironment(const UString& env, char**& envp);
   static void   freeEnvironment(char** _envp, int32_t n) { UMemoryPool::_free(_envp, n + 1, sizeof(char*)); } // NB: we consider the null terminator...

   // run command

   bool executeWithFileArgument(UString* output, UFile* file);

   bool executeAndWait(UString* input = 0,                           int fd_stdin = -1, int fd_stderr = -1);
   bool execute(       UString* input = 0, UString* output = 0,      int fd_stdin = -1, int fd_stderr = -1);

   static UString outputCommand(const UString& cmd, char** envp = 0, int fd_stdin = -1, int fd_stderr = -1);

   bool checkForExecute(int mode = R_OK | X_OK)
      {
      U_TRACE(1, "UCommand::checkForExecute(%d)", mode)

      U_INTERNAL_ASSERT_POINTER(argv_exec)

      if (U_SYSCALL(access, "%S,%d", argv_exec[0], mode) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   // MANAGE MESSAGE ERROR

   static bool isStarted()
      {
      U_TRACE_NO_PARAM(0, "UCommand::isStarted()")

      if (pid > 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isTimeout()
      {
      U_TRACE_NO_PARAM(0, "UCommand::isTimeout()")

      if (exit_value == -EAGAIN) U_RETURN(true);

      U_RETURN(false);
      }

   static void printMsgError()
      {
      U_TRACE_NO_PARAM(0, "UCommand::printMsgError()")

      U_INTERNAL_ASSERT_MAJOR(u_buffer_len,0)

      U_WARNING("%.*s", u_buffer_len, u_buffer);

      u_buffer_len = 0;
      }

   static bool setMsgError(const char* cmd, bool only_if_error);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   char** envp;
   char* pathcmd;
   char** argv_exec;
   char** envp_exec;
   uint32_t flag_expand;
   int32_t ncmd, nenv, nfile;
   UString command, environment;

   void  setCommand();
   void freeCommand();
   void freeEnvironment();

   static void outputCommandWithDialog(const UString& cmd, char** envp, UString* output, int fd_stdin, int fd_stderr, bool dialog);

private:
          void setEnvironment(const UString& env) U_NO_EXPORT;
   inline void execute(bool flag_stdin, bool flag_stdout, bool flag_stderr) U_NO_EXPORT;

   static bool wait() U_NO_EXPORT;
   static bool postCommand(UString* input, UString* output) U_NO_EXPORT; // NB: (void*)-1 is a special value...
   static void setStdInOutErr(int fd_stdin, bool flag_stdin, bool flag_stdout, int fd_stderr) U_NO_EXPORT;

   U_DISALLOW_ASSIGN(UCommand)

   friend class UHTTP;
   friend class UDialog;
   friend class UServer_Base;
   friend class UProxyPlugIn;
   friend class UNoCatPlugIn;
};

#endif
