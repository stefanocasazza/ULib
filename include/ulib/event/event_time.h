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

   struct timeval ctime;

            UEventTime(long sec = 0L, long micro_sec = 1L);
   virtual ~UEventTime();

   bool operator<(const UEventTime& t) const __pure;

   // SERVICES

   bool isExpired() const __pure
      {
      U_TRACE_NO_PARAM(0, "UEventTime::isExpired()")

      U_CHECK_MEMORY

      long diff1 = ctime.tv_sec  + tv_sec  - timeout1.tv_sec,
           diff2 = ctime.tv_usec + tv_usec - timeout1.tv_usec;

      U_INTERNAL_DUMP("this = { %ld %6ld }, diff1 = %ld diff2 = %ld", ctime.tv_sec  + tv_sec,
                                                                      ctime.tv_usec + tv_usec, diff1, diff2)

      if ( diff1 <  0 ||
          (diff1 == 0 &&
           diff2 <= 0))
         {
         U_RETURN(true);
         }

      ms = (diff1 * 1000L) +
           (diff2 / 1000L);

      U_ASSERT(checkMilliSecond())

      U_RETURN(false);
      }

   bool isExpiredWithTolerance() const __pure
      {
      U_TRACE_NO_PARAM(0, "UEventTime::isExpiredWithTolerance()")

      if (isExpired() == false)
         {
         U_ASSERT(checkTolerance())

         if (ms > tolerance) U_RETURN(false);
         }

      U_RETURN(true);
      }

   time_t expire() const { return (ctime.tv_sec + tv_sec); }

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

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   long tolerance;

   static long ms;
   static struct timeval  timeout1;
   static struct timespec timeout2;

   void setCurrentTime()
      {
      U_TRACE_NO_PARAM(1, "UEventTime::setCurrentTime()")

      U_CHECK_MEMORY

      (void) U_SYSCALL(gettimeofday, "%p,%p", &ctime, 0);

      U_INTERNAL_DUMP("ctime = { %ld %6ld }", ctime.tv_sec, ctime.tv_usec)
      }

   void setTime(long timeoutMS)
      {
      U_TRACE(0, "UEventTime::setTime(%ld)", timeoutMS)

      setCurrentTime();

      UTimeVal::setMilliSecond(ms = timeoutMS);
      }

   bool checkMilliSecond() const 
      {
      U_TRACE_NO_PARAM(0, "UEventTime::checkMilliSecond()")

      long ms_calculated = ((ctime.tv_sec  + tv_sec  - timeout1.tv_sec)  * 1000L) +
                           ((ctime.tv_usec + tv_usec - timeout1.tv_usec) / 1000L);

      if ((ms - ms_calculated) <= 1) U_RETURN(true);

      U_DEBUG("ms = %ld ms_calculated = %ld", ms, ms_calculated);

      U_RETURN(false);
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

      U_ASSERT(checkMilliSecond())

      long tolerance_calculated = ((tv_sec  * 1000L) +
                                   (tv_usec / 1000L)) / 128;

      if ((tolerance - tolerance_calculated) <= 1) U_RETURN(true);

      U_DEBUG("tolerance = %ld tolerance_calculated = %ld", tolerance, tolerance_calculated);

      U_RETURN(false);
      }

   static long getMilliSecond(UEventTime* ptimeout)
      {
      U_TRACE(0, "UEventTime::getMilliSecond(%p)", ptimeout)

      if (ptimeout == 0) U_RETURN(-1);

      U_ASSERT(ptimeout->checkMilliSecond())

      U_RETURN(ms);
      }

   static struct timeval* getTimeVal(UEventTime* ptimeout)
      {
      U_TRACE(0, "UEventTime::getTimeVal(%p)", ptimeout)

      if (ptimeout == 0) U_RETURN_POINTER(0, struct timeval);

      U_ASSERT(ptimeout->checkMilliSecond())

      /**
       * struct timeval {
       *    long tv_sec;  //      seconds
       *    long tv_usec; // microseconds
       * };
       */

      long us = ms * 1000L;

      timeout1.tv_sec  = us / 1000000L;
      timeout1.tv_usec = us % 1000000L;

      U_INTERNAL_DUMP("timeout1 = { %ld %9ld }", timeout1.tv_sec, timeout1.tv_usec)

      U_RETURN_POINTER(&timeout1, struct timeval);
      }

   static struct timespec* getTimeSpec(UEventTime* ptimeout)
      {
      U_TRACE(0, "UEventTime::getTimeSpec(%p)", ptimeout)

      if (ptimeout == 0) U_RETURN_POINTER(0, struct timespec);

      U_ASSERT(ptimeout->checkMilliSecond())

      /**
       * struct timespec {
       *    time_t tv_sec;  //     seconds
       *    long   tv_nsec; // nanoseconds
       * };
       */

      long ns = ms * 1000000L;

      timeout2.tv_sec  = ns / 1000000000L;
      timeout2.tv_nsec = ns % 1000000000L;

      U_INTERNAL_DUMP("timeout2 = { %ld %9ld }", timeout2.tv_sec, timeout2.tv_nsec)

      U_RETURN_POINTER(&timeout2, struct timespec);
      }

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UEventTime& operator=(const UEventTime&) = delete;
#else
   UEventTime& operator=(const UEventTime&) { return *this; }
#endif

   friend class UTimer;
   friend class UNotifier;
};

#endif
