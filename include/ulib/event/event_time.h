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

   void setCurrentTime();
   bool isOld() const __pure;
   bool isExpired() const __pure;
   void setTimerVal(struct timeval* it_value);

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
