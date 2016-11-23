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

int      UValue::jsonParseFlags;
char*    UValue::pstringify;
UValue*  UValue::phead;
UValue*  UValue::pnode;
uint32_t UValue::size;
#ifdef DEBUG
uint32_t UValue::cnt_real;
uint32_t UValue::cnt_mreal;
#endif

enum jsonParseFlagsType {
   STRING_COPY    = 0x0001,
   CHECK_FOR_UTF  = 0x0002,
   FULL_PRECISION = 0x0004
};

UValue::~UValue()
{
   U_TRACE_UNREGISTER_OBJECT(0, UValue)

   switch (getTag())
      {
      case STRING_VALUE:
      case    UTF_VALUE:
         {
         UStringRep* rep = (UStringRep*)getPayload();

         U_INTERNAL_DUMP("rep(%p) = %V", rep, rep)

         rep->release();
         }
      break;

      case ARRAY_VALUE:
         {
         UValue* element = toNode();

         while (element)
            {
            U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), getDataTypeDescription(element->getTag()))

            next = element->next;

            delete element;

            element = next;
            }
         }
      break;

      case OBJECT_VALUE:
         {
         UValue* element = toNode();

         while (element)
            {
            U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), getDataTypeDescription(element->getTag()))

            U_ASSERT(isStringOrUTF(element->pkey.ival))

            UStringRep* rep = (UStringRep*)getPayload(element->pkey.ival);

            U_INTERNAL_DUMP("element->pkey(%p) = %V", rep, rep)

            rep->release();

            next = element->next;

            delete element;

            element = next;
            }
         }
      break;
      }
}

void UValue::clear()
{
   U_TRACE_NO_PARAM(0+256, "UValue::clear()")

   static const int dispatch_table[] = {
      0,/* 0 */
      0,/* 1 */
      0,/* 2 */
      0,/* 3 */
      0,/* 4 */
      (int)((char*)&&case_string-(char*)&&case_double),
      (int)((char*)&&case_utf-(char*)&&case_double),
      (int)((char*)&&case_array-(char*)&&case_double),
      (int)((char*)&&case_object-(char*)&&case_double),
      0,/* 9 */
   };

   int type = getTag();

   U_DUMP("dispatch_table[(%d,%S)] = %p", type, getDataTypeDescription(type), dispatch_table[type])

   goto *((char*)&&case_double + dispatch_table[type]);

case_double:
   value.ival = 0ULL;

   return;

case_string:
case_utf:
   {
   UStringRep* rep = (UStringRep*)getPayload();

   U_INTERNAL_DUMP("rep(%p) = %V", rep, rep)

   rep->release();

   goto case_double;
   }

case_array:
   {
   UValue* element = toNode();

   while (element)
      {
      U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), getDataTypeDescription(element->getTag()))

      next = element->next;

      delete element;

      element = next;
      }

   goto case_double;
   }

case_object:
   {
   UValue* element = toNode();

   while (element)
      {
      U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), getDataTypeDescription(element->getTag()))

      U_ASSERT(isStringOrUTF(element->pkey.ival))

      UStringRep* rep = (UStringRep*)getPayload(element->pkey.ival);

      U_INTERNAL_DUMP("element->pkey(%p) = %V", rep, rep)

      rep->release();

      next = element->next;

      delete element;

      element = next;
      }

   goto case_double;
   }
}

__pure UValue* UValue::at(uint32_t pos) const
{
   U_TRACE(0, "UValue::at(%u)", pos)

   if (getTag() == ARRAY_VALUE)
      {
      uint32_t i = 0;
      UValue* element = toNode();

      while (element)
         {
         U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), getDataTypeDescription(element->getTag()))

         if (i++ == pos) U_RETURN_POINTER(element, UValue);

         element = element->next;
         }
      }

   U_RETURN_POINTER(0, UValue);
}

__pure UValue* UValue::at(const char* key, uint32_t key_len) const
{
   U_TRACE(0, "UValue::at(%.*S,%u)", key_len, key, key_len)

   if (getTag() == OBJECT_VALUE)
      {
      UValue* element = toNode();

      while (element)
         {
         U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), getDataTypeDescription(element->getTag()))

         U_ASSERT(isStringOrUTF(element->pkey.ival))

         UStringRep* rep = (UStringRep*)getPayload(element->pkey.ival);

         U_INTERNAL_DUMP("element->pkey(%p) = %V", rep, rep)

         if (rep->equal(key, key_len)) U_RETURN_POINTER(element, UValue);

         element = element->next;
         }
      }

   U_RETURN_POINTER(0, UValue);
}

uint32_t UValue::getMemberNames(UVector<UString>& members) const
{
   U_TRACE(0, "UValue::getMemberNames(%p)", &members)

   if (getTag() == OBJECT_VALUE)
      {
      UValue* element = toNode();
      uint32_t sz, n = members.size();

      while (element)
         {
         U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), getDataTypeDescription(element->getTag()))

         U_ASSERT(isStringOrUTF(element->pkey.ival))

         UStringRep* rep = (UStringRep*)getPayload(element->pkey.ival);

         U_INTERNAL_DUMP("element->pkey(%p) = %V", rep, rep)

         members.push(rep);

         element = element->next;
         }

      sz = members.size() - n;

      U_RETURN(sz);
      }

   U_RETURN(0);
}

UString UValue::getString(uint64_t value)
{
   U_TRACE(0, "UValue::getString(0x%x)", value)

   U_ASSERT(isStringOrUTF(value))

   UStringRep* rep = (UStringRep*)getPayload(value);

   U_INTERNAL_ASSERT_POINTER(rep)

   int type = getTag(value);

   if (type == STRING_VALUE)
      {
      UString str(rep);

      U_RETURN_STRING(str);
      }

   U_INTERNAL_ASSERT_EQUALS(type, UTF_VALUE)

   uint32_t sz = rep->size();

   UString str(sz);

   UEscape::decode(rep->data(), sz, str);

   U_RETURN_STRING(str);
}

void UValue::emitUTF(UStringRep* rep) const
{
   U_TRACE(0, "UValue::emitUTF(%p)", rep)

   U_INTERNAL_DUMP("rep = %V", rep)

   uint32_t sz = rep->size();

   UString str(sz);

   UEscape::decode(rep->data(), sz, str);

   *pstringify++ = '"';

   const unsigned char* inptr = (const unsigned char*)str.data();
   const unsigned char* inend =               inptr + str.size();

   while (inptr < inend)
      {
      unsigned char c = *inptr++;

   // U_INTERNAL_DUMP("c = %u", c)

      if (c < 0x1F)
         {
                             *pstringify++ = '\\';
              if (c == '\b') *pstringify++ = 'b'; // 0x08
         else if (c == '\t') *pstringify++ = 't'; // 0x09
         else if (c == '\n') *pstringify++ = 'n'; // 0x0A
         else if (c == '\f') *pstringify++ = 'f'; // 0x0C
         else if (c == '\r') *pstringify++ = 'r'; // 0x0D

         else // \u four hex digits (unicode char)
            {
            *pstringify = 'u';

            u_put_unalignedp32(pstringify+1, U_MULTICHAR_CONSTANT32('0','0',
                                                                    "0123456789ABCDEF"[((c >> 4) & 0x0F)],
                                                                    "0123456789ABCDEF"[( c       & 0x0F)]));

            pstringify += 5;
            }

         continue;
         }

      // This syntax is given in RFC3629, which is the same as that given in The Unicode Standard, Version 6.0. It has the following properties:
      //
      // All codepoints U+0000..U+10FFFF may be encoded, except for U+D800..U+DFFF, which are reserved for UTF-16 surrogate pair encoding.
      // UTF-8 byte sequences longer than 4 bytes are not permitted, as they exceed the range of Unicode.
      // The sixty-six Unicode "non-characters" are permitted (namely, U+FDD0..U+FDEF, U+xxFFFE, and U+xxFFFF)

      if (c <= 0x7F) // 00..7F
         {
         if (c == '"' || // 0x22
             c == '\\')  // 0x5C
            {
            *pstringify++ = '\\';
            }

         *pstringify++ = c;

         continue;
         }

      if (c <= 0xC1) continue; // 80..C1 - Disallow overlong 2-byte sequence

      if (c <= 0xDF) // C2..DF
         {
         if ((inptr[0] & 0xC0) != 0x80) // Make sure subsequent byte is in the range 0x80..0xBF
            {
            u_put_unalignedp16(pstringify, U_MULTICHAR_CONSTANT16(c, *inptr++));

            pstringify += 2;
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
            *pstringify = c;

            u_put_unalignedp16(pstringify+1, U_MULTICHAR_CONSTANT16(inptr[0], inptr[1]));

                 inptr += 2;
            pstringify += 3;
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
            u_put_unalignedp32(pstringify, U_MULTICHAR_CONSTANT32(c,inptr[0],inptr[1],inptr[2]));

                 inptr += 3;
            pstringify += 4;
            }
         }

      // F5..FF
      }

   *pstringify++ = '"';
}

void UValue::stringify() const
{
   U_TRACE_NO_PARAM(0, "UValue::stringify()")

   static const int dispatch_table[] = {
      0,/* 0 */
      (int)((char*)&&case_int-(char*)&&case_double),
      (int)((char*)&&case_uint-(char*)&&case_double),
      (int)((char*)&&case_true-(char*)&&case_double),
      (int)((char*)&&case_false-(char*)&&case_double),
      (int)((char*)&&case_string-(char*)&&case_double),
      (int)((char*)&&case_utf-(char*)&&case_double),
      (int)((char*)&&case_array-(char*)&&case_double),
      (int)((char*)&&case_object-(char*)&&case_double),
      (int)((char*)&&case_null-(char*)&&case_double)
   };

   int type = getTag();

   U_DUMP("dispatch_table[(%d,%S)] = %p", type, getDataTypeDescription(type), dispatch_table[type])

   goto *((char*)&&case_double + dispatch_table[type]);

case_double:
   pstringify = u_dtoa(value.real, pstringify); 

   return;

case_int:
   pstringify = u_num2str32s(getInt(), pstringify);

   return;

case_uint:
   pstringify = u_num2str32(getUInt(), pstringify);

   return;

case_true:
   u_put_unalignedp32(pstringify, U_MULTICHAR_CONSTANT32('t','r','u','e'));

   pstringify += U_CONSTANT_SIZE("true");

   return;

case_false:
   u_put_unalignedp32(pstringify, U_MULTICHAR_CONSTANT32('f','a','l','s'));

   pstringify[4] = 'e';
   pstringify   += U_CONSTANT_SIZE("false");

   return;

case_string: emitString((UStringRep*)getPayload()); return;
case_utf:    emitUTF(   (UStringRep*)getPayload()); return;

case_array:
   {
   *pstringify++ = '[';

   UValue* element = toNode();

   while (element)
      {
      U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), getDataTypeDescription(element->getTag()))

      element->stringify();

      if ((element = element->next)) *pstringify++ = ',';
      }

   *pstringify++ = ']';

   return;
   }

case_object:
   {
   *pstringify++ = '{';

   UValue* element = toNode();

   while (element)
      {
      U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), getDataTypeDescription(element->getTag()))

      element->emitKey();

      *pstringify++ = ':';

      element->stringify();

      if ((element = element->next)) *pstringify++ = ',';
      }

   *pstringify++ = '}';

   return;
   }

case_null:
   u_put_unalignedp32(pstringify, U_MULTICHAR_CONSTANT32('n','u','l','l'));

   pstringify += U_CONSTANT_SIZE("null");
}

void UValue::prettify(uint32_t indent) const
{
   U_TRACE(0, "UValue::prettify(%u)", indent)

   static const int dispatch_table[] = {
      0,/* 0 */
      (int)((char*)&&case_int-(char*)&&case_double),
      (int)((char*)&&case_uint-(char*)&&case_double),
      (int)((char*)&&case_true-(char*)&&case_double),
      (int)((char*)&&case_false-(char*)&&case_double),
      (int)((char*)&&case_string-(char*)&&case_double),
      (int)((char*)&&case_utf-(char*)&&case_double),
      (int)((char*)&&case_array-(char*)&&case_double),
      (int)((char*)&&case_object-(char*)&&case_double),
      (int)((char*)&&case_null-(char*)&&case_double)
   };

   int type = getTag();

   U_DUMP("dispatch_table[(%d,%S)] = %p", type, getDataTypeDescription(type), dispatch_table[type])

   goto *((char*)&&case_double + dispatch_table[type]);

case_double:
   pstringify = u_dtoa(value.real, pstringify); 

   return;

case_int:
   pstringify = u_num2str32s(getInt(), pstringify);

   return;

case_uint:
   pstringify = u_num2str32(getUInt(), pstringify);

   return;

case_true:
   u_put_unalignedp32(pstringify, U_MULTICHAR_CONSTANT32('t','r','u','e'));

   pstringify += U_CONSTANT_SIZE("true");

   return;

case_false:
   u_put_unalignedp32(pstringify, U_MULTICHAR_CONSTANT32('f','a','l','s'));

   pstringify[4] = 'e';
   pstringify   += U_CONSTANT_SIZE("false");

   return;

case_string: emitString((UStringRep*)getPayload()); return;
case_utf:    emitUTF(   (UStringRep*)getPayload()); return;

case_array:
   {
   UValue* element = toNode();

   if (element)
      {
      u_put_unalignedp16(pstringify, U_MULTICHAR_CONSTANT16('[','\n'));
                         pstringify += 2;

      do {
         U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), getDataTypeDescription(element->getTag()))

         (void) memset(pstringify, ' ', indent+4);
                       pstringify +=    indent+4;

         element->prettify(indent);

         if ((element = element->next))
            {
            u_put_unalignedp16(pstringify, U_MULTICHAR_CONSTANT16(',','\n'));
                               pstringify += 2;
            }
         else
            {
            *pstringify++ = '\n';
            }
         }
      while (element);

      (void) memset(pstringify, ' ', indent);
                    pstringify +=    indent;

      *pstringify++ = ']';
      }
   else
      {
      u_put_unalignedp16(pstringify, U_MULTICHAR_CONSTANT16('[',']'));
                         pstringify += 2;
      }

   return;
   }

case_object:
   {
   UValue* element = toNode();

   if (element)
      {
      u_put_unalignedp16(pstringify, U_MULTICHAR_CONSTANT16('{','\n'));
                         pstringify += 2;

      do {
         U_DUMP("element = %p element->next = %p element->type = (%d,%S)", element, element->next, element->getTag(), getDataTypeDescription(element->getTag()))

         (void) memset(pstringify, ' ', indent+4);
                       pstringify +=    indent+4;

         element->emitKey();

         u_put_unalignedp16(pstringify, U_MULTICHAR_CONSTANT16(':',' '));
                            pstringify += 2;

         element->prettify(indent);

         if ((element = element->next))
            {
            u_put_unalignedp16(pstringify, U_MULTICHAR_CONSTANT16(',','\n'));
                               pstringify += 2;
            }
         else
            {
            *pstringify++ = '\n';
            }
         }
      while (element);

      (void) memset(pstringify, ' ', indent);
                    pstringify +=    indent;

      *pstringify++ = '}';
      }
   else
      {
      u_put_unalignedp16(pstringify, U_MULTICHAR_CONSTANT16('{','}'));
                         pstringify += 2;
      }

   return;
   }

case_null:
   u_put_unalignedp32(pstringify, U_MULTICHAR_CONSTANT32('n','u','l','l'));

   pstringify += U_CONSTANT_SIZE("null");
}

#ifndef U_JSON_PARSE_STACK_SIZE
#define U_JSON_PARSE_STACK_SIZE 256
#endif

bool UValue::parse(const UString& document)
{
   U_TRACE(0, "UValue::parse(%V)", document.rep)

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
      (int)((char*)&&case_plus-(char*)&&cdefault),/* '+' */
      (int)((char*)&&case_comma-(char*)&&cdefault),/* ',' */
      (int)((char*)&&case_minus-(char*)&&cdefault),/* '-' */
      (int)((char*)&&case_point-(char*)&&cdefault),/* '.' */
      (int)((char*)&&case_slash-(char*)&&cdefault),/* '/' */
      (int)((char*)&&case_zero-(char*)&&cdefault),/* '0' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '1' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '2' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '3' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '4' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '5' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '6' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '7' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '8' */
      (int)((char*)&&case_number-(char*)&&cdefault),/* '9' */
      (int)((char*)&&case_colon-(char*)&&cdefault),/* ':' */
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
      (int)((char*)&&case_svector-(char*)&&cdefault),/* '[' */
      0,/* '\' */
      (int)((char*)&&case_evector-(char*)&&cdefault),/* ']' */
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
      (int)((char*)&&case_sobject-(char*)&&cdefault),/* '{' */
      0,/* '|' */
      (int)((char*)&&case_eobject-(char*)&&cdefault),/* '}' */
      0/* '~' */
   };

#ifdef DEBUG
   cnt_real  =
   cnt_mreal = 0;
   double tmp;
#endif
   double val;
   const char* p;
   UStringRep* rep;
   unsigned char c;
   const char* start;
   uint64_t integerPart;
   union jval o = {0ULL};
   int pos = -1, gexponent, type;
   const char* s = document.data();
   const char* end = s + (size = document.size());
   uint32_t sz, significandDigit, decimalDigit, exponent;
   bool minus = false, colon = false, comma = false, separator = true;

   struct stack_data {
      uint64_t keys;
      UValue* tails;
          bool tags;
   };

   struct stack_data sd[U_JSON_PARSE_STACK_SIZE];

   U_INTERNAL_DUMP("jsonParseFlags = %d", jsonParseFlags)

   while (s < end)
      {
loop: while (u__isspace(*s)) ++s;

      if (s > end) break;

      start = s++;

      U_INTERNAL_DUMP("*start = %C bdigit = %b", *start, u__isdigit(*s))

      goto *((char*)&&cdefault + dispatch_table[*start-'!']);

case_dquote:
      type = STRING_VALUE;

      if ((jsonParseFlags & CHECK_FOR_UTF) == 0)
         {
         s = u_find_char(s, end, '"');

         goto dquote_assign;
         }

dquote:
      // U_INTERNAL_DUMP("c = %C", *s)

      if (u__isvalidchar((c = *s)) == false) break;

      if (c == '"') goto dquote_assign;

           if (c > 0x7F) type = UTF_VALUE; // 00..7F
      else if (c == '\\')
         {
         type = UTF_VALUE;

         switch ((c = *++s))
            {
            case 'b':
            case 't':
            case 'n':
            case 'f':
            case 'r':
            case '"':
            case '/':
            case '\\':
            break;

            case 'u':
               {
               U_INTERNAL_DUMP("unicode = %.5s", s)

               if (u__isxdigit(s[1]) == false ||
                   u__isxdigit(s[2]) == false ||
                   u__isxdigit(s[3]) == false ||
                   u__isxdigit(s[4]) == false)
                  {
                  goto cdefault;
                  }

               s += 4;
               }
            break;

            default: goto cdefault;
            }
         }

      if (++s >= end) break;

      goto dquote;

dquote_assign:
      if ((sz = s++ - ++start))
         {
         U_DUMP("type = (%d,%S) string(%u) = %.*S", type, getDataTypeDescription(type), sz, sz, start)

         if ((jsonParseFlags & STRING_COPY) == 0)
            {
            U_NEW(UStringRep, rep, UStringRep(start, sz));

            o.ival = getJsonValue(type, rep);
            }
         else
            {
            UString str((void*)start, sz);

            str.hold();

            o.ival = getJsonValue(type, str.rep);
            }
         }
      else
         {
         UStringRep::string_rep_null->hold();

         o.ival = getJsonValue(STRING_VALUE, UStringRep::string_rep_null);
         }

      goto next;

case_plus:
      if (u__isdigit(*s) == false) break;

      ++start;

      goto case_number;

case_comma:
      U_INTERNAL_DUMP("comma: comma = %b separator = %b sd[%d].keys = 0x%x", comma, separator, pos, sd[pos].keys)

      if (comma     ||
          separator ||
          sd[pos].keys)
         {
         break;
         }

      comma     =
      separator = true;

      continue;

case_minus:
      if (u__isdigit((c = *s)) == false) break;

      ++start;

      minus = true;

      if (c == '0')
         {
         c = *++s;

         goto zero;
         }

      goto case_number;

case_point:
      if (u__isdigit(*s) == false) break;

      goto case_number;

case_slash:
      c = *s;

      U_INTERNAL_DUMP("c = %C", c)

      if (c != '/' &&
          c != '*')
         {
         break;
         }

      if (c == '/') // skip line comment
         {
         while (*++s != '\n') {}

         if (++s > end) break;

         goto loop;
         }

      // skip comment

      ++s;

      while (s < end)
         {
         if (u_get_unalignedp16(s) == U_MULTICHAR_CONSTANT16('*','/')) break;

         s += 2;
         }

      s += 2;

      continue;

case_zero:
      if (u__isdigit((c = *s)) == false)
         {
zero:    if (c == '.') goto case_number;

         o.ival = getJsonValue(UINT_VALUE, 0);

         goto next;
         }

      break; // NB: json numbers cannot have leading zeroes...

case_number:
      gexponent = 0;
      decimalDigit = 0;

      if (u__isdigit(*s) == false)
         {
         U_INTERNAL_ASSERT_DIFFERS(*start, '.')

         integerPart      = *start - '0';
         significandDigit = 1;

         U_INTERNAL_DUMP("integerPart = %llu", integerPart)
         }
      else
         {
         while (u__isdigit(*++s)) {}

         if (s > end) s = end;

         p = start + ((c = *start) == '.');

         significandDigit = (s - p);

         U_INTERNAL_DUMP("significandDigit = %u", significandDigit)

         U_INTERNAL_ASSERT_MAJOR(significandDigit, 0)

         if (significandDigit > 16) goto real;

         integerPart = u_strtoull(p, s);

         U_INTERNAL_DUMP("integerPart = %llu", integerPart)

         if (s == end)
            {
            if (c == '.')
               {
               gexponent = -significandDigit;

               goto mreal;
               }

            goto noreal;
            }
         }

      U_INTERNAL_DUMP("*s = %C", *s)

      if (*s == '.')
         {
         U_INTERNAL_DUMP("significandDigit = %u decimalPosition = %u s = %.10S", significandDigit, (s-start), s)

         p = ++s;

         if (u__isdigit(*s) == false)
            {
            val = (double)integerPart;

            goto mreal1;
            }

         while (u__isdigit(*++s)) {}

         if (s > end) s = end;

         decimalDigit = (s-p);

         U_INTERNAL_DUMP("decimalDigit = %u significandDigit = %u *p = %C", decimalDigit, significandDigit, *p)

         U_INTERNAL_ASSERT_MAJOR(decimalDigit, 0)

         significandDigit += decimalDigit;

         if (significandDigit > 16)
            {
            if ((jsonParseFlags & FULL_PRECISION) != 0) goto real; // Parse number in full precision (but slower)

            sz = (significandDigit - 16);

            decimalDigit    -= sz;
            significandDigit = 16;

            U_INTERNAL_DUMP("decimalPart = %llu", u_strtoull(p, s - sz))
            }

#     ifndef U_COVERITY_FALSE_POSITIVE // Control flow issues (MISSING_BREAK)
         switch (decimalDigit)
            {
            case 15: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit-15] - '0');
            case 14: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit-14] - '0');
            case 13: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit-13] - '0');
            case 12: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit-12] - '0');
            case 11: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit-11] - '0');
            case 10: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit-10] - '0');
            case  9: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit- 9] - '0');
            case  8: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit- 8] - '0');
            case  7: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit- 7] - '0');
            case  6: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit- 6] - '0');
            case  5: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit- 5] - '0');
            case  4: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit- 4] - '0');
            case  3: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit- 3] - '0');
            case  2: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit- 2] - '0');
            case  1: integerPart = (integerPart << 3) + (integerPart << 1) + (p[decimalDigit- 1] - '0');
            }
#     endif

         gexponent = -decimalDigit;

         U_INTERNAL_DUMP("integerPart = %llu gexponent = %d", integerPart, gexponent)

         if (u__toupper(*s) == 'E') goto exp; // scientific notation (Ex: 1.45e-10)

         goto mreal;
         }

      if (u__toupper(*s) == 'E') // scientific notation (Ex: 1.45e-10) DBL_MAX => 1.7976931348623157E+308
         {
exp:     if (u__issign((c = *++s))) ++s;

         if (u__isdigit(*s) == false) break;

         p = s;

         while (u__isdigit(*++s)) {}

         if (s > end) s = end;

         exponent = u_strtoul(p, s);

         U_INTERNAL_DUMP("exponent = %u significandDigit = %u", exponent, significandDigit)

         // Use fast path for string-to-double conversion if possible
         // see http://www.exploringbinary.com/fast-path-decimal-to-floating-point-conversion/

         if (c == '-')
            {
            if (exponent > 22) goto real;

            gexponent = -exponent - decimalDigit;
            }
         else
            {
            gexponent =  exponent - decimalDigit;

            if (exponent > 22)
               {
               if (exponent < (22 + 16) &&
                   (significandDigit + (exponent - 22)) < 16)
                  {
                  // Fast Path Cases In Disguise

                  goto mreal;
                  }

               goto real;
               }

            if (*start == '9' &&
                significandDigit == 16) // 2^53-1 9007199254740991.0 (16 digit)
               {
               goto real;
               }
            }

         goto mreal;
         }

noreal:
      U_INTERNAL_ASSERT_DIFFERS(*start, '.')

      if (minus == false)
         {
         if (integerPart > UINT_MAX) // UINT_MAX => 4294967295 (9 digit)
            {
            val = (double)integerPart;

            goto mreal1;
            }

         o.ival = getJsonValue(UINT_VALUE, (void*)(integerPart & 0x00000000FFFFFFFFULL));

         U_INTERNAL_DUMP("value(%.*S) = %u", s-start, start, (uint32_t)integerPart)

         U_INTERNAL_ASSERT_EQUALS((uint32_t)integerPart, ::strtoul(start, 0, 10))
         }
      else
         {
         if (integerPart > 2147483648ULL) // INT_MIN => -2147483648 (9 digit)
            {
            val = (double)integerPart;

            goto mreal1;
            }

         minus = false;

         o.ival = getJsonValue(INT_VALUE, (void*)(-integerPart & 0x00000000FFFFFFFFULL));

         U_INTERNAL_DUMP("value(%.*S) = %d", s-(start-1), start-1, -(int32_t)integerPart)

         U_INTERNAL_ASSERT_EQUALS(-(int32_t)integerPart, ::strtol(start-1, 0, 10))
         }

      goto next;

real: o.real = ::strtod(start-minus, (char**)&s);

#  ifdef DEBUG
      ++cnt_real;

      U_INTERNAL_DUMP("::strtod(%.*S) = %g cnt_real = %u", s-(start-minus), start-minus, o.real, cnt_real)
#  endif

      minus = false;

      goto next;

mreal:
      U_INTERNAL_DUMP("gexponent = %d significandDigit = %u integerPart = %llu", gexponent, significandDigit, integerPart)

      U_INTERNAL_ASSERT_MINOR(significandDigit, 17)

      val = (gexponent > 0 ? (double)integerPart * u_pow10[ gexponent]
                           : (double)integerPart / u_pow10[-gexponent]);

mreal1:
#  ifdef DEBUG
      ++cnt_mreal;

      if (minus) --start;

      tmp = ::strtod(start, 0);
#  endif

      o.real = minus ? (minus = false, -val) : val;

#  ifdef DEBUG
      if (o.real == tmp)
         {
         U_INTERNAL_DUMP("o.real = %g cnt_mreal = %u", o.real, cnt_mreal)
         }
      else
         {
         U_WARNING("o.real(%.*S) = %g differs from ::strtod() = %g", s-start, start, o.real, tmp);
         }
#  endif

      goto next;

case_colon:
      U_INTERNAL_DUMP("colon: comma = %b colon = %b separator = %b sd[%d].keys = 0x%x", comma, colon, separator, pos, sd[pos].keys)

      if (separator      ||
          colon == false ||
          sd[pos].keys == 0)
         {
         break;
         }

      colon     = false;
      separator = true;

      continue;

case_svector:
      U_INTERNAL_DUMP("svector: comma = %b pos = %d separator = %b sd[%d].tails = %p s = %.10S", comma, pos, separator, pos, sd[pos].tails, s)

      if (*s != ']')
         {
         ++pos;

         U_INTERNAL_ASSERT_MINOR(pos, U_JSON_PARSE_STACK_SIZE)

         sd[pos].keys  = 0;
         sd[pos].tails = 0;
         sd[pos].tags  = false;

         comma     = false;
         separator = true;

         continue;
         }

      ++s;

      o.ival = listToValue(ARRAY_VALUE, 0);

      goto next;

case_evector:
      U_INTERNAL_DUMP("evector: comma = %b separator = %b sd[%d].tags = %b", comma, separator, pos, sd[pos].tags)

      if (comma     ||
          pos == -1 ||
          sd[pos].tags)
         {
         break;
         }

      o.ival = listToValue(ARRAY_VALUE, sd[pos--].tails);

      goto next;

case_false:
      if (u_get_unalignedp32(s) == U_MULTICHAR_CONSTANT32('a','l','s','e'))
         {
         o.ival = getJsonValue(FALSE_VALUE, 0);

         s = start+U_CONSTANT_SIZE("false");

         goto next;
         }

      break;   

case_null:
      if (u_get_unalignedp32(start) == U_MULTICHAR_CONSTANT32('n','u','l','l'))
         {
         o.ival = getJsonValue(NULL_VALUE, 0);

         s = start+U_CONSTANT_SIZE("null");

         goto next;
         }

      break;

case_true:
      if (u_get_unalignedp32(start) == U_MULTICHAR_CONSTANT32('t','r','u','e'))
         {
         o.ival = getJsonValue(TRUE_VALUE, 0);

         s = start+U_CONSTANT_SIZE("true");

         goto next;
         }

      break;

case_sobject:
      U_INTERNAL_DUMP("sobject: comma = %b pos = %d colon = %b separator = %b", comma, pos, colon, separator)

      if (*s != '}')
         {
         ++pos;

         U_INTERNAL_ASSERT_MINOR(pos, U_JSON_PARSE_STACK_SIZE)

         sd[pos].keys  = 0;
         sd[pos].tails = 0;
         sd[pos].tags  = true;

         comma     = false;
         separator = true;

         continue;
         }

      ++s;

      o.ival = listToValue(OBJECT_VALUE, 0);

      goto next;

case_eobject:
      U_INTERNAL_DUMP("eobject: comma = %b colon = %b separator = %b sd[%d].tags = %b sd[%d].tails = %p separator = %b", comma, colon, separator, pos, sd[pos].tags, pos, sd[pos].tails)

      if (comma        ||
          pos == -1    ||
          sd[pos].keys ||
          sd[pos].tags == false)
         {
         break;
         }

      o.ival = listToValue(OBJECT_VALUE, sd[pos--].tails);

next: U_INTERNAL_DUMP("next: comma = %b pos = %d colon = %b separator = %b s = %.10S", comma, pos, colon, separator, s)

      if (pos == -1)
         {
         value.ival = o.ival;

         while (u__isspace(*s)) ++s;

         if (s >= end)
            {
            U_INTERNAL_DUMP("cnt_real = %u cnt_mreal = %u next = %p", cnt_real, cnt_mreal, next)

            U_RETURN(true);
            }

         U_RETURN(false);
         }

      comma     =
      separator = false;

      U_DUMP("sd[%d].tags = (%d,%S) sd[%d].tails = %p", pos, (sd[pos].tags ? OBJECT_VALUE : ARRAY_VALUE),
                                      getDataTypeDescription((sd[pos].tags ? OBJECT_VALUE : ARRAY_VALUE)), pos, sd[pos].tails)

      if (sd[pos].tags == false) sd[pos].tails = insertAfter(sd[pos].tails, o.ival);
      else
         {
         if (colon) break;

         if (sd[pos].keys == 0)
            {
            sd[pos].keys = o.ival;

            colon = true;

            continue;
            }

         sd[pos].tails = insertAfter(sd[pos].tails, o.ival);

         if (isStringOrUTF(sd[pos].keys) == false) break;

         sd[pos].tails->pkey.ival = sd[pos].keys;
                                    sd[pos].keys = 0;
         }
      }

cdefault:
   U_INTERNAL_DUMP("cdefault: pos = %d sd[0].tags = %b sd[0].tails = %p sd[0].keys = 0x%x", pos, sd[0].tags, sd[0].tails, sd[0].keys)

   if (pos >= 0)
      {
      if (sd[0].tags == false) value.ival = (sd[0].tails ? listToValue(ARRAY_VALUE, sd[0].tails) : o.ival);
      else
         {
         if (sd[0].tails) value.ival = listToValue(OBJECT_VALUE, sd[0].tails);
         else
            {
            if (sd[0].keys &&
                isStringOrUTF(sd[0].keys))
               {
               rep = (UStringRep*)getPayload(sd[0].keys);

               U_INTERNAL_DUMP("rep(%p) = %V", rep, rep)

               rep->release();
               }
            }
         }
      }

   U_RETURN(false);
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

#define U_JR_EOL     (NULL_VALUE+1) // 10 end of input string (ptr at '\0')
#define U_JR_COLON   (NULL_VALUE+2) // 11 ":"
#define U_JR_COMMA   (NULL_VALUE+3) // 12 ","
#define U_JR_EARRAY  (NULL_VALUE+4) // 13 "]"
#define U_JR_QPARAM  (NULL_VALUE+5) // 14 "*" query string parameter
#define U_JR_EOBJECT (NULL_VALUE+6) // 15 "}"

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
      case '9': U_RETURN(REAL_VALUE);

      case 't': U_RETURN( TRUE_VALUE);
      case 'f': U_RETURN(FALSE_VALUE);

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
      case 'n': (void) tok.skipToken(U_CONSTANT_TO_PARAM("ull"));  U_RETURN( NULL_VALUE);
      case 't': (void) tok.skipToken(U_CONSTANT_TO_PARAM("rue"));  U_RETURN( TRUE_VALUE);
      case 'f': (void) tok.skipToken(U_CONSTANT_TO_PARAM("alse")); U_RETURN(FALSE_VALUE);

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

         U_RETURN(REAL_VALUE);
         }

      case '"':
         {
         const char* ptr  = tok.getPointer();
         const char* end  = tok.getEnd();
         const char* last = u_find_char(ptr, end, '"');

         if (last < end)
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
   const char* end  = tok.getEnd();
   const char* last = u_find_char(ptr, end, c);
   uint32_t sz      = (last < end ? last - ptr : 0);

   U_INTERNAL_DUMP("c = %C sz = %u", c, sz)

   if (sz) (void) result.assign(ptr, sz);

   if (last < end) tok.setPointer(last+1);

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

      U_DUMP("jTok = (%d,%S)", jTok, getDataTypeDescription(jTok))

      if (jTok != U_JR_COLON)
         {
         jread_error = 4; // Expected ":"

         break;
         }

      if (jread_skip(++tok) == -1) break;

      ++jread_elements;

      U_INTERNAL_DUMP("jread_elements = %u", jread_elements)

      jTok = jreadFindToken(tok);

      U_DUMP("jTok = (%d,%S)", jTok, getDataTypeDescription(jTok))

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

      U_DUMP("jTok = (%d,%S)", jTok, getDataTypeDescription(jTok))

      if (jTok != U_JR_COLON)
         {
         jread_error = 4; // Expected ":"

         break;
         }

      if (jread_skip(++tok) == -1) break;

      ++jread_elements;

      U_INTERNAL_DUMP("jread_elements = %u", jread_elements)

      jTok = jreadFindToken(tok);

      U_DUMP("jTok = (%d,%S)", jTok, getDataTypeDescription(jTok))

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

   U_DUMP("jTok = (%d,%S)", jTok, getDataTypeDescription(jTok))
   U_DUMP("qTok = (%d,%S)", qTok, getDataTypeDescription(qTok))

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

      case  NULL_VALUE: // null
      case  REAL_VALUE: // number
      case  TRUE_VALUE: // true
      case FALSE_VALUE: // false
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

         U_DUMP("qTok = (%d,%S)", qTok, getDataTypeDescription(qTok))

         if (qTok != STRING_VALUE)
            {
            index = 0;

            switch (qTok)
               {
               case REAL_VALUE: // index value
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

            U_DUMP("jTok = (%d,%S)", jTok, getDataTypeDescription(jTok))

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

            U_DUMP("jTok = (%d,%S)", jTok, getDataTypeDescription(jTok))

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

               U_DUMP("jTok = (%d,%S)", jTok, getDataTypeDescription(jTok))

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

         U_DUMP("qTok = (%d,%S)", qTok, getDataTypeDescription(qTok))

         if (qTok == U_JR_QPARAM)
            {
            ++tok2;

            index = (queryParams ? *queryParams++ : 0); // substitute parameter
            }
         else if (qTok == REAL_VALUE)
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

            U_DUMP("jTok = (%d,%S)", jTok, getDataTypeDescription(jTok))

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

   U_DUMP("qTok = (%d,%S)", qTok, getDataTypeDescription(qTok))

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

   U_DUMP("jTok = (%d,%S) result = %V jread_pos = %u", jTok, getDataTypeDescription(jTok), result.rep, jread_pos)

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

      U_DUMP("sTok = (%d,%S)", sTok, getDataTypeDescription(sTok))

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

   U_DUMP("jTok = (%d,%S)", jTok, getDataTypeDescription(jTok))

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
      U_ENTRY(REAL_VALUE),
      U_ENTRY(INT_VALUE),
      U_ENTRY(UINT_VALUE),
      U_ENTRY(TRUE_VALUE),
      U_ENTRY(FALSE_VALUE),
      U_ENTRY(STRING_VALUE),
      U_ENTRY(UTF_VALUE),
      U_ENTRY(ARRAY_VALUE),
      U_ENTRY(OBJECT_VALUE),
      U_ENTRY(NULL_VALUE),
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
   *UObjectIO::os << "next       " << next      << '\n'
                  << "pkey.ival  " << pkey.ival << '\n'
                  << "value.ival " << value.ival;

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

#endif
