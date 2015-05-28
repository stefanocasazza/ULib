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

U_CREAT_FUNC(server_plugin_http, UHttpPlugIn)

UHttpPlugIn::~UHttpPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, UHttpPlugIn)

   UHTTP::dtor(); // delete global HTTP context...
}

// define method VIRTUAL of class UEventFd

int UHttpPlugIn::handlerRead()
{
   U_TRACE(0, "UHttpPlugIn::handlerRead()")
   
#if defined(HAVE_SYS_INOTIFY_H) && defined(U_HTTP_INOTIFY_SUPPORT)
   U_INTERNAL_ASSERT_POINTER(UHTTP::cache_file)

   UHTTP::in_READ();
#endif

   U_RETURN(U_NOTIFIER_OK);
}

void UHttpPlugIn::handlerDelete()
{
   U_TRACE(0, "UHttpPlugIn::handlerDelete()")

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
   // CACHE_FILE_MASK        mask (DOS regexp) of pathfile that content      be cached in memory (default: "*.css|*.js|*.*html|*.png|*.gif|*.jpg")
   // CACHE_AVOID_MASK       mask (DOS regexp) of pathfile that presence NOT be cached in memory 
   // CACHE_FILE_STORE       pathfile of memory cache stored on filesystem
   //
   // CGI_TIMEOUT            timeout for cgi execution
   // MOUNT_POINT            mount point application (to adjust var SCRIPT_NAME)
   // VIRTUAL_HOST           flag to activate practice of maintaining more than one server on one machine,
   //                        as differentiated by their apparent hostname
   // DIGEST_AUTHENTICATION  flag authentication method (yes = digest, no = basic)
   //
   // ENABLE_CACHING_BY_PROXY_SERVERS enable caching by proxy servers (add "Cache control: public" directive)
   //
   // URI_PROTECTED_MASK       mask (DOS regexp) of URI protected from prying eyes
   // URI_PROTECTED_ALLOWED_IP list of comma separated client address for IP-based access control (IPADDR[/MASK]) for URI_PROTECTED_MASK
   //
   // URI_REQUEST_CERT_MASK                      mask (DOS regexp) of URI where client must comunicate a certificate in the SSL connection
   // URI_REQUEST_STRICT_TRANSPORT_SECURITY_MASK mask (DOS regexp) of URI where use HTTP Strict Transport Security to force client to use only SSL
   //
   // SESSION_COOKIE_OPTION  eventual params for session cookie (lifetime, path, domain, secure, HttpOnly)  
   // ----------------------------------------------------------------------------------------------------------------------------------------------------
   // This directive gives greater control over abnormal client request behavior, which may be useful for avoiding some forms of denial-of-service attacks
   // ----------------------------------------------------------------------------------------------------------------------------------------------------
   // LIMIT_REQUEST_BODY   restricts the total size of the HTTP request body sent from the client
   // REQUEST_READ_TIMEOUT set timeout for receiving requests
   // ------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef U_ALIAS
   uint32_t n;
   UVector<UString> tmp;

   if (cfg.loadVector(tmp, "ALIAS"))
      {
      n = tmp.size();

      if (n)
         {
         if (n == 1) UHTTP::setGlobalAlias(tmp[0]); // automatic alias of all uri request without suffix...
         else
            {
            U_INTERNAL_ASSERT_EQUALS(UHTTP::valias, 0)

            UHTTP::valias = U_NEW(UVector<UString>(n));

            for (uint32_t i = 0; i < n; ++i)
               {
               UStringRep* r = tmp.UVector<UStringRep*>::at(i);

               UHTTP::valias->UVector<UStringRep*>::push(r);
               }
            }

         tmp.clear();
         }
      }

#  ifdef USE_LIBPCRE // REWRITE RULE
   if (cfg.loadVector(tmp, "REWRITE_RULE_NF"))
      {
      n = tmp.size();

      if (n)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::vRewriteRule, 0)

         UHTTP::vRewriteRule = U_NEW(UVector<UHTTP::RewriteRule*>(n));

         for (int32_t i = 0; i < (int32_t)n; i += 2)
            {
            UHTTP::RewriteRule* rule = U_NEW(UHTTP::RewriteRule(tmp[i], tmp[i+1]));

            UHTTP::vRewriteRule->push_back(rule);
            }
         }
      }
#  endif
#endif

   if (cfg.loadTable())
      {
      UString x;

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

         UHTTP::cache_file_mask = U_NEW(UString(x));
         }

      x = cfg.at(U_CONSTANT_TO_PARAM("CACHE_AVOID_MASK"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::cache_avoid_mask, 0)

         if (x.findWhiteSpace() != U_NOT_FOUND) x = UStringExt::removeWhiteSpace(x);

         UHTTP::cache_avoid_mask = U_NEW(UString(x));
         }

#  ifdef U_STDCPP_ENABLE
      x = cfg.at(U_CONSTANT_TO_PARAM("CACHE_FILE_STORE"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::cache_file_store, 0)

         if (x.findWhiteSpace() != U_NOT_FOUND) x = UStringExt::removeWhiteSpace(x);

         UHTTP::cache_file_store = U_NEW(UString(x));
         }
#  endif

      // COOKIE OPTION

      x = cfg.at(U_CONSTANT_TO_PARAM("SESSION_COOKIE_OPTION"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::cgi_cookie_option, 0)

         UHTTP::cgi_cookie_option = U_NEW(UString(x));
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
         UHTTP::uri_strict_transport_security_mask = U_NEW(UString(x));
         }
#  endif

      // MOUNT POINT

      x = cfg.at(U_CONSTANT_TO_PARAM("MOUNT_POINT"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::mount_point, 0)

         UHTTP::mount_point = U_NEW(UString(x));
         }

      // VIRTUAL HOST

      bool bvirtual_host = cfg.readBoolean(U_CONSTANT_TO_PARAM("VIRTUAL_HOST"));

#  ifndef U_ALIAS
      if (bvirtual_host)
         {
         U_SRV_LOG("WARNING: Sorry, I can't enable virtual hosting because alias URI support is missing, please recompile ULib");
         }
#  else
      UHTTP::virtual_host = bvirtual_host;

      x = cfg.at(U_CONSTANT_TO_PARAM("MAINTENANCE_MODE"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::maintenance_mode_page, 0)

         UHTTP::maintenance_mode_page = U_NEW(UString(x));
         }

      UHTTP::setGlobalAlias(cfg.at(U_CONSTANT_TO_PARAM("USP_AUTOMATIC_ALIASING")));
#  endif

      // INOTIFY

#  if defined(HAVE_SYS_INOTIFY_H) && defined(U_HTTP_INOTIFY_SUPPORT)
      if (cfg.readBoolean(U_CONSTANT_TO_PARAM("ENABLE_INOTIFY")))
         {
         // NB: we ask to notify for change of file system (inotify)
         //     in the thread approach this is very dangerous...

#     if defined(ENABLE_THREAD) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
         if (UNotifier::pthread)
            {
            U_SRV_LOG("WARNING: Sorry, I can't enable inode based directory notification because PREFORK_CHILD == -1 (server thread approach)");
            }
         else
#     endif
         UServer_Base::handler_inotify = this;
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

         UHTTP::uri_protected_mask = U_NEW(UString(x));
         }

      x = cfg.at(U_CONSTANT_TO_PARAM("URI_PROTECTED_ALLOWED_IP"));

      if (x)
         {
         U_INTERNAL_ASSERT_EQUALS(UHTTP::vallow_IP, 0)

         UHTTP::vallow_IP = U_NEW(UVector<UIPAllow*>);

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

         UHTTP::uri_request_cert_mask = U_NEW(UString(x));
         }
#  endif

#  ifdef U_LOG_ENABLE
      x = cfg.at(U_CONSTANT_TO_PARAM("APACHE_LIKE_LOG"));

      if (x)
         {
         UServer_Base::update_date2 = true;

         uint32_t size = cfg.readLong(*UString::str_LOG_FILE_SZ);

         U_INTERNAL_ASSERT_EQUALS(UServer_Base::apache_like_log, 0)

         UServer_Base::apache_like_log = U_NEW(ULog(x, size));

#     ifdef USE_LIBZ
         if (size)
            {
            uint32_t log_rotate_size = size + (size / 10) + 12U;

            UServer_Base::apache_like_log->setShared(0, log_rotate_size, (UServer_Base::bssl == false));

            U_SRV_LOG("Mapped %u bytes (%u KB) of shared memory for apache like log", log_rotate_size, log_rotate_size / 1024);
            }
#     endif
         }
#  endif

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

int UHttpPlugIn::handlerInit()
{
   U_TRACE(0, "UHttpPlugIn::handlerInit()")

   u_init_http_method_list();

#ifdef USE_LIBSSL
   if (UServer_Base::bssl)
      {
      U_INTERNAL_DUMP("OPENSSL_VERSION_NUMBER = %ld", OPENSSL_VERSION_NUMBER)

      if (U_SYSCALL_NO_PARAM(SSLeay) < OPENSSL_VERSION_NUMBER)
         {
         U_ERROR("SSL: this version of mod_http was compiled against a newer library (%s, "
                 "version currently loaded is %s) - may result in undefined or erroneous behavior", OPENSSL_VERSION_TEXT, SSLeay_version(SSLEAY_VERSION));
         }

/*
#  ifdef OPENSSL_FIPS
      if (U_SYSCALL_NO_PARAM(FIPS_mode) == false)
         {
         if (U_SYSCALL(FIPS_mode_set, "%d", 1))
            {
            U_SRV_LOG("SSL: Operating in SSL FIPS mode");
            }
         else
            {
            U_WARNING("SSL: FIPS mode failed");
            }
         }
#  endif
*/

#  if !defined(OPENSSL_NO_TLSEXT) && defined(SSL_set_tlsext_host_name)

      // Configure TLS extensions support

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

   UHTTP::ctor(); // init HTTP context

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

int UHttpPlugIn::handlerRun() // NB: we use this method because now we have the shared data allocated by UServer...
{
   U_TRACE(0, "UHttpPlugIn::handlerRun()")

#ifdef USE_LIBSSL
   if (UServer_Base::bssl) UHTTP::initSessionSSL();
   else
#endif
   if (UServer_Base::handler_inotify) UHTTP::initDbNotFound();

   if (UServer_Base::vplugin_name->last() == *UString::str_http)
      {
      UServer_Base::update_date3 = true;

      UClientImage_Base::iov_vec[1].iov_base = (caddr_t) UServer_Base::ptr_static_date->date3; // Date: Wed, 20 Jun 2012 11:43:17 GMT\r\nServer: ULib\r\n...
      UClientImage_Base::iov_vec[1].iov_len  = 6+29+2+12+2+17+2;

      U_INTERNAL_DUMP("UClientImage_Base::iov_vec[0] = %.*S UClientImage_Base::iov_vec[1] = %.*S",
                       UClientImage_Base::iov_vec[0].iov_len, UClientImage_Base::iov_vec[0].iov_base,
                       UClientImage_Base::iov_vec[1].iov_len, UClientImage_Base::iov_vec[1].iov_base)

      u__memcpy(UClientImage_Base::iov_sav, UClientImage_Base::iov_vec, sizeof(struct iovec) * 4, __PRETTY_FUNCTION__);

      // NB: we can shortcut the http request processing...

      UClientImage_Base::callerHandlerRead        = UHTTP::handlerREAD;
      UClientImage_Base::callerHandlerCache       = UHTTP::handlerCache;
      UClientImage_Base::callerIsValidRequest     = UHTTP::isValidRequest;
      UClientImage_Base::callerIsValidRequestExt  = UHTTP::isValidRequestExt;
      UClientImage_Base::callerHandlerEndRequest  = UHTTP::setEndRequestProcessing;
      UClientImage_Base::callerHandlerDataPending = UHTTP::handlerDataPending;
      }

   U_ASSERT(UHTTP::cache_file_check_memory())

   U_SET_MODULE_NAME(usp_init);

#ifdef U_LOG_ENABLE
   if (UServer_Base::apache_like_log) UHTTP::initApacheLikeLog();
#endif

   UHTTP::cache_file->callForAllEntry(UHTTP::callInitForAllUSP);

   UHTTP::bcallInitForAllUSP = true;

   U_RESET_MODULE_NAME;

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

int UHttpPlugIn::handlerFork()
{
   U_TRACE(0, "UHttpPlugIn::handlerFork()")

   if (UHTTP::bcallInitForAllUSP) UHTTP::cache_file->callForAllEntry(UHTTP::callAfterForkForAllUSP);

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

int UHttpPlugIn::handlerStop()
{
   U_TRACE(0, "UHttpPlugIn::handlerStop()")

   U_INTERNAL_ASSERT_POINTER(UHTTP::cache_file)

   U_SET_MODULE_NAME(usp_end);

   UHTTP::cache_file->callForAllEntry(UHTTP::callEndForAllUSP);

   UHTTP::bcallInitForAllUSP = false;

#ifdef USE_PHP
   UHTTP::php_embed->php_end();
#endif
#ifdef USE_RUBY
   UHTTP::ruby_embed->ruby_end();
#endif

   U_RESET_MODULE_NAME;

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

// Connection-wide hooks

int UHttpPlugIn::handlerREAD()
{
   U_TRACE(0, "UHttpPlugIn::handlerREAD()")

   return UHTTP::handlerREAD();
}

int UHttpPlugIn::handlerRequest()
{
   U_TRACE(0, "UHttpPlugIn::handlerRequest()")

   U_INTERNAL_DUMP("method_type = %C uri = %.*S", U_http_method_type, U_HTTP_URI_TO_TRACE)

   return (UClientImage_Base::isRequestNeedProcessing()
               ? UHTTP::processRequest()
               : U_PLUGIN_HANDLER_FINISHED);
}

// SigHUP hook

int UHttpPlugIn::handlerSigHUP()
{
   U_TRACE(0, "UHttpPlugIn::handlerSigHUP()")

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
