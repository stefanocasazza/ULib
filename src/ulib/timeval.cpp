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

#ifndef HAVE_NANOSLEEP
extern "C" { int nanosleep (const struct timespec* requested_time,
                                  struct timespec* remaining); }
#endif

struct timeval UTimeVal::time_stop;

void UTimeVal::nanosleep()
{
   U_TRACE_NO_PARAM(1, "UTimeVal::nanosleep()")

   int result;
   struct timespec req, rem;

   setTimeSpec(&req);

   // EINVAL: The value in the tv_nsec field was not in the range 0 to 999999999 or tv_sec was negative

   U_INTERNAL_ASSERT(req.tv_sec >= 0L)
   U_INTERNAL_ASSERT_RANGE(0L, req.tv_nsec, 999999999L)

   U_INTERNAL_DUMP("Call   nanosleep(%2D)")

loop:
   U_INTERNAL_DUMP("req = {%ld,%ld}", req.tv_sec, req.tv_nsec)

   result = U_SYSCALL(nanosleep, "%p,%p", &req, &rem);

   if (result == -1 &&
        errno == EINTR)
      {
      UInterrupt::checkForEventSignalPending();

      if (UInterrupt::isSysCallToRestart() &&
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
