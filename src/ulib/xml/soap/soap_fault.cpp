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
   U_TRACE(0, "USOAPFault::encode(%V)", response.rep)

   UString code = getFaultCode();

   response.setBuffer(1000U + code.size() + faultReason.size() + detail.size());

   response.snprintf(U_CONSTANT_TO_PARAM("<?xml version='1.0' ?>"
                     "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\">"
                        "<soap:Body>"
                           "<soap:Fault>"
                              "<soap:Code>"
                                 "<soap:Value>soap:%v</soap:Value>"
                              "</soap:Code>"
                              "<soap:Reason>"
                                 "<soap:Text xml:lang=\"en-US\">%v</soap:Text>"
                              "</soap:Reason>"
                              "<soap:Detail>%v</soap:Detail>"
                           "</soap:Fault>"
                        "</soap:Body>"
                     "</soap:Envelope>"),
                     code.rep, faultReason.rep, detail.rep);
}
