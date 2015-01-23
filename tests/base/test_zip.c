// test_zip.c

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/base.h>
#include <ulib/base/zip/ziptool.h>

#include <stdio.h>

int main(int argc, char* argv[])
{
   int i = 0, result;
   char filename[4096];
   const char* files[512];

   u_init_ulib(argv);

   U_INTERNAL_TRACE("main(%d,%p)", argc, argv)

   if (argc > 2)
      {
      char** filenames;
      unsigned* filenames_len;

      result = zip_extract(argv[1], 0, &filenames, &filenames_len);

      U_INTERNAL_ASSERT(result > 0)
      }
   else
      {
   /* while (gets(filename)) */
      while (fgets(filename, 4096, stdin))
         {
         U_INTERNAL_TRACE("filename = %s", filename)

         files[i++] = strndup(filename, strlen(filename)-1);
         }

      files[i] = 0;

      result = zip_archive(argv[1], files);

      U_INTERNAL_ASSERT(result == 0)
      }

   U_INTERNAL_PRINT("i = %d result = %d", i, result)

   if (argc <= 2)
      {
      bool match = (zip_match(argv[1], files) == 1);

      U_INTERNAL_PRINT("argv[1] = %s match = %d", argv[1], match)

      U_INTERNAL_ASSERT(match == true)
      }

   return result;
}
