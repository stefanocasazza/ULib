/* crypto_url.c */

#include <ulib/base/coder/url.h>

#include <stdlib.h>

#define U_DECODE      0
#define U_ENCODE      1
#define U_ENCODE_GPG  2
#define U_BUFLEN      8192

static const char* usage = "Usage: crypto_url [-d|-gpg]\n";

/*
Synopsis: Performs HTTP escaping on a string. This works as follows: all characters except alphanumerics
          and spaces are converted into the 3-byte sequence "%xx" where xx is the character's hexadecimal
          value; spaces are replaced by '+'. Line breaks are stored as "%0D%0A", where a 'line break' is
          any one of: "\n", "\r", "\n\r", or "\r\n"
*/

static void do_cipher(int operation)
{
   long ebuflen;
   unsigned char  buf[U_BUFLEN];
   unsigned char ebuf[U_BUFLEN + 8];

   U_INTERNAL_TRACE("do_cipher(%d)", operation)

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

      if (operation == U_DECODE) ebuflen = u_url_decode((const char* restrict)buf, readlen, ebuf);
      else
         {
         if (operation == U_ENCODE) ebuflen = u_url_encode(buf, readlen, ebuf);
         else
            {
            char* new_buf;
            char* ptr = (char*)buf;

            /* search for an empty line */

            while (true)
               {
               while (*ptr++ != '\n');
               if    (*ptr++ == '\n') break;
               }

            /* skip all lines before the first empty line */

            new_buf = ptr;

            /* stop on PGP END line */

            while (true)
               {
               while (*ptr++ != '\n');

               if (strncmp(ptr, "-----", 5) == 0) break;
               }

            ebuflen = u_url_encode((unsigned char*)new_buf, ptr - new_buf, ebuf);
            }
         }

      write(STDOUT_FILENO, ebuf, ebuflen);
      }
}

int main(int argc, char* argv[])
{
   int operation = -1;

   u_init_ulib(argv);

   U_INTERNAL_TRACE("main(%d,%p)", argc, argv)

   if      (argc == 1)                       operation = U_ENCODE;
   else if (argc == 2)
      {
      if      (strcmp(argv[1], "-d")   == 0) operation = U_DECODE;
      else if (strcmp(argv[1], "-gpg") == 0) operation = U_ENCODE_GPG;
      }

   if (operation == -1)
      {
      fprintf(stderr, "%s", usage);

      exit(1);
      }

   do_cipher(operation);

   return 0;
}
