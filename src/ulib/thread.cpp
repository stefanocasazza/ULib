// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    thread.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/thread.h>

#include <time.h>

#ifdef HAVE_SYS_SYSCALL_H
#  include <sys/syscall.h>
#endif

typedef void* (*exec_t)(void*);
typedef void  (*cleanup_t)(void*);

#ifndef HAVE_NANOSLEEP
extern "C" { int nanosleep (const struct timespec* requested_time,
                                  struct timespec* remaining); }
#endif

UThread*        UThread::first;
pthread_cond_t  UThread::cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t UThread::_lock = PTHREAD_MUTEX_INITIALIZER;

class UThreadImpl {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

    UThreadImpl(bool suspendEnable, bool joinEnable)
      {
      U_TRACE(0, "UThreadImpl::UThreadImpl(%b,%b)", suspendEnable, joinEnable)

      _tid           = 0;
      _signal        = 0;
      _cancel        = 0;
      _suspendCount  = 0;
      _suspendEnable = suspendEnable;

      (void) U_SYSCALL(pthread_attr_init,           "%p",    &_attr);
      (void) U_SYSCALL(pthread_attr_setdetachstate, "%p,%d", &_attr, (joinEnable ? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED));
      }

   ~UThreadImpl()
      {
   // U_TRACE(0, "UThreadImpl::~UThreadImpl()") // problem with sanitize address

   // (void) U_SYSCALL(pthread_attr_destroy, "%p", &_attr);
      (void)           pthread_attr_destroy(       &_attr);
      }

#ifdef DEBUG
   const char* dump(bool reset) const;
#endif

private:
   pthread_t _tid;
   pthread_attr_t _attr;
   int _cancel, _signal, _suspendCount;
   bool _suspendEnable;

   // derived class copy constructor creates new instance, so base copy constructor of ThreadImpl should do nothing...

#ifdef U_COMPILER_DELETE_MEMBERS
   UThreadImpl(const UThreadImpl&) = delete;
   UThreadImpl& operator=(const UThreadImpl&) = delete;
#else
   UThreadImpl(const UThreadImpl&)            {}
   UThreadImpl& operator=(const UThreadImpl&) { return *this; }
#endif

   friend class UThread;
};

UThread::UThread(bool suspendEnable, bool joinEnable)
{
   U_TRACE_REGISTER_OBJECT(0, UThread, "%b,%b", suspendEnable, joinEnable)

   priv  = U_NEW(UThreadImpl(suspendEnable, joinEnable));
   next  = first;
   first = this;

   U_INTERNAL_DUMP("first = %p next = %p", first, next)
}

UThread::~UThread()
{
   U_TRACE_UNREGISTER_OBJECT(0, UThread)

   if (priv) stop();

   U_INTERNAL_DUMP("first = %p next = %p", first, next)

   if (next) delete next;
}

pid_t UThread::getTID()
{
   U_TRACE(0, "UThread::getTID()")

   pid_t _tid = syscall(SYS_gettid);

   U_RETURN(_tid);
}

__pure UThread* UThread::getThread()
{
   U_TRACE(1, "UThread::getThread()")

   U_INTERNAL_DUMP("first = %p", first)

   pthread_t _tid = (pthread_t) U_SYSCALL_NO_PARAM(pthread_self);

   for (UThread* obj = first; obj; obj = obj->next)
      {
      if (pthread_equal(_tid, obj->priv->_tid)) U_RETURN_POINTER(obj, UThread);
      }

   U_RETURN_POINTER(0, UThread);
}

void UThread::stop()
{
   U_TRACE(1, "UThread::stop()")

   bool bdetached = isDetached();

   (void) U_SYSCALL(pthread_cancel, "%p", priv->_tid);

   if (bdetached == false)
      {
      (void) U_SYSCALL(pthread_join, "%p,%p", priv->_tid, 0);
      }
#ifdef HAVE_PTHREAD_YIELD
   else
      {
      (void) U_SYSCALL_NO_PARAM(pthread_yield);
      }
#endif
}

void UThread::close()
{
   U_TRACE(1, "UThread::close()")

   if (priv)
      {
      UThread* obj;
      UThread** ptr = &first;

      while ((obj = *ptr))
         {
         U_INTERNAL_ASSERT_POINTER(obj)

         if (pthread_equal(priv->_tid, obj->priv->_tid))
            {
            *ptr = obj->next;
                   obj->next = 0;

            break;
            }

         ptr = &(*ptr)->next;
         }

      delete priv;
             priv = 0;
      }
}

void UThread::threadCleanup(UThread* th)
{
   U_TRACE(0, "UThread::threadCleanup(%p)", th)

   th->close();
}

void UThread::yield()
{
   U_TRACE(1, "UThread::yield()")

   // Yields the current thread's CPU time slice to allow another thread to begin immediate execution

   U_INTERNAL_DUMP("_cancel = %d", priv->_cancel)

#ifndef CCXX_SIG_THREAD_CANCEL
   U_SYSCALL_VOID_NO_PARAM(pthread_testcancel);
#else
   sigset_t cancel, old;
   bool bcancel = (priv->_cancel != cancelDisabled && priv->_cancel != cancelInitial);

   if (bcancel)
      {
#     ifdef sigemptyset
      sigemptyset(&cancel);
#     else
      (void) U_SYSCALL(sigemptyset, "%p", &cancel);
#     endif

#     ifdef sigaddset
      sigaddset(&cancel, CCXX_SIG_THREAD_CANCEL);
#     else
      (void) U_SYSCALL(sigaddset, "%p,%d", &cancel, CCXX_SIG_THREAD_CANCEL);
#     endif

      (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_UNBLOCK, &cancel, &old);
      }
#endif

#ifdef HAVE_PTHREAD_YIELD
   (void) U_SYSCALL_NO_PARAM(pthread_yield);
#endif

#ifdef CCXX_SIG_THREAD_CANCEL
   if (bcancel) (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_SETMASK, &old, 0);
#endif
}

bool UThread::isDetached() const
{
   U_TRACE(1, "UThread::isDetached()")

   U_INTERNAL_DUMP("priv = %p", priv)

   U_INTERNAL_ASSERT_POINTER(priv)

   U_INTERNAL_DUMP("_tid = %p", priv->_tid)

   U_INTERNAL_ASSERT_POINTER(priv->_tid)

   int state;

   (void) U_SYSCALL(pthread_attr_getdetachstate, "%p,%p", &(priv->_attr), &state);

   if (state == PTHREAD_CREATE_DETACHED) U_RETURN(true);

   U_RETURN(false);
}

void UThread::sigInstall(int signo)
{
   U_TRACE(1, "UThread::sigInstall(%d)", signo)

   struct sigaction sa;

#  ifdef sigemptyset
   sigemptyset(&sa.sa_mask);
#  else
   (void) U_SYSCALL(sigemptyset, "%p", &sa.sa_mask);
#  endif

#ifdef SA_RESTART
   sa.sa_flags  = SA_RESTART;
#else
   s.sa_flags   = 0;
#endif

#ifdef SA_INTERRUPT
   sa.sa_flags |= SA_INTERRUPT;
#endif
   sa.sa_handler = sigHandler;

   (void) U_SYSCALL(sigaction, "%d,%p,%p", signo,  &sa, 0);
}

void UThread::sigHandler(int signo)
{
   U_TRACE(1, "UThread::sigHandler(%d)", signo)

   (void) U_SYSCALL(pthread_mutex_lock, "%p", &_lock);

   UThread* th = getThread();

   U_INTERNAL_DUMP("_signal = %d _suspendCount = %d", th->priv->_signal, th->priv->_suspendCount)

   th->priv->_signal = signo;

   if (signo == U_SIGSTOP &&
       th->priv->_suspendCount++ == 0)
      {
      U_INTERNAL_DUMP("SUSPEND: start(%2D)")

      (void) U_SYSCALL(pthread_cond_wait, "%p,%p", &cond, &_lock);

      U_INTERNAL_DUMP("SUSPEND: end(%2D)")
      }
   else if (signo == U_SIGCONT             &&
            th->priv->_suspendCount    > 0 &&
            th->priv->_suspendCount-- == 1)
      {
      (void) U_SYSCALL(pthread_cond_signal, "%p",  &cond);
      }

   (void) U_SYSCALL(pthread_mutex_unlock, "%p", &_lock);
}

void UThread::signal(int signo)
{
   U_TRACE(1, "UThread::signal(%d)", signo)

   // set the _signal variable to the given value. Can only be done if called from the process that the
   // thread is associated with. If called from a different process, the given signal is sent to the process

   pthread_t _tid = (pthread_t) U_SYSCALL_NO_PARAM(pthread_self);

   if (pthread_equal(_tid, priv->_tid)) sigHandler(signo);
   else
      {
      (void) U_SYSCALL(pthread_kill, "%p,%d", priv->_tid, signo);

      yield(); // give the signal a time to kick in
      }
}

/**
 * You can't kill or stop just one thread from another process. You can send a signal to a particular thread,
 * but the stop/abort action that is taken by the signal affects the whole process. In the earlier implementation
 * of Linux threads, it was possible to stop a single thread with SIGSTOP, but this behaviour has now been fixed
 * to conform to the Posix standard (so it stops all threads in the process)
 */

void UThread::suspend()
{
   U_TRACE(1, "UThread::suspend()")

   U_INTERNAL_DUMP("priv = %p", priv)

   U_INTERNAL_ASSERT_POINTER(priv)

   U_INTERNAL_DUMP("_tid = %p", priv->_tid)

   U_INTERNAL_ASSERT_POINTER(priv->_tid)

   if (priv->_suspendEnable) signal(U_SIGSTOP);
}

void UThread::resume()
{
   U_TRACE(1, "UThread::resume()")

   U_INTERNAL_ASSERT_POINTER(priv->_tid)

   if (priv->_suspendEnable) signal(U_SIGCONT);
}

void UThread::execHandler(UThread* th)
{
   U_TRACE(1, "UThread::execHandler(%p)", th)

   th->priv->_tid = (pthread_t) U_SYSCALL_NO_PARAM(pthread_self);

   U_INTERNAL_DUMP("_tid = %p", th->priv->_tid)

   sigset_t mask;

#  ifdef sigemptyset
   sigemptyset(&mask);
#  else
   (void) U_SYSCALL(sigemptyset, "%p", &mask);
#  endif

#  ifdef sigaddset
// sigaddset(&mask, SIGHUP);
   sigaddset(&mask, SIGINT);
   sigaddset(&mask, SIGABRT);
   sigaddset(&mask, SIGPIPE);
   sigaddset(&mask, SIGALRM);
#  else
// (void) U_SYSCALL(sigaddset, "%p,%d", &mask, SIGHUP);
   (void) U_SYSCALL(sigaddset, "%p,%d", &mask, SIGINT);
   (void) U_SYSCALL(sigaddset, "%p,%d", &mask, SIGABRT);
   (void) U_SYSCALL(sigaddset, "%p,%d", &mask, SIGPIPE);
   (void) U_SYSCALL(sigaddset, "%p,%d", &mask, SIGALRM);
#  endif

   (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_BLOCK, &mask, 0);

   if (th->priv->_suspendEnable)
      {
#  ifndef HAVE_PTHREAD_SUSPEND
      // You can't kill or stop just one thread from another process. You can send a signal to a particular thread,
      // but the stop/abort action that is taken by the signal affects the whole process. In the earlier implementation
      // of Linux threads, it was possible to stop a single thread with SIGSTOP, but this behaviour has now been fixed
      // to conform to the Posix standard (so it stops all threads in the process)

#     ifdef sigemptyset
      sigemptyset(&mask);
#     else
      (void) U_SYSCALL(sigemptyset, "%p", &mask);
#     endif

#     ifdef sigaddset
      sigaddset(&mask, U_SIGSTOP);
      sigaddset(&mask, U_SIGCONT);
#     else
      (void) U_SYSCALL(sigaddset, "%p,%d", &mask, U_SIGSTOP);
      (void) U_SYSCALL(sigaddset, "%p,%d", &mask, U_SIGCONT);
#     endif

      (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_UNBLOCK, &mask, 0);

      th->sigInstall(U_SIGSTOP);
      th->sigInstall(U_SIGCONT);
#  endif
      }

   pthread_cleanup_push((cleanup_t)UThread::threadCleanup, th);

   th->setCancel(cancelDeferred);

   th->run();

   pthread_cleanup_pop(0);

   th->close();
}

bool UThread::start(uint32_t timeoutMS)
{
   U_TRACE(1, "UThread::start(%u)", timeoutMS)

   U_INTERNAL_ASSERT_EQUALS(priv->_tid, 0)

#ifdef DEBUG
   if (u_plock == 0)
      {
      static pthread_mutex_t plock = PTHREAD_MUTEX_INITIALIZER;
                  u_plock = &plock;
      }
#endif

   if (U_SYSCALL(pthread_create, "%p,%p,%p,%p", &(priv->_tid), &(priv->_attr), (exec_t)execHandler, this) == 0)
      {
      if (timeoutMS)
         {
         // give at the thread the time to initialize...

         struct timespec ts = { 0L, 1000000L };

         ts.tv_nsec *= (long)timeoutMS;

         (void) U_SYSCALL(nanosleep, "%p,%p", &ts, 0);
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UThread::detach()
{
   U_TRACE(1, "UThread::detach()")

   if (priv->_tid)
      {
      if (U_SYSCALL(pthread_detach, "%p", priv->_tid)) U_RETURN(false);
      }
   else
      {
      if (U_SYSCALL(pthread_attr_setdetachstate, "%p,%d", &(priv->_attr), PTHREAD_CREATE_DETACHED)) U_RETURN(false);
      }

   U_RETURN(true);
}

void UThread::setCancel(int mode)
{
   U_TRACE(1, "UThread::setCancel(%d)", mode)

   int old;

   switch (mode)
      {
      case cancelImmediate:
      case cancelDeferred:
         {
         (void) U_SYSCALL(pthread_setcancelstate, "%d,%p",                           PTHREAD_CANCEL_ENABLE,        &old);
         (void) U_SYSCALL(pthread_setcanceltype,  "%d,%p", (mode == cancelDeferred ? PTHREAD_CANCEL_DEFERRED
                                                                                   : PTHREAD_CANCEL_ASYNCHRONOUS), &old);
         }
      break;

      case cancelInitial:
      case cancelDisabled:
         (void) U_SYSCALL(pthread_setcancelstate, "%d,%p", PTHREAD_CANCEL_DISABLE, &old);
      break;
      }

   priv->_cancel = mode;

   U_INTERNAL_DUMP("_cancel = %d", priv->_cancel)
}

int UThread::enterCancel()
{
   U_TRACE(1, "UThread::enterCancel()")

   U_INTERNAL_DUMP("_cancel = %d", priv->_cancel)

    int old = priv->_cancel;

    if (old != cancelDisabled &&
        old != cancelImmediate)
       {
       setCancel(cancelImmediate);

       U_SYSCALL_VOID_NO_PARAM(pthread_testcancel);
       }

    U_RETURN(old);
}

void UThread::exitCancel(int old)
{
   U_TRACE(1, "UThread::exitCancel(%d)", old)

   U_INTERNAL_DUMP("_cancel = %d", priv->_cancel)

   if (old != priv->_cancel)
      {
      U_SYSCALL_VOID_NO_PARAM(pthread_testcancel);

      setCancel(old);
      }
}

void UThread::sleep(time_t timeoutMS)
{
   U_TRACE(1+256, "UThread::sleep(%ld)", timeoutMS)

#ifdef CCXX_SIG_THREAD_CANCEL
   UThread* th = getThread();

   int old = th->enterCancel();
#endif
   struct timespec ts = {  timeoutMS / 1000L, (timeoutMS % 1000L) * 1000000L };

   U_INTERNAL_ASSERT(ts.tv_sec >= 0L)
   U_INTERNAL_ASSERT_RANGE(0L, ts.tv_nsec, 999999999L)

#ifdef  HAVE_PTHREAD_DELAY
    pthread_delay(&ts);
#else
   U_INTERNAL_DUMP("Call   nanosleep(%2D)")

   (void) U_SYSCALL(nanosleep, "%p,%p", &ts, 0);

   U_INTERNAL_DUMP("Return nanosleep(%2D)")
#endif

#ifdef CCXX_SIG_THREAD_CANCEL
   th->exitCancel(old);
#endif
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UThreadImpl::dump(bool reset) const
{
   *UObjectIO::os << "_tid           " << _tid           << '\n'
                  << "_cancel        " << _cancel        << '\n'
                  << "_signal        " << _signal        << '\n'
                  << "_suspendCount  " << _suspendCount  << '\n'
                  << "_suspendEnable " << _suspendEnable;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UThread::dump(bool reset) const
{
   *UObjectIO::os << "next  (UThread " << (void*)next  << ")\n"
                  << "first (UThread " << (void*)first << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
