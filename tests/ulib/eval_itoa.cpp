/**
 * eval_itoa.cpp
 *
 * Testing itoa...
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <ulib/string.h>
#include "branchlut.h"

#include "tsc.h"

#define N 100000

typedef void (*uPFi32pc) ( int32_t,char*);
typedef void (*uPFi64pc) ( int64_t,char*);
typedef void (*uPFu32pc) (uint32_t,char*);
typedef void (*uPFu64pc) (uint64_t,char*);

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
         ((uPFu32pc)func)(i,a);
         ((uPFu32pc)func)(i*10,a);
         ((uPFu32pc)func)(i*100,a);
         ((uPFu32pc)func)(i*1000,a);
         ((uPFu32pc)func)(i*10000,a);
         }
      break;
      case 1:
         {
         ((uPFi32pc)func)(-i,a);
         ((uPFi32pc)func)(-i*10,a);
         ((uPFi32pc)func)(-i*100,a);
         ((uPFi32pc)func)(-i*1000,a);
         ((uPFi32pc)func)(-i*10000,a);
         }
      break;
      case 2:
         {
         ((uPFu64pc)func)((uint64_t)i,a);
         ((uPFu64pc)func)((uint64_t)i*10,a);
         ((uPFu64pc)func)((uint64_t)i*100,a);
         ((uPFu64pc)func)((uint64_t)i*1000,a);
         ((uPFu64pc)func)((uint64_t)i*10000,a);
         }
      break;
      case 3:
         {
         ((uPFi64pc)func)((int64_t)-i,a);
         ((uPFi64pc)func)((int64_t)-i*10,a);
         ((uPFi64pc)func)((int64_t)-i*100,a);
         ((uPFi64pc)func)((int64_t)-i*1000,a);
         ((uPFi64pc)func)((int64_t)-i*10000,a);
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

   printf("=> Testing itoa...\n");

   /*
   char a[32], b[32];
   uint32_t sz1, sz2;

   for (int64_t i = LLONG_MIN; i < 0; ++i)
      {
      sz1 = sprintf(a, "%lld", i);
      sz2 = u_num2str64s(i, b);

      if (sz1 != sz2 ||
          memcmp(a, b, sz1))
         {
         U_ERROR("sz1=%u sz2=%u a=%.*S b=%.*S", sz1, sz2, sz1, a, sz2, b)
         }
      }

   for (uint64_t j = 0; j <= LLONG_MAX; ++j)
      {
      sz1 = sprintf(a, "%llu", j);
      sz2 = u_num2str64(j, b);

      if (sz1 != sz2 ||
          memcmp(a, b, sz1))
         {
         U_ERROR("sz1=%u sz2=%u a=%.*S b=%.*S", sz1, sz2, sz1, a, sz2, b)
         }
      }
   */

   func_overhead((void*)u_num2str32, "u_num2str32", 0);
   func_overhead((void*)u_num2str64, "u_num2str64", 2);

   func_overhead((void*)u32toa_branchlut, "u32toa_branchlut", 0);
   func_overhead((void*)u64toa_branchlut, "u64toa_branchlut", 2);

   return 0;
}
