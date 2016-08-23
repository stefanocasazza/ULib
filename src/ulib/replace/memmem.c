/* memmem.c */

#include <ulib/base/base.h>

/**
 * Return the first occurrence of NEEDLE in HAYSTACK.
 * Return HAYSTACK if NEEDLE_LEN is 0, otherwise NULL if NEEDLE is not found in HAYSTACK
 */

extern U_EXPORT void* memmem(const void* haystack_start, size_t haystack_len, const void* needle_start, size_t needle_len);
       U_EXPORT void* memmem(const void* haystack_start, size_t haystack_len, const void* needle_start, size_t needle_len)
{
   /*
    * Abstract memory is considered to be an array of 'unsigned char' values, not an array of 'char' values. See ISO C 99 section 6.2.6.1
    */

   const unsigned char* haystack = (const unsigned char*) haystack_start;
   const unsigned char* needle   = (const unsigned char*)   needle_start;

   if (needle_len == 0) return (void*) haystack; /* The first occurrence of the empty string is deemed to occur at the beginning of the string */

loop:
   if (haystack_len < needle_len) return 0; /* Sanity check, otherwise the loop might search through the whole memory */

   haystack = memchr(haystack, *needle, haystack_len);

   if (haystack)
      {
      haystack_len -= haystack - (const unsigned char*) haystack_start;

      if (haystack_len < needle_len) return 0;

      if (memcmp(haystack, needle, needle_len) == 0) return (void*) haystack;

      ++haystack;
      --haystack_len;

      goto loop;
      }

   return 0;
}
