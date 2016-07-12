// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    timer.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/timer.h>

int     UTimer::mode;
UTimer* UTimer::pool;
UTimer* UTimer::first;

void UTimer::init(Type _mode)
{
   U_TRACE(0, "UTimer::init(%d)", _mode)

   if (u_start_time     == 0 &&
       u_setStartTime() == false)
      {
      U_ERROR("UTimer::init(%d): system date not updated", _mode);
      }

   if ((mode = _mode) != NOSIGNAL)
      {
           if (_mode ==  SYNC) UInterrupt::setHandlerForSignal(SIGALRM, (sighandler_t)UTimer::handlerAlarm);
      else if (_mode == ASYNC) UInterrupt::insert(             SIGALRM, (sighandler_t)UTimer::handlerAlarm); // async signal
      }
}

U_NO_EXPORT void UTimer::insertEntry()
{
   U_TRACE_NO_PARAM(0, "UTimer::insertEntry()")

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("first = %p", first)

   if (first)
      {
      UTimer** ptr = &first;

      do {
         U_INTERNAL_DUMP("this = %p *ptr = %p", this, *ptr)

         if (*this < **ptr) break;

         ptr = &(*ptr)->next;
         }
      while (*ptr);

      next = *ptr;
      *ptr = this;
      }
   else
      {
      // The list is empty

      next  = 0;
      first = this;
      }

   U_ASSERT(invariant())
}

void UTimer::insert(UEventTime* a)
{
   U_TRACE(0, "UTimer::insert(%p)", a)

   // set an alarm to more than 2 month is very suspect...

   U_INTERNAL_ASSERT_MINOR(a->tv_sec, 60L * U_ONE_DAY_IN_SECOND) // 60 gg (2 month)

   UTimer* item;

   if (pool == 0 ||
       mode != NOSIGNAL)
      {
      U_NEW(UTimer, item, UTimer);
      }
   else
      {
      item = pool;
      pool = pool->next;
      }

   // add it in to its new list, sorted correctly

   (item->alarm = a)->setTimeToExpire();

   item->insertEntry();
}

void UTimer::callHandlerTimeout()
{
   U_TRACE_NO_PARAM(0, "UTimer::callHandlerTimeout()")

   U_INTERNAL_ASSERT_POINTER(first)

   UTimer* item = first;
                  first = first->next; // remove it from its active list

   int result = item->alarm->handlerTime();

        if (result == -1) erase(item); // -1 => normal
   else if (result ==  0)              //  0 => monitoring
      {
      U_INTERNAL_DUMP("UEventTime::timeout1 = %#19D (next alarm expire) = %#19D", UEventTime::timeout1.tv_sec, item->next ? item->next->alarm->expire() : 0L)

      u_gettimeofday(&UEventTime::timeout1);

      U_INTERNAL_DUMP("UEventTime::timeout1 = { %ld %6ld } first = %p", UEventTime::timeout1.tv_sec, UEventTime::timeout1.tv_usec, first)

      // add it back in to its new list, sorted correctly

      item->alarm->updateTimeToExpire();

      item->insertEntry();
      }
}

void UTimer::updateTimeToExpire(UEventTime* ptime)
{
   U_TRACE(0, "UTimer::updateTimeToExpire(%p)", ptime)

   U_INTERNAL_ASSERT_POINTER(first)

   UTimer* item;

   for (UTimer** ptr = &first; (item = *ptr); ptr = &(*ptr)->next)
      {
      if (item->alarm == ptime)
         {
         U_INTERNAL_DUMP("*ptr = %p item->next = %p", *ptr, item->next)

         *ptr = item->next; // remove it from its active list

         break;
         }
      }

   U_ASSERT(invariant())
   U_INTERNAL_ASSERT_POINTER(item)

   U_INTERNAL_DUMP("UEventTime::timeout1 = %#19D (next alarm expire) = %#19D", UEventTime::timeout1.tv_sec, item->next ? item->next->alarm->expire() : 0L)

   u_gettimeofday(&UEventTime::timeout1);

   U_INTERNAL_DUMP("UEventTime::timeout1 = { %ld %6ld } first = %p", UEventTime::timeout1.tv_sec, UEventTime::timeout1.tv_usec, first)

   // add it back in to its new list, sorted correctly

   ptime->updateTimeToExpire();

#ifndef U_COVERITY_FALSE_POSITIVE // Dereference after null check (FORWARD_NULL)
   item->insertEntry();
#endif
}

void UTimer::run()
{
   U_TRACE_NO_PARAM(1, "UTimer::run()")

   u_gettimeofday(&UEventTime::timeout1);

   U_INTERNAL_DUMP("UEventTime::timeout1 = { %ld %6ld } first = %p", UEventTime::timeout1.tv_sec, UEventTime::timeout1.tv_usec, first)

   UTimer* item = first;
   bool bnosignal = (mode == NOSIGNAL);

loop:
#ifdef DEBUG
   U_INTERNAL_DUMP("item = %p item->next = %p", item, item->next)

   U_INTERNAL_ASSERT_POINTER(item)

   if (item->next) U_INTERNAL_ASSERT(*item <= *(item->next))
#endif

   if (bnosignal ? item->alarm->isExpired()
                 : item->alarm->isExpiredWithTolerance())
      {
      item = item->next;

      callHandlerTimeout();

      if (item) goto loop;
      }

   U_INTERNAL_DUMP("first = %p", first)

   if (UInterrupt::event_signal_pending) UInterrupt::callHandlerSignal();
}

void UTimer::setTimer()
{
   U_TRACE_NO_PARAM(1, "UTimer::setTimer()")

   U_INTERNAL_ASSERT_DIFFERS(mode, NOSIGNAL)

   run();

   if (first) first->alarm->setTimeVal(&(UInterrupt::timerval.it_value));
   else
      {
      UInterrupt::timerval.it_value.tv_sec  =
      UInterrupt::timerval.it_value.tv_usec = 0L;
      }

   // NB: it can happen that setitimer() produce immediatly a signal because the interval is very short (< 10ms)... 

   U_INTERNAL_DUMP("UInterrupt::timerval.it_value = { %ld %6ld }", UInterrupt::timerval.it_value.tv_sec, UInterrupt::timerval.it_value.tv_usec)

   U_INTERNAL_ASSERT(UInterrupt::timerval.it_value.tv_sec  >= 0 &&
                     UInterrupt::timerval.it_value.tv_usec >= 0)

   (void) U_SYSCALL(setitimer, "%d,%p,%p", ITIMER_REAL, &UInterrupt::timerval, 0);
}

void UTimer::erase(UEventTime* palarm)
{
   U_TRACE(0, "UTimer::erase(%p)", palarm)

   U_INTERNAL_ASSERT_POINTER(first)

   UTimer* item;

   for (UTimer** ptr = &first; (item = *ptr); ptr = &(*ptr)->next)
      {
      if (item->alarm == palarm)
         {
         U_INTERNAL_DUMP("*ptr = %p item->next = %p", *ptr, item->next)

         *ptr = item->next; // remove it from its active list

         erase(item);

         break;
         }
      }

   U_ASSERT(invariant())
}

void UTimer::clear()
{
   U_TRACE_NO_PARAM(1, "UTimer::clear()")

   U_INTERNAL_DUMP("mode = %d first = %p pool = %p", mode, first, pool)

   if (mode != NOSIGNAL)
      {
      UInterrupt::timerval.it_value.tv_sec  =
      UInterrupt::timerval.it_value.tv_usec = 0L;

      (void) U_SYSCALL(setitimer, "%d,%p,%p", ITIMER_REAL, &UInterrupt::timerval, 0);
      }

   if (first)
      {
      for (UTimer* item = first; item; item = item->next)
         {
         delete item->alarm;
         delete item;
         }

      first = 0;
      }

   if (pool)
      {
      for (UTimer* item = pool; item; item = item->next)
         {
         delete item->alarm;
         delete item;
         }

      pool = 0;
      }
}

#ifdef DEBUG
bool UTimer::invariant()
{
   U_TRACE_NO_PARAM(0, "UTimer::invariant()")

   if (first)
      {
      for (UTimer* item = first; item->next; item = item->next)
         {
         if (          item->next &&
             *item > *(item->next))
            {
            U_ERROR("UTimer::invariant() failed: item = %p { %ld %6ld } item->next = %p { %ld %6ld }",
                        item,             item->alarm->xtime.tv_sec,       item->alarm->xtime.tv_usec,
                        item->next, item->next->alarm->xtime.tv_sec, item->next->alarm->xtime.tv_usec);
            }
         }
      }

   U_RETURN(true);
}
#endif

// STREAM

#ifdef U_STDCPP_ENABLE
U_EXPORT ostream& operator<<(ostream& os, const UTimer& t)
{
   U_TRACE(0+256, "UTimer::operator<<(%p,%p)", &os, &t)

   os.put('(');
   os.put(' ');

   if (t.alarm) os << *t.alarm;
   else         os << (void*)&t;

   for (UTimer* item = t.next; item; item = item->next)
      {
      os.put(' ');

      if (item->alarm) os << *item->alarm;
      else             os << (void*)item;
      }

   os.put(' ');
   os.put(')');

   return os;
}

#  ifdef DEBUG
void UTimer::printInfo(ostream& os)
{
   U_TRACE(0+256, "UTimer::printInfo(%p)", &os)

   os << "first = ";

   if (first) os << *first;
   else       os << (void*)first;

   os << "\npool  = ";

   if (pool) os << *pool;
   else      os << (void*)pool;

   os << "\n";
}

const char* UTimer::dump(bool reset) const
{
   *UObjectIO::os << "timerval                 " << "{ { "  << UInterrupt::timerval.it_interval.tv_sec
                                                 << " "     << UInterrupt::timerval.it_interval.tv_usec
                                                 << " } { " << UInterrupt::timerval.it_value.tv_sec
                                                 << " "     << UInterrupt::timerval.it_value.tv_usec
                                                                 << " } }\n"
                  << "pool         (UTimer     " << (void*)pool  << ")\n"
                  << "first        (UTimer     " << (void*)first << ")\n"
                  << "next         (UTimer     " << (void*)next  << ")\n"
                  << "alarm        (UEventTime " << (void*)alarm << ")";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
