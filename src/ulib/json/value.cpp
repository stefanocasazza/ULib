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
 *   FLOAT_VALUE = 12, //       float value
 *    REAL_VALUE = 13, //      double value
 *   LREAL_VALUE = 14, // long double value
 *  STRING_VALUE = 15, // UTF-8 string value
 *   ARRAY_VALUE = 16, // array value (ordered list)
 *  OBJECT_VALUE = 17, // object value (collection of name/value pairs)
 *  NUMBER_VALUE = 18  // generic number value (may be -ve) int or float
} ValueType;
*/

char*       UValue::pstringify;
UTokenizer* UValue::ptok;

UValue::UValue(const UString& _key, const UString& value_)
{
   U_TRACE_REGISTER_OBJECT(0, UValue, "%V,%V", _key.rep, value_.rep)

   parent     =
   prev       =
   next       = 0;
   key        = 0;
   value.ptr_ = 0;
   type_      = OBJECT_VALUE;

   UValue* child;

   U_NEW(UValue, child, UValue(STRING_VALUE));

   children.head =
   children.tail = child;

   U_NEW(UString, child->key,        UString(_key));
   U_NEW(UString, child->value.ptr_, UString(value_));

   U_INTERNAL_DUMP("this = %p", this)
}

void UValue::reset()
{
   U_TRACE_NO_PARAM(0, "UValue::reset()")

   parent =
   prev   =
   next   = 0;
   key    = 0;

   children.head =
   children.tail = 0;

   size = 0;

   U_INTERNAL_DUMP("this = %p", this)
}

void UValue::clear()
{
   U_TRACE_NO_PARAM(0, "UValue::clear()")

   U_INTERNAL_ASSERT_RANGE(0,type_,OBJECT_VALUE)

   U_INTERNAL_DUMP("this = %p parent = %p prev = %p next = %p key = %p", this, parent, prev, next, key)

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

      delete (UString*)value.ptr_;

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

#ifdef U_COMPILER_RVALUE_REFS
UValue& UValue::operator=(UValue && v)
{
   U_TRACE_NO_PARAM(0, "UValue::operator=(move)")

   UValue*        tmp  = parent;
   UString*       tmpk = key;
   union anyvalue tmpv = value;
   int            tmpi = type_;

     parent = v.parent;
   v.parent = tmp;

   tmp = prev;

     prev = v.prev;
   v.prev = tmp;

   tmp = next;

     next = v.next;
   v.next = tmp;

     key = v.key;
   v.key = tmpk;

   tmp = children.head;

     children.head = v.children.head;
   v.children.head = tmp;

   tmp = children.tail;

     children.tail = v.children.tail;
   v.children.tail = tmp;

     value = v.value;
   v.value = tmpv;

     type_ = v.type_;
   v.type_ = tmpi;

   return *this;
}

# if (!defined(__GNUC__) || GCC_VERSION_NUM > 50300) // GCC has problems dealing with move constructor, so turn the feature on for 5.3.1 and above, only
UValue::UValue(UValue && v)
{
   U_TRACE_NO_PARAM(0, "UValue::UValue(move)")

          parent = v.parent;
            prev = v.prev;
            next = v.next;
            key  = v.key;
   children.head = v.children.head;
   children.tail = v.children.tail;
           type_ = v.type_;
           value = v.value;

   v.reset();

   v.type_       = NULL_VALUE;
   v.value.real_ = .0;

   U_ASSERT(  invariant())
   U_ASSERT(v.invariant())
}
# endif
#endif

void UValue::set(const UValue& v)
{
   U_TRACE(0, "UValue::set(%p)", &v)

   U_INTERNAL_ASSERT_RANGE(0,v.type_,OBJECT_VALUE)

   parent   = v.parent;
   prev     = v.prev;
   next     = v.next;
   key      = v.key;

   type_ = v.type_;
   value = v.value;

   children.head =
   children.tail = 0;

   if (type_ == STRING_VALUE)
      {
      U_INTERNAL_ASSERT_POINTER(v.value.ptr_)

      U_NEW(UString, value.ptr_, UString(v.getString()));
      }
   else if (type_ ==  ARRAY_VALUE ||
            type_ == OBJECT_VALUE)
      {
      U_INTERNAL_DUMP("v.parent = %p v.prev = %p v.next = %p v.key = %p", v.parent, v.prev, v.next, v.key)

      if (v.key)
         {
         U_INTERNAL_DUMP("v.key = %V", v.key->rep)

         U_NEW(UString, key, UString(*(v.key)));
         }

      for (UValue* child = v.children.head; child; child = child->next)
         {
         UValue* _child;

         U_NEW(UValue, _child, UValue(*child));

         appendNode(_child);
         }
      }

   U_ASSERT(invariant())
}

__pure bool UValue::operator==(const UValue& v) const
{
   U_TRACE(0+256, "UValue::operator==(%p)", &v)

   U_INTERNAL_DUMP("  type_ = %d   parent = %p   prev = %p   next = %p   key = %p",   type_,   parent,   prev,   next,   key)
   U_INTERNAL_DUMP("v.type_ = %d v.parent = %p v.prev = %p v.next = %p v.key = %p", v.type_, v.parent, v.prev, v.next, v.key)

   U_INTERNAL_ASSERT_RANGE(0,  type_,OBJECT_VALUE)
   U_INTERNAL_ASSERT_RANGE(0,v.type_,OBJECT_VALUE)

   if (type_ != v.type_) U_RETURN(false);

   if (type_ == NULL_VALUE) U_RETURN(true); // Both null types

   if (type_ == BOOLEAN_VALUE)
      {
      if (value.bool_ == v.value.bool_) U_RETURN(true);

      U_RETURN(false);
      }

   if (type_ == STRING_VALUE)
      {
      if (getString() == v.getString()) U_RETURN(true);

      U_RETURN(false);
      }

   if (type_ ==  ARRAY_VALUE ||
       type_ == OBJECT_VALUE)
      {
      if (  parent &&
          v.parent)
         {
         if ( key && !v.key) U_RETURN(false);
         if (!key &&  v.key) U_RETURN(false);
         if ( key &&  v.key)
            {
            U_INTERNAL_DUMP("key = %V v.key = %V", key->rep, v.key->rep)

            if (*key != *v.key) U_RETURN(false);
            }
         }

      U_INTERNAL_DUMP("  children.head = %p   children.tail = %p",   children.head,   children.tail)
      U_INTERNAL_DUMP("v.children.head = %p v.children.tail = %p", v.children.head, v.children.tail)

      if ( children.head && !v.children.head) U_RETURN(false);
      if (!children.head &&  v.children.head) U_RETURN(false);
      if ( children.tail && !v.children.tail) U_RETURN(false);
      if (!children.tail &&  v.children.tail) U_RETURN(false);

      UValue* child1 =   children.head;
      UValue* child2 = v.children.head;

      for (; child1 && child2; child1 = child1->next, child2 = child2->next)
         {
         if (*child1 != *child2) U_RETURN(false);
         }

      if ( child1 && !child2) U_RETURN(false);
      if (!child1 &&  child2) U_RETURN(false);

      U_RETURN(true);
      }

   if (isNumeric())
      {
      if (asDouble() == v.asDouble()) U_RETURN(true);
      }

   U_RETURN(false);
}

// CONVERSION

__pure bool UValue::asBool() const
{
   U_TRACE_NO_PARAM(0, "UValue::asBool()")

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
case_string:   U_RETURN(getString().empty() == false);
case_array:
case_object:   U_RETURN(children.head == 0);
}

__pure int UValue::asInt() const
{
   U_TRACE_NO_PARAM(0, "UValue::asInt()")

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
   U_TRACE_NO_PARAM(0, "UValue::asUInt()")

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
   U_TRACE_NO_PARAM(0, "UValue::asDouble()")

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
   U_INTERNAL_ASSERT_MSG(false, "Type is not convertible to double number...")

   U_RETURN(0.0);
}

__pure UString UValue::asString() const
{
   U_TRACE_NO_PARAM(0, "UValue::asString()")

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

case_string: U_RETURN_STRING(getString());

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

case_string:   U_RETURN((other == NULL_VALUE && getString().empty()) || other == STRING_VALUE); 
case_array:    U_RETURN((other == NULL_VALUE && (children.head == 0)) || other ==  ARRAY_VALUE);
case_object:   U_RETURN((other == NULL_VALUE && (children.head == 0)) || other == OBJECT_VALUE);
}

__pure UValue* UValue::at(uint32_t pos) const
{
   U_TRACE(0, "UValue::at(%u)", pos)

   if (type_ == ARRAY_VALUE)
      {
      uint32_t i = 0;

      for (UValue* child = children.head; child; ++i, child = child->next)
         {
         if (i == pos) U_RETURN_POINTER(child, UValue);
         }
      }

   U_RETURN_POINTER(0, UValue);
}

__pure UValue* UValue::at(const char* _key, uint32_t key_len) const
{
   U_TRACE(0, "UValue::at(%.*S)", key_len, _key)

   if (type_ == OBJECT_VALUE)
      {
      for (UValue* child = children.head; child; child = child->next)
         {
         U_INTERNAL_ASSERT_POINTER(child->key)

         if (child->key->equal(_key, key_len)) U_RETURN_POINTER(child, UValue);
         }
      }

   U_RETURN_POINTER(0, UValue);
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
   U_TRACE(0+256, "UValue::emitString(%.*S,%u,%p)", len, inptr, len, out)

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

void UValue::stringify() const
{
   U_TRACE(0, "UValue::stringify()")

   U_INTERNAL_DUMP("type_ = %u", type_)

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

   bool bcomma;
   UValue* element;

   U_DUMP("dispatch_table[%d %S] = %p &&case_null = %p", type_, getDataTypeDescription(type_), dispatch_table[type_], &&case_null)

   goto *((char*)&&case_null + dispatch_table[type_]);

case_null:
   u_put_unalignedp32(pstringify, U_MULTICHAR_CONSTANT32('n','u','l','l'));

   pstringify += U_CONSTANT_SIZE("null");

   return;

case_bool:
   if (value.bool_)
      {
      u_put_unalignedp32(pstringify, U_MULTICHAR_CONSTANT32('t','r','u','e'));

      pstringify += U_CONSTANT_SIZE("true");
      }
   else
      {
      u_put_unalignedp32(pstringify, U_MULTICHAR_CONSTANT32('f','a','l','s'));

      pstringify[4] = 'e';
      pstringify   += U_CONSTANT_SIZE("false");
      }

   return;

case_char:
   *pstringify++ = value.char_;

   return;

case_uchar:
   *pstringify++ = value.uchar_;

   return;

case_short:
   pstringify += u_num2str32s(value.short_, pstringify); 

   return;

case_ushort:
   pstringify += u_num2str32(value.ushort_, pstringify); 

   return;

case_int:
   pstringify += u_num2str32s(value.int_, pstringify); 

   return;

case_uint:
   pstringify += u_num2str32(value.uint_, pstringify); 

   return;

case_long:
   pstringify += u_num2str64s(value.long_, pstringify); 

   return;

case_ulong:
   pstringify += u_num2str64(value.ulong_, pstringify); 

   return;

case_llong:
   pstringify += u_num2str64s(value.llong_, pstringify); 

   return;

case_ullong:
   pstringify += u_num2str64(value.ullong_, pstringify); 

   return;

case_float:
   pstringify += u_dtoa(value.float_, pstringify); 

   return;

case_double:
   pstringify += u_dtoa(value.real_, pstringify); 

   return;

case_ldouble:
   pstringify += u_dtoa(value.lreal_, pstringify); 

   return;

case_string:
   pstringify += emitString((const unsigned char*)((UString*)value.ptr_)->data(), ((UString*)value.ptr_)->size(), pstringify);

   return;

case_array:
   *pstringify++ = '[';

   U_INTERNAL_DUMP("children.head = %p children.tail = %p", children.head, children.tail)

   element = children.head;

   while (element)
      {
      element->stringify();

      if ((element = element->next)) *pstringify++ = ',';
      }

   *pstringify++ = ']';

   return;

case_object:
   bcomma = false;

   *pstringify++ = '{';

   for (UValue* member = children.head; member; member = member->next)
      {
      U_INTERNAL_ASSERT_POINTER(member->key)

      if (bcomma) *pstringify++ = ',';
      else        bcomma = true;

      pstringify += emitString((const unsigned char*)U_STRING_TO_PARAM(*(member->key)), pstringify);

      *pstringify++ = ':';

      member->stringify();
      }

   *pstringify++ = '}';
}

void UValue::appendNode(UValue* child)
{
   U_TRACE(0, "UValue::appendNode(%p)", child)

   U_INTERNAL_DUMP("child->parent = %p children.head = %p children.tail = %p",
                    child->parent,     children.head,     children.tail)

   child->parent = this;
   child->prev   = children.tail;
   child->next   = 0;

   if (children.tail) children.tail->next = child;
   else               children.head       = child;
                      children.tail       = child;

   U_INTERNAL_DUMP("child->parent = %p children.head = %p children.tail = %p",
                    child->parent,     children.head,     children.tail)
}

bool UValue::readValue()
{
   U_TRACE(0, "UValue::readValue()")

   static const int dispatch_table[] = {
      0,/* '!' */
      (int)((char*)&&case_dquote-(char*)&&cdefault),/* '"' */
      0,/* '#' */
      0,/* '$' */
      0,/* '%' */
      0,/* '&' */
      0,/* '\'' */
      0,/* '(' */
      0,/* ')' */
      0,/* '*' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '+' */
      0,/* ',' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '-' */
      0,/* '.' */
      0,/* '/' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '0' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '1' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '2' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '3' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '4' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '5' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '6' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '7' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '8' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '9' */
      0,/* ':' */
      0,/* ';' */
      0,/* '<' */
      0,/* '=' */
      0,/* '>' */
      0,/* '?' */
      0,/* '@' */
      0,/* 'A' */
      0,/* 'B' */
      0,/* 'C' */
      0,/* 'D' */
      0,/* 'E' */
      0,/* 'F' */
      0,/* 'G' */
      0,/* 'H' */
      0,/* 'I' */
      0,/* 'J' */
      0,/* 'K' */
      0,/* 'L' */
      0,/* 'M' */
      0,/* 'N' */
      0,/* 'O' */
      0,/* 'P' */
      0,/* 'Q' */
      0,/* 'R' */
      0,/* 'S' */
      0,/* 'T' */
      0,/* 'U' */
      0,/* 'V' */
      0,/* 'W' */
      0,/* 'X' */
      0,/* 'Y' */
      0,/* 'Z' */
      (int)((char*)&&case_vector-(char*)&&cdefault),/* '[' */
      0,/* '\' */
      0,/* ']' */
      0,/* '^' */
      0,/* '_' */
      0,/* '`' */
      0,/* 'a' */
      0,/* 'b' */
      0,/* 'c' */
      0,/* 'd' */
      0,/* 'e' */
      (int)((char*)&&case_false-(char*)&&cdefault),/* 'f' */
      0,/* 'g' */
      0,/* 'h' */
      0,/* 'i' */
      0,/* 'j' */
      0,/* 'k' */
      0,/* 'l' */
      0,/* 'm' */
      (int)((char*)&&case_null-(char*)&&cdefault),/* 'n' */
      0,/* 'o' */
      0,/* 'p' */
      0,/* 'q' */
      0,/* 'r' */
      0,/* 's' */
      (int)((char*)&&case_true-(char*)&&cdefault),/* 't' */
      0,/* 'u' */
      0,/* 'v' */
      0,/* 'w' */
      0,/* 'x' */
      0,/* 'y' */
      0,/* 'z' */
      (int)((char*)&&case_object-(char*)&&cdefault),/* '{' */
      0,/* '|' */
      0,/* '}' */
      0/* '~' */
   };

   ptok->skipSpaces();

   if (ptok->atEnd())
      {
      type_       = NULL_VALUE;
      value.real_ = 0.0;

      U_RETURN(false);
      }

   uint32_t sz;
   int type_num;
   UValue* child;
   const char* end;
   const char* last;
   const char* start = ptok->getPointer();

   char c = ptok->next();

   U_INTERNAL_DUMP("c = %d dispatch_table[%d] = %p &&cdefault = %p", c, c-'!', dispatch_table[c-'!'], &&cdefault)

   goto *((char*)&&cdefault + dispatch_table[c-'!']);

cdefault:
   U_RETURN(false);

case_dquote:
    end = ptok->getEnd();
   last = u_find_char(++start, end, '"');
     sz = (last < end ? last - start : 0);

   type_ = STRING_VALUE;

   U_INTERNAL_DUMP("sz = %u", sz)

   if (sz)
      {
      char bjson = 0;

      U_NEW(UString, value.ptr_, UString(sz));

      UEscape::decode(start, sz, *(UString*)value.ptr_, &bjson);

      U_INTERNAL_DUMP("bjson = %u", bjson)

      if (bjson) U_RETURN(false);

      sz = ((UString*)value.ptr_)->size();
      }
   else
      {
      sz = 1;

      U_NEW(UString, value.ptr_, UString); // NB: omit the () when invoking the default constructor...
      }

   U_INTERNAL_DUMP("value.ptr_ = %V", ((UString*)value.ptr_)->rep)

   if (last < end) ptok->setPointer(last+1);

   if (sz) U_RETURN(true);

   U_RETURN(false);

   /**
    * The JSON spec says that a number shall follow this precise pattern (spaces and quotes added for readability):
    *
    * '-'? (0 | [1-9][0-9]*) ('.' [0-9]+)? ([Ee] [+-]? [0-9]+)?
    *
    * However, some JSON parsers are more liberal. For instance, PHP accepts '.5' and '1.'. JSON.parse accepts '+3'
    */
case_number:
   type_num = ptok->getTypeNumber();

   if (type_num != 0)
      {
      if (type_num < 0)
         {
         type_       = REAL_VALUE;
         value.real_ = (type_num == INT_MIN // scientific notation (Ex: 1.45e10)
                                    ?   strtod(start, 0)
                                    : u_strtod(start, ptok->getPointer(), type_num));

         U_INTERNAL_DUMP("value.real_ = %g", value.real_)

         U_INTERNAL_ASSERT_EQUALS(value.real_, strtod(start, 0))
         }
      else if (c == '-')
         {
         type_        = LLONG_VALUE;
         value.llong_ = u_strtoll(start, ptok->getPointer());

         U_INTERNAL_DUMP("value.llong_ = %lld", value.llong_)

#     ifdef HAVE_STRTOULL
         U_INTERNAL_ASSERT_EQUALS(value.llong_, ::strtoll(start, 0, 10))
#     endif
         }
      else
         {
         value.ullong_ = u_strtoull(start, ptok->getPointer());

         U_INTERNAL_DUMP("value.ullong_ = %llu", value.ullong_)

#     ifdef HAVE_STRTOULL
         U_INTERNAL_ASSERT_EQUALS(value.ullong_, ::strtoull(start, 0, 10))
#     endif

         if (*start == '0' && // NB: numbers cannot have leading zeroes...
             value.ullong_)
            {
            U_RETURN(false);
            }

         type_ = ULLONG_VALUE;
         }

      U_RETURN(true);
      }

   U_RETURN(false);

case_vector:
   type_ = ARRAY_VALUE;

   while (true)
      {
      ptok->skipSpaces();

      if (ptok->atEnd()) break;

      c = ptok->next();

      if (c == ']') U_RETURN(true);

      if (c != ',') ptok->back();
      else
         {
         U_INTERNAL_DUMP("children.head = %p children.tail = %p", children.head, children.tail)

         if (children.head == 0) U_RETURN(false);
         }

      U_NEW(UValue, child, UValue);

      if (child->readValue() == false)
         {
         delete child;

         U_RETURN(false);
         }

      U_INTERNAL_DUMP("child->type_ = %u", child->type_)

      appendNode(child);
      }

   U_RETURN(false);

case_false:
      if (u_get_unalignedp32(ptok->getPointer()) == U_MULTICHAR_CONSTANT32('a','l','s','e'))
         {
         type_       = BOOLEAN_VALUE;
         value.bool_ = false;

         ptok->setPointer(start+U_CONSTANT_SIZE("false"));

         U_RETURN(true);
         }

      U_RETURN(false);

case_null:
      if (u_get_unalignedp32(start) == U_MULTICHAR_CONSTANT32('n','u','l','l'))
         {
         type_       = NULL_VALUE;
         value.real_ = 0.0;

         ptok->setPointer(start+U_CONSTANT_SIZE("null"));

         U_RETURN(true);
         }

      U_RETURN(false);

case_true:
      if (u_get_unalignedp32(start) == U_MULTICHAR_CONSTANT32('t','r','u','e'))
         {
         type_       = BOOLEAN_VALUE;
         value.bool_ = true;

         ptok->setPointer(start+U_CONSTANT_SIZE("true"));

         U_RETURN(true);
         }

      U_RETURN(false);

case_object:
      {
      UValue name;
      UValue* pvalue;

      type_ = OBJECT_VALUE;

      while (true)
         {
         ptok->skipSpaces();

         if (ptok->atEnd()) break;

         c = ptok->next();

         if (c == '}') U_RETURN(true);

         if (c != ',') ptok->back();

         if (name.readValue() == false ||
             name.isString()  == false)
            {
            U_RETURN(false);
            }

         if ((ptok->skipSpaces(),
              ptok->atEnd() ||
              ptok->next() != ':'))
            {
            U_RETURN(false);
            }

         U_NEW(UValue, pvalue, UValue);

         if (pvalue->readValue() == false)
            {
            delete pvalue;

            U_RETURN(false);
            }

         U_INTERNAL_DUMP("pvalue->type_ = %u", pvalue->type_)

         pvalue->key = (UString*)name.value.ptr_;

         name.type_ = NULL_VALUE;

         appendNode(pvalue);
         }

      U_RETURN(false);
      }
}

// =======================================================================================================================
// An in-place JSON element reader (@see http://www.codeproject.com/Articles/885389/jRead-an-in-place-JSON-element-reader)
// =======================================================================================================================
// Instead of parsing JSON into some structure, this maintains the input JSON as unaltered text and allows queries to be
// made on it directly. E.g. with the simple JSON:
// UString json = U_STRING_FROM_CONSTANT("
// {
//    "astring":"This is a string",
//    "anumber":42,
//    "myarray":[ "one", 2, {"description":"element 3"}, null ],
//    "yesno":true,
//    "HowMany":"1234",
//    "foo":null
// }");
//
// calling:
//    UString query = U_STRING_FROM_CONSTANT("{'myarray'[0"), value;
//    int dataType = jread(json, query, value);
//
// would return:
//    value -> "one"
//    dataType = STRING_VALUE;
//
// The query string simply defines the route to the required data item as an arbitary list of object or array specifiers:
//    object element = "{'keyname'"
//     array element = "[INDEX"
//
// The jread() function describe the located element, this can be used to locate any element, not just terminal values e.g.
//    query = U_STRING_FROM_CONSTANT("{'myarray'");
//    dataType = jread(json, query, value);
//
// in this case would return:
//    value -> "[ "one", 2, {"descripton":"element 3"}, null ]"
//    dataType = ARRAY_VALUE;
//
// allowing jread() to be called again on the array:
//    dataType = jread(value, U_STRING_FROM_CONSTANT("[3"), value); // get 4th element - the null value
//
// in this case would return:
//    value -> ""
//    dataType = NULL_VALUE;
//
// Note that jread() never modifies the source JSON and does not allocate any memory. i.e. elements are assigned as
// substring of the source json string
// =======================================================================================================================

#define U_JR_EOL     (NUMBER_VALUE+1) // 19 end of input string (ptr at '\0')
#define U_JR_COLON   (NUMBER_VALUE+2) // 20 ":"
#define U_JR_COMMA   (NUMBER_VALUE+3) // 21 ","
#define U_JR_EARRAY  (NUMBER_VALUE+4) // 22 "]"
#define U_JR_QPARAM  (NUMBER_VALUE+5) // 23 "*" query string parameter
#define U_JR_EOBJECT (NUMBER_VALUE+6) // 24 "}"

int      UValue::jread_error;
uint32_t UValue::jread_pos;
uint32_t UValue::jread_elements;

U_NO_EXPORT int UValue::jreadFindToken(UTokenizer& tok)
{
   U_TRACE(0, "UValue::jreadFindToken(%p)", &tok)

   tok.skipSpaces();

   if (tok.atEnd()) U_RETURN(U_JR_EOL);

   switch (tok.current())
      {
      case '+':
      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': U_RETURN(NUMBER_VALUE);

      case 't':
      case 'f': U_RETURN(BOOLEAN_VALUE);

      case 'n': U_RETURN(NULL_VALUE);

      case '[': U_RETURN(ARRAY_VALUE);
      case ']': U_RETURN(U_JR_EARRAY);

      case '{': U_RETURN(OBJECT_VALUE);
      case '}': U_RETURN(U_JR_EOBJECT);

      case '"':
      case '\'': U_RETURN(STRING_VALUE);

      case ':': U_RETURN(U_JR_COLON);
      case ',': U_RETURN(U_JR_COMMA);
      case '*': U_RETURN(U_JR_QPARAM);
      }

   U_RETURN(-1);
}

U_NO_EXPORT int UValue::jread_skip(UTokenizer& tok)
{
   U_TRACE(0, "UValue::jread_skip(%p)", &tok)

   tok.skipSpaces();

   if (tok.atEnd()) U_RETURN(NULL_VALUE);

   switch (tok.next())
      {
      case 'n': (void) tok.skipToken(U_CONSTANT_TO_PARAM("ull"));  U_RETURN(   NULL_VALUE);
      case 't': (void) tok.skipToken(U_CONSTANT_TO_PARAM("rue"));  U_RETURN(BOOLEAN_VALUE);
      case 'f': (void) tok.skipToken(U_CONSTANT_TO_PARAM("alse")); U_RETURN(BOOLEAN_VALUE);

      case '+':
      case '-':
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
         {
         tok.skipNumber();

         U_RETURN(NUMBER_VALUE);
         }

      case  '"':
         {
         const char* ptr  = tok.getPointer();
         const char* _end = tok.getEnd();
         const char* last = u_find_char(ptr, _end, '"');

         if (last < _end)
            {
            tok.setPointer(last+1);

            U_RETURN(STRING_VALUE);
            }
         }
      break;

      case '[':
         {
         while (true)
            {
            tok.skipSpaces();

            if (tok.atEnd()) break;

            char c = tok.next();

            if (c == ']') break;
            if (c != ',') tok.back();

            if (jread_skip(tok) == -1) U_RETURN(-1);
            }

         U_RETURN(ARRAY_VALUE);
         }

      case '{':
         {
         while (true)
            {
            tok.skipSpaces();

            if (tok.atEnd()) break;

            char c = tok.next();

            if (c == '}') break;
            if (c != ',') tok.back();

            int dataType = jread_skip(tok);

            if (dataType == -1) U_RETURN(-1);

            tok.skipSpaces();

            if (tok.atEnd()       || 
                tok.next() != ':' ||
                dataType != STRING_VALUE)
               {
               U_RETURN(-1);
               }

            if (jread_skip(tok) == -1) U_RETURN(-1);
            }

         U_RETURN(OBJECT_VALUE);
         }
      }

   U_RETURN(-1);
}

U_NO_EXPORT UString UValue::jread_string(UTokenizer& tok)
{
   U_TRACE(0, "UValue::jread_string(%p)", &tok)

   UString result;

   tok.skipSpaces();

         char  c    = tok.next();
   const char* ptr  = tok.getPointer();
   const char* _end = tok.getEnd();
   const char* last = u_find_char(ptr, _end, c);
   uint32_t sz      = (last < _end ? last - ptr : 0);

   U_INTERNAL_DUMP("c = %C sz = %u", c, sz)

   if (sz) (void) result.assign(ptr, sz);

   if (last < _end) tok.setPointer(last+1);

   U_RETURN_STRING(result);
}

// used when query ends at an object, we want to return the object -> "{... "

U_NO_EXPORT UString UValue::jread_object(UTokenizer& tok)
{
   U_TRACE(0, "UValue::jread_object(%p)", &tok)

   int jTok;
   int dataType;
   const char* start = tok.getPointer();

   jread_elements = 0;

   while (true)
      {
      dataType = jread_skip(++tok);

      if (dataType == -1 ||
          dataType != STRING_VALUE)
         {
         jread_error = 3; // Expected "key"

         break;
         }

      U_INTERNAL_DUMP("jread_elements = %u", jread_elements)

      jTok = jreadFindToken(tok);

      U_DUMP("jTok = (%d %S)", jTok, getDataTypeDescription(jTok))

      if (jTok != U_JR_COLON)
         {
         jread_error = 4; // Expected ":"

         break;
         }

      if (jread_skip(++tok) == -1) break;

      ++jread_elements;

      U_INTERNAL_DUMP("jread_elements = %u", jread_elements)

      jTok = jreadFindToken(tok);

      U_DUMP("jTok = (%d %S)", jTok, getDataTypeDescription(jTok))

      if (jTok == U_JR_EOBJECT)
         {
         ++tok;

         U_RETURN_STRING(tok.substr(start));
         }

      if (jTok != U_JR_COMMA)
         {
         jread_error = 6; // Expected "," in object

         break;
         }
      }

   return UString::getStringNull();
}

// we're looking for the nth "key" value in which case keyIndex is the index of the key we want

U_NO_EXPORT UString UValue::jread_object(UTokenizer& tok, uint32_t keyIndex)
{
   U_TRACE(0, "UValue::jread_object(%p,%u)", &tok, keyIndex)

   int jTok;
   UString key;

   jread_elements = 0;

   while (true)
      {
      key = jread_string(++tok);

      if (key.empty())
         {
         jread_error = 3; // Expected "key"

         break;
         }

      U_INTERNAL_DUMP("jread_elements = %u", jread_elements)

      if (jread_elements == keyIndex) U_RETURN_STRING(key); // if match keyIndex we return "key" at this index

      jTok = jreadFindToken(tok);

      U_DUMP("jTok = (%d %S)", jTok, getDataTypeDescription(jTok))

      if (jTok != U_JR_COLON)
         {
         jread_error = 4; // Expected ":"

         break;
         }

      if (jread_skip(++tok) == -1) break;

      ++jread_elements;

      U_INTERNAL_DUMP("jread_elements = %u", jread_elements)

      jTok = jreadFindToken(tok);

      U_DUMP("jTok = (%d %S)", jTok, getDataTypeDescription(jTok))

      if (jTok == U_JR_EOBJECT)
         {
         // we wanted a "key" value - that we didn't find

         jread_error = 11; // Object key not found (bad index)

         break;
         }

      if (jTok != U_JR_COMMA)
         {
         jread_error = 6; // Expected "," in object

         break;
         }
      }

   return UString::getStringNull();
}

int UValue::jread(const UString& json, const UString& query, UString& result, uint32_t* queryParams)
{
   U_TRACE(0+256, "UValue::jread(%V,%V,%p,%p)", json.rep, query.rep, &result, queryParams)

   U_ASSERT(result.empty())

   const char* start;
   uint32_t count, index;
   UString jElement, qElement; // qElement = query 'key'

   UTokenizer tok1(json),
              tok2(query);

   int jTok = jreadFindToken(tok1),
       qTok = jreadFindToken(tok2);

   U_DUMP("jTok = (%d %S)", jTok, getDataTypeDescription(jTok))
   U_DUMP("qTok = (%d %S)", qTok, getDataTypeDescription(qTok))

   jread_elements = 0;

   if (qTok != jTok &&
       qTok != U_JR_EOL)
      {
      jread_error = 1; // JSON does not match Query

      U_RETURN(jTok);
      }

   jread_error = 0;

   switch (jTok)
      {
      case -1:
         {
         jread_error = 2; // Error reading JSON value

         U_RETURN(-1);
         }

      case STRING_VALUE: // "string" 
         {
         jread_elements = 1;

         result = jread_string(tok1);
         }
      break;

      case    NULL_VALUE: // null
      case  NUMBER_VALUE: // number (may be -ve) int or float
      case BOOLEAN_VALUE: // true or false
         {
         const char* ptr =  start = tok1.getPointer();
               char    c = *start;

         while (u__isvalidchar(c) &&
                u__isendtoken(c) == false)
            {
            c = *++ptr;
            }

         jread_elements = 1;

         (void) result.assign(start, ptr-start);
         }
      break;

      case OBJECT_VALUE: // "{"
         {
         if (qTok == U_JR_EOL)
            {
            jTok   = OBJECT_VALUE;
            result = jread_object(tok1); 

            goto end;
            }

         qTok = jreadFindToken(++tok2); // "('key'...", "{NUMBER", "{*" or EOL

         U_DUMP("qTok = (%d %S)", qTok, getDataTypeDescription(qTok))

         if (qTok != STRING_VALUE)
            {
            index = 0;

            switch (qTok)
               {
               case NUMBER_VALUE: // index value
                  {
                  start = tok2.getPointer();

                  tok2.skipNumber();

                  index = u_strtoul(start, tok2.getPointer());

                  U_INTERNAL_DUMP("index = %u", index)

                  U_INTERNAL_ASSERT_EQUALS(index, ::strtol(start, 0, 10))
                  }
               break;

               case U_JR_QPARAM: ++tok2; index = (queryParams ? *queryParams++ : 0); break; // substitute parameter

               default:
                  {
                  jread_error = 12; // Bad Object key

                  U_RETURN(-1);
                  }
               }

            jTok   = OBJECT_VALUE;
            result = jread_object(tok1, index); 

            goto end;
            }

         qElement = jread_string(tok2);

         // read <key> : <value> , ... }
         // loop 'til key matched

         while (true)
            {
            jElement = jread_string(++tok1);

            if (jElement.empty())
               {
               jread_error = 3;  // Expected "key"

               break;
               }

            jTok = jreadFindToken(tok1);

            U_DUMP("jTok = (%d %S)", jTok, getDataTypeDescription(jTok))

            if (jTok != U_JR_COLON)
               {
               jread_error = 4; // Expected ":"

               break;
               }

            // compare object keys

            U_INTERNAL_DUMP("jElement = %V qElement = %V", jElement.rep, qElement.rep)

            if (jElement == qElement)
               {
               // found object key

               ++tok1;

               int ret = jread(tok1.substr(), tok2.substr(), result, queryParams);

               U_INTERNAL_DUMP("result = %V", result.rep)

               U_RETURN(ret);
               }

            // no key match... skip this value

            if (jread_skip(++tok1) == -1) break;

            jTok = jreadFindToken(tok1);

            U_DUMP("jTok = (%d %S)", jTok, getDataTypeDescription(jTok))

            if (jTok == U_JR_EOBJECT)
               {
               jread_error = 5; // Object key not found

               break;
               }

            if (jTok != U_JR_COMMA)
               {
               jread_error = 6; // Expected "," in object

               break;
               }
            }
         }
      break;

      case ARRAY_VALUE: // "[NUMBER" or "[*"
         {
         // read index, skip values 'til index

         if (qTok == U_JR_EOL)
            {
            tok1.skipSpaces();

            start = tok1.getPointer();

            while (true)
               {
               if (jread_skip(++tok1) == -1) break; // array value

               ++jread_elements;

               U_INTERNAL_DUMP("jread_elements = %u", jread_elements)

               jTok = jreadFindToken(tok1);

               U_DUMP("jTok = (%d %S)", jTok, getDataTypeDescription(jTok))

               if (jTok == U_JR_EARRAY)
                  {
                  ++tok1;

                  break;
                  }

               if (jTok != U_JR_COMMA)
                  {
                  jread_error = 9; // Expected "," in array

                  break;
                  }
               }

            jTok   = ARRAY_VALUE;
            result = tok1.substr(start);

            goto end;
            }

         index = 0;
         qTok  = jreadFindToken(++tok2); // "[NUMBER" or "[*"
         start = tok2.getPointer();

         U_DUMP("qTok = (%d %S)", qTok, getDataTypeDescription(qTok))

         if (qTok == U_JR_QPARAM)
            {
            ++tok2;

            index = (queryParams ? *queryParams++ : 0); // substitute parameter
            }
         else if (qTok == NUMBER_VALUE)
            {
            // get array index   

            tok2.skipNumber();

            index = u_strtoul(start, tok2.getPointer());

            U_INTERNAL_DUMP("index = %u", index)

            U_INTERNAL_ASSERT_EQUALS(index, ::strtol(start, 0, 10))
            }

         count = 0;

         while (true)
            {
            if (count == index)
               {
               int ret;

               ++tok1;

               ret = jread(tok1.substr(), tok2.substr(), result, queryParams); // return value at index

               U_INTERNAL_DUMP("result = %V", result.rep)

               U_RETURN(ret);
               }

            // not this index... skip this value

            if (jread_skip(++tok1) == -1) break;

            ++count;          

            jTok = jreadFindToken(tok1); // , or ]

            U_DUMP("jTok = (%d %S)", jTok, getDataTypeDescription(jTok))

            if (jTok == U_JR_EARRAY)
               {
               jread_error = 10; // Array element not found (bad index)

               break;
               }

            if (jTok != U_JR_COMMA)
               {
               jread_error = 9; // Expected "," in array

               break;
               }
            }
         }
      break;

      default: jread_error = 8; // unexpected character (in pResult->dataType)
      }

   // We get here on a 'terminal value' - make sure the query string is empty also

   qTok = jreadFindToken(tok2);

   U_DUMP("qTok = (%d %S)", qTok, getDataTypeDescription(qTok))

   if (qTok != U_JR_EOL)
      {
      if (jread_error == 0) jread_error = 7; // terminal value found before end of query
      }

   if (jread_error)
      {
      jread_elements = 0;

      U_INTERNAL_DUMP("jread_error = %d qElement = %V", jread_error, qElement.rep)
      }

end:
   jread_pos = tok1.getDistance();

   U_DUMP("jTok = (%d %S) result = %V jread_pos = %u", jTok, getDataTypeDescription(jTok), result.rep, jread_pos)

   U_RETURN(jTok);
}

bool UValue::jfind(const UString& json, const char* query, uint32_t query_len, UString& result)
{
   U_TRACE(0, "UValue::jfind(%V,%.*S,%u,%p)", json.rep, query_len, query, query_len, &result)

   U_ASSERT(result.empty())

   uint32_t pos = json.find(query, 0, query_len);

   U_INTERNAL_DUMP("pos = %d", pos)

   if (pos != U_NOT_FOUND)
      {
      pos += query_len;

      if (u__isquote(json.c_char(pos))) ++pos;

      UTokenizer tok(json.substr(pos));

      int sTok = jreadFindToken(tok);

      U_DUMP("sTok = (%d %S)", sTok, getDataTypeDescription(sTok))

      if ( sTok != U_JR_EOL   &&
          (sTok == U_JR_COMMA ||
           sTok == U_JR_COLON))
         {
         ++tok;
         }

      tok.skipSpaces();

      const char* start = tok.getPointer();

      if (jread_skip(tok) != -1)
         {
         const char* end = tok.getPointer();

         if (u__isquote(*start))
            {
            ++start;
            --end;

            U_INTERNAL_ASSERT(u__isquote(*end))
            }

         (void) result.assign(start, end-start);

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

// reads one value from an array - assumes jarray points at the start of an array or array element

int UValue::jreadArrayStep(const UString& jarray, UString& result)
{
   U_TRACE(0, "UValue::jreadArrayStep(%V,%V)", jarray.rep, result.rep)

   UTokenizer tok(jarray);

   tok.setDistance(jread_pos);

   int jTok = jreadFindToken(tok);

   U_DUMP("jTok = (%d %S)", jTok, getDataTypeDescription(jTok))

   switch (jTok)
      {
      case U_JR_COMMA:  // element separator
      case ARRAY_VALUE: // start of array
         {
         ++tok;

         jTok = jread(tok.substr(), UString::getStringNull(), result);
         }
      break;

      case U_JR_EARRAY: jread_error = 13; break; // End of array found
      default:          jread_error =  9;        // Expected comma in array
      }

   U_RETURN(jTok);
}

// DEBUG

#ifdef DEBUG
const char* UValue::getJReadErrorDescription()
{
   U_TRACE_NO_PARAM(0, "UValue::getJReadErrorDescription()")

   static const char* errlist[] = {
      "Ok",                                       //  0
      "JSON does not match Query",                //  1
      "Error reading JSON value",                 //  2
      "Expected \"key\"",                         //  3
      "Expected ':'",                             //  4
      "Object key not found",                     //  5
      "Expected ',' in object",                   //  6
      "Terminal value found before end of query", //  7
      "Unexpected character",                     //  8
      "Expected ',' in array",                    //  9
      "Array element not found (bad index)",      // 10
      "Object key not found (bad index)",         // 11
      "Bad object key",                           // 12
      "End of array found",                       // 13
      "End of object found"                       // 14
   };

   const char* descr = (jread_error >= 0 && (int)U_NUM_ELEMENTS(errlist) ? errlist[jread_error] : "Unknown jread error");

   U_RETURN(descr);
}

const char* UValue::getDataTypeDescription(int type)
{
   U_TRACE(0, "UValue::getDataTypeDescription(%d)", type)

   struct data_type_info {
      int value;        // The numeric value
      const char* name; // The equivalent symbolic value
   };

   static const struct data_type_info data_type_table[] = {
      U_ENTRY(NULL_VALUE),
      U_ENTRY(BOOLEAN_VALUE),
      U_ENTRY(CHAR_VALUE),
      U_ENTRY(UCHAR_VALUE),
      U_ENTRY(SHORT_VALUE),
      U_ENTRY(USHORT_VALUE),
      U_ENTRY(INT_VALUE),
      U_ENTRY(UINT_VALUE),
      U_ENTRY(LONG_VALUE),
      U_ENTRY(ULONG_VALUE),
      U_ENTRY(LLONG_VALUE),
      U_ENTRY(ULLONG_VALUE),
      U_ENTRY(FLOAT_VALUE),
      U_ENTRY(REAL_VALUE),
      U_ENTRY(LREAL_VALUE),
      U_ENTRY(STRING_VALUE),
      U_ENTRY(ARRAY_VALUE),
      U_ENTRY(OBJECT_VALUE),
      U_ENTRY(NUMBER_VALUE),
      U_ENTRY(U_JR_EOL),
      U_ENTRY(U_JR_COLON),
      U_ENTRY(U_JR_COMMA),
      U_ENTRY(U_JR_EARRAY),
      U_ENTRY(U_JR_QPARAM),
      U_ENTRY(U_JR_EOBJECT)
   };

   const char* descr = (type >= 0 && type < (int)U_NUM_ELEMENTS(data_type_table) ? data_type_table[type].name : "Data type unknown");

   U_RETURN(descr);
}

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
      U_WARNING("Error on value: (key contains invalid UTF-8)\n"
                "--------------------------------------------------\n%s", dump(true));

      return false;
      }

   if (type_ < 0 ||
       type_ > OBJECT_VALUE)
      {
      U_WARNING("Error on value: (tag is invalid)\n"
                "--------------------------------------------------\n%s", dump(true));

      return false;
      }

   if (type_ == BOOLEAN_VALUE &&
       value.bool_ != false   &&
       value.bool_ != true)
      {
      U_WARNING("Error on value: (bool_ is neither false nor true)\n"
                "--------------------------------------------------\n%s", dump(true));

      return false;
      }

   if (type_ == STRING_VALUE)
      {
      UString* str = (UString*)value.ptr_;

      if (str == 0)
         {
         U_WARNING("Error on value: (string is NULL)\n"
                   "--------------------------------------------------\n%s", dump(true));

         return false;
         }

      if (*str &&
           str->isUTF8(0) == false)
         {
         U_WARNING("Error on value: (string contains invalid UTF-8)\n"
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
            U_WARNING("Error on value: (tail is NULL, but head is not)\n"
                      "--------------------------------------------------\n%s", dump(true));

            return false;
            }

         if (children.tail)
            {
            U_WARNING("Error on value: (head is NULL, but tail is not)\n"
                      "--------------------------------------------------\n%s", dump(true));

            return false;
            }
         }
      else
         {
         if (children.head->prev)
            {
            U_WARNING("Error on value: (First child's prev pointer is not NULL)\n"
                      "--------------------------------------------------\n%s", dump(true));

            return false;
            }

         UValue* last = 0;

         for (UValue* child = children.head; child; last = child, child = child->next)
            {
            if (child == this)
               {
               U_WARNING("Error on value: (it is its own child)\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (child->next == child)
               {
               U_WARNING("Error on value: (child->next == child (cycle))\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (child->next == children.head)
               {
               U_WARNING("Error on value: (child->next == head (cycle))\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (child->parent != this)
               {
               U_WARNING("Error on value: (child does not point back to parent)\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (child->next &&
                child->next->prev != child)
               {
               U_WARNING("Error on value: (child->next does not point back to child)\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (type_ == ARRAY_VALUE && child->key)
               {
               U_WARNING("Error on value: (Array element's key is not NULL)\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (type_ == OBJECT_VALUE && child->key == 0)
               {
               U_WARNING("Error on value: (Object member's key is NULL)\n"
                         "--------------------------------------------------\n%s", dump(true));

               return false;
               }

            if (child->invariant() == false) return false;
            }

         if (last != children.tail)
            {
            U_WARNING("Error on value: (tail does not match pointer found by starting at head and following next links)\n"
                      "--------------------------------------------------\n%s", dump(true));

            return false;
            }
         }
      }

   return true;
}
#endif
