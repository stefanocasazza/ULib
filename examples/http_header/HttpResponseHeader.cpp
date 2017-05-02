// HttpResponseHeader.cpp

#include <HttpResponseHeader.h>

HttpResponseHeader::HttpResponseHeader(const char* h, unsigned h_len, const char* s, unsigned s_len, const char* r, unsigned r_len)
{
   U_TRACE_REGISTER_OBJECT(5, HttpResponseHeader, "%.*S,%u,%.*S,%u,%.*S,%u", h_len, h, h_len, s_len, s, s_len, r_len, r, r_len)

   httpver.assign(h, h_len);
   status.assign(s, s_len);
   reason.assign(r, r_len);
}

void HttpResponseHeader::stringify(UString& field)
{
   U_TRACE(5, "HttpResponseHeader::stringify(%.*S)", U_STRING_TO_TRACE(field))

   field += httpver;
   field.append(U_CONSTANT_TO_PARAM(" "));
   field += status;
   field.append(U_CONSTANT_TO_PARAM(" "));
   field += reason;
   field.append(U_CONSTANT_TO_PARAM("\r\n"));

   HttpHeader::stringify(field);

   U_INTERNAL_DUMP("field = %.*S", U_STRING_TO_TRACE(field))
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* HttpResponseHeader::dump(bool reset) const
{
   *UObjectIO::os << "httpver   (UString " << (void*)&httpver << ")\n"
                  << "status    (UString " << (void*)&status  << ")\n"
                  << "reason    (UString " << (void*)&reason  << ")";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
