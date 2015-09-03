// hello.cpp

#include <ulib/string.h>

#undef  PACKAGE
#define PACKAGE "hello"

#define ARGS "[name]"

#define U_OPTIONS \
"purpose 'simple application demo'\n" \
"option n name 1 'The name of the friend to say hello' ''\n"

#include <ulib/application.h>

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE(5, "Application::Application()")
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage options

      UString name;

      if (UApplication::isOptions()) name = opt['n'];

      // manage arg operation

      if (name.empty() &&
          argv[optind])
         {
         name = argv[optind];
         }

      if (name.empty())
         {
         UApplication::exit_value = 1;

         U_ERROR("I don't know the name");
         }

      UApplication::exit_value = 0;

      U_MESSAGE("Hello %v", name.rep);
      }
};

U_MAIN
