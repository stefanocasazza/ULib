// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    soap_parser.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/xml/soap/soap_fault.h>
#include <ulib/xml/soap/soap_object.h>
#include <ulib/xml/soap/soap_parser.h>
#include <ulib/xml/soap/soap_encoder.h>

bool USOAPParser::parse(const UString& msg)
{
   U_TRACE(0+256, "USOAPParser::parse(%V)", msg.rep)

   initParser();

   if (UXMLParser::parse(msg))
      {
      // If we succeeded, get the name of the method being called. This of course assumes only
      // one method in the body, and that there are no objects outside of the serialization root.
      // This method will need an override if this assumption is invalid

      U_INTERNAL_ASSERT_POINTER(body)

#  ifndef U_COVERITY_FALSE_POSITIVE // Explicit null dereferenced (FORWARD_NULL)
      method              = body->childAt(0);
      envelope.methodName = method->elem()->getAccessorName();

      // load the parameters for the method to execute

      UString param;

      for (uint32_t i = 0, num_arguments = method->numChild(); i < num_arguments; ++i)
         {
         param = method->childAt(i)->elem()->getValue();

         // check if parameter optional

         if (param) envelope.arg->push_back(param);
         }
#  endif

      U_RETURN(true);
      }

   U_RETURN(false);
}

UString USOAPParser::getFaultResponse()
{
   U_TRACE_NO_PARAM(0, "USOAPParser::getFaultResponse()")

   U_INTERNAL_ASSERT_POINTER(method)

   URPCFault fault;
   UString retval(U_CAPACITY);

   fault.setFaultCode();

                                 fault.getFaultReason() = (*envelope.arg)[0];
   if (envelope.arg->size() > 1) fault.getDetail()      = (*envelope.arg)[1];

   fault.encode(retval);

   (void) retval.shrink();

   U_RETURN_STRING(retval);
}

UString USOAPParser::processMessage(const UString& msg, URPCObject& object, bool& bContainsFault)
{
   U_TRACE(0, "USOAPParser::processMessage(%V,%p,%p,%b)", msg.rep, &object, &bContainsFault)

   U_INTERNAL_ASSERT(msg)

   UString retval;

   clearData();

   if (U_STRING_FIND(msg, 0, "http://schemas.xmlsoap.org/soap/envelope/") != U_NOT_FOUND)
      {
      zero();

      bContainsFault = true;

      (void) retval.assign(U_CONSTANT_TO_PARAM("<?xml version=\"1.0\" ?>"
                                               "<env:Envelope xmlns:env=\"http://schemas.xmlsoap.org/soap/envelope/\">"
                                                  "<env:Header>"
                                                   "<env:Upgrade>"
                                                      "<env:SupportedEnvelope qname=\"ns1:Envelope\" xmlns:ns1=\"http://www.w3.org/2003/05/soap-envelope\"/>"
                                                   "</env:Upgrade>"
                                                  "</env:Header>"
                                                  "<env:Body>"
                                                   "<env:Fault>"
                                                      "<faultcode>env:VersionMismatch</faultcode>"
                                                      "<faultstring>Version Mismatch</faultstring>"
                                                   "</env:Fault>"
                                                  "</env:Body>"
                                               "</env:Envelope>"));
      }
   else if (parse(msg))
      {
      U_ASSERT_EQUALS(envelope.methodName.equal(U_CONSTANT_TO_PARAM("Fault")), false)

      retval = object.processMessage(envelope, bContainsFault);
      }
   else
      {
      int line = ::XML_GetCurrentLineNumber(m_parser),
          coln = ::XML_GetCurrentColumnNumber(m_parser);

      object.setFailed();

      URPCMethod::pFault->getFaultReason() = UXMLParser::getErrorMessage();

      U_INTERNAL_DUMP("UXMLParser: %V (%d,%d) ", URPCMethod::pFault->getFaultReason().rep, line, coln)

      URPCMethod::pFault->setDetail(U_CONSTANT_TO_PARAM("The fault occurred near position (line: %d col: %d) within the message"), line, coln);

      bContainsFault = true;
      retval         = URPCMethod::encoder->encodeFault(URPCMethod::pFault);
      }

   U_RETURN_STRING(retval);
}

void USOAPParser::startElement(const XML_Char* name, const XML_Char** attrs)
{
   U_TRACE(0, "USOAPParser::startElement(%S,%p)", name, attrs)

   U_DUMP_ATTRS(attrs)

   UXMLAttribute* attribute;
   UString str((void*)name, u__strlen(name, __PRETTY_FUNCTION__)), namespaceName, accessorName, value;

   UXMLElement::splitNamespaceAndName(str, namespaceName, accessorName);

   if (flag_state == 0)
      {
      U_INTERNAL_ASSERT(u_rmatch(U_STRING_TO_PARAM(str), U_CONSTANT_TO_PARAM(":Envelope")))

      flag_state = 1;
      }

   U_DUMP("flag_state = %d ptree = %p ptree->parent() = %p ptree->numChild() = %u ptree->depth() = %u",
           flag_state, ptree, ptree->parent(), ptree->numChild(), ptree->depth())

   U_NEW(UXMLElement, current, UXMLElement(str, accessorName, namespaceName));

   ptree = ptree->push(current);

   if (flag_state <= 2)
      {
      if (flag_state == 1 &&
          u_rmatch(U_STRING_TO_PARAM(str), U_CONSTANT_TO_PARAM(":Header")))
         {
         header     = ptree;
         flag_state = 2;
         }
      else if (u_rmatch(U_STRING_TO_PARAM(str), U_CONSTANT_TO_PARAM(":Body")))
         {
         body       = ptree;
         flag_state = 3;
         }
      }

   U_INTERNAL_DUMP("flag_state = %d", flag_state)

   while (*attrs)
      {
        str.replace(*attrs++);
      value.replace(*attrs++);

      UXMLElement::splitNamespaceAndName(str, namespaceName, accessorName);

      U_NEW(UXMLAttribute, attribute, UXMLAttribute(str, accessorName, namespaceName, value));

      current->addAttribute(attribute);

      // check if anybody has mustUnderstand set to true

      if (flag_state == 2 &&
          accessorName.equal(U_CONSTANT_TO_PARAM("mustUnderstand")))
         {
         envelope.mustUnderstand = value.equal(U_CONSTANT_TO_PARAM("true"));

         U_INTERNAL_DUMP("envelope.mustUnderstand = %b", envelope.mustUnderstand)
         }

      // set the name of namespace qualified element information (gSOAP)

      if (flag_state == 1                                   &&
          namespaceName.equal(U_CONSTANT_TO_PARAM("xmlns")) &&
           accessorName == *UString::str_ns)
         {
         envelope.nsName = value;

         U_INTERNAL_DUMP("envelope.nsName = %V", envelope.nsName.rep)
         }

      // Manage the names and URNs of any and all namespaces found when parsing the message.
      // If duplicate namespace names are used with different URNs, only the last one found
      // will appear in the set of values

#  ifdef U_SOAP_NAMESPACE
      if (namespaceName.equal(U_CONSTANT_TO_PARAM("xmlns"))) XMLNStoURN.insert(accessorName, value);
#  endif
      }
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* USOAPParser::dump(bool reset) const
{
   URPCParser::dump(false);

   *UObjectIO::os << '\n';

   UXMLParser::dump(false);

   *UObjectIO::os << '\n'
                  << "flag_state                                        " << flag_state         << '\n'
#              ifdef U_SOAP_NAMESPACE
                  << "XMLNStoURN      (UHashMap<UString>                " << (void*)&XMLNStoURN << ")\n"
#              endif
                  << "tree            (UTree<UXMLElement*>              " << (void*)&tree       << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
