// test_compress.cpp

#include <ulib/file.h>
#include <ulib/utility/compress.h>
#include <ulib/utility/string_ext.h>

static void check(const UString& dati, const UString& file)
{
   U_INTERNAL_ASSERT( dati == UStringExt::decompress(UStringExt::compress(dati)) )

   char src[dati.size()];
   char dst[UCompress::space(dati.size())];

   size_t dst_len = UCompress::compress(U_STRING_TO_PARAM(dati), dst);
   size_t src_len = UCompress::decompress(dst, dst_len, src);

   U_INTERNAL_ASSERT( dati.size() == src_len );
   U_INTERNAL_ASSERT( memcmp(src, U_STRING_TO_PARAM(dati)) == 0 );

   int ratio = (dst_len * 100 / src_len);

   printf("%.*s - compression ratio (%d%%)\n", U_STRING_TO_TRACE(file), 100 - ratio);
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

      check(dati, filename);
      }
}
