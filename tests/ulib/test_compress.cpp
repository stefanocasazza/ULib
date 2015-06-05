// test_compress.cpp

#include <ulib/file.h>
#include <ulib/utility/string_ext.h>

static void check(const UString& file)
{
   UString dati         = UFile::contentOf(file),
           compressed   = UStringExt::compress(dati),
           uncompressed = UStringExt::decompress(compressed);

   U_INTERNAL_ASSERT_EQUALS(dati, uncompressed)

   printf("%.*s - compression ratio (%d%%)\n", U_STRING_TO_TRACE(file), 100 - (dati.size() * 100 / compressed.size()));
}

int U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UString filename;

   while (cin >> filename) check(filename);
}
