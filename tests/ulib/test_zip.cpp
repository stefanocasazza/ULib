// test_zip.cpp

#include <ulib/file.h>
#include <ulib/zip/zip.h>

#include <iostream>

static void print(UZIP& zip)
{
   U_TRACE(5, "print(%p)", &zip)

   UString content, filename;
   int count = zip.getFilesCount();

   for (int i = 0; i < count; ++i)
      {
      content  = zip.getFileContentAt(i);
      filename = zip.getFilenameAt(i);

      cout << "Parte " << i+1 << ": Filename='" << filename << "'" << endl;
      }
}

static void check(const UString& content)
{
   U_TRACE(5, "check(%p)", content.data())

   UZIP zip(content);

   if (zip.isValid())
      {
      if (zip.readContent())
         {
         print(zip);

         zip.clear();
         }

      if (zip.extract()) print(zip);
      }
}

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)", argc)

   UString filename;

   while (cin >> filename)
      {
      UString dati = UFile::contentOf(filename);

      check(dati);
      }
}
