// rsignclient_rpc.cpp

#include <ulib/file_config.h>
#include <ulib/ssl/net/sslsocket.h>
#include <ulib/net/rpc/rpc_client.h>

#undef  PACKAGE
#define PACKAGE "rsignclient_rpc"
#undef  ARGS
#define ARGS ""

#define U_OPTIONS \
"purpose \"client for interface to SIGN server\"\n" \
"option c config 1 \"path of configuration file\" \"\"\n" \
"option k inkey 1 \"path of private key file - format PEM\" \"\"\n"

#include <ulib/application.h>

template <class T> class UClientRSIGN : public URPCClient<T> {
public:

   // COSTRUTTORE

   explicit UClientRSIGN(UFileConfig* cfg) : URPCClient<T>(cfg) {}
   virtual ~UClientRSIGN()                                      {}

   // OBJECT FOR METHOD REQUEST

   class RSIGN_SIGN : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString data, key;

      RSIGN_SIGN()
         {
         U_TRACE_CTOR(5, RSIGN_SIGN, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("SIG1");
         }

      virtual ~RSIGN_SIGN()
         {
         U_TRACE_DTOR(5, RSIGN_SIGN)
         }

      // Transforms the method into something that RPC servers and clients can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "RSIGN_SIGN::encode()")

         U_RPC_ENCODE_ARG(data);
         U_RPC_ENCODE_ARG(key);
         }
   };

   RSIGN_SIGN m_RSIGN_SIGN;

   // SERVICES

   UString signData(const UString& n, const UString& k) // 1
      {
      U_TRACE(5, "UClientRSIGN::signData(%.*S,%.*S)", U_STRING_TO_TRACE(n), U_STRING_TO_TRACE(k))

      m_RSIGN_SIGN.data = n;
      m_RSIGN_SIGN.key  = k;

      UString result;

      if (URPCClient<T>::processRequest(m_RSIGN_SIGN))
         {
         result = URPCClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }
};

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE(5, "Application::Application()")

      client = U_NULLPTR;
      }

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      U_DELETE(client)
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      UString::str_allocate(STR_ALLOCATE_SOAP);

      // manage arg operation

      // manage options

      UFileConfig cfg;
      UString cfg_str, cfg_key;

      if (UApplication::isOptions())
         {
         cfg_str = opt['c'];
         cfg_key = opt['k'];
         }

      // manage file configuration

      if (cfg_str.empty()) cfg_str = U_STRING_FROM_CONSTANT("rsignclient.cfg");

      cfg.UFile::setPath(cfg_str);

      // -----------------------------------------------------------------------------------------------
      // client RSIGN - configuration parameters
      // -----------------------------------------------------------------------------------------------
      // ENABLE_IPV6  flag to indicate use of ipv6
      // SERVER       host name or ip address for server
      // PORT         port number for the server
      // CERT_FILE    certificate of client
      // KEY_FILE     private key of client
      // PASSWORD     password for private key of client
      // CA_FILE      locations of trusted CA certificates used in the verification
      // CA_PATH      locations of trusted CA certificates used in the verification
      // VERIFY_MODE  mode of verification (SSL_VERIFY_NONE=0, SSL_VERIFY_PEER=1,
      //                                    SSL_VERIFY_FAIL_IF_NO_PEER_CERT=2, SSL_VERIFY_CLIENT_ONCE=4)
      // -----------------------------------------------------------------------------------------------

      client = new UClientRSIGN<USSLSocket>(&cfg);

      UApplication::exit_value = 1;

      if (client->connect())
         {
         // int ns__RSIGN_SIGN(const char* ca, const char* policy, const char* pkcs10, char** response);

         UString result, data(U_CAPACITY),
                 key = UFile::contentOf(cfg_key);

         UServices::readEOF(STDIN_FILENO, data);
         
         result = client->signData(data, key);

         if (result.empty() == false)
            {
            (void) write(1, U_STRING_TO_PARAM(result));

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
   UClientRSIGN<USSLSocket>* client;
};

U_MAIN
