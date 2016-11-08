// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_http.cpp - this is a plugin http for userver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/db/rdb.h>
#include <ulib/command.h>
#include <ulib/file_config.h>
#include <ulib/utility/uhttp.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/string_ext.h>
#include <ulib/net/server/plugin/mod_http.h>

#ifndef U_HTTP2_DISABLE
#  include <ulib/utility/http2.h>
#endif

U_CREAT_FUNC(server_plugin_http, UHttpPlugIn)

UHttpPlugIn::~UHttpPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, UHttpPlugIn)

   UHTTP::dtor(); // delete global HTTP context...
}

// define method VIRTUAL of class UEventFd

int UHttpPlugIn::handlerRead()
{
   U_TRACE_NO_PARAM(0, "UHttpPlugIn::handlerRead()")
   
#if defined(HAVE_SYS_INOTIFY_H) && defined(U_HTTP_INOTIFY_SUPPORT) && !defined(U_SERVER_CAPTIVE_PORTAL)
   U_INTERNAL_ASSERT_POINTER(UHTTP::cache_file)

   UHTTP::in_READ();
#endif

   U_RETURN(U_NOTIFIER_OK);
}

void UHttpPlugIn::handlerDelete()
{
   U_TRACE_NO_PARAM(0, "UHttpPlugIn::handlerDelete()")

   U_INTERNAL_DUMP("UEventFd::fd = %d", UEventFd::fd)

   UEventFd::fd = -1;
}

// Server-wide hooks

int UHttpPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "UHttpPlugIn::handlerConfig(%p)", &cfg)

   // ------------------------------------------------------------------------------------------------------------------------------------------------
   // ALIAS                  vector of URI redirection (request -> alias)
   // REWRITE_RULE_NF        vector of URI rewrite rule applied after checks that files do not exist (regex1 -> uri1 ...)
   // USP_AUTOMATIC_ALIASING USP page that is recognized automatically as alias of all uri request without suffix
   //
   // MAINTENANCE_MODE       to switch the site to a maintenance page only
   //
   // APACHE_LIKE_LOG        file to write NCSA extended/combined log format: "%h %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-agent}i\""
   // LOG_FILE_SZ            memory size for file apache like log
   //
   // ENABLE_INOTIFY         enable automatic update of document root image with inotify
   // CACHE_FILE_MASK        mask (DOS regexp) of pathfile that content      be cached in memory
   // CACHE_AVOID_MASK       mask (DOS regexp) of pathfile that presence NOT be cached in memory
   // NOCACHE_FILE_MASK      mask (DOS regexp) of pathfile that content  NOT be cached in memory
   // CACHE_FILE_STORE       pathfile of memory cache stored on filesystem
   //
   // CGI_TIMEOUT            timeout for cgi execution
   // VIRTUAL_HOST           flag to activate practice of maintaining more than one server on one machine, as differentiated by their apparent hostname
   // DIGEST_AUTHENTICATION  flag authentication method (yes = digest, no = basic)
   //
   // ENABLE_CACHING_BY_PROXY_SERVERS enable caching by proxy servers (add "Cache control: public" directive)
   //
   // URI_PROTECTED_MASK       mask (DOS regexp) of URI protected from prying eyes
   // URI_PROTECTED_ALLOWED_IP list of comma separated client address for IP-based access control (IPADDR[/MASK]) for URI_PROTECTED_MASK
   //
   // URI_REQUEST_CERT_MASK                      mask (DOS regexp) of URI where client must comunicate a certificate in the SSL connection
   // BANDWIDTH_THROTTLING_MASK                  lets you set maximum byte rates on URLs or URL groups (*.jpg|*.gif 50)
   // URI_REQUEST_STRICT_TRANSPORT_SECURITY_MASK mask (DOS regexp) of URI where use HTTP Strict Transport Security to force client to use only SSL
   //
   // SESSION_COOKIE_OPTION  eventual params for session cookie (lifetime, path, domain, secure, HttpOnly)  
   // ----------------------------------------------------------------------------------------------------------------------------------------------------
   // This directive gives greater control over abnormal client request behavior, which may be useful for avoiding some forms of denial-of-service attacks
   // ----------------------------------------------------------------------------------------------------------------------------------------------------
   // LIMIT_REQUEST_BODY   restricts the total size of the HTTP request body sent from the client
   // REQUEST_READ_TIMEOUT set timeout for receiving requests
   // ------------------------------------------------------------------------------------------------------------------------------------------------
   //
   // ------------------------------------------------------------------------------------------------------------------------------------------------
   // PHP
   // ------------------------------------------------------------------------------------------------------------------------------------------------
   // MOUNT_POINT          mount point application (to adjust var SCRIPT_NAME)
   // ------------------------------------------------------------------------------------------------------------------------------------------------
   // RUBY
   // ------------------------------------------------------------------------------------------------------------------------------------------------
   // RUBY_LIBDIR          directory to add to the ruby libdir search path
   // ------------------------------------------------------------------------------------------------------------------------------------------------
   // PYTHON
   // ------------------------------------------------------------------------------------------------------------------------------------------------
   // PY_PROJECT_APP       full python name of WSGI entry point expected in form <module>.<app>
   // PY_PROJECT_ROOT      python module search root; relative to workdir
   // PY_VIRTUALENV_PATH
   // ------------------------------------------------------------------------------------------------------------------------------------------------

   if (cfg.loadTable())
      {
      UString x;

      // VIRTUAL HOST

      bool bvirtual_host = cfg.readBoolean(U_CONSTANT_TO_PARAM("VIRTUAL_HOST"));

#  ifndef U_ALIAS
      if (bvirtual_host) U_SRV_LOG("WARNING: Sorry, I can't enable virtual hosting because alias URI support is missing, please recompile ULib");
#  else
      x = cfg.at(U_CONSTANT_TO_PARAM("MAINTENANCE_MODE"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::maintenance_mode_page, 0)

         U_NEW(UString, UHTTP::maintenance_mode_page, UString(x));

         U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
         }

      UHTTP::virtual_host = bvirtual_host;

      x = cfg.at(U_CONSTANT_TO_PARAM("USP_AUTOMATIC_ALIASING"));

      if (x) UHTTP::setGlobalAlias(x); // NB: automatic alias of all uri request without suffix...
      else
         {
         x = cfg.at(U_CONSTANT_TO_PARAM("ALIAS"));

         if (x)
            {
            UVector<UString> vec(x);
            uint32_t n = vec.size();

            if (n < 2) U_ERROR("UHttpPlugIn::handlerConfig(): vector ALIAS malformed: %S", x.rep);

            U_INTERNAL_ASSERT_EQUALS(UHTTP::valias, 0)

            U_NEW(UVector<UString>, UHTTP::valias, UVector<UString>(vec, n));
            }
         }

#    ifdef USE_LIBPCRE
      x = cfg.at(U_CONSTANT_TO_PARAM("REWRITE_RULE_NF"));

      if (x)
         {
         UVector<UString> vec(x);
         uint32_t n = vec.size();

         if (n < 2) U_ERROR("UHttpPlugIn::handlerConfig(): vector REWRITE_RULE_NF malformed: %S", x.rep);

         U_INTERNAL_ASSERT_EQUALS(UHTTP::vRewriteRule, 0)

         U_NEW(UVector<UHTTP::RewriteRule*>, UHTTP::vRewriteRule, UVector<UHTTP::RewriteRule*>(n));

         for (int32_t i = 0; i < (int32_t)n; i += 2)
            {
            UHTTP::RewriteRule* prule;
            
            U_NEW(UHTTP::RewriteRule, prule, UHTTP::RewriteRule(vec[i], vec[i+1]));

            UHTTP::vRewriteRule->push_back(prule);
            }
         }
#   endif
#  endif

      UHTTP::cgi_timeout                     = cfg.readLong(U_CONSTANT_TO_PARAM("CGI_TIMEOUT"));
      UHTTP::limit_request_body              = cfg.readLong(U_CONSTANT_TO_PARAM("LIMIT_REQUEST_BODY"), U_STRING_MAX_SIZE);
      UHTTP::request_read_timeout            = cfg.readLong(U_CONSTANT_TO_PARAM("REQUEST_READ_TIMEOUT"));
      UHTTP::enable_caching_by_proxy_servers = cfg.readBoolean(U_CONSTANT_TO_PARAM("ENABLE_CACHING_BY_PROXY_SERVERS"));

      U_INTERNAL_DUMP("UHTTP::limit_request_body = %u", UHTTP::limit_request_body)

      // CACHE FILE

      x = cfg.at(U_CONSTANT_TO_PARAM("CACHE_FILE_MASK"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::cache_file_mask, 0)

         if (x.findWhiteSpace() != U_NOT_FOUND) x = UStringExt::removeWhiteSpace(x);

         U_NEW(UString, UHTTP::cache_file_mask, UString(x));
         }

      x = cfg.at(U_CONSTANT_TO_PARAM("CACHE_AVOID_MASK"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::cache_avoid_mask, 0)

         if (x.findWhiteSpace() != U_NOT_FOUND) x = UStringExt::removeWhiteSpace(x);

         U_NEW(UString, UHTTP::cache_avoid_mask, UString(x));
         }

      x = cfg.at(U_CONSTANT_TO_PARAM("NOCACHE_FILE_MASK"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::nocache_file_mask, 0)

         if (x.findWhiteSpace() != U_NOT_FOUND) x = UStringExt::removeWhiteSpace(x);

         U_NEW(UString, UHTTP::nocache_file_mask, UString(x));
         }

#  ifdef U_STDCPP_ENABLE
      x = cfg.at(U_CONSTANT_TO_PARAM("CACHE_FILE_STORE"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::cache_file_store, 0)

         if (x.findWhiteSpace() != U_NOT_FOUND) x = UStringExt::removeWhiteSpace(x);

         U_NEW(UString, UHTTP::cache_file_store, UString(x));
         }
#  endif

      // COOKIE OPTION

      x = cfg.at(U_CONSTANT_TO_PARAM("SESSION_COOKIE_OPTION"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::cgi_cookie_option, 0)

         U_NEW(UString, UHTTP::cgi_cookie_option, UString(x));
         }

      // HTTP STRICT TRANSPORT SECURITY

#  ifdef U_HTTP_STRICT_TRANSPORT_SECURITY
      x = cfg.at(U_CONSTANT_TO_PARAM("URI_REQUEST_STRICT_TRANSPORT_SECURITY_MASK"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::uri_strict_transport_security_mask, 0)

         if (x.findWhiteSpace() != U_NOT_FOUND) x = UStringExt::removeWhiteSpace(x);

#     ifdef USE_LIBSSL
         if (UServer_Base::bssl)
            {
            if (x.equal(U_CONSTANT_TO_PARAM("*"))) UHTTP::uri_strict_transport_security_mask = (UString*)1L;
            else
               {
               U_SRV_LOG("SSL: Sorry, the directive URI_REQUEST_STRICT_TRANSPORT_SECURITY_MASK for ssl server must have the '*' value, instead of %V", x.rep);
               }
            }
         else
#     endif
         U_NEW(UString, UHTTP::uri_strict_transport_security_mask, UString(x));
         }
#  endif

      // INOTIFY

#  if defined(HAVE_SYS_INOTIFY_H) && defined(U_HTTP_INOTIFY_SUPPORT) && !defined(U_SERVER_CAPTIVE_PORTAL)
      if (cfg.readBoolean(U_CONSTANT_TO_PARAM("ENABLE_INOTIFY")))
         {
         UServer_Base::handler_inotify = this;

#     ifdef U_CLASSIC_SUPPORT
         if (UServer_Base::isClassic())
            {
            UServer_Base::handler_inotify = 0;

            U_SRV_LOG("WARNING: Sorry, I can't enable inode based directory notification because PREFORK_CHILD == 1 (server classic mode)");
            }
         else
#     endif
#     if defined(ENABLE_THREAD) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
         if (UNotifier::pthread) // NB: we ask to notify for change of file system (inotify), in the thread approach this is not safe so far...
            {
            UServer_Base::handler_inotify = 0;

            U_SRV_LOG("WARNING: Sorry, I can't enable inode based directory notification because PREFORK_CHILD == -1 (server thread approach)");
            }
#     endif
         }
#  endif

      // URI PROTECTION (for example directory listing)

      U_INTERNAL_DUMP("UHTTP::digest_authentication = %b", UHTTP::digest_authentication)

      x = cfg.at(U_CONSTANT_TO_PARAM("DIGEST_AUTHENTICATION"));

      if (x) UHTTP::digest_authentication = x.strtob();
      else
         {
         U_INTERNAL_DUMP("UServer_Base::bssl = %b", UServer_Base::bssl)

#     ifdef USE_LIBSSL
         if (UServer_Base::bssl == false)
#     endif
         UHTTP::digest_authentication = true;
         }

      U_INTERNAL_DUMP("UHTTP::digest_authentication = %b", UHTTP::digest_authentication)

#  ifdef USE_LIBSSL
      x = cfg.at(U_CONSTANT_TO_PARAM("URI_PROTECTED_MASK"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::uri_protected_mask, 0)

         if (x.findWhiteSpace() != U_NOT_FOUND) x = UStringExt::removeWhiteSpace(x);

         U_NEW(UString, UHTTP::uri_protected_mask, UString(x));
         }

      x = cfg.at(U_CONSTANT_TO_PARAM("URI_PROTECTED_ALLOWED_IP"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::vallow_IP, 0)

         U_NEW(UVector<UIPAllow*>, UHTTP::vallow_IP, UVector<UIPAllow*>);

         if (UIPAllow::parseMask(x, *UHTTP::vallow_IP) == 0)
            {
            delete UHTTP::vallow_IP;
                   UHTTP::vallow_IP = 0;
            }
         }

      x = cfg.at(U_CONSTANT_TO_PARAM("URI_REQUEST_CERT_MASK"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::uri_request_cert_mask, 0)

         if (x.findWhiteSpace() != U_NOT_FOUND) x = UStringExt::removeWhiteSpace(x);

         U_NEW(UString, UHTTP::uri_request_cert_mask, UString(x));
         }
#  endif

#  ifndef U_LOG_DISABLE
      x = cfg.at(U_CONSTANT_TO_PARAM("APACHE_LIKE_LOG"));

      if (x)
         {
         UServer_Base::update_date  =
         UServer_Base::update_date2 = true;

         uint32_t size = cfg.readLong(U_CONSTANT_TO_PARAM("LOG_FILE_SIZE"));

         U_INTERNAL_ASSERT_EQUALS(UServer_Base::apache_like_log, 0)

         U_NEW(ULog, UServer_Base::apache_like_log, ULog(x, size));

#     ifdef USE_LIBZ
         if (size)
            {
            uint32_t log_rotate_size = size + (size / 10) + 12U;

            UServer_Base::apache_like_log->setShared(0, log_rotate_size, (UServer_Base::bssl == false));

            U_SRV_LOG("Mapped %u bytes (%u KB) of shared memory for apache like log", log_rotate_size, log_rotate_size / 1024);
            }
#     endif
         }
#   endif

#  ifdef U_THROTTLING_SUPPORT
      x = cfg.at(U_CONSTANT_TO_PARAM("BANDWIDTH_THROTTLING_MASK"));

      if (x)
         {
#     if defined(ENABLE_THREAD) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
         if (UServer_Base::preforked_num_kids == -1) // NB: in the thread approach this is not safe so far...
            {
            U_SRV_LOG("WARNING: Sorry, I can't enable bandwidth throttling because PREFORK_CHILD == -1 (server thread approach)");
            }
         else
#     endif
         {
         U_NEW(UString, UServer_Base::throttling_mask, UString(x));
         }
         }
#  endif

      x = cfg.at(U_CONSTANT_TO_PARAM("MOUNT_POINT"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::php_mount_point, 0)

         U_NEW(UString, UHTTP::php_mount_point, UString(x));
         }

#  ifdef USE_RUBY
      x = cfg.at(U_CONSTANT_TO_PARAM("RUBY_LIBDIR")); // directory to add to the ruby libdir search path

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::ruby_libdir, 0)

         U_NEW(UString, UHTTP::ruby_libdir, UString(x));
         }
#  endif

#  ifdef USE_PYTHON
      x = cfg.at(U_CONSTANT_TO_PARAM("PY_PROJECT_APP")); // full python name of WSGI entry point expected in form <module>.<app> 

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::py_project_app, 0)

         U_NEW(UString, UHTTP::py_project_app, UString(x));
         }

      x = cfg.at(U_CONSTANT_TO_PARAM("PY_PROJECT_ROOT")); // python module search root; relative to workdir

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::py_project_root, 0)

         U_NEW(UString, UHTTP::py_project_root, UString(x));
         }

      x = cfg.at(U_CONSTANT_TO_PARAM("PY_VIRTUALENV_PATH"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::py_virtualenv_path, 0)

         U_NEW(UString, UHTTP::py_virtualenv_path, UString(x));
         }
#  endif

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

int UHttpPlugIn::handlerInit()
{
   U_TRACE_NO_PARAM(0, "UHttpPlugIn::handlerInit()")

#ifdef USE_LIBSSL
   if (UServer_Base::bssl)
      {
      U_INTERNAL_DUMP("OPENSSL_VERSION_NUMBER = %ld", OPENSSL_VERSION_NUMBER)

#  if OPENSSL_VERSION_NUMBER < 0x10100000L
      if (U_SYSCALL_NO_PARAM(SSLeay) < OPENSSL_VERSION_NUMBER)
         {
         U_ERROR("SSL: this version of mod_http was compiled against a newer library (%s, "
                 "version currently loaded is %s) - may result in undefined or erroneous behavior",
                 OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION));
         }
#  endif

/**
 * #ifdef OPENSSL_FIPS
 * if (U_SYSCALL_NO_PARAM(FIPS_mode) == false)
 *    {
 *    if (U_SYSCALL(FIPS_mode_set, "%d", 1)) U_SRV_LOG("SSL: Operating in SSL FIPS mode");
 *    else                                   U_WARNING("SSL: FIPS mode failed");
 *    }
 * #endif
 */

   // Configure TLS extensions support

#  if !defined(OPENSSL_NO_TLSEXT) && defined(SSL_set_tlsext_host_name)
      if (SSL_CTX_set_tlsext_servername_callback(USSLSocket::sctx, USSLSocket::callback_ServerNameIndication) == false) // Server name indication (SNI)
         {
         U_WARNING("SSL: Unable to initialize TLS servername extension callback");
         }
#  endif

      U_SRV_LOG("SSL: server use configuration model: %s, protocol list: %s",
                  ((USSLSocket*)UServer_Base::socket)->getConfigurationModel(), ((USSLSocket*)UServer_Base::socket)->getProtocolList());

#  if defined(ENABLE_THREAD) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
      U_INTERNAL_ASSERT_EQUALS(USSLSocket::staple.data, 0)

      USSLSocket::staple.data = UServer_Base::getOffsetToDataShare(U_OCSP_MAX_RESPONSE_SIZE);
#  endif
      }
#endif

   UHTTP::init();

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

int UHttpPlugIn::handlerRun() // NB: we use this method instead of handlerInit() because now we have the shared data allocated by UServer...
{
   U_TRACE_NO_PARAM(0, "UHttpPlugIn::handlerRun()")

#ifndef U_HTTP2_DISABLE
   UHTTP2::Connection::preallocate(UNotifier::max_connection);
#endif

#ifdef USE_LIBSSL
   if (UServer_Base::bssl) UHTTP::initSessionSSL();
   else
#endif
   if (UServer_Base::handler_inotify) UHTTP::initDbNotFound();

   if (UServer_Base::vplugin_name->last() == *UString::str_http)
      {
      UServer_Base::update_date  =
      UServer_Base::update_date3 = true;

      UClientImage_Base::iov_vec[1].iov_len  = 6+29+2+12+2+17+2;
      UClientImage_Base::iov_vec[1].iov_base = (caddr_t)ULog::date.date3; // Date: Wed, 20 Jun 2012 11:43:17 GMT\r\nServer: ULib\r\nConnection: close\r\n

#  if defined(U_LINUX) && defined(ENABLE_THREAD) && defined(U_LOG_DISABLE) && !defined(USE_LIBZ)
      U_INTERNAL_ASSERT_POINTER(u_pthread_time)

      UClientImage_Base::iov_vec[1].iov_base = (caddr_t)UServer_Base::ptr_shared_data->log_date_shared.date3;
#  endif

      U_INTERNAL_DUMP("UClientImage_Base::iov_vec[0] = %.*S UClientImage_Base::iov_vec[1] = %.*S",
                       UClientImage_Base::iov_vec[0].iov_len, UClientImage_Base::iov_vec[0].iov_base,
                       UClientImage_Base::iov_vec[1].iov_len, UClientImage_Base::iov_vec[1].iov_base)

      U_MEMCPY(UClientImage_Base::iov_sav, UClientImage_Base::iov_vec, sizeof(struct iovec) * 4);

      // NB: we can shortcut the http request processing...

      UClientImage_Base::callerHandlerRead       = UHTTP::handlerREAD;
      UClientImage_Base::callerHandlerCache      = UHTTP::handlerCache;
      UClientImage_Base::callerIsValidMethod     = UHTTP::isValidMethod;
      UClientImage_Base::callerIsValidRequest    = UHTTP::isValidRequest;
      UClientImage_Base::callerIsValidRequestExt = UHTTP::isValidRequestExt;
      UClientImage_Base::callerHandlerEndRequest = UHTTP::setEndRequestProcessing;

      if (UServer_Base::vplugin_size == 1) UClientImage_Base::callerHandlerRequest = UHTTP::processRequest;
      }

   U_ASSERT(UHTTP::cache_file_check_memory())

   U_SET_MODULE_NAME(usp_init);

#ifndef U_LOG_DISABLE
   if (UServer_Base::apache_like_log) UHTTP::initApacheLikeLog();
#endif

   UHTTP::cache_file->callForAllEntry(UHTTP::callInitForAllUSP);

   UHTTP::bcallInitForAllUSP = true;

   if (UHTTP::upload_dir->empty()) (void) UHTTP::upload_dir->assign(U_CONSTANT_TO_PARAM("uploads"));

   U_RESET_MODULE_NAME;

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

int UHttpPlugIn::handlerFork()
{
   U_TRACE_NO_PARAM(0, "UHttpPlugIn::handlerFork()")

#if defined(HAVE_SYS_INOTIFY_H) && defined(U_HTTP_INOTIFY_SUPPORT) && !defined(U_SERVER_CAPTIVE_PORTAL)
   UHTTP::initInotify();
#endif

   if (UHTTP::bcallInitForAllUSP) UHTTP::cache_file->callForAllEntry(UHTTP::callAfterForkForAllUSP);

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

int UHttpPlugIn::handlerStop()
{
   U_TRACE_NO_PARAM(0, "UHttpPlugIn::handlerStop()")

   U_INTERNAL_ASSERT_POINTER(UHTTP::cache_file)

   U_SET_MODULE_NAME(usp_end);

   UHTTP::cache_file->callForAllEntry(UHTTP::callEndForAllUSP);

   UHTTP::bcallInitForAllUSP = false;

#ifdef USE_PHP
   if (UHTTP::php_embed) UHTTP::php_embed->endPHP();
#endif
#ifdef USE_RUBY
   if (UHTTP::ruby_embed) UHTTP::ruby_embed->endRUBY();
#endif
#ifdef USE_PYTHON
   if (UHTTP::python_embed) UHTTP::python_embed->endPYTHON();
#endif

   U_RESET_MODULE_NAME;

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

// Connection-wide hooks

int UHttpPlugIn::handlerREAD()
{
   U_TRACE_NO_PARAM(0, "UHttpPlugIn::handlerREAD()")

   return UHTTP::handlerREAD();
}

int UHttpPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "UHttpPlugIn::handlerRequest()")

   U_INTERNAL_DUMP("method_type = %C uri = %.*S", U_http_method_type, U_HTTP_URI_TO_TRACE)

   return (UClientImage_Base::isRequestNeedProcessing()
               ? UHTTP::processRequest()
               : U_PLUGIN_HANDLER_FINISHED);
}

// SigHUP hook

int UHttpPlugIn::handlerSigHUP()
{
   U_TRACE_NO_PARAM(0, "UHttpPlugIn::handlerSigHUP()")

#ifdef USE_LIBSSL
   if (UServer_Base::bssl &&
       UHTTP::db_session_ssl->compactionJournal() == false)
      {
      U_WARNING("SSL: compaction of db SSL session failed");

      (void) U_SYSCALL(SSL_CTX_set_session_cache_mode, "%p,%d", USSLSocket::sctx, SSL_SESS_CACHE_OFF);
      }
#endif

   if (UHTTP::bcallInitForAllUSP) UHTTP::cache_file->callForAllEntry(UHTTP::callSigHUPForAllUSP);

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}
