// lcspclient.cpp

#include <ulib/ssl/crl.h>
#include <ulib/ui/dialog.h>
#include <ulib/file_config.h>
#include <ulib/net/unixsocket.h>
#include <ulib/ssl/certificate.h>
#include <ulib/xml/soap/soap_client.h>

#undef  PACKAGE
#define PACKAGE "lcspclient"

#include "../csp/usage.txt"

#include <ulib/application.h>

template <class T> class UClientLCSP : public USOAPClient<T> {
public:

   // COSTRUTTORE

            UClientLCSP(UFileConfig* cfg) : USOAPClient<T>(cfg) {}
   virtual ~UClientLCSP()                                       {}

   // OBJECT FOR METHOD REQUEST

   class LCSP_CA : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString directory, openssl_cnf;
      uint32_t days;

      LCSP_CA()
         {
         U_TRACE_REGISTER_OBJECT(5, LCSP_CA, "")

         days                    = 0;
         URPCMethod::method_name = U_STRING_FROM_CONSTANT("CREA");
         }

      virtual ~LCSP_CA()
         {
         U_TRACE_UNREGISTER_OBJECT(5, LCSP_CA)
         }

      // Transforms the method into something that SOAP servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "LCSP_CA::encode()")

         U_SOAP_ENCODE_ARG(directory);
         U_SOAP_ENCODE_ARG(days);
         U_SOAP_ENCODE_ARG(openssl_cnf);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class LCSP_LIST_CA : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:

      LCSP_LIST_CA()
         {
         U_TRACE_REGISTER_OBJECT(5, LCSP_LIST_CA, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("LST1");
         }

      virtual ~LCSP_LIST_CA()
         {
         U_TRACE_UNREGISTER_OBJECT(5, LCSP_LIST_CA)
         }

      virtual void encode()
         {
         U_TRACE(5, "LCSP_LIST_CA::encode()")
         }
   };

   // OBJECT FOR METHOD REQUEST

   class LCSP_SIGN_P10 : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name, pkcs10, policy;

      LCSP_SIGN_P10()
         {
         U_TRACE_REGISTER_OBJECT(5, LCSP_SIGN_P10, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("SIGP");
         }

      virtual ~LCSP_SIGN_P10()
         {
         U_TRACE_UNREGISTER_OBJECT(5, LCSP_SIGN_P10)
         }

      // Transforms the method into something that SOAP servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "LCSP_SIGN_P10::encode()")

         U_SOAP_ENCODE_ARG(name);
         U_SOAP_ENCODE_ARG(pkcs10);
         U_SOAP_ENCODE_ARG(policy);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class LCSP_SIGN_SPKAC : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name, spkac, policy;

      LCSP_SIGN_SPKAC()
         {
         U_TRACE_REGISTER_OBJECT(5, LCSP_SIGN_SPKAC, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("SIGK");
         }

      virtual ~LCSP_SIGN_SPKAC()
         {
         U_TRACE_UNREGISTER_OBJECT(5, LCSP_SIGN_SPKAC)
         }

      // Transforms the method into something that SOAP servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "LCSP_SIGN_SPKAC::encode()")

         U_SOAP_ENCODE_ARG(name);
         U_SOAP_ENCODE_ARG(spkac);
         U_SOAP_ENCODE_ARG(policy);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class LCSP_LIST_CERTS : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name;
      uint32_t compress;

      LCSP_LIST_CERTS()
         {
         U_TRACE_REGISTER_OBJECT(5, LCSP_LIST_CERTS, "")

         compress                = 0;
         URPCMethod::method_name = U_STRING_FROM_CONSTANT("LST2");
         }

      virtual ~LCSP_LIST_CERTS()
         {
         U_TRACE_UNREGISTER_OBJECT(5, LCSP_LIST_CERTS)
         }

      // Transforms the method into something that SOAP servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "LCSP_LIST_CERTS::encode()")

         U_SOAP_ENCODE_ARG(name);
         U_SOAP_ENCODE_ARG(compress);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class LCSP_REMOVE_CERT : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name, serial;

      LCSP_REMOVE_CERT()
         {
         U_TRACE_REGISTER_OBJECT(5, LCSP_REMOVE_CERT, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("DELC");
         }

      virtual ~LCSP_REMOVE_CERT()
         {
         U_TRACE_UNREGISTER_OBJECT(5, LCSP_REMOVE_CERT)
         }

      // Transforms the method into something that SOAP servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "LCSP_REMOVE_CERT::encode()")

         U_SOAP_ENCODE_ARG(name);
         U_SOAP_ENCODE_ARG(serial);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class LCSP_ZERO_CERTS : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name;

      LCSP_ZERO_CERTS()
         {
         U_TRACE_REGISTER_OBJECT(5, LCSP_ZERO_CERTS, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("ZERO");
         }

      virtual ~LCSP_ZERO_CERTS()
         {
         U_TRACE_UNREGISTER_OBJECT(5, LCSP_ZERO_CERTS)
         }

      // Transforms the method into something that SOAP servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "LCSP_ZERO_CERTS::encode()")

         U_SOAP_ENCODE_ARG(name);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class LCSP_EMIT_CRL : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name;

      LCSP_EMIT_CRL()
         {
         U_TRACE_REGISTER_OBJECT(5, LCSP_EMIT_CRL, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("EMIT");
         }

      virtual ~LCSP_EMIT_CRL()
         {
         U_TRACE_UNREGISTER_OBJECT(5, LCSP_EMIT_CRL)
         }

      // Transforms the method into something that SOAP servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "LCSP_EMIT_CRL::encode()")

         U_SOAP_ENCODE_ARG(name);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class LCSP_GET_CRL : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name;

      LCSP_GET_CRL()
         {
         U_TRACE_REGISTER_OBJECT(5, LCSP_GET_CRL, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("GETL");
         }

      virtual ~LCSP_GET_CRL()
         {
         U_TRACE_UNREGISTER_OBJECT(5, LCSP_GET_CRL)
         }

      // Transforms the method into something that SOAP servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "LCSP_GET_CRL::encode()")

         U_SOAP_ENCODE_ARG(name);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class LCSP_GET_CA : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name;

      LCSP_GET_CA()
         {
         U_TRACE_REGISTER_OBJECT(5, LCSP_GET_CA, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("GETC");
         }

      virtual ~LCSP_GET_CA()
         {
         U_TRACE_UNREGISTER_OBJECT(5, LCSP_GET_CA)
         }

      // Transforms the method into something that SOAP servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "LCSP_GET_CA::encode()")

         U_SOAP_ENCODE_ARG(name);
         }
   };

   // OBJECT FOR METHOD REQUEST

   class LCSP_REVOKE_CERT : public URPCMethod { // URPCMethod provides an interface for the things that methods most know how to do
   public:
      UString name, serial;

      LCSP_REVOKE_CERT()
         {
         U_TRACE_REGISTER_OBJECT(5, LCSP_REVOKE_CERT, "")

         URPCMethod::method_name = U_STRING_FROM_CONSTANT("REVK");
         }

      virtual ~LCSP_REVOKE_CERT()
         {
         U_TRACE_UNREGISTER_OBJECT(5, LCSP_REVOKE_CERT)
         }

      // Transforms the method into something that SOAP servers and client can send.
      // The encoder holds the actual data while the client hands data to be entered in

      virtual void encode()
         {
         U_TRACE(5, "LCSP_REVOKE_CERT::encode()")

         U_SOAP_ENCODE_ARG(name);
         U_SOAP_ENCODE_ARG(serial);
         }
   };

   LCSP_CA          m_LCSP_CA;
   LCSP_LIST_CA     m_LCSP_LIST_CA;
   LCSP_SIGN_P10    m_LCSP_SIGN_P10;
   LCSP_SIGN_SPKAC  m_LCSP_SIGN_SPKAC;
   LCSP_LIST_CERTS  m_LCSP_LIST_CERTS;
   LCSP_REMOVE_CERT m_LCSP_REMOVE_CERT;
   LCSP_ZERO_CERTS  m_LCSP_ZERO_CERTS;

   LCSP_EMIT_CRL    m_LCSP_EMIT_CRL;
   LCSP_GET_CRL     m_LCSP_GET_CRL;
   LCSP_GET_CA      m_LCSP_GET_CA;
   LCSP_REVOKE_CERT m_LCSP_REVOKE_CERT;

   // SERVICES

   bool creatCA(const UString& n, uint32_t d, const UString& c) // 1
      {
      U_TRACE(5, "UClientLCSP::creatCA(%.*S,%u,%.*S)", U_STRING_TO_TRACE(n), d, U_STRING_TO_TRACE(c))

      m_LCSP_CA.directory   = n;
      m_LCSP_CA.days        = d;
      m_LCSP_CA.openssl_cnf = c;

      if (USOAPClient<T>::processRequest(m_LCSP_CA))
         {
         int result = USOAPClient<T>::getResponse().strtol(); // Get the value of the element inside the response

         U_RETURN(result == 1);
         }

      U_RETURN(false);
      }

   UString listCA() // 2
      {
      U_TRACE(5, "UClientLCSP::listCA()")

      UString result;

      if (USOAPClient<T>::processRequest(m_LCSP_LIST_CA))
         {
         result = USOAPClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }

   UString signP10(const UString& n, const UString& k, const UString& p) // 3
      {
      U_TRACE(5, "UClientLCSP::signP10(%.*S,%.*S,%.*S)", U_STRING_TO_TRACE(n), U_STRING_TO_TRACE(k), U_STRING_TO_TRACE(p))

      m_LCSP_SIGN_P10.name   = n;
      m_LCSP_SIGN_P10.pkcs10 = k;
      m_LCSP_SIGN_P10.policy = p;

      UString result;

      if (USOAPClient<T>::processRequest(m_LCSP_SIGN_P10))
         {
         result = USOAPClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }

   UString signSPKAC(const UString& n, const UString& k, const UString& p) // 4
      {
      U_TRACE(5, "UClientLCSP::signSPKAC(%.*S,%.*S,%.*S)", U_STRING_TO_TRACE(n), U_STRING_TO_TRACE(k), U_STRING_TO_TRACE(p))

      m_LCSP_SIGN_SPKAC.name   = n;
      m_LCSP_SIGN_SPKAC.spkac  = k;
      m_LCSP_SIGN_SPKAC.policy = p;

      UString result;

      if (USOAPClient<T>::processRequest(m_LCSP_SIGN_SPKAC))
         {
         result = USOAPClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }

   UString listCerts(const UString& name, uint32_t compress) // 5
      {
      U_TRACE(5, "UClientLCSP::listCerts(%.*S,%u)", U_STRING_TO_TRACE(name), compress)

      m_LCSP_LIST_CERTS.name     = name;
      m_LCSP_LIST_CERTS.compress = compress;

      UString result;

      if (USOAPClient<T>::processRequest(m_LCSP_LIST_CERTS))
         {
         result = USOAPClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }

   bool removeCert(const UString& n, const UString& s) // 6
      {
      U_TRACE(5, "UClientLCSP::removeCert(%.*S,%.*S)", U_STRING_TO_TRACE(n), U_STRING_TO_TRACE(s))

      m_LCSP_REMOVE_CERT.name   = n;
      m_LCSP_REMOVE_CERT.serial = s;

      if (USOAPClient<T>::processRequest(m_LCSP_REMOVE_CERT))
         {
         int result = USOAPClient<T>::getResponse().strtol(); // Get the value of the element inside the response

         U_RETURN(result == 1);
         }

      U_RETURN(false);
      }

   bool zeroCerts(const UString& name) // 7
      {
      U_TRACE(5, "UClientLCSP::zeroCerts(%.*S)", U_STRING_TO_TRACE(name))

      m_LCSP_ZERO_CERTS.name = name;

      if (USOAPClient<T>::processRequest(m_LCSP_ZERO_CERTS))
         {
         int result = USOAPClient<T>::getResponse().strtol(); // Get the value of the element inside the response

         U_RETURN(result == 1);
         }

      U_RETURN(false);
      }

   bool emitCRL(const UString& name) // 8
      {
      U_TRACE(5, "UClientLCSP::emitCRL(%.*S)", U_STRING_TO_TRACE(name))

      m_LCSP_EMIT_CRL.name = name;

      if (USOAPClient<T>::processRequest(m_LCSP_EMIT_CRL))
         {
         int result = USOAPClient<T>::getResponse().strtol(); // Get the value of the element inside the response

         U_RETURN(result == 1);
         }

      U_RETURN(false);
      }

   UString getCRL(const UString& name) // 9
      {
      U_TRACE(5, "UClientLCSP::getCRL(%.*S)", U_STRING_TO_TRACE(name))

      m_LCSP_GET_CRL.name = name;

      UString result;

      if (USOAPClient<T>::processRequest(m_LCSP_GET_CRL))
         {
         result = USOAPClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }

   UString getCA(const UString& name) // 10
      {
      U_TRACE(5, "UClientLCSP::getCA(%.*S)", U_STRING_TO_TRACE(name))

      m_LCSP_GET_CA.name = name;

      UString result;

      if (USOAPClient<T>::processRequest(m_LCSP_GET_CA))
         {
         result = USOAPClient<T>::getResponse(); // Get the value of the element inside the response
         }

      U_RETURN_STRING(result);
      }

   bool revokeCert(const UString& n, const UString& s) // 11 
      {
      U_TRACE(5, "UClientLCSP::revokeCert(%.*S,%.*S)", U_STRING_TO_TRACE(n), U_STRING_TO_TRACE(s))

      m_LCSP_REVOKE_CERT.name   = n;
      m_LCSP_REVOKE_CERT.serial = s;

      if (USOAPClient<T>::processRequest(m_LCSP_REVOKE_CERT))
         {
         int result = USOAPClient<T>::getResponse().strtol(); // Get the value of the element inside the response

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

      // manage file configuration

      if (cfg_str.empty()) cfg_str = U_STRING_FROM_CONSTANT("lcspclient.cfg");

      cfg.UFile::setPath(cfg_str);

      // -----------------------------------------------------------------------------------------------
      // client LCSP - configuration parameters
      // -----------------------------------------------------------------------------------------------
      // NAME_SOCKET  name file for connecting to the server
      // -----------------------------------------------------------------------------------------------

      client = new UClientLCSP<UUnixSocket>(&cfg);

#  include "../csp/main.cpp"
      }

private:
   UClientLCSP<UUnixSocket>* client;
};

U_MAIN
