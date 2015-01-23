// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    soap_fault.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/xml/soap/soap_fault.h>

void USOAPFault::encode(UString& response)
{
   U_TRACE(0, "USOAPFault::encode(%.*S)", U_STRING_TO_TRACE(response))

   UString code = getFaultCode();

   response.setBuffer(1000U + code.size() + faultReason.size() + detail.size());

   response.snprintf("<?xml version='1.0' ?>"
                     "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\">"
                        "<soap:Body>"
                           "<soap:Fault>"
                              "<soap:Code>"
                                 "<soap:Value>soap:%.*s</soap:Value>"
                              "</soap:Code>"
                              "<soap:Reason>"
                                 "<soap:Text xml:lang=\"en-US\">%.*s</soap:Text>"
                              "</soap:Reason>"
                              "<soap:Detail>%.*s</soap:Detail>"
                           "</soap:Fault>"
                        "</soap:Body>"
                     "</soap:Envelope>",
                     U_STRING_TO_TRACE(code),
                     U_STRING_TO_TRACE(faultReason),
                     U_STRING_TO_TRACE(detail));
}
