// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    socket_ext.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_SOCKET_EXT_H
#define ULIB_SOCKET_EXT_H 1

#include <ulib/string.h>
#include <ulib/net/socket.h>

typedef void (*vPFsu)(UString&,uint32_t);

class URPC;
class UHTTP;
class UHTTP2;
class UWebSocket;
class UMimeHeader;
class USmtpClient;
class UPop3Client;
class UClient_Base;
class UServer_Base;
class UStreamPlugIn;
class URPCClient_Base;
class UHttpClient_Base;
class UClientImage_Base;

class U_EXPORT USocketExt {
public:

   // SERVICES

   static UString getNodeName()
      {
      U_TRACE_NO_PARAM(0, "USocketExt::getNodeName()")

      U_INTERNAL_ASSERT_MAJOR(u_hostname_len, 0)

      UString result(u_hostname, u_hostname_len);

      U_INTERNAL_ASSERT(result.isNullTerminated())

      U_RETURN_STRING(result);
      }

   static void setRemoteInfo(USocket* sk, UString& logbuf)
      {
      U_TRACE(0, "USocketExt::setRemoteInfo(%p,%V)", sk, logbuf.rep)

      UString x(100U);

      x.snprintf(U_CONSTANT_TO_PARAM("%2d '%s:%u'"), sk->iSockDesc, sk->cRemoteAddress.pcStrAddress, sk->iRemotePort);

      (void) logbuf.insert(0, x);
      }

   static bool getARPCache(UString& cache, UVector<UString>& vec);

   static UString getNetworkDevice(const char* exclude); // eth0
   static UString getIPAddress(int fd, const char* device); // eth0
   static UString getMacAddress(int fd, const char* device); // eth0
   static UString getNetworkAddress(int fd, const char* device); // eth0

   static UString getMacAddress(          const char* ip, uint32_t ip_len);
   static UString getNetworkInterfaceName(const char* ip, uint32_t ip_len);
   static UString getGatewayAddress(const char* network, uint32_t network_len);

   static UString getMacAddress(          const UString& ip) { return getMacAddress(          U_STRING_TO_PARAM(ip)); }
   static UString getNetworkInterfaceName(const UString& ip) { return getNetworkInterfaceName(U_STRING_TO_PARAM(ip)); }
   static UString getGatewayAddress(const UString& network)  { return getGatewayAddress(U_STRING_TO_PARAM(network)); }

   // Send a command to a server and wait for a response (single line)

   static int vsyncCommand(USocket* sk, const char* format, uint32_t fmt_size, va_list argp)
      {
      U_TRACE(0, "USocketExt::vsyncCommand(%p,%.*S,%u)", sk, fmt_size, format, fmt_size)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      return vsyncCommand(sk, u_buffer, U_BUFFER_SIZE, format, fmt_size, argp);
      }

   static int vsyncCommand(USocket* sk, char* buffer, uint32_t buffer_size, const char* format, uint32_t fmt_size, va_list argp);

   // response from server (single line)

   static int readLineReply(USocket* sk)
      {
      U_TRACE(0, "USocketExt::readLineReply(%p)", sk)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      return readLineReply(sk, u_buffer, U_BUFFER_SIZE);
      }

   static int readLineReply(USocket* sk, char* buffer, uint32_t buffer_size);

   static void readLineReply(USocket* sk, UString& buffer)
      {
      U_TRACE(0, "USocketExt::readLineReply(%p,%p)", sk, &buffer)

      U_ASSERT(buffer.uniq())

      buffer.rep->_length = readLineReply(sk, buffer.data(), buffer.capacity());
      }

   // Send a command to a server and wait for a response (multi line)

   static int vsyncCommandML(USocket* sk, const char* format, uint32_t fmt_size, va_list argp)
      {
      U_TRACE(0, "USocketExt::vsyncCommandML(%p,%.*S,%u)", sk, fmt_size, format, fmt_size)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      return vsyncCommandML(sk, u_buffer, U_BUFFER_SIZE, format, fmt_size, argp);
      }

   static int vsyncCommandML(USocket* sk, char* buffer, uint32_t buffer_size, const char* format, uint32_t fmt_size, va_list argp);

   // response from server (multi line)

   static int readMultilineReply(USocket* sk)
      {
      U_TRACE(0, "USocketExt::readMultilineReply(%p)", sk)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      return readMultilineReply(sk, u_buffer, U_BUFFER_SIZE);
      }

   static int readMultilineReply(USocket* sk, char* buffer, uint32_t buffer_size);

   // Send a command to a server and wait for a response (check for token line)

   static int vsyncCommandToken(USocket* sk, UString& buffer, const char* format, uint32_t fmt_size, va_list argp);

#ifdef USE_C_ARES
   static char* endResolv()
      {
      U_TRACE_NO_PARAM(0, "USocketExt::endResolv()")

      U_INTERNAL_ASSERT_POINTER(resolv_channel)

      return (resolv_status == 0 ? resolv_hostname : 0);
      }

   static void  waitResolv();
   static void startResolv(const char* name, int family = AF_INET); // AF_INET6
#endif

private:
#ifdef USE_C_ARES
   static int   resolv_status;
   static char  resolv_hostname[INET6_ADDRSTRLEN];
   static void* resolv_channel;

   static void callbackResolv(void* arg, int status, int timeouts, struct hostent* host) U_NO_EXPORT;
#endif

   static inline bool parseCommandResponse(char* buffer, int r, int response) U_NO_EXPORT;

   /**
    * Read data from socket
    *
    * @param timeoutMS  specified the timeout value, in milliseconds. A negative value indicates no timeout, i.e. an infinite wait
    * @param time_limit specified the maximum execution time, in seconds. If set to zero, no time limit is imposed
    */

   static bool read(USocket* sk, UString& buffer, uint32_t count = U_SINGLE_READ, int timeoutMS = -1, uint32_t time_limit = 0); // read while not received almost count data

   // read while received data

   static void readEOF(USocket* sk, UString& buffer)
      {
      U_TRACE(0, "USocketExt::readEOF(%p,%p)", sk, &buffer)

      while (USocketExt::read(sk, buffer, U_SINGLE_READ, -1, 0)) {}
      }

   // read while not received token, return position of token in buffer

   static uint32_t readWhileNotToken(USocket* sk, UString& buffer, const char* token, uint32_t token_len, int timeoutMS = -1);

   // write data

   static int write(USocket* sk, const UString& buffer,           int timeoutMS) { return write(sk, U_STRING_TO_PARAM(buffer), timeoutMS); }
   static int write(USocket* sk, const char* ptr, uint32_t count, int timeoutMS);

   // write data from multiple buffers

   static void iov_resize(struct iovec* iov, int iovcnt, size_t value) U_NO_EXPORT;

   static int  writev(USocket* sk, struct iovec* iov, int iovcnt, uint32_t count, int timeoutMS);
   static int _writev(USocket* sk, struct iovec* iov, int iovcnt, uint32_t count, int timeoutMS);
   static int  writev(USocket* sk, struct iovec* iov, int iovcnt, uint32_t count, int timeoutMS, uint32_t cloop);

   /**
    * sendfile() copies data between one file descriptor and another. Either or both of these file descriptors may refer to a socket.
    * OUT_FD should be a descriptor opened for writing. POFFSET is a pointer to a variable holding the input file pointer position from
    * which sendfile() will start reading data. When sendfile() returns, this variable will be set to the offset of the byte following
    * the last byte that was read. COUNT is the number of bytes to copy between file descriptors. Because this copying is done within
    * the kernel, sendfile() does not need to spend time transferring data to and from user space
    */

   static int sendfile(USocket* sk, int in_fd, off_t* poffset, uint32_t count, int timeoutMS);

   friend class URPC;
   friend class UHTTP;
   friend class UHTTP2;
   friend class UWebSocket;
   friend class UMimeHeader;
   friend class USmtpClient;
   friend class UPop3Client;
   friend class UServer_Base;
   friend class UClient_Base;
   friend class UStreamPlugIn;
   friend class URPCClient_Base;
   friend class UHttpClient_Base;
   friend class UClientImage_Base;
};

#endif
