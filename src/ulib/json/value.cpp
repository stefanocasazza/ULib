// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//   value.cpp - Represents a JSON (JavaScript Object Notation) value
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/tokenizer.h>
#include <ulib/json/value.h>
#include <ulib/utility/escape.h>

UValue::UValue(ValueType _type)
{
   U_TRACE_REGISTER_OBJECT(0, UValue, "%d", _type)

   U_INTERNAL_ASSERT_RANGE(0,_type,OBJECT_VALUE)

   switch ((type_ = _type))
      {
      case BOOLEAN_VALUE: value.bool_ = false;                    break;
      case     INT_VALUE:
      case    UINT_VALUE: value.int_  = 0;                        break;
      case    NULL_VALUE:
      case    REAL_VALUE: value.real_ = 0.0;                      break;
      case  STRING_VALUE: value.ptr_  = UString::string_null;     break;
      case   ARRAY_VALUE: value.ptr_  = U_NEW( UVector<UValue*>); break;
      case  OBJECT_VALUE: value.ptr_  = U_NEW(UHashMap<UValue*>); break;
      }
}

UValue::UValue(const UString& key, const UString& value_)
{
   U_TRACE_REGISTER_OBJECT(0, UValue, "%.*S,%.*S", U_STRING_TO_TRACE(key), U_STRING_TO_TRACE(value_))

   type_      = OBJECT_VALUE;
   value.ptr_ = U_NEW(UHashMap<UValue*>(key, U_NEW(UValue(value_))));
}

UValue::~UValue()
{
   U_TRACE_UNREGISTER_OBJECT(0, UValue)

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   switch (type_)
      {
      case STRING_VALUE:
         {
         U_INTERNAL_ASSERT_POINTER(value.ptr_)

         delete getString();
         }
      break;

      case ARRAY_VALUE:
         {
         U_INTERNAL_ASSERT_POINTER(value.ptr_)

         delete getArray();
         }
      break;

      case OBJECT_VALUE:
         {
         U_INTERNAL_ASSERT_POINTER(value.ptr_)

         UHashMap<UValue*>* ptr = getObject();

         if (ptr->capacity() == 1) ptr->clearTmpNode();

         delete ptr;
         }
      break;
      }
}

// CONVERSION

__pure bool UValue::asBool() const
{
   U_TRACE(0, "UValue::asBool()")

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   bool result = false;

   switch (type_)
      {
      case    NULL_VALUE:                                                   break;
      case     INT_VALUE: if (value.int_)         result = true;            break;
      case    UINT_VALUE: if (value.uint_)        result = true;            break;
      case    REAL_VALUE: if (value.real_ != 0.0) result = true;            break;
      case BOOLEAN_VALUE:                         result = value.bool_;     break;

      case  STRING_VALUE: result = getString()->empty(); break;
      case   ARRAY_VALUE: result =  getArray()->empty(); break;
      case  OBJECT_VALUE: result = getObject()->empty(); break;
      }

   U_RETURN(result);
}

__pure int UValue::asInt() const
{
   U_TRACE(0, "UValue::asInt()")

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   int result = 0;

   switch (type_)
      {
      case    NULL_VALUE:                                       break;
      case     INT_VALUE:                  result = value.int_; break;
      case BOOLEAN_VALUE: if (value.bool_) result = 1;          break;
      case    UINT_VALUE:
         U_INTERNAL_ASSERT_MSG(value.uint_ < (unsigned)INT_MAX, "Integer out of signed integer range")
         result = (int)value.uint_;
      break;
      case    REAL_VALUE:
         U_INTERNAL_ASSERT_MSG(value.real_ >= INT_MIN && value.real_ <= INT_MAX, "Real out of signed integer range")
         result = (int)value.real_;
      break;

      case  STRING_VALUE:
      case   ARRAY_VALUE:
      case  OBJECT_VALUE:
         U_INTERNAL_ASSERT_MSG(false, "Type is not convertible to signed integer...")
      break;
      }

   U_RETURN(result);
}

__pure unsigned int UValue::asUInt() const
{
   U_TRACE(0, "UValue::asUInt()")

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   unsigned int result = 0;

   switch (type_)
      {
      case    NULL_VALUE:                                        break;
      case    UINT_VALUE:                  result = value.uint_; break;
      case BOOLEAN_VALUE: if (value.bool_) result = 1;           break;
      case     INT_VALUE:
         U_INTERNAL_ASSERT_MSG(value.int_ >= 0, "Negative integer can not be converted to unsigned integer")
         result = value.int_;
      break;

      case    REAL_VALUE:
         U_INTERNAL_ASSERT_MSG(value.real_ >= 0.0 && value.real_ <= UINT_MAX, "Real out of unsigned integer range")
         result = (unsigned int)value.real_;
      break;

      case  STRING_VALUE:
      case   ARRAY_VALUE:
      case  OBJECT_VALUE:
         U_INTERNAL_ASSERT_MSG(false, "Type is not convertible to unsigned integer...")
      break;
      }

   U_RETURN(result);
}

__pure double UValue::asDouble() const
{
   U_TRACE(0, "UValue::asDouble()")

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   double result = 0.0;

   switch (type_)
      {
      case    NULL_VALUE:                                        break;
      case     INT_VALUE:                  result = value.int_;  break;
      case    UINT_VALUE:                  result = value.uint_; break;
      case    REAL_VALUE:                  result = value.real_; break;
      case BOOLEAN_VALUE: if (value.bool_) result = 1.0;         break;

      case  STRING_VALUE:
      case   ARRAY_VALUE:
      case  OBJECT_VALUE:
         U_INTERNAL_ASSERT_MSG(false, "Type is not convertible to double...")
      break;
      }

   U_RETURN(result);
}

__pure UString UValue::asString() const
{
   U_TRACE(0, "UValue::asString()")

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   UString result;

   switch (type_)
      {
      case    NULL_VALUE:                        break;
      case  STRING_VALUE: result = *getString(); break;

      case BOOLEAN_VALUE: result = (value.bool_ ? U_STRING_FROM_CONSTANT("true")
                                                : U_STRING_FROM_CONSTANT("false")); break;

      case     INT_VALUE:
      case    UINT_VALUE:
      case    REAL_VALUE:
      case   ARRAY_VALUE:
      case  OBJECT_VALUE:
         U_INTERNAL_ASSERT_MSG(false, "Type is not convertible to string...")
      break;
      }

   U_RETURN_STRING(result);
}

__pure bool UValue::isConvertibleTo(ValueType other) const
{
   U_TRACE(0, "UValue::isConvertibleTo(%d)", other)

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   bool result = false;

   switch (type_)
      {
      case    NULL_VALUE: result = true;                                          break;
      case BOOLEAN_VALUE: result = (other != NULL_VALUE || value.bool_ == false); break;

      case     INT_VALUE: result = (other == NULL_VALUE && value.int_ == 0)        ||
                                    other ==  INT_VALUE                            ||
                                   (other == UINT_VALUE && value.int_ >= 0)        ||
                                    other == REAL_VALUE                            ||
                                    other == BOOLEAN_VALUE;
      break;
      case    UINT_VALUE: result = (other == NULL_VALUE && value.uint_ ==       0) ||
                                   (other ==  INT_VALUE && value.uint_ <= INT_MAX) ||
                                    other == UINT_VALUE                            ||
                                    other == REAL_VALUE                            ||
                                    other == BOOLEAN_VALUE;
      break;
      case    REAL_VALUE: result = (other == NULL_VALUE && value.real_ ==     0.0)                            ||
                                   (other ==  INT_VALUE && value.real_ >= INT_MIN && value.real_ <=  INT_MAX) ||
                                   (other == UINT_VALUE && value.real_ >=     0.0 && value.real_ <= UINT_MAX) ||
                                    other == REAL_VALUE                                                       ||
                                    other == BOOLEAN_VALUE;
      break;
      case  STRING_VALUE: result = (other == NULL_VALUE && getString()->empty()) ||
                                    other == STRING_VALUE;
      break;
      case   ARRAY_VALUE: result = (other == NULL_VALUE &&  getArray()->empty()) ||
                                    other == ARRAY_VALUE;
      break;
      case  OBJECT_VALUE: result = (other == NULL_VALUE && getObject()->empty()) ||
                                    other == OBJECT_VALUE;
      break;
      }

   U_RETURN(result);
}

__pure uint32_t UValue::size() const
{
   U_TRACE(0, "UValue::size()")

   uint32_t result = 0;

        if (type_ == STRING_VALUE) result = getString()->size();
   else if (type_ ==  ARRAY_VALUE) result =  getArray()->size();
   else if (type_ == OBJECT_VALUE) result = getObject()->size();

   U_RETURN(result);
}

__pure UValue& UValue::operator[](uint32_t pos)
{
   U_TRACE(0, "UValue::operator[](%u)", pos)

   UValue* result = (type_ == ARRAY_VALUE ? getArray()->at(pos) : this);

   return *result;
}

__pure UValue& UValue::operator[](const UString& key)
{
   U_TRACE(0, "UValue::operator[](%.*S)", U_STRING_TO_TRACE(key))

   UValue* result = (type_ == OBJECT_VALUE ? getObject()->operator[](key) : this);

   return *result;
}

uint32_t UValue::getMemberNames(UVector<UString>& members) const
{
   U_TRACE(0, "UValue::getMemberNames(%p)", &members)

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)
   U_INTERNAL_ASSERT(type_ == OBJECT_VALUE || type_ == NULL_VALUE)

   uint32_t n = members.size(), _size = 0;

   if (type_ != NULL_VALUE)
      {
      getObject()->getKeys(members);

      _size = members.size() - n;
      }

   U_RETURN(_size);
}

void UValue::stringify(UString& result, UValue& _value)
{
   U_TRACE(0, "UValue::stringify(%.*S,%p)", U_STRING_TO_TRACE(result), &_value)

   U_INTERNAL_DUMP("_value.type_ = %u", _value.type_)

   U_INTERNAL_ASSERT_RANGE(0,_value.type_,OBJECT_VALUE)

   char buffer[32];

   switch (_value.type_)
      {
      case    NULL_VALUE:                   (void) result.append(U_CONSTANT_TO_PARAM("null"));  break;
      case BOOLEAN_VALUE: _value.asBool() ? (void) result.append(U_CONSTANT_TO_PARAM("true"))
                                          : (void) result.append(U_CONSTANT_TO_PARAM("false")); break;

      case  INT_VALUE: (void) result.append(buffer, u_num2str32s(buffer, _value.asInt()));  break;
      case UINT_VALUE: (void) result.append(buffer, u_num2str32( buffer, _value.asUInt())); break;
      case REAL_VALUE:
         {
         uint32_t n = u__snprintf(buffer, sizeof(buffer), "%#.16g", _value.asDouble());

         const char* ch = buffer + n - 1;

         if (*ch == '0')
            {
            while (ch > buffer && *ch == '0') --ch;

            const char* last_nonzero = ch;

            while (ch >= buffer)
               {
               switch (*ch)
                  {
                  case '0':
                  case '1':
                  case '2':
                  case '3':
                  case '4':
                  case '5':
                  case '6':
                  case '7':
                  case '8':
                  case '9': --ch; continue;
                  case '.': n = last_nonzero - buffer + 2; // Truncate zeroes to save bytes in output, but keep one.
                  default: goto end;
                  }
               }
            }
end:
         (void) result.append(buffer, n);

         break;
         }

      case STRING_VALUE:
         {
         UString* pstring = _value.getString();

         (void) result.reserve(result.size() + pstring->size() * 6);

         UEscape::encode(*pstring, result, true);
         }
      break;

      case ARRAY_VALUE:
         {
         result.push_back('[');

         uint32_t sz = _value.size();

         if (sz)
            {
            uint32_t index = 0;

            while (true)
               {
               stringify(result, _value[index]);

               if (++index == sz) break;

               result.push_back(',');
               }
            }

         result.push_back(']');
         }
      break;

      case OBJECT_VALUE:
         {
         char* presult;
         const char* keyptr;
         UHashMapNode** ptr;
         UHashMapNode** end;
         UHashMapNode* _node;
         UHashMapNode* _next;
         bool bcomma = false;
         uint32_t pos, sz, keysz;
         UHashMap<UValue*>* t = _value.getObject();

         U_INTERNAL_DUMP("t->_length = %u t->_capacity = %u", t->_length, t->_capacity)

         result.push_back('{');

         for (end = (ptr = t->table) + t->_capacity; ptr < end; ++ptr)
            {
            if (*ptr)
               {
               _node = *ptr;

               do {
                  sz = result.size();

                  keysz  = ((UStringRep*)_node->key)->size();
                  keyptr = ((UStringRep*)_node->key)->data();

                  (void) result.reserve(sz + keysz * 6);

                  presult = result.c_pointer(sz);

                  if (bcomma == false) bcomma = true;
                  else
                     {
                     ++sz;

                     *presult++ = ',';
                     }

                  pos = u_escape_encode((const unsigned char*)keyptr, keysz, presult, result.space(), true);

                  presult[pos] = ':';

                  result.size_adjust(sz + 1 + pos);

                  stringify(result, *((UValue*)_node->elem));

                  _next = _node->next;
                  }
               while ((_node = _next));
               }
            }

         result.push_back('}');
         }
      break;
      }
}

U_NO_EXPORT bool UValue::readValue(UTokenizer& tok, UValue* _value)
{
   U_TRACE(0, "UValue::readValue(%p,%p)", &tok, _value)

   tok.skipSpaces();

   bool result;
   const char* start = tok.getPointer();

   char c = tok.next();

   switch (c)
      {
      case '\0':
         {
         result              = true;
         _value->type_       = NULL_VALUE;
         _value->value.real_ = 0.0;
         }
      break;

      case 'n':
         {
         _value->type_       = NULL_VALUE;
         _value->value.real_ = 0.0;

         result = tok.skipToken(U_CONSTANT_TO_PARAM("ull"));
         }
      break;

      case 't':
      case 'f':
         {
         result = (c == 't' ? tok.skipToken(U_CONSTANT_TO_PARAM("rue"))
                            : tok.skipToken(U_CONSTANT_TO_PARAM("alse")));

         if (result)
            {
            _value->type_       = BOOLEAN_VALUE;
            _value->value.bool_ = (c == 't');
            }
         }
      break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '-':
         {
         bool breal;

         if ((result = tok.skipNumber(breal)))
            {
            if (breal)
               {
               _value->type_       = REAL_VALUE;
               _value->value.real_ = strtod(start, 0);

               U_INTERNAL_DUMP("_value.real_ = %.16g", _value->value.real_)
               }
            else if (c == '-')
               {
               _value->type_      = INT_VALUE;
               _value->value.int_ = strtol(start, 0, 10);

               U_INTERNAL_DUMP("_value.int_ = %d", _value->value.int_)
               }
            else
               {
               _value->type_       = UINT_VALUE;
               _value->value.uint_ = strtoul(start, 0, 10);

               U_INTERNAL_DUMP("_value.uint_ = %u", _value->value.uint_)
               }
            }
         }
      break;

      case '"':
         {
         const char* ptr  = tok.getPointer();
         const char* _end = tok.getEnd();
         const char* last = u_find_char(ptr, _end, '"');
         uint32_t sz      = (last < _end ? last - ptr : 0);

         U_INTERNAL_DUMP("sz = %u", sz)

         _value->type_ = STRING_VALUE;

         if (sz)
            {
            _value->value.ptr_ = U_NEW(UString(sz));

            result = UEscape::decode(ptr, sz, *(_value->getString()));
            }
         else
            {
            _value->value.ptr_ = U_NEW(UString); // NB: omit the () when invoking the default constructor...

            result = true;
            }

         if (last < _end) tok.setPointer(last+1);

         U_INTERNAL_DUMP("_value.ptr_ = %.*S", U_STRING_TO_TRACE(*(_value->getString())))
         }
      break;

      case '[':
         {
         uint32_t sz;

         _value->type_      = ARRAY_VALUE;
         _value->value.ptr_ = U_NEW(UVector<UValue*>);

         while (true)
            {
            tok.skipSpaces();

            c = tok.next();

            if (c == ']' ||
                c == '\0')
               {
               break;
               }

            if (c != ',') tok.back();

            UValue* item = U_NEW(UValue);

            if (readValue(tok, item) == false)
               {
               delete item;

               U_RETURN(false);
               }

            _value->getArray()->push(item);
            }

         result = true;

         sz = _value->getArray()->size();

         U_INTERNAL_DUMP("sz = %u", sz)

         if (sz) _value->getArray()->reserve(sz);
         }
      break;

      case '{':
         {
         UValue name;

         _value->type_      = OBJECT_VALUE;
         _value->value.ptr_ = U_NEW(UHashMap<UValue*>);

         while (true)
            {
            tok.skipSpaces();

            c = tok.next();

            if (c == '}' ||
                c == '\0')
               {
               break;
               }

            if (c != ',') tok.back();

            if (readValue(tok, &name) == false) U_RETURN(false);

            tok.skipSpaces();

            if (tok.next()      != ':' ||
                name.isString() == false)
               {
               U_RETURN(false);
               }

            UValue* item = U_NEW(UValue);

            if (readValue(tok, item) == false)
               {
               delete item;

               U_RETURN(false);
               }

            U_INTERNAL_ASSERT_EQUALS(name.type_, STRING_VALUE)

            UString* pstring = name.getString();

            _value->getObject()->insert(*pstring, item);

            delete pstring;
            }

         result = true;

         name.type_ = NULL_VALUE;

         U_DUMP("hash map size = %u", _value->getObject()->size())
         }
      break;

      default:
         result = false;
      break;
      }

   U_RETURN(result);
}

bool UValue::parse(const UString& document)
{
   U_TRACE(0, "UValue::parse(%.*S)", U_STRING_TO_TRACE(document))

   UTokenizer tok(document);

   tok.skipSpaces();

   char c = tok.current();

   if ((c == '['   ||
        c == '{' ) && 
       readValue(tok, this))
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

// TEMPLATE SPECIALIZATIONS

void UJsonTypeHandler<UStringRep>::toJSON(UValue& json)
{
   U_TRACE(0, "UJsonTypeHandler<UStringRep>::toJSON(%p)", &json)

   UStringRep* rep = (UStringRep*)pval;

   U_INTERNAL_DUMP("pval(%p) = %.*S", pval, U_STRING_TO_TRACE(*rep))

   // TODO: we need to check if this code is yet necessary...

   if (json.isObject())
      {
      U_DUMP("hash map size = %u", json.getObject()->size())

      U_ASSERT(json.getObject()->elem()->isNull() ||
               json.getObject()->elem()->isString())

      json.getObject()->replaceAfterFind(U_NEW(UValue(rep)));
      }
   else
      {
      U_INTERNAL_ASSERT_EQUALS(json.type_, NULL_VALUE)

      json.type_      = STRING_VALUE;
      json.value.ptr_ = U_NEW(UString(rep));
      }
}

void UJsonTypeHandler<UStringRep>::fromJSON(UValue& json)
{
   U_TRACE(0, "UJsonTypeHandler<UStringRep>::fromJSON(%p)", &json)

   U_ASSERT(json.isString())

   UStringRep* rep = json.getString()->rep;

   U_INTERNAL_DUMP("rep = %.*S", U_STRING_TO_TRACE(*rep))

   ((UStringRep*)pval)->fromValue(rep);

   U_INTERNAL_DUMP("pval(%p) = %.*S", pval, U_STRING_TO_TRACE(*(UStringRep*)pval))
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UValue::dump(bool reset) const
{
   *UObjectIO::os << "type_ " << type_;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UJsonTypeHandler_Base::dump(bool _reset) const
{
   *UObjectIO::os << "pval " << pval;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
