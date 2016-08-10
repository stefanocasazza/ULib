// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_method.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_RPC_METHOD_H
#define ULIB_RPC_METHOD_H 1

#include <ulib/string.h>

/**
 * @class URPCMethod
 *
 * URPCMethod provides an interface for the things that methods most know how to do.
 *
 * Specifically, it needs to know the following:
 *   - Its name so the URPCObject can find it
 *   - How to execute itself given a URPCEnvelope
 *   - If it fails, how to fill in a URPCFault
 *   - How to encode itself for execution or response
 */

// Forward declaration

class URPCFault;
class URPCEncoder;
class URPCEnvelope;

class U_EXPORT URPCMethod {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   URPCMethod()
      {
      U_TRACE_REGISTER_OBJECT(0, URPCMethod, "", 0)
      }

   URPCMethod(const UString& n, const UString& _ns) : method_name(n), ns(_ns)
      {
      U_TRACE_REGISTER_OBJECT(0, URPCMethod, "%V,%V", n.rep, _ns.rep)
      }

   virtual ~URPCMethod()
      {
      U_TRACE_UNREGISTER_OBJECT(0, URPCMethod)
      }

   // GLOBAL SERVICES

   static URPCFault* pFault;
   static URPCEncoder* encoder;

   static bool hasFailed()
      {
      U_TRACE_NO_PARAM(0, "URPCMethod::hasFailed()")

      bool result = (pFault != 0);

      U_RETURN(result);
      }

   // VIRTUAL METHOD

   virtual UString getHeaderContent() const { return UString::getStringNull(); }

   // Used to get the name of the method. This is matched up using the URPCObject dispatcher to respond to a call

   virtual UString getMethodName() const { return method_name; }

   // Used to get the namespace of the method

   virtual UString getNamespaces() const { return ns; }

   // Transforms the method into something that servers and clients can send. The encoder holds the
   // actual data while the client hands data to be entered in. This makes a whole lot more sense in the
   // samples that should have shipped with the library

   virtual void encode()
      {
      U_TRACE_NO_PARAM(0, "URPCMethod::encode()")

      U_INTERNAL_ASSERT(false) // If this assert fires, you need to implement the method
      }

   // Only to be called on the server by the dispatcher.
   // This method executes the call and returns true if the call succeeded, false if it failed.
   // URPCMethod should keep any return data in a member variable. The information will be returned via a call to encode

   virtual bool execute(URPCEnvelope& theCall)
      {
      U_TRACE(0, "URPCMethod::execute(%p)", &theCall)

      // If this assert fires, you need to implement the method.
      // We include a default implementation because client-side Methods should never call execute.

      U_INTERNAL_ASSERT(false)

      U_RETURN(false);
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString method_name, ns;

private:
   U_DISALLOW_ASSIGN(URPCMethod)
};

#endif
