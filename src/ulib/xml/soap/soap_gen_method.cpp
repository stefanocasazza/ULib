// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//   soap_gen_method.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/xml/soap/soap_gen_method.h>

void USOAPGenericMethod::encode()
{
   U_TRACE_NO_PARAM(0, "USOAPGenericMethod::encode()")

   U_INTERNAL_ASSERT_POINTER(URPCMethod::encoder)

   if (hasFailed()) U_SOAP_ENCODE_RES(response);
   else
      {
      if (URPCObject::isOutputBinary(response_type)) U_SOAP_ENCB64_NAME_ARG(*UString::str_response, response);
      else                                           U_SOAP_ENCODE_NAME_ARG(*UString::str_response, response);
      }
}
