/* apex_memmove.c */

#include <ulib/base/apex/apex_memmove.h>

#include <string.h> /* memmove, memcpy */

#if defined(__MINGW32__) || !defined(HAVE_ARCH64)
pvPFpvpvs apex_memcpy  = &memcpy;
pvPFpvpvs apex_memmove = &memmove;
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86) /* Test for Intel/AMD architecture */
/* Intel/AMD SSE intrinsics */
#  ifdef _MSC_VER
#     include <nmmintrin.h>
#  else
#     include <x86intrin.h>
#  endif

/* apex_memmove (tiberium, kryptonite and mithril) memcpy/memmove functions written by Trevor Herselman in 2014 */

#  if defined(_MSC_VER)
#  pragma warning( push )
#  pragma warning( disable : 4146 ) /* warning C4146: unary minus operator applied to unsigned type, result still unsigned */
#  pragma warning( disable : 4244 ) /* warning C4244: '-=': conversion from '__int64' to 'std::size_t', possible loss of data */
#  endif

#  if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86) /* Test for Intel/AMD architecture */

void* APEXCALL apex_tiberium( void *dst, const void *src, size_t size );
void* APEXCALL apex_tiberium( void *dst, const void *src, size_t size )
{  /* based on memmove09 for "size <= 112" and memmove40 for "size > 112" */
   if ( size <= 112 )
   {
      if ( size >= 16 )
      {
         const __m128i xmm0 = _mm_loadu_si128( (__m128i*)src );
         if ( size > 16 )
         {
            long long rax, rcx;
            if ( size >= 32 )
            {
               const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + 16) );
               if ( size > 32 )
               {
                  rax = *(long long*)((char*)src + size - 16);
                  rcx = *(long long*)((char*)src + size - 8);
                  if ( size > 48 )
                  {
                     const __m128i xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + 32) );
                     if ( size > 64 )
                     {
                        const __m128i xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + 48) );
                        if ( size > 80 )
                        {
                           const __m128i xmm4 = _mm_loadu_si128( (__m128i*)((char*)src + 64) );
                           if ( size > 96 )
                           {
                              const __m128i xmm5 = _mm_loadu_si128( (__m128i*)((char*)src + 80) );
                              *(long long*)((char*)dst + size - 16) = rax;
                              *(long long*)((char*)dst + size - 8) = rcx;
                              _mm_storeu_si128( (__m128i*)dst, xmm0 );
                              _mm_storeu_si128( (__m128i*)((char*)dst + 16), xmm1 );
                              _mm_storeu_si128( (__m128i*)((char*)dst + 32), xmm2 );
                              _mm_storeu_si128( (__m128i*)((char*)dst + 48), xmm3 );
                              _mm_storeu_si128( (__m128i*)((char*)dst + 64), xmm4 );
                              _mm_storeu_si128( (__m128i*)((char*)dst + 80), xmm5 );
                              return dst;
                           }
                           *(long long*)((char*)dst + size - 16) = rax;
                           *(long long*)((char*)dst + size - 8) = rcx;
                           _mm_storeu_si128( (__m128i*)dst, xmm0 );
                           _mm_storeu_si128( (__m128i*)((char*)dst + 16), xmm1 );
                           _mm_storeu_si128( (__m128i*)((char*)dst + 32), xmm2 );
                           _mm_storeu_si128( (__m128i*)((char*)dst + 48), xmm3 );
                           _mm_storeu_si128( (__m128i*)((char*)dst + 64), xmm4 );
                           return dst;
                        }
                        *(long long*)((char*)dst + size - 16) = rax;
                        *(long long*)((char*)dst + size - 8) = rcx;
                        _mm_storeu_si128( (__m128i*)dst, xmm0 );
                        _mm_storeu_si128( (__m128i*)((char*)dst + 16), xmm1 );
                        _mm_storeu_si128( (__m128i*)((char*)dst + 32), xmm2 );
                        _mm_storeu_si128( (__m128i*)((char*)dst + 48), xmm3 );
                        return dst;
                     }
                     *(long long*)((char*)dst + size - 16) = rax;
                     *(long long*)((char*)dst + size - 8) = rcx;
                     _mm_storeu_si128( (__m128i*)dst, xmm0 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + 16), xmm1 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + 32), xmm2 );
                     return dst;
                  }
                  *(long long*)((char*)dst + size - 16) = rax;
                  *(long long*)((char*)dst + size - 8) = rcx;
               }
               _mm_storeu_si128( (__m128i*)dst, xmm0 );
               _mm_storeu_si128( (__m128i*)((char*)dst + 16), xmm1 );
               return dst;
            }
            rax = *(long long*)((char*)src + size - 16);
            rcx = *(long long*)((char*)src + size - 8);
            *(long long*)((char*)dst + size - 16) = rax;
            *(long long*)((char*)dst + size - 8) = rcx;
         }
         _mm_storeu_si128( (__m128i*)dst, xmm0 );
         return dst;
      }
      if ( size >= 8 )
      {
         long long rax = *(long long*)src;
         if ( size > 8 )
         {
            long long rcx = *(long long*)((char*)src + size - 8);
            *(long long*)dst = rax;
            *(long long*)((char*)dst + size - 8) = rcx;
         }
         else *(long long*)dst = rax;
      }
      else if ( size >= 4 )
      {
         int eax = *(int*)src;
         if ( size > 4 )
         {
            int ecx = *(int*)((char*)src + size - 4);
            *(int*)dst = eax;
            *(int*)((char*)dst + size - 4) = ecx;
         }
         else *(int*)dst = eax;
      }
      else if ( size >= 1 )
      {
         char al = *(char*)src;
         if ( size > 1 )
         {
            short cx = *(short*)((char*)src + size - 2);
            *(char*)dst = al;
            *(short*)((char*)dst + size - 2) = cx;
         }
         else *(char*)dst = al;
      }
      return dst;
   }
   else
   {
      void * const ret = dst;
      if ( ((size_t)dst - (size_t)src) >= size )
      {
         if ( size < (1024 * 256) )
         {
            long long offset = (long long)(size & -0x40);/* "Round down to nearest multiple of 64" */
            dst = (char*)dst + offset;                   /* "Point to the end" */
            src = (char*)src + offset;                   /* "Point to the end" */
            size -= offset;                              /* "Remaining data after loop" */
            offset = -offset;                            /* "Negative index from the end" */

            do
            {
               const __m128i xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               const __m128i xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               const __m128i xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               _mm_storeu_si128( (__m128i*)((char*)dst + offset), xmm0 );
               _mm_storeu_si128( (__m128i*)((char*)dst + offset + 16), xmm1 );
               _mm_storeu_si128( (__m128i*)((char*)dst + offset + 32), xmm2 );
               _mm_storeu_si128( (__m128i*)((char*)dst + offset + 48), xmm3 );
            } while ( offset += 64 );

            if ( size >= 16 )
            {
               const __m128i xmm0 = _mm_loadu_si128( (__m128i*)src );
               if ( size > 16 )
               {
                  const __m128i xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + size - 16) );
                  if ( size > 32 )
                  {
                     const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + 16) );
                     if ( size > 48 )
                     {
                        const __m128i xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + 32) );
                        _mm_storeu_si128( (__m128i*)dst, xmm0 );
                        _mm_storeu_si128( (__m128i*)((char*)dst + 16), xmm1 );
                        _mm_storeu_si128( (__m128i*)((char*)dst + 32), xmm2 );
                        _mm_storeu_si128( (__m128i*)((char*)dst + size - 16), xmm3 );
                        return ret;
                     }
                     _mm_storeu_si128( (__m128i*)dst, xmm0 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + 16), xmm1 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size - 16), xmm3 );
                     return ret;
                  }
                  _mm_storeu_si128( (__m128i*)dst, xmm0 );
                  _mm_storeu_si128( (__m128i*)((char*)dst + size - 16), xmm3 );
                  return ret;
               }
               _mm_storeu_si128( (__m128i*)dst, xmm0 );
               return ret;
            }
         }
         else  /* do forward streaming copy/move */
         {
            __m128i xmm0;
            __m128i xmm1;
            __m128i xmm2;
            __m128i xmm3;
            long long offset;
            /* We MUST do prealignment on streaming copies! */
            const size_t prealign = -(size_t)dst & 0xf;
            if ( prealign )
            {
               if ( prealign >= 8 )
               {
                  long long rax = *(long long*)src;
                  if ( prealign > 8 )
                  {
                     long long rcx = *(long long*)((char*)src + prealign - 8);
                     *(long long*)dst = rax;
                     *(long long*)((char*)dst + prealign - 8) = rcx;
                  }
                  else *(long long*)dst = rax;
               }
               else if ( prealign >= 4 )
               {
                  int eax = *(int*)src;
                  if ( prealign > 4 )
                  {
                     int ecx = *(int*)((char*)src + prealign - 4);
                     *(int*)dst = eax;
                     *(int*)((char*)dst + prealign - 4) = ecx;
                  }
                  else *(int*)dst = eax;
               }
               else
               {
                  char al = *(char*)src;
                  if ( prealign > 1 )
                  {
                     short cx = *(short*)((char*)src + prealign - 2);
                     *(char*)dst = al;
                     *(short*)((char*)dst + prealign - 2) = cx;
                  }
                  else *(char*)dst = al;
               }
               src = (char*)src + prealign;
               dst = (char*)dst + prealign;
               size -= prealign;
            }

            /* Begin prefetching upto 4KB */
            for ( offset = 0; offset < 4096; offset += 256 )
            {
               _mm_prefetch( ((char*)src + offset), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset + 64), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset + 128), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset + 192), _MM_HINT_NTA );
            }

            offset = (long long)(size & -0x40);    /* "Round down to nearest multiple of 64" */
            size -= offset;                        /* "Remaining data after loop" */
            offset -= 4096;                        /* stage 1 INCLUDES prefetches */
            dst = (char*)dst + offset;             /* "Point to the end" */
            src = (char*)src + offset;             /* "Point to the end" */
            offset = -offset;                      /* "Negative index from the end" */

            do                                     /* stage 1 ~~ WITH prefetching */
            {
               _mm_prefetch( (char*)src + offset + 4096, _MM_HINT_NTA );
               xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               _mm_stream_si128( (__m128i*)((char*)dst + offset), xmm0 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 16), xmm1 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 32), xmm2 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 48), xmm3 );
            } while ( offset += 64 );

            offset = -4096;
            dst = (char*)dst + 4096;
            src = (char*)src + 4096;

            _mm_prefetch( ((char*)src + size - 64), _MM_HINT_NTA );  /* prefetch the final tail section */

            do                                     /* stage 2 ~~ WITHOUT further prefetching */
            {
               xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               _mm_stream_si128( (__m128i*)((char*)dst + offset), xmm0 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 16), xmm1 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 32), xmm2 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 48), xmm3 );
            } while ( offset += 64 );

            if ( size >= 16 )
            {
               xmm0 = _mm_loadu_si128( (__m128i*)src );
               if ( size > 16 )
               {
                  __m128i xmm6;
                  __m128i xmm7;
                  if ( size > 32 )
                  {
                     xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + 16) );
                     xmm6 = _mm_loadu_si128( (__m128i*)((char*)src + size - 32) );
                     xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size - 16) );
                     _mm_stream_si128( (__m128i*)dst, xmm0 );
                     _mm_stream_si128( (__m128i*)((char*)dst + 16), xmm1 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size - 32), xmm6 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size - 16), xmm7 );
                     return ret;
                  }
                  xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size - 16) );
                  _mm_stream_si128( (__m128i*)dst, xmm0 );
                  _mm_storeu_si128( (__m128i*)((char*)dst + size - 16), xmm7 );
                  return ret;
               }
               _mm_stream_si128( (__m128i*)dst, xmm0 );
               return ret;
            }
         }

         if ( size >= 8 )
         {
            long long rax = *(long long*)src;
            if ( size > 8 )
            {
               long long rcx = *(long long*)((char*)src + size - 8);
               *(long long*)dst = rax;
               *(long long*)((char*)dst + size - 8) = rcx;
            }
            else *(long long*)dst = rax;
         }
         else if ( size >= 4 )
         {
            int eax = *(int*)src;
            if ( size > 4 )
            {
               int ecx = *(int*)((char*)src + size - 4);
               *(int*)dst = eax;
               *(int*)((char*)dst + size - 4) = ecx;
            }
            else *(int*)dst = eax;
         }
         else if ( size >= 1 )
         {
            char al = *(char*)src;
            if ( size > 1 )
            {
               short cx = *(short*)((char*)src + size - 2);
               *(char*)dst = al;
               *(short*)((char*)dst + size - 2) = cx;
            }
            else *(char*)dst = al;
         }
         return ret;
      }
      else /* src < dst ... do reverse copy */
      {
         int eax;
         int ecx;
         short cx;

         src = (char*)src + size;
         dst = (char*)dst + size;

         if ( size < (1024 * 256) )
         {
            __m128i xmm0;
            __m128i xmm1;
            __m128i xmm2;
            __m128i xmm3;
            long long offset = (long long)(size & -0x40);/* "Round down to nearest multiple of 64" */
            dst = (char*)dst - offset;                   /* "Point to the end" ... actually, we point to the start! */
            src = (char*)src - offset;                   /* "Point to the end" ... actually, we point to the start! */
            size -= offset;                              /* "Remaining data after loop" */
            /*offset = -offset;                          // "Negative index from the end" ... not when doing reverse copy/move! */

            offset -= 64;
            do
            {
               xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               _mm_storeu_si128( (__m128i*)((char*)dst + offset + 48), xmm0 );
               _mm_storeu_si128( (__m128i*)((char*)dst + offset + 32), xmm1 );
               _mm_storeu_si128( (__m128i*)((char*)dst + offset + 16), xmm2 );
               _mm_storeu_si128( (__m128i*)((char*)dst + offset), xmm3 );
            } while ( (offset -= 64) >= 0 );

            if ( size >= 16 )
            {
               xmm0 = _mm_loadu_si128( (__m128i*)((char*)src - 16) );
               if ( size > 16 )
               {
                  size = -size;
                  xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + size) );
                  if ( size > 32 )
                  {
                     xmm1 = _mm_loadu_si128( (__m128i*)((char*)src - 32) );
                     if ( size > 48 )
                     {
                        xmm2 = _mm_loadu_si128( (__m128i*)((char*)src - 48) );
                        _mm_storeu_si128( (__m128i*)((char*)dst - 16), xmm0 );
                        _mm_storeu_si128( (__m128i*)((char*)dst - 32), xmm1 );
                        _mm_storeu_si128( (__m128i*)((char*)dst - 48), xmm2 );
                        _mm_storeu_si128( (__m128i*)((char*)dst + size), xmm3 );
                        return ret;
                     }
                     _mm_storeu_si128( (__m128i*)((char*)dst - 16), xmm0 );
                     _mm_storeu_si128( (__m128i*)((char*)dst - 32), xmm1 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size), xmm3 );
                     return ret;
                  }
                  _mm_storeu_si128( (__m128i*)((char*)dst - 16), xmm0 );
                  _mm_storeu_si128( (__m128i*)((char*)dst + size), xmm3 );
                  return ret;
               }
               _mm_storeu_si128( (__m128i*)((char*)dst - 16), xmm0 );
               return ret;
            }
         }
         else  /* do reversed streaming copy/move */
         {
            __m128i xmm0;
            __m128i xmm1;
            __m128i xmm2;
            __m128i xmm3;
            __m128i xmm6;
            __m128i xmm7;
            long long offset;
            long long rax, rcx;
            /* We MUST do prealignment on streaming copies! */
            const size_t prealign = (size_t)dst & 0xf;
            if ( prealign )
            {
               src = (char*)src - prealign;
               dst = (char*)dst - prealign;
               size -= prealign;
               if ( prealign >= 8 )
               {
                  rax = *(long long*)((char*)src + prealign - 8);
                  if ( prealign > 8 )
                  {
                     rcx = *(long long*)src;
                     *(long long*)((char*)dst + prealign - 8) = rax;
                     *(long long*)dst = rcx;
                  }
                  else *(long long*)dst = rax; /* different on purpose, because we know the exact size now, which is 8, and "dst" has already been aligned! */
               }
               else if ( prealign >= 4 )
               {
                  eax = *(int*)((char*)src + prealign - 4);
                  if ( prealign > 4 )
                  {
                     ecx = *(int*)src;
                     *(int*)((char*)dst + prealign - 4) = eax;
                     *(int*)dst = ecx;
                  }
                  else *(int*)dst = eax; /* different on purpose! */
               }
               else
               {
                  char al = *(char*)((char*)src + prealign - 1);
                  if ( prealign > 1 )
                  {
                     cx = *(short*)src;
                     *(char*)((char*)dst + prealign - 1) = al;
                     *(short*)dst = cx;
                  }
                  else *(char*)dst = al; /* different on purpose! */
               }
            }

            /* Begin prefetching upto 4KB */
            for ( offset = 0; offset > -4096; offset -= 256 )
            {
               _mm_prefetch( ((char*)src + offset - 64), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset - 128), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset - 192), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset - 256), _MM_HINT_NTA );
            }

            offset = (long long)(size & -0x40);    /* "Round down to nearest multiple of 64" */
            size -= offset;                        /* "Remaining data after loop" */
            offset -= 4096;                        /* stage 1 INCLUDES prefetches */
            dst = (char*)dst - offset;             /* "Point to the end" ... actually, we point to the start! */
            src = (char*)src - offset;             /* "Point to the end" ... actually, we point to the start! */
            /*offset = -offset;                    // "Negative index from the end" ... not when doing reverse copy/move! */

            offset -= 64;
            do                                     /* stage 1 ~~ WITH prefetching */
            {
               _mm_prefetch( (char*)src + offset - 4096, _MM_HINT_NTA );
               xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 48), xmm0 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 32), xmm1 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 16), xmm2 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset), xmm3 );
            } while ( (offset -= 64) >= 0 );

            offset = 4096;
            dst = (char*)dst - 4096;
            src = (char*)src - 4096;

            _mm_prefetch( ((char*)src - 64), _MM_HINT_NTA );   /* prefetch the final tail section */

            offset -= 64;
            do                                     /* stage 2 ~~ WITHOUT further prefetching */
            {
               xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 48), xmm0 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 32), xmm1 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 16), xmm2 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset), xmm3 );
            } while ( (offset -= 64) >= 0 );

            if ( size >= 16 )
            {
               xmm0 = _mm_loadu_si128( (__m128i*)((char*)src - 16) );
               if ( size > 16 )
               {
                  if ( size > 32 )
                  {
                     size = -size;
                     xmm1 = _mm_loadu_si128( (__m128i*)((char*)src - 32) );
                     xmm6 = _mm_loadu_si128( (__m128i*)((char*)src + size + 16) );
                     xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size) );
                     _mm_stream_si128( (__m128i*)((char*)dst - 16), xmm0 );
                     _mm_stream_si128( (__m128i*)((char*)dst - 32), xmm1 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size + 16), xmm6 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size), xmm7 );
                     return ret;
                  }
                  size = -size;
                  xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size) );
                  _mm_stream_si128( (__m128i*)((char*)dst - 16), xmm0 );
                  _mm_storeu_si128( (__m128i*)((char*)dst + size), xmm7 );
                  return ret;
               }
               _mm_stream_si128( (__m128i*)((char*)dst - 16), xmm0 );
               return ret;
            }
         }

         if ( size >= 8 )
         {
            long long rcx;
            long long rax = *(long long*)((char*)src - 8);
            if ( size > 8 )
            {
               size = -size; /* that's right, we're converting an unsigned value to a negative, saves 2 clock cycles! */
               rcx = *(long long*)((char*)src + size);
               *(long long*)((char*)dst - 8) = rax;
               *(long long*)((char*)dst + size) = rcx;
            }
            else *(long long*)((char*)dst - 8) = rax;
         }
         else if ( size >= 4 )
         {
            eax = *(int*)((char*)src - 4);
            if ( size > 4 )
            {
               size = -size;
               ecx = *(int*)((char*)src + size);
               *(int*)((char*)dst - 4) = eax;
               *(int*)((char*)dst + size) = ecx;
            }
            else *(int*)((char*)dst - 4) = eax;
         }
         else if ( size >= 1 )
         {
            char al = *((char*)src - 1);
            if ( size > 1 )
            {
               size = -size;
               cx = *(short*)((char*)src + size);
               *((char*)dst - 1) = al;
               *(short*)((char*)dst + size) = cx;
            }
            else *((char*)dst - 1) = al;
         }
         return ret;
      }
   }
}

void* APEXCALL apex_kryptonite( void *dst, const void *src, size_t size );
void* APEXCALL apex_kryptonite( void *dst, const void *src, size_t size )
{  /* based on memmove09 for "size <= 112" and memmove41 for "size > 112"; memmove09's "size <= 112" proved fastest overall (weighted), even on Core i5! */
   if ( size <= 112 )
   {
      if ( size >= 16 )
      {
         const __m128i xmm0 = _mm_loadu_si128( (__m128i*)src );
         if ( size > 16 )
         {
            long long rax;
            long long rcx;
            if ( size >= 32 )
            {
               const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + 16) );
               if ( size > 32 )
               {
                  rax = *(long long*)((char*)src + size - 16);
                  rcx = *(long long*)((char*)src + size - 8);
                  if ( size > 48 )
                  {
                     const __m128i xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + 32) );
                     if ( size > 64 )
                     {
                        const __m128i xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + 48) );
                        if ( size > 80 )
                        {
                           const __m128i xmm4 = _mm_loadu_si128( (__m128i*)((char*)src + 64) );
                           if ( size > 96 )
                           {
                              const __m128i xmm5 = _mm_loadu_si128( (__m128i*)((char*)src + 80) );
                              *(long long*)((char*)dst + size - 16) = rax;
                              *(long long*)((char*)dst + size - 8) = rcx;
                              _mm_storeu_si128( (__m128i*)dst, xmm0 );
                              _mm_storeu_si128( (__m128i*)((char*)dst + 16), xmm1 );
                              _mm_storeu_si128( (__m128i*)((char*)dst + 32), xmm2 );
                              _mm_storeu_si128( (__m128i*)((char*)dst + 48), xmm3 );
                              _mm_storeu_si128( (__m128i*)((char*)dst + 64), xmm4 );
                              _mm_storeu_si128( (__m128i*)((char*)dst + 80), xmm5 );
                              return dst;
                           }
                           *(long long*)((char*)dst + size - 16) = rax;
                           *(long long*)((char*)dst + size - 8) = rcx;
                           _mm_storeu_si128( (__m128i*)dst, xmm0 );
                           _mm_storeu_si128( (__m128i*)((char*)dst + 16), xmm1 );
                           _mm_storeu_si128( (__m128i*)((char*)dst + 32), xmm2 );
                           _mm_storeu_si128( (__m128i*)((char*)dst + 48), xmm3 );
                           _mm_storeu_si128( (__m128i*)((char*)dst + 64), xmm4 );
                           return dst;
                        }
                        *(long long*)((char*)dst + size - 16) = rax;
                        *(long long*)((char*)dst + size - 8) = rcx;
                        _mm_storeu_si128( (__m128i*)dst, xmm0 );
                        _mm_storeu_si128( (__m128i*)((char*)dst + 16), xmm1 );
                        _mm_storeu_si128( (__m128i*)((char*)dst + 32), xmm2 );
                        _mm_storeu_si128( (__m128i*)((char*)dst + 48), xmm3 );
                        return dst;
                     }
                     *(long long*)((char*)dst + size - 16) = rax;
                     *(long long*)((char*)dst + size - 8) = rcx;
                     _mm_storeu_si128( (__m128i*)dst, xmm0 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + 16), xmm1 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + 32), xmm2 );
                     return dst;
                  }
                  *(long long*)((char*)dst + size - 16) = rax;
                  *(long long*)((char*)dst + size - 8) = rcx;
               }
               _mm_storeu_si128( (__m128i*)dst, xmm0 );
               _mm_storeu_si128( (__m128i*)((char*)dst + 16), xmm1 );
               return dst;
            }
            rax = *(long long*)((char*)src + size - 16);
            rcx = *(long long*)((char*)src + size - 8);
            *(long long*)((char*)dst + size - 16) = rax;
            *(long long*)((char*)dst + size - 8) = rcx;
         }
         _mm_storeu_si128( (__m128i*)dst, xmm0 );
         return dst;
      }
      if ( size >= 8 )
      {
         long long rax = *(long long*)src;
         if ( size > 8 )
         {
            long long rcx = *(long long*)((char*)src + size - 8);
            *(long long*)dst = rax;
            *(long long*)((char*)dst + size - 8) = rcx;
         }
         else *(long long*)dst = rax;
      }
      else if ( size >= 4 )
      {
         int eax = *(int*)src;
         if ( size > 4 )
         {
            int ecx = *(int*)((char*)src + size - 4);
            *(int*)dst = eax;
            *(int*)((char*)dst + size - 4) = ecx;
         }
         else *(int*)dst = eax;
      }
      else if ( size >= 1 )
      {
         char al = *(char*)src;
         if ( size > 1 )
         {
            short cx = *(short*)((char*)src + size - 2);
            *(char*)dst = al;
            *(short*)((char*)dst + size - 2) = cx;
         }
         else *(char*)dst = al;
      }
      return dst;
   }
   else
   {
      void * const ret = dst;
      if ( ((size_t)dst - (size_t)src) >= size )
      {
         if ( size < (1024 * 256) )
         {
            long long offset = (long long)(size & -0x20);/* "Round down to nearest multiple of 64" */
            dst = (char*)dst + offset;                   /* "Point to the end" */
            src = (char*)src + offset;                   /* "Point to the end" */
            size -= offset;                              /* "Remaining data after loop" */
            offset = -offset;                            /* "Negative index from the end" */

            do
            {
               const __m128i xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               _mm_storeu_si128( (__m128i*)((char*)dst + offset), xmm0 );
               _mm_storeu_si128( (__m128i*)((char*)dst + offset + 16), xmm1 );
            } while ( offset += 32 );

            if ( size >= 16 )
            {
               if ( size > 16 )
               {
                  const __m128i xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size - 16) );
                  const __m128i xmm0 = _mm_loadu_si128( (__m128i*)src );
                  _mm_storeu_si128( (__m128i*)((char*)dst + size - 16), xmm7 );
                  _mm_storeu_si128( (__m128i*)dst, xmm0 );
                  return ret;
               }
               _mm_storeu_si128( (__m128i*)dst, _mm_loadu_si128( (__m128i*)src ) );
               return ret;
            }
         }
         else  /* do forward streaming copy/move */
         {
            __m128i xmm0;
            __m128i xmm1;
            __m128i xmm2;
            __m128i xmm3;
            __m128i xmm6;
            __m128i xmm7;
            long long offset;
            /* We MUST do prealignment on streaming copies! */
            const size_t prealign = -(size_t)dst & 0xf;
            if ( prealign )
            {
               if ( prealign >= 8 )
               {
                  long long rax = *(long long*)src;
                  if ( prealign > 8 )
                  {
                     long long rcx = *(long long*)((char*)src + prealign - 8);
                     *(long long*)dst = rax;
                     *(long long*)((char*)dst + prealign - 8) = rcx;
                  }
                  else *(long long*)dst = rax;
               }
               else if ( prealign >= 4 )
               {
                  int eax = *(int*)src;
                  if ( prealign > 4 )
                  {
                     int ecx = *(int*)((char*)src + prealign - 4);
                     *(int*)dst = eax;
                     *(int*)((char*)dst + prealign - 4) = ecx;
                  }
                  else *(int*)dst = eax;
               }
               else
               {
                  char al = *(char*)src;
                  if ( prealign > 1 )
                  {
                     short cx = *(short*)((char*)src + prealign - 2);
                     *(char*)dst = al;
                     *(short*)((char*)dst + prealign - 2) = cx;
                  }
                  else *(char*)dst = al;
               }
               src = (char*)src + prealign;
               dst = (char*)dst + prealign;
               size -= prealign;
            }

            /* Begin prefetching upto 4KB */
            for ( offset = 0; offset < 4096; offset += 256 )
            {
               _mm_prefetch( ((char*)src + offset), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset + 64), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset + 128), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset + 192), _MM_HINT_NTA );
            }

            offset = (long long)(size & -0x40);    /* "Round down to nearest multiple of 64" */
            size -= offset;                        /* "Remaining data after loop" */
            offset -= 4096;                        /* stage 1 INCLUDES prefetches */
            dst = (char*)dst + offset;             /* "Point to the end" */
            src = (char*)src + offset;             /* "Point to the end" */
            offset = -offset;                      /* "Negative index from the end" */

            do                                     /* stage 1 ~~ WITH prefetching */
            {
               _mm_prefetch( (char*)src + offset + 4096, _MM_HINT_NTA );
               xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               _mm_stream_si128( (__m128i*)((char*)dst + offset), xmm0 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 16), xmm1 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 32), xmm2 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 48), xmm3 );
            } while ( offset += 64 );

            offset = -4096;
            dst = (char*)dst + 4096;
            src = (char*)src + 4096;

            _mm_prefetch( ((char*)src + size - 64), _MM_HINT_NTA );  /* prefetch the final tail section */

            do                                     /* stage 2 ~~ WITHOUT further prefetching */
            {
               xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               _mm_stream_si128( (__m128i*)((char*)dst + offset), xmm0 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 16), xmm1 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 32), xmm2 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 48), xmm3 );
            } while ( offset += 64 );

            if ( size >= 16 )
            {
               xmm0 = _mm_loadu_si128( (__m128i*)src );
               if ( size > 16 )
               {
                  if ( size > 32 )
                  {
                     xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + 16) );
                     xmm6 = _mm_loadu_si128( (__m128i*)((char*)src + size - 32) );
                     xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size - 16) );
                     _mm_stream_si128( (__m128i*)dst, xmm0 );
                     _mm_stream_si128( (__m128i*)((char*)dst + 16), xmm1 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size - 32), xmm6 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size - 16), xmm7 );
                     return ret;
                  }
                  xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size - 16) );
                  _mm_stream_si128( (__m128i*)dst, xmm0 );
                  _mm_storeu_si128( (__m128i*)((char*)dst + size - 16), xmm7 );
                  return ret;
               }
               _mm_stream_si128( (__m128i*)dst, xmm0 );
               return ret;
            }
         }

         if ( size >= 8 )
         {
            long long rax = *(long long*)src;
            if ( size > 8 )
            {
               long long rcx = *(long long*)((char*)src + size - 8);
               *(long long*)dst = rax;
               *(long long*)((char*)dst + size - 8) = rcx;
            }
            else *(long long*)dst = rax;
         }
         else if ( size >= 4 )
         {
            int eax = *(int*)src;
            if ( size > 4 )
            {
               int ecx = *(int*)((char*)src + size - 4);
               *(int*)dst = eax;
               *(int*)((char*)dst + size - 4) = ecx;
            }
            else *(int*)dst = eax;
         }
         else if ( size >= 1 )
         {
            char al = *(char*)src;
            if ( size > 1 )
            {
               short cx = *(short*)((char*)src + size - 2);
               *(char*)dst = al;
               *(short*)((char*)dst + size - 2) = cx;
            }
            else *(char*)dst = al;
         }
         return ret;
      }
      else /* src < dst ... do reverse copy */
      {
         src = (char*)src + size;
         dst = (char*)dst + size;

         if ( size < (1024 * 256) )
         {
            long long offset = (long long)(size & -0x20);/* "Round down to nearest multiple of 64" */
            dst = (char*)dst - offset;                   /* "Point to the end" ... actually, we point to the start! */
            src = (char*)src - offset;                   /* "Point to the end" ... actually, we point to the start! */
            size -= offset;                              /* "Remaining data after loop" */
            /*offset = -offset;                          // "Negative index from the end" ... not when doing reverse copy/move! */

            offset -= 32;
            do
            {
               const __m128i xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               const __m128i xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               _mm_storeu_si128( (__m128i*)((char*)dst + offset + 16), xmm2 );
               _mm_storeu_si128( (__m128i*)((char*)dst + offset), xmm3 );
            } while ( (offset -= 32) >= 0 );

            if ( size >= 16 )
            {
               if ( size > 16 )
               {
                  size = -size;
                  /* The order has been mixed so the compiler will not re-order the statements! */
                  const __m128i xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size) );
                  const __m128i xmm0 = _mm_loadu_si128( (__m128i*)((char*)src - 16) );
                  _mm_storeu_si128( (__m128i*)((char*)dst + size), xmm7 );
                  _mm_storeu_si128( (__m128i*)((char*)dst - 16), xmm0 );
                  return ret;
               }
               _mm_storeu_si128( (__m128i*)((char*)dst - 16), _mm_loadu_si128( (__m128i*)((char*)src - 16) ) );
               return ret;
            }
         }
         else  /* do reversed streaming copy/move */
         {
            __m128i xmm0;
            __m128i xmm1;
            __m128i xmm2;
            __m128i xmm3;
            __m128i xmm6;
            __m128i xmm7;
            long long offset;
            /* We MUST do prealignment on streaming copies! */
            const size_t prealign = (size_t)dst & 0xf;
            if ( prealign )
            {
               src = (char*)src - prealign;
               dst = (char*)dst - prealign;
               size -= prealign;
               if ( prealign >= 8 )
               {
                  long long rax = *(long long*)((char*)src + prealign - 8);
                  if ( prealign > 8 )
                  {
                     long long rcx = *(long long*)src;
                     *(long long*)((char*)dst + prealign - 8) = rax;
                     *(long long*)dst = rcx;
                  }
                  else *(long long*)dst = rax; /* different on purpose, because we know the exact size now, which is 8, and "dst" has already been aligned! */
               }
               else if ( prealign >= 4 )
               {
                  int eax = *(int*)((char*)src + prealign - 4);
                  if ( prealign > 4 )
                  {
                     int ecx = *(int*)src;
                     *(int*)((char*)dst + prealign - 4) = eax;
                     *(int*)dst = ecx;
                  }
                  else *(int*)dst = eax; /* different on purpose! */
               }
               else
               {
                  char al = *(char*)((char*)src + prealign - 1);
                  if ( prealign > 1 )
                  {
                     short cx = *(short*)src;
                     *(char*)((char*)dst + prealign - 1) = al;
                     *(short*)dst = cx;
                  }
                  else *(char*)dst = al; /* different on purpose! */
               }
            }

            /* Begin prefetching upto 4KB */
            for ( offset = 0; offset > -4096; offset -= 256 )
            {
               _mm_prefetch( ((char*)src + offset - 64), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset - 128), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset - 192), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset - 256), _MM_HINT_NTA );
            }

            offset = (long long)(size & -0x40);    /* "Round down to nearest multiple of 64" */
            size -= offset;                        /* "Remaining data after loop" */
            offset -= 4096;                        /* stage 1 INCLUDES prefetches */
            dst = (char*)dst - offset;             /* "Point to the end" ... actually, we point to the start! */
            src = (char*)src - offset;             /* "Point to the end" ... actually, we point to the start! */
            /*offset = -offset;                    // "Negative index from the end" ... not when doing reverse copy/move! */

            offset -= 64;
            do                                     /* stage 1 ~~ WITH prefetching */
            {
               _mm_prefetch( (char*)src + offset - 4096, _MM_HINT_NTA );
               xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 48), xmm0 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 32), xmm1 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 16), xmm2 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset), xmm3 );
            } while ( (offset -= 64) >= 0 );

            offset = 4096;
            dst = (char*)dst - 4096;
            src = (char*)src - 4096;

            _mm_prefetch( ((char*)src - 64), _MM_HINT_NTA );   /* prefetch the final tail section */

            offset -= 64;
            do                                     /* stage 2 ~~ WITHOUT further prefetching */
            {
               xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 48), xmm0 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 32), xmm1 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 16), xmm2 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset), xmm3 );
            } while ( (offset -= 64) >= 0 );

            if ( size >= 16 )
            {
               xmm0 = _mm_loadu_si128( (__m128i*)((char*)src - 16) );
               if ( size > 16 )
               {
                  if ( size > 32 )
                  {
                     size = -size;
                     xmm1 = _mm_loadu_si128( (__m128i*)((char*)src - 32) );
                     xmm6 = _mm_loadu_si128( (__m128i*)((char*)src + size + 16) );
                     xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size) );
                     _mm_stream_si128( (__m128i*)((char*)dst - 16), xmm0 );
                     _mm_stream_si128( (__m128i*)((char*)dst - 32), xmm1 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size + 16), xmm6 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size), xmm7 );
                     return ret;
                  }
                  size = -size;
                  xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size) );
                  _mm_stream_si128( (__m128i*)((char*)dst - 16), xmm0 );
                  _mm_storeu_si128( (__m128i*)((char*)dst + size), xmm7 );
                  return ret;
               }
               _mm_stream_si128( (__m128i*)((char*)dst - 16), xmm0 );
               return ret;
            }
         }

         if ( size >= 8 )
         {
            long long rcx;
            long long rax = *(long long*)((char*)src - 8);
            if ( size > 8 )
            {
               size = -size; /* that's right, we're converting an unsigned value to a negative, saves 2 clock cycles! */
               rcx = *(long long*)((char*)src + size);
               *(long long*)((char*)dst - 8) = rax;
               *(long long*)((char*)dst + size) = rcx;
            }
            else *(long long*)((char*)dst - 8) = rax;
         }
         else if ( size >= 4 )
         {
            int ecx;
            int eax = *(int*)((char*)src - 4);
            if ( size > 4 )
            {
               size = -size;
               ecx = *(int*)((char*)src + size);
               *(int*)((char*)dst - 4) = eax;
               *(int*)((char*)dst + size) = ecx;
            }
            else *(int*)((char*)dst - 4) = eax;
         }
         else if ( size >= 1 )
         {
            short cx;
            char al = *((char*)src - 1);
            if ( size > 1 )
            {
               size = -size;
               cx = *(short*)((char*)src + size);
               *((char*)dst - 1) = al;
               *(short*)((char*)dst + size) = cx;
            }
            else *((char*)dst - 1) = al;
         }
         return ret;
      }
   }
}

#  if (!defined(_M_X64) && !defined(__x86_64__))   /* `mithril` is only used in 32-bit code (not necessary for 64-bit) */

/**
 * "Mithril" - Written by Trevor Herselman on 4 December 2013 @ 9pm
 * This is my "compact power house"! Decided to make a "mini" powerful version, one that can easily beat Microsoft's memmove, but doesn't contain all the "bells and whistles"!
 * Probably Very similar to mm04 in terms of performance, but this should be faster under all conditions and ranges!
 * Includes a new "prealignment" check that detects if "size <= 48", then uses 3x SSE loads instead of passing through the prealignment code!
 * originally called memmove13() during development (mm13 is based on mm04)
 */
void* APEXCALL apex_mithril( void *dst, const void *src, size_t size )
{
   if ( size <= 32 )
   {
      if ( size >= 16 )
      {
         long long r1 = *(long long*)src;
         long long r2 = *(long long*)((char*)src + 8);
         if ( size > 16 )
         {
            long long r3 = *(long long*)((char*)src + size - 16);
            long long r4 = *(long long*)((char*)src + size - 8);
            *(long long*)dst = r1;
            *(long long*)((char*)dst + 8) = r2;
            *(long long*)((char*)dst + size - 16) = r3;
            *(long long*)((char*)dst + size - 8) = r4;
            return dst;
         }
         *(long long*)dst = r1;
         *(long long*)((char*)dst + 8) = r2;
         return dst;
      }
      if ( size >= 8 )
      {
         long long rax = *(long long*)src;
         if ( size > 8 )
         {
            long long rcx = *(long long*)((char*)src + size - 8);
            *(long long*)dst = rax;
            *(long long*)((char*)dst + size - 8) = rcx;
         }
         else *(long long*)dst = rax;
      }
      else if ( size >= 4 )
      {
         int eax = *(int*)src;
         if ( size > 4 )
         {
            int ecx = *(int*)((char*)src + size - 4);
            *(int*)dst = eax;
            *(int*)((char*)dst + size - 4) = ecx;
         }
         else *(int*)dst = eax;
      }
      else if ( size >= 1 )
      {
         char al = *(char*)src;
         if ( size > 1 )
         {
            short cx = *(short*)((char*)src + size - 2);
            *(char*)dst = al;
            *(short*)((char*)dst + size - 2) = cx;
         }
         else *(char*)dst = al;
      }
      return dst;
   }
   else
   {
      void * const ret = dst;
      if ( ((size_t)dst - (size_t)src) >= size )
      {
         if ( (size_t)dst & 0xf )
         {
            const size_t prealign = -(size_t)dst & 0xf;
            if ( size <= 48 )
            {
               /**
                * prealignment might drop the remaining size below 32-bytes,
                * we could also check "size - prealign <= 32", but since we're already here,
                * and these statements can load upto 48-bytes, we might as well just do it!
                * We `could` copy up to 64-bytes without any additional checks by using another SSE "loadu" & "storeu"
                */
               const __m128i xmm0 = _mm_loadu_si128( (__m128i*)src );
               long long r1 = *(long long*)((char*)src + 16);
               long long r2 = *(long long*)((char*)src + 24);
               long long r3 = *(long long*)((char*)src + size - 16);
               long long r4 = *(long long*)((char*)src + size - 8);
               _mm_storeu_si128( (__m128i*)dst, xmm0 );
               *(long long*)((char*)dst + 16) = r1;
               *(long long*)((char*)dst + 24) = r2;
               *(long long*)((char*)dst + size - 16) = r3;
               *(long long*)((char*)dst + size - 8) = r4;
               return dst;
            }

            if ( prealign >= 8 )
            {
               long long rax = *(long long*)src;
               if ( prealign > 8 )
               {
                  long long rcx = *(long long*)((char*)src + prealign - 8);
                  *(long long*)dst = rax;
                  *(long long*)((char*)dst + prealign - 8) = rcx;
               }
               else *(long long*)dst = rax;
            }
            else if ( prealign >= 4 )
            {
               int eax = *(int*)src;
               if ( prealign > 4 )
               {
                  int ecx = *(int*)((char*)src + prealign - 4);
                  *(int*)dst = eax;
                  *(int*)((char*)dst + prealign - 4) = ecx;
               }
               else *(int*)dst = eax;
            }
            else
            {
               char al = *(char*)src;
               if ( prealign > 1 )
               {
                  short cx = *(short*)((char*)src + prealign - 2);
                  *(char*)dst = al;
                  *(short*)((char*)dst + prealign - 2) = cx;
               }
               else *(char*)dst = al;
            }
            src = (char*)src + prealign;
            dst = (char*)dst + prealign;
            size -= prealign;
         }

         if ( size < (1024 * 256) )
         {
            long long offset = (long long)(size & -0x20);/* "Round down to nearest multiple of 32" */
            dst = (char*)dst + offset;                   /* "Point to the end" */
            src = (char*)src + offset;                   /* "Point to the end" */
            size -= offset;                              /* "Remaining data after loop" */
            offset = -offset;                            /* "Negative index from the end" */

            if ( ((size_t)src & 0xf) == 0 )
            {
               do
               {
                  const __m128i xmm0 = _mm_load_si128( (__m128i*)((char*)src + offset) );
                  const __m128i xmm1 = _mm_load_si128( (__m128i*)((char*)src + offset + 16) );
                  _mm_store_si128( (__m128i*)((char*)dst + offset), xmm0 );
                  _mm_store_si128( (__m128i*)((char*)dst + offset + 16), xmm1 );
               } while ( offset += 32 );

               if ( size >= 16 )
               {
                  const __m128i xmm0 = _mm_load_si128( (__m128i*)src );
                  if ( size > 16 )
                  {
                     long long rax = *(long long*)((char*)src + size - 16);
                     long long rcx = *(long long*)((char*)src + size - 8);
                     *(long long*)((char*)dst + size - 16) = rax;
                     *(long long*)((char*)dst + size - 8) = rcx;
                  }
                  _mm_store_si128( (__m128i*)dst, xmm0 );
                  return ret;
               }
            }
            else
            {
               do
               {
                  const __m128i xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
                  const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
                  _mm_store_si128( (__m128i*)((char*)dst + offset), xmm0 );
                  _mm_store_si128( (__m128i*)((char*)dst + offset + 16), xmm1 );
               } while ( offset += 32 );

               if ( size >= 16 )
               {
                  const __m128i xmm0 = _mm_loadu_si128( (__m128i*)src );
                  if ( size > 16 )
                  {
                     long long rax = *(long long*)((char*)src + size - 16);
                     long long rcx = *(long long*)((char*)src + size - 8);
                     *(long long*)((char*)dst + size - 16) = rax;
                     *(long long*)((char*)dst + size - 8) = rcx;
                  }
                  _mm_store_si128( (__m128i*)dst, xmm0 );
                  return (void*)ret;
               }
            }
         }
         else  /* do forward streaming copy/move */
         {
            /* Begin prefetching upto 4KB */
            for ( long long i = 0; i < 4096; i += 256 )
            {
               _mm_prefetch( ((char*)src + i), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + i + 64), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + i + 128), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + i + 192), _MM_HINT_NTA );
            }

            long long offset = (long long)(size & -0x40);/* "Round down to nearest multiple of 64" */
            size -= offset;                              /* "Remaining data after loop" */
            offset -= 4096;                              /* stage 1 INCLUDES prefetches */
            dst = (char*)dst + offset;                   /* "Point to the end" */
            src = (char*)src + offset;                   /* "Point to the end" */
            offset = -offset;                            /* "Negative index from the end" */

            do                                     /* stage 1 ~~ WITH prefetching */
            {
               _mm_prefetch( (char*)src + offset + 4096, _MM_HINT_NTA );
               const __m128i xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               const __m128i xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               const __m128i xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               _mm_stream_si128( (__m128i*)((char*)dst + offset), xmm0 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 16), xmm1 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 32), xmm2 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 48), xmm3 );
            } while ( offset += 64 );

            offset = -4096;
            dst = (char*)dst + 4096;
            src = (char*)src + 4096;

            _mm_prefetch( ((char*)src + size - 64), _MM_HINT_NTA );  /* prefetch the final tail section */

            do                                     /* stage 2 ~~ WITHOUT further prefetching */
            {
               const __m128i xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               const __m128i xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               const __m128i xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               _mm_stream_si128( (__m128i*)((char*)dst + offset), xmm0 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 16), xmm1 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 32), xmm2 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 48), xmm3 );
            } while ( offset += 64 );

            if ( size >= 16 )
            {
               const __m128i xmm0 = _mm_loadu_si128( (__m128i*)src );
               if ( size > 16 )
               {
                  if ( size > 32 )
                  {
                     const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + 16) );
                     const __m128i xmm6 = _mm_loadu_si128( (__m128i*)((char*)src + size - 32) );
                     const __m128i xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size - 16) );
                     _mm_stream_si128( (__m128i*)dst, xmm0 );
                     _mm_stream_si128( (__m128i*)((char*)dst + 16), xmm1 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size - 32), xmm6 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size - 16), xmm7 );
                     return ret;
                  }
                  const __m128i xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size - 16) );
                  _mm_stream_si128( (__m128i*)dst, xmm0 );
                  _mm_storeu_si128( (__m128i*)((char*)dst + size - 16), xmm7 );
                  return ret;
               }
               _mm_stream_si128( (__m128i*)dst, xmm0 );
               return ret;
            }
         }

         if ( size >= 8 )
         {
            long long rax = *(long long*)src;
            if ( size > 8 )
            {
               long long rcx = *(long long*)((char*)src + size - 8);
               *(long long*)dst = rax;
               *(long long*)((char*)dst + size - 8) = rcx;
            }
            else *(long long*)dst = rax;
         }
         else if ( size >= 4 )
         {
            int eax = *(int*)src;
            if ( size > 4 )
            {
               int ecx = *(int*)((char*)src + size - 4);
               *(int*)dst = eax;
               *(int*)((char*)dst + size - 4) = ecx;
            }
            else *(int*)dst = eax;
         }
         else if ( size >= 1 )
         {
            char al = *(char*)src;
            if ( size > 1 )
            {
               short cx = *(short*)((char*)src + size - 2);
               *(char*)dst = al;
               *(short*)((char*)dst + size - 2) = cx;
            }
            else *(char*)dst = al;
         }
         return ret;
      }
      else /* src < dst ... do reverse copy */
      {
         src = (char*)src + size;
         dst = (char*)dst + size;

         const size_t prealign = (size_t)dst & 0xf;
         if ( prealign )
         {
            if ( size <= 48 )
            {
               /**
                * prealignment might drop the remaining size below 32-bytes,
                * we could also check "size - prealign <= 32", but since we're already here,
                * and these statements can load upto 48-bytes, we might as well just do it!
                * Actually, we can copy upto 64-bytes without any additional checks!
                */
               size = -size;
               const __m128i xmm0 = _mm_loadu_si128( (__m128i*)((char*)src - 16) ); /* first 32-bytes in reverse */
               const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src - 32) );
               const __m128i xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size) );
               _mm_storeu_si128( (__m128i*)((char*)dst - 16), xmm0 );
               _mm_storeu_si128( (__m128i*)((char*)dst - 32), xmm1 );
               _mm_storeu_si128( (__m128i*)((char*)dst + size), xmm7 ); /* the "back" bytes are actually the base address! */
               return dst;
            }

            src = (char*)src - prealign;
            dst = (char*)dst - prealign;
            size -= prealign;
            if ( prealign >= 8 )
            {
               long long rax = *(long long*)((char*)src + prealign - 8);
               if ( prealign > 8 )
               {
                  long long rcx = *(long long*)src;
                  *(long long*)((char*)dst + prealign - 8) = rax;
                  *(long long*)dst = rcx;
               }
               else *(long long*)dst = rax; /* different on purpose, because we know the exact size now! */
            }
            else if ( prealign >= 4 )
            {
               int eax = *(int*)((char*)src + prealign - 4);
               if ( prealign > 4 )
               {
                  int ecx = *(int*)src;
                  *(int*)((char*)dst + prealign - 4) = eax;
                  *(int*)dst = ecx;
               }
               else *(int*)dst = eax; /* different on purpose! */
            }
            else
            {
               char al = *(char*)((char*)src + prealign - 1);
               if ( prealign > 1 )
               {
                  short cx = *(short*)src;
                  *(char*)((char*)dst + prealign - 1) = al;
                  *(short*)dst = cx;
               }
               else *(char*)dst = al; /* different on purpose! */
            }
         }

         if ( size < (1024 * 256) )
         {
            long long offset = (long long)(size & -0x20);/* "Round down to nearest multiple of 32" */
            dst = (char*)dst - offset;                   /* "Point to the end" ... actually, we point to the start! */
            src = (char*)src - offset;                   /* "Point to the end" ... actually, we point to the start! */
            size -= offset;                              /* "Remaining data after loop" */
            /*offset = -offset;                          // "Negative index from the end" ... not when doing reverse copy/move! */

            offset -= 32;                                /* MSVC completely re-engineers this! */

            if ( ((size_t)src & 0xf) == 0 )
            {
               do
               {
                  const __m128i xmm0 = _mm_load_si128( (__m128i*)((char*)src + offset + 16) );
                  const __m128i xmm1 = _mm_load_si128( (__m128i*)((char*)src + offset) );
                  _mm_store_si128( (__m128i*)((char*)dst + offset + 16), xmm0 );
                  _mm_store_si128( (__m128i*)((char*)dst + offset), xmm1 );
               } while ( (offset -= 32) >= 0 );

               if ( size >= 16 )
               {
                  const __m128i xmm0 = _mm_load_si128( (__m128i*)((char*)src - 16) );
                  if ( size > 16 )
                  {
                     size = -size;
                     /*const __m128i xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size) ); // SSE2 alternative */
                     long long rax = *(long long*)((char*)src + size + 8);
                     long long rcx = *(long long*)((char*)src + size);
                     /*_mm_storeu_si128( (__m128i*)((char*)dst + size), xmm7 ); */
                     *(long long*)((char*)dst + size + 8) = rax;
                     *(long long*)((char*)dst + size) = rcx;
                  }
                  _mm_store_si128( (__m128i*)((char*)dst - 16), xmm0 );
                  return ret;
               }
            }
            else
            {
               do
               {
                  const __m128i xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
                  const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
                  _mm_store_si128( (__m128i*)((char*)dst + offset + 16), xmm0 );
                  _mm_store_si128( (__m128i*)((char*)dst + offset), xmm1 );
               } while ( (offset -= 32) >= 0 );

               if ( size >= 16 )
               {
                  const __m128i xmm0 = _mm_loadu_si128( (__m128i*)((char*)src - 16) );
                  if ( size > 16 )
                  {
                     size = -size;
                     long long rax = *(long long*)((char*)src + size + 8);
                     long long rcx = *(long long*)((char*)src + size);
                     *(long long*)((char*)dst + size + 8) = rax;
                     *(long long*)((char*)dst + size) = rcx;
                  }
                  _mm_store_si128( (__m128i*)((char*)dst - 16), xmm0 );
                  return ret;
               }
            }
         }
         else  /* do reversed streaming copy/move */
         {
            /* Begin prefetching upto 4KB */
            for ( long long offset = 0; offset > -4096; offset -= 256 )
            {
               _mm_prefetch( ((char*)src + offset - 64), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset - 128), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset - 192), _MM_HINT_NTA );
               _mm_prefetch( ((char*)src + offset - 256), _MM_HINT_NTA );
            }

            long long offset = (long long)(size & -0x40);/* "Round down to nearest multiple of 32" */
            size -= offset;                              /* "Remaining data after loop" */
            offset -= 4096;                              /* stage 1 INCLUDES prefetches */
            dst = (char*)dst - offset;                   /* "Point to the end" ... actually, we point to the start! */
            src = (char*)src - offset;                   /* "Point to the end" ... actually, we point to the start! */
            /*offset = -offset;                          // "Negative index from the end" ... not when doing reverse copy/move! */

            offset -= 64;
            do                                     /* stage 1 ~~ WITH prefetching */
            {
               _mm_prefetch( (char*)src + offset - 4096, _MM_HINT_NTA );
               const __m128i xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               const __m128i xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               const __m128i xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 48), xmm0 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 32), xmm1 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 16), xmm2 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset), xmm3 );
            } while ( (offset -= 64) >= 0 );

            offset = 4096;
            dst = (char*)dst - 4096;
            src = (char*)src - 4096;

            _mm_prefetch( ((char*)src - 64), _MM_HINT_NTA );   /* prefetch the final tail section */

            offset -= 64;
            do                                     /* stage 2 ~~ WITHOUT further prefetching */
            {
               const __m128i xmm0 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 48) );
               const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 32) );
               const __m128i xmm2 = _mm_loadu_si128( (__m128i*)((char*)src + offset + 16) );
               const __m128i xmm3 = _mm_loadu_si128( (__m128i*)((char*)src + offset) );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 48), xmm0 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 32), xmm1 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset + 16), xmm2 );
               _mm_stream_si128( (__m128i*)((char*)dst + offset), xmm3 );
            } while ( (offset -= 64) >= 0 );

            if ( size >= 16 )
            {
               const __m128i xmm0 = _mm_loadu_si128( (__m128i*)((char*)src - 16) );
               if ( size > 16 )
               {
                  if ( size > 32 )
                  {
                     size = -size;
                     const __m128i xmm1 = _mm_loadu_si128( (__m128i*)((char*)src - 32) );
                     const __m128i xmm6 = _mm_loadu_si128( (__m128i*)((char*)src + size + 16) );
                     const __m128i xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size) );
                     _mm_stream_si128( (__m128i*)((char*)dst - 16), xmm0 );
                     _mm_stream_si128( (__m128i*)((char*)dst - 32), xmm1 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size + 16), xmm6 );
                     _mm_storeu_si128( (__m128i*)((char*)dst + size), xmm7 );
                     return ret;
                  }
                  size = -size;
                  const __m128i xmm7 = _mm_loadu_si128( (__m128i*)((char*)src + size) );
                  _mm_stream_si128( (__m128i*)((char*)dst - 16), xmm0 );
                  _mm_storeu_si128( (__m128i*)((char*)dst + size), xmm7 );
                  return ret;
               }
               _mm_stream_si128( (__m128i*)((char*)dst - 16), xmm0 );
               return ret;
            }
         }

         if ( size >= 8 )
         {
            long long rax = *(long long*)((char*)src - 8);
            if ( size > 8 )
            {
               size = -size; /* that's right, we're converting an unsigned value to a negative, saves 2 clock cycles! */
               long long rcx = *(long long*)((char*)src + size);
               *(long long*)((char*)dst - 8) = rax;
               *(long long*)((char*)dst + size) = rcx;
            }
            else *(long long*)((char*)dst - 8) = rax;
         }
         else if ( size >= 4 )
         {
            int eax = *(int*)((char*)src - 4);
            if ( size > 4 )
            {
               size = -size;
               int ecx = *(int*)((char*)src + size);
               *(int*)((char*)dst - 4) = eax;
               *(int*)((char*)dst + size) = ecx;
            }
            else *(int*)((char*)dst - 4) = eax;
         }
         else if ( size >= 1 )
         {
            char al = *((char*)src - 1);
            if ( size > 1 )
            {
               size = -size;
               short cx = *(short*)((char*)src + size);
               *((char*)dst - 1) = al;
               *(short*)((char*)dst + size) = cx;
            }
            else *((char*)dst - 1) = al;
         }
         return ret;
      }
   }
}

#  endif /* end test for 32-bit */
#  endif /* end test for Intel/AMD (32-bit & 64-bit) */

#  if defined(_MSC_VER)
#  pragma warning( pop )
#  endif

void* (APEXCALL *apex_memcpy)( void* dst, const void* src, size_t size);
void* (APEXCALL *apex_memmove)(void* dst, const void* src, size_t size);

void apex_memmove_dispatcher(void)
{
#if (defined(_M_X64) || defined(__x86_64__) || defined(__i386) || defined(_M_IX86)) && (defined(_MSC_VER) || defined(__GNUC__))  /* Compiler and architecture test (Intel/AMD Architecture)! Visual Studio and GCC / LLVM (Clang) */
# if defined(_M_X64) || defined(__x86_64__) /* 64-bit */
   if (u_flag_sse == 42) /* detect SSE4.2, available on Core i and newer processors, they include "fast unaligned" memory access */
      {
      apex_memcpy  =
      apex_memmove = &apex_kryptonite;
      }
   else
      {
      apex_memcpy  =
      apex_memmove = &apex_tiberium;
      }
# else /* 32-bit */
   if (u_flag_sse == 42) /* detect SSE4.2, available on Core i and newer processors, they include "fast unaligned" memory access */
      {
      apex_memcpy  =
      apex_memmove = &apex_kryptonite;
      }
   else if (u_flag_sse == 3) /* detect SSSE3, available on Core/Core 2 and newer */
      {
      apex_memcpy  =
      apex_memmove = &apex_tiberium; /* `tiberium` CAN run with the SSE2 instructions alone, however there was something about very old SSE2 (only) computers, some penalty when using `tiberiums` algorithm, so I've restricted `tiberium` to Core/Core 2 based machines on 32-bit by doing this! */
      }
   else if (u_flag_sse == 2)
      {
      apex_memcpy  =
      apex_memmove = &apex_mithril; /* this is for very, very old computers with SSE2 only! eg. old Pentium 4! There was something about computers WITHOUT SSSE3 (Core microarchitecture) that made `tiberium` a bit slower. So this is for super old (P4) PC's! */
      }
   else
      {
      apex_memcpy  = &memcpy; /* No SSE2 no cry! */
      apex_memmove = &memmove;
      }
# endif
#else /* unknown compiler or architecture! eg. __arm__ / __mips__ / __ppc__ / __sparc / __ia64 (Itanium) etc. */
# if defined(__x86_64__)
   apex_memcpy  =
   apex_memmove = &apex_tiberium; /* unknown compiler BUT it declared support for Intel/AMD x64/AMD64 (64-bit), so it should come with the <emmintrin.h> file from Intel! 64-bit x64/AMD64 includes SSE2! This includes `Portland (PGCC/PGCPP)`, `Oracle Solaris Studio` and `Intel compiler (ICC)`! I don't have code for CPUID for these compilers so I don't know how to check for SSE2/SSSE3/SSE4.2 on them! */
# else
   apex_memcpy  = &memcpy; /* unknown compiler or architecture! */
   apex_memmove = &memmove;
# endif
#endif
}
#endif
