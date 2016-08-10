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

class UTimer;
class UNotifier;
#ifdef USE_LIBEVENT
template <class T> class UTimerEv;
#endif

class U_EXPORT UEventTime : public UTimeVal {
public:

            UEventTime(long sec = 0L, long micro_sec = 1L);
   virtual ~UEventTime();

   // SERVICES

   struct timeval xtime;

   time_t expire() const { return xtime.tv_sec; }

   bool isExpired() const __pure
      {
      U_TRACE_NO_PARAM(0, "UEventTime::isExpired()")

      U_CHECK_MEMORY

      diff1 = xtime.tv_sec  - timeout1.tv_sec,
      diff2 = xtime.tv_usec - timeout1.tv_usec;

      U_INTERNAL_DUMP("xtime = { %ld %6ld }, diff1 = %ld diff2 = %ld", xtime.tv_sec, xtime.tv_usec, diff1, diff2)

      if ( diff1 <  0 ||
          (diff1 == 0 &&
           diff2 <= 0))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool isExpiredWithTolerance() const __pure
      {
      U_TRACE_NO_PARAM(0, "UEventTime::isExpiredWithTolerance()")

      if (isExpired() == false)
         {
         U_ASSERT(checkTolerance())

         long ms = (diff1 * 1000L) +
                   (diff2 / 1000L);

         if (ms > tolerance) U_RETURN(false);
         }

      U_RETURN(true);
      }

   // ------------------------
   // method VIRTUAL to define
   // ------------------------

   virtual int handlerTime() { return -1; } // return value: -1 -> normal,
                                            //                0 -> monitoring

#ifdef USE_LIBEVENT
   UTimerEv<UEventTime>* pevent;

   void operator()(int fd, short event);
#endif

   bool operator<(const UEventTime& t) const __pure
      {
      U_TRACE(0, "UEventTime::operator<(%p)", &t)

      U_INTERNAL_DUMP("{ %ld %6ld } < { %ld %6ld }",   xtime.tv_sec,   xtime.tv_usec,
                                                     t.xtime.tv_sec, t.xtime.tv_usec)

      if (  xtime.tv_sec <  t.xtime.tv_sec  ||
          ((xtime.tv_sec == t.xtime.tv_sec) &&
          ((xtime.tv_usec < t.xtime.tv_usec))))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool operator==(const UEventTime& t) const __pure
      {
      U_TRACE(0, "UEventTime::operator==(%p)", &t)

      U_INTERNAL_DUMP("{ %ld %6ld } == { %ld %6ld }",   xtime.tv_sec,   xtime.tv_usec,
                                                      t.xtime.tv_sec, t.xtime.tv_usec)

      if (xtime.tv_sec  == t.xtime.tv_sec &&
          xtime.tv_usec == t.xtime.tv_usec)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT ostream& operator<<(ostream& os, const UEventTime& t);

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   long tolerance;

   static long diff1, diff2;
   static struct timeval  timeout1;
   static struct timespec timeout2;

   bool checkMilliSecond() const
      {
      U_TRACE_NO_PARAM(0, "UEventTime::checkMilliSecond()")

      long ms1 = (tv_sec  * 1000L) +
                 (tv_usec / 1000L),
           ms2 = ((xtime.tv_sec  - timeout1.tv_sec)  * 1000L) +
                 ((xtime.tv_usec - timeout1.tv_usec) / 1000L);

      if ((ms1 - ms2) <= 1) U_RETURN(true);

      U_DEBUG("ms1 = %ld ms2 = %ld", ms1, ms2)

      U_RETURN(false);
      }

   void setTimeToExpire()
      {
      U_TRACE_NO_PARAM(1, "UEventTime::setTimeToExpire()")

      U_CHECK_MEMORY

      u_gettimeofday(&xtime);

      U_INTERNAL_DUMP("now = { %ld %6ld } xtime = { %ld %6ld }", xtime.tv_sec, xtime.tv_usec, xtime.tv_sec + tv_sec, xtime.tv_usec + tv_usec)

      xtime.tv_sec  += tv_sec;
      xtime.tv_usec += tv_usec;
      }

   void setTimeToExpire(uint32_t timeout)
      {
      U_TRACE(0, "UEventTime::setTimeToExpire(%u)", timeout)

      UTimeVal::setSecond(timeout);

      setTolerance();

      u_gettimeofday(&timeout1);

      updateTimeToExpire();
      }

   void setTimeToExpireMS(int timeoutMS)
      {
      U_TRACE(0, "UEventTime::setTimeToExpireMS(%d)", timeoutMS)

      UTimeVal::setMilliSecond(timeoutMS);

      setTolerance();

      u_gettimeofday(&timeout1);

      updateTimeToExpire();
      }

   void setTolerance() 
      {
      U_TRACE_NO_PARAM(0, "UEventTime::setTolerance()")

      tolerance = ((tv_sec  * 1000L) +
                   (tv_usec / 1000L)) / 128;

      U_INTERNAL_DUMP("tolerance = %ld", tolerance)
      }

   bool checkTolerance() const 
      {
      U_TRACE_NO_PARAM(0, "UEventTime::checkTolerance()")

      long tolerance_calculated = ((tv_sec  * 1000L) +
                                   (tv_usec / 1000L)) / 128;

      if ((tolerance - tolerance_calculated) <= 1) U_RETURN(true);

      U_DEBUG("tolerance = %ld tolerance_calculated = %ld", tolerance, tolerance_calculated)

      U_RETURN(false);
      }

   void updateTimeToExpire()
      {
      U_TRACE_NO_PARAM(1, "UEventTime::updateTimeToExpire()")

      U_CHECK_MEMORY

      xtime.tv_sec  = timeout1.tv_sec  + tv_sec;
      xtime.tv_usec = timeout1.tv_usec + tv_usec;

      U_INTERNAL_DUMP("now = { %ld %6ld } xtime = { %ld %6ld }", xtime.tv_sec, xtime.tv_usec, xtime.tv_sec + tv_sec, xtime.tv_usec + tv_usec)

      U_ASSERT(checkTolerance())
      U_ASSERT(checkMilliSecond())
      }

   static long getMilliSecond(UEventTime* ptimeout)
      {
      U_TRACE(0, "UEventTime::getMilliSecond(%p)", ptimeout)

      if (ptimeout == 0) U_RETURN(-1);

      long ms = ((ptimeout->xtime.tv_sec  - timeout1.tv_sec)  * 1000L) +
                ((ptimeout->xtime.tv_usec - timeout1.tv_usec) / 1000L);

      U_RETURN(ms);
      }

   static struct timeval* getTimeVal(UEventTime* ptimeout)
      {
      U_TRACE(0, "UEventTime::getTimeVal(%p)", ptimeout)

      if (ptimeout == 0) U_RETURN_POINTER(0, struct timeval);

      /**
       * struct timeval {
       *    long tv_sec;  //      seconds
       *    long tv_usec; // microseconds
       * };
       */

      timeout1.tv_sec  = ptimeout->xtime.tv_sec  - timeout1.tv_sec;
      timeout1.tv_usec = ptimeout->xtime.tv_usec - timeout1.tv_usec;

      u_adjtime(&(timeout1.tv_sec), &(timeout1.tv_usec));

      U_INTERNAL_DUMP("timeout1 = { %ld %9ld }", timeout1.tv_sec, timeout1.tv_usec)

      U_INTERNAL_ASSERT(timeout1.tv_sec <= ptimeout->tv_sec)

      U_RETURN_POINTER(&timeout1, struct timeval);
      }

   void setTimeVal(struct timeval* timerval)
      {
      U_TRACE(0, "UEventTime::setTimeVal(%p)", timerval)

      timerval->tv_sec  = xtime.tv_sec  - timeout1.tv_sec;
      timerval->tv_usec = xtime.tv_usec - timeout1.tv_usec;

      u_adjtime(&(timerval->tv_sec), &(timerval->tv_usec));

      U_INTERNAL_DUMP("timerval = { %ld %9ld }", timerval->tv_sec, timerval->tv_usec)

      U_INTERNAL_ASSERT(timerval->tv_sec <= tv_sec)
      }

   static struct timespec* getTimeSpec(UEventTime* ptimeout)
      {
      U_TRACE(0, "UEventTime::getTimeSpec(%p)", ptimeout)

      if (ptimeout == 0) U_RETURN_POINTER(0, struct timespec);

      /**
       * struct timespec {
       *    time_t tv_sec;  //     seconds
       *    long   tv_nsec; // nanoseconds
       * };
       */

      timeout2.tv_sec  =  ptimeout->xtime.tv_sec  - timeout1.tv_sec;
      timeout2.tv_nsec = (ptimeout->xtime.tv_usec - timeout1.tv_usec) * 1000L;

      U_INTERNAL_DUMP("timeout2 = { %ld %9ld }", timeout2.tv_sec, timeout2.tv_nsec)

      U_RETURN_POINTER(&timeout2, struct timespec);
      }

private:
   U_DISALLOW_ASSIGN(UEventTime)

   friend class UTimer;
   friend class UNotifier;
};

#endif
