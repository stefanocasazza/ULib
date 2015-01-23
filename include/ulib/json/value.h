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

#include <ulib/container/vector.h>
#include <ulib/container/hash_map.h>

/**
 * \brief Represents a <a HREF="http://www.json.org">JSON</a> value.
 *
 * This class is a discriminated union wrapper that can represents a:
 *
 * - 'null'
 * - boolean
 * - signed integer
 * - unsigned integer
 * - double
 * - UTF-8 string
 * - an ordered list of UValue
 * - collection of name/value pairs (javascript object)
 *
 * The type of the held value is represented by a #ValueType and can be obtained using type().
 * Values of an #OBJECT_VALUE or #ARRAY_VALUE can be accessed using operator[]() methods.
 * The sequence of an #ARRAY_VALUE will be automatically resize and initialized with #NULL_VALUE.
 * It is possible to iterate over the list of a #OBJECT_VALUE values using the getMemberNames() method.
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

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   void* pval;

private:
#ifdef U_COMPILER_DELETE_MEMBERS
// UJsonTypeHandler_Base(const UJsonTypeHandler_Base&) = delete;
   UJsonTypeHandler_Base& operator=(const UJsonTypeHandler_Base&) = delete;
#else
// UJsonTypeHandler_Base(const UJsonTypeHandler_Base&) {}
   UJsonTypeHandler_Base& operator=(const UJsonTypeHandler_Base&) { return *this; }
#endif
};

#define U_JSON_TYPE_HANDLER(class_name, name_object_member, type_object_member) \
   U_CONSTANT_TO_PARAM(#name_object_member), UJsonTypeHandler<type_object_member>(((class_name*)pval)->name_object_member)

/// Provide template specializations to support your own complex types
///
/// Take as example the following (simplified) class:
///
///   class Person {
///   public:
///      UString _lastName;
///      UString _firstName;
///      int     _age;
///   };
///
/// The UJsonTypeHandler must provide a custom toJSON and fromJSON method:
///
///   template <> class UJsonTypeHandler<Person> : public UJsonTypeHandler_Base {
///   public:
///      explicit UJsonTypeHandler(Person& val) : UJsonTypeHandler_Base(&val)
///
///      void toJSON(UValue& json)
///      {
///         json.toJSON(U_JSON_TYPE_HANDLER(Person, _lastName,  UString));
///         json.toJSON(U_JSON_TYPE_HANDLER(Person, _firstName, UString));
///         json.toJSON(U_JSON_TYPE_HANDLER(Person, _age,       int));
///      }
///
///      void fromJSON(UValue& json)
///      {
///         json.fromJSON(U_JSON_TYPE_HANDLER(Person, _lastName,  UString));
///         json.fromJSON(U_JSON_TYPE_HANDLER(Person, _firstName, UString));
///         json.fromJSON(U_JSON_TYPE_HANDLER(Person, _age,       int));
///      }
///   };

template <class T> class U_EXPORT UJsonTypeHandler : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(T* val) : UJsonTypeHandler_Base( val) {}
   explicit UJsonTypeHandler(T& val) : UJsonTypeHandler_Base(&val) {}

   // SERVICES

   void   toJSON(UValue& json);
   void fromJSON(UValue& json);

private:
#ifdef U_COMPILER_DELETE_MEMBERS
// UJsonTypeHandler(const UJsonTypeHandler&) = delete;
   UJsonTypeHandler& operator=(const UJsonTypeHandler&) = delete;
#else
// UJsonTypeHandler(const UJsonTypeHandler&) : UJsonTypeHandler_Base(0) {}
   UJsonTypeHandler& operator=(const UJsonTypeHandler&)                 { return *this; }
#endif
};

class U_EXPORT UValue {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // Costruttori

   UValue()
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "", 0)

      type_       = NULL_VALUE;
      value.real_ = 0.0;
      }

   UValue(bool value_)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%b", value_)

      type_       = BOOLEAN_VALUE;
      value.bool_ = value_;
      }

   UValue(int value_)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%d", value_)

      type_      = INT_VALUE;
      value.int_ = value_;
      }

   UValue(unsigned int value_)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%u", value_)

      type_       = UINT_VALUE;
      value.uint_ = value_;
      }

   UValue(double value_)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%g", value_)

      type_       = REAL_VALUE;
      value.real_ = value_;
      }

   UValue(UStringRep* value_)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%.*S", U_STRING_TO_TRACE(*value_))

      type_      = STRING_VALUE;
      value.ptr_ = U_NEW(UString(value_));
      }

   UValue(const UString& value_)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%.*S", U_STRING_TO_TRACE(value_))

      type_      = STRING_VALUE;
      value.ptr_ = U_NEW(UString(value_));
      }

    UValue(ValueType type);
    UValue(const UString& key, const UString& value_);
   ~UValue();

   // ASSEGNAZIONI

   void set(UValue& value_)
      {
      type_      = value_.type_;
                   value_.type_ = NULL_VALUE;
      value.ptr_ = value_.value.ptr_;
                   value_.value.ptr_ = 0;
      }

   UValue(const UValue& value_)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%p", &value_)

      set(*(UValue*)&value_);
      }

   UValue& operator=(UValue& value_)
      {
      U_TRACE(0, "UValue::operator=(%p)", &value_)

      U_MEMORY_TEST_COPY(value_)

      set(value_);

      return *this;
      }

   // SERVICES

   bool isNull()
      {
      U_TRACE(0, "UValue::isNull()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      bool result = (type_ == NULL_VALUE);

      U_RETURN(result);
      }

   bool isBool()
      {
      U_TRACE(0, "UValue::isBool()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      bool result = (type_ == BOOLEAN_VALUE);

      U_RETURN(result);
      }

   bool isInt()
      {
      U_TRACE(0, "UValue::isInt()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      bool result = (type_ == INT_VALUE);

      U_RETURN(result);
      }

   bool isUInt()
      {
      U_TRACE(0, "UValue::isUInt()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      bool result = (type_ == UINT_VALUE);

      U_RETURN(result);
      }

   bool isIntegral()
      {
      U_TRACE(0, "UValue::isIntegral()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      bool result = (type_ ==     INT_VALUE ||
                     type_ ==    UINT_VALUE ||
                     type_ == BOOLEAN_VALUE);

      U_RETURN(result);
      }

   bool isDouble()
      {
      U_TRACE(0, "UValue::isDouble()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      bool result = (type_ == REAL_VALUE);

      U_RETURN(result);
      }

   bool isNumeric()
      {
      U_TRACE(0, "UValue::isNumeric()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      bool result = (type_ ==     INT_VALUE ||
                     type_ ==    UINT_VALUE ||
                     type_ == BOOLEAN_VALUE ||
                     type_ ==    REAL_VALUE);

      U_RETURN(result);
      }

   bool isString() const
      {
      U_TRACE(0, "UValue::isString()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      bool result = (type_ == STRING_VALUE);

      U_RETURN(result);
      }

   bool isArray() const
      {
      U_TRACE(0, "UValue::isArray()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      bool result = (type_ == ARRAY_VALUE);

      U_RETURN(result);
      }

   bool isObject() const
      {
      U_TRACE(0, "UValue::isObject()")

      U_INTERNAL_DUMP("type_ = %d", type_)

      bool result = (type_ == OBJECT_VALUE);

      U_RETURN(result);
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

   uint32_t size() const __pure;

   UValue& operator[](uint32_t pos) __pure;
   UValue& operator[](const UString& key) __pure;

             UString* getString() const { return (          UString*)value.ptr_; }
    UVector<UValue*>* getArray()  const { return ( UVector<UValue*>*)value.ptr_; }
   UHashMap<UValue*>* getObject() const { return (UHashMap<UValue*>*)value.ptr_; }

   // \brief Return a list of the member names.
   //
   // If null, return an empty list.
   // \pre type() is OBJECT_VALUE or NULL_VALUE
   // \post if type() was NULL_VALUE, it remains NULL_VALUE 

   uint32_t getMemberNames(UVector<UString>& members) const;

   /** \brief Read a UValue from a <a HREF="http://www.json.org">JSON</a> document.
    *
    * \param document UTF-8 encoded string containing the document to read.
    *
    * \return \c true if the document was successfully parsed, \c false if an error occurred.
    */

   bool parse(const UString& document);

   /** \brief Outputs a UValue in <a HREF="http://www.json.org">JSON</a> format without formatting (not human friendly).
    *
    * The JSON document is written in a single line. It is not intended for 'human' consumption,
    * but may be usefull to support feature such as RPC where bandwith is limited.
    */

   UString output()
      {
      U_TRACE(0, "UValue::output()")

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

      if (isNull())
         {
         type_      = OBJECT_VALUE;
         value.ptr_ = U_NEW(UHashMap<UValue*>);
         }

      U_INTERNAL_ASSERT_EQUALS(type_, OBJECT_VALUE)

      UValue* pval = U_NEW(UValue);

      member.toJSON(*pval);

      UString str(name, len);

      getObject()->insert(str, pval);
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

      U_INTERNAL_ASSERT_EQUALS(type_, OBJECT_VALUE)

      UString str(name, len);

      UValue* result = getObject()->operator[](str);

      U_DUMP("hash map size = %u result = %p", getObject()->size(), result)

      U_INTERNAL_ASSERT_POINTER(result)

      member.fromJSON(*result);
      }

   static void stringify(UString& result, UValue& value);

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   union anyvalue value;
   int type_;

private:
   static bool readValue(UTokenizer& tok, UValue* value) U_NO_EXPORT;

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

   void   toJSON(UValue& json);
   void fromJSON(UValue& json);
};

template <> class U_EXPORT UJsonTypeHandler<UString> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(UString& val) : UJsonTypeHandler_Base(val.rep) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UString>::toJSON(%p)", &json)

      ((UJsonTypeHandler<UStringRep>*)this)->toJSON(json);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UString>::fromJSON(%p)", &json)

      ((UJsonTypeHandler<UStringRep>*)this)->fromJSON(json);
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

      uvector* pvec = (uvector*)pval;

      if (json.isNull() == false) json.getArray()->clear();
      else
         {
         json.type_      = ARRAY_VALUE;
         json.value.ptr_ = U_NEW(UVector<UValue*>(pvec->_length));
         }

      U_ASSERT(json.isArray())

      UValue* item;
      const void** ptr = pvec->vec;
      const void** end = pvec->vec + pvec->_length;
      UVector<UValue*>* array = json.getArray();

      for (; ptr < end; ++ptr)
         {
         item = U_NEW(UValue);

         item->toJSON(UJsonTypeHandler<T>(*(T*)(*ptr)));

         array->push_back(item);
         }
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<uvector>::fromJSON(%p)", &json)

      U_ASSERT(json.isArray())

      T* elem;
      UValue* item;
      uvector* pvec = (uvector*)pval;
      UVector<UValue*>* array = json.getArray();

      for (unsigned i = 0, n = array->size(); i < n; ++i)
         {
         elem = U_NEW(T);
         item = (*array)[i];

         item->fromJSON(UJsonTypeHandler<T>(*elem));

         pvec->push_back(elem);
         }
      }
};

template <> class U_EXPORT UJsonTypeHandler<UVector<UString> > : public UJsonTypeHandler<UVector<UStringRep*> > {
public:
   typedef UVector<UStringRep*> uvectorbase;

   explicit UJsonTypeHandler(UVector<UString>& val) : UJsonTypeHandler<uvectorbase>(*((uvector*)&val)) {}

   void   toJSON(UValue& json) { ((UJsonTypeHandler<uvectorbase>*)this)->toJSON(json); }
   void fromJSON(UValue& json) { ((UJsonTypeHandler<uvectorbase>*)this)->fromJSON(json); }
};

template <class T> class U_EXPORT UJsonTypeHandler<UHashMap<T*> > : public UJsonTypeHandler_Base {
public:
   typedef UHashMap<T*> uhashmap;

   explicit UJsonTypeHandler(uhashmap& map) : UJsonTypeHandler_Base(&map) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<uhashmap>::toJSON(%p)", &json)

      uhashmap* pmap = (uhashmap*)pval;

      if (json.isNull())
         {
         json.type_      = OBJECT_VALUE;
         json.value.ptr_ = U_NEW(UHashMap<UValue*>(pmap->_capacity));
         }

      U_ASSERT(json.isObject())

      UValue* item;
      UHashMapNode* node;
      UHashMapNode* next;
      UHashMapNode** ptr = pmap->table;
      UHashMapNode** end = pmap->table + pmap->_capacity;
      UHashMap<UValue*>* t1 = json.getObject();

      for (; ptr < end; ++ptr)
         {
         if (*ptr)
            {
            node = *ptr;

            do {
               next = node->next;

               item = U_NEW(UValue);

               item->toJSON(UJsonTypeHandler<T>(*(T*)node->elem)); // *item << *((T*)node->elem);

               t1->lookup(node->key);

               t1->insertAfterFind(node->key, item);
               }
            while ((node = next));
            }
         }
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<uhashmap>::fromJSON(%p)", &json)

      U_ASSERT(json.isObject())

      T* elem;
      UHashMapNode* node;
      UHashMapNode* next;
      uhashmap* pmap = (uhashmap*)pval;
      UHashMap<UValue*>* t1 = json.getObject();

      U_DUMP("json hash map size = %u", t1->size())

      UHashMapNode** ptr = t1->table;
      UHashMapNode** end = t1->table + t1->_capacity;

      for (; ptr < end; ++ptr)
         {
         if (*ptr)
            {
            node = *ptr;

            do {
               next = node->next;

               elem = U_NEW(T);

               ((UValue*)node->elem)->fromJSON(UJsonTypeHandler<T>(*elem)); // *elem << *((UValue*)node->elem);

               pmap->lookup(node->key);

               pmap->insertAfterFind(node->key, elem);
               }
            while ((node = next));
            }
         }
      }
};

template <> class U_EXPORT UJsonTypeHandler<UHashMap<UString> > : public UJsonTypeHandler<UHashMap<UStringRep*> > {
public:
   typedef UHashMap<UStringRep*> uhashmapbase;

   explicit UJsonTypeHandler(UHashMap<UString>& val) : UJsonTypeHandler<uhashmapbase>(*((uhashmap*)&val)) {}

   void   toJSON(UValue& json) { ((UJsonTypeHandler<uhashmapbase>*)this)->toJSON(json); }
   void fromJSON(UValue& json) { ((UJsonTypeHandler<uhashmapbase>*)this)->fromJSON(json); }
};

#endif
