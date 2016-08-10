// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    soap_object.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_SOAP_OBJECT_H
#define ULIB_SOAP_OBJECT_H 1

#include <ulib/xml/soap/soap_fault.h>
#include <ulib/xml/soap/soap_parser.h>
#include <ulib/xml/soap/soap_encoder.h>
#include <ulib/xml/soap/soap_gen_method.h>

/**
 * A USOAPObject acts as a container for a set of methods. Objects derived from it may do with incoming requests:
 * save persistence information, perform logging, or whatever else is needed. As a user of the class, you may also
 * decide to just have the object hold stateless methods. In its simplest form, an object derived from USOAPObject
 * may just act as repository for a functional group. When the component handling client requests receives a message
 * that component tells the USOAPObject to process the request. USOAPObject assumes that all requests are wellformed
 * SOAP messages. On return, the external component gets the SOAP response. This string may contain the result of the
 * method/message or it may contain a Fault
 */

class U_EXPORT USOAPObject : public URPCObject {
public:

   USOAPObject()
      {
      U_TRACE_REGISTER_OBJECT(0, USOAPObject, "", 0)
      }

   virtual ~USOAPObject()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USOAPObject)
      }

   // Triggers the creation of the USOAPFault

   virtual void setFailed() U_DECL_OVERRIDE
      {
      U_TRACE_NO_PARAM(0, "USOAPObject::setFailed()")

      U_NEW(USOAPFault, URPCMethod::pFault, USOAPFault);
      }

   // GLOBAL SERVICES

   static void loadGenericMethod(UFileConfig* file_method)
      {
      U_TRACE(0, "USOAPObject::loadGenericMethod(%p)", file_method)

      U_INTERNAL_ASSERT_EQUALS(dispatcher,0)
      U_INTERNAL_ASSERT_EQUALS(URPCMethod::encoder,0)

      U_NEW(USOAPObject, dispatcher, USOAPObject);
      U_NEW(USOAPEncoder, URPCMethod::encoder, USOAPEncoder);

      if (file_method) URPCObject::readGenericMethod(*file_method);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return URPCObject::dump(reset); }
#endif

protected:

   // Adds an object method to the list of method the object can call

   virtual void insertGenericMethod(const UString& n, const UString& ns, UCommand* cmd, int rtype) U_DECL_OVERRIDE
      {
      U_TRACE(0, "USOAPObject::insertGenericMethod(%V,%V,%p,%d)", n.rep, ns.rep, cmd, rtype)

      URPCMethod* pmethod;

      U_NEW(USOAPGenericMethod, pmethod, USOAPGenericMethod(n, ns, cmd, rtype));

      methodList.push_back(pmethod);
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(USOAPObject)
};

#endif
