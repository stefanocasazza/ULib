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
 * Double use zero tag, so infinity and nan are accessible.
 *
 * The type of the held value is represented by a #ValueType and can be obtained using method getTag().
 * Values of an #U_OBJECT_VALUE or #U_ARRAY_VALUE can be accessed using operator[]() methods.
 * It is possible to iterate over the list of a #U_OBJECT_VALUE values using the getMemberNames() method
 */

#ifndef U_JSON_PARSE_STACK_SIZE
#define U_JSON_PARSE_STACK_SIZE 256
#endif

#define U_JFIND(json,str,result) UValue::jfind(json,#str,U_CONSTANT_SIZE(#str),result)

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

      value.ival = getValue(tag, payload);

      U_INTERNAL_DUMP("this = %p", this)
      }

   UValue(const UString& key, const UString& val)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%V,%V", key.rep, val.rep)

      UValue* node;

      U_NEW(UValue, node, UValue((UValue*)U_NULLPTR));

      value.ival = getValue(U_OBJECT_VALUE, node);

      key.hold();
      val.hold();

      node->pkey.ival  = getValue(U_STRING_VALUE, key.rep);
      node->value.ival = getValue(U_STRING_VALUE, val.rep);

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

   bool empty() const
      {
      U_TRACE_NO_PARAM(0, "UValue::empty()")

      if (value.ival == 0ULL) U_RETURN(true);

      U_RETURN(false);
      }

   uint32_t getTag() const { return getTag(value.ival); }

   bool isNull() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isNull()")

      if (getTag() == U_NULL_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isBool() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isBool()")

      uint32_t type = getTag();

      if (type ==  U_TRUE_VALUE ||
          type == U_FALSE_VALUE)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool isInt() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isInt()")

      if (getTag() == U_INT_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isUInt() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isUInt()")

      if (getTag() == U_UINT_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isDouble() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isDouble()")

      if (getTag() == U_REAL_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isNumeric() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isNumeric()")

      if (getTag() <= U_UINT_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isString() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isString()")

      if (getTag() == U_STRING_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isArray() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isArray()")

      if (getTag() == U_ARRAY_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   bool isObject() const
      {
      U_TRACE_NO_PARAM(0, "UValue::isObject()")

      if (getTag() == U_OBJECT_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   union jval getKey() const { return pkey; }

            int getInt() const  { return value.sint; }
   unsigned int getUInt() const { return value.uint; }

   UString getString() { return getString(value.ival); }

   bool   getBool() const   { return (getTag() == U_TRUE_VALUE); }
   double getDouble() const { return value.real; }

   bool isStringUTF() const     { return isStringUTF(value.ival); }
   bool isStringOrUTF() const   { return isStringOrUTF(value.ival); }
   bool isArrayOrObject() const { return isArrayOrObject(value.ival); }

   static uint32_t getStringSize(const union jval value) { return getString(value.ival).size(); }

   long long getNumber() const { return (isDouble() ? llrint(value.real) : (long long)(long)getPayload()); } 

   // manage values in array or object

   UValue* at(uint32_t pos) const __pure;
   UValue* at(const char* key, uint32_t key_len) const __pure;

   UValue* at(const UString& key) const { return at(U_STRING_TO_PARAM(key)); }

   UValue& operator[](uint32_t pos) const          { return *at(pos); }
   UValue& operator[](const UString& key) const    { return *at(U_STRING_TO_PARAM(key)); }
   UValue& operator[](const UStringRep* key) const { return *at(U_STRING_TO_PARAM(*key)); }

   bool isMemberExist(const UString& key) const    { return (at(U_STRING_TO_PARAM(key))  != U_NULLPTR); }
   bool isMemberExist(const UStringRep* key) const { return (at(U_STRING_TO_PARAM(*key)) != U_NULLPTR); }

   bool isMemberExist(const char* key, uint32_t key_len) const { return (at(key, key_len) != U_NULLPTR); }

   /**
    * \brief Return a list of the member names.
    *
    * \pre getTag() is U_OBJECT_VALUE
    * \post if getTag() was U_NULL_VALUE, it remains U_NULL_VALUE 
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
    * The JSON document is written in a single line. It is not intended for 'human'
    * consumption, but may be usefull to support feature such as RPC where bandwith is limited
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

  template <typename T> static void toJSON(const UString& name, UJsonTypeHandler<T> member)
      {
      U_TRACE(0, "UValue::toJSON<T>(%V,%p)", name.rep, &member)

      addString(name);

      member.toJSON();
      }

   template <typename T> static void toJSON(const char* name, uint32_t len, UJsonTypeHandler<T> member)
      {
      U_TRACE(0, "UValue::toJSON<T>(%.*S,%u,%p)", len, name, len, &member)

      addString(name, len);

      member.toJSON();
      }

   template <typename T> void fromJSON(const char* name, uint32_t len, UJsonTypeHandler<T> member)
      {
      U_TRACE(0, "UValue::fromJSON<T>(%.*S,%u,%p)", len, name, len, &member)

      UValue* node = at(name+1, len-2);

      if (node) member.fromJSON(*node);
      else      member.clear();
      }

   // =======================================================================================================================
   // An in-place JSON element reader (@see http://www.codeproject.com/Articles/885389/jRead-an-in-place-JSON-element-reader)
   // =======================================================================================================================

   static int      jread_error;
   static uint32_t jread_elements, jread_pos;

   static int  jread(const UString& json, const UString& query,                  UString& result, uint32_t* queryParams = U_NULLPTR);
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
         result = u__strtol(U_STRING_TO_PARAM(x));

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool jfind(const UString& json, const char* query, uint32_t query_len, unsigned int& result)
      {
      U_TRACE(0, "UValue::jfind(%V,%.*S,%u,%p)", json.rep, query_len, query, query_len, &result)

      UString x;

      if (jfind(json, query, query_len, x))
         {
         result = u__strtoul(U_STRING_TO_PARAM(x));

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
         result = u__strtoll(U_STRING_TO_PARAM(x));

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool jfind(const UString& json, const char* query, uint32_t query_len, uint64_t& result)
      {
      U_TRACE(0, "UValue::jfind(%V,%.*S,%u,%p)", json.rep, query_len, query, query_len, &result)

      UString x;

      if (jfind(json, query, query_len, x))
         {
         result = u__strtoull(U_STRING_TO_PARAM(x));

         U_RETURN(true);
         }

      U_RETURN(false);
      }

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

   static bool jfind(const UString& json, const UString& query,      UString& result) { return jfind(json, U_STRING_TO_PARAM(query), result); }
   static bool jfind(const UString& json, const UString& query,         bool& result) { return jfind(json, U_STRING_TO_PARAM(query), result); }
   static bool jfind(const UString& json, const UString& query,          int& result) { return jfind(json, U_STRING_TO_PARAM(query), result); }
   static bool jfind(const UString& json, const UString& query, unsigned int& result) { return jfind(json, U_STRING_TO_PARAM(query), result); }
   static bool jfind(const UString& json, const UString& query,      int64_t& result) { return jfind(json, U_STRING_TO_PARAM(query), result); }
   static bool jfind(const UString& json, const UString& query,     uint64_t& result) { return jfind(json, U_STRING_TO_PARAM(query), result); }
   static bool jfind(const UString& json, const UString& query,       double& result) { return jfind(json, U_STRING_TO_PARAM(query), result); }

   // reads one value from an array

   static void jreadArrayStepInit() { jread_pos = 0; }

   static int  jreadArrayStep(const UString& jarray, UString& result); // assumes jarray points at the start of an array or array element

#ifdef DEBUG
   const char* dump(bool _reset) const;

   static const char* getJReadErrorDescription();
   static const char* getDataTypeDescription(uint32_t type);
#else
   static const char* getJReadErrorDescription()       { return ""; }
   static const char* getDataTypeDescription(uint32_t) { return ""; }
#endif

protected:
   UValue* next;    // only if binded to an object or array
   union jval pkey, // only if binded to an object
              value;

   static uint32_t size;
   static char* pstringify; // buffer to stringify json

   typedef struct parser_stack_data {
      uint64_t keys;
      UValue* tails;
      bool obj;
   } parser_stack_data;

   static int pos;
   static union jval o;
   static parser_stack_data sd[U_JSON_PARSE_STACK_SIZE];

   static void initParser()
      {
      U_TRACE_NO_PARAM(0, "UValue::initParser()")

      pos    = -1;
      o.ival = 0ULL;
      }

   static void nextParser();
 
   static uint32_t getTag(uint64_t val)
      {
      U_TRACE(0, "UValue::getTag(%#llx)", val)

      uint32_t tag = ((int64_t)val <= (int64_t)U_VALUE_NAN_MASK ? (uint32_t)U_REAL_VALUE : u_getTag(val));

      U_INTERNAL_DUMP("tag = %u", tag)

      U_INTERNAL_ASSERT(tag <= U_NULL_VALUE)

      U_RETURN(tag);
      }

   static uint64_t getValue(uint16_t tag, void* payload)
      {
      U_TRACE(0+256, "UValue::getValue(%hu,%p)", tag, payload)

      U_INTERNAL_ASSERT(payload <= (void*)U_VALUE_PAYLOAD_MASK)

      uint64_t val = u_getValue(tag, payload);

   // U_INTERNAL_DUMP("val = %#llx tag = %#llx payload = %#llx", val, ((uint64_t)tag << U_VALUE_TAG_SHIFT), ((uint64_t)(long)payload & U_VALUE_PAYLOAD_MASK))

      U_ASSERT_EQUALS(getTag(val), tag)

      U_RETURN(val);
      }

   static void addString(const char* ptr, uint32_t sz)
      {
      U_TRACE(0, "UValue::addString(%.*S,%u)", sz, ptr, sz)

      if (sz)
         {
         UStringRep* rep;

         U_NEW(UStringRep, rep, UStringRep(ptr, sz));

         o.ival = getValue(U_STRING_VALUE, rep);
         }
      else
         {
         UStringRep::string_rep_null->hold();

         o.ival = getValue(U_STRING_VALUE, UStringRep::string_rep_null);
         }

      nextParser();
      }

   static void addString(const UString& str) { addString(U_STRING_TO_PARAM(str)); }

#ifdef DEBUG
   static uint32_t cnt_real, cnt_mreal;
#endif

   explicit UValue(uint64_t val)
      {
      U_TRACE_REGISTER_OBJECT(0, UValue, "%#llx", val)

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
      U_TRACE_REGISTER_OBJECT(0, UValue, "%#llx,%p", val, node)

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
         uint32_t len = rep->size();

         U_MEMCPY(pstringify, rep->data(), len);
                  pstringify +=            len;
         }

      *pstringify++ = '"';
      }

   void emitKey() const
      {
      U_TRACE_NO_PARAM(0, "UValue::emitKey()")

      uint32_t type = getTag(pkey.ival);

      if (type == U_STRING_VALUE) emitString((UStringRep*)u_getPayload(pkey.ival));
      else
         {
         U_INTERNAL_ASSERT_EQUALS(type, U_UTF_VALUE)

         emitUTF((UStringRep*)u_getPayload(pkey.ival));
         }
      }

   void stringify() const;
   void prettify(uint32_t indent) const;

   UValue* toNode() const { return toNode(value.ival); }

   uint64_t getPayload() const { return u_getPayload(value.ival); }

   static UValue* toNode(uint64_t val)
      {
      U_TRACE(0, "UValue::toNode(%#llx)", val)

      UValue* ptr = (UValue*)u_getPayload(val);

      U_RETURN_POINTER(ptr, UValue);
      }

   static bool isStringUTF(uint64_t value)
      {
      U_TRACE(0+256, "UValue::isStringUTF(%#llx)", value)

      if (getTag(value) == U_UTF_VALUE) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isStringOrUTF(uint64_t value)
      {
      U_TRACE(0+256, "UValue::isStringOrUTF(%#llx)", value)

      uint32_t type = getTag(value);

      if (type == U_STRING_VALUE ||
          type ==    U_UTF_VALUE)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool isArrayOrObject(uint64_t value)
      {
      U_TRACE(0+256, "UValue::isArrayOrObject(%#llx)", value)

      uint32_t type = getTag(value);

      if (type ==  U_ARRAY_VALUE ||
          type == U_OBJECT_VALUE)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static uint64_t listToValue(ValueType tag, UValue* tail)
      {
      U_TRACE(0, "UValue::listToValue(%d,%p)", tag, tail)

      if (tail)
         {
         UValue* head = tail->next;
                        tail->next = U_NULLPTR;

         return getValue(tag, head);
         }

      return getValue(tag, U_NULLPTR);
      }

   static UValue* insertAfter(UValue* tail, uint64_t value)
      {
      U_TRACE(0, "UValue::insertAfter(%p,%#llx)", tail, value)

      UValue* node;

      U_DUMP("value tag = %u", getTag(value))

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

      value.ival = getValue(U_STRING_VALUE, rep);
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

      // although not strictly necessary for a range-based for loop following
      // the normal convention of returning a value from operator++ is a good idea

      return *this;
      }

private:
   const UValue* _p;
};

inline UValueIter end(  const union UValue::jval)   { return UValueIter(U_NULLPTR); }
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

#define U_JSON_METHOD_HANDLER(name_object_member, type_object_member) \
                        "\"" #name_object_member "\"", sizeof(#name_object_member)+1, UJsonTypeHandler<type_object_member>(name_object_member)

#define U_JSON_TYPE_HANDLER(name_object_member, type_object_member) \
                        UValue::toJSON<type_object_member>(#name_object_member, sizeof(#name_object_member)-1, UJsonTypeHandler<type_object_member>(name_object_member))

/**
 * Provide template specializations to support your own complex types
 *
 * Take as example the following (simplified) class:
 *
 *   class Person {
 *   public:
 *      int age;
 *      UString  lastName;
 *      UString firstName;
 *   };
 *
 * add this methods to the (simplified) class:
 *
 * void Person::clear()
 *    {
 *    age = 0;
 *
 *     lastName.clear();
 *    firstName.clear();
 *    }
 *
 * void Person::toJSON(UString& json)
 *    {
 *    json.toJSON(U_JSON_METHOD_HANDLER(age,       int));
 *    json.toJSON(U_JSON_METHOD_HANDLER( lastName, UString));
 *    json.toJSON(U_JSON_METHOD_HANDLER(firstName, UString));
 *    }
 *
 * void Person::toJSON()
 *    {
 *    U_JSON_TYPE_HANDLER(age,       int);
 *    U_JSON_TYPE_HANDLER( lastName, UString);
 *    U_JSON_TYPE_HANDLER(firstName, UString);
 *    }
 *
 * void Person::fromJSON(UValue& json)
 *    {
 *    json.fromJSON(U_JSON_METHOD_HANDLER(age,       int));
 *    json.fromJSON(U_JSON_METHOD_HANDLER( lastName, UString));
 *    json.fromJSON(U_JSON_METHOD_HANDLER(firstName, UString));
 *    }
 */

template <class T> class U_EXPORT UJsonTypeHandler : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(T* val) : UJsonTypeHandler_Base( val) {}
   explicit UJsonTypeHandler(T& val) : UJsonTypeHandler_Base(&val) {}

   // SERVICES

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<T>::clear()")

      ((T*)pval)->clear();
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<T>::toJSON(%V)", json.rep)

      json.push_back('{');

      ((T*)pval)->toJSON(json);

      json.setLastChar('}');

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<T>::toJSON()")

      ++UValue::pos;

      U_INTERNAL_ASSERT_MINOR(UValue::pos, U_JSON_PARSE_STACK_SIZE)

#     ifndef HAVE_OLD_IOSTREAM
         UValue::sd[UValue::pos] = {0, U_NULLPTR, true};
#     else
         UValue::sd[UValue::pos].keys  = 0;
         UValue::sd[UValue::pos].tails = U_NULLPTR;
         UValue::sd[UValue::pos].obj   = true;
#     endif

      ((T*)pval)->toJSON();

      U_INTERNAL_DUMP("UValue::pos = %d UValue::sd[0].obj = %b UValue::sd[0].tails = %p UValue::sd[0].keys = %#llx",
                       UValue::pos,     UValue::sd[0].obj,     UValue::sd[0].tails,     UValue::sd[0].keys)

      U_INTERNAL_ASSERT_DIFFERS(UValue::pos, -1)
      U_INTERNAL_ASSERT(UValue::sd[UValue::pos].obj)
      U_INTERNAL_ASSERT_EQUALS(UValue::sd[UValue::pos].keys, 0)

      UValue::o.ival = UValue::listToValue(U_OBJECT_VALUE, UValue::sd[UValue::pos--].tails);

      if (UValue::pos != -1) UValue::nextParser();
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<T>::fromJSON(%p)", &json)

      ((T*)pval)->fromJSON(json);
      }

private:
   U_DISALLOW_ASSIGN(UJsonTypeHandler)
};

// manage object <=> JSON representation

template <class T> bool JSON_parse(const UString& str, T& obj)
{
   U_TRACE(0, "JSON_parse(%p,%p)", &str, &obj)

   UValue json;

   if (json.parse(str))
      {
      UJsonTypeHandler<T>(obj).fromJSON(json);

      U_RETURN(true);
      }

   U_RETURN(false);
}

template <class T> void JSON_OBJ_stringify(UString& str, T& obj)
{
   U_TRACE(0, "JSON_OBJ_stringify(%V,%p)", str.rep, &obj)

   UJsonTypeHandler<T>(obj).toJSON(str);

   U_INTERNAL_DUMP("str(%u) = %V", str.size(), str.rep)
}

template <class T> void JSON_stringify(UString& str, UValue& json, T& obj)
{
   U_TRACE(0, "JSON_stringify(%V,%p,%p)", str.rep, &json, &obj)

   U_ASSERT(json.empty())

   UValue::initParser();

   UJsonTypeHandler<T>(obj).toJSON();

   json.value.ival = UValue::o.ival;

   UValue::stringify(str, json);
}

// TEMPLATE SPECIALIZATIONS

template <> class U_EXPORT UJsonTypeHandler<null> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(null&) : UJsonTypeHandler_Base(U_NULLPTR) {}

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<null>::clear()")
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<null>::toJSON(%V)", json.rep)

      (void) json.append(U_CONSTANT_TO_PARAM("null"));

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<null>::toJSON()")

      UValue::o.ival = UValue::getValue(U_NULL_VALUE, U_NULLPTR);

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<bool>::clear()")

      *(bool*)pval = false;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<bool>::toJSON(%V)", json.rep)

      (void) json.append(*(*(bool*)pval ? UString::str_true : UString::str_false));

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<bool>::toJSON()")

      UValue::o.ival = UValue::getValue(*(bool*)pval ? U_TRUE_VALUE : U_FALSE_VALUE, U_NULLPTR);

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<char>::clear()")

      *(char*)pval = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<char>::toJSON(%V)", json.rep)

      json.push_back(*(char*)pval);

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<char>::toJSON()")

      UValue::o.ival = UValue::getValue(U_UINT_VALUE, (void*)(*(char*)pval & 0x00000000FFFFFFFFULL));

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<unsigned char>::clear()")

      *(unsigned char*)pval = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned char>::toJSON(%V)", json.rep)

      json.push_back(*(unsigned char*)pval);

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<unsigned char>::toJSON()")

      UValue::o.ival = UValue::getValue(U_UINT_VALUE, (void*)(*(unsigned char*)pval & 0x00000000FFFFFFFFULL));

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<short>::clear()")

      *(short*)pval = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<short>::toJSON(%V)", json.rep)

      json.appendNumber32s(*(short*)pval);

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<short>::toJSON()")

      UValue::o.ival = UValue::getValue(U_INT_VALUE, (void*)(*(short*)pval & 0x00000000FFFFFFFFULL));

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<unsigned short>::clear()")

      *(unsigned short*)pval = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned short>::toJSON(%V)", json.rep)

      json.appendNumber32(*(unsigned short*)pval);

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<unsigned short>::toJSON()")

      UValue::o.ival = UValue::getValue(U_UINT_VALUE, (void*)(*(unsigned short*)pval & 0x00000000FFFFFFFFULL));

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<int>::clear()")

      *(int*)pval = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<int>::toJSON(%V)", json.rep)

      json.appendNumber32s(*(int*)pval);

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<int>::toJSON()")

      UValue::o.ival = UValue::getValue(U_INT_VALUE, (void*)(*(int*)pval & 0x00000000FFFFFFFFULL));

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<unsigned int>::clear()")

      *(unsigned int*)pval = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned int>::toJSON(%V)", json.rep)

      json.appendNumber32(*(unsigned int*)pval);

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<unsigned int>::toJSON()")

      UValue::o.ival = UValue::getValue(U_UINT_VALUE, (void*)(*(unsigned int*)pval & 0x00000000FFFFFFFFULL));

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<long>::clear()")

      *(long*)pval = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long>::toJSON(%V)", json.rep)

#  if SIZEOF_LONG == 4
      json.appendNumber32s(*(long*)pval);
#  else
      json.appendNumber64s(*(long*)pval);
#  endif

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<long>::toJSON()")

      long l = *(long*)pval;

      U_INTERNAL_DUMP("pval = %ld", *(long*)pval)

#  if SIZEOF_LONG == 4
      uint32_t type = U_INT_VALUE; 
#  else
      uint32_t type = (l > UINT_MAX || l < INT_MIN ? U_REAL_VALUE :
                       l > 0                       ? U_UINT_VALUE : U_INT_VALUE);
#  endif

      UValue::o.ival = UValue::getValue(type, (void*)(l & 0x00000000FFFFFFFFULL));

      UValue::nextParser();
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long>::fromJSON(%p)", &json)

      *(long*)pval = json.getNumber();

      U_INTERNAL_DUMP("pval = %ld", *(long*)pval)
      }
};

template <> class U_EXPORT UJsonTypeHandler<unsigned long> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(unsigned long& val) : UJsonTypeHandler_Base(&val) {}

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<unsigned long>::clear()")

      *(unsigned long*)pval = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned long>::toJSON(%V)", json.rep)

#  if SIZEOF_LONG == 4
      json.appendNumber32(*(unsigned long*)pval);
#  else
      json.appendNumber64(*(unsigned long*)pval);
#  endif

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<unsigned long>::toJSON()")

      unsigned long l = *(unsigned long*)pval;

#  if SIZEOF_LONG == 4
      uint32_t type = U_UINT_VALUE; 
#  else
      uint32_t type = (l > UINT_MAX ? U_REAL_VALUE : U_UINT_VALUE);
#  endif

      UValue::o.ival = UValue::getValue(type, (void*)(l & 0x00000000FFFFFFFFULL));

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<long long>::clear()")

      *(long long*)pval = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long long>::toJSON(%V)", json.rep)

      json.appendNumber64s(*(long long*)pval);

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<long long>::toJSON()")

      long long l = *(long long*)pval;

      uint32_t type = (l > UINT_MAX || l < INT_MIN ? U_REAL_VALUE :
                       l > 0                       ? U_UINT_VALUE : U_INT_VALUE);

      U_INTERNAL_DUMP("l = %ld type = %u", l, type)

      UValue::o.ival = UValue::getValue(type, (void*)(l & 0x00000000FFFFFFFFULL));

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<unsigned long long>::clear()")

      *(unsigned long long*)pval = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<unsigned long long>::toJSON(%V)", json.rep)

      json.appendNumber64(*(unsigned long long*)pval);

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<unsigned long long>::toJSON()")

      unsigned long long l = *(unsigned long long*)pval;

      UValue::o.ival = UValue::getValue(l > UINT_MAX ? U_REAL_VALUE : U_UINT_VALUE, (void*)(l & 0x00000000FFFFFFFFULL));

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<float>::clear()")

      *(float*)pval = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<float>::toJSON(%V)", json.rep)

      json.appendNumberDouble(*(float*)pval);

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<float>::toJSON()")

      UValue::o.ival = UValue::getValue(U_REAL_VALUE, (void*)lrintf(*(float*)pval));

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<double>::clear()")

      *(double*)pval = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<double>::toJSON(%V)", json.rep)

      json.appendNumberDouble(*(double*)pval);

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<double>::toJSON()")

      UValue::o.ival = UValue::getValue(U_REAL_VALUE, (void*)lrint(*(double*)pval));

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<long double>::clear()")

      *(long double*)pval = 0;
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<long double>::toJSON(%V)", json.rep)

      json.appendNumberDouble(*(long double*)pval);

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<long double>::toJSON()")

#  ifdef HAVE_LRINTL
      UValue::o.ival = UValue::getValue(U_REAL_VALUE, (void*)lrintl(*(long double*)pval));
#  else
      UValue::o.ival = UValue::getValue(U_REAL_VALUE, (void*)lrint(*(       double*)pval));
#  endif

      UValue::nextParser();
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<UStringRep>::clear()")

      U_ERROR("UJsonTypeHandler<UStringRep>::fromJSON(): sorry, we cannot use UStringRep type from JSON type handler...");
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UStringRep>::toJSON(%V)", json.rep)

      json.appendDataQuoted(U_STRING_TO_PARAM(*(UStringRep*)pval));

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<UStringRep>::toJSON()")

      UValue::addString(U_STRING_TO_PARAM(*(UStringRep*)pval));
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UStringRep>::fromJSON(%p)", &json)

      U_VAR_UNUSED(json)

      U_ASSERT(json.isString())

      U_INTERNAL_DUMP("pval(%p) = %p json(%p) = %V", pval, pval, json.getPayload(), json.getPayload())

      U_ERROR("UJsonTypeHandler<UStringRep>::fromJSON(): sorry, we cannot use UStringRep type from JSON type handler...");
      }
};

template <> class U_EXPORT UJsonTypeHandler<UString> : public UJsonTypeHandler_Base {
public:
   explicit UJsonTypeHandler(UString& val) : UJsonTypeHandler_Base(&val) {}

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<UString>::clear()")

      U_INTERNAL_DUMP("pval(%p) = %V", pval, ((UString*)pval)->rep, ((UString*)pval)->rep)

      ((UString*)pval)->clear();
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UString>::toJSON(%V)", json.rep)

      json.appendDataQuoted(U_STRING_TO_PARAM(*(UString*)pval));

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<UString>::toJSON()")

      UValue::addString(*(UString*)pval);
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

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<uvector>::clear()")

      ((uvector*)pval)->clear();
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<uvector>::toJSON(%V)", json.rep)

      uvector* pvec = (uvector*)pval;

      json.push_back('[');

      if (pvec->_length)
         {
         const void** ptr = pvec->vec;
         const void** end = pvec->vec + pvec->_length;

         while (true)
            {
            UJsonTypeHandler<T>(*(T*)(*ptr)).toJSON(json);

            if (++ptr >= end) break;

            json.push_back(',');
            }
         }

      json.push_back(']');

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<uvector>::toJSON()")

      uvector* pvec = (uvector*)pval;

      if (pvec->_length == 0) UValue::o.ival = UValue::listToValue(U_ARRAY_VALUE, U_NULLPTR);
      else
         {
         const void** ptr = pvec->vec;
         const void** end = pvec->vec + pvec->_length;

         ++UValue::pos;

         U_INTERNAL_ASSERT_MINOR(UValue::pos, U_JSON_PARSE_STACK_SIZE)

#     ifndef HAVE_OLD_IOSTREAM
         UValue::sd[UValue::pos] = {0, U_NULLPTR, false};
#     else
         UValue::sd[UValue::pos].keys  = 0;
         UValue::sd[UValue::pos].tails = U_NULLPTR;
         UValue::sd[UValue::pos].obj   = false;
#     endif

         do { UJsonTypeHandler<T>(*(T*)(*ptr)).toJSON(); } while (++ptr < end);

         U_INTERNAL_ASSERT_DIFFERS(UValue::pos, -1)
         U_INTERNAL_ASSERT_EQUALS(UValue::sd[UValue::pos].obj, false)

         UValue::o.ival = UValue::listToValue(U_ARRAY_VALUE, UValue::sd[UValue::pos--].tails);
         }

      if (UValue::pos != -1) UValue::nextParser();
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<uvector>::fromJSON(%p)", &json)

      U_ASSERT(((uvector*)pval)->empty())

      UValue* pelement = json.toNode();

      while (pelement)
         {
         T* pitem;

         U_NEW(T, pitem, T);

         U_DUMP("pelement = %p pelement->next = %p pelement->type = (%u,%S)", pelement, pelement->next, pelement->getTag(), UValue::getDataTypeDescription(pelement->getTag()))

         UJsonTypeHandler<T>(*pitem).fromJSON(*pelement);

         ((UVector<void*>*)pval)->push_back(pitem);

         pelement = pelement->next;
         }
      }
};

template <> class U_EXPORT UJsonTypeHandler<UVector<UString> > : public UJsonTypeHandler<UVector<UStringRep*> > {
public:
   typedef UVector<UStringRep*> uvectorbase;

   explicit UJsonTypeHandler(UVector<UString>& val) : UJsonTypeHandler<uvectorbase>(*((uvector*)&val)) {}

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<UVector<UString>>::clear()")

      ((UVector<UString>*)pval)->clear();
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UVector<UString>>::toJSON(%V)", json.rep)

      uvectorbase* pvec = (uvectorbase*)pval;

      json.push_back('[');

      if (pvec->_length)
         {
         const void** ptr = pvec->vec;
         const void** end = pvec->vec + pvec->_length;

         while (true)
            {
            json.appendDataQuoted(U_STRING_TO_PARAM(*(UStringRep*)(*ptr)));

            if (++ptr >= end) break;

            json.push_back(',');
            }
         }

      json.push_back(']');

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<UVector<UString>>::toJSON()")

      uvectorbase* pvec = (uvectorbase*)pval;

      if (pvec->_length == 0) UValue::o.ival = UValue::listToValue(U_ARRAY_VALUE, U_NULLPTR);
      else
         {
         const void** ptr = pvec->vec;
         const void** end = pvec->vec + pvec->_length;

         ++UValue::pos;

         U_INTERNAL_ASSERT_MINOR(UValue::pos, U_JSON_PARSE_STACK_SIZE)

#     ifndef HAVE_OLD_IOSTREAM
         UValue::sd[UValue::pos] = {0, U_NULLPTR, false};
#     else
         UValue::sd[UValue::pos].keys  = 0;
         UValue::sd[UValue::pos].tails = U_NULLPTR;
         UValue::sd[UValue::pos].obj   = false;
#     endif

         do { UValue::addString(U_STRING_TO_PARAM(*(const UStringRep*)(*ptr))); } while (++ptr < end);

         U_INTERNAL_ASSERT_DIFFERS(UValue::pos, -1)
         U_INTERNAL_ASSERT_EQUALS(UValue::sd[UValue::pos].obj, false)

         UValue::o.ival = UValue::listToValue(U_ARRAY_VALUE, UValue::sd[UValue::pos--].tails);
         }

      if (UValue::pos != -1) UValue::nextParser();
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UVector<UString>>::fromJSON(%p)", &json)

      U_ASSERT(((UVector<UString>*)pval)->empty())

      UValue* pelement = json.toNode();

      while (pelement)
         {
         UString item;

         U_DUMP("pelement = %p pelement->next = %p pelement->type = (%u,%S)", pelement, pelement->next, pelement->getTag(), UValue::getDataTypeDescription(pelement->getTag()))

         UJsonTypeHandler<UString>(item).fromJSON(*pelement);

         ((UVector<UString>*)pval)->push_back(item);

         pelement = pelement->next;
         }
      }
};

template <class T> class U_EXPORT UJsonTypeHandler<UHashMap<T*> > : public UJsonTypeHandler_Base {
public:
   typedef UHashMap<T*> uhashmap;

   explicit UJsonTypeHandler(uhashmap& map) : UJsonTypeHandler_Base(&map) {}

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<uhashmap>::clear()")

      ((uhashmap*)pval)->clear();
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<uhashmap>::toJSON(%V)", json.rep)

      uhashmap* pmap = (uhashmap*)pval;

      if (pmap->first() == 0) (void) json.append(U_CONSTANT_TO_PARAM("{}"));
      else
         {
         json.push_back('{');

#     ifndef HAVE_OLD_IOSTREAM
         do { json.toJSON<T>(pmap->getKey(), UJsonTypeHandler<T>(*(pmap->elem()))); } while (pmap->next());
#     endif

         json.setLastChar('}');
         }

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<uhashmap>::toJSON()")

      uhashmap* pmap = (uhashmap*)pval;

      if (pmap->first() == U_NULLPTR) UValue::o.ival = UValue::listToValue(U_OBJECT_VALUE, U_NULLPTR);
      else
         {
         ++UValue::pos;

         U_INTERNAL_ASSERT_MINOR(UValue::pos, U_JSON_PARSE_STACK_SIZE)

#     ifndef HAVE_OLD_IOSTREAM
         UValue::sd[UValue::pos] = {0, U_NULLPTR, true};
#     else
         UValue::sd[UValue::pos].keys  = 0;
         UValue::sd[UValue::pos].tails = U_NULLPTR;
         UValue::sd[UValue::pos].obj   = true;
#     endif

         do { UValue::toJSON<T>(pmap->getKey(), *(pmap->elem())); } while (pmap->next());

         U_INTERNAL_ASSERT_DIFFERS(UValue::pos, -1)
         U_INTERNAL_ASSERT(UValue::sd[UValue::pos].obj)
         U_INTERNAL_ASSERT_EQUALS(UValue::sd[UValue::pos].keys, 0)

         UValue::o.ival = UValue::listToValue(U_OBJECT_VALUE, UValue::sd[UValue::pos--].tails);
         }

      if (UValue::pos != -1) UValue::nextParser();
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<uhashmap>::fromJSON(%p)", &json)

      U_ASSERT(((uhashmap*)pval)->empty())

      UValue* pelement = json.toNode();

      while (pelement)
         {
         T* pitem;

         U_NEW(T, pitem, T);

         U_DUMP("pelement = %p pelement->next = %p pelement->type = (%u,%S)", pelement, pelement->next, pelement->getTag(), UValue::getDataTypeDescription(pelement->getTag()))

         UJsonTypeHandler<T>(*pitem).fromJSON(*pelement);

         UStringRep* rep = (UStringRep*)u_getPayload(pelement->pkey.ival);

         U_INTERNAL_DUMP("pelement->pkey(%p) = %V", rep, rep)

         ((uhashmap*)pval)->lookup(rep);

         ((uhashmap*)pval)->insertAfterFind(rep, pitem);

         pelement = pelement->next;
         }
      }
};

template <> class U_EXPORT UJsonTypeHandler<UHashMap<UString> > : public UJsonTypeHandler<UHashMap<UStringRep*> > {
public:
   typedef UHashMap<UStringRep*> uhashmapbase;

   explicit UJsonTypeHandler(UHashMap<UString>& val) : UJsonTypeHandler<uhashmapbase>(*((uhashmap*)&val)) {}

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<UHashMap<UString>>::clear()")

      ((UHashMap<UString>*)pval)->clear();
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UHashMap<UString>>::toJSON(%V)", json.rep)

      UHashMap<UString>* pmap = (UHashMap<UString>*)pval;

      if (pmap->first() == U_NULLPTR) (void) json.append(U_CONSTANT_TO_PARAM("{}"));
      else
         {
         json.push_back('{');

         do { json.toJSON<UStringRep>(pmap->getKey(), UJsonTypeHandler<UStringRep>(*(UStringRep*)(pmap->elem()))); } while (pmap->next());

         json.setLastChar('}');
         }

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<UHashMap<UString>>::toJSON()")

      UHashMap<UString>* pmap = (UHashMap<UString>*)pval;

      if (pmap->first() == U_NULLPTR) UValue::o.ival = UValue::listToValue(U_OBJECT_VALUE, U_NULLPTR);
      else
         {
         ++UValue::pos;

         U_INTERNAL_ASSERT_MINOR(UValue::pos, U_JSON_PARSE_STACK_SIZE)

#     ifndef HAVE_OLD_IOSTREAM
         UValue::sd[UValue::pos] = {0, U_NULLPTR, true};
#     else
         UValue::sd[UValue::pos].keys  = 0;
         UValue::sd[UValue::pos].tails = U_NULLPTR;
         UValue::sd[UValue::pos].obj   = true;
#     endif

         do {
            UValue::addString(pmap->getKey());

            UValue::addString(U_STRING_TO_PARAM(*(const UStringRep*)pmap->elem()));
            }
         while (pmap->next());

         U_INTERNAL_ASSERT_DIFFERS(UValue::pos, -1)
         U_INTERNAL_ASSERT(UValue::sd[UValue::pos].obj)
         U_INTERNAL_ASSERT_EQUALS(UValue::sd[UValue::pos].keys, 0)

         UValue::o.ival = UValue::listToValue(U_OBJECT_VALUE, UValue::sd[UValue::pos--].tails);
         }

      if (UValue::pos != -1) UValue::nextParser();
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<UHashMap<UString>>::fromJSON(%p)", &json)

      U_ASSERT(((UHashMap<UString>*)pval)->empty())

      UValue* pelement = json.toNode();

      while (pelement)
         {
         UString item;

         U_DUMP("pelement = %p pelement->next = %p pelement->type = (%u,%S)", pelement, pelement->next, pelement->getTag(), UValue::getDataTypeDescription(pelement->getTag()))

         UJsonTypeHandler<UString>(item).fromJSON(*pelement);

         UStringRep* rep = (UStringRep*)u_getPayload(pelement->pkey.ival);

         U_INTERNAL_DUMP("pelement->pkey(%p) = %V", rep, rep)

         ((UHashMap<UString>*)pval)->lookup(rep);

         ((uhashmapbase*)pval)->insertAfterFind(rep, item.rep);

         pelement = pelement->next;
         }
      }
};

#ifdef U_STDCPP_ENABLE
#  include <vector>

template <class T> class U_EXPORT UJsonTypeHandler<std::vector<T> > : public UJsonTypeHandler_Base {
public:
   typedef std::vector<T> stdvector;

   explicit UJsonTypeHandler(stdvector& val) : UJsonTypeHandler_Base(&val) {}

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<stdvector>::clear()")

      // TODO
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<stdvector>::toJSON(%V)", json.rep)

      stdvector* pvec = (stdvector*)pval;
      uint32_t i = 0, n = pvec->size();

      json.push_back('[');

      while (true)
         {
         UJsonTypeHandler<T>(pvec->at(i)).toJSON(json);

         if (++i >= n) break;

         json.push_back(',');
         }

      json.push_back(']');

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void toJSON()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<stdvector>::toJSON()")

      stdvector* pvec = (stdvector*)pval;

      uint32_t n = pvec->size();

      if (n == 0) UValue::o.ival = UValue::listToValue(U_ARRAY_VALUE, U_NULLPTR);
      else
         {
         ++UValue::pos;

         U_INTERNAL_ASSERT_MINOR(UValue::pos, U_JSON_PARSE_STACK_SIZE)

#     ifndef HAVE_OLD_IOSTREAM
         UValue::sd[UValue::pos] = {0, U_NULLPTR, false};
#     else
         UValue::sd[UValue::pos].keys  = 0;
         UValue::sd[UValue::pos].tails = U_NULLPTR;
         UValue::sd[UValue::pos].obj   = false;
#     endif

         for (uint32_t i = 0; i < n; ++i) UJsonTypeHandler<T>(pvec->at(i)).toJSON();

         U_INTERNAL_ASSERT_DIFFERS(UValue::pos, -1)
         U_INTERNAL_ASSERT_EQUALS(UValue::sd[UValue::pos].obj, false)

         UValue::o.ival = UValue::listToValue(U_ARRAY_VALUE, UValue::sd[UValue::pos--].tails);
         }

      if (UValue::pos != -1) UValue::nextParser();
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<stdvector>::fromJSON(%p)", &json)

   // U_ASSERT(((stdvector*)pval)->empty())

      UValue* pelement = json.toNode();

      while (pelement)
         {
         T item;

         U_DUMP("pelement = %p pelement->next = %p pelement->type = (%u,%S)", pelement, pelement->next, pelement->getTag(), UValue::getDataTypeDescription(pelement->getTag()))

         UJsonTypeHandler<T>(item).fromJSON(*pelement);

         ((stdvector*)pval)->push_back(item);

         pelement = pelement->next;
         }
      }
};

// by Victor Stewart

#  ifdef HAVE_CXX17
#     include <unordered_map>

template <class T> class U_EXPORT UJsonTypeHandler<std::unordered_map<UString, T> > : public UJsonTypeHandler_Base {
public:
   typedef std::unordered_map<UString, T> stringtobitmaskmap;

   explicit UJsonTypeHandler(stringtobitmaskmap& map) : UJsonTypeHandler_Base(&map) {}

   void clear()
      {
      U_TRACE_NO_PARAM(0, "UJsonTypeHandler<stringtobitmaskmap>::clear()")

      ((stringtobitmaskmap*)pval)->clear();   
      }

   void toJSON(UString& json)
      {
      U_TRACE(0, "UJsonTypeHandler<stringtobitmaskmap>::toJSON(%V)", json.rep)

      stringtobitmaskmap* pmap = (stringtobitmaskmap*)pval;

      if (pmap->empty()) (void) json.append(U_CONSTANT_TO_PARAM("{}"));
      else
         {
         json.push_back('{');

         // this is is C++17 vvv
         for (const auto & [ key, value ] : *pmap) json.toJSON<T>(key, UJsonTypeHandler<T>(value)); 

         json.setLastChar('}');
         }

      U_INTERNAL_DUMP("json(%u) = %V", json.size(), json.rep)
      }

   void fromJSON(UValue& json)
      {
      U_TRACE(0, "UJsonTypeHandler<stringtobitmaskmap>::fromJSON(%p)", &json)

      U_ASSERT(((stringtobitmaskmap*)pval)->empty())

      UValue* pelement = json.toNode();

      while (pelement)
         {
         uint64_t pitem;

         UJsonTypeHandler<uint64_t>(pitem).fromJSON(*pelement);

         UStringRep* rep = (UStringRep*)u_getPayload(pelement->pkey.ival);

         U_INTERNAL_DUMP("pelement->pkey(%p) = %V", rep, rep)

         ((stringtobitmaskmap*)pval)->insert_or_assign(rep, pitem); // insert_or_assign is C++17 

         pelement = pelement->next;
         }
      }
};
#  endif
#endif
#endif
