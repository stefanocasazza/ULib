// test_pkcs10.cpp

#include <ulib/file.h>
#include <ulib/ssl/pkcs10.h>

#include <iostream>

static EVP_PKEY* CAPublicKey;

static void check(UString& dati, const char* file)
{
   U_TRACE(5,"check(%p,%S)", dati.data(), file)

   UPKCS10 c(dati);

   cout << c                        << "\n"
        << c.getSubject()           << "\n"
        << c.getVersionNumber()     << "\n"           
   //   << c.getSignature()         << "\n" 
   //   << c.getSignable()          << "\n" 
        << c.getSignatureAlgorithm();

   UString encoded = c.getEncoded("PEM");

   /*
   UFile save_file;

   if (save_file.creat("pkcs10.encode"))
      {
      save_file.write(encoded, true);
      save_file.close();
      }

   U_ASSERT( dati == encoded )
   */

   if (CAPublicKey)
      {
      cout << "verify() = " << c.verify(CAPublicKey)  << "\n";
      }

   CAPublicKey = c.getSubjectPublicKey();
}

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)", argc)

   UString filename;

   while (cin >> filename)
      {
      UString dati = UFile::contentOf(filename);

      check(dati, filename.c_str());
      }
}
