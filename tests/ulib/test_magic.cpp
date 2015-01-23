// test_magic.cpp

#include <ulib/file.h>
#include <ulib/magic/magic.h>

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UMagic x(MAGIC_MIME);
   UString filename, dati;

   while (cin >> filename)
      {
      dati = UFile::contentOf(filename);

      x.setFlags(MAGIC_NONE);
      cout << x.getType(dati) << "\n";

      x.setFlags(MAGIC_MIME);
      cout << x.getType(dati) << "\n";
      }

   // exit(0);
}
