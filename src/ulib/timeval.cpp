// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    timeval.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/timeval.h>
#include <ulib/utility/interrupt.h>

#include <errno.h>

#ifndef HAVE_NANOSLEEP
extern "C" { int nanosleep (const struct timespec* requested_time,
                                  struct timespec* remaining); }
#endif

#ifndef __suseconds_t_defined
typedef long suseconds_t;
#endif

void UTimeVal::adjust(void* tv_sec, void* tv_usec)
{
   U_TRACE(0, "UTimeVal::adjust(%p, %p)", tv_sec, tv_usec)

   long riporto = *((suseconds_t*)tv_usec) / U_SECOND;

   // NB: riporto puo' essere anche negativo...

   if (riporto)
      {
      *((long*)tv_sec)         += riporto;
      *((suseconds_t*)tv_usec) %= U_SECOND;
      }

   U_INTERNAL_ASSERT_MINOR(*((suseconds_t*)tv_usec), U_SECOND)

   if (*((suseconds_t*)tv_usec) < 0) { *((suseconds_t*)tv_usec) += U_SECOND; --(*((long*)tv_sec)); }

   U_INTERNAL_ASSERT_RANGE(0, *((suseconds_t*)tv_usec), U_SECOND)
}

void UTimeVal::nanosleep()
{
   U_TRACE(1, "UTimeVal::nanosleep()")

   int result;
   struct timespec req, rem;

   setTimeSpec(&req);

   // EINVAL: The value in the tv_nsec field was not in the range 0 to 999999999 or tv_sec was negative.

   U_INTERNAL_ASSERT(req.tv_sec >= 0L)
   U_INTERNAL_ASSERT_RANGE(0L, req.tv_nsec, 999999999L)

   U_INTERNAL_DUMP("Call   nanosleep(%2D)")

loop:
   U_INTERNAL_DUMP("req = {%ld,%ld}", req.tv_sec, req.tv_nsec)

   result = U_SYSCALL(nanosleep, "%p,%p", &req, &rem);

   if (result == -1 &&
        errno == EINTR)
      {
      if (UInterrupt::checkForEventSignalPending() &&
          operator>(&rem))
         {
         req = rem;

         goto loop;
         }
      }

   U_INTERNAL_DUMP("Return nanosleep(%2D)")
}

__pure bool UTimeVal::operator<(struct timeval& t) const
{
   U_TRACE(0, "UTimeVal::operator<(%p,%p)", this, &t)

   U_CHECK_MEMORY

   bool result = (tv_sec <  t.tv_sec ||
                 (tv_sec == t.tv_sec && tv_usec < t.tv_usec));

   U_RETURN(result);
}

__pure bool UTimeVal::operator>(struct timeval& t) const
{
   U_TRACE(0, "UTimeVal::operator<(%p,%p)", this, &t)

   U_CHECK_MEMORY

   bool result = (t.tv_sec <  tv_sec ||
                 (t.tv_sec == tv_sec && t.tv_usec < tv_usec));

   U_RETURN(result);
}

__pure bool UTimeVal::operator<(const UTimeVal& t) const
{
   U_TRACE(0, "UTimeVal::operator<(%p,%p)", this, &t)

   U_CHECK_MEMORY

   bool result = (tv_sec <  t.tv_sec ||
                 (tv_sec == t.tv_sec && tv_usec < t.tv_usec));

   U_RETURN(result);
}


// CHRONOMETER

long UTimeVal::restart()
{
   U_TRACE(1, "UTimeVal::restart()")

   (void) U_SYSCALL(gettimeofday, "%p,%p", u_now, 0);

   struct timeval time_from_last_start = { u_now->tv_sec - tv_sec, u_now->tv_usec - tv_usec };

   tv_sec  = u_now->tv_sec;
   tv_usec = u_now->tv_usec;

   while (time_from_last_start.tv_usec < 0L)
      {
      time_from_last_start.tv_sec--;
      time_from_last_start.tv_usec += 1000000L;
      }

   long ms = time_from_last_start.tv_sec * 1000L + (time_from_last_start.tv_usec / 1000L);

   if (ms <= 0L) U_RETURN(0L);

   U_RETURN(ms);
}

long UTimeVal::stop()
{
   U_TRACE(1, "UTimeVal::stop()")

   (void) U_SYSCALL(gettimeofday, "%p,%p", u_now, 0);

   struct timeval time_elapsed = { u_now->tv_sec - tv_sec, u_now->tv_usec - tv_usec };

   if (time_elapsed.tv_usec < 0L)
      {
      time_elapsed.tv_sec--;
      time_elapsed.tv_usec += 1000000L;
      }

   if (time_elapsed.tv_sec < 0L) U_RETURN(0);

   long ms = time_elapsed.tv_sec * 1000L + (time_elapsed.tv_usec / 1000L);

   U_RETURN(ms);
}

__pure double UTimeVal::getTimeElapsed() const
{
   U_TRACE(0, "UTimeVal::getTimeElapsed()")

   struct timeval time_elapsed = { u_now->tv_sec - tv_sec, u_now->tv_usec - tv_usec };

   if (time_elapsed.tv_usec < 0L)
      {
      time_elapsed.tv_sec--;
      time_elapsed.tv_usec += 1000000L;
      }

   double ms = (time_elapsed.tv_sec * 1000.) + (time_elapsed.tv_usec / 1000.);

   U_RETURN(ms);
}

// STREAMS

#ifdef U_STDCPP_ENABLE
U_EXPORT istream& operator>>(istream& is, UTimeVal& t)
{
   U_TRACE(0,"UTimeVal::operator>>(%p,%p)", &is, &t)

   U_INTERNAL_ASSERT_EQUALS(is.peek(),'{')

   int c;
   streambuf* sb = is.rdbuf();

   (void) sb->sbumpc(); // skip '{'

   is >> t.tv_sec >> t.tv_usec;

   // skip '}'

   do { c = sb->sbumpc(); } while (c != '}' && c != EOF);

   if (c == EOF) is.setstate(ios::eofbit);

   return is;
}

U_EXPORT ostream& operator<<(ostream& os, const UTimeVal& t)
{
   U_TRACE(0, "UTimeVal::operator<<(%p,%p)", &os, &t)

   os.put('{');
   os.put(' ');
   os << t.tv_sec;
   os.put(' ');
   os.width(6);
   os << t.tv_usec;
   os.put(' ');
   os.put('}');

   return os;
}

#  ifdef DEBUG
const char* UTimeVal::dump(bool reset) const
{
   *UObjectIO::os << "tv_sec  " << tv_sec << '\n'
                  << "tv_usec " << tv_usec;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
