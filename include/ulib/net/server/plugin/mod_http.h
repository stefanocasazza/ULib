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

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // COSTRUTTORI

   UHttpPlugIn()
      {
      U_TRACE_REGISTER_OBJECT(0, UHttpPlugIn, "", 0)
      }

   virtual ~UHttpPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg) U_DECL_OVERRIDE;
   virtual int handlerInit() U_DECL_OVERRIDE;
   virtual int handlerRun() U_DECL_OVERRIDE;
   virtual int handlerFork() U_DECL_OVERRIDE;
   virtual int handlerStop() U_DECL_OVERRIDE;

   // Connection-wide hooks

   virtual int handlerREAD() U_DECL_OVERRIDE;
   virtual int handlerRequest() U_DECL_OVERRIDE;

   // SigHUP hook

   virtual int handlerSigHUP() U_DECL_OVERRIDE;

   // define method VIRTUAL of class UEventFd

   virtual int  handlerRead() U_DECL_OVERRIDE;
   virtual void handlerDelete() U_DECL_OVERRIDE;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return UEventFd::dump(reset); }
#endif

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UHttpPlugIn(const UHttpPlugIn&) = delete;
   UHttpPlugIn& operator=(const UHttpPlugIn&) = delete;
#else
   UHttpPlugIn(const UHttpPlugIn&) : UServerPlugIn(), UEventFd() {}
   UHttpPlugIn& operator=(const UHttpPlugIn&)                    { return *this; }
#endif
};

#endif
