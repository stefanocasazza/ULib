// crypto_des3.c

#include <ulib/base/ssl/des3.h>

#include <stdlib.h>
#include <openssl/ssl.h>

#define U_ENCRYPT 1
#define U_DECRYPT 0
#define U_BUFLEN  (4096 * 1)

#ifndef   EVP_MAX_BLOCK_LENGTH
#  define EVP_MAX_BLOCK_LENGTH 32
#endif

static const char* usage = "Usage: crypto_des3 [-d] passwd\n";

static void u_do_cipher(char* pw, int operation)
{
   long ebuflen;
   static unsigned char  buf[U_BUFLEN];
   static unsigned char ebuf[U_BUFLEN + EVP_MAX_BLOCK_LENGTH];

   U_INTERNAL_TRACE("u_do_cipher(%s,%d)", pw, operation)

   u_des3_key((const char*)pw);

#  ifdef __MINGW32__
   (void) setmode(1, O_BINARY);
#  endif

   while (true)
      {
      int readlen = read(STDIN_FILENO, buf, sizeof(buf));

      U_INTERNAL_PRINT("readlen = %d", readlen)

      if (readlen <= 0)
         {
         if (!readlen) break;
         else
            {
            perror("read");

            exit(1);
            }
         }

      U_INTERNAL_PRINT("buf = %.*s", readlen, buf)

      if (operation == U_ENCRYPT) ebuflen = u_des3_encode(buf, readlen, ebuf);
      else                        ebuflen = u_des3_decode(buf, readlen, ebuf);

      if (ebuflen) write(STDOUT_FILENO, ebuf, ebuflen);
      }
}

int main(int argc, char** argv)
{
   u_init_ulib(argv);

   U_INTERNAL_TRACE("main(%d,%p)", argc, argv)

   U_INTERNAL_PRINT("argv[0] = %s\nargv[1] = %s", argv[0], argv[1])

   if      (argc == 2)                                                    u_do_cipher(argv[1], U_ENCRYPT);
   else if (argc == 3 && memcmp(argv[1], U_CONSTANT_TO_PARAM("-d")) == 0) u_do_cipher(argv[2], U_DECRYPT);
   else
      {
      fprintf(stderr, "%s", usage);

      exit(1);
      }

   return 0;
}
