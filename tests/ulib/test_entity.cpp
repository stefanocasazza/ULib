// test_entity.cpp

#include <ulib/file.h>
#include <ulib/mime/entity.h>

#ifdef USE_LIBSSL
#  include <ulib/utility/services.h>
#  include <ulib/ssl/mime/mime_pkcs7.h>
#endif

static void check_content(const UString& content, const UString& filename)
{
   U_TRACE(5, "check_content(%p,%p)", content.data(), filename.data())

   UString dati = UFile::contentOf(filename);

   if (dati != content)
      {
      UFile save_file;

      if (save_file.creat(U_STRING_FROM_CONSTANT("tmp/content.different")))
         {
         save_file.write(content);

         save_file.close();
         }
      }

   U_ASSERT( dati == content )
}

static void check_multipart(UMimeMultipart& item, const UString& file)
{
   U_TRACE(5, "check_multipart(%p,%p)", &item, file.data())

   if (file == U_STRING_FROM_CONSTANT("./inp/multipart.eml") ||
       file == U_STRING_FROM_CONSTANT("./inp/multipart_dos.eml") )
      {
      UVector<UMimeEntity*>& bodypart = item.getBodyPart();

      UMimeEntity* item_xml  = bodypart[0];
      UMimeEntity* item_zip  = bodypart[1];
      UMimeEntity* item_time = bodypart[2];

      U_ASSERT(item_xml->getFileName()  == U_STRING_FROM_CONSTANT("t-archive.xml") )
      U_ASSERT(item_zip->getFileName()  == U_STRING_FROM_CONSTANT("archive.zip") )
      U_ASSERT(item_time->getFileName() == U_STRING_FROM_CONSTANT("timestamp.pk7") )

      check_content(item_xml->getContent(),  U_STRING_FROM_CONSTANT("inp/t-archive.xml"));
      check_content(item_zip->getContent(),  U_STRING_FROM_CONSTANT("inp/archive.zip"));
      check_content(item_time->getContent(), U_STRING_FROM_CONSTANT("inp/timestamp.pk7"));
      }
}

#ifdef USE_LIBSSL
static void check_pkcs7(UMimePKCS7& item, const UString& file)
{
   U_TRACE(5, "check_pkcs7(%p,%p)", &item, file.data())

   if (file == U_STRING_FROM_CONSTANT("./inp/pkcs7.eml"))
      {
      check_content(item.getContent(), U_STRING_FROM_CONSTANT("inp/pkcs7.out"));

      bool ok = item.getPKCS7().verify();

      U_ASSERT(ok)
      }
}
#endif

static void parse(const UString& dati, const UString& file)
{
   U_TRACE(5, "parse(%p.%p)", dati.data(), file.data())

   UMimeEntity tmp(dati);
   const char* type = "entity";

   U_ASSERT(tmp.isMime())
   U_ASSERT(tmp.getCharSet().equal(U_CONSTANT_TO_PARAM("us-ascii")))

   if (tmp.isMessage())
      {
      type = "message";

      UMimeMessage item(tmp);
      cout << item << "\n";
      }
   else if (tmp.isMultipart())
      {
      type = "multipart";

      UMimeMultipart item(dati);
      cout << item << "\n";

      check_multipart(item, file);
      }
   else if (tmp.isApplication())
      {
      type = "application";

      if (tmp.isPKCS7())
         {
         type = "application/pkcs7";

#     ifdef USE_LIBSSL
         UMimePKCS7 item(tmp);

         /*
         if (item.isValid()) cout << item << "\n";
         else
            {
            UServices::setOpenSSLError();

            if (u_buffer_len)
               {
               cout.write(u_buffer, u_buffer_len);

               cout << "\n";

               u_buffer_len = 0;
               }
            }
         */

         check_pkcs7(item, file);
#     endif
         }
      }

   printf("%.*s parsed as %s \n", U_STRING_TO_TRACE(file), type);
}

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UString filename, dati;

   while (cin >> filename)
      {
      dati = UFile::contentOf(filename);

      parse(dati, filename);
      }
}
