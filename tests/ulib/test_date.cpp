// test_date.cpp

#include <ulib/log.h>
#include <ulib/date.h>

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

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

   UString x = data7.strftime("%Y-%m-%d");

   U_ASSERT( x == U_STRING_FROM_CONSTANT("2004-02-29") )

   U_ASSERT( UTimeDate("14/09/1752").getJulian() == 2361222 )

   cout << "Date: " << data6.strftime("%d/%m/%y") << '\n';

   while (cin >> data6) cout << data6 << '\n';

   U_ASSERT( UTimeDate::getSecondFromTime("19030314104248Z", true, "%4u%2u%2u%2u%2u%2uZ") < u_now->tv_sec )

   /*
   typedef struct static_date {
      struct timeval _timeval;      // => u_now
      char lock1[1];
      char date1[17+1];             // 18/06/12 18:45:56
      char lock2[1];
      char date2[26+1];             // 04/Jun/2012:18:18:37 +0200
      char lock3[1];
      char date3[6+29+2+12+2+19+1]; // Date: Wed, 20 Jun 2012 11:43:17 GMT\r\nServer: ULib\r\nConnection: close\r\n
   } static_date;
   */

   ULog::static_date log_data;

   (void) u_strftime2(log_data.date1, 17,               "%d/%m/%y %T",    u_now->tv_sec + u_now_adjust);
   (void) u_strftime2(log_data.date2, 26,               "%d/%b/%Y:%T %z", u_now->tv_sec + u_now_adjust);
   (void) u_strftime2(log_data.date3, 6+29+2+12+2+17+2, "Date: %a, %d %b %Y %T GMT\r\nServer: ULib\r\nConnection: close\r\n", u_now->tv_sec);

   U_INTERNAL_DUMP("date1 = %.17S date2 = %.26S date3+6 = %.29S", log_data.date1, log_data.date2, log_data.date3+6)

   /*
   for (int i = 0; i < 360; ++i)
      {
      u_now->tv_sec++;

      UTimeDate::updateTime(log_data.date1   + 12);
      UTimeDate::updateTime(log_data.date2   + 15);
      UTimeDate::updateTime(log_data.date3+6 + 20);

      cout.write(log_data.date1, 17);
      cout.write(" - ", 3);
      cout.write(log_data.date2, 26);
      cout.write(" - ", 3);
      cout.write(log_data.date3+6, 29);
      cout.put('\n');
      }
   */
}
