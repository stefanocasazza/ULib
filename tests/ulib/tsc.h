// Support for using the TSC register on intel machines as a timing method.
//
// Should compile with -O to ensure inline attribute is honoured.
//

#ifndef __TSC_HDR
#define __TSC_HDR

#include <stdint.h>

#define TSC_OVERHEAD_N 100000

static inline void _sync_tsc(void)
{
  asm volatile("cpuid" : : : "%rax", "%rbx", "%rcx", "%rdx");
}

static inline uint64_t _rdtsc(void)
{
  unsigned a, d;
  asm volatile("rdtsc" : "=a" (a), "=d" (d) : : "%rbx", "%rcx");
  return ((uint64_t) a) | (((uint64_t) d) << 32);
}

static inline uint64_t _rdtscp(void)
{
  unsigned a, d;
  asm volatile("rdtscp" : "=a" (a), "=d" (d) : : "%rbx", "%rcx");
  return ((uint64_t) a) | (((uint64_t) d) << 32);
}

static inline uint64_t bench_start(void)
{
  // unsigned  cycles_low, cycles_high;
  // uint64_t t;
  //
  // asm volatile( "CPUID\n\t" // serialize
  //               "RDTSC\n\t" // read clock
  //               "mov %%edx, %0\n\t"
  //               "mov %%eax, %1\n\t"
  //               : "=r" (cycles_high), "=r" (cycles_low)
  //               :: "%rax", "%rbx", "%rcx", "%rdx" );
  // return ((uint64_t) cycles_high << 32) | cycles_low;

  _sync_tsc();
  return _rdtsc();
}

static inline uint64_t bench_end(void)
{
  // unsigned  cycles_low, cycles_high;
  // uint64_t t;
  //
  // asm volatile( "RDTSCP\n\t" // read clock + serialize
  //               "mov %%edx, %0\n\t"
  //               "mov %%eax, %1\n\t"
  //               "CPUID\n\t" // serialze -- but outside clock region!
  //               : "=r" (cycles_high), "=r" (cycles_low)
  //               :: "%rax", "%rbx", "%rcx", "%rdx" );
  // return ((uint64_t) cycles_high << 32) | cycles_low;

  uint64_t t = _rdtscp();
  _sync_tsc();
  return t;
}

static uint64_t measure_tsc_overhead(void)
{
  uint64_t t0, t1, overhead = ~0;
  int i;

  for (i = 0; i < TSC_OVERHEAD_N; i++) {
    t0 = bench_start();
    asm volatile("");
    t1 = bench_end();
    if (t1 - t0 < overhead)
      overhead = t1 - t0;
  }

  return overhead;
}

/*
# TSC Frequency
To convert from cycles to wall-clock time we need to know TSC frequency
Frequency scaling on modern Intel chips doesn't affect the TSC.

Sadly, there doesn't seem to be a good way to do this.

# Intel V3B: 17.14
That rate may be set by the maximum core-clock to bus-clock ratio of the
processor or may be set by the maximum resolved frequency at which the
processor is booted. The maximum resolved frequency may differ from the
processor base frequency, see Section 18.15.5 for more detail. On certain
processors, the TSC frequency may not be the same as the frequency in the brand
string.

# Linux Source
http://lxr.free-electrons.com/source/arch/x86/kernel/tsc.c?v=2.6.31#L399

Linux runs a calibration phase where it uses some hardware timers and checks
how many TSC cycles occur in 50ms.
*/
#define TSC_FREQ_MHZ 3500

static inline uint64_t cycles_to_ns(uint64_t cycles)
{
  // XXX: This is not safe! We don't have a good cross-platform way to
  // determine the TSC frequency for some strange reason.
  return cycles * 1000 / TSC_FREQ_MHZ;
}

#endif /* __TSC_HDR */
