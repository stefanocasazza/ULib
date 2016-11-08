/* ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    cquoted_printable.c
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================ */

#include <ulib/base/utility.h>
#include <ulib/base/coder/quoted_printable.h>

#include <ctype.h>

/**
 * Example: Encoded MIME message
 * ------------------------------------------------------------------------------------------------------------
 * From: Jerry Peek <jpeek@jpeek.com>
 * To: carlos@entelfam.cl
 * Subject: Un =?iso-8859-1?Q?d=EDa_dif=EDcil?=
 * MIME-Version: 1.0
 * Content-Type: text/plain; charset=ISO-8859-1
 * Content-Transfer-Encoding: quoted-printable
 * 
 * Carlos, estoy en la casa de mi amigo.  Pero, =A1qu=E9 d=EDa dif=EDcil!
 * Tom=E9 un taxi entre al aeropuerto y el hotel.  "=A1Tenga cuidado!",
 * me dijo el chofer.  "=A1Esta parte de la ciudad es peligrosa!"  Fue
 * evidente para m=ED.  Yo o=EDa que la ciudad tiene partes malas, y esa
 * parte pareci=F3 as=ED.  Los edificios ten=EDan barrotes sobre sus
 * ventanas, hubo gente sospechosa en la calle, y todas las puertas estaban
 * cerrada con llave.  "=A1Vaya con di=F3s!"  =C9l se fue.
 * ------------------------------------------------------------------------------------------------------------
 * quoted-printable is used for text that is mostly 7-bit but which has a small percentage of 8-bit characters.
 * For instance, characters with the eighth bit on in the ISO-8859-n sets should be encoded as quoted-printable.
 * Each 8-bit character is encoded into three 7-bit characters: = (an equal sign) and the hexadecimal value of
 * the character. So the ISO-8859-1 character ñ, which has the hex value F1 (that's 11110001 binary), would be
 * encoded as =F1. To keep the message readable on non-MIME readers, characters that don't have the eighth bit
 * set generally shouldn't be encoded. The = character itself must be encoded, though; it's encoded as =3D. Also,
 * space and tab characters at the ends of lines must be encoded (as =20 and =09, respectively); this keeps broken
 * gateways from eating them. If a line ends with = followed by CR-LF, those characters are ignored; this lets you
 * continue ("wrap") a long line. Lines must be no more than 76 characters long, not counting the final CR-LF. Longer
 * lines will be broken when the message is encoded and joined again by decoding.
 * Quoted-printable text was designed to be (mostly) readable by people with non-MIME mail programs 
 */

uint32_t u_quoted_printable_encode(const unsigned char* restrict inptr, uint32_t len, unsigned char* restrict out)
{
         int n = 0;
         unsigned char ws  = '\0';
         unsigned char* restrict outptr = out;
   const unsigned char* restrict inend  = inptr + len;

   U_INTERNAL_TRACE("u_quoted_printable_encode(%.*s,%u,%p)", U_min(len,128), inptr, len, out)

   U_INTERNAL_ASSERT_POINTER(inptr)

   while (inptr < inend)
      {
      unsigned char ch = *inptr++;

      switch (ch)
         {
         case ' ':
         case '\t':
            {
            if (ws)
               {
               *outptr++ = ws;

               n += 1;
               }

            ws = ch;
            }
         break;

         case '\r':
         case '\n':
            {
            if (ws)
               {
               *outptr++ = '=';
               *outptr++ = "0123456789ABCDEF"[(ws >> 4) & 0xf];
               *outptr++ = "0123456789ABCDEF"[ ws       & 0xf];

               ws = '\0';
               }

            *outptr++ = ch;

            n = 0;
            }
         break;

         default:
            {
            if (ws)
               {
               *outptr++ = ws;

               n += 1;
               ws = '\0';
               }

            if (ch > 126 ||
                ch <  33 ||
                ch == '=')
               {
               *outptr++ = '=';
               *outptr++ = "0123456789ABCDEF"[(ch >> 4) & 0xf];
               *outptr++ = "0123456789ABCDEF"[ ch       & 0xf];

               n += 3;
               }
            else
               {
               *outptr++ = ch;

               n += 1;
               }
            }
         break;
         }

      if (n > 71)
         {
         if (ws)
            {
            *outptr++ = ws;

            ws = '\0';
            }

         u_put_unalignedp16(outptr, U_MULTICHAR_CONSTANT16('=','\n'));
         
         outptr += 2;

         n = 0;
         }
      }

   *outptr = 0;

   return (outptr - out);
}

/* this decodes rfc2047's version of quoted-printable */

uint32_t u_quoted_printable_decode(const char* restrict inptr, uint32_t len, unsigned char* restrict out)
{
            char c1, v;
   const    char* restrict inend  = inptr + len;
   unsigned char* restrict outptr = out;

   U_INTERNAL_TRACE("u_quoted_printable_decode(%.*s,%u,%p)", U_min(len,128), inptr, len, out)

   U_INTERNAL_ASSERT_POINTER(inptr)

   while (inptr < inend)
      {
      char c0 = *inptr;

      if (c0 == '=')
         {
         if ((inend - inptr) < 3)
            {
            /* data was truncated */

            U_WARNING("u_quoted_printable_decode(): Decoding incomplete: data was truncated");

            break;
            }

         c0 = *(inptr+1);
         c1 = *(inptr+2);

         if (u__isxdigit(c0) &&
             u__isxdigit(c1))
            {
            v = ((u__hexc2int(c0) & 0x0F) << 4) |
                 (u__hexc2int(c1) & 0x0F);

            if (v == 0x0D)
               {
               if (*(inptr+3) == '=' &&
                   *(inptr+4) == 0x0A)
                  {
                  *outptr++ = '\n';

                  inptr += (u__isalnum(*(inptr+5)) ? 5 : 6);

                  continue;
                  }
               }

            *outptr++ = v;

            inptr += 3;

            continue;
            }
         else
            {
            if (c0 == '\n')
               {
               inptr += 2;
               
               continue;
               }

            *outptr++ = c0;
            }
         }
      else if (c0 == '_') *outptr++ = ' '; /* _'s are an rfc2047 shortcut for encoding spaces */
      else                *outptr++ = c0;

      ++inptr;
      }

   *outptr = 0;

   return (outptr - out);
}
