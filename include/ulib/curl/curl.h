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

#include <ulib/string.h>

extern "C" {
#include <curl/curl.h>
}

/**
 * @class UCURL
 *
 * @brief This class is a wrapper around the cURL library.
 */

class U_EXPORT UCURL {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // COSTRUTTORI

    UCURL();
   ~UCURL();

   // VARIE

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

      /* new stuff needed for libcurl 7.10 */

      setOption(CURLOPT_SSL_VERIFYPEER, 0);
      setOption(CURLOPT_SSL_VERIFYHOST, 1);
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

   void setUserAgent(const char* userAgent)
      {
      U_TRACE(0, "UCURL::setUserAgent(%S)", userAgent)

      setOption(CURLOPT_USERAGENT, (long)userAgent);
      }

   void setDNSCachingTime(int time)
      {
      U_TRACE(0, "UCURL::setDNSCachingTime(%d)", time)

      setOption(CURLOPT_DNS_CACHE_TIMEOUT, (long)time);
      }

   void setProgressFunction(curl_progress_callback func, void* data)
      {
      U_TRACE(0, "UCURL::setProgressFunction(%p,%p)", func, data)

      if (func)
         {
         setOption(CURLOPT_PROGRESSFUNCTION, (long)func);
         setOption(CURLOPT_PROGRESSDATA,     (long)data);
         setOption(CURLOPT_NOPROGRESS,       0L);
         }
      else
         {
         setOption(CURLOPT_NOPROGRESS,       1L);
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

      if (modified_since)
         {
         setOption(CURLOPT_TIMECONDITION, CURL_TIMECOND_IFMODSINCE);
         setOption(CURLOPT_TIMEVALUE,     (long)t);
         }
      else
         {
         setOption(CURLOPT_TIMECONDITION, CURL_TIMECOND_NONE);
         }
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

      setOption(CURLOPT_POSTFIELDS,    (long)postData);
      setOption(CURLOPT_POSTFIELDSIZE, (long)size);
      setOption(CURLOPT_POST,          1L);
      }

   void addFormData(const char* key, const char* value)
      {
      U_TRACE(0, "UCURL::addFormData(%S,%S)", key, value)

      CURLFORMcode res = (CURLFORMcode) U_SYSCALL(curl_formadd, "%p,%p,%d,%S,%d,%S,%d,%d,%d",
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

      result = U_SYSCALL(curl_easy_perform, "%p", easyHandle);

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

#ifdef U_COMPILER_DELETE_MEMBERS
   UCURL(const UCURL&) = delete;
   UCURL& operator=(const UCURL&) = delete;
#else
   UCURL(const UCURL&)            {}
   UCURL& operator=(const UCURL&) { return *this; }
#endif
};

#endif
