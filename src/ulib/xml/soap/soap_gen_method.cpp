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
   U_TRACE(0, "USOAPGenericMethod::encode()")

   U_INTERNAL_ASSERT_POINTER(URPCMethod::encoder)

   if (hasFailed()) U_SOAP_ENCODE_RES(response);
   else
      {
      static const UString* str_response;

      static ustringrep stringrep_storage[] = {
         { U_STRINGREP_FROM_CONSTANT("response") }
      };

      if (str_response == 0) U_NEW_ULIB_OBJECT(str_response, U_STRING_FROM_STRINGREP_STORAGE(0));

      if (URPCObject::isOutputBinary(response_type)) U_SOAP_ENCB64_NAME_ARG(*str_response, response);
      else                                           U_SOAP_ENCODE_NAME_ARG(*str_response, response);
      }
}
