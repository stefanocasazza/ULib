// HttpLocation.h

#ifndef HTTP_LOCATION_H
#define HTTP_LOCATION_H 1

#include <HttpField.h>

class HttpLocation : public HttpField {
public:
   UString url;

   HttpLocation(const char* name_, unsigned name_len, const char* value_, unsigned value_len);

    /**
      * @param url_ Redirect URL
      */
    HttpLocation(const UString& url_) : HttpField(U_STRING_FROM_CONSTANT("Location")), url(url_)
      {
      U_TRACE_REGISTER_OBJECT(5, HttpLocation, "%.*S", U_STRING_TO_TRACE(url_))
      }

   /** Destructor of the class.
   */
   virtual ~HttpLocation()
      {
      U_TRACE_UNREGISTER_OBJECT(0, HttpLocation)
      }

   /**
   * @param field_ String where to save header as string
   */
   virtual void stringify(UString& field);

   /// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif
};

#endif
