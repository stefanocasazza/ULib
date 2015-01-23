// test_timer.cpp

#include <ulib/timer.h>

#include <iostream>

static char buffer[4096];

class MyAlarm1 : public UEventTime {
public:

   // COSTRUTTORI

   MyAlarm1(long sec, long usec) : UEventTime(sec, usec)
      {
      U_TRACE_REGISTER_OBJECT(0, MyAlarm1, "%ld,%ld", sec, usec)
      }

   virtual ~MyAlarm1()
      {
      U_TRACE_UNREGISTER_OBJECT(0, MyAlarm1)
      }

   virtual int handlerTime()
      {
      U_TRACE(0+256, "MyAlarm1::handlerTime()")

      // return value:
      // ---------------
      // -1 - normal
      //  0 - monitoring
      // ---------------

      cout.write(buffer, u__snprintf(buffer, sizeof(buffer), "MyAlarm1::handlerTime() u_now = %1D expire = %#1D\n", UEventTime::expire()));

      U_RETURN(-1);
      }

#ifdef DEBUG
   const char* dump(bool _reset) const { return UEventTime::dump(_reset); }
#endif
};

class MyAlarm2 : public MyAlarm1 {
public:

   // COSTRUTTORI

   MyAlarm2(long sec, long usec) : MyAlarm1(sec, usec)
      {
      U_TRACE_REGISTER_OBJECT(0, MyAlarm2, "%ld,%ld", sec, usec)
      }

   virtual ~MyAlarm2()
      {
      U_TRACE_UNREGISTER_OBJECT(0, MyAlarm2)
      }

   virtual int handlerTime()
      {
      U_TRACE(0+256, "MyAlarm2::handlerTime()")

      // return value:
      // ---------------
      // -1 - normal
      //  0 - monitoring
      // ---------------

      cout.write(buffer, u__snprintf(buffer, sizeof(buffer), "MyAlarm2::handlerTime() u_now = %1D expire = %#1D\n", UEventTime::expire()));

      U_RETURN(0);
      }

#ifdef DEBUG
   const char* dump(bool _reset) const { return MyAlarm1::dump(_reset); }
#endif
};

int U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UTimer::init(false);

   UTimeVal s(0L, 50L * 1000L);
   MyAlarm1* a = U_NEW(MyAlarm1(0L, 50L * 1000L));
   MyAlarm2* b = U_NEW(MyAlarm2(0L, 50L * 1000L));

   UTimer::insert(a, false);
   UTimer::insert(b, false);

#ifdef DEBUG
   if (argc > 2) UTimer::printInfo(cout);
#endif

   UTimer::erase(a, true, false);

#ifdef DEBUG
   if (argc > 2) UTimer::printInfo(cout);
#endif

   int n = (argc > 1 ? atoi(argv[1]) : 5);

   for (int i = 0; i < n; ++i)
      {
      s.nanosleep();

      (void) UTimer::insert(U_NEW(MyAlarm1(0L, 50L * 1000L)));

#  ifdef DEBUG
      if (argc > 2) UTimer::printInfo(cout);
#  endif
      }

   s.nanosleep();
   s.nanosleep();

   UTimer::stop();

#ifdef DEBUG
   if (argc > 2) UTimer::printInfo(cout);
#endif
   UTimer::clear(false);
#ifdef DEBUG
   if (argc > 2) UTimer::printInfo(cout);
#endif
}
