// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    value.h - Represents a JSON (JavaScript Object Notation) value
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_VALUE_H
#define ULIB_VALUE_H 1

#include <ulib/container/hash_map.h>

#define U_JFIND(json,str,result) UValue::jfind(json,str,U_CONSTANT_SIZE(str),result)

/**
 * \brief Represents a <a HREF="http://www.json.org">JSON</a> value.
 *
 * This class is a discriminated union wrapper that can represents a:
 *
 * - 'null'
 * - boolean
 * -   signed integer
 * - unsigned integer
 * - double
 * - UTF-8 string
 * - list of UValue
 * - collection of name/value pairs (javascript object)
 *
 * The type of the held value is represented by a #ValueType and can be obtained using method type().
 * Values of an #OBJECT_VALUE or #ARRAY_VALUE can be accessed using operator[]() methods.
 * It is possible to iterate over the list of a #OBJECT_VALUE values using the getMemberNames() method
 */

class UValue;
class UTokenizer;

class U_EXPORT UJsonTypeHandler_Base {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UJsonTypeHandler_Base(const void* ptr) : pval((void*)ptr)
      {
      U_TRACE_REGISTER_OBJECT(0, UJsonTypeHandler_Base, "%p", ptr)

      U_INTERNAL_ASSERT_POINTER(pval)
      }

   ~UJsonTypeHandler_Base()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UJsonTypeHandler_Base)

      U_INTERNAL_ASSERT_POINTER(pval)
      }

#ifdef DEBUG
   const char* dump(bool _reset) const;
#endif

protected:
   void* pval;

private:
   U_DISALLOW_ASSIGN(UJsonTypeHandler_Base)
};

#define U_JSON_TYPE_HANDLER(class_name, name_object_member, type_object_member) \
   U_CONSTANT_TO_PARAM(#name_object_member), UJsonTypeHandler<type_object_member>(((class_name*)pval)->name_object_member)

/// Provide template specializations to support your own complex types
///
/// Take as example the following (simplified) class:
///
///   class Person {
///   public:
///      UString  lastName;
///      UString firstName;
///      int     age;
///   };
///
/// The UJsonTypeHandler must provide a custom toJSON and fromJSON method:
///
///   template <> class UJsonTypeHandler<Person> : public UJsonTypeHandler_Base {
///   public:
///      explicit UJsonTypeHandler(Person& val) : UJsonTypeHandler_Base(&val)
///
///      void toJSON(UValue& json)
///         {
///         json.toJSON(U_JSON_TYPE_HANDLER(Person,  lastName, UString));
///         json.toJSON(U_JSON_TYPE_HANDLER(Person, firstName, UString));
///         json.toJSON(U_JSON_TYPE_HANDLER(Person, age,       int));
///         }
///
///      void fromJSON(UValue& json)
///         {
///         json.fromJSON(U_JSON_TYPE_HANDLER(Person,  lastName, UString));
///         json.fromJSON(U_JSON_TYPE_HANDLER(Person, firstName, UString));
///         json.fromJSON(U_JSON_TYPE_HANDLER(Person, age,       int));
///         }
///   };

template <class T> class U_EXPORT UJsonTypeHandler : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(T* val) : UJsonTypeHandler_Base( val) {}
   explicit UJsonTypeHandler(T& val) : UJsonTypeHandler_Base(&val) {}

   // SERVICES

   void   toJSON(UValue& json);
   void fromJSON(UValue& json);

private:
   U_DISALLOW_ASSIGN(UJsonTypeHandler)
};

class U_EXPORT UValue {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // constructors

   UValue(ValueType _type = NULL_VALUE)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%d", _type)

      U_INTERNAL_ASSERT_RANGE(0, _type, OBJECT_VALUE)

      (void) memset(this, 0, sizeof(UValue)-sizeof(int));

#  ifdef DEBUG
      memory._this = (void*)U_CHECK_MEMORY_SENTINEL;
#  endif

      type_ = _type;
      }

   UValue(bool value_)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%b", value_)

      type_       = BOOLEAN_VALUE;
      value.bool_ = value_;

      reset();
      }

   UValue(int value_)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%d", value_)

      type_      = INT_VALUE;
      value.int_ = value_;

      reset();
      }

   UValue(unsigned int value_)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%u", value_)

      type_       = UINT_VALUE;
      value.uint_ = value_;

      reset();
      }

   UValue(double value_)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%g", value_)

      type_       = REAL_VALUE;
      value.real_ = value_;

      reset();
      }

   UValue(UStringRep* value_)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%V", value_)

      type_ = STRING_VALUE;

      U_NEW(UString, value.ptr_, UString(value_));

      reset();
      }

   UValue(const UString& value_)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%V", value_.rep)

      type_ = STRING_VALUE;

      U_NEW(UString, value.ptr_, UString(value_));

      reset();
      }

    UValue(const UString& _key, const UString& value_);

   ~UValue()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UValue)

      clear(); 
      }

   // ASSIGNMENT

   UValue(const UValue& v)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%p", &v)

      U_MEMORY_TEST_COPY(v)

      set(v);
      }

   UValue& operator=(const UValue& v)
      {
      U_TRACE(0, "UValue::operator=(%p)", &v)

      U_MEMORY_TEST_COPY(v)

      clear();

      set(v);

      return *this;
      }

#ifdef U_COMPILER_RVALUE_REFS
   UValue& operator=(UValue && v);
# if (!defined(__GNUC__) || GCC_VERSION_NUM > 50300) // GCC has problems dealing with move constructor, so turn the feature on for 5.3.1 and above, only
   UValue(UValue && v);
# endif
#endif

   void set(const UValue& v);

   // OPERATOR

   bool operator==(const UValue& v) const __pure;
   bool operator!=(const UValue& v) const { return ! operator==(v); }

   // SERVICES

   void clear();

   bool isNull() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isNull()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      if (type_ == NULL_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isBool() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isBool()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      if (type_ == BOOLEAN_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isInt() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isInt()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      if (type_ == INT_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isUInt() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isUInt()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      if (type_ == UINT_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isIntegral() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isIntegral()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      if (type_ ==     INT_VALUE ||
          type_ ==    UINT_VALUE ||
          type_ == BOOLEAN_VALUE)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool isDouble() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isDouble()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      if (type_ == REAL_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isNumeric() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isNumeric()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      if (type_ ==     INT_VALUE ||
          type_ ==    UINT_VALUE ||
          type_ == BOOLEAN_VALUE ||
          type_ ==    REAL_VALUE)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool isString() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isString()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      if (type_ == STRING_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isArray() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isArray()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      if (type_ == ARRAY_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isObject() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isObject()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      if (type_ == OBJECT_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isArrayOrObject() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isArrayOrObject()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      if (type_ ==  ARRAY_VALUE ||
          type_ == OBJECT_VALUE)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   ValueType type() const { return (ValueType)type_; }

   // CONVERSION

   bool         asBool() const __pure;
   int          asInt() const __pure;
   unsigned int asUInt() const __pure;
   double       asDouble() const __pure;
   UString      asString() const __pure;

   bool isConvertibleTo(ValueType other) const __pure;

   // manage values in array or object

   UValue* at(uint32_t pos) const __pure;
   UValue* at(const char* _key, uint32_t key_len) const __pure;

   UValue& operator[](uint32_t pos)           const { return *at(pos); }
   UValue& operator[](const UString& _key)    const { return *at(U_STRING_TO_PARAM(_key)); }
   UValue& operator[](const UStringRep* _key) const { return *at(U_STRING_TO_PARAM(*_key)); }

   bool isMemberExist(const char* _key, uint32_t key_len) const { return (at(_key, key_len)            != 0); }
   bool isMemberExist(const UString& _key) const                { return (at(U_STRING_TO_PARAM(_key))  != 0); }
   bool isMemberExist(const UStringRep* _key) const             { return (at(U_STRING_TO_PARAM(*_key)) != 0); }

   UString& getString() const { return *(UString*)value.ptr_; }

   /**
    * \brief Return a list of the member names.
    *
    * If null, return an empty list.
    * \pre type() is OBJECT_VALUE or NULL_VALUE
    * \post if type() was NULL_VALUE, it remains NULL_VALUE 
    */

   uint32_t getMemberNames(UVector<UString>& members) const;

   /**
    * \brief Read a UValue from a <a HREF="http://www.json.org">JSON</a> document.
    *
    * \param document UTF-8 encoded string containing the document to read.
    *
    * \return \c true if the document was successfully parsed, \c false if an error occurred
    */

   bool parse(const UString& document);

   /**
    * \brief Outputs a UValue in <a HREF="http://www.json.org">JSON</a> format without formatting (not human friendly).
    *
    * The JSON document is written in a single line. It is not intended for 'human' consumption,
    * but may be usefull to support feature such as RPC where bandwith is limited
    */

   UString output()
      {
      U_TRACE_NO_PARAM(0, "UValue::output()")

      UString result(U_CAPACITY);

      UValue::stringify(result, *this);

      U_RETURN_STRING(result);
      }

   template <typename T> void toJSON(UJsonTypeHandler<T> t)
      {
      U_TRACE(0, "UValue::toJSON<T>(%p)", &t)

      t.toJSON(*this);
      }

   template <typename T> void toJSON(UJsonTypeHandler<T>& t)
      {
      U_TRACE(0, "UValue::toJSON<T>(%p)", &t)

      t.toJSON(*this);
      }

   template <typename T> void toJSON(const char* name, uint32_t len, UJsonTypeHandler<T> member)
      {
      U_TRACE(0, "UValue::toJSON<T>(%.*S,%u,%p)", len, name, len, &member)

      U_INTERNAL_DUMP("type_ = %d", type_)

      type_ = OBJECT_VALUE;

      UValue* child;

      U_NEW(UValue,  child,      UValue);
      U_NEW(UString, child->key, UString(name, len));

      member.toJSON(*child);

      appendNode(this, child);
      }

   template <typename T> void fromJSON(UJsonTypeHandler<T> t)
      {
      U_TRACE(0, "UValue::fromJSON<T>(%p)", &t)

      t.fromJSON(*this);
      }

   template <typename T> void fromJSON(UJsonTypeHandler<T>& t)
      {
      U_TRACE(0, "UValue::fromJSON<T>(%p)", &t)

      t.fromJSON(*this);
      }

   template <typename T> void fromJSON(const char* name, uint32_t len, UJsonTypeHandler<T> member)
      {
      U_TRACE(0, "UValue::fromJSON<T>(%.*S,%u,%p)", len, name, len, &member)

      U_INTERNAL_DUMP("type_ = %d", type_)

      U_INTERNAL_ASSERT_EQUALS(type_, OBJECT_VALUE)

      UString str(name, len);

      UValue& json = operator[](str);

      member.fromJSON(json);
      }

   static void stringify(UString& result, UValue& value);

   // =======================================================================================================================
   // An in-place JSON element reader (@see http://www.codeproject.com/Articles/885389/jRead-an-in-place-JSON-element-reader)
   // =======================================================================================================================

   static int      jread_error;
   static uint32_t jread_elements, jread_pos;

   static int  jread(const UString& json, const UString& query,                  UString& result, uint32_t* queryParams = 0);
   static bool jfind(const UString& json, const char* query, uint32_t query_len, UString& result);

   static bool jfind(const UString& json, const char* query, uint32_t query_len, bool& result)
      {
      U_TRACE(0, "UValue::jfind(%V,%.*S,%u,%p)", json.rep, query_len, query, query_len, &result)

      UString x;

      if (jfind(json, query, query_len, x))
         {
         result = x.strtob();

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool jfind(const UString& json, const char* query, uint32_t query_len, int& result)
      {
      U_TRACE(0, "UValue::jfind(%V,%.*S,%u,%p)", json.rep, query_len, query, query_len, &result)

      UString x;

      if (jfind(json, query, query_len, x))
         {
         result = x.strtol();

         U_RETURN(true);
         }

      U_RETURN(false);
      }

#ifdef HAVE_STRTOULL
   static bool jfind(const UString& json, const char* query, uint32_t query_len, int64_t& result)
      {
      U_TRACE(0, "UValue::jfind(%V,%.*S,%u,%p)", json.rep, query_len, query, query_len, &result)

      UString x;

      if (jfind(json, query, query_len, x))
         {
         result = x.strtoll();

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool jfind(const UString& json, const UString& query, int64_t& result) { return jfind(json, U_STRING_TO_PARAM(query), result); }
#endif

   static bool jfind(const UString& json, const char* query, uint32_t query_len, double& result)
      {
      U_TRACE(0, "UValue::jfind(%V,%.*S,%u,%p)", json.rep, query_len, query, query_len, &result)

      UString x;

      if (jfind(json, query, query_len, x))
         {
         result = x.strtod();

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool jfind(const UString& json, const UString& query, UString& result) { return jfind(json, U_STRING_TO_PARAM(query), result); }
   static bool jfind(const UString& json, const UString& query,    bool& result) { return jfind(json, U_STRING_TO_PARAM(query), result); }
   static bool jfind(const UString& json, const UString& query,     int& result) { return jfind(json, U_STRING_TO_PARAM(query), result); }
   static bool jfind(const UString& json, const UString& query,  double& result) { return jfind(json, U_STRING_TO_PARAM(query), result); }

   // reads one value from an array

   static void jreadArrayStepInit() { jread_pos = 0; }

   static int  jreadArrayStep(const UString& jarray, UString& result); // assumes jarray points at the start of an array or array element


#ifdef DEBUG
   bool invariant() const;
   const char* dump(bool _reset) const;

   static const char* getJReadErrorDescription();
   static const char* getDataTypeDescription(int type);
#else
   static const char* getJReadErrorDescription()       { return ""; }
   static const char* getDataTypeDescription(int type) { return ""; }
#endif

protected:
   // only if parent is an object or array (0 otherwise)
   UValue* parent;
   UValue* prev;
   UValue* next;
   // only if parent is an object (0 otherwise)
   UString* key; // must be valid UTF-8
   // ARRAY_VALUE - OBJECT_VALUE
   struct { UValue* head; UValue* tail; } children;

   union anyvalue value;
   int type_;

   void reset();

   static void appendNode(UValue* parent, UValue* child);

private:
   static int jread_skip(UTokenizer& tok) U_NO_EXPORT;
   static int jreadFindToken(UTokenizer& tok) U_NO_EXPORT;

   static UString jread_string(UTokenizer& tok) U_NO_EXPORT;
   static UString jread_object(UTokenizer& tok) U_NO_EXPORT;
   static UString jread_object(UTokenizer& tok, uint32_t keyIndex) U_NO_EXPORT;

   static bool readValue(UTokenizer& tok, UValue* value) U_NO_EXPORT;
   static uint32_t emitString(const unsigned char* ptr, uint32_t sz, char* presult) U_NO_EXPORT;

   template <class T> friend class UVector;
   template <class T> friend class UHashMap;
   template <class T> friend class UJsonTypeHandler;
};

// manage object <=> JSON representation

template <class T> inline bool JSON_parse(const UString& str, T& obj)
{
   U_TRACE(0, "JSON_parse(%p,%p)", &str, &obj)

   UValue json;

   if (json.parse(str))
      {
      json.fromJSON(UJsonTypeHandler<T>(obj));

      U_RETURN(true);
      }

   U_RETURN(false);
}

template <class T> inline UString JSON_stringify(UValue& json, T& obj)
{
   U_TRACE(0, "JSON_stringify(%p,%p)", &json, &obj)

   json.toJSON(UJsonTypeHandler<T>(obj));

   return json.output();
}

// TEMPLATE SPECIALIZATIONS

template <> class U_EXPORT UJsonTypeHandler<null> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(null& val) : UJsonTypeHandler_Base(0) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<null>::toJSON(%p)", &json)

      json.type_       = NULL_VALUE;
      json.value.real_ = 0.0;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<null>::fromJSON(%p)", &json)
      }
};

template <> class U_EXPORT UJsonTypeHandler<bool> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(bool& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<bool>::toJSON(%p)", &json)

      json.type_       = BOOLEAN_VALUE;
      json.value.bool_ = *(bool*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<bool>::fromJSON(%p)", &json)

      *(bool*)pval = json.value.bool_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<char> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(char& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<char>::toJSON(%p)", &json)

      json.type_       = CHAR_VALUE;
      json.value.char_ = *(char*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<char>::fromJSON(%p)", &json)

      *(char*)pval = json.value.char_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<unsigned char> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(unsigned char& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned char>::toJSON(%p)", &json)

      json.type_        = UCHAR_VALUE;
      json.value.uchar_ = *(unsigned char*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned char>::fromJSON(%p)", &json)

      *(unsigned char*)pval = json.value.uchar_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<short> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(short& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<short>::toJSON(%p)", &json)

      json.type_        = SHORT_VALUE;
      json.value.short_ = *(short*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<short>::fromJSON(%p)", &json)

      *(short*)pval = json.value.short_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<unsigned short> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(unsigned short& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned short>::toJSON(%p)", &json)

      json.type_         = USHORT_VALUE;
      json.value.ushort_ = *(unsigned short*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned short>::fromJSON(%p)", &json)

      *(unsigned short*)pval = json.value.ushort_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<int> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(int& val) : UJsonTypeHandler_Base(&val)
      {
      U_TRACE(0, "UJsonTypeHandler::UJsonTypeHandler<int>(%d)", val)

      U_INTERNAL_DUMP("this = %p", this)
      }

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<int>::toJSON(%p)", &json)

      json.type_      = INT_VALUE;
      json.value.int_ = *(int*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<int>::fromJSON(%p)", &json)

      U_INTERNAL_DUMP("pval(%p) = %d", pval, *(int*)pval)

      *(int*)pval = json.value.int_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<unsigned int> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(unsigned int& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned int>::toJSON(%p)", &json)

      json.type_       = UINT_VALUE;
      json.value.uint_ = *(unsigned int*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned int>::fromJSON(%p)", &json)

      *(unsigned int*)pval = json.value.uint_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<long> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(long& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long>::toJSON(%p)", &json)

      json.type_       = LONG_VALUE;
      json.value.long_ = *(long*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long>::fromJSON(%p)", &json)

      *(long*)pval = json.value.long_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<unsigned long> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(unsigned long& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned long>::toJSON(%p)", &json)

      json.type_        = ULONG_VALUE;
      json.value.ulong_ = *(unsigned long*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned long>::fromJSON(%p)", &json)

      *(unsigned long*)pval = json.value.ulong_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<long long> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(long long& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long long>::toJSON(%p)", &json)

      json.type_        = LLONG_VALUE;
      json.value.llong_ = *(long long*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long long>::fromJSON(%p)", &json)

      *(long long*)pval = json.value.llong_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<unsigned long long> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(unsigned long long& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned long long>::toJSON(%p)", &json)

      json.type_         = ULLONG_VALUE;
      json.value.ullong_ = *(unsigned long long*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned long long>::fromJSON(%p)", &json)

      *(unsigned long long*)pval = json.value.ullong_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<float> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(float& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<float>::toJSON(%p)", &json)

      json.type_        = FLOAT_VALUE;
      json.value.float_ = *(float*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<float>::fromJSON(%p)", &json)

      *(float*)pval = json.value.float_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<double> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(double& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<double>::toJSON(%p)", &json)

      json.type_       = REAL_VALUE;
      json.value.real_ = *(double*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<double>::fromJSON(%p)", &json)

      *(double*)pval = json.value.real_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<long double> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(long double& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long double>::toJSON(%p)", &json)

      json.type_        = LREAL_VALUE;
      json.value.lreal_ = *(long double*)pval;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long double>::fromJSON(%p)", &json)

      *(long double*)pval = json.value.lreal_;
      }
};

template <> class U_EXPORT UJsonTypeHandler<UStringRep> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(UStringRep& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UStringRep>::toJSON(%p)", &json)

      U_INTERNAL_DUMP("json.type_ = %d", json.type_)

      json.type_ = STRING_VALUE;

      U_INTERNAL_DUMP("pval(%p) = %V", pval, pval)

      U_NEW(UString, json.value.ptr_, UString((UStringRep*)pval));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UStringRep>::fromJSON(%p)", &json)

      U_INTERNAL_DUMP("json.type_ = %d", json.type_)

      U_ASSERT(json.isString())

      UStringRep* rep = json.getString().rep;

      U_INTERNAL_DUMP("pval(%p) = %p rep(%p) = %V", pval, pval, rep, rep)

      U_ERROR("UJsonTypeHandler<UStringRep>::fromJSON(): sorry, we cannot use UStringRep type from JSON type handler...");
      }
};

template <> class U_EXPORT UJsonTypeHandler<UString> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(UString& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UString>::toJSON(%p)", &json)

      U_INTERNAL_DUMP("json.type_ = %d", json.type_)

      json.type_ = STRING_VALUE;

      U_INTERNAL_DUMP("pval(%p) = %V", pval, ((UString*)pval)->rep)

      U_NEW(UString, json.value.ptr_, UString(*((UString*)pval)));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UString>::fromJSON(%p)", &json)

      U_INTERNAL_DUMP("json.type_ = %d", json.type_)

      U_ASSERT(json.isString())

      UStringRep* rep = json.getString().rep;

      U_INTERNAL_DUMP("pval(%p) = %p rep(%p) = %V", pval, ((UString*)pval)->rep, rep, rep)

      ((UString*)pval)->_assign(rep);

      U_INTERNAL_ASSERT(((UString*)pval)->invariant())
      }
};

// TEMPLATE SPECIALIZATIONS FOR CONTAINERS

template <class T> class U_EXPORT UJsonTypeHandler<UVector<T*> > : public UJsonTypeHandler_Base {
public:
   typedef UVector<T*> uvector;

   explicit UJsonTypeHandler(uvector& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<uvector>::toJSON(%p)", &json)

      U_INTERNAL_DUMP("json.type_ = %d", json.type_)

      json.type_ = ARRAY_VALUE;

      uvector* pvec = (uvector*)pval;

      UValue* child;
      const void** ptr = pvec->vec;
      const void** end = pvec->vec + pvec->_length;

      for (; ptr < end; ++ptr)
         {
         U_NEW(UValue, child, UValue);

         child->toJSON(UJsonTypeHandler<T>(*(T*)(*ptr)));

         UValue::appendNode(&json, child);
         }
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<uvector>::fromJSON(%p)", &json)

      U_INTERNAL_DUMP("json.type_ = %d", json.type_)

      for (UValue* child = json.children.head; child; child = child->next)
         {
         T* pelem;

         U_NEW(T, pelem, T);

         child->fromJSON(UJsonTypeHandler<T>(*pelem));

         ((uvector*)pval)->push_back(pelem);
         }
      }
};

template <> class U_EXPORT UJsonTypeHandler<UVector<UString> > : public UJsonTypeHandler<UVector<UStringRep*> > {
public:
   typedef UVector<UStringRep*> uvectorbase;

   explicit UJsonTypeHandler(UVector<UString>& val) : UJsonTypeHandler<uvectorbase>(*((uvector*)&val)) {}

   void toJSON(UValue& json) { ((UJsonTypeHandler<uvectorbase>*)this)->toJSON(json); }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UVector<UString>>::fromJSON(%p)", &json)

      U_INTERNAL_DUMP("json.type_ = %d", json.type_)

      for (UValue* child = json.children.head; child; child = child->next)
         {
         UString elem;

         child->fromJSON(UJsonTypeHandler<UString>(elem));

         ((UVector<UString>*)pval)->push_back(elem);
         }
      }
};

template <class T> class U_EXPORT UJsonTypeHandler<UHashMap<T*> > : public UJsonTypeHandler_Base {
public:
   typedef UHashMap<T*> uhashmap;

   explicit UJsonTypeHandler(uhashmap& map) : UJsonTypeHandler_Base(&map) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<uhashmap>::toJSON(%p)", &json)

      U_INTERNAL_DUMP("json.type_ = %d", json.type_)

      json.type_ = OBJECT_VALUE;

      uhashmap* pmap = (uhashmap*)pval;

      UValue* child;
      UHashMapNode* node;
      UHashMapNode* next;
      UHashMapNode** ptr = pmap->table;
      UHashMapNode** end = pmap->table + pmap->_capacity;

      for (; ptr < end; ++ptr)
         {
         if (*ptr)
            {
            node = *ptr;

            do {
               next = node->next;

               U_NEW(UValue,  child,      UValue);
               U_NEW(UString, child->key, UString((UStringRep*)node->key));

               child->toJSON(UJsonTypeHandler<T>(*(T*)node->elem)); // *child << *((T*)node->elem);

               UValue::appendNode(&json, child);
               }
            while ((node = next));
            }
         }
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<uhashmap>::fromJSON(%p)", &json)

      U_INTERNAL_DUMP("json.type_ = %d", json.type_)

      uhashmap* pmap = (uhashmap*)pval;

      for (UValue* child = json.children.head; child; child = child->next)
         {
         T* elem;

         U_NEW(T, elem, T);

         child->fromJSON(UJsonTypeHandler<T>(*elem)); // *elem << *child;

         U_INTERNAL_ASSERT_POINTER(child->key)

         pmap->lookup(*(child->key));

         pmap->insertAfterFind(*(child->key), elem);
         }
      }
};

template <> class U_EXPORT UJsonTypeHandler<UHashMap<UString> > : public UJsonTypeHandler<UHashMap<UStringRep*> > {
public:
   typedef UHashMap<UStringRep*> uhashmapbase;

   explicit UJsonTypeHandler(UHashMap<UString>& val) : UJsonTypeHandler<uhashmapbase>(*((uhashmap*)&val)) {}

   void toJSON(UValue& json) { ((UJsonTypeHandler<uhashmapbase>*)this)->toJSON(json); }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<<UVector<UString>>::fromJSON(%p)", &json)

      U_INTERNAL_DUMP("json.type_ = %d", json.type_)

      uhashmap* pmap = (uhashmap*)pval;

      for (UValue* child = json.children.head; child; child = child->next)
         {
         UString elem;

         child->fromJSON(UJsonTypeHandler<UString>(elem));

         U_INTERNAL_ASSERT_POINTER(child->key)

         pmap->lookup(*(child->key));

         pmap->insertAfterFind(*(child->key), elem.rep);
         }
      }
};

#if defined(U_STDCPP_ENABLE)
#  include <vector>
template <class T> class U_EXPORT UJsonTypeHandler<std::vector<T> > : public UJsonTypeHandler_Base {
public:
   typedef std::vector<T> stdvector;

   explicit UJsonTypeHandler(stdvector& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<stdvector>::toJSON(%p)", &json)

      U_INTERNAL_DUMP("json.type_ = %d", json.type_)

      json.type_ = ARRAY_VALUE;

      stdvector* pvec = (stdvector*)pval;

      UValue* child;

      for (uint32_t i = 0, n = pvec->size(); i < n; ++i)
         {
         U_NEW(UValue, child, UValue);

         child->toJSON(UJsonTypeHandler<T>(pvec->at(i)));

         UValue::appendNode(&json, child);
         }
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<stdvector>::fromJSON(%p)", &json)

      U_INTERNAL_DUMP("json.type_ = %d", json.type_)

      for (UValue* child = json.children.head; child; child = child->next)
         {
         T elem;

         child->fromJSON(UJsonTypeHandler<T>(elem));

         ((stdvector*)pval)->push_back(elem);
         }
      }
};
#endif
#endif
