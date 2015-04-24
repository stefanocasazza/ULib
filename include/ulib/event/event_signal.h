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
#ifdef U_COMPILER_DELETE_MEMBERS
   UEventSignal(const UEventSignal&) = delete;
   UEventSignal& operator=(const UEventSignal&) = delete;
#else
   UEventSignal(const UEventSignal&)            {}
   UEventSignal& operator=(const UEventSignal&) { return *this; }
#endif
};

#endif
