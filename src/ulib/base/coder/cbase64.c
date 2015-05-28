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

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/utility.h>
#include <ulib/base/coder/base64.h>

#include <ctype.h>

#define PAD '='

int u_base64_errors;
int u_base64_max_columns;

// u_alphabet: "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
//                                  base64url substitute 62 and 63 chars with -_ (minus) (underline)

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
   static const int dispatch_table[] = {
      0,
      0,
      (char*)&&case_2-(char*)&&case_1,
      (char*)&&case_3-(char*)&&case_1
   };

   /**
    * for (int i = 0; i < sizeof(u_alphabet); ++i) { member[u_alphabet[i]] = 1; decoder[u_alphabet[i]] = i; }
    */

   static const unsigned char member[256] = {
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /*  15 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /*  31 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 001, 000, 000, 000, 001, /*  47 */
      001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 000, 000, 000, 000, 000, 000, /*  63 */
      000, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, /*  79 */
      001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 000, 000, 000, 000, 000, /*  95 */
      000, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, /* 111 */
      001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 000, 000, 000, 000, 000, /* 127 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* 143 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* ... */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000
   };

   static const unsigned char decoder[256] = {
      '@', 000, 000, 000, 000, 000, 000, 000, 000, 000, /*   9 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /*  19 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /*  29 */ 
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /*  39 */
      000, 000, 000, 076, 000, 000, 000, 077, 064, 065, /*  49 */
      066, 067, 070, 071, 072, 073, 074, 075, 000, 000, /*  59 */
      000, 000, 000, 000, 000, 000, 001, 002, 003, 004, /*  69 */
      005, 006, 007, 010, 011, 012, 013, 014, 015, 016, /*  79 */
      017, 020, 021, 022, 023, 024, 025, 026, 027, 030, /*  89 */
      031, 000, 000, 000, 000, 000, 000, 032, 033, 034, /*  99 */
      035, 036, 037, 040, 041, 042, 043, 044, 045, 046, /* 109 */
      047, 050, 051, 052, 053, 054, 055, 056, 057, 060, /* 119 */
      061, 062, 063, 000, 000, 000, 000, 000, 000, 000, /* 129 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* 139 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
      000, 000, 000, 000, 000, 000
   };

   char c;
   uint32_t input_len, i = 0;
   int char_count = 0, bits = 0;
   const    char* restrict ptr = input;
   unsigned char* restrict r   = result;
   const    char* restrict end = input + len;

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
       *             | byte 1 | byte 2 | byte 3 | byte 4 |
       * Encoded block 654321   654321   654321   654321  -> 4 bytes of 6 bits
       *             | byte 1 | byte 2 | byte 3 |
       * Decoded block 65432165 43216543 21654321         -> 3 bytes of 8 bits
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

      goto next;
      }

   U_INTERNAL_ASSERT_RANGE(1, char_count, 3)

   goto *((char*)&&case_1 + dispatch_table[char_count]);

case_1:
   ++u_base64_errors;

   U_INTERNAL_PRINT("Decoding incomplete: at least 2 bits missing", 0)

   goto next;

case_2:
   *r++ = bits >> 10;

   goto next;

case_3:
   *r++ =  bits >> 16;
   *r++ = (bits >>  8) & 0xff;

next:
   *r = 0;

   return (r - result);
}

uint32_t u_base64url_encode(const unsigned char* restrict input, uint32_t len, unsigned char* restrict result)
{
   uint32_t i;
   bool columns = false;
   unsigned char* restrict r = result;
   int char_count = 0, bits = 0, cols = 0;

   static const unsigned char* b64url = (const unsigned char*) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

   U_INTERNAL_TRACE("u_base64url_encode(%.*s,%u,%p)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   for (i = 0; i < len; ++i)
      {
      bits += input[i];

      if (++char_count != 3) bits <<= 8;
      else
         {
         *r++ = b64url[ bits >> 18];
         *r++ = b64url[(bits >> 12) & 0x3f];
         *r++ = b64url[(bits >>  6) & 0x3f];
         *r++ = b64url[ bits        & 0x3f];

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

      *r++ = b64url[ bits >> 18];
      *r++ = b64url[(bits >> 12) & 0x3f];

      if (char_count == 2) *r++ = b64url[(bits >> 6) & 0x3f];
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

uint32_t u_base64url_decode(const char* restrict input, uint32_t len, unsigned char* restrict result)
{
   static const int dispatch_table[] = {
      0,
      0,
      (char*)&&case_2-(char*)&&case_1,
      (char*)&&case_3-(char*)&&case_1
   };

   static const unsigned char member[256] = {
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /*  15 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /*  31 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 001, 000, 000, /*  47 */
      001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 000, 000, 000, 000, 000, 000, /*  63 */
      000, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, /*  79 */
      001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 000, 000, 000, 000, 001, /*  95 */
      001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, /* 111 */
      001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 000, 000, 000, 000, 000, /* 127 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* 143 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* ... */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* ... */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* ... */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* ... */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* ... */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* ... */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000
   };

   static const unsigned char decoder[256] = {
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*   9 */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*  19 */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*  29 */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*  39 */
     0,   0,   0,   0,   0,  62,   0,   0,  52,  53, /*  49 */
    54,  55,  56,  57,  58,  59,  60,  61,   0,   0, /*  59 */
     0,   0,   0,   0,   0,   0,   1,   2,   3,   4, /*  69 */
     5,   6,   7,   8,   9,  10,  11,  12,  13,  14, /*  79 */
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24, /*  89 */
    25,   0,   0,   0,   0,  63,   0,  26,  27,  28, /*  99 */
    29,  30,  31,  32,  33,  34,  35,  36,  37,  38, /* 109 */
    39,  40,  41,  42,  43,  44,  45,  46,  47,  48, /* 119 */
    49,  50,  51,   0,   0,   0,   0,   0,   0,   0, /* 129 */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* 139 */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0
   };

   char c;
   uint32_t input_len, i = 0;
   int char_count = 0, bits = 0;
   const    char* restrict ptr = input;
   unsigned char* restrict r   = result;
   const    char* restrict end = input + len;

   U_INTERNAL_TRACE("u_base64url_decode(%.*s,%u,%p)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   u_base64_errors = 0;

   while (ptr < end)
      {
      c = *ptr;

      if (u__isspace(c) ||
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

      if (member[(int)c] == 0) continue;

      if (u__isb64url(c) == false)
         {
         U_INTERNAL_PRINT("Decoding error: not base64url encoding", 0)

         return 0;
         }

      bits += decoder[(int)c];

      /**
       * Bit positions
       *
       *             | byte 1 | byte 2 | byte 3 | byte 4 |
       * Encoded block 654321   654321   654321   654321  -> 4 bytes of 6 bits
       *             | byte 1 | byte 2 | byte 3 |
       * Decoded block 65432165 43216543 21654321         -> 3 bytes of 8 bits
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

   if (char_count == 0) goto next;

   U_INTERNAL_ASSERT_RANGE(1, char_count, 3)

   goto *((char*)&&case_1 + dispatch_table[char_count]);

case_1:
   ++u_base64_errors;

   U_INTERNAL_PRINT("Decoding incomplete: at least 2 bits missing", 0)

   goto next;

case_2:
   *r++ = bits >> 10;

   goto next;

case_3:
   *r++ =  bits >> 16;
   *r++ = (bits >>  8) & 0xff;

next:
   *r = 0;

   return (r - result);
}

uint32_t u_base64all_decode(const char* restrict input, uint32_t len, unsigned char* restrict result)
{
   static const int dispatch_table[] = {
      0,
      0,
      (char*)&&case_2-(char*)&&case_1,
      (char*)&&case_3-(char*)&&case_1
   };

   static const unsigned char member[256] = {
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /*  15 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /*  31 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 001, 000, 001, 000, 001, /*  47 */
      001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 000, 000, 000, 000, 000, 000, /*  63 */
      000, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, /*  79 */
      001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 000, 000, 000, 000, 001, /*  95 */
      001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, /* 111 */
      001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 001, 000, 000, 000, 000, 000, /* 127 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* 143 */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* ... */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* ... */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* ... */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* ... */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* ... */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, /* ... */
      000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000, 000
   };

   static const unsigned char decoder[256] = {
   '@',   0,   0,   0,   0,   0,   0,   0,   0,   0, /*   9 */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*  19 */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*  29 */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /*  39 */
     0,   0,   0, 076,   0,  62,   0, 077,  52,  53, /*  49 */
    54,  55,  56,  57,  58,  59,  60,  61,   0,   0, /*  59 */
     0,   0,   0,   0,   0,   0,   1,   2,   3,   4, /*  69 */
     5,   6,   7,   8,   9,  10,  11,  12,  13,  14, /*  79 */
    15,  16,  17,  18,  19,  20,  21,  22,  23,  24, /*  89 */
    25,   0,   0,   0,   0,  63,   0,  26,  27,  28, /*  99 */
    29,  30,  31,  32,  33,  34,  35,  36,  37,  38, /* 109 */
    39,  40,  41,  42,  43,  44,  45,  46,  47,  48, /* 119 */
    49,  50,  51,   0,   0,   0,   0,   0,   0,   0, /* 129 */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* 139 */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0,   0,   0,   0,   0, /* ... */
     0,   0,   0,   0,   0,   0
   };

   char c;
   uint32_t input_len, i = 0;
   int char_count = 0, bits = 0;
   const    char* restrict ptr = input;
   unsigned char* restrict r   = result;
   const    char* restrict end = input + len;

   U_INTERNAL_TRACE("u_base64all_decode(%.*s,%u,%p)", U_min(len,128), input, len, result)

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

      bits += decoder[(int)c];

      /**
       * Bit positions
       *
       *             | byte 1 | byte 2 | byte 3 | byte 4 |
       * Encoded block 654321   654321   654321   654321  -> 4 bytes of 6 bits
       *             | byte 1 | byte 2 | byte 3 |
       * Decoded block 65432165 43216543 21654321         -> 3 bytes of 8 bits
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

   U_INTERNAL_ASSERT_RANGE(1, char_count, 3)

   goto *((char*)&&case_1 + dispatch_table[char_count]);

case_1:
   ++u_base64_errors;

   U_INTERNAL_PRINT("Decoding incomplete: at least 2 bits missing", 0)

   goto next;

case_2:
   *r++ = bits >> 10;

   goto next;

case_3:
   *r++ =  bits >> 16;
   *r++ = (bits >>  8) & 0xff;

next:
   *r = 0;

   return (r - result);
}
