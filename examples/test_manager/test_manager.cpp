// test_manager.cpp

#include <ulib/base/error.h>
#include <ulib/base/utility.h>

#include <ulib/process.h>
#include <ulib/file_config.h>
#include <ulib/utility/services.h>

#undef  PACKAGE
#define PACKAGE "test_manager"
#undef  ARGS
#define ARGS "<file_template_test>"

#define U_OPTIONS \
"purpose \"general manager for testing, read template test file specified with arg <file_template_test>...\"\n" \
"option l log  1 \"log file for data processing instead of stdout\" \"\"\n"

#include <ulib/application.h>

#ifdef HAVE_SYSEXITS_H
#  include <sysexits.h>
#else
#  include <ulib/base/replace/sysexits.h>
#endif

static pid_t pid;

class Application : public UApplication {
public:

   static void reset()
      {
      U_TRACE(5, "Application::reset()")

      if (pid > 0) UProcess::kill(pid, SIGTERM);
      }

   void print(int mode)
      {
      U_TRACE(5, "Application::print(%d)", mode)

      if (ulog)
         {
         iov[0].iov_len = buffer_len;

         ulog->write(iov, 1);

         if (mode)
            {
            char* ptr = (mode == 1 ? strstr(buffer, "TestID")
                                   : strrchr(buffer, '-') - 1);

            int len = strchr(ptr, '\n') - ptr + (mode == 2);

            (void) UFile::write(STDOUT_FILENO, ptr, len);
            }
         }
      else
         {
         (void) UFile::write(STDOUT_FILENO, buffer, buffer_len);
         }
      }

   void executeTests()
      {
      U_TRACE(5, "Application::executeTests()")

      bool pass;
      char** envp;
      unsigned output_len;
      char* envp_exec[128];
      char* argv_exec[128];
      uint32_t exit_status_len;
      char path[U_PATH_MAX + 1];
      int status, _exit_value, n;

      argv_exec[0] = argv_exec[1] = U_NULLPTR;

      UString id_key      = U_STRING_FROM_CONSTANT("ID"), id,
              env_key     = U_STRING_FROM_CONSTANT("ENVIRON"), env,
              command_key = U_STRING_FROM_CONSTANT("COMMAND"), command,
              result_key  = U_STRING_FROM_CONSTANT("RESULT"), result;

      do {
         command = template_file[command_key];

         if (command)
            {
            id     = template_file[id_key];
            result = template_file[result_key];

            buffer_len = u__snprintf(buffer, sizeof(buffer),
                               U_CONSTANT_TO_PARAM("\nStart TestID <%W%s%W>\n"
                               "----------------------------------"
                               "----------------------------------\n"),
                               YELLOW, id.c_str(), RESET);

            print(1);

            n = u_splitCommand(U_STRING_TO_PARAM(command), argv_exec, path, sizeof(path));

            if (n > 0)
               {
               U_INTERNAL_ASSERT_RANGE(1,n,127)

               env = template_file[env_key];

               if (env.empty()) envp = U_NULLPTR;
               else
                  {
                  n = u_split(U_STRING_TO_PARAM(env), envp_exec, U_NULLPTR);

                  U_INTERNAL_ASSERT_RANGE(1,n,127)

                  envp = envp_exec;
                  }

               if (ulog)
                  {
                  UProcess::pipe(STDOUT_FILENO); // UProcess::filedes[2] is for READING,
                                                 // UProcess::filedes[3] is for WRITING.

                  UProcess::filedes[4] = 0;
                  UProcess::filedes[5] = UProcess::filedes[3]; // stderr map on stdout...
                  }

               pid = UProcess::execute(argv_exec[0], argv_exec+1, envp, false, flag_stdout, flag_stderr);

               if (ulog)
                  {
                  // UProcess::filedes[3] for WRITING...

                  UFile::close(UProcess::filedes[3]);
                  }
               }

            if (pid <= 0)
               {
               if (ulog ||
                   n == -1)
                  {
                  buffer_len = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("command '%s' didn't start %R\n"),
                                             argv_exec[(n == -1 ? 1 : 0)], 0); // NB: the last argument (0) is necessary...

                  print(0);
                  }

               _exit_value = EX_UNAVAILABLE;
               }
            else
               {
               if (ulog)
                  {
                  if (output) output.setEmpty();

                  UServices::readEOF(UProcess::filedes[2], output);

                  output_len = output.size();

                  if (output_len)
                     {
                     struct iovec _iov[1] = { { (caddr_t)output.data(), output_len } };

                     ulog->write(_iov, 1);
                     }

                  UFile::close(UProcess::filedes[2]);
                  }

               UProcess::waitpid(pid, &status, 0);

               _exit_value = UProcess::exitValue(status);
               }

            if (u_isNumber(U_STRING_TO_PARAM(result))) pass = (_exit_value == result.strtol());
            else
               {
               u_getExitStatus(_exit_value, &exit_status_len);

               pass = (memcmp(result.data(), u_err_buffer, result.size()) == 0);
               }

            if (pass)
               {
               buffer_len = u__snprintf(buffer, sizeof(buffer),
                                        U_CONSTANT_TO_PARAM("----------------------------------"
                                        "----------------------------------\n"
                                        "End   TestID <%W%s%W> - %WPASS%W\n\n"),
                                        YELLOW, id.data(), RESET, GREEN, RESET);
               }
            else
               {
               buffer_len = u__snprintf(buffer, sizeof(buffer),
                                        U_CONSTANT_TO_PARAM("----------------------------------"
                                        "----------------------------------\n"
                                        "End   TestID <%W%s%W> - %WFAIL%W\n"
                                        "Expected exit code <%s>, returned <%d>\n\n"),
                                        YELLOW, id.data(), RESET, RED, RESET,
                                        result.c_str(), _exit_value);
               }

            print(2);

            pid = 0;
            }
         }
      while (template_file.loadSection(U_NULLPTR,0));
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage template test file

      if (argv[optind] == U_NULLPTR) U_ERROR("arg <file_template_test> not specified");

      template_file.load(UString(argv[optind], strlen(argv[optind])));

      // maybe manage logging...

      UString log_file = opt['l'];

      ulog = U_NULLPTR;

      if (log_file) U_NEW(ULog, ulog, ULog(log_file, 1024 * 1024));

      if (ulog)
         {
         flag_stdout = flag_stderr = true;

         iov[0].iov_base = (caddr_t)buffer;
         }
      else
         {
         flag_stdout = flag_stderr = false;
         }

      u_atexit(reset);

      executeTests(); // ciclo esecuzione test

      if (ulog) ulog->close();
      }

private:
   ULog* ulog;
   size_t buffer_len;
   struct iovec iov[1];
   UString data, output;
   UFileConfig template_file;
   char buffer[4096];
   bool flag_stdout, flag_stderr;
};

U_MAIN
