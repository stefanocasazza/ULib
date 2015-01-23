// test_timestamp.cpp

#include <ulib/file.h>
#include <ulib/ssl/timestamp.h>

static void check(const UString& dati)
{
   U_TRACE(5, "check(%p)", dati.data())

   UTimeStamp item(dati);

   cout << item.isValid() << "\n";
}

int
U_EXPORT main (int argc, char* argv[])
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
