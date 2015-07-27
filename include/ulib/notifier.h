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

#include <ulib/container/vector.h>
#include <ulib/container/gen_hash_map.h>

/**
 * NB: to force the use of select() uncomment this define
 *
 * #if defined(U_SERVER_CAPTIVE_PORTAL) && defined(HAVE_EPOLL_WAIT)
 * #  undef HAVE_EPOLL_WAIT
 * #endif
 */

#ifndef _MSWINDOWS_
#  include <sys/select.h>
#  ifdef HAVE_EPOLL_WAIT
#     include <sys/epoll.h>
#     define U_EPOLL_CTL_CMD_SIZE 128
#  endif
#endif

#if defined(HAVE_EPOLL_WAIT) && !defined(USE_LIBEVENT)
#  define U_EPOLLET_POSTPONE_STRATEGY
#endif

#include <ulib/event/event_fd.h>
#include <ulib/event/event_time.h>

class UThread;
class USocket;
class USocketExt;
class UHttpPlugIn;
class UServer_Base;
class UClientImage_Base;

// interface to select() and epoll()

class U_EXPORT UNotifier {
public:

#if defined(HAVE_EPOLL_WAIT) && !defined(USE_LIBEVENT)
   static struct epoll_event* pevents;
#endif
   static uint32_t min_connection, num_connection, max_connection;

   // SERVICES

   static void clear();
   static void init(bool bacquisition);
   static void erase( UEventFd* handler_event);
   static void modify(UEventFd* handler_event);
   static void insert(UEventFd* handler_event);
   static void waitForEvent(UEventTime* timeout);
   static void callForAllEntryDynamic(bPFpv function);

#ifdef HAVE_EPOLL_CTL_BATCH
   static void insertBatch()
      {
      U_TRACE(0, "UNotifier::insertBatch()")

      if (ctl_cmd_cnt)
         {
         (void) U_SYSCALL(epoll_ctl_batch, "%d,%d,%d,%p", epollfd, 0, ctl_cmd_cnt, ctl_cmd);

         ctl_cmd_cnt = 0;
         }
      }

   static void batch((UEventFd* handler_event);
#endif

   static bool empty()
      {
      U_TRACE(0, "UNotifier::empty()")

      U_INTERNAL_ASSERT_POINTER(lo_map_fd)
      U_INTERNAL_ASSERT_POINTER(hi_map_fd)

      if (num_connection == 0)
         {
         U_ASSERT(hi_map_fd->empty())

         U_RETURN(true);
         }

      U_RETURN(false);
      }

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
   static UEventFd** lo_map_fd;
   static UEventFd* handler_event;
   static int max_nfd_ready, nfd_ready; // the number of file descriptors ready for the requested I/O
   static UGenericHashMap<int,UEventFd*>* hi_map_fd; // maps a fd to a node pointer

#ifdef U_EPOLLET_POSTPONE_STRATEGY
   static bool bepollet;
#endif

#ifdef USE_LIBEVENT
// nothing
#elif defined(HAVE_EPOLL_WAIT)
# if defined(ENABLE_THREAD) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   static void* pthread;
# endif
   static int epollfd;
   static struct epoll_event* events;
#else
   static UEventFd* first;
   static fd_set fd_set_read, fd_set_write;
   static int fd_set_max, fd_read_cnt, fd_write_cnt;

   static int  getNFDS();     // nfds is the highest-numbered file descriptor in any of the three sets, plus 1.
   static void removeBadFd(); // rimuove i descrittori di file diventati invalidi (possibile con EPIPE)
#endif
#ifndef HAVE_POLL_H
   static UEventTime* time_obj;
#else
   static struct pollfd fds[1];
   static int waitForEvent(int timeoutMS = -1);
#endif
#ifdef HAVE_EPOLL_CTL_BATCH
   static int ctl_cmd_cnt;
   static struct epoll_ctl_cmd ctl_cmd[U_EPOLL_CTL_CMD_SIZE];
#endif

   static bool isHandler(int fd);
   static bool setHandler(int fd);
   static void resetHandler(int fd);
   static int  waitForEvent(int fd_max, fd_set* read_set, fd_set* write_set, UEventTime* timeout);

private:
   static void handlerDelete(int fd, int mask);
   static void handlerDelete(UEventFd* item) U_NO_EXPORT;

#ifndef USE_LIBEVENT
   static void notifyHandlerEvent() U_NO_EXPORT; 
#endif

#ifdef U_COMPILER_DELETE_MEMBERS
   UNotifier(const UNotifier&) = delete;
   UNotifier& operator=(const UNotifier&) = delete;
#else
   UNotifier(const UNotifier&)            {}
   UNotifier& operator=(const UNotifier&) { return *this; }
#endif

   friend class USocket;
   friend class USocketExt;
   friend class UHttpPlugIn;
   friend class UServer_Base;
   friend class UClientImage_Base;
};

#endif
