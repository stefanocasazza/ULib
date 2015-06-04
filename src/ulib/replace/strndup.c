/* strndup.c */

#include <ulib/base/base.h>

#include <stdio.h>

/* Find the length of STRING, but scan at most MAXLEN characters. If no '\0' terminator is found in that many characters, return MAXLEN. */

extern U_EXPORT char*  strndup(const char* s, size_t n);
extern U_EXPORT size_t strnlen(const char* string, size_t maxlen);

U_EXPORT size_t strnlen(const char* string, size_t maxlen)
{
   const char* end = (const char*) memchr(string, '\0', maxlen);

   return (end ? (size_t)(end - string) : maxlen);
}

U_EXPORT char* strndup(const char* s, size_t n)
{
   size_t len = strnlen(s, n);
   char*  res = (char*) malloc(len + 1);

   if (res == 0) return 0;

   res[len] = '\0';

   return (char*) memcpy(res, s, len);
}
