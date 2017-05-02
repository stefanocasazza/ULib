// HttpBaWwwAuthenticate.cpp

#include <HttpBaWwwAuthenticate.h>

HttpBaWwwAuthenticate::HttpBaWwwAuthenticate(const char* name_, unsigned name_len, const char* value_, unsigned value_len)
{
   U_TRACE_REGISTER_OBJECT(5, HttpBaWwwAuthenticate, "%.*S,%u,%.*S,%u", name_len, name_, name_len, value_len, value_, value_len)

   U_INTERNAL_ASSERT(memcmp(name_,  U_CONSTANT_TO_PARAM("WWW-Authenticate")) == 0)
   U_INTERNAL_ASSERT(memcmp(value_, U_CONSTANT_TO_PARAM(" Basic realm")) == 0)

   name.assign(name_, name_len);

   UVector<UString> vec;
   buffer.assign(value_, value_len);

   (void) U_VEC_SPLIT(vec, buffer, "=");

   realm = vec[2];
}

void HttpBaWwwAuthenticate::stringify(UString& field)
{
   U_TRACE(5, "HttpBaWwwAuthenticate::stringify(%.*S)", U_STRING_TO_TRACE(field))

   field += name;

   field.append(U_CONSTANT_TO_PARAM(": Basic realm="));

   bool quoted = (realm[0] == '"');

   if (quoted == false) field += '"';

   field += realm;

   if (quoted == false) field += '"';

   field.append(U_CONSTANT_TO_PARAM("\r\n"));

   U_INTERNAL_DUMP("field = %.*S", U_STRING_TO_TRACE(field))
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* HttpBaWwwAuthenticate::dump(bool reset) const
{
   HttpField::dump(false);

   *UObjectIO::os << "\n"
                  << "realm     (UString " << (void*)&realm  << ")\n"
                  << "buffer    (UString " << (void*)&buffer << ")";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
