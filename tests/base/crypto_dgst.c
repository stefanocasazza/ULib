// crypto_dgst.c

#include <ulib/base/ssl/dgst.h>

#include <stdlib.h>
#include <openssl/ssl.h>

#define U_BUFLEN (4096 * 1)

static const char* usage = "Usage: crypto_dgst algo key keylen\n";

static void u_do_cipher(int alg, const char* key, uint32_t keylen)
{
   static unsigned char buf[U_BUFLEN];

   U_INTERNAL_TRACE("u_do_cipher(%d,%.*s,%u)", alg, keylen, key, keylen)

   u_dgst_init(alg, key, keylen);

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

      u_dgst_hash(buf, readlen);
      }

#  ifdef __MINGW32__
   (void) setmode(1, O_BINARY);
#  endif

   write(STDOUT_FILENO, buf, u_dgst_finish(buf, 0));
}

int main(int argc, char** argv)
{
   u_init_ulib(argv);

   U_INTERNAL_TRACE("main(%d,%p)", argc, argv)

   U_INTERNAL_PRINT("argv[0] = %s\nargv[1] = %s", argv[0], argv[1])

   if (argc == 4) u_do_cipher(atoi(argv[1]), argv[2], atoi(argv[3]));
   else
      {
      fprintf(stderr, "%s", usage);

      exit(1);
      }

   return 0;
}
