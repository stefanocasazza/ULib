// test_expat.cpp

#include <ulib/file.h>
#include <ulib/xml/expat/xml_parser.h>

static void check(const UString& data)
{
   U_TRACE(5,"check(%.*S)",U_STRING_TO_TRACE(data))

   UXMLParser parser;

   parser.initParser();

   if (parser.parse(data)) cout << "Parser\n";
}

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UString filename;

   while (cin >> filename)
      {
      UString dati = UFile::contentOf(filename);

      check(dati);
      }
}
