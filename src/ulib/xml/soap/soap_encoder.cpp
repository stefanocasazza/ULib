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
   U_TRACE(0, "USOAPEncoder::encodeArgument(%V,%V,%V)", argName.rep, argType.rep, argContent.rep)

   encodedValue.setBuffer(200U + (argName.size() * 2) + argType.size() + argContent.size());

   encodedValue.snprintf(U_CONSTANT_TO_PARAM("<%v xsi:type=\"xsd:%v\">%v</%v>"), argName.rep, argType.rep, argContent.rep, argName.rep);

   arg.push(encodedValue);
}

UString USOAPEncoder::encodeMethod(URPCMethod& method, const UString& nsName) // namespace qualified element information
{
   U_TRACE(0, "USOAPEncoder::encodeMethod(%p,%V)", &method, nsName.rep)

   method.encode(); // Encode the method by virtual method...

   if (method.hasFailed()) U_RETURN_STRING(encodedValue);

   uint32_t num_arg = arg.size();

   if      (num_arg  > 1) encodedValue = arg.join((const char*)0, 0U);
   else if (num_arg == 1) encodedValue = arg[0];
   else                   encodedValue.setEmpty();

   UString methodName     = method.getMethodName(),
           headerContents = method.getHeaderContent();

   if (bIsResponse) (void) methodName.append(U_CONSTANT_TO_PARAM("Response"));

   buffer.setBuffer(400U + (nsName.size() * 4) + (methodName.size() * 2) + encodedValue.size() + headerContents.size());

   buffer.snprintf(U_CONSTANT_TO_PARAM("<?xml version='1.0' ?>"
                   "<env:Envelope xmlns:env=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">"
                     "<env:Header>%v</env:Header>"
                        "<env:Body>"
                           "<%v:%v env:encodingStyle=\"http://www.w3.org/2003/05/soap-encoding\" xmlns:%v=\"%v\">%v</%v:%v>"
                        "</env:Body>"
                   "</env:Envelope>"), headerContents.rep, nsName.rep, methodName.rep, nsName.rep, nsName.rep, encodedValue.rep, nsName.rep,
                   methodName.rep);

   U_RETURN_STRING(buffer);
}
