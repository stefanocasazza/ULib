// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    cxml_coder.c
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/utility.h>
#include <ulib/base/coder/xml.h>

/**
 * -------------------------------------------------------------------
 * Encoding to escape and unescape character to valid CDATA characters
 * -------------------------------------------------------------------
 *  Character  XML escape sequence  name
 * -------------------------------------------------------------------
 *    '"'        "&quot;"           quote
 *    '&'        "&amp;"            amp
 *    '\''       "&apos;"           apostrophe
 *    '<'        "&lt;"             lower than
 *    '>'        "&gt;"             greater than
 * -------------------------------------------------------------------
 */

static inline unsigned char* u_set_quot(unsigned char* restrict r)
{
   u_put_unalignedp32(r,   U_MULTICHAR_CONSTANT32('&','q','u','o'));
   u_put_unalignedp16(r+4, U_MULTICHAR_CONSTANT16('t',';'));

   return r + U_CONSTANT_SIZE("&quot;");
}

static inline unsigned char* u_set_amp(unsigned char* restrict r)
{
   u_put_unalignedp32(r, U_MULTICHAR_CONSTANT32('&','a','m','p'));

   r[4] = ';';

   return r + U_CONSTANT_SIZE("&amp;");
}

static inline unsigned char* u_set_apos(unsigned char* restrict r)
{
   u_put_unalignedp32(r, U_MULTICHAR_CONSTANT32('&','a','p','o'));
   u_put_unalignedp16(r+4, U_MULTICHAR_CONSTANT16('s',';'));

   return r + U_CONSTANT_SIZE("&apos;");
}

static inline unsigned char* u_set_lt(unsigned char* restrict r)
{
   u_put_unalignedp32(r, U_MULTICHAR_CONSTANT32('&','l','t',';'));

   return r + U_CONSTANT_SIZE("&lt;");
}

static inline unsigned char* u_set_gt(unsigned char* restrict r)
{
   u_put_unalignedp32(r, U_MULTICHAR_CONSTANT32('&','g','t',';'));

   return r + U_CONSTANT_SIZE("&gt;");
}

uint32_t u_xml_encode(const unsigned char* restrict input, uint32_t len, unsigned char* restrict result)
{
         unsigned char* restrict r   = result;
   const unsigned char* restrict end = input + len;

   U_INTERNAL_TRACE("u_xml_encode(%.*s,%u,%p,%d)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   while (input < end)
      {
      unsigned char ch = *input++;

      U_INTERNAL_PRINT("ch = %C *input = %C", ch, *input)

      /**
       * From xml spec valid chars:
       *
       * #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD]
       *                                 | [#x10000-#x10FFFF]     
       *
       * any Unicode character, excluding the surrogate blocks, FFFE, and FFFF
       */

      if (u__isxmlvalidchar(ch) == false) continue; 

      if ( u__isalnum(ch)          ||
          (u__isquote(ch) == false &&
           u__ishtmlc(ch) == false))
         {
         if (ch != '\\') *r++ = ch;

         U_INTERNAL_PRINT("r = %.20s", r-1)

         continue;
         }

      switch (ch)
         {
         case '<': r = u_set_lt(r); break;
         case '>': r = u_set_gt(r); break;
         case '&':
            {
            /**
             * skip if already encoded... hexadecimal or decimal numerical
             * entities and named entities (&amp; &lt; &gt; &quot; &apos;)
             */

            if (      *input == '#'                              ||
                memcmp(input, U_CONSTANT_TO_PARAM("quot;")) == 0 ||
                memcmp(input, U_CONSTANT_TO_PARAM("amp;"))  == 0 ||
                memcmp(input, U_CONSTANT_TO_PARAM("lt;"))   == 0 ||
                memcmp(input, U_CONSTANT_TO_PARAM("gt;"))   == 0 ||
                memcmp(input, U_CONSTANT_TO_PARAM("apos;")) == 0)
               {
               *r++ = '&';

               while ((*r = *input) != ';')
                  {
                  ++r;
                  ++input;
                  }
               }
            else
               {
               r = u_set_amp(r);
               }
            }
         break;

         default:
            {
            U_INTERNAL_ASSERT(u__isquote(ch))

            r = (ch == '"' ? u_set_quot(r)
                           : u_set_apos(r));
            }
         }
      }

   *r = 0;

   U_INTERNAL_PRINT("result = %s", result)

   return (r - result);
}

uint32_t u_xml_decode(const char* restrict input, uint32_t len, unsigned char* restrict result)
{
         unsigned char* restrict r   = result;
   const          char* restrict end = input + len;

   U_INTERNAL_TRACE("u_xml_decode(%.*s,%u,%p,%lu)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   while (input < end)
      {
      char ch = *input++;

      if (ch != '&') *r++ = ch;
      else
         {
         /* check for hexadecimal or decimal numerical entities and named entities (&amp; &lt; &gt; &quot; &apos;) */

         if (memcmp(input, U_CONSTANT_TO_PARAM("amp;")) == 0)       /* '&' <-> &amp; */
            {
            *r++   = '&';
            input += U_CONSTANT_SIZE("amp;");
            }
         else if (memcmp(input, U_CONSTANT_TO_PARAM("lt;")) == 0)   /* '<' <-> &lt; */
            {
            *r++   = '<';
            input += U_CONSTANT_SIZE("lt;");
            }
         else if (memcmp(input, U_CONSTANT_TO_PARAM("gt;")) == 0)   /* '>' <-> &gt; */
            {
            *r++   = '>';
            input += U_CONSTANT_SIZE("gt;");
            }
         else if (memcmp(input, U_CONSTANT_TO_PARAM("quot;")) == 0) /* '"' <-> &quot; */
            {
            *r++   = '"';
            input += U_CONSTANT_SIZE("quot;");
            }
         else if (memcmp(input, U_CONSTANT_TO_PARAM("apos;")) == 0) /* '\'' <-> &apos; */
            {
            *r++   = '\'';
            input += U_CONSTANT_SIZE("apos;");
            }
         else if (*input == '#')
            {
            /*
            union uuarg {
                              char* p;
               const          char* cp;
               const unsigned char* cup;
            };

            union uuarg i = { ++input };

            *r++ = (unsigned char) strtol(i.cp, &i.p, 10);
            */

            ++input;

            *r++ = (unsigned char) strtol((const char*)input, (char**)&input, 10);

            ++input;
            }
         else
            {
            *r++ = ch;
            }
         }
      }

   *r = 0;

   return (r - result);
}
