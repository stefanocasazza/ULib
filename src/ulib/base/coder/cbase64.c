/* ============================================================================
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
// ============================================================================ */

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/utility.h>
#include <ulib/internal/chttp.h>
#include <ulib/base/coder/base64.h>

#include <ctype.h>

#define U_PAD '='

int u_base64_errors;
int u_base64_max_columns;

uint32_t u_base64_encode(const unsigned char* restrict input, uint32_t len, unsigned char* restrict result)
{
   int cols = 0;
   bool columns = false;
   uint32_t i = 0, bits;
   unsigned char* restrict r = result;

   U_INTERNAL_TRACE("u_base64_encode(%.*s,%u,%p)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   if (len > 2)
      {
      for (; i < len - 2; i += 3)
         {
         bits = ((((input[i]    << 8) +
                    input[i+1]) << 8) +
                    input[i+2]);

         u_put_unalignedp32(r, U_MULTICHAR_CONSTANT32("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[ bits >> 18],
                                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(bits >> 12) & 0x3f],
                                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(bits >>  6) & 0x3f],
                                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[ bits        & 0x3f]));

         r += 4;

         if (u_base64_max_columns)
            {
            cols += 4;

            if (cols == u_base64_max_columns)
               {
               cols    = 0;
               columns = true;

               if (U_line_terminator_len == 2) *r++ = '\r';

               *r++ = '\n';
               }
            }

         }
      }

   switch (len - i)
      {
      case 0: break;
      case 1:
         {
         bits = ((input[i] << 8) << 8);

         u_put_unalignedp32(r, U_MULTICHAR_CONSTANT32("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[ bits >> 18],
                                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(bits >> 12) & 0x3f],
                                                      U_PAD, U_PAD));

         r += 4;
         }
      break;

      default: /* case 2: */
         {
         bits = (((input[i]    << 8) +
                   input[i+1]) << 8);

         u_put_unalignedp32(r, U_MULTICHAR_CONSTANT32("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[ bits >> 18],
                                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(bits >> 12) & 0x3f],
                                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(bits >>  6) & 0x3f],
                                                      U_PAD));

         r += 4;
         }
      }

   if (columns &&
       cols > 0)
      {
      if (U_line_terminator_len == 2) *r++ = '\r';

      *r++ = '\n';
      }

   *r = 0;

   return (r - result);
}

uint32_t u_base64url_encode(const unsigned char* restrict input, uint32_t len, unsigned char* restrict result)
{
   uint32_t i = 0, bits;
   unsigned char* restrict r = result;

   U_INTERNAL_TRACE("u_base64url_encode(%.*s,%u,%p)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   if (len > 2)
      {
      for (; i < len - 2; i += 3)
         {
         bits = ((((input[i]    << 8) +
                    input[i+1]) << 8) +
                    input[i+2]);

         u_put_unalignedp32(r, U_MULTICHAR_CONSTANT32("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[ bits >> 18],
                                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[(bits >> 12) & 0x3f],
                                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[(bits >>  6) & 0x3f],
                                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[ bits        & 0x3f]));

         r += 4;
         }
      }

   switch (len - i)
      {
      case 0: break;
      case 1:
         {
         bits = ((input[i] << 8) << 8);

         u_put_unalignedp16(r, U_MULTICHAR_CONSTANT16("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[ bits >> 18],
                                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[(bits >> 12) & 0x3f]));

         r += 2;
         }
      break;

      default: /* case 2: */
         {
         bits = (((input[i]    << 8) +
                   input[i+1]) << 8);

         u_put_unalignedp32(r, U_MULTICHAR_CONSTANT32("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[ bits >> 18],
                                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[(bits >> 12) & 0x3f],
                                                      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"[(bits >>  6) & 0x3f],
                                                      '\0'));

         return (r + 3 - result);
         }
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
    * for (int i = 0; i < sizeof("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"); ++i)
    *    {
    *     member["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = 1;
    *    decoder["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
    *    }
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

   uint32_t i = 0;
   int char_count = 0, bits = 0;
   unsigned char* restrict r = result;

   U_INTERNAL_TRACE("u_base64_decode(%.*s,%u,%p)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   u_base64_errors = 0;

   for (; i < len; ++i)
      {
      char c = input[i];

      if (c == U_PAD) break;

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
         u_put_unalignedp32(r, U_MULTICHAR_CONSTANT32( bits >> 16,         /* Byte 1 */
                                                      (bits >>  8) & 0xff, /* Byte 2 */
                                                       bits        & 0xff, /* Byte 3 */
                                                      '\0'));

         r += 3;

         bits = char_count = 0;
         }
      }

   if (i == len)
      {
      if (char_count)
         {
         ++u_base64_errors;

         U_INTERNAL_PRINT("Decoding incomplete: at least %u bits truncated", (4 - char_count) * 6)
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
   u_put_unalignedp16(r, U_MULTICHAR_CONSTANT16( bits >> 16,
                                                (bits >>  8) & 0xff));

   r += 2;

next:
   *r = 0;

   return (r - result);
}

uint32_t u_base64url_decode(const char* restrict input, uint32_t len, unsigned char* restrict result)
{
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

   uint32_t i = 0;
   int char_count = 0, bits = 0;
   unsigned char* restrict r = result;

   U_INTERNAL_TRACE("u_base64url_decode(%.*s,%u,%p)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   u_base64_errors = 0;

   for (; i < len; ++i)
      {
      char c = input[i];

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
         u_put_unalignedp32(r, U_MULTICHAR_CONSTANT32( bits >> 16,         /* Byte 1 */
                                                      (bits >>  8) & 0xff, /* Byte 2 */
                                                       bits        & 0xff, /* Byte 3 */
                                                      '\0'));

         r += 3;

         bits = char_count = 0;
         }
      }

   if (char_count)
      {
      static const int dispatch_table[] = {
         0,
         0,
         (char*)&&case_2-(char*)&&case_1,
         (char*)&&case_3-(char*)&&case_1
      };

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
      u_put_unalignedp16(r, U_MULTICHAR_CONSTANT16( bits >> 16,
                                                   (bits >>  8) & 0xff));

      r += 2;
      }

next:
   *r = 0;

   return (r - result);
}

uint32_t u_base64all_decode(const char* restrict input, uint32_t len, unsigned char* restrict result)
{
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

   uint32_t i = 0;
   int char_count = 0, bits = 0;
   unsigned char* restrict r = result;

   U_INTERNAL_TRACE("u_base64all_decode(%.*s,%u,%p)", U_min(len,128), input, len, result)

   U_INTERNAL_ASSERT_POINTER(input)

   u_base64_errors = 0;

   for (; i < len; ++i)
      {
      char c = input[i];

      if (c == U_PAD) break;

      if (member[(int)c] == 0) continue;

      if (u__isbase64(c) == false &&
          u__isb64url(c) == false)
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
         u_put_unalignedp32(r, U_MULTICHAR_CONSTANT32( bits >> 16,         /* Byte 1 */
                                                      (bits >>  8) & 0xff, /* Byte 2 */
                                                       bits        & 0xff, /* Byte 3 */
                                                      '\0'));

         r += 3;

         bits = char_count = 0;
         }
      }

   if (char_count)
      {
      static const int dispatch_table[] = {
         0,
         0,
         (char*)&&case_2-(char*)&&case_1,
         (char*)&&case_3-(char*)&&case_1
      };

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
      u_put_unalignedp16(r, U_MULTICHAR_CONSTANT16( bits >> 16,
                                                   (bits >>  8) & 0xff));

      r += 2;
      }

next:
   *r = 0;

   return (r - result);
}
