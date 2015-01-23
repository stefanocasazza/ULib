/* test_random.c - Basically allows you to test any range of numbers. */

#include <ulib/base/hash.h>

#include <stdlib.h>

static uint32_t n, cnt, sides, maxroll;

int main(int argc, char* argv[])
{
#ifdef DEBUG
   uint32_t hash1, hash2, hash3;
#endif

   u_init_ulib(argv);

#ifdef DEBUG
   hash1 = u_hash((unsigned char*)U_CONSTANT_TO_PARAM("Set-Cookie2"), false),
   hash2 = u_hash((unsigned char*)U_CONSTANT_TO_PARAM("Set-Cookie2"), true),
   hash3 = u_hash((unsigned char*)U_CONSTANT_TO_PARAM("Set-COOkie2"), true);

   U_INTERNAL_ASSERT(hash1 != hash2)
   U_INTERNAL_ASSERT(hash2 == hash3)
#endif

   sides   = (argc > 1 ? atol(argv[1]) :    6);
   maxroll = (argc > 2 ? atol(argv[2]) : 1000);

   {
   uint32_t faces[sides];

   memset(faces, 0, sides * sizeof(long));

   for (cnt = 0; cnt < maxroll; ++cnt)
      {
      n = u_random(cnt) % sides;

      faces[n] += 1;
      }

   printf("Number of sides: %u\n", sides);
   printf("Number of rolls: %u\n", maxroll);
   printf("Face    Frequency      Percent\n");

   for (cnt = 0; cnt < sides; ++cnt)
      {
      printf("%.2u      %.4u             %.2f%%\n", cnt + 1, faces[cnt], (float)faces[cnt] / maxroll * 100);
      }
   }

   return 0;
}
