// HttpOtpPostLogin.cpp

#include <HttpOtpPostLogin.h>

#ifdef U_PROXY_UNIT
#  include <URL.h>
#else
#  include <ulib/url.h>
#endif

HttpOtpPostLogin::HttpOtpPostLogin(const char* buf, unsigned len, const UString& user_field,
                                   const UString& pin_field, const UString& token_field, const UString& passwd_field,
                                   const UString& cf_field, HttpHeader& header)
{
   U_TRACE(5, "HttpOtpPostLogin::HttpOtpPostLogin(%.*S,%u,%.*S,%.*S,%.*S,%.*S,%p)", len, buf, len,
                     U_STRING_TO_TRACE(user_field), U_STRING_TO_TRACE(pin_field), U_STRING_TO_TRACE(token_field),
                     U_STRING_TO_TRACE(passwd_field), U_STRING_TO_TRACE(cf_field), &header)

   (void) header.find(U_STRING_FROM_CONSTANT("Content-Type"));

   buffer.assign(buf, len);

   (void) U_VEC_SPLIT(vec, buffer, "=&"); // "user=stefano+casazza&pin=12345&token=autorizzativo"

   unsigned i = 0;

   while (i < vec.size())
      {
      if (vec[i] == user_field)
         {
         U_STR_RESERVE(user, 64);

         Url::decode(vec[i+1], user);
         }
      else if (vec[i] == pin_field)
         {
         U_STR_RESERVE(pin, 64);

         Url::decode(vec[i+1], pin);
         }
      else if (vec[i] == token_field)
         {
         U_STR_RESERVE(token, 64);

         Url::decode(vec[i+1], token);
         }
      else if (vec[i] == passwd_field)
         {
         U_STR_RESERVE(passwd, 64);

         Url::decode(vec[i+1], passwd);
         }
      else if (vec[i] == cf_field)
         {
         U_STR_RESERVE(cf, 64);

         Url::decode(vec[i+1], cf);
         }

      i += 2;
      }
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* HttpOtpPostLogin::dump(bool reset) const
{
   *UObjectIO::os << "cf        (UString " << (void*)&cf     << ")\n"
                  << "vec       (UVector " << (void*)&vec    << ")\n"
                  << "pin       (UString " << (void*)&pin    << ")\n"
                  << "user      (UString " << (void*)&user   << ")\n"
                  << "token     (UString " << (void*)&token  << ")\n"
                  << "passwd    (UString " << (void*)&passwd << ")\n"
                  << "buffer    (UString " << (void*)&buffer << ")";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
