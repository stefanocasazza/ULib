/**
 * eval_dtoa.cpp
 *
 * Testing dtoa...
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <ulib/string.h>
#include "dtoa_milo.h"

#include "tsc.h"

#define N 100000

typedef void (*vPFdpc) (double,char*);

static void func_overhead(void* func, const char* name, int type)
{
  int i;
  uint64_t t0, t1, tsc_overhead;
  uint64_t min, max, avg;
  uint64_t times[N];
  char a[32];

  tsc_overhead = measure_tsc_overhead();

  // we run N times and take the min
  for (i = 0; i < N; i++) {
    t0 = bench_start();

    switch (type)
      {
      case 0:
         {
         ((vPFdpc)func)(i,a);
         ((vPFdpc)func)(i*10.01,a);
         ((vPFdpc)func)(i*100.01,a);
         ((vPFdpc)func)(i*1000.01,a);
         ((vPFdpc)func)(i*10000.01,a);
         }
      break;
      case 1:
         {
         ((vPFdpc)func)(-i,a);
         ((vPFdpc)func)(-i*10.01,a);
         ((vPFdpc)func)(-i*100.01,a);
         ((vPFdpc)func)(-i*1000.01,a);
         ((vPFdpc)func)(-i*10000.01,a);
         }
      break;
      case 2:
         {
         ((vPFdpc)func)(i,a);
         ((vPFdpc)func)(i*10.01,a);
         ((vPFdpc)func)(i*100.01,a);
         ((vPFdpc)func)(i*1000.01,a);
         ((vPFdpc)func)(i*10000.01,a);
         }
      break;
      case 3:
         {
         ((vPFdpc)func)(-i,a);
         ((vPFdpc)func)(-i*10.01,a);
         ((vPFdpc)func)(-i*100.01,a);
         ((vPFdpc)func)(-i*1000.01,a);
         ((vPFdpc)func)(-i*10000.01,a);
         }
      break;
      }

    t1 = bench_end();
    times[i] = t1 - t0 - tsc_overhead;
  }
  
  min = ~0, max = 0, avg = 0;
  for (i = 0; i < N; i++) {
    avg += times[i];
    if (times[i] < min) { min = times[i]; }
    if (times[i] > max) { max = times[i]; }
  }
  avg /= N;
  
  printf("\n- %s -\n", name);
  printf("Cost (min): %" PRIu64 " cycles\n", min);
  printf("Cost (avg): %" PRIu64 " cycles\n", avg);
  printf("Cost (max): %" PRIu64 " cycles\n", max);
}

int
U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   printf("=> Testing dtoa...\n");

   /*
   char a[32], b[32];
   double sz1, sz2;

   for (double i = DOUBLE_MIN; i < 0; ++i)
      {
      sz1 = sprintf(a, "%g", i);
      sz2 = u_dtoa(i, b);

      if (sz1 != sz2 ||
          memcmp(a, b, sz1))
         {
         U_ERROR("sz1=%u sz2=%u a=%.*S b=%.*S", sz1, sz2, sz1, a, sz2, b)
         }
      }

   for (double j = 0; j <= DOUBLE_MAX; ++j)
      {
      sz1 = sprintf(a, "%g", j);
      sz2 = u_dtoa(j, b);

      if (sz1 != sz2 ||
          memcmp(a, b, sz1))
         {
         U_ERROR("sz1=%u sz2=%u a=%.*S b=%.*S", sz1, sz2, sz1, a, sz2, b)
         }
      }
   */

   func_overhead((void*)u_dtoa,    "u_dtoa",    2);
   func_overhead((void*)dtoa_milo, "dtoa_milo", 2);

   return 0;
}
