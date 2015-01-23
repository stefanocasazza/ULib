/* timegm.c */

#include <ulib/base/base.h>

#include <time.h>

extern U_EXPORT time_t timegm(struct tm* tm);
       U_EXPORT time_t timegm(struct tm* tm)
{
   time_t ret;

   char* tz = getenv("TZ");

   setenv("TZ", "", 1);

   tzset();

   ret = mktime(tm);

   if (tz) setenv("TZ", tz, 1);
   else
      {
      unsetenv("TZ");

      tzset();
      }

   return ret;
}
