// query.cpp

#include <ulib/query/parser.h>

#undef  PACKAGE
#define PACKAGE "query"
#undef  ARGS
#define ARGS "<query>"

#define U_OPTIONS \
"purpose 'search in index database of document files...'\n" \
"option c config 1 'path of configuration file' ''\n"

#include "cquery.h"

class Application : public IR {
public:

   Application()
      {
      U_TRACE(5, "Application::Application()")

      query = U_NULLPTR;
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      if (query)
         {
         delete posting;
                posting = U_NULLPTR;

         delete query;
         }
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      IR::run(argc, argv, env);

      // manage arg operation

      const char* ptr = argv[optind];

      U_INTERNAL_DUMP("optind = %d", optind)

      if (ptr == U_NULLPTR) U_ERROR("<query> not specified");

      UString::str_allocate(STR_ALLOCATE_QUERY_PARSER);

      if (IR::openCDB(false))
         {
         uint32_t len;

         U_NEW(Query, query, Query);

         ptr = Query::checkQuoting(argv, len);

         query->run(ptr, len, U_NULLPTR);

         WeightWord::dumpObjects();

         IR::deleteDB();
         }
      }

private:
   Query* query; // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
};

U_MAIN
