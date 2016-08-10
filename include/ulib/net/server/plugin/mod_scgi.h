// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_scgi.h - Simple CGI
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_SCGI_H
#define U_MOD_SCGI_H 1

#include <ulib/net/server/server_plugin.h>

class UClient_Base;

class U_EXPORT USCGIPlugIn : public UServerPlugIn {
public:

   // Check for memory error
   U_MEMORY_TEST

            USCGIPlugIn();
   virtual ~USCGIPlugIn();

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
   static bool scgi_keep_conn;
   static UClient_Base* connection;

private:
   U_DISALLOW_COPY_AND_ASSIGN(USCGIPlugIn)
};

#endif
