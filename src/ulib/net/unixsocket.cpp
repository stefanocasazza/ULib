// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    unixsocket.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>
#include <ulib/net/unixsocket.h>
#include <ulib/utility/interrupt.h>

socklen_t           UUnixSocket::len;
const char*         UUnixSocket::path;
union uusockaddr_un UUnixSocket::addr;

void UUnixSocket::setPath(const char* pathname)
{
   U_TRACE(1, "UUnixSocket::setPath(%S)", pathname)

   path = pathname;

   unsigned slen = u__strlen(pathname, __PRETTY_FUNCTION__);

   U_INTERNAL_ASSERT_MINOR(slen, sizeof(addr.psaUnixAddr.sun_path))

// if (slen > sizeof(addr.psaUnixAddr.sun_path)) slen = sizeof(addr.psaUnixAddr.sun_path);

   (void) U_SYSCALL(memset, "%p,%d,%u", &addr, 0, sizeof(addr));

   addr.psaUnixAddr.sun_family = AF_UNIX;

   U_MEMCPY(addr.psaUnixAddr.sun_path, pathname, slen);

#ifdef __SUN_LEN
   addr.psaUnixAddr.sun_len = len = sizeof(addr.psaUnixAddr.sun_len) + slen + sizeof(addr.psaUnixAddr.sun_family) + 1;
#else
                              len =                                    slen + sizeof(addr.psaUnixAddr.sun_family) + 1;
#endif
}

// VIRTUAL METHOD

bool UUnixSocket::connectServer(const UString& server, unsigned int iServPort, int timeoutMS)
{
   U_TRACE(1, "UUnixSocket::connectServer(%V,%u,%d)", server.rep, iServPort, timeoutMS)

   U_CHECK_MEMORY

   int result;

   if (isClosed())
      {
      if (path == 0)
         {
         U_INTERNAL_ASSERT(server.isNullTerminated())

         setPath(server.data());
         }

      USocket::_socket(SOCK_STREAM, AF_UNIX);
      }

loop:
   result = U_SYSCALL(connect, "%d,%p,%d", iSockDesc, &addr.psaGeneric, len);

   if (result == 0)
      {
      iState     = CONNECT;
      iLocalPort = iRemotePort = iServPort;

      U_socket_LocalSet(this) = true;

      U_RETURN(true);
      }

   if (errno == EINTR)
      {
      UInterrupt::checkForEventSignalPending();

      goto loop;
      }

   U_RETURN(false);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UUnixSocket::dump(bool reset) const
{
   USocket::dump(false);

   *UObjectIO::os << '\n'
                  << "len                           " << len          << '\n'
                  << "addr                          " << (void*)&addr << '\n'
                  << "path                          " << path;

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
