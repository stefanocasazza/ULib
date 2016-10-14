// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    client_image.cpp - Handles accepted TCP/IP connections
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/server/server.h>
#include <ulib/utility/websocket.h>
#include <ulib/internal/error.h>

#ifdef HAVE_SCHED_GETCPU
#  include <sched.h>
#endif
#ifndef U_HTTP2_DISABLE
#  include <ulib/utility/http2.h>
#endif

#ifdef U_SERVER_CHECK_TIME_BETWEEN_REQUEST
#  define U_NUM_CLIENT_THRESHOLD 128
#endif

int           UClientImage_Base::idx;
int           UClientImage_Base::csfd;
int           UClientImage_Base::iovcnt;
iPF           UClientImage_Base::callerHandlerRead = UServer_Base::pluginsHandlerREAD;
bPF           UClientImage_Base::callerHandlerCache = handlerCache;
iPF           UClientImage_Base::callerHandlerRequest = UServer_Base::pluginsHandlerRequest;
vPF           UClientImage_Base::callerHandlerEndRequest = do_nothing;
bool          UClientImage_Base::bIPv6;
char          UClientImage_Base::cbuffer[128];
long          UClientImage_Base::time_run;
long          UClientImage_Base::time_between_request = 10;
bPFpc         UClientImage_Base::callerIsValidMethod = isValidMethod;
bPFpcu        UClientImage_Base::callerIsValidRequest = isValidRequest;
bPFpcu        UClientImage_Base::callerIsValidRequestExt = isValidRequestExt;
uint32_t      UClientImage_Base::resto;
uint32_t      UClientImage_Base::rstart;
uint32_t      UClientImage_Base::ncount;
uint32_t      UClientImage_Base::nrequest;
uint32_t      UClientImage_Base::uri_offset;
uint32_t      UClientImage_Base::size_request;
UString*      UClientImage_Base::body;
UString*      UClientImage_Base::rbuffer;
UString*      UClientImage_Base::wbuffer;
UString*      UClientImage_Base::request;
UString*      UClientImage_Base::request_uri;
UString*      UClientImage_Base::environment;
UTimeVal*     UClientImage_Base::chronometer;
struct iovec  UClientImage_Base::iov_sav[4];
struct iovec  UClientImage_Base::iov_vec[4];
struct iovec* UClientImage_Base::piov;

// NB: these are for ULib Servlet Page (USP) - USP_PRINTF...

UString* UClientImage_Base::_value;
UString* UClientImage_Base::_buffer;
UString* UClientImage_Base::_encoded;

#ifndef U_LOG_DISABLE
int UClientImage_Base::log_request_partial;

void UClientImage_Base::logRequest()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::logRequest()")

   U_INTERNAL_ASSERT(*request)
   U_INTERNAL_ASSERT_POINTER(logbuf)
   U_INTERNAL_ASSERT(UServer_Base::isLog())
   U_INTERNAL_ASSERT(logbuf->isNullTerminated())

   const char* str_partial = "";
   const char* ptr = request->data();
   uint32_t str_partial_len = 0, sz = request->size();
   int32_t u_printf_string_max_length_save = u_printf_string_max_length;

   U_INTERNAL_DUMP("u_printf_string_max_length = %d", u_printf_string_max_length)

#ifndef U_HTTP2_DISABLE
   U_INTERNAL_DUMP("U_http_version = %C", U_http_version)

   if (U_http_version != '2')
#endif
   {
   if (u_printf_string_max_length == -1)
      {
      u_printf_string_max_length = u_findEndHeader1(ptr, sz);

      if (u_printf_string_max_length == -1)
         {
         str_partial         =                 "[partial] ";
         str_partial_len     = U_CONSTANT_SIZE("[partial] ");
         log_request_partial = UEventFd::fd;

         u_printf_string_max_length = U_min(sz,1000);
         }
      else
         {
         if (u_printf_string_max_length < (int)U_CONSTANT_SIZE("GET / HTTP/1.0\r\n\r\n")) u_printf_string_max_length = 128;

         if (log_request_partial == UEventFd::fd)
            {
            str_partial         =                 "[complete] ";
            str_partial_len     = U_CONSTANT_SIZE("[complete] ");
            log_request_partial = 0;
            }
         }

      U_INTERNAL_ASSERT_MAJOR(u_printf_string_max_length, 0)
      }
   }

   U_INTERNAL_DUMP("u_printf_string_max_length = %d U_ClientImage_pipeline = %b", u_printf_string_max_length, U_ClientImage_pipeline)

   ULog::log(U_CONSTANT_TO_PARAM("%sreceived request (%u bytes) %.*s%.*s%#.*S from %v"),
               UServer_Base::mod_name[0], sz,
               (U_ClientImage_pipeline ? U_CONSTANT_SIZE("[pipeline] ") : 0), "[pipeline] ",
               str_partial_len, str_partial,
               sz, ptr, logbuf->rep);

   u_printf_string_max_length = u_printf_string_max_length_save;
}
#endif

UClientImage_Base::UClientImage_Base()
{
   U_TRACE_REGISTER_OBJECT(0, UClientImage_Base, "")

   socket       = 0;
   logbuf       = 0;
   data_pending = 0;

   if (UServer_Base::isLog()) U_NEW(UString, logbuf, UString(200U));

   reset();

   flag.u     = 0;
   last_event = u_now->tv_sec;

   // NB: array are not pointers (virtual table can shift the address of 'this')...

   if (UServer_Base::pClientImage == 0)
      {
      UServer_Base::pClientImage = this;
      UServer_Base::eClientImage = this + UNotifier::max_connection;
      }
}

UClientImage_Base::~UClientImage_Base()
{
   U_TRACE_UNREGISTER_OBJECT(0, UClientImage_Base)

   // NB: array are not pointers (virtual table can shift the address of 'this')...

   delete socket;

   if (logbuf)
      {
      U_CHECK_MEMORY_OBJECT(logbuf->rep)

   // if (logbuf->rep->memory.invariant() == false) logbuf->rep->memory._this = (void*)U_CHECK_MEMORY_SENTINEL;

      delete logbuf;
      }
}

void UClientImage_Base::set()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::set()")

   U_INTERNAL_DUMP("this = %p socket = %p UEventFd::fd = %d", this, socket, UEventFd::fd)

   U_INTERNAL_ASSERT_POINTER(socket)
   U_INTERNAL_ASSERT_POINTER(UServer_Base::socket)

   if (UServer_Base::bipc == false &&
       UServer_Base::socket->isLocalSet())
      {
      socket->cLocalAddress.set(UServer_Base::socket->cLocalAddress);
      }

                                               socket->flags |= O_CLOEXEC;
   if (USocket::accept4_flags & SOCK_NONBLOCK) socket->flags |= O_NONBLOCK;

#ifdef DEBUG
               U_CHECK_MEMORY
               U_CHECK_MEMORY_OBJECT(socket)
   if (logbuf) U_CHECK_MEMORY_OBJECT(logbuf->rep)

   uint32_t index = (this - UServer_Base::pClientImage);

   if (index)
      {
      UClientImage_Base* ptr = UServer_Base::pClientImage + index-1;

      U_CHECK_MEMORY_OBJECT(ptr)
      U_CHECK_MEMORY_OBJECT(ptr->socket)

      if (logbuf)
         {
         U_INTERNAL_DUMP("ptr->logbuf = %p ptr->logbuf->rep = %p ptr->logbuf->rep->memory._this = %p",
                          ptr->logbuf,     ptr->logbuf->rep,     ptr->logbuf->rep->memory._this)

         U_CHECK_MEMORY_OBJECT(ptr->logbuf->rep)

         if (index == (UNotifier::max_connection-1))
            {
            for (index = 0, ptr = UServer_Base::pClientImage; index < UNotifier::max_connection; ++index, ++ptr) (void) ptr->check_memory();
            }
         }
      }
#endif
}
// ------------------------------------------------------------------------

#ifdef DEBUG
bool UClientImage_Base::check_memory()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::check_memory()")

   U_INTERNAL_DUMP("u_check_memory_vector<T>: elem %u of %u", this-UServer_Base::pClientImage, UNotifier::max_connection)

   U_INTERNAL_DUMP("this = %p socket = %p UEventFd::fd = %d", this, socket, UEventFd::fd)

   U_CHECK_MEMORY
   U_CHECK_MEMORY_OBJECT(socket)

   if (logbuf) U_CHECK_MEMORY_OBJECT(logbuf->rep)

   U_RETURN(true);
}
#endif

#ifdef DEBUG
void UClientImage_Base::saveRequestResponse()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::saveRequestResponse()")

# if defined(U_STDCPP_ENABLE) && !defined(U_HTTP2_DISABLE)
   U_INTERNAL_DUMP("U_http_version = %C", U_http_version)

   if (U_http_version == '2')
      {
      U_DUMP_OBJECT_TO_TMP(UHTTP2::pConnection->itable, request)

      return;
      }
#endif

   if (*rbuffer)                  (void) UFile::writeToTmp(U_STRING_TO_PARAM(*rbuffer), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("request.%P"),  0);
   if (U_http_info.nResponseCode) (void) UFile::writeToTmp(iov_sav, 4,                  O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("response.%P"), 0);
}
#endif

void UClientImage_Base::init()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::init()")

   U_INTERNAL_ASSERT_EQUALS(body, 0)
   U_INTERNAL_ASSERT_EQUALS(_value, 0)
   U_INTERNAL_ASSERT_EQUALS(rbuffer, 0)
   U_INTERNAL_ASSERT_EQUALS(wbuffer, 0)
   U_INTERNAL_ASSERT_EQUALS(request, 0)
   U_INTERNAL_ASSERT_EQUALS(_buffer, 0)
   U_INTERNAL_ASSERT_EQUALS(_encoded, 0)
   U_INTERNAL_ASSERT_EQUALS(request_uri, 0)

   U_NEW(UString, body, UString);
   U_NEW(UString, rbuffer, UString(8192));
   U_NEW(UString, wbuffer, UString(U_CAPACITY));
   U_NEW(UString, request, UString);
   U_NEW(UString, request_uri, UString);
   U_NEW(UString, environment, UString);

   // NB: these are for ULib Servlet Page (USP) - USP_PRINTF...

   U_NEW(UString, _value, UString(U_CAPACITY));
   U_NEW(UString, _buffer, UString(U_CAPACITY));
   U_NEW(UString, _encoded, UString(U_CAPACITY));

   U_NEW(UTimeVal, chronometer, UTimeVal);

   chronometer->start();

#ifdef DEBUG
   UError::callerDataDump = saveRequestResponse;
#endif
}

void UClientImage_Base::clear()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::clear()")

   U_INTERNAL_ASSERT_POINTER(body)
   U_INTERNAL_ASSERT_POINTER(wbuffer)
   U_INTERNAL_ASSERT_POINTER(request)
   U_INTERNAL_ASSERT_POINTER(rbuffer)
   U_INTERNAL_ASSERT_POINTER(request_uri)

   if (body)
      {
      delete body;
      delete wbuffer;
      delete request;
      delete rbuffer;
      delete request_uri;
      delete environment;
      delete chronometer;

      // NB: these are for ULib Servlet Page (USP) - USP_PRINTF...

      U_INTERNAL_ASSERT_POINTER(_value)
      U_INTERNAL_ASSERT_POINTER(_buffer)
      U_INTERNAL_ASSERT_POINTER(_encoded)

      delete _value;
      delete _buffer;
      delete _encoded;
      }
}

// Check whether the ip address client ought to be allowed

__pure bool UClientImage_Base::isAllowed(UVector<UIPAllow*>& vallow_IP)
{
   U_TRACE(0, "UClientImage_Base::isAllowed(%p)", &vallow_IP)

   if (UIPAllow::isAllowed(UServer_Base::csocket->remoteIPAddress().getInAddr(), vallow_IP)) U_RETURN(true);

   U_RETURN(false);
}

#ifndef U_CACHE_REQUEST_DISABLE
__pure bool UClientImage_Base::isRequestCacheable()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestCacheable()")

   if ((U_ClientImage_request & NO_CACHE) == 0   &&
        U_ClientImage_request_is_cached == false &&
       U_http_info.startHeader <= sizeof(cbuffer))
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}
#endif

// append on the log the peer certificate of client ("issuer","serial")

bool UClientImage_Base::logCertificate()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::logCertificate()")

   // NB: OpenSSL has already tested the cert validity during SSL handshake and returns a X509 ptr just if the certificate is valid...

#if defined(USE_LIBSSL) && !defined(U_LOG_DISABLE)
   X509* x509 = ((USSLSocket*)socket)->getPeerCertificate();

   if (x509)
      {
      U_INTERNAL_ASSERT_POINTER(logbuf)
      U_INTERNAL_ASSERT(UServer_Base::isLog())

      UCertificate::setForLog(x509, *logbuf);

      U_INTERNAL_ASSERT(logbuf->isNullTerminated())

      U_INTERNAL_DUMP("logbuf = %V", logbuf->rep)

      U_RETURN(true);
      }
#endif

   U_RETURN(false);
}

bool UClientImage_Base::askForClientCertificate()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::askForClientCertificate()")

#ifdef USE_LIBSSL
   U_ASSERT(((USSLSocket*)socket)->isSSL())

   if (((USSLSocket*)socket)->getPeerCertificate() == 0)
      {
      U_SRV_LOG_WITH_ADDR("Ask for a client certificate to");

      if (((USSLSocket*)socket)->askForClientCertificate() == false) U_RETURN(false);
      }

   if (logCertificate()) U_RETURN(true);
#endif

   U_RETURN(false);
}

void UClientImage_Base::setSendfile(int _sfd, uint32_t _start, uint32_t _count)
{
   U_TRACE(0, "UClientImage_Base::setSendfile(%d,%u,%u)", _sfd, _start, _count)

   U_INTERNAL_ASSERT_MAJOR(_count, 0)
   U_INTERNAL_ASSERT_DIFFERS(_sfd, -1)

   UServer_Base::pClientImage->start = _start;
   UServer_Base::pClientImage->count = _count;
   UServer_Base::pClientImage->sfd   = _sfd;
}

// define method VIRTUAL of class UEventFd

void UClientImage_Base::handlerDelete()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::handlerDelete()")

   bool bsocket_open = socket->isOpen();

#if !defined(USE_LIBEVENT) && defined(HAVE_EPOLL_WAIT) && defined(DEBUG)
   if (UNLIKELY(UNotifier::num_connection <= UNotifier::min_connection))
      {
      U_WARNING("handlerDelete(): "
                "UEventFd::fd = %d socket->iSockDesc = %d socket->isOpen() = %b "
                "UNotifier::num_connection = %d UNotifier::min_connection = %d "
                "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B",
                UEventFd::fd, socket->iSockDesc, bsocket_open, UNotifier::num_connection, UNotifier::min_connection, UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask);

      return;
      }
#endif

#ifndef U_LOG_DISABLE
   if (UServer_Base::isLog())
      {
      U_INTERNAL_ASSERT_POINTER(logbuf)

      U_INTERNAL_DUMP("UEventFd::fd = %d logbuf = %V", UEventFd::fd, logbuf->rep)

      char buffer[32];
      uint32_t len = UServer_Base::setNumConnection(buffer);

      ULog::log(U_CONSTANT_TO_PARAM("%s%.6s close connection from %v, %.*s clients still connected"),
                  UServer_Base::mod_name[0], bsocket_open ? "Client" : "Server", logbuf->rep, len, buffer);

#  ifdef DEBUG
      int fd_logbuf = ::strtoul(logbuf->data(), 0, 10);

      if (UNLIKELY(fd_logbuf != UEventFd::fd))
         {
         U_WARNING("handlerDelete(): "
                   "UEventFd::fd = %d socket->iSockDesc = %d "
                   "UNotifier::num_connection = %d UNotifier::min_connection = %d "
                   "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B fd_logbuf = %u",
                   UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
                   UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask, fd_logbuf);
         }
#  endif

      if (log_request_partial == UEventFd::fd) log_request_partial = 0;
      }
#endif

   U_SRV_TOT_CONNECTION--;

   U_INTERNAL_DUMP("tot_connection = %d", U_SRV_TOT_CONNECTION)

#ifdef U_CLASSIC_SUPPORT
   if (UServer_Base::isClassic()) U_EXIT(0);
#endif

#ifndef U_HTTP2_DISABLE
   U_INTERNAL_DUMP("U_http_version = %C", U_http_version)

   if (U_http_version == '2')
      {
      UHTTP2::reset(this - UServer_Base::vClientImage, bsocket_open);

      bsocket_open = socket->isOpen();
      }
#endif

   if (bsocket_open) socket->close();

   --UNotifier::num_connection;

#ifndef U_LOG_DISABLE
   if (UServer_Base::isLog())
      {
      logbuf->setEmpty();

      if (UNotifier::num_connection == UNotifier::min_connection) ULog::log(U_CONSTANT_TO_PARAM("Waiting for connection on port %u"), UServer_Base::port);
      }
#endif

   if (data_pending)
      {
      delete data_pending;
             data_pending = 0;
      }
   else if (isPendingSendfile())
      {
      U_INTERNAL_DUMP("sfd = %d count = %u UEventFd::op_mask = %B U_ClientImage_pclose(this) = %d %B",
                       sfd,     count,     UEventFd::op_mask,     U_ClientImage_pclose(this), U_ClientImage_pclose(this))

      if ((U_ClientImage_pclose(this) & U_CLOSE) != 0)
         {
#     ifdef DEBUG
         if (UNLIKELY(sfd == -1))
            {
            U_ERROR("handlerDelete(): "
                    "UEventFd::fd = %d socket->iSockDesc = %d "
                    "UNotifier::num_connection = %d UNotifier::min_connection = %d "
                    "U_ClientImage_parallelization = %d sfd = %d UEventFd::op_mask = %B",
                    UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
                    U_ClientImage_parallelization, sfd, UEventFd::op_mask);
            }
#     endif

         UFile::close(sfd);
         }

      reset();
      }

   flag.u = 0;

   UEventFd::fd = -1;

   U_INTERNAL_ASSERT_EQUALS(data_pending, 0)
   U_INTERNAL_ASSERT_EQUALS(UEventFd::op_mask, EPOLLIN | EPOLLRDHUP | EPOLLET)
#ifdef HAVE_ACCEPT4
   U_INTERNAL_ASSERT_EQUALS(((USocket::accept4_flags & SOCK_CLOEXEC)  != 0),((socket->flags & O_CLOEXEC)  != 0))
   U_INTERNAL_ASSERT_EQUALS(((USocket::accept4_flags & SOCK_NONBLOCK) != 0),((socket->flags & O_NONBLOCK) != 0))
#endif

#ifdef USE_LIBEVENT
   if (UEventFd::pevent)
      {
      U_INTERNAL_DUMP("UEventFd::pevent = %p", UEventFd::pevent)

      UDispatcher::del(pevent);
                delete pevent;
                       pevent = 0;
      }
#endif
}

int UClientImage_Base::handlerTimeout()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::handlerTimeout()")

#if !defined(USE_LIBEVENT) && defined(HAVE_EPOLL_WAIT) && defined(DEBUG)
   if (UNLIKELY(socket->iSockDesc == -1))
      {
      U_WARNING("handlerTimeout(): "
                "UEventFd::fd = %d socket->iSockDesc = %d "
                "UNotifier::num_connection = %d UNotifier::min_connection = %d "
                "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B",
                UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
                UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask);

      U_RETURN(U_NOTIFIER_OK);
      }

   U_INTERNAL_ASSERT_EQUALS(socket->iSockDesc, UEventFd::fd)
#endif

   U_INTERNAL_DUMP("U_ClientImage_idle(this) = %d %B", U_ClientImage_idle(this), U_ClientImage_idle(this))

   if (U_ClientImage_idle(this) != U_YES) // U_YES = 0x0001
      {
      // NB: maybe we have some more data to read...

      if (UNotifier::waitForRead(socket->iSockDesc, 0) == 1) U_RETURN(U_NOTIFIER_OK);

      socket->iState = USocket::TIMEOUT;

      U_RETURN(U_NOTIFIER_DELETE);
      }

   U_RETURN(U_NOTIFIER_OK);
}

const char* UClientImage_Base::getRequestUri(uint32_t& sz)
{
   U_TRACE(0, "UClientImage_Base::getRequestUri(%p)", &sz)

   U_INTERNAL_DUMP("U_http_is_request_nostat = %b", U_http_is_request_nostat)

   const char* ptr;

#ifdef U_ALIAS
   if (*request_uri)
      {
#  ifndef U_CACHE_REQUEST_DISABLE
      U_INTERNAL_ASSERT_EQUALS(U_ClientImage_request_is_cached, false)
#  endif

      sz  = request_uri->size();
      ptr = request_uri->data();

      U_INTERNAL_DUMP("request_uri(%u) = %.*S", sz, sz, ptr)
      }
   else
#endif
   {
   sz  = U_http_info.uri_len;
   ptr = U_http_info.uri;

   U_INTERNAL_DUMP("U_http_info.uri(%u) = %.*S", sz, sz, ptr)
   }

   return ptr;
}

#ifdef U_CACHE_REQUEST_DISABLE
#  define U_IOV_TO_SAVE  sizeof(struct iovec)
#else
#  define U_IOV_TO_SAVE (sizeof(struct iovec) * 4)
#endif

bool UClientImage_Base::startRequest()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::startRequest()")

#ifdef U_SERVER_CHECK_TIME_BETWEEN_REQUEST
   long time_elapsed = chronometer->restart();

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b time_elapsed = %ld time_run = %ld U_ClientImage_request_is_cached = %b csfd = %d",
                    U_ClientImage_pipeline,     time_elapsed,      time_run,      U_ClientImage_request_is_cached,     csfd)

   if (U_ClientImage_pipeline == false &&
       U_ClientImage_parallelization == 0)
      {
      time_between_request = time_elapsed;

#  ifdef USE_LIBSSL
      if (UServer_Base::bssl) U_RETURN(false);
#  endif
#  ifndef U_CACHE_REQUEST_DISABLE
      if (U_ClientImage_request_is_cached) U_RETURN(false);
#  endif

      if ((time_run - time_between_request) > 10L)
         {
         U_DEBUG("UClientImage_Base::startRequest(): time_between_request(%ld) < time_run(%ld) - isParallelizationGoingToStart(%u) = %b request = %V",
                     time_between_request, time_run, U_NUM_CLIENT_THRESHOLD, UServer_Base::isParallelizationGoingToStart(U_NUM_CLIENT_THRESHOLD), request->rep)

         if (U_http_info.startHeader > 2 &&
             UServer_Base::isParallelizationGoingToStart(U_NUM_CLIENT_THRESHOLD))
            {
            U_INTERNAL_DUMP("U_ClientImage_request_is_cached = %b", U_ClientImage_request_is_cached)

            if (U_ClientImage_request_is_cached == false)
               {
               U_ClientImage_advise_for_parallelization = true;

               UClientImage_Base::setRequestToCache();
               }

            U_RETURN(true);
            }
         }
      }
#endif

   U_RETURN(false);
}

void UClientImage_Base::endRequest()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::endRequest()")

#ifdef U_SERVER_CHECK_TIME_BETWEEN_REQUEST
   time_run = chronometer->stop();

# ifdef DEBUG
   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b time_between_request = %ld time_run = %ld U_ClientImage_request_is_cached = %b",
                    U_ClientImage_pipeline,     time_between_request,      time_run,      U_ClientImage_request_is_cached)

   if ((time_run - time_between_request) > 10L)
      {
      U_DEBUG("UClientImage_Base::endRequest(): time_between_request(%ld) < time_run(%ld) - request = %V", time_between_request, time_run, request->rep)
      }
# endif
#endif

   callerHandlerEndRequest();

   U_http_method_type = 0; // NB: this mark the end of http request processing...

#if defined(DEBUG) && !defined(U_LOG_DISABLE)
   if (UServer_Base::isLog())
      {
      uint32_t sz = 0;
      const char* ptr;

#  ifndef U_CACHE_REQUEST_DISABLE
      if (U_ClientImage_request_is_cached)
         {
         U_INTERNAL_ASSERT_RANGE(1,uri_offset,64)

         ptr = cbuffer; // request->c_pointer(uri_offset);

#     ifdef U_ALIAS
         sz = U_http_info.startHeader;
#     else
         sz = U_http_info.uri_len;
#     endif
         }
      else
#  endif
      {
      ptr = getRequestUri(sz);
      }

      // NB: URI requested can be URL encoded (ex: vuoto%2Etxt) so we cannot use snprintf()...

      char buffer1[256];
      char* ptr1 = buffer1;

      U_MEMCPY(ptr1, "request \"", U_CONSTANT_SIZE("request \""));
               ptr1 +=             U_CONSTANT_SIZE("request \"");

      if (sz)
         {
         U_INTERNAL_DUMP("sz = %u", sz)

         if (sz > (sizeof(buffer1)-32)) sz = sizeof(buffer1)-32;

         U_MEMCPY(ptr1, ptr, sz);
                  ptr1 +=    sz;
         }

      U_MEMCPY(ptr1, "\" run in ", U_CONSTANT_SIZE("\" run in "));
               ptr1 +=             U_CONSTANT_SIZE("\" run in ");

      if (time_run > 0L) ptr1 += u__snprintf(ptr1, sizeof(buffer1)-(ptr1-buffer1), U_CONSTANT_TO_PARAM("%ld ms"), time_run);
      else               ptr1 += u__snprintf(ptr1, sizeof(buffer1)-(ptr1-buffer1), U_CONSTANT_TO_PARAM( "%g ms"), chronometer->getTimeElapsed());

#  ifndef U_SERVER_CAPTIVE_PORTAL
      if (UServer_Base::csocket->isOpen())
         {
         uint32_t len = 0;
         int cpu = U_SYSCALL_NO_PARAM(sched_getcpu), scpu = -1;

#     ifdef SO_INCOMING_CPU
         if (USocket::bincoming_cpu)
            {
            len = sizeof(socklen_t);

            (void) UServer_Base::csocket->getSockOpt(SOL_SOCKET, SO_INCOMING_CPU, (void*)&scpu, len);

            len = (USocket::incoming_cpu == scpu ? 0 : U_CONSTANT_SIZE(" [DIFFER]"));
            }
#     endif

         U_INTERNAL_DUMP("USocket::incoming_cpu = %d USocket::bincoming_cpu = %b sched cpu = %d socket cpu = %d", USocket::incoming_cpu, USocket::bincoming_cpu, cpu, scpu)

         if (len) ptr1 += u__snprintf(ptr1, sizeof(buffer1)-(ptr1-buffer1), U_CONSTANT_TO_PARAM(", CPU: %d sched(%d) socket(%d)%.*s"), USocket::incoming_cpu, cpu, scpu, len, " [DIFFER]");
         }
#  endif

      U_INTERNAL_ASSERT_MINOR((ptrdiff_t)(ptr1-buffer1), (ptrdiff_t)sizeof(buffer1))

      ULog::write(buffer1, ptr1-buffer1);
      }
#endif

#ifdef U_ALIAS
   request_uri->clear();
#endif

   U_MEMCPY(iov_vec, iov_sav, U_IOV_TO_SAVE);
}

void UClientImage_Base::manageReadBufferResize(uint32_t n)
{
   U_TRACE(0, "UClientImage_Base::manageReadBufferResize(%u)", n)

   U_DUMP("U_ClientImage_pipeline = %b size_request = %u rbuffer->size() = %u rbuffer->capacity() = %u request->size() = %u rstart = %u",
           U_ClientImage_pipeline,     size_request,     rbuffer->size(),     rbuffer->capacity(),     request->size(),     rstart)

#ifndef U_HTTP2_DISABLE
   U_INTERNAL_DUMP("U_http_version = %C", U_http_version)

   if (U_http_version == '2')
      {
      if (rstart)
         {
         rbuffer->moveToBeginDataInBuffer(rstart);
                                          rstart = 0;
         }

      UString::_reserve(*rbuffer, n);

      return;
      }
#endif

   ptrdiff_t diff;
   const char* ptr;

   request->clear();

   if (U_ClientImage_pipeline)
      {
      U_INTERNAL_ASSERT_MAJOR(rstart, 0)

      U_ClientImage_pipeline = false;

      if (rbuffer->capacity() <= n) goto next1;

      rbuffer->moveToBeginDataInBuffer(rstart);

      if (U_http_method_type)
         {
         diff = -(ptrdiff_t)rstart;

         goto next2;
         }
      }
   else
      {
      U_INTERNAL_ASSERT_MAJOR(n, 0)
next1:
      ptr = rbuffer->data();

      UString::_reserve(*rbuffer, n);

      if (U_http_method_type)
         {
         diff = rbuffer->data() - ptr;
next2:
         U_INTERNAL_DUMP("diff = %d", diff)

         U_INTERNAL_ASSERT_POINTER(U_http_info.uri)

                                    U_http_info.uri    += diff;
         if (U_http_info.query_len) U_http_info.query  += diff;

         U_INTERNAL_DUMP("uri             = %.*S", U_HTTP_URI_TO_TRACE)
         U_INTERNAL_DUMP("query           = %.*S", U_HTTP_QUERY_TO_TRACE)

         U_INTERNAL_ASSERT_DIFFERS(U_http_info.uri[0], 0)

         if (U_http_host_len)             U_http_info.host            += diff;
         if (U_http_range_len)            U_http_info.range           += diff;
         if (U_http_accept_len)           U_http_info.accept          += diff;
         if (U_http_ip_client_len)        U_http_info.ip_client       += diff;
         if (U_http_info.cookie_len)      U_http_info.cookie          += diff;
         if (U_http_info.referer_len)     U_http_info.referer         += diff;
         if (U_http_content_type_len)     U_http_info.content_type    += diff;
         if (U_http_info.user_agent_len)  U_http_info.user_agent      += diff;
         if (U_http_accept_language_len)  U_http_info.accept_language += diff;

#     ifndef U_HTTP2_DISABLE
         if (U_http2_settings_len)     UHTTP2::upgrade_settings += diff;
#     endif
         if (U_http_websocket_len) UWebSocket::upgrade_settings += diff;

         U_INTERNAL_DUMP("host            = %.*S", U_HTTP_HOST_TO_TRACE)
         U_INTERNAL_DUMP("vhost           = %.*S", U_HTTP_VHOST_TO_TRACE)
         U_INTERNAL_DUMP("range           = %.*S", U_HTTP_RANGE_TO_TRACE)
         U_INTERNAL_DUMP("ctype           = %.*S", U_HTTP_CTYPE_TO_TRACE)
         U_INTERNAL_DUMP("cookie          = %.*S", U_HTTP_COOKIE_TO_TRACE)
         U_INTERNAL_DUMP("accept          = %.*S", U_HTTP_ACCEPT_TO_TRACE)
         U_INTERNAL_DUMP("referer         = %.*S", U_HTTP_REFERER_TO_TRACE)
         U_INTERNAL_DUMP("ip_client       = %.*S", U_HTTP_IP_CLIENT_TO_TRACE)
         U_INTERNAL_DUMP("user_agent      = %.*S", U_HTTP_USER_AGENT_TO_TRACE)
         U_INTERNAL_DUMP("accept_language = %.*S", U_HTTP_ACCEPT_LANGUAGE_TO_TRACE)
         }
      }

   *request = *rbuffer;

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_data_missing = %b", U_ClientImage_pipeline, U_ClientImage_data_missing)
}

void UClientImage_Base::resetReadBuffer()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::resetReadBuffer()")

   request->clear();

   U_INTERNAL_DUMP("rstart = %u size_request = %u", rstart, size_request)

   if (rstart)
      {
      rbuffer->moveToBeginDataInBuffer(rstart);
                                       rstart = 0;

      *request = *rbuffer;
      }

   U_ClientImage_pipeline = false;
}

void UClientImage_Base::prepareForRead()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::prepareForRead()")

   U_clientimage_flag.u = 0; // NB: U_ClientImage_parallelization is reset by this...

#ifdef U_CLASSIC_SUPPORT
   if (UServer_Base::isClassic())
      {
      U_ASSERT(UServer_Base::proc->child())

      U_ClientImage_parallelization = U_PARALLELIZATION_CHILD;
      }
#endif

   // NB: we need to check if we were called from UServer_Base::handlerRead or directly from UNotifier...

   U_INTERNAL_DUMP("UEventFd::fd = %d socket->iSockDesc = %d", UEventFd::fd, socket->iSockDesc)

   if (UEventFd::fd == socket->iSockDesc)
      {
      // NB: called directly from UNotifier...

      U_INTERNAL_ASSERT_DIFFERS(UEventFd::fd, -1)

      UServer_Base::csocket            = socket;
      UServer_Base::pClientImage       = this;
      UServer_Base::client_address     = socket->cRemoteAddress.pcStrAddress;
      UServer_Base::client_address_len = u__strlen(UServer_Base::client_address, __PRETTY_FUNCTION__);

      U_INTERNAL_DUMP("UServer_Base::client_address = %.*S", U_CLIENT_ADDRESS_TO_TRACE)
      }
   else
      {
      // NB: called from UServer_Base::handlerRead...

      U_INTERNAL_DUMP("UServer_Base::csocket = %p socket = %p UServer_Base::pClientImage = %p this = %p", UServer_Base::csocket, socket, UServer_Base::pClientImage, this)

      U_INTERNAL_ASSERT_DIFFERS(socket->iSockDesc, -1)
      U_INTERNAL_ASSERT_EQUALS(UServer_Base::csocket, socket)
      U_INTERNAL_ASSERT_EQUALS(UServer_Base::pClientImage, this)
      U_INTERNAL_ASSERT_MAJOR(UServer_Base::client_address_len, 0)

      UEventFd::fd = socket->iSockDesc;
      }

#ifdef U_THROTTLING_SUPPORT
   UServer_Base::initThrottlingClient();
#endif
}

bool UClientImage_Base::genericRead()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::genericRead()")

#ifdef DEBUG
   if (UNLIKELY(socket->iSockDesc == -1))
      {
      U_WARNING("genericRead(): "
                "UEventFd::fd = %d socket->iSockDesc = %d "
                "UNotifier::num_connection = %d UNotifier::min_connection = %d "
                "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B",
                UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
                UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask);

      U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;

      U_RETURN(false);
      }
#endif

   U_INTERNAL_ASSERT_EQUALS(socket->iSockDesc, UEventFd::fd)

#ifdef U_SERVER_CHECK_TIME_BETWEEN_REQUEST
   bool advise_for_parallelization = startRequest();
#endif

   request->clear(); // reset buffer before read

   U_INTERNAL_DUMP("rbuffer(%u) = %V", rbuffer->size(), rbuffer->rep)

   // NB: rbuffer string can be referenced more than one (often if U_SUBSTR_INC_REF is defined)...

   if (rbuffer->uniq()) rbuffer->rep->_length = 0; 
   else                 rbuffer->_set(UStringRep::create(0U, U_CAPACITY, 0));

   if (data_pending)
      {
      U_INTERNAL_DUMP("data_pending(%u) = %V", data_pending->size(), data_pending->rep)

      U_MEMCPY(rbuffer->data(), data_pending->data(), rbuffer->rep->_length = data_pending->size());
      }

   socket->iState = USocket::CONNECT; // prepare socket before read

   if (USocketExt::read(socket, *rbuffer, U_SINGLE_READ, 0) == false) // NB: timeout == 0 means that we put the socket fd on epoll queue if EAGAIN...
      {
      U_ClientImage_state = (socket->isOpen() ? U_PLUGIN_HANDLER_AGAIN
                                              : U_PLUGIN_HANDLER_ERROR);

      U_RETURN(false);
      }

   if (data_pending)
      {
      if (callerIsValidRequestExt(U_STRING_TO_PARAM(*rbuffer)) == false) // partial valid (not complete)
         {
         (void) data_pending->replace(*rbuffer);

         U_INTERNAL_DUMP("data_pending(%u) = %V", data_pending->size(), data_pending->rep)

         U_ClientImage_state = U_PLUGIN_HANDLER_AGAIN;

         U_RETURN(false);
         }

      delete data_pending;
             data_pending = 0;
      }

#ifdef U_SERVER_CHECK_TIME_BETWEEN_REQUEST
   if (advise_for_parallelization)
      {
      if (checkRequestToCache() == 2 &&
          UServer_Base::startParallelization(U_NUM_CLIENT_THRESHOLD))
         {
         // parent

         U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;

         U_RETURN(false);
         }

      if (U_ClientImage_advise_for_parallelization) U_ClientImage_request_is_cached = false;
      }
#endif

   U_ClientImage_state = U_PLUGIN_HANDLER_GO_ON;

   U_RETURN(true);
}

int UClientImage_Base::handlerRead() // Connection-wide hooks
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::handlerRead()")

   int result;
   uint32_t sz;

   prepareForRead();

start:
   U_INTERNAL_ASSERT_EQUALS(U_ClientImage_pipeline,     false)
   U_INTERNAL_ASSERT_EQUALS(U_ClientImage_data_missing, false)

   U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

   if (genericRead() == false)
      {
      if (U_ClientImage_state == U_PLUGIN_HANDLER_AGAIN &&
          U_ClientImage_parallelization != U_PARALLELIZATION_CHILD)
         {
         U_INTERNAL_ASSERT(socket->isOpen())

#     ifdef DEBUG
         UServer_Base::nread_again++;
#     endif

         U_RETURN(U_NOTIFIER_OK); // NOT BLOCKING...
         }

      goto end;
      }

#ifdef DEBUG
   UServer_Base::nread++;
#endif

   U_INTERNAL_ASSERT(socket->isOpen())

   rstart = 0;

loop:
   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b size_request = %u rstart = %u rbuffer(%u) = %V",
                    U_ClientImage_pipeline,     size_request,     rstart,     rbuffer->size(), rbuffer->rep)

   if (U_ClientImage_pipeline == false) *request = *rbuffer;
   else
      {
pipeline:
      sz = rbuffer->size();

      U_ASSERT_MINOR(rstart, sz)
      U_INTERNAL_ASSERT_MAJOR(rstart, 0)
      U_INTERNAL_ASSERT_EQUALS(request->same(*rbuffer), false)

      sz -= rstart;

      if (size_request <= sz) *request = rbuffer->substr(rstart, sz);
      else
         {
         resetReadBuffer();

         U_INTERNAL_DUMP("U_ClientImage_close = %b U_ClientImage_state = %d %B",
                          U_ClientImage_close,     U_ClientImage_state, U_ClientImage_state)

         U_INTERNAL_ASSERT_EQUALS(U_ClientImage_pipeline, false)

         if (callerIsValidRequestExt(request->data(), sz) == false) U_ClientImage_data_missing = true; // partial valid (not complete)
         }
      }

   U_INTERNAL_ASSERT(request->invariant())

#ifndef U_LOG_DISABLE
   if (logbuf) logRequest();
#endif

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_data_missing = %b", U_ClientImage_pipeline, U_ClientImage_data_missing)

   if (U_ClientImage_data_missing)
      {
dmiss:
      U_INTERNAL_DUMP("U_ClientImage_parallelization = %d", U_ClientImage_parallelization)

      if (U_ClientImage_parallelization == U_PARALLELIZATION_CHILD)
         {
         if (UNotifier::waitForRead(UServer_Base::csocket->iSockDesc, U_TIMEOUT_MS) != 1 ||
             (resetReadBuffer(), USocketExt::read(UServer_Base::csocket, *rbuffer, getCountToRead(), 0)) == false)
            {
            if ((U_ClientImage_state & U_PLUGIN_HANDLER_ERROR) != 0) U_RETURN(U_NOTIFIER_DELETE);

            goto death;
            }

         goto loop;
         }

      U_ClientImage_data_missing = false;

#  ifndef U_HTTP2_DISABLE
      U_INTERNAL_DUMP("U_http_version = %C", U_http_version)

      if (U_http_version == '2')
         {
         U_ClientImage_state = UHTTP2::handlerRequest();

         if (UNLIKELY((U_ClientImage_state & U_PLUGIN_HANDLER_ERROR) != 0)) goto error;

         goto processing;
         }
#  endif

      U_INTERNAL_ASSERT_EQUALS(data_pending, 0)

      U_NEW(UString, data_pending, UString((void*)U_STRING_TO_PARAM(*request)));

      U_INTERNAL_DUMP("data_pending(%u) = %V", data_pending->size(), data_pending->rep)

      U_INTERNAL_ASSERT(socket->isOpen())
      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD)

      U_RETURN(U_NOTIFIER_OK);
      }

#ifndef U_CACHE_REQUEST_DISABLE
   if (U_ClientImage_request_is_cached)
      {
      sz = checkRequestToCache();

      if (sz)
         {
         if (sz == 1) goto dmiss; // partial valid (not complete)

         U_INTERNAL_ASSERT_EQUALS(sz, 2)

         setRequestProcessed();

         goto next;
         }

      csfd                            = -1;
      U_ClientImage_request_is_cached = false;
      }
#endif

   size_request = U_ClientImage_request = 0;

   if (body->isNull() == false) body->clear();

   U_INTERNAL_DUMP("wbuffer(%u) = %V", wbuffer->size(), wbuffer->rep)

   wbuffer->setBuffer(U_CAPACITY); // NB: this string can be referenced more than one (often if U_SUBSTR_INC_REF is defined)...

#ifndef U_HTTP2_DISABLE
   U_INTERNAL_DUMP("U_http_version = %C", U_http_version)

   if (U_http_version == '2')
      {
      U_ASSERT_EQUALS(request->isPrintable(), false)

#  if defined(DEBUG) && !defined(U_LOG_DISABLE)
      U_http_info.uri_len = 0;
#    ifdef U_ALIAS
      request_uri->clear();
#    endif
#  endif

      U_ClientImage_state = UHTTP2::handlerRequest();

      if (UNLIKELY((U_ClientImage_state & U_PLUGIN_HANDLER_ERROR) != 0)) goto error;

      goto processing;
      }
#endif

   U_ClientImage_state = callerHandlerRead();

   U_INTERNAL_DUMP("socket->isClosed() = %b U_http_info.nResponseCode = %u U_ClientImage_close = %b U_ClientImage_state = %d %B",
                    socket->isClosed(),     U_http_info.nResponseCode,     U_ClientImage_close,     U_ClientImage_state, U_ClientImage_state)

   if (UNLIKELY((U_ClientImage_state & U_PLUGIN_HANDLER_ERROR) != 0))
      {
      U_ASSERT(socket->isClosed())

      goto error;
      }

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_data_missing = %b size_request = %u rstart = %u",
                    U_ClientImage_pipeline,     U_ClientImage_data_missing,     size_request,     rstart)

   U_INTERNAL_ASSERT(socket->isOpen())

   if (U_ClientImage_data_missing) goto dmiss;

   if (LIKELY(size_request))
      {
#  if !defined(U_LOG_DISABLE) && !defined(U_COVERITY_FALSE_POSITIVE)
      U_INTERNAL_DUMP("log_request_partial = %u UEventFd::fd = %d", log_request_partial, UEventFd::fd)

      if (log_request_partial == UEventFd::fd) logRequest();
#  endif
#  ifndef U_CACHE_REQUEST_DISABLE
next:
#  endif
      sz = rbuffer->size();

      if (U_ClientImage_pipeline == false)
         {
         const char* ptr1 = rbuffer->c_pointer(size_request);

         // NB: we check if we have a pipeline...

         if (size_request < sz &&
             callerIsValidMethod(ptr1))
            {
            U_INTERNAL_DUMP("U_http_info.nResponseCode = %u U_ClientImage_close = %b U_ClientImage_state = %d %B",
                             U_http_info.nResponseCode,     U_ClientImage_close,     U_ClientImage_state, U_ClientImage_state)

            U_ClientImage_pipeline = true;

#        ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
                 resto = (sz % size_request);
            uint32_t n = (sz / size_request);

            U_INTERNAL_DUMP("n = %u resto = %u size_request = %u", n, resto, size_request)

            if (n > 1                                    &&
                *wbuffer                                 &&
                callerIsValidRequest(ptr1, size_request) &&
                (resto == 0 || callerIsValidMethod(rbuffer->c_pointer(sz-resto))))
               {
               U_INTERNAL_ASSERT_EQUALS(nrequest, 0)

               const char* ptr = rbuffer->data();
               const char* end = ptr + sz;

               while (true)
                  {
                  if (memcmp(ptr, ptr1, size_request) != 0) break;

                  ptr1 += size_request;

                  if (ptr1 >= end)
                     {
                     nrequest = n;

                     goto check;
                     }

                  ptr += size_request;
                  }

               U_INTERNAL_ASSERT_MINOR(ptr1, end)

               nrequest = (rbuffer->distance(ptr1) / size_request);

               if (nrequest == 1) nrequest = 0;
check:
               U_INTERNAL_DUMP("nrequest = %u resto = %u", nrequest, resto)

               U_INTERNAL_ASSERT(nrequest <= n)
               U_INTERNAL_ASSERT_DIFFERS(nrequest, 1)

               if (resto ||
                   nrequest != n)
                  {
                  *request = rbuffer->substr((rstart = (nrequest * size_request)));
                  }

               U_INTERNAL_DUMP("request(%u) = %V", request->size(), request->rep)

               goto write;
               }
#        endif

            *request = rbuffer->substr(0U, (rstart = size_request));
            }
         }
      else
         {
         uint32_t new_rstart = rstart + size_request;

         U_INTERNAL_DUMP("rstart = %u size_request = %u new_rstart = %u sz = %u", rstart, size_request, new_rstart, sz)

         U_INTERNAL_ASSERT(sz >= new_rstart)

         if (sz == new_rstart) U_ClientImage_pipeline = false;
         else
            {
            *request = rbuffer->substr(rstart, size_request);
                                       rstart = new_rstart;
            }
         }
      }

#ifndef U_HTTP2_DISABLE
processing:
#endif
   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b size_request = %u request->size() = %u",
                    U_ClientImage_pipeline,     size_request,     request->size())

   if (isRequestNeedProcessing())
      {
      U_INTERNAL_ASSERT_POINTER(callerHandlerRequest)
      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_PARENT)
      U_INTERNAL_ASSERT_EQUALS(U_ClientImage_state & (U_PLUGIN_HANDLER_AGAIN | U_PLUGIN_HANDLER_ERROR), 0)

      U_ClientImage_state = callerHandlerRequest();

      if (UNLIKELY(socket->isClosed())) goto error;
      }

   U_INTERNAL_DUMP("socket->isClosed() = %b U_http_info.nResponseCode = %u U_ClientImage_close = %b U_ClientImage_state = %d %B",
                    socket->isClosed(),     U_http_info.nResponseCode,     U_ClientImage_close,     U_ClientImage_state, U_ClientImage_state)

   U_INTERNAL_DUMP("wbuffer(%u) = %V", wbuffer->size(), wbuffer->rep)
   U_INTERNAL_DUMP("   body(%u) = %V",    body->size(),    body->rep)

   if (LIKELY(*wbuffer))
      {
      U_INTERNAL_DUMP("U_http_info.nResponseCode = %u count = %u UEventFd::op_mask = %d %B",
                       U_http_info.nResponseCode,     count,     UEventFd::op_mask, UEventFd::op_mask)

      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_PARENT)

      if (count == 0)
         {
#ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
write:
#endif
         U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_data_missing = %b", U_ClientImage_pipeline, U_ClientImage_data_missing)

         result = handlerResponse();

#     ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
         U_INTERNAL_DUMP("nrequest = %u resto = %u U_ClientImage_pipeline = %b U_ClientImage_close = %b rstart = %u",
                          nrequest,     resto,     U_ClientImage_pipeline,     U_ClientImage_close,     rstart)

         if (nrequest)
            {
            nrequest = 0;

            if (resto)
               {
               resto = 0;

               if (result == U_NOTIFIER_DELETE) goto error;

               endRequest();

               goto dmiss;
               }

            if (U_ClientImage_pipeline &&
                (rstart == 0 || resto == 0))
               {
               U_ClientImage_pipeline = false;
               }
            }
#     endif

         if (result == U_NOTIFIER_DELETE) goto error;
         }
      else
         {
         // NB: we are managing a sendfile() request...

         U_INTERNAL_ASSERT_EQUALS(nrequest, 0)
         U_INTERNAL_ASSERT_EQUALS(UEventFd::op_mask, EPOLLIN | EPOLLRDHUP | EPOLLET)

         if (writeResponse() == false ||
             UClientImage_Base::handlerWrite() == U_NOTIFIER_DELETE)
            {
            goto error;
            }
         }
      }

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b", U_ClientImage_pipeline)

   if (LIKELY((U_ClientImage_state & U_PLUGIN_HANDLER_ERROR) == 0))
      {
      if (U_ClientImage_pipeline)
         {
                  endRequest();
         (void) startRequest();

         goto pipeline;
         }
      }
   else
      {
      U_INTERNAL_ASSERT_EQUALS(U_ClientImage_pipeline, false)
error:
      U_INTERNAL_ASSERT_DIFFERS(UEventFd::fd, -1)

      U_ClientImage_close = true;
      }

   endRequest();

#ifdef U_THROTTLING_SUPPORT
   if (uri) UServer_Base::clearThrottling();
#endif

   U_DUMP("U_ClientImage_close = %b UServer_Base::isParallelizationChild() = %b", U_ClientImage_close, UServer_Base::isParallelizationChild())

   if (U_ClientImage_close)
      {
end:  if (U_ClientImage_parallelization == U_PARALLELIZATION_CHILD) goto death;

      U_RETURN(U_NOTIFIER_DELETE);
      }

   // NB: maybe we have some more request to services on the same connection...

   if (U_ClientImage_parallelization == U_PARALLELIZATION_CHILD)
      {
      U_INTERNAL_ASSERT_DIFFERS(socket->iSockDesc, -1)

      if (UNotifier::waitForRead(socket->iSockDesc, U_TIMEOUT_MS) == 1) goto start;

death:
      UServer_Base::endNewChild(); // no return;
      }

   last_event = u_now->tv_sec;

   U_INTERNAL_ASSERT(socket->isOpen())
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD)

   U_INTERNAL_DUMP("request(%u) = %V", request->size(), request->rep);

   U_RETURN(U_NOTIFIER_OK);
}

bool UClientImage_Base::writeResponse()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::writeResponse()")

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_close = %b nrequest = %u", U_ClientImage_pipeline, U_ClientImage_close, nrequest)

   U_INTERNAL_ASSERT(*wbuffer)
   U_INTERNAL_ASSERT(socket->isOpen())
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_PARENT)

   int iBytesWrite;
   uint32_t sz1     = wbuffer->size(),
            sz2     = (U_http_method_type == HTTP_HEAD ? 0 : body->size()),
            msg_len = (U_ClientImage_pipeline ? U_CONSTANT_SIZE("[pipeline] ") : 0);

   iov_vec[2].iov_len  = sz1;
   iov_vec[2].iov_base = (caddr_t)wbuffer->data();
   iov_vec[3].iov_len  = sz2;
   iov_vec[3].iov_base = (caddr_t)body->data();

   ncount = sz1 + sz2;

   if (UNLIKELY(iov_vec[1].iov_len == 0))
      {
      U_INTERNAL_ASSERT_EQUALS(nrequest, 0)

      idx    = 2;
      iovcnt = 2;

      U_SRV_LOG_WITH_ADDR("send response (%u bytes) %.*s%#.*S to", ncount, msg_len, "[pipeline] ", iov_vec[2].iov_len, iov_vec[2].iov_base);
      }
   else
      {
      idx    = 0;
      iovcnt = 4;

      if (U_ClientImage_close &&
          U_ClientImage_pipeline == false)
         {
         iov_vec[1].iov_len += 17+2; // Connection: close\r\n
         }

      ncount += iov_vec[0].iov_len +
                iov_vec[1].iov_len;

      U_MEMCPY(iov_sav, iov_vec, U_IOV_TO_SAVE);

#  if defined(U_LINUX) && defined(ENABLE_THREAD) && defined(U_LOG_DISABLE) && !defined(USE_LIBZ)
      U_INTERNAL_ASSERT_POINTER(u_pthread_time)
      U_INTERNAL_ASSERT_EQUALS(iov_vec[1].iov_base, ULog::ptr_shared_date->date3)
#  else
      U_INTERNAL_ASSERT_EQUALS(iov_vec[1].iov_base, ULog::date.date3)

      ULog::updateDate3();
#  endif

#  ifndef U_LOG_DISABLE
      if (logbuf) ULog::log(iov_vec+idx, UServer_Base::mod_name[0], "response", ncount, "[pipeline] ", msg_len, U_CONSTANT_TO_PARAM(" to %v"), logbuf->rep);
#  endif
      }

#ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
   if (nrequest)
      {
      ncount *= nrequest;

      iBytesWrite = USocketExt::writev(socket, iov_vec+idx, iovcnt, ncount, 0, nrequest);
      }
   else
#endif
   {
   U_INTERNAL_ASSERT_EQUALS(nrequest, 0)

#if defined(USE_LIBSSL) || defined(_MSWINDOWS_)
   iBytesWrite = USocketExt::writev( socket, iov_vec+idx, iovcnt, ncount, U_ClientImage_pipeline ? U_TIMEOUT_MS : 0);
#else
   iBytesWrite = USocketExt::_writev(socket, iov_vec+idx, iovcnt, ncount, U_ClientImage_pipeline ? U_TIMEOUT_MS : 0);
#endif
   }

#ifdef U_THROTTLING_SUPPORT
   if (iBytesWrite > 0) bytes_sent += iBytesWrite;
#endif
#ifdef DEBUG
   if (iBytesWrite > 0) UServer_Base::stats_bytes += iBytesWrite;
#endif

   if (iBytesWrite == (int)ncount) U_RETURN(true);

   if (socket->isClosed()) U_RETURN(false);

   if (iBytesWrite == 0)
      {
      piov = iov_vec+idx;

      U_RETURN(false);
      }

#ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
   if (nrequest)
      {
      idx    = 3;
      iovcnt = 1;

      iov_vec[3].iov_base = u_buffer;
      iov_vec[3].iov_len  = u_buffer_len;
      }
#endif

   bool result = false, bflag = false;

   if (UServer_Base::startParallelization())
      {
      // parent

      socket->close();

      U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;

      goto end;
      }

#ifdef DEBUG
   int i;
   uint32_t sum;
#endif

   bool bopen;

loop:
   U_INTERNAL_ASSERT(socket->isOpen())
   U_INTERNAL_ASSERT_MAJOR(iBytesWrite, 0)

   U_SRV_LOG_WITH_ADDR("sent partial response: (%u bytes of %u)%.*s parallelization(%u,%u) - to",
                        iBytesWrite, ncount, msg_len, " [pipeline]", U_ClientImage_parallelization, U_SRV_CNT_PARALLELIZATION);

   ncount -= iBytesWrite;

   while (iov_vec[idx].iov_len == 0)
      {
      ++idx;
      --iovcnt;

      U_INTERNAL_ASSERT_MINOR(idx, 4)
      U_INTERNAL_ASSERT_MAJOR(iovcnt, 0)
      }

   piov = iov_vec+idx;

#ifdef DEBUG
   for (i = sum = 0; i < iovcnt; ++i) sum += piov[i].iov_len;

   if (sum != ncount)
      {
      U_ERROR("sum = %u ncount = %u iBytesWrite = %u iov_vec[%u].iov_len = %u iovcnt = %u", sum, ncount, iBytesWrite, idx, iov_vec[idx].iov_len, iovcnt);
      }
#endif

#ifdef USE_LIBSSL
   if (UServer_Base::bssl == false)
#endif
   {
#ifdef U_CLIENT_RESPONSE_PARTIAL_WRITE_SUPPORT
   if (U_ClientImage_parallelization != U_PARALLELIZATION_CHILD) goto end;
#endif
   }

#if defined(USE_LIBSSL) || defined(_MSWINDOWS_)
   iBytesWrite = USocketExt::writev( socket, piov, iovcnt, ncount, U_TIMEOUT_MS);
#else
   iBytesWrite = USocketExt::_writev(socket, piov, iovcnt, ncount, U_TIMEOUT_MS);
#endif

#ifdef U_THROTTLING_SUPPORT
   if (iBytesWrite > 0) bytes_sent += iBytesWrite;
#endif
#ifdef DEBUG
   if (iBytesWrite > 0) UServer_Base::stats_bytes += iBytesWrite;
#endif

   if (iBytesWrite != (int)ncount)
      {
      bopen = socket->isOpen();

      if (bopen &&
          iBytesWrite > 0)
         {
         if (UServer_Base::bssl ||
             U_ClientImage_parallelization == U_PARALLELIZATION_CHILD) // NB: we must not have pending write...
            {
            if (bflag == false)
               {
               bflag = true;

               socket->setBlocking();
               }
            }

         goto loop;
         }

      bflag = false;

      U_SRV_LOG_WITH_ADDR("sending partial response: failed - sk %s, (%u bytes of %u)%.*s to", bopen ? "open" : "close", iBytesWrite, ncount, msg_len, " [pipeline]");

      if (bopen) socket->abortive_close();

      goto end;
      }

   result = true;

   U_SRV_LOG_WITH_ADDR("sending partial response: completed (%u bytes of %u)%.*s to", iBytesWrite, ncount, msg_len, " [pipeline]");

end:
#ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
   if (nrequest) u_buffer_len = 0;
#endif

   if (bflag) socket->setNonBlocking(); // restore socket status flags

   U_RETURN(result);
}

void UClientImage_Base::close()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::close()")

   UServer_Base::csocket->close();

   setRequestProcessed();

   U_ClientImage_close = true;

   if (U_ClientImage_pipeline) resetPipeline();
}

void UClientImage_Base::abortive_close()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::abortive_close()")

   setRequestProcessed();

   U_ClientImage_close = true;

   if (U_ClientImage_pipeline) resetPipeline();

   if (UServer_Base::csocket->isOpen()) UServer_Base::csocket->abortive_close();
}

void UClientImage_Base::resetPipeline()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::resetPipeline()")

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_parallelization = %d U_ClientImage_request_is_cached = %b U_ClientImage_close = %b",
                    U_ClientImage_pipeline,     U_ClientImage_parallelization,     U_ClientImage_request_is_cached,     U_ClientImage_close)

   U_INTERNAL_ASSERT(U_ClientImage_pipeline)
   U_INTERNAL_ASSERT(request->same(*rbuffer) == false)

   U_ClientImage_pipeline = false;

#ifndef U_CACHE_REQUEST_DISABLE
   if (U_ClientImage_request_is_cached == false)
#endif
   size_request = 0; // NB: we don't want to process further the read buffer...
}

void UClientImage_Base::prepareForSendfile()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::prepareForSendfile()")

   U_INTERNAL_ASSERT_MAJOR(count, 0)
   U_INTERNAL_ASSERT_DIFFERS(sfd, -1)

   if (U_ClientImage_close)
      {
      U_ClientImage_close        = false;
      U_ClientImage_pclose(this) = U_YES;
      }

   if (U_ClientImage_pipeline) resetPipeline();

   UEventFd::op_mask = EPOLLOUT;

   if (UNotifier::isHandler(UEventFd::fd)) UNotifier::modify(this);

   U_INTERNAL_DUMP("start = %u count = %u", start, count)
}

int UClientImage_Base::handlerResponse()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::handlerResponse()")

   if (writeResponse()) U_RETURN(U_NOTIFIER_OK);

   if (socket->isOpen())
      {
      U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)
      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD) // NB: we must not have pending write...
      U_INTERNAL_ASSERT_EQUALS(UEventFd::op_mask, EPOLLIN | EPOLLRDHUP | EPOLLET)

#  ifndef U_CLIENT_RESPONSE_PARTIAL_WRITE_SUPPORT
      resetPipelineAndSetCloseConnection();
#  else
      sfd = UFile::mkTemp();

      if (sfd == -1)
         {
         U_SRV_LOG("partial write failed: (remain %u bytes) - error on create temporary file - sock_fd %d", ncount, socket->iSockDesc);

         U_RETURN(U_NOTIFIER_DELETE);
         }

      U_INTERNAL_ASSERT_POINTER(piov)

      int iBytesWrite = UFile::writev(sfd, piov, iovcnt);

      if (iBytesWrite != (int)ncount)
         {
         U_SRV_LOG("partial write failed: (remain %u bytes) - error on write (%d bytes) on temporary file - sock_fd %d sfd %d", ncount, iBytesWrite, socket->iSockDesc, sfd);

         UFile::close(sfd);
                      sfd = -1;

         U_RETURN(U_NOTIFIER_DELETE);
         }

      U_SRV_LOG("partial write: (remain %u bytes) - create temporary file - sock_fd %d sfd %d", ncount, socket->iSockDesc, sfd);

      setPendingSendfile(); // NB: now we have a pending sendfile...

      U_RETURN(U_NOTIFIER_OK);
#  endif
      }

   U_INTERNAL_ASSERT_DIFFERS(UEventFd::fd, -1)

   U_RETURN(U_NOTIFIER_DELETE);
}

int UClientImage_Base::handlerWrite()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::handlerWrite()")

#if !defined(USE_LIBEVENT) && defined(HAVE_EPOLL_WAIT) && defined(DEBUG)
   if (UNLIKELY(count == 0))
      {
      U_WARNING("handlerWrite(): "
                "UEventFd::fd = %d socket->iSockDesc = %d "
                "UNotifier::num_connection = %d UNotifier::min_connection = %d "
                "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B",
                UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
                UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask);

      if (UNotifier::num_connection > UNotifier::min_connection) U_RETURN(U_NOTIFIER_DELETE);

      U_RETURN(U_NOTIFIER_OK);
      }
#endif

   U_INTERNAL_ASSERT_MAJOR(count, 0)
   U_INTERNAL_ASSERT_DIFFERS(sfd, -1)
   U_INTERNAL_ASSERT(socket->isOpen())

   bool bwrite = (UEventFd::op_mask == EPOLLOUT);

   U_INTERNAL_DUMP("bwrite = %b", bwrite)

#ifdef U_THROTTLING_SUPPORT
   if (UServer_Base::checkThrottlingBeforeSend(bwrite) == false) U_RETURN(U_NOTIFIER_OK);
#endif

   off_t offset;
   int iBytesWrite;

write:
   offset      = start;
   iBytesWrite = USocketExt::sendfile(socket, sfd, &offset, count, 0);

#ifdef U_THROTTLING_SUPPORT
   if (iBytesWrite > 0) bytes_sent += iBytesWrite;
#endif
#ifdef DEBUG
   if (iBytesWrite > 0) UServer_Base::stats_bytes += iBytesWrite;
#endif

   if (iBytesWrite == (int)count)
      {
      U_SRV_LOG_WITH_ADDR("sending sendfile response completed (%u bytes of %u) to", iBytesWrite, count);

      if (bwrite)
         {
         UEventFd::op_mask = EPOLLIN | EPOLLRDHUP | EPOLLET;

         UNotifier::modify(this);
         }
#  ifdef DEBUG
      else
         {
         U_INTERNAL_ASSERT_EQUALS(UEventFd::op_mask, EPOLLIN | EPOLLRDHUP | EPOLLET)
         U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_PARENT)
         }
#  endif

      if ((U_ClientImage_pclose(this) & U_CLOSE) != 0) UFile::close(sfd);

      start =
      count =  0;
      sfd   = -1;

      if ((U_ClientImage_pclose(this) & U_YES) != 0) U_RETURN(U_NOTIFIER_DELETE);

      U_RETURN(U_NOTIFIER_OK);
      }

   if (iBytesWrite > 0)
      {
      U_SRV_LOG_WITH_ADDR("sent sendfile partial response (%u bytes of %u) to", iBytesWrite, count);

      start += iBytesWrite;
      count -= iBytesWrite;

      U_INTERNAL_ASSERT_MAJOR(count, 0)

      if (bwrite) U_RETURN(U_NOTIFIER_OK);

      if (U_ClientImage_parallelization == U_PARALLELIZATION_CHILD)
         {
wait:    if (UNotifier::waitForWrite(socket->iSockDesc, U_TIMEOUT_MS) == 1) goto write;

         U_RETURN(U_NOTIFIER_DELETE);
         }

      if (UServer_Base::startParallelization())
         {
         // parent

         U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;

         U_RETURN(U_NOTIFIER_DELETE);
         }

      if (U_ClientImage_parallelization == U_PARALLELIZATION_CHILD) goto wait;

      prepareForSendfile();

      U_RETURN(U_NOTIFIER_OK);
      }

   U_SRV_LOG("sendfile failed - sock_fd %d sfd %d count %u U_ClientImage_pclose(this) %d %B", socket->iSockDesc, sfd, count, U_ClientImage_pclose(this), U_ClientImage_pclose(this));

   if ((U_ClientImage_pclose(this) & U_CLOSE) != 0) UFile::close(sfd);

   start =
   count =  0;
   sfd   = -1;

   U_ClientImage_pclose(this) = 0;

   U_RETURN(U_NOTIFIER_DELETE);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UClientImage_Base::dump(bool _reset) const
{
   *UObjectIO::os << "sfd                                " << sfd                 << '\n'
                  << "start                              " << start               << '\n'
                  << "count                              " << count               << '\n'
                  << "bIPv6                              " << bIPv6               << '\n'
                  << "last_event                         " << last_event          << '\n'
                  << "socket          (USocket           " << (void*)socket       << ")\n"
                  << "body            (UString           " << (void*)body         << ")\n"
                  << "logbuf          (UString           " << (void*)logbuf       << ")\n"
                  << "rbuffer         (UString           " << (void*)rbuffer      << ")\n"
                  << "wbuffer         (UString           " << (void*)wbuffer      << ")\n"
                  << "request         (UString           " << (void*)request      << ")\n"
                  << "environment     (UString           " << (void*)environment  << ")\n"
                  << "data_pending    (UString           " << (void*)data_pending << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
