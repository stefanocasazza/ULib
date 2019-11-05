// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    uhttp.h - HTTP utility
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_HTTP_H
#define ULIB_HTTP_H 1

#include <ulib/timeval.h>
#include <ulib/internal/chttp.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/string_ext.h>
#include <ulib/utility/data_session.h>

#if defined(U_ALIAS) && defined(USE_LIBPCRE) // REWRITE RULE
#  include <ulib/pcre/pcre.h>
#else
#  include <ulib/container/vector.h>
#endif

#define U_MAX_UPLOAD_PROGRESS   16
#define U_MIN_SIZE_FOR_DEFLATE 150 // NB: google advice...

#define U_HTTP_URI_EQUAL(str)               ((str).equal(U_HTTP_URI_TO_PARAM))
#define U_HTTP_URI_DOSMATCH(mask,len,flags) (UServices::dosMatchWithOR(U_HTTP_URI_TO_PARAM, mask, len, flags))

class UFile;
class ULock;
class UHTTP2;
class UEventFd;
class UCommand;
class UPageSpeed;
class USSIPlugIn;
class UHttpPlugIn;
class USSLSession;
class UProxyPlugIn;
class UMimeMultipart;
class UModProxyService;
class UClientImage_Base;

template <class T> class UClient;
template <class T> class URDBObjectHandler;

extern "C" { extern void runDynamicPage_dirlist(int); };

class U_EXPORT UHTTP {
public:

   static void init();
   static void dtor();

   // TYPE

   static bool isMobile() __pure;
   static bool isProxyRequest();

   static bool isTSARequest() __pure;
   static bool isSOAPRequest() __pure;

   static bool isGET()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isGET()")

      if (U_http_method_type == HTTP_GET) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isHEAD()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isHEAD()")

      if (U_http_method_type == HTTP_HEAD) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isPOST()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isPOST()")

      if (U_http_method_type == HTTP_POST) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isPUT()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isPUT()")

      if (U_http_method_type == HTTP_PUT) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isPATCH()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isPATCH()")

      if (U_http_method_type == HTTP_PATCH) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isDELETE()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isDELETE()")

      if (U_http_method_type == HTTP_DELETE) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isCOPY()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isCOPY()")

      if (U_http_method_type == HTTP_COPY) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isGETorHEAD()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isGETorHEAD()")

      if ((U_http_method_type & (HTTP_GET | HTTP_HEAD)) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isGETorPOST()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isGETorPOST()")

      if ((U_http_method_type & (HTTP_GET | HTTP_POST)) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isGETorHEADorPOST()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isGETorHEADorPOST()")

      if ((U_http_method_type & (HTTP_GET | HTTP_HEAD | HTTP_POST)) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isPOSTorPUTorPATCH()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isPOSTorPUTorPATCH()")

      if ((U_http_method_type & (HTTP_POST | HTTP_PUT | HTTP_PATCH)) != 0) U_RETURN(true);

      U_RETURN(false);
      }

   // SERVICES

   static UFile* file;
   static UString* ext;
   static UString* etag;
   static UString* body;
   static UString* qcontent;
   static UString* pathname;
   static UString* rpathname;
   static UString* upload_dir;
   static UString* string_HTTP_Variables;

   static URDB* db_not_found;
   static UModProxyService* service;
   static UVector<UString>* vmsg_error;
   static UHashMap<UString>* prequestHeader;
   static UVector<UModProxyService*>* vservice;

   static char response_buffer[64];
   static off_t range_start, range_size;
   static int mime_index, cgi_timeout; // the time-out value in seconds for output cgi process
   static bool enable_caching_by_proxy_servers, skip_check_cookie_ip_address;
   static uint32_t limit_request_body, request_read_timeout, gzip_level_for_dynamic_content, brotli_level_for_dynamic_content;

   static bool readRequest();
   static bool handlerCache();
   static int  manageRequest();
   static void initDbNotFound();
   static void setStatusDescription();
   static void setEndRequestProcessing();
   static bool callService(const UString& path);
   static void checkContentLength(off_t length);
   static bool isUriRequestNeedCertificate() __pure;
   static bool isValidMethod(const char* ptr) __pure;
   static bool checkContentLength(const UString& response);
   static bool scanfHeaderRequest(const char* ptr, uint32_t size);
   static bool scanfHeaderResponse(const char* ptr, uint32_t size);
   static bool readHeaderResponse(USocket* socket, UString& buffer);
   static bool readBodyResponse(USocket* socket, UString* buffer, UString& lbody);

   static UString getPathComponent(uint32_t index); // Returns the path element at the specified index

   static bool isValidRequest(const char* ptr, uint32_t sz)
      {
      U_TRACE(0, "UHTTP::isValidRequest(%.*S,%u)", 30, ptr, sz)

      U_INTERNAL_ASSERT_MAJOR(sz, 0)

      U_INTERNAL_DUMP("sz = %u UClientImage_Base::size_request = %u", sz, UClientImage_Base::size_request)

      if (u_get_unalignedp32(ptr+sz-4) == U_MULTICHAR_CONSTANT32('\r','\n','\r','\n')) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isValidRequestExt(const char* ptr, uint32_t sz)
      {
      U_TRACE(0, "UHTTP::isValidRequestExt(%.*S,%u)", 30, ptr, sz)

      U_INTERNAL_ASSERT_MAJOR(sz, 0)

      if (sz >= U_CONSTANT_SIZE("GET / HTTP/1.0\r\n\r\n")        &&
          isValidMethod(ptr)                                     &&
          (isValidRequest(ptr, sz)                               ||
                              (UClientImage_Base::size_request   &&
           isValidRequest(ptr, UClientImage_Base::size_request)) ||
           memmem(ptr, sz, U_CONSTANT_TO_PARAM(U_CRLF2)) != U_NULLPTR))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

#ifndef U_HTTP2_DISABLE
   static bool    copyHeaders(UStringRep* key, void* elem);
   static bool upgradeHeaders(UStringRep* key, void* elem)
      {
      U_TRACE(0, "UHTTP::upgradeHeaders(%V,%p)", key, elem)

      if (key->equal(U_CONSTANT_TO_PARAM("Date"))   == false &&
          key->equal(U_CONSTANT_TO_PARAM("Server")) == false)
         {
         if (key->equal(U_CONSTANT_TO_PARAM("Set-Cookie"))) set_cookie->_assign(key);
         else                                               ext->snprintf_add(U_CONSTANT_TO_PARAM("%v: %v\r\n"), key, (const UStringRep*)elem);
         }

      U_RETURN(true);
      }

   static void upgradeResponse(UHashMap<UString>* ptable)
      {
      U_TRACE(0, "UHTTP::upgradeResponse(%p)", ptable)

      U_INTERNAL_DUMP("U_http_info.nResponseCode = %u U_http_info.clength = %u U_http_version = %C", U_http_info.nResponseCode, U_http_info.clength, U_http_version)

      ext->setBuffer(U_CAPACITY);

      ptable->callForAllEntry(upgradeHeaders);

      U_http_version = '2';

      handlerResponse();
      }
#endif

#ifdef USE_LOAD_BALANCE
   static UClient<USSLSocket>* client_http;

   static bool manageRequestOnRemoteServer();
#endif

#ifdef DEBUG
   static uint32_t parserExecute(const char* ptr, uint32_t len, bool response = false);
#endif

   static void setHostname(const char* ptr, uint32_t len);

   static void setHostname(const UString& name) { setHostname(U_STRING_TO_PARAM(name)); }

   static const char* getStatusDescription(uint32_t* plen = U_NULLPTR);

   static uint32_t getUserAgent()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::getUserAgent()")

      uint32_t agent = (U_http_info.user_agent_len ? u_cdb_hash((unsigned char*)U_HTTP_USER_AGENT_TO_PARAM, -1) : 0);

      U_RETURN(agent);
      }

   static bool isSizeForSendfile(off_t sz)
      {
      U_TRACE(0, "UHTTP::isSizeForSendfile(%I)", sz)

      U_INTERNAL_DUMP("U_http_version = %C UServer_Base::min_size_for_sendfile = %u UServer_Base::bssl = %b", U_http_version, UServer_Base::min_size_for_sendfile, UServer_Base::bssl)

#  ifndef U_HTTP2_DISABLE
      if (U_http_version != '2') // NB: within HTTP/2 we can't use sendfile...
#  endif
      {
      if (sz >= UServer_Base::min_size_for_sendfile)
         {
         U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false) // NB: we can't use sendfile with SSL...

         U_RETURN(true);
         }
      }

      U_RETURN(false);
      }

   static void addHTTPVariables(UString& buffer)
      {
      U_TRACE(0, "UHTTP::addHTTPVariables(%V)", buffer.rep)

      U_INTERNAL_ASSERT_POINTER(prequestHeader)
      U_INTERNAL_ASSERT_EQUALS(prequestHeader->empty(), false)

      prequestHeader->callForAllEntry(addHTTPVariables);

      (void) buffer.append(*string_HTTP_Variables);

      string_HTTP_Variables->clear();
      }

   static bool checkDirectoryForDocumentRoot(const char* ptr, uint32_t len)
      {
      U_TRACE(0, "UHTTP::checkDirectoryForDocumentRoot(%.*S,%u)", len, ptr, len)

      U_INTERNAL_DUMP("document_root(%u) = %V", UServer_Base::document_root_size, UServer_Base::document_root->rep)

      U_INTERNAL_ASSERT_POINTER(UServer_Base::document_root_ptr)

      if (len < UServer_Base::document_root_size         ||
            ptr[UServer_Base::document_root_size] != '/' ||
          memcmp(ptr, UServer_Base::document_root_ptr, UServer_Base::document_root_size) != 0)
         {
         U_RETURN(false);
         }

      U_RETURN(true);
      }

   static void startRequest()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::startRequest()")

#  if defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST) || (defined(DEBUG) && !defined(U_LOG_DISABLE))
      UClientImage_Base::startRequest();
#  endif

      // ------------------------------
      // U_http_info.uri
      // ....
      // U_http_info.nResponseCode
      // ....
      // ------------------------------
      U_HTTP_INFO_RESET(0);

      u_clientimage_info.flag.u = 0;
      }

   // UPLOAD

   static vPF on_upload;
   static uint32_t min_size_request_body_for_parallelization;

   static void setUploadDir(const UString& dir)
      {
      U_TRACE(0, "UHTTP::setUploadDir(%V)", dir.rep)

      U_INTERNAL_ASSERT(dir)
      U_INTERNAL_ASSERT_POINTER(upload_dir)

      UString result = checkDirectoryForUpload(dir);

      if (result) *upload_dir = result;
      }

   static UString checkDirectoryForUpload(const char* ptr, uint32_t len);

   static UString checkDirectoryForUpload(const UString& dir) { return checkDirectoryForUpload(U_STRING_TO_PARAM(dir)); }

   static const char* getHeaderValuePtr(const UString& request, const char* name, uint32_t name_len, bool nocase)
      {
      U_TRACE(0, "UHTTP::getHeaderValuePtr(%V,%.*S,%u,%b)", request.rep, name_len, name, name_len, nocase)

      if (U_http_info.endHeader)
         {
         return UStringExt::getValueFromName(request, U_http_info.startHeader,
                                                      U_http_info.endHeader - U_CONSTANT_SIZE(U_CRLF2) - U_http_info.startHeader, name, name_len, nocase);
         }

      U_RETURN((const char*)U_NULLPTR);
      }

#ifndef U_HTTP2_DISABLE
   static const char* getHeaderValuePtr(const char* name, uint32_t name_len, bool nocase);
#else
   static const char* getHeaderValuePtr(const char* name, uint32_t name_len, bool nocase) { return getHeaderValuePtr(*UClientImage_Base::request, name, name_len, nocase); }
#endif

   static const char* getHeaderValuePtr(                        const UString& name, bool nocase) { return getHeaderValuePtr(         U_STRING_TO_PARAM(name), nocase); }
   static const char* getHeaderValuePtr(const UString& request, const UString& name, bool nocase) { return getHeaderValuePtr(request, U_STRING_TO_PARAM(name), nocase); }

   static UString getHeaderMimeType(const char* content, uint32_t size, const char* content_type, time_t expire = 0L);

   // set HTTP response message

   enum RedirectResponseType {
      NO_BODY                         = 0x001,
      REFRESH                         = 0x002,
      PARAM_DEPENDENCY                = 0x004,
      NETWORK_AUTHENTICATION_REQUIRED = 0x008
   };

   static void setResponse()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setResponse()")

      U_ASSERT(ext->empty())
      U_ASSERT(UClientImage_Base::body->empty())

      handlerResponse();
      }

   static void setResponseBody(const UString& content)
      {
      U_TRACE(0, "UHTTP::setResponseBody(%V)", content.rep)

      if (content.empty())
         {
         U_http_info.nResponseCode = HTTP_NO_CONTENT;

         setResponse();

         return;
         }

      U_INTERNAL_ASSERT_EQUALS(U_http_info.nResponseCode, HTTP_OK)

      U_http_info.endHeader       = 0;
      *UClientImage_Base::wbuffer = content;
      }

   static void setResponseMimeIndex(const UString& content, int mime_idx = U_unknow)
      {
      U_TRACE(0, "UHTTP::setResponseMimeIndex(%V,%d)", content.rep, mime_idx)

      setResponseBody(content);

      setDynamicResponse();
      }

   static uint32_t setUrl(char* buffer, uint32_t sz)
      {
      U_TRACE(0, "UHTTP::setUrl(%p,%u)", buffer, sz)

      uint32_t len = 7+U_http_host_len+U_HTTP_URI_QUERY_LEN;

      if (sz > len)
         {
         u_put_unalignedp64(buffer, U_MULTICHAR_CONSTANT64('h','t','t','p',':','/','/','\0'));

         buffer += 7;

         u__memcpy(buffer,                 u_clientimage_info.http_info.host, U_http_host_len,      __PRETTY_FUNCTION__);
         u__memcpy(buffer+U_http_host_len, u_clientimage_info.http_info.uri,  U_HTTP_URI_QUERY_LEN, __PRETTY_FUNCTION__);

         U_RETURN(len);
         }

      U_RETURN(0);
      }

   static void setDynamicResponse();
   static void setResponse(const UString& content_type, UString* pbody = U_NULLPTR);
   static void setRedirectResponse(int mode, const char* ptr_location, uint32_t len_location);
   static void setErrorResponse(const UString& content_type, int code, const char* fmt, uint32_t fmt_size, bool bformat = true);

   // set HTTP main error message

   static void setNotFound()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setNotFound()")

      setErrorResponse(*UString::str_ctype_html, HTTP_NOT_FOUND, U_CONSTANT_TO_PARAM("Your requested URL %.*S was not found on this server"));
      }

   static void setBadMethod()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setBadMethod()")

      U_INTERNAL_ASSERT_EQUALS(U_http_info.nResponseCode, HTTP_BAD_METHOD)

      setErrorResponse(*UString::str_ctype_html, HTTP_BAD_METHOD, U_CONSTANT_TO_PARAM("The requested method is not allowed for the URL %.*S"));
      }

   static void setBadRequest()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setBadRequest()")

      UClientImage_Base::resetPipelineAndSetCloseConnection();

      setErrorResponse(*UString::str_ctype_html, HTTP_BAD_REQUEST, U_CONSTANT_TO_PARAM("Your requested URL %.*S was a request that this server could not understand"));
      }

   static void setForbidden()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setForbidden()")

      UClientImage_Base::setRequestForbidden();

      setErrorResponse(*UString::str_ctype_html, HTTP_FORBIDDEN, U_CONSTANT_TO_PARAM("You don't have permission to access %.*S on this server"));
      }

   static void setMethodNotImplemented()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setMethodNotImplemented()")

      setErrorResponse(*UString::str_ctype_html, HTTP_NOT_IMPLEMENTED, U_CONSTANT_TO_PARAM("Sorry, the method you requested is not implemented"), false);
      }

   static void setEntityTooLarge()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setEntityTooLarge()")

      UClientImage_Base::resetPipelineAndSetCloseConnection();

      setErrorResponse(*UString::str_ctype_html, HTTP_ENTITY_TOO_LARGE, U_CONSTANT_TO_PARAM("Sorry, the data you requested is too large"), false);
      }

   static void setLengthRequired()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setLengthRequired()")

      UClientImage_Base::resetPipelineAndSetCloseConnection();

      setErrorResponse(*UString::str_ctype_html, HTTP_LENGTH_REQUIRED, U_CONSTANT_TO_PARAM("Sorry, you must give the length of your data in your header request"), false);
      }

   static void setUnavailable()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setUnavailable()")

      UClientImage_Base::resetPipelineAndSetCloseConnection();

      setErrorResponse(*UString::str_ctype_html, HTTP_UNAVAILABLE, U_CONSTANT_TO_PARAM("Sorry, the service you requested is unavailable"), false);
      }

   static void setUnAuthorized(bool bstale);

   static void setInternalError()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setInternalError()")

      setErrorResponse(*UString::str_ctype_html, HTTP_INTERNAL_ERROR,
                       U_CONSTANT_TO_PARAM("The server encountered an internal error or misconfiguration "
                                           "and was unable to complete your request. Please contact the server "
                                           "administrator, and inform them of the time the error occurred, and "
                                           "anything you might have done that may have caused the error. More "
                                           "information about this error may be available in the server error log"), false);
      }

   static void setServiceUnavailable()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setServiceUnavailable()")

      UClientImage_Base::resetPipelineAndSetCloseConnection();

      setErrorResponse(*UString::str_ctype_html, HTTP_UNAVAILABLE,
                       U_CONSTANT_TO_PARAM("Sorry, the service you requested is not available at this moment. "
                                           "Please contact the server administrator and inform them about this"), false);
      }

#ifdef U_HTTP_STRICT_TRANSPORT_SECURITY
   static UString* uri_strict_transport_security_mask;

   static bool isUriRequestStrictTransportSecurity() __pure;
#endif

#ifdef U_ALIAS
   static UString* alias;
   static bool virtual_host;
   static UString* global_alias;
   static UVector<UString>* valias;
   static UString* maintenance_mode_page;

   static void setGlobalAlias(const UString& alias);
#endif

   // manage HTTP request service: the tables must be ordered alphabetically cause of binary search...

   typedef struct service_info {
      const char* name;
      uint32_t    len;
      vPF         function;
   } service_info;

#define  GET_ENTRY(name) {#name,U_CONSTANT_SIZE(#name), GET_##name}
#define POST_ENTRY(name) {#name,U_CONSTANT_SIZE(#name),POST_##name}

   static void manageRequest(service_info*  GET_table, uint32_t n1,
                             service_info* POST_table, uint32_t n2);

   // -----------------------------------------------------------------------
   // FORM
   // -----------------------------------------------------------------------
   // retrieve information on specific HTML form elements
   // (such as checkboxes, radio buttons, and text fields), or uploaded files
   // -----------------------------------------------------------------------

   static UString* tmpdir;
   static UMimeMultipart* formMulti;
   static UVector<UString>* form_name_value;

   static uint32_t processForm();

   static void getFormValue(UString& value, uint32_t pos)
      {
      U_TRACE(0, "UHTTP::getFormValue(%V,%u)", value.rep, pos)

      U_INTERNAL_ASSERT_POINTER(form_name_value)

      if (pos >= form_name_value->size()) value.clear();
      else                         (void) value.replace((*form_name_value)[pos]);
      }

   static int getFormFirstNumericValue(int _min, int _max) __pure;

   static void    getFormValue(UString& value, const char* name, uint32_t len);
   static UString getFormValue(                const char* name, uint32_t len, uint32_t start,               uint32_t end);
   static void    getFormValue(UString& value, const char* name, uint32_t len, uint32_t start, uint32_t pos, uint32_t end);

   // COOKIE

   static UString* set_cookie;
   static uint32_t sid_counter_gen;
   static UString* set_cookie_option;
   static UString* cgi_cookie_option;

   static bool    getCookie(      UString* cookie, UString* data);
   static void addSetCookie(const UString& cookie);

   // -----------------------------------------------------------------------------------------------------------------------------------
   // param: "[ data expire path domain secure HttpOnly ]"
   // -----------------------------------------------------------------------------------------------------------------------------------
   // string -- key_id or data to put in cookie    -- must
   // int    -- lifetime of the cookie in HOURS    -- must (0 -> valid until browser exit)
   // string -- path where the cookie can be used  --  opt
   // string -- domain which can read the cookie   --  opt
   // bool   -- secure mode                        --  opt
   // bool   -- only allow HTTP usage              --  opt
   // -----------------------------------------------------------------------------------------------------------------------------------
   // RET: Set-Cookie: ulib.s<counter>=data&expire&HMAC-MD5(data&expire); expires=expire(GMT); path=path; domain=domain; secure; HttpOnly
   // -----------------------------------------------------------------------------------------------------------------------------------

   static void setCookie(const UString& param);
   static void setCookie(const char* format, uint32_t fmt_size, ...)
      {
      U_TRACE(0, "UHTTP::setCookie(%.*S,%u)", fmt_size, format, fmt_size)

      U_INTERNAL_ASSERT_POINTER(format)

      UString param(U_CAPACITY);

      va_list argp;
      va_start(argp, fmt_size);

      param.vsnprintf(format, fmt_size, argp);

      setCookie(param);

      va_end(argp);
      }

   // HTTP SESSION

   static uint32_t sid_counter_cur;
   static UDataSession* data_session;
   static UDataSession* data_storage;
   static URDBObjectHandler<UDataStorage*>* db_session;

   static void  initSession();
   static void clearSession();
   static void removeDataSession();

   static void removeCookieSession()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::removeCookieSession()")

      UString cookie(100U);

      cookie.snprintf(U_CONSTANT_TO_PARAM("ulib.s%u=; expires=%#8D"), sid_counter_cur, u_now->tv_sec - U_ONE_DAY_IN_SECOND);

      addSetCookie(cookie);
      }

   static void setSessionCookie(UString* param = U_NULLPTR);

   // LOGIN COOKIE

   static UString* loginCookie;
   static UString* loginCookieUser;
   static UString* loginCookiePasswd;

   static bool isPostLogin();
   static bool getPostLoginUserPasswd();

   static bool getLoginCookie()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::getLoginCookie()")

      loginCookie->clear();

      if (U_http_info.cookie_len &&
          getCookie(U_NULLPTR, loginCookie))
         {
         if (loginCookie->empty()) (void) loginCookie->replace(data_session->keyid);

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool getDataStorage();
   static bool getDataSession();
   static bool getDataStorage(uint32_t index, UString& value);
   static bool getDataSession(uint32_t index, UString& value);

   static void putDataSTORAGE();
   static void putDataSESSION();
   static void putDataStorage(uint32_t index, const char* val, uint32_t sz);
   static void putDataSession(uint32_t index, const char* val, uint32_t sz);

   static bool    isNewSession()               { return data_session->isNewSession(); }
   static bool    isDataSession()              { return data_session->isDataSession(); }
   static UString getSessionCreationTime()     { return data_session->getSessionCreationTime(); }
   static UString getSessionLastAccessedTime() { return data_session->getSessionLastAccessedTime(); }

#ifdef USE_LIBSSL
   static USSLSession* data_session_ssl;
   static URDBObjectHandler<UDataStorage*>* db_session_ssl;

   static void  initSessionSSL();
   static void clearSessionSSL();
#endif

   static UString getKeyIdDataSession()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::getKeyIdDataSession()")

      U_INTERNAL_ASSERT_POINTER(data_session)

      U_RETURN_STRING(data_session->keyid);
      }

   static UString getKeyIdDataSession(const UString& data)
      {
      U_TRACE(0, "UHTTP::getKeyIdDataSession(%V)", data.rep)

      UString keyid = UDataSession::getKeyIdDataSession(++sid_counter_gen, data);

      U_RETURN_STRING(keyid);
      }

   static void setKeyIdDataStorage(const UString& key)
      {
      U_TRACE(0, "UHTTP::setKeyIdDataStorage(%V)", key.rep)

      U_INTERNAL_ASSERT_POINTER(data_storage)

      data_storage->UDataStorage::setKeyIdDataSession(key);
      }

#ifdef U_SSE_ENABLE // SERVER SENT EVENTS (SSE)
   typedef UString (*strPF)();

   static strPF sse_func;
   static int sse_pipe_fd;
   static bool sse_auth, sse_req;
   static const char* sse_corsbase;

   static bool isValidationSSE();

# ifdef USE_LIBSSL
   static UString SSE_event() { return UString::getStringNull(); }
# endif

   static void manageSSE();
   static void   readSSE(int timeoutMS) __noreturn;
   static void   sendSSE(const UString& data)
      {
      U_TRACE(0, "UHTTP::sendSSE(%V)", data.rep)

      UString buffer = UServer_Base::printSSE(U_SRV_SSE_CNT1, data, UServer_Base::sse_event);

      uint32_t sz = buffer.size();

      if (USocketExt::write(UServer_Base::csocket, buffer.data(), sz, UServer_Base::timeoutMS) != sz)
         {
         UServer_Base::eventSSE(U_CONSTANT_TO_PARAM("DEL %v\n"), UServer_Base::sse_id->rep);

         UServer_Base::endNewChild(); // no return
         }
      }
#endif

   // HTML Pagination

   static uint32_t num_page_end,
                   num_page_cur,
                   num_page_start,
                   num_item_tot,
                   num_item_for_page;

   static UString getLinkPagination();

   static void addLinkPagination(UString& link, uint32_t num_page)
      {
      U_TRACE(0, "UHTTP::addLinkPagination(%V,%u)", link.rep, num_page)

#  ifdef U_HTML_PAGINATION_SUPPORT
      UString x(100U);

      U_INTERNAL_DUMP("num_page_cur = %u", num_page_cur)

      if (num_page == num_page_cur) x.snprintf(U_CONSTANT_TO_PARAM("<span class=\"pnow\">%u</span>"),             num_page);
      else                          x.snprintf(U_CONSTANT_TO_PARAM("<a href=\"?page=%u\" class=\"pnum\">%u</a>"), num_page, num_page);

      (void) link.append(x);
             link.push_back(' ');
#  endif
      }

   // CGI

   typedef struct ucgi {
      const char* interpreter;
      char        environment_type;
      char        dir[503];
   } ucgi;

   static bool bnph;
   static UCommand* pcmd;
   static UString* geoip;
   static UString* fcgi_uri_mask;
   static UString* scgi_uri_mask;

   static bool isFCGIRequest() __pure;
   static bool isSCGIRequest() __pure;

   static bool runCGI(bool set_environment);
   static bool getCGIEnvironment(UString& environment, int type);
   static bool processCGIOutput(bool cgi_sh_script, bool bheaders);
   static bool processCGIRequest(UCommand* cmd, UHTTP::ucgi* cgi = U_NULLPTR);
   static bool setEnvironmentForLanguageProcessing(int type, void* env, vPFpvpcpc func);

#if defined(U_ALIAS) && defined(USE_LIBPCRE) // REWRITE RULE
   class RewriteRule {
   public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UPCRE key;
   UString replacement;

   RewriteRule(const UString& _key, const UString& _replacement) : key(_key, PCRE_FOR_REPLACE), replacement(_replacement)
      {
      U_TRACE_CTOR(0, RewriteRule, "%V,%V", _key.rep, _replacement.rep)

      key.study();
      }

   ~RewriteRule()
      {
      U_TRACE_DTOR(0, RewriteRule)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_ASSIGN(RewriteRule)
   };

   static UVector<RewriteRule*>* vRewriteRule;
#endif      

   // ------------------------------------------------------------------------------------------------------------------------------------------------ 
   // COMMON LOG FORMAT (APACHE LIKE LOG)
   // ------------------------------------------------------------------------------------------------------------------------------------------------ 
   // The Common Log Format, also known as the NCSA Common log format, is a standardized text file format used by web servers
   // when generating server log files. Because the format is standardized, the files may be analyzed by a variety of web analysis programs.
   // Each line in a file stored in the Common Log Format has the following syntax: host ident authuser date request status bytes
   // ------------------------------------------------------------------------------------------------------------------------------------------------ 
   // 10.10.25.2 - - [21/May/2012:16:29:41 +0200] "GET / HTTP/1.1" 200 598 "-" "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/536.5 (KHTML, like Gecko)"
   // 10.10.25.2 - - [21/May/2012:16:29:41 +0200] "GET /unirel_logo.gif HTTP/1.1" 200 3414 "http://www.unirel.com/" "Mozilla/5.0 (X11; Linux x86_64)"
   // ------------------------------------------------------------------------------------------------------------------------------------------------ 

#ifndef U_LOG_DISABLE
   static char iov_buffer[20];
   static struct iovec iov_vec[10];
# if !defined(U_CACHE_REQUEST_DISABLE) || defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST)
   static uint32_t request_offset, referer_offset, agent_offset;
# endif

   static void    initApacheLikeLog();
   static void prepareApacheLikeLog();
   static void   resetApacheLikeLog()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::resetApacheLikeLog()")

      iov_vec[6].iov_len  =
      iov_vec[8].iov_len  = 1;
      iov_vec[6].iov_base =
      iov_vec[8].iov_base = (caddr_t) "-";
      }
#endif

   // USP (ULib Servlet Page)

   class UServletPage : public UDynamic {
   public:

   vPF  runDynamicPage;
   vPFu runDynamicPageParam;
   UString path, basename;

   UServletPage(const void* name, uint32_t nlen, const char* base = U_NULLPTR, uint32_t blen = 0, vPF _runDynamicPage = U_NULLPTR, vPFu _runDynamicPageParam = U_NULLPTR)
         : runDynamicPage(_runDynamicPage), runDynamicPageParam(_runDynamicPageParam), path(name, nlen)
      {
      U_TRACE_CTOR(0, UServletPage, "%.*S,%u,%.*S,%u,%p,%p", nlen, name, nlen, blen, base, blen, _runDynamicPage, _runDynamicPageParam)

      if (blen) (void) basename.replace(base, blen);
      }

   ~UServletPage()
      {
      U_TRACE_DTOR(0, UServletPage)
      }

   // SERVICE

   bool operator<(const UServletPage& other) const { return cmp_obj(&basename, &other.basename); }

   static int cmp_obj(const void* a, const void* b)
      {
      U_TRACE(0, "UServletPage::cmp_obj(%p,%p)", a, b)

#  ifdef U_STDCPP_ENABLE
      /**
       * The comparison function must follow a strict-weak-ordering
       *
       * 1) For all x, it is not the case that x < x (irreflexivity)
       * 2) For all x, y, if x < y then it is not the case that y < x (asymmetry)
       * 3) For all x, y, and z, if x < y and y < z then x < z (transitivity)
       * 4) For all x, y, and z, if x is incomparable with y, and y is incomparable with z, then x is incomparable with z (transitivity of incomparability)
       */

      return (((const UServletPage*)a)->basename.compare(((const UServletPage*)b)->basename) < 0);
#  else
      return (*(const UServletPage**)a)->basename.compare((*(const UServletPage**)b)->basename);
#  endif
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

private:
   bool load() U_NO_EXPORT;
   void loadConfig() U_NO_EXPORT;
   bool isPath(const char* pathname, uint32_t len)
      {
      U_TRACE(0, "UServletPage::isPath(%.*S,%u)", len, pathname, len)

      if (path.equal(pathname, len)) U_RETURN(true);

      U_RETURN(false);
      }

   U_DISALLOW_ASSIGN(UServletPage)

                      friend class UHTTP;
   template <class T> friend void u_construct(const T**,bool);
   };

   static UServletPage* usp;
   static bool bcallInitForAllUSP;
   static UVector<UServletPage*>* vusp;

   static void       callEndForAllUSP();
   static void      callInitForAllUSP();
   static void    callSigHUPForAllUSP();
   static void callAfterForkForAllUSP();

   static bool checkForUSP();
   static bool getUSP(const char* key, uint32_t key_len);
   static bool runUSP(const char* key, uint32_t key_len);
   static bool runUSP(const char* key, uint32_t key_len, int param);

   // CSP (C Servlet Page)

   typedef int (*iPFipvc)(int,const char**);

   class UCServletPage {
   public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   int size;
   void* relocated;
   iPFipvc prog_main;

   UCServletPage()
      {
      U_TRACE_CTOR(0, UCServletPage, "")

      size      = 0;
      relocated = U_NULLPTR;
      prog_main = U_NULLPTR;
      }

   ~UCServletPage()
      {
      U_TRACE_DTOR(0, UCServletPage)

      if (relocated) UMemoryPool::_free(relocated, size, 1);
      }

   bool compile(UString& program);

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_COPY_AND_ASSIGN(UCServletPage)
   };

#ifdef USE_PHP // (wrapper to embed the PHP interpreter)
   class UPHP : public UDynamic {
   public:

   bPF initPHP;
   bPF  runPHP;
   vPF  endPHP;

   UPHP()
      {
      U_TRACE_CTOR(0, UPHP, "")

      initPHP =
       runPHP = U_NULLPTR;
       endPHP = U_NULLPTR;
      }

   ~UPHP()
      {
      U_TRACE_DTOR(0, UPHP)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_COPY_AND_ASSIGN(UPHP)
   };

   static UPHP* php_embed;
#endif
   static uint32_t npathinfo;
   static UString* php_mount_point;

#ifdef USE_RUBY // (wrapper to embed the RUBY interpreter)
   class URUBY : public UDynamic {
   public:

   bPF initRUBY;
   bPF  runRUBY;
   vPF  endRUBY;

   URUBY()
      {
      U_TRACE_CTOR(0, URUBY, "")

      initRUBY =
       runRUBY = U_NULLPTR;
       endRUBY = U_NULLPTR;
      }

   ~URUBY()
      {
      U_TRACE_DTOR(0, URUBY)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_COPY_AND_ASSIGN(URUBY)
   };

   static URUBY* ruby_embed;
   static bool ruby_on_rails;
   static UString* ruby_libdir;
#endif

#ifdef USE_PYTHON // (wrapper to embed the PYTHON interpreter)
   class UPYTHON : public UDynamic {
   public:

   bPF initPYTHON;
   bPF  runPYTHON;
   vPF  endPYTHON;

   UPYTHON()
      {
      U_TRACE_CTOR(0, UPYTHON, "")

      initPYTHON =
       runPYTHON = U_NULLPTR;
       endPYTHON = U_NULLPTR;
      }

   ~UPYTHON()
      {
      U_TRACE_DTOR(0, UPYTHON)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_COPY_AND_ASSIGN(UPYTHON)
   };

   static UPYTHON* python_embed;
   static UString* py_project_app;
   static UString* py_project_root;
   static UString* py_virtualenv_path;
#endif

#if defined(USE_PAGE_SPEED) || defined(USE_LIBV8)
   typedef void (*vPFstr)(UString&);
#endif

#ifdef USE_PAGE_SPEED // (Google Page Speed)
   typedef void (*vPFpcstr)(const char*, UString&);

   class UPageSpeed : public UDynamic {
   public:

   vPFpcstr minify_html;
   vPFstr optimize_gif, optimize_png, optimize_jpg;

   UPageSpeed()
      {
      U_TRACE_CTOR(0, UPageSpeed, "")

      minify_html  = 0;
      optimize_gif = optimize_png = optimize_jpg = 0;
      }

   ~UPageSpeed()
      {
      U_TRACE_DTOR(0, UPageSpeed)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_COPY_AND_ASSIGN(UPageSpeed)
   };

   static UPageSpeed* page_speed;
#endif

#ifdef USE_LIBV8 // (Google V8 JavaScript Engine)
   class UV8JavaScript : public UDynamic {
   public:

   vPFstr runv8;

   UV8JavaScript()
      {
      U_TRACE_CTOR(0, UV8JavaScript, "")

      runv8 = U_NULLPTR;
      }

   ~UV8JavaScript()
      {
      U_TRACE_DTOR(0, UV8JavaScript)
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
   U_DISALLOW_COPY_AND_ASSIGN(UV8JavaScript)
   };

   static UV8JavaScript* v8_javascript;
#endif

   // DOCUMENT ROOT CACHE

   class UFileCacheData {
   public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   void* ptr;               // data
   UVector<UString>* array; // content, header, gzip(content, header), brotli(content, header)
#ifndef U_HTTP2_DISABLE
   UVector<UString>* http2; //          header, gzip(         header), brotli(         header)
#endif
   time_t mtime;            // time of last modification
   time_t expire;           // expire time of the entry
   uint32_t size;           // size content
   int wd;                  // if directory it is a "watch list" associated with an inotify instance...
   mode_t mode;             // file type
   int mime_index;          // index file mime type
   int fd;                  // file descriptor
   bool link;               // true => ptr data point to another entry

    UFileCacheData();
    UFileCacheData(const UFileCacheData& elem);
   ~UFileCacheData();

   // STREAM

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT istream& operator>>(istream& is,       UFileCacheData& d);
   friend U_EXPORT ostream& operator<<(ostream& os, const UFileCacheData& d);

# ifdef DEBUG
   const char* dump(bool reset) const U_EXPORT;
# endif
#endif

   private:
   U_DISALLOW_ASSIGN(UFileCacheData)

   template <class T> friend void u_construct(const T**,bool);
   };

   static UString* cache_file_mask;
   static UString* cache_avoid_mask;
   static UString* cache_file_store;
   static UFileCacheData* file_data;
   static UString* nocache_file_mask;
   static UFileCacheData* file_gzip_bomb;
   static UString* cache_file_as_dynamic_mask;
   static UHashMap<UFileCacheData*>* cache_file;
   static UFileCacheData* file_not_in_cache_data;

   static bool isDataFromCache()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isDataFromCache()")

      U_INTERNAL_ASSERT_POINTER(file_data)

      U_INTERNAL_DUMP("file_data->array = %p", file_data->array)

      if (file_data->array != U_NULLPTR) U_RETURN(true);

      U_RETURN(false);
      }

   static void renewFileDataInCache();
   static void checkFileForCache(const UString& path);
   static void setResponseFromFileCache(UFileCacheData* pdata);

   static UFileCacheData* getFileCachePointer(const char* path, uint32_t len)
      {
      U_TRACE(0, "UHTTP::getFileCachePointer(%.*S,%u)", len, path, len)

      return cache_file->at(path, len);
      }

   static UFileCacheData* getFileCachePointer(const UString& path) { return getFileCachePointer(U_STRING_TO_PARAM(path)); }

   static UFileCacheData* getFileCachePointerVar(const char* format, uint32_t fmt_size, ...)
      {
      U_TRACE(0, "UHTTP::getFileCachePointerVar(%.*S,%u)", fmt_size, format, fmt_size)

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      va_list argp;
      va_start(argp, fmt_size);

      return cache_file->at(u_buffer, u__vsnprintf(u_buffer, U_BUFFER_SIZE, format, fmt_size, argp));

      va_end(argp);
      }

   static bool   getFileInCache(const char* path, uint32_t len);
   static bool checkFileInCache(const char* path, uint32_t len)
      {
      U_TRACE(0, "UHTTP::checkFileInCache(%.*S,%u)", len, path, len)

      if ((file_data = getFileCachePointer(path, len)))
         {
         file->st_size  = file_data->size;
         file->st_mode  = file_data->mode;
         file->st_mtime = file_data->mtime;

         U_DUMP("file_data->fd = %d st_size = %I st_mtime = %#3D dir() = %b", file_data->fd, file->st_size, file->st_mtime, file->dir())

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool isFileInCache()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::isFileInCache()")

      return checkFileInCache(U_FILE_TO_PARAM(*file));
      }

   static uint32_t old_path_len;

   static bool checkFileInCacheOld(const char* path, uint32_t len)
      {
      U_TRACE(0, "UHTTP::checkFileInCacheOld(%.*S,%u)", len, path, len)

      U_INTERNAL_DUMP("old_path_len = %u", old_path_len)

      if (old_path_len != len) return checkFileInCache(path, (old_path_len = len));

      if (file_data) U_RETURN(true);

      U_RETURN(false);
      }

   static UString getDataFromCache(UVector<UString>* array, uint32_t idx)
      {
      U_TRACE(0, "UHTTP::getDataFromCache(%p,%u)", array, idx)

      U_INTERNAL_ASSERT_MINOR(idx, 6)

      U_INTERNAL_DUMP("U_http_version = %C", U_http_version)

      if (array &&
          idx < array->size())
         {
         UString result = array->at(idx);

         U_RETURN_STRING(result);
         }

      return UString::getStringNull();
      }

#ifdef U_HTTP2_DISABLE
   static UString getHeaderFromCache(uint32_t idx) { return                          getDataFromCache(file_data->array, idx); }
#else
   static UString getHeaderFromCache(uint32_t idx) { return (U_http_version != '2' ? getDataFromCache(file_data->array, idx) : getDataFromCache(file_data->http2, idx / 2)); }
#endif

   static UString getBodyFromCache()                 { return getDataFromCache(file_data->array, 0); }
   static UString getHeaderFromCache()               { return getHeaderFromCache(1); }
   static UString getBodyCompressFromCache()         { return getDataFromCache(file_data->array, 2); }
   static UString getHeaderCompressFromCache()       { return getHeaderFromCache(3); }
   static UString getBodyCompressBrotliFromCache()   { return getDataFromCache(file_data->array, 4); }
   static UString getHeaderCompressBrotliFromCache() { return getHeaderFromCache(5); }

   static UString contentOfFromCache(const char* path, uint32_t len)
      {
      U_TRACE(0, "UHTTP::contentOfFromCache(%.*S,%u)", len, path, len)

      if ((file_data = getFileCachePointer(path, len)))
         {
         UString result = getBodyFromCache();

         U_RETURN_STRING(result);
         }

      return UString::getStringNull();
      }

   static UString contentOfFromCache(const UString& path) { return contentOfFromCache(U_STRING_TO_PARAM(path)); }

   // URI PROTECTION (for example directory listing)

   static UString*  fpasswd;
   static UString* htpasswd;
   static UString* htdigest;
   static UString* user_authentication;
   static time_t htdigest_mtime, htpasswd_mtime;
   static bool uri_overload_authentication, buri_overload_authentication, digest_authentication; // authentication method (digest|basic)

   static UString getUserAuthentication() { return *user_authentication; }

   // -----------------------------------------------------------------------------------------------
   // for Jonathan Kelly
   // -----------------------------------------------------------------------------------------------
   static UFileCacheData* getPasswdDB(const char* name, uint32_t len);
   static bool           savePasswdDB(const char* name, uint32_t len, UFileCacheData* ptr_file_data); // Save Changes to Disk and Cache

   static void    setPasswdUser(const UString& username, const UString& password); // Add/Update passwd User
   static bool revokePasswdUser(const UString& username);                          //     Remove passwd User
   // -----------------------------------------------------------------------------------------------

#ifdef USE_LIBSSL
   static UString* uri_protected_mask;
   static UVector<UIPAllow*>* vallow_IP;
   static UString* uri_request_cert_mask;

   static bool checkUriProtected();
#endif

#if defined(U_HTTP_STRICT_TRANSPORT_SECURITY) || defined(USE_LIBSSL)
   static bool isValidation();
#endif

private:
   static uint32_t is_response_compressed;

   static void setMimeIndex()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::setMimeIndex()")

      U_INTERNAL_ASSERT_POINTER(file)
      U_ASSERT_EQUALS(UClientImage_Base::isRequestNotFound(), false)

      const char* uri_suffix = u_getsuffix(U_FILE_TO_PARAM(*file));

      U_INTERNAL_DUMP("uri_suffix = %p", uri_suffix)

      if (uri_suffix == U_NULLPTR) mime_index = U_unknow;
      else                         (void) u_get_mimetype(uri_suffix+1, &mime_index);
      }

   static const char* setMimeIndex(const char* suffix)
      {
      U_TRACE(0, "UHTTP::setMimeIndex(%S)", suffix)

      U_INTERNAL_ASSERT_POINTER(file)

      mime_index = U_unknow;

      const char* ctype = file->getMimeType(suffix, &mime_index);

      file_data->mime_index = mime_index;

      return ctype;
      }

   static int handlerREAD();
   static void processRequest();
   static void handlerResponse();
   static void setDynamicResponse(const UString& lbody, const UString& header = UString::getStringNull(), const UString& content_type = UString::getStringNull());

#ifndef U_LOG_DISABLE
   static int handlerREADWithLog()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::handlerREADWithLog()")

      U_ASSERT(UServer_Base::isLog())

      (void) strcpy(UServer_Base::mod_name[0], "[http] ");

      U_ClientImage_state = handlerREAD();

      UServer_Base::mod_name[0][0] = '\0';

      U_RETURN(U_ClientImage_state);
      }

   static void processRequestWithLog()
      {
      U_TRACE_NO_PARAM(0, "UHTTP::processRequestWithLog()")

      U_ASSERT(UServer_Base::isLog())

      (void) strcpy(UServer_Base::mod_name[0], "[http] ");

      processRequest();

      UServer_Base::mod_name[0][0] = '\0';
      }
#endif

   static UString getHTMLDirectoryList();

#if defined(U_ALIAS) && defined(USE_LIBPCRE) // REWRITE RULE
   static void processRewriteRule() U_NO_EXPORT;
#endif

#if defined(HAVE_SYS_INOTIFY_H) && defined(U_HTTP_INOTIFY_SUPPORT)
   static int             inotify_wd;
   static char*           inotify_name;
   static uint32_t        inotify_len;
   static UString*        inotify_pathname;
   static UStringRep*     inotify_dir;
   static UFileCacheData* inotify_file_data;

   static void in_READ();
   static void initInotify();
   static void setInotifyPathname() U_NO_EXPORT;
   static bool getInotifyPathDirectory(UStringRep* key, void* value) U_NO_EXPORT;
   static bool checkForInotifyDirectory(UStringRep* key, void* value) U_NO_EXPORT;
#endif

#if defined(USE_LIBZ) || defined(USE_LIBBROTLI)
   static bool checkForCompression(uint32_t size)
      {
      U_TRACE(0, "UHTTP::checkForCompression(%u)", size)

      U_INTERNAL_DUMP("U_http_is_accept_gzip = %b U_http_is_accept_brotli = %b", U_http_is_accept_gzip, U_http_is_accept_brotli)

      if (size > U_MIN_SIZE_FOR_DEFLATE)
         {
#     ifdef USE_LIBBROTLI
         if (U_http_is_accept_brotli) U_RETURN(true);
#     endif
#     ifdef USE_LIBZ
      if (U_http_is_accept_gzip) U_RETURN(true);
#     endif
         }

      U_RETURN(false);
      }

# ifdef USE_LIBBROTLI
   static void checkArrayCompressData(UFileCacheData* ptr) U_NO_EXPORT;
# endif

   static inline bool compress(UString& header, const UString& lbody) U_NO_EXPORT;
   static inline void setAcceptEncoding(const char* ptr, uint32_t len) U_NO_EXPORT;
#endif

#ifdef U_STATIC_ONLY
   static void loadStaticLinkedServlet(const char* name, uint32_t len, vPF runDynamicPage, vPFu runDynamicPageParam) U_NO_EXPORT;
#endif      

   static bool callService() U_NO_EXPORT;
   static bool checkPathName() U_NO_EXPORT;
   static void checkIPClient() U_NO_EXPORT;
   static bool runDynamicPage() U_NO_EXPORT;
   static void processFileCache() U_NO_EXPORT;
   static bool readHeaderRequest() U_NO_EXPORT;
   static bool processGetRequest() U_NO_EXPORT;
   static void processDataFromCache() U_NO_EXPORT;
   static bool checkRequestForHeader() U_NO_EXPORT;
   static bool checkGetRequestIfRange() U_NO_EXPORT;
   static void setCGIShellScript(UString& command) U_NO_EXPORT;
   static bool checkIfSourceHasChangedAndCompileUSP() U_NO_EXPORT;
   static int  checkGetRequestForRange(const char* pdata) U_NO_EXPORT;
   static bool compileUSP(const char* path, uint32_t len) U_NO_EXPORT;
   static bool manageSendfile(const char* ptr, uint32_t len) U_NO_EXPORT;
   static int  sortRange(const void* a, const void* b) __pure U_NO_EXPORT;
   static bool addHTTPVariables(UStringRep* key, void* value) U_NO_EXPORT;
   static void setContentResponse(const UString& content_type) U_NO_EXPORT;
   static bool splitCGIOutput(const char*& ptr1, const char* ptr2) U_NO_EXPORT;
   static void setHeaderForCache(UFileCacheData* ptr, UString& data) U_NO_EXPORT;
   static void setResponseForRange(off_t start, off_t end, uint32_t header) U_NO_EXPORT;
   static bool readDataChunked(USocket* sk, UString* pbuffer, UString& lbody) U_NO_EXPORT;
   static void manageDataForCache(const UString& basename, const UString& suffix) U_NO_EXPORT;
   static bool checkDataSession(const UString& token, time_t expire, UString* data) U_NO_EXPORT;
   static void putDataInCache(const UString& path, const UString& fmt, UString& content) U_NO_EXPORT;
   static void addContentLengthToHeader(UString& header, char* ptr, uint32_t size, const char* pEndHeader = U_NULLPTR) U_NO_EXPORT;
   static void setDataInCache(const UString& fmt, const UString& content, const char* encoding, uint32_t encoding_len) U_NO_EXPORT;
   static bool processAuthorization(const char* ptr, uint32_t sz, const char* pattern = U_NULLPTR, uint32_t len = 0) U_NO_EXPORT;

   static UString getPathToWriteUploadData(const char* ptr, uint32_t sz) U_NO_EXPORT;

   static inline void resetFileCache() U_NO_EXPORT;
   static inline void setUpgrade(const char* ptr) U_NO_EXPORT;
   static inline bool checkPathName(uint32_t len) U_NO_EXPORT;
   static inline bool checkGetRequestIfModified() U_NO_EXPORT;
   static inline void setConnection(const char* ptr) U_NO_EXPORT;
   static inline void setIfModSince(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline bool setSendfile(int fd, off_t start, off_t count) U_NO_EXPORT;
   static inline void setContentLength(const char* ptr1, const char* ptr2) U_NO_EXPORT;

   static inline bool checkDataChunked(UString* pbuffer) U_NO_EXPORT;
   static inline void setRange(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setUserAgent(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setAccept(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setReferer(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setXRealIP(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setContentType(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setCookieHeader(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setAcceptLanguage(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setXForwardedFor(const char* ptr, uint32_t len) U_NO_EXPORT;
   static inline void setXHttpForwardedFor(const char* ptr, uint32_t len) U_NO_EXPORT;

   static uint32_t getPosPasswd(const UString& line) __pure U_NO_EXPORT;
   static uint32_t  checkPasswd(UFileCacheData* ptr_file_data, const UString& line) U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UHTTP)

   friend class UHTTP2;
   friend class USSIPlugIn;
   friend class UHttpPlugIn;
   friend class UProxyPlugIn;
   friend class UClientImage_Base;

   friend void runDynamicPage_dirlist(int);

#ifdef U_STDCPP_ENABLE
   friend istream& operator>>(istream&, UHTTP::UFileCacheData&);
#endif
};

template <> inline void u_destroy(const UHTTP::UFileCacheData* elem)
{
   U_TRACE(0, "u_destroy<UFileCacheData>(%p)", elem)

   if (elem <= (const void*)0x0000ffff) U_ERROR("u_destroy<UFileCacheData>(%p)", elem);

   // NB: we need this check if we are at the end of UHashMap<UHTTP::UFileCacheData*>::operator>>()...

   if (UHashMap<void*>::istream_loading)
      {
      ((UHTTP::UFileCacheData*)elem)->ptr   =
      ((UHTTP::UFileCacheData*)elem)->array = U_NULLPTR;
#  ifndef U_HTTP2_DISABLE
      ((UHTTP::UFileCacheData*)elem)->http2 = U_NULLPTR;
#  endif
      }

   U_DELETE(elem)
}
#endif
