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
vPFu           UClient_Base::resize_response_buffer;
bool           UClient_Base::bIPv6;
bool           UClient_Base::log_shared_with_server;
ULog*          UClient_Base::log;
USocket*       UClient_Base::csocket;
UFileConfig*   UClient_Base::pcfg;
const UString* UClient_Base::queue_dir;

UClient_Base::UClient_Base(UFileConfig* cfg) : response(U_CAPACITY), buffer(U_CAPACITY), host_port(100U)
{
   U_TRACE_CTOR(0, UClient_Base, "%p", cfg)

   if (u_hostname_len == 0)
      {
      u_init_ulib_hostname();
      u_init_ulib_username();
      }

   socket    = U_NULLPTR;
   port      = verify_mode = iovcnt = 0;
   timeoutMS = U_TIMEOUT_MS;

   (void) memset(iov, 0, sizeof(struct iovec) * 6);

   if (cfg)
      {
      if (pcfg == U_NULLPTR)
         {
         (pcfg = cfg)->load();
         }

      if (pcfg->empty() == false) loadConfigParam();
      }

#ifndef U_LOG_DISABLE
   U_INTERNAL_DUMP("log = %p UServer_Base::isLog() = %b log_shared_with_server = %b", log, UServer_Base::isLog(), log_shared_with_server)

   if (log == U_NULLPTR      &&
       UServer_Base::isLog() &&
       log_shared_with_server == false)
      {
      log                    = UServer_Base::log;
      log_shared_with_server = true;
      }
#endif
}

UClient_Base::~UClient_Base()
{
   U_TRACE_DTOR(0, UClient_Base)

#ifndef U_LOG_DISABLE
   closeLog();
#endif

   U_INTERNAL_ASSERT_POINTER(socket)

   U_DELETE(socket)

#ifdef DEBUG
   UStringRep::check_dead_of_source_string_with_child_alive = false;

        uri.clear(); // uri can depend on url...
        url.clear(); // url can depend on response... (Location: xxx)
   response.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE... (response may be substr of buffer)

   UStringRep::check_dead_of_source_string_with_child_alive = true;
#endif
}

void UClient_Base::closeLog()
{
   U_TRACE_NO_PARAM(0, "UClient_Base::closeLog()")

#ifndef U_LOG_DISABLE
   if (log &&
       log_shared_with_server == false)
      {
      log->closeLog();

      U_DELETE(log)

      log = U_NULLPTR;
      }
#endif
}

#ifdef USE_LIBSSL
void UClient_Base::setSSLContext()
{
   U_TRACE_NO_PARAM(0, "UClient_Base::setSSLContext()")

   U_NEW(USSLSocket, socket, USSLSocket(bIPv6, U_NULLPTR, false));

   U_ASSERT(((USSLSocket*)socket)->isSSL())

   if (pcfg) ((USSLSocket*)socket)->ciphersuite_model = pcfg->readLong(U_CONSTANT_TO_PARAM("CIPHER_SUITE"));

   // Load our certificate

   U_INTERNAL_ASSERT(  ca_file.isNullTerminated())
   U_INTERNAL_ASSERT(  ca_path.isNullTerminated())
   U_INTERNAL_ASSERT( key_file.isNullTerminated())
   U_INTERNAL_ASSERT( password.isNullTerminated())
   U_INTERNAL_ASSERT(cert_file.isNullTerminated())

   if (((USSLSocket*)socket)->setContext(U_NULLPTR, cert_file.data(), key_file.data(), password.data(),
                                                      ca_file.data(),  ca_path.data(),  verify_mode) == false)
      {
      U_ERROR("SSL: client setContext() failed");
      }

   U_MESSAGE("SSL: client use configuration model: %s, protocol list: %s", ((USSLSocket*)socket)->getConfigurationModel(), ((USSLSocket*)socket)->getProtocolList());
}
#endif

bool UClient_Base::setHostPort(const UString& host, unsigned int _port)
{
   U_TRACE(0, "UClient_Base::setHostPort(%V,%u)", host.rep, _port)

   U_INTERNAL_ASSERT(host)

   bool host_differs = (host  != server),
        port_differs = (_port != port);

   U_INTERNAL_DUMP("host_port = %V host_differs = %b port_differs = %b", host_port.rep, host_differs, port_differs)

   server = host;
   port   = _port;

   U_ASSERT_EQUALS(server.isSubStringOf(url.get()), false) // NB: server must not depend on url...

   // If the URL contains a port, then add that to the Host header

   if (host_differs ||
       port_differs)
      {
      (void) host_port.replace(host);

      if (_port &&
          _port != 80)
         {
#     ifdef USE_LIBSSL
         if (_port == 443         &&
             url.isHTTPS()        &&
             (socket == U_NULLPTR ||
              socket->isSSLActive()))
            {
            U_INTERNAL_DUMP("host_port = %V", host_port.rep)

            U_RETURN(true);
            }
#     endif
         (void) host_port.reserve(10U);

         (void) host_port.push_back(':');

         uint32_t sz = host_port.size();

         char* ptr = host_port.c_pointer(sz);

         host_port.size_adjust(sz + u_num2str32(_port, ptr) - ptr);
         }

      U_INTERNAL_DUMP("host_port = %V", host_port.rep)

      U_RETURN(true);
      }

   U_RETURN(false);
}

void UClient_Base::loadConfigParam()
{
   U_TRACE_NO_PARAM(0, "UClient_Base::loadConfigParam()")

   U_INTERNAL_ASSERT_POINTER(pcfg)

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
   // LOG_FILE        locations for file log
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

   ca_file   = pcfg->at(U_CONSTANT_TO_PARAM("CA_FILE"));
   ca_path   = pcfg->at(U_CONSTANT_TO_PARAM("CA_PATH"));
   key_file  = pcfg->at(U_CONSTANT_TO_PARAM("KEY_FILE"));
   password  = pcfg->at(U_CONSTANT_TO_PARAM("PASSWORD"));
   cert_file = pcfg->at(U_CONSTANT_TO_PARAM("CERT_FILE"));

   // write pid on file...

   UString x = pcfg->at(U_CONSTANT_TO_PARAM("PID_FILE"));

   if (x) (void) UFile::writeTo(x, UString(u_pid_str, u_pid_str_len));

#ifndef U_LOG_DISABLE
   x = pcfg->at(U_CONSTANT_TO_PARAM("LOG_FILE"));

   if (x                               &&
       log == U_NULLPTR                &&
       (UServer_Base::isLog() == false ||
        UServer_Base::log->getPath().equal(x) == false))
      {
      U_NEW(ULog, log, ULog(x, pcfg->readLong(U_CONSTANT_TO_PARAM("LOG_FILE_SZ"))));

      log->setPrefix(U_CONSTANT_TO_PARAM(U_SERVER_LOG_PREFIX));
      }
#endif

#ifdef ENABLE_IPV6
   bIPv6       = pcfg->readBoolean(U_CONSTANT_TO_PARAM("ENABLE_IPV6"));
#endif
   verify_mode = pcfg->readLong(U_CONSTANT_TO_PARAM("VERIFY_MODE"));

   UString host      = pcfg->at(U_CONSTANT_TO_PARAM("SERVER")),
           name_sock = pcfg->at(U_CONSTANT_TO_PARAM("SOCKET_NAME"));

   if (host ||
       name_sock)
      {
      (void) setHostPort(name_sock.empty() ? host : name_sock, pcfg->readLong(U_CONSTANT_TO_PARAM("PORT")));
      }

   UString value = pcfg->at(U_CONSTANT_TO_PARAM("RES_TIMEOUT"));

   if (value)
      {
      timeoutMS = value.strtoul();

      if (timeoutMS) timeoutMS *= 1000;
      else           timeoutMS  = -1;
      }

   U_INTERNAL_DUMP("timeoutMS = %u", timeoutMS)
}

bool UClient_Base::connect()
{
   U_TRACE_NO_PARAM(0, "UClient_Base::connect()")

   response.setBuffer(U_CAPACITY);

   if (isConnected()) U_RETURN(true); // Guard against multiple connections

   socket->iRemotePort = 0;

   if (socket->connectServer(server, port, timeoutMS)) U_RETURN(true);

   response.snprintf(U_CONSTANT_TO_PARAM("Sorry, couldn't connect to server %v%R"), host_port.rep, 0); // NB: the last argument (0) is necessary...

   U_CLIENT_LOG("%v", response.rep)

   U_RETURN(false);
}

bool UClient_Base::connectServer(const UString& _url)
{
   U_TRACE(0, "UClient_Base::connectServer(%V)", _url.rep)

   reset();

   if (setUrl(_url)) close(); // NB: is changed server and/or port to connect...

   iovcnt = 0;

   // QUEUE MODE

   if (queue_dir)
      {
      U_INTERNAL_ASSERT(*queue_dir)

      char _buffer[U_PATH_MAX+1];

      (void) u__snprintf(_buffer, U_PATH_MAX, U_CONSTANT_TO_PARAM("%v/%v.%4D"), queue_dir->rep, host_port.rep); // 4D => _millisec

      queue_fd = UFile::creat(_buffer, O_RDWR | O_EXCL, PERM_FILE);

      if (queue_fd != -1) U_RETURN(true);

      U_RETURN(false);
      }

   if (connect()) U_RETURN(true);

   U_RETURN(false);
}

bool UClient_Base::setUrl(const char* str, uint32_t len)
{
   U_TRACE(0, "UClient_Base::setUrl(%.*S,%u)", len, str, len)

   U_INTERNAL_ASSERT_POINTER(str)
   U_INTERNAL_ASSERT_MAJOR(len, 0)

   // we check we've been passed an absolute URL

   if (u_isUrlScheme(str, len) == U_NULLPTR)
      {
      U_INTERNAL_DUMP("uri = %V", uri.rep)

      if (uri.empty()) (void) uri.replace(str, len);
      else
         {
         char* p;
         char* ptr;
         char* dest;
         char buf[U_PATH_MAX];

         const char*  src =       uri.data();
         const char* _end = src + uri.size();

         ptr = dest = buf;

         while (src < _end)
            {
            p = (char*) memchr(src, '/', _end - src);

            if (p == U_NULLPTR) break;

            uint32_t sz = p - src + 1;

            U_INTERNAL_DUMP("segment = %.*S", sz, src)

            U_MEMCPY(dest, src, sz);

            src   = p + 1;
            dest += sz;
            }

         U_MEMCPY(dest, str, len);

         (void) uri.replace(buf, dest - ptr + len);
         }

      U_INTERNAL_DUMP("uri = %V", uri.rep)

      U_RETURN(false);
      }

   url.set(str, len);

   if (socket->isSSL()) socket->setSSLActive(url.isHTTPS() | url.isWSS());

   uri = url.getPathAndQuery();

   U_INTERNAL_DUMP("uri = %V", uri.rep)

   // NB: return if it has modified host or port...

   if (setHostPort(U_STRING_COPY(url.getHost()), url.getPortNumber())) U_RETURN(true); // NB: server must not depend on url...

   U_RETURN(false);
}

bool UClient_Base::sendRequest(bool bread_response)
{
   U_TRACE(0, "UClient_Base::sendRequest(%b)", bread_response)

   U_INTERNAL_ASSERT_RANGE(1,iovcnt,6)

   U_DUMP_IOVEC(iov,iovcnt)

   // QUEUE MODE

   if (queue_fd != -1)
      {
      (void) UFile::writev(queue_fd, iov, iovcnt);

      UFile::close(queue_fd);
                   queue_fd = -1;

      U_RETURN(false);
      }

   bool ok = false;
   uint32_t ncount = 0, counter = 0;

   for (int i = 0; i < iovcnt; ++i) ncount += iov[i].iov_len;

resend:
   if (connect())
      {
      U_CLIENT_LOG_REQUEST(ncount)

      ok = (USocketExt::writev(socket, iov, iovcnt, ncount, timeoutMS) == ncount);

      if (ok == false)
         {
         close();

         if (++counter <= 2)
            {
            U_CLIENT_LOG_WITH_ADDR("failed attempts (%u) to sending data (%u bytes) to", counter, ncount)

            goto resend;
            }

         U_CLIENT_LOG_WITH_ADDR("error on sending data (%u bytes) to", ncount)

         goto end;
         }

      if (bread_response                                                        &&
          USocketExt::read(socket, response, U_SINGLE_READ, timeoutMS) == false &&
          isConnected()                                                == false)
         {
         if (++counter <= 2                   &&
             (log_shared_with_server == false || // check for SIGTERM event...
              UServer_Base::flag_loop))
            {
#        ifndef U_LOG_DISABLE
            if (log) errno = 0;
#        endif

            goto resend;
            }

         U_CLIENT_LOG_WITH_ADDR("error on reading data from")

         goto end;
         }

      U_CLIENT_LOG_RESPONSE()

      reset();
      }

end:
   if (ok &&
       socket->isOpen())
      {
      if (socket->isConnected() == false) socket->iState = USocket::CONNECT;

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UClient_Base::sendRequestAndReadResponse(const UString& header, const UString& body)
{
   U_TRACE(0, "UClient_Base::sendRequestAndReadResponse(%V,%V)", header.rep, body.rep)

   if (body.empty() ||
       body.isSubStringOf(header))
      {
      UClient_Base::prepareRequest(header);
      }
   else
      {
      UClient_Base::request = header;

      UClient_Base::iovcnt = 2;

      UClient_Base::iov[0].iov_base = (caddr_t)header.data();
      UClient_Base::iov[0].iov_len  =          header.size();
      UClient_Base::iov[1].iov_base = (caddr_t)  body.data();
      UClient_Base::iov[1].iov_len  =            body.size();

      (void) U_SYSCALL(memset, "%p,%d,%u", UClient_Base::iov+2, 0, sizeof(struct iovec) * 4);

      U_INTERNAL_ASSERT_EQUALS(UClient_Base::iov[2].iov_len, 0)
      U_INTERNAL_ASSERT_EQUALS(UClient_Base::iov[3].iov_len, 0)
      U_INTERNAL_ASSERT_EQUALS(UClient_Base::iov[4].iov_len, 0)
      U_INTERNAL_ASSERT_EQUALS(UClient_Base::iov[5].iov_len, 0)
      }

   if (UClient_Base::sendRequest(true)) U_RETURN(true);

   U_RETURN(false);
}

// read data response

bool UClient_Base::readResponse(uint32_t count)
{
   U_TRACE(0, "UClient_Base::readResponse(%u)", count)

   U_ASSERT(response.uniq())

   if (USocketExt::read(socket, response, count, timeoutMS))
      {
      U_CLIENT_LOG_RESPONSE()

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UClient_Base::readHTTPResponse()
{
   U_TRACE_NO_PARAM(0, "UClient_Base::readHTTPResponse()")

   clearData();

   // read HTTP message data

   if (UHTTP::readHeaderResponse(socket, buffer))
      {
      if (UHTTP::checkContentLength(buffer))
         {
         if (UHTTP::readBodyResponse(socket, &buffer, response) == false) U_RETURN(false);

         U_CLIENT_LOG_RESPONSE()
         }

      U_RETURN(true);
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
                  << "request        (UString             " << (void*)&request        << ")\n"
                  << "response       (UString             " << (void*)&response        << ")\n"
                  << "host_port      (UString             " << (void*)&host_port       << ")\n"
                  << "socket         (USocket             " << (void*)socket           << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
