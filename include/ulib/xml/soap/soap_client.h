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

#include <ulib/utility/uhttp.h>
#include <ulib/net/client/http.h>
#include <ulib/net/server/server.h>
#include <ulib/net/rpc/rpc_client.h>
#include <ulib/xml/soap/soap_parser.h>
#include <ulib/xml/soap/soap_encoder.h>

class U_EXPORT USOAPClient_Base : public URPCClient_Base {
public:

   void clearData()
      {
      U_TRACE(0, "USOAPClient_Base::clearData()")

             parser.clearData();
      UClient_Base::clearData();
      }

   // define method VIRTUAL

   virtual bool sendRequest()
      {
      U_TRACE(0, "USOAPClient_Base::sendRequest()")

      if (UClient_Base::sendRequest(request, false)) U_RETURN(true);

      U_RETURN(false);
      }

   virtual bool readResponse()
      {
      U_TRACE(0, "USOAPClient_Base::readResponse()")

      if (UClient_Base::readHTTPResponse()) U_RETURN(true);

      U_RETURN(false);
      }

   bool processRequest(URPCMethod& method)
      {
      U_TRACE(0, "USOAPClient_Base::processRequest(%p)", &method)

      U_INTERNAL_ASSERT_POINTER(URPCMethod::encoder)

      request = URPCMethod::encoder->encodeMethodCall(method, *UString::str_ns);

      request = UHttpClient_Base::wrapRequest(&request, UClient_Base::host_port,
                                                        U_CONSTANT_TO_PARAM("POST"),
                                                        U_CONSTANT_TO_PARAM("/soap"), "",
                                                        "application/soap+xml; charset=\"utf-8\"");

      if (sendRequest()  &&
          readResponse() &&
          parser.parse(UClient_Base::response))
         {
         if (parser.getMethodName() == *UString::str_fault) UClient_Base::response = parser.getFaultResponse();
         else
            {
            UClient_Base::response = parser.getResponse();

            U_RETURN(true);
            }
         }

      U_RETURN(false);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const;
#endif

protected:
   UString request;
   USOAPParser parser;

   // COSTRUTTORI

   USOAPClient_Base(UFileConfig* _cfg) : URPCClient_Base(_cfg)
      {
      U_TRACE_REGISTER_OBJECT(0, USOAPClient_Base, "%p", _cfg)

      delete URPCMethod::encoder;
             URPCMethod::encoder = U_NEW(USOAPEncoder);
      }

   virtual ~USOAPClient_Base()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USOAPClient_Base)
      }

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   USOAPClient_Base(const USOAPClient_Base&) = delete;
   USOAPClient_Base& operator=(const USOAPClient_Base&) = delete;
#else
   USOAPClient_Base(const USOAPClient_Base&) : URPCClient_Base(0) {}
   USOAPClient_Base& operator=(const USOAPClient_Base&)           { return *this; }
#endif      
};

template <class Socket> class U_EXPORT USOAPClient : public USOAPClient_Base {
public:

   // Costruttori

   USOAPClient(UFileConfig* _cfg) : USOAPClient_Base(_cfg)
      {
      U_TRACE_REGISTER_OBJECT(0, USOAPClient, "%p", _cfg)

      UClient_Base::socket = U_NEW(Socket(UClient_Base::bIPv6));
      }

   virtual ~USOAPClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USOAPClient)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return USOAPClient_Base::dump(_reset); }
#endif

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   USOAPClient(const USOAPClient&) = delete;
   USOAPClient& operator=(const USOAPClient&) = delete;
#else
   USOAPClient(const USOAPClient&) : USOAPClient_Base(0) {}
   USOAPClient& operator=(const USOAPClient&)            { return *this; }
#endif      
};

#ifdef USE_LIBSSL // specializzazione con USSLSocket

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

protected:

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   USOAPClient<USSLSocket>(const USOAPClient<USSLSocket>&) = delete;
   USOAPClient<USSLSocket>& operator=(const USOAPClient<USSLSocket>&) = delete;
#else
   USOAPClient<USSLSocket>(const USOAPClient<USSLSocket>&) : USOAPClient_Base(0) {}
   USOAPClient<USSLSocket>& operator=(const USOAPClient<USSLSocket>&)            { return *this; }
#endif
};

#endif
#endif
