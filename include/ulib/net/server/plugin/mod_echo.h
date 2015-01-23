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

   // COSTRUTTORE

            UEchoPlugIn() : UServerPlugIn() {}
   virtual ~UEchoPlugIn() __pure            {}

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   // Connection-wide hooks

   virtual int handlerRequest() U_DECL_OVERRIDE;

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UEchoPlugIn(const UEchoPlugIn&) = delete;
   UEchoPlugIn& operator=(const UEchoPlugIn&) = delete;
#else
   UEchoPlugIn(const UEchoPlugIn&) : UServerPlugIn() {}
   UEchoPlugIn& operator=(const UEchoPlugIn&)        { return *this; }
#endif
};

#endif
