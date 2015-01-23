// test_json.cpp

#include <ulib/file.h>
#include <ulib/json/value.h>

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UValue json;
   UString filename, content, result;

   while (cin >> filename)
      {
      content = UFile::contentOf(filename);

      if (json.parse(content))
         {
         result.setBuffer(U_CAPACITY);

         UValue::stringify(result, json);

         cout << result << '\n';
         }
      }
}
