// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    client_image.h - Handles accepted connections from UServer's client
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_CLIENT_IMAGE_H
#define U_CLIENT_IMAGE_H 1

#include <ulib/timeval.h>
#include <ulib/event/event_fd.h>
#include <ulib/internal/chttp.h>
#include <ulib/net/server/server_plugin.h>

#ifdef USE_LIBSSL
#  include <ulib/ssl/certificate.h>
#  include <ulib/ssl/net/sslsocket.h>
#endif

#ifdef USE_LIBURING
#  ifndef U_IO_BUFFER_SIZE
#  define U_IO_BUFFER_SIZE U_1M // 1 MB in bytes
#  endif
#endif

/**
 * @class UClientImage
 *
 * @brief Handles accepted connections from UServer's client (A representation of the Server responsible for handling Client instances)
 */

class UHTTP;
class UHTTP2;
class UIPAllow;
class UEventDB;
class USSIPlugIn;
class USocketExt;
class UHttpPlugIn;
class UNoCatPlugIn;
class UServer_Base;
class UStreamPlugIn;
class UBandWidthThrottling;

template <class T> class UServer;
template <class T> class UHashMap;

#define U_ClientImage_request_is_cached UClientImage_Base::cbuffer[0]

#define U_ClientImage_status(obj)        (obj)->UClientImage_Base::flag.c[0]
#define U_ClientImage_op_pending(obj)    (obj)->UClientImage_Base::flag.c[1]
#define U_ClientImage_write_pending(obj) (obj)->UClientImage_Base::flag.c[2]
#define U_ClientImage_user_value(obj)    (obj)->UClientImage_Base::flag.c[3]

class U_EXPORT UClientImage_Base : public UEventFd {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // SERVICES

   bool    genericRead();
   void prepareForRead();

   bool isPendingSendfile()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isPendingSendfile()")

      U_INTERNAL_DUMP("sfd = %d count = %I", sfd, count)

      if (count > 0) U_RETURN(true);

      U_RETURN(false);
      }

   // define method VIRTUAL of class UEventFd

   virtual int  handlerRead() U_DECL_OVERRIDE;
   virtual int  handlerWrite() U_DECL_FINAL;
   virtual int  handlerTimeout() U_DECL_FINAL;
   virtual void handlerDelete() U_DECL_FINAL;

   static void init();
   static void clear();

   static void          close();
   static void abortive_close();

   static const char* getRequestUri(uint32_t& len);

   static bool isAllowed(UVector<UIPAllow*>& vallow_IP) __pure; // Check whether the ip address client ought to be allowed

   static void setHeaderForResponse(uint32_t len)
      {
      U_TRACE(0, "UClientImage_Base::setHeaderForResponse(%u)", len)

      iov_vec[1].iov_len = len;

      U_INTERNAL_DUMP("UClientImage_Base::iov_vec[1] = %.*S", UClientImage_Base::iov_vec[1].iov_len, UClientImage_Base::iov_vec[1].iov_base)
      }

   // manage if other data already available... (pipelining)

   static void resetPipeline();
   static void setCloseConnection()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setCloseConnection()")

      U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_parallelization = %d U_ClientImage_request_is_cached = %b U_ClientImage_close = %b",
                       U_ClientImage_pipeline,     U_ClientImage_parallelization,     U_ClientImage_request_is_cached,     U_ClientImage_close)

      if (U_ClientImage_pipeline == false) U_ClientImage_close = true;
      }

   static void resetPipelineAndSetCloseConnection()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::resetPipelineAndSetCloseConnection()")

      // NB: because we close the connection we don't need to process other request in pipeline...

      U_ClientImage_close = true;

      if (U_ClientImage_pipeline) resetPipeline();
      }

   // request state processing

   enum RequestStatusType {
   // NOT_FOUND            = 0x00,
      FORBIDDEN            = 0x01,
      NO_CACHE             = 0x02,
      IN_FILE_CACHE        = 0x04,
      ALREADY_PROCESSED    = 0x08,
      FILE_CACHE_PROCESSED = 0x10,
      REQUEST_FROM_USERVER = 0x20
   };

   static bool isRequestNotFound()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestNotFound()")

      U_INTERNAL_DUMP("U_ClientImage_request = %u %B", U_ClientImage_request, U_ClientImage_request)

#  ifdef U_CACHE_REQUEST_DISABLE
      if (U_ClientImage_request == 0) U_RETURN(true);
#  else
      if ((U_ClientImage_request & ~NO_CACHE) == 0) U_RETURN(true);
#  endif

      U_RETURN(false);
      }

   static bool isRequestAlreadyProcessed()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestAlreadyProcessed()")

      if ((U_ClientImage_request & ALREADY_PROCESSED) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static void setRequestNeedProcessing()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setRequestNeedProcessing()")

      U_ASSERT(isRequestAlreadyProcessed())

      U_ClientImage_request &= ~ALREADY_PROCESSED;
      }

   static bool isRequestNeedProcessing()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestNeedProcessing()")

      if ((U_ClientImage_request & ALREADY_PROCESSED) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   static void setRequestProcessed()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setRequestProcessed()")

      U_ASSERT(isRequestNeedProcessing())

      U_ClientImage_request |= ALREADY_PROCESSED;
      }

   static bool isRequestForbidden()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestForbidden()")

      if ((U_ClientImage_request & FORBIDDEN) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static void setRequestForbidden()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setRequestForbidden()")

      U_ASSERT_EQUALS(isRequestForbidden(), false)

      U_ClientImage_request |= FORBIDDEN;
      }

   static bool isRequestInFileCache()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestInFileCache()")

      if ((U_ClientImage_request & IN_FILE_CACHE) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static void setRequestInFileCache()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setRequestInFileCache()")

      U_ASSERT_EQUALS(isRequestInFileCache(), false)

      U_ClientImage_request |= IN_FILE_CACHE;
      }

   static bool isRequestFileCacheProcessed()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestFileCacheProcessed()")

      if ((U_ClientImage_request & FILE_CACHE_PROCESSED) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static void setRequestFileCacheProcessed()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setRequestFileCacheProcessed()")

      U_ASSERT_EQUALS(isRequestFileCacheProcessed(), false)

      U_ClientImage_request |= FILE_CACHE_PROCESSED;
      }

   static bool isRequestRedirected()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestRedirected()")

      U_INTERNAL_DUMP("U_ClientImage_request = %u %B U_http_info.nResponseCode = %u", U_ClientImage_request,
                       U_ClientImage_request,        U_http_info.nResponseCode)

      if ((U_ClientImage_request & ALREADY_PROCESSED) != 0 &&
          (U_http_info.nResponseCode == HTTP_MOVED_TEMP    ||
           U_http_info.nResponseCode == HTTP_NETWORK_AUTHENTICATION_REQUIRED))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static void setRequestNoCache()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setRequestNoCache()")

#  ifndef U_CACHE_REQUEST_DISABLE
      U_ClientImage_request |= NO_CACHE;

      U_INTERNAL_DUMP("U_ClientImage_request = %u %B", U_ClientImage_request, U_ClientImage_request)
#  endif
      }

   static void setRequestToCache()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setRequestToCache()")

      U_INTERNAL_DUMP("U_ClientImage_pipeline = %b size_request = %u U_http_uri_offset = %u U_http_info.startHeader = %u",
                       U_ClientImage_pipeline,     size_request,     U_http_uri_offset,     U_http_info.startHeader)

#  if !defined(U_CACHE_REQUEST_DISABLE) || defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST)
      U_INTERNAL_ASSERT_MAJOR(size_request, 0)
      U_INTERNAL_ASSERT_RANGE(1,U_http_uri_offset,254)
      U_INTERNAL_ASSERT_MAJOR(U_http_info.startHeader, 2)

      U_http_info.startHeader -= U_http_uri_offset + U_CONSTANT_SIZE(" HTTP/1.1\r\n");

      U_INTERNAL_ASSERT(U_http_info.startHeader <= sizeof(cbuffer))

      U_MEMCPY(cbuffer, request->c_pointer(U_http_uri_offset), U_http_info.startHeader);

      U_INTERNAL_DUMP("request(%u) = %V", request->size(), request->rep)
      U_INTERNAL_DUMP("cbuffer(%u) = %.*S", U_http_info.startHeader, U_http_info.startHeader, cbuffer)
#  endif
      }

   static uint32_t checkRequestToCache();

   // pending operation processing

   enum PendingOperationType {
      _POLL    = 0x01,
      _ACCEPT  = 0x02,
      _READ    = 0x04,
      _WRITE   = 0x08,
      _WRITEV  = 0x10,
      _CLOSE   = 0x20,
      _CANCEL  = 0x40,
      _UPDATE  = 0x80,
      _CONNECT = _READ | _ACCEPT
   };

   static const char* getPendingOperationDescription(int op)
      {
      return (op == 0        ? "NONE"     :
              op == _POLL    ? "_POLL"    :
              op == _ACCEPT  ? "_ACCEPT"  :
              op == _READ    ? "_READ"    :
              op == _WRITE   ? "_WRITE"   :
              op == _WRITEV  ? "_WRITEV"  :
              op == _CLOSE   ? "_CLOSE"   :
              op == _CANCEL  ? "_CANCEL"  :
              op == _UPDATE  ? "_UPDATE"  :
              op == _CONNECT ? "_CONNECT" : "???");
      }

   const char* getPendingOperationDescription() { return getPendingOperationDescription( U_ClientImage_op_pending(this)); }

   void resetPendingOperation()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::resetPendingOperation()")

      U_ClientImage_op_pending(this) = 0;
      }

   bool isFlagPendingOperation()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isFlagPendingOperation()")

      if (U_ClientImage_op_pending(this) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isPendingOperationRead(char op)
      {
      U_TRACE(0, "UClientImage_Base::isPendingOperationRead(%u %B)", op, op)

      if ((op & _READ) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isPendingOperationWrite(char op)
      {
      U_TRACE(0, "UClientImage_Base::isPendingOperationWrite(%u %B)", op, op)

      if ((op & _WRITE) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isPendingOperationClose(char op)
      {
      U_TRACE(0, "UClientImage_Base::isPendingOperationClose(%u %B)", op, op)

      if ((op & _CLOSE) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   bool isPendingOperationRead()  { return isPendingOperationRead( U_ClientImage_op_pending(this)); }
   bool isPendingOperationWrite() { return isPendingOperationWrite(U_ClientImage_op_pending(this)); }
   bool isPendingOperationClose() { return isPendingOperationClose(U_ClientImage_op_pending(this)); }

   void setPendingOperationRead()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setPendingOperationRead()")

      U_DUMP("isPendingOperationRead() = %b", isPendingOperationRead())

      U_ClientImage_op_pending(this) |= _READ;
      }

   void resetPendingOperationRead()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::resetPendingOperationRead()")

      U_DUMP("isPendingOperationRead() = %b", isPendingOperationRead())

      U_ClientImage_op_pending(this) &= ~_READ;
      }

   void setPendingOperationWrite()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setPendingOperationWrite()")

      U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_write_pending = %u", U_ClientImage_pipeline, U_ClientImage_write_pending(this))

      U_ClientImage_write_pending(this) += 1;

      U_ClientImage_op_pending(this) |= _WRITE;
      }

   bool resetPendingOperationWrite()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::resetPendingOperationWrite()")

      U_INTERNAL_DUMP("U_ClientImage_write_pending = %u", U_ClientImage_write_pending(this))

      if ((U_ClientImage_write_pending(this) -= 1) == 0)
         {
         U_ClientImage_op_pending(this) &= ~_WRITE;

         if (isPendingOperationClose()) U_RETURN(true);
         }

      U_RETURN(false);
      }

   void setPendingOperationClose()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setPendingOperationClose()")

      U_ASSERT_EQUALS(isPendingOperationClose(), false)

      U_ClientImage_op_pending(this) |= _CLOSE;
      }

   void resetPendingOperationClose()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::resetPendingOperationClose()")

      U_DUMP("isPendingOperationClose() = %b", isPendingOperationClose())

      U_ClientImage_op_pending(this) &= ~_CLOSE;
      }

   // flag status processing

   enum FlagStatusType {
   // NONE       = 0x00,
      _WEBSOCKET = 0x01,
      _HTTP2     = 0x02,
      _HTTP3     = 0x04,
   // _CLOSE     = 0x08,
      _IDLE      = 0x10,
      _DELETE    = 0x20
   };

   void resetFlagStatus()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::resetFlagStatus()")

      U_ClientImage_op_pending(this) = 0;
      }

   bool isFlagStatus()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isFlagStatus()")

      if (U_ClientImage_op_pending(this) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isFlagStatusWebSocket(char op)
      {
      U_TRACE(0, "UClientImage_Base::isFlagStatusWebSocket(%u %B)", op, op)

      if ((op & _WEBSOCKET) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   bool isFlagStatusWebSocket() { return isFlagStatusWebSocket(U_ClientImage_status(this)); }

   void setFlagStatusWebSocket()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setFlagStatusWebSocket()")

      U_ASSERT_EQUALS(isFlagStatusWebSocket(), false)

      U_ClientImage_status(this) |= _WEBSOCKET;
      }

   void resetFlagStatusWebSocket()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::resetFlagStatusWebSocket()")

      U_ASSERT(isFlagStatusWebSocket())

      U_ClientImage_op_pending(this) &= ~_WEBSOCKET;
      }

   static bool isFlagStatusHttp2(char op)
      {
      U_TRACE(0, "UClientImage_Base::isFlagStatusHttp2(%u %B)", op, op)

      if ((op & _HTTP2) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   bool isFlagStatusHttp2() { return isFlagStatusHttp2(U_ClientImage_status(this)); }

   void setFlagStatusHttp2()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setFlagStatusHttp2()")

      U_ASSERT_EQUALS(isFlagStatusHttp2(), false)

      U_ClientImage_status(this) |= _HTTP2;
      }

   void resetFlagStatusHttp2()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::resetFlagStatusHttp2()")

      U_ASSERT(isFlagStatusHttp2())

      U_ClientImage_op_pending(this) &= ~_HTTP2;
      }

   static bool isFlagStatusHttp3(char op)
      {
      U_TRACE(0, "UClientImage_Base::isFlagStatusHttp3(%u %B)", op, op)

      if ((op & _HTTP3) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   bool isFlagStatusHttp3() { return isFlagStatusHttp3(U_ClientImage_status(this)); }

   void setFlagStatusHttp3()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setFlagStatusHttp3()")

      U_ASSERT_EQUALS(isFlagStatusHttp3(), false)

      U_ClientImage_status(this) |= _HTTP3;
      }

   void resetFlagStatusHttp3()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::resetFlagStatusHttp3()")

      U_ASSERT(isFlagStatusHttp3())

      U_ClientImage_op_pending(this) &= ~_HTTP3;
      }

   static bool isFlagStatusClose(char op)
      {
      U_TRACE(0, "UClientImage_Base::isFlagStatusClose(%u %B)", op, op)

      if ((op & _CLOSE) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   bool isFlagStatusClose() { return isFlagStatusClose(U_ClientImage_status(this)); }

   void setFlagStatusClose()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setFlagStatusClose()")

      U_ASSERT_EQUALS(isFlagStatusClose(), false)

      U_ClientImage_status(this) |= _CLOSE;
      }

   void resetFlagStatusClose()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::resetFlagStatusClose()")

      U_ASSERT(isFlagStatusClose())

      U_ClientImage_op_pending(this) &= ~_CLOSE;
      }

   static bool isFlagStatusIdle(char op)
      {
      U_TRACE(0, "UClientImage_Base::isFlagStatusIdle(%u %B)", op, op)

      if ((op & _IDLE) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   bool isFlagStatusIdle() { return isFlagStatusIdle(U_ClientImage_status(this)); }

   void setFlagStatusIdle()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setFlagStatusIdle()")

      U_ASSERT_EQUALS(isFlagStatusIdle(), false)

      U_ClientImage_status(this) |= _IDLE;
      }

   void resetFlagStatusIdle()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::resetFlagStatusIdle()")

      U_ASSERT(isFlagStatusIdle())

      U_ClientImage_op_pending(this) &= ~_IDLE;
      }

   static bool isFlagStatusDelete(char op)
      {
      U_TRACE(0, "UClientImage_Base::isFlagStatusDelete(%u %B)", op, op)

      if ((op & _DELETE) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   bool isFlagStatusDelete() { return isFlagStatusDelete(U_ClientImage_status(this)); }

   void setFlagStatusDelete()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setFlagStatusDelete()")

      U_ASSERT_EQUALS(isFlagStatusDelete(), false)

      U_ClientImage_status(this) |= _DELETE;
      }

   void resetFlagStatusDelete()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::resetFlagStatusDelete()")

      U_ASSERT(isFlagStatusDelete())

      U_ClientImage_op_pending(this) &= ~_DELETE;
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

   // NB: these are public for plugin access...

public:
   UString* logbuf; // it is needed for U_SRV_LOG_WITH_ADDR...

   static UString* body;
   static UString* rbuffer;
   static UString* wbuffer;
   static UString* request;
   static bool bIPv6, bsendGzipBomb, bnoheader;

   static char cbuffer[128];
   static UString* request_uri;
   static UString* environment;
   static struct iovec iov_vec[4];
   static uint32_t rstart, size_request;

   static bPF callerHandlerCache;
   static bPFpc callerIsValidMethod;
   static iPF callerHandlerRead;
   static vPF callerHandlerRequest;
   static bPFpcu callerIsValidRequest, callerIsValidRequestExt;

   // NB: these are for ULib Servlet Page (USP) - USP_PRINTF...

   static UString* usp_value;
   static UString* usp_buffer;
   static UString* usp_encoded;

   bool isOpen()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isOpen()")

      if (socket->isOpen()) U_RETURN(true);

      U_RETURN(false);
      }

   bool writeResponse();
   void writeResponseCompact();

   uint32_t writev(struct iovec* iov, int iovcnt, uint32_t count, int timeoutMS);

protected:
   USocket* socket;
#ifdef U_THROTTLING_SUPPORT
   UString uri;
   uint64_t bytes_sent;
   uint32_t min_limit, max_limit, started_at;
#endif
   UString* data_pending;
   off_t offset, count;
   int sfd;
   uucflag flag;
   long last_event;
   // HTTP3
   void* conn;
   void* http3;

   bool isCallHandlerFailed();

#ifndef U_LOG_DISABLE
   static int log_request_partial;

   void logRequest();
#endif

   void   set();
   void reset()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::reset()")

      UEventFd::op_mask = EPOLLIN | EPOLLRDHUP | EPOLLET;

      offset =
      count  = 0;
      sfd    = -1;
      }

   int manageRead()
      {
      U_TRACE_NO_PARAM(0, "UClientImage::manageRead()")

      if ((prepareForRead(), genericRead()) == false &&
          U_ClientImage_state != U_PLUGIN_HANDLER_AGAIN) // NOT BLOCKING...
         {
         U_INTERNAL_ASSERT_EQUALS(U_ClientImage_state, U_PLUGIN_HANDLER_ERROR)

         U_RETURN(U_NOTIFIER_DELETE);
         }

      U_RETURN(U_NOTIFIER_OK);
      }

   static void resetBuffer()
      {
      U_TRACE_NO_PARAM(0, "UClientImage::resetBuffer()")

      body->clear();

      U_INTERNAL_DUMP("wbuffer(%u) = %V", wbuffer->size(), wbuffer->rep)

      wbuffer->setBuffer(U_CAPACITY); // NB: this string can be referenced more than one (often if U_SUBSTR_INC_REF is defined)...
      }

   static void resetReadBuffer(UStringRep* rep)
      {
      U_TRACE(0, "UClientImage::resetReadBuffer(%p)", rep)

         body->clear();
      request->clear();
      rbuffer->clear();

      rbuffer->_assign(rep);
      }

   int handlerResponse()
      {
      U_TRACE_NO_PARAM(0, "UClientImage::handlerResponse()")

      bnoheader = true;

      if (writeResponse()) U_RETURN(U_NOTIFIER_OK);

      U_RETURN(U_NOTIFIER_DELETE);
      }

   static uint32_t getCountToRead()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::getCountToRead()")

      if (size_request == 0) U_RETURN(U_SINGLE_READ);

      uint32_t sz = rbuffer->size();

      if (size_request > sz)
         {
         sz = size_request - sz;

         U_RETURN(sz);
         }

      U_RETURN(U_SINGLE_READ);
      }

   bool logCertificate(); // append on log the peer certicate of client ("issuer","serial")
   bool askForClientCertificate();

   static UTimeVal* chronometer;
   static uint32_t nrequest, resto;
   static long time_between_request, time_run;

#if defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST) || (defined(DEBUG) && !defined(U_LOG_DISABLE))
   static void startRequest();
#endif

#if defined(DEBUG) || (defined(U_SERVER_CAPTIVE_PORTAL) && !defined(ENABLE_THREAD))
   static void saveRequestResponse();
#endif

   static void endRequest();
   static void resetReadBuffer();
   static void resetWriteBuffer();
   static void manageReadBufferResize(uint32_t n);
   static void setSendfile(int fd, off_t start, off_t count);

   void prepareForSendfile();

#if defined(U_THROTTLING_SUPPORT) || defined(U_CLIENT_RESPONSE_PARTIAL_WRITE_SUPPORT)
   void setPendingSendfile()
      {
      U_TRACE_NO_PARAM(0, "UClientImage::setPendingSendfile()")

      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD)

      prepareForSendfile();

      setFlagStatusClose();
      }
#endif

#ifndef U_CACHE_REQUEST_DISABLE
   static bool isRequestCacheable() __pure;
#endif

            UClientImage_Base();
   virtual ~UClientImage_Base();

#ifdef DEBUG
   bool check_memory();
#endif

private:
   static inline bool handlerCache() U_NO_EXPORT;
   static inline bool isValidMethod(    const char* ptr) U_NO_EXPORT;
   static inline bool isValidRequest(   const char* ptr, uint32_t sz) U_NO_EXPORT;
   static inline bool isValidRequestExt(const char* ptr, uint32_t sz) U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UClientImage_Base)

                      friend class UHTTP;
                      friend class UHTTP2;
                      friend class UHTTP3;
                      friend class UEventDB;
                      friend class USocketExt;
                      friend class USSIPlugIn;
                      friend class UHttpPlugIn;
                      friend class UNoCatPlugIn;
                      friend class UServer_Base;
                      friend class UStreamPlugIn;
                      friend class UBandWidthThrottling;

   template <class T> friend class UServer;
   template <class T> friend class UHashMap;
   template <class T> friend void u_delete_vector(      T* _vec, uint32_t offset, uint32_t n);
#ifdef DEBUG
   template <class T> friend bool u_check_memory_vector(T* _vec,                  uint32_t n);
#endif
};

template <class Socket> class U_EXPORT UClientImage : public UClientImage_Base {
public:

   UClientImage() : UClientImage_Base()
      {
      U_TRACE_CTOR(0, UClientImage<Socket>, "")

      U_NEW(Socket, socket, Socket(UClientImage_Base::bIPv6))

      set();
      }

   virtual ~UClientImage()
      {
      U_TRACE_DTOR(0, UClientImage<Socket>)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return UClientImage_Base::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UClientImage)
};

#ifdef USERVER_UDP
template <> class U_EXPORT UClientImage<UUDPSocket> : public UClientImage_Base {
public:

   UClientImage() : UClientImage_Base()
      {
      U_TRACE_CTOR(0, UClientImage<UUDPSocket>, "")

      U_NEW(UUDPSocket, socket, UUDPSocket(UClientImage_Base::bIPv6))

      set();
      }

   virtual ~UClientImage()
      {
      U_TRACE_DTOR(0, UClientImage<UUDPSocket>)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return UClientImage_Base::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UClientImage<UUDPSocket>)
};
#endif

#ifdef USE_LIBSSL
template <> class U_EXPORT UClientImage<USSLSocket> : public UClientImage_Base {
public:

   UClientImage() : UClientImage_Base()
      {
      U_TRACE_CTOR(0, UClientImage<USSLSocket>, "")

      U_NEW(USSLSocket, socket, USSLSocket(UClientImage_Base::bIPv6, USSLSocket::sctx, true))

      set();
      }

   virtual ~UClientImage()
      {
      U_TRACE_DTOR(0, UClientImage<USSLSocket>)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return UClientImage_Base::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UClientImage<USSLSocket>)
};
#endif

#endif
