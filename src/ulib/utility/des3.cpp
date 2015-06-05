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

   UString buffer1(U_CAPACITY), buffer2(U_CAPACITY), signed_data(U_CAPACITY);
   const char* ptr = buffer1.data();

   va_list argp;
   va_start(argp, fmt);

   buffer1.vsnprintf(fmt, argp);

   va_end(argp);

// uint32_t sz1 = buffer1.size();
//
// if (sz1 <= 2048) encode((const unsigned char*)ptr, sz1, buffer2);
// else
//    {
//    UString data = UStringExt::compress(ptr, sz1);
//    uint32_t sz2 = data.size();
//
//    if (sz2 < (sz1 - (sz1 / 4))) encode((const unsigned char*)data.data(), sz2, buffer2);
//    else                         encode((const unsigned char*)ptr,         sz1, buffer2);
//    }

   encode((const unsigned char*)ptr, buffer1.size(), buffer2);

   UBase64::encodeUrl(buffer2, signed_data);

   (void) signed_data.shrink();

   U_RETURN_STRING(signed_data);
}

UString UDES3::getSignedData(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UDES3::getSignedData(%.*S,%u)", len, ptr, len)

   UString buffer(U_CAPACITY), output(U_CAPACITY);

   UBase64::decodeUrl(ptr, len, buffer);

   if (buffer &&
       u_base64_errors == 0)
      {
      UDES3::decode(buffer, output);

      if (output)
         {
         if (UStringExt::isCompress(output)) output = UStringExt::decompress(output);

         (void) output.shrink();
         }
      }

   U_RETURN_STRING(output);
}
