/* ============================================================================
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
// ============================================================================ */

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
         u_put_unalignedp32(outptr, U_MULTICHAR_CONSTANT32('.','.','.','"')); // to be continued...

         outptr[4] = '\0';

         return (outptr - out) + 4;
         }
      }

   *outptr++ = '"';
   *outptr   = 0;

   return (outptr - out);
}

/**
 * Decode escape sequences into a buffer, the following are recognized:
 * --------------------------------------------------------------------
 *  \b  BS  backspace       (\010  8  8) 
 *  \t  HT  horizontal tab  (\011  9  9)
 *  \n  LF  newline         (\012 10  A) 
 *  \f  FF  formfeed        (\014 12  C) 
 *  \r  CR  carriage return (\015 13  D)
 *  \a  BEL                 (\007  7  7)
 *  \v  VT  vertical tab    (\013 11  B)
 *  \e  ESC character       (\033 27 1B)
 * --------------------------------------------------------------------
 *  \^C  C = any letter (Control code)
 *  \u   four hex digits (unicode char)
 *  \xDD number formed of 1-2 hex   digits
 *  \DDD number formed of 1-3 octal digits
 * --------------------------------------------------------------------
 */

uint32_t u_escape_decode(const char* restrict inptr, uint32_t len, unsigned char* restrict out)
{
   uint32_t c;
         unsigned char* restrict p;
   const unsigned char* restrict inend  = (const unsigned char* restrict)inptr + len;
         unsigned char* restrict outptr = out;

   U_INTERNAL_TRACE("u_escape_decode(%.*s,%u,%p)", U_min(len,128), inptr, len, out)

   U_INTERNAL_ASSERT_POINTER(out)
   U_INTERNAL_ASSERT_POINTER(inptr)

   while ((const unsigned char* restrict)inptr < inend)
      {
      U_INTERNAL_PRINT("inptr = \"%.*s\"", (int)(long)(inend-(const unsigned char* restrict)inptr), inptr)

      if (*inptr == '\\') p = (unsigned char* restrict)inptr;
      else
         {
         p = (unsigned char* restrict) memchr(inptr, '\\', inend - (const unsigned char* restrict)inptr);

         len = (p ? p : inend) - (const unsigned char* restrict)inptr;

         U_INTERNAL_PRINT("len = %u", len)

         U_INTERNAL_ASSERT_MAJOR(len, 0)

         u__memcpy(outptr, inptr, len, __PRETTY_FUNCTION__);

         outptr += len;

         if (p == 0) break;
         }

      inptr = (const char* restrict)p+1;
          c = *inptr++;

      U_INTERNAL_PRINT("c = %u", c)

      switch (c)
         {
         case 'b':  c = '\b';   break;
         case 't':  c = '\t';   break;
         case 'n':  c = '\n';   break;
         case 'f':  c = '\f';   break;
         case 'r':  c = '\r';   break;
         case 'a':  c = '\a';   break;
         case 'v':  c = '\v';   break;
         case 'e':  c = '\033'; break;
         case '\\': c = '\\';   break;

         case 'u': /* \u four hex digits (unicode char) */
            {
            U_INTERNAL_ASSERT(u__isxdigit(inptr[0]))
            U_INTERNAL_ASSERT(u__isxdigit(inptr[1]))
            U_INTERNAL_ASSERT(u__isxdigit(inptr[2]))
            U_INTERNAL_ASSERT(u__isxdigit(inptr[3]))

            if (inptr[0] == '0' &&
                inptr[1] == '0')
               {
               c = ((u__hexc2int(inptr[2]) & 0x0F) << 4) |
                    (u__hexc2int(inptr[3]) & 0x0F);

               U_INTERNAL_PRINT("case_u(0): c = %u", c)

               inptr += 4;

               if (c <= 0x7F) break; /* U+0000..U+007F */

               goto next;
               }

            c = u_hex2int(inptr, inptr+4);

            U_INTERNAL_PRINT("case_u(1): c = %u", c)

            U_INTERNAL_ASSERT(c > 0x7F)

            if (c >= 0xD800      &&
                c <= 0xDFFF      &&
                inptr[4] == '\\' &&
                inptr[5] == 'u')
               {
               /* Handle UTF-16 surrogate pair */

               uint32_t lc = u_hex2int(inptr+6, inptr+6+4);

               U_INTERNAL_PRINT("lc = %u", lc)

               if (lc >= 0xDC00 &&
                   lc <= 0xDFFF)
                  {
                  inptr += 6;

                  c = 0x10000 + (((c & 0x3FF) << 10) | (lc & 0x3FF));
                  }
               }

            U_INTERNAL_PRINT("case_u(2): c = %u", c)

            U_INTERNAL_ASSERT(c > 0x7F)

            inptr += 4;

next:       if (c <= 0x7FF) /* U+0080..U+07FF */
               {
               u_put_unalignedp16(outptr, U_MULTICHAR_CONSTANT16((unsigned char)(0xC0 |  c >> 6),
                                                                 (unsigned char)(0x80 | (c & 0x3F))));

               outptr += 2;
               }
            else if (c <= 0xFFFF) /* U+0800..U+FFFF */
               {
               u_put_unalignedp32(outptr, U_MULTICHAR_CONSTANT32((unsigned char)(0xE0 |  c >> 12),
                                                                 (unsigned char)(0x80 | (c >>  6 & 0x3F)),
                                                                 (unsigned char)(0x80 | (c       & 0x3F)), 0));

               outptr += 3;
               }
            else /* U+10000..U+10FFFF */
               {
               U_INTERNAL_ASSERT(c <= 0x10FFFF)

               u_put_unalignedp32(outptr, U_MULTICHAR_CONSTANT32((unsigned char)(0xF0 |  c >> 18),
                                                                 (unsigned char)(0x80 | (c >> 12 & 0x3F)),
                                                                 (unsigned char)(0x80 | (c >>  6 & 0x3F)),
                                                                 (unsigned char)(0x80 | (c       & 0x3F))));

               outptr += 4;
               }

            continue;
            }
         break;

         case '^': /* check control code */
            {
            c = u__toupper(*inptr++) - '@';
            }
         break;

         case 'x': /* check hexadecimal escape sequence */
            {
            if (u__isxdigit(*inptr))
               {
                                        c =            u__hexc2int(*inptr++);
               if (u__isxdigit(*inptr)) c = (c << 4) | u__hexc2int(*inptr++);
               }
            }
         break;

         /* check octal escape sequence */

         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
            {
            c -= '0';

            if (u__isoctal(*inptr))
               {
                                       c = (c << 3) | u__octc2int(*inptr++);
               if (u__isoctal(*inptr)) c = (c << 3) | u__octc2int(*inptr++);
               }
            }
         break;
         }

      *outptr++ = (unsigned char)c;
      }

   *outptr = 0;

   return (outptr - out);
}
