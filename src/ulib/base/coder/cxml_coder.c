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

#include <ulib/base/utility.h>
#include <ulib/base/coder/xml.h>

/**
 * -------------------------------------------------------------------
 * Encoding to escape and unescape character to valid CDATA characters
 * -------------------------------------------------------------------
 *  Character  XML escape sequence  name
 * -------------------------------------------------------------------
 *  '"'          "&quot;"           quote
 *  '&'          "&amp;"            amp
 *  '\''         "&apos;"           apostrophe
 *  '<'          "&lt;"             lower than
 *  '>'          "&gt;"             greater than
 * -------------------------------------------------------------------
 */

static inline unsigned char* u_set_quot(unsigned char* restrict r)
{
   *r++ = '&';
   *r++ = 'q';
   *r++ = 'u';
   *r++ = 'o';
   *r++ = 't';
   *r++ = ';';

   return r;
}

static inline unsigned char* u_set_amp(unsigned char* restrict r)
{
   *r++ = '&';
   *r++ = 'a';
   *r++ = 'm';
   *r++ = 'p';
   *r++ = ';';

   return r;
}

static inline unsigned char* u_set_apos(unsigned char* restrict r)
{
   *r++ = '&';
   *r++ = 'a';
   *r++ = 'p';
   *r++ = 'o';
   *r++ = 's';
   *r++ = ';';

   return r;
}

static inline unsigned char* u_set_lt(unsigned char* restrict r)
{
   *r++ = '&';
   *r++ = 'l';
   *r++ = 't';
   *r++ = ';';

   return r;
}

static inline unsigned char* u_set_gt(unsigned char* restrict r)
{
   *r++ = '&';
   *r++ = 'g';
   *r++ = 't';
   *r++ = ';';

   return r;
}

uint32_t u_xml_encode(const unsigned char* restrict input, uint32_t len, unsigned char* restrict result)
{
         unsigned char ch;
         unsigned char* restrict r   = result;
   const unsigned char* restrict end = input + len;

   U_INTERNAL_TRACE("u_xml_encode(%.*s,%u,%p,%d)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   while (input < end)
      {
      ch = *input++;

      if ( u__isalnum(ch)          ||
          (u__isquote(ch) == false &&
           u__ishtmlc(ch) == false))
         {
         *r++ = ch;

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

   return (r - result);
}

uint32_t u_xml_decode(const char* restrict input, uint32_t len, unsigned char* restrict result)
{
                  char ch;
         unsigned char* restrict r   = result;
   const          char* restrict end = input + len;

   U_INTERNAL_TRACE("u_xml_decode(%.*s,%u,%p,%lu)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   while (input < end)
      {
      ch = *input++;

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

            *r++ = (unsigned char) strtol(i.cp, &i.p, 0);
            */

            ++input;

            *r++ = (unsigned char) strtol((const char*)input, (char**)&input, 0);

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
