// HttpBaAuthorization.h

#ifndef HTTP_BA_AUTHORIZATION_H
#define HTTP_BA_AUTHORIZATION_H 1

#include <HttpField.h>

class HttpBaAuthorization : public HttpField {
public:
   HttpBaAuthorization(const char* name_, unsigned name_len, const char* value_, unsigned value_len);

    /**
      * @param user_   User for basic authorization
      * @param passwd_ Password for basic authorization
      */
    HttpBaAuthorization(const UString& user_, const UString& passwd_) : HttpField(U_STRING_FROM_CONSTANT("Authorization")),
         user(user_), passwd(passwd_)
      {
      U_TRACE_CTOR(5, HttpBaAuthorization, "%.*S,%.*S", U_STRING_TO_TRACE(user_), U_STRING_TO_TRACE(passwd_))
      }

   /** Destructor of the class.
   */
   virtual ~HttpBaAuthorization()
      {
      U_TRACE_DTOR(5, HttpBaAuthorization)
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
public:
   UString user, passwd;
};

#endif
