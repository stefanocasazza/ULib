// test_timer.cpp

#include <ulib/timer.h>

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

#  if defined(U_STDCPP_ENABLE)
      cout.write(buffer, u__snprintf(buffer, sizeof(buffer), "MyAlarm1::handlerTime() u_now = %1D expire = %#1D\n", UEventTime::expire()));
#  endif

      U_RETURN(-1);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
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

#  if defined(U_STDCPP_ENABLE)
      cout.write(buffer, u__snprintf(buffer, sizeof(buffer), "MyAlarm2::handlerTime() u_now = %1D expire = %#1D\n", UEventTime::expire()));
#  endif

      U_RETURN(0);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return MyAlarm1::dump(_reset); }
#endif
};

int U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)",argc)

   UTimer::init(UTimer::SYNC);

   UTimeVal s(0L, 50L * 1000L);

   MyAlarm1* a;
   MyAlarm2* b;

   U_NEW(MyAlarm1, a, MyAlarm1(0L, 50L * 1000L));
   U_NEW(MyAlarm2, b, MyAlarm2(0L, 50L * 1000L));

   UTimer::insert(a);
   UTimer::insert(b);

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   if (argc > 2) UTimer::printInfo(cout);
#endif

   UTimer::erase(a);

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   if (argc > 2) UTimer::printInfo(cout);
#endif

   int n = (argc > 1 ? atoi(argv[1]) : 5);

   for (int i = 0; i < n; ++i)
      {
      s.nanosleep();

      U_NEW(MyAlarm1, a, MyAlarm1(0L, 50L * 1000L));

      UTimer::insert(a);

      UTimer::setTimer();

#  if defined(U_STDCPP_ENABLE) && defined(DEBUG)
      if (argc > 2) UTimer::printInfo(cout);
#  endif
      }

   s.nanosleep();
   s.nanosleep();

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   if (argc > 2) UTimer::printInfo(cout);
#endif

   UTimer::clear();

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   if (argc > 2) UTimer::printInfo(cout);
#endif
}
