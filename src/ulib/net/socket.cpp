// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    socket.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>
#include <ulib/timeval.h>
#include <ulib/notifier.h>
#include <ulib/internal/chttp.h>
#include <ulib/utility/interrupt.h>
#include <ulib/utility/string_ext.h>

#ifdef _MSWINDOWS_
#  include <ulib/net/socket.h>
#  include <ws2tcpip.h>
#else
#  include <ulib/net/unixsocket.h>
#  ifndef __clang__
U_DUMP_KERNEL_VERSION(LINUX_VERSION_CODE)
#  endif
#endif

#ifdef USE_LIBSSL
#  include <ulib/ssl/net/sslsocket.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
#endif
#ifdef HAVE_ARPA_INET_H
#  include <net/if_arp.h>
#endif

int            USocket::incoming_cpu = -1;
int            USocket::iBackLog = SOMAXCONN;
int            USocket::accept4_flags;  // If flags is 0, then accept4() is the same as accept()
bool           USocket::tcp_reuseport;
bool           USocket::bincoming_cpu;
SocketAddress* USocket::cLocal;

#include "socket_address.cpp"

USocket::USocket(bool bSocketIsIPv6)
{
   U_TRACE_REGISTER_OBJECT(0, USocket, "%b", bSocketIsIPv6)

   flags      = O_RDWR;
   iState     = CLOSE;
   iSockDesc  = -1;
   iLocalPort = iRemotePort = 0;

#ifdef ENABLE_IPV6
   U_socket_IPv6(this) = bSocketIsIPv6;
#else
   U_socket_IPv6(this) = false;
#endif

   U_socket_Type(this)     = 0;
   U_socket_LocalSet(this) = false;

#ifdef _MSWINDOWS_
   fh = -1;
#endif
}

USocket::~USocket()
{
   U_TRACE_UNREGISTER_OBJECT(0, USocket)

   if (isOpen()) _closesocket();
}

__pure unsigned int USocket::localPortNumber()
{
   U_TRACE_NO_PARAM(0, "USocket::localPortNumber()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isLocalSet())

   U_RETURN(iLocalPort);
}

__pure UIPAddress& USocket::localIPAddress()
{
   U_TRACE_NO_PARAM(0, "USocket::localIPAddress()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isLocalSet())

   return cLocalAddress;
}

void USocket::_socket(int iSocketType, int domain, int protocol)
{
   U_TRACE(1, "USocket::_socket(%d,%d,%d)", iSocketType, domain, protocol)

   U_INTERNAL_ASSERT(isClosed())

   if (domain == 0)
      {
       domain = ((U_socket_Type(this) & SK_UNIX) != 0 ? AF_UNIX  :
                  U_socket_IPv6(this)                 ? AF_INET6 : AF_INET);
      }
   else if (domain == AF_UNIX) U_socket_Type(this) |= SK_UNIX; // AF_UNIX == 1

   if (iSocketType == 0)
      {
      iSocketType = ((U_socket_Type(this) & SK_DGRAM) != 0 ? SOCK_DGRAM : SOCK_STREAM);
      }
   else if (iSocketType == SOCK_RAW)
      {
      U_socket_Type(this) |= SK_RAW;
      }
   else if (iSocketType == SOCK_DGRAM)
      {
      U_socket_Type(this) |= SK_DGRAM;
      }

   U_INTERNAL_DUMP("U_socket_Type = %d %B", U_socket_Type(this), U_socket_Type(this))

#ifdef _MSWINDOWS_
   fh        = U_SYSCALL(socket, "%d,%d,%d", domain, iSocketType, protocol);
   iSockDesc = _open_osfhandle((long)fh, O_RDWR | O_BINARY);
#else
   iSockDesc = U_SYSCALL(socket, "%d,%d,%d", domain, iSocketType, protocol);
#endif

   if (isOpen())
      {
      flags       = O_RDWR;
      iRemotePort = 0;

      U_socket_LocalSet(this) = false;
      }
}

bool USocket::connectServer(const UIPAddress& cAddr, unsigned int iServPort)
{
   U_TRACE(1, "USocket::connectServer(%p,%d)", &cAddr, iServPort)

   U_CHECK_MEMORY

   if (isOpen() == false) _socket();

   if ((iRemotePort = iServPort, cRemoteAddress = cAddr, connect())) U_RETURN(true);

   U_RETURN(false);
}

bool USocket::checkErrno()
{
   U_TRACE_NO_PARAM(0, "USocket::checkErrno()")

   U_INTERNAL_DUMP("errno = %d", errno)

   if (errno == EAGAIN)
      {
      iState |= TIMEOUT;

      U_RETURN(true);
      }

   if (errno != ECONNRESET) iState = BROKEN;

   closesocket();

   U_INTERNAL_DUMP("state = %d", iState)

   U_RETURN(false);
}

bool USocket::checkTime(long time_limit, long& timeout)
{
   U_TRACE(1, "USocket::checkTime(%ld,%ld)", time_limit, timeout)

   U_INTERNAL_ASSERT_RANGE(1,time_limit,8L*60L) // 8 minuts

   U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

   if (timeout == 0) timeout = u_now->tv_sec + time_limit;

   if (u_now->tv_sec > timeout)
      {
      iState = BROKEN | TIMEOUT;

      closesocket();

      U_RETURN(false);
      }

   U_RETURN(true);
}

void USocket::setLocal()
{
   U_TRACE_NO_PARAM(1, "USocket::setLocal()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isOpen())

   SocketAddress tmp;

   socklen_t slDummy = tmp.sizeOf();

   if (U_SYSCALL(getsockname, "%d,%p,%p", getFd(), (sockaddr*)tmp, &slDummy) == 0)
      {
      iLocalPort = tmp.getPortNumber();
                   tmp.getIPAddress(cLocalAddress);

      U_socket_LocalSet(this) = true;
      }
}

void USocket::setLocal(const UIPAddress& addr)
{
   U_TRACE(0, "USocket::setLocal(%p)", &addr)

   cLocalAddress = addr;

   if (cLocal) cLocal->setIPAddress(cLocalAddress);

   U_socket_LocalSet(this) = true;
}

UString USocket::getMacAddress(const char* device)
{
   U_TRACE(1, "USocket::getMacAddress(%S)", device)

   U_INTERNAL_ASSERT_POINTER(device)

   UString result(100U);

#if defined(U_LINUX) && defined(HAVE_SYS_IOCTL_H) && defined(HAVE_ARPA_INET_H)
   U_INTERNAL_ASSERT(isOpen())

   /**
    * ARP ioctl request
    *
    * struct arpreq {
    *    struct sockaddr arp_pa;       // Protocol address
    *    struct sockaddr arp_ha;       // Hardware address
    *    int arp_flags;                // Flags
    *    struct sockaddr arp_netmask;  // Netmask (only for proxy arps)
    *    char arp_dev[16];
    * };
    */

   struct arpreq arpreq;

   (void) U_SYSCALL(memset, "%p,%d,%u", &arpreq, 0, sizeof(arpreq));

   union uupsockaddr {
      struct sockaddr*    p;
      struct sockaddr_in* psin;
   };

   union uupsockaddr u = { &arpreq.arp_pa };

   // arp_pa must be an AF_INET address
   // arp_ha must have the same type as the device which is specified in arp_dev
   // arp_dev is a zero-terminated string which names a device

   u.psin->sin_family      = AF_INET;
   u.psin->sin_addr.s_addr = cRemoteAddress.getInAddr();
   arpreq.arp_ha.sa_family = AF_INET;

   (void) u__strncpy(arpreq.arp_dev, device, 15);

   if (U_SYSCALL(ioctl, "%d,%d,%p", iSockDesc, SIOCGARP, &arpreq) == 0)
      {
      if ((arpreq.arp_flags & ATF_COM) != 0)
         {
         unsigned char* hwaddr = (unsigned char*)arpreq.arp_ha.sa_data;

         result.snprintf(U_CONSTANT_TO_PARAM("%02x:%02x:%02x:%02x:%02x:%02x"),
            hwaddr[0] & 0xFF,
            hwaddr[1] & 0xFF,
            hwaddr[2] & 0xFF,
            hwaddr[3] & 0xFF,
            hwaddr[4] & 0xFF,
            hwaddr[5] & 0xFF);

         /**
          * if (arpreq.arp_flags & ATF_PERM)        printf("PERM");
          * if (arpreq.arp_flags & ATF_PUBL)        printf("PUBLISHED");
          * if (arpreq.arp_flags & ATF_USETRAILERS) printf("TRAILERS");
          * if (arpreq.arp_flags & ATF_PROXY)       printf("PROXY");
          */
         }
      // else printf("*** INCOMPLETE ***");
      }
#endif

   U_RETURN_STRING(result);
}

/**
 * The method is called with a local IP address and port number to bind the socket to.
 * A default port number of zero is a wildcard and lets the OS choose the port number
 */

bool USocket::bind()
{
   U_TRACE_NO_PARAM(1, "USocket::bind()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isOpen())
   U_INTERNAL_ASSERT_POINTER(cLocal)
   U_INTERNAL_ASSERT_MAJOR(iLocalPort, 0)

   int result, counter = 0;

loop:
   result = U_SYSCALL(bind, "%d,%p,%d", getFd(), (sockaddr*)&(cLocal->addr.psaGeneric), cLocal->sizeOf());

   if (result == -1         &&
       errno  == EADDRINUSE &&
       ++counter <= 3)
      {
      UTimeVal::nanosleep(1L);

      goto loop;
      }

   if (result == 0) U_RETURN(true);

   if (errno == EADDRINUSE) U_WARNING("Probably another instance of userver is running on the same port: %u", iLocalPort);

   U_RETURN(false);
}

void USocket::setReusePort()
{
   U_TRACE_NO_PARAM(0, "USocket::setReusePort()")

   /**
    * As with TCP, SO_REUSEPORT allows multiple UDP sockets to be bound to the same port. This
    * facility could, for example, be useful in a DNS server operating over UDP. With SO_REUSEPORT,
    * each thread could use recv() on its own socket to accept datagrams arriving on the port.
    * The traditional approach is that all threads would compete to perform recv() calls on a
    * single shared socket. This can lead to unbalanced loads across the threads. By contrast,
    * SO_REUSEPORT distributes datagrams evenly across all of the receiving threads
    */

#if !defined(U_SERVER_CAPTIVE_PORTAL) && defined(U_LINUX) // && LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
# ifndef SO_REUSEPORT
# define SO_REUSEPORT 15
# endif
   U_INTERNAL_DUMP("U_socket_Type = %d %B", U_socket_Type(this), U_socket_Type(this))

   tcp_reuseport = ((U_socket_Type(this) & SK_UNIX) == 0                     &&
                    setSockOpt(SOL_SOCKET, SO_REUSEPORT, (const int[]){ 1 }) &&
                    (U_socket_Type(this) & SK_DGRAM) == 0);

   U_INTERNAL_DUMP("tcp_reuseport = %b", tcp_reuseport)
#endif
}

void USocket::setReuseAddress()
{
   U_TRACE_NO_PARAM(0, "USocket::setReuseAddress()")

   /**
    * SO_REUSEADDR allows your server to bind to an address which is in a TIME_WAIT state.
    * It does not allow more than one server to bind to the same address. It was mentioned
    * that use of this flag can create a security risk because another server can bind to a
    * the same port, by binding to a specific address as opposed to INADDR_ANY
    */

   (void) setSockOpt(SOL_SOCKET, SO_REUSEADDR, (const int[]){ 1 }); 
}

void USocket::setTcpKeepAlive()
{
   U_TRACE_NO_PARAM(0, "USocket::setTcpKeepAlive()")

   // Set TCP keep alive option to detect dead peers

#ifdef SO_KEEPALIVE
   (void) setSockOpt(SOL_SOCKET, SO_KEEPALIVE, (const int[]){ 1 });

# ifdef U_LINUX // Default settings are more or less garbage, with the keepalive time set to 7200 by default on Linux. Modify settings to make the feature actually useful
   (void) setSockOpt(IPPROTO_TCP, TCP_KEEPIDLE, (const int[]){ 15 }); // Send first probe after interval

   // Send next probes after the specified interval. Note that we set the delay as interval / 3, as we send three probes before detecting an error (see the next setsockopt call)

   (void) setSockOpt(IPPROTO_TCP, TCP_KEEPINTVL, (const int[]){ 15 / 3 }); // Send next probe after interval

   // Consider the socket in error state after three we send three ACK probes without getting a reply

   (void) setSockOpt(IPPROTO_TCP, TCP_KEEPCNT, (const int[]){ 3 });
# endif
#endif
}

void USocket::setAddress(void* address)
{
   U_TRACE(0, "USocket::setAddress(%p)", address)

   U_INTERNAL_ASSERT_POINTER(cLocal)

   cLocalAddress.setAddress(address, (bool)U_socket_IPv6(this));

   cLocal->setIPAddress(cLocalAddress);

   U_socket_LocalSet(this) = true;
}

bool USocket::setHostName(const UString& pcNewHostName)
{
   U_TRACE(0, "USocket::setHostName(%V)", pcNewHostName.rep)

   U_INTERNAL_ASSERT_POINTER(cLocal)

   if (cLocalAddress.setHostName(pcNewHostName, U_socket_IPv6(this)))
      {
      cLocal->setIPAddress(cLocalAddress);

      U_socket_LocalSet(this) = true;

      U_RETURN(true);
      }

   U_RETURN(false);
}

// We try to bind the USocket to the specified port number and any local IP Address using the bind() method

bool USocket::setServer(unsigned int port, UString* localAddress)
{
   U_TRACE(0, "USocket::setServer(%u,%p)", port, localAddress)

   U_CHECK_MEMORY

   if (isOpen() == false) _socket();

#ifndef _MSWINDOWS_
   setReusePort();
   setReuseAddress();

   if (isIPC())
      {
      /**
       * A Unix domain "server" is created as a Unix domain socket that is bound
       * to a pathname and that has a backlog queue to listen for connection requests
       */

      U_INTERNAL_ASSERT_POINTER(UUnixSocket::path)

      (void) UFile::_unlink(UUnixSocket::path);

      if (U_SYSCALL(bind, "%d,%p,%d", iSockDesc, &(UUnixSocket::addr.psaGeneric), UUnixSocket::len) == 0 &&
          U_SYSCALL(listen, "%d,%d",  iSockDesc,                                          iBackLog) == 0)
         {
         iLocalPort = iRemotePort = port;

         U_socket_LocalSet(this) = true;

         U_RETURN(true);
         }

      U_RETURN(false);
      }
#endif

   U_INTERNAL_DUMP("cLocal = %p", cLocal)

   U_INTERNAL_ASSERT_EQUALS(cLocal, 0)

   cLocal = new SocketAddress;

   if (localAddress)
      {
      if (setHostName(*localAddress) == false) U_RETURN(false);
      }
   else
      {
      cLocal->setIPAddressWildCard(U_socket_IPv6(this));
      }

   cLocal->setPortNumber(iLocalPort = port);

   /**
    * The normal TCP termination sequence looks like this (simplified). We have two peers: A and B
    *
    * 1. A calls close()
    *       A sends FIN to B
    *       A goes into FIN_WAIT_1 state
    * 2. B receives FIN
    *       B sends ACK to A
    *       B goes into CLOSE_WAIT state
    * 3. A receives ACK
    *       A goes into FIN_WAIT_2 state
    * 4. B calls close()
    *       B sends FIN to A
    *       B goes into LAST_ACK state
    * 5. A receives FIN
    *       A sends ACK to B
    *       A goes into TIME_WAIT state
    * 6. B receives ACK
    *       B goes to CLOSED state - i.e. is removed from the socket tables
    *
    * So the peer that initiates the termination - i.e. calls close() first - will end up in the TIME_WAIT state.
    * It can be a problem with lots of sockets in TIME_WAIT state on a server as it could eventually prevent new
    * connections from being accepted. Setting SO_LINGER with timeout 0 prior to calling close() will cause the
    * normal termination sequence not to be initiated. Instead, the peer setting this option and calling close()
    * will send a RST (connection reset) which indicates an error condition and this is how it will be perceived
    * at the other end. You will typically see errors like "Connection reset by peer".
    *
    * When linger is off the TCP stack doesn't wait for pending data to be sent before closing the connection. Data
    * could be lost due to this but by setting linger to off you're accepting this and asking that the connection be
    * reset straight away rather than closed gracefully. This causes an RST to be sent rather than the usual FIN
    *
    * SO_KEEPALIVE makes the kernel more aggressive about continually verifying the connection even when you're not doing anything,
    * but does not change or enhance the way the information is delivered to you. You'll find out when you try to actually do something
    * (for example "write"), and you'll find out right away since the kernel is now just reporting the status of a previously set flag,
    * rather than having to wait a few seconds (or much longer in some cases) for network activity to fail. The exact same code logic you
    * had for handling the "other side went away unexpectedly" condition will still be used; what changes is the timing (not the method).
    *
    * Virtually every "practical" sockets program in some way provides non-blocking access to the sockets during the data phase (maybe with
    * select()/poll(), or maybe with fcntl()/O_NONBLOCK/EINPROGRESS/EWOULDBLOCK, or if your kernel supports it maybe with MSG_DONTWAIT).
    * Assuming this is already done for other reasons, it's trivial (sometimes requiring no code at all) to in addition find out right away
    * about a connection dropping. But if the data phase does not already somehow provide non-blocking access to the sockets, you won't find
    * out about the connection dropping until the next time you try to do something.
    *
    * A TCP socket connection without some sort of non-blocking behaviour during the data phase is notoriously fragile, as if the wrong packet
    * encounters a network problem it's very easy for the program to then "hang" indefinitely, and there's not a whole lot you can do about it
    */

   U_INTERNAL_DUMP("tcp_reuseport = %b", tcp_reuseport)

   if (tcp_reuseport ||
       (bind()       &&
        U_SYSCALL(listen, "%d,%d", iSockDesc, iBackLog) == 0))
      {
      U_RETURN(true);
      }

   iState = -errno;

   U_RETURN(false);
}

void USocket::reusePort(int _flags)
{
   U_TRACE(1, "USocket::reusePort(%d)", _flags)

   U_CHECK_MEMORY

#if !defined(U_SERVER_CAPTIVE_PORTAL) && defined(U_LINUX)
   U_INTERNAL_DUMP("tcp_reuseport = %b", tcp_reuseport)

   if (tcp_reuseport)
      {
      U_INTERNAL_DUMP("U_socket_Type = %d %B", U_socket_Type(this), U_socket_Type(this))

      U_ASSERT_EQUALS(isUDP(), false)
      U_ASSERT_EQUALS(isIPC(), false)

      int old         = iSockDesc,
          domain      = ((U_socket_Type(this) & SK_UNIX)  != 0 ? AF_UNIX  :
                          U_socket_IPv6(this)                  ? AF_INET6 : AF_INET),
          iSocketType = ((U_socket_Type(this) & SK_DGRAM) != 0 ? SOCK_DGRAM : SOCK_STREAM);

#  ifndef U_COVERITY_FALSE_POSITIVE // NEGATIVE_RETURNS
      // coverity[+alloc]
      iSockDesc = U_SYSCALL(socket, "%d,%d,%d", domain, iSocketType, 0);
#  endif

      U_INTERNAL_DUMP("iLocalPort = %u cLocal->getPortNumber() = %u", iLocalPort, cLocal->getPortNumber())

      U_INTERNAL_ASSERT_MAJOR( iLocalPort, 0)
      U_INTERNAL_ASSERT_EQUALS(iLocalPort, cLocal->getPortNumber())

      if (isClosed()                                                           ||
          (setReuseAddress(), setReusePort(),
           cLocal->setIPAddressWildCard(U_socket_IPv6(this)), bind()) == false ||
          U_SYSCALL(listen, "%d,%d", iSockDesc, iBackLog) != 0)
         {
         U_ERROR("SO_REUSEPORT failed, port %d", iLocalPort);
         }

#  if defined(SO_INCOMING_CPU) && !defined(U_COVERITY_FALSE_POSITIVE)
      if (incoming_cpu != -1) bincoming_cpu = setSockOpt(SOL_SOCKET, SO_INCOMING_CPU, (void*)&incoming_cpu);
#  endif

      (void) U_SYSCALL(close, "%d", old);
      }
#endif

#ifndef U_COVERITY_FALSE_POSITIVE // USE_AFTER_FREE
   setFlags(_flags);
#endif
}

void USocket::setRemote()
{
   U_TRACE_NO_PARAM(1, "USocket::setRemote()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isOpen())

   SocketAddress cRemote;

   socklen_t slDummy = cRemote.sizeOf();

   if (U_SYSCALL(getpeername, "%d,%p,%p", getFd(), (sockaddr*)cRemote, &slDummy) == 0)
      {
      iRemotePort = cRemote.getPortNumber();
                    cRemote.getIPAddress(cRemoteAddress);
      }
}

bool USocket::connect()
{
   U_TRACE_NO_PARAM(1, "USocket::connect()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isOpen())

   int result;
   SocketAddress cServer;

   cServer.setPortNumber(iRemotePort);
   cServer.setIPAddress(cRemoteAddress);

   setTcpFastOpen();

loop:
   result = U_SYSCALL(connect, "%d,%p,%d", getFd(), (sockaddr*)cServer, cServer.sizeOf());

   if (result == 0)
      {
      setLocal();

      iState = CONNECT;

      U_RETURN(true);
      }

   if (errno == EINTR)
      {
      UInterrupt::checkForEventSignalPending();

      goto loop;
      }

   if (errno == EISCONN)
      {
      _closesocket();

      _socket();

      goto loop;
      }

   U_RETURN(false);
}

int USocket::recvFrom(void* pBuffer, uint32_t iBufLength, uint32_t uiFlags, UIPAddress& cSourceIP, unsigned int& iSourcePortNumber)
{
   U_TRACE(1, "USocket::recvFrom(%p,%u,%u,%p,%p)", pBuffer, iBufLength, uiFlags, &cSourceIP, &iSourcePortNumber)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isOpen())

   int iBytesRead;
   SocketAddress cSource;

   socklen_t slDummy = cSource.sizeOf();

loop:
   iBytesRead = U_SYSCALL(recvfrom, "%d,%p,%u,%u,%p,%p", getFd(), CAST(pBuffer), iBufLength, uiFlags, (sockaddr*)cSource, &slDummy);

   if (iBytesRead > 0)
      {
      U_INTERNAL_DUMP("BytesRead(%d) = %#.*S", iBytesRead, iBytesRead, CAST(pBuffer))

      iSourcePortNumber = cSource.getPortNumber();
                          cSource.getIPAddress(cSourceIP);

      U_RETURN(iBytesRead);
      }

   if (errno == EINTR)
      {
      UInterrupt::checkForEventSignalPending();

      goto loop;
      }

   U_RETURN(-1);
}

int USocket::sendTo(void* pPayload, uint32_t iPayloadLength, uint32_t uiFlags, UIPAddress& cDestinationIP, unsigned int iDestinationPortNumber)
{
   U_TRACE(1, "USocket::sendTo(%p,%u,%u,%p,%d)", pPayload, iPayloadLength, uiFlags, &cDestinationIP, iDestinationPortNumber)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isOpen())

   int iBytesWrite;
   SocketAddress cDestination;

   cDestination.setIPAddress(cDestinationIP);
   cDestination.setPortNumber(iDestinationPortNumber);

   loop:
   iBytesWrite = U_SYSCALL(sendto, "%d,%p,%u,%u,%p,%d", getFd(), CAST(pPayload), iPayloadLength, uiFlags, (sockaddr*)cDestination, cDestination.sizeOf());

   if (iBytesWrite > 0)
      {
      U_INTERNAL_DUMP("BytesWrite(%d) = %#.*S", iBytesWrite, iBytesWrite, CAST(pPayload))

      U_RETURN(iBytesWrite);
      }

   if (errno == EINTR)
      {
      UInterrupt::checkForEventSignalPending();

      goto loop;
      }

   U_RETURN(-1);
}

/**
 * Enables/disables the @c SO_TIMEOUT pseudo option.
 * @c SO_TIMEOUT is not one of the options defined for Berkeley sockets, but
 * was actually introduced as part of the Java API. For client sockets
 * it has the same meaning as the @c SO_RCVTIMEO option, which specifies
 * the maximum number of milliseconds that a blocking @c read() call will
 * wait for data to arrive on the socket.
 *
 * @param timeoutMS the specified timeout value, in milliseconds. A value of zero indicates no timeout, i.e. an infinite wait
 */

bool USocket::setTimeoutRCV(uint32_t timeoutMS)
{
   U_TRACE(1, "USocket::setTimeoutRCV(%u)", timeoutMS)

   U_INTERNAL_ASSERT(timeoutMS >= 200) // suspicious...

#if !defined(SO_RCVTIMEO)
   U_RETURN(false);
#endif

   // SO_RCVTIMEO is poorly documented in Winsock API, but it appears
   // to be measured as an int value in milliseconds, whereas BSD-style
   // sockets use a timeval

#ifdef _MSWINDOWS_
   bool result = setSockOpt(SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMS);
#else
   // Convert the timeout value (in milliseconds) into a timeval struct

   struct timeval timer;
   timer.tv_sec  =  timeoutMS / 1000;
   timer.tv_usec = (timeoutMS % 1000) * 1000;

   bool result = setSockOpt(SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer));
#endif

   U_RETURN(result);
}

bool USocket::setTimeoutSND(uint32_t timeoutMS)
{
   U_TRACE(1, "USocket::setTimeoutSND(%u)", timeoutMS)

   U_INTERNAL_ASSERT(timeoutMS >= 200) // suspicious...

#ifndef SO_SNDTIMEO 
   U_RETURN(false);
#endif

#ifdef _MSWINDOWS_
   bool result = setSockOpt(SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutMS);
#else
   // Convert the timeout value (in milliseconds) into a timeval struct

   struct timeval timer;
   timer.tv_sec  =  timeoutMS / 1000;
   timer.tv_usec = (timeoutMS % 1000) * 1000;

   bool result = setSockOpt(SOL_SOCKET, SO_SNDTIMEO, &timer, sizeof(timer));
#endif

   U_RETURN(result);
}

int USocket::recvBinary16Bits()
{
   U_TRACE_NO_PARAM(0, "USocket::recvBinary16Bits()")

   uint16_t uiNetOrder;
   uint32_t iBytesLeft = sizeof(uint16_t);
   char* pcEndReadBuffer = ((char*)&uiNetOrder) + iBytesLeft;

   do {
      iBytesLeft -= recv((void*)(pcEndReadBuffer - iBytesLeft), iBytesLeft);
      }
   while (iBytesLeft);

   int result = ntohs(uiNetOrder);

   U_RETURN(result);
}

uint32_t USocket::recvBinary32Bits()
{
   U_TRACE_NO_PARAM(0, "USocket::recvBinary32Bits()")

   uint32_t uiNetOrder, iBytesLeft = sizeof(uint32_t);
   char* pcEndReadBuffer = ((char*)&uiNetOrder) + iBytesLeft;

   do {
      iBytesLeft -= recv((void*)(pcEndReadBuffer - iBytesLeft), iBytesLeft);
      }
   while (iBytesLeft);

   int result = ntohl(uiNetOrder);

   U_RETURN(result);
}

bool USocket::sendBinary16Bits(uint16_t iData)
{
   U_TRACE(0, "USocket::sendBinary16Bits(%u)", iData)

   uint16_t uiNetOrder = htons(iData);

   bool result = (send((const char*)&uiNetOrder, sizeof(uint16_t)) == sizeof(uint16_t));

   U_RETURN(result);
}

bool USocket::sendBinary32Bits(uint32_t lData)
{
   U_TRACE(0, "USocket::sendBinary32Bits(%u)", lData)

   uint32_t uiNetOrder = htonl(lData);

   bool result = (send((const char*)&uiNetOrder, sizeof(uint32_t)) == sizeof(uint32_t));

   U_RETURN(result);
}

void USocket::setBlocking()
{
   U_TRACE_NO_PARAM(1, "USocket::setBlocking()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isOpen())
   U_INTERNAL_ASSERT_EQUALS(flags & O_NONBLOCK, O_NONBLOCK)

   flags &= ~O_NONBLOCK;

   (void) U_SYSCALL(fcntl, "%d,%d,%d", getFd(), F_SETFL, flags);

   U_INTERNAL_DUMP("O_NONBLOCK = %B, flags = %B", O_NONBLOCK, flags)
}

void USocket::setNonBlocking()
{
   U_TRACE_NO_PARAM(1, "USocket::setNonBlocking()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isOpen())
   U_INTERNAL_ASSERT_DIFFERS(flags & O_NONBLOCK, O_NONBLOCK)

   flags |= O_NONBLOCK;

   (void) U_SYSCALL(fcntl, "%d,%d,%d", getFd(), F_SETFL, flags);

   U_INTERNAL_DUMP("O_NONBLOCK = %B, flags = %B", O_NONBLOCK, flags)
}

#ifdef closesocket
#undef closesocket
#endif

void USocket::_closesocket()
{
   U_TRACE_NO_PARAM(1, "USocket::_closesocket()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isOpen())

#ifdef U_SERVER_CAPTIVE_PORTAL
   (void) U_SYSCALL(shutdown, "%d,%d", iSockDesc, SHUT_WR);
#endif

#ifdef _MSWINDOWS_
   (void) U_SYSCALL(closesocket, "%d", fh);
                                       fh = -1;
#elif defined(DEBUG)
   if (U_SYSCALL(   close, "%d", iSockDesc)) U_ERROR_SYSCALL("closesocket");
#else
   (void) U_SYSCALL(close, "%d", iSockDesc);
#endif

   iSockDesc   = -1;
   iRemotePort =  0;
}

void USocket::close()
{
   U_TRACE_NO_PARAM(0, "USocket::close()")

   closesocket();

   iState = CLOSE;
}

void USocket::closesocket()
{
   U_TRACE_NO_PARAM(1, "USocket::closesocket()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isOpen())

   U_INTERNAL_DUMP("U_ClientImage_parallelization = %u", U_ClientImage_parallelization)

   if (U_ClientImage_parallelization == U_PARALLELIZATION_CHILD)
      {
      iSockDesc = -1;

      return;
      }

#ifdef USE_LIBSSL
   if (isSSLActive()) ((USSLSocket*)this)->closesocket();
#endif

   U_INTERNAL_DUMP("isBroken()   = %b", isBroken())
   U_INTERNAL_DUMP("isTimeout()  = %b", isTimeout())
   U_INTERNAL_DUMP("isEpollErr() = %b", isEpollErr())

   /**
    * To obtain a clean closure sockets, one would call shutdown() with SHUT_WR
    * on the socket, call recv() until obtaining a return value of 0 indicating
    * that the peer has also performed an orderly shutdown, and finally call
    * close() on the socket.
    *
    * The shutdown() tells the receiver the server is done sending data. No
    * more data is going to be send. More importantly, it doesn't close the
    * socket. At the socket layer, this sends a TCP/IP FIN packet to the receiver
    */

   if ((iState & BROKEN) != 0 &&
       shutdown(SHUT_WR))
      {
      uint32_t count = 0;
      char _buf[8 * 1024];

      /**
       * At this point, the socket layer has to wait until the receiver has
       * acknowledged the FIN packet by receiving a ACK packet. This is done by
       * using the recv() command in a loop until 0 or less value is returned.
       * Once recv() returns 0 (or less), 1/2 of the socket is closed
       */

      if (isBlocking()) (void) UFile::setBlocking(iSockDesc, flags, false);

      do {
         if (++count > 5) break;

         errno = 0;

         if (count == 2 &&
             USocket::isTimeout() == false)
            {
            (void) UFile::setBlocking(iSockDesc, flags, true);
            }
         }
      while ((U_SYSCALL(recv, "%d,%p,%u,%d", getFd(), _buf, sizeof(_buf), 0) > 0) ||
             (errno == EAGAIN && UNotifier::waitForRead(iSockDesc, 500) > 0));
      }

   // NB: to avoid epoll_wait() fire events on file descriptor already closed...

#if defined(HAVE_EPOLL_WAIT) && !defined(USE_LIBEVENT)
   if (UNotifier::isHandler(iSockDesc))
      {
      (void) U_SYSCALL(epoll_ctl, "%d,%d,%d,%p", UNotifier::epollfd, EPOLL_CTL_DEL, iSockDesc, (struct epoll_event*)1);

      UNotifier::handlerDelete(iSockDesc, EPOLLIN | EPOLLRDHUP);
      }
#endif

   // Now we know that our FIN is ACK-ed, then you can close the second half of the socket by calling closesocket()

   _closesocket();
}

bool USocket::acceptClient(USocket* pcNewConnection)
{
   U_TRACE(1, "USocket::acceptClient(%p)", pcNewConnection)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isOpen())
   U_INTERNAL_ASSERT_POINTER(pcNewConnection)

   SocketAddress cRemote;

   socklen_t slDummy = cRemote.sizeOf();

#ifdef HAVE_ACCEPT4
       pcNewConnection->iSockDesc  = U_SYSCALL(accept4, "%d,%p,%p,%d", iSockDesc, (sockaddr*)cRemote, &slDummy, accept4_flags);
// if (pcNewConnection->iSockDesc != -1 || errno != ENOSYS) goto next;
#elif defined(_MSWINDOWS_)
   pcNewConnection->fh        = U_SYSCALL(accept, "%d,%p,%p", fh, (sockaddr*)cRemote, &slDummy);
   pcNewConnection->iSockDesc = _open_osfhandle((long)(pcNewConnection->fh), O_RDWR | O_BINARY);
#else
   pcNewConnection->iSockDesc = U_SYSCALL(accept, "%d,%p,%p", iSockDesc, (sockaddr*)cRemote, &slDummy);
#endif
//next:
   // -------------------------------------------------------------------------------------------------------
   // On success, this system call return a NONNEGATIVE INTEGER that is a descriptor for the accepted socket.
   // On error, -1 is returned, and errno is set appropriately
   // -------------------------------------------------------------------------------------------------------
   if (pcNewConnection->iSockDesc != -1)
      {
      pcNewConnection->iState = CONNECT;

      pcNewConnection->iRemotePort = cRemote.getPortNumber();
                                     cRemote.getIPAddress( pcNewConnection->cRemoteAddress);

      U_INTERNAL_DUMP("pcNewConnection->iSockDesc = %d pcNewConnection->flags = %d %B",
                       pcNewConnection->iSockDesc, pcNewConnection->flags, pcNewConnection->flags)

      U_INTERNAL_ASSERT_EQUALS(U_socket_IPv6(pcNewConnection), (cRemoteAddress.getAddressFamily() == AF_INET6))
      
#  ifdef HAVE_ACCEPT4
      U_INTERNAL_ASSERT_EQUALS(((accept4_flags & SOCK_CLOEXEC)  != 0),((pcNewConnection->flags & O_CLOEXEC)  != 0))
      U_INTERNAL_ASSERT_EQUALS(((accept4_flags & SOCK_NONBLOCK) != 0),((pcNewConnection->flags & O_NONBLOCK) != 0))
#  else
      if (accept4_flags) (void) U_SYSCALL(fcntl, "%d,%d,%d", pcNewConnection->iSockDesc, F_SETFL, pcNewConnection->flags);
#  endif

/*
#ifdef DEBUG
   struct linger x = { 0, -1 }; // { int l_onoff; int l_linger; }
   uint32_t tmp0 = sizeof(struct linger), value = U_NOT_FOUND, tmp = sizeof(uint32_t);

   (void) pcNewConnection->getSockOpt(SOL_SOCKET, SO_LINGER, (void*)&x, tmp0);

   U_INTERNAL_DUMP("SO_LINGER = { %d %d }", x.l_onoff, x.l_linger)

   U_DUMP("getBufferRCV() = %u getBufferSND() = %u", pcNewConnection->getBufferRCV(), pcNewConnection->getBufferSND())

# ifdef TCP_CORK
   (void) pcNewConnection->getSockOpt(SOL_TCP, TCP_CORK, (void*)&value, tmp);

   U_INTERNAL_DUMP("TCP_CORK = %d", value)
# endif

# ifdef TCP_DEFER_ACCEPT
   (void) pcNewConnection->getSockOpt(SOL_TCP, TCP_DEFER_ACCEPT, (void*)&value, tmp);

   U_INTERNAL_DUMP("TCP_DEFER_ACCEPT = %d", value)
# endif

# ifdef TCP_QUICKACK
   (void) pcNewConnection->getSockOpt(SOL_TCP, TCP_QUICKACK, (void*)&value, tmp);

   U_INTERNAL_DUMP("TCP_QUICKACK = %d", value)
# endif

# ifdef TCP_NODELAY
   (void) pcNewConnection->getSockOpt(SOL_TCP, TCP_NODELAY, (void*)&value, tmp);

   U_INTERNAL_DUMP("TCP_NODELAY = %d", value)
# endif

# ifdef TCP_FASTOPEN
   (void) pcNewConnection->getSockOpt(SOL_TCP, TCP_FASTOPEN, (void*)&value, tmp);

   U_INTERNAL_DUMP("TCP_FASTOPEN = %d", value)
# endif

# ifdef SO_KEEPALIVE
   (void) pcNewConnection->getSockOpt(SOL_SOCKET, SO_KEEPALIVE, (void*)&value, tmp);

   U_INTERNAL_DUMP("SO_KEEPALIVE = %d", value)
# endif

# ifdef TCP_CONGESTION
   char buffer[32];
   uint32_t tmp1 = sizeof(buffer);

   (void) pcNewConnection->getSockOpt(IPPROTO_TCP, TCP_CONGESTION, (void*)buffer, tmp1);

   U_INTERNAL_DUMP("TCP_CONGESTION = %S", buffer)
# endif
#endif
*/

#  ifdef USE_LIBSSL
      if (isSSLActive() &&
          ((USSLSocket*)this)->acceptSSL((USSLSocket*)pcNewConnection) == false)
         {
         U_RETURN(false);
         }
#  endif

      U_socket_Type(pcNewConnection) = U_socket_Type(this);

      U_RETURN(true);
      }

   U_INTERNAL_ASSERT_EQUALS(pcNewConnection->iSockDesc, -1)

   if (errno == EINTR) UInterrupt::checkForEventSignalPending(); // NB: we never restart accept(), in general the socket server is NOT blocking...

   pcNewConnection->iState = -errno;

   U_RETURN(false);
}

void USocket::setMsgError()
{
   U_TRACE_NO_PARAM(0, "USocket::setMsgError()")

#ifdef USE_LIBSSL
   if (isSSLActive())
      {
      U_INTERNAL_DUMP("ret = %d", ((USSLSocket*)this)->ret)

      if (((USSLSocket*)this)->ret != SSL_ERROR_NONE)
         {
         ((USSLSocket*)this)->setStatus(false);

         return;
         }
      }
#endif

   U_INTERNAL_DUMP("iState = %d", iState)

   U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

   if (isSysError())
      {
      u_errno = errno = -iState;

      (void) u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("%#R"), 0); // NB: the last argument (0) is necessary...
      }
}

// VIRTUAL METHOD

bool USocket::connectServer(const UString& server, unsigned int iServPort, int timeoutMS)
{
   U_TRACE(1, "USocket::connectServer(%V,%u,%d)", server.rep, iServPort, timeoutMS)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(server.isNullTerminated())

   // This method is called to connect the socket to a server TCP socket that is specified
   // by the provided IP Address and port number. We call the connect() method to perform the connection

   if (isOpen() == false) _socket();

   if (cRemoteAddress.setHostName(server, U_socket_IPv6(this)))
      {
      int result;
      SocketAddress cServer;
      bool bflag = (timeoutMS && ((flags & O_NONBLOCK) != O_NONBLOCK));

      if (bflag) setNonBlocking(); // setting socket to nonblocking

      if (timeoutMS) (void) setTimeoutRCV(timeoutMS);

      cServer.setIPAddress(cRemoteAddress);
      cServer.setPortNumber((iRemotePort = iServPort));

loop:
      result = U_SYSCALL(connect, "%d,%p,%d", getFd(), (sockaddr*)cServer, cServer.sizeOf());

      if (result == 0)
         {
ok:      setLocal();

         iState = CONNECT;

         if (bflag) setBlocking(); // restore socket status flags

         U_RETURN(true);
         }

      if (result == -1)
         {
         if (errno == EINPROGRESS)
            {
            result = UNotifier::waitForWrite(iSockDesc, timeoutMS);

            if (result == 1)
               {
               uint32_t error = U_NOT_FOUND, tmp = sizeof(uint32_t);

               (void) getSockOpt(SOL_SOCKET, SO_ERROR, (void*)&error, tmp);

               if (error == 0) goto ok;

               iState = -(u_errno = errno = error);
               }
            else if (result == 0)
               {
               // timeout

               _closesocket();

               iState  = TIMEOUT;
               u_errno = errno = ETIMEDOUT;
               }

            U_RETURN(false);
            }

         if (errno == EINTR)
            {
            UInterrupt::checkForEventSignalPending();

            goto loop;
            }

         if (errno == EISCONN)
            {
            _closesocket();

            _socket();

            goto loop;
            }
         }
      }

   U_RETURN(false);
}

int USocket::send(const char* pData, uint32_t iDataLen)
{
   U_TRACE(1, "USocket::send(%p,%u)", pData, iDataLen)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isOpen())

   int iBytesWrite;

loop:
   iBytesWrite = U_SYSCALL(send, "%d,%p,%u,%u", getFd(), CAST(pData), iDataLen, 0);

   if (iBytesWrite >= 0)
      {
#  ifdef DEBUG
      if (iBytesWrite > 0) U_INTERNAL_DUMP("BytesWrite(%d) = %#.*S", iBytesWrite, iBytesWrite, CAST(pData))
#  endif

      U_RETURN(iBytesWrite);
      }

   if (errno == EINTR)
      {
      UInterrupt::checkForEventSignalPending();

      goto loop;
      }

   U_RETURN(-1);
}

int USocket::recv(void* pBuffer, uint32_t iBufLength)
{
   U_TRACE(0, "USocket::recv(%p,%u)", pBuffer, iBufLength)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(isOpen())

   int iBytesRead;

loop:
   iBytesRead = U_SYSCALL(recv, "%d,%p,%u,%d", getFd(), CAST(pBuffer), iBufLength, 0);

   if (iBytesRead >= 0)
      {
#  ifdef DEBUG
      if (iBytesRead > 0) U_INTERNAL_DUMP("BytesRead(%d) = %#.*S", iBytesRead, iBytesRead, CAST(pBuffer))
#  endif

      U_RETURN(iBytesRead);
      }

   if (errno == EINTR)
      {
      UInterrupt::checkForEventSignalPending();

      goto loop;
      }

   U_RETURN(-1);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* USocket::dump(bool reset) const
{
   *UObjectIO::os << "flags                         " << flags                  << '\n'
                  << "iState                        " << iState                 << '\n'
                  << "iSockDesc                     " << iSockDesc              << '\n'
                  << "iLocalPort                    " << iLocalPort             << '\n'
                  << "iRemotePort                   " << iRemotePort            << '\n'
                  << "cLocalAddress   (UIPAddress   " << (void*)&cLocalAddress  << ")\n"
                  << "cRemoteAddress  (UIPAddress   " << (void*)&cRemoteAddress << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
