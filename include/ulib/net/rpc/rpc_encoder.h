// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rpc_encoder.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_RPC_ENCODER_H
#define ULIB_RPC_ENCODER_H 1

#include <ulib/net/rpc/rpc_fault.h>
#include <ulib/net/rpc/rpc_method.h>
#include <ulib/utility/string_ext.h>

#define U_ENCODER_CLEAR_DATA        URPCMethod::encoder->clearData()
#define U_RPC_ENCODE_ARG(arg)       URPCMethod::encoder->encodeArgument(UString::getStringNull(), UString::getStringNull(), arg)
#define U_RPC_ENCODE_RES(tok,res)   URPCMethod::encoder->encodeResponse(tok, res)

/**
 * @class URPCEncoder
 */

class U_EXPORT URPCEncoder {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   URPCEncoder() : buffer(U_CAPACITY), encodedValue(U_CAPACITY)
      {
      U_TRACE_REGISTER_OBJECT(0, URPCEncoder, "", 0)

      bIsResponse = false;
      }

   virtual ~URPCEncoder()
      {
      U_TRACE_UNREGISTER_OBJECT(0, URPCEncoder)
      }

   void clearData()
      {
      U_TRACE_NO_PARAM(0, "URPCEncoder::clearData()")

      arg.clear();

            buffer.setEmptyForce();
      encodedValue.setEmptyForce();
      }

   // Used to indicate if the encoder is currently closing a response

   bool isEncodingResponse() const { return bIsResponse; }

   // ENCODING FUNCTIONS

   virtual void encodeArgument(const UString& argName, const UString& argType, const UString& argContent)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%V)", argName.rep, argType.rep, argContent.rep)

      if (   argName) arg.push(argName);
      if (   argType) arg.push(argType);
      if (argContent) arg.push(argContent);
      }

   void encodeArgument(const UString& argName, const UString& argType, bool value)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%b)", argName.rep, argType.rep, value)

      buffer.push_back(value ? '1' : '0');

      encodeArgument(argName, argType, buffer);
      }

   void encodeArgument(const UString& argName, const UString& argType, char value)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%C)", argName.rep, argType.rep, value)

      buffer.push_back(value);

      encodeArgument(argName, argType, buffer);
      }

   void encodeArgument(const UString& argName, const UString& argType, unsigned char value)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%C)", argName.rep, argType.rep, value)

      buffer.push_back(value);

      encodeArgument(argName, argType, buffer);
      }

   void encodeArgument(const UString& argName, const UString& argType, short value)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%d)", argName.rep, argType.rep, value)

      buffer.setFromNumber32s(value);

      encodeArgument(argName, argType, buffer);
      }

   void encodeArgument(const UString& argName, const UString& argType, unsigned short value)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%d)", argName.rep, argType.rep, value)

      buffer.setFromNumber32(value);

      encodeArgument(argName, argType, buffer);
      }

   void encodeArgument(const UString& argName, const UString& argType, int value)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%d)", argName.rep, argType.rep, value)

      buffer.setFromNumber32s(value);

      encodeArgument(argName, argType, buffer);
      }

   void encodeArgument(const UString& argName, const UString& argType, unsigned int value)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%u)", argName.rep, argType.rep, value)

      buffer.setFromNumber32(value);

      encodeArgument(argName, argType, buffer);
      }

   void encodeArgument(const UString& argName, const UString& argType, long int value)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%ld)", argName.rep, argType.rep, value)

#  if SIZEOF_LONG == 4
      buffer.setFromNumber32s(value);
#  else
      buffer.setFromNumber64s(value);
#  endif

      encodeArgument(argName, argType, buffer);
      }

   void encodeArgument(const UString& argName, const UString& argType, unsigned long int value)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%lu)", argName.rep, argType.rep, value)

#  if SIZEOF_LONG == 4
      buffer.setFromNumber32(value);
#  else
      buffer.setFromNumber64(value);
#  endif

      encodeArgument(argName, argType, buffer);
      }

   void encodeArgument(const UString& argName, const UString& argType, const long long& value)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%lld)", argName.rep, argType.rep, value)

      buffer.setFromNumber64s(value);

      encodeArgument(argName, argType, buffer);
      }

   void encodeArgument(const UString& argName, const UString& argType, const unsigned long long& value)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%llu)", argName.rep, argType.rep, value)

      buffer.setFromNumber64(value);

      encodeArgument(argName, argType, buffer);
      }

   void encodeArgument(const UString& argName, const UString& argType, float value)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%f)", argName.rep, argType.rep, value)

      buffer.snprintf(U_CONSTANT_TO_PARAM("%f"), value);

      encodeArgument(argName, argType, buffer);
      }

   void encodeArgument(const UString& argName, const UString& argType, const double& value)
      {
      U_TRACE(0, "URPCEncoder::encodeArgument(%V,%V,%g)", argName.rep, argType.rep, value)

      buffer.snprintf(U_CONSTANT_TO_PARAM("%g"), value);

      encodeArgument(argName, argType, buffer);
      }

   // Given a method, turns it into a message

   UString encodeMethodCall(URPCMethod& method, const UString& nsName)
      {
      U_TRACE(0, "URPCEncoder::encodeMethodCall(%p,%V)", &method, nsName.rep)

      bIsResponse = false;

      return encodeMethod(method, nsName);
      }

   UString encodeMethodResponse(URPCMethod& method, const UString& nsName)
      {
      U_TRACE(0, "URPCEncoder::encodeMethodResponse(%p,%V)", &method, nsName.rep)

      bIsResponse = true;

      return encodeMethod(method, nsName);
      }

   void encodeResponse(const char* token, const UString& response)
      {
      U_TRACE(0, "URPCEncoder::encodeResponse(%S,%V)", token, response.rep)

      encodedValue.setBuffer(response.size() + U_TOKEN_LN);

      UStringExt::buildTokenString(token, response, encodedValue);
      }

   // Encodes the complete fault into a complete URPCEnvelope

   virtual UString encodeFault(URPCFault* fault)
      {
      U_TRACE(0, "URPCEncoder::encodeFault(%p)", fault)

      fault->encode(buffer); // Encode the fault

      U_RPC_ENCODE_RES("ERR ", buffer);

      U_RETURN_STRING(encodedValue);
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString buffer,
           encodedValue; // Keeps the value of the encoded string as this object
                         // moves itself throughout the encoding process
   UVector<UString> arg; // this contains the argument for the method the caller wants to execute to encode
   bool bIsResponse;     // Remembers if this item is encoding a response

   // Called by the encodeMethodResponse and encodeMethodCall members to finish encoding the method.
   // As you may guess, the two methods share a lot, and this item holds that common code

   virtual UString encodeMethod(URPCMethod& method, const UString& nsName); // namespace qualified element information

private:
   U_DISALLOW_COPY_AND_ASSIGN(URPCEncoder)
};

#endif
