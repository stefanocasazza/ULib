// index2.cpp

#include <ulib/string.h>

#undef  PACKAGE
#define PACKAGE "index2"
#undef  ARGS
#define ARGS ""

#define U_OPTIONS \
"purpose 'index document files to be searched by query...'\n" \
"option c config 1 'path of configuration file' ''\n"

#define U_CDB_CLASS URDB
#define U_RDB_OPEN_WORDS (uint32_t)(cfg_dimension * 2048U)
#define U_RDB_OPEN_NAMES (uint32_t)(cfg_dimension *  128U)

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

         // process all filenames in argument DIRECTORY

      // operation = 0; // add

         IR::loadFiles();

         // register to constant database (CDB)

         IR::closeCDB(false);
         IR::deleteDB();
         }
      }

private:
};

U_MAIN
