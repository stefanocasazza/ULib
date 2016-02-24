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

struct event_base* u_ev_base;

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UEvent_Base::dump(bool reset) const
{
   *UObjectIO::os << "ev_base          " << (void*)ev_base             << '\n'
                  << "ev_fd            " << ev_fd                      << '\n'
                  << "ev_res           " << ev_res                     << '\n'
                  << "ev_events        " << ev_events                  << '\n'

                  << "ev_timeout       " << "{ " << ev_timeout.tv_sec  << ' '
                                         <<         ev_timeout.tv_usec << " }";

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
