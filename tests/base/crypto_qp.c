/* crypto_quoted_printable.c */

#include <ulib/base/coder/quoted_printable.h>

#include <stdlib.h>

#define U_ENCODE  1
#define U_DECODE  0
#define U_BUFLEN  4096

static const char* usage = "Usage: crypto_quoted_printable [-d]\n";

static void do_cipher(char* pw, int operation)
{
   long ebuflen;
   unsigned char  buf[U_BUFLEN];
   unsigned char ebuf[U_BUFLEN + 8];

   U_INTERNAL_TRACE("do_cipher(%s,%p)", pw, operation)

#ifdef __MINGW32__
  (void) setmode(1, O_BINARY);
#endif

   while (1)
      {
      int readlen = read(STDIN_FILENO, buf, sizeof(buf));

      if (readlen <= 0)
         {
         if (readlen == 0) break;
         else
            {
            perror("read");

            exit(1);
            }
         }

      if (operation == U_ENCODE) ebuflen = u_quoted_printable_encode(                      buf, readlen, ebuf);
      else                       ebuflen = u_quoted_printable_decode((const char* restrict)buf, readlen, ebuf);

      write(STDOUT_FILENO, ebuf, ebuflen);
      }
}

int main(int argc, char* argv[])
{
   u_init_ulib(argv);

   U_INTERNAL_TRACE("main(%d,%p)", argc, argv)

   if      (argc == 1)                               do_cipher(argv[1], U_ENCODE);
   else if (argc == 2 && strcmp(argv[1], "-d") == 0) do_cipher(argv[2], U_DECODE);
   else
      {
      fprintf(stderr, "%s", usage);

      exit(1);
      }

   return 0;
}
