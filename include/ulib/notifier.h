// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    notifier.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_NOTIFIER_H
#define ULIB_NOTIFIER_H

#include <ulib/thread.h>
#include <ulib/container/gen_hash_map.h>

/**
 * NB: to force the use of select() uncomment this define
 *
 * #if defined(U_SERVER_CAPTIVE_PORTAL) && defined(HAVE_EPOLL_WAIT)
 * #undef HAVE_EPOLL_WAIT
 * #endif
 */

#ifndef _MSWINDOWS_
#  include <sys/select.h>
#  ifdef HAVE_EPOLL_WAIT
#     include <sys/epoll.h>
#     define U_EPOLL_CTL_CMD_SIZE 128
#  endif
#endif

#ifndef EPOLLEXCLUSIVE // Provides exclusive wakeups when attaching multiple epoll fds to a shared wakeup source
#  if !defined(U_LINUX) || LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0)
#     define EPOLLEXCLUSIVE 0
#  else
#     define EPOLLEXCLUSIVE (1 << 28)
#  endif
#endif
#ifndef EPOLLROUNDROBIN // Provides balancing for exclusive wakeups when attaching multiple epoll fds to a shared wakeup source
#define EPOLLROUNDROBIN 0 // (1 << 27)
#endif

#if defined(HAVE_EPOLL_WAIT) && !defined(USE_LIBEVENT) && !defined(U_SERVER_CAPTIVE_PORTAL) && \
      (!defined(U_LINUX) || !defined(ENABLE_THREAD) || !defined(U_LOG_DISABLE) || defined(USE_LIBZ))
#  define U_EPOLLET_POSTPONE_STRATEGY
#endif

#include <ulib/event/event_fd.h>
#include <ulib/event/event_time.h>

class USocket;
class UTimeStat;
class USocketExt;
class UHttpPlugIn;
class UServer_Base;
class UClientImage_Base;
class UClientThrottling;

// interface to select() and epoll()

class U_EXPORT UNotifier {
public:

   static long last_event;
   static uint32_t min_connection,
                   num_connection,
                   max_connection;

   // SERVICES

   static bool empty()
      {
      U_TRACE_NO_PARAM(0, "UNotifier::empty()")

      U_INTERNAL_ASSERT_POINTER(lo_map_fd)
      U_INTERNAL_ASSERT_POINTER(hi_map_fd)

      if (num_connection == 0)
         {
         U_ASSERT(hi_map_fd->empty())

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static void handlerDelete(UEventFd* item)
      {
      U_TRACE(0, "UNotifier::handlerDelete(%p)", item)

      U_INTERNAL_ASSERT_POINTER(item)

            handlerDelete(item->fd, item->op_mask);
      item->handlerDelete();
      }

   static void clear();
   static void modify(UEventFd* handler_event);
   static void callForAllEntryDynamic(bPFpv function);
   static void insert(UEventFd* handler_event, int op = 0);

#ifndef USE_LIBEVENT
   static void init();
   static void resume(UEventFd* item);
   static void suspend(UEventFd* item);

   static void waitForEvent();
   static void waitForEvent(                                                 UEventTime* ptimeout);
   static int  waitForEvent(int fd_max, fd_set* read_set, fd_set* write_set, UEventTime* ptimeout);

# ifdef HAVE_EPOLL_CTL_BATCH
   static void insertBatch()
      {
      U_TRACE_NO_PARAM(0, "UNotifier::insertBatch()")

      if (ctl_cmd_cnt)
         {
         (void) U_SYSCALL(epoll_ctl_batch, "%d,%d,%d,%p", epollfd, 0, ctl_cmd_cnt, ctl_cmd);

         ctl_cmd_cnt = 0;
         }
      }

   static void batch((UEventFd* handler_event);
# endif
#else
   static void init()
      {
      U_TRACE(0, "UNotifier::init()")

      if (u_ev_base == 0) u_ev_base = (struct event_base*) U_SYSCALL_NO_PARAM(event_base_new);
      else (void) U_SYSCALL(event_reinit, "%p", u_ev_base); // NB: reinitialized the event base after fork()...

      U_INTERNAL_ASSERT_POINTER(u_ev_base)

      if (lo_map_fd == 0) createMapFd();
      }

   static void suspend(UEventFd* item)
      {
      U_TRACE(0, "UNotifier::suspend(%p)", item)

      U_INTERNAL_ASSERT_POINTER(item)

      // TODO
      }

   static void resume(UEventFd* item)
      {
      U_TRACE(0, "UNotifier::resume(%p)", item)

      U_INTERNAL_ASSERT_POINTER(item)

      // TODO
      }

   static int waitForEvent(int fd_max, fd_set* read_set, fd_set* write_set, UEventTime* ptimeout)
      {
      U_TRACE(0, "UNotifier::waitForEvent(%d,%p,%p,%p)", fd_max, read_set, write_set, ptimeout)

      U_RETURN(-1);
      }

   static void waitForEvent(UEventTime* ptimeout)
      {
      U_TRACE(0, "UNotifier::waitForEvent(%p)", ptimeout)

      (void) UDispatcher::dispatch(UDispatcher::ONCE);
      }

   static void waitForEvent()
      {
      U_TRACE_NO_PARAM(0, "UNotifier::waitForEvent()")

      (void) UDispatcher::dispatch(UDispatcher::ONCE);

#  ifdef DEBUG
      ++nwatches;
#  endif
      }
#endif

   // READ - WRITE

   // param timeoutMS specified the timeout value, in milliseconds.
   // A negative value indicates wait for event with no timeout, i.e. an infinite wait

   static int waitForRead( int fd, int timeoutMS = -1);
   static int waitForWrite(int fd, int timeoutMS = -1);

   static int      read( int fd,       char* buf, int count, int timeoutMS = -1);
   static uint32_t write(int fd, const char* buf, int count, int timeoutMS = -1);

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   static bool bread;
   static int nfd_ready; // the number of file descriptors ready for the requested I/O
   static UEventFd** lo_map_fd;
   static UEventFd* handler_event;
   static UGenericHashMap<int,UEventFd*>* hi_map_fd; // maps a fd to a node pointer
   static uint32_t bepollet_threshold, lo_map_fd_len;

#ifndef USE_LIBEVENT
# ifdef HAVE_EPOLL_WAIT
   static int epollfd;
   static struct epoll_event*  events;
   static struct epoll_event* pevents;
#  ifdef U_EPOLLET_POSTPONE_STRATEGY
   static bool bepollet;
#  endif
#  ifdef HAVE_EPOLL_CTL_BATCH
   static int ctl_cmd_cnt;
   static struct epoll_ctl_cmd ctl_cmd[U_EPOLL_CTL_CMD_SIZE];
#  endif
# elif defined(HAVE_KQUEUE)
   static int kq, nkqevents;
   static struct kevent* kqevents;
   static struct kevent* kqrevents;
# else
   static UEventFd* first;
   static fd_set fd_set_read, fd_set_write;
   static int fd_set_max, fd_read_cnt, fd_write_cnt;

   static int  getNFDS();     // nfds is the highest-numbered file descriptor in any of the three sets, plus 1.
   static void removeBadFd(); // rimuove i descrittori di file diventati invalidi (possibile con EPIPE)
# endif
#endif

#ifdef DEBUG
   static uint32_t nwatches, max_nfd_ready;
#endif

#ifndef HAVE_POLL_H
   static UEventTime* time_obj;
#else
   static struct pollfd fds[1];
   static int waitForEvent(int timeoutMS);
#endif

   static void createMapFd();
   static bool isHandler(int fd);
   static bool setHandler(int fd);

private:
   static void handlerDelete(int fd, int mask);

#ifndef USE_LIBEVENT
   static void notifyHandlerEvent() U_NO_EXPORT;
#endif

#if defined(ENABLE_THREAD) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   static UThread* pthread;
# ifdef _MSWINDOWS_
   static CRITICAL_SECTION mutex;
# else
   static pthread_mutex_t mutex;
# endif
   static void   lock() { if (pthread) UThread::lock(&mutex); }
   static void unlock() { if (pthread) UThread::unlock(&mutex); }
#else
   static void   lock() {}
   static void unlock() {}
#endif

   U_DISALLOW_COPY_AND_ASSIGN(UNotifier)

   friend class ULib;
   friend class USocket;
   friend class UTimeStat;
   friend class USocketExt;
   friend class UHttpPlugIn;
   friend class UApplication;
   friend class UServer_Base;
   friend class UClientImage_Base;
   friend class UClientThrottling;
};
#endif
