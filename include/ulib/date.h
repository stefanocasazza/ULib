// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    date.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_DATE_H
#define ULIB_DATE_H

#include <ulib/string.h>

/**
 * The UTimeDate class is based on the Gregorian (modern western) calendar.
 * England adopted the Gregorian calendar on September 14th 1752, which is the
 * earliest date that is supported by UTimeDate. Using earlier dates will give
 * undefined results. Some countries adopted the Gregorian calendar later than
 * England, thus the week day of early dates might be incorrect for these countries
 * (but correct for England). The end of time is reached around 8000AD, by which
 * time we expect UTimeDate to be obsolete...
 */

#define FIRST_DAY  2361222 // Julian day for 1752/09/14
#define FIRST_YEAR 1752    // wrong for many countries

class U_EXPORT UTimeDate {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // default move assignment operator
   U_MOVE_ASSIGNMENT(UTimeDate)

   UTimeDate()
      {
      U_TRACE_REGISTER_OBJECT(0, UTimeDate, "", 0)

      julian = _day = _month = _year = 0;
      }

   void fromTime(time_t tm);

   UTimeDate(time_t tm)
      {
      U_TRACE_REGISTER_OBJECT(0, UTimeDate, "%#19D", tm)

      fromTime(tm);
      }

   void fromUTC(const char* str, const char* frm = "%02u%02u%02u");

   UTimeDate(const char* str, bool UTC = false) // UTC is flag for date and time in Coordinated Universal Time: format YYMMDDMMSSZ
      {
      U_TRACE_REGISTER_OBJECT(0, UTimeDate, "%S,%b", str, UTC)

      julian = _day = _month = _year = 0;

      fromUTC(str, UTC ? "%02u%02u%02u" : "%d/%d/%d");
      }

   // set the current object by year

   void setYear(int year)
      {
      U_TRACE(0, "UTimeDate::setYear(%d)", year)

      // [   0,  50) -> [2000,2050)
      // [  50, 150] -> [1950,2050)
      // [1752,8000] -> [1752,8000]

      _year = year;

      if (year < 150) _year += 1900;
      if (year <  50) _year +=  100;

      U_INTERNAL_ASSERT_RANGE(1752, _year, 8000)

      julian = toJulian(_day, _month, _year);
      }

   // set the current object by month

   void setMonth(int month)
      {
      U_TRACE(0, "UTimeDate::setMonth(%d)", month)

      U_INTERNAL_ASSERT_RANGE(1, month, 12)

      julian = toJulian(_day, (_month = month), _year);
      }

   // set the current object by day

   void setDay(int day)
      {
      U_TRACE(0, "UTimeDate::setDay(%d)", day)

      U_INTERNAL_ASSERT_RANGE(1, day, 31)

      julian = toJulian((_day = day), _month, _year);
      }

   void set(int day, int month, int year)
      {
      U_TRACE(0, "UTimeDate::set(%d,%d,%d)", day, month, year)

      U_INTERNAL_ASSERT_RANGE(1,   day, 31)
      U_INTERNAL_ASSERT_RANGE(1, month, 12)

      _day   = day;
      _month = month;

      setYear(year);
      }

   // Construct a UTimeDate with a given day of the year and a given year. The base date for this computation
   // is 31 Dec. of the previous year. i.e., UTimeDate(-1,1901) = 31 Dec. 1900 and UTimeDate(1,1901) = 2 Jan. 1901

   UTimeDate(int day, int year)
      {
      U_TRACE_REGISTER_OBJECT(0, UTimeDate, "%d,%d", day, year)

      fromJulian(julian = toJulian(1,1,year) - 1 + day);
      }

   UTimeDate(int day, int month, int year)
      {
      U_TRACE_REGISTER_OBJECT(0, UTimeDate, "%d,%d,%d", day, month, year)

      set(day, month, year);
      }

   ~UTimeDate()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTimeDate)
      }

   void set(const UTimeDate& d)
      {
      _day   = d._day;
      _month = d._month;
      _year  = d._year;
      julian = d.julian;
      }

   UTimeDate(const UTimeDate& d)
      {
      U_TRACE_REGISTER_OBJECT(0, UTimeDate, "%p", &d)

      U_MEMORY_TEST_COPY(d)

      set(d);
      }

   UTimeDate& operator=(const UTimeDate& d)
      {
      U_TRACE(0, "UTimeDate::operator=(%p)", &d)

      U_MEMORY_TEST_COPY(d)

      set(d);

      return *this;
      }

   // SERVICES

   int getJulian()
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::getJulian()")

      if (julian == 0) julian = toJulian(_day, _month, _year);

      U_RETURN(julian);
      }

   bool isValid() const __pure;

   int getDay() const   { return _day; }
   int getYear() const  { return _year; }
   int getMonth() const { return _month; }

   const char* getDayName()
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::getDayName()")

      U_INTERNAL_ASSERT_RANGE(1, _day, 31)

      return u_day_name[getDayOfWeek()];
      }

   UString getDayNameShort()
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::getDayNameShort()")

      UString result = UString(getDayName(), 3);

      U_RETURN_STRING(result);
      }

   const char* getMonthName() const
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::getMonthName()")

      U_INTERNAL_ASSERT_RANGE(1, _month, 12)

      return u_month_name[_month-1];
      }

   static void updateTime(char* ptr)
      {
      U_TRACE(0, "UTimeDate::updateTime(%.5S)", ptr)

      U_INTERNAL_ASSERT(u_now->tv_sec % U_ONE_HOUR_IN_SECOND)

      U_NUM2STR16(ptr, (u_now->tv_sec / 60) % 60);
      U_NUM2STR16(ptr+3,u_now->tv_sec       % 60);
      }

   // The daysTo() function returns the number of days between two date

   int daysTo(UTimeDate& date)
      {
      U_TRACE(0, "UTimeDate::daysTo(%p)", &date)

      int diff = (date.getJulian() - getJulian());

      U_RETURN(diff);
      }

   void addDays(int day)
      {
      U_TRACE(0, "UTimeDate::addDays(%d)", day)

      // NB: we need to consider the case (day < 0)...

      fromJulian(julian = getJulian() + day);
      }

   void addYears(int year)
      {
      U_TRACE(0, "UTimeDate::addYears(%d)", year)

      julian = toJulian(_day, _month, _year += year);
      }

   void addMonths(int month);

   // Returns the number of days in the month (28..31) for this date

   int getDaysInMonth() const __pure
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::getDaysInMonth()")

      if (_month == 2 &&
          leapYear(_year))
         {
         U_RETURN(29);
         }

      U_RETURN(monthDays[_month]);
      }

   // Returns the day of the year [1,366] for this date

   int getDayOfYear() __pure
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::getDayOfYear()")

      int      y = _year - 1901,
          result = getJulian() - (y * 365) - (y / 4) - 2415385; // 2415385 -> 31/12/1900

      U_RETURN(result);
      }

   // Returns true if the specified year is a leap year; otherwise returns false

   static bool leapYear(int year)
      {
      U_TRACE(0, "UTimeDate::leapYear(%d)", year)

      // 1. Every year that is divisible by four is a leap year
      // 2. of those years, if it can be divided by 100, it is NOT a leap year, unless
      // 3. the year is divisible by 400. Then it is a leap year

      if ( ((year %   4) == 0) &&
          (((year % 100) != 0) ||
           ((year % 400) == 0)))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   // Week management 

   int getWeekOfYear()
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::getWeekOfYear()")

      int week_number = ((getJulian() - toJulian(1, 1, _year)) / 7) + 1;

      U_RETURN(week_number);
      }

   uint32_t getDayOfWeek()
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::getDayOfWeek()")

      uint32_t day_of_week = (getJulian() + 1) % 7; // gives the value 0 for Sunday ... 6 for Saturday: [Sun,Sat]([0,6])

      U_RETURN(day_of_week);
      }

   void setMondayPrevWeek()
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::setMondayPrevWeek()")

      int day_of_week = getDayOfWeek();

      // 1 => -7
      // 2 => -1
      // 3 => -2
      // 4 => -3
      // 5 => -4
      // 6 => -5
      // 0 => -6

      addDays(day_of_week >= 2 ? -day_of_week+1 : -6-day_of_week);
      }

   void setMondayNextWeek()
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::setMondayNextWeek()")

      int day_of_week = getDayOfWeek();

      // 1 => 7
      // 2 => 6
      // 3 => 5
      // 4 => 4
      // 5 => 3
      // 6 => 2
      // 0 => 1

      addDays(day_of_week ? 8-day_of_week : 1);
      }

   void setDayOfWeek(uint32_t day_of_week)
      {
      U_TRACE(0, "UTimeDate::setDayOfWeek(%u)", day_of_week)

      U_INTERNAL_ASSERT(day_of_week <= 6)

      uint32_t diff = weekday_difference(day_of_week, getDayOfWeek());

      if (diff) addDays(diff);

      U_ASSERT_EQUALS(getDayOfWeek(), day_of_week)
      }

   void setYearAndWeek(int year, int week)
      {
      U_TRACE(0, "UTimeDate::setYearAndWeek(%d,%d)", year, week)

      U_INTERNAL_ASSERT_RANGE(1, week, 53)

      set(1, 1, year);

      if (week > 1) addDays((week-1) * 7);
      }

   void setMondayPrevMonth(int month) // set monday of previous month
      {
      U_TRACE(0, "UTimeDate::setMondayPrevMonth(%d)", month)

      fromJulian(julian = toJulian(1, month, _year));

      int diff = 1 - getDayOfWeek();

      U_INTERNAL_DUMP("diff = %d", diff)

      if (diff) addDays(diff);

      U_ASSERT_EQUALS(getDayOfWeek(), 1)
      }

   // Print date with format

          UString strftime(const char* fmt, uint32_t fmt_size);
   static UString strftime(const char* fmt, uint32_t fmt_size, time_t t, bool blocale = false);

   time_t getSecond()
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::getSecond()")

      time_t t = getSecondFromJulian(getJulian());

      U_RETURN(t);
      }

   static time_t getSecondFromJulian(int _julian)
      {
      U_TRACE(0, "UTimeDate::getSecondFromJulian(%d)", _julian)

      if (_julian >= 2440588) // 2440588 -> 01/01/1970
         {
         time_t t = (_julian - 2440588) * U_ONE_DAY_IN_SECOND;

         U_RETURN(t);
         }

      U_RETURN(0);
      }

   static time_t getSecondFromTime(const char* str) // seconds from string in HH:MM:SS format
      {
      U_TRACE(0, "UTimeDate::getSecondFromTime(%.*S)", 8, str)

      U_INTERNAL_ASSERT_EQUALS(str[2], ':')
      U_INTERNAL_ASSERT_EQUALS(str[5], ':')

      // NB: sscanf() is VERY HEAVY...!!!

      time_t t = (60 * 60 * atoi(str)) + (60 * atoi(str+3)) + atoi(str+6);

      U_RETURN(t);
      }

   static time_t getSecondFromDayLight() __pure;

   static time_t getSecondFromTime(const char* str, bool gmt, const char* fmt = "%a, %d %b %Y %T GMT", struct tm* tm = 0);

   void setCurrentDate() // UNIX system time - SecsSince1Jan1970UTC
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::setCurrentDate()")

      U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

      fromTime(u_now->tv_sec);
      }

   static void setCurrentDate(UTimeDate& date)
      {
      U_TRACE(0, "UTimeDate::setCurrentDate(%p)", &date)

      date.setCurrentDate();
      }

   // This can be used for comments and other from of communication to tell the time ago
   // instead of the exact time which might not be correct to some one in another time zone

   UString ago(uint32_t granularity = 0)
      {
      U_TRACE(0, "UTimeDate::ago(%u)", granularity)

      return ago(getSecond(), granularity);
      }

   static UString ago(time_t tm, uint32_t granularity = 0)
      {
      U_TRACE(0, "UTimeDate::ago(%ld,%u)", tm, granularity)

      U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

      return _ago(tm, granularity);
      }

   // OPERATOR

   bool operator==(UTimeDate& date)
      {
      U_TRACE(0, "UTimeDate::operator==(%p,%p)", this, &date)

      if (getJulian() == date.getJulian()) U_RETURN(true);

      U_RETURN(false);
      }

   bool operator!=(UTimeDate& date);
   bool operator< (UTimeDate& date);
   bool operator<=(UTimeDate& date);
   bool operator> (UTimeDate& date);
   bool operator>=(UTimeDate& date);

   UTimeDate& operator++() { addDays( 1); return *this; }
   UTimeDate& operator--() { addDays(-1); return *this; }

   UTimeDate& operator+=(UTimeDate& d)
      {
      U_TRACE(0, "UTimeDate::operator+=(%p)", &d)

      fromJulian(getJulian() + d.getJulian());

      return *this;
      }

   UTimeDate& operator-=(UTimeDate& d)
      {
      U_TRACE(0, "UTimeDate::operator-=(%p)", &d)

      fromJulian(getJulian() - d.getJulian());

      return *this;
      }

   friend UTimeDate operator+(const UTimeDate& d1, UTimeDate& d2)
      {
      U_TRACE(0, "UTimeDate::operator+(%p,%p)", &d1, &d2)

      return UTimeDate(d1) += d2;
      }

   friend UTimeDate operator-(const UTimeDate& d1, UTimeDate& d2)
      {
      U_TRACE(0, "UTimeDate::operator+(%p,%p)", &d1, &d2)

      return UTimeDate(d1) -= d2;
      }

   UTimeDate& operator+=(int days)
      {
      U_TRACE(0, "UTimeDate::operator+=(%d)", days)

      addDays(days);

      return *this;
      }

   UTimeDate& operator-=(int days)
      {
      U_TRACE(0, "UTimeDate::operator-=(%d)", days)

      addDays(-days);

      return *this;
      }

   friend UTimeDate operator+(UTimeDate& d, int days)
      {
      U_TRACE(0, "UTimeDate::operator+(%p,%d)", &d, days)

      return UTimeDate(d) += days;
      }

   friend UTimeDate operator-(UTimeDate& d, int days)
      {
      U_TRACE(0, "UTimeDate::operator-(%p,%d)", &d, days)

      return UTimeDate(d) -= days;
      }
   
   // STREAM

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT istream& operator>>(istream& is,       UTimeDate& d); // dd/mm/yyyy
   friend U_EXPORT ostream& operator<<(ostream& os, const UTimeDate& d); // dd/mm/yyyy

# ifdef DEBUG
   bool invariant() { return (julian >= FIRST_DAY); } // 14/09/1752

   const char* dump(bool reset) const;
# endif
#endif

protected:
   int julian, _day, _month, _year;

   static const char* periods[8];
   static const uint32_t lengths[8];
   static const short monthDays[13];

          void fromJulian(int julian);
   static int    toJulian(int day, int month, int year);
   static UString _ago(time_t tm, uint32_t granularity);

   static uint32_t weekday_difference(uint32_t x, uint32_t y) // Returns the number of days from the weekday y to the weekday x
      {
      U_TRACE(0, "UTimeDate::weekday_difference(%u,%u)", x, y)

      U_INTERNAL_ASSERT(x <= 6)
      U_INTERNAL_ASSERT(y <= 6)

      x -= y;

      U_RETURN(x <= 6 ? x : x + 7); // The result is always in the range [0, 6]
      }

   static uint32_t next_weekday(uint32_t x)
      {
      U_TRACE(0, "UTimeDate::next_weekday(%u)", x)

      U_INTERNAL_ASSERT(x <= 6)

      U_RETURN(x < 6 ? x+1 : 0);
      }

   static uint32_t prev_weekday(uint32_t x)
      {
      U_TRACE(0, "UTimeDate::prev_weekday(%u)", x)

      U_INTERNAL_ASSERT(x <= 6)

      U_RETURN(x > 0 ? x-1 : 6);
      }
};
#endif
