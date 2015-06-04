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

/**
 * typedef enum ValueType {
 *    NULL_VALUE =  0, // null value
 * BOOLEAN_VALUE =  1, // bool value
 *    CHAR_VALUE =  2, //   signed char value
 *   UCHAR_VALUE =  3, // unsigned char value
 *   SHORT_VALUE =  4, //   signed short integer value
 *  USHORT_VALUE =  5, // unsigned short integer value
 *     INT_VALUE =  6, //   signed integer value
 *    UINT_VALUE =  7, // unsigned integer value
 *    LONG_VALUE =  8, //   signed long value
 *   ULONG_VALUE =  9, // unsigned long value
 *   LLONG_VALUE = 10, //   signed long long value
 *  ULLONG_VALUE = 11, // unsigned long long value
 *   FLOAT_VALUE = 12, // float value
 *    REAL_VALUE = 13, // double value
 *   LREAL_VALUE = 14, // long double value
 *  STRING_VALUE = 15, // UTF-8 string value
 *   ARRAY_VALUE = 16, // array value (ordered list)
 *  OBJECT_VALUE = 17  // object value (collection of name/value pairs)
} ValueType;
*/

UValue::UValue(const UString& _key, const UString& value_)
{
   U_TRACE_REGISTER_OBJECT(0, UValue, "%V,%V", _key.rep, value_.rep)

   parent     =
   prev       =
   next       = 0;
   key        = 0;
   value.ptr_ = 0;
   type_      = OBJECT_VALUE;

   UValue* child = U_NEW(UValue(STRING_VALUE));

   children.head =
   children.tail = child;

   child->key        = U_NEW(UString(_key));
   child->value.ptr_ = U_NEW(UString(value_));
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
         U_INTERNAL_DUMP("key = %V", key->rep)

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

   if (type_ == STRING_VALUE)
      {
      U_INTERNAL_ASSERT_POINTER(value.ptr_)

      delete getString();

      type_ = NULL_VALUE;
      }
   else if (type_ ==  ARRAY_VALUE ||
            type_ == OBJECT_VALUE)
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
}

// CONVERSION

__pure bool UValue::asBool() const
{
   U_TRACE(0, "UValue::asBool()")

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   static const int dispatch_table[] = {
      0,
      (int)((char*)&&case_bool-(char*)&&case_null),
      (int)((char*)&&case_char-(char*)&&case_null),
      (int)((char*)&&case_uchar-(char*)&&case_null),
      (int)((char*)&&case_short-(char*)&&case_null),
      (int)((char*)&&case_ushort-(char*)&&case_null),
      (int)((char*)&&case_int-(char*)&&case_null),
      (int)((char*)&&case_uint-(char*)&&case_null),
      (int)((char*)&&case_long-(char*)&&case_null),
      (int)((char*)&&case_ulong-(char*)&&case_null),
      (int)((char*)&&case_llong-(char*)&&case_null),
      (int)((char*)&&case_ullong-(char*)&&case_null),
      (int)((char*)&&case_float-(char*)&&case_null),
      (int)((char*)&&case_double-(char*)&&case_null),
      (int)((char*)&&case_ldouble-(char*)&&case_null),
      (int)((char*)&&case_string-(char*)&&case_null),
      (int)((char*)&&case_array-(char*)&&case_null),
      (int)((char*)&&case_object-(char*)&&case_null)
   };

   U_INTERNAL_DUMP("dispatch_table[%d] = %p &&case_null = %p", type_, dispatch_table[type_], &&case_null)

   goto *((char*)&&case_null + dispatch_table[type_]);

case_null:     U_RETURN(false);
case_bool:     U_RETURN(value.bool_);
case_char:     U_RETURN(value.char_);
case_uchar:    U_RETURN(value.uchar_);
case_short:    U_RETURN(value.short_);
case_ushort:   U_RETURN(value.short_);
case_int:      U_RETURN(value.int_);
case_uint:     U_RETURN(value.uint_);
case_long:     U_RETURN(value.long_);
case_ulong:    U_RETURN(value.ulong_);
case_llong:    U_RETURN(value.llong_);
case_ullong:   U_RETURN(value.ullong_);
case_float:    U_RETURN(value.float_ != 0.0);
case_double:   U_RETURN(value.real_  != 0.0);
case_ldouble:  U_RETURN(value.lreal_ != 0.0);
case_string:   U_RETURN(getString()->empty() == false);
case_array:
case_object:   U_RETURN(children.head == 0);
}

__pure int UValue::asInt() const
{
   U_TRACE(0, "UValue::asInt()")

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   static const int dispatch_table[] = {
      0,
      (int)((char*)&&case_bool-(char*)&&case_null),
      (int)((char*)&&case_char-(char*)&&case_null),
      (int)((char*)&&case_uchar-(char*)&&case_null),
      (int)((char*)&&case_short-(char*)&&case_null),
      (int)((char*)&&case_ushort-(char*)&&case_null),
      (int)((char*)&&case_int-(char*)&&case_null),
      (int)((char*)&&case_uint-(char*)&&case_null),
      (int)((char*)&&case_long-(char*)&&case_null),
      (int)((char*)&&case_ulong-(char*)&&case_null),
      (int)((char*)&&case_llong-(char*)&&case_null),
      (int)((char*)&&case_ullong-(char*)&&case_null),
      (int)((char*)&&case_float-(char*)&&case_null),
      (int)((char*)&&case_double-(char*)&&case_null),
      (int)((char*)&&case_ldouble-(char*)&&case_null),
      (int)((char*)&&case_string-(char*)&&case_null),
      (int)((char*)&&case_array-(char*)&&case_null),
      (int)((char*)&&case_object-(char*)&&case_null)
   };

   U_INTERNAL_DUMP("dispatch_table[%d] = %p &&case_null = %p", type_, dispatch_table[type_], &&case_null)

   goto *((char*)&&case_null + dispatch_table[type_]);

case_null:     U_RETURN(0);
case_bool:     U_RETURN(value.bool_);
case_char:     U_RETURN(value.char_);
case_uchar:    U_RETURN(value.uchar_);
case_short:    U_RETURN(value.short_);
case_ushort:   U_RETURN(value.short_);
case_int:      U_RETURN(value.int_);
case_uint:     U_RETURN(value.uint_);
case_long:     U_RETURN(value.long_);
case_ulong:    U_RETURN(value.ulong_);
case_llong:    U_RETURN(value.llong_);
case_ullong:   U_RETURN(value.ullong_);

case_float:
   U_INTERNAL_ASSERT_MSG(value.float_ >= INT_MIN && value.float_ <= INT_MAX, "float out of signed integer range")

   U_RETURN(value.float_);

case_double:
   U_INTERNAL_ASSERT_MSG(value.real_ >= INT_MIN && value.real_ <= INT_MAX, "double out of signed integer range")

   U_RETURN(value.real_);
case_ldouble:
   U_INTERNAL_ASSERT_MSG(value.lreal_ >= INT_MIN && value.lreal_ <= INT_MAX, "long double out of signed integer range")

   U_RETURN(value.lreal_);

case_string:
case_array:
case_object:
   U_INTERNAL_ASSERT_MSG(false, "Type is not convertible to signed integer...")

   U_RETURN(-1);
}

__pure unsigned int UValue::asUInt() const
{
   U_TRACE(0, "UValue::asUInt()")

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   static const int dispatch_table[] = {
      0,
      (int)((char*)&&case_bool-(char*)&&case_null),
      (int)((char*)&&case_char-(char*)&&case_null),
      (int)((char*)&&case_uchar-(char*)&&case_null),
      (int)((char*)&&case_short-(char*)&&case_null),
      (int)((char*)&&case_ushort-(char*)&&case_null),
      (int)((char*)&&case_int-(char*)&&case_null),
      (int)((char*)&&case_uint-(char*)&&case_null),
      (int)((char*)&&case_long-(char*)&&case_null),
      (int)((char*)&&case_ulong-(char*)&&case_null),
      (int)((char*)&&case_llong-(char*)&&case_null),
      (int)((char*)&&case_ullong-(char*)&&case_null),
      (int)((char*)&&case_float-(char*)&&case_null),
      (int)((char*)&&case_double-(char*)&&case_null),
      (int)((char*)&&case_ldouble-(char*)&&case_null),
      (int)((char*)&&case_string-(char*)&&case_null),
      (int)((char*)&&case_array-(char*)&&case_null),
      (int)((char*)&&case_object-(char*)&&case_null)
   };

   U_INTERNAL_DUMP("dispatch_table[%d] = %p &&case_null = %p", type_, dispatch_table[type_], &&case_null)

   goto *((char*)&&case_null + dispatch_table[type_]);

case_null:     U_RETURN(0);
case_bool:     U_RETURN(value.bool_);
case_char:     U_RETURN(value.char_);
case_uchar:    U_RETURN(value.uchar_);
case_short:    U_RETURN(value.short_);
case_ushort:   U_RETURN(value.short_);

case_int:
   U_INTERNAL_ASSERT_MSG(value.int_ >= 0, "Negative integer cannot be converted to unsigned integer")

   U_RETURN(value.int_);

case_uint:     U_RETURN(value.uint_);
case_long:     U_RETURN(value.long_);
case_ulong:    U_RETURN(value.ulong_);
case_llong:    U_RETURN(value.llong_);
case_ullong:   U_RETURN(value.ullong_);

case_float:
   U_INTERNAL_ASSERT_MSG(value.float_ >= 0.0 && value.float_ <= UINT_MAX, "float out of unsigned integer range")

   U_RETURN(value.float_);

case_double:
   U_INTERNAL_ASSERT_MSG(value.real_ >= 0.0 && value.real_ <= UINT_MAX, "double out of unsigned integer range")

   U_RETURN(value.real_);
case_ldouble:
   U_INTERNAL_ASSERT_MSG(value.lreal_ >= 0.0 && value.lreal_ <= UINT_MAX, "long double out of unsigned integer range")

   U_RETURN(value.lreal_);

case_string:
case_array:
case_object:
   U_INTERNAL_ASSERT_MSG(false, "Type is not convertible to signed integer...")

   U_RETURN(-1);
}

__pure double UValue::asDouble() const
{
   U_TRACE(0, "UValue::asDouble()")

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   static const int dispatch_table[] = {
      0,
      (int)((char*)&&case_bool-(char*)&&case_null),
      (int)((char*)&&case_char-(char*)&&case_null),
      (int)((char*)&&case_uchar-(char*)&&case_null),
      (int)((char*)&&case_short-(char*)&&case_null),
      (int)((char*)&&case_ushort-(char*)&&case_null),
      (int)((char*)&&case_int-(char*)&&case_null),
      (int)((char*)&&case_uint-(char*)&&case_null),
      (int)((char*)&&case_long-(char*)&&case_null),
      (int)((char*)&&case_ulong-(char*)&&case_null),
      (int)((char*)&&case_llong-(char*)&&case_null),
      (int)((char*)&&case_ullong-(char*)&&case_null),
      (int)((char*)&&case_float-(char*)&&case_null),
      (int)((char*)&&case_double-(char*)&&case_null),
      (int)((char*)&&case_ldouble-(char*)&&case_null),
      (int)((char*)&&case_string-(char*)&&case_null),
      (int)((char*)&&case_array-(char*)&&case_null),
      (int)((char*)&&case_object-(char*)&&case_null)
   };

   U_INTERNAL_DUMP("dispatch_table[%d] = %p &&case_null = %p", type_, dispatch_table[type_], &&case_null)

   goto *((char*)&&case_null + dispatch_table[type_]);

case_null:     U_RETURN(0.0);
case_bool:     U_RETURN(value.bool_ ? 1.0 : 0.0);
case_char:     U_RETURN(value.char_);
case_uchar:    U_RETURN(value.uchar_);
case_short:    U_RETURN(value.short_);
case_ushort:   U_RETURN(value.short_);
case_int:      U_RETURN(value.int_);
case_uint:     U_RETURN(value.uint_);
case_long:     U_RETURN(value.long_);
case_ulong:    U_RETURN(value.ulong_);
case_llong:    U_RETURN(value.llong_);
case_ullong:   U_RETURN(value.ullong_);
case_float:    U_RETURN(value.float_);
case_double:   U_RETURN(value.real_);
case_ldouble:  U_RETURN(value.lreal_);

case_string:
case_array:
case_object:
   U_INTERNAL_ASSERT_MSG(false, "Type is not convertible to signed integer...")

   U_RETURN(0.0);
}

__pure UString UValue::asString() const
{
   U_TRACE(0, "UValue::asString()")

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   static const int dispatch_table[] = {
      0,
      (int)((char*)&&case_bool-(char*)&&case_null),
      (int)((char*)&&case_char-(char*)&&case_null),
      (int)((char*)&&case_uchar-(char*)&&case_null),
      (int)((char*)&&case_short-(char*)&&case_null),
      (int)((char*)&&case_ushort-(char*)&&case_null),
      (int)((char*)&&case_int-(char*)&&case_null),
      (int)((char*)&&case_uint-(char*)&&case_null),
      (int)((char*)&&case_long-(char*)&&case_null),
      (int)((char*)&&case_ulong-(char*)&&case_null),
      (int)((char*)&&case_llong-(char*)&&case_null),
      (int)((char*)&&case_ullong-(char*)&&case_null),
      (int)((char*)&&case_float-(char*)&&case_null),
      (int)((char*)&&case_double-(char*)&&case_null),
      (int)((char*)&&case_ldouble-(char*)&&case_null),
      (int)((char*)&&case_string-(char*)&&case_null),
      (int)((char*)&&case_array-(char*)&&case_null),
      (int)((char*)&&case_object-(char*)&&case_null)
   };

   U_INTERNAL_DUMP("dispatch_table[%d] = %p &&case_null = %p", type_, dispatch_table[type_], &&case_null)

   goto *((char*)&&case_null + dispatch_table[type_]);

case_null:  return UString::getStringNull();
case_bool:  U_RETURN_STRING(value.bool_ ? *UString::str_true : *UString::str_false);

case_char:
case_uchar:
case_short:
case_ushort:
case_int:
case_uint:
case_long:
case_ulong:
case_llong:
case_ullong:
case_float:
case_double:
case_ldouble:
   U_INTERNAL_ASSERT_MSG(false, "Type is not convertible to string...")

   return UString::getStringNull();

case_string: U_RETURN_STRING(*getString());

case_array:
case_object:
   U_INTERNAL_ASSERT_MSG(false, "Type is not convertible to string...")

   return UString::getStringNull();
}

__pure bool UValue::isConvertibleTo(ValueType other) const
{
   U_TRACE(0, "UValue::isConvertibleTo(%d)", other)

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   static const int dispatch_table[] = {
      0,
      (int)((char*)&&case_bool-(char*)&&case_null),
      (int)((char*)&&case_char-(char*)&&case_null),
      (int)((char*)&&case_uchar-(char*)&&case_null),
      (int)((char*)&&case_short-(char*)&&case_null),
      (int)((char*)&&case_ushort-(char*)&&case_null),
      (int)((char*)&&case_int-(char*)&&case_null),
      (int)((char*)&&case_uint-(char*)&&case_null),
      (int)((char*)&&case_long-(char*)&&case_null),
      (int)((char*)&&case_ulong-(char*)&&case_null),
      (int)((char*)&&case_llong-(char*)&&case_null),
      (int)((char*)&&case_ullong-(char*)&&case_null),
      (int)((char*)&&case_float-(char*)&&case_null),
      (int)((char*)&&case_double-(char*)&&case_null),
      (int)((char*)&&case_ldouble-(char*)&&case_null),
      (int)((char*)&&case_string-(char*)&&case_null),
      (int)((char*)&&case_array-(char*)&&case_null),
      (int)((char*)&&case_object-(char*)&&case_null)
   };

   U_INTERNAL_DUMP("dispatch_table[%d] = %p &&case_null = %p", type_, dispatch_table[type_], &&case_null)

   goto *((char*)&&case_null + dispatch_table[type_]);

case_null:     U_RETURN(true);
case_bool:     U_RETURN(other != NULL_VALUE || value.bool_   == false);
case_char:     U_RETURN(other != NULL_VALUE || value.char_   == 0);
case_uchar:    U_RETURN(other != NULL_VALUE || value.uchar_  == 0);
case_short:    U_RETURN(other != NULL_VALUE || value.short_  == 0);
case_ushort:   U_RETURN(other != NULL_VALUE || value.ushort_ == 0);

case_int:      U_RETURN((other == NULL_VALUE && value.int_ == 0) ||
                         other ==  INT_VALUE                     ||
                        (other == UINT_VALUE && value.int_ >= 0) ||
                         other == REAL_VALUE                     ||
                         other == BOOLEAN_VALUE);

case_uint:     U_RETURN((other == NULL_VALUE && value.uint_ ==       0) ||
                        (other ==  INT_VALUE && value.uint_ <= INT_MAX) ||
                         other == UINT_VALUE                            ||
                         other == REAL_VALUE                            ||
                         other == BOOLEAN_VALUE);

case_long:     U_RETURN(false);
case_ulong:    U_RETURN(false);
case_llong:    U_RETURN(false);
case_ullong:   U_RETURN(false);
case_float:    U_RETURN(false);

case_double:   U_RETURN((other == NULL_VALUE && value.real_ ==     0.0)                            ||
                        (other ==  INT_VALUE && value.real_ >= INT_MIN && value.real_ <=  INT_MAX) ||
                        (other == UINT_VALUE && value.real_ >=     0.0 && value.real_ <= UINT_MAX) ||
                         other == REAL_VALUE                                                       ||
                         other == BOOLEAN_VALUE);

case_ldouble:  U_RETURN((other == NULL_VALUE && value.lreal_ ==     0.0)                             ||
                        (other ==  INT_VALUE && value.lreal_ >= INT_MIN && value.lreal_ <=  INT_MAX) ||
                        (other == UINT_VALUE && value.lreal_ >=     0.0 && value.lreal_ <= UINT_MAX) ||
                         other == REAL_VALUE                                                         ||
                         other == BOOLEAN_VALUE);

case_string:   U_RETURN((other == NULL_VALUE && getString()->empty()) || other == STRING_VALUE); 
case_array:    U_RETURN((other == NULL_VALUE && (children.head == 0)) || other ==  ARRAY_VALUE);
case_object:   U_RETURN((other == NULL_VALUE && (children.head == 0)) || other == OBJECT_VALUE);
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
   U_TRACE(0, "UValue::operator[](%V)", _key.rep)

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

uint32_t UValue::emitString(const unsigned char* inptr, uint32_t len, char* out)
{
   U_TRACE(0, "UValue::emitString(%.*S,%u,%p)", len, inptr, len, out)

   U_INTERNAL_ASSERT_POINTER(out)
   U_INTERNAL_ASSERT_POINTER(inptr)

   const unsigned char* restrict inend  = inptr + len;
                  char* restrict outptr = out;

   *outptr++ = '"';

   while (inptr < inend)
      {
      unsigned char c = *inptr++;

      U_INTERNAL_DUMP("c = %u", c)

      if (c < 0x1F)
         {
                             *outptr++ = '\\';
              if (c == '\b') *outptr++ = 'b'; // 0x08
         else if (c == '\t') *outptr++ = 't'; // 0x09
         else if (c == '\n') *outptr++ = 'n'; // 0x0A
         else if (c == '\f') *outptr++ = 'f'; // 0x0C
         else if (c == '\r') *outptr++ = 'r'; // 0x0D

         else // \u four hex digits (unicode char)
            {
            *outptr++ = 'u';
            *outptr++ = '0';
            *outptr++ = '0';
            *outptr++ = u_hex_upper[((c >> 4) & 0x0F)];
            *outptr++ = u_hex_upper[( c       & 0x0F)];
            }

         continue;
         }

      /**
       * This syntax is given in RFC3629, which is the same as that given in The Unicode Standard, Version 6.0. It has the following properties:
       *
       * All codepoints U+0000..U+10FFFF may be encoded, except for U+D800..U+DFFF, which are reserved for UTF-16 surrogate pair encoding.
       * UTF-8 byte sequences longer than 4 bytes are not permitted, as they exceed the range of Unicode.
       * The sixty-six Unicode "non-characters" are permitted (namely, U+FDD0..U+FDEF, U+xxFFFE, and U+xxFFFF)
       */

      if (c <= 0x7F) // 00..7F
         {
         if (c == '"' || // 0x22
             c == '\\')  // 0x5C
            {
            *outptr++ = '\\';
            }

         *outptr++ = c;

         continue;
         }

      if (c <= 0xC1) continue; // 80..C1 - Disallow overlong 2-byte sequence

      if (c <= 0xDF) // C2..DF
         {
         if ((inptr[0] & 0xC0) != 0x80) // Make sure subsequent byte is in the range 0x80..0xBF
            {
            *outptr++ = c;
            *outptr++ = *inptr++;
            }

         continue;
         }

      if (c <= 0xEF) // E0..EF
         {
         if ((c != 0xE0 || inptr[0] >= 0xA0) && // Disallow overlong 3-byte sequence
             (c != 0xED || inptr[0] <= 0x9F) && // Disallow U+D800..U+DFFF
             (inptr[0] & 0xC0) == 0x80       && // Make sure subsequent bytes are in the range 0x80..0xBF
             (inptr[1] & 0xC0) == 0x80)
            {
            *outptr++ = c;
            *outptr++ = inptr[0];
            *outptr++ = inptr[1];

            inptr += 2;
            }

         continue;
         }

      if (c <= 0xF4) // F0..F4
         {
         if ((c != 0xF0 || inptr[0] >= 0x90) && // Disallow overlong 4-byte sequence
             (c != 0xF4 || inptr[0] <= 0x8F) && // Disallow codepoints beyond U+10FFFF
             (inptr[0] & 0xC0) == 0x80       && // Make sure subsequent bytes are in the range 0x80..0xBF
             (inptr[1] & 0xC0) == 0x80       &&
             (inptr[2] & 0xC0) == 0x80)
            {
            *outptr++ = c;
            *outptr++ = inptr[0];
            *outptr++ = inptr[1];
            *outptr++ = inptr[2];

            inptr += 3;
            }
         }

      // F5..FF
      }

   *outptr++ = '"';

   U_RETURN(outptr - out);
}

void UValue::stringify(UString& result, UValue& _value)
{
   U_TRACE(0, "UValue::stringify(%V,%p)", result.rep, &_value)

   U_INTERNAL_DUMP("_value.type_ = %u", _value.type_)

   U_INTERNAL_ASSERT_RANGE(0,_value.type_,OBJECT_VALUE)

   static const int dispatch_table[] = {
      0,
      (int)((char*)&&case_bool-(char*)&&case_null),
      (int)((char*)&&case_char-(char*)&&case_null),
      (int)((char*)&&case_uchar-(char*)&&case_null),
      (int)((char*)&&case_short-(char*)&&case_null),
      (int)((char*)&&case_ushort-(char*)&&case_null),
      (int)((char*)&&case_int-(char*)&&case_null),
      (int)((char*)&&case_uint-(char*)&&case_null),
      (int)((char*)&&case_long-(char*)&&case_null),
      (int)((char*)&&case_ulong-(char*)&&case_null),
      (int)((char*)&&case_llong-(char*)&&case_null),
      (int)((char*)&&case_ullong-(char*)&&case_null),
      (int)((char*)&&case_float-(char*)&&case_null),
      (int)((char*)&&case_double-(char*)&&case_null),
      (int)((char*)&&case_ldouble-(char*)&&case_null),
      (int)((char*)&&case_string-(char*)&&case_null),
      (int)((char*)&&case_array-(char*)&&case_null),
      (int)((char*)&&case_object-(char*)&&case_null)
   };

   bool bcomma;
   char* presult;
   const char* ch;
   char buffer[32];
   UString* pstring;
   const char* last_nonzero;
   uint32_t n, pos, sz, keysz;

   U_INTERNAL_DUMP("dispatch_table[%d] = %p &&case_null = %p", _value.type_, dispatch_table[_value.type_], &&case_null)

   goto *((char*)&&case_null + dispatch_table[_value.type_]);

case_null:
   (void) result.append(U_CONSTANT_TO_PARAM("null"));

   return;

case_bool:
   _value.value.bool_ ? (void) result.append(U_CONSTANT_TO_PARAM("true"))
                      : (void) result.append(U_CONSTANT_TO_PARAM("false"));

   return;

case_char:
   (void) result.push_back(_value.value.char_);

   return;

case_uchar:
   (void) result.push_back(_value.value.uchar_);

   return;

case_short:
   (void) result.append(buffer, u_num2str32s(buffer, _value.value.short_));

   return;

case_ushort:
   (void) result.append(buffer, u_num2str32(buffer, _value.value.ushort_));

   return;

case_int:
   (void) result.append(buffer, u_num2str32s(buffer, _value.value.int_));

   return;

case_uint:
   (void) result.append(buffer, u_num2str32s(buffer, _value.value.uint_));

   return;

case_long:
   (void) result.append(buffer, u_num2str64s(buffer, _value.value.long_));

   return;

case_ulong:
   (void) result.append(buffer, u_num2str64s(buffer, _value.value.ulong_));

   return;

case_llong:
   (void) result.append(buffer, u_num2str64s(buffer, _value.value.llong_));

   return;

case_ullong:
   (void) result.append(buffer, u_num2str64s(buffer, _value.value.ullong_));

   return;

case_float:
   n = u__snprintf(buffer, sizeof(buffer), "%#.6f", _value.value.float_);

   goto next;

case_double:
   n = u__snprintf(buffer, sizeof(buffer), "%#.16g", _value.value.real_);

   goto next;

case_ldouble:
   n = u__snprintf(buffer, sizeof(buffer), "%#.16g", _value.value.lreal_);

next:
   ch = buffer + n - 1;

   if (*ch == '0')
      {
      while (ch > buffer && *ch == '0') --ch;

      last_nonzero = ch;

      while (ch >= buffer)
         {
         char c = *ch;

         if (u__isdigit(c))
            {
            --ch;

            continue; 
            }

         if (c == '.') n = last_nonzero - buffer + 2; // Truncate zeroes to save bytes in output, but keep one

         break;
         }
      }

   (void) result.append(buffer, n);

   return;

case_string:
   pstring = _value.getString();

   (void) result.reserve((sz = result.size()) + (keysz = pstring->size()) * 6);

   presult = result.c_pointer(sz);

   result.rep->_length = sz + emitString((const unsigned char*)pstring->data(), keysz, presult);

   return;

case_array:
   result.push_back('[');

   for (UValue* element = _value.children.head; element; element = element->next)
      {
      stringify(result, *element);

      if (element->next) result.push_back(',');
      }

   result.push_back(']');

   return;

case_object:
   bcomma = false;

   result.push_back('{');

   for (UValue* member = _value.children.head; member; member = member->next)
      {
      U_INTERNAL_ASSERT_POINTER(member->key)

      (void) result.reserve((sz = result.size()) + (keysz = member->key->size()) * 6);

      presult = result.c_pointer(sz);

      if (bcomma == false) bcomma = true;
      else
         {
         ++sz;

         *presult++ = ',';
         }

      pos = emitString((const unsigned char*)member->key->data(), keysz, presult);

      presult[pos] = ':';

      result.rep->_length = sz + pos + 1;

      stringify(result, *member);
      }

   result.push_back('}');
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

            UEscape::decode(ptr, sz, *(_value->getString()));

            result = ((_value->getString())->empty() == false);
            }
         else
            {
            _value->value.ptr_ = U_NEW(UString); // NB: omit the () when invoking the default constructor...

            result = true;
            }

         if (last < _end) tok.setPointer(last+1);

         U_INTERNAL_DUMP("_value.ptr_ = %V", _value->getString()->rep)
         }
      break;

      case '[':
         {
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

            UValue* child = U_NEW(UValue);

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

            UValue* child = U_NEW(UValue);

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
   U_TRACE(0, "UValue::parse(%V)", document.rep)

   UTokenizer tok(document);

   tok.skipSpaces();

   char c = tok.current();

   if ((c == '['   ||
        c == '{' ) && 
       readValue(tok, this))
      {
      U_INTERNAL_ASSERT(invariant())

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

   U_INTERNAL_DUMP("pval(%p) = %V", pval, rep)

   U_INTERNAL_ASSERT_EQUALS(json.type_, NULL_VALUE)

   json.type_      = STRING_VALUE;
   json.value.ptr_ = U_NEW(UString(rep));
}

void UJsonTypeHandler<UStringRep>::fromJSON(UValue& json)
{
   U_TRACE(0, "UJsonTypeHandler<UStringRep>::fromJSON(%p)", &json)

   U_ASSERT(json.isString())

   UStringRep* rep = json.getString()->rep;

   U_INTERNAL_DUMP("rep = %V", rep)

   ((UStringRep*)pval)->fromValue(rep);

   U_INTERNAL_DUMP("pval(%p) = %V", pval, pval)
}

// DEBUG

#ifdef DEBUG
const char* UValue::dump(bool _reset) const
{
#ifdef U_STDCPP_ENABLE
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
#endif

   return 0;
}

const char* UJsonTypeHandler_Base::dump(bool _reset) const
{
#ifdef U_STDCPP_ENABLE
   *UObjectIO::os << "pval " << pval;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }
#endif

   return 0;
}

bool UValue::invariant() const
{
   if ( key &&
       *key &&
        key->isUTF8(0) == false)
      {
      U_WARNING("error on value: (key contains invalid UTF-8)\n"
                "--------------------------------------------------\n%s", dump(true));

      return false;
      }

   if (type_ < 0 ||
       type_ > OBJECT_VALUE)
      {
      U_WARNING("error on value: (tag is invalid)\n"
                "--------------------------------------------------\n%s", dump(true));

      return false;
      }

   if (type_ == BOOLEAN_VALUE &&
       value.bool_ != false   &&
       value.bool_ != true)
      {
      U_WARNING("error on value: (bool_ is neither false nor true)\n"
                "--------------------------------------------------\n%s", dump(true));

      return false;
      }

   if (type_ == STRING_VALUE)
      {
      UString* str = (UString*)value.ptr_;

      if (str == 0)
         {
         U_WARNING("error on value: (string is NULL)\n"
                   "--------------------------------------------------\n%s", dump(true));

         return false;
         }

      if (*str &&
           str->isUTF8(0) == false)
         {
         U_WARNING("error on value: (string contains invalid UTF-8)\n"
                   "--------------------------------------------------\n%s", dump(true));

         return false;
         }
      }
   else if (type_ ==  ARRAY_VALUE ||
            type_ == OBJECT_VALUE)
      {
      if (children.head == 0 ||
          children.tail == 0)
         {
         if (children.head)
            {
            U_WARNING("error on value: (tail is NULL, but head is not)\n"
                      "--------------------------------------------------\n%s", dump(true));

            return false;
            }

         if (children.tail)
            {
            U_WARNING("error on value: (head is NULL, but tail is not)\n"
                      "--------------------------------------------------\n%s", dump(true));

            return false;
            }
         }
      else
         {
         if (children.head->prev)
            {
            U_WARNING("error on value: (First child's prev pointer is not NULL)\n"
                      "--------------------------------------------------\n%s", dump(true));

            return false;
            }

         UValue* last = 0;

         for (UValue* child = children.head; child; last = child, child = child->next)
            {
            if (child == this)
               {
               U_WARNING("error on value: (it is its own child)\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (child->next == child)
               {
               U_WARNING("error on value: (child->next == child (cycle))\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (child->next == children.head)
               {
               U_WARNING("error on value: (child->next == head (cycle))\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (child->parent != this)
               {
               U_WARNING("error on value: (child does not point back to parent)\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (child->next &&
                child->next->prev != child)
               {
               U_WARNING("error on value: (child->next does not point back to child)\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (type_ == ARRAY_VALUE && child->key)
               {
               U_WARNING("error on value: (Array element's key is not NULL)\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (type_ == OBJECT_VALUE && child->key == 0)
               {
               U_WARNING("error on value: (Object member's key is NULL)\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (child->invariant() == false) return false;
            }

         if (last != children.tail)
            {
            U_WARNING("error on value: (tail does not match pointer found by starting at head and following next links)\n"
                      "--------------------------------------------------\n%s", dump(true));

            return false;
            }
         }
      }

   return true;
}
#endif
