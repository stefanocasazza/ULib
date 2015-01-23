// HttpBaAuthorization.cpp

#include <HttpBaAuthorization.h>

#ifdef U_PROXY_UNIT
#  include <Base64engine.h>
#else
#  include <ulib/utility/base64.h>
#endif

HttpBaAuthorization::HttpBaAuthorization(const char* name_, unsigned name_len, const char* value_, unsigned value_len)
{
   U_TRACE_REGISTER_OBJECT(5, HttpBaAuthorization, "%.*S,%u,%.*S,%u", name_len, name_, name_len, value_len, value_, value_len)

   U_INTERNAL_ASSERT(memcmp(name_,  U_CONSTANT_TO_PARAM("Authorization")) == 0)
   U_INTERNAL_ASSERT(memcmp(value_, U_CONSTANT_TO_PARAM(" Basic ")) == 0)

   name.assign(name_, name_len);

   unsigned len    = value_len - (sizeof(" Basic ")-1);
   const char* ptr = value_    + (sizeof(" Basic ")-1);

   U_STR_RESERVE(buffer, len * 4);

#ifdef U_PROXY_UNIT
   Base64engine eng;
   long pos = eng.decode(ptr, len, (unsigned char*)buffer.data());
   buffer.resize(pos);
#else
   if (UBase64::decode(ptr, len, buffer))
#endif
      {
      unsigned index = buffer.find(':');

      if (index != unsigned(-1))
         {
         user   = buffer.substr(0U, (uint32_t)index);
         passwd = buffer.substr(index + 1);
         }
      }
}

void HttpBaAuthorization::stringify(UString& field)
{
   U_TRACE(5, "HttpBaAuthorization::stringify(%.*S)", U_STRING_TO_TRACE(field))

   UString tmp = user;
   tmp += ':';
   tmp += passwd;

   unsigned n = tmp.size() * 4;

   U_STR_RESERVE(buffer, n);

#ifdef U_PROXY_UNIT
   Base64engine eng;
   long pos = eng.encode((unsigned char*)tmp.data(), tmp.size(), (unsigned char*)buffer.data());
   buffer.resize(pos);
#else
   UBase64::encode(tmp, buffer);
#endif

   field += name;
   field.append(U_CONSTANT_TO_PARAM(": Basic "));
   field += buffer;
   field.append(U_CONSTANT_TO_PARAM("\r\n"));

   U_INTERNAL_DUMP("field = %.*S", U_STRING_TO_TRACE(field))
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* HttpBaAuthorization::dump(bool reset) const
{
   HttpField::dump(false);

   *UObjectIO::os << "\n"
                  << "user      (UString " << (void*)&user   << ")\n"
                  << "passwd    (UString " << (void*)&passwd << ")\n"
                  << "buffer    (UString " << (void*)&buffer << ")";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
