// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    des3.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_DES3_H
#define ULIB_DES3_H 1

#include <ulib/base/ssl/des3.h>
#include <ulib/utility/base64.h>

struct U_EXPORT UDES3 {

   static void setPassword(const char* passwd)
      {
      U_TRACE(0, "UDES3::setPassword(%S)", passwd)

      u_des3_key(passwd);
      }

   static UString signData(const UString& data)
      {
      U_TRACE(0, "UDES3::signData(%V)", data.rep)

      uint32_t sz;
      UString buffer((sz = data.size())+32U);

      encode((const unsigned char*)data.data(), sz, buffer);

      UString signed_data((sz = buffer.size())*4);

      UBase64::encodeUrl(buffer.data(), sz, signed_data);

      U_RETURN_STRING(signed_data);
      }

   static UString signData(const char* fmt, uint32_t fmt_size, ...);

   static UString getSignedData(const char* ptr, uint32_t len);

   static UString getSignedData(const UString& s) { return getSignedData(U_STRING_TO_PARAM(s)); }

   static void encode(const unsigned char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UDES3::encode(%.*S,%u,%p)", n, s, n, &buffer)

      U_ASSERT(buffer.uniq())
      U_ASSERT(buffer.capacity() >= n)

      buffer.rep->_length = u_des3_encode(s, n, (unsigned char*)buffer.data());
      }

   static void encode(const UString& s, UString& buffer) { encode((const unsigned char*)U_STRING_TO_PARAM(s), buffer); }

   static void decode(const unsigned char* s, uint32_t n, UString& buffer)
      {
      U_TRACE(0, "UDES3::decode(%.*S,%u,%p)", n, s, n, &buffer)

      U_ASSERT(buffer.uniq())
      U_ASSERT(buffer.capacity() >= n)

      buffer.rep->_length = u_des3_decode(s, n, (unsigned char*)buffer.data());

      U_INTERNAL_ASSERT(buffer.invariant())

      U_INTERNAL_DUMP("buffer(%u) = %#V", buffer.size(), buffer.rep)
      }

   static void decode(const UString& s, UString& buffer) { decode((const unsigned char*)U_STRING_TO_PARAM(s), buffer); }
};

#endif
