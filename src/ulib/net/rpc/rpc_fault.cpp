// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_fault.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/rpc/rpc_fault.h>

void URPCFault::setDetail(const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0, "URPCFault::setDetail(%.*S,%u)", fmt_size, format, fmt_size)

   va_list argp;
   va_start(argp, fmt_size);

   detail.vsnprintf(format, fmt_size, argp);

   va_end(argp);
}

UString URPCFault::getFaultCode()
{
   U_TRACE_NO_PARAM(0, "URPCFault::getFaultCode()")

   UString retval;

   /* SOAP:
    * ----------------------------------------------------------------------------------------------------
    * VersionMismatch: The faulting node found an invalid element information item instead of the expected
    * Envelope element information item. The namespace, local name or both did not match
    * the Envelope element information item required by this recommendation.  
    *
    * MustUnderstand: An immediate child element information item of the SOAP Header element information
    * item targeted at the faulting node that was not understood by the faulting node
    * contained a SOAP mustUnderstand attribute information item with a value of "true".
    *
    * DataEncodingUnknown: A SOAP header block or SOAP body child element information item targeted at the
    * faulting SOAP node is scoped with a data encoding that the faulting node does not support.
    *
    * Sender: The message was incorrectly formed or did not contain the appropriate information in order to
    * succeed. For example, the message could lack the proper authentication or payment information.
    * It is generally an indication that the message is not to be resent without change.
    *
    * Receiver: The message could not be processed for reasons attributable to the processing of the message
    * rather than to the contents of the message itself. For example, processing could include
    * communicating with an upstream SOAP node, which did not respond.  The message could succeed
    * if resent at a later point in time
    * ----------------------------------------------------------------------------------------------------
    */

   switch (faultCode)
      {
      case Sender:               (void) retval.assign(U_CONSTANT_TO_PARAM("Client"));              break;
      case Receiver:             (void) retval.assign(U_CONSTANT_TO_PARAM("Server"));              break;
      case MustUnderstand:       (void) retval.assign(U_CONSTANT_TO_PARAM("MustUnderstand"));      break;
      case VersionMismatch:      (void) retval.assign(U_CONSTANT_TO_PARAM("VersionMismatch"));     break;
      case DataEncodingUnknown:  (void) retval.assign(U_CONSTANT_TO_PARAM("DataEncodingUnknown")); break;
      }

   U_RETURN_STRING(retval);
}

void URPCFault::encode(UString& response)
{
   U_TRACE(0, "URPCFault::encode(%V)", response.rep)

   UString code = getFaultCode();

   response.setBuffer(100U + code.size() + faultReason.size() + detail.size());

   response.snprintf(U_CONSTANT_TO_PARAM("%v: %v%s%v"), code.rep, faultReason.rep, (detail.empty() ? "" : " - "), detail.rep);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* URPCFault::dump(bool reset) const
{
   *UObjectIO::os << "faultCode              " << faultCode           << '\n'
                  << "detail        (UString " << (void*)&detail      << ")\n"
                  << "faultReason   (UString " << (void*)&faultReason << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
