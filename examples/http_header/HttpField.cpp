// HttpField.cpp

#include <HttpField.h>

void HttpField::stringify(UString& field)
{
   U_TRACE(5, "HttpField::stringify(%.*S)", U_STRING_TO_TRACE(field))

   field += name;
   field.append(U_CONSTANT_TO_PARAM(":"));
   field += value;
   field.append(U_CONSTANT_TO_PARAM("\r\n"));

   U_INTERNAL_DUMP("field = %.*S", U_STRING_TO_TRACE(field))
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* HttpField::dump(bool reset) const
{
   *UObjectIO::os << "name      (UString " << (void*)&name  << ")\n"
                  << "value     (UString " << (void*)&value << ")";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
