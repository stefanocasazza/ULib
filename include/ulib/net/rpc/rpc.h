// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc.h - RPC utility
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_RPC_H
#define ULIB_RPC_H 1

#include <ulib/utility/socket_ext.h>
#include <ulib/utility/string_ext.h>

// -------------------------------------------------------------------------------------------------
// Very simple RPC-like layer
//
// Requests and responses are build of little packets each containing a U_TOKEN_NM-byte ascii token,
// an 8-byte hex value or length, and optionally data corresponding to the length
// -------------------------------------------------------------------------------------------------

class U_EXPORT URPC {
public:

   // RPC representation

   static UVector<UString>* rpc_info;

   static void allocate()
      {
      U_TRACE_NO_PARAM(0, "URPC::allocate()")

      U_INTERNAL_ASSERT_EQUALS(rpc_info,0)

      U_NEW(UVector<UString>, rpc_info, UVector<UString>);
      }

   static void resetInfo()
      {
      U_TRACE_NO_PARAM(0, "URPC::resetInfo()")

      U_INTERNAL_ASSERT_POINTER(rpc_info)

      rpc_info->clear();
      }

   static bool readRequest(USocket* s);

   // Read a token and value

   static uint32_t readTokenInt(USocket* s, const char* token, UString& buffer, uint32_t& rstart);

   // Read a token, and then the string data

   static uint32_t readTokenString(USocket* s, const char* token, UString& buffer, uint32_t& rstart, UString& data);

   // Read an vector of string from the network

   static uint32_t readTokenVector(USocket* s, const char* token, UString& buffer, UVector<UString>& vec);

   // Transmit token name (U_TOKEN_NM characters) and value (32-bit int, as 8 hex characters)

   static bool sendTokenInt(USocket* s, const char* token, uint32_t value, UString& buffer)
      {
      U_TRACE(0, "URPC::sendTokenInt(%p,%S,%u,%V)", s, token, value, buffer.rep)

      UStringExt::buildTokenInt(token, value, buffer);

      bool result = USocketExt::write(s, buffer, -1);

      U_RETURN(result);
      }

   // Write a token, and then the string data

   static bool sendTokenString(USocket* s, const char* token, const UString& data, UString& buffer)
      {
      U_TRACE(0, "URPC::sendTokenString(%p,%S,%V,%V)", s, token, data.rep, buffer.rep)

      UStringExt::buildTokenString(token, data, buffer);

      bool result = USocketExt::write(s, buffer, -1);

      U_RETURN(result);
      }

   // Transmit an vector of string

   static bool sendTokenVector(USocket* s, const char* token, UVector<UString>& vec, UString& buffer)
      {
      U_TRACE(0, "URPC::sendTokenVector(%p,%S,%p,%V)", s, token, &vec, buffer.rep)

      UStringExt::buildTokenVector(token, vec, buffer);

      bool result = USocketExt::write(s, buffer, -1);

      U_RETURN(result);
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(URPC)
};

#endif
