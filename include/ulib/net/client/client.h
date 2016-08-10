// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    client.h - manages a client connection with a server
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_CLIENT_H
#define U_CLIENT_H 1

#include <ulib/url.h>
#include <ulib/net/rpc/rpc.h>
#include <ulib/utility/uhttp.h>

#ifdef USE_LIBSSL
#  include <ulib/ssl/certificate.h>
#  include <ulib/ssl/net/sslsocket.h>
#endif

/**
 * @class UClient
 *
 * @brief Handles a connections with a server
 */

class ULog;
class USSLSocket;
class UHttpPlugIn;
class UFileConfig;
class UFCGIPlugIn;
class USCGIPlugIn;
class UProxyPlugIn;
class UNoCatPlugIn;
class UServer_Base;
class UHttpClient_Base;
class UElasticSearchClient;

class U_EXPORT UClient_Base {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // SERVICES

   bool isOpen() const
      {
      U_TRACE_NO_PARAM(0, "UClient_Base::isOpen()")

      if (socket->isOpen()) U_RETURN(true);

      U_RETURN(false);
      }

   bool isClosed() const
      {
      U_TRACE_NO_PARAM(0, "UClient_Base::isClosed()")

      if (socket->isClosed()) U_RETURN(true);

      U_RETURN(false);
      }

   bool isConnected() const
      {
      U_TRACE_NO_PARAM(0, "UClient_Base::isConnected()")

      if (socket->isOpen() &&
          socket->isConnected())
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   void reserve(uint32_t n)
      {
      U_TRACE(0, "UClient_Base::reserve(%u)", n)

      (void) response.reserve(n);
      }

   void close()
      {
      U_TRACE_NO_PARAM(0, "UClient_Base::close()")

      U_INTERNAL_ASSERT_POINTER(socket)

      if (isOpen()) socket->_closesocket();
      }

   bool shutdown(int how = SHUT_WR)
      {
      U_TRACE(0, "UClient_Base::shutdown(%d)", how)

      U_INTERNAL_ASSERT_POINTER(socket)

      if (socket->shutdown(how)) U_RETURN(true);

      U_RETURN(false);
      }

   void setTimeOut(uint32_t t)
      {
      U_TRACE(0, "UClient_Base::setTimeOut()")

      timeoutMS = t;
      }

   void adjustTimeOut()
      {
      U_TRACE_NO_PARAM(0, "UClient_Base::adjustTimeOut()")

      if (timeoutMS < U_TIMEOUT_MS) timeoutMS = U_TIMEOUT_MS;
      }

   void reset()
      {
      U_TRACE_NO_PARAM(0, "UClient_Base::reset()")

      uri.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE... (uri can be a substr of url)
      url.clear();
      }

   // LOG 

   static void closeLog();
   static void setLogShared();

   static bool isLogSharedWithServer()
      {
      U_TRACE_NO_PARAM(0, "UClient_Base::isLogSharedWithServer()")

      U_RETURN(log_shared_with_server);
      }

   UString getUrl() const      { return url.get(); }
   UString getServer() const   { return server; }
   UString getBuffer() const   { return buffer; }
   UString getResponse() const { return response; }

   int          getFd() const           { return socket->getFd(); }
   const char*  getResponseData() const { return response.data(); }
   unsigned int getPort() const         { return port; }

   bool connect();
   void clearData();
   bool remoteIPAddress(UIPAddress& addr);
   bool readResponse(uint32_t count = U_SINGLE_READ);
   bool setHostPort(const UString& host, unsigned int port);

   // NB: return if it has modified host or port...

   bool setUrl(const char* str, uint32_t len);

   bool setUrl(const UString& _url) { return setUrl(U_STRING_TO_PARAM(_url)); }

   /**
    * Establishes a TCP/IP socket connection with the host that will satisfy requests for the provided URL.
    * This may connect to the host name contained within the URL, or to a proxy server if one has been set.
    * This function does not send any information to the remote server after the connection is established
    *
    * @param url a fully-qualified http URL for the required resource
    */

   bool connectServer(const UString& url);

   // QUEUE MODE

   static int queue_fd;
   static const UString* queue_dir;

   // -----------------------------------------------------------------------------------------------------------------------
   // Very simple RPC-like layer
   //
   // Requests and responses are build of little packets each containing a 4-byte ascii token, an 8-byte hex value or length,
   // and optionally data corresponding to the length
   // -----------------------------------------------------------------------------------------------------------------------

   // Transmit token name (4 characters) and value (32-bit int, as 8 hex characters)

   bool sendTokenInt(const char* token, uint32_t value)
      { buffer.setEmpty(); return URPC::sendTokenInt(socket, token, value, buffer); }

   // Write a token, and then the string data

   bool sendTokenString(const char* token, const UString& data)
      { buffer.setEmpty(); return URPC::sendTokenString(socket, token, data, buffer); }

   // Transmit an vector of string

   bool sendTokenVector(const char* token, UVector<UString>& vec)
      { buffer.setEmpty(); return URPC::sendTokenVector(socket, token, vec, buffer); }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

   USocket* socket;
protected:
   UString server,    // host name or ip address for server
           cert_file, // locations for certificate of client
           key_file,  // locations for private key of client
           password,  // password  for private key of client
           ca_file,   // locations of trusted CA certificates used in the verification
           ca_path,   // locations of trusted CA certificates used in the verification
           uri,
           request,
           response,
           buffer,
           host_port;

   Url url;
   int iovcnt,
       timeoutMS,     // the time-out value in milliseconds
       verify_mode;   // mode of verification of connection
   unsigned int port; // the port number to connect to

   struct iovec iov[6];

   static ULog* log;
   static UFileConfig* cfg;
   static bool log_shared_with_server, bIPv6;

   bool readHTTPResponse();

   void prepareRequest(const char* req, uint32_t len)
      {
      U_TRACE(0, "UClient_Base::prepareRequest(%.*S,%u)", len, req, len)

      iovcnt = 1;

      iov[0].iov_base = (caddr_t)req;
      iov[0].iov_len  =          len;

      (void) U_SYSCALL(memset, "%p,%d,%u", iov+1, 0, sizeof(struct iovec) * 5);

      U_INTERNAL_ASSERT_EQUALS(iov[1].iov_len, 0)
      U_INTERNAL_ASSERT_EQUALS(iov[2].iov_len, 0)
      U_INTERNAL_ASSERT_EQUALS(iov[3].iov_len, 0)
      U_INTERNAL_ASSERT_EQUALS(iov[4].iov_len, 0)
      U_INTERNAL_ASSERT_EQUALS(iov[5].iov_len, 0)
      }

   void prepareRequest(const UString& req) { request = req; prepareRequest(U_STRING_TO_PARAM(req)); }

   bool sendRequest(bool bread_response = false);
   bool sendRequestAndReadResponse() { return sendRequest(true); }

#ifdef USE_LIBSSL
   void setSSLContext();

   void setActive(bool _flag)
      {
      U_TRACE(0, "UClient_Base::setActive(%b)", _flag)

      ((USSLSocket*)socket)->setSSLActive(_flag);
      }
#endif

   void loadConfigParam();

    UClient_Base(UFileConfig* pcfg = 0);
   ~UClient_Base();

private:
   U_DISALLOW_COPY_AND_ASSIGN(UClient_Base)

   friend class USSLSocket;
   friend class UFCGIPlugIn;
   friend class USCGIPlugIn;
   friend class Application;
   friend class UHttpPlugIn;
   friend class UProxyPlugIn;
   friend class UNoCatPlugIn;
   friend class UServer_Base;
   friend class UHttpClient_Base;
   friend class UElasticSearchClient;
};

template <class Socket> class U_EXPORT UClient : public UClient_Base {
public:

   UClient(UFileConfig* pcfg) : UClient_Base(pcfg)
      {
      U_TRACE_REGISTER_OBJECT(0, UClient, "%p", pcfg)

      U_NEW(Socket, socket, Socket(UClient_Base::bIPv6));
      }

   ~UClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UClient)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return UClient_Base::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UClient)
};

#ifdef USE_LIBSSL
template <> class U_EXPORT UClient<USSLSocket> : public UClient_Base {
public:

   UClient(UFileConfig* pcfg) : UClient_Base(pcfg)
      {
      U_TRACE_REGISTER_OBJECT(0, UClient<USSLSocket>, "%p", pcfg)

      UClient_Base::setSSLContext();
      }

   ~UClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UClient<USSLSocket>)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return UClient_Base::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UClient<USSLSocket>)
};
#endif
#endif
