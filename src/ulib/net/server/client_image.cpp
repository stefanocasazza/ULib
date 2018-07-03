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

#include <ulib/utility/uhttp.h>
#include <ulib/utility/websocket.h>

#ifndef U_HTTP2_DISABLE
#  include <ulib/utility/http2.h>
#endif
#ifdef U_SERVER_CHECK_TIME_BETWEEN_REQUEST
#  include <ulib/net/client/client.h>
#  define U_NUM_CLIENT_THRESHOLD 128
#endif

#ifdef HAVE_SCHED_GETCPU
#  include <sched.h>
#endif

int          UClientImage_Base::idx;
int          UClientImage_Base::iovcnt;
bool         UClientImage_Base::bIPv6;
bool         UClientImage_Base::bsendGzipBomb;
char         UClientImage_Base::cbuffer[128];
long         UClientImage_Base::time_run;
long         UClientImage_Base::time_between_request = 10;
uint32_t     UClientImage_Base::resto;
uint32_t     UClientImage_Base::rstart;
uint32_t     UClientImage_Base::ncount;
uint32_t     UClientImage_Base::nrequest;
uint32_t     UClientImage_Base::size_request;
UString*     UClientImage_Base::body;
UString*     UClientImage_Base::rbuffer;
UString*     UClientImage_Base::wbuffer;
UString*     UClientImage_Base::request;
UString*     UClientImage_Base::request_uri;
UString*     UClientImage_Base::environment;
UTimeVal*    UClientImage_Base::chronometer;
struct iovec UClientImage_Base::iov_vec[4];

iPF    UClientImage_Base::callerHandlerRead       = UServer_Base::pluginsHandlerREAD;
vPF    UClientImage_Base::callerHandlerRequest    = UServer_Base::pluginsHandlerRequest;
bPF    UClientImage_Base::callerHandlerCache      = handlerCache; 
bPFpc  UClientImage_Base::callerIsValidMethod     = isValidMethod;
bPFpcu UClientImage_Base::callerIsValidRequest    = isValidRequest;
bPFpcu UClientImage_Base::callerIsValidRequestExt = isValidRequestExt;

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

   if (u_printf_string_max_length == -1 &&
       u_isPrintable(ptr, sz, true))
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

   U_INTERNAL_DUMP("u_printf_string_max_length = %d U_ClientImage_pipeline = %b", u_printf_string_max_length, U_ClientImage_pipeline)

   UServer_Base::log->log(U_CONSTANT_TO_PARAM("received request (%u bytes) %.*s%.*s%#.*S from %v"), sz,
                          (U_ClientImage_pipeline ? U_CONSTANT_SIZE("[pipeline] ") : 0), "[pipeline] ",
                          str_partial_len, str_partial,
                          sz, ptr, logbuf->rep);

   u_printf_string_max_length = u_printf_string_max_length_save;
}
#endif

UClientImage_Base::UClientImage_Base()
{
   U_TRACE_CTOR(0, UClientImage_Base, "")

   socket       = U_NULLPTR;
   logbuf       = U_NULLPTR;
   data_pending = U_NULLPTR;

   if (UServer_Base::isLog()) U_NEW_STRING(logbuf, UString(200U));

   reset();

   flag.u     = 0;
   last_event = u_now->tv_sec;

   // NB: array are not pointers (virtual table can shift the address of 'this')...

   if (UServer_Base::pClientImage == U_NULLPTR)
      {
      UServer_Base::pClientImage = this;
      UServer_Base::eClientImage = this + UNotifier::max_connection;
      }
}

UClientImage_Base::~UClientImage_Base()
{
   U_TRACE_DTOR(0, UClientImage_Base)

   // NB: array are not pointers (virtual table can shift the address of 'this')...

   U_DELETE(socket)

   if (logbuf)
      {
      U_CHECK_MEMORY_OBJECT(logbuf->rep)

   // if (logbuf->rep->memory.invariant() == false) logbuf->rep->memory._this = (void*)U_CHECK_MEMORY_SENTINEL;

      U_DELETE(logbuf)
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

   if (U_http_version == '2') U_DUMP_OBJECT_TO_TMP(UHTTP2::pConnection->itable, request)
   else
#endif
   {
   if (*rbuffer) U_FILE_WRITE_TO_TMP(*rbuffer, "request.%P");

   if (U_http_info.nResponseCode) (void) UFile::writeToTmp(iov_vec, 4, O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("response.%P"), 0);
   }
}
#endif

void UClientImage_Base::init()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::init()")

   U_INTERNAL_ASSERT_EQUALS(body, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(_value, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(rbuffer, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(wbuffer, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(request, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(_buffer, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(_encoded, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(request_uri, U_NULLPTR)

   U_NEW_STRING(body, UString);
   U_NEW_STRING(rbuffer, UString(8192));
   U_NEW_STRING(wbuffer, UString(U_CAPACITY));
   U_NEW_STRING(request, UString);
   U_NEW_STRING(request_uri, UString);
   U_NEW_STRING(environment, UString(U_CAPACITY));

   // NB: these are for ULib Servlet Page (USP) - USP_PRINTF...

   U_NEW_STRING(_value, UString(U_CAPACITY));
   U_NEW_STRING(_buffer, UString(U_CAPACITY));
   U_NEW_STRING(_encoded, UString(U_CAPACITY));

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
      U_DELETE(body)
      U_DELETE(wbuffer)
      U_DELETE(request)
      U_DELETE(rbuffer)
      U_DELETE(request_uri)
      U_DELETE(environment)
      U_DELETE(chronometer)

      // NB: these are for ULib Servlet Page (USP) - USP_PRINTF...

      U_INTERNAL_ASSERT_POINTER(_value)
      U_INTERNAL_ASSERT_POINTER(_buffer)
      U_INTERNAL_ASSERT_POINTER(_encoded)

      U_DELETE(_value)
      U_DELETE(_buffer)
      U_DELETE(_encoded)
      }
}

// Check whether the ip address client ought to be allowed

__pure bool UClientImage_Base::isAllowed(UVector<UIPAllow*>& vallow_IP)
{
   U_TRACE(0, "UClientImage_Base::isAllowed(%p)", &vallow_IP)

   if (UIPAllow::isAllowed(UServer_Base::getClientAddress(), vallow_IP)) U_RETURN(true);

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

   if (((USSLSocket*)socket)->getPeerCertificate() == U_NULLPTR)
      {
      U_SRV_LOG_WITH_ADDR("Ask for a client certificate to");

      if (((USSLSocket*)socket)->askForClientCertificate() == false) U_RETURN(false);
      }

   if (logCertificate()) U_RETURN(true);
#endif

   U_RETURN(false);
}

void UClientImage_Base::setSendfile(int fd, off_t lstart, off_t lcount)
{
   U_TRACE(0, "UClientImage_Base::setSendfile(%d,%I,%I)", fd, lstart, lcount)

   U_INTERNAL_DUMP("U_http_version = %C", U_http_version)

   U_ASSERT(body->empty())
   U_INTERNAL_ASSERT_DIFFERS(fd, -1)
   U_INTERNAL_ASSERT_MAJOR(lcount, 0)
   U_INTERNAL_ASSERT_DIFFERS(U_http_version, '2')

   setRequestNoCache();

   UServer_Base::pClientImage->offset = lstart;
   UServer_Base::pClientImage->count  = lcount;
   UServer_Base::pClientImage->sfd    = fd;
}

// NB: we have default as true to manage pipeline for protocol as RPC...

U_NO_EXPORT inline bool UClientImage_Base::handlerCache() { return true; }

U_NO_EXPORT inline bool UClientImage_Base::isValidMethod(    const char* ptr)              { return true; }
U_NO_EXPORT inline bool UClientImage_Base::isValidRequest(   const char* ptr, uint32_t sz) { return true; }
U_NO_EXPORT inline bool UClientImage_Base::isValidRequestExt(const char* ptr, uint32_t sz) { return true; }

uint32_t UClientImage_Base::checkRequestToCache()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::checkRequestToCache()")

   U_INTERNAL_DUMP("U_ClientImage_request_is_cached = %b", U_ClientImage_request_is_cached)

#if !defined(U_CACHE_REQUEST_DISABLE) || defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST)
   U_INTERNAL_ASSERT(U_ClientImage_request_is_cached)

   uint32_t    sz  = request->size();
   const char* ptr = request->data();

   U_INTERNAL_DUMP("cbuffer(%u) = %.*S", U_http_info.startHeader, U_http_info.startHeader, cbuffer)
   U_INTERNAL_DUMP("request(%u) = %.*S", sz, sz, ptr)
   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b size_request = %u U_http_uri_offset = %u", U_ClientImage_pipeline, size_request, U_http_uri_offset)

   U_INTERNAL_ASSERT_MAJOR(size_request, 0)
   U_INTERNAL_ASSERT_RANGE(1,U_http_uri_offset,254)
   U_INTERNAL_ASSERT_MAJOR(U_http_info.uri_len, 0)
   U_INTERNAL_ASSERT_MAJOR(U_http_info.startHeader, 0)
   U_INTERNAL_ASSERT_EQUALS(U_ClientImage_data_missing, false)

   if (u__isblank((ptr+U_http_uri_offset)[U_http_info.startHeader]) &&
            memcmp(ptr+U_http_uri_offset, cbuffer, U_http_info.startHeader) == 0)
      {
      if (size_request > sz &&
          (callerIsValidMethod( ptr)     == false ||
           callerIsValidRequest(ptr, sz) == false))
         {
         U_RETURN(1); // partial valid (not complete)
         }

      if (callerHandlerCache()) U_RETURN(2);
      }
#endif

   U_RETURN(0);
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

   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD)

#ifndef U_LOG_DISABLE
   if (UServer_Base::isLog())
      {
      U_INTERNAL_ASSERT_POINTER(logbuf)

      U_INTERNAL_DUMP("UEventFd::fd = %d logbuf = %V", UEventFd::fd, logbuf->rep)

      char buffer[32];
      uint32_t len = UServer_Base::setNumConnection(buffer);
      const char* agent = (bsocket_open == false || UServer_Base::isParallelizationParent() ? "Server" : "Client");

      UServer_Base::log->log(U_CONSTANT_TO_PARAM("%.6s close connection from %v, %.*s clients still connected"), agent, logbuf->rep, len, buffer);

#  ifdef DEBUG
      int fd_logbuf = ::strtoul(logbuf->data(), U_NULLPTR, 10);

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

#if !defined(U_LOG_DISABLE) && defined(U_LINUX) && defined(ENABLE_THREAD)
   ULock::atomicDecrement(U_SRV_TOT_CONNECTION);

   U_INTERNAL_DUMP("U_SRV_TOT_CONNECTION = %u", U_SRV_TOT_CONNECTION)
#endif

#ifdef U_CLASSIC_SUPPORT
   if (UServer_Base::isClassic()) U_EXIT(0);
#endif

#ifndef U_HTTP2_DISABLE
   U_INTERNAL_DUMP("U_ClientImage_http = %C U_http_version = %C", U_ClientImage_http(this), U_http_version)

   if (U_ClientImage_http(this) == '2') UHTTP2::handlerDelete(this, bsocket_open);
#endif

   if (bsocket_open) socket->close();

   --UNotifier::num_connection;

#ifndef U_LOG_DISABLE
   if (UServer_Base::isLog())
      {
      logbuf->setEmpty();

      if (UNotifier::num_connection == UNotifier::min_connection) UServer_Base::log->log(U_CONSTANT_TO_PARAM("Waiting for connection on port %u"), UServer_Base::port);
      }
#endif

   if (data_pending)
      {
      U_DELETE(data_pending)

      data_pending = U_NULLPTR;
      }
   else if (isPendingSendfile())
      {
      U_INTERNAL_DUMP("sfd = %d count = %I UEventFd::op_mask = %B U_ClientImage_pclose(this) = %d %B",
                       sfd,     count,     UEventFd::op_mask,     U_ClientImage_pclose(this), U_ClientImage_pclose(this))

      if ((U_ClientImage_pclose(this) & U_CLOSE) != 0)
         {
#     ifdef DEBUG
         if (UNLIKELY(sfd <= 0))
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

   U_INTERNAL_ASSERT_EQUALS(data_pending, U_NULLPTR)
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

      U_DELETE(pevent)

      pevent = U_NULLPTR;
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

   if (socket->iSockDesc != UEventFd::fd)
      {
      U_WARNING("handlerTimeout(): UEventFd::fd = %d socket->iSockDesc = %d", UEventFd::fd,  socket->iSockDesc);
      }
#endif

   U_INTERNAL_DUMP("U_ClientImage_idle(this) = %d %B", U_ClientImage_idle(this), U_ClientImage_idle(this))

   if (U_ClientImage_idle(this) != U_YES) // U_YES = 0x0001
      {
      // NB: maybe we have some more data to read...

      if (UNotifier::waitForRead(socket->iSockDesc, 0) == 1) U_RETURN(U_NOTIFIER_OK);

      socket->iState |= USocket::TIMEOUT;

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

#ifdef USERVER_UDP
   if (sz == 0 &&
       UServer_Base::budp)
      {
      sz  = U_CONSTANT_SIZE("unknow");
      ptr =                 "unknow";
      }
#endif
   }

   return ptr;
}

#if defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST) || (defined(DEBUG) && !defined(U_LOG_DISABLE))
void UClientImage_Base::startRequest()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::startRequest()")

#ifdef U_SERVER_CHECK_TIME_BETWEEN_REQUEST
   long time_elapsed = chronometer->restart();

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b time_elapsed = %ld time_run = %ld U_ClientImage_request_is_cached = %b",
                    U_ClientImage_pipeline,     time_elapsed,      time_run,      U_ClientImage_request_is_cached)

   if (U_ClientImage_pipeline == false &&
       U_ClientImage_parallelization == 0)
      {
      time_between_request = time_elapsed;

#  ifdef USE_LIBSSL
      if (UServer_Base::bssl) return;
#  endif
#  ifndef U_CACHE_REQUEST_DISABLE
      if (U_ClientImage_request_is_cached) return;
#  endif

      if ((time_run - time_between_request) > 10L)
         {
         U_DEBUG("UClientImage_Base::startRequest(): time_between_request(%ld) < time_run(%ld) - isParallelizationGoingToStart(%u) = %b request = %V",
                     time_between_request, time_run, U_NUM_CLIENT_THRESHOLD, UServer_Base::isParallelizationGoingToStart(U_NUM_CLIENT_THRESHOLD), request->rep)

         if (U_http_info.startHeader > 2 &&
             UServer_Base::isParallelizationGoingToStart(U_NUM_CLIENT_THRESHOLD))
            {
            U_INTERNAL_DUMP("U_ClientImage_request_is_cached = %b", U_ClientImage_request_is_cached)

            U_ClientImage_advise_for_parallelization = 1;

            if (U_ClientImage_request_is_cached == false)
               {
               U_ClientImage_advise_for_parallelization = 2;

               setRequestToCache();
               }

            return;
            }
         }
      }
#endif

#if defined(DEBUG) && !defined(U_LOG_DISABLE)
   if (UServer_Base::isLog())
      {
#  ifndef U_SERVER_CHECK_TIME_BETWEEN_REQUEST
      (void) chronometer->restart();
#  endif
      }
#endif
}
#endif

void UClientImage_Base::endRequest()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::endRequest()")

   if (U_http_method_type)
      {
      UHTTP::setEndRequestProcessing();

      U_http_method_type = 0; // NB: this mark the end of http request processing...
      }

   if (UServer_Base::isParallelizationParent() == false)
      {
#  ifdef U_SERVER_CHECK_TIME_BETWEEN_REQUEST
      time_run = chronometer->stop();

#   ifdef DEBUG
      U_INTERNAL_DUMP("U_ClientImage_pipeline = %b time_between_request = %ld time_run = %ld U_ClientImage_request_is_cached = %b",
                       U_ClientImage_pipeline,     time_between_request,      time_run,      U_ClientImage_request_is_cached)

      if ((time_run - time_between_request) > 10L)
         {
         U_DEBUG("UClientImage_Base::endRequest(): time_between_request(%ld) < time_run(%ld) - request = %V", time_between_request, time_run, request->rep)
         }
#   endif
#  endif

#  ifndef U_HTTP2_DISABLE
      U_INTERNAL_DUMP("U_http_version = %C U_http_info.uri_len = %u", U_http_version, U_http_info.uri_len)

      if (U_http_info.uri_len)
#  endif
      {
#  if defined(DEBUG) && !defined(U_LOG_DISABLE)
      if (UServer_Base::isLog())
         {
         uint32_t sz = 0;
         const char* ptr;

#     ifndef U_CACHE_REQUEST_DISABLE
         if (U_ClientImage_request_is_cached)
            {
            U_INTERNAL_DUMP("U_http_uri_offset = %u U_http_info.startHeader = %u U_http_info.uri_len = %u", U_http_uri_offset, U_http_info.startHeader, U_http_info.uri_len)

            U_INTERNAL_ASSERT_RANGE(1,U_http_uri_offset,254)

            ptr = cbuffer; // request->c_pointer(U_http_uri_offset);

#        ifdef U_ALIAS
            sz = U_http_info.startHeader;
#        else
            sz = U_http_info.uri_len;
#        endif
            }
         else
#     endif
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

            if (sz > (sizeof(buffer1)-64)) sz = sizeof(buffer1)-64;

            U_MEMCPY(ptr1, ptr, sz);
                     ptr1 +=    sz;
            }

         U_MEMCPY(ptr1, "\" run in ", U_CONSTANT_SIZE("\" run in "));
                  ptr1 +=             U_CONSTANT_SIZE("\" run in ");

#     ifndef U_SERVER_CHECK_TIME_BETWEEN_REQUEST
         time_run = chronometer->stop();
#     endif

         if (time_run > 0L) ptr1 += u__snprintf(ptr1, sizeof(buffer1)-(ptr1-buffer1), U_CONSTANT_TO_PARAM("%ld ms"), time_run);
         else               ptr1 += u__snprintf(ptr1, sizeof(buffer1)-(ptr1-buffer1), U_CONSTANT_TO_PARAM( "%g ms"), chronometer->getTimeElapsed());

         if (UServer_Base::csocket->isOpen())
            {
            uint32_t len = 0;
            int cpu = U_SYSCALL_NO_PARAM(sched_getcpu), scpu = -1;

#        ifdef SO_INCOMING_CPU
            if (USocket::bincoming_cpu)
               {
               len = sizeof(socklen_t);

               (void) UServer_Base::csocket->getSockOpt(SOL_SOCKET, SO_INCOMING_CPU, (void*)&scpu, len);

               len = (USocket::incoming_cpu == scpu ? 0 : U_CONSTANT_SIZE(" [DIFFER]"));
               }
#        endif

            U_INTERNAL_DUMP("USocket::incoming_cpu = %d USocket::bincoming_cpu = %b sched cpu = %d socket cpu = %d", USocket::incoming_cpu, USocket::bincoming_cpu, cpu, scpu)

            if (len) ptr1 += u__snprintf(ptr1,sizeof(buffer1)-(ptr1-buffer1),U_CONSTANT_TO_PARAM(", CPU: %d sched(%d) socket(%d)%.*s"),USocket::incoming_cpu,cpu,scpu,len," [DIFFER]");
            }

         U_INTERNAL_ASSERT_MINOR((ptrdiff_t)(ptr1-buffer1), (ptrdiff_t)sizeof(buffer1))

         UServer_Base::log->write(buffer1, ptr1-buffer1);
         }
#  endif
      }
      }

#ifdef U_ALIAS
   U_INTERNAL_DUMP("request_uri(%u) = %V", request_uri->size(), request_uri->rep)

   request_uri->clear();
#endif
}

void UClientImage_Base::manageReadBufferResize(uint32_t n)
{
   U_TRACE(0, "UClientImage_Base::manageReadBufferResize(%u)", n)

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b size_request = %u rbuffer->size() = %u rbuffer->capacity() = %u request->size() = %u rstart = %u",
                    U_ClientImage_pipeline,     size_request,     rbuffer->size(),     rbuffer->capacity(),     request->size(),     rstart)

   U_INTERNAL_ASSERT_MAJOR(n, 0)

   ptrdiff_t diff = 0;

   request->clear();

   if (U_ClientImage_pipeline)
      {
      U_ClientImage_pipeline = false;

      U_INTERNAL_ASSERT_MAJOR(rstart, 0)
      }

   diff = -(ptrdiff_t)rstart;

   if (diff)
      {
      rbuffer->moveToBeginDataInBuffer(rstart);
                                       rstart = 0;
      }

   if (rbuffer->space() < n)
      {
      const char* ptr = rbuffer->data();

      UString::_reserve(*rbuffer, rbuffer->getReserveNeed(n));

      diff += rbuffer->data() - ptr;
      }

#ifndef U_HTTP2_DISABLE
   U_INTERNAL_DUMP("U_ClientImage_http = %C U_http_version = %C", U_ClientImage_http(UServer_Base::pClientImage), U_http_version)

   if (U_ClientImage_http(UServer_Base::pClientImage) != '2')
#endif
   {
   if (U_http_method_type)
      {
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

#  if defined(USE_LIBSSL) && !defined(U_SERVER_CAPTIVE_PORTAL)
      if (U_http_websocket_len) UWebSocket::upgrade_settings += diff;
#  endif

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

   *request = *rbuffer;
   }

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

   u_clientimage_info.flag.u = 0; // NB: U_ClientImage_parallelization is reset by this...

#ifdef USERVER_UDP
   if (UServer_Base::budp == false)
#endif
   {
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

#  ifdef U_EVASIVE_SUPPORT
      if (UServer_Base::checkHold(socket->getClientAddress()))
         {
         abortive_close();

         return;
         }
#  endif

   // resetRequestFromUServer();
      }
   else
      {
      // NB: called from UServer_Base::handlerRead...

      U_INTERNAL_DUMP("UServer_Base::csocket = %p socket = %p UServer_Base::pClientImage = %p this = %p", UServer_Base::csocket, socket, UServer_Base::pClientImage, this)

      U_INTERNAL_ASSERT_DIFFERS(socket->iSockDesc, -1)
      U_INTERNAL_ASSERT_EQUALS(UServer_Base::csocket, socket)
      U_INTERNAL_ASSERT_EQUALS(UServer_Base::pClientImage, this)

      UEventFd::fd = socket->iSockDesc;

   // setRequestFromUServer();
      }

#ifdef U_EVASIVE_SUPPORT
   if (UServer_Base::checkHitSiteStats())
      {
      if (UHTTP::file_gzip_bomb &&
          UServer_Base::bssl == false)
         {
         bsendGzipBomb = true;
         }
      else
         {
         abortive_close();
         }

      return;
      }
#endif

#ifdef U_THROTTLING_SUPPORT
   UServer_Base::initThrottlingClient();
#endif
   }
}

bool UClientImage_Base::genericRead()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::genericRead()")

#if defined(DEBUG) || defined(U_EVASIVE_SUPPORT)
   if (UNLIKELY(socket->iSockDesc == -1))
      {
#  ifndef U_EVASIVE_SUPPORT
      U_WARNING("genericRead(): "
                "UEventFd::fd = %d socket->iSockDesc = %d "
                "UNotifier::num_connection = %d UNotifier::min_connection = %d "
                "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B",
                UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
                UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask);
#  endif

      U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;

      U_RETURN(false);
      }
#endif

   U_INTERNAL_ASSERT_EQUALS(socket->iSockDesc, UEventFd::fd)

   rstart = 0;

   request->clear(); // reset buffer before read

   U_INTERNAL_DUMP("rbuffer(%u) = %V", rbuffer->size(), rbuffer->rep)

   // NB: rbuffer string can be referenced more than one (often if U_SUBSTR_INC_REF is defined)...

   if (rbuffer->uniq()) rbuffer->rep->_length = 0; 
   else                 rbuffer->_set(UStringRep::create(0U, U_CAPACITY, U_NULLPTR));

   if (data_pending)
      {
      U_INTERNAL_DUMP("data_pending(%u) = %V", data_pending->size(), data_pending->rep)

      U_MEMCPY(rbuffer->data(), data_pending->data(), rbuffer->rep->_length = data_pending->size());
      }

   socket->iState = USocket::CONNECT; // prepare socket before read

#ifdef USERVER_UDP
   if (UServer_Base::budp)
      {
      uint32_t sz = rbuffer->size();
      int iBytesTransferred = socket->recvFrom(rbuffer->data()+sz, rbuffer->capacity());

      if (iBytesTransferred <= 0) U_RETURN(false);

      rbuffer->size_adjust(sz+iBytesTransferred);

      UServer_Base::setClientAddress();

#  ifndef U_LOG_DISABLE
      UServer_Base::logNewClient(socket, this);
#  endif
      }
   else
#endif
   {
   if (USocketExt::read(socket, *rbuffer, U_SINGLE_READ, 0) == false) // NB: timeout == 0 means that we put the socket fd on epoll queue if EAGAIN...
      {
      U_ClientImage_state = (socket->isOpen() ? U_PLUGIN_HANDLER_AGAIN
                                              : U_PLUGIN_HANDLER_ERROR);

      U_RETURN(false);
      }
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

      U_DELETE(data_pending)

      data_pending = U_NULLPTR;
      }

   U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

#if defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST) || (defined(DEBUG) && !defined(U_LOG_DISABLE))
   startRequest();
#endif

#ifdef U_SERVER_CHECK_TIME_BETWEEN_REQUEST
   if (U_ClientImage_advise_for_parallelization)
      {
      U_INTERNAL_DUMP("U_ClientImage_advise_for_parallelization = %u", U_ClientImage_advise_for_parallelization)

      if (checkRequestToCache() == 2         &&
          UClient_Base::csocket == U_NULLPTR &&
          UServer_Base::startParallelization(U_NUM_CLIENT_THRESHOLD))
         {
         // parent

         U_RETURN(false);
         }

      if (U_ClientImage_advise_for_parallelization == 2) U_ClientImage_request_is_cached = false;
      }
#endif

   U_ClientImage_state = 0;

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
   U_INTERNAL_ASSERT(socket->isOpen())

   UServer_Base::nread++;
#endif

loop:
   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b size_request = %u rstart = %u rbuffer(%u) = %V",
                    U_ClientImage_pipeline,     size_request,     rstart,     rbuffer->size(), rbuffer->rep)

   if (U_ClientImage_pipeline == false) *request = *rbuffer;
   else
      {
pipeline:
      sz = rbuffer->size();

      U_INTERNAL_DUMP("size_request = %u sz = %u rstart = %u", size_request, sz, rstart)

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
data_missing:
      U_INTERNAL_DUMP("U_ClientImage_parallelization = %d U_http_version = %C", U_ClientImage_parallelization, U_http_version)

      U_INTERNAL_ASSERT_DIFFERS(U_http_version, '2')

      if (U_ClientImage_parallelization == U_PARALLELIZATION_CHILD)
         {
         if (UNotifier::waitForRead(UServer_Base::csocket->iSockDesc, U_TIMEOUT_MS) != 1 ||
             (resetReadBuffer(), USocketExt::read(UServer_Base::csocket, *rbuffer, getCountToRead(), 0)) == false)
            {
            if ((U_ClientImage_state & U_PLUGIN_HANDLER_ERROR) != 0) U_RETURN(U_NOTIFIER_DELETE);

            goto death;
            }
         }

      U_ClientImage_data_missing = false;

      U_INTERNAL_ASSERT_EQUALS(data_pending, U_NULLPTR)

      if (U_ClientImage_parallelization == U_PARALLELIZATION_CHILD) goto loop;

      U_NEW_STRING(data_pending, UString((void*)U_STRING_TO_PARAM(*request)));

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
         if (sz == 1) goto data_missing; // partial valid (not complete)

         U_INTERNAL_ASSERT_EQUALS(sz, 2)

         setRequestProcessed();

         goto next2;
         }

      U_ClientImage_request_is_cached = false;
      }
#endif

   resetBuffer();

#if defined(U_SERVER_CAPTIVE_PORTAL) && defined(ENABLE_THREAD)
   if (UHTTP::checkForUSP()) U_RETURN(U_NOTIFIER_OK);

   if (U_ClientImage_parallelization == U_PARALLELIZATION_PARENT) U_RETURN(U_NOTIFIER_DELETE);
#endif

   size_request = 0;

   U_INTERNAL_ASSERT_EQUALS(UServer_Base::csocket, UServer_Base::pClientImage->socket)

   U_ClientImage_state = callerHandlerRead();

   U_INTERNAL_DUMP("socket->isClosed() = %b U_http_info.nResponseCode = %u U_ClientImage_close = %b U_ClientImage_state = %d %B",
                    socket->isClosed(),     U_http_info.nResponseCode,     U_ClientImage_close,     U_ClientImage_state, U_ClientImage_state)

   if (UNLIKELY(socket->isClosed()))
      {
cls:  if (U_ClientImage_parallelization == U_PARALLELIZATION_PARENT)
         {
         U_ASSERT(wbuffer->empty())
         U_INTERNAL_ASSERT_EQUALS(U_ClientImage_data_missing, false)

         endRequest();

         U_RETURN(U_NOTIFIER_DELETE);
         }

      goto error;
      }

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_parallelization = %d U_ClientImage_data_missing = %b",
                    U_ClientImage_pipeline,     U_ClientImage_parallelization,     U_ClientImage_data_missing)

   U_INTERNAL_ASSERT(socket->isOpen())
   U_INTERNAL_ASSERT_EQUALS(U_ClientImage_state & U_PLUGIN_HANDLER_ERROR, 0)

   if (U_ClientImage_data_missing) goto data_missing;

   U_INTERNAL_DUMP("size_request = %u", size_request)

   if (size_request)
      {
#  if !defined(U_LOG_DISABLE) && !defined(U_COVERITY_FALSE_POSITIVE)
      U_INTERNAL_DUMP("log_request_partial = %u UEventFd::fd = %d", log_request_partial, UEventFd::fd)

      if (log_request_partial == UEventFd::fd) logRequest();
#  endif
#  ifndef U_CACHE_REQUEST_DISABLE
next2:
#  endif
      sz = rbuffer->size();

      if (U_ClientImage_pipeline)
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
      else if (size_request < sz) // we check if we have a pipeline...
         {
         const char* ptr1 = rbuffer->c_pointer(size_request);

         if (UNLIKELY(u__isspace(*ptr1))) while (u__isspace(*++ptr1)) {}

         if (ptr1 < rbuffer->pend())
            {
            U_ClientImage_pipeline = true;

#        ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
            U_INTERNAL_DUMP("U_http_info.nResponseCode = %u U_ClientImage_close = %b U_ClientImage_state = %d %B",
                             U_http_info.nResponseCode,     U_ClientImage_close,     U_ClientImage_state, U_ClientImage_state)

            if (callerIsValidMethod(ptr1))
               {
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

check:            U_INTERNAL_DUMP("nrequest = %u resto = %u", nrequest, resto)

                  U_INTERNAL_ASSERT(nrequest <= n)
                  U_INTERNAL_ASSERT_DIFFERS(nrequest, 1)

                  if (resto ||
                      nrequest != n)
                     {
                     rstart = (nrequest ? nrequest : 1) * size_request;

                     *request = rbuffer->substr(rstart);
                     }

                  U_INTERNAL_DUMP("rstart = %u request(%u) = %V", rstart, request->size(), request->rep)

                  goto write;
                  }
               }
#        endif
            }

         *request = rbuffer->substr(0U, (rstart = size_request));
         }
      }

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b size_request = %u request->size() = %u rstart = %u", U_ClientImage_pipeline, size_request, request->size(), rstart)

   if (isRequestNeedProcessing())
      {
      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_PARENT)
      U_INTERNAL_ASSERT_EQUALS(U_ClientImage_state & (U_PLUGIN_HANDLER_AGAIN | U_PLUGIN_HANDLER_ERROR), 0)

      callerHandlerRequest();

      if (UNLIKELY(socket->isClosed())) goto cls;
      }

   U_INTERNAL_DUMP("socket->isClosed() = %b U_http_info.nResponseCode = %u U_ClientImage_close = %b U_ClientImage_state = %d %B",
                    socket->isClosed(),     U_http_info.nResponseCode,     U_ClientImage_close,     U_ClientImage_state, U_ClientImage_state)

   U_INTERNAL_DUMP("wbuffer(%u) = %V", wbuffer->size(), wbuffer->rep)
   U_INTERNAL_DUMP("   body(%u) = %V",    body->size(),    body->rep)

   if (LIKELY(*wbuffer))
      {
      U_INTERNAL_DUMP("U_http_info.nResponseCode = %u count = %I UEventFd::op_mask = %d %B",
                       U_http_info.nResponseCode,     count,     UEventFd::op_mask, UEventFd::op_mask)

      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_PARENT)

      if (count == 0)
         {
#ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
write:
#endif
         U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_data_missing = %b", U_ClientImage_pipeline, U_ClientImage_data_missing)

         result = (writeResponse() ? U_NOTIFIER_OK : U_NOTIFIER_DELETE);

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

               goto data_missing;
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
         U_INTERNAL_ASSERT_DIFFERS(U_http_version, '2')
         U_INTERNAL_ASSERT_EQUALS(UEventFd::op_mask, EPOLLIN | EPOLLRDHUP | EPOLLET)

         if (writeResponse() == false ||
             UClientImage_Base::handlerWrite() == U_NOTIFIER_DELETE)
            {
            U_INTERNAL_DUMP("count = %I", count)

            goto error;
            }
         }
      }

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b (U_ClientImage_state & U_PLUGIN_HANDLER_ERROR) = %u", U_ClientImage_pipeline, U_ClientImage_state & U_PLUGIN_HANDLER_ERROR)

   if (LIKELY((U_ClientImage_state & U_PLUGIN_HANDLER_ERROR) == 0))
      {
      if (U_ClientImage_pipeline)
         {
         endRequest();

#     if defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST) || (defined(DEBUG) && !defined(U_LOG_DISABLE))
         startRequest();
#     endif

         U_ClientImage_request = 0;

         goto pipeline;
         }
      }
   else
      {
      U_INTERNAL_ASSERT_EQUALS(U_ClientImage_pipeline, false)
error:
      U_INTERNAL_ASSERT_DIFFERS(UEventFd::fd, -1)

      U_ClientImage_close = true;

      if (         UHTTP::file_data &&
          u_is_usp(UHTTP::file_data->mime_index))
         {
         U_INTERNAL_ASSERT_POINTER(UHTTP::usp)
         U_INTERNAL_ASSERT_POINTER(UHTTP::usp->runDynamicPage)

         UHTTP::usp->runDynamicPage(U_DPAGE_ERROR);
         }
      }

#ifdef U_THROTTLING_SUPPORT
   if (uri) UServer_Base::clearThrottling();
#endif

#if defined(U_SERVER_CAPTIVE_PORTAL) && defined(ENABLE_THREAD)
   U_INTERNAL_DUMP("U_ClientImage_request_is_cached = %b", U_ClientImage_request_is_cached)

   if (U_ClientImage_request_is_cached == false) endRequest();

   U_RETURN(U_NOTIFIER_OK);
#else
   endRequest();
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

      if (UNotifier::waitForRead(socket->iSockDesc, U_TIMEOUT_MS) == 1)
         {
         U_ClientImage_request = 0;

         goto start;
         }

death:
      UServer_Base::endNewChild(); // no return
      }

   last_event = u_now->tv_sec;

   U_INTERNAL_ASSERT(socket->isOpen())
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD)
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_PARENT)

   U_INTERNAL_DUMP("request(%u) = %V", request->size(), request->rep);

   U_RETURN(U_NOTIFIER_OK);
}

bool UClientImage_Base::writeResponse()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::writeResponse()")

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_close = %b nrequest = %u", U_ClientImage_pipeline, U_ClientImage_close, nrequest)

   U_INTERNAL_ASSERT(*wbuffer)
   U_INTERNAL_ASSERT(socket->isOpen())
   U_INTERNAL_ASSERT_DIFFERS(U_http_version, '2')
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_PARENT)

   struct iovec* iov;
#if !defined(U_PIPELINE_HOMOGENEOUS_DISABLE) || defined(U_CLIENT_RESPONSE_PARTIAL_WRITE_SUPPORT) 
   struct iovec liov[256];
#endif
   uint32_t iBytesWrite,
            sz1 = wbuffer->size(),
            sz2 = (U_http_method_type == HTTP_HEAD ? 0 : body->size());
#ifndef U_LOG_DISABLE
   uint32_t msg_len = (U_ClientImage_pipeline ? U_CONSTANT_SIZE("[pipeline] ") : 0);
#endif

   iov_vec[2].iov_len  = sz1;
   iov_vec[2].iov_base = (caddr_t)wbuffer->data();
   iov_vec[3].iov_len  = sz2;
   iov_vec[3].iov_base = (caddr_t)body->data();

   ncount = sz1 + sz2;

   if (isNoHeaderForResponse())
      {
      U_INTERNAL_ASSERT_EQUALS(nrequest, 0)

      U_SRV_LOG_WITH_ADDR("send response (%u bytes) %.*s%#.*S to", ncount, msg_len, "[pipeline] ", iov_vec[2].iov_len, iov_vec[2].iov_base);

#  ifdef USERVER_UDP
      if (UServer_Base::budp)
         {
         U_INTERNAL_ASSERT_EQUALS(iov_vec[2].iov_len, ncount)

         if (socket->sendTo(iov_vec[2].iov_base, ncount) == (int)ncount) U_RETURN(true);

         U_RETURN(false);
         }
#  endif

      idx    = 2;
      iovcnt = 2;
      }
   else
      {
      idx    = 0;
      iovcnt = 4;

#  ifdef U_SSE_ENABLE // SERVER SENT EVENTS (SSE)
      U_INTERNAL_DUMP("UHTTP::sse_func = %p", UHTTP::sse_func)

      if (UHTTP::sse_func != (void*)1L)
#  endif
      {
      if (U_ClientImage_close &&
          U_ClientImage_pipeline == false)
         {
         iov_vec[1].iov_len += 17+2; // Connection: close\r\n
         }
      }

      ncount += iov_vec[0].iov_len +
                iov_vec[1].iov_len;

#  if defined(U_LINUX) && defined(ENABLE_THREAD)
      U_INTERNAL_ASSERT_EQUALS(iov_vec[1].iov_base, ULog::ptr_shared_date->date3)
#  else
      U_INTERNAL_ASSERT_EQUALS(iov_vec[1].iov_base, ULog::date.date3)

      ULog::updateDate3(U_NULLPTR);
#  endif

#  ifndef U_LOG_DISABLE
      if (logbuf) UServer_Base::log->log(iov_vec+idx, "response", ncount, "[pipeline] ", msg_len, U_CONSTANT_TO_PARAM(" to %v"), logbuf->rep);
#  endif
      }

   iov = iov_vec+idx;

#ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
   if (nrequest)
      {
      U_INTERNAL_ASSERT_MAJOR(nrequest, 1)

      char* ptr   = (char*)liov;
      uint32_t sz = sizeof(struct iovec) * iovcnt;

      U_MEMCPY(ptr, iov, sz);

      for (uint32_t i = 1; i < nrequest; ++i)
         {
                  ptr +=    sz;
         U_MEMCPY(ptr, iov, sz);
         }

      iovcnt *= nrequest;
      ncount *= nrequest;

      iBytesWrite = USocketExt::writev(socket, (iov = liov), iovcnt, ncount, 0);
      }
   else
#endif
   {
   U_DUMP_IOVEC(iov,iovcnt)

   U_INTERNAL_ASSERT_EQUALS(nrequest, 0)

   iBytesWrite = USocketExt::writev(socket, iov, iovcnt, ncount, U_ClientImage_pipeline ? U_TIMEOUT_MS : 0);
   }

#ifdef U_THROTTLING_SUPPORT
   if (iBytesWrite > 0) bytes_sent += iBytesWrite;
#endif
#ifdef DEBUG
   if (iBytesWrite > 0) UServer_Base::stats_bytes += iBytesWrite;
#endif

   if (iBytesWrite == ncount) U_RETURN(true);

   U_SRV_LOG("write failed (remain %u bytes) - sock_fd %u", ncount - iBytesWrite, socket->iSockDesc);

   if (socket->isClosed()) U_RETURN(false);

#ifndef U_CLIENT_RESPONSE_PARTIAL_WRITE_SUPPORT
   resetPipelineAndSetCloseConnection();

   U_RETURN(false);
#else
   U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)
   U_INTERNAL_ASSERT_EQUALS(UEventFd::op_mask, EPOLLIN | EPOLLRDHUP | EPOLLET)
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD) // NB: we must not have pending write...

   sfd = UFile::mkTemp();

   if (sfd == -1)
      {
      U_SRV_LOG("partial write failed: (remain %u bytes) - error on create temporary file - sock_fd %u", ncount, socket->iSockDesc);

      U_RETURN(false);
      }

   iBytesWrite = UFile::writev(sfd, liov, USocketExt::iov_resize(liov, iov, iovcnt, iBytesWrite));

   if (iBytesWrite != ncount)
      {
      U_SRV_LOG("partial write failed: (remain %u bytes) - error on write (%u bytes) on temporary file - sock_fd %u sfd %u", ncount, iBytesWrite, socket->iSockDesc, sfd);

      UFile::close(sfd);
                   sfd = -1;

      U_RETURN(false);
      }

   U_SRV_LOG("partial write: (remain %u bytes) - create temporary file - sock_fd %u sfd %u", ncount, socket->iSockDesc, sfd);

   setPendingSendfile(); // NB: now we have a pending sendfile...

   U_RETURN(true);
#endif
}

void UClientImage_Base::close()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::close()")

   setRequestProcessed();

   UServer_Base::csocket->close();

   resetPipelineAndSetCloseConnection();
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
   U_TRACE_NO_PARAM(0, "UClientImage::prepareForSendfile()")

   U_INTERNAL_ASSERT_MAJOR(sfd, 0)
   U_INTERNAL_ASSERT_MAJOR(count, 0)
   U_INTERNAL_ASSERT_DIFFERS(U_http_version, '2')
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD)

   if (U_ClientImage_close)
      {
      U_ClientImage_close        = false;
      U_ClientImage_pclose(this) = U_YES;
      }

   if (U_ClientImage_pipeline) resetPipeline();

   UEventFd::op_mask = EPOLLOUT;

   if (UNotifier::isHandler(UEventFd::fd)) (void) UNotifier::modify(this);

   U_INTERNAL_DUMP("offset = %I count = %I", offset, count)
}

int UClientImage_Base::handlerWrite()
{
   U_TRACE_NO_PARAM(0, "UClientImage_Base::handlerWrite()")

   U_INTERNAL_ASSERT_DIFFERS(U_http_version, '2')

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

   U_INTERNAL_ASSERT_MAJOR(sfd, 0)
   U_INTERNAL_ASSERT_MAJOR(count, 0)
   U_INTERNAL_ASSERT(socket->isOpen())

   bool bwrite = (UEventFd::op_mask == EPOLLOUT);

   U_INTERNAL_DUMP("bwrite = %b", bwrite)

#ifdef U_THROTTLING_SUPPORT
   if (UServer_Base::checkThrottlingBeforeSend(bwrite) == false) U_RETURN(U_NOTIFIER_OK);
#endif

   uint32_t iBytesWrite;

write:
   iBytesWrite = USocketExt::sendfile(socket, sfd, &offset, count, 0);

#ifdef U_THROTTLING_SUPPORT
   if (iBytesWrite > 0) bytes_sent += iBytesWrite;
#endif
#ifdef DEBUG
   if (iBytesWrite > 0) UServer_Base::stats_bytes += iBytesWrite;
#endif

   if (iBytesWrite == count)
      {
      U_SRV_LOG_WITH_ADDR("sending sendfile response completed (%u bytes of %I) to", iBytesWrite, count);

      if (bwrite)
         {
         UEventFd::op_mask = EPOLLIN | EPOLLRDHUP | EPOLLET;

         (void) UNotifier::modify(this);
         }
#  ifdef DEBUG
      else
         {
         U_INTERNAL_ASSERT_EQUALS(UEventFd::op_mask, EPOLLIN | EPOLLRDHUP | EPOLLET)
         U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_PARENT)
         }
#  endif

      offset =
      count  =  0;
      sfd    = -1;

      if ((U_ClientImage_pclose(this) & U_CLOSE) != 0) UFile::close(sfd);

      if ((U_ClientImage_pclose(this) & U_YES) != 0) U_RETURN(U_NOTIFIER_DELETE);

      U_RETURN(U_NOTIFIER_OK);
      }

   if (iBytesWrite > 0)
      {
      U_SRV_LOG_WITH_ADDR("sent sendfile partial response (%u bytes of %I) to", iBytesWrite, count);

      count -= iBytesWrite;

      U_INTERNAL_ASSERT_MAJOR(count, 0)

      if (socket->isOpen() == false) goto end;

      if (bwrite) U_RETURN(U_NOTIFIER_OK);

      if (U_ClientImage_parallelization == U_PARALLELIZATION_CHILD)
         {
wait:    socket->setBlocking();

         if (UNotifier::waitForWrite(socket->iSockDesc, U_TIMEOUT_MS) == 1) goto write;

         goto end;
         }

      if (UServer_Base::startParallelization()) U_RETURN(U_NOTIFIER_DELETE); // parent

      if (U_ClientImage_parallelization == U_PARALLELIZATION_CHILD) goto wait;

      prepareForSendfile();

      U_RETURN(U_NOTIFIER_OK);
      }

end:
   U_SRV_LOG("sendfile failed - sock_fd: %d sfd: %d count: %I U_ClientImage_pclose(this): %d %B", socket->iSockDesc, sfd, count, U_ClientImage_pclose(this), U_ClientImage_pclose(this));

   if (U_ClientImage_parallelization != U_PARALLELIZATION_CHILD)
      {
      if ((U_ClientImage_pclose(this) & U_CLOSE) != 0) UFile::close(sfd);

      offset =
      count  =  0;
      sfd    = -1;

      U_ClientImage_pclose(this) = 0;
      }

   U_RETURN(U_NOTIFIER_DELETE);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UClientImage_Base::dump(bool _reset) const
{
   *UObjectIO::os << "sfd                                " << sfd                 << '\n'
                  << "bIPv6                              " << bIPv6               << '\n'
                  << "count                              " << count               << '\n'
                  << "offset                             " << offset              << '\n'
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

   return U_NULLPTR;
}
#endif
