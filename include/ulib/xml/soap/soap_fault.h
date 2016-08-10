// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    soap_fault.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_SOAP_FAULT_H
#define ULIB_SOAP_FAULT_H 1

#include <ulib/net/rpc/rpc_fault.h>

/**
 * @class USOAPFault
 */

class U_EXPORT USOAPFault : public URPCFault {
public:

   USOAPFault()
      {
      U_TRACE_REGISTER_OBJECT(0, USOAPFault, "", 0)
      }

   virtual ~USOAPFault()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USOAPFault)
      }

   // Encodes the complete fault into a string

   virtual void encode(UString& response) U_DECL_OVERRIDE;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return URPCFault::dump(reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(USOAPFault)
};

#endif
