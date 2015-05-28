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

   // COSTRUTTORI

            USoapPlugIn();
   virtual ~USoapPlugIn();

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
   static USOAPParser* soap_parser;

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   USoapPlugIn(const USoapPlugIn&) = delete;
   USoapPlugIn& operator=(const USoapPlugIn&) = delete;
#else
   USoapPlugIn(const USoapPlugIn&) : UServerPlugIn() {}
   USoapPlugIn& operator=(const USoapPlugIn&)        { return *this; }
#endif
};

#endif
