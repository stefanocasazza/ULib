// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    soap_encoder.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/xml/soap/soap_encoder.h>

void USOAPEncoder::encodeArgument(const UString& argName, const UString& argType, const UString& argContent)
{
   U_TRACE(0, "USOAPEncoder::encodeArgument(%.*S,%.*S,%.*S)", U_STRING_TO_TRACE(argName),
                                                              U_STRING_TO_TRACE(argType),
                                                              U_STRING_TO_TRACE(argContent))

   encodedValue.setBuffer(200U + (argName.size() * 2) + argType.size() + argContent.size());

   encodedValue.snprintf("<%.*s xsi:type=\"xsd:%.*s\">%.*s</%.*s>",
                         U_STRING_TO_TRACE(argName),
                         U_STRING_TO_TRACE(argType),
                         U_STRING_TO_TRACE(argContent),
                         U_STRING_TO_TRACE(argName));

   arg.push(encodedValue);
}

UString USOAPEncoder::encodeMethod(URPCMethod& method, const UString& nsName) // namespace qualified element information
{
   U_TRACE(0, "USOAPEncoder::encodeMethod(%p,%.*S)", &method, U_STRING_TO_TRACE(nsName))

   method.encode(); // Encode the method by virtual method...

   if (method.hasFailed()) U_RETURN_STRING(encodedValue);

   uint32_t num_arg = arg.size();

   if      (num_arg  > 1) encodedValue = arg.join(0, 0);
   else if (num_arg == 1) encodedValue = arg[0];
   else                   encodedValue.setEmpty();

   UString methodName     = method.getMethodName(),
           headerContents = method.getHeaderContent();

   if (bIsResponse) (void) methodName.append(U_CONSTANT_TO_PARAM("Response"));

   uint32_t sz_nsName         =         nsName.size(),
            sz_methodName     =     methodName.size(),
            sz_encodedValue   =   encodedValue.size(),
            sz_headerContents = headerContents.size();

   const char* ptr_nsName         =         nsName.data();
   const char* ptr_methodName     =     methodName.data();
   const char* ptr_encodedValue   =   encodedValue.data();
   const char* ptr_headerContents = headerContents.data();

   buffer.setBuffer(400U + (sz_nsName * 4) + (sz_methodName * 2) + sz_encodedValue + sz_headerContents);

   buffer.snprintf("<?xml version='1.0' ?>"
                   "<env:Envelope xmlns:env=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
                     "<env:Header>%.*s</env:Header>"
                        "<env:Body>"
                           "<%.*s:%.*s env:encodingStyle=\"http://www.w3.org/2003/05/soap-encoding\" xmlns:%.*s=\"%.*s\">%.*s</%.*s:%.*s>"
                        "</env:Body>"
                   "</env:Envelope>",
                   sz_headerContents, ptr_headerContents,
                   sz_nsName, ptr_nsName,
                   sz_methodName, ptr_methodName,
                   sz_nsName, ptr_nsName,
                   sz_nsName, ptr_nsName,
                   sz_encodedValue, ptr_encodedValue,
                   sz_nsName, ptr_nsName,
                   sz_methodName, ptr_methodName);

   U_RETURN_STRING(buffer);
}
