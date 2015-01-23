// error.cpp

#include <ulib/string.h>

#undef  PACKAGE
#define PACKAGE "error"
#undef  ARGS
#define ARGS ""

#define U_OPTIONS \
"purpose \"program for workflow error...\"\n" \
"option c config 1 \"path of configuration file\" \"\"\n"

#include "action.h"

class Application : public Action {
public:

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      Action::run(argc, argv, env);

      // read stdin and scanf
      // ----------------------------------------
      //  wfeng:id="%*[^+]+%*[^+]+%[^"]"
      // <wfeng:data wfeng:error="event=%[^:]:state=%[^:]:action-name=%[^:]:error-message=%[^"]">%[^<]</wfeng:data>
      // ----------------------------------------

      Action::processInputData(6);

      Action::writeToSTDOUT(Action::sendEmail(), false);
      }
};

U_MAIN
