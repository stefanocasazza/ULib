// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    tcpsocket.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_TCPSOCKET_H
#define ULIB_TCPSOCKET_H 1

#include <ulib/net/socket.h>

class U_EXPORT UTCPSocket : public USocket {
public:

   UTCPSocket(bool bSocketIsIPv6 = false) : USocket(bSocketIsIPv6)
      {
      U_TRACE_REGISTER_OBJECT(0, UTCPSocket, "%b", bSocketIsIPv6)
      }

   ~UTCPSocket()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTCPSocket)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return USocket::dump(reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UTCPSocket)
};

#endif
