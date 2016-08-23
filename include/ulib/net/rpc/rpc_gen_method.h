// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_gen_method.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_RPC_GENERIC_METHOD_H
#define ULIB_RPC_GENERIC_METHOD_H 1

#include <ulib/command.h>
#include <ulib/net/rpc/rpc_method.h>

class U_EXPORT URPCGenericMethod : public URPCMethod {
public:

   URPCGenericMethod(const UString& n, const UString& _ns, UCommand* cmd, int rtype) : URPCMethod(n, _ns), response(U_CAPACITY)
      {
      U_TRACE_REGISTER_OBJECT(0, URPCGenericMethod, "%V,%V,%p,%d", n.rep, _ns.rep, cmd, rtype) 

      command       = cmd; 
      response_type = rtype;
      }

   virtual ~URPCGenericMethod()
      {
      U_TRACE_UNREGISTER_OBJECT(0, URPCGenericMethod)

      if (command) delete command;
      }

   // Transforms the method into something that servers and clients can send. The encoder holds the actual
   // data while the client hands data to be entered in. This makes a whole lot more sense in the samples that
   // should have shipped with the library

   virtual void encode()
      {
      U_TRACE_NO_PARAM(0, "URPCGenericMethod::encode()")

      U_INTERNAL_ASSERT_POINTER(URPCMethod::encoder)

      U_RPC_ENCODE_RES(hasFailed() ? "ERR " : "DONE", response);
      }

   // Only to be called on the server by the object dispatcher. This method executes the call and returns true
   // if the call succeeded, false if it failed. URPCGenericMethod should keep any return data in a member
   // variable. The information will be returned via a call to encode

   virtual bool execute(URPCEnvelope& theCall);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString response;
   UCommand* command;
   int response_type;

   URPCGenericMethod() : URPCMethod() {}

private:
   U_DISALLOW_COPY_AND_ASSIGN(URPCGenericMethod)
};

#endif
