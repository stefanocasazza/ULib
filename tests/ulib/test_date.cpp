// test_date.cpp

#include <ulib/log.h>
#include <ulib/date.h>

static void checkForDaylightSavingTime(const char* tz, const char* start, const char* end)
{
   U_TRACE(5, "::checkForDaylightSavingTime(%S,%S,%S)", tz, start, end)

   (void) U_SYSCALL(putenv, "%S", (char*)tz);

   if (u_setStartTime() == false)
      {
      U_WARNING("System date not updated: %#5D", u_now->tv_sec);

      return;
      }

   int res1_exp = (u_is_daylight() ? 0 : 1);

   time_t dst_start = UTimeDate::getSecondFromDate(start),
          dst_end   = UTimeDate::getSecondFromDate(end);

   int res1 = UTimeDate::checkForDaylightSavingTime(dst_start),
       res2 = UTimeDate::checkForDaylightSavingTime(dst_end);

   U_INTERNAL_DUMP("res1 = %d res1_exp = %d", res1, res1_exp)

   U_INTERNAL_ASSERT_EQUALS(res1, res1_exp)
   U_INTERNAL_ASSERT_EQUALS(res2, -1)
}

int U_EXPORT main(int argc, char* argv[], char** env)
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   /*
   */
   UTimeDate date0(28,12,14);

   cout << date0.strftime(U_CONSTANT_TO_PARAM("%d/%m/%Y")) << ' '
        << date0.ago(7U) << '\n';

   u_now->tv_sec = time(U_NULLPTR);

   cout << UTimeDate::ago(u_now->tv_sec -  1) << ' '
        << UTimeDate::ago(u_now->tv_sec -  1, 7U) << '\n';
   cout << UTimeDate::ago(u_now->tv_sec - 10) << ' '
        << UTimeDate::ago(u_now->tv_sec - 10, 7U) << '\n';
   cout << UTimeDate::ago(u_now->tv_sec - 60) << ' '
        << UTimeDate::ago(u_now->tv_sec - 60, 7U) << '\n';
   cout << UTimeDate::ago(u_now->tv_sec - 10 * 60) << ' '
        << UTimeDate::ago(u_now->tv_sec - 10 * 60, 7U) << '\n';
   cout << UTimeDate::ago(u_now->tv_sec - U_ONE_HOUR_IN_SECOND) << ' '
        << UTimeDate::ago(u_now->tv_sec - U_ONE_HOUR_IN_SECOND, 7U) << '\n';
   cout << UTimeDate::ago(u_now->tv_sec - U_ONE_DAY_IN_SECOND) << ' '
        << UTimeDate::ago(u_now->tv_sec - U_ONE_DAY_IN_SECOND, 7U) << '\n';
   cout << UTimeDate::ago(u_now->tv_sec - U_ONE_WEEK_IN_SECOND) << ' '
        << UTimeDate::ago(u_now->tv_sec - U_ONE_WEEK_IN_SECOND, 7U) << '\n';
   cout << UTimeDate::ago(u_now->tv_sec - U_ONE_MONTH_IN_SECOND) << ' '
        << UTimeDate::ago(u_now->tv_sec - U_ONE_MONTH_IN_SECOND, 7U) << '\n';
   cout << UTimeDate::ago(u_now->tv_sec - U_ONE_YEAR_IN_SECOND) << ' '
        << UTimeDate::ago(u_now->tv_sec - U_ONE_YEAR_IN_SECOND, 7U) << '\n';

   UTimeDate data1(31,12,99), data2("31/12/99");

   U_ASSERT( UTimeDate("14/09/1752").getJulian() == 2361222 )
   U_ASSERT( UTimeDate("31/12/1900").getJulian() == 2415385 )
   U_ASSERT( UTimeDate("01/01/1970").getJulian() == 2440588 )

   U_ASSERT( data1 == data2 )
   U_ASSERT( data1.getDayOfWeek() == 5 ) // Venerdi
   U_ASSERT( data2.getDayOfYear() == 365 )

   U_ASSERT( UTimeDate("1/3/00").getDayOfWeek() == 3 ) // Mercoledi
   U_ASSERT( UTimeDate(31,12,0).getDayOfYear() == 366 )

   UTimeDate data3(60,2000);
   UTimeDate data4("29/02/00");

   U_ASSERT( data3 == data4 )
   U_ASSERT( data3.getDayOfYear() == 60 )

   UTimeDate data5(60,1901);
   UTimeDate data6("1/3/1901");

   U_ASSERT( data5 == data6 )

   U_ASSERT( UTimeDate(17, 5, 2002).isValid() == true )  // TRUE   May 17th 2002 is valid
   U_ASSERT( UTimeDate(30, 2, 2002).isValid() == false ) // FALSE  Feb 30th does not exist
   U_ASSERT( UTimeDate(29, 2, 2004).isValid() == true )  // TRUE   2004 is a leap year

   UTimeDate data7(29, 2, 2004);

   UString x = data7.strftime(U_CONSTANT_TO_PARAM("%Y-%m-%d"));

   U_ASSERT( x == U_STRING_FROM_CONSTANT("2004-02-29") )

   U_ASSERT( UTimeDate("14/09/1752").getJulian() == 2361222 )

   cout << "Date: " << data6.strftime(U_CONSTANT_TO_PARAM("%d/%m/%y")) << '\n';

   while (cin >> data6) cout << data6 << '\n';

   time_t t = UTimeDate::getSecondFromTime("01:01:00");

   U_INTERNAL_ASSERT_EQUALS( t, 3660 )

   t = UTimeDate::getSecondFromTime("01:01 A");

   U_INTERNAL_ASSERT_EQUALS( t, 3660 )

   t = UTimeDate::getSecondFromDate("Fri, 31 Dec 1999 23:59:59 GMT");

   U_INTERNAL_ASSERT_MINOR( t, u_now->tv_sec )

   t = UTimeDate::getSecondFromDate("Jan 25 11:54:00 2005 GMT");

   U_INTERNAL_ASSERT_MINOR( t, u_now->tv_sec )

   t = UTimeDate::getSecondFromDate("100212124550Z");

   U_INTERNAL_ASSERT_MINOR( t, u_now->tv_sec )

   t = UTimeDate::getSecondFromDate("20030314104248Z", false, U_NULLPTR, "%4u%2u%2u%2u%2u%2uZ");

   U_INTERNAL_ASSERT_MINOR( t, u_now->tv_sec )

// typedef struct log_date {
//    char date1[17+1];             // 18/06/12 18:45:56
//    char date2[26+1];             // 04/Jun/2012:18:18:37 +0200
//    char date3[6+29+2+12+2+19+1]; // Date: Wed, 20 Jun 2012 11:43:17 GMT\r\nServer: ULib\r\nConnection: close\r\n
// } log_date;

   ULog::log_date log_date;
   uint32_t lnow = u_getLocalTime();

   (void) u_strftime2(log_date.date1, 17,               U_CONSTANT_TO_PARAM("%d/%m/%y %T"),                                                        lnow);
   (void) u_strftime2(log_date.date2, 26,               U_CONSTANT_TO_PARAM("%d/%b/%Y:%T %z"),                                                     lnow);
   (void) u_strftime2(log_date.date3, 6+29+2+12+2+17+2, U_CONSTANT_TO_PARAM("Date: %a, %d %b %Y %T GMT\r\nServer: ULib\r\nConnection: close\r\n"), u_now->tv_sec);

   U_INTERNAL_DUMP("date1 = %.17S date2 = %.26S date3+6 = %.29S", log_date.date1, log_date.date2, log_date.date3+6)

   // (01/01/2015) Giovedì
   // (02/01/2015) Venerdì
   // (03/01/2015) Sabato

   for (int i = 0; i < 357; ++i)
      {
      data1.setYearAndWeek(2015, (i/7)+1);
      data1.setDayOfWeek(i%7);

      data3 = data2 = data1;

      data2.setMondayPrevWeek();
      data3.setMondayNextWeek();

      U_DUMP("setMondayPrevWeek() = %V data = %V setMondayNextWeek() = %V", data2.strftime(U_CONSTANT_TO_PARAM("%d/%m/%y")).rep,
                                                                            data1.strftime(U_CONSTANT_TO_PARAM("%d/%m/%y")).rep,
                                                                            data3.strftime(U_CONSTANT_TO_PARAM("%d/%m/%y")).rep)

      cout << data1.strftime(U_CONSTANT_TO_PARAM("%d/%m/%y")) << ' '
           << data1.ago()                << ' '
           << data1.ago(7U)              << '\n';
      }

   checkForDaylightSavingTime("TZ=CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/01:00:00", "Sun, 26 Mar 2017 01:00:00 GMT", "Sun, 29 Oct 2017 01:00:00 GMT");

   /**
    * In New Zealand, where the standard time (NZST) is 12 hours ahead of UTC, and daylight saving time (NZDT), 13 hours ahead of UTC, runs from the first Sunday in
    * October to the third Sunday in March, and the changeovers happen at the default time of 02:00:00.
    */

   checkForDaylightSavingTime("TZ=NZST-12:00:00NZDT-13:00:00,M10.1.0,M3.3.0", "Sun, 8 Oct 2017 02:00:00 GMT", "Sun, 18 Mar 2018 02:00:00 GMT");

   /**
    * Western Greenland Time (WGT) and Western Greenland Summer Time (WGST) are 3 hours behind UTC in the winter. Its clocks follow the European Union rules of springing
    * forward by one hour on March last Sunday at 01:00 UTC (-02:00 local time) and falling back on October last Sunday at 01:00 UTC (-01:00 local time).
    */

   checkForDaylightSavingTime("TZ=WGT3WGST,M3.5.0/-2,M10.5.0/-1", "Sun, 26 Mar 2017 01:00:00 GMT", "Sun, 29 Oct 2017 01:00:00 GMT");

   /**
    * In North American Eastern Standard Time (EST) and Eastern Daylight Time (EDT), the normal offset from UTC is 5 hours; since this is west of the prime meridian,
    * the sign is positive. Summer time begins on March second Sunday at 2:00am, and ends on November first Sunday at 2:00am.

   checkForDaylightSavingTime("TZ=EST+5EDT,M3.2.0/2,M11.1.0/2", "Sun, 12 Mar 2017 02:00:00 GMT", "Sun, 5 Nov 2017 02:00:00 GMT");
    */

   /**
    * Israel Standard Time (IST) and Israel Daylight Time (IDT) are 2 hours ahead of the prime meridian in winter, springing forward an hour on March fourth Thursday
    * at 26:00 (i.e., 02:00 on the first Friday on or after March 23), and falling back on October last Sunday at 02:00.
    */

   checkForDaylightSavingTime("TZ=IST-2IDT,M3.4.4/26,M10.5.0", "Fri, 24 Mar 2017 02:00:00 GMT", "Sun, 29 Oct 2017 02:00:00 GMT");

   /**
    * Western Argentina Summer Time (WARST) is 3 hours behind the prime meridian all year. There is a dummy fall-back transition on December 31 at 25:00 daylight saving
    * time (i.e., 24:00 standard time, equivalent to January 1 at 00:00 standard time), and a simultaneous spring-forward transition on January 1 at 00:00 standard time,
    * so daylight saving time is in effect all year and the initial WART is a placeholder.

   checkForDaylightSavingTime("TZ=WART4WARST,J1/0,J365/25", "Sun, 1 Jan 2017 00:00:00 GMT", "Sun, 1 Jan 2017 00:00:00 GMT");
    */
}
