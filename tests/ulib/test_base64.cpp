// test_base64.cpp

#include <ulib/file.h>
#include <ulib/utility/base64.h>

#include <iostream>

static void check(const UString& dati, const UString& file)
{
   U_TRACE(5,"check(%p,%p)", dati.data(), file.data())

   uint32_t sz = dati.size() * 4;

   UString buffer1(sz), buffer2(sz);

   u_base64_max_columns = U_OPENSSL_BASE64_MAX_COLUMN;

   UBase64::encode(U_STRING_TO_PARAM(dati), buffer1);

   u_base64_max_columns = 0;

   UBase64::decode(U_STRING_TO_PARAM(buffer1), buffer2);

   U_ASSERT_EQUALS(dati, buffer2)
   U_INTERNAL_ASSERT_EQUALS(u_base64_errors, 0)

   UBase64::decodeAll(U_STRING_TO_PARAM(buffer1), buffer2);

   U_ASSERT_EQUALS(dati, buffer2)
   U_INTERNAL_ASSERT_EQUALS(u_base64_errors, 0)

   UBase64::encodeUrl(U_STRING_TO_PARAM(dati), buffer1);

   U_INTERNAL_ASSERT_EQUALS(u_base64_errors, 0)

   UBase64::decodeUrl(U_STRING_TO_PARAM(buffer1), buffer2);

   U_ASSERT_EQUALS(dati, buffer2)
   U_INTERNAL_ASSERT_EQUALS(u_base64_errors, 0)

   UBase64::decodeAll(U_STRING_TO_PARAM(buffer1), buffer2);

   U_INTERNAL_DUMP("buffer1 = %V", buffer1.rep)
   U_INTERNAL_DUMP("   dati = %V",    dati.rep)
   U_INTERNAL_DUMP("buffer2 = %V", buffer2.rep)

   U_ASSERT_EQUALS(dati, buffer2)
   U_INTERNAL_ASSERT_EQUALS(u_base64_errors, 0)
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
