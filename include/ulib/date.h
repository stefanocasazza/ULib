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

#define FIRST_DAY   2361222 // Julian day for 1752/09/14
#define FIRST_YEAR  1752    // wrong for many countries

class U_EXPORT UTimeDate {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // COSTRUTTORI

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

   void setYear(int year);

   void set(int day, int month, int year)
      {
      U_TRACE(0, "UTimeDate::set(%d,%d,%d)", day, month, year)

      U_INTERNAL_ASSERT_RANGE(1,   day, 31)
      U_INTERNAL_ASSERT_RANGE(1, month, 12)

      julian = 0;
      _day   = day;
      _month = month;

      setYear(year);
      }

   // Construct a UTimeDate with a given day of the year and a given year.
   // The base date for this computation is 31 Dec. of the previous year.
   // i.e., UTimeDate(-1,1901) = 31 Dec. 1900 and UTimeDate(1,1901) = 2 Jan. 1901

   UTimeDate(int day,            int year);
   UTimeDate(int day, int month, int year);

   ~UTimeDate()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTimeDate)
      }

   // ASSEGNAZIONI

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

   int getJulian();
   int getDay() const   { return _day; }
   int getYear() const  { return _year; }
   int getMonth() const { return _month; }

   bool isValid() const __pure;

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

      U_RETURN(date.getJulian() - getJulian());
      }

   void addDays(int days)
      {
      U_TRACE(0, "UTimeDate::addDays(%d)", days)

      julian = getJulian() + days;

      fromJulian(julian);
      }

   void addMonths(int months);

   void addYears(int years)
      {
      U_TRACE(0, "UTimeDate::addYears(%d)", years)

      _year += years;

      julian = toJulian(_day, _month, _year);
      }

   int getDayOfWeek()
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::getDayOfWeek()")

      // (julian + 1) % 7 gives the value 0 for Sunday ... 6 for Saturday

      U_RETURN((getJulian() + 1) % 7);
      }

   // Returns the number of days in the month (28..31) for this date

   int getDaysInMonth() const __pure;

   // Returns the day of the year [1,366] for this date

   int getDayOfYear() __pure;

   // Returns true if the specified year is a leap year; otherwise returns false

   static bool leapYear(int y);

   // print date with format

          UString strftime(const char* fmt);
   static UString strftime(const char* fmt, time_t t, bool blocale = false);

   time_t getSecond()
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::getSecond()")

      time_t t = getSecondFromJulian(getJulian());

      U_RETURN(t);
      }

   static time_t getSecondFromJulian(int _julian);

   static time_t getSecondFromTime(const char* str) // seconds from string in HH:MM:SS format
      {
      U_TRACE(0, "UTimeDate::getSecondFromTime(%.*S)", 8, str)

      U_INTERNAL_ASSERT_EQUALS(str[2], ':')
      U_INTERNAL_ASSERT_EQUALS(str[5], ':')

      // NB: sscanf() is VERY HEAVY...!!!

      time_t t = (60 * 60 * atoi(str)) + (60 * atoi(str+3)) + atoi(str+6);

      U_RETURN(t);
      }

   static time_t getSecondFromTime(const char* str, bool gmt, const char* fmt = "%a, %d %b %Y %T GMT", struct tm* tm = 0);

   void setCurrentDate() // UNIX system time - SecsSince1Jan1970UTC
      {
      U_TRACE_NO_PARAM(0, "UTimeDate::setCurrentDate()")

      U_gettimeofday; // NB: optimization if it is enough a time resolution of one second...

      fromTime(u_now->tv_sec);
      }

   static void setCurrentDate(UTimeDate& date)
      {
      U_TRACE(0, "UTimeDate::setCurrentDate(%p)", &date)

      date.setCurrentDate();
      }

   // OPERATOR

   bool operator==(UTimeDate& date);
   bool operator!=(UTimeDate& date);
   bool operator< (UTimeDate& date);
   bool operator<=(UTimeDate& date);
   bool operator> (UTimeDate& date);
   bool operator>=(UTimeDate& date);

   UTimeDate& operator+=(UTimeDate& d)
      {
      U_TRACE(0, "UTimeDate::operator+=(%p)", &d)

      julian = getJulian() + d.getJulian();

      fromJulian(julian);

      return *this;
      }

   UTimeDate& operator-=(UTimeDate& d)
      {
      U_TRACE(0, "UTimeDate::operator-=(%p)", &d)

      julian = getJulian() - d.getJulian();

      fromJulian(julian);

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

#  ifdef DEBUG
   bool invariant() { return (julian >= FIRST_DAY); } // 14/09/1752

   const char* dump(bool reset) const;
#  endif
#endif

protected:
   int julian, _day, _month, _year;

   void fromJulian(int julian);

   static int toJulian(int day, int month, int year);
};
#endif
