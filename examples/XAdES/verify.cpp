// verify.cpp

#include <ulib/file_config.h>
#include <ulib/base/ssl/dgst.h>
#include <ulib/utility/base64.h>
#include <ulib/ssl/certificate.h>
#include <ulib/utility/services.h>
#include <ulib/utility/string_ext.h>
#include <ulib/xml/libxml2/schema.h>

#ifdef HAVE_SSL_TS
#  include <ulib/ssl/timestamp.h>
#endif

#include "utility.h"
#include "context.h"

#undef  PACKAGE
#define PACKAGE "verify"

#define ARGS "[ SCHEMA ]"

#define U_OPTIONS \
"purpose \"XAdES verify\"\n" \
"option c config 1 \"path of configuration file\" \"\"\n"

#include <ulib/application.h>

#define U_SCHEMA               (const char*)(num_args >= 0 ? argv[optind+0] : 0)
#define U_TAG_SIGNED_INFO      "ds:SignedInfo"
#define U_TAG_X509_CERTIFICATE "ds:X509Certificate"

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE(5, "Application::Application()")

      alg = num_args = 0;
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      U_SYSCALL_VOID_NO_PARAM(xmlCleanupParser);
      U_SYSCALL_VOID_NO_PARAM(xmlMemoryDump);
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      U_SYSCALL_VOID_NO_PARAM(xmlInitParser); // init libxml

      LIBXML_TEST_VERSION

      // manage options

      num_args = (argc - optind);

      U_INTERNAL_DUMP("optind = %d num_args = %d", optind, num_args)

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
      // CAStore
      // ArchiveTimeStamp           the time-stamp token within this property covers the archive validation data
      //
      // SignatureTimeStamp         the time-stamp token within this property covers the digital signature value element
      // Schema                     the pathname XML Schema of XAdES
      // ----------------------------------------------------------------------------------------------------------------------------------

      cfg.UFile::setPath(cfg_str);

      UString x(U_CAPACITY);

      UServices::readEOF(STDIN_FILENO, x);

      if (x.empty()) U_ERROR("cannot read data from <stdin>");

      (void) content.reserve(x.size());

      if (UBase64::decode(x, content) == false) U_ERROR("decoding data read failed");

      // manage arguments...

      schema = ( U_SCHEMA == 0 ||
                *U_SCHEMA == '\0'
                  ? cfg[U_STRING_FROM_CONSTANT("XAdES-L.Schema")]
                  : UString(U_SCHEMA));

      if (schema.empty()) U_ERROR("error on XAdES schema: empty");

      /*
      UString str_CApath       = cfg[U_STRING_FROM_CONSTANT("XAdES-C.CAStore")],
              digest_algorithm = cfg[U_STRING_FROM_CONSTANT("XAdES-C.DigestAlgorithm")];

      if (str_CApath.empty() ||
          UServices::setupOpenSSLStore(0, str_CApath.c_str()) == false)
         {
         U_ERROR("error on setting CA Store: %S", str_CApath.data());
         }
      */

      UXML2Schema XAdES_schema(UFile::contentOf(schema));

      // ---------------------------------------------------------------------------------------------------------------
      // check for OOffice or MS-Word document...
      // ---------------------------------------------------------------------------------------------------------------
      utility.handlerConfig(cfg);

      if (utility.checkDocument(content, "XAdES", false)) content = utility.getSigned();
      // ---------------------------------------------------------------------------------------------------------------

      UApplication::exit_value = 1;

      UXML2Document document(content);

      if (XAdES_schema.validate(document) == false)
         {
         UString content1;

         if (document.getElement(content1, 0, U_CONSTANT_TO_PARAM(U_TAG_SIGNED_INFO)) &&
             content1.empty() == false)
            {
            UXML2Document document1(content1);

            if (XAdES_schema.validate(document1) == false)
               {
               U_ERROR("fail to validate data input based on XAdES schema");
               }
            }
         }

      UDSIGContext dsigCtx;
      UString data, signature;
      const char* digest_algorithm = 0;

      if (dsigCtx.verify(document, digest_algorithm, data, signature))
         {
         UString element = document.getElementData(128, U_CONSTANT_TO_PARAM(U_TAG_X509_CERTIFICATE));

         if (element.empty() == false)
            {
            UString certificate(element.size());

            if (UBase64::decode(element, certificate))
               {
               alg = u_dgst_get_algoritm(digest_algorithm);

               if (alg == -1) U_ERROR("I can't find the digest algorithm for: %s", digest_algorithm);

               X509* x509 = UCertificate::readX509(certificate, "DER");

               u_pkey = UCertificate::getSubjectPublicKey(x509);

               U_SYSCALL_VOID(X509_free, "%p", x509);

               if (UServices::verifySignature(alg, data, signature, UString::getStringNull(), 0))
                  {
                  UApplication::exit_value = 0;

#              ifdef _MSWINDOWS_
                  (void) setmode(1, O_BINARY);
#              endif

               // std::cout.write(U_STRING_TO_PARAM(certificate));
                  std::cout.write(U_STRING_TO_PARAM(content));
                  }

               U_SYSCALL_VOID(EVP_PKEY_free, "%p", u_pkey);
                                                   u_pkey = 0;
               }
            }
         }

      utility.clean();
      }

private:
   UFileConfig cfg;
   int alg, num_args;
   UXAdESUtility utility;
   UString cfg_str, content, schema;
};

U_MAIN
