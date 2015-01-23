/* assert.c */

#include <ulib/base/base.h>

#include <unistd.h>
#include <stdio.h>

extern U_EXPORT void __assert(const char* assertion, const char* filename, int linenumber, const char* function);
       U_EXPORT void __assert(const char* assertion, const char* filename, int linenumber, const char* function)
{
   fprintf(stderr,
      "%s: %s: %d: %s: Assertion `%s' failed.\n", u_progname,
      filename,
      linenumber,
      /* Function name isn't available with some compilers. */
      ((function == NULL) ? "?function?" : function),
      assertion
      );

   abort();
}
