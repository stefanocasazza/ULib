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

   // COSTRUTTORI

            UProxyPlugIn();
   virtual ~UProxyPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg) U_DECL_OVERRIDE;
   virtual int handlerInit() U_DECL_OVERRIDE;

   // Connection-wide hooks

   virtual int handlerRequest() U_DECL_OVERRIDE;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   static UHttpClient<UTCPSocket>* client_http;

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UProxyPlugIn(const UProxyPlugIn&) = delete;
   UProxyPlugIn& operator=(const UProxyPlugIn&) = delete;
#else
   UProxyPlugIn(const UProxyPlugIn&) : UServerPlugIn() {}
   UProxyPlugIn& operator=(const UProxyPlugIn&)        { return *this; }
#endif
};

#endif
