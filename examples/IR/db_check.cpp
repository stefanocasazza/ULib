// db_check.cpp

#include <ulib/string.h>

#undef  PACKAGE
#define PACKAGE "db_check"
#undef  ARGS
#define ARGS ""

#define U_OPTIONS \
"purpose 'check database of index documents files...'\n" \
"option c config 1 'path of configuration file' ''\n"

#include "IR.h"

class Application : public IR {
public:

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      IR::run(argc, argv, env);

      if (IR::openCDB(true))
         {
         IR::setBadWords();

         loadFilters();

         operation = 3; // check

         IR::loadFiles();

         if (UPosting::dir_content_as_doc == false)
            {
            (void) write(1, U_CONSTANT_TO_PARAM("\nCHECK_2"));

            UPosting::checkAllEntry();

            (void) write(1, U_CONSTANT_TO_PARAM("OK\n"));
            }

         IR::deleteDB();
         }
      }

private:
};

U_MAIN
