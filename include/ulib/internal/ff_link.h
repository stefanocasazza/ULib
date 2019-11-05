// =======================================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    ff_link.h - external function to link if defined USE_FSTACK
//
// = AUTHOR
//    Stefano Casazza
//
// =======================================================================================

#ifdef USE_FSTACK
#  include <ff_api.h>
#  include <ff_epoll.h>

extern "C" {
extern U_EXPORT int ff_epoll_create(int size)                                                                          { return 0; /* epoll_create(size); */ }
extern U_EXPORT int ff_epoll_ctl(int epfd, int op, int fd, struct epoll_event* event)                                  { return 0; /* epoll_ctl(epfd, op, fd, event); */ }
extern U_EXPORT int ff_epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)                    { return 0; /* epoll_wait(epfd, events, maxevents, timeout); */ }
extern U_EXPORT int ff_close(int fd)                                                                                   { return 0; /* close(fd); */ }
extern U_EXPORT int ff_shutdown(int s, int how)                                                                        { return 0; /* shutdown(s, how); */ }
extern U_EXPORT int ff_listen(int s, int backlog)                                                                      { return 0; /* listen(s, backlog); */ }
extern U_EXPORT int ff_socket(int domain, int type, int protocol)                                                      { return 0; /* socket(domain, type, protocol); */ }
extern U_EXPORT int ff_poll(struct pollfd fds[], nfds_t nfds, int timeout)                                             { return 0; /* poll(fds, nfds, timeout); */ }
extern U_EXPORT int ff_accept(int s, struct linux_sockaddr* addr, socklen_t* addrlen)                                  { return 0; /* accept(s, (sockaddr*)addr, addrlen); */ }
extern U_EXPORT int ff_bind(int s, const struct linux_sockaddr* addr, socklen_t addrlen)                               { return 0; /* bind(s, (sockaddr*)addr, addrlen); */ }
extern U_EXPORT int ff_getpeername(int s, struct linux_sockaddr* name, socklen_t* namelen)                             { return 0; /* getpeername(s, (sockaddr*)name, namelen); */ }
extern U_EXPORT int ff_getsockname(int s, struct linux_sockaddr* name, socklen_t* namelen)                             { return 0; /* getsockname(s, (sockaddr*)name, namelen); */ }
extern U_EXPORT int ff_connect(int s, const struct linux_sockaddr* name, socklen_t namelen)                            { return 0; /* connect(s, (sockaddr*)name, namelen); */ }
extern U_EXPORT int ff_getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen)                      { return 0; /* getsockopt(s, level, optname, optval, optlen); */ }
extern U_EXPORT int ff_setsockopt(int s, int level, int optname, const void* optval, socklen_t optlen)                 { return 0; /* setsockopt(s, level, optname, optval, optlen); */ }
extern U_EXPORT int ff_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout) { return 0; /* select(nfds, readfds, writefds, exceptfds, timeout); */ }

extern U_EXPORT ssize_t ff_recv(int s, void* buf, size_t len, int flags)       { return 0; /* recv(s, buf, len, flags); */ }
extern U_EXPORT ssize_t ff_recvmsg(int s, struct msghdr* msg, int flags)       { return 0; /* recvmsg(s, msg, flags); */ }
extern U_EXPORT ssize_t ff_write(int fd, const void* buf, size_t nbytes)       { return 0; /* write(fd, buf, nbytes); */ }
extern U_EXPORT ssize_t ff_writev(int fd, const struct iovec* iov, int iovcnt) { return 0; /* writev(fd, iov, iovcnt); */ }
extern U_EXPORT ssize_t ff_send(int s, const void* buf, size_t len, int flags) { return 0; /* send(s, buf, len, flags); */ }

extern U_EXPORT ssize_t ff_recvfrom(int s, void* buf, size_t len, int flags, struct linux_sockaddr* from, socklen_t* fromlen)
{ return 0; /* recvfrom(s, buf, len, flags, (sockaddr*)from, fromlen); */ }

extern U_EXPORT ssize_t ff_sendto(int s, const void* buf, size_t len, int flags, const struct linux_sockaddr* to, socklen_t tolen)
{ return 0; /* sendto(s, buf, len, flags, (sockaddr*)to, tolen); */ }

extern U_EXPORT int ff_fcntl(int fd, int cmd, ...)               { return 0; /* fcntl(fd, cmd, argp); */ }
extern U_EXPORT int ff_ioctl(int fd, unsigned long request, ...) { return 0; /* ioctl(fd, request, argp); */ }
}
#endif
