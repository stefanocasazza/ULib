// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    application.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_APPLICATION_H
#define ULIB_APPLICATION_H 1

#include <ulib/log.h>
#include <ulib/options.h>

#ifndef DEBUG
#  define U_MAIN \
int U_EXPORT main(int argc, char* argv[], char* env[]) \
{ \
   ULib::init(argv, U_NULLPTR); \
   Application app; \
   app.run(argc, argv, env); \
   ::exit(UApplication::exit_value); \
}
#else
#  ifndef U_STDCPP_ENABLE
#     define U_PRINT_MEM_USAGE
#  else
#     define U_PRINT_MEM_USAGE UApplication::printMemUsage();
#  endif
#  define U_MAIN \
int U_EXPORT main(int argc, char* argv[], char* env[]) \
{ \
   U_ULIB_INIT(argv); \
   U_TRACE(5, "::main(%d,%p,%p)", argc, argv, env) \
   Application().run(argc, argv, env); \
   U_INTERNAL_ASSERT_EQUALS(ULog::first, U_NULLPTR) \
   U_INTERNAL_ASSERT_EQUALS(USemaphore::first, U_NULLPTR) \
   U_PRINT_MEM_USAGE \
   return UApplication::exit_value; \
}
#endif

/*
#define U_MAIN(_class) \
int WINAPI WinMain (HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR command_line, int cmd_show) \
{ \
   U_ULIB_INIT(__argv); \
   int argc = 0; \
   char** _argv = __argv; \
   while (*_argv++) ++argc; \
   U_TRACE(5, "::main(%d,%p,%p)", argc, __argv, 0) \
   _class app;
   app.run(argc, argv, 0); \
   ::exit(UApplication::exit_value); \
}
*/

/*
#if defined(USE_FSTACK) && !defined(U_SSL_SOCKET) && !defined(U_UDP_SOCKET) && !defined(U_UNIX_SOCKET)
#  include <ff_api.h>
#  include <ff_epoll.h>

extern "C" {
extern U_EXPORT int ff_epoll_create(int size)                                                                          { return epoll_create(size); }
extern U_EXPORT int ff_epoll_ctl(int epfd, int op, int fd, struct epoll_event* event)                                  { return epoll_ctl(epfd, op, fd, event); }
extern U_EXPORT int ff_epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)                    { return epoll_wait(epfd, events, maxevents, timeout); }
extern U_EXPORT int ff_close(int fd)                                                                                   { return close(fd); }
extern U_EXPORT int ff_shutdown(int s, int how)                                                                        { return shutdown(s, how); }
extern U_EXPORT int ff_listen(int s, int backlog)                                                                      { return listen(s, backlog); }
extern U_EXPORT int ff_socket(int domain, int type, int protocol)                                                      { return socket(domain, type, protocol); }
extern U_EXPORT int ff_poll(struct pollfd fds[], nfds_t nfds, int timeout)                                             { return poll(fds, nfds, timeout); }
extern U_EXPORT int ff_accept(int s, struct linux_sockaddr* addr, socklen_t* addrlen)                                  { return accept(s, (sockaddr*)addr, addrlen); }
extern U_EXPORT int ff_bind(int s, const struct linux_sockaddr* addr, socklen_t addrlen)                               { return bind(s, (sockaddr*)addr, addrlen); }
extern U_EXPORT int ff_getpeername(int s, struct linux_sockaddr* name, socklen_t* namelen)                             { return getpeername(s, (sockaddr*)name, namelen); }
extern U_EXPORT int ff_getsockname(int s, struct linux_sockaddr* name, socklen_t* namelen)                             { return getsockname(s, (sockaddr*)name, namelen); }
extern U_EXPORT int ff_connect(int s, const struct linux_sockaddr* name, socklen_t namelen)                            { return connect(s, (sockaddr*)name, namelen); }
extern U_EXPORT int ff_getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen)                      { return getsockopt(s, level, optname, optval, optlen); }
extern U_EXPORT int ff_setsockopt(int s, int level, int optname, const void* optval, socklen_t optlen)                 { return setsockopt(s, level, optname, optval, optlen); }
extern U_EXPORT int ff_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout) { return select(nfds, readfds, writefds, exceptfds, timeout); }

extern U_EXPORT ssize_t ff_recv(int s, void* buf, size_t len, int flags)       { return recv(s, buf, len, flags); }
extern U_EXPORT ssize_t ff_recvmsg(int s, struct msghdr* msg, int flags)       { return recvmsg(s, msg, flags); }
extern U_EXPORT ssize_t ff_write(int fd, const void* buf, size_t nbytes)       { return write(fd, buf, nbytes); }
extern U_EXPORT ssize_t ff_writev(int fd, const struct iovec* iov, int iovcnt) { return writev(fd, iov, iovcnt); }
extern U_EXPORT ssize_t ff_send(int s, const void* buf, size_t len, int flags) { return send(s, buf, len, flags); }

extern U_EXPORT ssize_t ff_recvfrom(int s, void* buf, size_t len, int flags, struct linux_sockaddr* from, socklen_t* fromlen)
{ return recvfrom(s, buf, len, flags, (sockaddr*)from, fromlen); }

extern U_EXPORT ssize_t ff_sendto(int s, const void* buf, size_t len, int flags, const struct linux_sockaddr* to, socklen_t tolen)
{ return sendto(s, buf, len, flags, (sockaddr*)to, tolen); }

extern U_EXPORT int ff_fcntl(int fd, int cmd, void* argp)               { return fcntl(fd, cmd, argp); }
extern U_EXPORT int ff_ioctl(int fd, unsigned long request, void* argp) { return ioctl(fd, request, argp); }
}
#endif
*/

class Application;

class U_EXPORT UApplication {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   static int exit_value;
   static uint32_t num_args;

   UString _str; // NB: must be here to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
   UOptions opt;

   UApplication();

#ifdef U_COVERITY_FALSE_POSITIVE
   virtual ~UApplication();
#else
           ~UApplication();
#endif

   // SERVICES

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(0+256, "UApplication::run(%d,%p,%p)", argc, argv, env)

      U_DUMP_EXEC(argv, env)

      // if exist options, these are processed...

      is_options = (argc > 1);

      if (is_options)
         {
#     ifdef PACKAGE
         if (U_CONSTANT_SIZE(PACKAGE)) opt.package = U_STRING_FROM_CONSTANT(PACKAGE);
#     endif
#     ifdef VERSION
         if (U_CONSTANT_SIZE(VERSION)) opt.version = U_STRING_FROM_CONSTANT(VERSION);
#     else
         if (U_CONSTANT_SIZE(ULIB_VERSION)) opt.version = U_STRING_FROM_CONSTANT(ULIB_VERSION);
#     endif
#     ifdef PURPOSE
         if (U_CONSTANT_SIZE(PURPOSE)) opt.purpose = U_STRING_FROM_CONSTANT(PURPOSE);
#     endif
#     ifdef ARGS
         if (U_CONSTANT_SIZE(ARGS)) opt.args = U_STRING_FROM_CONSTANT(ARGS);
#     endif
#     ifdef REPORT_BUGS
         if (U_CONSTANT_SIZE(REPORT_BUGS)) opt.report_bugs = U_STRING_FROM_CONSTANT(REPORT_BUGS);
#     endif

#     ifdef U_OPTIONS
#        ifndef U_OPTIONS_1
#        define U_OPTIONS_1 ""
#        endif
#        ifndef U_OPTIONS_2
#        define U_OPTIONS_2 ""
#        endif
#        ifndef U_OPTIONS_3
#        define U_OPTIONS_3 ""
#        endif

         const char* options;
         uint32_t len = strlen((options = U_OPTIONS U_OPTIONS_1 U_OPTIONS_2 U_OPTIONS_3));

         if (len)
            {
            (void) _str.assign(options, len);

            opt.load(_str);
            }
#     endif

         num_args = opt.getopt(argc, argv, &optind);
         }

      U_INTERNAL_DUMP("optind = %d argv[optind] = %S", optind, argv[optind])
      }

   static bool isOptions()
      {
      U_TRACE_NO_PARAM(0, "UApplication::isOptions()")

      U_RETURN(is_options);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   static void printMemUsage();

   const char* dump(bool reset) const;
#endif

protected:
   static bool is_options;

   void usage() { opt.printHelp(U_NULLPTR); }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UApplication)

   friend class Application;
};

#endif
