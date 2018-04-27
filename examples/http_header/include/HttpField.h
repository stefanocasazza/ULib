// HttpField.h

#ifndef HTTP_FIELD_H
#define HTTP_FIELD_H 1

#include <ulib/internal/portable.h>

#include <ctype.h>

class HttpField {
public:
   //// Check for memory error
   U_MEMORY_TEST

   /// = Allocator e Deallocator.
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UString name, value;

   HttpField()
      {
      U_TRACE_CTOR(5, HttpField, "")
      }

   HttpField(const char* name_, unsigned name_len, const char* value_, unsigned value_len)
         : name(name_, name_len), value(value_, value_len)
      {
      U_TRACE_CTOR(5, HttpField, "%.*S,%u,%.*S,%u", name_len, name_, name_len, value_len, value_, value_len)
      }

   /**
   * @param name_  String containing the name for field
   */
   HttpField(const UString& name_) : name(name_)
      {
      U_TRACE_CTOR(5, HttpField, "%.*S", U_STRING_TO_TRACE(name_))
      }

   /**
   * @param name_  String containing the name for field
   * @param value_ String containing the value for field
   */
   HttpField(const UString& name_, const UString& value_) : name(name_), value(value_)
      {
      U_TRACE_CTOR(5, HttpField, "%.*S,%.*S", U_STRING_TO_TRACE(name_), U_STRING_TO_TRACE(value_))
      }

   /** Destructor of the class.
   */
   virtual ~HttpField()
      {
      U_TRACE_DTOR(0, HttpField)
      }

   /// ASSEGNAZIONI

   HttpField(const HttpField& f) : name(f.name), value(f.value)
      {
      U_MEMORY_TEST_COPY(f)
      }

   HttpField& operator=(const HttpField& f)
      {
      U_MEMORY_TEST_COPY(f)

      name  = f.name;
      value = f.value;

      return *this;
      }

   /**
   * @param field_ String where to save header as string
   */
   virtual void stringify(UString& field);

   static void trim(const char*& ptr, unsigned& len)
      {
      U_TRACE(5, "HttpField::trim(%.*S,%u)", len, ptr, len)

      while (isspace(*ptr))
         {
         ++ptr;
         --len;
         }
      }

   /// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif
};

#endif
