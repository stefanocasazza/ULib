// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    curl.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_CURL_H
#define ULIB_CURL_H 1

#include <ulib/container/vector.h>

extern "C" {
#include <curl/curl.h>
}

/**
 * @class UCURL
 *
 * @brief This class is a wrapper around the cURL library
 */

class U_EXPORT UCURL {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

    UCURL();
   ~UCURL();

   static const char* apple_cert;    // the path to the certificate
   static const char* http2_server;  // the Apple server url
   static const char* app_bundle_id; // the app bundle id

   /**
    * @param http2_server  the Apple server url
    * @param apple_cert    the path to the certificate
    * @param app_bundle_id the app bundle id
    */

   static void initHTTP2Push(const char* _http2_server = 0, const char* _apple_cert = 0, const char* _app_bundle_id = 0)
      {
      U_TRACE(0, "UCURL::initHTTP2Push(%S,%S,%S)", _http2_server, _apple_cert, _app_bundle_id)

      if (_http2_server)   http2_server = _http2_server;  
      if (_apple_cert)       apple_cert = _apple_cert;  
      if (_app_bundle_id) app_bundle_id = _app_bundle_id;  
      }

   /**
    * @param token   the token of the device
    * @param message the payload to send (JSON)
    */

   static bool sendHTTP2Push(const UString& token, const UString& message);

   UString& getResponse() { return response; }

   // EASY

   const char* error();

   void setOption(CURLoption option, long parameter)
      {
      U_TRACE(1, "UCURL::setOption(%p,%p)", option, parameter)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(easyHandle)

      result = U_SYSCALL(curl_easy_setopt, "%p,%p,%p", easyHandle, option, parameter);

#  ifdef DEBUG
      if (result) U_DUMP("status = %.*S", 512, error())
#  endif

      U_INTERNAL_ASSERT_EQUALS(result, CURLE_OK)
      }

   void setHTTP2()
      {
      U_TRACE_NO_PARAM(0, "UCURL::setHTTP2()")

      setOption(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
      }

   void setInterface(const char* interfaceIP)
      {
      U_TRACE(0, "UCURL::setInterface(%S)", interfaceIP)

      setOption(CURLOPT_INTERFACE, (long)interfaceIP);
      }

   void setURL(const char* url)
      {
      U_TRACE(0, "UCURL::setURL(%S)", url)

      U_INTERNAL_ASSERT(u_isURL(url, u__strlen(url, __PRETTY_FUNCTION__)))

      setOption(CURLOPT_URL, (long)url);
      }

   void setCertificate(const char* cert) // the string should be the file name of your client certificate
      {
      U_TRACE(0, "UCURL::setCertificate(%S)", cert)

      setOption(CURLOPT_SSLCERT, (long)cert);
      }

   void setPort(int port)
      {
      U_TRACE(0, "UCURL::setPort(%d)", port)

      setOption(CURLOPT_PORT, (long)port);
      }

   void setTimeout(long timeout)
      {
      U_TRACE(0, "UCURL::setTimeout(%ld)", timeout)

      U_INTERNAL_ASSERT_MINOR(timeout, 1000) // NB: must be in seconds...

      setOption(CURLOPT_CONNECTTIMEOUT, timeout);
      }

   void setMaxTime(long maxtime)
      {
      U_TRACE(0, "UCURL::setMaxTime(%ld)", maxtime)

      U_INTERNAL_ASSERT_MINOR(maxtime, 1000) // NB: must be in seconds...

      setOption(CURLOPT_TIMEOUT, maxtime);
      }

   void setInsecure()
      {
      U_TRACE_NO_PARAM(0, "UCURL::setInsecure()")

      // new stuff needed for libcurl 7.10

      setOption(CURLOPT_SSL_VERIFYPEER, 0);
      setOption(CURLOPT_SSL_VERIFYHOST, 0);
      }

   void setNoBody()
      {
      U_TRACE_NO_PARAM(0, "UCURL::setNoBody()")

      setOption(CURLOPT_NOBODY, 1L);
      }

   void setGetMode()
      {
      U_TRACE_NO_PARAM(0, "UCURL::setGetMode()")

      setOption(CURLOPT_HTTPGET, 1L);
      }

   void setDNSCachingTime(int time)
      {
      U_TRACE(0, "UCURL::setDNSCachingTime(%d)", time)

      setOption(CURLOPT_DNS_CACHE_TIMEOUT, (long)time);
      }

   void setProgressFunction(curl_progress_callback func, void* data)
      {
      U_TRACE(0, "UCURL::setProgressFunction(%p,%p)", func, data)

      if (func == 0) setOption(CURLOPT_NOPROGRESS, 1L);
      else
         {
         setOption(CURLOPT_PROGRESSFUNCTION, (long)func);
         setOption(CURLOPT_PROGRESSDATA, (long)data);
         setOption(CURLOPT_NOPROGRESS, 0L);
         }
      }

   void setRequestFileTime(bool request)
      {
      U_TRACE(0, "UCURL::setRequestFileTime(%b)", request)

      setOption(CURLOPT_FILETIME, request ? 1L : 0L);
      }

   void setTimeCondition(bool modified_since, time_t t)
      {
      U_TRACE(0, "UCURL::setTimeCondition(%b,%ld)", modified_since, t)

      if (modified_since == false) setOption(CURLOPT_TIMECONDITION, CURL_TIMECOND_NONE);
      else
         {
         setOption(CURLOPT_TIMECONDITION, CURL_TIMECOND_IFMODSINCE);
         setOption(CURLOPT_TIMEVALUE, (long)t);
         }
      }

   // HEADER

   void addHeader(const char* value)
      {
      U_TRACE(0, "UCURL::addHeader(%S)", value)

      headerlist = (struct curl_slist*) U_SYSCALL(curl_slist_append, "%p,%S", headerlist, value);
      }

   void addHeader(UVector<UString>& vec)
      {
      U_TRACE(0, "UCURL::addHeader(%p)", &vec)

      for (uint32_t i = 0, n = vec.size(); i < n; ++i) addHeader(vec[i].c_str());
      }

   void setUserAgent(const char* userAgent)
      {
      U_TRACE(0, "UCURL::setUserAgent(%S)", userAgent)

      setOption(CURLOPT_USERAGENT, (long)userAgent);
      }

   void setHeaderList()
      {
      U_TRACE_NO_PARAM(0, "UCURL::setHeaderList()")

      U_INTERNAL_ASSERT_POINTER(headerlist)

      setOption(CURLOPT_HTTPHEADER, (long)headerlist);
      }

   void setHeader() // pass headers to the data stream
      {
      U_TRACE_NO_PARAM(0, "UCURL::setHeader()")

      setOption(CURLOPT_HEADER, 1L);
      }

   // POST - FORM

   void setHTTPPostMode()
      {
      U_TRACE_NO_PARAM(0, "UCURL::setHTTPPostMode()")

      setOption(CURLOPT_HTTPPOST, (long)formPost);
      }

   void setPostMode(const char* postData, uint32_t size)
      {
      U_TRACE(0, "UCURL::setPostMode(%.*S,%u)", size, postData, size)

      U_INTERNAL_ASSERT_POINTER(postData)

      setOption(CURLOPT_POSTFIELDSIZE_LARGE, (long)size); // size of the data to copy from the buffer and send in the request
      setOption(CURLOPT_POSTFIELDS, (long)postData);      // send data from the local stack
      setOption(CURLOPT_POST, 1L);
      }

   void setPostMode(const UString& postData) { setPostMode(U_STRING_TO_PARAM(postData)); }

   void addFormData(const char* key, const char* value)
      {
      U_TRACE(0, "UCURL::addFormData(%S,%S)", key, value)

      CURLFORMcode res = (CURLFORMcode)
                           U_SYSCALL(curl_formadd, "%p,%p,%d,%S,%d,%S,%d,%d,%d",
                              &formPost, &formLast,
                              CURLFORM_COPYNAME, key,
                              CURLFORM_COPYCONTENTS, value,
                              CURLFORM_CONTENTSLENGTH, u__strlen(value, __PRETTY_FUNCTION__),
                              CURLFORM_END);

      U_VAR_UNUSED(res)

      U_INTERNAL_ASSERT_EQUALS(res, CURL_FORMADD_OK)

      U_INTERNAL_ASSERT_POINTER(formPost)
      U_INTERNAL_ASSERT_POINTER(formLast)
      }

   // EXEC

   void reserve(uint32_t n)
      {
      U_TRACE(0, "UCURL::reserve(%u)", n)

      (void) response.reserve(n);
      }

   bool performWait()
      {
      U_TRACE_NO_PARAM(1, "UCURL::performWait()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(easyHandle)

      response.setEmpty();

      result = U_SYSCALL(curl_easy_perform, "%p", easyHandle); // perform, then store the expected code in 'result'

      infoComplete();

      if (result == CURLE_OK) U_RETURN(true);

      U_RETURN(false);
      }

   // INFO

   bool getFileTime(time_t& t)
      {
      U_TRACE(1, "UCURL::getFileTime(%p)", &t)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(easyHandle)

      long filetime;

      result = U_SYSCALL(curl_easy_getinfo, "%p,%d,%p", easyHandle, CURLINFO_FILETIME, &filetime);

      if (result)
         {
         U_DUMP("status = %.*S", 512, error())

         U_RETURN(false);
         }

      t = (time_t) filetime;

      U_RETURN(true);
      }

   // MULTI

   void addHandle()
      {
      U_TRACE_NO_PARAM(1, "UCURL::addHandle()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_EQUALS(added,false)
      U_INTERNAL_ASSERT_POINTER(easyHandle)
      U_INTERNAL_ASSERT_POINTER(multiHandle)

      added = true;

      CURLMcode mresult = (CURLMcode) U_SYSCALL(curl_multi_add_handle, "%p,%p", multiHandle, easyHandle);

      U_VAR_UNUSED(mresult)

      U_INTERNAL_ASSERT_EQUALS(mresult, CURLM_OK)
      }

   void removeHandle()
      {
      U_TRACE_NO_PARAM(1, "UCURL::removeHandle()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(added)
      U_INTERNAL_ASSERT_POINTER(easyHandle)
      U_INTERNAL_ASSERT_POINTER(multiHandle)

      added = false;

      CURLMcode mresult = (CURLMcode) U_SYSCALL(curl_multi_remove_handle, "%p,%p", multiHandle, easyHandle);

      U_VAR_UNUSED(mresult)

      U_INTERNAL_ASSERT_EQUALS(mresult, CURLM_OK)
      }

   static int fdset(fd_set& read, fd_set& write)
      {
      U_TRACE(1, "UCURL::fdset(%p,%p)", &read, &write)

      U_INTERNAL_ASSERT_POINTER(multiHandle)

      fd_set exc;
      int max_fd = -1;

      FD_ZERO(&exc);

      CURLMcode mresult = (CURLMcode) U_SYSCALL(curl_multi_fdset, "%p,%p,%p,%p,%p", multiHandle, &read, &write, &exc, &max_fd);

      U_VAR_UNUSED(mresult)

      U_INTERNAL_ASSERT_EQUALS(mresult, CURLM_OK)

      U_RETURN(max_fd);
      }

   static bool perform();

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UCURL* next;
   CURL* easyHandle;
   struct curl_slist* headerlist;
   struct curl_httppost* formPost;
   struct curl_httppost* formLast;
   UString response;
   int result;
   bool added;

   static bool inited;
   static UCURL* first;
   static CURLM* multiHandle;
   static char errorBuffer[CURL_ERROR_SIZE];

   void infoComplete();

private:
   static void   setup()                                                           U_NO_EXPORT;
   static size_t writeFunction(void* ptr, size_t size, size_t nmemb, void* stream) U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UCURL)
};

#endif
