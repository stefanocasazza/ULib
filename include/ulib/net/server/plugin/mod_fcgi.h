// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_fcgi.h - Fast CGI
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_FCGI_H
#define U_MOD_FCGI_H 1

#include <ulib/net/server/server_plugin.h>

class UClient_Base;

class U_EXPORT UFCGIPlugIn : public UServerPlugIn {
public:

   // Check for memory error
   U_MEMORY_TEST

            UFCGIPlugIn();
   virtual ~UFCGIPlugIn();

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
   static bool fcgi_keep_conn;
   static char environment_type;
   static UClient_Base* connection;

          void  set_FCGIBeginRequest();
   static void fill_FCGIBeginRequest(u_char type, u_short content_length);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UFCGIPlugIn)
};

#endif
