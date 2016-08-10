// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_soap.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_SOAP_H
#define U_MOD_SOAP_H 1

#include <ulib/net/server/server_plugin.h>

class USOAPParser;

class U_EXPORT USoapPlugIn : public UServerPlugIn {
public:

   // Check for memory error
   U_MEMORY_TEST

   USoapPlugIn()
      {
      U_TRACE_REGISTER_OBJECT(0, USoapPlugIn, "")

      UString::str_allocate(STR_ALLOCATE_SOAP);
      }

   virtual ~USoapPlugIn();

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
   static USOAPParser* soap_parser;

private:
   U_DISALLOW_COPY_AND_ASSIGN(USoapPlugIn)
};

#endif
