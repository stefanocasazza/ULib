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

#include <ulib/debug/error.h>

#ifdef U_CACHE_REQUEST_DISABLE
#  define U_IOV_TO_SAVE  sizeof(struct iovec)
#else
#  define U_IOV_TO_SAVE (sizeof(struct iovec) * 4)
#endif

int                UClientImage_Base::idx;
int                UClientImage_Base::csfd;
int                UClientImage_Base::iovcnt;
iPF                UClientImage_Base::callerHandlerRead;
iPF                UClientImage_Base::callerHandlerRequest;
iPF                UClientImage_Base::callerHandlerReset;
bPF                UClientImage_Base::callerHandlerCache;
vPF                UClientImage_Base::callerHandlerEndRequest;
bool               UClientImage_Base::bIPv6;
bool               UClientImage_Base::log_request_partial;
char               UClientImage_Base::cbuffer[128];
long               UClientImage_Base::time_run;
long               UClientImage_Base::time_between_request;
bPFpc              UClientImage_Base::callerIsValidRequest;
bPFpcu             UClientImage_Base::callerIsValidRequestExt;
uint32_t           UClientImage_Base::resto;
uint32_t           UClientImage_Base::rstart;
uint32_t           UClientImage_Base::ncount;
uint32_t           UClientImage_Base::nrequest;
uint32_t           UClientImage_Base::uri_len;
uint32_t           UClientImage_Base::uri_offset;
uint32_t           UClientImage_Base::size_request;
UString*           UClientImage_Base::body;
UString*           UClientImage_Base::rbuffer;
UString*           UClientImage_Base::wbuffer;
UString*           UClientImage_Base::request;
UString*           UClientImage_Base::request_uri;
UString*           UClientImage_Base::environment;
USocket*           UClientImage_Base::psocket;
UTimeVal*          UClientImage_Base::chronometer;
struct iovec*      UClientImage_Base::piov;
struct iovec       UClientImage_Base::iov_sav[4];
struct iovec       UClientImage_Base::iov_vec[4];
UClientImage_Base* UClientImage_Base::pthis;

// NB: these are for ULib Servlet Page (USP) - USP_PRINTF...

UString* UClientImage_Base::_value;
UString* UClientImage_Base::_buffer;
UString* UClientImage_Base::_encoded;

#ifdef U_LOG_ENABLE
void UClientImage_Base::logRequest()
{
   U_TRACE(0, "UClientImage_Base::logRequest()")

   U_INTERNAL_ASSERT(*request)
   U_INTERNAL_ASSERT_POINTER(logbuf)
   U_INTERNAL_ASSERT(UServer_Base::isLog())
   U_INTERNAL_ASSERT(logbuf->isNullTerminated())

   const char* str_partial = "";
   const char* ptr = request->data();
   uint32_t str_partial_len = 0, sz = request->size();
   int32_t u_printf_string_max_length_save = u_printf_string_max_length;

   U_INTERNAL_DUMP("u_printf_string_max_length = %d", u_printf_string_max_length)

   if (u_printf_string_max_length == -1)
      {
      u_printf_string_max_length = u_findEndHeader(ptr, sz);

      if (u_printf_string_max_length == -1)
         {
         str_partial         =                 "[partial] ";
         str_partial_len     = U_CONSTANT_SIZE("[partial] ");
         log_request_partial = true;

         u_printf_string_max_length = U_min(sz,1000);
         }
      else
         {
         if (u_printf_string_max_length < 18) u_printf_string_max_length = 128; // 18 -> "GET / HTTP/1.0\r\n\r\n"

         if (log_request_partial)
            {
            str_partial         =                 "[complete] ";
            str_partial_len     = U_CONSTANT_SIZE("[complete] ");
            log_request_partial = false;
            }
         }

      U_INTERNAL_ASSERT_MAJOR(u_printf_string_max_length, 0)
      }

   U_INTERNAL_DUMP("u_printf_string_max_length = %d U_ClientImage_pipeline = %b", u_printf_string_max_length, U_ClientImage_pipeline)

   ULog::log("%sreceived request (%u bytes) %.*s%.*s%#.*S from %.*s",
               UServer_Base::mod_name[0], sz,
               (U_ClientImage_pipeline ? U_CONSTANT_SIZE("[pipeline] ") : 0), "[pipeline] ",
               str_partial_len, str_partial,
               sz, ptr, U_STRING_TO_TRACE(*logbuf));

   u_printf_string_max_length = u_printf_string_max_length_save;
}
#endif

UClientImage_Base::UClientImage_Base()
{
   U_TRACE_REGISTER_OBJECT(0, UClientImage_Base, "")

   socket       = 0;
   logbuf       = (UServer_Base::isLog() ? U_NEW(UString(200U)) : 0);
   data_pending = 0;

   reset();

   pending_close = 0;
   last_event    = u_now->tv_sec;
}

UClientImage_Base::~UClientImage_Base()
{
   U_TRACE_UNREGISTER_OBJECT(0, UClientImage_Base)

   // NB: array are not pointers (virtual table can shift the address of 'this')...

   delete socket;

   if (logbuf)
      {
#  ifndef ENABLE_NEW_VECTOR
      U_CHECK_MEMORY_OBJECT(logbuf->rep)
#  else
      if (logbuf->rep->memory.invariant() == false) logbuf->rep->memory._this = (void*)U_CHECK_MEMORY_SENTINEL;
#  endif

      delete logbuf;
      }
}

void UClientImage_Base::set()
{
   U_TRACE(0, "UClientImage_Base::set()")

   // NB: array are not pointers (virtual table can shift the address of 'this')...

   if (UServer_Base::vClientImage == 0)
      {
      UServer_Base::pClientIndex = UServer_Base::vClientImage = this;
                                   UServer_Base::eClientImage = this + UNotifier::max_connection;

      U_INTERNAL_DUMP("UServer_Base::vClientImage = %p UServer_Base::eClientImage = %p UNotifier::max_connection = %u",
                       UServer_Base::vClientImage,     UServer_Base::eClientImage,     UNotifier::max_connection)
      }

   U_INTERNAL_DUMP("u_new_vector<T>: elem %u of %u", (this - UServer_Base::vClientImage), UNotifier::max_connection)

   U_INTERNAL_DUMP("this = %p socket = %p UEventFd::fd = %d", this, socket, UEventFd::fd)

   U_INTERNAL_ASSERT_POINTER(socket)
   U_INTERNAL_ASSERT_POINTER(UServer_Base::socket)

   if (UServer_Base::socket->isLocalSet()) socket->cLocalAddress.set(UServer_Base::socket->cLocalAddress);

                                               socket->flags |= O_CLOEXEC;
   if (USocket::accept4_flags & SOCK_NONBLOCK) socket->flags |= O_NONBLOCK;

#ifdef DEBUG
               U_CHECK_MEMORY
               U_CHECK_MEMORY_OBJECT(socket)
   if (logbuf) U_CHECK_MEMORY_OBJECT(logbuf->rep)

   uint32_t index = (this - UServer_Base::vClientImage);

   if (index)
      {
      UClientImage_Base* ptr = UServer_Base::vClientImage + index-1;

      U_CHECK_MEMORY_OBJECT(ptr)
      U_CHECK_MEMORY_OBJECT(ptr->socket)

      if (logbuf)
         {
         U_INTERNAL_DUMP("ptr->logbuf = %p ptr->logbuf->rep = %p ptr->logbuf->rep->memory._this = %p",
                          ptr->logbuf,     ptr->logbuf->rep,     ptr->logbuf->rep->memory._this)

         U_CHECK_MEMORY_OBJECT(ptr->logbuf->rep)

         if (index == (UNotifier::max_connection-1))
            {
            for (index = 0, ptr = UServer_Base::vClientImage; index < UNotifier::max_connection; ++index, ++ptr)
               {
               (void) ptr->check_memory();
               }
            }
         }
      }
#endif
}
// ------------------------------------------------------------------------

#ifdef DEBUG
bool UClientImage_Base::check_memory()
{
   U_TRACE(0, "UClientImage_Base::check_memory()")

   U_INTERNAL_DUMP("u_check_memory_vector<T>: elem %u of %u", this - UServer_Base::vClientImage, UNotifier::max_connection)

   U_INTERNAL_DUMP("this = %p socket = %p UEventFd::fd = %d", this, socket, UEventFd::fd)

   U_CHECK_MEMORY
   U_CHECK_MEMORY_OBJECT(socket)

   if (logbuf)
      {
#  ifndef ENABLE_NEW_VECTOR
      U_CHECK_MEMORY_OBJECT(logbuf->rep)
#  else
      if (logbuf->rep->memory.invariant() == false)
         {
         U_INTERNAL_DUMP("logbuf = %p logbuf->rep = %p logbuf->rep->memory._this = %p",
                          logbuf,     logbuf->rep,     logbuf->rep->memory._this)

         logbuf->rep->memory._this = (void*)U_CHECK_MEMORY_SENTINEL;
         }
#  endif
      }

   U_RETURN(true);
}
#endif

#ifdef DEBUG
void UClientImage_Base::saveRequestResponse()
{
   U_TRACE(0, "UClientImage_Base::saveRequestResponse()")

                                  (void) UFile::writeToTmp(U_STRING_TO_PARAM(*rbuffer), false,  "request.%P", 0);
   if (u_http_info.nResponseCode) (void) UFile::writeToTmp(iov_sav, 4,                  false, "response.%P", 0);
}
#endif

U_NO_EXPORT bool UClientImage_Base::isValidRequest(   const char* ptr)              { return true; }
U_NO_EXPORT bool UClientImage_Base::isValidRequestExt(const char* ptr, uint32_t sz) { return sz; }

void UClientImage_Base::init()
{
   U_TRACE(0, "UClientImage_Base::init()")

   U_INTERNAL_ASSERT_EQUALS(body, 0)
   U_INTERNAL_ASSERT_EQUALS(_value, 0)
   U_INTERNAL_ASSERT_EQUALS(rbuffer, 0)
   U_INTERNAL_ASSERT_EQUALS(wbuffer, 0)
   U_INTERNAL_ASSERT_EQUALS(request, 0)
   U_INTERNAL_ASSERT_EQUALS(_buffer, 0)
   U_INTERNAL_ASSERT_EQUALS(_encoded, 0)
   U_INTERNAL_ASSERT_EQUALS(request_uri, 0)

   body        = U_NEW(UString);
   rbuffer     = U_NEW(UString(8192));
   wbuffer     = U_NEW(UString(U_CAPACITY));
   request     = U_NEW(UString);
   request_uri = U_NEW(UString);
   environment = U_NEW(UString);
   chronometer = U_NEW(UTimeVal);

   // NB: these are for ULib Servlet Page (USP) - USP_PRINTF...

   _value   = U_NEW(UString(U_CAPACITY));
   _buffer  = U_NEW(UString(U_CAPACITY));
   _encoded = U_NEW(UString(U_CAPACITY));

   chronometer->start();

   callerIsValidRequest    = isValidRequest;
   callerIsValidRequestExt = isValidRequestExt;

#ifdef DEBUG
   UError::callerDataDump = saveRequestResponse;
#endif
}

void UClientImage_Base::clear()
{
   U_TRACE(0, "UClientImage_Base::clear()")

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

   U_INTERNAL_ASSERT_POINTER(psocket)

   if (UIPAllow::isAllowed(psocket->remoteIPAddress().getInAddr(), vallow_IP)) U_RETURN(true);

   U_RETURN(false);
}

#ifndef U_CACHE_REQUEST_DISABLE
__pure bool UClientImage_Base::isRequestCacheable()
{
   U_TRACE(0, "UClientImage_Base::isRequestCacheable()")

   if ((U_ClientImage_request & NO_CACHE) == 0   &&
        U_ClientImage_request_is_cached == false &&
       u_http_info.startHeader <= sizeof(cbuffer))
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}
#endif

// append on the log the peer certificate of client ("issuer","serial")

bool UClientImage_Base::logCertificate()
{
   U_TRACE(0, "UClientImage_Base::logCertificate()")

   // NB: OpenSSL has already tested the cert validity during SSL handshake and returns a X509 ptr just if the certificate is valid...

#if defined(USE_LIBSSL) && defined(U_LOG_ENABLE)
   X509* x509 = ((USSLSocket*)socket)->getPeerCertificate();

   if (x509)
      {
      U_INTERNAL_ASSERT_POINTER(logbuf)
      U_INTERNAL_ASSERT(UServer_Base::isLog())

      UCertificate::setForLog(x509, *logbuf);

      U_INTERNAL_ASSERT(logbuf->isNullTerminated())

      U_INTERNAL_DUMP("logbuf = %.*S", U_STRING_TO_TRACE(*logbuf))

      U_RETURN(true);
      }
#endif

   U_RETURN(false);
}

bool UClientImage_Base::askForClientCertificate()
{
   U_TRACE(0, "UClientImage_Base::askForClientCertificate()")

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

// define method VIRTUAL of class UEventFd

void UClientImage_Base::handlerDelete()
{
   U_TRACE(0, "UClientImage_Base::handlerDelete()")

   U_DUMP("UEventFd::fd = %d socket->iSockDesc = %d "
          "UNotifier::num_connection = %d UNotifier::min_connection = %d "
          "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B",
          UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
          UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask);

#if !defined(USE_LIBEVENT) && defined(HAVE_EPOLL_WAIT) && defined(DEBUG)
   if (UNLIKELY(UNotifier::num_connection <= UNotifier::min_connection))
      {
      U_WARNING("handlerDelete(): "
                "UEventFd::fd = %d socket->iSockDesc = %d "
                "UNotifier::num_connection = %d UNotifier::min_connection = %d "
                "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B",
                UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
                UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask);

      return;
      }
#endif

   bool bsocket_open = socket->isOpen();

#ifdef U_LOG_ENABLE
   if (UServer_Base::isLog())
      {
      U_INTERNAL_ASSERT_POINTER(logbuf)

      U_INTERNAL_DUMP("UEventFd::fd = %d logbuf = %.*S", UEventFd::fd, U_STRING_TO_TRACE(*logbuf))

      U_INTERNAL_ASSERT_DIFFERS(UEventFd::fd, -1)

      char buffer[32];
      uint32_t len = UServer_Base::getNumConnection(buffer);

      ULog::log("%s%.6s close connection from %.*s, %.*s clients still connected",
                  UServer_Base::mod_name[0], bsocket_open ? "Client" : "Server", U_STRING_TO_TRACE(*logbuf), len, buffer);

#  ifdef DEBUG
      int fd_logbuf = logbuf->strtol();

      if (UNLIKELY(fd_logbuf != UEventFd::fd))
         {
         U_WARNING("handlerDelete(): "
                   "UEventFd::fd = %d socket->iSockDesc = %d "
                   "UNotifier::num_connection = %d UNotifier::min_connection = %d "
                   "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B logbuf->strtol() = %d",
                   UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
                   UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask, fd_logbuf);
         }
#  endif
      }
#endif

   U_TOT_CONNECTION--;

   U_INTERNAL_DUMP("tot_connection = %d", U_TOT_CONNECTION)

#ifdef U_CLASSIC_SUPPORT
   if (UServer_Base::isClassic()) U_EXIT(0);
#endif

   if (bsocket_open) socket->close();

   --UNotifier::num_connection;

#ifdef U_LOG_ENABLE
   if (UServer_Base::isLog())
      {
      logbuf->setEmpty();

      if (UNotifier::num_connection == UNotifier::min_connection) ULog::log("Waiting for connection on port %u", UServer_Base::port);
      }
#endif

   if (data_pending)
      {
      delete data_pending;
             data_pending = 0;
      }
   else if (isPendingSendfile())
      {
      U_INTERNAL_DUMP("sfd = %d count = %u UEventFd::op_mask = %B pending_close = %d %B",
                       sfd,     count,     UEventFd::op_mask,     pending_close, pending_close)

      if ((pending_close & U_CLOSE) != 0)
         {
#     ifdef DEBUG
         if (UNLIKELY(sfd == -1))
            {
            U_ERROR("handlerDelete(): "
                    "UEventFd::fd = %d socket->iSockDesc = %d "
                    "UNotifier::num_connection = %d UNotifier::min_connection = %d "
                    "U_ClientImage_parallelization = %d sfd = %d UEventFd::op_mask = %B",
                    UEventFd::fd,   socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
                    U_ClientImage_parallelization, sfd, UEventFd::op_mask);
            }
#     endif

         pending_close = 0;

         UFile::close(sfd);
         }

      reset();
      }

   if (UNLIKELY(pending_close)) pending_close = 0;

   U_INTERNAL_ASSERT_EQUALS(data_pending, 0)
   U_INTERNAL_ASSERT_EQUALS(UEventFd::op_mask, EPOLLIN | EPOLLRDHUP)
#ifdef HAVE_ACCEPT4
   U_INTERNAL_ASSERT_EQUALS(((USocket::accept4_flags & SOCK_CLOEXEC)  != 0),((socket->flags & O_CLOEXEC)  != 0))
   U_INTERNAL_ASSERT_EQUALS(((USocket::accept4_flags & SOCK_NONBLOCK) != 0),((socket->flags & O_NONBLOCK) != 0))
#endif
}

void UClientImage_Base::handlerError()
{
   U_TRACE(0, "UClientImage_Base::handlerError()")

   U_INTERNAL_ASSERT_POINTER(socket)

   U_DUMP("UEventFd::fd = %d socket->iSockDesc = %d "
          "UNotifier::num_connection = %d UNotifier::min_connection = %d "
          "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B",
          UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
          UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask);

#if !defined(USE_LIBEVENT) && defined(HAVE_EPOLL_WAIT) && defined(DEBUG)
   if (UNLIKELY(socket->iSockDesc == -1))
      {
      U_WARNING("handlerError(): "
                "UEventFd::fd = %d socket->iSockDesc = %d "
                "UNotifier::num_connection = %d UNotifier::min_connection = %d "
                "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B",
                UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
                UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask);

      return;
      }

   U_INTERNAL_ASSERT_EQUALS(socket->iSockDesc, UEventFd::fd)
#endif

    pthis   = this;
   (psocket = socket)->iState = USocket::EPOLLERROR;

   UNotifier::handlerDelete(UEventFd::fd, UEventFd::op_mask);

   UClientImage_Base::handlerDelete();
}

int UClientImage_Base::handlerTimeout()
{
   U_TRACE(0, "UClientImage_Base::handlerTimeout()")

   U_DUMP("UEventFd::fd = %d socket->iSockDesc = %d "
          "UNotifier::num_connection = %d UNotifier::min_connection = %d "
          "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B",
          UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
          UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask);

#if !defined(USE_LIBEVENT) && defined(HAVE_EPOLL_WAIT) && defined(DEBUG)
   if (UNLIKELY(socket->iSockDesc == -1))
      {
      U_WARNING("handlerTimeout(): "
                "UEventFd::fd = %d socket->iSockDesc = %d "
                "UNotifier::num_connection = %d UNotifier::min_connection = %d "
                "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B",
                UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
                UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask);

      last_event = u_now->tv_sec;

      U_RETURN(U_NOTIFIER_OK);
      }

   U_INTERNAL_ASSERT_EQUALS(socket->iSockDesc, UEventFd::fd)
#endif

   // NB: maybe we have some more data to read...

   if (UNotifier::waitForRead(socket->iSockDesc, 0) == 1) U_RETURN(U_NOTIFIER_OK);

   pthis = this; // NB: U_SRV_LOG_WITH_ADDR macro depend on UClientImage_Base::pthis...

   socket->iState = USocket::TIMEOUT | USocket::EPOLLERROR;

   U_RETURN(U_NOTIFIER_DELETE);
}

bool UClientImage_Base::startRequest()
{
   U_TRACE(0, "UClientImage_Base::startRequest()")

#if !defined(DEBUG) && !defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST)
   U_gettimeofday; // NB: optimization if it is enough a time resolution of one second...
#else
   long tmp = chronometer->restart();

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b time_between_request = %ld time_run = %ld U_ClientImage_request_is_cached = %b csfd = %d",
                    U_ClientImage_pipeline,     time_between_request,      time_run,      U_ClientImage_request_is_cached,     csfd)

   if (U_ClientImage_pipeline == false &&
       U_ClientImage_parallelization == 0)
      {
      time_between_request = tmp;

#  ifndef U_CACHE_REQUEST_DISABLE
      if (U_ClientImage_request_is_cached) U_RETURN(false);
#  endif
#  ifdef USE_LIBSSL
      if (UServer_Base::bssl) U_RETURN(false);
#  endif

      if ((time_run - time_between_request) > 10) U_RETURN(true);
      }
#endif

   U_RETURN(false);
}

const char* UClientImage_Base::getRequestUri(uint32_t& sz)
{
   U_TRACE(0, "UClientImage_Base::getRequestUri(%p)", &sz)

   U_INTERNAL_DUMP("U_http_is_request_nostat = %b", U_http_is_request_nostat)

   const char* ptr;

#ifdef U_ALIAS
   if (*request_uri)
      {
      sz  = request_uri->size();
      ptr = request_uri->data();

      U_INTERNAL_DUMP("request_uri(%u) = %.*S", sz, sz, ptr)
      }
   else
#endif
   {
   sz  = u_http_info.uri_len;
   ptr = u_http_info.uri;

   U_INTERNAL_DUMP("u_http_info.uri(%u) = %.*S", sz, sz, ptr)
   }

   return ptr;
}

void UClientImage_Base::endRequest()
{
   U_TRACE(0, "UClientImage_Base::endRequest()")

#if !defined(DEBUG) && !defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST)
   U_gettimeofday; // NB: optimization if it is enough a time resolution of one second...
#else
   time_run = chronometer->stop();

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b time_between_request = %ld time_run = %ld U_ClientImage_request_is_cached = %b",
                    U_ClientImage_pipeline,     time_between_request,      time_run,      U_ClientImage_request_is_cached)
#endif

   if (callerHandlerEndRequest &&
       U_ClientImage_parallelization != 2) // 2 => parent of parallelization
      {
      callerHandlerEndRequest();
      }

   U_http_method_type = 0; // NB: this mark the end of http request processing...

#if defined(DEBUG) && defined(U_LOG_ENABLE)
   if (UServer_Base::isLog())
      {
      uint32_t sz = 0;
      const char* ptr;

#  ifndef U_CACHE_REQUEST_DISABLE
      if (U_ClientImage_request_is_cached)
         {
         U_INTERNAL_ASSERT_MAJOR(uri_len, 0)
         U_INTERNAL_ASSERT_MAJOR(uri_offset, 0)

         sz  = uri_len;
         ptr = request->c_pointer(uri_offset);
         }
      else
#  endif
      {
      ptr = getRequestUri(sz);
      }

      // NB: URI requested can be URL encoded (ex: vuoto%2Etxt) so we cannot use snprintf()...

      char buffer[256];
      char* ptr1 = buffer;

      u__memcpy(ptr1, "request \"", U_CONSTANT_SIZE("request \""), __PRETTY_FUNCTION__);
                ptr1 +=             U_CONSTANT_SIZE("request \"");

      if (sz)
         {
         if (sz > (sizeof(buffer)-32)) sz = sizeof(buffer)-32;

         u__memcpy(ptr1, ptr, sz, __PRETTY_FUNCTION__);
                   ptr1 +=    sz;
         }

      u__memcpy(ptr1, "\" run in ", U_CONSTANT_SIZE("\" run in "), __PRETTY_FUNCTION__);
                ptr1 +=             U_CONSTANT_SIZE("\" run in ");

      if (time_run > 0L) ptr1 += u__snprintf(ptr1, sizeof(buffer)-(ptr1-buffer), "%ld ms", time_run);
      else               ptr1 += u__snprintf(ptr1, sizeof(buffer)-(ptr1-buffer),  "%g ms", chronometer->getTimeElapsed());

      U_INTERNAL_ASSERT_MINOR((ptrdiff_t)(ptr1-buffer), (ptrdiff_t)sizeof(buffer))

      ULog::write(buffer, ptr1-buffer);
      }
#endif

#ifdef U_ALIAS
   if (request_uri->isNull() == false) request_uri->clear();
#endif

   u__memcpy(iov_vec, iov_sav, U_IOV_TO_SAVE, __PRETTY_FUNCTION__);
}

void UClientImage_Base::manageReadBufferResize(UString& buffer, uint32_t n)
{
   U_TRACE(0, "UClientImage_Base::manageReadBufferResize(%.*S,%u)", U_STRING_TO_TRACE(buffer), n)

   ptrdiff_t diff;
   const char* ptr;

   U_DUMP("U_ClientImage_pipeline = %b size_request = %u rbuffer->size() = %u rbuffer->capacity() = %u request->size() = %u rstart = %u",
           U_ClientImage_pipeline,     size_request,     rbuffer->size(),     rbuffer->capacity(),     request->size(),     rstart)

   U_ASSERT_EQUALS(*request, buffer)

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
         U_INTERNAL_DUMP("diff = %d request_uri = %.*S", diff, U_STRING_TO_TRACE(*request_uri))

         U_ASSERT(request_uri->empty())

         U_INTERNAL_ASSERT_POINTER(u_http_info.uri)

                                    u_http_info.uri    += diff;
         if (u_http_info.query_len) u_http_info.query  += diff;

#     ifdef DEBUG
      // U_MESSAGE("manageReadBufferResize(%u): uri = %.*S query = %.*S", n, U_HTTP_URI_TO_TRACE, U_HTTP_QUERY_TO_TRACE);
#     endif

         U_INTERNAL_DUMP("uri             = %.*S", U_HTTP_URI_TO_TRACE)
         U_INTERNAL_DUMP("query           = %.*S", U_HTTP_QUERY_TO_TRACE)

         U_INTERNAL_ASSERT_DIFFERS(u_http_info.uri[0], 0)

         if (U_http_host_len)             u_http_info.host            += diff;
         if (U_http_range_len)            u_http_info.range           += diff;
         if (U_http_accept_len)           u_http_info.accept          += diff;
         if (U_http_ip_client_len)        u_http_info.ip_client       += diff;
         if (U_http_websocket_len)        u_http_info.websocket       += diff;
         if (U_http2_settings_len)        u_http_info.http2_settings  += diff;
         if (u_http_info.cookie_len)      u_http_info.cookie          += diff;
         if (u_http_info.referer_len)     u_http_info.referer         += diff;
         if (U_http_content_type_len)     u_http_info.content_type    += diff;
         if (u_http_info.user_agent_len)  u_http_info.user_agent      += diff;
         if (U_http_accept_language_len)  u_http_info.accept_language += diff;


         U_INTERNAL_DUMP("host            = %.*S", U_HTTP_HOST_TO_TRACE)
         U_INTERNAL_DUMP("vhost           = %.*S", U_HTTP_VHOST_TO_TRACE)
         U_INTERNAL_DUMP("range           = %.*S", U_HTTP_RANGE_TO_TRACE)
         U_INTERNAL_DUMP("ctype           = %.*S", U_HTTP_CTYPE_TO_TRACE)
         U_INTERNAL_DUMP("cookie          = %.*S", U_HTTP_COOKIE_TO_TRACE)
         U_INTERNAL_DUMP("accept          = %.*S", U_HTTP_ACCEPT_TO_TRACE)
         U_INTERNAL_DUMP("referer         = %.*S", U_HTTP_REFERER_TO_TRACE)
         U_INTERNAL_DUMP("ip_client       = %.*S", U_HTTP_IP_CLIENT_TO_TRACE)
         U_INTERNAL_DUMP("websocket       = %.*S", U_HTTP_WEBSOCKET_TO_TRACE)
         U_INTERNAL_DUMP("user_agent      = %.*S", U_HTTP_USER_AGENT_TO_TRACE)
         U_INTERNAL_DUMP("http2_settings  = %.*S", U_HTTP2_SETTINGS_TO_TRACE)
         U_INTERNAL_DUMP("accept_language = %.*S", U_HTTP_ACCEPT_LANGUAGE_TO_TRACE)
         }
      }

   *request = *rbuffer;

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_data_missing = %b", U_ClientImage_pipeline, U_ClientImage_data_missing)
}

void UClientImage_Base::resetReadBuffer()
{
   U_TRACE(0, "UClientImage_Base::resetReadBuffer()")

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

bool UClientImage_Base::genericRead()
{
   U_TRACE(0, "UClientImage_Base::genericRead()")

   U_DUMP("UEventFd::fd = %d socket->iSockDesc = %d "
          "UNotifier::num_connection = %d UNotifier::min_connection = %d "
          "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B",
          UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
          UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask);

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

#if defined(DEBUG) || defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST)
   bool advise_for_parallelization =
#else
   (void)
#endif
   startRequest();

   request->clear(); // reset buffer before read

   U_INTERNAL_DUMP("rbuffer(%u) = %.*S", rbuffer->size(), U_STRING_TO_TRACE(*rbuffer))

   rbuffer->setBuffer(U_CAPACITY); // NB: this string can be referenced more than one (often if U_SUBSTR_INC_REF is defined)...

   // NB: timeout == 0 means that we put the socket fd on epoll queue if EAGAIN...

   socket->iState = USocket::CONNECT;

   if (USocketExt::read(socket, *rbuffer, U_SINGLE_READ, 0) == false)
      {
      U_ClientImage_state = (socket->isOpen() ? U_PLUGIN_HANDLER_AGAIN
                                              : U_PLUGIN_HANDLER_ERROR);

      U_RETURN(false);
      }

   if (UNLIKELY(data_pending))
      {
      U_INTERNAL_DUMP("data_pending(%u) = %.*S", data_pending->size(), U_STRING_TO_TRACE(*data_pending))

      (void) rbuffer->insert(0, *data_pending);

      delete data_pending;
             data_pending = 0;

      U_INTERNAL_DUMP("rbuffer(%u) = %.*S", rbuffer->size(), U_STRING_TO_TRACE(*rbuffer))
      }

#if defined(DEBUG) || defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST)
   if (advise_for_parallelization)
      {
#  ifndef U_SERVER_CHECK_TIME_BETWEEN_REQUEST
      U_MESSAGE("UClientImage_Base::genericRead(): time_between_request(%ld) < time_run(%ld)", time_between_request, time_run);
#  else
      if (UServer_Base::startParallelization(UNotifier::max_connection-(UNotifier::max_connection/10)))
         {
         // parent

         U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;

         U_RETURN(false);
         }
#  endif
      }
#endif

   pthis   = this;
   psocket = socket;

   UServer_Base::client_address     = socket->cRemoteAddress.pcStrAddress;
   UServer_Base::client_address_len = u__strlen(UServer_Base::client_address, __PRETTY_FUNCTION__);

   U_INTERNAL_DUMP("UServer_Base::client_address = %.*S", U_CLIENT_ADDRESS_TO_TRACE)

   U_ClientImage_state = U_PLUGIN_HANDLER_GO_ON;

   U_RETURN(true);
}

int UClientImage_Base::handlerRead() // Connection-wide hooks
{
   U_TRACE(0, "UClientImage_Base::handlerRead()")

   u_clientimage_flag.u = 0;

#ifdef U_CLASSIC_SUPPORT
   if (UServer_Base::isClassic())
      {
      U_ASSERT(UServer_Base::proc->child())

      U_ClientImage_parallelization = 1; // 1 => child of parallelization
      }
#endif

start:
   U_INTERNAL_ASSERT_EQUALS(U_ClientImage_pipeline,     false)
   U_INTERNAL_ASSERT_EQUALS(U_ClientImage_data_missing, false)

   if (genericRead() == false)
      {
      if (U_ClientImage_state           == U_PLUGIN_HANDLER_AGAIN &&
          U_ClientImage_parallelization != 1) // 1 => child of parallelization
         {
         U_INTERNAL_ASSERT(socket->isOpen())

         U_RETURN(U_NOTIFIER_OK); // NOT BLOCKING...
         }

      goto end;
      }

   U_INTERNAL_ASSERT(socket->isOpen())

   rstart = 0;

loop:
   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b size_request = %u rstart = %u rbuffer(%u) = %.*S",
                    U_ClientImage_pipeline,     size_request,     rstart,     rbuffer->size(), U_STRING_TO_TRACE(*rbuffer))

   if (LIKELY(U_ClientImage_pipeline == false)) *request = *rbuffer;
   else
      {
      uint32_t sz = rbuffer->size();

      U_ASSERT_MINOR(rstart, sz)
      U_INTERNAL_ASSERT_MAJOR(rstart, 0)
      U_INTERNAL_ASSERT_EQUALS(request->same(*rbuffer), false)

      sz -= rstart;

      if (size_request <= sz) *request = rbuffer->substr(rstart, sz);
      else
         {
         resetReadBuffer();

         if (callerIsValidRequestExt(request->data(), sz) == false) U_ClientImage_data_missing = true; // partial valid (not complete)
         }
      }

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_data_missing = %b", U_ClientImage_pipeline, U_ClientImage_data_missing)

   U_INTERNAL_ASSERT(request->invariant())

#ifdef U_LOG_ENABLE
   if (logbuf) logRequest();
#endif

   if (U_ClientImage_data_missing)
      {
dmiss:
      U_ClientImage_data_missing = false;

      U_INTERNAL_ASSERT_EQUALS(data_pending, 0)

      U_INTERNAL_DUMP("U_http_version = %C", U_http_version)

      if (U_http_version == '2')
         {
         // TODO
         }
      else
         {
         if (UServer_Base::startParallelization(UServer_Base::num_client_for_parallelization))
            {
            // parent

            U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;

            U_RETURN(U_NOTIFIER_DELETE);
            }

         if (U_ClientImage_parallelization == 1) // 1 => child of parallelization
            {
            if (UNotifier::waitForRead(socket->iSockDesc, U_TIMEOUT_MS) != 1 ||
                (resetReadBuffer(), USocketExt::read(socket, *rbuffer, size_request == 0 ? U_SINGLE_READ : size_request - rbuffer->size(), 0)) == false)
               {
               goto death;
               }

            goto loop;
            }

         data_pending = U_NEW(UString((void*)U_STRING_TO_PARAM(*request)));

         U_INTERNAL_DUMP("data_pending(%u) = %.*S", data_pending->size(), U_STRING_TO_TRACE(*data_pending))
         }

      U_INTERNAL_ASSERT(socket->isOpen())
      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, 1) // 1 => child of parallelization

      U_RETURN(U_NOTIFIER_OK);
      }

#ifndef U_CACHE_REQUEST_DISABLE
   U_INTERNAL_DUMP("U_ClientImage_request_is_cached = %b", U_ClientImage_request_is_cached)

   if (U_ClientImage_request_is_cached)
      {
      uint32_t sz     = request->size();
      const char* ptr = request->data();

      U_INTERNAL_DUMP("cbuffer(%u) = %.*S", u_http_info.startHeader, u_http_info.startHeader, cbuffer)
      U_INTERNAL_DUMP("request(%u) = %.*S", sz, sz, ptr)

      U_INTERNAL_DUMP("U_ClientImage_pipeline = %b size_request = %u uri_offset = %u",
                       U_ClientImage_pipeline,     size_request,     uri_offset)

      U_INTERNAL_ASSERT_MAJOR(size_request, 0)
      U_INTERNAL_ASSERT_MAJOR(u_http_info.uri_len, 0)
      U_INTERNAL_ASSERT_MAJOR(u_http_info.startHeader, 0)
      U_INTERNAL_ASSERT_EQUALS(U_ClientImage_data_missing, false)

      if (u__isblank((ptr+uri_offset)[u_http_info.startHeader]) &&
          memcmp(ptr+uri_offset, cbuffer, u_http_info.startHeader) == 0)
         {
         if (size_request > sz &&
             callerIsValidRequestExt(ptr, sz) == false)
            {
            goto dmiss; // partial valid (not complete)
            }

         if (callerHandlerCache())
            {
            setRequestProcessed();

            goto next;
            }
         }

      csfd                            = -1;
      U_ClientImage_request_is_cached = false;
      }
#endif

   if (body->isNull() == false) body->clear();

   U_INTERNAL_DUMP("wbuffer(%u) = %.*S", wbuffer->size(), U_STRING_TO_TRACE(*wbuffer))

   wbuffer->setBuffer(U_CAPACITY); // NB: this string can be referenced more than one (often if U_SUBSTR_INC_REF is defined)...

   U_ClientImage_request          = 0;
   USocketExt::read_buffer_resize = manageReadBufferResize;

   U_ClientImage_state = callerHandlerRead();

   USocketExt::read_buffer_resize = UString::_reserve;

   U_INTERNAL_DUMP("socket->isClosed() = %b u_http_info.nResponseCode = %u U_ClientImage_close = %b U_ClientImage_state = %d %B",
                    socket->isClosed(),     u_http_info.nResponseCode,     U_ClientImage_close,     U_ClientImage_state, U_ClientImage_state)

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
#  if defined(U_LOG_ENABLE) && !defined(U_COVERITY_FALSE_POSITIVE)
      if (log_request_partial) logRequest();
#  endif
#  ifndef U_CACHE_REQUEST_DISABLE
next:
#  endif
      uint32_t sz = rbuffer->size();

      if (LIKELY(U_ClientImage_pipeline == false))
         {
         const char* ptr1 = rbuffer->c_pointer(size_request);

         // NB: we check if we have a pipeline...

         if (size_request < sz &&
             callerIsValidRequest(ptr1))
            {
            U_ClientImage_pipeline = true;

#        ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
                 resto = (sz % size_request);
            uint32_t n = (sz / size_request);

            U_INTERNAL_DUMP("n = %u resto = %u size_request = %u", n, resto, size_request)

            if (n > 1                                       &&
                *wbuffer                                    &&
                callerIsValidRequestExt(ptr1, size_request) &&
                (resto == 0 || callerIsValidRequest(rbuffer->c_pointer(sz-resto))))
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

               if (resto    == 0 &&
                   nrequest == n)
                  {
                  U_INTERNAL_DUMP("U_ClientImage_close = %b", U_ClientImage_close)

                  if (U_ClientImage_close == false) U_ClientImage_pipeline = false;
                  }
               else
                  {
                  *request = rbuffer->substr((rstart = (nrequest * size_request)));
                  }

               U_INTERNAL_DUMP("request(%u) = %.*S", request->size(), U_STRING_TO_TRACE(*request))

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

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b size_request = %u request->size() = %u",
                    U_ClientImage_pipeline,     size_request,     request->size())

   if (isRequestNeedProcessing())
      {
      U_INTERNAL_ASSERT_POINTER(callerHandlerRequest)
      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, 2) // 2 => parent of parallelization
      U_INTERNAL_ASSERT_EQUALS(U_ClientImage_state & (U_PLUGIN_HANDLER_AGAIN | U_PLUGIN_HANDLER_ERROR), 0)

      U_ClientImage_state = callerHandlerRequest();

      if (UNLIKELY(socket->isClosed())) goto error;
      }

   U_INTERNAL_DUMP("socket->isClosed() = %b u_http_info.nResponseCode = %u U_ClientImage_close = %b U_ClientImage_state = %d %B",
                    socket->isClosed(),     u_http_info.nResponseCode,     U_ClientImage_close,     U_ClientImage_state, U_ClientImage_state)

   U_INTERNAL_DUMP("wbuffer(%u) = %.*S", wbuffer->size(), U_STRING_TO_TRACE(*wbuffer))
   U_INTERNAL_DUMP("   body(%u) = %.*S",    body->size(), U_STRING_TO_TRACE(*body))

   if (LIKELY(*wbuffer))
      {
      U_INTERNAL_DUMP("u_http_info.nResponseCode = %u count = %u UEventFd::op_mask = %d %B",
                       u_http_info.nResponseCode,     count,     UEventFd::op_mask, UEventFd::op_mask)

      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, 2) // 2 => parent of parallelization

      if (count == 0)
         {
#ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
write:
#endif
         U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_data_missing = %b", U_ClientImage_pipeline, U_ClientImage_data_missing)

         if (handlerResponse() == U_NOTIFIER_DELETE) goto error;

#     ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
         U_INTERNAL_DUMP("nrequest = %u resto = %u U_ClientImage_pipeline = %b U_ClientImage_close = %b rstart = %u",
                          nrequest,     resto,     U_ClientImage_pipeline,     U_ClientImage_close,     rstart)

         if (nrequest)
            {
            nrequest = 0;

            if (UNLIKELY(resto))
               {
               resto = 0;

               endRequest();

               goto dmiss;
               }

            if (U_ClientImage_pipeline &&
                (rstart == 0 || resto == 0))
               {
               U_INTERNAL_ASSERT(U_ClientImage_close)

               U_ClientImage_pipeline = false;
               }
            }
#     endif
         }
      else
         {
         // NB: we are managing a sendfile() request...

         U_INTERNAL_ASSERT_EQUALS(nrequest, 0)
         U_INTERNAL_ASSERT_EQUALS(UEventFd::op_mask, EPOLLIN | EPOLLRDHUP)

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

         goto loop;
         }

      if (callerHandlerReset) U_ClientImage_state = callerHandlerReset();
      }
   else
      {
      U_INTERNAL_ASSERT_EQUALS(U_ClientImage_pipeline, false)
error:
      U_INTERNAL_ASSERT_DIFFERS(UEventFd::fd, -1)

      U_ClientImage_close = true;
      }

   endRequest();

   U_DUMP("U_ClientImage_close = %b UServer_Base::isParallelizationChild() = %b", U_ClientImage_close, UServer_Base::isParallelizationChild())

   if (U_ClientImage_close)
      {
end:  if (U_ClientImage_parallelization == 1) goto death; // 1 => child of parallelization

      U_RETURN(U_NOTIFIER_DELETE);
      }

   // NB: maybe we have some more request to services on the same connection...

   if (U_ClientImage_parallelization == 1)
      {
#  ifdef DEBUG
      if (count) // NB: we must not have pending write...
         {
         U_ERROR("handlerRead(): "
                 "UEventFd::fd = %d socket->iSockDesc = %d "
                 "UNotifier::num_connection = %d UNotifier::min_connection = %d "
                 "count = %u sfd = %d UEventFd::op_mask = %B",
                 UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
                 count, sfd, UEventFd::op_mask);
         }
#  endif

      U_INTERNAL_ASSERT_DIFFERS(socket->iSockDesc, -1)

      if (UNotifier::waitForRead(socket->iSockDesc, U_TIMEOUT_MS) == 1) goto start;

death:
      UServer_Base::endNewChild(); // no return;
      }

   last_event = u_now->tv_sec;

   U_INTERNAL_ASSERT(socket->isOpen())
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, 1) // 1 => child of parallelization

   U_RETURN(U_NOTIFIER_OK);
}

bool UClientImage_Base::writeResponse()
{
   U_TRACE(0, "UClientImage_Base::writeResponse()")

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_close = %b nrequest = %u", U_ClientImage_pipeline, U_ClientImage_close, nrequest)

   U_INTERNAL_ASSERT(*wbuffer)
   U_INTERNAL_ASSERT(socket->isOpen())
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, 2) // 2 => parent of parallelization

   int iBytesWrite;
   uint32_t sz1     = wbuffer->size(),
            sz2     = (U_http_method_type == HTTP_HEAD ? 0 : body->size()),
            msg_len = (U_ClientImage_pipeline ? U_CONSTANT_SIZE("[pipeline] ") : 0);

   iov_vec[2].iov_len  = sz1;
   iov_vec[2].iov_base = (caddr_t) wbuffer->data();
   iov_vec[3].iov_len  = sz2;
   iov_vec[3].iov_base = (caddr_t) body->data();

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

      u__memcpy(iov_sav, iov_vec, U_IOV_TO_SAVE, __PRETTY_FUNCTION__);

      ULog::updateStaticDate(UServer_Base::ptr_static_date->date3+6, 3);

#  ifdef U_LOG_ENABLE
      if (logbuf) ULog::log(iov_vec+idx, UServer_Base::mod_name[0], "response", ncount, "[pipeline] ", msg_len, " to %.*s", U_STRING_TO_TRACE(*logbuf));
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
   iBytesWrite = USocketExt::writev( socket, iov_vec+idx, iovcnt, ncount, 0);
#else
   iBytesWrite = USocketExt::_writev(socket, iov_vec+idx, iovcnt, ncount, 0);
#endif
   }

   if (iBytesWrite == (int)ncount) U_RETURN(true);

   if (iBytesWrite == 0 ||
       socket->isClosed())
      {
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
                        iBytesWrite, ncount, msg_len, " [pipeline]", U_ClientImage_parallelization, U_CNT_PARALLELIZATION);

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
      U_ERROR("sum = %u ncount = %u nrequest = %u iBytesWrite = %u iov_vec[%u].iov_len = %u iovcnt = %u", sum, ncount, nrequest, iBytesWrite, idx, iov_vec[idx].iov_len, iovcnt);
      }
#endif

#ifdef USE_LIBSSL
   if (UServer_Base::bssl == false)
#endif
   {
#ifdef U_CLIENT_RESPONSE_PARTIAL_WRITE_SUPPORT
   if (U_ClientImage_parallelization != 1) goto end; // 1 => child of parallelization
#endif
   }

#if defined(USE_LIBSSL) || defined(_MSWINDOWS_)
   iBytesWrite = USocketExt::writev( socket, piov, iovcnt, ncount, U_TIMEOUT_MS);
#else
   iBytesWrite = USocketExt::_writev(socket, piov, iovcnt, ncount, U_TIMEOUT_MS);
#endif

   if (iBytesWrite != (int)ncount)
      {
      if (iBytesWrite > 0)
         {
         if (UServer_Base::bssl ||
             U_ClientImage_parallelization == 1) // NB: we must not have pending write...
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
      bopen = socket->isOpen();

      U_SRV_LOG_WITH_ADDR("sending partial response: failed - sk %s, (%u bytes of %u)%.*s to", bopen ? "open" : "close", iBytesWrite, ncount, msg_len, " [pipeline]");

      if (bopen) socket->close();

      goto end;
      }

   result = true;

   U_SRV_LOG_WITH_ADDR("sending partial response: completed (%u bytes of %u)%.*s to", iBytesWrite, ncount, msg_len, " [pipeline]");

end:
#ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
   if (nrequest) u_buffer_len = 0;
#endif

   if (bflag) socket->setNonBlocking(); // restore file status flags

   U_RETURN(result);
}

void UClientImage_Base::prepareForSendfile()
{
   U_TRACE(0, "UClientImage_Base::prepareForSendfile()")

   U_INTERNAL_ASSERT_MAJOR(count, 0)
   U_INTERNAL_ASSERT_DIFFERS(sfd, -1)

   resetPipeline();

   if (U_ClientImage_close)
      {
      pending_close       = U_YES;
      U_ClientImage_close = false;
      }

   UEventFd::op_mask = EPOLLOUT;

   if (UNotifier::isHandler(UEventFd::fd)) UNotifier::modify(this);

   U_INTERNAL_DUMP("start = %u count = %u", start, count)
}

int UClientImage_Base::handlerResponse()
{
   U_TRACE(0, "UClientImage_Base::handlerResponse()")

   if (writeResponse()) U_RETURN(U_NOTIFIER_OK);

   if (socket->isOpen())
      {
      U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)
      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, 1) // NB: we must not have pending write...
      U_INTERNAL_ASSERT_EQUALS(UEventFd::op_mask, EPOLLIN | EPOLLRDHUP)

#  ifndef U_CLIENT_RESPONSE_PARTIAL_WRITE_SUPPORT
      resetPipelineAndClose();
#  else
      char path[MAX_FILENAME_LEN];

      // By default, /tmp on Fedora 18 will be on a tmpfs. Storage of large temporary files should be done in /var/tmp.
      // This will reduce the I/O generated on disks, increase SSD lifetime, save power, and improve performance of the /tmp filesystem. 

      const char* format = (ncount < (2 * 1024 * 1024)
                                 ?     "/tmp/pwrite.%P.%4D"
                                 : "/var/tmp/pwrite.%P.%4D");

      uint32_t len = u__snprintf(path, sizeof(path), format, 0);

      sfd = UFile::creat(path);

      if (sfd == -1)
         {
         U_SRV_LOG("partial write failed: (remain %u bytes) - error on create temporary file %.*S sock_fd %d sfd %d", ncount, len, path, socket->iSockDesc, sfd);

         U_RETURN(U_NOTIFIER_DELETE);
         }

      (void) UFile::_unlink(path);

      int iBytesWrite = UFile::writev(sfd, piov, iovcnt);

      if (iBytesWrite != (int)ncount)
         {
         U_SRV_LOG("partial write failed: (remain %u bytes) - error on write (%d bytes) on temporary file %.*S sock_fd %d sfd %d", ncount, iBytesWrite, len, path, socket->iSockDesc, sfd);

         UFile::close(sfd);
                      sfd = -1;

         U_RETURN(U_NOTIFIER_DELETE);
         }

      U_SRV_LOG("partial write: (remain %u bytes) - create temporary file %.*S sock_fd %d sfd %d", ncount, len, path, socket->iSockDesc, sfd);

      // NB: we have a pending sendfile...

      count = ncount;

      prepareForSendfile();

      pending_close |= U_CLOSE;

      U_RETURN(U_NOTIFIER_OK);
#  endif
      }

   U_INTERNAL_ASSERT_DIFFERS(UEventFd::fd, -1)

   U_RETURN(U_NOTIFIER_DELETE);
}

int UClientImage_Base::handlerWrite()
{
   U_TRACE(0, "UClientImage_Base::handlerWrite()")

   U_DUMP("UEventFd::fd = %d socket->iSockDesc = %d "
          "UNotifier::num_connection = %d UNotifier::min_connection = %d "
          "UServer_Base::isParallelizationChild() = %b sfd = %d UEventFd::op_mask = %B",
          UEventFd::fd, socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection,
          UServer_Base::isParallelizationChild(), sfd, UEventFd::op_mask);

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

   off_t offset;
   int iBytesWrite;
   bool bwrite = (UEventFd::op_mask == EPOLLOUT);

   U_INTERNAL_DUMP("bwrite = %b", bwrite)

#ifdef U_LOG_ENABLE
   pthis = this; // NB: U_SRV_LOG_WITH_ADDR macro depend on UClientImage_Base::pthis...
#endif

write:
   offset      = start;
   iBytesWrite = USocketExt::sendfile(socket, sfd, &offset, count, 0);

   if (iBytesWrite == (int)count)
      {
      U_SRV_LOG_WITH_ADDR("sending sendfile response completed (%u bytes of %u) to", iBytesWrite, count);

      if (bwrite)
         {
         UEventFd::op_mask = EPOLLIN | EPOLLRDHUP;

         UNotifier::modify(this);
         }
#  ifdef DEBUG
      else
         {
         U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, 2) // 2 => parent of parallelization
         U_INTERNAL_ASSERT_EQUALS(UEventFd::op_mask, EPOLLIN | EPOLLRDHUP)
         }
#  endif

      if ((pending_close & U_CLOSE) != 0) UFile::close(sfd);

      start =
      count =  0;
      sfd   = -1;

      if ((pending_close & U_YES) != 0) U_RETURN(U_NOTIFIER_DELETE);

      U_RETURN(U_NOTIFIER_OK);
      }

   if (iBytesWrite > 0)
      {
      U_SRV_LOG_WITH_ADDR("sent sendfile partial response (%u bytes of %u) to", iBytesWrite, count);

      start += iBytesWrite;
      count -= iBytesWrite;

      U_INTERNAL_ASSERT_MAJOR(count, 0)

      if (bwrite) U_RETURN(U_NOTIFIER_OK);

      if (U_ClientImage_parallelization == 1) // 1 => child of parallelization
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

      if (U_ClientImage_parallelization == 1) goto wait; // 1 => child of parallelization

      prepareForSendfile();

      U_RETURN(U_NOTIFIER_OK);
      }

   U_SRV_LOG("sendfile failed - sock_fd %d sfd %d count %u pending_close %d %B", socket->iSockDesc, sfd, count, pending_close, pending_close);

   if ((pending_close & U_CLOSE) != 0) UFile::close(sfd);

   start =
   count =  0;
   sfd   = -1;

   pending_close = 0;

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
