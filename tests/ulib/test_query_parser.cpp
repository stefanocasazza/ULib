// test_query_parser.cpp

#include <ulib/query/parser.h>

static bool eval(UStringRep* word)
{
   U_TRACE(5,"eval(%.*S)", U_STRING_TO_TRACE(*word))

   return false;
}

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)", argc)

   UString::str_allocate(STR_ALLOCATE_QUERY_PARSER);

   UString query;
   uint32_t i, sz;
   UQueryNode* term;
   UQueryParser parser;
   UVector<UQueryNode*> termRoots;
   UVector<UString> positives, negatives;

   while (query.getline(cin) &&
          parser.parse(query))
      {
      cout << "Original expression     : " << query  << "\n"
           << "Disjunctive normal form : " << parser << "\n";

      parser.getTree()->getDNFTermRoots(&termRoots);

      for (i = 0, sz = termRoots.size(); i < sz; ++i)
         {
         term = termRoots[i];

         term->getTreeVariables(&positives, &negatives);

         cout << "Term       : " << *term << "\n"
              << "  Positives: " << positives << "\n"
              << "  Negatives: " << negatives << "\n";

         positives.clear();
         negatives.clear();
         }

      parser.startEvaluate(eval);

      cout << "Evaluate   : " << (parser.evaluate() ? "true" : "false")
           << "\n--------------------------------------------------------------------------------------\n";

          query.clear();
         parser.clear();
      termRoots.clear();
      }
}
