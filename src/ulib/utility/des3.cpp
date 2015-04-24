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
#include <ulib/utility/base64.h>
#include <ulib/utility/string_ext.h>

UString UDES3::signData(const char* fmt, ...)
{
   U_TRACE(0, "UDES3::signData(%S)", fmt)

   uint32_t sz1;
   UString buffer1(U_CAPACITY), buffer2(U_CAPACITY), signed_data(U_CAPACITY);

   va_list argp;
   va_start(argp, fmt);

   buffer1.vsnprintf(fmt, argp);

   va_end(argp);

   sz1 = buffer1.size();

   if (sz1 <= 512) encode((const unsigned char*)buffer1.data(), sz1, buffer2);
   else
      {
      UString data = UStringExt::compress(buffer1.data(), sz1);
      uint32_t sz2 = data.size();

      if (sz2 < (sz1 - (sz1 / 4))) encode((const unsigned char*)   data.data(), sz2, buffer2);
      else                         encode((const unsigned char*)buffer1.data(), sz1, buffer2);
      }

   UBase64::encodeUrl(buffer2, signed_data);

   (void) signed_data.shrink();

   U_RETURN_STRING(signed_data);
}

UString UDES3::getSignedData(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UDES3::getSignedData(%.*S,%u)", len, ptr, len)

   UString buffer(U_CAPACITY), output(U_CAPACITY);

   bool old = (u_isBase64(ptr, len) &&
               UBase64::decode(ptr, len, buffer));

   if (old ||
       UBase64::decodeUrl(ptr, len, buffer))
      {
      UDES3::decode(buffer, output);

      if (UStringExt::isCompress(output)) output = UStringExt::decompress(output);

      (void) output.shrink();
      }

   U_RETURN_STRING(output);
}
