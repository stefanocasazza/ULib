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

#include <math.h>

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
 * We stores values using NaN-boxing technique. By [IEEE-754](http://en.wikipedia.org/wiki/IEEE_floating_point)
 * standard we have 2^52-1 variants for encoding double's [NaN](http://en.wikipedia.org/wiki/NaN). So let's use
 * this to store value type and payload:
 *
 * sign
 * |  exponent
 * |  |
 * [0][11111111111][yyyyxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx]
 *                  |   |
 *                  tag |
 *                      payload
 *
 * 48 bits payload [enough](http://en.wikipedia.org/wiki/X86-64#Virtual_address_space_details) for store any pointer on x64.
 * double use zero tag, so infinity and nan are accessible.
 *
 * The type of the held value is represented by a #ValueType and can be obtained using method getTag().
 * Values of an #OBJECT_VALUE or #ARRAY_VALUE can be accessed using operator[]() methods.
 * It is possible to iterate over the list of a #OBJECT_VALUE values using the getMemberNames() method
 */

#define U_JSON_VALUE_TAG_MASK     0xF
#define U_JSON_VALUE_TAG_SHIFT    47
#define U_JSON_VALUE_NAN_MASK     0x7FF8000000000000ULL
#define U_JSON_VALUE_PAYLOAD_MASK 0x00007FFFFFFFFFFFULL

#define U_JFIND(json,str,result) UValue::jfind(json,str,U_CONSTANT_SIZE(str),result)

class UTokenizer;
class UValueIter;

class U_EXPORT UValue {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   union jval {
           int sint;
  unsigned int uint;
      uint64_t ival;
        double real;
   };

   typedef enum ValueType {
      REAL_VALUE = 0, // double value
       INT_VALUE = 1, //   signed integer value
      UINT_VALUE = 2, // unsigned integer value
      TRUE_VALUE = 3, // bool value
     FALSE_VALUE = 4, // bool value
    STRING_VALUE = 5, // string value
       UTF_VALUE = 6, // string value (need to be emitted)
     ARRAY_VALUE = 7, //  array value (ordered list)
    OBJECT_VALUE = 8, // object value (collection of name/value pairs)
      NULL_VALUE = 9  //    null value
   } ValueType;

   static int jsonParseFlags;

   // Constructors

   explicit UValue(double fval = 0.0)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%g", fval)

      // coverity[uninit_ctor]
#  ifdef U_COVERITY_FALSE_POSITIVE
      next = 0;
      pkey.ival = 0ULL;
#  endif

      value.real = fval;

      U_INTERNAL_DUMP("this = %p", this)
      }

   explicit UValue(ValueType tag, void* payload)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%d,%p", tag, payload)

      // coverity[uninit_ctor]
#  ifdef U_COVERITY_FALSE_POSITIVE
      next = 0;
      pkey.ival = 0ULL;
#  endif

      value.ival = getJsonValue(tag, payload);

      U_INTERNAL_DUMP("this = %p", this)
      }

   UValue(const UString& key, const UString& val)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%V,%V", key.rep, val.rep)

      UValue* node;

      U_NEW(UValue, node, UValue((UValue*)0));

      value.ival = getJsonValue(OBJECT_VALUE, node);

      key.hold();
      val.hold();

      node->pkey.ival  = getJsonValue(STRING_VALUE, key.rep);
      node->value.ival = getJsonValue(STRING_VALUE, val.rep);

      U_INTERNAL_DUMP("this = %p", this)
      }

   ~UValue();

   // ASSIGNMENT

   void set(const UValue& v)
      {
      U_TRACE(0, "UValue::set(%p)", &v)

            next = v.next;
       pkey.ival = v.pkey.ival;
      value.ival = v.value.ival;
      }

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

      set(v);

      return *this;
      }

#ifdef U_COMPILER_RVALUE_REFS
   UValue& operator=(UValue && v)
      {
      U_TRACE_NO_PARAM(0, "UValue::operator=(move)")

      UValue*  tmpn = next;
      uint64_t tmpk = pkey.ival,
               tmpv = value.ival;

      next  = v.next;
              v.next = tmpn;

      pkey.ival = v.pkey.ival;
                  v.pkey.ival = tmpk;

      value.ival = v.value.ival;
                   v.value.ival = tmpv;

      return *this;
      }

# if (!defined(__GNUC__) || GCC_VERSION_NUM > 50300) // GCC has problems dealing with move constructor, so turn the feature on for 5.3.1 and above, only
   UValue(UValue && v)
      {
      U_TRACE_NO_PARAM(0, "UValue::UValue(move)")

      set(v);

      v.value.ival = 0ULL;
      }
# endif
#endif

   // OPERATOR

   bool operator==(const UValue& v) const __pure
      {
      U_TRACE(0, "UValue::operator==(%p)", &v)

      if (value.ival == v.value.ival) U_RETURN(true);

      U_RETURN(false);
      }

   bool operator!=(const UValue& v) const { return ! operator==(v); }

   // SERVICES

   void clear();

   static int getTag(uint64_t val)
      {
      U_TRACE(0, "UValue::getTag(0x%x)", val)

      if ((int64_t)val <= (int64_t)U_JSON_VALUE_NAN_MASK) U_RETURN(REAL_VALUE);
      
      int type = (val >> U_JSON_VALUE_TAG_SHIFT) & U_JSON_VALUE_TAG_MASK;

      U_RETURN(type);
      }

   bool isNull() const
      {
      U_TRACE_NO_PARAM(0+256, "UValue::isNull()")

      if (getTag() == NULL_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isBool() const
      {
      U_TRACE_NO_PARAM(0+256, "UValue::isBool()")

      int type = getTag();

      if (type ==  TRUE_VALUE ||
          type == FALSE_VALUE)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool isInt() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isInt()")

      if (getTag() == INT_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isUInt() const
      {
      U_TRACE_NO_PARAM(0+256, "UValue::isUInt()")

      if (getTag() == UINT_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isDouble() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isDouble()")

      if (value.ival <= (int64_t)U_JSON_VALUE_NAN_MASK) U_RETURN(true);

      U_RETURN(false);
      }

   bool isNumeric() const
      {
      U_TRACE_NO_PARAM(0+256, "UValue::isNumeric()")

      if (getTag() <= UINT_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isString() const
      {
      U_TRACE_NO_PARAM(0+256, "UValue::isString()")

      if (getTag() == STRING_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isArray() const
      {
      U_TRACE_NO_PARAM(0+256, "UValue::isArray()")

      if (getTag() == ARRAY_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isObject() const
      {
      U_TRACE_NO_PARAM(0+256, "UValue::isObject()")

      if (getTag() == OBJECT_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isArrayOrObject() const { return isArrayOrObject(value.ival); }

   bool isStringUTF() const   { return isStringUTF(value.ival); }
   bool isStringOrUTF() const { return isStringOrUTF(value.ival); }

   union jval getKey() const   { return  pkey; }
   union jval getValue() const { return value; }

            int getInt() const  { return value.sint; }
   unsigned int getUInt() const { return value.uint; }

   bool   getBool() const   { return (getTag() == TRUE_VALUE); }
   double getDouble() const { return value.real; }

          int getTag() const   { return getTag(value.ival); }
   long long getNumber() const { return (isDouble() ? llrint(value.real) : (long long)(long)getPayload()); } 

   UString getString() { return getString(value.ival); }

   static uint32_t getStringSize(const union jval value) { return getString(value.ival).size(); }

   // manage values in array or object

   UValue* at(uint32_t pos) const __pure;
   UValue* at(const char* key, uint32_t key_len) const __pure;

   UValue& operator[](uint32_t pos)        const   { return *at(pos); }
   UValue& operator[](const UString& key)  const   { return *at(U_STRING_TO_PARAM(key)); }
   UValue& operator[](const UStringRep* key) const { return *at(U_STRING_TO_PARAM(*key)); }

   bool isMemberExist(const UString& key) const    { return (at(U_STRING_TO_PARAM(key))  != 0); }
   bool isMemberExist(const UStringRep* key) const { return (at(U_STRING_TO_PARAM(*key)) != 0); }

   bool isMemberExist(const char* key, uint32_t key_len) const { return (at(key, key_len) != 0); }

   /**
    * \brief Return a list of the member names.
    *
    * \pre getTag() is OBJECT_VALUE
    * \post if getTag() was NULL_VALUE, it remains NULL_VALUE 
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

   UString output() const
      {
      U_TRACE_NO_PARAM(0, "UValue::output()")

      UString result(size+100U);

      pstringify = result.data(); // buffer to stringify json

      stringify();

      result.size_adjust(pstringify);

      U_RETURN_STRING(result);
      }

   UString prettify() const
      {
      U_TRACE_NO_PARAM(0, "UValue::prettify()")

      UString result(size*2+800U);

      pstringify = result.data(); // buffer to stringify json

      prettify(0);

      *pstringify++ = '\n';

      result.size_adjust(pstringify);

      U_RETURN_STRING(result);
      }

   static void stringify(UString& result, const UValue& json)
      {
      U_TRACE(0, "UValue::stringify(%V,%p)", result.rep, &json)

      (void) result.reserve(json.size+100U);

      pstringify = result.pend(); // buffer to stringify json

      json.stringify();

      result.size_adjust(pstringify);

      U_INTERNAL_DUMP("result(%u) = %V", result.size(), result.rep)
      }

   template <typename T> void toJSON(UJsonTypeHandler<T> t)
      {
      U_TRACE(0, "UValue::toJSON<T>(%p)", &t)

      t.toJSON(*this);
      }

   template <typename T> static void toJSON1(UJsonTypeHandler<T> member)
      {
      U_TRACE(0, "UValue::toJSON1<T>(%p)", &member)

      UValue* node;

      U_NEW(UValue, node, UValue((UValue*)0));

      member.toJSON(*node);

      if (pnode) pnode->next = node;
      else             phead = node;

      pnode = node;
      }

   template <typename T> static void toJSON(const UString& name, UJsonTypeHandler<T> member)
      {
      U_TRACE(0, "UValue::toJSON<T>(%V,%p)", name.rep, &member)

      toJSON1<T>(member);

      name.hold();

      pnode->pkey.ival = getJsonValue(STRING_VALUE, name.rep);
      }

   template <typename T> void fromJSON(UJsonTypeHandler<T> t)
      {
      U_TRACE(0, "UValue::fromJSON<T>(%p)", &t)

      t.fromJSON(*this);
      }

   template <typename T> void fromJSON(const UString& name, UJsonTypeHandler<T> member)
      {
      U_TRACE(0, "UValue::fromJSON<T>(%V,%p)", name.rep, &member)

      UValue& json = operator[](name);

      member.fromJSON(json);
      }

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
         result = u_strtol(x.data(), x.pend());

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool jfind(const UString& json, const char* query, uint32_t query_len, int64_t& result)
      {
      U_TRACE(0, "UValue::jfind(%V,%.*S,%u,%p)", json.rep, query_len, query, query_len, &result)

      UString x;

      if (jfind(json, query, query_len, x))
         {
         result = u_strtoll(x.data(), x.pend());

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool jfind(const UString& json, const UString& query, int64_t& result) { return jfind(json, U_STRING_TO_PARAM(query), result); }

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
   const char* dump(bool _reset) const;

   static const char* getJReadErrorDescription();
   static const char* getDataTypeDescription(int type);
#else
   static const char* getJReadErrorDescription()  { return ""; }
   static const char* getDataTypeDescription(int) { return ""; }
#endif

protected:
   UValue* next;    // only if binded to an object or array
   union jval pkey, // only if binded to an object
              value;

   static UValue* phead;
   static UValue* pnode;
   static uint32_t size;
   static char* pstringify; // buffer to stringify json

#ifdef DEBUG
   static uint32_t cnt_real, cnt_mreal;
#endif

   explicit UValue(uint64_t val)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "0x%x", val)

      next = this;

      // coverity[uninit_ctor]
#  ifdef U_COVERITY_FALSE_POSITIVE
      pkey.ival = 0ULL;
#  endif

      value.ival = val;

      U_INTERNAL_DUMP("this = %p", this)
      }

   explicit UValue(UValue* node)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%p", node)

      next = node;

      // coverity[uninit_ctor]
#  ifdef U_COVERITY_FALSE_POSITIVE
      pkey.ival = 0ULL;
#  endif

      value.ival = 0ULL;

      U_INTERNAL_DUMP("this = %p", this)
      }

   explicit UValue(uint64_t val, UValue* node)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "0x%x,%p", val, node)

      next = node;

      // coverity[uninit_ctor]
#  ifdef U_COVERITY_FALSE_POSITIVE
      pkey.ival = 0ULL;
#  endif

      value.ival = val;

      U_INTERNAL_DUMP("this = %p", this)
      }

   void emitUTF(UStringRep* rep) const;

   void emitString(UStringRep* rep) const
      {
      U_TRACE(0, "UValue::emitString(%p)", rep)

      U_INTERNAL_DUMP("rep = %V", rep)

      *pstringify++ = '"';

      if (rep != UStringRep::string_rep_null)
         {
         uint32_t sz = rep->size();

         U_MEMCPY(pstringify, rep->data(), sz);
                  pstringify +=            sz;
         }

      *pstringify++ = '"';
      }

   void emitKey() const
      {
      U_TRACE_NO_PARAM(0, "UValue::emitKey()")

      int type = getTag(pkey.ival);

      if (type == STRING_VALUE) emitString((UStringRep*)getPayload(pkey.ival));
      else
         {
         U_INTERNAL_ASSERT_EQUALS(type, UTF_VALUE)

         emitUTF((UStringRep*)getPayload(pkey.ival));
         }
      }

   void stringify() const;
   void  prettify(uint32_t indent) const;

   uint64_t getPayload() const { return getPayload(value.ival); }

   UValue* toNode() const { return toNode(value.ival); }

   static UValue* toNode(uint64_t val)
      {
      U_TRACE(0, "UValue::toNode(0x%x)", val)

      U_ASSERT(isArrayOrObject(val))

      UValue* ptr = (UValue*)getPayload(val);

      U_RETURN_POINTER(ptr, UValue);
      }

   static uint64_t getPayload(uint64_t val)
      {
      U_TRACE(0, "UValue::getPayload(0x%x)", val)

      return ((uint64_t)(val & U_JSON_VALUE_PAYLOAD_MASK));
      }

   static bool isStringUTF(uint64_t value)
      {
      U_TRACE(0+256, "UValue::isStringUTF(0x%x)", value)

      if (getTag(value) == UTF_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isStringOrUTF(uint64_t value)
      {
      U_TRACE(0+256, "UValue::isStringOrUTF(0x%x)", value)

      int type = getTag(value);

      if (type == STRING_VALUE ||
          type ==    UTF_VALUE)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool isArrayOrObject(uint64_t value)
      {
      U_TRACE(0+256, "UValue::isArrayOrObject(0x%x)", value)

      int type = getTag(value);

      if (type ==  ARRAY_VALUE ||
          type == OBJECT_VALUE)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static uint64_t getJsonValue(int tag, void* payload)
      {
      U_TRACE(0, "UValue::getJsonValue(%d,%p)", tag, payload)

      U_INTERNAL_ASSERT(payload <= (void*)U_JSON_VALUE_PAYLOAD_MASK)

      return                   U_JSON_VALUE_NAN_MASK   |
             ((uint64_t)tag << U_JSON_VALUE_TAG_SHIFT) |
              (uint64_t)payload;
      }

   static uint64_t listToValue(ValueType tag, UValue* tail)
      {
      U_TRACE(0, "UValue::listToValue(%d,%p)", tag, tail)

      if (tail)
         {
         UValue* head = tail->next;
                        tail->next = 0;

         return getJsonValue(tag, head);
         }

      return getJsonValue(tag, 0);
      }

   static UValue* insertAfter(UValue* tail, uint64_t value)
      {
      U_TRACE(0, "UValue::insertAfter(%p,0x%x)", tail, value)

      UValue* node;

      if (tail)
         {
         U_NEW(UValue, node, UValue(value, tail->next));
                                           tail->next = node;
         }
      else
         {
         U_NEW(UValue, node, UValue(value));
         }

      U_RETURN_POINTER(node, UValue);
      }

   void setString(UStringRep* rep)
      {
      U_TRACE(0, "UValue::setString(%p)", rep)

      U_INTERNAL_DUMP("rep = %V", rep)

      rep->hold();

      value.ival = getJsonValue(STRING_VALUE, rep);
      }

   static UString getString(uint64_t value);

private:
   static int jread_skip(UTokenizer& tok) U_NO_EXPORT;
   static int jreadFindToken(UTokenizer& tok) U_NO_EXPORT;

   static UString jread_string(UTokenizer& tok) U_NO_EXPORT;
   static UString jread_object(UTokenizer& tok) U_NO_EXPORT;
   static UString jread_object(UTokenizer& tok, uint32_t keyIndex) U_NO_EXPORT;

   friend class UValueIter;
   friend class UTokenizer;
   friend UValueIter begin(const union jval);

   template <class T> friend class UVector;
   template <class T> friend class UHashMap;
   template <class T> friend class UJsonTypeHandler;
   template <class T> friend void JSON_stringify(UString&, UValue&, T&);
};

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX11)
class UValueIter { // this class is to make work Range-based for loop: for ( auto i : UValue ) loop_statement     
public:
   explicit UValueIter(const UValue* p) : _p(p) {}

   // these three methods form the basis of an iterator for use with a range-based for loop
   bool operator!=(const UValueIter& other) const { return (_p != other._p); }

   const UValue& operator*() const { return *_p; }

   const UValueIter& operator++()
      {
      _p = _p->next;

      // although not strictly necessary for a range-based for loop
      // following the normal convention of returning a value from
      // operator++ is a good idea

      return *this;
      }

private:
   const UValue* _p;
};

inline UValueIter end(  const union UValue::jval)   { return UValueIter(0); }
inline UValueIter begin(const union UValue::jval v) { return UValueIter(UValue::toNode(v.ival)); }
#endif

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
                            U_STRING_FROM_CONSTANT(#name_object_member), UJsonTypeHandler<type_object_member>(((class_name*)pval)->name_object_member)

/**
 * Provide template specializations to support your own complex types
 *
 * Take as example the following (simplified) class:
 *
 *   class Person {
 *   public:
 *      UString  lastName;
 *      UString firstName;
 *      int     age;
 *   };
 *
 * The UJsonTypeHandler must provide a custom toJSON and fromJSON method:
 *
 *   template <> class UJsonTypeHandler<Person> : public UJsonTypeHandler_Base {
 *   public:
 *      explicit UJsonTypeHandler(Person& val) : UJsonTypeHandler_Base(&val)
 *
 *      void toJSON(UValue& json)
 *       {
 *         json.toJSON(U_JSON_TYPE_HANDLER(Person,  lastName, UString));
 *         json.toJSON(U_JSON_TYPE_HANDLER(Person, firstName, UString));
 *         json.toJSON(U_JSON_TYPE_HANDLER(Person, age,       int));
 *       }
 *
 *      void fromJSON(UValue& json)
 *       {
 *         json.fromJSON(U_JSON_TYPE_HANDLER(Person,  lastName, UString));
 *         json.fromJSON(U_JSON_TYPE_HANDLER(Person, firstName, UString));
 *         json.fromJSON(U_JSON_TYPE_HANDLER(Person, age,       int));
 *       }
 *   };
 */

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

template <class T> inline void JSON_stringify(UString& result, UValue& json, T& obj)
{
   U_TRACE(0, "JSON_stringify(%V,%p,%p)", result.rep, &json, &obj)

   UValue::pnode = 0;

   json.toJSON(UJsonTypeHandler<T>(obj));

   if (UValue::pnode) json.value.ival = UValue::getJsonValue(UValue::OBJECT_VALUE, UValue::phead);

   UValue::stringify(result, json);
}

// TEMPLATE SPECIALIZATIONS

template <> class U_EXPORT UJsonTypeHandler<null> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(null&) : UJsonTypeHandler_Base(0) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<null>::toJSON(%p)", &json)

      json.value.ival = UValue::getJsonValue(UValue::NULL_VALUE, 0);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<null>::fromJSON(%p)", &json)

      U_VAR_UNUSED(json)
      }
};

template <> class U_EXPORT UJsonTypeHandler<bool> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(bool& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<bool>::toJSON(%p)", &json)

      json.value.ival = UValue::getJsonValue(*(bool*)pval ? UValue::TRUE_VALUE : UValue::FALSE_VALUE, 0);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<bool>::fromJSON(%p)", &json)

      *(bool*)pval = json.getBool();
      }
};

template <> class U_EXPORT UJsonTypeHandler<char> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(char& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<char>::toJSON(%p)", &json)

      json.value.ival = UValue::getJsonValue(UValue::UINT_VALUE, (void*)(*(char*)pval & 0x00000000FFFFFFFFULL));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<char>::fromJSON(%p)", &json)

      *(char*)pval = json.getUInt();
      }
};

template <> class U_EXPORT UJsonTypeHandler<unsigned char> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(unsigned char& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned char>::toJSON(%p)", &json)

      json.value.ival = UValue::getJsonValue(UValue::UINT_VALUE, (void*)(*(unsigned char*)pval & 0x00000000FFFFFFFFULL));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned char>::fromJSON(%p)", &json)

      *(unsigned char*)pval = json.getUInt();
      }
};

template <> class U_EXPORT UJsonTypeHandler<short> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(short& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<short>::toJSON(%p)", &json)

      json.value.ival = UValue::getJsonValue(UValue::UINT_VALUE, (void*)(*(short*)pval & 0x00000000FFFFFFFFULL));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<short>::fromJSON(%p)", &json)

      *(short*)pval = json.getUInt();
      }
};

template <> class U_EXPORT UJsonTypeHandler<unsigned short> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(unsigned short& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned short>::toJSON(%p)", &json)

      json.value.ival = UValue::getJsonValue(UValue::UINT_VALUE, (void*)(*(unsigned short*)pval & 0x00000000FFFFFFFFULL));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned short>::fromJSON(%p)", &json)

      *(unsigned short*)pval = json.getUInt();
      }
};

template <> class U_EXPORT UJsonTypeHandler<int> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(int& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<int>::toJSON(%p)", &json)

      json.value.ival = UValue::getJsonValue(UValue::INT_VALUE, (void*)(*(int*)pval & 0x00000000FFFFFFFFULL));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<int>::fromJSON(%p)", &json)

      *(int*)pval = json.getInt();
      }
};

template <> class U_EXPORT UJsonTypeHandler<unsigned int> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(unsigned int& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned int>::toJSON(%p)", &json)

      json.value.ival = UValue::getJsonValue(UValue::UINT_VALUE, (void*)(*(unsigned int*)pval & 0x00000000FFFFFFFFULL));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned int>::fromJSON(%p)", &json)

      *(unsigned int*)pval = json.getUInt();
      }
};

template <> class U_EXPORT UJsonTypeHandler<long> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(long& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long>::toJSON(%p)", &json)

      long l = *(long*)pval;

      int type = (l > (long)UINT_MAX || l < (long)INT_MAX ? UValue::REAL_VALUE :
                  l > 0                                   ? UValue::UINT_VALUE : UValue::INT_VALUE);

      json.value.ival = UValue::getJsonValue(type, (void*)(l & 0x00000000FFFFFFFFULL));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long>::fromJSON(%p)", &json)

      *(long*)pval = json.getNumber();
      }
};

template <> class U_EXPORT UJsonTypeHandler<unsigned long> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(unsigned long& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned long>::toJSON(%p)", &json)

      unsigned long l = *(unsigned long*)pval;

      json.value.ival = UValue::getJsonValue(l > UINT_MAX ? UValue::REAL_VALUE : UValue::UINT_VALUE, (void*)(l & 0x00000000FFFFFFFFULL));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned long>::fromJSON(%p)", &json)

      *(unsigned long*)pval = json.getNumber();
      }
};

template <> class U_EXPORT UJsonTypeHandler<long long> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(long long& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long long>::toJSON(%p)", &json)

      long long l = *(long long*)pval;

      int type = (l > UINT_MAX || l < INT_MAX ? UValue::REAL_VALUE :
                  l > 0                       ? UValue::UINT_VALUE : UValue::INT_VALUE);

      json.value.ival = UValue::getJsonValue(type, (void*)(l & 0x00000000FFFFFFFFULL));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long long>::fromJSON(%p)", &json)

      *(long long*)pval = json.getNumber();
      }
};

template <> class U_EXPORT UJsonTypeHandler<unsigned long long> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(unsigned long long& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned long long>::toJSON(%p)", &json)

      unsigned long long l = *(unsigned long long*)pval;

      json.value.ival = UValue::getJsonValue(l > UINT_MAX ? UValue::REAL_VALUE : UValue::UINT_VALUE, (void*)(l & 0x00000000FFFFFFFFULL));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned long long>::fromJSON(%p)", &json)

      *(unsigned long long*)pval = json.getNumber();
      }
};

template <> class U_EXPORT UJsonTypeHandler<float> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(float& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<float>::toJSON(%p)", &json)

      json.value.ival = UValue::getJsonValue(UValue::REAL_VALUE, (void*)lrintf(*(float*)pval));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<float>::fromJSON(%p)", &json)

      *(float*)pval = json.getDouble();
      }
};

template <> class U_EXPORT UJsonTypeHandler<double> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(double& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<double>::toJSON(%p)", &json)

      json.value.ival = UValue::getJsonValue(UValue::REAL_VALUE, (void*)lrint(*(double*)pval));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<double>::fromJSON(%p)", &json)

      *(double*)pval = json.getDouble();
      }
};

template <> class U_EXPORT UJsonTypeHandler<long double> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(long double& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long double>::toJSON(%p)", &json)

#  ifdef HAVE_LRINTL
      json.value.ival = UValue::getJsonValue(UValue::REAL_VALUE, (void*)lrintl(*(long double*)pval));
#  else
      json.value.ival = UValue::getJsonValue(UValue::REAL_VALUE, (void*)lrint(*(double*)pval));
#  endif
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long double>::fromJSON(%p)", &json)

      *(long double*)pval = json.getDouble();
      }
};

template <> class U_EXPORT UJsonTypeHandler<UStringRep> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(UStringRep& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UStringRep>::toJSON(%p)", &json)

      json.setString((UStringRep*)pval);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UStringRep>::fromJSON(%p)", &json)

      U_VAR_UNUSED(json)

      U_ASSERT(json.isString())

      U_INTERNAL_DUMP("pval(%p) = %p rep(%p) = %V", pval, pval, json.getPayload(), json.getPayload())

      U_ERROR("UJsonTypeHandler<UStringRep>::fromJSON(): sorry, we cannot use UStringRep type from JSON type handler...");
      }
};

template <> class U_EXPORT UJsonTypeHandler<UString> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(UString& val) : UJsonTypeHandler_Base(&val) {}

   void toJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UString>::toJSON(%p)", &json)

      json.setString(((UString*)pval)->rep);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UString>::fromJSON(%p)", &json)

      U_ASSERT(json.isString())

      UStringRep* rep = (UStringRep*)json.getPayload();

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

      UValue::phead = 0;
      uvector* pvec = (uvector*)pval;

      if (pvec->_length)
         {
         const void** ptr = pvec->vec;
         const void** end = pvec->vec + pvec->_length;

         for (; ptr < end; ++ptr)
            {
            UValue::toJSON1<T>(UJsonTypeHandler<T>(*(T*)(*ptr)));
            }
         }

      json.value.ival = UValue::getJsonValue(UValue::ARRAY_VALUE, UValue::phead);

      UValue::pnode = 0;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<uvector>::fromJSON(%p)", &json)

      UValue* element = json.toNode();

      while (element)
         {
         T* pitem;

         U_NEW(T, pitem, T);

         U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), UValue::getDataTypeDescription(element->getTag()))

         element->fromJSON(UJsonTypeHandler<T>(*pitem));

         ((uvector*)pval)->push_back(pitem);

         element = element->next;
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

      UValue* element = json.toNode();

      while (element)
         {
         UString item;

         U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), UValue::getDataTypeDescription(element->getTag()))

         element->fromJSON(UJsonTypeHandler<UString>(item));

         ((UVector<UString>*)pval)->push_back(item);

         element = element->next;
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

      UValue::phead = 0;
      uhashmap* pmap = (uhashmap*)pval;

      if (pmap->first())
         {
         do {
            UValue::toJSON<T>(pmap->getKey(), UJsonTypeHandler<T>(*(pmap->elem())));
            }
         while (pmap->next());
         }

      json.value.ival = UValue::getJsonValue(UValue::OBJECT_VALUE, UValue::phead);
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<uhashmap>::fromJSON(%p)", &json)

      UValue* element = json.toNode();

      while (element)
         {
         T* pitem;

         U_NEW(T, pitem, T);

         U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), UValue::getDataTypeDescription(element->getTag()))

         element->fromJSON(UJsonTypeHandler<T>(*pitem));

         UStringRep* rep = (UStringRep*)UValue::getPayload(element->pkey.ival);

         U_INTERNAL_DUMP("element->pkey(%p) = %V", rep, rep)

         ((uhashmap*)pval)->lookup(rep);

         ((uhashmap*)pval)->insertAfterFind(rep, pitem);

         element = element->next;
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
      U_TRACE(0, "UJsonTypeHandler<UHashMap<UString>>::fromJSON(%p)", &json)

      UValue* element = json.toNode();

      while (element)
         {
         UString item;

         U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), UValue::getDataTypeDescription(element->getTag()))

         element->fromJSON(UJsonTypeHandler<UString>(item));

         UStringRep* rep = (UStringRep*)UValue::getPayload(element->pkey.ival);

         U_INTERNAL_DUMP("element->pkey(%p) = %V", rep, rep)

         ((uhashmap*)pval)->lookup(rep);

         ((uhashmap*)pval)->insertAfterFind(rep, item.rep);

         element = element->next;
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

      UValue::phead = 0;
      stdvector* pvec = (stdvector*)pval;

      for (uint32_t i = 0, n = pvec->size(); i < n; ++i)
         {
         UValue::toJSON1<T>(UJsonTypeHandler<T>(pvec->at(i)));
         }

      json.value.ival = UValue::getJsonValue(UValue::ARRAY_VALUE, UValue::phead);

      UValue::pnode = 0;
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<stdvector>::fromJSON(%p)", &json)

      UValue* element = json.toNode();

      while (element)
         {
         T item;

         U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), UValue::getDataTypeDescription(element->getTag()))

         element->fromJSON(UJsonTypeHandler<T>(item));

         ((stdvector*)pval)->push_back(item);

         element = element->next;
         }
      }
};
#endif
#endif
