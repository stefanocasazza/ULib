// HttpOtpPostLogin.h

#ifndef HTTP_OTP_POST_LOGIN_H
#define HTTP_OTP_POST_LOGIN_H 1

#include <HttpHeader.h>

class HttpOtpPostLogin {
public:
   //// Check for memory error
   U_MEMORY_TEST

   /// = Allocator e Deallocator.
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UString user, pin, token, passwd, cf;

   /**
    * @param user_field   Name of field where username is stored
    * @param pin_field    Name of field where PIN is stored
    * @param token_field  Name of field where Token is stored
    * @param passwd_field Name of field where passwd is stored
    * @param cf           Name of field where cf is stored
    * @param header       HttpOtpPostLogin object of HTTP request
    * @param buf          Input buffer containing OTP authentication token
    * @param len          Bytes in input buffer
    */
    HttpOtpPostLogin(const char* buf, unsigned len,
                     const UString& user_field, const UString& pin_field, const UString& token_field, const UString& passwd_field,
                     const UString& cf_field, HttpHeader& header);

   /** Destructor of the class.
   */
   ~HttpOtpPostLogin()
      {
      U_TRACE_UNREGISTER_OBJECT(0, HttpOtpPostLogin)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString buffer;
   UVector<UString> vec;
};

#endif
