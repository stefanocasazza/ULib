// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    soap_gen_method.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_SOAP_GENERIC_METHOD_H
#define ULIB_SOAP_GENERIC_METHOD_H 1

#include <ulib/net/rpc/rpc_object.h>
#include <ulib/xml/soap/soap_fault.h>
#include <ulib/xml/soap/soap_encoder.h>
#include <ulib/net/rpc/rpc_gen_method.h>

class U_EXPORT USOAPGenericMethod : public URPCGenericMethod {
public:

   USOAPGenericMethod(const UString& n, const UString& _ns, UCommand* cmd, int rtype) : URPCGenericMethod(n, _ns, cmd, rtype)
      {
      U_TRACE_REGISTER_OBJECT(0, USOAPGenericMethod, "%V,%V,%p,%d", n.rep, _ns.rep, cmd, rtype) 
      }

   virtual ~USOAPGenericMethod()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USOAPGenericMethod)
      }

   // VIRTUAL METHOD

   virtual void encode() U_DECL_OVERRIDE;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return USOAPGenericMethod::dump(reset); }
#endif

protected:

   // Triggers the creation of the USOAPFault

   virtual void setFailed()
      {
      U_TRACE_NO_PARAM(0, "USOAPGenericMethod::setFailed()")

      U_NEW(USOAPFault, pFault, USOAPFault);
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(USOAPGenericMethod)
};

#endif
