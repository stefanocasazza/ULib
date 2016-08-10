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

class UCommand;

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

   // Connection-wide hooks

   virtual int handlerRequest() U_DECL_FINAL;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   static vPFi on_message;
   static UCommand* command;

   static RETSIGTYPE handlerForSigTERM(int signo);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UWebSocketPlugIn)
};

#endif
