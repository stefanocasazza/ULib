// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    cescape.c
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/utility.h>
#include <ulib/base/coder/escape.h>

uint32_t u_escape_encode(const unsigned char* restrict inptr, uint32_t len, char* restrict out, uint32_t max_output)
{
   const unsigned char* restrict inend  = inptr + len;
                  char* restrict outptr = out;
                  char* restrict outend = out + (max_output - 4);

   U_INTERNAL_TRACE("u_escape_encode(%.*s,%u,%p,%u)", U_min(len,128), inptr, len, out, max_output)

   U_INTERNAL_ASSERT_POINTER(out)
   U_INTERNAL_ASSERT_POINTER(inptr)

   *outptr++ = '"';

   while (inptr < inend)
      {
      outptr += u_sprintc(outptr, *inptr++);

      if (outptr >= outend)
         {
         *outptr++ = '.';
         *outptr++ = '.';
         *outptr++ = '.';

         break;
         }
      }

   *outptr++ = '"';
   *outptr   = 0;

   return (outptr - out);
}

/**
 * --------------------------------------------------------------------
 * Decode escape sequences into a buffer, the following are recognized:
 * --------------------------------------------------------------------
 *  \a  BEL                 (\007  7  7)
 *  \b  BS  backspace       (\010  8  8) 
 *  \t  HT  horizontal tab  (\011  9  9)
 *  \n  LF  newline         (\012 10  A) 
 *  \v  VT  vertical tab    (\013 11  B)
 *  \f  FF  formfeed        (\014 12  C) 
 *  \r  CR  carriage return (\015 13  D)
 *  \e  ESC character       (\033 27 1B)
 *
 *  \u    four-hex-digits (unicode char)
 *  \^C   C = any letter (Control code)
 *  \xDD  number formed of 1-2 hex   digits
 *  \DDD  number formed of 1-3 octal digits
 * ---------------------------------------------------------------------------
 */

uint32_t u_escape_decode(const char* restrict inptr, uint32_t len, unsigned char* restrict out)
{
   int c;
   char* p;
   const    char* restrict inend  = inptr + len;
   unsigned char* restrict outptr = out;

   U_INTERNAL_TRACE("u_escape_decode(%.*s,%u,%p)", U_min(len,128), inptr, len, out)

   U_INTERNAL_ASSERT_POINTER(out)
   U_INTERNAL_ASSERT_POINTER(inptr)

   while (inptr < inend)
      {
      U_INTERNAL_PRINT("inptr = %.16s", inptr)

      if (*inptr == '\\') p = (char*)inptr;
      else
         {
         p = (char* restrict) memchr(inptr, '\\', inend - inptr);

         len = (p ? p : inend) - inptr;

         U_INTERNAL_PRINT("len = %u", len)

         U_INTERNAL_ASSERT_MAJOR(len, 0)

         u__memcpy(outptr, inptr, len, __PRETTY_FUNCTION__);

         outptr += len;

         if (p == 0) break;
         }

      /* \u four-hex-digits (unicode char) */

      if ( p[1] == 'u' &&
          (p[2] != '0' ||
           p[3] != '0'))
         {
         u__memcpy(outptr, p, 6, __PRETTY_FUNCTION__);

         inptr   = p+6;
         outptr += 6;

         continue;
         }

           inptr = p+1;
      c = *inptr++;

      U_INTERNAL_PRINT("c = %d", c)

      switch (c)
         {
         case 'a': c = '\a';   break;
         case 'b': c = '\b';   break;
         case 't': c = '\t';   break;
         case 'n': c = '\n';   break;
         case 'v': c = '\v';   break;
         case 'f': c = '\f';   break;
         case 'r': c = '\r';   break;
         case 'e': c = '\033'; break;

         /* check control code */

         case '^': c = u__toupper(*inptr++) - '@'; break;

         /* check sequenza escape esadecimale */

         case 'x':
            {
            if (u__isxdigit(*inptr))
               {
                                        c =            u__hexc2int(*inptr++);
               if (u__isxdigit(*inptr)) c = (c << 4) | u__hexc2int(*inptr++);
               }
            }
         break;

         /* check sequenza escape ottale */

         case '0': case '1': case '2': case '3':
         case '4': case '5': case '6': case '7':
            {
            c -= '0';

            if (u__isoctal(*inptr))
               {
                                       c = (c << 3) | u__octc2int(*inptr++);
               if (u__isoctal(*inptr)) c = (c << 3) | u__octc2int(*inptr++);
               }
            }
         break;

         /* \u four-hex-digits (unicode char) */

         case 'u':
            {
            U_INTERNAL_ASSERT_EQUALS(inptr[0], '0')
            U_INTERNAL_ASSERT_EQUALS(inptr[1], '0')

            inptr += 2;

            U_INTERNAL_ASSERT(u__isxdigit(inptr[0]))
            U_INTERNAL_ASSERT(u__isxdigit(inptr[1]))

            c = ((u__hexc2int(inptr[0]) & 0x0F) << 4) |
                 (u__hexc2int(inptr[1]) & 0x0F);

            inptr += 2;
            }
         break;
         }

      *outptr++ = c;
      }

   *outptr = 0;

   return (outptr - out);
}
