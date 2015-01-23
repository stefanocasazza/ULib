// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    udpsocket.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_UDPSOCKET_H
#define ULIB_UDPSOCKET_H 1

#include <ulib/net/socket.h>

class U_EXPORT UUDPSocket : public USocket {
public:

   // COSTRUTTORI

   UUDPSocket(bool bSocketIsIPv6 = false) : USocket(bSocketIsIPv6)
      {
      U_TRACE_REGISTER_OBJECT(0, UUDPSocket, "%b", bSocketIsIPv6)

      U_socket_Type(this) = USocket::SK_DGRAM;
      }

   ~UUDPSocket()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UUDPSocket)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return USocket::dump(reset); }
#endif

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UUDPSocket(const UUDPSocket&) = delete;
   UUDPSocket& operator=(const UUDPSocket&) = delete;
#else
   UUDPSocket(const UUDPSocket&) : USocket(false) {}
   UUDPSocket& operator=(const UUDPSocket&)       { return *this; }
#endif
};

#endif
