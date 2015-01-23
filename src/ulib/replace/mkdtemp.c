/* mkdtemp.c */

#include <ulib/base/base.h>

#include <errno.h>

/* Very simple-minded mkdtemp() replacement */

extern U_EXPORT char* mkdtemp(char* rltemplate);
       U_EXPORT char* mkdtemp(char* rltemplate)
{
   char* ltemplate;
   int i, oerrno, error;

   if (!(ltemplate = (char*) malloc(strlen(rltemplate) + 1))) return(NULL);

   for (error = 0, i = 0; i < 1000; ++i)
      {
      (void) strcpy(ltemplate, rltemplate);

      if (mktemp(ltemplate) == NULL)
         {
         error = 1;

         break;
         }

      error = mkdir(ltemplate, 0700);

      if (!error || errno != EEXIST) break;
      }

   oerrno = errno;

   if (*ltemplate) (void) strcpy(rltemplate, ltemplate);

   free(ltemplate);

   errno = oerrno;

   return (error ? NULL : rltemplate);
}
