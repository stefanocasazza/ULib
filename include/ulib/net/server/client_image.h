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
#include <ulib/utility/socket_ext.h>
#include <ulib/net/server/server_plugin.h>

#ifdef USE_LIBSSL
#  include <ulib/ssl/certificate.h>
#  include <ulib/ssl/net/sslsocket.h>
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

#define U_ClientImage_request_is_cached UClientImage_Base::cbuffer[0]

#define U_ClientImage_http(obj)                    (obj)->UClientImage_Base::flag.c[0]
#define U_ClientImage_idle(obj)                    (obj)->UClientImage_Base::flag.c[1]
#define U_ClientImage_pclose(obj)                  (obj)->UClientImage_Base::flag.c[2]
#define U_ClientImage_request_is_from_userver(obj) (obj)->UClientImage_Base::flag.c[3]

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
      }

   static void setNoHeaderForResponse()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setNoHeaderForResponse()")

      iov_vec[0].iov_len = 0;
      }

   static bool isNoHeaderForResponse()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isNoHeaderForResponse()")

      if (UNLIKELY(iov_vec[0].iov_len == 0)) U_RETURN(true);

      U_RETURN(false);
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

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)

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

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B U_http_info.nResponseCode = %d", U_ClientImage_request,
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

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)
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
   static bool bIPv6, bsendGzipBomb;

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

   static UString* _value;
   static UString* _buffer;
   static UString* _encoded;

   bool isOpen()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isOpen()")

      if (socket->isOpen()) U_RETURN(true);

      U_RETURN(false);
      }

   bool writeResponse();
   void writeResponseCompact()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::writeResponseCompact()")

      uint32_t sz = wbuffer->size();

      U_ASSERT(body->empty())
      U_INTERNAL_ASSERT_MAJOR(sz, 0)

      iov_vec[2].iov_len  = sz;
      iov_vec[2].iov_base = (caddr_t)wbuffer->data();

      U_INTERNAL_DUMP("iov_vec[0].iov_len = %u iov_vec[1].iov_len = %u", iov_vec[0].iov_len, iov_vec[1].iov_len)

      U_INTERNAL_ASSERT_EQUALS(iov_vec[0].iov_len, 17)
      U_INTERNAL_ASSERT_EQUALS(iov_vec[1].iov_len, 51)

#  ifndef U_PIPELINE_HOMOGENEOUS_DISABLE
      if (nrequest)
         {
         struct iovec iov[256];

         U_INTERNAL_ASSERT_MAJOR(nrequest, 1)

         char* ptr = (char*)iov;

         U_MEMCPY(ptr, iov_vec, sizeof(struct iovec) * 3);

         for (uint32_t i = 1; i < nrequest; ++i)
            {
                     ptr +=        sizeof(struct iovec) * 3;
            U_MEMCPY(ptr, iov_vec, sizeof(struct iovec) * 3);
            }

         (void) USocketExt::writev(socket, iov, 3*nrequest, (17+51+sz)*nrequest, 0);
         }
      else
#  endif
      {
      U_INTERNAL_ASSERT_EQUALS(nrequest, 0)

      (void) USocketExt::writev(socket, iov_vec, 3, 17+51+sz, 0);
      }
      }

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

      U_DUMP("wbuffer(%u) = %V isConstant() = %b", wbuffer->size(), wbuffer->rep, wbuffer->isConstant())

      wbuffer->setBuffer(U_CAPACITY); // NB: this string can be referenced more than one (often if U_SUBSTR_INC_REF is defined)...
      }

   int handlerResponse()
      {
      U_TRACE_NO_PARAM(0, "UClientImage::handlerResponse()")

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

   static int idx, iovcnt;
   static UTimeVal* chronometer;
   static uint32_t ncount, nrequest, resto;
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

   bool isRequestFromUServer()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestFromUServer()")

      if (U_ClientImage_request_is_from_userver(this) != false) U_RETURN(true);

      U_RETURN(false);
      }

   void setRequestFromUServer()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setRequestFromUServer()")

      U_ClientImage_request_is_from_userver(this) = true;
      }

   void resetRequestFromUServer()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::resetRequestFromUServer()")

      U_ClientImage_request_is_from_userver(this) = true;
      }

   void prepareForSendfile();

#if defined(U_THROTTLING_SUPPORT) || defined(U_CLIENT_RESPONSE_PARTIAL_WRITE_SUPPORT)
   void setPendingSendfile()
      {
      U_TRACE_NO_PARAM(0, "UClientImage::setPendingSendfile()")

      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD)

      count = ncount;

      prepareForSendfile();

      U_ClientImage_pclose(this) |= U_CLOSE;
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
                      friend class UEventDB;
                      friend class USocketExt;
                      friend class USSIPlugIn;
                      friend class UHttpPlugIn;
                      friend class UNoCatPlugIn;
                      friend class UServer_Base;
                      friend class UStreamPlugIn;
                      friend class UBandWidthThrottling;

   template <class T> friend class UServer;
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
