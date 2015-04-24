// test_base64.cpp

#include <ulib/file.h>
#include <ulib/utility/base64.h>

#include <iostream>

static void check(const UString& dati, const UString& file)
{
   U_TRACE(5,"check(%p,%p)", dati.data(), file.data())

   uint32_t sz = dati.size() * 4;

   UString buffer1(sz), buffer2(sz);

   /*
   UBase64::decode(U_STRING_TO_PARAM(dati), buffer1);
   U_INTERNAL_DUMP("buffer1 = %#.*S", U_STRING_TO_TRACE(buffer1))
   exit(0);
   */

   UBase64::encode(U_STRING_TO_PARAM(dati),    buffer1);
   UBase64::decode(U_STRING_TO_PARAM(buffer1), buffer2);

   U_INTERNAL_DUMP("buffer1 = %#.*S", U_STRING_TO_TRACE(buffer1))
   U_INTERNAL_DUMP("dati    = %#.*S", U_STRING_TO_TRACE(dati))
   U_INTERNAL_DUMP("buffer2 = %#.*S", U_STRING_TO_TRACE(buffer2))

   /*
   UFile save_file;

   if (save_file.creat("base64.encode"))
      {
      save_file.write(buffer1, true);
      save_file.close();
      }

   if (save_file.creat("base64.decode"))
      {
      save_file.write(buffer2, true);
      save_file.close();
      }
   */

   U_ASSERT( dati == buffer2 )

   UBase64::encodeUrl(U_STRING_TO_PARAM(dati),    buffer1);
   UBase64::decodeUrl(U_STRING_TO_PARAM(buffer1), buffer2);

   U_INTERNAL_DUMP("buffer1 = %#.*S", U_STRING_TO_TRACE(buffer1))
   U_INTERNAL_DUMP("dati    = %#.*S", U_STRING_TO_TRACE(dati))
   U_INTERNAL_DUMP("buffer2 = %#.*S", U_STRING_TO_TRACE(buffer2))

   U_ASSERT( dati == buffer2 )
}

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)", argc)

   UString filename;

   while (cin >> filename)
      {
      UString dati = UFile::contentOf(filename);

      check(dati, filename);
      }
}
