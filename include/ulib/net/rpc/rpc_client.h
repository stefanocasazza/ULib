// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_client.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_RPC_CLIENT_H
#define ULIB_RPC_CLIENT_H 1

#include <ulib/net/client/client.h>
#include <ulib/net/rpc/rpc_encoder.h>

class URDBClient_Base;

class U_EXPORT URPCClient_Base : public UClient_Base {
public:

   // SERVICES

   bool processRequest(URPCMethod& method)
      {
      U_TRACE(0, "URPCClient_Base::processRequest(%p)", &method)

      U_INTERNAL_ASSERT_POINTER(URPCMethod::encoder)

      UString name;
      
      UClient_Base::prepareRequest(URPCMethod::encoder->encodeMethodCall(method, name));

      if (UClient_Base::sendRequest()            &&
          readResponse(socket, buffer, response) &&
          buffer.equal(U_CONSTANT_TO_PARAM("DONE")))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const;
#endif

protected:

   URPCClient_Base(UFileConfig* _cfg = 0) : UClient_Base(_cfg)
      {
      U_TRACE_REGISTER_OBJECT(0, URPCClient_Base, "%p", _cfg)

      U_NEW(URPCEncoder, URPCMethod::encoder, URPCEncoder);
      }

   ~URPCClient_Base()
      {
      U_TRACE_UNREGISTER_OBJECT(0, URPCClient_Base)

      U_INTERNAL_ASSERT_POINTER(URPCMethod::encoder)

      delete URPCMethod::encoder;
      }

   static bool readResponse(USocket* sk, UString& buffer, UString& response);

private:
   U_DISALLOW_COPY_AND_ASSIGN(URPCClient_Base)

   friend class URDBClient_Base;
};

template <class Socket> class U_EXPORT URPCClient : public URPCClient_Base {
public:

   URPCClient(UFileConfig* _cfg) : URPCClient_Base(_cfg)
      {
      U_TRACE_REGISTER_OBJECT(0, URPCClient, "%p", _cfg)

      U_NEW(Socket, UClient_Base::socket, Socket(UClient_Base::bIPv6));
      }

   ~URPCClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, URPCClient)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return URPCClient_Base::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(URPCClient)
};

#ifdef USE_LIBSSL
template <> class U_EXPORT URPCClient<USSLSocket> : public URPCClient_Base {
public:

   URPCClient(UFileConfig* _cfg) : URPCClient_Base(_cfg)
      {
      U_TRACE_REGISTER_OBJECT(0, URPCClient<USSLSocket>, "%p", _cfg)

      UClient_Base::setSSLContext();
      }

   ~URPCClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, URPCClient<USSLSocket>)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return URPCClient_Base::dump(_reset); }
#endif

protected:

private:
   U_DISALLOW_COPY_AND_ASSIGN(URPCClient<USSLSocket>)
};
#endif
#endif
