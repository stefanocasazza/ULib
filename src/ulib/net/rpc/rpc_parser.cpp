// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_parser.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/rpc/rpc_object.h>
#include <ulib/net/rpc/rpc_parser.h>
#include <ulib/net/rpc/rpc_encoder.h>

// gcc: call is unlikely and code size would grow

URPCParser::URPCParser(UVector<UString>* arg)
{
   U_TRACE_REGISTER_OBJECT(0, URPCParser, "%p", arg)

   if (arg == 0)
      {
      URPC::allocate();

      arg = URPC::rpc_info;
      }

   envelope.arg = arg;
}

void URPCParser::clearData()
{
   U_TRACE_NO_PARAM(0, "URPCParser::clearData()")

   U_ENCODER_CLEAR_DATA;

   envelope.arg->clear();
   envelope.nsName.clear();
   envelope.methodName.clear();

   if (URPCMethod::pFault)
      {
      delete URPCMethod::pFault;
             URPCMethod::pFault = 0;
      }
}

UString URPCParser::processMessage(const UString& msg, URPCObject& object, bool& bContainsFault)
{
   U_TRACE(0, "URPCParser::processMessage(%V,%p,%p)", msg.rep, &object, &bContainsFault)

   U_INTERNAL_ASSERT(msg)

   envelope.methodName = msg;

   UString retval = object.processMessage(envelope, bContainsFault);

   retval.duplicate(); // NB: is needed to permit to call clearData() after...

   clearData(); // to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...

   U_RETURN_STRING(retval);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* URPCParser::dump(bool reset) const
{
   *UObjectIO::os << "envelope        (URPCEnvelope                     " << (void*)&envelope << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
