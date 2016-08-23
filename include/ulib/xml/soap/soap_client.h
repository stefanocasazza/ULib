// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    soap_client.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_SOAP_CLIENT_H
#define ULIB_SOAP_CLIENT_H 1

#include <ulib/net/rpc/rpc_client.h>
#include <ulib/xml/soap/soap_encoder.h>

class USOAPParser;

class U_EXPORT USOAPClient_Base : public URPCClient_Base {
public:

   void clearData();
   bool processRequest(URPCMethod& method);

   // define method VIRTUAL

   virtual bool sendRequest()
      {
      U_TRACE_NO_PARAM(0, "USOAPClient_Base::sendRequest()")

      if (UClient_Base::sendRequest()) U_RETURN(true);

      U_RETURN(false);
      }

   virtual bool readResponse();

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const;
#endif

protected:
   USOAPParser* parser;

   USOAPClient_Base(UFileConfig* _cfg = 0) : URPCClient_Base(_cfg)
      {
      U_TRACE_REGISTER_OBJECT(0, USOAPClient_Base, "%p", _cfg)

      parser = 0;

      u_init_http_method_list();

      delete URPCMethod::encoder;

      U_NEW(USOAPEncoder, URPCMethod::encoder, USOAPEncoder);
      }

   virtual ~USOAPClient_Base();

private:
   U_DISALLOW_COPY_AND_ASSIGN(USOAPClient_Base)
};

template <class Socket> class U_EXPORT USOAPClient : public USOAPClient_Base {
public:

   USOAPClient(UFileConfig* _cfg) : USOAPClient_Base(_cfg)
      {
      U_TRACE_REGISTER_OBJECT(0, USOAPClient, "%p", _cfg)

      U_NEW(Socket, UClient_Base::socket, Socket(UClient_Base::bIPv6));
      }

   virtual ~USOAPClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USOAPClient)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return USOAPClient_Base::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(USOAPClient)
};

#ifdef USE_LIBSSL
template <> class U_EXPORT USOAPClient<USSLSocket> : public USOAPClient_Base {
public:

   USOAPClient(UFileConfig* _cfg) : USOAPClient_Base(_cfg)
      {
      U_TRACE_REGISTER_OBJECT(0, USOAPClient<USSLSocket>, "%p", _cfg)

      UClient_Base::setSSLContext();
      }

   ~USOAPClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USOAPClient<USSLSocket>)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return USOAPClient_Base::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(USOAPClient<USSLSocket>)
};
#endif
#endif
