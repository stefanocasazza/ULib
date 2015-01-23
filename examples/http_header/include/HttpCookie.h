// HttpCookie.h

#ifndef HTTP_COOKIE_H
#define HTTP_COOKIE_H 1

#include <HttpField.h>

class HttpCookie : public HttpField {
public:
   /**
   * Parse an existed cookie extracting field for session: See RFC 2965
   */
   HttpCookie(const char* name_, unsigned name_len, const char* value_, unsigned value_len);

   /**
   * @param name_  The name of first field in cookie
   * @param value_ Optional value for field
   */
   HttpCookie(const UString& name_, const UString& value_) : HttpField(U_STRING_FROM_CONSTANT("Cookie"))
      {
      U_TRACE_REGISTER_OBJECT(5, HttpCookie, "%.*S,%.*S", U_STRING_TO_TRACE(name_),  U_STRING_TO_TRACE(value_))

      add(name_, value_);
      }

   /** Destructor of the class.
   */
   virtual ~HttpCookie()
      {
      U_TRACE_UNREGISTER_OBJECT(0, HttpCookie)
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
   * @param name_  Field name in cookie
   * @param value_ Value of field
   * @param path   Value of path
   * @param domain Value of domain
   * @param port   Value of port
   * @param index_ Index of field with name specified to find
   * return: true if succeded
   */
   bool find(const UString& name_, UString& value_, UString& path, UString& domain, UString& port, unsigned index_ = 0);

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
   bool is_path, is_domain, is_port;

   bool check(unsigned i);
};

#endif
