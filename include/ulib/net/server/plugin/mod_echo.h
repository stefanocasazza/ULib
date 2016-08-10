// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_echo.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_ECHO_H
#define U_MOD_ECHO_H 1

#include <ulib/net/server/server_plugin.h>

class U_EXPORT UEchoPlugIn : public UServerPlugIn {
public:

   // Check for memory error
   U_MEMORY_TEST

   UEchoPlugIn()
      {
      U_TRACE_REGISTER_OBJECT(0, UEchoPlugIn, "", 0)
      }

   virtual ~UEchoPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   // Connection-wide hooks

   virtual int handlerRequest() U_DECL_FINAL;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return ""; }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UEchoPlugIn)
};

#endif
