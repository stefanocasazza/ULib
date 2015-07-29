// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    event_time.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_EVENT_TIME_H
#define ULIB_EVENT_TIME_H 1

#include <ulib/timeval.h>

#ifdef USE_LIBEVENT
template <class T> class UTimerEv;
#endif

class U_EXPORT UEventTime : public UTimeVal {
public:

   struct timeval ctime;

   void reset() { ctime.tv_sec = ctime.tv_usec = 0L; }

            UEventTime(long sec = 0L, long usec = 1L);
   virtual ~UEventTime();

   bool operator<(const UEventTime& t) const __pure;

   // SERVICES

   void setCurrentTime()
      {
      U_TRACE(1, "UEventTime::setCurrentTime()")

      U_CHECK_MEMORY

      (void) U_SYSCALL(gettimeofday, "%p,%p", &ctime, 0);

      *u_now = ctime;

      U_INTERNAL_DUMP("u_now = { %ld %6ld }", u_now->tv_sec, u_now->tv_usec)
      }

   bool isOld() const __pure
      {
      U_TRACE(0, "UEventTime::isOld()")

      U_CHECK_MEMORY

      long t1 = (ctime.tv_sec + tv_sec);

      U_INTERNAL_DUMP("this = { %ld %6ld }", t1, ctime.tv_usec + tv_usec)

      bool result = (  t1  < u_now->tv_sec) ||
                     ((t1 == u_now->tv_sec) &&
                      ((ctime.tv_usec + tv_usec) < u_now->tv_usec));

      U_RETURN(result);
      }

   bool isExpired() const __pure;

   void setTimerVal(struct timeval* it_value)
      {
      U_TRACE(0, "UEventTime::setTimerVal(%p)", it_value)

      U_CHECK_MEMORY

      it_value->tv_sec  = ctime.tv_sec  + tv_sec  - u_now->tv_sec;
      it_value->tv_usec = ctime.tv_usec + tv_usec - u_now->tv_usec;

      UTimeVal::adjust(&(it_value->tv_sec), &(it_value->tv_usec));

      U_INTERNAL_DUMP("it_value = { %ld %6ld }", it_value->tv_sec, it_value->tv_usec)

      U_INTERNAL_ASSERT(it_value->tv_sec  >= 0)
      U_INTERNAL_ASSERT(it_value->tv_usec >= 0)
      }

   time_t expire() { return (ctime.tv_sec + tv_sec); }

   // -------------------------------------------
   // method VIRTUAL to define
   // -------------------------------------------
   // return value: -1 -> normal, 0 -> monitoring
   // -------------------------------------------

   virtual int handlerTime() { return -1; }

#ifdef USE_LIBEVENT
   UTimerEv<UEventTime>* pevent;

   void operator()(int fd, short event);
#endif

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT ostream& operator<<(ostream& os, const UEventTime& t);

#  ifdef DEBUG
   const char* dump(bool reset) const;
#  endif
#endif

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UEventTime& operator=(const UEventTime&) = delete;
#else
   UEventTime& operator=(const UEventTime&) { return *this; }
#endif
};

#endif
