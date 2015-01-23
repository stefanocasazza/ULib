/* test_gzio.c */

/*
#define DEBUG_DEBUG
*/

#include <ulib/base/coder/gzio.h>

#include <stdlib.h>

#define U_ENCODE  1
#define U_DECODE  0
#define U_BUFLEN  4096

static const char* usage = "Usage: test_gzio [-d]\n";

static void do_cipher(int fd, int operation)
{
   uint32_t ebuflen;
   char  buf[U_BUFLEN];
   char ebuf[U_BUFLEN * 8];

   U_INTERNAL_TRACE("do_cipher(%d,%d)", fd, operation)

#  ifdef __MINGW32__
   (void) setmode(1, O_BINARY);
#  endif

   while (1)
      {
      uint32_t readlen = read(fd, buf, U_BUFLEN);

      U_INTERNAL_TRACE("readlen = %u", readlen)

      if (readlen <= 0)
         {
         if (readlen == 0) break;
         else
            {
            perror("read");

            exit(1);
            }
         }

      if (operation == U_ENCODE) ebuflen = u_gz_deflate(buf, readlen, ebuf, true);
      else                       ebuflen = u_gz_inflate(buf, readlen, ebuf);

      write(STDOUT_FILENO, ebuf, ebuflen);
      }
}

int main(int argc, char* argv[])
{
   int fd, cipher;
   char* filename;

   u_init_ulib(argv);

   U_INTERNAL_TRACE("main(%d,%p)", argc, argv)

   if (argc == 2)
      {
      cipher   = U_ENCODE;
      filename = argv[1];
      }
   else if (argc == 3 && strcmp(argv[1], "-d") == 0)
      {
      cipher   = U_DECODE;
      filename = argv[2];
      }
   else
      {
      fprintf(stderr, "%s", usage);

      exit(1);
      }

   fd = open(filename, O_RDONLY | O_BINARY); 

   do_cipher(fd, cipher);

   return 0;
}
