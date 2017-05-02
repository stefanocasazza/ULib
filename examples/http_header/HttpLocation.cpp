// HttpLocation.cpp

#include <HttpLocation.h>

HttpLocation::HttpLocation(const char* name_, unsigned name_len, const char* value_, unsigned value_len)
{
   U_TRACE_REGISTER_OBJECT(5, HttpLocation, "%.*S,%u,%.*S,%u", name_len, name_, name_len, value_len, value_, value_len)

   U_INTERNAL_ASSERT(memcmp(name_,  U_CONSTANT_TO_PARAM("Location")) == 0)

   name.assign(name_, name_len);

   trim(value_, value_len);

   url.assign(value_, value_len);
}

void HttpLocation::stringify(UString& field)
{
   U_TRACE(5, "HttpLocation::stringify(%.*S)", U_STRING_TO_TRACE(field))

   field += name;
   field.append(U_CONSTANT_TO_PARAM(": "));
   field += url;
   field.append(U_CONSTANT_TO_PARAM("\r\n"));

   U_INTERNAL_DUMP("field = %.*S", U_STRING_TO_TRACE(field))
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* HttpLocation::dump(bool reset) const
{
   HttpField::dump(false);

   *UObjectIO::os << "\n"
                  << "url       (UString " << (void*)&url  << ")";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
