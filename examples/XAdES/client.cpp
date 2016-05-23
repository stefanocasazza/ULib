// client.cpp

#include <ulib/file_config.h>
#include <ulib/net/tcpsocket.h>
#include <ulib/xml/soap/soap_client.h>

#undef  PACKAGE
#define PACKAGE "client"

#undef  ARGS
#define ARGS "type_of_signature <parameters>"

#define PURPOSE \
"client interface to server SOAP managing XAdES\n" \
"----------------------------------------------------------------------------------------------------------------------------------\n" \
"List of XAdES signatures:\n" \
"\n" \
" 1 - BES - parameters: <URI> <X509> <key> [digest] [SigningTime] [ClaimedRole] [ProductionPlace...] \n" \
" 2 - C   - parameters: <URI> <X509> <key> [digest] [SigningTime] [ClaimedRole] [ProductionPlace...] [CAStore] [SignatureTimeStamp]\n" \
" 3 - L   - parameters: <XAdES-C> [ArchiveTimeStamp] [Schema]\n" \
"----------------------------------------------------------------------------------------------------------------------------------"

#define U_OPTIONS \
"option c config 1 'path of configuration file' ''\n"

#include <ulib/application.h>

#define U_DATA_URI                           (const char*)(num_args >= 1  ?      argv[optind+0]  : "")
#define U_X509                               (const char*)(num_args >= 2  ?      argv[optind+1]  : "")
#define U_KEY_HANDLE                         (const char*)(num_args >= 3  ?      argv[optind+2]  : "")
#define U_DIGEST_ALGORITHM                   (const char*)(num_args >= 4  ?      argv[optind+3]  : "")
#define U_SIGNING_TIME                                    (num_args >= 5  ? atoi(argv[optind+4]) : 0)
#define U_CLAIMED_ROLE                       (const char*)(num_args >= 6  ?      argv[optind+5]  : "")
#define U_PRODUCTION_PLACE_CITY              (const char*)(num_args >= 7  ?      argv[optind+6]  : "")
#define U_PRODUCTION_PLACE_STATE_OR_PROVINCE (const char*)(num_args >= 8  ?      argv[optind+7]  : "")
#define U_PRODUCTION_PLACE_POSTAL_CODE       (const char*)(num_args >= 9  ?      argv[optind+8]  : "")
#define U_PRODUCTION_PLACE_COUNTRY_NAME      (const char*)(num_args >= 10 ?      argv[optind+9]  : "")
#define U_CA_STORE                           (const char*)(num_args >= 11 ?      argv[optind+10] : "")
#define U_SIGNATURE_TIMESTAMP                (const char*)(num_args >= 12 ?      argv[optind+11] : "")

#define U_ARCHIVE_TIMESTAMP                  (const char*)(num_args >= 0  ?      argv[optind+0]  : "")
#define U_SCHEMA                             (const char*)(num_args >= 1  ?      argv[optind+1]  : "")

template <class T> class UClientXAdES : public USOAPClient<T> {
public:

   // COSTRUTTORE

   explicit UClientXAdES(UFileConfig* cfg) : USOAPClient<T>(cfg) {}
   virtual ~UClientXAdES()                                       {}

   // OBJECT FOR METHOD REQUEST

   class XAdES_BES : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:

      // ---------------------------------------------------------------------------------------------------------------
      // Firma dati: XAdES-BES
      // ---------------------------------------------------------------------------------------------------------------
      // DATA                                = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" stdin (base64 encoded)
      // DATA_URI                            = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input
      // X509                                = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (formato PEM)
      // KEY_HANDLE                          = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input
      // DIGEST_ALGORITHM                    = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // SIGNING_TIME                        = boolean "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // CLAIMED_ROLE                        = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // PRODUCTION_PLACE_CITY               = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // PRODUCTION_PLACE_STATE_OR_PROVINCE  = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // PRODUCTION_PLACE_POSTAL_CODE        = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // PRODUCTION_PLACE_COUNTRY_NAME       = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // ---------------------------------------------------------------------------------------------------------------
      // result = xml signed (base64 encoded)
      // ---------------------------------------------------------------------------------------------------------------

      uint32_t SIGNING_TIME;
      UString DATA, DATA_URI, X509, KEY_HANDLE, DIGEST_ALGORITHM, CLAIMED_ROLE, PRODUCTION_PLACE_CITY,
              PRODUCTION_PLACE_STATE_OR_PROVINCE, PRODUCTION_PLACE_POSTAL_CODE, PRODUCTION_PLACE_COUNTRY_NAME;

      XAdES_BES()
         {
         U_TRACE_REGISTER_OBJECT(5, XAdES_BES, "")

         SIGNING_TIME            = 0;
         URPCMethod::method_name = U_STRING_FROM_CONSTANT("XAdES-BES");
         }

      virtual ~XAdES_BES()
         {
         U_TRACE_UNREGISTER_OBJECT(5, XAdES_BES)
         }

      // Transforms the method into something that SOAP servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "XAdES_BES::encode()")

         U_SOAP_ENCB64_ARG(DATA);
         U_SOAP_ENCODE_ARG(DATA_URI);
         U_SOAP_ENCODE_ARG(X509);
         U_SOAP_ENCODE_ARG(KEY_HANDLE);
         U_SOAP_ENCODE_ARG(DIGEST_ALGORITHM);
         U_SOAP_ENCODE_ARG(SIGNING_TIME);
         U_SOAP_ENCODE_ARG(CLAIMED_ROLE);
         U_SOAP_ENCODE_ARG(PRODUCTION_PLACE_CITY);
         U_SOAP_ENCODE_ARG(PRODUCTION_PLACE_STATE_OR_PROVINCE);
         U_SOAP_ENCODE_ARG(PRODUCTION_PLACE_POSTAL_CODE);
         U_SOAP_ENCODE_ARG(PRODUCTION_PLACE_COUNTRY_NAME);
         }
   };

   class XAdES_C : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:

      // ---------------------------------------------------------------------------------------------------------------
      // Firma dati: XAdES-C
      // ---------------------------------------------------------------------------------------------------------------
      // DATA                                = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" stdin (base64 encoded)
      // DATA_URI                            = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input
      // X509                                = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (formato PEM)
      // KEY_HANDLE                          = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input
      // DIGEST_ALGORITHM                    = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // SIGNING_TIME                        = boolean "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // CLAIMED_ROLE                        = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // PRODUCTION_PLACE_CITY               = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // PRODUCTION_PLACE_STATE_OR_PROVINCE  = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // PRODUCTION_PLACE_POSTAL_CODE        = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // PRODUCTION_PLACE_COUNTRY_NAME       = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // CA_STORE                            = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // SIGNATURE_TIMESTAMP                 = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // ---------------------------------------------------------------------------------------------------------------
      // result = xml signed (base64 encoded)
      // ---------------------------------------------------------------------------------------------------------------

      uint32_t SIGNING_TIME;
      UString DATA, DATA_URI, X509, KEY_HANDLE, DIGEST_ALGORITHM, CLAIMED_ROLE, PRODUCTION_PLACE_CITY,
              PRODUCTION_PLACE_STATE_OR_PROVINCE, PRODUCTION_PLACE_POSTAL_CODE, PRODUCTION_PLACE_COUNTRY_NAME,
              CA_STORE, SIGNATURE_TIMESTAMP;

      XAdES_C()
         {
         U_TRACE_REGISTER_OBJECT(5, XAdES_C, "")

         SIGNING_TIME            = 0;
         URPCMethod::method_name = U_STRING_FROM_CONSTANT("XAdES-C");
         }

      virtual ~XAdES_C()
         {
         U_TRACE_UNREGISTER_OBJECT(5, XAdES_C)
         }

      // Transforms the method into something that SOAP servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "XAdES_C::encode()")

         U_SOAP_ENCB64_ARG(DATA);
         U_SOAP_ENCODE_ARG(DATA_URI);
         U_SOAP_ENCODE_ARG(X509);
         U_SOAP_ENCODE_ARG(KEY_HANDLE);
         U_SOAP_ENCODE_ARG(DIGEST_ALGORITHM);
         U_SOAP_ENCODE_ARG(SIGNING_TIME);
         U_SOAP_ENCODE_ARG(CLAIMED_ROLE);
         U_SOAP_ENCODE_ARG(PRODUCTION_PLACE_CITY);
         U_SOAP_ENCODE_ARG(PRODUCTION_PLACE_STATE_OR_PROVINCE);
         U_SOAP_ENCODE_ARG(PRODUCTION_PLACE_POSTAL_CODE);
         U_SOAP_ENCODE_ARG(PRODUCTION_PLACE_COUNTRY_NAME);
         U_SOAP_ENCODE_ARG(CA_STORE);
         U_SOAP_ENCODE_ARG(SIGNATURE_TIMESTAMP);
         }
   };

   class XAdES_L : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:

      // ---------------------------------------------------------------------------------------------------------------
      // Firma dati: XAdES-L
      // ---------------------------------------------------------------------------------------------------------------
      // XAdES-C           = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" stdin (base64 encoded)
      // ARCHIVE_TIMESTAMP = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // SCHEMA            = stringa "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" input (op)
      // ---------------------------------------------------------------------------------------------------------------
      // result = xml signed (base64 encoded)
      // ---------------------------------------------------------------------------------------------------------------

      UString DATA, ARCHIVE_TIMESTAMP, SCHEMA;

      XAdES_L()
         {
         U_TRACE_REGISTER_OBJECT(5, XAdES_L, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("XAdES-L");
         }

      virtual ~XAdES_L()
         {
         U_TRACE_UNREGISTER_OBJECT(5, XAdES_L)
         }

      // Transforms the method into something that SOAP servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "XAdES_L::encode()")

         U_SOAP_ENCB64_ARG(DATA);
         U_SOAP_ENCODE_ARG(ARCHIVE_TIMESTAMP);
         U_SOAP_ENCODE_ARG(SCHEMA);
         }
   };

   XAdES_BES m_XAdES_BES;
   XAdES_C   m_XAdES_C;
   XAdES_L   m_XAdES_L;

   // SERVICES

   UString creatBES(const UString& data, const char* data_uri, const UString& x509, const char* key_handle,
                    const char* digest_algorithm, uint32_t signing_time, const char* claimed_role,
                    const char* production_place_city, const char* production_place_state_or_province,
                    const char* production_place_postal_code, const char* production_place_country_name) // 1
      {
      U_TRACE(5, "UClientXAdES::creatBES(%.*S,%S,%.*S,%S,%S,%u,%S,%S,%S,%S,%S)", U_STRING_TO_TRACE(data), data_uri, U_STRING_TO_TRACE(x509),
                  key_handle, digest_algorithm, signing_time, claimed_role, production_place_city,
                  production_place_state_or_province, production_place_postal_code, production_place_country_name)

      m_XAdES_BES.DATA                               = data;
      m_XAdES_BES.DATA_URI                           = data_uri;
      m_XAdES_BES.X509                               = x509;
      m_XAdES_BES.KEY_HANDLE                         = key_handle;
      m_XAdES_BES.DIGEST_ALGORITHM                   = digest_algorithm;
      m_XAdES_BES.SIGNING_TIME                       = signing_time;
      m_XAdES_BES.CLAIMED_ROLE                       = claimed_role;
      m_XAdES_BES.PRODUCTION_PLACE_CITY              = production_place_city;
      m_XAdES_BES.PRODUCTION_PLACE_STATE_OR_PROVINCE = production_place_state_or_province;
      m_XAdES_BES.PRODUCTION_PLACE_POSTAL_CODE       = production_place_postal_code;
      m_XAdES_BES.PRODUCTION_PLACE_COUNTRY_NAME      = production_place_country_name;

      if (USOAPClient<T>::processRequest(m_XAdES_BES))
         {
         UString result = USOAPClient<T>::getResponse(); // Get the value of the element inside the response

         UString _buffer(result.size());

         UBase64::decode(result, _buffer);

         if (_buffer) U_RETURN_STRING(_buffer);
         }

      return UString::getStringNull();
      }

   UString creatC(const UString& data, const char* data_uri, const UString& x509, const char* key_handle,
                  const char* digest_algorithm, uint32_t signing_time, const char* claimed_role,
                  const char* production_place_city, const char* production_place_state_or_province,
                  const char* production_place_postal_code, const char* production_place_country_name,
                  const char* ca_store, const char* signature_timestamp) // 2
      {
      U_TRACE(5, "UClientXAdES::creatC(%.*S,%S)", U_STRING_TO_TRACE(data), data_uri)

      m_XAdES_C.DATA                               = data;
      m_XAdES_C.DATA_URI                           = data_uri;
      m_XAdES_C.X509                               = x509;
      m_XAdES_C.KEY_HANDLE                         = key_handle;
      m_XAdES_C.DIGEST_ALGORITHM                   = digest_algorithm;
      m_XAdES_C.SIGNING_TIME                       = signing_time;
      m_XAdES_C.CLAIMED_ROLE                       = claimed_role;
      m_XAdES_C.PRODUCTION_PLACE_CITY              = production_place_city;
      m_XAdES_C.PRODUCTION_PLACE_STATE_OR_PROVINCE = production_place_state_or_province;
      m_XAdES_C.PRODUCTION_PLACE_POSTAL_CODE       = production_place_postal_code;
      m_XAdES_C.PRODUCTION_PLACE_COUNTRY_NAME      = production_place_country_name;
      m_XAdES_C.CA_STORE                           = ca_store;
      m_XAdES_C.SIGNATURE_TIMESTAMP                = signature_timestamp;

      if (USOAPClient<T>::processRequest(m_XAdES_C))
         {
         UString result = USOAPClient<T>::getResponse(); // Get the value of the element inside the response

         UString _buffer(result.size());

         UBase64::decode(result, _buffer);

         if (_buffer) U_RETURN_STRING(_buffer);
         }

      return UString::getStringNull();
      }

   UString creatL(const UString& data, const char* archive_timestamp, const char* schema) // 2
      {
      U_TRACE(5, "UClientXAdES::creatL(%.*S,%S,%S)", U_STRING_TO_TRACE(data), archive_timestamp, schema)

      m_XAdES_L.DATA              = data;
      m_XAdES_L.ARCHIVE_TIMESTAMP = archive_timestamp;
      m_XAdES_L.SCHEMA            = schema;

      if (USOAPClient<T>::processRequest(m_XAdES_L))
         {
         UString result = USOAPClient<T>::getResponse(); // Get the value of the element inside the response

         UString _buffer(result.size());

         UBase64::decode(result, _buffer);

         if (_buffer) U_RETURN_STRING(_buffer);
         }

      return UString::getStringNull();
      }
};

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE(5, "Application::Application()")

      client = 0;
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      delete client;
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage options

      UString cfg_str;
      UFileConfig cfg;

      if (UApplication::isOptions()) cfg_str = opt['c'];

      // manage arg operation

      const char* method = argv[optind++];

      if (method == 0) usage();

      int op = atoi(method), num_args = (argc - optind);

      U_INTERNAL_DUMP("optind = %d num_args = %d", optind, num_args)

      // manage file configuration

      if (cfg_str.empty()) cfg_str = U_STRING_FROM_CONSTANT("XAdESclient.cfg");

      cfg.UFile::setPath(cfg_str);

      // -----------------------------------------------------------------------------------------------
      // client XAdES - configuration parameters
      // -----------------------------------------------------------------------------------------------
      // ENABLE_IPV6  flag to indicate use of ipv6
      // SERVER       host name or ip address for server
      // PORT         port number for the server
      // -----------------------------------------------------------------------------------------------

      client = new UClientXAdES<UTCPSocket>(&cfg);

      UString result;

      UApplication::exit_value = 1;

      if (client->connect())
         {
         switch (op)
            {
            case 1: // Firma dati: XAdES-BES
               {
               UString x    = UFile::contentOf(U_X509),
                       data = UFile::contentOf(U_DATA_URI);

               result = client->creatBES(data, U_DATA_URI, x, U_KEY_HANDLE, U_DIGEST_ALGORITHM,
                                         U_SIGNING_TIME, U_CLAIMED_ROLE, U_PRODUCTION_PLACE_CITY,
                                         U_PRODUCTION_PLACE_STATE_OR_PROVINCE, U_PRODUCTION_PLACE_POSTAL_CODE,
                                         U_PRODUCTION_PLACE_COUNTRY_NAME);
               }
            break;

            case 2: // Firma dati: XAdES-C
               {
               UString x    = UFile::contentOf(U_X509),
                       data = UFile::contentOf(U_DATA_URI);

               result = client->creatC(data, U_DATA_URI, x, U_KEY_HANDLE, U_DIGEST_ALGORITHM,
                                       U_SIGNING_TIME, U_CLAIMED_ROLE, U_PRODUCTION_PLACE_CITY,
                                       U_PRODUCTION_PLACE_STATE_OR_PROVINCE, U_PRODUCTION_PLACE_POSTAL_CODE,
                                       U_PRODUCTION_PLACE_COUNTRY_NAME,
                                       U_CA_STORE, U_SIGNATURE_TIMESTAMP);
               }
            break;

            case 3: // Archiviazione XAdES-C: XAdES-L
               {
               UString data = UFile::contentOf(U_DATA_URI);

               result = client->creatL(data, U_ARCHIVE_TIMESTAMP, U_SCHEMA);
               }
            break;

            default:
               U_ERROR("type_of_signature not valid");
            break;
            }

         if (result)
            {
            std::cout.write(result.data(), result.size());

            UApplication::exit_value = 0;
            }

         if (UApplication::exit_value == 1)
            {
            result = client->getResponse();

            if (result) U_WARNING("%v", result.rep);
            }
         }

      client->closeLog();
      }

private:
   UClientXAdES<UTCPSocket>* client;
};

U_MAIN
