// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_envelope.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_RPC_ENVELOPE_H
#define ULIB_RPC_ENVELOPE_H 1

#include <ulib/container/vector.h>

class URPCParser;
class USOAPParser;

class U_EXPORT URPCEnvelope {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   URPCEnvelope()
      {
      U_TRACE_REGISTER_OBJECT(0, URPCEnvelope, "", 0)

      arg            = 0;
      mustUnderstand = false;
      }

   ~URPCEnvelope()
      {
      U_TRACE_UNREGISTER_OBJECT(0, URPCEnvelope)
      }

   // SERVICES

   UString getNsName() const     { return nsName; }         // return the name of namespace qualified element information
   UString getMethodName() const { return methodName; }     // return the name of the method the caller wants to execute
   bool isMustUnderstand() const { return mustUnderstand; }

   int         getNumArgument() const       { return arg->size(); }
   UString     getArgument(int n) const     { return (*arg)[n]; }
   const char* getArgumentCStr(int n) const { return (*arg)[n].c_strdup(); }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString nsName,         // this contains the name of the namespace qualified element information
           methodName;     // this contains the name of the method the caller wants to execute
   UVector<UString>* arg;  // this contains the argument for the method the caller wants to execute
   bool mustUnderstand;

private:
   U_DISALLOW_COPY_AND_ASSIGN(URPCEnvelope)

   friend class URPCParser;
   friend class USOAPParser;
};

#endif
