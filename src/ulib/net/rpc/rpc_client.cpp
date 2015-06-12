// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_client.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/rpc/rpc_client.h>

bool URPCClient_Base::readResponse(USocket* sk, UString& buffer, UString& response)
{
   U_TRACE(0, "URPCClient_Base::readResponse(%p,%V,%V)", sk, buffer.rep, response.rep)

   uint32_t rstart = 0;

   // NB: we force for U_SUBSTR_INC_REF case (string can be referenced more)...

     buffer.setEmptyForce();
   response.setEmptyForce();

   if (URPC::readTokenString(sk, 0, buffer, rstart, response))
      {
      // NB: we force for U_SUBSTR_INC_REF case (string can be referenced more)...

      buffer.size_adjust_force(U_TOKEN_NM);
      }

   U_INTERNAL_DUMP("buffer = %V response = %V)", buffer.rep, response.rep)

   if (buffer) U_RETURN(true);

   U_RETURN(false);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* URPCClient_Base::dump(bool _reset) const { return UClient_Base::dump(_reset); }
#endif
