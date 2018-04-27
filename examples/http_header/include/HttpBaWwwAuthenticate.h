// HttpBaWwwAuthenticate.h

#ifndef HTTP_BA_WWW_AUTH_H
#define HTTP_BA_WWW_AUTH_H 1

#include <HttpField.h>

class HttpBaWwwAuthenticate : public HttpField {
public:
   UString realm;

   HttpBaWwwAuthenticate(const char* name_, unsigned name_len, const char* value_, unsigned value_len);

    /**
      * @param realm_ Realm for authentication
      */
    HttpBaWwwAuthenticate(const UString& realm_) : HttpField(U_STRING_FROM_CONSTANT("WWW-Authenticate")), realm(realm_)
      {
      U_TRACE_CTOR(5, HttpBaWwwAuthenticate, "%.*S", U_STRING_TO_TRACE(realm_))
      }

   /** Destructor of the class.
   */
   virtual ~HttpBaWwwAuthenticate()
      {
      U_TRACE_DTOR(0, HttpBaWwwAuthenticate)
      }

   /**
   * @param field_ String where to save header as string
   */
   virtual void stringify(UString& field);

   /// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString buffer;
};

#endif
