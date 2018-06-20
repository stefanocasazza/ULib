// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_socket.h - web socket
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_SOCKET_H
#define U_MOD_SOCKET_H 1

#include <ulib/net/server/server_plugin.h>

class U_EXPORT UWebSocketPlugIn : public UServerPlugIn {
public:

   // Check for memory error
   U_MEMORY_TEST

            UWebSocketPlugIn();
   virtual ~UWebSocketPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg) U_DECL_FINAL;
   virtual int handlerRun() U_DECL_FINAL;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return ""; }
#endif

private:
   static bool enable_db;

   U_DISALLOW_COPY_AND_ASSIGN(UWebSocketPlugIn)
};
#endif
