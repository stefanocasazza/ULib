// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    event_message.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_EVENT_MESSAGE_H
#define ULIB_EVENT_MESSAGE_H 1

#include <ulib/internal/common.h>

class UString;

class U_EXPORT UEventMessage {
public:

            UEventMessage() {}
   virtual ~UEventMessage() {}

   // -------------------------------------------
   // method VIRTUAL to define
   // -------------------------------------------
   // return value: -1 -> unsuscribe, 0 -> normal
   // -------------------------------------------

   virtual int handlerMessage(const UString& message) { return -1; }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool) const { return ""; }
#endif

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UEventMessage& operator=(const UEventMessage&) = delete;
#else
   UEventMessage& operator=(const UEventMessage&) { return *this; }
#endif
};

#endif
