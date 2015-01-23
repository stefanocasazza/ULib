/* nanosleep.c */

#include <ulib/base/base.h>

#include <errno.h>
#include <time.h>

/* Pause execution for a number of nanoseconds */

extern U_EXPORT int nanosleep(const struct timespec* requested_time, struct timespec* remaining);
       U_EXPORT int nanosleep(const struct timespec* requested_time, struct timespec* remaining)
{
   int result;

   /* Some code calls select() with all three sets empty, nfds zero,
    * and a non-NULL timeout as a fairly portable way to sleep with subsecond precision.
    */

   ((struct timespec*)requested_time)->tv_nsec /= 1000; /* nanoseconds to microseconds */

#ifdef _MSWINDOWS_
   result = select_w32(0, 0, 0, 0, (struct timeval*)requested_time);
#else
   result =     select(0, 0, 0, 0, (struct timeval*)requested_time);
#endif

   if (remaining)
      {
      if (result == 0) remaining->tv_sec = remaining->tv_nsec = 0;
      else
         {
         remaining->tv_sec  = requested_time->tv_sec;
         remaining->tv_nsec = requested_time->tv_nsec * 1000; /* microseconds to nanoseconds */
         }
      }

   return result;
}
