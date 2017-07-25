// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    date.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/date.h>

const char*    UTimeDate::periods[8]    = { "second", "minute", "hour", "day", "week", "month",   "year", "decade" };
const uint32_t UTimeDate::lengths[8]    = {        1,       60,   3600, 86400, 604800, 2630880, 31570560, 315705600 };
const short    UTimeDate::monthDays[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

__pure bool UTimeDate::isValid() const
{
   U_TRACE_NO_PARAM(0, "UTimeDate::isValid()")

   U_CHECK_MEMORY

   if ( _year <  FIRST_YEAR ||
       (_year == FIRST_YEAR &&
        (_month <  9        ||
        (_month == 9        &&
         _day < 14))))
      {
      U_RETURN(false);
      }

   if ((_day   >   0              &&
        _month >   0              &&
        _month <= 12)             &&
       (_day <= monthDays[_month] ||
       (_day == 29                &&
        _month == 2               &&
        leapYear(_year))))
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

int UTimeDate::toJulian(int day, int month, int year)
{
   U_TRACE(0, "UTimeDate::toJulian(%d,%d,%d)", day, month, year)

   U_INTERNAL_ASSERT_RANGE(1,     day,   31)
   U_INTERNAL_ASSERT_RANGE(1,   month,   12)
   U_INTERNAL_ASSERT_RANGE(1752, year, 8000)

   int _julian, century;

   /*
   _julian = day - 32075l +
            1461l * (year  + 4800l + (month - 14l) / 12l) /   4l +
             367l * (month - 2l    - (month - 14l) / 12l  *  12l) / 12l -
               3l * ((year + 4900l + (month - 14l) / 12l) / 100l) / 4l;

   U_INTERNAL_DUMP("_julian = %d", _julian)
   */

   // -----------------------------------------------------------------------
   // algorithm 199 from Communications of the ACM, Volume 6, No. 8
   //
   // (Aug. 1963), p. 444. Gregorian calendar started on 14 Sep 1752
   // -----------------------------------------------------------------------

   month += (month > 2 ? -3 : (--year, 9));

   century = year / 100;

   year -= century * 100;

   _julian = 1721119 + day + ((146097 * century) / 4) +
                             ((  1461 * year)    / 4) +
                             ((   153 * month) + 2) / 5;

   U_INTERNAL_ASSERT(_julian >= FIRST_DAY)

   U_RETURN(_julian);
}

// gcc: call is unlikely and code size would grow

bool UTimeDate::operator!=(UTimeDate& date) { return getJulian() != date.getJulian(); }
bool UTimeDate::operator< (UTimeDate& date) { return getJulian() <  date.getJulian(); }
bool UTimeDate::operator<=(UTimeDate& date) { return getJulian() <= date.getJulian(); }
bool UTimeDate::operator> (UTimeDate& date) { return getJulian() >  date.getJulian(); }
bool UTimeDate::operator>=(UTimeDate& date) { return getJulian() >= date.getJulian(); }

void UTimeDate::fromJulian(int j)
{
   U_TRACE(0, "UTimeDate::fromJulian(%d)", j)

   /*
   int t1, t2;

   t1    = j + 32075;
   _year = 4 * t1 / 1461;

   t1     = t1 - 1461 * _year / 4;
   t2     = 3 * (_year + 100) / 400;
   t1     = t1 + t2;
   _month = 12 * t1 / 367;
   t2     = _month / 11;

   _day   = t1 - 367 * _month / 12;
   _month = _month + 2 - 12 * t2;
   _year  = _year - 4800 + t2;

   U_INTERNAL_DUMP("_day = %d, _month = %d, _year = %d", _day, _month, _year)
   */

   j      -= 1721119;
   _year   = ((j * 4) - 1) /  146097;
   j       =  (j * 4) - 1  - (146097 * _year);
   _day    =   j / 4;
   j       = ((_day * 4) + 3) /  1461;
   _day    =  (_day * 4) + 3  - (1461 * j);
   _day    =  (_day + 4) / 4;
   _month  = (5 * _day - 3) / 153;
   _day    =  5 * _day - 3  - 153 * _month;
   _day    = (_day + 5) / 5;
   _year   = 100 * _year + j;
   _month += (_month < 10 ? 3 : (++_year, -9));

   U_INTERNAL_DUMP("_day = %d, _month = %d, _year = %d", _day, _month, _year)

   U_INTERNAL_ASSERT_RANGE(1,     _day,   31)
   U_INTERNAL_ASSERT_RANGE(1,   _month,   12)
   U_INTERNAL_ASSERT_RANGE(1752, _year, 8000)

   U_INTERNAL_ASSERT(isValid())
}

int UTimeDate::checkForDaylightSavingTime(time_t sec)
{
   U_TRACE(0, "UTimeDate::checkForDaylightSavingTime(%ld)", sec)

   U_INTERNAL_DUMP("u_now_adjust = %d timezone = %ld daylight = %d u_daylight = %b tzname[2] = { %s, %s }",
                   *u_pnow_adjust,    timezone,      daylight,  u_is_daylight(),   tzname[0], tzname[1])

   U_INTERNAL_ASSERT(daylight)
   U_INTERNAL_ASSERT_EQUALS(sec % U_ONE_HOUR_IN_SECOND, 0)

   int now_adjust = *u_pnow_adjust; /* GMT based time */

   getLocalTime(sec);

   /**
    * The timegm() function converts the broken-down time representation, expressed in Coordinated Universal Time (UTC) to calendar time
    */

   (void) u_getLocalNow(sec);

   U_INTERNAL_DUMP("now_adjust = %d *u_pnow_adjust = %d", now_adjust, *u_pnow_adjust)

   if (now_adjust == *u_pnow_adjust) U_RETURN(0);

   if (now_adjust < *u_pnow_adjust)
      {
      *u_pdaylight = true;

      U_DEBUG("DST start: %#5D", sec)

      U_RETURN(1);
      }

   *u_pdaylight = false;

   U_DEBUG("DST end: %#5D", sec)

   U_RETURN(-1);
}

// UTC is flag for date and time in Coordinated Universal Time : format YYMMDDMMSSZ

void UTimeDate::fromUTC(const char* str, const char* format)
{
   U_TRACE(1, "UTimeDate::fromUTC(%S,%S)", str, format)

   int year,
       scanned = (str && *str ? U_SYSCALL(sscanf, "%S,%S,%p,%p,%p", str, format, &_day, &_month, &year) : 0);

   if (scanned == 0) return; 
   if (scanned == 3) setYear(year);
   else
      {
      // NB: we complete on behalf of the user

      U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

      getLocalTime(u_now->tv_sec);

      if (scanned == 1)
         {
         _month = u_strftime_tm.tm_mon  + 1;
         _year  = u_strftime_tm.tm_year + 1900;
         }
      else
         {
         U_INTERNAL_ASSERT_EQUALS(scanned, 2)

         _year  = u_strftime_tm.tm_year + 1900;
         }
      }
}

UString UTimeDate::strftime(const char* fmt, uint32_t fmt_size, time_t t, bool blocale)
{
   U_TRACE(0, "UTimeDate::strftime(%.*S,%u,%ld,%b)", fmt_size, fmt, fmt_size, t, blocale)

   UString res(100U);

   res.rep->_length = u_strftime2(res.data(), res.capacity(), fmt, fmt_size, t + (blocale ? *u_pnow_adjust : 0));

#ifdef DEBUG
   char dbg[4096];

   /* NB: strftime(3) call stat("etc/localtime") everytime... */

   if (blocale) getLocalTime(t);
   else
      {
      (void) gmtime_r(&t, &u_strftime_tm);
      }

   if (::strftime(dbg, sizeof(dbg), fmt, &u_strftime_tm))
      {
      U_INTERNAL_DUMP("res = %s", res.data())
      U_INTERNAL_DUMP("dbg = %s", dbg)
      U_INTERNAL_DUMP("u_now_adjust = %d timezone = %ld daylight = %d u_daylight = %b tzname[2] = { %s, %s }",
                      *u_pnow_adjust,    timezone,      daylight,  u_is_daylight(),   tzname[0], tzname[1])

      U_INTERNAL_ASSERT_EQUALS(strcmp(res.data(),dbg), 0)
      }
#endif

   U_RETURN_STRING(res);
}

time_t UTimeDate::getSecondFromDate(const char* str, bool gmt, struct tm* tm, const char* fmt)
{
   U_TRACE(1, "UTimeDate::getSecondFromDate(%S,%b,%p,%S)", str, gmt, tm, fmt)

   if (tm == U_NULLPTR)
      {
      static struct tm tm_;

      tm = &tm_;
      }

   time_t t;

   if (gmt)
      {
      if (str[3] == ',')
         {
         /**
          * Fri, 31 Dec 1999 23:59:59 GMT
          * |  | |  |   |    |  |  | 
          * 0  3 5  8  12   17 20 23
          */

         str += 5;

         tm->tm_mday = u_strtoulp(&str);

         ++str;

         tm->tm_mon  = u_getMonth(str);

         str += 4;

         tm->tm_year = u__strtoul(str, 4);

         str += 5;

         tm->tm_hour = u__strtoul(str, 2);

         str += 3;

         tm->tm_min = u__strtoul(str, 2);

         str += 3;

         tm->tm_sec = u__strtoul(str, 2);
         }
      else if ((tm->tm_mon = u_getMonth(str)))
         {
         /**
          * Jan 25 11:54:00 2005 GMT
          * |   |  |  |  |  |
          * 0   4  7 10 13 16
          */

         str += 4;

         tm->tm_mday = u_strtoulp(&str);

         ++str;

         tm->tm_hour = u__strtoul(str, 2);

         str += 3;

         tm->tm_min = u__strtoul(str, 2);

         str += 3;

         tm->tm_sec = u__strtoul(str, 2);

         str += 3;

         tm->tm_year = u__strtoul(str, 4);
         }
      else
         {
         /**
          * 100212124550Z (zulu time)
          * | | | | | |
          * 0 2 4 6 8 10
          */

         tm->tm_mday = u__strtoul(str, 2);

         str += 2;

         tm->tm_mon = u__strtoul(str, 2);

         str += 2;

         tm->tm_year = u__strtoul(str, 2) + 2000; // ts.tm_year is number of years since 1900

      // if (tm->tm_year > 2050) tm->tm_year -= 100;

         str += 2;

         tm->tm_hour = u__strtoul(str, 2);

         str += 2;

         tm->tm_min = u__strtoul(str, 2);

         str += 2;

         tm->tm_sec = u__strtoul(str, 2);

         U_INTERNAL_ASSERT_EQUALS(str[2], 'Z')
         }

      U_INTERNAL_DUMP("tm_year = %u tm_mon = %u tm_mday = %u tm_hour = %d tm_min = %d tm_sec = %d", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec)

      U_INTERNAL_ASSERT((tm->tm_mon  >= 0) && (tm->tm_mon  <= 12) &&
                        (tm->tm_mday >= 1) && (tm->tm_mday <= 31) &&
                        (tm->tm_hour >= 0) && (tm->tm_hour <= 23) &&
                        (tm->tm_min  >= 0) && (tm->tm_min  <= 59) &&
                        (tm->tm_sec  >= 0) && (tm->tm_sec  <= 60))

      t = getSecondFromJulian(toJulian(tm->tm_mday, tm->tm_mon, tm->tm_year));

#  if SIZEOF_TIME_T == 8
      if (t < 0L) t  = LONG_MAX;
#  else
      if (t < 0L) t  =  INT_MAX;
#  endif
      else        t += tm->tm_sec + (tm->tm_min * 60) + (tm->tm_hour * U_ONE_HOUR_IN_SECOND);
      }
   else
      {
   // (void) U_SYSCALL(memset, "%p,%d,%u", tm, 0, sizeof(struct tm));

      // NB: format must be compatible with the sequence "YY MM DD HH MM SS"...

      if (U_SYSCALL(sscanf, "%S,%S,%p,%p,%p,%p,%p,%p", str, fmt, &tm->tm_year, &tm->tm_mon, &tm->tm_mday, &tm->tm_hour, &tm->tm_min, &tm->tm_sec) != 6) U_RETURN(-1);

      tm->tm_mon  -=  1; // tm relative format month - range from 0-11
   // tm->tm_isdst = -1;

      // ts.tm_year is number of years since 1900

           if (tm->tm_year <   50) { tm->tm_year +=  100; }
      else if (tm->tm_year > 1900) { tm->tm_year -= 1900; }

      t = U_SYSCALL(mktime, "%p", tm); // NB: The timelocal() function is equivalent to the POSIX standard function mktime(3)
      }

   U_RETURN(t);
}

void UTimeDate::addMonths(int nmonths)
{
   U_TRACE(0, "UTimeDate::addMonths(%d)", nmonths)

   julian = 0;

   while (nmonths != 0)
      {
      if (nmonths      <  0 &&
          nmonths + 12 <= 0)
         {
         --_year;

         nmonths += 12;
         }
      else if (nmonths < 0)
         {
         _month += nmonths;
                   nmonths = 0;

         if (_month <= 0)
            {
            --_year;

            _month += 12;
            }
         }
      else if (nmonths - 12 >= 0)
         {
         ++_year;

         nmonths -= 12;
         }
      else if (_month == 12)
         {
         ++_year;

         _month = 0;
         }
      else
         {
         _month += nmonths;

         nmonths = 0;

         if (_month > 12)
            {
            ++_year;

            _month -= 12;
            }
         }
      }

   int days_in_month = getDaysInMonth();

   if (_day > days_in_month) _day = days_in_month;

   U_INTERNAL_DUMP("_day = %d, _month = %d, _year = %d", _day, _month, _year)
}

// This can be used for comments and other form of communication to tell the time ago instead of the exact time which might not be correct to someone in another time zone

UString UTimeDate::_ago(time_t sec, uint32_t granularity)
{
   U_TRACE(0, "UTimeDate::_ago(%ld,%u)", sec, granularity)

   int j = 7;
   UString result(100U);
   uint32_t no, diff, difference = u_now->tv_sec - sec;

   U_INTERNAL_ASSERT(u_now->tv_sec >= sec)

   while (true)
      {
      if ((no = (difference / lengths[j])) >= 1)
         {
         U_INTERNAL_DUMP("j = %u no = %u", j, no)

         break;
         }

      if (--j < 0)
         {
         j = 0;

         break;
         }
      }

   result.snprintf(U_CONSTANT_TO_PARAM("%lu %s%s"), no, periods[j], no != 1 ? "s " : " ");

   if (granularity > 0 &&
       j >= 1          &&
       (u_now->tv_sec - (diff = (u_now->tv_sec - (difference % lengths[j])))) > 0)
      {
      (void) result.append(_ago(diff, --granularity));

      U_RETURN_STRING(result);
      }

   (void) result.append(U_CONSTANT_TO_PARAM("ago"));

   U_RETURN_STRING(result);
}

// STREAM

#ifdef U_STDCPP_ENABLE
U_EXPORT istream& operator>>(istream& is, UTimeDate& d)
{
   U_TRACE(0, "UTimeDate::operator>>(%p,%p)", &is, &d)

   streambuf* sb = is.rdbuf();

   is >> d._day;

   (void) sb->sbumpc(); // skip '/'

   is >> d._month;

   (void) sb->sbumpc(); // skip '/'

   is >> d._year;

   d.julian = 0;

   return is;
}

U_EXPORT ostream& operator<<(ostream& os, const UTimeDate& d)
{
   U_TRACE(0, "UTimeDate::operator<<(%p,%p)", &os, &d)

   os << d._day   << '/'
      << d._month << '/'
      << d._year;

   return os;
}

// DEBUG

#  ifdef DEBUG
const char* UTimeDate::dump(bool reset) const
{
   *UObjectIO::os << "_day    " << _day   << '\n'
                  << "_month  " << _month << '\n'
                  << "_year   " << _year  << '\n'
                  << "julian  " << julian;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#  endif
#endif
