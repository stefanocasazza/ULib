// archive.cpp

#include <ulib/url.h>
#include <ulib/date.h>
#include <ulib/file_config.h>
#include <ulib/internal/chttp.h>
#include <ulib/utility/base64.h>
#include <ulib/utility/services.h>
#include <ulib/utility/string_ext.h>
#include <ulib/xml/libxml2/schema.h>

#ifdef HAVE_SSL_TS
#  include <ulib/ssl/timestamp.h>
#endif

#include "utility.h"

#undef  PACKAGE
#define PACKAGE "archive"

#define ARGS "[ ARCHIVE_TIMESTAMP SCHEMA ]"

#define U_OPTIONS \
"purpose \"XAdES signature (L)\"\n" \
"option c config 1 \"path of configuration file\" \"\"\n"

#include <ulib/application.h>

#define U_ARCHIVE_TIMESTAMP (const char*)(num_args >= 0 ? argv[optind+0] : 0)
#define U_SCHEMA            (const char*)(num_args >= 1 ? argv[optind+1] : 0)

// ArchiveTimeStamp

#define U_XADES_ARCHIVE_TIMESTAMP_TEMPLATE \
"          <xadesv141:ArchiveTimeStamp xmlns:xadesv141=\"http://uri.etsi.org/01903/v1.4.1#\">\r\n" \
"%.*s" \
"          </xadesv141:ArchiveTimeStamp>\r\n" \
"        </xades:UnsignedSignatureProperties>"

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE(5, "Application::Application()")

      num_args = 0;
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      vec.clear();

      U_SYSCALL_VOID_NO_PARAM(xmlCleanupParser);
      U_SYSCALL_VOID_NO_PARAM(xmlMemoryDump);
      }

   UString getTimeStampToken(const UString& data, const UString& url)
      {
      U_TRACE(5, "Application::getTimeStampToken(%.*S,%.*S)", U_STRING_TO_TRACE(data), U_STRING_TO_TRACE(url))

      UString token;

#  ifdef HAVE_SSL_TS
      token = UTimeStamp::getTimeStampToken(U_HASH_SHA1, data, url);
#  endif

      U_RETURN_STRING(token);
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
      // XAdES signature - configuration parameters
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

      content.setBuffer(x.size());

      UBase64::decode(x, content);

      if (content.empty()) U_ERROR("decoding data read failed");

      // manage arguments...

      archive_timestamp = ( U_ARCHIVE_TIMESTAMP == 0 ||
                           *U_ARCHIVE_TIMESTAMP == '\0'
                              ? cfg[U_STRING_FROM_CONSTANT("XAdES-L.ArchiveTimeStamp")]
                              : UString(U_ARCHIVE_TIMESTAMP));

      if (archive_timestamp.empty()) U_ERROR("error on archive timestamp: empty");

      schema = ( U_SCHEMA == 0 ||
                *U_SCHEMA == '\0'
                  ? cfg[U_STRING_FROM_CONSTANT("XAdES-L.Schema")]
                  : UString(U_SCHEMA));

      if (schema.empty()) U_ERROR("error on XAdES schema: empty");

      UXML2Schema XAdES_schema(UFile::contentOf(schema));

      // ---------------------------------------------------------------------------------------------------------------
      // check for OOffice or MS-Word document...
      // ---------------------------------------------------------------------------------------------------------------
      utility.handlerConfig(cfg);

      if (utility.checkDocument(content, "XAdES-C", false)) content = utility.getSigned();
      // ---------------------------------------------------------------------------------------------------------------

      UXML2Document document(content);

      if (XAdES_schema.validate(document) == false) U_ERROR("error on input data: not XAdES");

      // manage arguments...

      /*
      The input to the computation of the digest value MUST be built as follows:

      1) Initialize the final octet stream as an empty octet stream.

      2) Take all the ds:Reference elements in their order of appearance within ds:SignedInfo referencing
         whatever the signer wants to sign including the SignedProperties element. Process each one as indicated below:

         - Process the retrieved ds:Reference element according to the reference processing model of XMLDSIG.

         - If the result is a XML node set, canonicalize it. If ds:Canonicalization is present, the algorithm
           indicated by this element is used. If not, the standard canonicalization method specified by XMLDSIG
           is used.

         - Concatenate the resulting octets to the final octet stream.
      */

      uint32_t i, n = document.getElement(vec, U_CONSTANT_TO_PARAM("ds:Reference"));

      for (i = 0; i < n; ++i)
         {
         x = vec[i];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      /*
      3) Take the following XMLDSIG elements in the order they are listed below, canonicalize each one and
         concatenate each resulting octet stream to the final octet stream:

         - The ds:SignedInfo element.
         - The ds:SignatureValue element.
         - The ds:KeyInfo element, if present.
      */

      if (document.getElement(vec, U_CONSTANT_TO_PARAM("ds:SignedInfo")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      if (document.getElement(vec, U_CONSTANT_TO_PARAM("ds:SignatureValue")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      if (document.getElement(vec, U_CONSTANT_TO_PARAM("ds:KeyInfo")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      /*
      4) Take the unsigned signature properties that appear before the current xadesv141:ArchiveTimeStamp
         in the order they appear within the xades:UnsignedSignatureProperties, canonicalize each one and
         concatenate each resulting octet stream to the final octet stream. While concatenating the following
         rules apply:

         - The xades:CertificateValues property MUST be added if it is not already present and the
           ds:KeyInfo element does not contain the full set of certificates used to validate the electronic signature.

         - The xades:RevocationValues property MUST be added if it is not already present and the
           ds:KeyInfo element does not contain the revocation information that has to be shipped with the electronic signature.

         - The xades:AttrAuthoritiesCertValues property MUST be added if not already present and the following
           conditions are true: there exist an attribute certificate in the signature AND a number of certificates
           that have been used in its validation do not appear in CertificateValues. Its content will satisfy with
           the rules specified in clause 7.6.3.

         - The xades:AttributeRevocationValues property MUST be added if not already present and there the following
           conditions are true: there exist an attribute certificate AND some revocation data that have been used in
           its validation do not appear in RevocationValues. Its content will satisfy with the rules specified in clause 7.6.4.
      */

      if (document.getElement(vec, U_CONSTANT_TO_PARAM("xades:CompleteCertificateRefs")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      if (document.getElement(vec, U_CONSTANT_TO_PARAM("xades:CertificateValues")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      if (document.getElement(vec, U_CONSTANT_TO_PARAM("xades:CompleteRevocationRefs")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      if (document.getElement(vec, U_CONSTANT_TO_PARAM("xades:RevocationValues")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      if (document.getElement(vec, U_CONSTANT_TO_PARAM("xades:SignatureTimeStamp")))
         {
         x = vec[0];

         to_digest += UXML2Document::xmlC14N(x);
         }

      vec.clear();

      /*
      5) Take all the ds:Object elements except the one containing xades:QualifyingProperties element. Canonicalize each one
         and concatenate each resulting octet stream to the final octet stream. If ds:Canonicalization is present, the algorithm
         indicated by this element is used. If not, the standard canonicalization method specified by XMLDSIG is used.
      */

      n = document.getElement(vec, U_CONSTANT_TO_PARAM("ds:Object"));

      for (i = 0; i < n; ++i)
         {
         x = vec[i];

         if (U_STRING_FIND(x, 0, "xades:QualifyingProperties") == U_NOT_FOUND) to_digest += UXML2Document::xmlC14N(x);
         }

      u_base64_max_columns  = U_OPENSSL_BASE64_MAX_COLUMN;
      U_line_terminator_len = 2;

      UString archiveTimeStamp(U_CAPACITY), token = getTimeStampToken(to_digest, archive_timestamp);

      archiveTimeStamp.snprintf(U_CONSTANT_TO_PARAM(U_XADES_ARCHIVE_TIMESTAMP_TEMPLATE), U_STRING_TO_TRACE(token));

      UString _output = UStringExt::substitute(content,
                             U_CONSTANT_TO_PARAM("        </xades:UnsignedSignatureProperties>"),
                             U_STRING_TO_PARAM(archiveTimeStamp));

      // ---------------------------------------------------------------------------------------------------------------
      // check for OOffice or MS-Word document...
      // ---------------------------------------------------------------------------------------------------------------
      utility.outputDocument(_output);
      // ---------------------------------------------------------------------------------------------------------------
      }

private:
   int num_args;
   UFileConfig cfg;
   UVector<UString> vec;
   UXAdESUtility utility;
   UString cfg_str, content, to_digest, archive_timestamp, schema, output;
};

U_MAIN
