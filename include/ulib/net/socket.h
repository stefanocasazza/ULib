// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    socket.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_SOCKET_H
#define ULIB_SOCKET_H 1

#include <ulib/net/ipaddress.h>

#ifdef _MSWINDOWS_
#  include <ws2tcpip.h>
#  define CAST(a) (char*)a
#else
#  define CAST(a) a
#  include <netinet/tcp.h>
#  if defined(U_LINUX) && !defined(SO_INCOMING_CPU)
#     define SO_INCOMING_CPU 49
#  endif
#endif

#ifndef SOL_TCP
#define SOL_TCP IPPROTO_TCP
#endif

#if !defined(AF_INET6) && !defined(ENABLE_IPV6)
#define AF_INET6 AF_INET
#endif

/* Atomically mark descriptor(s) as non-blocking */
#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK 04000
#endif
/* Atomically set close-on-exec flag for the new descriptor(s) */
#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 02000000
#endif

/**
 * @class USocket
 *
 * @brief basic IP socket functionality
 *
 * This class is used to provide basic IP socket functionality within a C++ class environment.
 * The socket descriptor is stored as a member variable within the socket class.
 * The member methods are simple wrappers to the standard socket library function calls except
 * they use class UIPAddress instances and port numbers rather than sockaddr structures
 */

class UFile;
class UHTTP;
class UHTTP2;
class UNotifier;
class USocketExt;
class UFtpClient;
class UClient_Base;
class UServer_Base;
class SocketAddress;
class URDBClientImage;
class UHttpClient_Base;
class UClientImage_Base;
class UREDISClient_Base;

#define U_socket_IPv6(obj)     (obj)->USocket::flag[0]
#define U_socket_LocalSet(obj) (obj)->USocket::flag[1]
#define U_socket_Type(obj)     (obj)->USocket::flag[2]
#define U_socket_unused(obj)   (obj)->USocket::flag[3]

class U_EXPORT USocket {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   enum State {
      CLOSE       = 0x000,
      TIMEOUT     = 0x001,
      BROKEN      = 0x002,
      EPOLLERROR  = 0x004,
      CONNECT     = 0x008
   };

   enum Type {
      SK_STREAM     = 0x000,
      SK_DGRAM      = 0x001,
      SK_RAW        = 0x002,
      SK_UNIX       = 0x004,
      SK_SSL        = 0x008,
      SK_SSL_ACTIVE = 0x010
   };

            USocket(bool bSocketIsIPv6 = false);
   virtual ~USocket();

   int getFd() const
#ifdef _MSWINDOWS_
      { return fh; }
#else
      { return iSockDesc; }
#endif

   bool isOpen() const      { return (iSockDesc != -1); }
   bool isClosed() const    { return (iSockDesc == -1); }
   bool isBroken() const    { return ((iState & BROKEN)     != 0); }
   bool isTimeout() const   { return ((iState & TIMEOUT)    != 0); }
   bool isEpollErr() const  { return ((iState & EPOLLERROR) != 0); }
   bool isSysError() const  { return (iState  < CLOSE); }
   bool isConnected() const { return (iState >= CONNECT); }

   /**
    * This method is called after read block of data of remote connection
    */

   bool checkErrno();
   bool checkTime(long time_limit, long& timeout);

   /**
    * This method is called after send block of data to remote connection
    */

   bool checkIO(int iBytesTransferred)
      {
      U_TRACE(0, "USocket::checkIO(%d)", iBytesTransferred)

      if (iBytesTransferred <= 0)
         {
         if (iBytesTransferred < 0) (void) checkErrno();

         U_RETURN(false);
         }

      U_RETURN(true);
      }

   // coverity[+alloc]
   static int socket(int domain, int type, int protocol)
      {
      U_TRACE(1, "USocket::socket(%d,%d,%d)", domain, type, protocol)

      int fd = U_SYSCALL(socket, "%d,%d,%d", domain, type, protocol);

      U_RETURN(fd);
      }

   bool isUDP() const
      {
      U_TRACE_NO_PARAM(0, "USocket::isUDP()")

      U_INTERNAL_DUMP("U_socket_Type = %d %B", U_socket_Type(this), U_socket_Type(this))

      if ((U_socket_Type(this) & SK_DGRAM) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   bool isIPC() const
      {
      U_TRACE_NO_PARAM(0, "USocket::isIPC()")

      U_INTERNAL_DUMP("U_socket_Type = %d %B", U_socket_Type(this), U_socket_Type(this))

      if ((U_socket_Type(this) & SK_UNIX) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   bool isSSL() const
      {
      U_TRACE_NO_PARAM(0, "USocket::isSSL()")

      U_INTERNAL_DUMP("U_socket_Type = %d %B", U_socket_Type(this), U_socket_Type(this))

#  ifdef USE_LIBSSL
      if ((U_socket_Type(this) & SK_SSL) != 0) U_RETURN(true);
#  endif

      U_RETURN(false);
      }

   bool isSSLActive() const
      {
      U_TRACE_NO_PARAM(0, "USocket::isSSLActive()")

      U_INTERNAL_DUMP("U_socket_Type = %d %B", U_socket_Type(this), U_socket_Type(this))

#  ifdef USE_LIBSSL
      if ((U_socket_Type(this) & SK_SSL_ACTIVE) != 0) U_RETURN(true);
#  endif

      U_RETURN(false);
      }

   void setSSLActive(bool _flag)
      {
      U_TRACE(0, "USocket::setSSLActive(%b)", _flag)

#  ifdef USE_LIBSSL
      U_ASSERT(isSSL())

      if (_flag) U_socket_Type(this) |=  SK_SSL_ACTIVE;
      else       U_socket_Type(this) &= ~SK_SSL_ACTIVE;

      U_INTERNAL_DUMP("U_socket_Type = %d %B", U_socket_Type(this), U_socket_Type(this))
#  endif
      }

   /**
    * The getsockopt() function is called with the provided parameters to obtain the desired value
    */

   bool getSockOpt(int iCodeLevel, int iOptionName, void* pOptionData, uint32_t& iDataLength)
      {
      U_TRACE(1, "USocket::getSockOpt(%d,%d,%p,%p)", iCodeLevel, iOptionName, pOptionData, iDataLength)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(isOpen())

      if (U_SYSCALL(getsockopt, "%d,%d,%d,%p,%p", getFd(), iCodeLevel, iOptionName, CAST(pOptionData), (socklen_t*)&iDataLength) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   /**
    * The setsockopt() function is called with the provided parameters to obtain the desired value
    */

   bool setSockOpt(int iCodeLevel, int iOptionName, const void* pOptionData, uint32_t iDataLength = sizeof(int))
      {
      U_TRACE(1, "USocket::setSockOpt(%d,%d,%p,%u)", iCodeLevel, iOptionName, pOptionData, iDataLength) // problem with sanitize address

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(isOpen())

      if (U_SYSCALL(setsockopt, "%d,%d,%d,%p,%u", getFd(), iCodeLevel, iOptionName, CAST(pOptionData), iDataLength) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   /**
    * Actual state is blocking...?
    */

   bool isBlocking()
      {
      U_TRACE_NO_PARAM(0, "USocket::isBlocking()")

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("O_NONBLOCK = %B, flags = %B", O_NONBLOCK, flags)

      bool blocking = ((flags & O_NONBLOCK) != O_NONBLOCK);

      U_RETURN(blocking);
      }

   void setBlocking();
   void setNonBlocking();

   /**
    * Connect the socket to the specified server IP Address and port number
    */

   bool connectServer(const UIPAddress& cAddr, unsigned int iServPort);

   /**
    * The default local port number is automatically allocated, the default back logged queue length is 5.
    * We then try to bind the USocket to the specified port number and any local IP Address using the bind() method.
    * Following this, we call the listen() method to cause the socket to begin listening for new connections
    */

   void reusePort(int flags);
   bool setServer(unsigned int port, UString* localAddress = 0);

   /**
    * This method is called to accept a new pending connection on the server socket.
    * The USocket pointed to by the provided parameter is modified to refer to the
    * newly connected socket. The remote IP Address and port number are also set
    */

   bool acceptClient(USocket* pcConnection);

   /**
    * Get details of the IP address and port number bound to the local socket
    */

   void setLocal();

   UString getMacAddress(const char* device);

   UIPAddress&  localIPAddress()  __pure;
   unsigned int localPortNumber() __pure;

   const char* getLocalInfo()
      {
      U_TRACE_NO_PARAM(0, "USocket::getLocalInfo()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(isLocalSet())

      const char* address = cLocalAddress.getAddressString();

      U_RETURN(address);
      }

   bool isLocalSet() const { return U_socket_LocalSet(this); }

   /**
    * Get details of the IP address and port number bound to the remote socket
    */

   unsigned int remotePortNumber()
      {
      U_TRACE_NO_PARAM(0, "USocket::remotePortNumber()")

      U_CHECK_MEMORY

      U_RETURN(iRemotePort);
      }

   UIPAddress& remoteIPAddress()
      {
      U_TRACE_NO_PARAM(0, "USocket::remoteIPAddress()")

      U_CHECK_MEMORY

      return cRemoteAddress;
      }

   /**
    * This method manage the buffer of the socket connection
    */

   uint32_t getBufferRCV()
      {
      U_TRACE_NO_PARAM(1, "USocket::getBufferRCV()")

      uint32_t size = U_NOT_FOUND, tmp = sizeof(uint32_t);

      (void) getSockOpt(SOL_SOCKET, SO_RCVBUF, (void*)&size, tmp);

      U_RETURN(size);
      }

   uint32_t getBufferSND()
      {
      U_TRACE_NO_PARAM(1, "USocket::getBufferSND()")

      uint32_t size = U_NOT_FOUND, tmp = sizeof(uint32_t);

      (void) getSockOpt(SOL_SOCKET, SO_SNDBUF, (void*)&size, tmp);

      U_RETURN(size);
      }

   bool setBufferRCV(uint32_t size)
      {
      U_TRACE(1, "USocket::setBufferRCV(%u)", size)

      if (setSockOpt(SOL_SOCKET, SO_RCVBUF, (const void*)&size)) U_RETURN(true);

      U_RETURN(false);
      }

   bool setBufferSND(uint32_t size)
      {
      U_TRACE(1, "USocket::setBufferSND(%u)", size)

      if (setSockOpt(SOL_SOCKET, SO_SNDBUF, (const void*)&size)) U_RETURN(true); 

      U_RETURN(false);
      }

   /**
    * The shutdown() tells the receiver the server is done sending data. No
    * more data is going to be send. More importantly, it doesn't close the
    * socket. At the socket layer, this sends a TCP/IP FIN packet to the receiver
    */

   bool shutdown(int how = SHUT_WR)
      {
      U_TRACE(1, "USocket::shutdown(%d)", how)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(isOpen())

      if (U_SYSCALL(shutdown, "%d,%d", getFd(), how) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   void          close();
   void abortive_close() /* Abort connection */
      {
      U_TRACE_NO_PARAM(0, "USocket::abortive_close()")

      U_INTERNAL_ASSERT(isOpen())

      setTcpLingerOff();

      close();
      }

   /**
    * When enabled, a close(2) or shutdown(2) will not return until all queued messages for the socket have been
    * successfully sent or the linger timeout has been reached. Otherwise, the call returns immediately and the
    * closing is done in the background. When the socket is closed as part of exit(2), it always lingers in the background
    */

   void setTcpLinger(int val)
      {
      U_TRACE(0, "USocket::setTcpLinger(%d)", val)

      const struct linger l = {
         (val >= 0), /* linger active */
          val        /* how many seconds to linger for */
      };

      (void) setSockOpt(SOL_SOCKET, SO_LINGER, &l, sizeof(struct linger));
      }

   /**
    * When linger is off the TCP stack doesn't wait for pending data to be sent before closing the connection. Data
    * could be lost due to this but by setting linger to off you're accepting this and asking that the connection be
    * reset straight away rather than closed gracefully. This causes an RST to be sent rather than the usual FIN
    */

   void setTcpLingerOff() { setTcpLinger(0); }

   /**
    * Stick a TCP cork in the socket. It's not clear that this will help performance, but it might
    *
    * TCP_CORK: If set, don't send out partial frames. All queued partial frames are sent when the option is cleared again. This is useful
    *           for prepending headers before calling sendfile(), or for throughput optimization. As currently implemented, there is a 200
    *           millisecond ceiling on the time for which output is corked by TCP_CORK. If this ceiling is reached, then queued data is
    *           automatically transmitted
    *
    * This is a no-op if we don't think this platform has corks
    */

   static void setTcpCork(USocket* sk, uint32_t value)
      {
      U_TRACE(1, "USocket::setTcpCork(%p,%u)", sk, value)

      U_INTERNAL_ASSERT_POINTER(sk)

#  if defined(TCP_CORK) && defined(U_LINUX)
      (void) sk->setSockOpt(SOL_TCP, TCP_CORK, (const void*)&value);
#  endif
      }

   /**
    * Ask for the server not to be awakened until some data has arrived on the socket. Takes an integer value (seconds), this can bound the
    * maximum number of attempts TCP will make to complete the connection. This works for RPC protocol because the client sends a request immediately
    * after connection without waiting for anything from the server
    */

   void setTcpDeferAccept()
      {
      U_TRACE_NO_PARAM(0, "USocket::setTcpDeferAccept()")

#  if defined(TCP_DEFER_ACCEPT) && defined(U_LINUX)
      (void) setSockOpt(SOL_TCP, TCP_DEFER_ACCEPT, (const int[]){ 1 });
#  endif
      }

   void setTcpFastOpen()
      {
      U_TRACE_NO_PARAM(0, "USocket::setTcpFastOpen()")

#  if !defined(U_SERVER_CAPTIVE_PORTAL) && defined(U_LINUX) // && LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
#    ifndef TCP_FASTOPEN
#    define TCP_FASTOPEN 23 /* Enable FastOpen on listeners */
#    endif
      (void) setSockOpt(SOL_TCP, TCP_FASTOPEN, (const int[]){ 5 });
#  endif
      }

   void setTcpQuickAck(int value)
      {
      U_TRACE(0, "USocket::setTcpQuickAck(%d)", value)

#  if defined(TCP_QUICKACK) && defined(U_LINUX)
      (void) setSockOpt(SOL_TCP, TCP_QUICKACK, &value);
#  endif
      }

   void setTcpNoDelay()
      {
      U_TRACE_NO_PARAM(0, "USocket::setTcpNoDelay()")

#  ifdef TCP_NODELAY
      (void) setSockOpt(SOL_TCP, TCP_NODELAY, (const int[]){ 1 });
#  endif
      }

   void setTcpCongestion(const char* value)
      {
      U_TRACE(0, "USocket::setTcpCongestion(%S)", value)

#  if defined(TCP_CONGESTION) && defined(U_LINUX)
      (void) setSockOpt(IPPROTO_TCP, TCP_CONGESTION, (const void*)&value, u__strlen(value, __PRETTY_FUNCTION__) + 1);
#  endif
      }

   /**
    * SO_KEEPALIVE makes the kernel more aggressive about continually verifying the connection even when you're not doing anything,
    * but does not change or enhance the way the information is delivered to you. You'll find out when you try to actually do something
    * (for example "write"), and you'll find out right away since the kernel is now just reporting the status of a previously set flag,
    * rather than having to wait a few seconds (or much longer in some cases) for network activity to fail. The exact same code logic you
    * had for handling the "other side went away unexpectedly" condition will still be used; what changes is the timing (not the method)
    *
    * Ref1: FIN_WAIT2 [https://kb.iu.edu/d/ajmi]
    * Ref2: tcp_fin_timeout [https://www.frozentux.net/ipsysctl-tutorial/chunkyhtml/tcpvariables.html#AEN370]
    * Ref3: tcp_retries2 [https://www.frozentux.net/ipsysctl-tutorial/chunkyhtml/tcpvariables.html#AEN444]
    * Ref4: tcp_max_orphans [https://www.frozentux.net/ipsysctl-tutorial/chunkyhtml/tcpvariables.html#AEN388]
    */

   void setTcpKeepAlive();

   /**
    * Enables/disables the @c SO_TIMEOUT pseudo option
    * 
    * @c SO_TIMEOUT is not one of the options defined for Berkeley sockets, but was actually introduced
    * as part of the Java API. For client sockets it has the same meaning as the @c SO_RCVTIMEO option,
    * which specifies the maximum number of milliseconds that a blocking @c read() call will wait for
    * data to arrive on the socket. Timeouts only have effect for system calls that perform socket I/O
    * (e.g., read(2), recvmsg(2), send(2), sendmsg(2));
    * 
    * @param timeoutMS the specified timeout value, in milliseconds. A zero value indicates no timeout, i.e. an infinite wait
    */

   bool setTimeoutRCV(uint32_t timeoutMS = U_TIMEOUT_MS);
   bool setTimeoutSND(uint32_t timeoutMS = U_TIMEOUT_MS);

   /**
    * The recvfrom() function is called with the proper parameters, params is placed for obtaining
    * the source address information. The number of bytes read is returned
    */

   int recvFrom(void* pBuffer, uint32_t iBufLength, uint32_t uiFlags, UIPAddress& cSourceIP, unsigned int& iSourcePortNumber);

   /**
    * The socket transmits the data to the remote socket
    */

   int sendTo(void* pPayload, uint32_t iPayloadLength, uint32_t uiFlags, UIPAddress& cDestinationIP, unsigned int iDestinationPortNumber);

   /**
    * This method is called to read a 16-bit binary value from the remote connection.
    * We loop - calling recv() - until the required number of bytes are read, if recv()
    * returns a smaller number of bytes due to the remaining values not yet arriving, we go back into a loop.
    * Once the value is read into uiNetOrder, we convert it to host byte order and return the read value
    */

   int recvBinary16Bits();

   /**
    * This method is called to read a 32-bit binary value from the remote connection.
    * We loop - calling recv() - until the required number of bytes are read, if recv()
    * returns a smaller number of bytes due to the remaining values not yet arriving, we go back into a loop.
    * Once the value is read into uiNetOrder, we convert it to host byte order and return the read value
    */

   uint32_t recvBinary32Bits();

   /**
    * This method is called to send a 16-bit binary value to the remote connection.
    * We convert the parameter to network byte order and call the send() method to send it.
    * If two bytes are not sent (the returned value is not two), return false
    */

   bool sendBinary16Bits(uint16_t iData);

   /**
    * This method is called to send a 32-bit binary value to the remote connection.
    * We convert the parameter to network byte order and call the send() method to send it.
    * If four bytes are not sent (the returned value is not four), return false
    */

   bool sendBinary32Bits(uint32_t lData);

   // -----------------------------------------------------------------------------------------------------------
   // VIRTUAL METHOD
   // -----------------------------------------------------------------------------------------------------------

   /**
    * This method is called to connect the socket to a server TCP socket that is specified
    * by the provided IP Address and port number. We call the connect() method to perform the connection

    * @param timeoutMS the specified timeout value, in milliseconds. A zero value indicates no timeout, i.e. a system depend waiting
    */

   virtual bool connectServer(const UString& server, unsigned int iServPort, int timeoutMS = 0);

   /**
    * This method is called to receive a block of data on the connected socket.
    * The parameters signify the payload receiving buffer and its size.
    * If the socket is not connected, then we failed on assertion, otherwise we call
    * the recv() method to receive the data, returning the number of bytes actually readden
    */

   virtual int recv(void* pBuffer, uint32_t iBufLength);

   /**
    * This method is called to send a block of data to the remote connection.
    * The parameters signify the Data Payload and its size.
    * If the socket is not connected, then we failed on assertion, otherwise we call
    * the send() method to send the data, returning the number of bytes actually sent
    */

   virtual int send(const char* pPayload, uint32_t iPayloadLength);
   // -----------------------------------------------------------------------------------------------------------

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UIPAddress cLocalAddress, cRemoteAddress;
   unsigned char flag[4];
   int iSockDesc, iState, flags;
   unsigned int iLocalPort, iRemotePort;
#ifdef _MSWINDOWS_
   SOCKET fh;
#endif

   bool bind();
   bool connect();
   void setRemote();
   void setMsgError();
   void setReusePort();
   void setReuseAddress();
   void setAddress(void* address);
   void setLocal(const UIPAddress& addr);
   bool setHostName(const UString& pcNewHostName);

   void setFlags(int _flags)
      {
      U_TRACE(1, "USocket::setFlags(%d)", _flags)

      U_INTERNAL_ASSERT(isOpen())

      (void) U_SYSCALL(fcntl, "%d,%d,%d", iSockDesc, F_SETFL, (flags = _flags));
      }

#ifdef closesocket
#undef closesocket
#endif

   void  closesocket();
   void _closesocket();

   static SocketAddress* cLocal;
   static bool tcp_reuseport, bincoming_cpu;
   static int iBackLog, incoming_cpu, accept4_flags; // If flags is 0, then accept4() is the same as accept()

   /**
    * The _socket() function is called to create the socket of the specified type.
    * The parameters indicate whether the socket will use IPv6 or IPv4 and the type of socket
    * (the default being SOCK_STREAM or TCP). The returned descriptor is stored in iSockDesc
    */

   void _socket(int iSocketType = 0, int domain = 0, int protocol = 0);

private:
   U_DISALLOW_COPY_AND_ASSIGN(USocket)

                      friend class UHTTP;
                      friend class UHTTP2;
                      friend class UFile;
                      friend class UNotifier;
                      friend class USocketExt;
                      friend class UFtpClient;
                      friend class UClient_Base;
                      friend class UServer_Base;
                      friend class UStreamPlugIn;
                      friend class URDBClientImage;
                      friend class UHttpClient_Base;
                      friend class UWebSocketPlugIn;
                      friend class UClientImage_Base;
                      friend class UREDISClient_Base;
   template <class T> friend class UServer;
   template <class T> friend class UClientImage;
};

#endif
