// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    event.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/libevent/event.h>

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UEvent_Base::dump(bool reset) const
{
   /*
   struct event {
   TAILQ_ENTRY (event) ev_next;
   TAILQ_ENTRY (event) ev_active_next;
   TAILQ_ENTRY (event) ev_signal_next;
   RB_ENTRY (event)    ev_timeout_node;

   struct event_base* ev_base;
   int ev_fd;
   short ev_events;
   short ev_ncalls;
   short* ev_pncalls; // Allows deletes in callback

   struct timeval ev_timeout;

   int ev_pri;    // smaller numbers are higher priority

   void (*ev_callback)(int, short, void* arg);
   void* ev_arg;

   int ev_res;    // result passed to event callback
   int ev_flags;
   };

   *UObjectIO::os << "ev_next          " << (void*)&ev_next            << '\n'
                  << "ev_active_next   " << (void*)&ev_active_next     << '\n'
                  << "ev_signal_next   " << (void*)&ev_signal_next     << '\n'
                  << "ev_timeout_node  " << (void*)&ev_timeout_node    << '\n'
   */

   *UObjectIO::os << "ev_base          " << (void*)ev_base             << '\n'
                  << "ev_fd            " << ev_fd                      << '\n'
                  << "ev_events        " << ev_events                  << '\n'

                  << "ev_timeout       " << "{ " << ev_timeout.tv_sec  << ' '
                                         <<         ev_timeout.tv_usec << " }"
                                         << '\n'

                  << "ev_pri           " << ev_pri                     << '\n'

                  << "ev_callback      " << (void*)ev_callback         << '\n'
                  << "ev_arg           " << (void*)ev_arg              << '\n'

                  << "ev_res           " << ev_res                     << '\n'
                  << "ev_flags         " << ev_flags;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
