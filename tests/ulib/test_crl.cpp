// test_crl.cpp

#include <ulib/file.h>
#include <ulib/ssl/crl.h>
#include <ulib/ssl/certificate.h>

#include <iostream>

static void check(const UString& dati_crl, const UString& dati_ca)
{
   U_TRACE(5,"check(%p,%p)", &dati_crl, &dati_ca)

// long revoked[10];
   UCrl c(dati_crl);
   UCertificate ca(dati_ca);

   cout << c                                 << "\n"
        << c.isUpToDate()                    << "\n"
        << c.getIssuer()                     << "\n"
        << c.isIssued(ca)                    << "\n"
        << c.getVersionNumber()              << "\n"
        << c.getLastUpdate()                 << "\n"
   //   << c.getRevokedSerials(revoked, 10)  << "\n"
   //   << c.getSignature()                  << "\n"
        << c.getNextUpdate();

   UString encoded = c.getEncoded("PEM");

   /*
   UFile::writeTo("crl.encode", encoded);

   U_ASSERT( dati_crl == encoded )
   */
}

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)", argc)

   UString filename_crl, filename_ca;

   while (cin >> filename_crl &&
          cin >> filename_ca)
      {
      UString dati_crl = UFile::contentOf(filename_crl),
              dati_ca  = UFile::contentOf(filename_ca);

      check(dati_crl, dati_ca);
      }
}
