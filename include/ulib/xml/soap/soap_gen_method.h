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

   // COSTRUTTORI

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

      pFault = (URPCFault*) U_NEW(USOAPFault);
      }

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   USOAPGenericMethod(const USOAPGenericMethod& g) = delete;
   USOAPGenericMethod& operator=(const USOAPGenericMethod& g) = delete;
#else
   USOAPGenericMethod(const USOAPGenericMethod& g) : URPCGenericMethod(UString::getStringNull(), UString::getStringNull(), 0, 0) {}
   USOAPGenericMethod& operator=(const USOAPGenericMethod& g)                                                                    { return *this; }
#endif      
};

#endif
