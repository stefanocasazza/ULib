// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_object.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file_config.h>
#include <ulib/net/rpc/rpc_object.h>
#include <ulib/net/rpc/rpc_envelope.h>

URPCObject* URPCObject::dispatcher;

// gcc: call is unlikely and code size would grow

void URPCObject::loadGenericMethod(UFileConfig* file_method)
{
   U_TRACE(0, "URPCObject::loadGenericMethod(%p)", file_method)

   U_INTERNAL_ASSERT_EQUALS(dispatcher, 0)
   U_INTERNAL_ASSERT_EQUALS(URPCMethod::encoder, 0)

   U_NEW(URPCObject, dispatcher, URPCObject);
   U_NEW(URPCEncoder, URPCMethod::encoder, URPCEncoder);

   if (file_method) readGenericMethod(*file_method);
}

void URPCObject::readFileMethod(UFileConfig& file_method)
{
   U_TRACE(0, "URPCObject::readFileMethod(%p)", &file_method) // problem with sanitize address

   UString method_name, method_ns, response_type;

   while (file_method.loadSection(0,0))
      {
      method_ns     = file_method.at(U_CONSTANT_TO_PARAM("NAMESPACE"));
      method_name   = file_method.at(U_CONSTANT_TO_PARAM("METHOD_NAME"));
      response_type = file_method.at(U_CONSTANT_TO_PARAM("RESPONSE_TYPE"));

      int rtype  = (response_type.equal(U_CONSTANT_TO_PARAM("success_or_failure"))       ?       success_or_failure :
                    response_type.equal(U_CONSTANT_TO_PARAM("stdin_success_or_failure")) ? stdin_success_or_failure :
                    response_type.equal(U_CONSTANT_TO_PARAM("standard_output"))          ?       standard_output    :
                    response_type.equal(U_CONSTANT_TO_PARAM("stdin_standard_output"))    ? stdin_standard_output    :
                    response_type.equal(U_CONSTANT_TO_PARAM("standard_output_binary"))   ? standard_output_binary   :
                                                                                           stdin_standard_output_binary);

      UCommand* command = UCommand::loadConfigCommand(&file_method);

      // Adds an object method to the list of method the object can call. Take ownership of the memory

      insertGenericMethod(method_name, method_ns, command, rtype);

      file_method.table.clear();
      }

   U_ASSERT(methodList.size())
}

URPCMethod* URPCObject::find(const UString& methodName)
{
   U_TRACE(0, "URPCObject::find(%V)", methodName.rep)

   URPCMethod* method;
   uint32_t n = methodList.size();

   // Iterate over the list of methods of the object

   for (uint32_t i = 0; i < n; ++i)
      {
      method = methodList[i];

      if (methodName == method->getMethodName()) U_RETURN_POINTER(method, URPCMethod);
      }

   U_RETURN_POINTER(0, URPCMethod);
}

UString URPCObject::processMessage(URPCEnvelope& envelope, bool& bContainsFault)
{
   U_TRACE(0, "URPCObject::processMessage(%p,%p)", &envelope, &bContainsFault)

   U_INTERNAL_ASSERT_POINTER(URPCMethod::encoder)

   UString retval;

   // Iterate over the list of methods

   URPCMethod* method = find(envelope.getMethodName());

   if (method == 0)
      {
      // Return object not found error. This would be a Client fault

      setFailed();

      URPCMethod::pFault->setFaultReason(U_CONSTANT_TO_PARAM("The requested method does not exist on this server"));

      bContainsFault = true;
      retval         = URPCMethod::encoder->encodeFault(URPCMethod::pFault);
      }
   else
      {
      UString ns = envelope.getNsName();

      U_INTERNAL_DUMP("envelope.nsName = %V", ns.rep)

      // check the name of namespace qualified element information (gSOAP)

      if (ns.empty()) ns = method->getNamespaces();
      if (ns.empty()) ns = *UString::str_ns;

      bContainsFault = (method->execute(envelope) == false);
      retval         = URPCMethod::encoder->encodeMethodResponse(*method, ns);
      }

   U_RETURN_STRING(retval);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* URPCObject::dump(bool reset) const
{
   *UObjectIO::os << "dispatcher (URPCObject           " << (void*)dispatcher  << ")\n"
                  << "methodList (UVector<URPCMethod*> " << (void*)&methodList << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
