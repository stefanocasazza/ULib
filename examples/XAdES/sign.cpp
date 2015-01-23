// sign.cpp

#include <ulib/url.h>
#include <ulib/date.h>
#include <ulib/zip/zip.h>
#include <ulib/ssl/crl.h>
#include <ulib/file_config.h>
#include <ulib/base/ssl/dgst.h>
#include <ulib/utility/base64.h>
#include <ulib/ssl/certificate.h>
#include <ulib/utility/services.h>
#include <ulib/utility/xml_escape.h>
#include <ulib/utility/string_ext.h>
#include <ulib/xml/libxml2/document.h>

#ifdef HAVE_SSL_TS
#  include <ulib/ssl/timestamp.h>
#endif

#include "utility.h"

#undef  PACKAGE
#define PACKAGE "sign"

#define ARGS "DATA_URI X509 KEY_HANDLE DIGEST_ALGORITHM" \
             " SIGNING_TIME CLAIMED_ROLE" \
             " PRODUCTION_PLACE_CITY" \
             " PRODUCTION_PLACE_STATE_OR_PROVINCE" \
             " PRODUCTION_PLACE_POSTAL_CODE" \
             " PRODUCTION_PLACE_COUNTRY_NAME" \
             " [ CA_STORE SIGNATURE_TIMESTAMP ]"

#define U_OPTIONS \
"purpose \"XAdES signature (BES and C)\"\n" \
"option c config 1 \"path of configuration file\" \"\"\n"

#include <ulib/application.h>

#define U_DATA_URI                           (const char*)(num_args >= 0  ? argv[optind+0]  : 0)
#define U_X509                               (const char*)(num_args >= 1  ? argv[optind+1]  : 0)
#define U_KEY_HANDLE                         (const char*)(num_args >= 2  ? argv[optind+2]  : 0)
#define U_DIGEST_ALGORITHM                   (const char*)(num_args >= 3  ? argv[optind+3]  : 0)
#define U_SIGNING_TIME                       (const char*)(num_args >= 4  ? argv[optind+4]  : 0)
#define U_CLAIMED_ROLE                       (const char*)(num_args >= 5  ? argv[optind+5]  : 0)
#define U_PRODUCTION_PLACE_CITY              (const char*)(num_args >= 6  ? argv[optind+6]  : 0)
#define U_PRODUCTION_PLACE_STATE_OR_PROVINCE (const char*)(num_args >= 7  ? argv[optind+7]  : 0)
#define U_PRODUCTION_PLACE_POSTAL_CODE       (const char*)(num_args >= 8  ? argv[optind+8]  : 0)
#define U_PRODUCTION_PLACE_COUNTRY_NAME      (const char*)(num_args >= 9  ? argv[optind+9]  : 0)
#define U_CA_STORE                           (const char*)(num_args >= 10 ? argv[optind+10] : 0)
#define U_SIGNATURE_TIMESTAMP                (const char*)(num_args >= 11 ? argv[optind+11] : 0)

#define U_XMLDSIG_DIGESTMETHOD_TEMPLATE \
"<ds:DigestMethod Algorithm=\"http://www.w3.org/2001/04/xmlenc#%.*s\"></ds:DigestMethod>\r\n"

#define U_XMLDSIG_REFERENCE_TEMPLATE \
"    <ds:Reference xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\" URI=\"%s\">\r\n" \
"      " U_XMLDSIG_DIGESTMETHOD_TEMPLATE \
"      <ds:DigestValue>%.*s</ds:DigestValue>\r\n" \
"    </ds:Reference>\r\n"

#define U_XMLDSIG_SIGNED_INFO_TEMPLATE \
"  <ds:SignedInfo xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\">\r\n" \
"    <ds:CanonicalizationMethod Algorithm=\"http://www.w3.org/2006/12/xml-c14n11\"></ds:CanonicalizationMethod>\r\n" \
"    <ds:SignatureMethod Algorithm=\"http://www.w3.org/2001/04/xmldsig-more#rsa-%.*s\"></ds:SignatureMethod>\r\n" \
"%.*s" \
"%.*s" \
"  </ds:SignedInfo>\r\n"

#define U_XMLDSIG_SIGNATURE_VALUE_TEMPLATE \
"  <ds:SignatureValue xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\">\r\n" \
"%.*s" \
"  </ds:SignatureValue>\r\n" \

#define U_XMLDSIG_KEYINFO_TEMPLATE \
"  <ds:KeyInfo xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\">\r\n" \
"    <ds:KeyValue>\r\n" \
"      <ds:RSAKeyValue>\r\n" \
"        <ds:Modulus>\r\n" \
"%.*s" \
"        </ds:Modulus>\r\n" \
"        <ds:Exponent>%.*s</ds:Exponent>\r\n" \
"      </ds:RSAKeyValue>\r\n" \
"    </ds:KeyValue>\r\n" \
"    <ds:X509Data>\r\n" \
"      <ds:X509SubjectName>%.*s</ds:X509SubjectName>\r\n" \
"      <ds:X509IssuerSerial>\r\n" \
"        <ds:X509IssuerName>%.*s</ds:X509IssuerName>\r\n" \
"        <ds:X509SerialNumber>%ld</ds:X509SerialNumber>\r\n" \
"      </ds:X509IssuerSerial>\r\n" \
"      <ds:X509Certificate>\r\n" \
"%.*s" \
"      </ds:X509Certificate>\r\n" \
"    </ds:X509Data>\r\n" \
"  </ds:KeyInfo>\r\n"

// NB: if we change id name (idPackageSignature) remember to change also context.cpp...

#define U_XMLDSIG_TEMPLATE \
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n" \
"%.*s" \
"<ds:Signature Id=\"idPackageSignature\" xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\">\r\n" \
"%.*s" \
"%.*s" \
"%.*s" \
"%.*s" \
"</ds:Signature>\r\n" \
"%.*s"

#define U_XADES_REFERENCE_TEMPLATE \
"    <ds:Reference xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\" URI=\"#idPackageSignature-SignedProperties\" Type=\"http://uri.etsi.org/01903#SignedProperties\">\r\n" \
"      " U_XMLDSIG_DIGESTMETHOD_TEMPLATE \
"      <ds:DigestValue>%.*s</ds:DigestValue>\r\n" \
"    </ds:Reference>\r\n"

#define U_XADES_SIGNING_TIME_TEMPLATE \
"          <xades:SigningTime>%.*s</xades:SigningTime>\r\n"

#define U_XADES_SIGNER_ROLE_TEMPLATE \
"          <xades:SignerRole>\r\n" \
"            <xades:ClaimedRoles>\r\n" \
"              <xades:ClaimedRole>%.*s</xades:ClaimedRole>\r\n" \
"            </xades:ClaimedRoles>\r\n" \
"          </xades:SignerRole>\r\n"

#define U_XADES_DATA_OBJECT_FORMAT_TEMPLATE \
"         <xades:DataObjectFormat ObjectReference=\"%s\">\r\n" \
"           <xades:MimeType>%.*s</xades:MimeType>\r\n" \
"         </xades:DataObjectFormat>\r\n"

#define U_XADES_ALL_DATA_OBJECTS_TIMESTAMP_TEMPLATE \
"         <xades:AllDataObjectsTimeStamp>\r\n" \
"           <xades:EncapsulatedTimeStamp>\r\n" \
"%.*s" \
"           </xades:EncapsulatedTimeStamp>\r\n" \
"         </xades:AllDataObjectsTimeStamp>\r\n"

#define U_XADES_CERTIFICATE_TEMPLATE \
"            <xades:Cert>\r\n" \
"              <xades:CertDigest>\r\n" \
"                " U_XMLDSIG_DIGESTMETHOD_TEMPLATE \
"                <ds:DigestValue>%.*s</ds:DigestValue>\r\n" \
"              </xades:CertDigest>\r\n" \
"              <xades:IssuerSerial>\r\n" \
"                <ds:X509IssuerName>%.*s</ds:X509IssuerName>\r\n" \
"                <ds:X509SerialNumber>%ld</ds:X509SerialNumber>\r\n" \
"              </xades:IssuerSerial>\r\n" \
"            </xades:Cert>\r\n"

#define U_XADES_CRL_TEMPLATE \
"              <xades:CRLRef>\r\n" \
"                <xades:DigestAlgAndValue>\r\n" \
"                  " U_XMLDSIG_DIGESTMETHOD_TEMPLATE \
"                  <ds:DigestValue>%.*s</ds:DigestValue>\r\n" \
"                </xades:DigestAlgAndValue>\r\n" \
"                <xades:CRLIdentifier>\r\n" \
"                  <xades:Issuer>\r\n" \
"                    %.*s\r\n" \
"                  </xades:Issuer>\r\n" \
"                  <xades:IssueTime>%.*s</xades:IssueTime>\r\n" \
"                  <xades:Number>%ld</xades:Number>\r\n" \
"                </xades:CRLIdentifier>\r\n" \
"              </xades:CRLRef>\r\n"

#define U_XADES_ENCAPSULATED_X509_CERTIFICATE_TEMPLATE \
"            <xades:EncapsulatedX509Certificate>\r\n" \
"%.*s" \
"            </xades:EncapsulatedX509Certificate>\r\n"

#define U_XADES_ENCAPSULATED_CRL_VALUE_TEMPLATE \
"            <xades:EncapsulatedCRLValue>\r\n" \
"%.*s" \
"            </xades:EncapsulatedCRLValue>\r\n"

#define U_XADES_SIGNED_PROPERTIES_TEMPLATE \
"      <xades:SignedProperties xmlns:xades=\"http://uri.etsi.org/01903/v1.4.1#\" xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\" Id=\"idPackageSignature-SignedProperties\">\r\n" \
"        <xades:SignedSignatureProperties>\r\n" \
"%.*s" \
"          <xades:SigningCertificate>\r\n" \
"%.*s" \
"          </xades:SigningCertificate>\r\n" \
"          <xades:SignaturePolicyIdentifier>\r\n" \
"            <xades:SignaturePolicyImplied></xades:SignaturePolicyImplied>\r\n" \
"          </xades:SignaturePolicyIdentifier>\r\n" \
"          <xades:SignatureProductionPlace>\r\n" \
"            <xades:City>%.*s</xades:City>\r\n" \
"            <xades:StateOrProvince>%.*s</xades:StateOrProvince>\r\n" \
"            <xades:PostalCode>%.*s</xades:PostalCode>\r\n" \
"            <xades:CountryName>%.*s</xades:CountryName>\r\n" \
"          </xades:SignatureProductionPlace>\r\n" \
"%.*s" \
"        </xades:SignedSignatureProperties>\r\n" \
"        <xades:SignedDataObjectProperties>\r\n" \
"%.*s" \
"        </xades:SignedDataObjectProperties>\r\n" \
"      </xades:SignedProperties>\r\n"

#define U_XADES_COMPLETE_CERTIFICATE_REFS_TEMPLATE \
"          <xades:CompleteCertificateRefs xmlns:xades=\"http://uri.etsi.org/01903/v1.4.1#\" xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\">\r\n" \
"%.*s" \
"          </xades:CompleteCertificateRefs>\r\n"

#define U_XADES_COMPLETE_REVOCATION_REFS_TEMPLATE \
"          <xades:CompleteRevocationRefs xmlns:xades=\"http://uri.etsi.org/01903/v1.4.1#\" xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\">\r\n" \
"            <xades:CRLRefs>\r\n" \
"%.*s" \
"            </xades:CRLRefs>\r\n" \
"          </xades:CompleteRevocationRefs>\r\n"

#define U_XADES_CERTIFICATE_VALUES_TEMPLATE \
"          <xades:CertificateValues xmlns:xades=\"http://uri.etsi.org/01903/v1.4.1#\">\r\n" \
"%.*s" \
"          </xades:CertificateValues>\r\n"

#define U_XADES_REVOCATION_VALUES_TEMPLATE \
"          <xades:RevocationValues xmlns:xades=\"http://uri.etsi.org/01903/v1.4.1#\">\r\n" \
"%.*s" \
"          </xades:RevocationValues>\r\n"

// ArchiveTimeStamp

#define U_XADES_ARCHIVE_TIMESTAMP_TEMPLATE \
"          <xadesv141:ArchiveTimeStamp xmlns:xadesv141=\"http://uri.etsi.org/01903/v1.4.1#\">\r\n" \
"%.*s" \
"          </xadesv141:ArchiveTimeStamp>\r\n"

#define U_XADES_SIGNATURE_TIMESTAMP_TEMPLATE \
"          <xades:SignatureTimeStamp xmlns:xades=\"http://uri.etsi.org/01903/v1.4.1#\">\r\n" \
"%.*s" \
"          </xades:SignatureTimeStamp>\r\n"

// RefsOnlyTimeStamp: it contains a time-stamp token only over all certificate and revocation
//                    information references.

#define U_XADES_REFSONLY_TIMESTAMP_TEMPLATE \
"          <xades:RefsOnlyTimeStamp>\r\n" \
"%.*s" \
"          </xades:RefsOnlyTimeStamp>\r\n"

// SigAndRefsTimeStamp: it contains a time-stamp token computed over the signature value, the
//                      signature time-stamp and the certificate and revocation information references.

#define U_XADES_SIGANDREFS_TIMESTAMP_TEMPLATE \
"          <xades:SigAndRefsTimeStamp>\r\n" \
"%.*s" \
"          </xades:SigAndRefsTimeStamp>\r\n"

#define U_XADES_UNSIGNED_SIGNATURE_PROPERTIES_TEMPLATE \
"%.*s" \
"%.*s" \
"%.*s" \
"%.*s"

#define U_XADES_TEMPLATE \
"  <ds:Object Id=\"idPackageObject\" xmlns:xades=\"http://uri.etsi.org/01903/v1.4.1#\" xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\">\r\n" \
"    <xades:QualifyingProperties Target=\"#idPackageSignature\">\r\n" \
"%.*s" \
"      <xades:UnsignedProperties>\r\n" \
"        <xades:UnsignedSignatureProperties>\r\n" \
"%.*s" \
"%.*s" \
"%.*s" \
"        </xades:UnsignedSignatureProperties>\r\n" \
"      </xades:UnsignedProperties>\r\n" \
"    </xades:QualifyingProperties>\r\n" \
"  </ds:Object>\r\n"

/* XAdES-C

XML Advanced Electronic Signature with Complete validation data references (XAdES-C)
in accordance with the present document adds to the XAdES-T the CompleteCertificateRefs
and CompleteRevocationRefs unsigned properties as defined by the present document.
If attribute certificates appear in the signature, then XAdES-C also incorporates the
AttributeCertificateRefs and the AttributeRevocationRefs elements. CompleteCertificateRefs
element contains a sequence of references to the full set of CA certificates that have been
used to validate the electronic signature up to (but not including) the signing certificate.
CompleteRevocationRefs element contains a full set of references to the revocation data that
have been used in the validation of the signer and CA certificates. AttributeCertificateRefs
and AttributeRevocationRefs elements contain references to the full set of Attribute Authorities
certificates and references to the full set of revocation data that have been used in the validation
of the attribute certificates present in the signature, respectively. Storing the references allows
the values of the certification path and revocation data to be stored elsewhere, reducing the size
of a stored electronic signature format. 

XMLDSIG of simple text string.
-------------------------------------------------------------------------------------------------------------------
INPUT: T    text-to-be-signed, a byte string
       Ks   RSA private key

OUTPUT: XML file

   1. Canonicalize (strictly, what we are doing here is encapsulating the text string T inside an <Object> element,
      then canonicalizing that element) the text-to-be-signed, C = C14n(T).
   2. Compute the message digest of the canonicalized text, m = Hash(C).
   3. Encapsulate the message digest in an XML <SignedInfo> element, SI, in canonicalized form.
   4. Compute the RSA signatureValue of the canonicalized <SignedInfo> element, SV = RsaSign(Ks, SI).
   5. Compose the final XML document including the signatureValue, this time in non-canonicalized form.
-------------------------------------------------------------------------------------------------------------------
*/

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE(5, "Application::Application()")

      num_ca = 0;
      xades_c = false;
      alg = num_args = 0;
      X509SerialNumber = signing_time = 0;
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      U_SYSCALL_VOID_NO_PARAM(xmlCleanupParser);
      U_SYSCALL_VOID_NO_PARAM(xmlMemoryDump);
      }

   UString getOptionValue(const char* param, const char* tag)
      {
      U_TRACE(5, "Application::getOptionValue(%S,%S)", param, tag)

      UString result;

      if (param && *param) result = param;
      else
         {
         UString tmp(100U);

         U_INTERNAL_DUMP("xades_c = %b", xades_c)

         tmp.snprintf("XAdES-%s.%s", (xades_c ? "C" : "BES"), tag);

         result = cfg[tmp];
         }

      U_RETURN_STRING(result);
      }

   UString getTimeStampToken(const UString& data, const UString& url)
      {
      U_TRACE(5, "Application::getTimeStampToken(%.*S,%.*S)", U_STRING_TO_TRACE(data), U_STRING_TO_TRACE(url))

      UString token;

#  ifdef HAVE_SSL_TS
      token = UTimeStamp::getTimeStampToken(alg, data, url);
#  endif

      U_RETURN_STRING(token);
      }

   void setXAdESReference()
      {
      U_TRACE(5, "Application::setXAdESReference()")

      // SIGNED PROPERTIES

      UString signingCertificate(U_CAPACITY);

      // Compute the digest of the signer certificate

      UString DigestValue(U_CAPACITY);

      UServices::generateDigest(alg, 0, X509Certificate, DigestValue, true);

      signingCertificate.snprintf(U_XADES_CERTIFICATE_TEMPLATE,
                                  U_STRING_TO_TRACE(digest_algorithm),
                                  U_STRING_TO_TRACE(DigestValue),
                                  U_STRING_TO_TRACE(X509IssuerName),
                                  X509SerialNumber);

      UString signingTime(100U);

      if (signing_time)
         {
         UString dateTime = UTimeDate::strftime("%Y-%m-%dT%H:%M:%SZ", u_now->tv_sec);

         signingTime.snprintf(U_XADES_SIGNING_TIME_TEMPLATE, U_STRING_TO_TRACE(dateTime));
         }

      UString roleTemplate(U_CAPACITY);

      if (claimed_role.empty() == false) roleTemplate.snprintf(U_XADES_SIGNER_ROLE_TEMPLATE, U_STRING_TO_TRACE(claimed_role));

      (void) signedProperties.reserve(U_CONSTANT_SIZE(U_XADES_SIGNED_PROPERTIES_TEMPLATE) + 8192U + signingTime.size());

      signedProperties.snprintf(U_XADES_SIGNED_PROPERTIES_TEMPLATE,
                                U_STRING_TO_TRACE(signingTime),
                                U_STRING_TO_TRACE(signingCertificate),
                                U_STRING_TO_TRACE(production_place_city),
                                U_STRING_TO_TRACE(production_place_state_or_province),
                                U_STRING_TO_TRACE(production_place_postal_code),
                                U_STRING_TO_TRACE(production_place_country_name),
                                U_STRING_TO_TRACE(roleTemplate),
                                U_STRING_TO_TRACE(DataObjectFormat));

      to_digest = UXML2Document::xmlC14N(signedProperties);

      UString signedPropertiesDigestValue(200U);

      UServices::generateDigest(alg, 0, to_digest, signedPropertiesDigestValue, true);

      (void) XAdESReference.reserve(U_CONSTANT_SIZE(U_XADES_REFERENCE_TEMPLATE) + signedPropertiesDigestValue.size());

      XAdESReference.snprintf(U_XADES_REFERENCE_TEMPLATE,
                              U_STRING_TO_TRACE(digest_algorithm),
                              U_STRING_TO_TRACE(signedPropertiesDigestValue));
      }

   // -------------------------------------------------------------------------------------------------------------  
   // XAdES-C
   // -------------------------------------------------------------------------------------------------------------  
   void setXAdESUnsignedSignatureProperties()
      {
      U_TRACE(5, "Application::setXAdESUnsignedSignatureProperties()")

      // UNSIGNED SIGNATURE PROPERTIES

      // <CompleteCertificateRefs>...</CompleteCertificateRefs>

      UString completeCertificateRef(U_CAPACITY), completeCertificateRefs(U_CAPACITY);

      uint32_t i, n;
      long CASerialNumber;
      UVector<UString> vec_CACertificateValue;
      UString item(U_CAPACITY), CACertificateValue(U_CAPACITY), CAIssuerName, CACertificate, DigestValue(U_CAPACITY);

      UCertificate* ca;

      for (i = 0; i < num_ca; ++i)
         {
         ca = vec_ca[i];

         CAIssuerName   = ca->getIssuerForLDAP();
         CACertificate  = ca->getEncoded("DER");
         CASerialNumber = ca->getSerialNumber();

         DigestValue.setEmpty();

         UServices::generateDigest(alg, 0, CACertificate, DigestValue, true);

         item.snprintf(U_XADES_CERTIFICATE_TEMPLATE,
                       U_STRING_TO_TRACE(digest_algorithm),
                       U_STRING_TO_TRACE(DigestValue),
                       U_STRING_TO_TRACE(CAIssuerName),
                       CASerialNumber);

         (void) completeCertificateRef.append(item);

         u_base64_max_columns = U_OPENSSL_BASE64_MAX_COLUMN;

         UBase64::encode(CACertificate, CACertificateValue);

         u_base64_max_columns = 0;

         vec_CACertificateValue.push_back(CACertificateValue);
         }

      completeCertificateRefs.snprintf(U_XADES_COMPLETE_CERTIFICATE_REFS_TEMPLATE, U_STRING_TO_TRACE(completeCertificateRef));

      unsignedSignaturePropertiesC14N += UXML2Document::xmlC14N(completeCertificateRefs);

      // <CertificateValues>....</CertificateValues>

      UString certificateValue(U_CAPACITY), certificateValues(U_CAPACITY);

      for (i = 0, n = vec_CACertificateValue.size(); i < n; ++i)
         {
         CACertificateValue = vec_CACertificateValue[i];

         item.snprintf(U_XADES_ENCAPSULATED_X509_CERTIFICATE_TEMPLATE,
                       U_STRING_TO_TRACE(CACertificateValue));

         (void) certificateValue.append(item);
         }

      certificateValues.snprintf(U_XADES_CERTIFICATE_VALUES_TEMPLATE, U_STRING_TO_TRACE(certificateValue));

      unsignedSignaturePropertiesC14N += UXML2Document::xmlC14N(certificateValues);

      // <CompleteRevocationRefs>...</CompleteRevocationRefs>

      UString completeRevocationRef(U_CAPACITY), completeRevocationRefs(U_CAPACITY);

      long CRLNumber;
      bool crl_exist;
      UString crlpath;
      UVector<UString> vec_CRLValue;
      UString CRLValue(U_CAPACITY), CRLIssuerName, CRL, CRLIssueTime;

      for (i = 0; i < num_ca; ++i)
         {
         ca      = vec_ca[i];
         crlpath = UCertificate::getFileName(ca->hashCode(), true,  &crl_exist);

         if (crl_exist)
            {
            UCrl crl(UFile::contentOf(crlpath));

            CRL             = crl.getEncoded("DER");
            CRLNumber       = crl.getNumber();
            CRLIssuerName   = crl.getIssuerForLDAP();
            CRLIssueTime    = UTimeDate::strftime("%Y-%m-%dT%H:%M:%SZ", crl.getIssueTime());

            DigestValue.setEmpty();

            UServices::generateDigest(alg, 0, CRL, DigestValue, true);

            item.snprintf(U_XADES_CRL_TEMPLATE,
                          U_STRING_TO_TRACE(digest_algorithm),
                          U_STRING_TO_TRACE(DigestValue),
                          U_STRING_TO_TRACE(CRLIssuerName),
                          U_STRING_TO_TRACE(CRLIssueTime),
                          CRLNumber);

            (void) completeRevocationRef.append(item);

            u_base64_max_columns = U_OPENSSL_BASE64_MAX_COLUMN;

            UBase64::encode(CRL, CRLValue);

            u_base64_max_columns = 0;

            vec_CRLValue.push_back(CRLValue);
            }
         }

      completeRevocationRefs.snprintf(U_XADES_COMPLETE_REVOCATION_REFS_TEMPLATE, U_STRING_TO_TRACE(completeRevocationRef));

      unsignedSignaturePropertiesC14N += UXML2Document::xmlC14N(completeRevocationRefs);

      // <RevocationValues>...</RevocationValues>

      UString revocationValue(U_CAPACITY), revocationValues(U_CAPACITY);

      for (i = 0, n = vec_CRLValue.size(); i < n; ++i)
         {
         CRLValue = vec_CRLValue[i];

         item.snprintf(U_XADES_ENCAPSULATED_CRL_VALUE_TEMPLATE,
                       U_STRING_TO_TRACE(CRLValue));

         (void) revocationValue.append(item);
         }

      revocationValues.snprintf(U_XADES_REVOCATION_VALUES_TEMPLATE, U_STRING_TO_TRACE(revocationValue));

      unsignedSignaturePropertiesC14N += UXML2Document::xmlC14N(revocationValues);

      (void) unsignedSignatureProperties.reserve(U_CONSTANT_SIZE(U_XADES_UNSIGNED_SIGNATURE_PROPERTIES_TEMPLATE) +
                                          completeCertificateRefs.size() +
                                          completeRevocationRefs.size() +
                                          certificateValues.size() +
                                          revocationValues.size());

      unsignedSignatureProperties.snprintf(U_XADES_UNSIGNED_SIGNATURE_PROPERTIES_TEMPLATE,
                                           U_STRING_TO_TRACE(completeCertificateRefs),
                                           U_STRING_TO_TRACE(certificateValues),
                                           U_STRING_TO_TRACE(completeRevocationRefs),
                                           U_STRING_TO_TRACE(revocationValues));
      }
   // -------------------------------------------------------------------------------------------------------------  

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

      (void) document.reserve(x.size());

      if (UBase64::decode(x, document) == false) U_ERROR("decoding data read failed");

      // manage arguments...

      U_INTERNAL_ASSERT_POINTER(U_DATA_URI)

      if (*U_DATA_URI == '\0') U_ERROR("DATA_URI is mandatory");

      U_INTERNAL_ASSERT_POINTER(U_X509)

      if (*U_X509 == '\0') U_ERROR("X509 is mandatory");

      UCertificate cert(UString(U_X509));

      if (cert.isValid() == false) U_ERROR("certificate not valid");

      U_INTERNAL_ASSERT_POINTER(U_KEY_HANDLE)

      if (*U_KEY_HANDLE == '\0') U_ERROR("KEY_HANDLE is mandatory");

      U_INTERNAL_DUMP("U_CA_STORE = %S", U_CA_STORE)

      xades_c = (U_CA_STORE != 0);

      digest_algorithm = getOptionValue(U_DIGEST_ALGORITHM, "DigestAlgorithm");

      alg = u_dgst_get_algoritm(digest_algorithm.c_str());

      if (alg == -1) U_ERROR("I can't find the digest algorithm for: %s", digest_algorithm.data());

      signing_time                       = getOptionValue(U_SIGNING_TIME,                       "SigningTime").strtol();
      claimed_role                       = getOptionValue(U_CLAIMED_ROLE,                       "ClaimedRole");
      production_place_city              = getOptionValue(U_PRODUCTION_PLACE_CITY,              "ProductionPlaceCity");
      production_place_state_or_province = getOptionValue(U_PRODUCTION_PLACE_STATE_OR_PROVINCE, "ProductionPlaceStateOrProvince");
      production_place_postal_code       = getOptionValue(U_PRODUCTION_PLACE_POSTAL_CODE,       "ProductionPlacePostalCode");
      production_place_country_name      = getOptionValue(U_PRODUCTION_PLACE_COUNTRY_NAME,      "ProductionPlaceCountryName");
      data_object_format_mimetype        = getOptionValue("",                                   "DataObjectFormatMimeType");

      if (xades_c == false) num_ca = 0;
      else
         {
         // XAdES-C
         // -------------------------------------------------------------------------------------------------------------  
         str_CApath          = getOptionValue(U_CA_STORE,            "CAStore");
         signature_timestamp = getOptionValue(U_SIGNATURE_TIMESTAMP, "SignatureTimeStamp");

         if (str_CApath.empty() ||
             UServices::setupOpenSSLStore(0, str_CApath.c_str()) == false)
            {
            U_ERROR("error on setting CA Store: %S", str_CApath.data());
            }

         num_ca = cert.getSignerCertificates(vec_ca, 0, 0);

         if (UCertificate::verify_result == false)
            {
            UServices::setVerifyStatus();

            U_ERROR("error on verifying the certificate: %.*s", u_buffer_len, u_buffer);
            }
         // -------------------------------------------------------------------------------------------------------------  
         }

      u_line_terminator_len = 2;
      u_base64_max_columns  = U_OPENSSL_BASE64_MAX_COLUMN;

      UString modulus          = cert.getModulus(),
              exponent         = cert.getExponent();
              X509IssuerName   = cert.getIssuerForLDAP(),
              X509SubjectName  = cert.getSubjectForLDAP(),
              X509Certificate  = cert.getEncoded("DER");
              X509SerialNumber = cert.getSerialNumber();

      UString X509CertificateValue(U_CAPACITY), KeyInfo(U_CAPACITY);

      UBase64::encode(X509Certificate, X509CertificateValue);

      u_base64_max_columns = 0;

      KeyInfo.snprintf(U_XMLDSIG_KEYINFO_TEMPLATE,
                       U_STRING_TO_TRACE(modulus),
                       U_STRING_TO_TRACE(exponent),
                       U_STRING_TO_TRACE(X509SubjectName),
                       U_STRING_TO_TRACE(X509IssuerName),
                       X509SerialNumber,
                       U_STRING_TO_TRACE(X509CertificateValue));

      UString ObjectDigestValue(200U),
              Reference(U_CAPACITY), dataObjectFormat(U_CAPACITY),
              XMLDSIGReference(U_CAPACITY), XMLDSIGReferenceC14N(U_CAPACITY);

      // ---------------------------------------------------------------------------------------------------------------
      // check for OOffice or MS-Word document...
      // ---------------------------------------------------------------------------------------------------------------
      utility.handlerConfig(cfg);

      (void) utility.checkDocument(document, U_DATA_URI, true);
      // ---------------------------------------------------------------------------------------------------------------

      for (uint32_t i = 0, n = utility.vdocument.size(); i < n; ++i)
         {
         uri       = utility.vuri[i];
         to_digest = utility.vdocument[i];

         // ---------------------------------------------------------------------------------------------------------------
         // 2. Compute the message digest of the text, m = Hash(C).
         // ---------------------------------------------------------------------------------------------------------------
         ObjectDigestValue.setEmpty();

         UServices::generateDigest(alg, 0, to_digest, ObjectDigestValue, true);
         // ---------------------------------------------------------------------------------------------------------------

         Reference.snprintf(U_XMLDSIG_REFERENCE_TEMPLATE, uri.c_str(),
                            U_STRING_TO_TRACE(digest_algorithm),
                            U_STRING_TO_TRACE(ObjectDigestValue));

         XMLDSIGReference     +=                        Reference;
         XMLDSIGReferenceC14N += UXML2Document::xmlC14N(Reference);

         if (data_object_format_mimetype.empty() == false)
            {
            dataObjectFormat.snprintf(U_XADES_DATA_OBJECT_FORMAT_TEMPLATE, uri.c_str(), U_STRING_TO_TRACE(data_object_format_mimetype));

            DataObjectFormat += dataObjectFormat;
            }
         }

      setXAdESReference(); // XAdES management

      // ---------------------------------------------------------------------------------------------------------------
      // 3. Encapsulate the message digest in an XML <SignedInfo> element, SI, in canonicalized form.
      // ---------------------------------------------------------------------------------------------------------------
      UString SignedInfo(U_CONSTANT_SIZE(U_XMLDSIG_SIGNED_INFO_TEMPLATE) + XMLDSIGReference.size() + XAdESReference.size());

      SignedInfo.snprintf(U_XMLDSIG_SIGNED_INFO_TEMPLATE,
                          U_STRING_TO_TRACE(digest_algorithm),
                          U_STRING_TO_TRACE(XMLDSIGReference),
                          U_STRING_TO_TRACE(XAdESReference));

      UString to_sign = UXML2Document::xmlC14N(SignedInfo);
      // ---------------------------------------------------------------------------------------------------------------

      // ---------------------------------------------------------------------------------------------------------------
      // 4. Compute the RSA signatureValue of the canonicalized <SignedInfo> element, SV = RsaSign(Ks, SI).
      // ---------------------------------------------------------------------------------------------------------------
      UString SignatureValue(U_CAPACITY), signatureTimeStamp(U_CAPACITY), archiveTimeStamp(U_CAPACITY);

      u_base64_max_columns = U_OPENSSL_BASE64_MAX_COLUMN;

      ENGINE* e;

#  ifdef _MSWINDOWS_
      e = UServices::loadEngine("HCSP", ENGINE_METHOD_RSA);
      x = U_KEY_HANDLE;
#  else
      e = 0;
      x = UFile::contentOf(U_KEY_HANDLE);

      if (x.empty() ||
          (u_pkey = UServices::loadKey(x, 0, true, 0, e)) == 0)
         {
         U_ERROR("I can't load the private key: %S", U_KEY_HANDLE);
         }

#     ifdef HAVE_OPENSSL_98
      if (cert.matchPrivateKey(u_pkey) == false) U_ERROR("the private key doesn't matches the public key of the certificate");
#     endif

      x.clear();
#  endif

      UString sign = UServices::getSignatureValue(alg, to_sign, x, UString::getStringNull(), true, e);

      u_base64_max_columns = 0;

      SignatureValue.snprintf(U_XMLDSIG_SIGNATURE_VALUE_TEMPLATE, U_STRING_TO_TRACE(sign));

      if (signature_timestamp.empty() == false)
         {
         to_digest = UXML2Document::xmlC14N(SignatureValue);

         UString token = getTimeStampToken(to_digest, signature_timestamp);

         signatureTimeStamp.snprintf(U_XADES_SIGNATURE_TIMESTAMP_TEMPLATE, U_STRING_TO_TRACE(token));
         }

      // XAdES-C
      // -------------------------------------------------------------------------------------------------------------  
      if (xades_c) setXAdESUnsignedSignatureProperties();
      // -------------------------------------------------------------------------------------------------------------  

      (void) XAdESObject.reserve(U_CONSTANT_SIZE(U_XADES_TEMPLATE) +
                          signedProperties.size() +
                          unsignedSignatureProperties.size() +
                          archiveTimeStamp.size() +
                          signatureTimeStamp.size());

      XAdESObject.snprintf(U_XADES_TEMPLATE,
                           U_STRING_TO_TRACE(signedProperties),
                           U_STRING_TO_TRACE(unsignedSignatureProperties),
                           U_STRING_TO_TRACE(archiveTimeStamp),
                           U_STRING_TO_TRACE(signatureTimeStamp));
      // ---------------------------------------------------------------------------------------------------------------

      // ---------------------------------------------------------------------------------------------------------------
      // 5. Compose the final XML document including the signatureValue, this time in non-canonicalized form.
      // ---------------------------------------------------------------------------------------------------------------
      UString output(U_CONSTANT_SIZE(U_XMLDSIG_TEMPLATE) + 8192U + 
                     SignedInfo.size() + SignatureValue.size() + XAdESObject.size());

      if (utility.ooffice)
         {
         OpenDocumentStart = U_STRING_FROM_CONSTANT("<document-signatures xmlns=\"urn:oasis:names:tc:opendocument:xmlns:digitalsignature:1.0\">");
         OpenDocumentEnd   = U_STRING_FROM_CONSTANT("</document-signatures>");
         }

      output.snprintf(U_XMLDSIG_TEMPLATE,
                        U_STRING_TO_TRACE(OpenDocumentStart),
                        U_STRING_TO_TRACE(SignedInfo),
                        U_STRING_TO_TRACE(SignatureValue),
                        U_STRING_TO_TRACE(KeyInfo),
                        U_STRING_TO_TRACE(XAdESObject),
                        U_STRING_TO_TRACE(OpenDocumentEnd));
      // ---------------------------------------------------------------------------------------------------------------

      // ---------------------------------------------------------------------------------------------------------------
      // check for OOffice or MS-Word document...
      // ---------------------------------------------------------------------------------------------------------------
      utility.outputDocument(output);
      // ---------------------------------------------------------------------------------------------------------------
      }

private:
   bool xades_c;
   uint32_t num_ca;
   int alg, num_args;
   long X509SerialNumber, signing_time;
   UFileConfig cfg;
   UXAdESUtility utility;
   UVector<UCertificate*> vec_ca;
   UString cfg_str, str_CApath, digest_algorithm, canonicalization_algorithm, claimed_role, document, production_place_city,
           production_place_state_or_province, production_place_postal_code, production_place_country_name, uri,
           data_object_format_mimetype, signature_algorithm, to_digest, DataObjectFormat, XAdESObject, XAdESReference,
           X509IssuerName, X509SubjectName, X509Certificate, signedProperties, signature_timestamp, unsignedSignatureProperties,
           unsignedSignaturePropertiesC14N, OpenDocumentStart, OpenDocumentEnd;

   U_APPLICATION_PRIVATE
};

U_MAIN
