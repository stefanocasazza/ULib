// test_certificate.cpp

#include <ulib/file.h>
#include <ulib/ssl/certificate.h>
#include <ulib/container/vector.h>

#include <iostream>

static void check(const UString& dati_cert, const UString& dati_ca)
{
   U_TRACE(5,"check(%p,%p)", &dati_cert, &dati_ca)

   UCertificate c(dati_cert);
   UCertificate ca(dati_ca);

   UVector<UString> vec1, vec2;
   (void) c.getCAIssuers(vec1);
   (void) c.getRevocationURL(vec2);

   cout << c                        << '\n'
        << c.isSelfSigned()         << '\n'
        << c.isIssued(ca)           << '\n'
        << c.getIssuer()            << '\n' 
   //   << c.getIssuerForLDAP()     << '\n' 
        << c.getSubject()           << '\n'
        << c.getVersionNumber()     << '\n'           
        << c.getSerialNumber()      << '\n' 
        << c.hashCode()             << '\n' 
        << c.getSignatureAlgorithm()<< '\n' 
        << c.getNotBefore()         << '\n' 
        << c.getNotAfter()          << '\n' 
        << c.checkValidity()        << '\n'
   //   << c.getSignature()         << '\n' 
   //   << c.getSignable()          << '\n' 
        << vec1                     << '\n'
        << vec2                     << '\n';

   UString encoded = c.getEncoded("PEM");

   /*
   UFile::writeTo("certificate.encode", encoded);

   U_ASSERT( dati_cert == encoded )
   */
}

int U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)", argc)

   UString filename_cert, filename_ca;

   while (cin >> filename_cert &&
          cin >> filename_ca)
      {
      UString dati_cert = UFile::contentOf(filename_cert),
              dati_ca   = UFile::contentOf(filename_ca);

      check(dati_cert, dati_ca);
      }
}
