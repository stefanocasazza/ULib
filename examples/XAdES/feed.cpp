// feed.cpp

#include <ulib/zip/zip.h>
#include <ulib/ssl/crl.h>
#include <ulib/curl/curl.h>
#include <ulib/ssl/pkcs7.h>
#include <ulib/file_config.h>
#include <ulib/ssl/certificate.h>
#include <ulib/net/client/http.h>
#include <ulib/ssl/net/sslsocket.h>
#include <ulib/utility/string_ext.h>

#ifdef HAVE_LDAP
#  include <ulib/ldap/ldap.h>
#endif

#undef  PACKAGE
#define PACKAGE "feed"

#define ARGS "[ URI public certificate list ]"

#define U_OPTIONS \
"purpose \"feed CA storage with public certificate list\"\n" \
"option c config 1 \"path of configuration file\" \"\"\n"

#include <ulib/application.h>

#define U_ZIPPONE (const char*)(num_args >= 0  ? argv[optind+0]  : 0)

// --------------------------------------------------------------
// http://www.cnipa.gov.it/site/_files/LISTACER_20100907.zip.p7m
// --------------------------------------------------------------
// La lista dei certificati, completata con le informazioni previste
// dal DPCM 30 marzo 2009 sopra citato, è strutturata in un archivio
// WINZIP non compresso, come insieme di directory, ciascuna dedicata
// ad un certificatore iscritto, ognuna contenente i certificati forniti
// dalle aziende (in formato binario DER ed ove fornito in formato B64)
// ed un file in formato Rich Text Format che presenta le informazioni
// previste dal più volte citato articolo 41.
// --------------------------------------------------------------

class Application : public UApplication {
public:

   Application() : client(0)
      {
      U_TRACE(5, "Application::Application()")

      num_args = 0;
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")
      }

   bool download(bool bcrl)
      {
      U_TRACE(5, "Application::download(%b)", bcrl)

      if (u_isURL(U_STRING_TO_PARAM(uri)) == false) U_RETURN(false);

      output.clear();

      if (u_startsWith(U_STRING_TO_PARAM(uri), U_CONSTANT_TO_PARAM("http")))
         {
         if (client.connectServer(uri) &&
             client.sendRequest())
            {
            output = client.getContent();
            }

         client.reset();
         }
      else if (u_startsWith(U_STRING_TO_PARAM(uri), U_CONSTANT_TO_PARAM("ldap")))
         {
#     ifdef HAVE_LDAP
         if (ldap.init(uri.c_str()) &&
             ldap.set_protocol()    &&
             ldap.bind())
            {
            int n = ldap.search();

            if (n > 0)
               {
               static const char*  ca_attr_name[] = { "cACertificate",             "crossCertificatePair",    0 };
               static const char* crl_attr_name[] = { "certificateRevocationList", "authorityRevocationList", 0 };

               ULDAPEntry entry(2, (bcrl ? crl_attr_name : ca_attr_name), n);

               ldap.get(entry);

               for (int i = 0; i < n; ++i)
                  {
                  output = entry.getString(0, i);

                  if (output.empty() == false) break;
                  }
               }
            }
#     else
         curl.setURL(uri.c_str());

         if (curl.performWait())
            {
            UString tmp = curl.getResponse();

            // Example of output
            //
            // DN: ou=Actalis - Firma Digitale,o=CSP
            //    certificateRevocationList;binary:: MIIHHDCCBgQCAQEwDQYJKoZIhvcNAQELBQAwcjELMAkGA1UEBhMCSVQxFz....
            //
            //

            uint32_t pos = U_STRING_FIND(tmp, 0, "\n\t");

            if (pos == U_NOT_FOUND) U_RETURN(false);

            pos += 2;

            pos = U_STRING_FIND(tmp, pos, ": ");

            if (pos == U_NOT_FOUND) U_RETURN(false);

            pos += 2;

            output = UStringExt::trim(tmp.c_pointer(pos), tmp.size() - pos);
            }
#     endif
         }
      else
         {
         curl.setURL(uri.c_str());

         if (curl.performWait()) output = curl.getResponse();
         }

      bool result = (output.empty() == false);

      if (result == false)
         {
         // U_ERROR(  "failed to download URI: %.*s", U_STRING_TO_TRACE(uri));
            U_WARNING("failed to download URI: %.*s", U_STRING_TO_TRACE(uri));
         }

      U_RETURN(result);
      }

   bool manageCRL()
      {
      U_TRACE(5, "Application::manageCRL()")

      if (download(true) &&
          crl.set(output))
         {
         UString hash(100U);
         long hash_code = crl.hashCode();

         hash.snprintf("%08x.r0", hash_code);

         U_DUMP("hash = %.*S exist = %b", U_STRING_TO_TRACE(hash), UFile::access(hash.data(), R_OK))

         if (UFile::writeTo(hash, crl.getEncoded("PEM"))) U_RETURN(true);
         }

      U_RETURN(false);
      }

   void writeCertificate()
      {
      U_TRACE(5, "Application::writeCertificate()")

      // Link a certificate to its subject name hash value, each hash is of
      // the form <hash>.<n> where n is an integer. If the hash value already exists
      // then we need to up the value of n, unless its a duplicate in which
      // case we skip the link. We check for duplicates by comparing the
      // certificate fingerprints

      UString hash(100U);
      long hash_code = cert.hashCode();

      hash.snprintf("%08x.0", hash_code);

      bool exist = UFile::access(hash.data(), R_OK);

      U_INTERNAL_DUMP("hash = %.*S exist = %b", U_STRING_TO_TRACE(hash), exist)

      if (exist == false) (void) UFile::writeTo(hash, cert.getEncoded("PEM"));
      }

   void manageCertificate()
      {
      U_TRACE(5, "Application::manageCertificate()")

      if (download(false) &&
          cert.set(output))
         {
         writeCertificate();
         }
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage options

      num_args = (argc - optind);

      U_INTERNAL_DUMP("optind = %d num_args = %d", optind, num_args)

      UString cfg_str;
      UFileConfig cfg;

      if (UApplication::isOptions()) cfg_str = opt['c'];

      // manage file configuration

      if (cfg_str.empty()) cfg_str = U_STRING_FROM_CONSTANT("XAdES.ini");

      // ----------------------------------------------------------------------------------------------------------------------------------
      // XAdES - configuration parameters
      // ----------------------------------------------------------------------------------------------------------------------------------
      // DigestAlgorithm   md2 | md5 | sha | sha1 | sha224 | sha256 | sha384 | sha512 | mdc2 | ripmed160
      //
      // SigningTime this property contains the time at which the signer claims to have performed the signing process (yes/no)
      // ClaimedRole this property contains claimed or certified roles assumed by the signer in creating the signature
      //
      // this property contains the indication of the purported place where the signer claims to have produced the signature
      // -------------------------------------------------------------------------------------------------------------------
      // ProductionPlaceCity
      // ProductionPlaceStateOrProvince
      // ProductionPlacePostalCode
      // ProductionPlaceCountryName
      // -------------------------------------------------------------------------------------------------------------------
      //
      // DataObjectFormatMimeType   this property identifies the format of a signed data object (when electronic signatures
      //                            are not exchanged in a restricted context) to enable the presentation to the verifier or
      //                            use by the verifier (text, sound or video) in exactly the same way as intended by the signer
      //
      // CAStore                    the location of CA and CRL
      // UriPublicListCerticate     the uri of Public Certificate List
      // ArchiveTimeStamp           the time-stamp token within this property covers the archive validation data
      //
      // SignatureTimeStamp         the time-stamp token within this property covers the digital signature value element
      // Schema                     the pathname XML Schema of XAdES
      // ----------------------------------------------------------------------------------------------------------------------------------

      cfg.UFile::setPath(cfg_str);

      UString ca_store = cfg[U_STRING_FROM_CONSTANT("XAdES-C.CAStore")];

      if (ca_store.empty()) U_ERROR("XAdES - CAStore is empty");

      if (UFile::chdir(ca_store.c_str(), true) == false) U_ERROR("XAdES - chdir() on CAStore %S failed", ca_store.data());

      // manage arguments...

      uri = ( U_ZIPPONE == 0 ||
             *U_ZIPPONE == '\0'
               ? cfg[U_STRING_FROM_CONSTANT("XAdES-C.UriPublicListCerticate")]
               : UString(U_ZIPPONE));

      if (uri.empty()) U_ERROR("XAdES - UriPublicListCerticate is empty");

      curl.setInsecure();
      curl.setTimeout(60);

      client.reserve(512U * 1024U);
      client.setFollowRedirects(true);
      client.getResponseHeader()->setIgnoreCase(true);

      if (download(false))
         {
         UPKCS7 zippone(output, "DER");

         if (zippone.isValid() == false) U_ERROR("Error reading S/MIME Public Certificate List, may be the file is not signed");

         UZIP zip(zippone.getContent());

         if (zip.readContent() == false) U_ERROR("Error reading ZIP Public Certificate List, may be the file is not zipped");

         UString namefile;
         UVector<UString> vec_ca(5U), vec_crl(5U);
         uint32_t i, j, k, n = zip.getFilesCount();

         U_INTERNAL_DUMP("ZIP: %d parts", n)

         for (i = 0; i < n; ++i)
            {
            namefile = zip.getFilenameAt(i);

            U_INTERNAL_DUMP("Part %d: Filename=%.*S", i+1, U_STRING_TO_TRACE(namefile))

            // .cer .crt .der

            if ((UStringExt::endsWith(namefile, U_CONSTANT_TO_PARAM(".cer"))  ||
                 UStringExt::endsWith(namefile, U_CONSTANT_TO_PARAM(".crt"))  ||
                 UStringExt::endsWith(namefile, U_CONSTANT_TO_PARAM(".der"))) &&
                cert.set(zip.getFileContentAt(i)))
               {
               writeCertificate();

               for (j = 0, k = cert.getCAIssuers(vec_ca); j < k; ++j)
                  {
                  uri = vec_ca[j];

                  U_INTERNAL_DUMP("uri(CA) = %.*S", U_STRING_TO_TRACE(uri))

                  manageCertificate();
                  }

               vec_ca.clear();

               for (j = 0, k = cert.getRevocationURL(vec_crl); j < k; ++j)
                  {
                  uri = vec_crl[j];

                  U_INTERNAL_DUMP("uri(CRL) = %.*S", U_STRING_TO_TRACE(uri))

                  if (manageCRL()) break;
                  }

               vec_crl.clear();
               }
            }
         }

      (void) UFile::chdir(0, true);
      }

private:
   int num_args;
   UHttpClient<USSLSocket> client; // NB: must be here to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#ifdef HAVE_LDAP
   ULDAP ldap;
#endif
   UCrl crl;
   UCURL curl;
   UCertificate cert;
   UString uri, output;
};

U_MAIN
