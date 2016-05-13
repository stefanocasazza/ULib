// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    soap_client.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/client/http.h>
#include <ulib/xml/soap/soap_client.h>
#include <ulib/xml/soap/soap_parser.h>

USOAPClient_Base::~USOAPClient_Base()
{
   U_TRACE_UNREGISTER_OBJECT(0, USOAPClient_Base)

   if (parser) delete parser;
}

void USOAPClient_Base::clearData()
{
   U_TRACE_NO_PARAM(0, "USOAPClient_Base::clearData()")

   if (parser)
      {
            parser->clearData();
      UClient_Base::clearData();
      }
}

bool USOAPClient_Base::readResponse()
{
   U_TRACE_NO_PARAM(0, "USOAPClient_Base::readResponse()")

   if (UClient_Base::readHTTPResponse()) U_RETURN(true);

   U_RETURN(false);
}

bool USOAPClient_Base::processRequest(URPCMethod& method)
{
   U_TRACE(0, "USOAPClient_Base::processRequest(%p)", &method)

   U_INTERNAL_ASSERT_POINTER(URPCMethod::encoder)

   UString req = URPCMethod::encoder->encodeMethodCall(method, *UString::str_ns);

   UClient_Base::prepareRequest(UHttpClient_Base::wrapRequest(&req, UClient_Base::host_port, 2,
                                U_CONSTANT_TO_PARAM("/soap"), "", U_CONSTANT_TO_PARAM("application/soap+xml; charset=\"utf-8\"")));

   if (sendRequest() &&
       readResponse())
      {
      if (parser == 0) U_NEW(USOAPParser, parser, USOAPParser);

      if (parser->parse(UClient_Base::response))
         {
         if (parser->getMethodName().equal(U_CONSTANT_TO_PARAM("Fault"))) UClient_Base::response = parser->getFaultResponse();
         else
            {
#        ifndef U_COVERITY_FALSE_POSITIVE // Explicit null dereferenced (FORWARD_NULL)
            UClient_Base::response = parser->getResponse();
#        endif

            U_RETURN(true);
            }
         }
      }

   U_RETURN(false);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* USOAPClient_Base::dump(bool _reset) const
{
   URPCClient_Base::dump(false);

   *UObjectIO::os << '\n'
                  << "parser         (USOAPParser         " << (void*)parser  << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
