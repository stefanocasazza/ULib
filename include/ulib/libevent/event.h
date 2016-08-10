// =====================================================================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    event.h - wrapper for libevent (A library to execute a function when a specific event occurs on a file descriptor)
//
// = AUTHOR
//    Stefano Casazza
//
// =====================================================================================================================

#ifndef ULIB_EVENT_H
#define ULIB_EVENT_H

#include <ulib/timeval.h>

#include <event.h>

extern U_EXPORT struct event_base* u_ev_base;

/**
 * Basic event from which all events derive.
 *
 * All events derive from this class, so it's useful for use in containers
 *
 * libevent is a popular API that provides a mechanism to execute a callback
 * function when a specific event occurs on a file descriptor or after a
 * timeout has been reached. Furthermore, libevent also support callbacks due
 * to signals or regular timeouts
 */

class U_EXPORT UEvent_Base : public event {
public:

   // Check Memory
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   /**
    * Event's file descriptor
    *
    * @return Event's file descriptor
    */

   int fd() { return EVENT_FD(this); }

   /**
    * Checks if there is an event pending
    *
    * @param ev type of event to check. There are 4 kind of events: EV_TIMEOUT, EV_READ, EV_WRITE or EV_SIGNAL. EV_PERSIST is not
    *           an event, is an event modifier flag, that tells libevent that this event should live until del() is called
    *
    * @return true if there is a pending event, false if not
    */

   bool pending(short __ev = EV_READ, UTimeVal* tv = 0)
      {
      U_TRACE(1, "UEvent_Base::pending(%hd,%p)", __ev, tv)

      if (U_SYSCALL(event_pending, "%p,%hd,%p", this, __ev, tv)) U_RETURN(true);

      U_RETURN(false);
      }

   /**
    * Timeout of the event
    *
    * @return Timeout of the event
    */

   bool timeout(UTimeVal& tv) { return pending(EV_TIMEOUT, &tv); }

   /**
    * Sets the event's priority
    *
    * @param priority New event priority
    *
    * @pre The event must be added to UDispatcher
    * @see UDispatcher::priority_init()
    */

   bool priority(int _priority)
      {
      U_TRACE(1, "UEvent_Base::priority(%d)", _priority)

      if (U_SYSCALL(event_priority_set, "%p,%d", this, _priority)) U_RETURN(false);

      U_RETURN(true);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UEvent_Base() {} // NB: This is an concept class, you can't instantiate it

private:
   U_DISALLOW_COPY_AND_ASSIGN(UEvent_Base)
};

/**
 * Generic event object
 *
 * This object stores all the information about an event, including a callback functor, which is called when the event is fired. The template parameter
 * must be a functor (callable object or function) that can take 2 parameters: an integer (the file descriptor of the fired event) and an event::type (the
 * type of event being fired). There is a specialized version of this class which takes as the template parameter a C function with the ccallback_type
 * signature, just like C @libevent API does
 *
 * @see event< ccallback_type >
 */

typedef void (*ccallback_type)(int, short, void*); // C function used as callback in the C API

template <typename F> class U_EXPORT UEvent : public UEvent_Base {
public:

   /**
   * Creates a new event
   *
   * @param fd       File descriptor to monitor for events
   * @param ev       Type of events to monitor (see event::type)
   * @param handler  Callback functor
   */

   UEvent(int _fd, short __ev, F& handler)
      {
      U_TRACE_REGISTER_OBJECT(0, UEvent<F>, "%d,%hd,%p", _fd, __ev, &handler)

      U_SYSCALL_VOID(event_set, "%p,%d,%hd,%p,%p", this, _fd, __ev, wrapper, (void*)&handler);
      }

   ~UEvent()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UEvent)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return UEvent_Base::dump(reset); }
#endif

   static void wrapper(int _fd, short __ev, void* h)
      {
      U_TRACE(0, "UEvent::wrapper(%d,%hd,%p)", _fd, __ev, h)

      F& handler = *(F*)h;

      handler(_fd, __ev);
      }

protected:
   UEvent() {}

private:
   U_DISALLOW_COPY_AND_ASSIGN(UEvent<F>)
};

/**
 * This is the specialization of UEvent for C-style callbacks
 *
 * @see UEvent 
 */

template <> class U_EXPORT UEvent<ccallback_type> : public UEvent_Base {
public:

   /**
    * Creates a new event
    *
    * @param fd      File descriptor to monitor for events
    * @param ev      Type of events to monitor (see event::type)
    * @param handler C-style callback function
    * @param arg     Arbitrary pointer to pass to the handler as argument
    */

   UEvent(int _fd, short __ev, ccallback_type handler, void* arg = 0)
      {
      U_TRACE_REGISTER_OBJECT(0, UEvent<ccallback_type>, "%d,%hd,%p,%p", _fd, __ev, handler, arg)

      U_SYSCALL_VOID(event_set, "%p,%d,%hd,%p,%p", this, _fd, __ev, handler, arg);
      }

   ~UEvent()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UEvent)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return UEvent_Base::dump(reset); }
#endif

protected:
   UEvent() {}

private:
   U_DISALLOW_COPY_AND_ASSIGN(UEvent<ccallback_type>)
};

/**
 * UTimerEv event object
 *
 * This is just a special case of event that is fired only when a timeout is reached. It's just a shortcut to:
 * @code
 * UEvent(-1, 0, handler);
 * @endcode
 *
 * @note This event can't EV_PERSIST
 *
 * @see timer< ccallback_type >
 */

template <typename F> class U_EXPORT UTimerEv : public UEvent<F> {
public:

   /**
    * Creates a new timer event
    *
    * @param handler Callback functor
    */

   UTimerEv(F& handler)
      {
      U_TRACE_REGISTER_OBJECT(0, UTimerEv, "%p", &handler)

      U_SYSCALL_VOID(event_set, "%p,%d,%hd,%p,%p", this, -1, 0, UEvent<F>::wrapper, (void*)&handler);
      }

   ~UTimerEv()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTimerEv)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return UEvent_Base::dump(reset); }
#endif

protected:
   UTimerEv() {}

private:
   U_DISALLOW_COPY_AND_ASSIGN(UTimerEv)
};

/**
 * This is the specialization of UTimerEv for C-style callbacks
 *
 * @note This event can't PERSIST
 *
 * @see UTimerEv
 */

template <> class U_EXPORT UTimerEv<ccallback_type> : public UEvent<ccallback_type> {
public:

   /**
    * Creates a new timer event
    *
    * @param handler C-style callback function
    * @param arg     Arbitrary pointer to pass to the handler as argument
    */

   UTimerEv(ccallback_type handler, void* arg = 0)
      {
      U_TRACE_REGISTER_OBJECT(0, UTimerEv<ccallback_type>, "%p,%p", handler, arg)

      U_SYSCALL_VOID(event_set, "%p,%d,%hd,%p,%p", this, -1, 0, handler, arg);
      }

   ~UTimerEv()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTimerEv)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return UEvent_Base::dump(reset); }
#endif

protected:
   UTimerEv() {}

private:
   U_DISALLOW_COPY_AND_ASSIGN(UTimerEv<ccallback_type>)
};

/**
 * Signal event object
 *
 * This is just a special case of event that is fired when a signal is raised (instead of a file descriptor being active). It's just a shortcut to:
 * @code
 * event(signum, EV_SIGNAL, handler);
 * @endcode
 *
 * @note This event always EV_PERSIST
 *
 * @see signal< ccallback_type >
 */

template <typename F> class U_EXPORT USignal : public UEvent<F> {
public:

   /**
    * Creates a new signal event
    *
    * @param signum  Signal number to monitor
    * @param handler Callback functor
    */

   USignal(int _signum, F& handler)
      {
      U_TRACE_REGISTER_OBJECT(0, USignal, "%d,%p", _signum, &handler)

      U_SYSCALL_VOID(event_set, "%p,%d,%hd,%p,%p", this, _signum, EV_SIGNAL | EV_PERSIST, UEvent<F>::wrapper, (void*)&handler);
      }

   ~USignal()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USignal)
      }

   /**
    * Event's signal number
    *
    * @return Event's signal number
    */

   int signum() const { return EVENT_SIGNAL(this); }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return UEvent_Base::dump(reset); }
#endif

protected:
   USignal() {}

private:
   U_DISALLOW_COPY_AND_ASSIGN(USignal)
};

/**
 * This is the specialization of USignal for C-style callbacks
 *
 * @note This event always EV_PERSIST
 *
 * @see USignal
 */

template <> class U_EXPORT USignal<ccallback_type> : public UEvent<ccallback_type> {
public:

   /**
    * Creates a new signal event
    *
    * @param signum  Signal number to monitor
    * @param handler C-style callback function
    * @param arg     Arbitrary pointer to pass to the handler as argument
    */

   USignal(int _signum, ccallback_type handler, void* arg = 0)
      {
      U_TRACE_REGISTER_OBJECT(0, USignal<ccallback_type>, "%d,%p,%p", _signum, handler, arg)

      U_SYSCALL_VOID(event_set, "%p,%d,%hd,%p,%p", this, _signum, EV_SIGNAL | EV_PERSIST, handler, arg);
      }

   ~USignal()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USignal)
      }

   /**
    * Event's signal number
    *
    * @return Event's signal number
    */

   int signum() const { return EVENT_SIGNAL(this); }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return UEvent_Base::dump(reset); }
#endif

protected:
   USignal() {}

private:
   U_DISALLOW_COPY_AND_ASSIGN(USignal<ccallback_type>)
};

typedef UEvent<ccallback_type>   cevent;  // Shortcut to C-style event
typedef USignal<ccallback_type>  csignal; // Shortcut to C-style signal handler
typedef UTimerEv<ccallback_type> ctimer;  // Shortcut to C-style timer

/**
 * Helper functor to use an arbitrary member function as an event handler
 *
 * With this wrapper, you can use any object method, which accepts the right parameters (int, short) and returns void, as an event handler.
 * This way you don't have to overload the operator() which can be confusing depending on the context
 */

template <typename O, typename M> class U_EXPORT UMemCb {
public:

   /**
    * Member function callback constructor
    *
    * It expects to receive a class as the first parameter (O), and a member function (of that class O) as the second parameter
    * When this instance is called with fd and ev as function arguments, object.method(fd, ev) will be called
    *
    * @param object Object to be used
    * @param method Method to be called
    */

   UMemCb(O& object, M method) : _object(object), _method(method) {}

   void operator()(int fd, short _ev) { (_object.*_method)(fd, _ev); }

protected:
   O& _object;
   M  _method;
};

/**
 * Event dispatcher
 *
 * This class is the responsible for looping and dispatching events. Every time you need an event loop you should use this class
 *
 * You can @link UDispatcher::add add @endlink events to the UDispatcher, and you can @link UDispatcher::del remove @endlink them later or
 * you can @link UDispatcher::add_once add events to be processed just once @endlink. You can @link UDispatcher::dispatch loop once or forever
 * @endlink (well, of course you can break that forever removing all the events or by @link UDispatcher::exit exiting the loop @endlink)
 */

class U_EXPORT UDispatcher {
public:

   // Miscellaneous constants

   enum {
      DEFAULT_PRIORITY = -1,              // Default priority (the middle value)
      ONCE             = EVLOOP_ONCE,     // Loop just once
      NONBLOCK         = EVLOOP_NONBLOCK  // Don't block the event loop
   };

   /**
   * set npriorities priorities
   *
   * @param npriorities Number of priority queues to use
   */

   static bool priority_init(int npriorities)
      {
      U_TRACE(1, "UDispatcher::priority_init(%d)", npriorities)

      if (U_SYSCALL(event_base_priority_init, "%p,%d", u_ev_base, npriorities)) U_RETURN(false);

      U_RETURN(true);
      }

   /**
   * Adds an event to UDispatcher
   *
   * @param e        Event to add
   * @param priority Priority of the event
   */

   static bool add(UEvent_Base& e, int priority = DEFAULT_PRIORITY)
      {
      U_TRACE(1, "UDispatcher::add(%p,%d)", &e, priority)

      if (priority != DEFAULT_PRIORITY &&
          U_SYSCALL(event_priority_set, "%p,%d", &e, priority))
         {
         U_RETURN(false);
         }

      if (U_SYSCALL(event_add, "%p,%p", &e, 0)) U_RETURN(false);

      U_RETURN(true);
      }

   /**
   * Adds an event to UDispatcher with a timeout
   *
   * The event is fired when there is activity on e or when to has elapsed, whatever come first
   *
   * @param e        Event to add
   * @param to       Timeout
   * @param priority Priority of the event
   */

   static bool add(UEvent_Base& e, const UTimeVal& to, int priority = DEFAULT_PRIORITY)
      {
      U_TRACE(1, "UDispatcher::add(%p,%p,%d)", &e, &to, priority)

      if (priority != DEFAULT_PRIORITY &&
          U_SYSCALL(event_priority_set, "%p,%d", &e, priority))
         {
         U_RETURN(false);
         }

      if (U_SYSCALL(event_add, "%p,%p", &e, (timeval*)&to)) U_RETURN(false);

      U_RETURN(true);
      }

   /**
   * Adds a temporary event
   *
   * Adds a temporary event, without the need of instantiating a new event object. Events added this way can't EV_PERSIST
   *
   * @param fd       File descriptor to monitor for events
   * @param ev       Type of events to monitor
   * @param handler  Callback function
   */

   template <typename F> static bool add_once(int fd, short _ev, F& handler)
      {
      U_TRACE(1, "UDispatcher::add_once(%d,%hd,%p)", fd, _ev, &handler)

      if (U_SYSCALL(event_once, "%d,%hd,%p,%p,%p", fd, _ev, UDispatcher::wrapper<F>, (void*)&handler, 0)) U_RETURN(false);

      U_RETURN(true);
      }

   /**
   * Adds a temporary event
   *
   * Adds a temporary event, without the need of instantiating a new event object. Events added this way can't EV_PERSIST
   *
   * @param fd       File descriptor to monitor for events
   * @param ev       Type of events to monitor
   * @param handler  Callback function
   * @param to       Timeout
   */

   template <typename F> static bool add_once(int fd, short _ev, F& handler, const UTimeVal& to)
      {
      U_TRACE(1, "UDispatcher::add_once(%d,%hd,%p,%p)", fd, _ev, &handler, &to)

      if (U_SYSCALL(event_once, "%d,%hd,%p,%p,%p", fd, _ev, UDispatcher::wrapper<F>, (void*)&handler, (timeval*)&to)) U_RETURN(false);

      U_RETURN(true);
      }

   /**
   * Adds a temporary event to with a C-style callback
   *
   * Adds a temporary event, without the need of instantiating a new event object. Events added this way can't EV_PERSIST
   *
   * @param fd       File descriptor to monitor for events
   * @param ev       Type of events to monitor
   * @param handler  Callback function
   * @param arg      Arbitrary pointer to pass to the handler as argument
   */

   static bool add_once(int fd, short _ev, ccallback_type handler, void* arg)
      {
      U_TRACE(1, "UDispatcher::add_once(%d,%hd,%p,%p)", fd, _ev, handler, arg)

      if (U_SYSCALL(event_once, "%d,%hd,%p,%p,%p", fd, _ev, handler, arg, 0)) U_RETURN(false);

      U_RETURN(true);
      }

   /**
   * Adds a temporary event with a C-style callback
   *
   * Adds a temporary event, without the need of instantiating a new event object. Events added this way can't EV_PERSIST
   *
   * @param fd       File descriptor to monitor for events
   * @param ev       Type of events to monitor
   * @param handler  Callback function
   * @param arg      Arbitrary pointer to pass to the handler as argument
   * @param to       Timeout
   */

   static bool add_once(int fd, short _ev, ccallback_type handler, void* arg, const UTimeVal& to)
      {
      U_TRACE(1, "UDispatcher::add_once(%d,%hd,%p,%p,%p)", fd, _ev, handler, arg, &to)

      if (U_SYSCALL(event_once, "%d,%hd,%p,%p,%p", fd, _ev, handler, arg, (timeval*)&to)) U_RETURN(false);

      U_RETURN(true);
      }

   /**
   * Adds a temporary timer
   *
   * Adds a temporary timer, without the need of instantiating a new timer object
   *
   * @param handler  Callback function
   * @param to       UTimerEv's timeout
   */

   template <typename F> static bool add_once_timer(F& handler, const UTimeVal& to)
      {
      U_TRACE(1, "UDispatcher::add_once_timer(%p,%p)", &handler, &to)

      if (U_SYSCALL(event_once,"%d,%hd,%p,%p,%p",-1,EV_TIMEOUT,UDispatcher::wrapper<F>,(void*)&handler,(timeval*)&to)) U_RETURN(false);

      U_RETURN(true);
      }

   /**
   * Adds a temporary timer with a C-style callback
   *
   * Adds a temporary timer, without the need of instantiating a new timer object
   *
   * @param handler  Callback function
   * @param arg      Arbitrary pointer to pass to the handler as argument
   * @param to       UTimerEv's timeout
   */

   static bool add_once_timer(ccallback_type handler, void* arg, const UTimeVal& to)
      {
      U_TRACE(1, "UDispatcher::add_once_timer(%p,%p,%p)", handler, arg, &to)

      if (U_SYSCALL(event_once, "%d,%hd,%p,%p,%p", -1, EV_TIMEOUT, handler, arg, (timeval*)&to)) U_RETURN(false);

      U_RETURN(true);
      }

   /**
   * Removes an event
   *
   * The event e will be no longer monitored by UDispatcher
   *
   * @param e Event to remove
   */

   static bool del(UEvent_Base* e)
      {
      U_TRACE(1, "UDispatcher::del(%p)", e)

      U_INTERNAL_ASSERT_POINTER(e)

      if (U_SYSCALL(event_del, "%p", e)) U_RETURN(false);

      U_RETURN(true);
      }

   /**
   * Main UDispatcher loop
   *
   * This function takes the control of the program, waiting for an event and calling its callbacks when it's fired. It only returns under this conditions:
   * - exit() was called
   * - All events were del()eted
   * - Another internal error
   * - ONCE flag was set
   * - NONBLOCK flag was set
   *
   * @param flags If ONCE is specified, then just one event is
   *              processed, if NONBLOCK is specified, then this
   *              function returns even if there are no pending events
   *
   * @return 0 if NONBLOCK or ONCE is set, 1 if there
   *         are no more events registered and EINTR if you use the
   *         @libevent's  @c event_gotsig and return -1 in your
   *         @c event_sigcb callback
   */

   static int dispatch(int flags = 0)
      {
      U_TRACE(1, "UDispatcher::dispatch(%d)", flags)

      int ret = U_SYSCALL(event_base_loop, "%p,%d", u_ev_base, flags);

      U_RETURN(ret);
      }

   /**
   * Exit the dispatch() loop
   *
   * @param to If a timeout is given, the loop exits after the specified time is elapsed
   *
   * @return Not very well specified by @libevent
   */

   static int exit(const UTimeVal* to = 0)
      {
      U_TRACE(1, "UDispatcher::exit(%p)", to)

      int ret = U_SYSCALL(event_base_loopexit, "%p,%p", u_ev_base, (timeval*)to);

      U_RETURN(ret);
      }

protected:
   template <typename F> static void wrapper(int fd, short _ev, void* h)
      {
      U_TRACE(0, "UDispatcher::wrapper(%d,%hd,%p)", fd, _ev, h)

      F& handler = *(F*)h;

      handler(fd, _ev);
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UDispatcher)
};

#endif
