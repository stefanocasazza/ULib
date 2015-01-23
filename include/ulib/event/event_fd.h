// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    event_fd.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_EVENT_FD_H
#define ULIB_EVENT_FD_H 1

#include <ulib/internal/common.h>

#ifdef USE_LIBEVENT
#  include <ulib/libevent/event.h>
#endif

#ifndef EPOLLIN
#define EPOLLIN    0x0001
#endif
#ifndef EPOLLOUT
#define EPOLLOUT   0x0004
#endif
#ifndef EPOLLRDHUP
#define EPOLLRDHUP 0x2000
#endif
// -------------------------------------------------------------------------------------------------------------------------
// EPOLLET is edge-triggered (alas SIGIO, when that descriptor transitions from not ready to ready, the kernel notifies you)
// -------------------------------------------------------------------------------------------------------------------------
#ifndef EPOLLET
#define EPOLLET (1 << 31)
#endif

#define U_NOTIFIER_OK      0
#define U_NOTIFIER_DELETE -1

class U_EXPORT UEventFd {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   int fd;
   uint32_t op_mask; // [ EPOLLIN | EPOLLOUT ]

   UEventFd()
      {
      U_TRACE_REGISTER_OBJECT(0, UEventFd, "", 0)
   
      fd      = -1;
      op_mask = EPOLLIN | EPOLLRDHUP;

#  ifdef USE_LIBEVENT
      pevent = 0;
#  endif
      }

   virtual ~UEventFd() __pure
      {
      U_TRACE_UNREGISTER_OBJECT(0, UEventFd)

#  ifdef USE_LIBEVENT
      if (pevent)
         {
         UDispatcher::del(pevent);
                   delete pevent;
         }
#  endif
      }

   // method VIRTUAL to define

   virtual int handlerRead()    { return U_NOTIFIER_DELETE; }
   virtual int handlerWrite()   { return U_NOTIFIER_DELETE; }
   virtual int handlerTimeout() { return U_NOTIFIER_DELETE; }

   virtual void handlerError()  { }
   virtual void handlerDelete() { delete this; }

#ifdef USE_LIBEVENT
   UEvent<UEventFd>* pevent;

   void operator()(int fd, short event);
#endif

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool) const { return ""; } 
#endif

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UEventFd(const UEventFd&) = delete;
   UEventFd& operator=(const UEventFd&) = delete;
#else
   UEventFd(const UEventFd&)            {}
   UEventFd& operator=(const UEventFd&) { return *this; }
#endif
};

#endif
