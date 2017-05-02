// HttpRequestHeader.cpp

#include <HttpRequestHeader.h>

HttpRequestHeader::HttpRequestHeader(const char* m, unsigned m_len, const char* u, unsigned u_len, const char* h, unsigned h_len)
{
   U_TRACE_REGISTER_OBJECT(5, HttpRequestHeader, "%.*S,%u,%.*S,%u,%.*S,%u", m_len, m, m_len, u_len, u, u_len, h_len, h, h_len)

   url.assign(u, u_len);
   method.assign(m, m_len);
   httpver.assign(h, h_len);
}

void HttpRequestHeader::stringify(UString& field)
{
   U_TRACE(5, "HttpRequestHeader::stringify(%.*S)", U_STRING_TO_TRACE(field))

   field += method;
   field.append(U_CONSTANT_TO_PARAM(" "));
   field += url;
   field.append(U_CONSTANT_TO_PARAM(" "));
   field += httpver;
   field.append(U_CONSTANT_TO_PARAM("\r\n"));

   HttpHeader::stringify(field);

   U_INTERNAL_DUMP("field = %.*S", U_STRING_TO_TRACE(field))
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* HttpRequestHeader::dump(bool reset) const
{
   *UObjectIO::os << "url       (UString " << (void*)&url     << "\n"
                  << "method    (UString " << (void*)&method  << "\n"
                  << "httpver   (UString " << (void*)&httpver << ")";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
