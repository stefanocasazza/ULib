// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_proxy.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_PROXY_H
#define U_MOD_PROXY_H 1

#include <ulib/net/tcpsocket.h>
#include <ulib/net/client/http.h>
#include <ulib/net/server/server_plugin.h>

class U_EXPORT UProxyPlugIn : public UServerPlugIn {
public:

   // Check for memory error
   U_MEMORY_TEST

            UProxyPlugIn();
   virtual ~UProxyPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg) U_DECL_FINAL;
   virtual int handlerInit() U_DECL_FINAL;

   // Connection-wide hooks

   virtual int handlerRequest() U_DECL_FINAL;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   static UHttpClient<UTCPSocket>* client_http;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UProxyPlugIn)
};

#endif
