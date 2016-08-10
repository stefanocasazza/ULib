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
class USSIPlugIn;
class USocketExt;
class UHttpPlugIn;
class UNoCatPlugIn;
class UServer_Base;
class UStreamPlugIn;
class UBandWidthThrottling;

template <class T> class UServer;

#define U_ClientImage_idle(obj)   (obj)->UClientImage_Base::flag.c[0]
#define U_ClientImage_pclose(obj) (obj)->UClientImage_Base::flag.c[1]

#define U_ClientImage_request_is_cached UClientImage_Base::cbuffer[0]

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

      if (count > 0) U_RETURN(true);

      U_RETURN(false);
      }

   // define method VIRTUAL of class UEventFd

   virtual int  handlerRead() U_DECL_OVERRIDE;
   virtual int  handlerWrite() U_DECL_FINAL;
   virtual int  handlerTimeout() U_DECL_FINAL;
   virtual void handlerDelete() U_DECL_FINAL;

   static void setNoHeaderForResponse()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setNoHeaderForResponse()")

      iov_vec[1].iov_len = 0;
      }

   static void setHeaderForResponse(uint32_t len)
      {
      U_TRACE(0, "UClientImage_Base::setHeaderForResponse(%u)", len)

      iov_vec[1].iov_len = len;
      }

   static void init();
   static void clear();

   static void          close();
   static void abortive_close();

   static const char* getRequestUri(uint32_t& len);

   static bool isAllowed(UVector<UIPAllow*>& vallow_IP) __pure; // Check whether the ip address client ought to be allowed

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
   // NOT_FOUND            = 0x0000,
      FORBIDDEN            = 0x0001,
      NO_CACHE             = 0x0002,
      IN_FILE_CACHE        = 0x0004,
      ALREADY_PROCESSED    = 0x0008,
      FILE_CACHE_PROCESSED = 0x0010
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

   static void setRequestProcessed()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setRequestProcessed()")

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)

      if ((U_ClientImage_request & ALREADY_PROCESSED) == 0) U_ClientImage_request |= ALREADY_PROCESSED;
      }

   static void setRequestNeedProcessing()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setRequestNeedProcessing()")

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)

      U_ClientImage_request &= ~ALREADY_PROCESSED;

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)
      }

   static bool isRequestNeedProcessing()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestNeedProcessing()")

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)

      if ((U_ClientImage_request & ALREADY_PROCESSED) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isRequestAlreadyProcessed()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestAlreadyProcessed()")

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)

      if ((U_ClientImage_request & ALREADY_PROCESSED) != 0) U_RETURN(true);

      U_RETURN(false);
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

   static void setRequestForbidden()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setRequestForbidden()")

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)

      U_ClientImage_request |= FORBIDDEN | ALREADY_PROCESSED;
      }

   static bool isRequestForbidden()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestForbidden()")

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)

      if ((U_ClientImage_request & FORBIDDEN) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static void setRequestInFileCache()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setRequestInFileCache()")

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)

      U_ClientImage_request |= IN_FILE_CACHE | ALREADY_PROCESSED;
      }

   static bool isRequestInFileCache()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestInFileCache()")

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)

      if ((U_ClientImage_request & IN_FILE_CACHE) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static void setRequestFileCacheProcessed()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::setRequestFileCacheProcessed()")

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)

      U_ClientImage_request |= FILE_CACHE_PROCESSED;
      }

   static bool isRequestFileCacheProcessed()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::isRequestFileCacheProcessed()")

      U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)

      if ((U_ClientImage_request & FILE_CACHE_PROCESSED) != 0) U_RETURN(true);

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

#  if !defined(U_CACHE_REQUEST_DISABLE) || defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST) 
      U_INTERNAL_ASSERT_MAJOR(U_http_info.startHeader, 2)
      U_INTERNAL_ASSERT_MAJOR(UClientImage_Base::size_request, 0)
      U_INTERNAL_ASSERT_RANGE(1,UClientImage_Base::uri_offset,64)

      U_http_info.startHeader -= UClientImage_Base::uri_offset + U_CONSTANT_SIZE(" HTTP/1.1\r\n");

      U_INTERNAL_ASSERT(U_http_info.startHeader <= sizeof(cbuffer))

      U_MEMCPY(UClientImage_Base::cbuffer, UClientImage_Base::request->c_pointer(UClientImage_Base::uri_offset), U_http_info.startHeader);

      U_INTERNAL_DUMP("request(%u) = %V", UClientImage_Base::request->size(), UClientImage_Base::request->rep)
      U_INTERNAL_DUMP("UClientImage_Base::cbuffer(%u) = %.*S", U_http_info.startHeader, U_http_info.startHeader, UClientImage_Base::cbuffer)
#  endif
      }

   static uint32_t checkRequestToCache()
      {
      U_TRACE_NO_PARAM(0, "UClientImage_Base::checkRequestToCache()")

      U_INTERNAL_DUMP("U_ClientImage_request_is_cached = %b", U_ClientImage_request_is_cached)

#  if !defined(U_CACHE_REQUEST_DISABLE) || defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST) 
      U_INTERNAL_ASSERT(U_ClientImage_request_is_cached)

      uint32_t    sz  = request->size();
      const char* ptr = request->data();

      U_INTERNAL_DUMP("cbuffer(%u) = %.*S", U_http_info.startHeader, U_http_info.startHeader, cbuffer)
      U_INTERNAL_DUMP("request(%u) = %.*S", sz, sz, ptr)

      U_INTERNAL_DUMP("U_ClientImage_pipeline = %b size_request = %u uri_offset = %u", U_ClientImage_pipeline, size_request, uri_offset)

      U_INTERNAL_ASSERT_MAJOR(size_request, 0)
      U_INTERNAL_ASSERT_RANGE(1,uri_offset,64)
      U_INTERNAL_ASSERT_MAJOR(U_http_info.uri_len, 0)
      U_INTERNAL_ASSERT_MAJOR(U_http_info.startHeader, 0)
      U_INTERNAL_ASSERT_EQUALS(U_ClientImage_data_missing, false)

      if (u__isblank((ptr+uri_offset)[U_http_info.startHeader]) &&
          memcmp(ptr+uri_offset, cbuffer, U_http_info.startHeader) == 0)
         {
         if (size_request > sz &&
             (callerIsValidMethod( ptr)     == false ||
              callerIsValidRequest(ptr, sz) == false))
            {
            U_RETURN(1); // partial valid (not complete)
            }

         if (callerHandlerCache()) U_RETURN(2);
         }
#  endif

      U_RETURN(0);
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

   // NB: these are public for plugin access...

public:
   UString* logbuf; // it is needed for U_SRV_LOG_WITH_ADDR...

   static bool bIPv6; 
   static UString* body;
   static UString* rbuffer;
   static UString* wbuffer;
   static UString* request;

   static char cbuffer[128];
   static UString* request_uri;
   static UString* environment;
   static bPF callerHandlerCache;
   static struct iovec iov_sav[4];
   static struct iovec iov_vec[4];
   static bPFpc callerIsValidMethod;
   static vPF callerHandlerEndRequest;
   static uint32_t rstart, size_request;
   static bPFpcu callerIsValidRequest, callerIsValidRequestExt;
   static iPF callerHandlerRead, callerHandlerRequest, callerHandlerDataPending;

   // NB: these are for ULib Servlet Page (USP) - USP_PRINTF...

   static UString* _value;
   static UString* _buffer;
   static UString* _encoded;

protected:
   USocket* socket;
#ifdef U_THROTTLING_SUPPORT
   UString uri;
   uint64_t bytes_sent;
   uint32_t min_limit, max_limit, started_at;
#endif
   UString* data_pending;
   uint32_t start, count;
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

      start =
      count = 0;
      sfd   = -1;
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

   int handlerResponse();
   void prepareForSendfile();

   void setPendingSendfile()
      {
      U_TRACE_NO_PARAM(0, "UClientImage::setPendingSendfile()")

      count = ncount;

      prepareForSendfile();

      U_ClientImage_pclose(this) |= U_CLOSE;
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

   bool writeResponse();
   bool logCertificate(); // append on log the peer certicate of client ("issuer","serial")
   bool askForClientCertificate();

   static struct iovec* piov;
   static int csfd, idx, iovcnt;
   static UTimeVal* chronometer;
   static long time_between_request, time_run;
   static uint32_t ncount, nrequest, resto, uri_offset;

   static void   endRequest();
   static bool startRequest();

   static void resetReadBuffer();
   static void resetWriteBuffer();
   static void saveRequestResponse();
   static void manageReadBufferResize(uint32_t n);
   static void setSendfile(int _sfd, uint32_t _start, uint32_t _count);

   static void do_nothing() {}

   // NB: we have default to true to manage pipeline for protocol as RPC...

   static bool handlerCache()                                  { return true; }
   static bool isValidMethod(    const char* ptr)              { return true; }
   static bool isValidRequest(   const char* ptr, uint32_t sz) { return true; }
   static bool isValidRequestExt(const char* ptr, uint32_t sz) { return true; }

#ifndef U_CACHE_REQUEST_DISABLE
   static bool isRequestCacheable() __pure;
#endif

            UClientImage_Base();
   virtual ~UClientImage_Base();

#ifdef DEBUG
   bool check_memory();
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UClientImage_Base)

                      friend class UHTTP;
                      friend class UHTTP2;
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
      U_TRACE_REGISTER_OBJECT(0, UClientImage<Socket>, "", 0)

      U_NEW(Socket, socket, Socket(UClientImage_Base::bIPv6));

      set();
      }

   virtual ~UClientImage()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UClientImage<Socket>)
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
      U_TRACE_REGISTER_OBJECT(0, UClientImage<USSLSocket>, "", 0)

      U_NEW(USSLSocket, socket, USSLSocket(UClientImage_Base::bIPv6, USSLSocket::sctx, true));

      set();
      }

   virtual ~UClientImage()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UClientImage<USSLSocket>)
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
