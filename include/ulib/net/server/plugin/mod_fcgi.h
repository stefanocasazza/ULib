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

#include <ulib/string.h>
#include <ulib/net/server/server_plugin.h>

class UClient_Base;

class U_EXPORT UFCGIPlugIn : public UServerPlugIn {
public:

   // COSTRUTTORI

            UFCGIPlugIn();
   virtual ~UFCGIPlugIn();

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
   static UClient_Base* connection;
   static bool fcgi_keep_conn, bphp;

          void set_FCGIBeginRequest();
   static void fill_FCGIBeginRequest(u_char type, u_short content_length);

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UFCGIPlugIn(const UFCGIPlugIn&) = delete;
   UFCGIPlugIn& operator=(const UFCGIPlugIn&) = delete;
#else
   UFCGIPlugIn(const UFCGIPlugIn&) : UServerPlugIn() {}
   UFCGIPlugIn& operator=(const UFCGIPlugIn&)        { return *this; }
#endif
};

#endif
