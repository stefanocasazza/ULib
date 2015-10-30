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

   void setTime(long timeoutMS)
      {
      U_TRACE(0, "UEventTime::setTime(%ld)", timeoutMS)

      setCurrentTime();

      UTimeVal::setMilliSecond(timeoutMS);
      }

   void setCurrentTime()
      {
      U_TRACE_NO_PARAM(1, "UEventTime::setCurrentTime()")

      U_CHECK_MEMORY

      (void) U_SYSCALL(gettimeofday, "%p,%p", &ctime, 0);

      U_INTERNAL_DUMP("ctime = { %ld %6ld }", ctime.tv_sec, ctime.tv_usec)
      }

   time_t expire() const { return (ctime.tv_sec + tv_sec); }

   bool isExpired(long tolerance) const __pure
      {
      U_TRACE(0, "UEventTime::isExpired(%ld)", tolerance)

      U_CHECK_MEMORY

      long t1    = expire(),
           diff1 =                      t1 - u_now->tv_sec,
           diff2 = ctime.tv_usec + tv_usec - u_now->tv_usec;

      U_INTERNAL_DUMP("this = { %ld %6ld }, diff1 = %ld diff2 = %ld", t1, ctime.tv_usec + tv_usec, diff1, diff2)

      if ( diff1  < 0 ||
          (diff1 == 0 &&
           diff2 <= 0))
         {
         U_RETURN(true);
         }

      long diff = (diff1 * 1000L) +
                  (diff2 / 1000L);

      U_INTERNAL_DUMP("diff = %ld", diff)

      if (diff <= tolerance) U_RETURN(true);

      U_RETURN(false);
      }

   bool isOld() const __pure     { return isExpired(0); }
   bool isExpired() const __pure { return isExpired(UTimeVal::getTolerance()); }

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

   long getTimerVal()
      {
      U_TRACE_NO_PARAM(0, "UEventTime::getTimerVal()")

      U_CHECK_MEMORY

      long ms = ((ctime.tv_sec  + tv_sec  - u_now->tv_sec)  * 1000L) +
                ((ctime.tv_usec + tv_usec - u_now->tv_usec) / 1000L);

      U_RETURN(ms);
      }

   struct timespec* getTimerValSpec()
      {
      U_TRACE_NO_PARAM(0, "UEventTime::getTimerValSpec()")

      U_CHECK_MEMORY

      /**
       * struct timespec {
       *    time_t tv_sec;  // seconds
       *    long   tv_nsec; // nanoseconds
       * };
       */

      long ns = ((ctime.tv_sec  + tv_sec  - u_now->tv_sec)  * 1000000000L) +
                ((ctime.tv_usec + tv_usec - u_now->tv_usec) *       1000L);

      timeout.tv_sec  = ns / 1000000000L;
      timeout.tv_nsec = ns % 1000000000L;

      U_INTERNAL_DUMP("timeout = { %ld %9ld }", timeout.tv_sec, timeout.tv_nsec)

      U_RETURN_POINTER(&timeout, struct timespec);
      }

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
   static struct timespec timeout;

#ifdef U_COMPILER_DELETE_MEMBERS
   UEventTime& operator=(const UEventTime&) = delete;
#else
   UEventTime& operator=(const UEventTime&) { return *this; }
#endif
};

#endif
