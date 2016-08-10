// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_tsa.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_TSA_H
#define U_MOD_TSA_H 1

#include <ulib/net/server/server_plugin.h>

class UCommand;

class U_EXPORT UTsaPlugIn : public UServerPlugIn {
public:

   // Check for memory error
   U_MEMORY_TEST

            UTsaPlugIn();
   virtual ~UTsaPlugIn();

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
   static UCommand* command;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UTsaPlugIn)
};

#endif
