// HttpSetCookie.h

#ifndef HTTP_SETCOOKIE_H
#define HTTP_SETCOOKIE_H 1

#include <HttpField.h>

class HttpSetCookie : public HttpField {
public:
   /**
   * Parse an existed cookie extracting field for session: See RFC 2965
   */
    HttpSetCookie(const char* name_, unsigned name_len_, const char* value_, unsigned value_len_);

   /**
   * @param name_  The name of first field in cookie
   * @param value_ Optional value for field
   */
   HttpSetCookie(const UString& name_, const UString& value_, bool version2_)
         : HttpField(version2_ ? U_STRING_FROM_CONSTANT("Set-Cookie2") : U_STRING_FROM_CONSTANT("Set-Cookie"))
      {
      U_TRACE_CTOR(5, HttpSetCookie, "%.*S,%.*S,%b", U_STRING_TO_TRACE(name_),  U_STRING_TO_TRACE(value_), version2_)

      version2 = version2_;

      add(name_, value_);
      }

   /** Destructor of the class.
   */
   virtual ~HttpSetCookie()
      {
      U_TRACE_DTOR(0, HttpSetCookie)
      }

   /**
   * @param name_ Field name in cookie
   * return: number of field founded
   */
   unsigned count(const UString& name_);

   /**
   * @param name_   name of field in cookie
   * @param value_ Value of field in cookie
   */
   void add(const UString& name_, const UString& value_);

   /**
   * @param name_       Field name in cookie
   * @param value_      Value of field
   * @param path        Value of path
   * @param domain      Value of domain
   * @param max_age     Value of max-age
   * @param comment     Value of comment
   * @param comment_url Value of comment_url
   * @param version     Value of version
   * @param port        Value of port
   * @param secure      Value of secure
   * @param discard     Value of discard
   * @param index       Index of field with name specified to find
   * return: true if succeded
   */
   bool find(const UString& name_,   UString& value_,  UString& path,        UString& domain,
                   UString& max_age, UString& comment, UString& comment_url, UString& port,
                   UString& version, bool& secure, bool& discard, unsigned index = 0);

   /**
   * @param name_  Name of cookie
   * @param index_ Index of field with name specified to find
   * return: true if succeded
   */
   bool del(const UString& name_, unsigned index_ = 0);

   /**
   * @param name_  Field name in cookie
   * return: true if succeded
   */
   bool del_all(const UString& name_);

   /**
     * @param str_ Where to save cookie
     */
   virtual void stringify(UString& str_);

   /// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString buffer;
public:
   UVector<UString> vec;
   bool version2, is_path, is_domain, is_max_age, is_comment, is_comment_url, is_port, is_version, is_secure, is_discard;

   bool check(unsigned i);
};

#endif
