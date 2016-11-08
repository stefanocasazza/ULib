// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    application.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_APPLICATION_H
#define ULIB_APPLICATION_H 1

#include <ulib/options.h>

#define U_PRINT_MEM_USAGE

#ifdef DEBUG
#define U_MAIN_END(value) return value
#  ifdef U_STDCPP_ENABLE
#     undef  U_PRINT_MEM_USAGE
#     define U_PRINT_MEM_USAGE UApplication::printMemUsage();
#  endif
#else
#define U_MAIN_END(value) ::exit(value)
#endif

#define U_MAIN \
int U_EXPORT main(int argc, char* argv[], char* env[]) \
{ \
   U_ULIB_INIT(argv); \
   U_TRACE(5, "::main(%d,%p,%p)", argc, argv, env) \
   Application application; \
   application.run(argc, argv, env); \
   U_PRINT_MEM_USAGE \
   U_MAIN_END(UApplication::exit_value); \
}

/*
#define U_MAIN(_class) \
int WINAPI WinMain (HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR command_line, int cmd_show) \
{ \
   U_ULIB_INIT(__argv); \
   int argc = 0; \
   char** _argv = __argv; \
   while (*_argv++) ++argc; \
   U_TRACE(5, "::main(%d,%p,%p)", argc, __argv, 0) \
   _class application; \
   application.run(argc, argv, 0); \
   U_MAIN_END(UApplication::exit_value); \
}
*/

class Application;

class U_EXPORT UApplication {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   static int exit_value;
   static uint32_t num_args;

   UString str; // NB: must be here to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
   UOptions opt;

   UApplication();

#ifdef U_COVERITY_FALSE_POSITIVE
   virtual ~UApplication();
#else
           ~UApplication();
#endif

   // SERVICES

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(0+256, "UApplication::run(%d,%p,%p)", argc, argv, env)

      U_DUMP_EXEC(argv, env)

      // if exist options, these are processed...

      is_options = (argc > 1);

      if (is_options)
         {
#     ifdef PACKAGE
         opt.package = U_STRING_FROM_CONSTANT(PACKAGE);
#     endif
#     ifdef VERSION
         opt.version = U_STRING_FROM_CONSTANT(VERSION);
#     else
         opt.version = U_STRING_FROM_CONSTANT(ULIB_VERSION);
#     endif
#     ifdef PURPOSE
         opt.purpose = U_STRING_FROM_CONSTANT(PURPOSE);
#     endif
#     ifdef ARGS
         opt.args = U_STRING_FROM_CONSTANT(ARGS);
#     endif
#     ifdef REPORT_BUGS
         opt.report_bugs = U_STRING_FROM_CONSTANT(REPORT_BUGS);
#     endif

#     ifdef U_OPTIONS
#        ifndef U_OPTIONS_1
#        define U_OPTIONS_1 ""
#        endif
#        ifndef U_OPTIONS_2
#        define U_OPTIONS_2 ""
#        endif
#        ifndef U_OPTIONS_3
#        define U_OPTIONS_3 ""
#        endif

         str = UString(U_CONSTANT_TO_PARAM(U_OPTIONS U_OPTIONS_1 U_OPTIONS_2 U_OPTIONS_3));

         if (str) opt.load(str);
#     endif

         num_args = opt.getopt(argc, argv, &optind);
         }
      }

   static bool isOptions()
      {
      U_TRACE_NO_PARAM(0, "UApplication::isOptions()")

      U_RETURN(is_options);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   static void printMemUsage();

   const char* dump(bool reset) const;
#endif

protected:
   static bool is_options;

   void usage() { opt.printHelp(0); }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UApplication)

   friend class Application;
};

#endif
