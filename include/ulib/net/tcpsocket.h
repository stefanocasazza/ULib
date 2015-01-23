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

   // COSTRUTTORI

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
#ifdef U_COMPILER_DELETE_MEMBERS
   UTCPSocket(const UTCPSocket&) = delete;
   UTCPSocket& operator=(const UTCPSocket&) = delete;
#else
   UTCPSocket(const UTCPSocket&) : USocket(false) {}
   UTCPSocket& operator=(const UTCPSocket&)       { return *this; }
#endif
};

#endif
