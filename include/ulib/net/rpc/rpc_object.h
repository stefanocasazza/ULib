// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_object.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_RPC_OBJECT_H
#define ULIB_RPC_OBJECT_H 1

#include <ulib/net/rpc/rpc_fault.h>
#include <ulib/net/rpc/rpc_parser.h>
#include <ulib/net/rpc/rpc_encoder.h>
#include <ulib/net/rpc/rpc_gen_method.h>

/**
 * A URPCObject acts as a container for a set of methods. Objects derived from it may do with incoming requests:
 * save persistence information, perform logging, or whatever else is needed. As a user of the class, you may also
 * decide to just have the object hold stateless methods. In its simplest form, an object derived from URPCObject
 * may just act as repository for a functional group. When the component handling client requests receives a message
 * that component tells the URPCObject to process the request. URPCObject assumes that all requests are wellformed
 * messages. On return, the external component gets the response. This string may contain the result of the
 * method/message or it may contain a URPCFault.
 *
 * how to override the default...
 *
 * template <> inline void u_destroy(const URPCMethod** ptr, uint32_t n) { U_TRACE(0,"u_destroy<URPCMethod*>(%p,%u)", ptr, n) }
 *
 * NB: used by method
 * void assign(uint32_t n, T* elem))
 * void erase(uint32_t first, uint32_t last)
 */

class UFileConfig;

class U_EXPORT URPCObject {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   enum ResponseType {       success_or_failure     = 0,
                       stdin_success_or_failure     = 1,
                             standard_output        = 2,
                       stdin_standard_output        = 3,
                             standard_output_binary = 4,
                       stdin_standard_output_binary = 5 };

   URPCObject()
      {
      U_TRACE_REGISTER_OBJECT(0, URPCObject, "", 0)
      }

   virtual ~URPCObject()
      {
      U_TRACE_UNREGISTER_OBJECT(0, URPCObject)

      if (URPCMethod::pFault) delete URPCMethod::pFault;
      }

   // Triggers the creation of the URPCFault

   virtual void setFailed()
      {
      U_TRACE_NO_PARAM(0, "URPCObject::setFailed()")

      U_NEW(URPCFault, URPCMethod::pFault, URPCFault);
      }

   // GLOBAL SERVICES

   static URPCObject* dispatcher;

   static void loadGenericMethod(UFileConfig* file_method);

   // SERVICES

   static bool isSuccessOrFailure(int response_type)
      {
      U_TRACE(0, "URPCObject::isSuccessOrFailure(%d)", response_type)

      bool result = (response_type <= stdin_success_or_failure);

      U_RETURN(result);
      }

   static bool isStdInput(int response_type)
      {
      U_TRACE(0, "URPCObject::isStdInput(%d)", response_type)

      bool result = ((response_type & 1) != 0); // 1,3,5...

      U_RETURN(result);
      }

   static bool isOutputBinary(int response_type)
      {
      U_TRACE(0, "URPCObject::isOutputBinary(%d)", response_type)

      bool result = (response_type ==       standard_output_binary ||
                     response_type == stdin_standard_output_binary);

      U_RETURN(result);
      }

   URPCMethod* find(const UString& methodName);

   UVector<URPCMethod*>& getMethodList() { return methodList; }

   // -------------------------------------------------------------------------------------
   // basic program flow: Given a URPCEnvelope this calls the appropriate URPCMethod.
   //                     If this fails, ask for a URPCFault. Returns a send-ready response
   // -------------------------------------------------------------------------------------
   // 1. See if we know anything about the requested method
   // 2. If the requested method exists, execute it
   // 3. Encode the method response
   // 4. Return the method response
   // -------------------------------------------------------------------------------------
   // theCall:        Pre-parsed message
   // bContainsFault: Indicates if the returned string contains a fault
   // -------------------------------------------------------------------------------------

   UString processMessage(URPCEnvelope& theCall, bool& bContainsFault);

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UVector<URPCMethod*> methodList; // List of all the methods implemented by the derived object

          void readFileMethod(   UFileConfig& file_method);
   static void readGenericMethod(UFileConfig& file_method) { dispatcher->readFileMethod(file_method); }

   void insertMethod(URPCMethod* pmethod)
      {
      U_TRACE(0, "URPCObject::insertMethod(%p)", pmethod)

      methodList.push_back(pmethod);
      }

   // Meant to be called by classes derived from URPCObject.
   // Adds an object method to the list of method the object can call

   virtual void insertGenericMethod(const UString& n, const UString& ns, UCommand* cmd, int rtype)
      {
      U_TRACE(0, "URPCObject::insertGenericMethod(%V,%V,%p,%d)", n.rep, ns.rep, cmd, rtype) // problem with sanitize address

      URPCMethod* pmethod;

      U_NEW(URPCGenericMethod, pmethod, URPCGenericMethod(n, ns, cmd, rtype));

      methodList.push_back(pmethod);
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(URPCObject)
};

#endif
