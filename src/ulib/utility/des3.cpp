// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    des3.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/des3.h>

UString UDES3::signData(const char* fmt, uint32_t fmt_size, ...)
{
   U_TRACE(0, "UDES3::signData(%.*S,%u)", fmt_size, fmt, fmt_size)

   UString x, data(U_CAPACITY);

   va_list argp;
   va_start(argp, fmt_size);

   data.vsnprintf(fmt, fmt_size, argp);

   va_end(argp);

   x = signData(data);

   U_RETURN_STRING(x);
}

UString UDES3::getSignedData(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UDES3::getSignedData(%.*S,%u)", len, ptr, len)

   UString buffer(len+32U);

   UBase64::decodeUrl(ptr, len, buffer);

   if (buffer &&
       u_base64_errors == 0)
      {
      uint32_t sz = buffer.size();
      UString output(sz+32U);

      decode((const unsigned char*)buffer.data(), sz, output);

      U_RETURN_STRING(output);
      }

   return UString::getStringNull();
}
