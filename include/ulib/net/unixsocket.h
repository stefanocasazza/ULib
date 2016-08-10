// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    unixsocket.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_UNIXSOCKET_H
#define ULIB_UNIXSOCKET_H 1

#include <ulib/net/socket.h>

#ifdef _WIN32
#  include <io.h>
#else
#  include <sys/un.h>
#endif

/**
 * Unix domain sockets are used for stream based connected sessions between processes on the same machine
 * --------------------------------------------------------------------------------------------------------------------------------------------------
 * - UNIX domain sockets use the file system as the address name space. This means you can use UNIX file permissions to control access to communicate
 *   with them.  I.e., you can limit what other processes can connect to the daemon -- maybe one user can, but the web server can't, or the like.
 *   With IP sockets, the ability to connect to your daemon is exposed off the current system, so additional steps may have to be taken for security.
 *   On the other hand, you get network transparency. With UNIX domain sockets, you can actually retrieve the credential of the process that created
 *   the remote socket, and use that for access control also, which can be quite convenient on multi-user systems.
 *
 * - IP sockets over localhost are basically looped back network on-the-wire IP. There is intentionally "no special knowledge" of the fact that the
 *   connection is to the same system, so no effort is made to bypass the normal IP stack mechanisms for performance reasons. For example,
 *   transmission over TCP will always involve two context switches to get to the remote socket, as you have to switch through the netisr, which
 *   occurs following the "loopback" of the packet through the synthetic loopback interface.  Likewise, you get all the overhead of ACKs, TCP flow
 *   control, encapsulation/decapsulation, etc. Routing will be performed in order to decide if the packets go to the localhost. Large sends will
 *   have to be broken down into MTU-size datagrams, which also adds overhead for large writes. It's really TCP, it just goes over a loopback
 *   interface by virtue of a special address, or discovering that the address requested is served locally rather than over an ethernet (etc).
 *
 * - UNIX domain sockets have explicit knowledge that they're executing on the same system. They avoid the extra context switch through the netisr,
 *   and a sending thread will write the stream or datagrams directly into the receiving socket buffer. No checksums are calculated, no headers are
 *   inserted, no routing is performed, etc. Because they have access to the remote socket buffer, they can also directly provide feedback to the
 *   sender when it is filling, or more importantly, emptying, rather than having the added overhead of explicit acknowledgement and window changes.
 *   The one piece of functionality that UNIX domain sockets don't provide that TCP does is out-of-band data. In practice, this is an issue for almost
 *   noone.
 *
 * In general, the argument for implementing over TCP is that it gives you location independence and immediate portability -- you can move the client
 * or the daemon, update an address, and it will "just work". The sockets layer provides a reasonable abstraction of communications services, so it's
 * not hard to write an application so that the connection/binding portion knows about TCP and UNIX domain sockets, and all the rest just uses the socket
 * it's given. So if you're looking for performance locally, I think UNIX domain sockets probably best meet your need. Many people will code to TCP
 * anyway because performance is often less critical, and the network portability benefit is substantial.
 *
 * Robert N M Watson
 * --------------------------------------------------------------------------------------------------------------------------------------------------
*/

union uusockaddr_un {
   struct sockaddr     psaGeneric;
   struct sockaddr_un  psaUnixAddr;
};

class UServer_Base;

class U_EXPORT UUnixSocket : public USocket {
public:

   UUnixSocket(bool _flag = false) : USocket(false)
      {
      U_TRACE_REGISTER_OBJECT(0, UUnixSocket, "%b", _flag)

      U_socket_Type(this) = USocket::SK_UNIX;
      }

   virtual ~UUnixSocket()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UUnixSocket)
      }

   static void setPath(const char* pathname);

   // VIRTUAL METHOD

   virtual bool connectServer(const UString& pathname, unsigned int iServPort, int timeoutMS = 0) U_DECL_FINAL;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

private:
   static socklen_t len;
   static const char* path;
   static union uusockaddr_un addr;

   U_DISALLOW_COPY_AND_ASSIGN(UUnixSocket)

   friend class USocket;
   friend class UServer_Base;
};

#endif
