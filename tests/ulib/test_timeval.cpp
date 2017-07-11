// test_timeval.cpp

#include <ulib/timeval.h>
#include <ulib/utility/interrupt.h>

#include <sys/uio.h>

static struct itimerval timeval = { { 0, 200000 }, { 0, 200000 } };

static RETSIGTYPE manage_alarm(int signo)
{
   U_TRACE(4, "[SIGALRM] manage_alarm(%d)", signo)

   (void) U_SYSCALL(setitimer,"%d,%p,%p",ITIMER_REAL,&timeval,0);
}

static void set_alarm()
{
   U_TRACE(4, "set_alarm()")

   UInterrupt::setHandlerForSignal(SIGALRM, (sighandler_t)&manage_alarm);

   (void) U_SYSCALL(setitimer,"%d,%p,%p",ITIMER_REAL,&timeval,0);
}

static inline void print_time(char* cp, int val)
{
   cp[0] = '0' + (val >= 10 ? (val / 10) : 0);
   cp[1] = '0' + (val  % 10);
}

/*
static void check_time(long tv_sec)
{
   U_TRACE(4, "check_time(%ld)", tv_sec)

   static char buf[75];
   static long last_sec;
   static char* start[3] = { buf, buf+17+1, buf+17+1+26+1 };
   static struct iovec iov[2] = { { (caddr_t)buf, sizeof(buf) },
                                  { (caddr_t)"\n",          1 } };

   if (last_sec != tv_sec)
      {
      last_sec = tv_sec;

      bool bchangehour = ((tv_sec % U_ONE_HOUR_IN_SECOND) == 0);

      if (bchangehour)
         {
         uint32_t lnow = u_get_localtime(last_sec);

         (void) u_strftime2(start[0], 17, "%d/%m/%y %H:%M:%S",         lnow);
         (void) u_strftime2(start[1], 26, "%d/%b/%Y:%H:%M:%S %z",      lnow);
         (void) u_strftime2(start[2], 29, "%a, %d %b %Y %H:%M:%S GMT", last_sec);
         }
      else
         {
         int seconds =  tv_sec % 60,
             minutes = (tv_sec / 60) % 60;

         print_time(start[0]+12, minutes);
         print_time(start[0]+15, seconds);
         print_time(start[1]+15, minutes);
         print_time(start[1]+18, seconds);
         print_time(start[2]+20, minutes);
         print_time(start[2]+23, seconds);
         }

      (void) writev(1, iov, 2);
      }
}
*/

int U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UTimeVal x;

   x.set(0L);
   U_ASSERT(x.isZero());

   UTimeVal a(1L);
   UTimeVal y(a);

   a.add(8L, 1999999L);
   a.sub(8L, 1999999L);

   U_ASSERT(y == a);
   U_ASSERT(y == UTimeVal(1L));
   U_ASSERT(y <= UTimeVal(1L));
   U_ASSERT(y >= UTimeVal(1L));
   U_ASSERT(y >  UTimeVal(0L, 999999));
   U_ASSERT(y <  UTimeVal(1L, 2L));

   y = a;
   U_ASSERT(y == a);

   UTimeVal tv1;
   UTimeVal tv2(2);
   UTimeVal tv3(100);
   UTimeVal tv4(1, 100000);
   UTimeVal tv5(2);
   UTimeVal tv6(1, -100000);

   tv1.set(0L);

   U_ASSERT(tv1 == x);
   U_ASSERT(tv2 < tv3);
   U_ASSERT(tv2 <= tv2);
   U_ASSERT(tv2 >= tv4);
   U_ASSERT(tv5 >= tv6);
   U_ASSERT(tv5 != tv4);
   U_ASSERT(tv2 != tv4);
   U_ASSERT(tv1 != tv2);
   U_ASSERT(tv6 != tv1);

   U_gettimeofday

   U_ASSERT(y < *u_now);

   set_alarm();

   y.nanosleep();

   y += *u_now;

   U_ASSERT(y > *u_now);

   /*
   long start = (u_now->tv_sec - (u_now->tv_sec % U_ONE_HOUR_IN_SECOND));

   for (uint32_t i = 0; i <= U_ONE_DAY_IN_SECOND; ++i) check_time(start+i);
   */
}
