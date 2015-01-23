// HttpResponseHeader.h

#ifndef HTTP_RESPONSE_HEADER_H
#define HTTP_RESPONSE_HEADER_H 1

#include <HttpHeader.h>

class HttpResponseHeader : public HttpHeader {
public:
   UString httpver, status, reason;

   HttpResponseHeader(const char* h, unsigned h_len, const char* s, unsigned s_len, const char* r, unsigned r_len);

   HttpResponseHeader(const UString& httpver_, const UString& status_, const UString& reason_)
               : httpver(httpver_), status(status_), reason(reason_)
      {
      U_TRACE_REGISTER_OBJECT(5, HttpResponseHeader, "%.*S,%.*S,%.*S",
                              U_STRING_TO_TRACE(httpver_),  U_STRING_TO_TRACE(status_), U_STRING_TO_TRACE(reason_))
      }

   /** Destructor of the class.
   */
   ~HttpResponseHeader()
      {
      U_TRACE_UNREGISTER_OBJECT(0, HttpResponseHeader)
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
