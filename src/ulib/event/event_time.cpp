// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//   event_time.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/event/event_time.h>

#ifdef USE_LIBEVENT
#  include <ulib/libevent/event.h>
#endif

long            UEventTime::diff1;
long            UEventTime::diff2;
struct timeval  UEventTime::timeout1;
struct timespec UEventTime::timeout2;

UEventTime::UEventTime(long sec, long micro_sec) : UTimeVal(sec, micro_sec)
{
   U_TRACE_REGISTER_OBJECT(0, UEventTime, "%ld,%ld", sec, micro_sec)

   setTolerance();

   xtime.tv_sec =
   xtime.tv_usec = 0L;

#ifdef USE_LIBEVENT
   if (u_ev_base == 0) u_ev_base = (struct event_base*) U_SYSCALL_NO_PARAM(event_base_new);

   U_INTERNAL_ASSERT_POINTER(u_ev_base)

   U_NEW(UTimerEv<UEventTime>, pevent, UTimerEv<UEventTime>(*this));

   (void) UDispatcher::add(*pevent, *(UTimeVal*)this);
#endif
}

UEventTime::~UEventTime()
{
   U_TRACE_UNREGISTER_OBJECT(0, UEventTime)

#ifdef USE_LIBEVENT
   UDispatcher::del(pevent);
             delete pevent;
#endif
}

#ifdef USE_LIBEVENT
void UEventTime::operator()(int fd, short event)
{
   U_TRACE(0, "UEventTime::operator()(%d,%hd)", fd, event)

   int result = handlerTime();

   // return value:
   // ---------------
   // -1 - normal
   //  0 - monitoring
   // ---------------

   if (result == 0) (void) UDispatcher::add(*pevent, *(UTimeVal*)this);
}
#endif

// STREAM

#ifdef U_STDCPP_ENABLE
U_EXPORT ostream& operator<<(ostream& os, const UEventTime& t)
{
   U_TRACE(0, "UEventTime::operator<<(%p,%p)", &os, &t)

   os.put('{');
   os.put(' ');
   os << t.xtime.tv_sec;
   os.put(' ');
   os.width(6);
   os << t.xtime.tv_usec;
   os.put(' ');
   os.put('{');
   os.put(' ');
   os << t.tv_sec;
   os.put(' ');
   os.width(6);
   os << t.tv_usec;
   os.put(' ');
   os.put('}');
   os.put(' ');
   os.put('}');

   return os;
}

// DEBUG

#  ifdef DEBUG
const char* UEventTime::dump(bool _reset) const
{
   UTimeVal::dump(false);

   *UObjectIO::os << '\n'
                  << "xtime   " << "{ " << xtime.tv_sec
                                << " "  << xtime.tv_usec
                                << " }";

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
