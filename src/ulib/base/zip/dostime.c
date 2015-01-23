/* dostime.c - convert dos time to/from time_t */

#include <ulib/base/base.h>
#include <ulib/base/zip/dostime.h>

#include <time.h>

/*
 * The specification to which this was written.  From Joe Buck.
 *
 * The DOS format appears to have only 2 second resolution.  It is an unsigned long, and ORs together
 * 
 * (year-1980)<<25
 * month<<21  (month is tm_mon + 1, 1=Jan through 12=Dec)
 * day<<16    (day is tm_mday, 1-31)
 * hour<<11   (hour is tm_hour, 0-23)
 * min<<5     (min is tm_min, 0-59)
 * sec>>1     (sec is tm_sec, 0-59, that's right, we throw away the LSB)
 * 
 * DOS uses local time, so the localtime() call is used to turn the time_t into a struct tm.
 */

time_t dos2unixtime(unsigned long dostime)
{
   struct tm ltime;

   U_gettimeofday; /* NB: optimization if it is enough a time resolution of one second... */

   /* Call localtime to initialize timezone in TIME */
   ltime = *localtime(&(u_now->tv_sec));

   ltime.tm_year =  (dostime >> 25) + 80;
   ltime.tm_mon  = ((dostime >> 21) & 0x0f) - 1;
   ltime.tm_mday =  (dostime >> 16) & 0x1f;
   ltime.tm_hour =  (dostime >> 11) & 0x0f;
   ltime.tm_min  =  (dostime >>  5) & 0x3f;
   ltime.tm_sec  =  (dostime & 0x1f) << 1;

   ltime.tm_wday  = -1;
   ltime.tm_yday  = -1;
   ltime.tm_isdst = -1;

   return mktime(&ltime);
}

unsigned long unix2dostime(time_t* _time)
{
   struct tm* ltime = localtime(_time);
   int year         = ltime->tm_year - 80;

   if (year < 0) year = 0;

   return (year << 25                |
           (ltime->tm_mon + 1) << 21 |
            ltime->tm_mday     << 16 |
            ltime->tm_hour     << 11 |
            ltime->tm_min      <<  5 |
            ltime->tm_sec      >> 1);
}
