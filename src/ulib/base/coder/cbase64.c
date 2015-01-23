// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    cbase64.c
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/base/utility.h>
#include <ulib/base/coder/base64.h>

#include <ctype.h>

#define PAD '='

int u_base64_errors;
int u_base64_max_columns;

uint32_t u_base64_encode(const unsigned char* restrict input, uint32_t len, unsigned char* restrict result)
{
   uint32_t i;
   bool columns = false;
   unsigned char* restrict r = result;
   int char_count = 0, bits = 0, cols = 0;

   U_INTERNAL_TRACE("u_base64_encode(%.*s,%u,%p)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   for (i = 0; i < len; ++i)
      {
      bits += input[i];

      if (++char_count != 3) bits <<= 8;
      else
         {
         *r++ = u_alphabet[ bits >> 18];
         *r++ = u_alphabet[(bits >> 12) & 0x3f];
         *r++ = u_alphabet[(bits >>  6) & 0x3f];
         *r++ = u_alphabet[ bits        & 0x3f];

         if (u_base64_max_columns)
            {
            cols += 4;

            if (cols == u_base64_max_columns)
               {
               cols    = 0;
               columns = true;

               if (u_line_terminator_len == 2) *r++ = '\r';
                                               *r++ = '\n';
               }
            }

         bits       = 0;
         char_count = 0;
         }
      }

   if (char_count != 0)
      {
      bits <<= (16 - (8 * char_count));

      *r++ = u_alphabet[ bits >> 18];
      *r++ = u_alphabet[(bits >> 12) & 0x3f];

      if (char_count == 1)
         {
         *r++ = PAD;
         *r++ = PAD;
         }
      else
         {
         *r++ = u_alphabet[(bits >> 6) & 0x3f];
         *r++ = PAD;
         }
      }

   if (columns &&
       cols > 0)
      {
      if (u_line_terminator_len == 2) *r++ = '\r';
                                      *r++ = '\n';
      }

   *r = 0;

   return (r - result);
}

uint32_t u_base64_decode(const char* restrict input, uint32_t len, unsigned char* restrict result)
{
   char c;
   uint32_t input_len, i = 0;
   int char_count = 0, bits = 0;
   const    char* restrict ptr = input;
   unsigned char* restrict r   = result;
   const    char* restrict end = input + len;

   /**
    * int struct data
    * --------------------------------------------------------------------------------------------------------
    * for (int i = 0; i < sizeof(u_alphabet); ++i) { member[u_alphabet[i]]  = 1; decoder[u_alphabet[i]] = i; }
    * --------------------------------------------------------------------------------------------------------
    */

   static unsigned char member[256] = {
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 001, 000, 000, 000, 001,
   001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 000, 000, 000, 000, 000, 000,
   000, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001,
   001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 000, 000, 000, 000, 000,
   000, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001,
   001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 };

   static unsigned char decoder[256] = {
   '@', 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 076, 000, 000, 000, 077,
   064, 065, 066, 067, 070, 071, 072, 073, 074, 075, 000, 000, 000, 000, 000, 000,
   000, 000, 001, 002, 003, 004, 005, 006, 007, 010, 011, 012, 013, 014, 015, 016,
   017, 020, 021, 022, 023, 024, 025, 026, 027, 030, 031, 000, 000, 000, 000, 000,
   000, 032, 033, 034, 035, 036, 037, 040, 041, 042, 043, 044, 045, 046, 047, 050,
   051, 052, 053, 054, 055, 056, 057, 060, 061, 062, 063, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
   000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000 };

   U_INTERNAL_TRACE("u_base64_decode(%.*s,%u,%p)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   u_base64_errors = 0;

   while (ptr < end)
      {
      c = *ptr;

      if (c == PAD      ||
          u__isspace(c) ||
          member[(int)c])
         {
         ++ptr;

         continue;
         }

      break;
      }

   input_len = ptr - input;

   U_INTERNAL_PRINT("input_len = %d *ptr = %d", input_len, *ptr)

   for (; i < input_len; ++i)
      {
      c = input[i];

      if (c == PAD) break;

      if (member[(int)c] == 0) continue;

      if (u__isbase64(c) == false)
         {
         U_INTERNAL_PRINT("Decoding error: not base64 encoding", 0)

         return 0;
         }

      bits += decoder[(int)c];

      /**
       * Bit positions
       *
       *                | byte 1 | byte 2 | byte 3 | byte 4 |
       * Encoded block  654321   654321   654321   654321  -> 4 bytes of 6 bits
       *                | byte 1 | byte 2 | byte 3 |
       * Decoded block  65432165 43216543 21654321         -> 3 bytes of 8 bits
       */

      if (++char_count != 4) bits <<= 6;
      else
         {
         *r++ =  bits >> 16;         /* Byte 1 */
         *r++ = (bits >>  8) & 0xff; /* Byte 2 */
         *r++ =  bits        & 0xff; /* Byte 3 */

         bits = char_count = 0;
         }
      }

   if (i == input_len)
      {
      if (char_count)
         {
         ++u_base64_errors;

         U_INTERNAL_PRINT("Decoding incomplete: at least %d bits truncated", (4 - char_count) * 6)
         }
      }
   else
      {
      switch (char_count)
         {
         case 1:
            {
            ++u_base64_errors;

            U_INTERNAL_PRINT("Decoding incomplete: at least 2 bits missing", 0)
            }
         break;

         case 2:
            *r++ = bits >> 10;
         break;

         case 3:
            {
            *r++ =  bits >> 16;
            *r++ = (bits >>  8) & 0xff;
            }
         break;
         }
      }

   *r = 0;

   return (r - result);
}
