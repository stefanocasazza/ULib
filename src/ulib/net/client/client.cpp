// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    client.cpp - Handles a connections with a server
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file_config.h>
#include <ulib/net/client/client.h>
#include <ulib/net/server/server.h>

int            UClient_Base::queue_fd = -1;
bool           UClient_Base::bIPv6;
bool           UClient_Base::log_shared_with_server;
ULog*          UClient_Base::log;
UFileConfig*   UClient_Base::cfg;
const UString* UClient_Base::queue_dir;

UClient_Base::UClient_Base(UFileConfig* pcfg) : response(U_CAPACITY), buffer(U_CAPACITY), host_port(100U)
{
   U_TRACE_REGISTER_OBJECT(0, UClient_Base, "%p", pcfg)

   if (u_hostname_len == 0)
      {
      u_init_ulib_hostname();
      u_init_ulib_username();
      }

   socket    = 0;
   port      = verify_mode = iovcnt = 0;
   timeoutMS = U_TIMEOUT_MS;

   (void) memset(iov, 0, sizeof(struct iovec) * 6);

   if (pcfg)
      {
      if (cfg == 0)
         {
         cfg = pcfg;

         cfg->load();
         }

      if (cfg->empty() == false) loadConfigParam();
      }
}

void UClient_Base::closeLog()
{
   U_TRACE(0, "UClient_Base::closeLog()")

   if (log &&
       log_shared_with_server == false)
      {
      u_unatexit(&ULog::close); // unregister function of close at exit()...
                  ULog::close();
      }
}

UClient_Base::~UClient_Base()
{
   U_TRACE_UNREGISTER_OBJECT(0, UClient_Base)

   U_INTERNAL_ASSERT_POINTER(socket)

   if (log &&
       log_shared_with_server == false)
      {
      delete log;
      }

   delete socket;

#ifdef DEBUG
   UStringRep::check_dead_of_source_string_with_child_alive = false;

        uri.clear(); // uri can depend on url...
        url.clear(); // url can depend on response... (Location: xxx)
   response.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE... (response may be substr of buffer)
#endif
}

#ifdef USE_LIBSSL
void UClient_Base::setSSLContext()
{
   U_TRACE(0, "UClient_Base::setSSLContext()")

   socket = U_NEW(USSLSocket(bIPv6, 0, false));

   U_ASSERT(((USSLSocket*)socket)->isSSL())

   if (cfg) ((USSLSocket*)socket)->ciphersuite_model = cfg->readLong(U_CONSTANT_TO_PARAM("CIPHER_SUITE"));

   // Load our certificate

   U_INTERNAL_ASSERT(  ca_file.isNullTerminated())
   U_INTERNAL_ASSERT(  ca_path.isNullTerminated())
   U_INTERNAL_ASSERT( key_file.isNullTerminated())
   U_INTERNAL_ASSERT( password.isNullTerminated())
   U_INTERNAL_ASSERT(cert_file.isNullTerminated())

   if (((USSLSocket*)socket)->setContext(0, cert_file.data(), key_file.data(), password.data(),
                                              ca_file.data(),  ca_path.data(),    verify_mode) == false)
      {
      U_ERROR("SSL: client setContext() failed");
      }

   U_MESSAGE("SSL: client use configuration model: %s, protocol list: %s",
               ((USSLSocket*)socket)->getConfigurationModel(), ((USSLSocket*)socket)->getProtocolList());
}
#endif

void UClient_Base::clearData()
{
   U_TRACE(0, "UClient_Base::clearData()")

     buffer.setEmpty();
   response.setEmpty();
}

bool UClient_Base::setHostPort(const UString& host, unsigned int _port)
{
   U_TRACE(0, "UClient_Base::setHostPort(%.*S,%u)", U_STRING_TO_TRACE(host), _port)

   U_INTERNAL_ASSERT(host)

   bool host_differs = (host  != server),
        port_differs = (_port != port);

   U_INTERNAL_DUMP("host_port = %.*S host_differs = %b port_differs = %b", U_STRING_TO_TRACE(host_port), host_differs, port_differs)

   server = host.copy(); // NB: we must not depend on url...
   port   = _port;

   // If the URL contains a port, then add that to the Host header

   if (host_differs ||
       port_differs)
      {
      host_port.replace(host);

      if (_port &&
          _port != 80)
         {
#     ifdef USE_LIBSSL
         if (_port == 443  &&
             url.isHTTPS() &&
             (socket == 0  ||
              socket->isSSL(true)))
            {
            U_INTERNAL_DUMP("host_port = %.*S", U_STRING_TO_TRACE(host_port))

            U_RETURN(true);
            }
#     endif
         (void) host_port.push_back(':');

         uint32_t sz = host_port.size();

         host_port.size_adjust(sz + u_num2str32(host_port.c_pointer(sz), _port));
         }

      U_INTERNAL_DUMP("host_port = %.*S", U_STRING_TO_TRACE(host_port))

      U_RETURN(true);
      }

   U_RETURN(false);
}

void UClient_Base::setLogShared()
{
   U_TRACE(0, "UClient_Base::setLogShared()")

   U_INTERNAL_ASSERT_POINTER(UServer_Base::log)

   log                    = UServer_Base::log;
   log_shared_with_server = true;
}

void UClient_Base::loadConfigParam()
{
   U_TRACE(0, "UClient_Base::loadConfigParam()")

   U_INTERNAL_ASSERT_POINTER(cfg)

   // --------------------------------------------------------------------------------------------------------------------------------------
   // client - configuration parameters
   // --------------------------------------------------------------------------------------------------------------------------------------
   // SOCKET_NAME   name file for the listening socket
   //
   // ENABLE_IPV6   flag to indicate use of ipv6
   // SERVER        host name or ip address for server
   // PORT          port number for the server
   //
   // PID_FILE      write pid on file indicated
   // RES_TIMEOUT   timeout for response from server
   //
   // LOG_FILE      locations for file log
   // LOG_FILE_SZ   memory size for file log
   //
   // CERT_FILE     certificate of client
   // KEY_FILE      private key of client
   // PASSWORD      password for private key of client
   // CA_FILE       locations of trusted CA certificates used in the verification
   // CA_PATH       locations of trusted CA certificates used in the verification
   // VERIFY_MODE   mode of verification (SSL_VERIFY_NONE=0, SSL_VERIFY_PEER=1, SSL_VERIFY_FAIL_IF_NO_PEER_CERT=2, SSL_VERIFY_CLIENT_ONCE=4)
   // CIPHER_SUITE  cipher suite model (Intermediate=0, Modern=1, Old=2)
   // --------------------------------------------------------------------------------------------------------------------------------------

   ca_file   = (*cfg)[*UString::str_CA_FILE];
   ca_path   = (*cfg)[*UString::str_CA_PATH];
   key_file  = (*cfg)[*UString::str_KEY_FILE];
   password  = (*cfg)[*UString::str_PASSWORD];
   cert_file = (*cfg)[*UString::str_CERT_FILE];

   // write pid on file...

   UString x = (*cfg)[*UString::str_PID_FILE];

   if (x) (void) UFile::writeTo(x, UString(u_pid_str, u_pid_str_len));

#ifdef U_LOG_ENABLE
   x = (*cfg)[*UString::str_LOG_FILE];

   if (x)
      {
      if (UServer_Base::isLog())
         {
         U_ASSERT_EQUALS(x, UServer_Base::log->getPath())

         setLogShared();
         }
      else
         {
         log = U_NEW(ULog(x, cfg->readLong(*UString::str_LOG_FILE_SZ)));

         u_atexit(&ULog::close); // register function of close at exit()...

         log->setPrefix(U_SERVER_LOG_PREFIX);
         }
      }

   if (log == 0              &&
       UServer_Base::isLog() &&
       isLogSharedWithServer() == false)
      {
      setLogShared();
      }
#endif

#ifdef ENABLE_IPV6
   bIPv6       = cfg->readBoolean(U_CONSTANT_TO_PARAM("ENABLE_IPV6"));
#endif
   verify_mode = cfg->readLong(*UString::str_VERIFY_MODE);

   UString host      = (*cfg)[*UString::str_SERVER],
           name_sock = (*cfg)[*UString::str_SOCKET_NAME];

   if (host ||
       name_sock)
      {
      (void) setHostPort(name_sock.empty() ? host : name_sock, cfg->readLong(*UString::str_PORT));
      }

   UString value = cfg->at(U_CONSTANT_TO_PARAM("RES_TIMEOUT"));

   if (value)
      {
      timeoutMS = value.strtol();

      if (timeoutMS) timeoutMS *= 1000;
      else           timeoutMS  = -1;
      }

   U_INTERNAL_DUMP("timeoutMS = %u", timeoutMS)
}

bool UClient_Base::connect()
{
   U_TRACE(0, "UClient_Base::connect()")

   response.setBuffer(U_CAPACITY);

   if (socket->isConnected()) U_RETURN(true); // Guard against multiple connections

   socket->iRemotePort = 0;

   if (socket->connectServer(server, port, timeoutMS)) U_RETURN(true);

   response.snprintf("Sorry, couldn't connect to server %.*S%R", U_STRING_TO_TRACE(host_port), 0); // NB: the last argument (0) is necessary...

   if (log) ULog::log("%s%.*s", log_shared_with_server ? UServer_Base::mod_name[0] : "", U_STRING_TO_TRACE(response));

   U_RETURN(false);
}

bool UClient_Base::connectServer()
{
   U_TRACE(0, "UClient_Base::connectServer()")

   iovcnt = 0;

   if (isOpen() == false) socket->_socket();

   if (timeoutMS &&
       socket->isBlocking())
      {
      socket->setNonBlocking(); // setting socket to nonblocking
      }

   if (connect()) U_RETURN(true);

   U_RETURN(false);
}

bool UClient_Base::connectServer(const UString& _url)
{
   U_TRACE(0, "UClient_Base::connectServer(%.*S)", U_STRING_TO_TRACE(_url))

   reset();

   if (setUrl(_url) &&
       isOpen())
      {
      socket->close(); // NB: is changed server and/or port to connect...
      }

   // QUEUE MODE

   if (queue_dir)
      {
      U_INTERNAL_ASSERT(*queue_dir)

      char _buffer[U_PATH_MAX];

      (void) u__snprintf(_buffer, sizeof(_buffer), "%.*s/%.*s.%4D", U_STRING_TO_TRACE(*queue_dir), U_STRING_TO_TRACE(host_port));

      queue_fd = UFile::creat(_buffer);

      if (queue_fd == -1) U_RETURN(false);

      U_RETURN(true);
      }

   if (connectServer()) U_RETURN(true);

   U_RETURN(false);
}

bool UClient_Base::remoteIPAddress(UIPAddress& addr)
{
   U_TRACE(0, "UClient_Base::::remoteIPAddress(%p)", &addr)

   if (socket->iRemotePort)
      {
      addr = socket->cRemoteAddress;

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UClient_Base::setUrl(const UString& location)
{
   U_TRACE(0, "UClient_Base::setUrl(%.*S)", U_STRING_TO_TRACE(location))

   U_INTERNAL_ASSERT(location)

   // check we've been passed a absolute URL

   if (u_isUrlScheme(U_STRING_TO_PARAM(location)) == 0)
      {
      char* p;
      char* ptr;
      char* dest;
      uint32_t len;
      char buf[U_PATH_MAX];

      const char*  src =       uri.data();
      const char* _end = src + uri.size();

      U_INTERNAL_DUMP("uri = %.*S", U_STRING_TO_TRACE(uri))

      ptr = dest = buf;

      while (src < _end)
         {
         p = (char*) memchr(src, '/', _end - src);

         if (p == 0) break;

         len = p - src + 1;

         U_INTERNAL_DUMP("segment = %.*S", len, src)

         U_MEMCPY(dest, src, len);

         src   = p + 1;
         dest += len;
         }

      len = location.size();

      U_MEMCPY(dest, location.data(), len);

      (void) uri.replace(buf, dest - ptr + len);

      U_INTERNAL_DUMP("uri = %.*S", U_STRING_TO_TRACE(uri))

      U_RETURN(false);
      }

   url.set(location);

   if (socket->isSSL()) socket->setSSLActive(url.isHTTPS());

   uri = url.getPathAndQuery();

   U_INTERNAL_DUMP("uri = %.*S", U_STRING_TO_TRACE(uri))

   // NB: return if it has modified host or port...

   bool bchange = setHostPort(url.getHost(), url.getPort());

   U_RETURN(bchange);
}

bool UClient_Base::sendRequest(const UString& req, bool bread_response)
{
   U_TRACE(0, "UClient_Base::sendRequest(%.*S,%b)", U_STRING_TO_TRACE(req), bread_response)

   iovcnt = 1;

   iov[0].iov_base = (caddr_t)req.data();
   iov[0].iov_len  =          req.size();

   (void) U_SYSCALL(memset, "%p,%d,%u", iov+1, 0, sizeof(struct iovec) * 5);

   U_INTERNAL_ASSERT_EQUALS(iov[1].iov_len, 0)
   U_INTERNAL_ASSERT_EQUALS(iov[2].iov_len, 0)
   U_INTERNAL_ASSERT_EQUALS(iov[3].iov_len, 0)
   U_INTERNAL_ASSERT_EQUALS(iov[4].iov_len, 0)
   U_INTERNAL_ASSERT_EQUALS(iov[5].iov_len, 0)

   if (sendRequest(bread_response)) U_RETURN(true);

   U_RETURN(false);
}

bool UClient_Base::sendRequest(bool bread_response)
{
   U_TRACE(0, "UClient_Base::sendRequest(%b)", bread_response)

   U_INTERNAL_ASSERT_RANGE(1,iovcnt,6)

   // QUEUE MODE

   if (queue_fd != -1)
      {
      (void) UFile::writev(queue_fd, iov, iovcnt);

      UFile::close(queue_fd);
                   queue_fd = -1;

      U_RETURN(false);
      }

   bool ko;
   int ncount = 0, counter = 0;

   for (int i = 0; i < iovcnt; ++i) ncount += iov[i].iov_len;

   const char* name = (log_shared_with_server ? UServer_Base::mod_name[0] : "");

resend:
   if (connect())
      {
      ko = (USocketExt::writev(socket, iov, iovcnt, ncount, timeoutMS, 1) != ncount);

      if (ko)
         {
         close();

         if (++counter <= 2) goto resend;

         if (log) ULog::log("%serror on sending data to %.*S%R", name, U_STRING_TO_TRACE(host_port), 0); // NB: the last argument (0) is necessary...

         U_RETURN(false);
         }

      if (bread_response                                                        &&
          USocketExt::read(socket, response, U_SINGLE_READ, timeoutMS) == false &&
          isConnected()                                                == false)
         {
         if (++counter <= 2                   &&
             (log_shared_with_server == false || // check for SIGTERM event...
              UServer_Base::flag_loop))
            {
            if (log) errno = 0;

            goto resend;
            }

         if (log)
            {
            ULog::log(iov,                                   name, "request", ncount, "", 0, " to %.*s", U_STRING_TO_TRACE(host_port));
            ULog::log("%serror on reading data from %.*S%R", name,                                       U_STRING_TO_TRACE(host_port), 0); // 0 is necessary
            }

         U_RETURN(false);
         }

      if (log)
         {
                       ULog::log(iov,              name, "request", ncount, "", 0, " to %.*s",   U_STRING_TO_TRACE(host_port));
         if (response) ULog::logResponse(response, name,                           " from %.*s", U_STRING_TO_TRACE(host_port));
         }

      reset();

      U_RETURN(true);
      }

   U_RETURN(false);
}

// read data response

bool UClient_Base::readResponse(uint32_t count)
{
   U_TRACE(0, "UClient_Base::readResponse(%u)", count)

   if (USocketExt::read(socket, response, count, timeoutMS))
      {
      if (log &&
          response)
         {
         ULog::logResponse(response, (log_shared_with_server ? UServer_Base::mod_name[0] : ""), " from %.*S", U_STRING_TO_TRACE(host_port));
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}


bool UClient_Base::readHTTPResponse()
{
   U_TRACE(0, "UClient_Base::readHTTPResponse()")

   // read HTTP message data

   clearData();

   if (UHTTP::readHeader(socket, buffer) &&
       UHTTP::findEndHeader(     buffer))
      {
      uint32_t pos = buffer.find(*UString::str_content_length, u_http_info.startHeader, u_http_info.szHeader);

      if (pos != U_NOT_FOUND)
         {
         u_http_info.clength = (uint32_t) strtoul(buffer.c_pointer(pos + UString::str_content_length->size() + 2), 0, 0);

         if (u_http_info.clength == 0) UHTTP::data_chunked = false;

         if (UHTTP::readBody(socket, &buffer, response))
            {
            if (log &&
                response)
               {
               ULog::logResponse(response, (log_shared_with_server ? UServer_Base::mod_name[0] : ""), " from %.*S", U_STRING_TO_TRACE(host_port));
               }

            U_RETURN(true);
            }
         }
      }

   U_RETURN(false);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UClient_Base::dump(bool _reset) const
{
   *UObjectIO::os << "bIPv6                               " << bIPv6                   << '\n'
                  << "port                                " << port                    << '\n'
                  << "timeoutMS                           " << timeoutMS               << '\n'
                  << "verify_mode                         " << verify_mode             << '\n'
                  << "log_shared_with_server              " << log_shared_with_server  << '\n'
                  << "log            (ULog                " << (void*)log              << ")\n"
                  << "uri            (UString             " << (void*)&uri             << ")\n"
                  << "server         (UString             " << (void*)&server          << ")\n"
                  << "ca_file        (UString             " << (void*)&ca_file         << ")\n"
                  << "ca_path        (UString             " << (void*)&ca_path         << ")\n"
                  << "key_file       (UString             " << (void*)&key_file        << ")\n"
                  << "password       (UString             " << (void*)&password        << ")\n"
                  << "cert_file      (UString             " << (void*)&cert_file       << ")\n"
                  << "buffer         (UString             " << (void*)&buffer          << ")\n"
                  << "response       (UString             " << (void*)&response        << ")\n"
                  << "host_port      (UString             " << (void*)&host_port       << ")\n"
                  << "socket         (USocket             " << (void*)socket           << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
