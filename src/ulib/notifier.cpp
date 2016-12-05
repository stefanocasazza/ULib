// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    notifier.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/timer.h>
#include <ulib/notifier.h>
#include <ulib/net/socket.h>
#include <ulib/internal/chttp.h>
#include <ulib/utility/interrupt.h>
#include <ulib/net/server/server_plugin.h>

#ifdef HAVE_KQUEUE
#  include <sys/event.h>
#endif

#ifndef HAVE_POLL_H
UEventTime* UNotifier::time_obj;
#else
#  include <poll.h>
struct pollfd UNotifier::fds[1];
#endif

/**
 * typedef union epoll_data {
 *    void* ptr;
 *    int fd;
 *    uint32_t u32;
 *    uint64_t u64;
 * } epoll_data_t;
 *
 * struct epoll_event {
 *    uint32_t events;   // Epoll events
 *    epoll_data_t data; // User data variable
 * };
 *
 * struct epoll_ctl_cmd {
 *    int flags;        // Reserved flags for future extension, must be 0
 *    int op;           // The same as epoll_ctl() op parameter
 *    int fd;           // The same as epoll_ctl() fd parameter
 *    uint32_t events;  // The same as the "events" field in struct epoll_event
 *    uint64_t data;    // The same as the "data"   field in struct epoll_event
 *    int error_hint;   // Output field, will be set to the return code after this command is executed by kernel
 * };
 */

int                             UNotifier::nfd_ready; // the number of file descriptors ready for the requested I/O
long                            UNotifier::last_event;
uint32_t                        UNotifier::min_connection;
uint32_t                        UNotifier::num_connection;
uint32_t                        UNotifier::max_connection;
uint32_t                        UNotifier::lo_map_fd_len;
uint32_t                        UNotifier::bepollet_threshold = 10;
UEventFd*                       UNotifier::handler_event;
UEventFd**                      UNotifier::lo_map_fd;
UGenericHashMap<int,UEventFd*>* UNotifier::hi_map_fd; // maps a fd to a node pointer

#ifdef DEBUG
uint32_t UNotifier::nwatches;
uint32_t UNotifier::max_nfd_ready;
#endif

#if defined(ENABLE_THREAD) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
UThread* UNotifier::pthread;
#  ifdef _MSWINDOWS_
CRITICAL_SECTION UNotifier::mutex;
#  else
pthread_mutex_t UNotifier::mutex = PTHREAD_MUTEX_INITIALIZER;
#  endif
#endif

#ifdef USE_LIBEVENT
void UEventFd::operator()(int _fd, short event)
{
   U_TRACE(0, "UEventFd::operator()(%d,%hd)", _fd, event)

   int ret = (event == EV_READ ? handlerRead() : handlerWrite());

   U_INTERNAL_DUMP("ret = %d", ret)

   if (ret == U_NOTIFIER_DELETE) UNotifier::handlerDelete(this);
}
#else
# ifdef HAVE_EPOLL_WAIT
int                  UNotifier::epollfd;
struct epoll_event*  UNotifier::events;
struct epoll_event*  UNotifier::pevents;
#   ifdef U_EPOLLET_POSTPONE_STRATEGY
bool                 UNotifier::bepollet;
#  endif
#  ifdef HAVE_EPOLL_CTL_BATCH
int                  UNotifier::ctl_cmd_cnt;
struct epoll_ctl_cmd UNotifier::ctl_cmd[U_EPOLL_CTL_CMD_SIZE];

void UNotifier::batch(UEventFd* item)
{
   U_TRACE(0, "UNotifier::batch(%p)", item)

   U_INTERNAL_ASSERT_POINTER(item)

   U_INTERNAL_DUMP("item->fd = %d item->op_mask = %B num_connection = %d", item->fd, item->op_mask, num_connection)

   U_INTERNAL_ASSERT_EQUALS(ctl_cmd[ctl_cmd_cnt].op, EPOLL_CTL_ADD)
   U_INTERNAL_ASSERT_EQUALS(ctl_cmd[ctl_cmd_cnt].events, EPOLLIN | EPOLLRDHUP | EPOLLET)

   ctl_cmd[ctl_cmd_cnt].fd   = item->fd;
   ctl_cmd[ctl_cmd_cnt].data = item;

   if (++ctl_cmd_cnt >= U_EPOLL_CTL_CMD_SIZE) insertBatch();
}
#  endif
# elif defined(HAVE_KQUEUE)
int            UNotifier::kq;
int            UNotifier::nkqevents;
struct kevent* UNotifier::kqevents;
struct kevent* UNotifier::kqrevents;
# else
int            UNotifier::fd_set_max;
int            UNotifier::fd_read_cnt;
int            UNotifier::fd_write_cnt;
fd_set         UNotifier::fd_set_read;
fd_set         UNotifier::fd_set_write;
# endif
bool           UNotifier::bread;

U_NO_EXPORT void UNotifier::notifyHandlerEvent()
{
   U_TRACE_NO_PARAM(0, "UNotifier::notifyHandlerEvent()")

   U_INTERNAL_ASSERT_POINTER(handler_event)

   U_INTERNAL_DUMP("handler_event = %p bread = %b nfd_ready = %d fd = %d op_mask = %d %B",
                    handler_event, bread, nfd_ready, handler_event->fd, handler_event->op_mask, handler_event->op_mask)

   if ((bread ? handler_event->handlerRead()
              : handler_event->handlerWrite()) == U_NOTIFIER_DELETE)
      {
      handlerDelete(handler_event);
      }
}

void UNotifier::init()
{
   U_TRACE(0, "UNotifier::init()")

#ifdef HAVE_EPOLL_WAIT
   int old = epollfd;

   U_INTERNAL_DUMP("old = %d", old)

# ifdef HAVE_EPOLL_CREATE1
   epollfd = U_SYSCALL(epoll_create1, "%d", EPOLL_CLOEXEC);
   if (epollfd != -1 || errno != ENOSYS) goto next;
# endif
   epollfd = U_SYSCALL(epoll_create, "%u", max_connection);
next:
   U_INTERNAL_ASSERT_DIFFERS(epollfd, -1)

   if (old)
      {
      U_INTERNAL_DUMP("num_connection = %u", num_connection)

      if (num_connection)
         {
         U_INTERNAL_ASSERT_POINTER(lo_map_fd)
         U_INTERNAL_ASSERT_POINTER(hi_map_fd)

         // NB: reinitialized all after fork()...

         for (int fd = 1; fd < (int32_t)lo_map_fd_len; ++fd)
            {
            if ((handler_event = lo_map_fd[fd]))
               {
               U_INTERNAL_DUMP("fd = %d op_mask = %d %B", fd, handler_event->op_mask, handler_event->op_mask)

               (void) U_SYSCALL(epoll_ctl, "%d,%d,%d,%p", old, EPOLL_CTL_DEL, fd, (struct epoll_event*)1);

               struct epoll_event _events = { handler_event->op_mask | EPOLLEXCLUSIVE | EPOLLROUNDROBIN, { handler_event } };

               (void) U_SYSCALL(epoll_ctl, "%d,%d,%d,%p", epollfd, EPOLL_CTL_ADD, fd, &_events);
               }
            }

         if (hi_map_fd->first())
            {
            do {
               handler_event = hi_map_fd->elem();

               U_INTERNAL_DUMP("op_mask = %d %B", handler_event->op_mask, handler_event->op_mask)

               (void) U_SYSCALL(epoll_ctl, "%d,%d,%d,%p", old, EPOLL_CTL_DEL, handler_event->fd, (struct epoll_event*)1);

               struct epoll_event _events = { handler_event->op_mask | EPOLLEXCLUSIVE | EPOLLROUNDROBIN, { handler_event } };

               (void) U_SYSCALL(epoll_ctl, "%d,%d,%d,%p", epollfd, EPOLL_CTL_ADD, handler_event->fd, &_events);
               }
            while (hi_map_fd->next());
            }
         }

      (void) U_SYSCALL(close, "%d", old);

      return;
      }
#elif defined(HAVE_KQUEUE)
   int old = kq;

   U_INTERNAL_DUMP("old = %d", old)

# ifdef HAVE_KQUEUE1
   kq = U_SYSCALL(kqueue1, "%d", O_CLOEXEC);
   if (kq != -1 || errno != ENOSYS) goto next;
# endif
   kq = U_SYSCALL_NO_PARAM(kqueue);
next:
   U_INTERNAL_ASSERT_DIFFERS(kq, -1)

   if (old)
      {
      U_INTERNAL_DUMP("num_connection = %u", num_connection)

      if (num_connection)
         {
         U_INTERNAL_ASSERT_POINTER(kqevents)
         U_INTERNAL_ASSERT_POINTER(kqrevents)
         U_INTERNAL_ASSERT_POINTER(lo_map_fd)
         U_INTERNAL_ASSERT_POINTER(hi_map_fd)

         // NB: reinitialized all after fork()...

         for (int fd = 1; fd < (int32_t)lo_map_fd_len; ++fd)
            {
            if ((handler_event = lo_map_fd[fd]))
               {
               U_INTERNAL_DUMP("fd = %d op_mask = %d %B", fd, handler_event->op_mask, handler_event->op_mask)

               EV_SET(kqevents+nkqevents++, handler_event->fd,
                        ((handler_event->op_mask & (EPOLLIN | EPOLLRDHUP)) != 0) ? EVFILT_READ : EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, (void*)handler_event);
               }
            }

         if (hi_map_fd->first())
            {
            do {
               handler_event = hi_map_fd->elem();

               U_INTERNAL_DUMP("op_mask = %d %B", handler_event->op_mask, handler_event->op_mask)

               EV_SET(kqevents+nkqevents++, handler_event->fd,
                        ((handler_event->op_mask & (EPOLLIN | EPOLLRDHUP)) != 0) ? EVFILT_READ : EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, (void*)handler_event);
               }
            while (hi_map_fd->next());
            }
         }

      (void) U_SYSCALL(close, "%d", old);

      return;
      }

   U_INTERNAL_ASSERT_EQUALS(kqevents, 0)
   U_INTERNAL_ASSERT_EQUALS(kqrevents, 0)

   kqevents  = (struct kevent*) UMemoryPool::_malloc(max_connection, sizeof(struct kevent), true);
   kqrevents = (struct kevent*) UMemoryPool::_malloc(max_connection, sizeof(struct kevent), true);

   // Check for Mac OS X kqueue bug. If kqueue works, then kevent will succeed, and it will stick an error in events[0].
   // If kqueue is broken, then kevent will fail (This detects an old bug in Mac OS X 10.4, fixed in 10.5)

   EV_SET(kqevents, -1, EVFILT_READ, EV_ADD, 0, 0, 0);

   if (U_SYSCALL(kevent, "%d,%p,%d,%p,%d,%p", kq, kqevents, 1, kqrevents, max_connection, 0) != 1 ||
       kqrevents[0].ident != -1                                                                   ||
       (kqrevents[0].flags & EV_ERROR) == 0)
      {
      U_ERROR("Detected broken kevent()...");
      }
#endif

   if (lo_map_fd == 0)
      {
      createMapFd();

#ifdef HAVE_EPOLL_WAIT
      U_INTERNAL_ASSERT_EQUALS(events, 0)

       events =
      pevents = (struct epoll_event*) UMemoryPool::_malloc(max_connection+1, sizeof(struct epoll_event), true);

#  ifdef HAVE_EPOLL_CTL_BATCH
      for (int i = 0; i < U_EPOLL_CTL_CMD_SIZE; ++i)
         {
         ctl_cmd[i].op     = EPOLL_CTL_ADD;
         ctl_cmd[i].events = EPOLLIN | EPOLLRDHUP | EPOLLET;
         }
#  endif
#endif
      }
}

void UNotifier::resume(UEventFd* item)
{
   U_TRACE(0, "UNotifier::resume(%p)", item)

   U_INTERNAL_ASSERT_POINTER(item)
   U_INTERNAL_ASSERT_EQUALS(item->op_mask, EPOLLOUT)

#ifdef HAVE_EPOLL_WAIT
   struct epoll_event _events = { EPOLLOUT, { item } };

   (void) U_SYSCALL(epoll_ctl, "%d,%d,%d,%p", epollfd, EPOLL_CTL_ADD, item->fd, &_events);
#elif defined(HAVE_KQUEUE)
   U_INTERNAL_ASSERT_MAJOR(kq, 0)
   U_INTERNAL_ASSERT_MINOR(nkqevents, max_connection)

   EV_SET(kqevents+nkqevents++, item->fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, (void*)item);
#else
   U_INTERNAL_ASSERT_DIFFERS(item->op_mask & EPOLLOUT, 0)
   U_INTERNAL_ASSERT_EQUALS(FD_ISSET(item->fd, &fd_set_write), 0)

   FD_SET(item->fd, &fd_set_write);

# ifndef _MSWINDOWS_
   U_INTERNAL_DUMP("fd_set_write = %B", __FDS_BITS(&fd_set_write)[0])
# endif

   ++fd_write_cnt;

   U_INTERNAL_ASSERT(fd_write_cnt >= 0)
#endif
}

void UNotifier::suspend(UEventFd* item)
{
   U_TRACE(0, "UNotifier::suspend(%p)", item)

   U_INTERNAL_ASSERT_POINTER(item)
   U_INTERNAL_ASSERT_EQUALS(item->op_mask, EPOLLOUT)

#ifdef HAVE_EPOLL_WAIT
   (void) U_SYSCALL(epoll_ctl, "%d,%d,%d,%p", epollfd, EPOLL_CTL_DEL, item->fd, (struct epoll_event*)1);
#elif defined(HAVE_KQUEUE)
   U_INTERNAL_ASSERT_MAJOR(kq, 0)
   U_INTERNAL_ASSERT_MINOR(nkqevents, max_connection)

   EV_SET(kqevents+nkqevents++, item->fd, EVFILT_WRITE, EV_DELETE | EV_DISABLE, 0, 0, (void*)item);
#else
   U_INTERNAL_ASSERT_DIFFERS(item->op_mask & EPOLLOUT, 0)
   U_INTERNAL_ASSERT(FD_ISSET(item->fd, &fd_set_write))

   FD_CLR(item->fd, &fd_set_write);

# ifndef _MSWINDOWS_
   U_INTERNAL_DUMP("fd_set_write = %B", __FDS_BITS(&fd_set_write)[0])
# endif

   --fd_write_cnt;

   U_INTERNAL_ASSERT(fd_write_cnt >= 0)
#endif
}

int UNotifier::waitForEvent(int fd_max, fd_set* read_set, fd_set* write_set, UEventTime* ptimeout)
{
   U_TRACE(1, "UNotifier::waitForEvent(%d,%p,%p,%p)", fd_max, read_set, write_set, ptimeout)

   int result;

#ifdef HAVE_EPOLL_WAIT
   result = U_SYSCALL(epoll_wait, "%d,%p,%u,%d", epollfd, events, max_connection, UEventTime::getMilliSecond(ptimeout));
#elif defined(HAVE_KQUEUE)
   result = U_SYSCALL(kevent, "%d,%p,%d,%p,%d,%p", kq, kqevents, nkqevents, kqrevents, max_connection, UEventTime::getTimeSpec(ptimeout));
   nkqevents = 0;
#else
   // If both fields of the timeval structure are zero, then select() returns immediately.
   // (This is useful for polling). If ptimeout is NULL (no timeout), select() can block indefinitely...
   //
   // On Linux, the function select modifies timeout to reflect the amount of time not slept; most other implementations do not do this.
   // This causes problems both when Linux code which reads timeout is ported to other operating systems, and when code is ported to Linux
   // that reuses a struct timeval for multiple selects in a loop without reinitializing it. Consider timeout to be undefined after select returns

# if defined(DEBUG) && !defined(_MSWINDOWS_)
   if ( read_set) U_INTERNAL_DUMP(" read_set = %B", __FDS_BITS( read_set)[0])
   if (write_set) U_INTERNAL_DUMP("write_set = %B", __FDS_BITS(write_set)[0])
# endif

   result = U_SYSCALL(select, "%d,%p,%p,%p,%p", fd_max, read_set, write_set, 0, UEventTime::getTimeVal(ptimeout));

# if defined(DEBUG) && !defined(_MSWINDOWS_)
   if ( read_set) U_INTERNAL_DUMP(" read_set = %B", __FDS_BITS( read_set)[0])
   if (write_set) U_INTERNAL_DUMP("write_set = %B", __FDS_BITS(write_set)[0])
# endif
#endif

   U_RETURN(result);
}

void UNotifier::waitForEvent(UEventTime* ptimeout)
{
   U_TRACE(0, "UNotifier::waitForEvent(%p)", ptimeout)

#if !defined(HAVE_EPOLL_WAIT) && !defined(HAVE_KQUEUE)
   fd_set read_set, write_set;
loop:
   U_INTERNAL_ASSERT(fd_read_cnt > 0 || fd_write_cnt > 0)

   if (fd_read_cnt)   read_set = fd_set_read;
   if (fd_write_cnt) write_set = fd_set_write;

   nfd_ready = waitForEvent(fd_set_max,
                (fd_read_cnt  ? &read_set
                              : 0),
                (fd_write_cnt ? &write_set
                              : 0),
                ptimeout);
#elif defined(HAVE_KQUEUE)
loop:
   nfd_ready = U_SYSCALL(kevent, "%d,%p,%d,%p,%d,%p", kq, kqevents, nkqevents, kqrevents, max_connection, UEventTime::getTimeSpec(ptimeout));
# ifdef DEBUG
   if (nfd_ready == -1 &&
       errno == EINVAL)
      {
      U_ERROR("kqueue() return EINVAL...");
      }
# endif
   nkqevents = 0;
#else
loop:
   nfd_ready = U_SYSCALL(epoll_wait, "%d,%p,%u,%d", epollfd, events, max_connection, UEventTime::getMilliSecond(ptimeout));
#endif

   if (nfd_ready > 0)
      {
#  ifdef DEBUG
      if (max_nfd_ready < (uint32_t)nfd_ready) max_nfd_ready = nfd_ready;

      U_INTERNAL_DUMP("max_nfd_ready = %u", max_nfd_ready)
#  endif

      U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

      last_event = u_now->tv_sec;

#  if !defined(HAVE_EPOLL_WAIT) && !defined(HAVE_KQUEUE)
      int fd = 1, fd_cnt = (fd_read_cnt + fd_write_cnt);

      U_INTERNAL_DUMP("fd_cnt = %d fd_set_max = %d", fd_cnt, fd_set_max)

      U_INTERNAL_ASSERT(nfd_ready <= fd_cnt)

      for (int i = nfd_ready; fd < fd_set_max; ++fd)
         {
         bread = (fd_read_cnt && FD_ISSET(fd, &read_set));

         if ((bread                                            ||
              (fd_write_cnt && FD_ISSET(fd, &write_set)) != 0) &&
             setHandler(fd))
            {
            notifyHandlerEvent();

            if (--i == 0)
               {
               U_INTERNAL_DUMP("fd = %d: return", fd)

               if (fd_cnt > (fd_read_cnt + fd_write_cnt)) fd_set_max = getNFDS();

               return;
               }
            }
         }
#  elif defined(HAVE_KQUEUE)
      int i = 0;
      struct kevent* pkqrevents = kqrevents;

loop0:
      U_INTERNAL_ASSERT_POINTER(pkqrevents->udata)

      handler_event = (UEventFd*)pkqrevents->udata;

      U_INTERNAL_DUMP("i = %d handler_event->fd = %d", i, handler_event->fd)

      if (handler_event->fd != -1)
         {
         U_INTERNAL_DUMP("bread = %b bwrite = %b", ((pkqrevents->flags & EVFILT_READ)  != 0), ((pkqrevents->flags & EVFILT_WRITE) != 0))

         if (UNLIKELY((pkqrevents->flags & EV_ERROR)    != 0) ||
              (LIKELY((pkqrevents->flags & EVFILT_READ) != 0) ? handler_event->handlerRead()
                                                              : handler_event->handlerWrite()) == U_NOTIFIER_DELETE)
            {
            handlerDelete(handler_event);
            }
         }

      if (++i < nfd_ready)
         {
         ++pkqrevents;

         goto loop0;
         }
#  else
      int i = 0;
      pevents = events;
#    ifdef U_EPOLLET_POSTPONE_STRATEGY
      bool bloop1 = false;
      bepollet = ((uint32_t)nfd_ready >= bepollet_threshold);

      U_INTERNAL_DUMP("bepollet = %b nfd_ready = %d bepollet_threshold = %u", bepollet, nfd_ready, bepollet_threshold)
#    endif

loop0:
      U_INTERNAL_ASSERT_POINTER(pevents->data.ptr)

      handler_event = (UEventFd*)pevents->data.ptr;

      U_INTERNAL_DUMP("i = %d handler_event->fd = %d", i, handler_event->fd)

      if (handler_event->fd != -1)
         {
         U_INTERNAL_DUMP("bread = %b bwrite = %b events[%d].events = %d %B", ((pevents->events & (EPOLLIN | EPOLLRDHUP)) != 0),
                                                                             ((pevents->events &  EPOLLOUT)              != 0), (pevents -events), pevents->events, pevents->events)

         /**
          * EPOLLIN     = 0x0001
          * EPOLLPRI    = 0x0002
          * EPOLLOUT    = 0x0004
          * EPOLLERR    = 0x0008
          * EPOLLHUP    = 0x0010
          * EPOLLRDNORM = 0x0040
          * EPOLLRDBAND = 0x0080
          * EPOLLWRNORM = 0x0100
          * EPOLLWRBAND = 0x0200
          * EPOLLMSG    = 0x0400
          * EPOLLRDHUP  = 0x2000
          *
          * <10000000 00000100 00000000 00000000>
          *  EPOLLIN  EPOLLRDHUP
          */

         if (UNLIKELY((pevents->events & (EPOLLERR | EPOLLHUP))   != 0) ||
              (LIKELY((pevents->events & (EPOLLIN  | EPOLLRDHUP)) != 0) ? handler_event->handlerRead()
                                                                        : handler_event->handlerWrite()) == U_NOTIFIER_DELETE)
            {
            handlerDelete(handler_event);

#        if defined(U_EPOLLET_POSTPONE_STRATEGY)
            if (bepollet) pevents->events = 0;
#        endif
            }
#       if defined(U_EPOLLET_POSTPONE_STRATEGY)
         else if (bepollet)
            {
            if (U_ClientImage_state != U_PLUGIN_HANDLER_AGAIN &&
                LIKELY((pevents->events & (EPOLLIN | EPOLLRDHUP)) != 0))
               {
               bloop1 = true;
               }
            else
               {
               pevents->events = 0;
               }
            }
#       endif
         }

      if (++i < nfd_ready)
         {
         ++pevents;

         goto loop0;
         }

#    ifdef U_EPOLLET_POSTPONE_STRATEGY
      if (bepollet)
         {
         if (bloop1 == false)
            {
            bepollet_threshold += bepollet_threshold / 2;

            U_INTERNAL_DUMP("bepollet_threshold = %u", bepollet_threshold)
            }
         else
            {
            do {
               i       = 0;
               bloop1  = false;
               pevents = events;

loop2:         if (pevents->events)
                  {
                  handler_event = (UEventFd*)pevents->data.ptr;

                  U_INTERNAL_DUMP("i = %d handler_event->fd = %d ", i, handler_event->fd)

                  U_INTERNAL_ASSERT_DIFFERS(handler_event->fd, -1)

                  if (handler_event->handlerRead() == U_NOTIFIER_DELETE)
                     {
                     handlerDelete(handler_event);

                     pevents->events = 0;
                     }
                  else
                     {
                     if (U_ClientImage_state != U_PLUGIN_HANDLER_AGAIN) bloop1 = true;
                     else                                               pevents->events = 0;
                     }
                  }

               if (++i < nfd_ready)
                  {
                  ++pevents;

                  goto loop2;
                  }
               }
            while (bloop1);
            }
         }
#    endif
#  endif
      }
   else if (nfd_ready == -1)
      {
      U_INTERNAL_DUMP("errno = %d num_connection = %u", errno, num_connection)

      if (errno == EINTR                   &&
          UInterrupt::event_signal_pending &&
          (UInterrupt::callHandlerSignal(), UInterrupt::exit_loop_wait_event_for_signal) == false)
         {
         goto loop;
         }

#  if !defined(HAVE_EPOLL_WAIT) && !defined(HAVE_KQUEUE)
      if (errno == EBADF) // there are fd that become not valid (it is possible if EPIPE)
         {
         removeBadFd();

         if (num_connection) goto loop;
         }
#  endif
      }
}

void UNotifier::waitForEvent()
{
   U_TRACE_NO_PARAM(0, "UNotifier::waitForEvent()")

#ifdef DEBUG
   ++nwatches;
#endif

   UEventTime* ptimeout;

loop:
   U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

   last_event = u_now->tv_sec;

   waitForEvent(ptimeout = UTimer::getTimeout());

   if (nfd_ready == 0 &&
       ptimeout  != 0)
      {
      U_INTERNAL_ASSERT_EQUALS(UTimer::first->alarm, ptimeout)

      U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

      last_event = u_now->tv_sec;

      UTimer::callHandlerTimeout();

      goto loop;
      }
}
#endif

void UNotifier::createMapFd()
{
   U_TRACE_NO_PARAM(0, "UNotifier::createMapFd()")

   U_INTERNAL_ASSERT_EQUALS(lo_map_fd, 0)
   U_INTERNAL_ASSERT_EQUALS(hi_map_fd, 0)
   U_INTERNAL_ASSERT_MAJOR(max_connection, 0)

   lo_map_fd_len = 20+max_connection;
   lo_map_fd     = (UEventFd**) UMemoryPool::_malloc(&lo_map_fd_len, sizeof(UEventFd*), true);

   typedef UGenericHashMap<int,UEventFd*> umapfd;

   U_NEW(umapfd, hi_map_fd, umapfd);

   hi_map_fd->allocate(max_connection);
}

bool UNotifier::isHandler(int fd)
{
   U_TRACE(0, "UNotifier::isHandler(%d)", fd)

   U_INTERNAL_ASSERT_DIFFERS(fd, -1)

   bool result = false;

   U_INTERNAL_DUMP("num_connection = %d min_connection = %d", num_connection, min_connection)

   if (num_connection > min_connection)
      {
      U_INTERNAL_ASSERT_POINTER(lo_map_fd)
      U_INTERNAL_ASSERT_POINTER(hi_map_fd)

      if (fd < (int32_t)lo_map_fd_len) result = (lo_map_fd[fd] != 0);
      else
         {
         lock();

         result = hi_map_fd->find(fd);

         unlock();
         }
      }

   U_RETURN(result);
}

bool UNotifier::setHandler(int fd)
{
   U_TRACE(0, "UNotifier::setHandler(%d)", fd)

   U_INTERNAL_ASSERT_DIFFERS(fd, -1)
   U_INTERNAL_ASSERT_POINTER(lo_map_fd)
   U_INTERNAL_ASSERT_POINTER(hi_map_fd)

   if (fd < (int32_t)lo_map_fd_len)
      {
      if (lo_map_fd[fd])
         {
         handler_event = lo_map_fd[fd];

         U_INTERNAL_DUMP("handler_event = %p", handler_event)

         U_RETURN(true);
         }
      }

   bool result;

   lock();

   result = hi_map_fd->find(fd);

   unlock();

   if (result)
      {
      handler_event = hi_map_fd->elem();

      U_INTERNAL_DUMP("handler_event = %p", handler_event)

      U_RETURN(true);
      }

   U_RETURN(false);
}

void UNotifier::insert(UEventFd* item, int op)
{
   U_TRACE(0, "UNotifier::insert(%p%d)", item, op)

   U_INTERNAL_ASSERT_POINTER(item)

   int fd = item->fd;

   U_INTERNAL_DUMP("fd = %d item->op_mask = %B num_connection = %d", fd, item->op_mask, num_connection)

   U_INTERNAL_ASSERT_DIFFERS(fd, -1)

   if (fd < (int32_t)lo_map_fd_len) lo_map_fd[fd] = item;
   else
      {
      lock();

      hi_map_fd->insert(fd, item);

      unlock();
      }

#ifdef USE_LIBEVENT
   U_INTERNAL_ASSERT_POINTER(u_ev_base)

   int mask = EV_PERSIST | ((item->op_mask & (EPOLLIN | EPOLLRDHUP)) != 0 ? EV_READ : EV_WRITE);

   U_INTERNAL_DUMP("mask = %B", mask)

   U_NEW(UEvent<UEventFd>, item->pevent, UEvent<UEventFd>(fd, mask, *item));

   (void) UDispatcher::add(*(item->pevent));
#elif defined(HAVE_EPOLL_WAIT)
   U_INTERNAL_ASSERT_MAJOR(epollfd, 0)

# ifdef DEBUG
   if (op)
      {
      U_INTERNAL_ASSERT_EQUALS(item->op_mask & EPOLLRDHUP, 0)
      }
# endif

   struct epoll_event _events = { item->op_mask | op, { item } };

   (void) U_SYSCALL(epoll_ctl, "%d,%d,%d,%p", epollfd, EPOLL_CTL_ADD, fd, &_events);
#elif defined(HAVE_KQUEUE)
   U_INTERNAL_ASSERT_MAJOR(kq, 0)
   U_INTERNAL_ASSERT_MINOR(nkqevents, max_connection)

   EV_SET(kqevents+nkqevents++, fd, ((item->op_mask & (EPOLLIN | EPOLLRDHUP)) != 0) ? EVFILT_READ : EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, (void*)item);
#else
   if ((item->op_mask & (EPOLLIN | EPOLLRDHUP)) != 0)
      {
      U_INTERNAL_ASSERT_EQUALS(FD_ISSET(fd, &fd_set_read), 0)
      U_INTERNAL_ASSERT_EQUALS(FD_ISSET(fd, &fd_set_write), 0)

      FD_SET(fd, &fd_set_read);

      U_INTERNAL_ASSERT(fd_read_cnt >= 0)

      ++fd_read_cnt;

#  ifndef _MSWINDOWS_
      U_INTERNAL_DUMP("fd_set_read = %B", __FDS_BITS(&fd_set_read)[0])
#  endif
      }
   else
      {
      U_INTERNAL_ASSERT_DIFFERS(item->op_mask & EPOLLOUT, 0)
      U_INTERNAL_ASSERT_EQUALS(FD_ISSET(fd, &fd_set_write), 0)

      FD_SET(fd, &fd_set_write);

      U_INTERNAL_ASSERT(fd_write_cnt >= 0)

      ++fd_write_cnt;

#  ifndef _MSWINDOWS_
      U_INTERNAL_DUMP("fd_set_write = %B", __FDS_BITS(&fd_set_write)[0])
#  endif
      }

   if (fd_set_max <= fd) fd_set_max = fd + 1;

   U_INTERNAL_DUMP("fd_set_max = %d", fd_set_max)
#endif
}

void UNotifier::modify(UEventFd* item)
{
   U_TRACE(0, "UNotifier::modify(%p)", item)

   U_INTERNAL_ASSERT_POINTER(item)

   int fd = item->fd;

   U_INTERNAL_DUMP("fd = %d op_mask = %B", fd, item->op_mask)

#ifdef USE_LIBEVENT
   U_INTERNAL_ASSERT_POINTER(u_ev_base)

   int mask = EV_PERSIST | ((item->op_mask & (EPOLLIN | EPOLLRDHUP)) != 0 ? EV_READ : EV_WRITE);

   U_INTERNAL_DUMP("mask = %B", mask)

   UDispatcher::del(item->pevent);
             delete item->pevent;

   U_NEW(UEvent<UEventFd>, item->pevent, UEvent<UEventFd>(fd, mask, *item));

   (void) UDispatcher::add(*(item->pevent));
#elif defined(HAVE_EPOLL_WAIT)
   U_INTERNAL_ASSERT_MAJOR(epollfd, 0)

   struct epoll_event _events = { item->op_mask, { item } };

   (void) U_SYSCALL(epoll_ctl, "%d,%d,%d,%p", epollfd, EPOLL_CTL_MOD, fd, &_events);
#elif defined(HAVE_KQUEUE)
   U_INTERNAL_ASSERT_MAJOR(kq, 0)
   U_INTERNAL_ASSERT_MINOR(nkqevents, max_connection)

   int read_flags, write_flags;

   if ((item->op_mask & (EPOLLIN | EPOLLRDHUP)) != 0)
      {
       read_flags = EV_ADD    | EV_ENABLE;
      write_flags = EV_DELETE | EV_DISABLE;
      }
   else
      {
      U_INTERNAL_DUMP("op_mask = %d", item->op_mask)

      U_INTERNAL_ASSERT_DIFFERS(item->op_mask & EPOLLOUT, 0)

       read_flags = EV_DELETE | EV_DISABLE;
      write_flags = EV_ADD    | EV_ENABLE;
      }

   EV_SET(kqevents+nkqevents++, fd, EVFILT_READ,   read_flags, 0, 0, (void*)item);
   EV_SET(kqevents+nkqevents++, fd, EVFILT_WRITE, write_flags, 0, 0, (void*)item);
#else
#  ifndef _MSWINDOWS_
   U_INTERNAL_DUMP("fd_set_read  = %B", __FDS_BITS(&fd_set_read)[0])
   U_INTERNAL_DUMP("fd_set_write = %B", __FDS_BITS(&fd_set_write)[0])
#endif

   if ((item->op_mask & (EPOLLIN | EPOLLRDHUP)) != 0)
      {
      U_INTERNAL_ASSERT(FD_ISSET(fd, &fd_set_write))
      U_INTERNAL_ASSERT_EQUALS(FD_ISSET(fd, &fd_set_read), 0)

      FD_SET(fd, &fd_set_read);
      FD_CLR(fd, &fd_set_write);

      ++fd_read_cnt;
      --fd_write_cnt;
      }
   else
      {
      U_INTERNAL_DUMP("op_mask = %d", item->op_mask)

      U_INTERNAL_ASSERT(FD_ISSET(fd, &fd_set_read))
      U_INTERNAL_ASSERT_DIFFERS(item->op_mask & EPOLLOUT, 0)
      U_INTERNAL_ASSERT_EQUALS(FD_ISSET(fd, &fd_set_write), 0)

      FD_CLR(fd, &fd_set_read);
      FD_SET(fd, &fd_set_write);

      --fd_read_cnt;
      ++fd_write_cnt;
      }

#  ifndef _MSWINDOWS_
   U_INTERNAL_DUMP("fd_set_read  = %B", __FDS_BITS(&fd_set_read)[0])
   U_INTERNAL_DUMP("fd_set_write = %B", __FDS_BITS(&fd_set_write)[0])
#endif

   U_INTERNAL_ASSERT(fd_read_cnt  >= 0)
   U_INTERNAL_ASSERT(fd_write_cnt >= 0)
#endif
}

void UNotifier::handlerDelete(int fd, int mask)
{
   U_TRACE(0, "UNotifier::handlerDelete(%d,%d)", fd, mask)

   U_INTERNAL_ASSERT_MAJOR(fd, 0)

   if (fd < (int32_t)lo_map_fd_len) lo_map_fd[fd] = 0;
   else
      {
      lock();

      (void) hi_map_fd->erase(fd);

      unlock();
      }

#if !defined(USE_LIBEVENT) && !defined(HAVE_EPOLL_WAIT) && !defined(HAVE_KQUEUE)
   if ((mask & (EPOLLIN | EPOLLRDHUP)) != 0)
      {
      U_INTERNAL_ASSERT(FD_ISSET(fd, &fd_set_read))
      U_INTERNAL_ASSERT_EQUALS(FD_ISSET(fd, &fd_set_write), 0)

      FD_CLR(fd, &fd_set_read);

#  ifndef _MSWINDOWS_
      U_INTERNAL_DUMP("fd_set_read = %B", __FDS_BITS(&fd_set_read)[0])
#  endif

      --fd_read_cnt;

      U_INTERNAL_ASSERT(fd_read_cnt >= 0)
      }
   else
      {
      U_INTERNAL_ASSERT_DIFFERS(mask & EPOLLOUT, 0)
      U_INTERNAL_ASSERT(FD_ISSET(fd, &fd_set_write))
      U_INTERNAL_ASSERT_EQUALS(FD_ISSET(fd, &fd_set_read), 0)

      FD_CLR(fd, &fd_set_write);

#  ifndef _MSWINDOWS_
      U_INTERNAL_DUMP("fd_set_write = %B", __FDS_BITS(&fd_set_write)[0])
#  endif

      --fd_write_cnt;

      U_INTERNAL_ASSERT(fd_write_cnt >= 0)
      }

   if (empty() == false) fd_set_max = getNFDS();
#endif
}

void UNotifier::callForAllEntryDynamic(bPFpv function)
{
   U_TRACE(0, "UNotifier::callForAllEntryDynamic(%p)", function)

   U_INTERNAL_DUMP("num_connection = %u", num_connection)

   U_INTERNAL_ASSERT_MAJOR(num_connection, min_connection)

   UEventFd* item;

   for (int fd = 1; fd < (int32_t)lo_map_fd_len; ++fd)
      {
      if ((item = lo_map_fd[fd]))
         {
#     ifdef DEBUG
         if (UNLIKELY((const void*)item <= U_NULL_POINTER))
            {
            U_DEBUG("UNotifier::callForAllEntryDynamic(): lo_map_fd[%d] = %p", fd, item);

            return;
            }
#     endif

         U_INTERNAL_DUMP("fd = %d op_mask = %B", item->fd, item->op_mask)

         if (item->fd != fd ||
             function(item))
            {
            handlerDelete(item);

            U_INTERNAL_DUMP("num_connection = %u", num_connection)

            if (num_connection == min_connection) return;
            }
         }
      }

   UGenericHashMap<int,UEventFd*>::UGenericHashMapNode* _node;

   if ((_node = hi_map_fd->first()))
      {
      do {
         item = hi_map_fd->elem();

         U_INTERNAL_ASSERT_MAJOR(item->fd, 0)

         if (function(item))
            {
            U_DEBUG("UNotifier::callForAllEntryDynamic(): hi_map_fd(%p) = %p %d", _node, item, item->fd);

            handlerDelete(item);

            U_INTERNAL_DUMP("num_connection = %u", num_connection)

            if (num_connection == min_connection) return;
            }
         }
      while ((_node = hi_map_fd->next(_node)));
      }
}

void UNotifier::clear()
{
   U_TRACE_NO_PARAM(0, "UNotifier::clear()")

   U_INTERNAL_DUMP("lo_map_fd = %p", lo_map_fd)

   if (lo_map_fd == 0) return;

   U_INTERNAL_ASSERT_POINTER(hi_map_fd)
   U_INTERNAL_ASSERT_MAJOR(max_connection, 0)

   UEventFd* item;

   U_INTERNAL_DUMP("num_connection = %u", num_connection)

   if (num_connection)
      {
      for (int fd = 1; fd < (int32_t)lo_map_fd_len; ++fd)
         {
         if ((item = lo_map_fd[fd]))
            {
            U_INTERNAL_DUMP("fd = %d op_mask = %B", item->fd, item->op_mask)

            if (item->fd != -1)
               {
               U_INTERNAL_ASSERT_EQUALS(item->fd, fd)

               handlerDelete(item);
               }
            }
         }

      UGenericHashMap<int,UEventFd*>::UGenericHashMapNode* _node;

      if ((_node = hi_map_fd->first()))
         {
         do {
            item = hi_map_fd->elem();

            U_INTERNAL_DUMP("fd = %d op_mask = %B", item->fd, item->op_mask)

            if (item->fd != -1) handlerDelete(item);
            }
         while ((_node = hi_map_fd->next(_node)));
         }
      }

   UMemoryPool::_free(lo_map_fd, lo_map_fd_len, sizeof(UEventFd*));

   hi_map_fd->deallocate();

   delete hi_map_fd;

#ifndef USE_LIBEVENT
# ifdef HAVE_EPOLL_WAIT
   U_INTERNAL_ASSERT_POINTER(events)

   UMemoryPool::_free(events, max_connection + 1, sizeof(struct epoll_event));

   (void) U_SYSCALL(close, "%d", epollfd);
# elif defined(HAVE_KQUEUE)
   U_INTERNAL_ASSERT_MAJOR(kq, 0)
   U_INTERNAL_ASSERT_POINTER(kqevents)
   U_INTERNAL_ASSERT_POINTER(kqrevents)

   UMemoryPool::_free(kqevents,  max_connection, sizeof(struct kevent));
   UMemoryPool::_free(kqrevents, max_connection, sizeof(struct kevent));

   (void) U_SYSCALL(close, "%d", kq);
# endif
#endif
}

#if !defined(USE_LIBEVENT) && !defined(HAVE_EPOLL_WAIT) && !defined(HAVE_KQUEUE)
int UNotifier::getNFDS() // nfds is the highest-numbered file descriptor in any of the three sets, plus 1
{
   U_TRACE_NO_PARAM(0, "UNotifier::getNFDS()")

   int nfds = 0;

   U_INTERNAL_DUMP("fd_set_max = %d", fd_set_max)

   for (int fd = 1; fd < fd_set_max; ++fd)
      {
      if (isHandler(fd))
         {
         U_INTERNAL_DUMP("fd = %d", fd)

         if (nfds < fd) nfds = fd;
         }
      }

   ++nfds;

   U_RETURN(nfds);
}

void UNotifier::removeBadFd()
{
   U_TRACE_NO_PARAM(1, "UNotifier::removeBadFd()")

   int nfd;
   bool bwrite;
   fd_set fdmask;
   fd_set* rmask;
   fd_set* wmask;
   struct timeval polling = { 0L, 0L };

   for (int fd = 1; fd < fd_set_max; ++fd)
      {
      if (setHandler(fd))
         {
         U_INTERNAL_DUMP("fd = %d op_mask = %B handler_event = %p", handler_event->fd, handler_event->op_mask, handler_event)

         bread  = (fd_read_cnt  && ((handler_event->op_mask & (EPOLLIN | EPOLLRDHUP)) != 0));
         bwrite = (fd_write_cnt && ((handler_event->op_mask &  EPOLLOUT)              != 0));

         FD_ZERO(&fdmask);
         FD_SET(fd, &fdmask);

         rmask = (bread  ? &fdmask : 0);
         wmask = (bwrite ? &fdmask : 0);

         U_INTERNAL_DUMP("fdmask = %B", __FDS_BITS(&fdmask)[0])

         nfd = U_SYSCALL(select, "%d,%p,%p,%p,%p", fd+1, rmask, wmask, 0, &polling);

         U_INTERNAL_DUMP("fd = %d op_mask = %B ISSET(read) = %b ISSET(write) = %b", fd, handler_event->op_mask,
                           (rmask ? FD_ISSET(fd, rmask) : false),
                           (wmask ? FD_ISSET(fd, wmask) : false))

         if (nfd)
            {
            if (nfd == -1 &&
                (bread || bwrite))
               {
               handlerDelete(handler_event);
               }
            else
               {
               notifyHandlerEvent();
               }
            }
         }
      }
}
#endif

// param timeoutMS specified the timeout value, in milliseconds.
// A negative value indicates no timeout, i.e. an infinite wait

#ifdef HAVE_POLL_H
int UNotifier::waitForEvent(int timeoutMS)
{
   U_TRACE(1, "UNotifier::waitForEvent(%d)", timeoutMS)

   int ret;

#ifdef DEBUG
   if (timeoutMS > 0) U_INTERNAL_ASSERT(timeoutMS >= 100)
#endif

loop:
   /**
    * The timeout argument specifies the minimum number of milliseconds that poll() will block (This interval will be rounded up
    * to the system clock granularity, and kernel scheduling delays mean that the blocking interval may overrun by a small amount)
    *
    * Specifying a negative value in timeout means an infinite timeout
    * Specifying a timeout of zero causes poll() to return immediately, even if no file descriptors are ready
    */

   ret = U_SYSCALL(poll, "%p,%d,%d", fds, 1, timeoutMS);

   U_INTERNAL_DUMP("errno = %d", errno)

   if (ret >  0) U_RETURN(ret);
   if (ret == 0)
      {
      u_errno = errno == EAGAIN;

      U_RETURN(0);
      }

   if (errno == EINTR)
      {
      UInterrupt::checkForEventSignalPending();

      goto loop;
      }

   U_RETURN(-1);
}
#endif

// param timeoutMS specified the timeout value, in milliseconds.
// a negative value indicates no timeout, i.e. an infinite wait

int UNotifier::waitForRead(int fd, int timeoutMS)
{
   U_TRACE(0, "UNotifier::waitForRead(%d,%d)", fd, timeoutMS)

   U_INTERNAL_ASSERT_RANGE(0,fd,64*1024)

#ifdef DEBUG
   if (timeoutMS > 0) U_INTERNAL_ASSERT(timeoutMS >= 100)
#endif

#ifdef HAVE_POLL_H
   // NB: POLLRDHUP stream socket peer closed connection, or ***** shut down writing half of connection ****

   fds[0].fd     = fd;
   fds[0].events = POLLIN;

   int ret = waitForEvent(timeoutMS);

   // NB: we don't check for POLLERR or POLLHUP because we have problem in same case (command.test)

   U_INTERNAL_DUMP("revents = %d %B", fds[0].revents, fds[0].revents)
#else
# ifdef _MSWINDOWS_
   HANDLE h = is_pipe(fd);

   if (h != INVALID_HANDLE_VALUE)
      {
      DWORD count = 0;

      while (U_SYSCALL(PeekNamedPipe, "%p,%p,%ld,%p,%p,%p", h, 0, 0, 0, &count, 0) &&
             count == 0                                                            &&
             timeoutMS > 0)
         {
         Sleep(1000);

         timeoutMS -= 1000;
         }

      U_RETURN(count);
      }
# endif

   // If both fields of the timeval structure are zero, then select() returns immediately.
   // (This is useful for polling). If ptime is NULL (no timeout), select() can block indefinitely...

   UEventTime* ptime;

   if (timeoutMS < 0) ptime = 0;
   else              (ptime = time_obj)->setTimeToExpireMS(timeoutMS);

   fd_set fdmask;
   FD_ZERO(&fdmask);
   FD_SET(fd, &fdmask);

   int ret = waitForEvent(fd + 1, &fdmask, 0, ptime);
#endif

   U_RETURN(ret);
}

int UNotifier::waitForWrite(int fd, int timeoutMS)
{
   U_TRACE(0, "UNotifier::waitForWrite(%d,%d)", fd, timeoutMS)

   U_INTERNAL_ASSERT_RANGE(0, fd, 64*1024)
   U_INTERNAL_ASSERT_DIFFERS(timeoutMS, 0)

#ifdef DEBUG
   if (timeoutMS != -1) U_INTERNAL_ASSERT(timeoutMS >= 100)
#endif

#ifdef HAVE_POLL_H // NB: POLLRDHUP stream socket peer closed connection, or ***** shut down writing half of connection ****
   fds[0].fd      = fd;
   fds[0].events  = POLLOUT;
   fds[0].revents = 0;

   int ret = waitForEvent(timeoutMS);

   // NB: POLLERR Error condition (output only)
   //     POLLHUP Hang up         (output only)

   U_INTERNAL_DUMP("revents = %d %B", fds[0].revents, fds[0].revents)

   if (ret == 1 &&
       (fds[0].revents & (POLLERR | POLLHUP)) != 0)
      {
      U_INTERNAL_ASSERT_EQUALS(::write(fd,fds,1), -1)

      U_RETURN(-1);
      }
#else
# ifdef _MSWINDOWS_
   if (is_pipe(fd) != INVALID_HANDLE_VALUE) U_RETURN(1);
# endif

   // If both fields of the timeval structure are zero, then select() returns immediately.
   // (This is useful for polling). If ptime is NULL (no timeout), select() can block indefinitely...

   UEventTime* ptime;

   if (timeoutMS < 0) ptime = 0;
   else              (ptime = time_obj)->setTimeToExpireMS(timeoutMS);

   fd_set fdmask;
   FD_ZERO(&fdmask);
   FD_SET(fd, &fdmask);

   int ret = waitForEvent(fd + 1, 0, &fdmask, ptime);
#endif

   U_RETURN(ret);
}

// param timeoutMS specified the timeout value, in milliseconds.
// a negative value indicates no timeout, i.e. an infinite wait

int UNotifier::read(int fd, char* buffer, int count, int timeoutMS)
{
   U_TRACE(1, "UNotifier::read(%d,%p,%d,%d)", fd, buffer, count, timeoutMS)

   if (fd < 0          &&
       timeoutMS != -1 &&
       waitForRead(fd, timeoutMS) != 1)
      {
      U_RETURN(-1);
      }

   int iBytesRead;

loop:
#ifdef _MSWINDOWS_
   (void) U_SYSCALL(ReadFile, "%p,%p,%lu,%p,%p", (HANDLE)_get_osfhandle(fd), buffer, count, (DWORD*)&iBytesRead, 0);
#else
   iBytesRead = U_SYSCALL(read, "%d,%p,%u", fd, buffer, count);
#endif

   if (iBytesRead > 0)
      {
      U_INTERNAL_DUMP("BytesRead(%d) = %#.*S", iBytesRead, iBytesRead, buffer)

      U_RETURN(iBytesRead);
      }

   if (iBytesRead == -1)
      {
      if (errno == EAGAIN &&
         timeoutMS != -1  &&
         waitForRead(fd, timeoutMS) == 1)
         {
         goto loop;
         }

      if (errno == EINTR)
         {
         UInterrupt::checkForEventSignalPending();

         goto loop;
         }
      }

   U_RETURN(-1);
}

// param timeoutMS specified the timeout value, in milliseconds.
// A negative value indicates no timeout, i.e. an infinite wait

uint32_t UNotifier::write(int fd, const char* str, int count, int timeoutMS)
{
   U_TRACE(1, "UNotifier::write(%d,%.*S,%d,%d)", fd, count, str, count, timeoutMS)

   U_INTERNAL_ASSERT_DIFFERS(fd, -1)

   ssize_t value;
   int byte_written = 0;

   do {
      if (timeoutMS >= 0)
         {
         if (fd < 0) U_RETURN(U_NOT_FOUND);

         if (waitForWrite(fd, timeoutMS) <= 0) break;

         timeoutMS = -1; // NB: in this way it is only for the first write...
         }

#  ifdef _MSWINDOWS_
      (void) U_SYSCALL(WriteFile, "%p,%p,%lu,%p,%p", (HANDLE)_get_osfhandle(fd), str + byte_written, count - byte_written, (DWORD*)&value, 0);
#  else
      value = U_SYSCALL(write, "%d,%S,%u", fd, str + byte_written, count - byte_written);
#  endif

      if (value ==  0) break;
      if (value == -1)
         {
         if (errno == EINTR)
            {
            UInterrupt::checkForEventSignalPending();

            continue;
            }

         break;
         }

      U_INTERNAL_DUMP("BytesWritten(%d) = %#.*S", value, value, str + byte_written)

      byte_written += value;
      }
   while (byte_written < count);

   U_RETURN(byte_written);
}

// STREAM

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UNotifier::dump(bool reset) const
{
#ifdef USE_LIBEVENT
// nothing
#elif defined(HAVE_EPOLL_WAIT)
   *UObjectIO::os << "epollfd                     " << epollfd        << '\n';
#elif defined(HAVE_KQUEUE)
   *UObjectIO::os << "kq                          " << kq             << '\n';
#else
   *UObjectIO::os << "fd_set_max                  " << fd_set_max     << '\n'
                  << "fd_read_cnt                 " << fd_read_cnt    << '\n'
                  << "fd_write_cnt                " << fd_write_cnt   << '\n';
   *UObjectIO::os << "fd_set_read                 ";
   char buffer[70];
    UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%B"), __FDS_BITS(&fd_set_read)[0]));
    UObjectIO::os->put('\n');
   *UObjectIO::os << "fd_set_write                ";
    UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%B"), __FDS_BITS(&fd_set_write)[0]));
    UObjectIO::os->put('\n');
#endif
   *UObjectIO::os << "nfd_ready                   " << nfd_ready      << '\n'
                  << "last_event                  " << last_event     << '\n'
                  << "min_connection              " << min_connection << '\n'
                  << "num_connection              " << num_connection << '\n'
                  << "max_connection              " << max_connection;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
