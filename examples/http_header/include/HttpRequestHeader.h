// HttpRequestHeader.h

#ifndef HTTP_REQUEST_HEADER_H
#define HTTP_REQUEST_HEADER_H 1

#include <HttpHeader.h>

class HttpRequestHeader : public HttpHeader {
public:
   UString method, url, httpver;

   HttpRequestHeader(const char* m, unsigned m_len, const char* u, unsigned u_len, const char* h, unsigned h_len);

   HttpRequestHeader(const UString& method_, const UString& url_, const UString& httpver_)
               : method(method_), url(url_), httpver(httpver_)
      {
      U_TRACE_CTOR(5, HttpRequestHeader, "%.*S,%.*S,%.*S",
                              U_STRING_TO_TRACE(method_),  U_STRING_TO_TRACE(url_), U_STRING_TO_TRACE(httpver_))
      }

   /** Destructor of the class.
   */
   ~HttpRequestHeader()
      {
      U_TRACE_DTOR(0, HttpRequestHeader)
      }

   /**
     * @param str Resulting header as a string
   */
    void stringify(UString& str);

   /// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif
};

#endif
