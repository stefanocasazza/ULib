// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    event_signal.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_EVENT_SIGNAL_H
#define ULIB_EVENT_SIGNAL_H 1

#include <ulib/internal/common.h>

class U_EXPORT UEventSignal {
public:

            UEventSignal() {}
   virtual ~UEventSignal() {}

   // method VIRTUAL to define

   virtual int handlerSignal() { return -1; }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UEventSignal)
};

#endif
