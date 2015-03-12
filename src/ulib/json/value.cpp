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
      case BOOLEAN_VALUE: value.bool_ = false;                break;
      case     INT_VALUE:
      case    UINT_VALUE: value.int_  = 0;                    break;
      case    NULL_VALUE:
      case    REAL_VALUE: value.real_ = 0.0;                  break;
      case  STRING_VALUE: value.ptr_  = UString::string_null; break;
      case   ARRAY_VALUE: value.ptr_  = 0;                    break;
      case  OBJECT_VALUE: value.ptr_  = 0;                    break;
      }

   reset();
}

UValue::UValue(const UString& _key, const UString& value_)
{
   U_TRACE_REGISTER_OBJECT(0, UValue, "%.*S,%.*S", U_STRING_TO_TRACE(_key), U_STRING_TO_TRACE(value_))

   UValue* child = U_NEW(UValue(value_));

   child->key = U_NEW(UString(_key));

   parent     =
   prev       =
   next       = 0;
   key        = 0;
   type_      = OBJECT_VALUE;
   value.ptr_ = 0;

   children.head =
   children.tail = child;
}

void UValue::reset()
{
   U_TRACE(0, "UValue::reset()")

   parent =
   prev   =
   next   = 0;
   key    = 0;

   children.head =
   children.tail = 0;
}

void UValue::clear()
{
   U_TRACE(0, "UValue::clear()")

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   U_INTERNAL_DUMP("parent = %p prev = %p next = %p key = %p", parent, prev, next, key)

   if (parent)
      {
      if (key)
         {
         U_INTERNAL_DUMP("key = %.*S", U_STRING_TO_TRACE(*key))

         delete key;
                key = 0;
         }

      if (prev) prev->next            = next;
      else      parent->children.head = next;

      if (next) next->prev            = prev;
      else      parent->children.tail = prev;

      parent =
      prev   =
      next   = 0;
      }

   switch (type_)
      {
      case STRING_VALUE:
         {
         U_INTERNAL_ASSERT_POINTER(value.ptr_)

         delete getString();

         type_ = NULL_VALUE;
         }
      break;

      case ARRAY_VALUE:
      case OBJECT_VALUE:
         {
         UValue* _next;

         U_INTERNAL_DUMP("children.head = %p", children.head)

         for (UValue* child = children.head; child; child = _next)
            {
            _next = child->next;

            U_INTERNAL_DUMP("_next = %p", _next)

            delete child;
            }

         children.head =
         children.tail = 0;
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
      case   ARRAY_VALUE:
      case  OBJECT_VALUE: result = (children.head == 0); break;
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
      case   ARRAY_VALUE: result = (other == NULL_VALUE && (children.head == 0)) ||
                                    other == ARRAY_VALUE;
      break;
      case  OBJECT_VALUE: result = (other == NULL_VALUE && (children.head == 0)) ||
                                    other == OBJECT_VALUE;
      break;
      }

   U_RETURN(result);
}

__pure UValue& UValue::operator[](uint32_t pos)
{
   U_TRACE(0, "UValue::operator[](%u)", pos)

   if (type_ == ARRAY_VALUE)
      {
      uint32_t i = 0;

      for (UValue* child = children.head; child; child = child->next)
         {
         if (i == pos) return *child;

         i++;
         }
      }

   return *this;
}

__pure UValue& UValue::operator[](const UString& _key)
{
   U_TRACE(0, "UValue::operator[](%.*S)", U_STRING_TO_TRACE(_key))

   if (type_ == OBJECT_VALUE)
      {
      for (UValue* child = children.head; child; child = child->next)
         {
         U_INTERNAL_ASSERT_POINTER(child->key)

         if (child->key->equal(_key)) return *child;
         }
      }

   return *this;
}

uint32_t UValue::getMemberNames(UVector<UString>& members) const
{
   U_TRACE(0, "UValue::getMemberNames(%p)", &members)

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)
   U_INTERNAL_ASSERT(type_ == OBJECT_VALUE || type_ == NULL_VALUE)

   uint32_t n = members.size(), _size = 0;

   if (type_ != NULL_VALUE)
      {
      for (UValue* child = children.head; child; child = child->next)
         {
         U_INTERNAL_ASSERT_POINTER(child->key)

         members.push(child->key->rep);
         }

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
                  case '.': n = last_nonzero - buffer + 2; // Truncate zeroes to save bytes in output, but keep one
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

         for (UValue* element = _value.children.head; element; element = element->next)
            {
            stringify(result, *element);

            if (element->next) result.push_back(',');
            }

         result.push_back(']');
         }
      break;

      case OBJECT_VALUE:
         {
         char* presult;
         const char* keyptr;
         bool bcomma = false;
         uint32_t pos, sz, keysz;

         result.push_back('{');

         for (UValue* member = _value.children.head; member; member = member->next)
            {
            sz = result.size();

            U_INTERNAL_ASSERT_POINTER(member->key)

            keysz  = member->key->size();
            keyptr = member->key->data();

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

            stringify(result, *member);
            }

         result.push_back('}');
         }
      break;
      }
}

void UValue::appendNode(UValue* parent, UValue* child)
{
   U_TRACE(0, "UValue::appendNode(%p,%p)", parent, child)

   U_INTERNAL_DUMP("child->parent = %p parent->children.head = %p parent->children.tail = %p",
                    child->parent,     parent->children.head,     parent->children.tail)

   child->parent = parent;
   child->prev   = parent->children.tail;
   child->next   = 0;

   if (parent->children.tail) parent->children.tail->next = child;
   else                       parent->children.head       = child;
                              parent->children.tail       = child;

   U_INTERNAL_DUMP("child->parent = %p parent->children.head = %p parent->children.tail = %p",
                    child->parent,     parent->children.head,     parent->children.tail)
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
         UValue* child;

         _value->type_ = ARRAY_VALUE;

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

            child = U_NEW(UValue);

            if (readValue(tok, child) == false)
               {
               delete child;

               U_RETURN(false);
               }

            appendNode(_value, child);
            }

         result = true;
         }
      break;

      case '{':
         {
         UValue name;
         UValue* child;

         _value->type_ = OBJECT_VALUE;

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

            child = U_NEW(UValue);

            if (readValue(tok, child) == false)
               {
               delete child;

               U_RETURN(false);
               }

            U_INTERNAL_ASSERT_EQUALS(name.type_, STRING_VALUE)

            child->key = name.getString();

            appendNode(_value, child);

            name.type_ = NULL_VALUE;
            }

         result = true;

         U_INTERNAL_ASSERT_EQUALS(name.type_, NULL_VALUE)
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

   U_INTERNAL_DUMP("type_ = %u", type_)

   U_RETURN(false);
}

// TEMPLATE SPECIALIZATIONS

void UJsonTypeHandler<UStringRep>::toJSON(UValue& json)
{
   U_TRACE(0, "UJsonTypeHandler<UStringRep>::toJSON(%p)", &json)

   UStringRep* rep = (UStringRep*)pval;

   U_INTERNAL_DUMP("pval(%p) = %.*S", pval, U_STRING_TO_TRACE(*rep))

   U_INTERNAL_ASSERT_EQUALS(json.type_, NULL_VALUE)

   json.type_      = STRING_VALUE;
   json.value.ptr_ = U_NEW(UString(rep));
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
const char* UValue::dump(bool _reset) const
{
   *UObjectIO::os << "key        " << key    << '\n'
                  << "prev       " << prev   << '\n'
                  << "next       " << next   << '\n'
                  << "parent     " << parent << '\n'
                  << "type_      " << type_  << '\n'
                  << "value.ptr_ " << value.ptr_;

   if (_reset)
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
