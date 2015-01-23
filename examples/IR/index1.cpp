// index1.cpp

#include <ulib/string.h>

#undef  PACKAGE
#define PACKAGE "index1"
#undef  ARGS
#define ARGS ""

#define U_OPTIONS \
"purpose 'index document files to be searched by query...'\n" \
"option c config 1 'path of configuration file' ''\n"

#include "IR.h"

class Application : public IR {
public:

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      IR::run(argc, argv, env);

      if (IR::openCDB(true, true))
         {
         IR::setBadWords();

         loadFilters();

         // process all filenames in argument DIRECTORY

      // operation = 0; // add

         IR::loadFiles();

         // save hash table as constant database

         if (cdb_names->writeTo(UPosting::tbl_name))
            {
            UPosting::tbl_words->setSpace(UPosting::tbl_words_space);

            if (cdb_words->writeTo(UPosting::tbl_words)) IR::deleteDB();
            }
         }
      }

private:
};

U_MAIN
