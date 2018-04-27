// cspclient_rpc.cpp

#include <ulib/ssl/crl.h>
#include <ulib/ui/dialog.h>
#include <ulib/file_config.h>
#include <ulib/ssl/certificate.h>
#include <ulib/ssl/net/sslsocket.h>
#include <ulib/net/rpc/rpc_client.h>

#undef  PACKAGE
#define PACKAGE "cspclient_rpc"

#include "./usage.txt"

#include <ulib/application.h>

template <class T> class UClientCSP : public URPCClient<T> {
public:

   // COSTRUTTORE

   explicit UClientCSP(UFileConfig* cfg) : URPCClient<T>(cfg) {}
   virtual ~UClientCSP()                                      {}

   // OBJECT FOR METHOD REQUEST

   class CSP_CA : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString directory, openssl_cnf;
      uint32_t days;

      CSP_CA()
         {
         U_TRACE_CTOR(5, CSP_CA, "")

         days                    = 0;
         URPCMethod::method_name = U_STRING_FROM_CONSTANT("CREA");
         }

      virtual ~CSP_CA()
         {
         U_TRACE_DTOR(5, CSP_CA)
         }

      // Transforms the method into something that RPC servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "CSP_CA::encode()")

         U_RPC_ENCODE_ARG(directory);
         U_RPC_ENCODE_ARG(days);
         U_RPC_ENCODE_ARG(openssl_cnf);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class CSP_LIST_CA : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:

      CSP_LIST_CA()
         {
         U_TRACE_CTOR(5, CSP_LIST_CA, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("LST1");
         }

      virtual ~CSP_LIST_CA()
         {
         U_TRACE_DTOR(5, CSP_LIST_CA)
         }

      virtual void encode()
         {
         U_TRACE(5, "CSP_LIST_CA::encode()")
         }
   };

   // OBJECT FOR METHOD REQUEST

   class CSP_SIGN_P10 : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name, pkcs10, policy;

      CSP_SIGN_P10()
         {
         U_TRACE_CTOR(5, CSP_SIGN_P10, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("SIGP");
         }

      virtual ~CSP_SIGN_P10()
         {
         U_TRACE_DTOR(5, CSP_SIGN_P10)
         }

      // Transforms the method into something that RPC servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "CSP_SIGN_P10::encode()")

         U_RPC_ENCODE_ARG(name);
         U_RPC_ENCODE_ARG(pkcs10);
         U_RPC_ENCODE_ARG(policy);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class CSP_SIGN_SPKAC : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name, spkac, policy;

      CSP_SIGN_SPKAC()
         {
         U_TRACE_CTOR(5, CSP_SIGN_SPKAC, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("SIGK");
         }

      virtual ~CSP_SIGN_SPKAC()
         {
         U_TRACE_DTOR(5, CSP_SIGN_SPKAC)
         }

      // Transforms the method into something that RPC servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "CSP_SIGN_SPKAC::encode()")

         U_RPC_ENCODE_ARG(name);
         U_RPC_ENCODE_ARG(spkac);
         U_RPC_ENCODE_ARG(policy);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class CSP_LIST_CERTS : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name;
      uint32_t compress;

      CSP_LIST_CERTS()
         {
         U_TRACE_CTOR(5, CSP_LIST_CERTS, "")

         compress                = 0;
         URPCMethod::method_name = U_STRING_FROM_CONSTANT("LST2");
         }

      virtual ~CSP_LIST_CERTS()
         {
         U_TRACE_DTOR(5, CSP_LIST_CERTS)
         }

      // Transforms the method into something that RPC servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "CSP_LIST_CERTS::encode()")

         U_RPC_ENCODE_ARG(name);
         U_RPC_ENCODE_ARG(compress);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class CSP_REMOVE_CERT : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name, serial;

      CSP_REMOVE_CERT()
         {
         U_TRACE_CTOR(5, CSP_REMOVE_CERT, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("DELC");
         }

      virtual ~CSP_REMOVE_CERT()
         {
         U_TRACE_DTOR(5, CSP_REMOVE_CERT)
         }

      // Transforms the method into something that RPC servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "CSP_REMOVE_CERT::encode()")

         U_RPC_ENCODE_ARG(name);
         U_RPC_ENCODE_ARG(serial);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class CSP_ZERO_CERTS : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name;

      CSP_ZERO_CERTS()
         {
         U_TRACE_CTOR(5, CSP_ZERO_CERTS, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("ZERO");
         }

      virtual ~CSP_ZERO_CERTS()
         {
         U_TRACE_DTOR(5, CSP_ZERO_CERTS)
         }

      // Transforms the method into something that RPC servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "CSP_ZERO_CERTS::encode()")

         U_RPC_ENCODE_ARG(name);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class CSP_EMIT_CRL : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name;

      CSP_EMIT_CRL()
         {
         U_TRACE_CTOR(5, CSP_EMIT_CRL, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("EMIT");
         }

      virtual ~CSP_EMIT_CRL()
         {
         U_TRACE_DTOR(5, CSP_EMIT_CRL)
         }

      // Transforms the method into something that RPC servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "CSP_EMIT_CRL::encode()")

         U_RPC_ENCODE_ARG(name);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class CSP_GET_CRL : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name;

      CSP_GET_CRL()
         {
         U_TRACE_CTOR(5, CSP_GET_CRL, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("GETL");
         }

      virtual ~CSP_GET_CRL()
         {
         U_TRACE_DTOR(5, CSP_GET_CRL)
         }

      // Transforms the method into something that RPC servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "CSP_GET_CRL::encode()")

         U_RPC_ENCODE_ARG(name);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class CSP_GET_CA : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name;

      CSP_GET_CA()
         {
         U_TRACE_CTOR(5, CSP_GET_CA, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("GETC");
         }

      virtual ~CSP_GET_CA()
         {
         U_TRACE_DTOR(5, CSP_GET_CA)
         }

      // Transforms the method into something that RPC servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "CSP_GET_CA::encode()")

         U_RPC_ENCODE_ARG(name);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class CSP_REVOKE_CERT : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name, serial;

      CSP_REVOKE_CERT()
         {
         U_TRACE_CTOR(5, CSP_REVOKE_CERT, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("REVK");
         }

      virtual ~CSP_REVOKE_CERT()
         {
         U_TRACE_DTOR(5, CSP_REVOKE_CERT)
         }

      // Transforms the method into something that RPC servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "CSP_REVOKE_CERT::encode()")

         U_RPC_ENCODE_ARG(name);
         U_RPC_ENCODE_ARG(serial);
         }
   };

   CSP_CA          m_CSP_CA;
   CSP_LIST_CA     m_CSP_LIST_CA;
   CSP_SIGN_P10    m_CSP_SIGN_P10;
   CSP_SIGN_SPKAC  m_CSP_SIGN_SPKAC;
   CSP_LIST_CERTS  m_CSP_LIST_CERTS;
   CSP_REMOVE_CERT m_CSP_REMOVE_CERT;
   CSP_ZERO_CERTS  m_CSP_ZERO_CERTS;

   CSP_EMIT_CRL    m_CSP_EMIT_CRL;
   CSP_GET_CRL     m_CSP_GET_CRL;
   CSP_GET_CA      m_CSP_GET_CA;
   CSP_REVOKE_CERT m_CSP_REVOKE_CERT;

   // SERVICES

   bool creatCA(const UString& n, uint32_t d, const UString& c) // 1
      {
      U_TRACE(5, "UClientCSP::creatCA(%.*S,%u,%.*S)", U_STRING_TO_TRACE(n), d, U_STRING_TO_TRACE(c))

      m_CSP_CA.directory   = n;
      m_CSP_CA.days        = d;
      m_CSP_CA.openssl_cnf = c;

      if (URPCClient<T>::processRequest(m_CSP_CA))
         {
         int result = URPCClient<T>::getResponse().strtol(); // Get the value of the element inside the response

         U_RETURN(result == 1);
         }

      U_RETURN(false);
      }

   UString listCA() // 2
      {
      U_TRACE(5, "UClientCSP::listCA()")

      UString result;

      if (URPCClient<T>::processRequest(m_CSP_LIST_CA))
         {
         result = URPCClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }

   UString signP10(const UString& n, const UString& k, const UString& p) // 3
      {
      U_TRACE(5, "UClientCSP::signP10(%.*S,%.*S,%.*S)", U_STRING_TO_TRACE(n), U_STRING_TO_TRACE(k), U_STRING_TO_TRACE(p))

      m_CSP_SIGN_P10.name   = n;
      m_CSP_SIGN_P10.pkcs10 = k;
      m_CSP_SIGN_P10.policy = p;

      UString result;

      if (URPCClient<T>::processRequest(m_CSP_SIGN_P10))
         {
         result = URPCClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }

   UString signSPKAC(const UString& n, const UString& k, const UString& p) // 4
      {
      U_TRACE(5, "UClientCSP::signSPKAC(%.*S,%.*S,%.*S)", U_STRING_TO_TRACE(n), U_STRING_TO_TRACE(k), U_STRING_TO_TRACE(p))

      m_CSP_SIGN_SPKAC.name   = n;
      m_CSP_SIGN_SPKAC.spkac  = k;
      m_CSP_SIGN_SPKAC.policy = p;

      UString result;

      if (URPCClient<T>::processRequest(m_CSP_SIGN_SPKAC))
         {
         result = URPCClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }

   UString listCerts(const UString& name, uint32_t compress) // 5
      {
      U_TRACE(5, "UClientCSP::listCerts(%.*S,%u)", U_STRING_TO_TRACE(name), compress)

      m_CSP_LIST_CERTS.name     = name;
      m_CSP_LIST_CERTS.compress = compress;

      UString result;

      if (URPCClient<T>::processRequest(m_CSP_LIST_CERTS))
         {
         result = URPCClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }

   bool removeCert(const UString& n, const UString& s) // 6
      {
      U_TRACE(5, "UClientCSP::removeCert(%.*S,%.*S)", U_STRING_TO_TRACE(n), U_STRING_TO_TRACE(s))

      m_CSP_REMOVE_CERT.name   = n;
      m_CSP_REMOVE_CERT.serial = s;

      if (URPCClient<T>::processRequest(m_CSP_REMOVE_CERT))
         {
         int result = URPCClient<T>::getResponse().strtol(); // Get the value of the element inside the response

         U_RETURN(result == 1);
         }

      U_RETURN(false);
      }

   bool zeroCerts(const UString& name) // 7
      {
      U_TRACE(5, "UClientCSP::zeroCerts(%.*S)", U_STRING_TO_TRACE(name))

      m_CSP_ZERO_CERTS.name = name;

      if (URPCClient<T>::processRequest(m_CSP_ZERO_CERTS))
         {
         int result = URPCClient<T>::getResponse().strtol(); // Get the value of the element inside the response

         U_RETURN(result == 1);
         }

      U_RETURN(false);
      }

   bool emitCRL(const UString& name) // 8
      {
      U_TRACE(5, "UClientCSP::emitCRL(%.*S)", U_STRING_TO_TRACE(name))

      m_CSP_EMIT_CRL.name = name;

      if (URPCClient<T>::processRequest(m_CSP_EMIT_CRL))
         {
         int result = URPCClient<T>::getResponse().strtol(); // Get the value of the element inside the response

         U_RETURN(result == 1);
         }

      U_RETURN(false);
      }

   UString getCRL(const UString& name) // 9
      {
      U_TRACE(5, "UClientCSP::getCRL(%.*S)", U_STRING_TO_TRACE(name))

      m_CSP_GET_CRL.name = name;

      UString result;

      if (URPCClient<T>::processRequest(m_CSP_GET_CRL))
         {
         result = URPCClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }

   UString getCA(const UString& name) // 10
      {
      U_TRACE(5, "UClientCSP::getCA(%.*S)", U_STRING_TO_TRACE(name))

      m_CSP_GET_CA.name = name;

      UString result;

      if (URPCClient<T>::processRequest(m_CSP_GET_CA))
         {
         result = URPCClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }

   bool revokeCert(const UString& n, const UString& s) // 11 
      {
      U_TRACE(5, "UClientCSP::revokeCert(%.*S,%.*S)", U_STRING_TO_TRACE(n), U_STRING_TO_TRACE(s))

      m_CSP_REVOKE_CERT.name   = n;
      m_CSP_REVOKE_CERT.serial = s;

      if (URPCClient<T>::processRequest(m_CSP_REVOKE_CERT))
         {
         int result = URPCClient<T>::getResponse().strtol(); // Get the value of the element inside the response

         U_RETURN(result == 1);
         }

      U_RETURN(false);
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

      // manage options

      UString cfg_str;
      UFileConfig cfg;

      if (UApplication::isOptions()) cfg_str = opt['c'];

      // manage arg operation

      const char* method = argv[optind++];

      // manage file configuration

      if (cfg_str.empty()) cfg_str = U_STRING_FROM_CONSTANT("cspclient.cfg");

      cfg.UFile::setPath(cfg_str);

      // -----------------------------------------------------------------------------------------------
      // client CSP - configuration parameters
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
      //                                    SSL_VERIFY_FAIL_IF_NO_PEER_CERT=2,
      //                                    SSL_VERIFY_CLIENT_ONCE=4)
      // -----------------------------------------------------------------------------------------------

      client = new UClientCSP<USSLSocket>(&cfg);

#  include "./main.cpp"
      }

private:
   UClientCSP<USSLSocket>* client;
};

U_MAIN
