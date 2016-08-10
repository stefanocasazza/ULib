// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_http.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_HTTP_H
#define U_MOD_HTTP_H 1

#include <ulib/event/event_fd.h>
#include <ulib/net/server/server_plugin.h>

class U_EXPORT UHttpPlugIn : public UServerPlugIn, UEventFd {
public:

   // Check for memory error
   U_MEMORY_TEST

   UHttpPlugIn()
      {
      U_TRACE_REGISTER_OBJECT(0, UHttpPlugIn, "", 0)
      }

   virtual ~UHttpPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg) U_DECL_FINAL;
   virtual int handlerInit() U_DECL_FINAL;
   virtual int handlerRun() U_DECL_FINAL;
   virtual int handlerFork() U_DECL_FINAL;
   virtual int handlerStop() U_DECL_FINAL;

   // Connection-wide hooks

   virtual int handlerREAD() U_DECL_FINAL;
   virtual int handlerRequest() U_DECL_FINAL;

   // SigHUP hook

   virtual int handlerSigHUP() U_DECL_FINAL;

   // define method VIRTUAL of class UEventFd

   virtual int  handlerRead() U_DECL_FINAL;
   virtual void handlerDelete() U_DECL_FINAL;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return UEventFd::dump(reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHttpPlugIn)
};

#endif
