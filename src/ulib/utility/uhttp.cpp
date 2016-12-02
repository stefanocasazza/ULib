// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    uhttp.cpp - HTTP utility
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/base/coder/xml.h>

#include <ulib/url.h>
#include <ulib/date.h>
#include <ulib/db/rdb.h>
#include <ulib/tokenizer.h>
#include <ulib/mime/entity.h>
#include <ulib/utility/uhttp.h>
#include <ulib/mime/multipart.h>
#include <ulib/utility/escape.h>
#include <ulib/utility/base64.h>
#include <ulib/base/coder/url.h>
#include <ulib/utility/dir_walk.h>
#include <ulib/utility/websocket.h>
#include <ulib/utility/socket_ext.h>
#include <ulib/net/server/plugin/mod_proxy.h>
#include <ulib/net/server/plugin/mod_proxy_service.h>

#ifndef U_HTTP2_DISABLE
#  include <ulib/utility/http2.h>
#endif
#ifdef USE_LIBZ // check for crc32
#  include <ulib/base/coder/gzio.h>
#  include <ulib/utility/interrupt.h>
#  include <ulib/utility/string_ext.h>
#endif
#ifdef USE_LIBSSL
#  include <ulib/ssl/net/ssl_session.h>
#endif
#ifdef USE_LIBMAGIC
#  include <ulib/magic/magic.h>
#endif
#ifdef HAVE_LIBTCC
#  include <libtcc.h>
#endif
#ifndef _MSWINDOWS_
#  include <sys/resource.h>
#endif

#ifdef U_HTTP_INOTIFY_SUPPORT
#  ifdef SYS_INOTIFY_H_EXISTS_AND_WORKS
#     include <sys/inotify.h>
#  elif defined(U_LINUX)
#     ifdef HAVE_SYS_INOTIFY_H
#     undef HAVE_SYS_INOTIFY_H
#     endif
#     include <ulib/replace/inotify-nosys.h>
#  endif
#endif

#define U_FLV_HEAD        "FLV\x1\x1\0\0\0\x9\0\0\0\x9"
#define U_TIME_FOR_EXPIRE (u_now->tv_sec + (365 * U_ONE_DAY_IN_SECOND))

int         UHTTP::mime_index;
int         UHTTP::cgi_timeout;
bool        UHTTP::bcallInitForAllUSP;
bool        UHTTP::digest_authentication;
bool        UHTTP::enable_caching_by_proxy_servers;
char        UHTTP::response_buffer[64];
URDB*       UHTTP::db_not_found;
UFile*      UHTTP::file;
UString*    UHTTP::ext;
UString*    UHTTP::etag;
UString*    UHTTP::geoip;
UString*    UHTTP::suffix;
UString*    UHTTP::tmpdir;
UString*    UHTTP::request;
UString*    UHTTP::htpasswd;
UString*    UHTTP::htdigest;
UString*    UHTTP::qcontent;
UString*    UHTTP::pathname;
UString*    UHTTP::rpathname;
UString*    UHTTP::set_cookie;
UString*    UHTTP::upload_dir;
UString*    UHTTP::fcgi_uri_mask;
UString*    UHTTP::scgi_uri_mask;
UString*    UHTTP::cache_file_mask;
UString*    UHTTP::nocache_file_mask;
UString*    UHTTP::cache_avoid_mask;
UString*    UHTTP::cache_file_store;
UString*    UHTTP::cgi_cookie_option;
UString*    UHTTP::set_cookie_option;
UString*    UHTTP::string_HTTP_Variables;
uint32_t    UHTTP::range_size;
uint32_t    UHTTP::range_start;
uint32_t    UHTTP::old_path_len;
uint32_t    UHTTP::response_code;
uint32_t    UHTTP::sid_counter_gen;
uint32_t    UHTTP::sid_counter_cur;
uint32_t    UHTTP::usp_page_key_len;
uint32_t    UHTTP::limit_request_body = U_STRING_MAX_SIZE;
uint32_t    UHTTP::request_read_timeout;
const char* UHTTP::usp_page_key;

UCommand*                         UHTTP::pcmd;
UDataSession*                     UHTTP::data_session;
UDataSession*                     UHTTP::data_storage;
UMimeMultipart*                   UHTTP::formMulti;
UModProxyService*                 UHTTP::service;
UVector<UString>*                 UHTTP::vmsg_error;
UVector<UString>*                 UHTTP::form_name_value;
UHTTP::UServletPage*              UHTTP::usp_page_ptr;
UVector<UModProxyService*>*       UHTTP::vservice;
URDBObjectHandler<UDataStorage*>* UHTTP::db_session;

         UHTTP::UFileCacheData*   UHTTP::file_data;
         UHTTP::UFileCacheData*   UHTTP::file_not_in_cache_data;
UHashMap<UHTTP::UFileCacheData*>* UHTTP::cache_file;

#ifdef USE_PHP
UHTTP::UPHP* UHTTP::php_embed;
#endif
uint32_t     UHTTP::npathinfo;
UString*     UHTTP::php_mount_point;
#ifdef USE_RUBY
bool          UHTTP::ruby_on_rails;
UString*      UHTTP::ruby_libdir;
UHTTP::URUBY* UHTTP::ruby_embed;
#endif
#ifdef USE_PYTHON
UString*        UHTTP::py_project_app;  // full python name of WSGI entry point expected in form <module>.<app>
UString*        UHTTP::py_project_root; // python module search root; relative to workdir
UString*        UHTTP::py_virtualenv_path;
UHTTP::UPYTHON* UHTTP::python_embed;
#endif
#ifdef USE_PAGE_SPEED
UHTTP::UPageSpeed* UHTTP::page_speed;
#endif
#ifdef USE_LIBV8
UHTTP::UV8JavaScript* UHTTP::v8_javascript;
#endif
#ifdef U_ALIAS
bool              UHTTP::virtual_host;
UString*          UHTTP::alias;
UString*          UHTTP::global_alias;
UString*          UHTTP::maintenance_mode_page;
UVector<UString>* UHTTP::valias;
#endif
#ifdef USE_LIBSSL
UString*                          UHTTP::uri_protected_mask;
UString*                          UHTTP::uri_request_cert_mask;
USSLSession*                      UHTTP::data_session_ssl;
UVector<UIPAllow*>*               UHTTP::vallow_IP;
URDBObjectHandler<UDataStorage*>* UHTTP::db_session_ssl;
#endif
#ifdef U_HTTP_STRICT_TRANSPORT_SECURITY
UString* UHTTP::uri_strict_transport_security_mask;
#endif
#ifndef U_LOG_DISABLE
char         UHTTP::iov_buffer[20];
struct iovec UHTTP::iov_vec[10];
#  if !defined(U_CACHE_REQUEST_DISABLE) || defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST) 
uint32_t     UHTTP::agent_offset;
uint32_t     UHTTP::request_offset;
uint32_t     UHTTP::referer_offset;
#  endif
#endif

uint32_t UHTTP::num_item_tot;
uint32_t UHTTP::num_page_cur;
uint32_t UHTTP::num_page_end;    // modified by getLinkPagination()...
uint32_t UHTTP::num_page_start;  // modified by getLinkPagination()...
uint32_t UHTTP::num_item_for_page;

UString UHTTP::getLinkPagination()
{
   U_TRACE_NO_PARAM(0, "UHTTP::getLinkPagination()")

   U_INTERNAL_DUMP("num_page_cur = %u num_item_tot = %u num_page_start = %u num_page_end = %u num_item_for_page = %u",
                    num_page_cur,     num_item_tot,     num_page_start,     num_page_end,     num_item_for_page)

   UString link(U_CAPACITY);

#ifdef U_HTML_PAGINATION_SUPPORT
   if (num_item_for_page)
      {
      if (num_item_tot <= num_item_for_page)
         {
         num_page_end   =  num_item_tot;
         num_page_start = (num_item_tot > 0);

         (void) link.assign(U_CONSTANT_TO_PARAM("<span class=\"void\">PREV</span><span class=\"void\">NEXT</span>"));
         }
      else
         {
         uint32_t pagina_precedente =  num_page_cur - 1,
                  pagina_successiva =  num_page_cur + 1,
                  i, tot_pagine     = (num_item_tot / num_item_for_page);

         if ((num_item_tot % num_item_for_page) != 0) ++tot_pagine;

         uint32_t    ultima_pagina =    tot_pagine - 1,
                  penultima_pagina = ultima_pagina - 1;

         // link to previous page

         if (num_page_cur == 1)
            {
            num_page_start = 1;

            (void) link.assign(U_CONSTANT_TO_PARAM("<span class=\"void\">PREV</span> "));
            }
         else
            {
            num_page_start = 1 + (pagina_precedente * num_item_for_page);

            link.snprintf(U_CONSTANT_TO_PARAM("<a href=\"?page=%u\" class=\"pnum\">PREV</a> "), pagina_precedente);
            }

         // we always show the link to the first page

         addLinkPagination(link,1);

         // if the next link is to the second page, add the dots ... or only the missing page

         if (pagina_precedente > 2)
            {
            if (pagina_precedente == 3) addLinkPagination(link,2);
            else                 (void) link.append(U_CONSTANT_TO_PARAM(" ... "));
            }

         // we create links to the current page and those close to it

         for (i = pagina_precedente; (int32_t)i < (int32_t)(pagina_successiva+1); ++i) // NB: we need this (gcc: cannot optimize possibly infinite loops)
            {
            // check if there is among those near the first page (already reported)

            if (i < 2) continue;

            // check if there is among those near the last page (which will show with the next instruction)

            if (i > ultima_pagina) continue;

            addLinkPagination(link,i);
            }

         // if the previous link was not on the penultimate page, add the dots ... or only the missing page

         if (pagina_successiva < ultima_pagina)
            {
            if (pagina_successiva == penultima_pagina) addLinkPagination(link,ultima_pagina);
            else                                (void) link.append(U_CONSTANT_TO_PARAM(" ... "));
            }

         // show the link to the last page if it does not coincide with the first

         if (tot_pagine != 1) addLinkPagination(link,tot_pagine);

         // link to next page

         if (num_page_cur == tot_pagine)
            {
            num_page_end = num_item_tot;

            (void) link.append(U_CONSTANT_TO_PARAM("<span class=\"void\">NEXT</span>"));
            }
         else
            {
            num_page_end = num_page_start + num_item_for_page - 1;

            link.snprintf_add(U_CONSTANT_TO_PARAM("<a href=\"?page=%u\" class=\"pnum\">NEXT</a>"), pagina_successiva);
            }
         }

      U_INTERNAL_DUMP("num_page_cur = %u num_item_tot = %u num_page_start = %u num_page_end = %u num_item_for_page = %u",
                       num_page_cur,     num_item_tot,     num_page_start,     num_page_end,     num_item_for_page)

      (void) link.shrink();
      }
#endif

   U_RETURN_STRING(link);
}

// CACHE FILE SYSTEM

UHTTP::UFileCacheData::UFileCacheData()
{
   U_TRACE_REGISTER_OBJECT(0, UFileCacheData, "")

   ptr = array = 0;
   size        = 0;
   mode        = 0;
   mtime       = 0;
   link        = false;
   expire      = U_TIME_FOR_EXPIRE;
   mime_index  = U_unknow;
   wd = fd     = -1;
}

UHTTP::UFileCacheData::UFileCacheData(const UHTTP::UFileCacheData& elem)
{
   U_TRACE_REGISTER_OBJECT(0, UFileCacheData, "%p", &elem)

   fd         = wd = -1;
   mode       = 0;
   ptr        = elem.ptr;        // data
   link       = elem.link;       // true => ptr point to another entry
   array      = elem.array;      // content, header, gzip(content, header)
   size       = elem.size;       // size content
   mtime      = elem.mtime;      // time of last modification
   mime_index = elem.mime_index; // index file mime type

   // check expire time of the entry

   expire = (u_now->tv_sec < elem.expire ? elem.expire : U_TIME_FOR_EXPIRE);
}

UHTTP::UFileCacheData::~UFileCacheData()
{
   U_TRACE_UNREGISTER_OBJECT(0, UFileCacheData)

   if (ptr &&
       link == false)
      {
           if (mime_index == U_usp) delete (UServletPage*)ptr;
#  ifdef HAVE_LIBTCC
      else if (mime_index == U_csp) delete (UCServletPage*)ptr;
#  endif
      else if (mime_index == U_cgi)
         {
         U_FREE_TYPE(ptr, UHTTP::ucgi);
         }
      else delete (UString*)ptr;
      }

   if (array) delete array;

#if defined(HAVE_SYS_INOTIFY_H) && defined(U_HTTP_INOTIFY_SUPPORT) && !defined(U_SERVER_CAPTIVE_PORTAL)
   if (wd != -1                      &&
       UServer_Base::handler_inotify &&
       UServer_Base::isChild() == false)
      {
      U_INTERNAL_ASSERT_POINTER(UServer_Base::handler_inotify)
      U_INTERNAL_ASSERT_DIFFERS(UServer_Base::handler_inotify->fd, -1)

      (void) inotify_rm_watch(UServer_Base::handler_inotify->fd, wd);
      }
#endif
}

// INOTIFY FOR CACHE FILE SYSTEM

#if defined(HAVE_SYS_INOTIFY_H) && defined(U_HTTP_INOTIFY_SUPPORT) && !defined(U_SERVER_CAPTIVE_PORTAL)
int                    UHTTP::inotify_wd;
char*                  UHTTP::inotify_name;
uint32_t               UHTTP::inotify_len;
UString*               UHTTP::inotify_pathname;
UStringRep*            UHTTP::inotify_dir;
UHTTP::UFileCacheData* UHTTP::inotify_file_data;

U_NO_EXPORT void UHTTP::setInotifyPathname()
{
   U_TRACE_NO_PARAM(0, "UHTTP::setInotifyPathname()")

   inotify_pathname->setBuffer(inotify_dir->size() + 1 + inotify_len);

   inotify_pathname->snprintf(U_CONSTANT_TO_PARAM("%v/%.*s"), inotify_dir, inotify_len, inotify_name);

   pathname->setBuffer(inotify_pathname->size());

   pathname->snprintf(U_CONSTANT_TO_PARAM("%v"), inotify_pathname->rep);

   file_data = inotify_file_data = cache_file->at(*pathname);
}

U_NO_EXPORT bool UHTTP::getInotifyPathDirectory(UStringRep* key, void* value)
{
   U_TRACE(0+256, "UHTTP::getInotifyPathDirectory(%V,%p)", key, value)

   U_INTERNAL_ASSERT_POINTER(value)

   if (((UHTTP::UFileCacheData*)value)->wd == inotify_wd)
      {
      inotify_dir = key;

      setInotifyPathname();

      U_RETURN(false);
      }

   U_RETURN(true);
}

U_NO_EXPORT bool UHTTP::checkForInotifyDirectory(UStringRep* key, void* value)
{
   U_TRACE(0, "UHTTP::checkForInotifyDirectory(%V,%p)", key, value)

   U_INTERNAL_ASSERT_POINTER(value)

   UHTTP::UFileCacheData* cptr = (UHTTP::UFileCacheData*)value;

   if (S_ISDIR(cptr->mode) &&
       cptr->link == false)
      {
      U_INTERNAL_ASSERT_EQUALS(cptr->ptr, 0)
      U_INTERNAL_ASSERT_EQUALS(cptr->array, 0)
      U_INTERNAL_ASSERT(key->isNullTerminated())

      cptr->wd = U_SYSCALL(inotify_add_watch, "%d,%s,%u", UServer_Base::handler_inotify->fd, key->data(), IN_ONLYDIR | IN_CREATE | IN_DELETE | IN_MODIFY);
      }

   U_RETURN(true);
}

void UHTTP::initInotify()
{
   U_TRACE_NO_PARAM(0, "UHTTP::initInotify()")

   if (UServer_Base::handler_inotify)
      {
      // INIT INOTIFY FOR DOCUMENT ROOT CACHE

      U_INTERNAL_ASSERT_POINTER(cache_file)
      U_INTERNAL_ASSERT_EQUALS(UServer_Base::handler_inotify->fd, -1)

#  ifdef HAVE_INOTIFY_INIT1
      UServer_Base::handler_inotify->fd = U_SYSCALL(inotify_init1, "%d", IN_NONBLOCK | IN_CLOEXEC);

      if (UServer_Base::handler_inotify->fd != -1 || errno != ENOSYS) goto next;
#  endif

      UServer_Base::handler_inotify->fd = U_SYSCALL_NO_PARAM(inotify_init);

      (void) U_SYSCALL(fcntl, "%d,%d,%d", UServer_Base::handler_inotify->fd, F_SETFL, O_NONBLOCK | O_CLOEXEC);
next:
      if (UServer_Base::handler_inotify->fd == -1)
         {
         UServer_Base::handler_inotify = 0;

         U_SRV_LOG("WARNING: inode based directory notification failed");

         return;
         }

      U_SRV_LOG("Inode based directory notification enabled");

      U_NEW(UString, inotify_pathname, UString(U_CAPACITY));

      cache_file->callForAllEntry(checkForInotifyDirectory);
      }
}

#define U_IN_BUFLEN (1024 * (sizeof(struct inotify_event) + 16))

void UHTTP::in_READ()
{
   U_TRACE_NO_PARAM(1+256, "UHTTP::in_READ()")

   U_INTERNAL_ASSERT_POINTER(cache_file)
   U_INTERNAL_ASSERT_POINTER(UServer_Base::handler_inotify)

   char buffer[U_IN_BUFLEN];
   int length = U_SYSCALL(read, "%d,%p,%u", UServer_Base::handler_inotify->fd, buffer, U_IN_BUFLEN);  

   if (length <= 0)
      {
#  ifdef U_EPOLLET_POSTPONE_STRATEGY
      if (errno == EAGAIN) U_ClientImage_state = U_PLUGIN_HANDLER_AGAIN;
#  endif
      }
   else
      {
      /**
       * struct inotify_event {
       *    int wd;           // The watch descriptor
       *    uint32_t mask;    // Watch mask
       *    uint32_t cookie;  // A cookie to tie two events together
       *    uint32_t len;     // The length of the filename found in the name field
       *    char name[];      // The name of the file, padding to the end with NULs
       * }
       */

      union uuinotify_event {
         char*  p;
         struct inotify_event* ip;
      };

      char* name;
      int wd, i = 0;  
      uint32_t len, mask;
      bool binotify_path; 
      union uuinotify_event event;

      while (i < length)
         {
         event.p = buffer + i;

         i += sizeof(struct inotify_event);

         if (event.ip->len)
            {
            wd   = event.ip->wd;
            mask = event.ip->mask;
            len  = event.ip->len;
            name = event.ip->name;

            // NB: The length contains any potential padding that is, the result of strlen() on the name field may be smaller than len...

            while (name[len-1] == '\0') --len;

            U_DEBUG("INOTIFY: %s %.*S was %s", (mask & IN_ISDIR  ? "DIRECTORY" : "FILE"), len, name,
                                               (mask & IN_CREATE ? "created"   :
                                                mask & IN_DELETE ? "deleted"   :
                                                mask & IN_MODIFY ? "modified"  : "???"))

            U_INTERNAL_ASSERT_EQUALS(len, u__strlen(name, __PRETTY_FUNCTION__))

            file_data = 0;

            binotify_path = false;

            if (wd == inotify_wd)
               {
               if (inotify_file_data  &&
                   len == inotify_len &&
                   memcmp(name, inotify_name, len) == 0)
                  {
                  binotify_path = true;
                  }
               else if (inotify_dir)
                  {
                  inotify_len  = len;
                  inotify_name = name;

                  setInotifyPathname();

                  binotify_path = true;
                  }
               }

            if (binotify_path == false)
               {
               inotify_wd        = wd;
               inotify_len       = len;
               inotify_name      = name;
               inotify_dir       = 0;
               inotify_file_data = 0;
               }

            if (*inotify_name != '.' ||
                u_isSuffixSwap(inotify_name + len - U_CONSTANT_SIZE(".swp")) == false) // NB: vi tmp...
               {
               if (binotify_path == false) cache_file->callForAllEntry(getInotifyPathDirectory);

               if ((mask & IN_CREATE) != 0)
                  {
                  if (inotify_file_data == 0)
                     {
                     (void) pathname->replace(*inotify_pathname);

                     checkFileForCache();
                     }
                  }
               else
                  {
                  if ((mask & IN_DELETE) != 0)
                     {
                     if (inotify_file_data)
                        {
                        if (file_data == 0) file_data = cache_file->at(*inotify_pathname);

                        if (file_data)
                           {
                           U_INTERNAL_ASSERT_EQUALS(file_data, inotify_file_data)

                           cache_file->eraseAfterFind();
                           }

                        inotify_file_data = 0;
                        }
                     }
                  else if ((mask & IN_MODIFY) != 0)
                     {
                     if (inotify_file_data)
                        {
                        // NB: check if we have the content of file in cache...

                        if (inotify_file_data->array) inotify_file_data->expire = 0; // NB: we delay the renew...
                        else
                           {
                           if (file_data == 0) file_data = cache_file->at(*inotify_pathname);

                           if (file_data)
                              {
                              U_INTERNAL_ASSERT_EQUALS(file_data, inotify_file_data)

                              renewFileDataInCache();
                              }
                           }
                        }
                     }
                  }
               }

            i += event.ip->len;
            }
         }
      }

   file_data = 0;
}
#endif

// CSP (C Servlet Page)

#ifdef HAVE_LIBTCC
static char*        get_reply(void)                    { return UClientImage_Base::wbuffer->data(); }
static unsigned int get_reply_capacity(void)           { return UClientImage_Base::wbuffer->capacity(); }
static void         set_reply_capacity(unsigned int n) {        UClientImage_Base::wbuffer->setBuffer(n); }
#  ifdef USE_LIBV8
static char* runv8(const char* jssrc) // compiles and executes javascript and returns the script return value as string
   {
   if (UHTTP::v8_javascript)
      {
      UString x(jssrc);

      UHTTP::v8_javascript->runv8(x);

      return x.c_strdup();
      }

   return 0;
   }
#  endif
#endif

bool UHTTP::UCServletPage::compile(const UString& program)
{
   U_TRACE(1, "UCServletPage::compile(%V)", program.rep)

   bool result = false;
#ifdef HAVE_LIBTCC
   TCCState* s = (TCCState*) U_SYSCALL_NO_PARAM(tcc_new);

   (void) U_SYSCALL(tcc_set_output_type, "%p,%d", s, TCC_OUTPUT_MEMORY);

   const char* ptr = program.c_str();

   if (U_SYSCALL(tcc_compile_string, "%p,%S", s, ptr) != -1)
      {
      /* we add a symbol that the compiled program can use */

      (void) U_SYSCALL(tcc_add_symbol, "%p,%S,%p", s, "get_reply",          (void*)get_reply);
      (void) U_SYSCALL(tcc_add_symbol, "%p,%S,%p", s, "get_reply_capacity", (void*)get_reply_capacity);
      (void) U_SYSCALL(tcc_add_symbol, "%p,%S,%p", s, "set_reply_capacity", (void*)set_reply_capacity);
#  ifdef USE_LIBV8
      (void) U_SYSCALL(tcc_add_symbol, "%p,%S,%p", s, "runv8",              (void*)runv8);
#  endif

      /* define preprocessor symbol 'sym'. Can put optional value */

      U_SYSCALL_VOID(tcc_define_symbol, "%p,%S,%S", s, "HAVE_CONFIG_H", 0);

      /* You may also open a dll with tcc_add_file() and use symbols from that */

#  ifdef HAVE_ARCH64
#     ifdef DEBUG
#        define U_LIB_SO U_PREFIXDIR "/lib64/libulib_g.so" 
#     else
#        define U_LIB_SO U_PREFIXDIR "/lib64/libulib.so" 
#     endif
#  else
#     ifdef DEBUG
#        define U_LIB_SO U_PREFIXDIR "/lib/libulib_g.so" 
#     else
#        define U_LIB_SO U_PREFIXDIR "/lib/libulib.so" 
#     endif
#  endif

      (void) U_SYSCALL(tcc_add_file, "%p,%S,%d", s, U_LIB_SO);

      int rc;
      UString token;
      uint32_t pos = 0;
      UTokenizer t(program);
      char buffer[U_PATH_MAX];

      while ((pos = U_STRING_FIND(program, pos, "#pragma ")) != U_NOT_FOUND)
         {
         pos += U_CONSTANT_SIZE("#pragma ");

         t.setDistance(pos);

         if (t.next(token, (bool*)0) == false) break;

         if (token.equal(U_CONSTANT_TO_PARAM("link")))
            {
            if (t.next(token, (bool*)0) == false) break;

            pos += U_CONSTANT_SIZE("link ");

            if (token.first_char() != '/')
               {
               (void) u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("../libraries/%v"), token.rep);

               if (UFile::access(buffer, R_OK))
                  {
                  rc = U_SYSCALL(tcc_add_file, "%p,%S,%d", s, buffer);

                  if (rc != -1) continue;
                  }
               }

            (void) U_SYSCALL(tcc_add_file, "%p,%S,%d", s, token.c_str());
            }
         else if (token.equal(U_CONSTANT_TO_PARAM("include")))
            {
            if (t.next(token, (bool*)0) == false) break;

            pos += U_CONSTANT_SIZE("include ");

            if (token.first_char() != '/')
               {
               (void) u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("../include/%v"), token.rep);

               if (UFile::access(buffer, R_OK | X_OK))
                  {
                  rc = U_SYSCALL(tcc_add_include_path, "%p,%S,%d", s, buffer);

                  if (rc != -1) continue;
                  }
               }

            (void) U_SYSCALL(tcc_add_include_path, "%p,%S,%d", s, token.c_str());
            }
         }

      size = U_SYSCALL(tcc_relocate, "%p,%p", s, 0);

      if (size > 0)
         {
         relocated = UMemoryPool::_malloc((uint32_t*)&size);

         (void) U_SYSCALL(tcc_relocate, "%p,%p", s, relocated);

         prog_main = (iPFipvc) U_SYSCALL(tcc_get_symbol, "%p,%S", s, "main");

         if (prog_main) result = true;
         }
      }

   U_SYSCALL_VOID(tcc_delete, "%p", s);
#endif

   U_RETURN(result);
}

#ifdef U_STATIC_ONLY
extern "C" void __cxa_pure_virtual();
extern "C" void __cxa_pure_virtual() { U_ERROR("__cxa_pure_virtual"); }

#  ifdef U_STATIC_SERVLET_WI_AUTH
//#   define WI_AUTH_DOMAIN "auth.t-unwired.com"
#     define WI_AUTH_DOMAIN "wifi-aaa.comune.fi.it"
#  endif
#  include "../net/server/plugin/usp/loader.autoconf.ext" // NB: the extern "C" must be in a place NOT inside a class definition or implementation...

U_NO_EXPORT void UHTTP::loadStaticLinkedServlet(const char* name, uint32_t len, vPFi runDynamicPage)
{
   U_TRACE(0, "UHTTP::loadStaticLinkedServlet(%.*S,%u,%p)", len, name, len, runDynamicPage)

   U_NEW(UHTTP::UFileCacheData, file_data, UHTTP::UFileCacheData);
   U_NEW(UHTTP::UServletPage,   file_data->ptr, UHTTP::UServletPage);

                   file_data->mime_index           = U_usp;
   ((UServletPage*)file_data->ptr)->runDynamicPage = runDynamicPage;

   (void) pathname->replace(name, len);

# if defined(U_STATIC_SERVLET_WI_AUTH)
   if (pathname->equal(U_CONSTANT_TO_PARAM("wi_auth"))) (void) pathname->insert(0, U_CONSTANT_TO_PARAM(WI_AUTH_DOMAIN "/servlet/"));
# endif

   cache_file->insert(*pathname, file_data); // NB: we don't need to call u_construct<UHTTP::UFileCacheData>()...

   U_SRV_LOG("linked static servlet: %.*S, USP service registered (URI): %V", len, name, pathname->rep);
}
#endif

void UHTTP::init()
{
   U_TRACE_NO_PARAM(1, "UHTTP::init()")

   UString::str_allocate(STR_ALLOCATE_HTTP);

   U_INTERNAL_ASSERT_EQUALS(ext, 0)
   U_INTERNAL_ASSERT_EQUALS(etag, 0)
   U_INTERNAL_ASSERT_EQUALS(file, 0)
   U_INTERNAL_ASSERT_EQUALS(pcmd, 0)
   U_INTERNAL_ASSERT_EQUALS(geoip, 0)
   U_INTERNAL_ASSERT_EQUALS(tmpdir, 0)
   U_INTERNAL_ASSERT_EQUALS(request, 0)
   U_INTERNAL_ASSERT_EQUALS(qcontent, 0)
   U_INTERNAL_ASSERT_EQUALS(pathname, 0)
   U_INTERNAL_ASSERT_EQUALS(rpathname, 0)
   U_INTERNAL_ASSERT_EQUALS(formMulti, 0)
   U_INTERNAL_ASSERT_EQUALS(upload_dir, 0)
   U_INTERNAL_ASSERT_EQUALS(set_cookie, 0)
   U_INTERNAL_ASSERT_EQUALS(form_name_value, 0)
   U_INTERNAL_ASSERT_EQUALS(set_cookie_option, 0)
   U_INTERNAL_ASSERT_EQUALS(string_HTTP_Variables, 0)

   U_NEW(UFile, file, UFile);
   U_NEW(UCommand, pcmd, UCommand);
   U_NEW(UMimeMultipart, formMulti, UMimeMultipart);
   U_NEW(UVector<UString>, form_name_value, UVector<UString>);

   U_NEW(UString, ext, UString);
   U_NEW(UString, etag, UString);
   U_NEW(UString, geoip, UString(U_CAPACITY));
   U_NEW(UString, suffix, UString);
   U_NEW(UString, tmpdir, UString(U_PATH_MAX));
   U_NEW(UString, request, UString);
   U_NEW(UString, qcontent, UString);
   U_NEW(UString, pathname, UString(U_CAPACITY));
   U_NEW(UString, rpathname, UString);
   U_NEW(UString, upload_dir, UString);
   U_NEW(UString, set_cookie, UString);
   U_NEW(UString, set_cookie_option, UString(200U));
   U_NEW(UString, string_HTTP_Variables, UString(U_CAPACITY));

   if (cgi_cookie_option == 0) U_NEW(UString, cgi_cookie_option, U_STRING_FROM_CONSTANT("[\"\" 0]"));

#ifdef U_ALIAS
   U_INTERNAL_ASSERT_EQUALS(alias, 0)

   U_NEW(UString, alias, UString);

   if (virtual_host) U_SRV_LOG("Virtual host service enabled");
#endif

#ifdef USE_LIBMAGIC
   (void) UMagic::init();
#endif

#ifdef USE_LIBSSL
// if (UServer_Base::bssl) enable_caching_by_proxy_servers = true;
#endif

#if defined(USE_PAGE_SPEED) || defined(USE_LIBV8) || defined(USE_RUBY) || defined(USE_PHP) || defined(USE_PYTHON)
   const char* msg;
#endif

#ifdef USE_PAGE_SPEED
   U_INTERNAL_ASSERT_EQUALS(page_speed, 0)

   U_NEW(UHTTP::UPageSpeed, page_speed, UHTTP::UPageSpeed);

   msg = "WARNING: load of plugin pagespeed failed";

   if (page_speed->load(U_CONSTANT_TO_PARAM("server_plugin_pagespeed")) == false)
      {
      delete page_speed;
             page_speed = 0;
      }
   else
      {
      msg = "Load of plugin pagespeed success";

      page_speed->minify_html  = (vPFpcstr)(*page_speed)["minify_html"];
      page_speed->optimize_gif = (vPFstr)  (*page_speed)["optimize_gif"];
      page_speed->optimize_png = (vPFstr)  (*page_speed)["optimize_png"];
      page_speed->optimize_jpg = (vPFstr)  (*page_speed)["optimize_jpg"];

      U_INTERNAL_ASSERT_POINTER(page_speed->minify_html)
      U_INTERNAL_ASSERT_POINTER(page_speed->optimize_gif)
      U_INTERNAL_ASSERT_POINTER(page_speed->optimize_png)
      U_INTERNAL_ASSERT_POINTER(page_speed->optimize_jpg)
      }

   U_SRV_LOG("%s", msg);
#endif

#ifdef USE_LIBV8
   U_INTERNAL_ASSERT_EQUALS(v8_javascript, 0)

   U_NEW(UHTTP::UV8JavaScript, v8_javascript, UHTTP::UV8JavaScript);

   msg = "WARNING: load of plugin v8 failed";

   if (v8_javascript->load(U_CONSTANT_TO_PARAM("server_plugin_v8")) == false)
      {
      delete v8_javascript;
             v8_javascript = 0;
      }
   else
      {
      msg = "Load of plugin v8 success";

      v8_javascript->runv8 = (vPFstr)(*v8_javascript)["runv8"];

      U_INTERNAL_ASSERT_POINTER(v8_javascript->runv8)
      }

   U_SRV_LOG("%s", msg);
#endif

#ifdef USE_RUBY
   U_INTERNAL_ASSERT_EQUALS(ruby_embed, 0)

   U_NEW(UHTTP::URUBY, ruby_embed, UHTTP::URUBY);

   msg = "WARNING: load of plugin ruby failed";

   if (ruby_embed->load(U_CONSTANT_TO_PARAM("server_plugin_ruby")) == false)
      {
      delete ruby_embed;
             ruby_embed = 0;
      }
   else
      {
      ruby_embed->initRUBY = (bPF)(*ruby_embed)["initRUBY"];
      ruby_embed->runRUBY  = (bPF)(*ruby_embed)["runRUBY"];
      ruby_embed->endRUBY  = (vPF)(*ruby_embed)["endRUBY"];

      U_INTERNAL_ASSERT_POINTER(ruby_embed->initRUBY)
      U_INTERNAL_ASSERT_POINTER(ruby_embed->runRUBY)
      U_INTERNAL_ASSERT_POINTER(ruby_embed->endRUBY)

      // check for RoR (Ruby on Rails)

      if (UStringExt::endsWith(u_cwd, u_cwd_len, U_CONSTANT_TO_PARAM("public")) == false ||
          UFile::access("../config.ru", R_OK) == false)
         {
         if (ruby_embed->initRUBY()) msg = "Load of plugin ruby success";
         }
      else
         {
         ruby_on_rails = true;

         UString dir = UStringExt::dirname(u_cwd, u_cwd_len).copy();

         if (UFile::chdir(dir.data(), true) == false) U_ERROR("Chdir to directory %V failed", dir.rep);

         ruby_on_rails = ruby_embed->initRUBY();

         if (ruby_on_rails) msg = 0;
         else               msg = "WARNING: load of Ruby on Rails application failed";

         (void) UFile::chdir(0, true);
         }
      }

   if (msg) { U_SRV_LOG("%s", msg); }
#endif

#ifdef USE_PYTHON
   U_INTERNAL_ASSERT_EQUALS(python_embed, 0)

   U_NEW(UHTTP::UPYTHON, python_embed, UHTTP::UPYTHON);

   msg = "WARNING: load of plugin python failed";

   if (python_embed->load(U_CONSTANT_TO_PARAM("server_plugin_python")) == false)
      {
      delete python_embed;
             python_embed = 0;
      }
   else
      {
      python_embed->initPYTHON = (bPF)(*python_embed)["initPYTHON"];
      python_embed->runPYTHON  = (bPF)(*python_embed)["runPYTHON"];
      python_embed->endPYTHON  = (vPF)(*python_embed)["endPYTHON"];

      U_INTERNAL_ASSERT_POINTER(python_embed->initPYTHON)
      U_INTERNAL_ASSERT_POINTER(python_embed->runPYTHON)
      U_INTERNAL_ASSERT_POINTER(python_embed->endPYTHON)

      if (py_project_app == 0) msg = 0; // python wsgi app not specified; skipping python initialization...
      else
         {
         if (python_embed->initPYTHON()) msg = "Load of plugin python success";
         }
      }

   if (msg) { U_SRV_LOG("%s", msg); }
#endif

#ifdef USE_PHP
   U_INTERNAL_ASSERT_EQUALS(php_embed, 0)

   U_NEW(UHTTP::UPHP, php_embed, UHTTP::UPHP);

   msg = "WARNING: load of plugin php failed";

   if (php_embed->load(U_CONSTANT_TO_PARAM("server_plugin_php")) == false)
      {
      delete php_embed;
             php_embed = 0;
      }
   else
      {
      php_embed->initPHP = (bPF)(*php_embed)["initPHP"];
      php_embed->runPHP  = (bPF)(*php_embed)["runPHP"];
      php_embed->endPHP  = (vPF)(*php_embed)["endPHP"];

      U_INTERNAL_ASSERT_POINTER(php_embed->initPHP)
      U_INTERNAL_ASSERT_POINTER(php_embed->runPHP)
      U_INTERNAL_ASSERT_POINTER(php_embed->endPHP)

      if (php_embed->initPHP())
         {
         msg = 0;

         if (UFile::access("index.php", R_OK)) npathinfo = U_CONSTANT_SIZE("/index.php"); // check for some CMS (Ex: Drupal)
         }
      }

   if (msg) { U_SRV_LOG("%s", msg); }
#endif

   /**
    * Set up static environment variables
    * -------------------------------------------------------------------------------------------------------------------------------------------
    * static variable server description:
    *
    * - SERVER_PORT
    * - SERVER_ADDR
    * - SERVER_NAME       Server's hostname, DNS alias, or IP address as it appears in self-referencing URLs
    * - DOCUMENT_ROOT     The root directory of your server
    * - SERVER_SOFTWARE   Name and version of the information server software answering the request (and running the gateway). Format: name/version
    * - GATEWAY_INTERFACE CGI specification revision with which this server complies. Format: CGI/revision
    *
    * Example:
    * SERVER_PORT=80
    * SERVER_ADDR=127.0.0.1
    * SERVER_NAME=localhost
    * DOCUMENT_ROOT="/var/www/localhost/htdocs"
    * SERVER_SOFTWARE=Apache
    * GATEWAY_INTERFACE=CGI/1.1
    * -------------------------------------------------------------------------------------------------------------------------------------------
    */

   U_INTERNAL_ASSERT_POINTER(UServer_Base::cenvironment)

   U_INTERNAL_DUMP("UServer_Base::cenvironment(%u) = %V", UServer_Base::cenvironment->size(), UServer_Base::cenvironment->rep)

   UServer_Base::cenvironment->snprintf_add(U_CONSTANT_TO_PARAM("SERVER_NAME=%.*s\n" // Your server's fully qualified domain name (e.g. www.cgi101.com)
                                            "SERVER_PORT=%d\n"), // The port number your server is listening on
                                            u_hostname_len, u_hostname, UServer_Base::port);

   U_ASSERT_EQUALS(UServer_Base::cenvironment->isBinary(), false)

   (void) UServer_Base::cenvironment->shrink();

   U_INTERNAL_ASSERT_POINTER(UServer_Base::senvironment)

   UServer_Base::senvironment->snprintf_add(U_CONSTANT_TO_PARAM("SERVER_ADDR=%v\n"
                                            "DOCUMENT_ROOT=%w\n" // The root directory of your server
                                            "SERVER_SOFTWARE=" PACKAGE_NAME "/" ULIB_VERSION "\n"),
                                            UServer_Base::getIPAddress().rep);

   U_ASSERT_EQUALS(UServer_Base::senvironment->isBinary(), false)

   (void) UServer_Base::senvironment->shrink();

   // CACHE DOCUMENT ROOT FILE SYSTEM

   uint32_t n = 0, sz;
   UVector<UString> vec(4000);
   UString content_cache, item, updir = U_STRING_FROM_CONSTANT("..");

   U_INTERNAL_ASSERT_EQUALS(cache_file, 0)

   U_NEW(UHashMap<UHTTP::UFileCacheData*>, cache_file, UHashMap<UHTTP::UFileCacheData*>);

#ifdef U_STATIC_ONLY
# if defined(U_ALIAS) && !defined(U_STATIC_SERVLET_WI_AUTH)
   U_INTERNAL_ASSERT_EQUALS(virtual_host, false)
# endif
   /**
    * I do know that to include code in the middle of a function is hacky and dirty, but this is the best solution that I could figure out.
    * If you have some idea to clean it up, please, don't hesitate and let me know
    */
# include "../net/server/plugin/usp/loader.autoconf.cpp"
#endif

   U_INTERNAL_ASSERT_EQUALS(file_not_in_cache_data, 0)

   U_NEW(UHTTP::UFileCacheData, file_not_in_cache_data, UHTTP::UFileCacheData);

   // manage authorization data...

   UDirWalk dirwalk(&updir, U_CONSTANT_TO_PARAM("*.htpasswd|*.htdigest"));

   n = dirwalk.walk(vec);

   if (cache_file_mask &&
       cache_file_mask->equal(U_CONSTANT_TO_PARAM("_off_")))
      {
      if (nocache_file_mask == 0) U_NEW(UString, nocache_file_mask, U_STRING_FROM_CONSTANT("*"));
      }
   else
      {
      if (cache_avoid_mask == 0) UDirWalk::setDirectory(*UString::str_point);
      else                       UDirWalk::setDirectory(*UString::str_point, *cache_avoid_mask, FNM_INVERT);

      UDirWalk::setFollowLinks(true);
      UDirWalk::setRecurseSubDirs(true, true);
      UDirWalk::setSuffixFileType(U_CONSTANT_TO_PARAM("usp|c|cgi|template|" U_LIB_SUFFIX));

      n = dirwalk.walk(vec);

      UDirWalk::setRecurseSubDirs(false, false);
      }

   if (cache_file_store)
      {
      content_cache = (UStringExt::endsWith(U_STRING_TO_PARAM(*cache_file_store), U_CONSTANT_TO_PARAM(".gz"))
                        ? UStringExt::gunzip(UFile::contentOf(*cache_file_store))
                        :                    UFile::contentOf(*cache_file_store));

#  ifdef U_STDCPP_ENABLE
      if (content_cache)
         {
         n += (content_cache.size() / (1024 + 512)); // NB: we assume as medium file size something like ~1.5k...

         UString2Object(U_STRING_TO_PARAM(content_cache), *cache_file);

         U_ASSERT_MAJOR(cache_file->size(), 0)

         U_SRV_LOG("Loaded cache file store: %V", cache_file_store->rep);

         U_ASSERT(cache_file_check_memory())
         }
#  endif
      }

   sz = n + (15 * (n / 100)) + 32;

   if (sz > cache_file->capacity()) cache_file->reserve(sz);

   U_INTERNAL_ASSERT_POINTER(pathname)

   for (uint32_t i = 0, v = vec.size(); i < v; ++i)
      {
      item = vec[i];

      // NB: we can have duplication (symlink, cache_file_store)

      if (cache_file->at(U_STRING_TO_PARAM(item)))
         {
         U_SRV_LOG("WARNING: found duplicate file: %V", item.rep);
         }
      else
         {
         (void) pathname->replace(item);

         checkFileForCache();
         }
      }

#if !defined(U_SERVER_CAPTIVE_PORTAL) && defined(U_STDCPP_ENABLE)
   if (cache_file_store &&
       content_cache.empty())
      {
      char buffer[2 * 1024 * 1024];

      sz = UObject2String(*cache_file, buffer, sizeof(buffer));

      U_INTERNAL_ASSERT_MINOR(sz, sizeof(buffer))

      if (UFile::writeTo(*cache_file_store, buffer, sz)) U_SRV_LOG("Saved (%u bytes) cache file store: %V", sz, cache_file_store->rep);
      }
#endif

   U_ASSERT(cache_file_check_memory())

   // manage authorization data...

   file_data = cache_file->at(U_CONSTANT_TO_PARAM("../.htpasswd"));

   if (file_data)
      {
      U_INTERNAL_ASSERT_POINTER(file_data->array)

      U_NEW(UString, htpasswd, UString(file_data->array->operator[](0)));

      U_SRV_LOG("File data users permission: ../.htpasswd loaded");
      }

   file_data = cache_file->at(U_CONSTANT_TO_PARAM("../.htdigest"));

   if (file_data)
      {
      U_INTERNAL_ASSERT_POINTER(file_data->array)

      U_NEW(UString, htdigest, UString(file_data->array->operator[](0)));

      U_SRV_LOG("File data users permission: ../.htdigest loaded");
      }

   UServices::generateKey(UServices::key, 0); // for ULib facility request TODO session cookies... 

   if (htdigest ||
       htpasswd)
      {
      // manage icons for directory listing...

      file_data = cache_file->at(U_CONSTANT_TO_PARAM("icons/dir.png"));

      if (file_data == 0)
         {
         static const unsigned char dir_store[] = {
#           include "dir_store.bin" // od -A n -t x1 dir_store.bin.gz
         };

         content_cache = UStringExt::gunzip((const char*)dir_store, sizeof(dir_store), 0);

         UString2Object(U_STRING_TO_PARAM(content_cache), *cache_file);

         U_ASSERT_MAJOR(cache_file->size(), 0)

         U_SRV_LOG("Loaded cache icon store for directory listing");

         U_ASSERT(cache_file_check_memory())
         }
      }

#ifdef U_HTML_PAGINATION_SUPPORT // manage css for HTML Pagination
   file_data = cache_file->at(U_CONSTANT_TO_PARAM("css/pagination.min.css"));

   if (file_data == 0)
      {
      static const unsigned char pagination_store[] = {
#        include "pagination_store.bin" // od -A n -t x1 pagination_store.bin.gz
      };

      content_cache = UStringExt::gunzip((const char*)pagination_store, sizeof(pagination_store), 0);

      UString2Object(U_STRING_TO_PARAM(content_cache), *cache_file);

      U_ASSERT_MAJOR(cache_file->size(), 0)

      U_SRV_LOG("Loaded cache css store for HTML pagination");

      U_ASSERT(cache_file_check_memory())
      }
#endif

   sz = cache_file->size();

   U_INTERNAL_DUMP("cache size = %u", sz)

#ifdef U_LINUX // set fd_max limit...
   uint32_t rlim = (sz + UNotifier::max_connection + 100);

   U_INTERNAL_DUMP("rlim = %u", rlim)

   if (rlim > 1024)
      {
      /**
       * struct rlimit {
       *    rlim_t rlim_cur; // Soft limit
       *    rlim_t rlim_max; // Hard limit (ceiling for rlim_cur)
       * };
       */

      struct rlimit nofile = { rlim, rlim };

      if (U_SYSCALL(setrlimit, "%d,%p", RLIMIT_NOFILE, &nofile) == 0)
         {
         U_SRV_LOG("Updated program fd_max: %u", rlim);
         }
      else
         {
         U_WARNING("Your DOCUMENT_ROOT cache may be require at least %u max file descriptors. I can't set maximum open files because of OS error", rlim);

         if (U_SYSCALL(getrlimit, "%d,%p", RLIMIT_NOFILE, &nofile) == 0)
            {
            U_WARNING("Current maximum open files is %u. If you need higher increase 'ulimit -n'", nofile.rlim_max);
            }
         }
      }
#endif

   // various setting...

   U_INTERNAL_DUMP("U_http_method_list = %p", U_http_method_list)

   u_init_http_method_list();

   U_http_info.nResponseCode = HTTP_OK;

   setStatusDescription();

   U_http_info.nResponseCode = 0;

   U_INTERNAL_ASSERT_EQUALS(response_code, HTTP_OK)

   U_MEMCPY(response_buffer, UClientImage_Base::iov_vec[0].iov_base, UClientImage_Base::iov_vec[0].iov_len);

   U_INTERNAL_ASSERT_EQUALS(strncmp(response_buffer, U_CONSTANT_TO_PARAM("HTTP/1.1 200 OK\r\n")), 0)

#ifndef U_HTTP2_DISABLE
   UHTTP2::ctor();
#endif
}

#ifdef DEBUG
U_NO_EXPORT bool UHTTP::check_memory(UStringRep* key, void* value)
{
   U_TRACE(0, "UHTTP::check_memory(%V,%p)", key, value)

   U_INTERNAL_ASSERT_POINTER(value)

   UHTTP::UFileCacheData* cptr = (UHTTP::UFileCacheData*)value;

   if (cptr->array) (void) cptr->array->check_memory();

   U_RETURN(true);
}

bool UHTTP::cache_file_check_memory()
{
   U_TRACE_NO_PARAM(0, "UHTTP::cache_file_check_memory()")

   U_INTERNAL_ASSERT_POINTER(cache_file)

   cache_file->check_memory();

   cache_file->callForAllEntry(check_memory);

   U_RETURN(true);
}
#endif

#ifdef U_ALIAS
void UHTTP::setGlobalAlias(const UString& _alias) // NB: automatic alias for all uri request without suffix...
{
   U_TRACE(0, "UHTTP::setGlobalAlias(%V)", _alias.rep)

   U_INTERNAL_ASSERT(_alias)

   if (global_alias)
      {
      U_WARNING("UHTTP::setGlobalAlias(): global alias not empty: %V", global_alias->rep);

      delete global_alias;
      }

   U_INTERNAL_ASSERT_EQUALS(global_alias, 0)

   U_NEW(UString, global_alias, UString(_alias));

   if (global_alias->first_char() != '/') (void) global_alias->insert(0, '/');
}
#endif

void UHTTP::dtor()
{
   U_TRACE_NO_PARAM(0, "UHTTP::dtor()")

#if defined(HAVE_SYS_INOTIFY_H) && defined(U_HTTP_INOTIFY_SUPPORT) && !defined(U_SERVER_CAPTIVE_PORTAL)
   if (UServer_Base::handler_inotify && // inotify: Inode based directory notification...
       UServer_Base::handler_inotify->fd != -1)
      {
      (void) U_SYSCALL(close, "%d", UServer_Base::handler_inotify->fd);

      delete inotify_pathname;

      UServer_Base::handler_inotify = 0;
      }
#endif

   if (vservice)              delete vservice;
   if (vmsg_error)            delete vmsg_error;
   if (fcgi_uri_mask)         delete fcgi_uri_mask;
   if (scgi_uri_mask)         delete scgi_uri_mask;
   if (cache_file_store)      delete cache_file_store;
   if (string_HTTP_Variables) delete string_HTTP_Variables;

   if (file)
      {
      delete ext;
      delete etag;
      delete file;
      delete pcmd;
      delete geoip;
      delete suffix;
      delete tmpdir;
      delete request;
      delete qcontent;
      delete pathname;
      delete rpathname;
      delete formMulti;
      delete upload_dir;
      delete set_cookie;
      delete form_name_value;
      delete cache_avoid_mask;
      delete cgi_cookie_option;
      delete set_cookie_option;

      if (htpasswd)          delete htpasswd;
      if (htdigest)          delete htdigest;
      if (  cache_file_mask) delete   cache_file_mask;
      if (nocache_file_mask) delete nocache_file_mask;

#  ifdef U_ALIAS
                                 delete  alias;
      if (valias)                delete valias;
      if (global_alias)          delete global_alias;
      if (maintenance_mode_page) delete maintenance_mode_page;
#    ifdef USE_LIBPCRE
      if (vRewriteRule) delete vRewriteRule;
#    endif
#  endif

#  ifdef USE_PHP
      if (php_embed)       delete php_embed;
#  endif
      if (php_mount_point) delete php_mount_point;
#  ifdef USE_RUBY
      if (ruby_embed)  delete ruby_embed;
      if (ruby_libdir) delete ruby_libdir;
#  endif
#  ifdef USE_PYTHON
      if (python_embed)       delete python_embed;
      if (py_project_app)     delete py_project_app;
      if (py_project_root)    delete py_project_root;
      if (py_virtualenv_path) delete py_virtualenv_path;
#  endif
#  ifdef USE_PAGE_SPEED
      if (page_speed) delete page_speed;
#  endif
#  ifdef USE_LIBV8
      if (v8_javascript) delete v8_javascript;
#  endif

      // CACHE DOCUMENT ROOT FILE SYSTEM

      if (file_data != file_not_in_cache_data)
         {
         file_not_in_cache_data->ptr   =
         file_not_in_cache_data->array = 0;

         delete file_not_in_cache_data;
         }

      file_data = 0;

      delete cache_file;

      if (db_session) clearSession();

      if (db_not_found)
         {
         db_not_found->close();

         delete db_not_found;
         }
         
#  ifdef USE_LIBSSL
      if (db_session_ssl) clearSessionSSL();

      if (vallow_IP)             delete vallow_IP;
      if (uri_protected_mask)    delete uri_protected_mask;
      if (uri_request_cert_mask) delete uri_request_cert_mask;
#  endif

#  ifdef U_HTTP_STRICT_TRANSPORT_SECURITY
      if (uri_strict_transport_security_mask &&
          uri_strict_transport_security_mask != (void*)1L)
         {
         delete uri_strict_transport_security_mask;
         }
#  endif
      }

#ifndef U_HTTP2_DISABLE
   UHTTP2::dtor();
#endif
}

__pure bool UHTTP::isMobile()
{
   U_TRACE_NO_PARAM(0, "UHTTP::isMobile()")

   if (U_http_info.user_agent_len &&
       UServices::dosMatchWithOR(U_HTTP_USER_AGENT_TO_PARAM,
                                 U_CONSTANT_TO_PARAM("*android*|"
                                                     "*iphone*|*ipod*|"
                                                     "*windows ce*|*windows phone*|"
                                                     "*blackberry*|*palm*|*opera mini*|"
                                                     "*webkit*series60*|*webkit*symbian*"), FNM_IGNORECASE))
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

/**
 * HTTP message
 * ---------------------------------------------------------------------------------------------------------------------------
 * There are four parts to an HTTP request:
 * ---------------------------------------------------------------------------------------------------------------------------
 * 1) the request line    [REQUIRED]: the method, the URL, the version of the protocol
 * 2) the request headers [OPTIONAL]: a series of lines (one per) in the format of name, colon(:), and the value of the header
 * 3) a blank line        [REQUIRED]: worth mentioning by itself
 * 4) the request Body    [OPTIONAL]: used in POST/PUT/PATCH requests to send content to the server
 * ======================================================================================
 * Read the request line and attached headers. A typical http request will take the form:
 * ======================================================================================
 * GET / HTTP/1.1
 * Host: 127.0.0.1
 * User-Agent: Mozilla/5.0 (compatible; Konqueror/3.1; Linux; it, en_US, en)
 * Accept-Encoding: gzip, deflate
 * Accept-Charset: iso-8859-1, utf-8;q=0.5, *;q=0.5
 * Accept-Language: it, en
 * Connection: Keep-Alive
 * <empty line>
 * ======================================================================================
 * Read the result line and attached headers. A typical http response will take the form:
 * ======================================================================================
 * HTTP/1.1 200 OK
 * Date: Wed, 25 Oct 2000 16:54:02 GMT
 * Age: 57718
 * Server: Apache/1.3b5
 * Last-Modified: Sat, 23 Sep 2000 15:00:57 GMT
 * Accept-Ranges: bytes
 * Etag: "122b12-2d8c-39ccc5a9"
 * Content-Type: text/html
 * Content-Length: 11660
 * <empty line>
 * .......<the data>
 * ====================================================================================
 * A body is not required by the IETF standard, though the content-length should be 0
 * if there's no body. Use the method that's appropriate for what you're doing.
 * If you were to put it into code, given
 * 
 * int x;
 * int f() { return x; }
 * 
 * and a remote variable called r.
 * 
 * POST is equivalent to: r = f(); - create new object
 * PUT  is equivalent to: r = x;   - update     object
 * GET  is equivalent to: x = r;   - retrieve   object (list objects)
 * ====================================================================================
 */

__pure bool UHTTP::isValidMethod(const char* ptr)
{
   U_TRACE(0, "UHTTP::isValidMethod(%.*S)", 30, ptr)

   if (*ptr != 'G')
      {
      // RFC 2616 4.1 "servers SHOULD ignore any empty line(s) received where a Request-Line is expected"

      if (UNLIKELY(u__isspace(*ptr))) while (u__isspace((*++ptr))) {}

      // GET
      // HEAD
      // POST/PUT/PATCH
      // COPY
      // DELETE
      // OPTIONS

      if (u__ismethod(*ptr) == false) U_RETURN(false);
      }

   switch (u_get_unalignedp32(ptr))
      {
      case U_MULTICHAR_CONSTANT32('g','e','t',' '):
      case U_MULTICHAR_CONSTANT32('G','E','T',' '):
      case U_MULTICHAR_CONSTANT32('h','e','a','d'):
      case U_MULTICHAR_CONSTANT32('H','E','A','D'):
      case U_MULTICHAR_CONSTANT32('p','o','s','t'):
      case U_MULTICHAR_CONSTANT32('P','O','S','T'):
      case U_MULTICHAR_CONSTANT32('p','u','t',' '):
      case U_MULTICHAR_CONSTANT32('P','U','T',' '):
      case U_MULTICHAR_CONSTANT32('d','e','l','e'):
      case U_MULTICHAR_CONSTANT32('D','E','L','E'):
      case U_MULTICHAR_CONSTANT32('o','p','t','i'):
      case U_MULTICHAR_CONSTANT32('O','P','T','I'):
      // NOT IMPLEMENTED
      case U_MULTICHAR_CONSTANT32('T','R','A','C'):
      case U_MULTICHAR_CONSTANT32('C','O','N','N'):
      case U_MULTICHAR_CONSTANT32('C','O','P','Y'):
      case U_MULTICHAR_CONSTANT32('M','O','V','E'):
      case U_MULTICHAR_CONSTANT32('L','O','C','K'):
      case U_MULTICHAR_CONSTANT32('U','N','L','O'):
      case U_MULTICHAR_CONSTANT32('M','K','C','O'):
      case U_MULTICHAR_CONSTANT32('S','E','A','R'):
      case U_MULTICHAR_CONSTANT32('P','R','O','P'):
      case U_MULTICHAR_CONSTANT32('P','A','T','C'):
      case U_MULTICHAR_CONSTANT32('P','U','R','G'):
      case U_MULTICHAR_CONSTANT32('M','E','R','G'):
      case U_MULTICHAR_CONSTANT32('R','E','P','O'):
      case U_MULTICHAR_CONSTANT32('C','H','E','C'):
      case U_MULTICHAR_CONSTANT32('M','K','A','C'):
      case U_MULTICHAR_CONSTANT32('N','O','T','I'):
      case U_MULTICHAR_CONSTANT32('M','S','E','A'):
      case U_MULTICHAR_CONSTANT32('S','U','B','S'):
      case U_MULTICHAR_CONSTANT32('U','N','S','U'): U_RETURN(true);
      }

   U_RETURN(false);
}

__pure bool UHTTP::isValidRequest(const char* ptr, uint32_t sz)
{
   U_TRACE(0, "UHTTP::isValidRequest(%.*S,%u)", 30, ptr, sz)

   U_INTERNAL_ASSERT_MAJOR(sz, 0)

   U_INTERNAL_DUMP("sz = %u UClientImage_Base::size_request = %u", sz, UClientImage_Base::size_request)

   if (u_get_unalignedp32(ptr+sz-4) == U_MULTICHAR_CONSTANT32('\r','\n','\r','\n')) U_RETURN(true);

   U_RETURN(false);
}

__pure bool UHTTP::isValidRequestExt(const char* ptr, uint32_t sz)
{
   U_TRACE(0, "UHTTP::isValidRequestExt(%.*S,%u)", 30, ptr, sz)

   U_INTERNAL_ASSERT_MAJOR(sz, 0)

   if (sz >= U_CONSTANT_SIZE("GET / HTTP/1.0\r\n\r\n")        &&
       isValidMethod(ptr)                                     &&
       (isValidRequest(ptr, sz)                               ||
                           (UClientImage_Base::size_request   &&
        isValidRequest(ptr, UClientImage_Base::size_request)) ||
        u_findEndHeader1(ptr, sz) != U_NOT_FOUND))
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UHTTP::scanfHeaderRequest(const char* ptr, uint32_t size)
{
   U_TRACE(0, "UHTTP::scanfHeaderRequest(%.*S,%u)", size, ptr, size)

   /**
    * Check HTTP request
    * -------------------------------------------------------------------
    * The default is GET for input requests and POST for output requests.
    * Other possible alternatives are:
    * -------------------------------------------------------------------
    *  - PUT
    *  - HEAD
    *  - COPY
    *  - PATCH
    *  - DELETE
    *  - OPTIONS
    * ---------------------- NOT implemented ----------------------------
    *  - CONNECT
    *  - TRACE (because can send client cookie information, dangerous...)
    * -------------------------------------------------------------------
    * for further information about HTTP request methods see:
    *
    * http://ietf.org/rfc/rfc2616.txt
    * -------------------------------------------------------------------
    */

   unsigned char c;
   const char* endptr;
   const char* start = ptr;

   if (u_get_unalignedp32(ptr) == U_MULTICHAR_CONSTANT32('G','E','T',' ')) // try to parse the request line: GET / HTTP/1.n
      {
      ptr += 4;

      U_INTERNAL_ASSERT_EQUALS(U_http_method_num, 0)

      U_http_method_type = HTTP_GET;

      goto next;
      }

   if (*ptr == 'P')
      {
#  ifndef U_HTTP2_DISABLE
      if (u_get_unalignedp64(ptr)    == U_MULTICHAR_CONSTANT64( 'P', 'R','I',' ', '*', ' ', 'H', 'T') &&
          u_get_unalignedp64(ptr+8)  == U_MULTICHAR_CONSTANT64( 'T', 'P','/','2', '.', '0','\r','\n') &&
          u_get_unalignedp64(ptr+16) == U_MULTICHAR_CONSTANT64('\r','\n','S','M','\r','\n','\r','\n'))
         {
         U_http_version = '2';

         UClientImage_Base::size_request = U_CONSTANT_SIZE(HTTP2_CONNECTION_PREFACE);

         goto end;
         }
#  endif

      if (u_get_unalignedp32(ptr) == U_MULTICHAR_CONSTANT32('P','O','S','T'))
         {
         ptr += 5;

         U_http_method_num  = 2;
         U_http_method_type = HTTP_POST;

         goto next;
         }
      }

   U_INTERNAL_DUMP("char (before method) = %C", *ptr)

   if (UNLIKELY(u__isspace(*ptr))) while (u__isspace(*++ptr)); // RFC 2616 4.1 "servers SHOULD ignore any empty line(s) received where a Request-Line is expected"

   U_INTERNAL_ASSERT_EQUALS(U_http_method_num, 0)

   // GET
   // HEAD
   // POST/PUT/PATCH
   // COPY
   // DELETE
   // OPTIONS

   switch (u_get_unalignedp32(ptr))
      {
      case U_MULTICHAR_CONSTANT32('g','e','t',' '):
      case U_MULTICHAR_CONSTANT32('G','E','T',' '): U_http_method_type = HTTP_GET;         ptr +=  4;                         break;
      case U_MULTICHAR_CONSTANT32('h','e','a','d'):
      case U_MULTICHAR_CONSTANT32('H','E','A','D'): U_http_method_type = HTTP_HEAD;        ptr +=  5; U_http_method_num =  1; break;
      case U_MULTICHAR_CONSTANT32('p','o','s','t'):
      case U_MULTICHAR_CONSTANT32('P','O','S','T'): U_http_method_type = HTTP_POST;        ptr +=  5; U_http_method_num =  2; break;
      case U_MULTICHAR_CONSTANT32('p','u','t',' '):
      case U_MULTICHAR_CONSTANT32('P','U','T',' '): U_http_method_type = HTTP_PUT;         ptr +=  4; U_http_method_num =  3; break;
      case U_MULTICHAR_CONSTANT32('d','e','l','e'):
      case U_MULTICHAR_CONSTANT32('D','E','L','E'): U_http_method_type = HTTP_DELETE;      ptr +=  7; U_http_method_num =  4; break;
      case U_MULTICHAR_CONSTANT32('o','p','t','i'):
      case U_MULTICHAR_CONSTANT32('O','P','T','I'): U_http_method_type = HTTP_OPTIONS;     ptr +=  8; U_http_method_num =  5; break;
      // NOT IMPLEMENTED
      case U_MULTICHAR_CONSTANT32('T','R','A','C'): U_http_method_type = HTTP_TRACE;       ptr +=  6; U_http_method_num =  6; break;
      case U_MULTICHAR_CONSTANT32('C','O','N','N'): U_http_method_type = HTTP_CONNECT;     ptr +=  8; U_http_method_num =  7; break;
      case U_MULTICHAR_CONSTANT32('C','O','P','Y'): U_http_method_type = HTTP_COPY;        ptr +=  5; U_http_method_num =  8; break;
      case U_MULTICHAR_CONSTANT32('M','O','V','E'): U_http_method_type = HTTP_MOVE;        ptr +=  5; U_http_method_num =  9; break;
      case U_MULTICHAR_CONSTANT32('L','O','C','K'): U_http_method_type = HTTP_LOCK;        ptr +=  5; U_http_method_num = 10; break;
      case U_MULTICHAR_CONSTANT32('U','N','L','O'): U_http_method_type = HTTP_UNLOCK;      ptr +=  7; U_http_method_num = 11; break;
      case U_MULTICHAR_CONSTANT32('M','K','C','O'): U_http_method_type = HTTP_MKCOL;       ptr +=  6; U_http_method_num = 12; break;
      case U_MULTICHAR_CONSTANT32('S','E','A','R'): U_http_method_type = HTTP_SEARCH;      ptr +=  7; U_http_method_num = 13; break;
      case U_MULTICHAR_CONSTANT32('P','R','O','P'): U_http_method_type = HTTP_PROPFIND;    ptr +=  9; U_http_method_num = 14; break;
      case U_MULTICHAR_CONSTANT32('P','A','T','C'): U_http_method_type = HTTP_PATCH;       ptr +=  6; U_http_method_num = 16; break;
      case U_MULTICHAR_CONSTANT32('P','U','R','G'): U_http_method_type = HTTP_PURGE;       ptr +=  6; U_http_method_num = 17; break;
      case U_MULTICHAR_CONSTANT32('M','E','R','G'): U_http_method_type = HTTP_MERGE;       ptr +=  6; U_http_method_num = 18; break;
      case U_MULTICHAR_CONSTANT32('R','E','P','O'): U_http_method_type = HTTP_REPORT;      ptr +=  7; U_http_method_num = 19; break;
      case U_MULTICHAR_CONSTANT32('C','H','E','C'): U_http_method_type = HTTP_CHECKOUT;    ptr +=  9; U_http_method_num = 20; break;
      case U_MULTICHAR_CONSTANT32('M','K','A','C'): U_http_method_type = HTTP_MKACTIVITY;  ptr += 11; U_http_method_num = 21; break;
      case U_MULTICHAR_CONSTANT32('N','O','T','I'): U_http_method_type = HTTP_NOTIFY;      ptr +=  7; U_http_method_num = 22; break;
      case U_MULTICHAR_CONSTANT32('M','S','E','A'): U_http_method_type = HTTP_MSEARCH;     ptr +=  8; U_http_method_num = 23; break;
      case U_MULTICHAR_CONSTANT32('S','U','B','S'): U_http_method_type = HTTP_SUBSCRIBE;   ptr += 10; U_http_method_num = 24; break;
      case U_MULTICHAR_CONSTANT32('U','N','S','U'): U_http_method_type = HTTP_UNSUBSCRIBE; ptr += 12; U_http_method_num = 25; break;

      default: U_RETURN(false);
      }

next:
   U_INTERNAL_DUMP("U_http_method_type = %B U_http_method_num = %d", U_http_method_type, U_http_method_num)

   U_INTERNAL_ASSERT_MAJOR(U_http_method_type, 0)

   c = *(unsigned char*)(ptr-1);

   U_INTERNAL_DUMP("char (after method) = %C", c)

   if (UNLIKELY(u__isblank(c) == false))
      {
      U_http_method_type = 0;

      U_RETURN(false);
      }

   if (UNLIKELY(u__isblank(*ptr))) while (u__isblank(*++ptr)); // RFC 2616 19.3 "[servers] SHOULD accept any amount of SP or HT characters between [Request-Line] fields"

   U_http_info.uri = ptr;

   endptr = start + size;

   while (ptr < endptr)
      {
      c = *(unsigned char*)ptr;

      if (u__isblank(c))
         {
         if (U_http_info.uri_len == 0) U_http_info.uri_len = ptr - U_http_info.uri;
         else
            {
            U_http_info.query_len = ptr - U_http_info.query;

            U_INTERNAL_DUMP("query = %.*S", U_HTTP_QUERY_TO_TRACE)
            }

         U_INTERNAL_DUMP("URI = %.*S", U_HTTP_URI_TO_TRACE)

         if (UNLIKELY(u__isblank(*++ptr))) while (u__isblank(*++ptr)); // RFC 2616 19.3 "[servers] SHOULD accept any amount of SP or HT characters between [Request-Line] fields"

         if (u_get_unalignedp64(ptr-1) == U_MULTICHAR_CONSTANT64(' ','H','T','T','P','/','1','.'))
            {
            ptr += U_CONSTANT_SIZE("HTTP/1.");

            U_http_version = *ptr++;

            U_INTERNAL_DUMP("U_http_version = %C U_http_info.nResponseCode = %d", U_http_version, U_http_info.nResponseCode)

            if (ptr <= (endptr-U_CONSTANT_SIZE(U_CRLF)) &&
                u_get_unalignedp16(ptr) == U_MULTICHAR_CONSTANT16('\r','\n'))
               {
               U_line_terminator_len   = U_CONSTANT_SIZE(U_CRLF);
               U_http_info.startHeader = ptr - start;

               U_INTERNAL_DUMP("U_line_terminator_len = %u U_http_info.startHeader(%u) = %.20S", U_line_terminator_len, U_http_info.startHeader, ptr)

#           if !defined(U_CACHE_REQUEST_DISABLE) || defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST) 
               UClientImage_Base::uri_offset = U_http_info.uri - start; 

               U_INTERNAL_DUMP("UClientImage_Base::uri_offset = %u", UClientImage_Base::uri_offset)
#           endif

               U_RETURN(true);
               }

            goto end;
            }

         U_RETURN(false);
         }

      // check for invalid characters

      if (u__is2urlenc(c) &&
          u__isurlqry(c) == false)
         {
         U_INTERNAL_DUMP("char to url encode = %C U_http_info.uri_len = %u", c, U_http_info.uri_len)

         if (c == '?')
            {
            if (LIKELY(U_http_info.uri_len == 0)) // NB: we consider only the first '?' encountered...
               {
               U_http_info.uri_len = ptr - U_http_info.uri;

               if (UNLIKELY(U_http_info.uri_len == 0))
                  {
                  // NB: URI have query params but path empty...

                  U_http_info.uri     = "/";
                  U_http_info.uri_len = 1;
                  }

               U_http_info.query = ++ptr;

               continue;
               }
            }
         else
            {
            if (c == '%')
               {
               if (u__isxdigit(ptr[1]) &&
                   u__isxdigit(ptr[2]))
                  {
                  ptr += 3;

                  continue;
                  }

               goto error;
               }

            if (c != '+')
               {
error:         U_SRV_LOG("WARNING: invalid character %C in URI %.*S", c, ptr - U_http_info.uri, U_http_info.uri);

               U_RETURN(false);
               }
            }
         }

      ++ptr;
      }

   // NB: there are case of requests fragmented (maybe because of VPN tunnel)
   //
   // for example something like: GET /info?Mac=00%3A40%3A63%3Afb%3A42%3A1c&ip=172.16.93.235&gateway=172.16.93.254%3A5280&ap=ap%4010.8.0.9

end:
   U_ClientImage_data_missing = true;

   U_RETURN(false);
}

bool UHTTP::scanfHeaderResponse(const char* ptr, uint32_t size)
{
   U_TRACE(0, "UHTTP::scanfHeaderResponse(%.*S,%u)", size, ptr, size)

   // try to parse the response line: HTTP/1.n nnn <ssss>

   if (u_get_unalignedp32(ptr) == U_MULTICHAR_CONSTANT32('H','T','T','P') ||
       u_get_unalignedp32(ptr) == U_MULTICHAR_CONSTANT32('h','t','t','p'))
      {
      const char* start = ptr;

      ptr += U_CONSTANT_SIZE("HTTP/1.");

      U_http_version = *ptr++;

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      const char* ptr1 = ++ptr;

      U_INTERNAL_ASSERT(u__isdigit(*ptr1))

      while (u__isdigit(*ptr1)) ++ptr1;

      U_INTERNAL_ASSERT(u__isblank(*ptr1))

      U_http_info.nResponseCode = u_strtoul(ptr, ptr1);

      U_INTERNAL_DUMP("U_http_version = %C U_http_info.nResponseCode = %d", U_http_version, U_http_info.nResponseCode)

      U_INTERNAL_ASSERT_EQUALS(U_http_info.nResponseCode, ::strtol(ptr, 0, 10))

      if (U_IS_HTTP_VALID_RESPONSE(U_http_info.nResponseCode))
         {
         while (u__islterm(*ptr1) == false) { ++ptr1; }

         if (ptr1 <= (start+size-U_CONSTANT_SIZE(U_CRLF)) &&
             u_get_unalignedp16(ptr1) == U_MULTICHAR_CONSTANT16('\r','\n'))
            {
            U_line_terminator_len = U_CONSTANT_SIZE(U_CRLF);

            U_http_info.startHeader = ptr1 - start;

            U_INTERNAL_DUMP("U_http_info.startHeader(%u) = %.20S", U_http_info.startHeader, ptr1)

            U_RETURN(true);
            }
         }
      }

   U_RETURN(false);
}

U_NO_EXPORT bool UHTTP::readHeaderRequest()
{
   U_TRACE_NO_PARAM(0, "UHTTP::readHeaderRequest()")

   uint32_t sz     = UClientImage_Base::request->size();
   const char* ptr = UClientImage_Base::request->data();

   U_INTERNAL_DUMP("sz = %u", sz)

   if ( sz < U_CONSTANT_SIZE("GET / HTTP/1.0\r\n\r\n") &&
       (sz < U_CONSTANT_SIZE("\r\n\r\n")               ||
        u_get_unalignedp32(ptr+sz-4) != U_MULTICHAR_CONSTANT32('\r','\n','\r','\n')))
      {
      if (u_isPrintable(ptr, sz, true)) U_ClientImage_data_missing = true;
      else                              UClientImage_Base::abortive_close();

      U_RETURN(false);
      }

   if (scanfHeaderRequest(ptr, sz) == false) U_RETURN(false);

   U_INTERNAL_DUMP("sz = %u U_http_info.startHeader = %u", sz, U_http_info.startHeader)

   U_INTERNAL_ASSERT_MINOR(U_http_info.startHeader, sz)

   ptr += U_http_info.startHeader;

   U_ASSERT_EQUALS(UServer_Base::bssl, UServer_Base::csocket->isSSLActive())
   U_INTERNAL_ASSERT_EQUALS(u_get_unalignedp16(ptr), U_MULTICHAR_CONSTANT16('\r','\n'))

   if (u_get_unalignedp32(ptr) == U_MULTICHAR_CONSTANT32('\r','\n','\r','\n')) U_RETURN(true);

   sz = u_findEndHeader1(ptr+2, sz-U_http_info.startHeader-2);

   if (sz != U_NOT_FOUND) sz += U_http_info.startHeader-2;
   else
      {
#  ifdef USE_LIBSSL
      if (UServer_Base::bssl)
         {
         sz = USocketExt::readWhileNotToken(UServer_Base::csocket, *UClientImage_Base::request, U_CONSTANT_TO_PARAM(U_CRLF2), UServer_Base::timeoutMS);

         if (sz != U_NOT_FOUND) goto next;

         U_RETURN(false);
         }
#  endif

      U_ClientImage_data_missing = true;

      U_RETURN(false);
      }

#ifdef USE_LIBSSL
next:
#endif
   U_http_info.endHeader    = sz + U_CONSTANT_SIZE(U_CRLF2); // NB: U_http_info.endHeader includes also the blank line...
   U_http_info.startHeader +=      U_CONSTANT_SIZE(U_CRLF);

   U_INTERNAL_DUMP("U_http_info.startHeader(%u) = %.20S U_http_info.endHeader(%u) = %.20S",
                    U_http_info.startHeader, UClientImage_Base::request->c_pointer(U_http_info.startHeader),
                    U_http_info.endHeader,   UClientImage_Base::request->c_pointer(U_http_info.endHeader))

   U_RETURN(true);
}

bool UHTTP::readHeaderResponse(USocket* sk, UString& buffer)
{
   U_TRACE(0, "UHTTP::readHeaderResponse(%p,%V)", sk, buffer.rep)

   U_INTERNAL_ASSERT_POINTER(sk)

   const char* ptr;
   uint32_t sz = buffer.size();

   U_INTERNAL_DUMP("sz = %u", sz)

   if (sz == 0)
      {
      if (USocketExt::read(sk, buffer, U_SINGLE_READ, UServer_Base::timeoutMS, request_read_timeout) == false) U_RETURN(false);

loop: sz = buffer.size();

      U_INTERNAL_DUMP("sz = %u", sz)
      }

   ptr = buffer.data();

   if (scanfHeaderResponse(ptr, sz) == false) U_RETURN(false);

   // check for HTTP response line: HTTP/1.n 100 Continue

   U_INTERNAL_DUMP("U_http_info.nResponseCode = %d", U_http_info.nResponseCode)

   if (UNLIKELY(U_http_info.nResponseCode == HTTP_CONTINUE))
      {
      /**
       * During the course of an HTTP 1.1 client sending a request to a server, the server might respond with
       * an interim "100 Continue" response. This means the server has received the first part of the request,
       * and can be used to aid communication over slow links. In any case, all HTT 1.1 clients must handle the
       * 100 response correctly (perhaps by just ignoring it). The "100 Continue" response is structured like
       * any HTTP response, i.e. consists of a status line, optional headers, and a blank line. Unlike other
       * responses, it is always followed by another complete, final response. Example:
       *
       * HTTP/1.0 100 Continue
       * [blank line here]
       * HTTP/1.0 200 OK
       * Date: Fri, 31 Dec 1999 23:59:59 GMT
       * Content-Type: text/plain
       * Content-Length: 42
       * some-footer: some-value
       * another-footer: another-value
       * [blank line here]
       * abcdefghijklmnoprstuvwxyz1234567890abcdef
       *
       * To handle this, a simple HTTP 1.1 client might read one response from the socket;
       * if the status code is 100, discard the first response and read the next one instead
       */

      U_INTERNAL_ASSERT_EQUALS(u_get_unalignedp32(buffer.c_pointer(U_http_info.startHeader)), U_MULTICHAR_CONSTANT32('\r','\n','\r','\n'))

      U_http_info.startHeader += U_CONSTANT_SIZE(U_CRLF2);

      if (sz == U_http_info.startHeader)
         {
         buffer.setEmpty();

         if (USocketExt::read(sk, buffer, U_SINGLE_READ, UServer_Base::timeoutMS, request_read_timeout) == false) U_RETURN(false);
         }
      else
         {
         buffer.moveToBeginDataInBuffer(U_http_info.startHeader);
         }

      U_http_info.startHeader   =
      U_http_info.nResponseCode = 0;

      goto loop;
      }

   U_INTERNAL_ASSERT_MINOR(U_http_info.startHeader, sz)

   ptr = buffer.c_pointer(U_http_info.startHeader);

   U_INTERNAL_ASSERT_EQUALS(u_get_unalignedp16(ptr), U_MULTICHAR_CONSTANT16('\r','\n'))

   sz = u_findEndHeader1(ptr+U_CONSTANT_SIZE(U_CRLF), sz-U_http_info.startHeader-U_CONSTANT_SIZE(U_CRLF));

   if (sz != U_NOT_FOUND) sz += U_http_info.startHeader-U_CONSTANT_SIZE(U_CRLF);
   else
      {
      sz = USocketExt::readWhileNotToken(sk, buffer, U_CONSTANT_TO_PARAM(U_CRLF2), UServer_Base::timeoutMS);

      if (sz == U_NOT_FOUND) U_RETURN(false);
      }

   U_INTERNAL_ASSERT_MINOR(sz, buffer.size())

   U_http_info.endHeader    = sz + U_CONSTANT_SIZE(U_CRLF2); // NB: U_http_info.endHeader includes also the blank line...
   U_http_info.startHeader +=      U_CONSTANT_SIZE(U_CRLF);

   U_INTERNAL_DUMP("U_http_info.startHeader(%u) = %.20S U_http_info.endHeader(%u) = %.20S",
                    U_http_info.startHeader, buffer.c_pointer(U_http_info.startHeader),
                    U_http_info.endHeader,   buffer.c_pointer(U_http_info.endHeader))

   U_RETURN(true);
}

__pure const char* UHTTP::getHeaderValuePtr(const UString& request, const char* name, uint32_t name_len, bool nocase)
{
   U_TRACE(0, "UHTTP::getHeaderValuePtr(%V,%.*S,%u,%b)", request.rep, name_len, name, name_len, nocase)

   if (U_http_info.endHeader)
      {
      return UStringExt::getValueFromName(request, U_http_info.startHeader,
                                                   U_http_info.endHeader - U_CONSTANT_SIZE(U_CRLF2) - U_http_info.startHeader, name, name_len, nocase);
      }

   U_RETURN((const char*)0);
}

__pure const char* UHTTP::getHeaderValuePtr(const char* name, uint32_t name_len, bool nocase)
{
   U_TRACE(0, "UHTTP::getHeaderValuePtr(%.*S,%u,%b)", name_len, name, name_len, nocase)

#ifndef U_HTTP2_DISABLE
   if (U_http_version == '2')
      {
      UHTTP2::pConnection->itable.hash = u_hash_ignore_case((unsigned char*)name, name_len);

      *UClientImage_Base::request = UHTTP2::pConnection->itable.at(name, name_len);

      return (*UClientImage_Base::request ? UClientImage_Base::request->data() : (const char*)0);
      }
#endif

   return getHeaderValuePtr(*UClientImage_Base::request, name, name_len, nocase);
}

U_NO_EXPORT bool UHTTP::readDataChunked(USocket* sk, UString* pbuffer, UString& body)
{
   U_TRACE(0, "UHTTP::readDataChunked(%p,%V,%V)", sk, pbuffer->rep, body.rep)

   U_ASSERT(body.empty())
   U_INTERNAL_ASSERT_EQUALS(U_line_terminator_len, 2)

   U_INTERNAL_DUMP("U_http_data_chunked = %b", U_http_data_chunked)

   if (U_http_data_chunked == false)
      {
      const char* chunk_ptr = getHeaderValuePtr(*pbuffer, U_CONSTANT_TO_PARAM("Transfer-Encoding"), true);

      if (chunk_ptr)
         {
         if (UString::str_chunked->equal(chunk_ptr, U_CONSTANT_SIZE("chunked")) == false) U_RETURN(false);

         U_http_flag |= HTTP_IS_DATA_CHUNKED;
         }
      }

   if (U_http_data_chunked)
      {
            char* out;
      const char* inp;
      const char* ptr;
      const char* end;

      /**
       * If a server wants to start sending a response before knowing its total length (like with long script output),
       * it might use the simple chunked transfer-encoding, which breaks the complete response into smaller chunks and
       * sends them in series. You can identify such a response because it contains the "Transfer-Encoding: chunked"
       * header. All HTTP 1.1 clients must be able to receive chunked messages. A chunked message body contains a
       * series of chunks, followed by a line with a single "0" (zero), followed by optional footers (just like headers),
       * and a blank line. Each chunk consists of two parts: a line with the size of the chunk data, in hex, possibly
       * followed by a semicolon and extra parameters you can ignore (none are currently standard), and ending with
       * CRLF, the data itself, followed by CRLF. An example:
       *
       * HTTP/1.1 200 OK
       * Date: Fri, 31 Dec 1999 23:59:59 GMT
       * Content-Type: text/plain
       * Transfer-Encoding: chunked
       * [blank line here]
       * 1a; ignore-stuff-here
       * abcdefghijklmnopqrstuvwxyz
       * 10
       * 1234567890abcdef
       * 0
       * some-footer: some-value
       * another-footer: another-value
       * [blank line here]
       *
       * Note the blank line after the last footer. The length of the text data is 42 bytes (1a + 10, in hex),
       * and the data itself is 'abcdefghijklmnopqrstuvwxyz1234567890abcdef'. The footers should be treated like
       * headers, as if they were at the top of the response. The chunks can contain any binary data, and may
       * be much larger than the examples here. The size-line parameters are rarely used, but you should at
       * least ignore them correctly. Footers are also rare, but might be appropriate for things like checksums
       * or digital signatures
       */

      U_INTERNAL_ASSERT_DIFFERS(U_http_info.endHeader, 0)

      uint32_t chunkSize, count = pbuffer->find(U_CRLF2, U_http_info.endHeader, U_CONSTANT_SIZE(U_CRLF2));

      if (count == U_NOT_FOUND) count = USocketExt::readWhileNotToken(sk, *pbuffer, U_CONSTANT_TO_PARAM(U_CRLF2), U_SSL_TIMEOUT_MS);

      if (count == U_NOT_FOUND) U_RETURN(false);

      count += U_CONSTANT_SIZE(U_CRLF2); // NB: the message include also the blank line...

      U_INTERNAL_DUMP("count = %u U_http_info.endHeader = %u", count, U_http_info.endHeader)

      if (count <= U_http_info.endHeader) U_RETURN(false);

      U_http_info.clength = (count - U_http_info.endHeader);

      U_INTERNAL_DUMP("U_http_info.clength = %u", U_http_info.clength)

      body.setBuffer(U_http_info.clength);

      out = body.data();
      end = pbuffer->c_pointer(count);
      inp = pbuffer->c_pointer(U_http_info.endHeader);

      do {
         // Decode the hexadecimal chunk size into an understandable number

         U_INTERNAL_DUMP("inp = %.20S", inp)

         while (u__isspace(*inp)) { ++inp; }

         if (inp > end) chunkSize = 0;
         else
            {
            for (ptr = inp; u__isxdigit(*inp); ++inp) {}

            chunkSize = u_hex2int(ptr, inp);
            }

         U_INTERNAL_DUMP("chunkSize = %u", chunkSize)

         // The last chunk is followed by zero or more trailers, followed by a blank line

         if (chunkSize == 0)
            {
            body.size_adjust(body.distance(out));

            U_INTERNAL_DUMP("body = %V", body.rep)

            if (body) U_RETURN(true);

            U_RETURN(false);
            }

         U_INTERNAL_DUMP("inp = %.20S", inp)

         if (*++inp != '\n')
            {
            // discard the rest of the line

            inp = (const char*) U_SYSCALL(memchr, "%p,%C,%p", inp, '\n', pbuffer->remain(inp));

            if (UNLIKELY(inp == 0)) U_RETURN(false);
            }

         ++inp;

         U_MEMCPY(out, inp, chunkSize);

         inp += chunkSize + U_CONSTANT_SIZE(U_CRLF);
         out += chunkSize;
         }
      while (inp <= end);
      }

   U_RETURN(false);
}

U_NO_EXPORT bool UHTTP::readBodyRequest()
{
   U_TRACE_NO_PARAM(0, "UHTTP::readBodyRequest()")

   U_ASSERT(UClientImage_Base::body->empty())
   U_INTERNAL_ASSERT_EQUALS(U_line_terminator_len, 2)

   uint32_t body_byte_read = UClientImage_Base::request->size() - U_http_info.endHeader;

   U_INTERNAL_DUMP("UClientImage_Base::request->size() = %u body_byte_read = %u Content-Length = %u U_http_data_chunked = %b",
                    UClientImage_Base::request->size(),     body_byte_read, U_http_info.clength,    U_http_data_chunked)

   if (U_http_info.clength > body_byte_read)
      {
      if (U_http_info.clength > limit_request_body)
         {
         UClientImage_Base::setCloseConnection();

         U_http_info.nResponseCode = HTTP_ENTITY_TOO_LARGE;

         setResponse();

         U_RETURN(false);
         }

      // NB: check for 'Expect: 100-continue' (as curl does)...

      if (body_byte_read == 0 &&
          UClientImage_Base::request->find("Expect: 100-continue", U_http_info.startHeader,
                           U_CONSTANT_SIZE("Expect: 100-continue"), U_http_info.endHeader - U_CONSTANT_SIZE(U_CRLF2) - U_http_info.startHeader) != U_NOT_FOUND &&
          USocketExt::write(UServer_Base::csocket, U_CONSTANT_TO_PARAM("HTTP/1.1 100 Continue\r\n\r\n"), UServer_Base::timeoutMS) == false)
         {
         U_INTERNAL_ASSERT_EQUALS(U_http_version, '1')

         U_RETURN(false);
         }

      if (UServer_Base::startParallelization(UServer_Base::num_client_for_parallelization))
         {
parallelization: // parent
         UServer_Base::csocket->close();

         U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;

         U_RETURN(false);
         }

      // NB: wait for other data to complete the read of the request...

      if (USocketExt::read(UServer_Base::csocket, *UClientImage_Base::request, U_http_info.clength - body_byte_read, U_SSL_TIMEOUT_MS, request_read_timeout) == false)
         {
         U_INTERNAL_DUMP("UClientImage_Base::request->size() = %u Content-Length = %u", UClientImage_Base::request->size(), U_http_info.clength)

         if (UServer_Base::csocket->isTimeout() &&
             UServer_Base::startParallelization())
            {
            goto parallelization; // parent
            }

         U_RETURN(false);
         }
      }
   else if (U_http_info.clength == 0)
      {
      if (U_http_version == '1')
         {
         // HTTP/1.1 compliance: no missing Content-Length on POST requests

         UClientImage_Base::setCloseConnection();

         U_http_info.nResponseCode = HTTP_LENGTH_REQUIRED;

         setResponse();

         U_RETURN(false);
         }

      if (readDataChunked(UServer_Base::csocket, UClientImage_Base::request, *UClientImage_Base::body)) U_RETURN(true);

      U_INTERNAL_DUMP("U_http_data_chunked = %b", U_http_data_chunked)

      if (U_http_data_chunked) U_RETURN(false);

      // wait for other data (max 256k)...

      if (USocketExt::read(UServer_Base::csocket, *UClientImage_Base::request, 256 * 1024, UServer_Base::timeoutMS, request_read_timeout) == false)
         {
         if (UServer_Base::csocket->isTimeout())
            {
            UClientImage_Base::setCloseConnection();

            U_http_info.nResponseCode = HTTP_CLIENT_TIMEOUT;

            setResponse();
            }

         U_RETURN(false);
         }

      U_http_info.clength = (UClientImage_Base::request->size() - U_http_info.endHeader);
      }

   U_INTERNAL_DUMP("U_http_info.clength = %u", U_http_info.clength)

   U_INTERNAL_ASSERT_MAJOR(U_http_info.clength, 0)

   *UClientImage_Base::body = UClientImage_Base::request->substr(U_http_info.endHeader, U_http_info.clength);

   UClientImage_Base::setRequestNoCache();

   U_RETURN(true);
}

bool UHTTP::readBodyResponse(USocket* sk, UString* pbuffer, UString& body)
{
   U_TRACE(0, "UHTTP::readBodyResponse(%p,%V,%V)", sk, pbuffer->rep, body.rep)

   U_ASSERT(body.empty())
   U_INTERNAL_ASSERT_EQUALS(U_line_terminator_len, 2)
   U_INTERNAL_ASSERT_DIFFERS(U_http_info.endHeader, 0)

   if (U_http_info.clength)
      {
      uint32_t body_byte_read = (pbuffer->size() - U_http_info.endHeader);

      U_INTERNAL_DUMP("pbuffer->size() = %u body_byte_read = %u Content-Length = %u", pbuffer->size(), body_byte_read, U_http_info.clength)

      if (U_http_info.clength > body_byte_read)
         {
         // NB: wait for other data to complete the read of the response...

         if (USocketExt::read(sk, *pbuffer, U_http_info.clength - body_byte_read, U_SSL_TIMEOUT_MS, request_read_timeout) == false)
            {
            U_INTERNAL_DUMP("pbuffer->size() = %u body_byte_read = %u Content-Length = %u", pbuffer->size(), pbuffer->size() - U_http_info.endHeader, U_http_info.clength)

            U_RETURN(false);
            }
         }

      body = pbuffer->substr(U_http_info.endHeader, U_http_info.clength);

      U_RETURN(true);
      }

   if (readDataChunked(sk, pbuffer, body)) U_RETURN(true);

   U_RETURN(false);
}

void UHTTP::setHostname(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHTTP::setHostname(%.*S,%u)", len, ptr, len)

   // The difference between HTTP_HOST and U_HTTP_VHOST is that HTTP_HOST can include the «:PORT» text, and U_HTTP_VHOST only the name

   U_http_info.host = ptr;
   U_http_host_len  =
   U_http_host_vlen = len;

   U_INTERNAL_DUMP("U_http_host_len = %u U_HTTP_HOST = %.*S", U_http_host_len, U_HTTP_HOST_TO_TRACE)

   // hostname[:port]

   for (const char* endptr = ptr+len; ptr < endptr; ++ptr)
      {
      if (*ptr == ':')
         {
         U_http_host_vlen = ptr-U_http_info.host;

         break;
         }
      }

   U_INTERNAL_DUMP("U_http_host_vlen = %u U_HTTP_VHOST = %.*S", U_http_host_vlen, U_HTTP_VHOST_TO_TRACE)
}

U_NO_EXPORT inline void UHTTP::setRange(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHTTP::setRange(%.*S,%u)", len, ptr, len)

   U_http_range_len  = len;
   U_http_info.range = ptr;

   U_INTERNAL_DUMP("Range = %.*S", U_HTTP_RANGE_TO_TRACE)
}

U_NO_EXPORT inline void UHTTP::setCookie(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHTTP::setCookie(%.*S,%u)", len, ptr, len)

   U_http_info.cookie     = ptr;
   U_http_info.cookie_len = len;

   U_INTERNAL_DUMP("Cookie(%u): = %.*S", U_http_info.cookie_len, U_HTTP_COOKIE_TO_TRACE)
}

U_NO_EXPORT inline void UHTTP::setAccept(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHTTP::setAccept(%.*S,%u)", len, ptr, len)

   U_http_info.accept = ptr;
   U_http_accept_len  = len;

   U_INTERNAL_DUMP("Accept: = %.*S", U_HTTP_ACCEPT_TO_TRACE)
}

U_NO_EXPORT inline void UHTTP::setReferer(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHTTP::setReferer(%.*S,%u)", len, ptr, len)

   U_http_info.referer     = ptr;
   U_http_info.referer_len = len;

   U_INTERNAL_DUMP("Referer(%u): = %.*S", U_http_info.referer_len, U_HTTP_REFERER_TO_TRACE)
}

U_NO_EXPORT inline void UHTTP::setUpgrade(const char* ptr)
{
   U_TRACE(0, "UHTTP::setUpgrade(%p)", ptr)

   if (u_get_unalignedp16(ptr) != U_MULTICHAR_CONSTANT16('h','2') &&
           u__strncasecmp(ptr, U_CONSTANT_TO_PARAM("websocket")) == 0)
      {
      U_http_flag |= HTTP_IS_REQUEST_NOSTAT;

      U_INTERNAL_DUMP("U_http_websocket_len = %u U_http_is_request_nostat = %b", U_http_websocket_len, U_http_is_request_nostat)
      }
}

U_NO_EXPORT inline void UHTTP::setUserAgent(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHTTP::setUserAgent(%.*S,%u)", len, ptr, len)

   U_http_info.user_agent     = ptr;
   U_http_info.user_agent_len = len;

   U_INTERNAL_DUMP("User-Agent: = %.*S", U_HTTP_USER_AGENT_TO_TRACE)
}

U_NO_EXPORT inline void UHTTP::setConnection(const char* ptr)
{
   U_TRACE(0, "UHTTP::setConnection(%p)", ptr)

   char c = u__toupper(*ptr);

   if (c == 'C')
      {
      if (u__strncasecmp(ptr+1, U_CONSTANT_TO_PARAM("lose")) == 0) UClientImage_Base::setCloseConnection();
      }
   else if (c == 'K')
      {
      if (u__strncasecmp(ptr+1, U_CONSTANT_TO_PARAM("eep-alive")) == 0)
         {
         U_http_flag |= HTTP_IS_KEEP_ALIVE;

         U_INTERNAL_DUMP("U_http_keep_alive = %b", U_http_keep_alive)
         }
      }
   /*
   else if (c == 'U')
      {
      if (u__strncasecmp(ptr+1, U_CONSTANT_TO_PARAM("pgrade")) == 0) {}
      }
   */
}

U_NO_EXPORT inline void UHTTP::setContentType(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHTTP::setContentType(%.*S,%u)", len, ptr, len)

   U_http_content_type_len  = len;
   U_http_info.content_type = ptr;

   U_INTERNAL_DUMP("Content-Type(%u): = %.*S", U_http_content_type_len, U_HTTP_CTYPE_TO_TRACE)
}

U_NO_EXPORT inline void UHTTP::setContentLength(const char* ptr1, const char* ptr2)
{
   U_TRACE(0, "UHTTP::setContentLength(%p,%p)", ptr1, ptr2)

   U_http_info.clength = u_strtoul(ptr1, ptr2);

   U_INTERNAL_DUMP("Content-Length: = %.*S U_http_info.clength = %u", ptr2-ptr1, ptr1, U_http_info.clength)

   U_INTERNAL_ASSERT_EQUALS(U_http_info.clength, (uint32_t)strtoul(ptr1, 0, 10))
}

U_NO_EXPORT inline void UHTTP::setAcceptEncoding(const char* ptr)
{
   U_TRACE(0, "UHTTP::setAcceptEncoding(%p)", ptr)

   ptr = (u_get_unalignedp32(ptr) == U_MULTICHAR_CONSTANT32('g','z','i','p')
               ?                     ptr
               : (const char*)u_find(ptr, 30, U_CONSTANT_TO_PARAM("gzip")));

   if (                   ptr &&
       u_get_unalignedp32(ptr) != U_MULTICHAR_CONSTANT32(';','q','=','0'))
      {
      U_http_flag |= HTTP_IS_ACCEPT_GZIP;

      U_INTERNAL_DUMP("U_http_is_accept_gzip = %b", U_http_is_accept_gzip)
      }
}

U_NO_EXPORT inline void UHTTP::setAcceptLanguage(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHTTP::setAcceptLanguage(%.*S,%u)", len, ptr, len)

   U_http_accept_language_len  = len;
   U_http_info.accept_language = ptr;

   U_INTERNAL_DUMP("Accept-Language: = %.*S", U_HTTP_ACCEPT_LANGUAGE_TO_TRACE)
}

U_NO_EXPORT inline void UHTTP::setIfModSince(const char* ptr)
{
   U_TRACE(0, "UHTTP::setIfModSince(%p)", ptr)

   U_http_info.if_modified_since = UTimeDate::getSecondFromTime(ptr, true);

   U_INTERNAL_DUMP("If-Modified-Since = %u", U_http_info.if_modified_since)
}

U_NO_EXPORT void UHTTP::checkIPClient()
{
   U_TRACE_NO_PARAM(0, "UHTTP::checkIPClient()")

   U_INTERNAL_ASSERT_MAJOR(U_http_ip_client_len, 0)

   uint32_t n = 0;

   do {
      if (u__islitem(U_http_info.ip_client[n])) break;
      }
   while (++n < (uint32_t)U_http_ip_client_len);

   U_INTERNAL_DUMP("ip_client = %.*S", n, U_http_info.ip_client)

   if (u_isIPAddr(UClientImage_Base::bIPv6, U_http_info.ip_client, n))
      {
      U_INTERNAL_ASSERT_MINOR(n, U_INET_ADDRSTRLEN)
      U_INTERNAL_ASSERT_EQUALS(UServer_Base::client_address, UServer_Base::csocket->cRemoteAddress.pcStrAddress)

      U_MEMCPY(UServer_Base::client_address, U_http_info.ip_client, n);

      UServer_Base::client_address[(UServer_Base::client_address_len = n)] = '\0';

      U_INTERNAL_DUMP("UServer_Base::client_address = %.*S", U_CLIENT_ADDRESS_TO_TRACE)
      }
}

U_NO_EXPORT inline void UHTTP::setXRealIP(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHTTP::setXRealIP(%.*S,%u)", len, ptr, len)

   U_http_ip_client_len  = len;
   U_http_info.ip_client = ptr;

   U_INTERNAL_DUMP("X-Real-IP: = %.*S", U_HTTP_IP_CLIENT_TO_TRACE)

   checkIPClient();
}

U_NO_EXPORT inline void UHTTP::setXForwardedFor(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHTTP::setXForwardedFor(%.*S,%u)", len, ptr, len)

   U_http_ip_client_len  = len;
   U_http_info.ip_client = ptr;

   U_INTERNAL_DUMP("X-Forwarded-For: = %.*S", U_HTTP_IP_CLIENT_TO_TRACE)

   checkIPClient();
}

U_NO_EXPORT inline void UHTTP::setXHttpForwardedFor(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHTTP::setXHttpForwardedFor(%.*S,%u)", len, ptr, len)

   U_http_ip_client_len  = len;
   U_http_info.ip_client = ptr;

   U_INTERNAL_DUMP("X-Http-X-Forwarded-For: = %.*S", U_HTTP_IP_CLIENT_TO_TRACE)

   checkIPClient();
}

#define SET_POINTER_CHECK_REQUEST_FOR_HEADER                                       \
   if (LIKELY(u_get_unalignedp16(pn) == U_MULTICHAR_CONSTANT16(':',' '))) pn += 2; \
   else                                                                            \
      {                                                                            \
      do { ++pn; } while (u__isblank(*pn));                                        \
                                                                                   \
      if (UNLIKELY(pn >= pend)) return;                                            \
      }                                                                            \
                                                                                   \
   pn = (const char*) memchr((ptr1 = pn), '\r', pend - pn);                        \
                                                                                   \
   if (UNLIKELY(pn == 0)) return;

U_NO_EXPORT void UHTTP::checkRequestForHeader()
{
   U_TRACE_NO_PARAM(0, "UHTTP::checkRequestForHeader()")

   U_INTERNAL_DUMP("U_line_terminator_len = %d", U_line_terminator_len)

   U_INTERNAL_ASSERT(*UClientImage_Base::request)
   U_INTERNAL_ASSERT_DIFFERS(U_http_method_type, 0)
   U_INTERNAL_ASSERT_MAJOR(U_http_info.endHeader, 0)
   U_INTERNAL_ASSERT_EQUALS(U_line_terminator_len, U_CONSTANT_SIZE(U_CRLF))

   // --------------------------------
   // check in header request for:
   // --------------------------------
   // "Host: ..."
   // "Range: ..."
   // "Accept: ..."
   // "Cookie: ..."
   // "Referer: ..."
   // "X-Real-IP: ..."
   // "User-Agent: ..."
   // "Connection: ..."
   // "Content-Type: ..."
   // "Content-Length: ..."
   // "HTTP2-Settings: ..."
   // "X-Forwarded-For: ..."
   // "Accept-Encoding: ..."
   // "Accept-Language: ..."
   // "If-Modified-Since: ..."
   // "Sec-WebSocket-Key: ..."
   // --------------------------------

   const char* pend;
   const char* ptr = UClientImage_Base::request->data();
   
   if (U_http_info.endHeader)
      {
      U_INTERNAL_ASSERT_EQUALS(U_ClientImage_data_missing, false)

      pend = ptr + U_http_info.endHeader - U_CONSTANT_SIZE(U_CRLF);
      }
   else
      {
      U_ClientImage_data_missing = true;

      pend = ptr + UClientImage_Base::request->size();

      u_put_unalignedp16((void*)pend, U_MULTICHAR_CONSTANT16('\r','\n'));
      }

   for (const char* pn = ptr + U_http_info.startHeader; pn < pend; pn += U_CONSTANT_SIZE(U_CRLF))
      {
      const char* p;
      const char* p1;
      unsigned char c;
      const char* ptr1;
      uint32_t remain = pend - pn;

      U_INTERNAL_DUMP("u__isheader(%C) = %b pn(%u) = %.*S", *pn, u__isheader(*pn), remain, remain, pn)

      if (u__isheader(*pn) == false)
         {
         pn = (const char*) memchr(pn, '\r', remain);

         if (UNLIKELY(pn == 0)) return; // NB: we can have too much advanced...

         goto next;
         }

      p = pn;

      if (pn[4] == ':') // "Host:"
         {
         pn += 4;

         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('H','o','s','t'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            setHostname(ptr1, pn-ptr1);

            goto next;
            }
         }
      else if (pn[5] == ':') // "Range:"
         {
         pn += 5;

         if (u_get_unalignedp32(p)   == U_MULTICHAR_CONSTANT32('R','a','n','g') &&
             u_get_unalignedp32(p+7) == U_MULTICHAR_CONSTANT32('b','y','t','e'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            setRange(ptr1+U_CONSTANT_SIZE("bytes="), pn-ptr1-U_CONSTANT_SIZE("bytes="));

            goto next;
            }
         }
      else if (pn[6] == ':') // "Cookie|Accept:"
         {
         pn += 6;

         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('C','o','o','k'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            setCookie(ptr1, pn-ptr1);

            goto next;
            }

         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('A','c','c','e'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            setAccept(ptr1, pn-ptr1);

            goto next;
            }
         }
      else if (pn[7] == ':') // "Referer|Upgrade|Cookie2:"
         {
         pn += 7;

         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('R','e','f','e'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            setReferer(ptr1, pn-ptr1);

            goto next;
            }

         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('U','p','g','r') ||
             u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('u','p','g','r'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            U_INTERNAL_DUMP("Upgrade: = %.*S", pn-ptr1, ptr1)

            setUpgrade(ptr1);

            goto next;
            }

         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('C','o','o','k'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            U_INTERNAL_DUMP("Cookie2: = %.*S", pn-ptr1, ptr1)

            goto next;
            }
         }
#  ifndef U_LOG_DISABLE
      else if (pn[9] == ':') // "X-Real-IP:"
         {
         pn += 9;

         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('X','-','R','e'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            setXRealIP(ptr1, pn-ptr1);

            goto next;
            }
         }
#  endif
      else if (pn[10] == ':') // "Connection|User-Agent:"
         {
         pn += 10;

         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('U','s','e','r'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            setUserAgent(ptr1, pn-ptr1);

            goto next;
            }

         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('C','o','n','n'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            setConnection(ptr1);

            goto next;
            }
         }
      else if (pn[12] == ':') // "Content-Type:"
         {
         pn += 12;

         if (u_get_unalignedp32(p)   == U_MULTICHAR_CONSTANT32('C','o','n','t') &&
             u_get_unalignedp32(p+7) == U_MULTICHAR_CONSTANT32('-','T','y','p'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            setContentType(ptr1, pn-ptr1);

            goto next;
            }
         }
      else if (pn[14] == ':') // "Content-Length:|HTTP2-Settings:"
         {
         pn += 14;

         if (u_get_unalignedp32(p)   == U_MULTICHAR_CONSTANT32('C','o','n','t') &&
             u_get_unalignedp32(p+7) == U_MULTICHAR_CONSTANT32('-','L','e','n'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            setContentLength(ptr1, pn);

            goto next;
            }

         // HTTP2-Settings

         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('H','T','T','P') ||
             u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('h','t','t','p'))
            {
            p1 = p+4;

            if (u_get_unalignedp64(p1) == U_MULTICHAR_CONSTANT64('2','-','S','e','t','t','i','n') ||
                u_get_unalignedp64(p1) == U_MULTICHAR_CONSTANT64('2','-','s','e','t','t','i','n'))
               {
               SET_POINTER_CHECK_REQUEST_FOR_HEADER

               U_INTERNAL_DUMP("HTTP2-Settings: = %.*S", pn-ptr1, ptr1)

#           ifndef U_HTTP2_DISABLE
               U_http2_settings_len     = pn-ptr1;
               UHTTP2::upgrade_settings =    ptr1;

               U_http_version             = '2';
               U_ClientImage_data_missing = true;
#           endif

               goto next;
               }
            }
         }
      else if (pn[15] == ':') // "Accept-Encoding/Language|X-Forwarded-For:"
         {
         pn += 15;

         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('A','c','c','e'))
            {
            p1 = p+6;

            if (u_get_unalignedp32(p1) == U_MULTICHAR_CONSTANT32('-','E','n','c'))
               {
               SET_POINTER_CHECK_REQUEST_FOR_HEADER

               U_INTERNAL_DUMP("Accept-Encoding: = %.*S", pn-ptr1, ptr1)

#           ifdef USE_LIBZ
               setAcceptEncoding(ptr1);
#           endif

               goto next;
               }

            if (u_get_unalignedp32(p1) == U_MULTICHAR_CONSTANT32('-','L','a','n'))
               {
               SET_POINTER_CHECK_REQUEST_FOR_HEADER

               setAcceptLanguage(ptr1, pn-ptr1);

               goto next;
               }
            }

#     ifndef U_LOG_DISABLE
         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('X','-','F','o'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            setXForwardedFor(ptr1, pn-ptr1);

            goto next;
            }
#     endif
         }
      else if (pn[17] == ':') // "If-Modified-Since|Sec-WebSocket-Key:"
         {
         pn += 17;

         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('I','f','-','M'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            setIfModSince(ptr1);

            goto next;
            }

         if (u_get_unalignedp64(p)   == U_MULTICHAR_CONSTANT64('S','e','c','-','W','e','b','S') &&
             u_get_unalignedp64(p+8) == U_MULTICHAR_CONSTANT64('o','c','k','e','t','-','K','e'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            U_http_websocket_len         = pn-ptr1;
            UWebSocket::upgrade_settings =    ptr1;

            U_INTERNAL_DUMP("Sec-WebSocket-Key: = %.*S", U_http_websocket_len, UWebSocket::upgrade_settings)

            goto next;
            }
         }
#  ifndef U_LOG_DISABLE
      else if (pn[22] == ':') // "X-Http-X-Forwarded-For:"
         {
         pn += 22;

         if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('X','-','H','t'))
            {
            SET_POINTER_CHECK_REQUEST_FOR_HEADER

            setXHttpForwardedFor(ptr1, pn-ptr1);

            goto next;
            }
         }
#  endif
      else
         {
         pn = (const char*) memchr(pn, ':', remain);

         if (UNLIKELY(pn == 0)) return; // NB: we can have too much advanced...
         }

      SET_POINTER_CHECK_REQUEST_FOR_HEADER

      switch (u__toupper(*p++))
         {
         case 'C':
            {
            if (memcmp(p, U_CONSTANT_TO_PARAM("ontent-")) == 0)
               {
               p1 = p+8;
               c  = u__toupper(*(p1-1));

               if (c == 'T' &&
                   memcmp(p1, U_CONSTANT_TO_PARAM("ype")) == 0)
                  {
                  setContentType(ptr1, pn-ptr1);
                  }
               else if (c == 'L' &&
                        memcmp(p1, U_CONSTANT_TO_PARAM("ength")) == 0)
                  {
                  setContentLength(ptr1, pn);
                  }
               }
            else if (memcmp(p, U_CONSTANT_TO_PARAM("onnection")) == 0)
               {
               setConnection(ptr1);
               }
            else if (memcmp(p, U_CONSTANT_TO_PARAM("ookie")) == 0)
               {
               U_INTERNAL_ASSERT_DIFFERS(p[5], '2') // "Cookie2"

               setCookie(ptr1, pn-ptr1);
               }
            }
         break;

         case 'A':
            {
            if (memcmp(p, U_CONSTANT_TO_PARAM("ccept")) == 0)
               {
               if (p[5] == '-')
                  {
                  p1 = p+7;
                  c  = u__toupper(*(p1-1));

                  if (c == 'E' &&
                      memcmp(p1, U_CONSTANT_TO_PARAM("ncoding")) == 0)
                     {
                     U_INTERNAL_DUMP("Accept-Encoding: = %.*S", pn-ptr1, ptr1)

                     setAcceptEncoding(ptr1);
                     }
                  else if (c == 'L' &&
                           memcmp(ptr1, U_CONSTANT_TO_PARAM("anguage")) == 0)
                     {
                     setAcceptLanguage(ptr1, pn-ptr1);
                     }
                  }
               else
                  {
                  setAccept(ptr1, pn-ptr1);
                  }
               }
            }
         break;

         case 'H':
            {
            if (memcmp(p, U_CONSTANT_TO_PARAM("ost")) == 0 ||
                (u__toupper(p[0]) == 'O'                   &&
                 u__toupper(p[1]) == 'S'                   &&
                 u__toupper(p[2]) == 'T'))
               {
               setHostname(ptr1, pn-ptr1);
               }
            }
         break;

         case 'U':
            {
            if (u__toupper(p[4]) == 'A'                                            &&
                u_get_unalignedp32(p)   == U_MULTICHAR_CONSTANT32('s','e','r','-') &&
                u_get_unalignedp32(p+5) == U_MULTICHAR_CONSTANT32('g','e','n','t'))
               {
               setUserAgent(ptr1, pn-ptr1);
               }
            else if (memcmp(p, U_CONSTANT_TO_PARAM("pgrade")) == 0)
               {
               U_INTERNAL_DUMP("Upgrade: = %.*S", pn-ptr1, ptr1)

               setUpgrade(ptr1);
               }
            }
         break;

         case 'I': // If-Modified-Since
            {
            if (u__toupper(p[2])  == 'M'                                                            &&
                u__toupper(p[11]) == 'S'                                                            &&
                u_get_unalignedp16(p)    == U_MULTICHAR_CONSTANT16('f','-')                         &&
                u_get_unalignedp64(p+3)  == U_MULTICHAR_CONSTANT64('o','d','i','f','i','e','d','-') &&
                u_get_unalignedp32(p+12) == U_MULTICHAR_CONSTANT32('i','n','c','e'))
               {
               setIfModSince(ptr1);
               }
            }
         break;

         case 'R':
            {
            if (u_get_unalignedp32(p)      == U_MULTICHAR_CONSTANT32('a','n','g','e') &&
                u_get_unalignedp32(ptr1)   == U_MULTICHAR_CONSTANT32('b','y','t','e') &&
                u_get_unalignedp16(ptr1+4) == U_MULTICHAR_CONSTANT16('s','='))
               {
               setRange(ptr1+U_CONSTANT_SIZE("bytes="), pn-ptr1-U_CONSTANT_SIZE("bytes="));
               }
            else if (u_get_unalignedp32(p)   == U_MULTICHAR_CONSTANT32('e','f','e','r') &&
                     u_get_unalignedp16(p+4) == U_MULTICHAR_CONSTANT16('e','r'))
               {
               setReferer(ptr1, pn-ptr1);
               }
            }
         break;

#     ifndef U_LOG_DISABLE
         case 'X':
            {
            if (p[0] == '-')
               {
               c = u__toupper(p[1]);

               // TODO: check of CLIENT-IP, WEBPROXY-REMOTE-ADDR, FORWARDED...

               if (c == 'F') // "X-Forwarded-For"
                  {
                  if (u__toupper(p[11]) == 'F'                            &&
                      memcmp(p+2,  U_CONSTANT_TO_PARAM("orwarded-")) == 0 &&
                      u_get_unalignedp16(p+12) == U_MULTICHAR_CONSTANT16('o','r'))
                     {
                     setXForwardedFor(ptr1, pn-ptr1);
                     }
                  }
               else if (c == 'R') // "X-Real-IP"
                  {
                  if (u__toupper(p[6]) == 'I' &&
                      u__toupper(p[7]) == 'P' &&
                      u_get_unalignedp32(p+2) == U_MULTICHAR_CONSTANT32('e','a','l','-'))
                     {
                     setXRealIP(ptr1, pn-ptr1);
                     }
                  }
               else if (c == 'H') // "X-Http-X-Forwarded-For"
                  {
                  if (u__toupper(p[2])  == 'T'                            &&
                      u__toupper(p[3])  == 'T'                            &&
                      u__toupper(p[4])  == 'P'                            &&
                      u__toupper(p[18]) == 'F'                            &&
                      memcmp(p+9,  U_CONSTANT_TO_PARAM("orwarded-")) == 0 &&
                      u_get_unalignedp16(p+19) == U_MULTICHAR_CONSTANT16('o','r'))
                     {
                     setXHttpForwardedFor(ptr1, pn-ptr1);
                     }
                  }
               }
            }
         break;
#     endif
         }

next: U_INTERNAL_DUMP("char (after cr/newline) = %C U_http_version = %C U_ClientImage_data_missing = %b", pn[2], U_http_version, U_ClientImage_data_missing)

      if (U_http_info.endHeader == 0 &&
          u_get_unalignedp32(pn) == U_MULTICHAR_CONSTANT32('\r','\n','\r','\n'))
         {
         U_http_info.endHeader = pn-ptr + U_CONSTANT_SIZE(U_CRLF2); // NB: U_http_info.endHeader includes also the blank line...

         U_INTERNAL_DUMP("endHeader(%u) = %.20S", U_http_info.endHeader, ptr+U_http_info.endHeader)

         U_INTERNAL_ASSERT(U_ClientImage_data_missing)

#     ifndef U_HTTP2_DISABLE
         if (U_http_version != '2')
#     endif
         U_ClientImage_data_missing = false;

         return;
         }
      }
}

// manage dynamic page request (CGI - C/ULib Servlet Page - RUBY - PHP - PYTHON)

bool UHTTP::checkIfSourceHasChangedAndCompileUSP()
{
   U_TRACE_NO_PARAM(1, "UHTTP::checkIfSourceHasChangedAndCompileUSP()")

#if defined(DEBUG) && !defined(U_STATIC_ONLY)
   checkForPathName();

   *suffix = file->getSuffix();

   U_INTERNAL_DUMP("pathname = %V file = %.*S suffix = %V", pathname->rep, U_FILE_TO_TRACE(*file), suffix->rep)

   const char* ptr  = file->getPathRelativ();
   uint32_t sz, len = file->getPathRelativLen() - (sz = suffix->size());

   U_INTERNAL_DUMP("ptr(%u) = %#.*S", len, len, ptr)

   U_INTERNAL_ASSERT_MAJOR(len, 0)

   char buffer[U_PATH_MAX];
   UServletPage* usp_page = (UServletPage*)file_data->ptr;

   if (suffix->empty())
      {
      struct stat st;

      if (cache_file->at(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%.*s.usp"), U_FILE_TO_TRACE(*file))) &&
          U_SYSCALL(stat, "%S,%p", buffer, &st) == 0                                                      &&
          st.st_mtime > file_data->mtime)
         {
         U_INTERNAL_ASSERT_POINTER(usp_page)

         U_INTERNAL_DUMP("st.st_mtime = %#3D file_data->mtime = %#3D", st.st_mtime, file_data->mtime)

         // NB: dlopen() fail if the module name is not prefixed with "./"...

         (void) u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("./%.*s.%s"), len, ptr, U_LIB_SUFFIX);

         usp_page->UDynamic::close();

         usp_page->runDynamicPage = 0;

         goto compile;
         }

      U_RETURN(true);
      }

   if (suffix->equal(U_CONSTANT_TO_PARAM("usp")))
      {
      if (U_HTTP_QUERY_STREQ("_nav_")) U_RETURN(false);

      if (usp_page) U_RETURN(true);

      U_NEW(UHTTP::UServletPage, usp_page, UHTTP::UServletPage);

      // NB: dlopen() fail if the module name is not prefixed with "./"...

      (void) u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("./%.*s%s"), len, ptr, U_LIB_SUFFIX);

      // NB: we must avoid the point '.' before the suffix...

      ++sz;
      --len;

compile:
      if (compileUSP(ptr, len) == false)
         {
err:     setInternalError();

         if (*suffix) delete usp_page;

         U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;

         U_RETURN(false);
         }

      if (usp_page->UDynamic::load(buffer) == false) goto err;

      char run_dynamic_page[128];
      UString file_name = UStringExt::basename(file->getPath());

      (void) u__snprintf(run_dynamic_page, sizeof(run_dynamic_page), U_CONSTANT_TO_PARAM("runDynamicPage_%.*s"), file_name.size() - sz, file_name.data());

      usp_page->runDynamicPage = (vPFi)(*usp_page)[run_dynamic_page];

      if (usp_page->runDynamicPage == 0) goto err;

      file_data->ptr = usp_page;
      }
#endif

   U_RETURN(true);
}

bool UHTTP::runCGI(bool set_environment)
{
   U_TRACE(0, "UHTTP::runCGI(%b)", set_environment)

   UHTTP::ucgi* cgi = (UHTTP::ucgi*)file_data->ptr;

   U_INTERNAL_ASSERT_POINTER(cgi)

   U_INTERNAL_DUMP("cgi->dir = %S cgi->environment_type = %d cgi->interpreter = %S", cgi->dir, cgi->environment_type, cgi->interpreter)

   // NB: we can't use the relativ path because after we call chdir()...

   UString command(U_CAPACITY), path(U_CAPACITY);

   // NB: we can't use U_HTTP_URI_TO_TRACE because this function can be called by SSI...

   uint32_t len = u__strlen(cgi->dir, __PRETTY_FUNCTION__);

   path.snprintf(U_CONSTANT_TO_PARAM("%w/%.*s/%s"), len, cgi->dir, cgi->dir + len + 1);

   U_INTERNAL_DUMP("path = %V", path.rep)

   if (cgi->interpreter) command.snprintf(U_CONSTANT_TO_PARAM("%s %v"), cgi->interpreter, path.rep);
   else           (void) command.assign(path);

   // ULIB facility: check if present form data and convert them in parameters for shell script...

   if (cgi->environment_type == U_SHELL) setCGIShellScript(command);

   U_INTERNAL_ASSERT_POINTER(pcmd)

   pcmd->setCommand(command);

   if (set_environment)
      {
      U_ASSERT(UClientImage_Base::environment->empty())

      // NB: process the CGI request with fork....

      if (getCGIEnvironment(*UClientImage_Base::environment, cgi->environment_type) == false ||
          UServer_Base::startParallelization()) // parent of parallelization
         {
         goto next;
         }
      }

   if (processCGIRequest(pcmd, cgi)) // NB: in case of failure we have already the response...
      {
next: U_DUMP("UServer_Base::isParallelizationChild() = %b UServer_Base::isParallelizationParent() = %b",
              UServer_Base::isParallelizationChild(),     UServer_Base::isParallelizationParent())

      U_INTERNAL_DUMP("U_http_info.nResponseCode = %d UClientImage_Base::wbuffer(%u) = %V UClientImage_Base::body(%u) = %V",
                       U_http_info.nResponseCode,     UClientImage_Base::wbuffer->size(), UClientImage_Base::wbuffer->rep,
                       UClientImage_Base::body->size(), UClientImage_Base::body->rep)

      if (set_environment == false                                   ||
          (U_ClientImage_parallelization != U_PARALLELIZATION_PARENT &&
           processCGIOutput(cgi->environment_type == U_SHELL, false)))
         {
         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

U_NO_EXPORT bool UHTTP::runDynamicPage()
{
   U_TRACE_NO_PARAM(0, "UHTTP::runDynamicPage()")

   U_INTERNAL_ASSERT_POINTER(file_data)
   U_INTERNAL_ASSERT_DIFFERS(U_http_method_type, 0)
   U_INTERNAL_ASSERT_EQUALS(u_is_usp(mime_index), false)
   U_INTERNAL_ASSERT_EQUALS(u_is_cgi(mime_index), false)
   U_INTERNAL_ASSERT_EQUALS(mime_index, file_data->mime_index)

#ifdef USE_PHP
   if (u_is_php(mime_index))
      {
      U_INTERNAL_ASSERT_POINTER(php_embed->runPHP)

      (void) php_embed->runPHP();

      goto next;
      }
#endif
#ifdef USE_RUBY
   if (u_is_ruby(mime_index))
      {
      U_INTERNAL_ASSERT_POINTER(ruby_embed->runRUBY)

      (void) ruby_embed->runRUBY();

      goto next;
      }
#endif
#ifdef USE_PYTHON
   if (u_is_python(mime_index))
      {
      U_INTERNAL_ASSERT_POINTER(python_embed->runPYTHON)

      (void) python_embed->runPYTHON();

      goto next;
      }
#endif
#ifdef HAVE_LIBTCC
   if (u_is_csp(mime_index))
      {
      UCServletPage* csp = (UCServletPage*)file_data->ptr;

      U_INTERNAL_ASSERT_POINTER(csp)
      U_INTERNAL_ASSERT_POINTER(csp->prog_main)

      U_SET_MODULE_NAME(csp);
      
      const char* argv[4096];
      uint32_t i, n = processForm();

      U_INTERNAL_ASSERT_MINOR(n, 4096)

      for (i = 0; i < n; ++i) argv[i] = (*form_name_value)[i].c_str();
                              argv[i] = 0;

      (void) csp->prog_main(n, argv);

      UClientImage_Base::wbuffer->size_adjust();

      U_RESET_MODULE_NAME;

      goto next;
      }
#endif

   U_RETURN(false);

#if defined(USE_RUBY) || defined(USE_PHP) || defined(HAVE_LIBTCC) || defined(USE_PYTHON)
next:
   U_DUMP("UServer_Base::isParallelizationChild() = %b UServer_Base::isParallelizationParent() = %b",
           UServer_Base::isParallelizationChild(),     UServer_Base::isParallelizationParent())

   if (U_ClientImage_parallelization != U_PARALLELIZATION_PARENT)
      {
      U_INTERNAL_DUMP("U_http_info.nResponseCode = %d UClientImage_Base::wbuffer(%u) = %V UClientImage_Base::body(%u) = %V",
                       U_http_info.nResponseCode,     UClientImage_Base::wbuffer->size(), UClientImage_Base::wbuffer->rep,
                       UClientImage_Base::body->size(), UClientImage_Base::body->rep)

      if (processCGIOutput(false, false) == false)
         {
         switch (U_http_info.nResponseCode)
            {
            case HTTP_NOT_FOUND:    setNotFound();           break;
            case HTTP_BAD_METHOD:   setBadMethod();          break;
            case HTTP_BAD_REQUEST:  setBadRequest();         break;
            case HTTP_UNAVAILABLE:  setServiceUnavailable(); break;
            case HTTP_UNAUTHORIZED: setUnAuthorized();       break;
            default:                setInternalError();      break;
            }

         U_RETURN(false);
         }
      }

   U_RETURN(true);
#endif
}

U_NO_EXPORT bool UHTTP::callService()
{
   U_TRACE_NO_PARAM(0, "UHTTP::callService()")

   pathname->setBuffer(U_CAPACITY);

   const char* psuffix = u_getsuffix(U_FILE_TO_PARAM(*file));

   if (psuffix) pathname->snprintf(U_CONSTANT_TO_PARAM("%.*s"),    U_FILE_TO_TRACE(*file));
   else         pathname->snprintf(U_CONSTANT_TO_PARAM("%.*s.%s"), U_FILE_TO_TRACE(*file), U_LIB_SUFFIX);

   file->setPath(*pathname);

   while (file->stat() == false)
      {
      if (psuffix) U_RETURN(false);

      pathname->setBuffer(U_CAPACITY);

      pathname->snprintf(U_CONSTANT_TO_PARAM("%.*s.usp"), U_FILE_TO_TRACE(*file));

      file->setPath(*pathname);

      psuffix = (const char*)U_INT2PTR(0xffff);
      }

   U_INTERNAL_DUMP("U_http_is_nocache_file = %b", U_http_is_nocache_file)

   if (U_http_is_nocache_file == false)
      {
      manageDataForCache(UStringExt::basename(file->getPath()));

      if (file_data)
         {
         U_DEBUG("Called service not in cache: %.*S - inotify %s enabled", U_FILE_TO_TRACE(*file), UServer_Base::handler_inotify ? "is" : "NOT")

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

bool UHTTP::callService(const UString& path) // NB: it is used also by server_plugin_ssi...
{
   U_TRACE(0, "UHTTP::callService(%V)", path.rep)

   file->setPath(path, UClientImage_Base::environment);

   if (isFileInCache() == false &&
       callService()   == false)
      {
      // NB: st_ino => stat() ok...

      if (file->st_ino == 0) setNotFound();
      else
         {
         U_INTERNAL_ASSERT_EQUALS(file_data, 0)

         setInternalError();
         }

      U_RETURN(false);
      }

   U_INTERNAL_ASSERT_POINTER(file_data)

   if (u_is_usp(file_data->mime_index))
      {
      int mime_index_save = mime_index;

      ((UServletPage*)(file_data->ptr))->runDynamicPage(0);

      mime_index = mime_index_save;
      }
   else
      {
      U_INTERNAL_ASSERT(u_is_cgi(file_data->mime_index))

      if (runCGI(false) == false) U_RETURN(false);
      }

   U_RETURN(true);
}

bool UHTTP::handlerCache()
{
   U_TRACE_NO_PARAM(0, "UHTTP::handlerCache()")

   U_INTERNAL_ASSERT(U_ClientImage_request_is_cached)

#if !defined(U_CACHE_REQUEST_DISABLE) || defined(U_SERVER_CHECK_TIME_BETWEEN_REQUEST) 
   const char* ptr = UClientImage_Base::request->data();

   switch (u_get_unalignedp32(ptr))
      {
      case U_MULTICHAR_CONSTANT32('g','e','t',' '):
      case U_MULTICHAR_CONSTANT32('G','E','T',' '): U_http_method_type = HTTP_GET;  U_http_method_num = 0; break;
      case U_MULTICHAR_CONSTANT32('h','e','a','d'):
      case U_MULTICHAR_CONSTANT32('H','E','A','D'): U_http_method_type = HTTP_HEAD; U_http_method_num = 1; break;

      default:
         {
         U_INTERNAL_DUMP("U_ClientImage_advise_for_parallelization = %b", U_ClientImage_advise_for_parallelization)

         if (U_ClientImage_advise_for_parallelization == false) U_RETURN(false);
         }
      }

   if (U_ClientImage_pipeline)
      {
      U_INTERNAL_ASSERT(UClientImage_Base::size_request <= UClientImage_Base::request->size())

      const char* ptr1 = ptr+UClientImage_Base::size_request;

      if (isValidMethod(ptr1) == false                                      ||
          (  isValidRequest(ptr1, UClientImage_Base::size_request) == false &&
           u_findEndHeader1(ptr1, UClientImage_Base::size_request) == U_NOT_FOUND))
         {
         U_RETURN(false);
         }
      }

# ifdef USE_LIBZ
   const char* p;
   unsigned char c;
   bool http_gzip = false;
   char http_keep_alive = false;

   U_INTERNAL_DUMP("UClientImage_Base::uri_offset = %u", UClientImage_Base::uri_offset)

   U_INTERNAL_ASSERT_RANGE(1,UClientImage_Base::uri_offset,64)

   for (uint32_t pos = UClientImage_Base::uri_offset + U_http_info.startHeader + U_CONSTANT_SIZE(" HTTP/1.1\r\n"),
                 end = UClientImage_Base::request->size(); pos < end; ++pos)
      {
      U_INTERNAL_DUMP("pos = %.20S", UClientImage_Base::request->c_pointer(pos))

      c = *(p = (ptr + pos));

      U_INTERNAL_DUMP("c = %C", c)

      if (c == '\r') goto next2;

      if (c == 'C')
         {
         if (u_get_unalignedp64(p+1) == U_MULTICHAR_CONSTANT64('o','n','n','e','c','t','i','o'))
            {
            p += U_CONSTANT_SIZE("Connection: ");

            U_INTERNAL_DUMP("Connection: = %.20S", p)

            if (u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('k','e','e','p') ||
                u_get_unalignedp32(p) == U_MULTICHAR_CONSTANT32('K','e','e','p'))
               {
               http_keep_alive = true;

               U_INTERNAL_DUMP("http_keep_alive = %b", http_keep_alive)
               }

            if (U_http_is_response_gzip == false || http_gzip) goto next2;

            p += U_CONSTANT_SIZE("keep-alive\r");

            pos = (p - ptr);
            }

         goto next1;
         }

      if (U_http_is_response_gzip == false || http_gzip) goto next1;

      if (c == 'A' &&
          u_get_unalignedp64(p+1) == U_MULTICHAR_CONSTANT64('c','c','e','p','t','-','E','n'))
         {
         p += U_CONSTANT_SIZE("Accept-Encoding:");

         U_INTERNAL_DUMP("Accept-Encoding: = %.20S", p+1)

         p = (const char*) u_find(p, 30, U_CONSTANT_TO_PARAM("gzip"));

         if (p &&
             u_get_unalignedp32(p+4) != U_MULTICHAR_CONSTANT32(';','q','=','0'))
            {
            http_gzip = true;

            U_INTERNAL_DUMP("http_gzip = %b", http_gzip)
            }
         }
next1:
      do { ++pos; } while (pos < end && ptr[pos] != '\n');
      }
next2:
   U_INTERNAL_DUMP("U_http_version = %C http_keep_alive = %b U_http_keep_alive = %b http_gzip = %b U_http_is_accept_gzip = %b",
                    U_http_version,     http_keep_alive,     U_http_keep_alive,     http_gzip,     U_http_is_accept_gzip)

   if (http_keep_alive != U_http_keep_alive ||
       (http_gzip == false && U_http_is_response_gzip))
      {
      U_RETURN(false);
      }
#  endif

# ifdef U_SERVER_CHECK_TIME_BETWEEN_REQUEST
   if (U_ClientImage_advise_for_parallelization) U_RETURN(true);
# endif

   if (UClientImage_Base::csfd > 0)
      {
      U_INTERNAL_ASSERT(U_http_sendfile)

      UClientImage_Base::setSendfile(UClientImage_Base::csfd, range_start, range_size);
      }

   UClientImage_Base::setHeaderForResponse(6+29+2+12+2); // Date: Wed, 20 Jun 2012 11:43:17 GMT\r\nServer: ULib\r\n

#ifndef U_LOG_DISABLE
   if (UServer_Base::apache_like_log)
      {
      U_INTERNAL_ASSERT_EQUALS(iov_vec[0].iov_len, 0)

        agent_offset =
      referer_offset = 0;

      iov_vec[0].iov_base = (caddr_t) UServer_Base::client_address;
      iov_vec[0].iov_len  =           UServer_Base::client_address_len;

      resetApacheLikeLog();

#  ifdef U_ALIAS
      iov_vec[4].iov_len  = U_http_info.startHeader;
#  else
      iov_vec[4].iov_len  = U_http_info.uri_len;
#  endif
      iov_vec[4].iov_base = (caddr_t) UClientImage_Base::cbuffer;
      }
#endif

   U_RETURN(true);
#endif

   U_RETURN(false);
}

int UHTTP::handlerREAD()
{
   U_TRACE_NO_PARAM(0, "UHTTP::handlerREAD()")

   U_INTERNAL_DUMP("UClientImage_Base::request(%u) = %V U_http_version = %C", UClientImage_Base::request->size(), UClientImage_Base::request->rep, U_http_version)

   U_INTERNAL_ASSERT_DIFFERS(U_http_version, '2')
   U_INTERNAL_ASSERT(*UClientImage_Base::request)

   // ------------------------------
   // U_http_info.uri
   // ....
   // U_http_info.nResponseCode
   // ....
   // ------------------------------
   U_HTTP_INFO_RESET(0);

   if (readHeaderRequest() == false)
      {
      if (U_ClientImage_data_missing) U_RETURN(U_PLUGIN_HANDLER_FINISHED);

      if (UNLIKELY(UServer_Base::csocket->isClosed())) U_RETURN(U_PLUGIN_HANDLER_ERROR);

      // -----------------------------------------------------
      // HTTP/1.1 compliance:
      // -----------------------------------------------------
      // Sends 501 for request-method != (GET|POST|HEAD|...)
      // Sends 505 for protocol != HTTP/1.[0-1]
      // Sends 400 for broken Request-Line
      // -----------------------------------------------------

      if (U_http_method_type == 0)
         {
         if (UClientImage_Base::request->isText() == false)
            {
            UClientImage_Base::abortive_close();

            U_RETURN(U_PLUGIN_HANDLER_ERROR);
            }

         U_http_info.nResponseCode = HTTP_NOT_IMPLEMENTED;
         }
      else if (U_http_version == 0 &&
               U_http_info.uri_len &&
               U_http_info.query == 0)
         {
         U_http_info.nResponseCode = HTTP_VERSION;
         }
      else
         {
         setBadRequest();

         U_RETURN(U_PLUGIN_HANDLER_FINISHED);
         }

      setResponse();

      UClientImage_Base::resetPipelineAndSetCloseConnection();

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }

   U_INTERNAL_ASSERT_DIFFERS(U_http_method_type, 0)

   if (U_http_info.endHeader)
      {
      checkRequestForHeader();

      if (U_ClientImage_data_missing) U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }

   U_INTERNAL_DUMP("U_http_host_len = %u U_HTTP_HOST = %.*S", U_http_host_len, U_HTTP_HOST_TO_TRACE)

   if (U_http_host_len == 0)
      {
      if (U_http_version == '1') // HTTP 1.1 want header "Host: " ...
         {
         setBadRequest();

         U_RETURN(U_PLUGIN_HANDLER_FINISHED);
         }
      }
#ifndef U_LOG_DISABLE
   else if (UServer_Base::public_address && // NB: as protection from DNS rebinding attack web servers can reject HTTP requests with an unrecognized Host header...
            ((U_http_host_len - U_http_host_vlen) > (1 + 5) || // NB: ':' + 0-65536
             u_isHostName(U_HTTP_VHOST_TO_PARAM) == false))
      {
      setBadRequest();

      U_SRV_LOG("WARNING: unrecognized header <Host> in request: %.*S", U_HTTP_VHOST_TO_TRACE);

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }
#endif

   if (U_http_method_type == HTTP_OPTIONS)
      {
      U_http_info.nResponseCode = HTTP_OPTIONS_RESPONSE;

      setResponse();

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }

   UClientImage_Base::size_request = (U_http_info.endHeader ? U_http_info.endHeader
                                                            : U_http_info.startHeader + U_CONSTANT_SIZE(U_CRLF2));

   return manageRequest();
}

int UHTTP::manageRequest()
{
   U_TRACE_NO_PARAM(0, "UHTTP::manageRequest()")

   // check the HTTP message

   U_INTERNAL_DUMP("U_ClientImage_request = %d %B", U_ClientImage_request, U_ClientImage_request)

   U_ASSERT(UClientImage_Base::isRequestNotFound())

   U_INTERNAL_DUMP("U_http_info.clength = %u", U_http_info.clength)

   if (isGETorHEAD() == false)
      {
      U_http_flag |= HTTP_IS_NOCACHE_FILE | HTTP_IS_REQUEST_NOSTAT;

      U_INTERNAL_DUMP("U_http_is_nocache_file = %b U_http_is_request_nostat = %b", U_http_is_nocache_file, U_http_is_request_nostat)

#  ifndef U_SERVER_CAPTIVE_PORTAL
      if (U_http_info.clength ||
          isPOSTorPUTorPATCH())
         {
         bool result_read_body;

#     ifndef U_HTTP2_DISABLE
         if (U_http_version == '2') result_read_body = UHTTP2::readBodyRequest();
         else
#     endif
         result_read_body = readBodyRequest();

         U_INTERNAL_ASSERT_EQUALS(U_ClientImage_data_missing, false)

         if (result_read_body == false)
            {
            U_INTERNAL_DUMP("UServer_Base::csocket->isClosed() = %b UClientImage_Base::wbuffer(%u) = %V",
                             UServer_Base::csocket->isClosed(),     UClientImage_Base::wbuffer->size(), UClientImage_Base::wbuffer->rep)

            if (UNLIKELY(UServer_Base::csocket->isClosed())) U_RETURN(U_PLUGIN_HANDLER_ERROR);

            if (UClientImage_Base::wbuffer->empty()) setBadRequest();

            U_RETURN(U_PLUGIN_HANDLER_FINISHED);
            }

#     ifndef U_HTTP2_DISABLE
         if (U_http_version != '2')
#     endif
         UClientImage_Base::size_request += U_http_info.clength;
         }
#  endif
      }

   // manage alias uri

#ifdef U_ALIAS
   if (maintenance_mode_page &&
       U_HTTP_URI_STREQ("favicon.ico") == false)
      {
      (void) UClientImage_Base::request_uri->assign(U_HTTP_URI_TO_PARAM);

      (void) alias->append(*maintenance_mode_page);

      goto set_uri;
      }

   // manage virtual host

   if (virtual_host &&
       U_http_host_vlen)
      {
      // Host: hostname[:port]

      alias->setBuffer(1 + U_http_host_vlen + U_http_info.uri_len);

      alias->snprintf(U_CONSTANT_TO_PARAM("/%.*s"), U_HTTP_VHOST_TO_TRACE);
      }

   if (valias)
      {
      UString str;
      int i, n = valias->size();

      // Ex: /admin /admin.html

      U_DUMP("valias = %S", UObject2String(*valias))

      for (i = 0; i < n; i += 2)
         {
         int flag = 0;

         str = (*valias)[i];

         const char* ptr = str.data();

         int len = str.size();

         if (ptr[0] == '!')
            {
            ++ptr;
            --len;

            flag = FNM_INVERT;
            }

         if (U_HTTP_URI_DOSMATCH(ptr, len, flag))
            {
            str = (*valias)[i+1];

            U_INTERNAL_DUMP("ALIAS MATCH: %.*S => %V", U_HTTP_URI_TO_TRACE, str.rep)

            // NB: this is exclusive with global alias...

            (void) UClientImage_Base::request_uri->assign(U_HTTP_URI_TO_PARAM);

            (void) alias->append(str);

            break;
            }
         }

      if (i < n)
         {
         if (UStringExt::endsWith(U_STRING_TO_PARAM(str), U_CONSTANT_TO_PARAM("nostat"))) U_http_flag |= HTTP_IS_REQUEST_NOSTAT; // NOT a static page...

         U_INTERNAL_DUMP("U_http_is_request_nostat = %b", U_http_is_request_nostat)
         }
      }

   // manage global alias

   if (global_alias                            &&
       UClientImage_Base::request_uri->empty() &&
       u_getsuffix(U_HTTP_URI_TO_PARAM) == 0)
      {
      (void) UClientImage_Base::request_uri->assign(U_HTTP_URI_TO_PARAM);

      (void) alias->append(*global_alias);
      }

   if (*alias)
      {
      U_INTERNAL_ASSERT_EQUALS(alias->first_char(), '/')

      if (U_http_info.uri[0] != '/') alias->clear(); // reset alias
      else
         {
         if (UClientImage_Base::request_uri->empty())
            {
            U_INTERNAL_ASSERT(virtual_host)

            (void) UClientImage_Base::request_uri->assign(U_HTTP_URI_TO_PARAM);

            (void) alias->append(U_HTTP_URI_TO_PARAM);
            }

set_uri: U_http_info.uri     = alias->data();
         U_http_info.uri_len = alias->size();

         U_SRV_LOG("ALIAS: URI request changed to: %V", alias->rep);

#     ifdef DEBUG
         if (UClientImage_Base::request_uri->findWhiteSpace(0) != U_NOT_FOUND) U_ERROR("Request URI has space: %V", UClientImage_Base::request_uri->rep);
#     endif
         }
      }
#endif

   // ...process the HTTP message

#ifdef U_THROTTLING_SUPPORT
   if (isGETorHEAD() &&
       UServer_Base::checkThrottling() == false)
      {
      setServiceUnavailable();

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }
#endif

   U_http_info.nResponseCode = HTTP_OK;

   U_INTERNAL_DUMP("U_http_method_type = %B old_path_len = %u URI = %.*S u_cwd(%u) = %.*S",
                    U_http_method_type,     old_path_len, U_HTTP_URI_TO_TRACE, u_cwd_len, u_cwd_len, u_cwd)

   U_INTERNAL_ASSERT_MAJOR(U_http_info.uri_len, 0)

   pathname->setBuffer(u_cwd_len + U_http_info.uri_len);

   old_path_len = U_http_info.uri_len-1;

   if (old_path_len == 0) file_data = 0;
   else
      {
      checkFileInCache(U_http_info.uri+1, old_path_len);

      if (file_data)
         {
         UClientImage_Base::setRequestInFileCache();

         goto file_in_cache;
         }
      }

   setPathName();

#ifndef U_SERVER_CAPTIVE_PORTAL
   if (nocache_file_mask &&
       UServices::dosMatchWithOR(UStringExt::basename(U_HTTP_URI_TO_PARAM), U_STRING_TO_PARAM(*nocache_file_mask), 0))
      {
      U_http_flag |= HTTP_IS_NOCACHE_FILE | HTTP_IS_REQUEST_NOSTAT;

      U_INTERNAL_DUMP("U_http_is_nocache_file = %b U_http_is_request_nostat = %b", U_http_is_nocache_file, U_http_is_request_nostat)
      }
#endif

   checkPath();

   if (UClientImage_Base::isRequestNotFound())
      {
#  ifndef U_SERVER_CAPTIVE_PORTAL
      U_INTERNAL_DUMP("U_http_is_nocache_file = %b", U_http_is_nocache_file)

      if (U_http_is_nocache_file) goto manage;

      // ------------------------------------------------------------------------------
      // NB: if status is 'file not found' and we have virtual host
      //     we check if it is present as shared file (without the virtual host prefix)
      // ------------------------------------------------------------------------------
      const char* ptr = pathname->c_pointer(u_cwd_len);

#    ifdef U_ALIAS
      U_INTERNAL_DUMP("virtual_host = %b U_http_host_vlen = %u U_http_is_request_nostat = %b", virtual_host, U_http_host_vlen, U_http_is_request_nostat)

      if (virtual_host &&
          U_http_host_vlen)
         {
         if (U_http_is_request_nostat)
            {
            // NB: U_HTTP_URI_TO_PARAM can be different from request_uri...

            U_INTERNAL_DUMP("UClientImage_Base::request_uri = %V", UClientImage_Base::request_uri->rep)

            file->path_relativ_len = UClientImage_Base::request_uri->size();

            U_MEMCPY(ptr, UClientImage_Base::request_uri->data(), file->path_relativ_len);
            }
         else
            {
            file->path_relativ_len = U_http_info.uri_len - U_http_host_vlen - 1;

#        ifdef U_APEX_ENABLE
            (void) U_SYSCALL(apex_memmove, "%p,%p,%u", (void*)ptr, ptr + 1 + U_http_host_vlen, file->path_relativ_len);
#        else
            (void) U_SYSCALL(     memmove, "%p,%p,%u", (void*)ptr, ptr + 1 + U_http_host_vlen, file->path_relativ_len);
#        endif
            }

         if (checkPath(file->path_relativ_len)) goto manage;
         }
#    endif

      // URI requested can be URL encoded...

      if (u_isUrlEncoded(U_HTTP_URI_TO_PARAM, false) &&
              u_isBase64(U_HTTP_URI_TO_PARAM) == false)
         {
         file->path_relativ_len = u_url_decode(U_HTTP_URI_TO_PARAM, (unsigned char*)ptr);

         if (checkPath(file->path_relativ_len)) goto manage;
         }
#  endif
      }

   // NB: apply rewrite rule if requested and if status is 'file forbidden or not exist'...

#if !defined(U_SERVER_CAPTIVE_PORTAL) && defined(U_ALIAS) && defined(USE_LIBPCRE)
   if (vRewriteRule &&
       U_ClientImage_request <= UClientImage_Base::FORBIDDEN)
      {
      processRewriteRule();
      }
#endif

   // ----------------------------------------------------------------------------------------
   // NB: in general at this point, after checkPath(), we can have as status:
   // ----------------------------------------------------------------------------------------
   // 1) the directory DOCUMENT_ROOT need to be processed (it can be forbidden)
   // 2) the file is forbidden or it is not present in DOCUMENT_ROOT
   // 3) the file is present in FILE CACHE with/without content (stat() cache)
   // 4) the file is not present in FILE CACHE or in DOCUMENT_ROOT 
   // 5) the file is not present in FILE CACHE or in DOCUMENT_ROOT and it is already processed
   // ----------------------------------------------------------------------------------------

#ifndef U_SERVER_CAPTIVE_PORTAL
manage:
#endif
   U_INTERNAL_DUMP("file_data = %p U_ClientImage_request = %B U_http_info.flag = %.8S", file_data, U_ClientImage_request, U_http_info.flag)

#if defined(DEBUG) && !defined(U_STATIC_ONLY)
   if (file_data == 0         &&
       U_http_is_nocache_file &&
       file->getSuffix().empty())
      {
      struct stat st;
      char buffer[U_PATH_MAX];
      uint32_t sz = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%.*s.usp"), U_FILE_TO_TRACE(*file));

      if (U_SYSCALL(stat, "%S,%p", buffer, &st) == 0)
         {
         U_DEBUG("Request usp service not in cache: %V - try to compile", pathname->rep)

         (void) pathname->replace(buffer, sz);

         file->setPath(*pathname);

         manageDataForCache(UStringExt::basename(file->getPath()));

         U_INTERNAL_ASSERT_POINTER(file_data)

         UClientImage_Base::setRequestInFileCache();
         }
      }
#endif

   if (file_data &&
       UClientImage_Base::isRequestInFileCache()) // => 3
      {
file_in_cache:
      mime_index = file_data->mime_index;

      U_INTERNAL_DUMP("mime_index(%d) = %C u_is_ssi() = %b", mime_index, mime_index, u_is_ssi(mime_index))

      /**
       * MIME type for dynamic content
       * -----------------------------------------------
       * #define U_usp    '0' // USP (ULib Servlet Page)
       * #define U_csp    '1' // CSP (C    Servlet Page)
       * #define U_cgi    '2' // cgi-bin
       * #define U_ssi    '3' // SSI
       * #define U_php    '4' // PHP    script
       * #define U_ruby   '5' // Ruby   script
       * #define U_perl   '6' // Perl   script
       * #define U_python '7' // Python script
       * -----------------------------------------------
       */

      if (u_is_usp(mime_index))
         {
#     if defined(DEBUG) && !defined(U_STATIC_ONLY)
         if (checkIfSourceHasChangedAndCompileUSP())
#     endif
         {
         UServletPage* usp_page = (UServletPage*)file_data->ptr;

         U_INTERNAL_ASSERT_POINTER(usp_page)
         U_INTERNAL_ASSERT_POINTER(usp_page->runDynamicPage)

         U_SET_MODULE_NAME(usp);

         usp_page->runDynamicPage(0);

         if (U_ClientImage_parallelization != U_PARALLELIZATION_PARENT) setDynamicResponse();

         U_RESET_MODULE_NAME;

         U_RETURN(U_PLUGIN_HANDLER_FINISHED);
         }

#     ifndef U_COVERITY_FALSE_POSITIVE // UNREACHABLE
         if (U_ClientImage_state != U_PLUGIN_HANDLER_ERROR) goto from_cache;

         U_RETURN(U_PLUGIN_HANDLER_FINISHED);
#     endif
         }

      if (u__isdigit(mime_index))
         {
         if (u_is_ssi(mime_index) == false)
            {
            UClientImage_Base::setRequestNoCache();

            if (u_is_cgi(mime_index))
               {
               (void) runCGI(true);

               UClientImage_Base::environment->setEmpty();

               U_RETURN(U_PLUGIN_HANDLER_FINISHED);
               }

            U_INTERNAL_DUMP("U_http_is_request_nostat = %b query(%u) = %.*S", U_http_is_request_nostat, U_http_info.query_len, U_HTTP_QUERY_TO_TRACE)

#        if defined(USE_RUBY) || defined(USE_PHP) || defined(HAVE_LIBTCC) || defined(USE_PYTHON)
            if (U_http_is_request_nostat    == false &&
                U_HTTP_QUERY_STREQ("_nav_") == false &&
                (checkForPathName(), runDynamicPage()))
               {
               U_RETURN(U_PLUGIN_HANDLER_FINISHED);
               }
#        endif
            }

         goto file_exist_and_need_to_be_processed; // NB: if it is not a dynamic page, set status to 'file exist and need to be processed'...
         }

      // NB: check if we can service the content of file directly from cache...

      U_INTERNAL_DUMP("file_data->link = %b file_data->ptr = %p", file_data->link, file_data->ptr)

      if (file_data->link)
         {
         file_data = (UHTTP::UFileCacheData*)file_data->ptr;

         file->st_mode  = file_data->mode;
         file->st_size  = file_data->size;
         file->st_mtime = file_data->mtime;

         U_INTERNAL_DUMP("st_mode = %d st_size = %I st_mtime = %ld", file->st_mode, file->st_size, file->st_mtime)
         }

from_cache:
      if (isGETorHEAD()      == false ||
          isDataFromCache()  == false ||
          processFileCache() == false)
         {
file_exist_and_need_to_be_processed: // NB: if we can't service the content of file directly from cache, set status to 'file exist and need to be processed'...

         checkForPathName();

         UClientImage_Base::setRequestNeedProcessing();
         }

#  if defined(U_HTTP_STRICT_TRANSPORT_SECURITY) || defined(USE_LIBSSL)
      goto need_to_be_processed;
#  else
      goto end;
#  endif
      }

#if defined(U_HTTP_STRICT_TRANSPORT_SECURITY) || defined(USE_LIBSSL)
   if (UClientImage_Base::isRequestNotFound() == false) // => 4
      {
      if (UClientImage_Base::isRequestAlreadyProcessed()) U_RETURN(U_PLUGIN_HANDLER_FINISHED); // => 5

need_to_be_processed:

#  ifdef U_HTTP_STRICT_TRANSPORT_SECURITY // check if the uri requested use HTTP Strict Transport Security to force client to use secure connections only
      if (isUriRequestStrictTransportSecurity())
         {
         // NB: we are in cleartext at the moment, prevent further execution and output

         char redirect_url[32 * 1024];

         // The Strict-Transport-Security header is ignored by the browser when your site is accessed using HTTP;
         // this is because an attacker may intercept HTTP connections and inject the header or remove it.
         // When your site is accessed over HTTPS with no certificate errors, the browser knows your site is HTTPS
         // capable and will honor the Strict-Transport-Security header

         setRedirectResponse(NO_BODY, (const char*)redirect_url,
                             u__snprintf(redirect_url, sizeof(redirect_url), U_CONSTANT_TO_PARAM("%s:/%v%.*s"), U_http_websocket_len ? "wss" : "https",
                             UServer_Base::getIPAddress().rep, U_HTTP_URI_QUERY_TO_TRACE));

         U_SRV_LOG("URI_STRICT_TRANSPORT_SECURITY: request redirected to %S", redirect_url);

         U_RETURN(U_PLUGIN_HANDLER_FINISHED);
         }
#  endif

#  ifdef USE_LIBSSL
      if (isUriRequestNeedCertificate() && // check if the uri requested need a certificate
          UServer_Base::pClientImage->askForClientCertificate() == false)
         {
         U_SRV_LOG("URI_REQUEST_CERT: request denied by mandatory certificate from client");

         setForbidden();

         U_RETURN(U_PLUGIN_HANDLER_FINISHED);
         }

      if (isUriRequestProtected() && // check if the uri requested is protected
          checkUriProtected() == false)
         {
         U_RETURN(U_PLUGIN_HANDLER_FINISHED);
         }
#  else
      ;
#  endif
      }
#endif

#ifndef U_SERVER_CAPTIVE_PORTAL
   U_INTERNAL_DUMP("U_http_websocket_len = %u", U_http_websocket_len)

   if (U_http_websocket_len &&
       UWebSocket::sendAccept() == false)
      {
      setBadRequest();
      }
#endif

#if !defined(U_HTTP_STRICT_TRANSPORT_SECURITY) && !defined(USE_LIBSSL)
end:
#endif
#ifdef DEBUG
   if (file_data                                 &&
       UClientImage_Base::isRequestInFileCache() &&
       UClientImage_Base::isRequestNeedProcessing())
      {
      U_INTERNAL_DUMP("pathname = %V file->getPath() = %V", pathname->rep, file->getPath().rep)

      U_INTERNAL_ASSERT(*pathname)
      U_ASSERT(UStringExt::endsWith(file->getPath(), *pathname))
      U_ASSERT_EQUALS(UClientImage_Base::isRequestNotFound(), false)
      }
#endif

   if (UClientImage_Base::isRequestFileCacheProcessed()) handlerResponse();

   U_RETURN(U_PLUGIN_HANDLER_FINISHED);
}

int UHTTP::processRequest()
{
   U_TRACE_NO_PARAM(1, "UHTTP::processRequest()")

   U_ASSERT(UClientImage_Base::isRequestNeedProcessing())

   if (isGETorHEADorPOST() == false)
      {
      if (isPUT())
         {
         uint32_t sz;
         const char* ptr = UClientImage_Base::getRequestUri(sz);

         if (memcmp(ptr, U_CONSTANT_TO_PARAM("/upload")) == 0)
            {
            U_INTERNAL_ASSERT(*upload_dir)

            /**
             * The PUT method, though not as widely used as the POST method is perhaps the more efficient way of uploading files to a server.
             * This is because in a POST upload the files need to be combined together into a multipart message and this message has to be
             * decoded at the server. In contrast, the PUT method allows you to simply write the contents of the file to the socket connection
             * that is established with the server.
             *
             * When using the POST method, all the files are combined together into a single multipart/form-data type object. This MIME message
             * when transferred to the server, has to be decoded by the server side handler. The decoding process may consume significant amounts
             * of memory and CPU cycles for very large files.
             *
             * The fundamental difference between the POST and PUT requests is reflected in the different meaning of the Request-URI. The URI in a
             * POST request identifies the resource that will handle the enclosed entity. That resource might be a data-accepting process, a gateway
             * to some other protocol, or a separate entity that accepts annotations. In contrast, the URI in a PUT request identifies the entity
             * enclosed with the request
             */

            U_INTERNAL_DUMP("U_http_info.clength = %u", U_http_info.clength)

            if (*UClientImage_Base::body)
               {
               UString dest(U_CAPACITY),
                       basename = UStringExt::basename(ptr, sz);

               dest.snprintf(U_CONSTANT_TO_PARAM("%v/%v"), upload_dir->rep, basename.rep);

               if (UFile::writeTo(dest, *UClientImage_Base::body)) UClientImage_Base::body->clear(); // clean body to avoid writev() in response...
               else                                                U_http_info.nResponseCode = HTTP_INTERNAL_ERROR;

               handlerResponse();

               U_RETURN(U_PLUGIN_HANDLER_FINISHED);
               }

            // ---------------------------------------------------------------------------------------------------------------------------------------------------
            // TODO
            // ---------------------------------------------------------------------------------------------------------------------------------------------------
            // To check how much of a file has transferred, perform the exact same PUT request without the file data and with the header: Content-Range: bytes */*
            //
            // An example request:
            //
            // PUT http://1234.cloud.vimeo.com/upload
            // Host: 1.2.3.4:8080
            // Content-Length: 0
            // Content-Range: bytes */*
            //
            // If this file exists, this will return a response with a HTTP 308 status code and a Range header with the number of bytes on the server.
            //
            // HTTP/1.1 308
            // Content-Length: 0
            // Range: bytes=0-1000
            //
            // If you perform a request like this and find that the returned number is NOT the size of your uploaded file, then it is a good idea to
            // resume where you left off. To resume, perform the same PUT you did before but with an additional header:
            // Content-Range: bytes (last byte on server + 1)-(last byte I will be sending)/(total filesize) 
            //
            // As shown before our uploaded file was 339108 total bytes in size but our "Content-Range: bytes */*" verification call
            // returned a value of 1000. To resume, we would send the following headers with the binary data of our file starting at byte 1001:
            //
            // PUT http://1234.cloud.vimeo.com/upload/video.mp4
            // Host: 1.2.3.4:8080
            // Content-Length: 338108
            // Content-Type: video/mp4
            // Content-Range: bytes 1001-339108/339108
            // .... binary data of your file here ....
            //
            // If all goes well, you will receive a 200 status code. A 400 code means the Content-Range header did not resume from the last byte on the server
            // ---------------------------------------------------------------------------------------------------------------------------------------------------
            }
         }

      U_http_flag |= HTTP_METHOD_NOT_IMPLEMENTED;

      U_INTERNAL_DUMP("U_http_method_not_implemented = %b", U_http_method_not_implemented)

      if (UServer_Base::vplugin_name->last() != *UString::str_http) U_RETURN(U_PLUGIN_HANDLER_GO_ON); // NB: there are other plugin after this...

      U_http_info.nResponseCode = HTTP_NOT_IMPLEMENTED;

      handlerResponse();

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }

   U_INTERNAL_DUMP("U_http_is_nocache_file = %b", U_http_is_nocache_file)

   if ((*UClientImage_Base::body         ||
        U_http_is_nocache_file == false) &&
       UClientImage_Base::isRequestNotFound())
      {
      setNotFound();

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }

   if (isPOST())
      {
      setBadRequest();

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }

   U_INTERNAL_ASSERT(*UClientImage_Base::request)

   ext->setBuffer(U_CAPACITY);

   /**
    * If the browser has to validate a component, it uses the If-None-Match header to pass the ETag back to
    * the origin server. If the ETags match, a 304 status code is returned reducing the response...
    *
    * For me it's enough Last-Modified: ...
    */

// *etag = file->etag();
//
// const char* ptr = getHeaderValuePtr(U_CONSTANT_TO_PARAM("If-None-Match"), false);
//
// if (ptr)
//    {
//    U_INTERNAL_ASSERT_EQUALS(*ptr, '"') // entity-tag
//
//    uint32_t sz = etag->size();
//
//    if (sz &&
//        etag->equal(ptr, sz))
//       {
//       U_http_info.nResponseCode = HTTP_NOT_MODIFIED;
//
//       handlerResponse();
//
//       U_RETURN(U_PLUGIN_HANDLER_FINISHED);
//       }
//    }
//
// ext->snprintf(U_CONSTANT_TO_PARAM("Etag: %v\r\n"), etag->rep));

   bool result;

   if (file->dir())
      {
      // NB: may be we want a directory list...

#  ifndef DEBUG
      if (u_fnmatch(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM("servlet"), 0))
         {
         setForbidden(); // set forbidden error response...
   
         U_RETURN(U_PLUGIN_HANDLER_FINISHED);
         }
#  endif

      // Check if there is an index file (index.html) in the directory... (we check in the CACHE FILE SYSTEM)

      U_INTERNAL_DUMP("query = %.*S", U_HTTP_QUERY_TO_TRACE)

      if (U_http_info.query_len == 0)
         {
         uint32_t sz      = file->getPathRelativLen(), len = UString::str_indexhtml->size();
         const char* ptr  = file->getPathRelativ();
         const char* ptr1 = UString::str_indexhtml->data();

         U_INTERNAL_ASSERT_MAJOR(sz, 0)

         pathname->setBuffer(sz + 1 + len);

         bool broot = (sz == 1 && ptr[0] == '/');

         if (broot) pathname->snprintf(U_CONSTANT_TO_PARAM(     "%.*s"),          len, ptr1);
         else       pathname->snprintf(U_CONSTANT_TO_PARAM("%.*s/%.*s"), sz, ptr, len, ptr1);

         file_data = cache_file->at(*pathname);

         if (file_data)
            {
            // NB: we have an index file (index.html) in the directory...

            if (isDataFromCache()) // NB: check if we have the content of the index file in cache...
               {
               U_http_info.nResponseCode = HTTP_OK;

#           ifdef USE_LIBZ
               if (U_http_is_accept_gzip &&
                   isDataCompressFromCache())
                  {
                  U_http_flag |= HTTP_IS_RESPONSE_GZIP;

                  U_INTERNAL_DUMP("U_http_is_response_gzip = %b", U_http_is_response_gzip)

                  *ext = getHeaderCompressFromCache();

                  *UClientImage_Base::body = getBodyCompressFromCache();
                  }
               else
                  {
                  *ext = getHeaderFromCache();

                  *UClientImage_Base::body = getBodyFromCache();
                  }

               handlerResponse();
#           endif

               U_RETURN(U_PLUGIN_HANDLER_FINISHED);
               }

            file->setPath(*pathname);

            file->st_size  = file_data->size;
            file->st_mode  = file_data->mode;
            file->st_mtime = file_data->mtime;

            goto check_file;
            }
         }

      // now we check the directory...

      if (checkGetRequestIfModified())
         {
         uint32_t sz;

         // check if it's OK to do directory listing via authentication (digest|basic)

         if (processAuthorization() == false) U_RETURN(U_PLUGIN_HANDLER_FINISHED);

         *UClientImage_Base::body = getHTMLDirectoryList();

#     ifdef USE_LIBZ
         if (U_http_is_accept_gzip)
            {
            U_http_flag |= HTTP_IS_RESPONSE_GZIP;

            U_INTERNAL_DUMP("U_http_is_response_gzip = %b", U_http_is_response_gzip)

            (void) ext->append(U_CONSTANT_TO_PARAM("Content-Encoding: gzip\r\n"));

            *UClientImage_Base::body = UStringExt::deflate(*UClientImage_Base::body, 1);
            }
#     endif

         sz = UClientImage_Base::body->size();

         mime_index                = U_unknow;
         U_http_info.nResponseCode = HTTP_OK;

         (void) ext->append(getHeaderMimeType(0, sz, U_CTYPE_HTML));
         }

      handlerResponse();

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }

check_file: // now we check the file...
   errno = 0;

   U_INTERNAL_DUMP("file_data = %p", file_data)

   if (file_data)
      {
      if (file_data->fd == -1)
         {
         result = (file->regular() &&
                   file->open());

         if (result) file_data->fd = file->fd;
         }
      else
         {
         result   = true;
         file->fd = file_data->fd;
         }
      }
   else
      {
      // NB: this can happen (ex: we don't cache usp file)...

      result        = file->open();
      file_data     = file_not_in_cache_data;
      mime_index    = U_unknow;
      file_data->fd = file->fd;
      }

   if (result == false)
      {
      U_INTERNAL_DUMP("errno = %d", errno)

           if (errno == ENOENT) setNotFound();
      else if (errno == EPERM)  setForbidden();
      else                      setServiceUnavailable();

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }

   U_INTERNAL_DUMP("file_data->fd = %d file_data->size = %u file_data->mode = %d file_data->mtime = %ld",
                    file_data->fd,     file_data->size,     file_data->mode,     file_data->mtime)

   errno = 0;

   if (U_SYSCALL(fstat, "%d,%p", file->fd, (struct stat*)file) != 0)
      {
      U_INTERNAL_DUMP("errno = %d", errno)

      if (errno == EBADF &&
          file->open())
         {
         file_data->fd = file->fd;

         if (U_SYSCALL(fstat, "%d,%p", file->fd, (struct stat*)file) != 0) goto error;
         }
      }

   file_data->mode  = file->st_mode;
   file_data->size  = file->st_size;
   file_data->mtime = file->st_mtime;

   U_INTERNAL_DUMP("file->st_mode = %d file->st_size = %u file->st_mtime = %ld", file->st_mode, file->st_size, file->st_mtime)

   U_INTERNAL_ASSERT_EQUALS(file->st_size, file_data->size)

   if (file_data == file_not_in_cache_data ||
       checkGetRequestIfModified())
      {
      if (file_data->size)
         {
         U_INTERNAL_ASSERT_EQUALS(U_http_sendfile, false)

         processGetRequest();

         U_INTERNAL_DUMP("U_http_is_nocache_file = %b", U_http_is_nocache_file)

         if (U_http_is_nocache_file)
            {
            file->close();

            file_data->fd = -1;
            }

         U_RETURN(U_PLUGIN_HANDLER_FINISHED);
         }

error:
      file->close();

      file_data->fd = -1;
      }

   handlerResponse();

   U_RETURN(U_PLUGIN_HANDLER_FINISHED);
}

void UHTTP::setEndRequestProcessing()
{
   U_TRACE_NO_PARAM(0, "UHTTP::setEndRequestProcessing()")

   ext->clear();

#ifdef U_ALIAS
   alias->clear();
#endif

   // if any clear form data

   if (form_name_value->size())
      {
      form_name_value->clear();

      // if any delete temporary directory

      uint32_t sz = tmpdir->size();

      if (sz)
         {
         // NB: if we have a parallelization only the child at the end of the request can remove the temporary directory... 

         U_INTERNAL_DUMP("U_ClientImage_parallelization = %d", U_ClientImage_parallelization)

         if (U_ClientImage_parallelization <= U_PARALLELIZATION_CHILD)
            {
            (void) UFile::rmdir(*tmpdir, true);

            U_SRV_LOG("temporary directory removed: %.*s", sz, tmpdir->data());
            }

         tmpdir->setEmpty();

         if (formMulti->isEmpty() == false) formMulti->setEmpty();
         }

      qcontent->clear();
      }

   if (UServer_Base::isParallelizationParent()) return;

   if (data_session) data_session->resetDataSession();

#ifndef U_LOG_DISABLE
   if (UServer_Base::apache_like_log)
      {
      U_INTERNAL_DUMP("iov_vec[0].iov_len = %u", iov_vec[0].iov_len)

      if (iov_vec[0].iov_len == 0) prepareApacheLikeLog(); 

      // response_code, body_len

      if (iov_vec[5].iov_len == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(iov_buffer, iov_vec[5].iov_base)

         uint32_t body_len = UClientImage_Base::body->size();

         iov_vec[5].iov_len = (body_len == 0 ? u__snprintf(iov_buffer, sizeof(iov_buffer), U_CONSTANT_TO_PARAM("\" %u - \""),  U_http_info.nResponseCode)
                                             : u__snprintf(iov_buffer, sizeof(iov_buffer), U_CONSTANT_TO_PARAM("\" %u %u \""), U_http_info.nResponseCode, body_len));
         }

#    ifndef U_CACHE_REQUEST_DISABLE
      if (iov_vec[4].iov_len != 1 &&
          U_ClientImage_request_is_cached == false)
         {
         iov_vec[4].iov_base = (caddr_t) UClientImage_Base::request->c_pointer(request_offset);

         U_INTERNAL_DUMP("request(%u) = %.*S", request_offset, iov_vec[4].iov_len, iov_vec[4].iov_base)
         }

      if (referer_offset)
         {
         iov_vec[6].iov_base = (caddr_t) UClientImage_Base::request->c_pointer(referer_offset);

         U_INTERNAL_DUMP("referer(%u) = %.*S", referer_offset, iov_vec[6].iov_len, iov_vec[6].iov_base)
         }

      if (agent_offset)
         {
         iov_vec[8].iov_base = (caddr_t) UClientImage_Base::request->c_pointer(agent_offset);

         U_INTERNAL_DUMP("agent(%u) = %.*S", agent_offset, iov_vec[8].iov_len, iov_vec[8].iov_base)
         }
#    endif

      U_INTERNAL_ASSERT_EQUALS(iov_vec[2].iov_base, ULog::date.date2)

      ULog::updateDate2();

      UServer_Base::apache_like_log->write(iov_vec, 10);

      iov_vec[0].iov_len = 0;
      }
#endif

   U_INTERNAL_DUMP("U_ClientImage_request = %b U_http_info.nResponseCode = %d U_ClientImage_request_is_cached = %b U_http_info.startHeader = %u",
                    U_ClientImage_request,     U_http_info.nResponseCode,     U_ClientImage_request_is_cached,     U_http_info.startHeader)

#ifndef U_CACHE_REQUEST_DISABLE
   if (isGETorHEAD()                           &&
       UClientImage_Base::isRequestCacheable() &&
       U_IS_HTTP_SUCCESS(U_http_info.nResponseCode))
      {
      UClientImage_Base::setRequestToCache();

      U_INTERNAL_DUMP("file_data = %p U_http_sendfile = %b", file_data, U_http_sendfile)

      if (file_data &&
          U_http_sendfile)
         {
         UClientImage_Base::csfd = file_data->fd;
         }
      }
#endif
}

/**
 * Set-Cookie: NAME=VALUE; expires=DATE; path=PATH; domain=DOMAIN_NAME; secure
 * 
 * NAME=VALUE
 * ------------------------------------------------------------------------------------------------------------------------------------
 * This string is a sequence of characters excluding semi-colon, comma and white space. If there is a need to place such data
 * in the name or value, some encoding method such as URL style %XX encoding is recommended, though no encoding is defined or required.
 * This is the only required attribute on the Set-Cookie header.
 * ------------------------------------------------------------------------------------------------------------------------------------
 * 
 * expires=DATE
 * ------------------------------------------------------------------------------------------------------------------------------------
 * The expires attribute specifies a date string that defines the valid life time of that cookie. Once the expiration date has been
 * reached, the cookie will no longer be stored or given out.
 * The date string is formatted as: Wdy, DD-Mon-YYYY HH:MM:SS GMT
 * expires is an optional attribute. If not specified, the cookie will expire when the user's session ends.
 * 
 * Note: There is a bug in Netscape Navigator version 1.1 and earlier. Only cookies whose path attribute is set explicitly to "/" will
 * be properly saved between sessions if they have an expires attribute.
 * ------------------------------------------------------------------------------------------------------------------------------------
 * 
 * domain=DOMAIN_NAME
 * ------------------------------------------------------------------------------------------------------------------------------------
 * When searching the cookie list for valid cookies, a comparison of the domain attributes of the cookie is made with the Internet
 * domain name of the host from which the URL will be fetched. If there is a tail match, then the cookie will go through path matching
 * to see if it should be sent. "Tail matching" means that domain attribute is matched against the tail of the fully qualified domain
 * name of the host. A domain attribute of "acme.com" would match host names "anvil.acme.com" as well as "shipping.crate.acme.com".
 * 
 * Only hosts within the specified domain can set a cookie for a domain and domains must have at least two (2) or three (3) periods in
 * them to prevent domains of the form: ".com", ".edu", and "va.us". Any domain that fails within one of the seven special top level
 * domains listed below only require two periods. Any other domain requires at least three. The seven special top level domains are:
 * "COM", "EDU", "NET", "ORG", "GOV", "MIL", and "INT".
 * 
 * The default value of domain is the host name of the server which generated the cookie response.
 * ------------------------------------------------------------------------------------------------------------------------------------
 * 
 * path=PATH
 * ------------------------------------------------------------------------------------------------------------------------------------
 * The path attribute is used to specify the subset of URLs in a domain for which the cookie is valid. If a cookie has already passed
 * domain matching, then the pathname component of the URL is compared with the path attribute, and if there is a match, the cookie is
 * considered valid and is sent along with the URL request. The path "/foo" would match "/foobar" and "/foo/bar.html". The path "/" is
 * the most general path.
 * 
 * If the path is not specified, it as assumed to be the same path as the document being described by the header which contains the cookie
 * 
 * secure
 * ------------------------------------------------------------------------------------------------------------------------------------
 * If a cookie is marked secure, it will only be transmitted if the communications channel with the host is a secure one. Currently
 * this means that secure cookies will only be sent to HTTPS (HTTP over SSL) servers.
 * 
 * If secure is not specified, a cookie is considered safe to be sent in the clear over unsecured channels. 
 * ------------------------------------------------------------------------------------------------------------------------------------

 * HttpOnly cookies are a Microsoft extension to the cookie standard. The idea is that cookies marked as httpOnly cannot be accessed
 * from JavaScript. This was implemented to stop cookie stealing through XSS vulnerabilities. This is unlike many people believe not
 * a way to stop XSS vulnerabilities, but a way to stop one of the possible attacks (cookie stealing) that are possible through XSS
 */

void UHTTP::setCookie(const UString& param)
{
   U_TRACE(0, "UHTTP::setCookie(%V)", param.rep)

   time_t expire;
   uint32_t n_hours;
   UVector<UString> vec(param);
   UString item, cookie(U_CAPACITY);

   // -----------------------------------------------------------------------------------------------------------------------------------
   // param: "[ data expire path domain secure HttpOnly ]"
   // -----------------------------------------------------------------------------------------------------------------------------------
   // string -- key_id or data to put in cookie    -- must
   // int    -- lifetime of the cookie in HOURS    -- must (0 -> valid until browser exit)
   // string -- path where the cookie can be used  -- opt
   // string -- domain which can read the cookie   -- opt
   // bool   -- secure mode                        -- opt
   // bool   -- only allow HTTP usage              -- opt
   // -----------------------------------------------------------------------------------------------------------------------------------
   // RET: Set-Cookie: ulib.s<counter>=data&expire&HMAC-MD5(data&expire); expires=expire(GMT); path=path; domain=domain; secure; HttpOnly
   // -----------------------------------------------------------------------------------------------------------------------------------

   set_cookie_option->setBuffer(200U);

   for (uint32_t i = 0, n = vec.size(); i < n; ++i)
      {
      item = vec[i];

      switch (i)
         {
         case 0:
            {
            // string -- key_id or data to put in cookie -- must

            if (item.empty())
               {
               U_INTERNAL_ASSERT_POINTER(data_session)

               item = data_session->setKeyIdDataSession(++sid_counter_gen);
               }

            // int -- lifetime of the cookie in HOURS -- must (0 -> valid until browser exit)

            n_hours = (++i < n ? vec[i].strtoul() : 0);
            expire  = (n_hours ? u_now->tv_sec + (n_hours * 60L * 60L) : 0L);

            cookie.snprintf(U_CONSTANT_TO_PARAM("ulib.s%u="), sid_counter_gen);

            (void) cookie.append(UServices::generateToken(item, expire)); // HMAC-MD5(data&expire)

            if (n_hours) cookie.snprintf_add(U_CONSTANT_TO_PARAM("; expires=%#8D"), expire);
            }
         break;

         case 2:
            {
            // string -- path where the cookie can be used -- opt

            if (item) set_cookie_option->snprintf_add(U_CONSTANT_TO_PARAM("; path=%v"), item.rep);
            }
         break;

         case 3:
            {
            // string -- domain which can read the cookie -- opt

            if (item) set_cookie_option->snprintf_add(U_CONSTANT_TO_PARAM("; domain=%v"), item.rep);
            }
         break;

         case 4:
            {
            // bool -- secure mode -- opt

            if (item.strtob()) (void) set_cookie_option->append(U_CONSTANT_TO_PARAM("; secure"));
            }
         break;

         case 5:
            {
            // bool -- only allow HTTP usage -- opt

            if (item.strtob()) (void) set_cookie_option->append(U_CONSTANT_TO_PARAM("; HttpOnly"));
            }
         break;
         }
      }

   U_SRV_LOG("Create new session ulib.s%u", sid_counter_gen);

   addSetCookie(cookie);
}

void UHTTP::initDbNotFound()
{
   U_TRACE_NO_PARAM(0, "UHTTP::initDbNotFound()")

   U_INTERNAL_ASSERT_EQUALS(db_not_found, 0)
   U_INTERNAL_ASSERT_POINTER(UServer_Base::handler_inotify)

   U_NEW(URDB, db_not_found, URDB(U_STRING_FROM_CONSTANT("../db/NotFound.http"), -1));

   if (db_not_found->open(4 * 1024 * 1024, false, true)) // NB: we don't want truncate (we have only the journal)...
      {
      U_SRV_LOG("db NotFound initialization success");

      if (UServer_Base::isPreForked()) db_not_found->setShared(U_SRV_LOCK_DB_NOT_FOUND, U_SRV_SPINLOCK_DB_NOT_FOUND);
      }
   else
      {
      U_SRV_LOG("WARNING: db NotFound initialization failed");

      delete db_not_found;
             db_not_found = 0;
      }
}

// HTTP session

void UHTTP::initSession()
{
   U_TRACE_NO_PARAM(0, "UHTTP::initSession()")

   if (db_session == 0)
      {
      // NB: the old sessions are automatically NOT valid because UServer generate the crypto key at startup...

      U_NEW(URDBObjectHandler<UDataStorage*>, db_session, URDBObjectHandler<UDataStorage*>(U_STRING_FROM_CONSTANT("../db/session.http"), -1, 0));

      if (db_session->open(4 * 1024 * 1024, false, true)) // NB: we don't want truncate (we have only the journal)...
         {
         U_SRV_LOG("db initialization of HTTP session success");

              if (data_session) db_session->setPointerToDataStorage(data_session);
         else if (data_storage) db_session->setPointerToDataStorage(data_storage);

         if (UServer_Base::isPreForked()) db_session->setShared(U_SRV_LOCK_DATA_SESSION, U_SRV_SPINLOCK_DATA_SESSION);
         }
      else
         {
         U_SRV_LOG("WARNING: db initialization of HTTP session failed");

         delete db_session;
                db_session = 0;
         }
      }
}

void UHTTP::clearSession()
{
   U_TRACE_NO_PARAM(0, "UHTTP::clearSession()")

   U_INTERNAL_ASSERT_POINTER(db_session)

   db_session->close();

   if (data_session) delete data_session;
   if (data_storage) delete data_storage;

   delete db_session;
          db_session = 0;
}

#ifdef USE_LIBSSL
void UHTTP::initSessionSSL()
{
   U_TRACE_NO_PARAM(0, "UHTTP::initSessionSSL()")

   U_INTERNAL_ASSERT_EQUALS(db_session_ssl, 0)
   U_INTERNAL_ASSERT_EQUALS(data_session_ssl, 0)

   U_NEW(USSLSession, data_session_ssl, USSLSession);
   U_NEW(URDBObjectHandler<UDataStorage*>, db_session_ssl, URDBObjectHandler<UDataStorage*>(U_STRING_FROM_CONSTANT("../db/session.ssl"), -1, data_session_ssl));

   if (db_session_ssl->open(4 * 1024 * 1024, false, true)) // NB: we don't want truncate (we have only the journal)...
      {
      U_SRV_LOG("db initialization of SSL session success");

      db_session_ssl->reset(); // Initialize the cache to contain no entries

      if (UServer_Base::isPreForked()) db_session_ssl->setShared(U_SRV_LOCK_SSL_SESSION, U_SRV_SPINLOCK_SSL_SESSION);

      /**
       * In order to allow external session caching, synchronization with the internal session cache is realized via callback functions.
       * Inside these callback functions, session can be saved to disk or put into a database using the d2i_SSL_SESSION(3) interface.
       *
       * The new_session_cb() is called, whenever a new session has been negotiated and session caching is enabled
       * (see SSL_CTX_set_session_cache_mode(3)). The new_session_cb() is passed the ssl connection and the ssl session sess.
       * If the callback returns 0, the session will be immediately removed again.
       *
       * The remove_session_cb() is called, whenever the SSL engine removes a session from the internal cache. This happens when
       * the session is removed because it is expired or when a connection was not shutdown cleanly. It also happens for all sessions
       * in the internal session cache when SSL_CTX_free(3) is called. The remove_session_cb() is passed the ctx and the ssl session sess.
       * It does not provide any feedback.
       *
       * The get_session_cb() is only called on SSL/TLS servers with the session id proposed by the client. The get_session_cb() is
       * always called, also when session caching was disabled. The get_session_cb() is passed the ssl connection, the session id and
       * the length at the memory location data. With the parameter copy the callback can require the SSL engine to increment the
       * reference count of the SSL_SESSION object, Normally the reference count is not incremented and therefore the session must not
       * be explicitly freed with SSL_SESSION_free(3)
       */

#if OPENSSL_VERSION_NUMBER < 0x10100000L
typedef SSL_SESSION* (*psPFpspcipi) (SSL*,      unsigned char*,int,int*);
#else
typedef SSL_SESSION* (*psPFpspcipi) (SSL*,const unsigned char*,int,int*);
#endif

      U_SYSCALL_VOID(SSL_CTX_sess_set_new_cb,    "%p,%p", USSLSocket::sctx,              USSLSession::newSession);
      U_SYSCALL_VOID(SSL_CTX_sess_set_get_cb,    "%p,%p", USSLSocket::sctx, (psPFpspcipi)USSLSession::getSession);
      U_SYSCALL_VOID(SSL_CTX_sess_set_remove_cb, "%p,%p", USSLSocket::sctx,              USSLSession::removeSession);

      // NB: All currently supported protocols have the same default timeout value of 300 seconds
      // ----------------------------------------------------------------------------------------
      // (void) U_SYSCALL(SSL_CTX_set_timeout,         "%p,%u", USSLSocket::sctx, 300);
         (void) U_SYSCALL(SSL_CTX_sess_set_cache_size, "%p,%u", USSLSocket::sctx, 1024 * 1024);

      U_INTERNAL_DUMP("timeout = %d", SSL_CTX_get_timeout(USSLSocket::sctx))
      }
   else
      {
      U_SRV_LOG("WARNING: db initialization of SSL session failed");

      delete db_session_ssl;
             db_session_ssl = 0;
      }
}

void UHTTP::clearSessionSSL()
{
   U_TRACE_NO_PARAM(0, "UHTTP::clearSessionSSL()")

   U_INTERNAL_ASSERT_POINTER(db_session_ssl)

   db_session_ssl->close();

   delete data_session_ssl;

   delete db_session_ssl;
          db_session_ssl = 0;
}
#endif

/**
 * How Does Authorization Work ?
 *
 * 1) User submits login form. Form sends login and password to ULib server.
 * 2) ULib server validates login data, generates random string (session id), saves it to closed server storage
 *    in pair with user login, and sends session id to browser in response as cookie. Browser stores cookie.
 * 3) User visits any page on this domain and browser sends a cookie to server for each request.
 * 4) ULib server checks if cookie has been sent, if such cookie exists in server storage with pair with login. Identifies user, provides access to his private content.
 * 5) Logout button removes the cookie from browser and sid-login pair from server storage. Browser does not send cookies, server does not see it and does not see
 *    sid-login pair
 */

void UHTTP::addSetCookie(const UString& cookie)
{
   U_TRACE(0, "UHTTP::addSetCookie(%V)", cookie.rep)

   U_INTERNAL_ASSERT(cookie)

   (void) set_cookie->append(U_CONSTANT_TO_PARAM("Set-Cookie: "));
   (void) set_cookie->append(cookie);

   if (*set_cookie_option) (void) set_cookie->append(*set_cookie_option);

   (void) set_cookie->append(U_CONSTANT_TO_PARAM("\r\n"));

   U_INTERNAL_DUMP("set_cookie = %V", set_cookie->rep)
}

void UHTTP::removeDataSession()
{
   U_TRACE_NO_PARAM(0, "UHTTP::removeDataSession()")

   U_INTERNAL_ASSERT_POINTER(db_session)
   U_INTERNAL_ASSERT_POINTER(data_session)

   if (data_session->isDataSession() ||
       (U_http_info.cookie_len       &&
        getCookie(0, 0)))
      {
      data_session->clear();

      removeCookieSession();

      U_SRV_LOG("Delete session ulib.s%u keyid=%V", sid_counter_cur, data_session->keyid.rep);

#  ifdef U_LOG_DISABLE
            (void) db_session->remove(data_session->keyid);
#  else
      int result = db_session->remove(data_session->keyid);

      if (result) U_SRV_LOG("WARNING: remove of session data on db failed with error %d", result);
#  endif

      data_session->resetDataSession();
      }
}

void UHTTP::setSessionCookie(UString* param)
{
   U_TRACE(0, "UHTTP::setSessionCookie(%p)", param)

   U_INTERNAL_ASSERT_POINTER(data_session)

   if (param)
      {
      removeDataSession();

      setCookie(*param);
      }
   else
      {
      if (data_session->isDataSession() == false) setCookie(*cgi_cookie_option);
      }
}

bool UHTTP::getCookie(UString* cookie, UString* data)
{
   U_TRACE(0, "UHTTP::getCookie(%p,%p)", cookie, data)

   U_INTERNAL_ASSERT_MAJOR(U_http_info.cookie_len, 0)

   time_t expire;
   uint32_t agent;
   const char* ptr;
   const char* end;
   bool check, result = false;
   UString cookies(U_http_info.cookie, U_http_info.cookie_len), item, token;
   UVector<UString> cookie_list; // NB: must be here to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...

   for (uint32_t i = 0, n = cookie_list.split(cookies, ';'); i < n; ++i)
      {
      item = cookie_list[i];

      item.trim();

      U_INTERNAL_DUMP("cookie[%u] = %V", i, item.rep)

      ptr = item.data();

      if (u_get_unalignedp32(ptr)   == U_MULTICHAR_CONSTANT32('u','l','i','b') &&
          u_get_unalignedp16(ptr+4) == U_MULTICHAR_CONSTANT16('.','s'))
         {
         ptr += U_CONSTANT_SIZE("ulib.s");

         for (end = ptr; u__isdigit(*end); ++end) {}

         U_INTERNAL_ASSERT_EQUALS(*end, '=')

         sid_counter_cur = u_strtoul(ptr, end++);

         U_INTERNAL_DUMP("sid_counter_cur = %u", sid_counter_cur)

         /**
          * XSRF (cross-site request forgery) is a problem that can be solved by using Crumbs. Crumbs is a large alphanumeric string you pass between each on your site, and it is
          * a timestamp + a HMAC-MD5 encoded result of your IP address and browser user agent and a pre-defined key known only to the web server. So, the application checks the crumb
          * value on every page, and the crumb value has a specific expiration time and also is based on IP and browser, so it is difficult to forge. If the crumb value is wrong,
          * then it just prevents the user from viewing that page...
          */

         check = false;

         token.setBuffer(100U);

         if (UServices::getTokenData(token, item.substr(end, item.remain(end)), expire))
            {
            U_INTERNAL_DUMP("token = %V", token.rep)

            ptr = token.data();

            U_INTERNAL_DUMP("ptr[0] = %C", ptr[0])

            if (ptr[0] == '$') check = true; // session shared... (ex. chat)
            else
               {
               // HTTP Session Hijacking mitigation: <IP>_<USER-AGENT>_<PID>_<COUNTER>

               U_INTERNAL_DUMP("ptr(%u) = %.*S", UServer_Base::client_address_len, UServer_Base::client_address_len, ptr)

               if (memcmp(ptr, UServer_Base::client_address, UServer_Base::client_address_len) == 0) // IP
                  {
                  ptr += UServer_Base::client_address_len+1;

                  U_INTERNAL_ASSERT(u__isdigit(ptr[0]))

                  for (end = ptr; u__isdigit(*end); ++end) {}

                  U_INTERNAL_ASSERT_EQUALS(*end, '_')

                  agent = u_strtoul(ptr, end);

                  U_INTERNAL_DUMP("agent = %u", agent)

                  if (agent == getUserAgent()) // USER-AGENT
                     {
                     ptr = end + 1;

                     U_INTERNAL_DUMP("ptr(%u) = %.*S", u_pid_str_len, u_pid_str_len, ptr)

                     U_INTERNAL_ASSERT(u__isdigit(ptr[0]))

                     if (UServer_Base::preforked_num_kids ||
                         memcmp(ptr, u_pid_str, u_pid_str_len) == 0) // PID
                        {
                        ptr += u_pid_str_len+1;

                        U_INTERNAL_ASSERT(u__isdigit(ptr[0]))

                        for (end = ptr; u__isdigit(*end); ++end) {}

                        U_INTERNAL_DUMP("sid_counter_cur = %u u_strtoul(ptr, end) = %u", sid_counter_cur, u_strtoul(ptr, end))

                        check = (sid_counter_cur == u_strtoul(ptr, end)); // COUNTER

                        if (data  &&
                            check &&
                            *end == ':')
                           {
                           ++end;

                           (void) data->replace(end, token.remain(end));

                           U_INTERNAL_DUMP("data = %V", data->rep)
                           }
                        }
                     }
                  }
               }
            }

         if (check == false)
            {
            removeCookieSession();

            U_SRV_LOG("Delete session ulib.s%u keyid=%V", sid_counter_cur, token.rep);

            continue;
            }

         if (checkDataSession(token, expire, data)) result = true;
         }
      else if (cookie)
         {
         if (*cookie) (void) cookie->append(U_CONSTANT_TO_PARAM("; "));
                      (void) cookie->append(item);
         }
      }

   U_RETURN(result);
}

U_NO_EXPORT bool UHTTP::checkDataSession(const UString& token, time_t expire, UString* data)
{
   U_TRACE(0, "UHTTP::checkDataSession(%V,%ld,%p)", token.rep, expire, data)

   U_INTERNAL_ASSERT(token)
   U_INTERNAL_ASSERT_POINTER(db_session)
   U_INTERNAL_ASSERT_POINTER(data_session)

   // NB: check for previous valid cookie...

   if (data_session->isDataSession()) goto remove;

   if (data == 0 ||
       data->empty())
      {
      data_session->keyid = token;

      db_session->setPointerToDataStorage(data_session);

      if (db_session->getDataStorage() &&
          expire == 0                  && // 0 -> valid until browser exit
          data_session->isDataSessionExpired())
         {
remove:  removeDataSession();

         U_RETURN(false);
         }
      }

   U_SRV_LOG("Found session ulib.s%u", sid_counter_cur);

   U_RETURN(true);
}

bool UHTTP::getDataStorage()
{
   U_TRACE_NO_PARAM(0, "UHTTP::getDataStorage()")

   U_INTERNAL_ASSERT_POINTER(db_session)
   U_INTERNAL_ASSERT_POINTER(data_storage)

   db_session->setPointerToDataStorage(data_storage);

   if (db_session->getDataStorage()) U_RETURN(true);

   U_RETURN(false);
}

bool UHTTP::getDataStorage(uint32_t index, UString& value)
{
   U_TRACE(0, "UHTTP::getDataStorage(%u,%V)", index, value.rep)

   U_INTERNAL_ASSERT_POINTER(db_session)
   U_INTERNAL_ASSERT_POINTER(data_storage)

   if (getDataStorage())
      {
      data_storage->getValueVar(index, value);

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UHTTP::getDataSession()
{
   U_TRACE_NO_PARAM(0, "UHTTP::getDataSession()")

   U_INTERNAL_ASSERT_POINTER(db_session)
   U_INTERNAL_ASSERT_POINTER(data_session)

   if (data_session->isDataSession() ||
       (U_http_info.cookie_len       &&
        getCookie(0, 0)))
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UHTTP::getDataSession(uint32_t index, UString& value)
{
   U_TRACE(0, "UHTTP::getDataSession(%u,%V)", index, value.rep)

   U_INTERNAL_ASSERT_POINTER(db_session)
   U_INTERNAL_ASSERT_POINTER(data_session)

   if (getDataSession())
      {
      data_session->getValueVar(index, value);

      U_RETURN(true);
      }

   U_RETURN(false);
}

void UHTTP::putDataSession()
{
   U_TRACE_NO_PARAM(0, "UHTTP::putDataSession()")

   U_INTERNAL_ASSERT_POINTER(db_session)
   U_INTERNAL_ASSERT_POINTER(data_session)

   db_session->setPointerToDataStorage(data_session);

   (void) db_session->putDataStorage();
}

void UHTTP::putDataSession(uint32_t index, const char* value, uint32_t size)
{
   U_TRACE(0, "UHTTP::putDataSession(%u,%.*S,%u)", index, size, value, size)

   U_INTERNAL_ASSERT_POINTER(data_session)

   if (size == 0) data_session->putValueVar(index, UString::getStringNull());
   else
      {
      UString _value((void*)value, size);

      data_session->putValueVar(index, _value);
      }

   putDataSession();
}

void UHTTP::putDataStorage()
{
   U_TRACE_NO_PARAM(0, "UHTTP::putDataStorage()")

   U_INTERNAL_ASSERT_POINTER(db_session)
   U_INTERNAL_ASSERT_POINTER(data_storage)

   db_session->setPointerToDataStorage(data_storage);

   (void) db_session->putDataStorage();
}

void UHTTP::putDataStorage(uint32_t index, const char* value, uint32_t size)
{
   U_TRACE(0, "UHTTP::putDataStorage(%u,%.*S,%u)", index, size, value, size)

   U_INTERNAL_ASSERT_POINTER(db_session)
   U_INTERNAL_ASSERT_POINTER(data_storage)

   if (size == 0) data_storage->putValueVar(index, UString::getStringNull());
   else
      {
      UString _value((void*)value, size);

      data_storage->putValueVar(index, _value);
      }

   putDataStorage();
}

#ifdef DEBUG
#  define U_HTTP_ENTRY(n,plen,msg) n: descr = msg; if (plen) *plen = U_CONSTANT_SIZE(msg); break
#else
#  define U_HTTP_ENTRY(n,plen,msg) n: descr = msg;           *plen = U_CONSTANT_SIZE(msg); break
#endif

const char* UHTTP::getStatusDescription(uint32_t* plen)
{
   U_TRACE(0, "UHTTP::getStatusDescription(%p)", plen)

   const char* descr;

   switch (U_http_info.nResponseCode)
      {
      // 1xx indicates an informational message only
      case U_HTTP_ENTRY(HTTP_CONTINUE,          plen, "Continue");
      case U_HTTP_ENTRY(HTTP_SWITCH_PROT,       plen, "Switching Protocol");
   // case U_HTTP_ENTRY(102,                    plen, "HTTP Processing");

      // 2xx indicates success of some kind
      case U_HTTP_ENTRY(HTTP_OK,                plen, "OK");
      case U_HTTP_ENTRY(HTTP_CREATED,           plen, "Created");
      case U_HTTP_ENTRY(HTTP_ACCEPTED,          plen, "Accepted");
      case U_HTTP_ENTRY(HTTP_NOT_AUTHORITATIVE, plen, "Non-Authoritative Information");
      case U_HTTP_ENTRY(HTTP_NO_CONTENT,         plen, "No Content");
      case U_HTTP_ENTRY(HTTP_RESET,              plen, "Reset Content");
      case U_HTTP_ENTRY(HTTP_PARTIAL,            plen, "Partial Content");
   // case U_HTTP_ENTRY(207,                     plen, "Webdav Multi-status");

      // 3xx Redirection - Further action must be taken in order to complete the request
      case U_HTTP_ENTRY(HTTP_MULT_CHOICE,        plen, "Multiple Choices");
      case U_HTTP_ENTRY(HTTP_MOVED_PERM,         plen, "Moved Permanently");
      case U_HTTP_ENTRY(HTTP_MOVED_TEMP,         plen, "Moved Temporarily");
   // case U_HTTP_ENTRY(HTTP_FOUND,              plen, "Found [Elsewhere]");
      case U_HTTP_ENTRY(HTTP_SEE_OTHER,          plen, "See Other");
      case U_HTTP_ENTRY(HTTP_NOT_MODIFIED,       plen, "Not Modified");
      case U_HTTP_ENTRY(HTTP_USE_PROXY,          plen, "Use Proxy");
      case U_HTTP_ENTRY(HTTP_TEMP_REDIR,         plen, "Temporary Redirect");

      // 4xx indicates an error on the client's part
      case U_HTTP_ENTRY(HTTP_BAD_REQUEST,                     plen, "Bad Request");
      case U_HTTP_ENTRY(HTTP_UNAUTHORIZED,                    plen, "Authorization Required");
      case U_HTTP_ENTRY(HTTP_PAYMENT_REQUIRED,                plen, "Payment Required");
      case U_HTTP_ENTRY(HTTP_FORBIDDEN,                       plen, "Forbidden");
      case U_HTTP_ENTRY(HTTP_NOT_FOUND,                       plen, "Not Found");
      case U_HTTP_ENTRY(HTTP_BAD_METHOD,                      plen, "Method Not Allowed");
      case U_HTTP_ENTRY(HTTP_NOT_ACCEPTABLE,                  plen, "Not Acceptable");
      case U_HTTP_ENTRY(HTTP_PROXY_AUTH,                      plen, "Proxy Authentication Required");
      case U_HTTP_ENTRY(HTTP_CLIENT_TIMEOUT,                  plen, "Request Time-out");
      case U_HTTP_ENTRY(HTTP_CONFLICT,                        plen, "Conflict");
      case U_HTTP_ENTRY(HTTP_GONE,                            plen, "Gone");
      case U_HTTP_ENTRY(HTTP_LENGTH_REQUIRED,                 plen, "Length Required");
      case U_HTTP_ENTRY(HTTP_PRECON_FAILED,                   plen, "Precondition Failed");
      case U_HTTP_ENTRY(HTTP_ENTITY_TOO_LARGE,                plen, "Request Entity Too Large");
      case U_HTTP_ENTRY(HTTP_REQ_TOO_LONG,                    plen, "Request-URI Too Long");
      case U_HTTP_ENTRY(HTTP_UNSUPPORTED_TYPE,                plen, "Unsupported Media Type");
      case U_HTTP_ENTRY(HTTP_REQ_RANGE_NOT_OK,                plen, "Requested Range not satisfiable");
      case U_HTTP_ENTRY(HTTP_EXPECTATION_FAILED,              plen, "Expectation Failed");
      case U_HTTP_ENTRY(HTTP_UNPROCESSABLE_ENTITY,            plen, "Unprocessable Entity");
   // case U_HTTP_ENTRY(423,                                  plen, "Locked");
   // case U_HTTP_ENTRY(424,                                  plen, "Failed Dependency");
   // case U_HTTP_ENTRY(425,                                  plen, "No Matching Vhost");
   // case U_HTTP_ENTRY(426,                                  plen, "Upgrade Required");
      case U_HTTP_ENTRY(HTTP_PRECONDITION_REQUIRED,           plen, "Precondition required");
      case U_HTTP_ENTRY(HTTP_TOO_MANY_REQUESTS,               plen, "Too many requests");
      case U_HTTP_ENTRY(HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE, plen, "Request_header_fields_too_large");
   // case U_HTTP_ENTRY(449,                                  plen, "Retry With Appropriate Action");

      // 5xx indicates an error on the server's part
      case U_HTTP_ENTRY(HTTP_INTERNAL_ERROR,                  plen, "Internal Server Error");
      case U_HTTP_ENTRY(HTTP_NOT_IMPLEMENTED,                 plen, "Not Implemented");
      case U_HTTP_ENTRY(HTTP_BAD_GATEWAY,                     plen, "Bad Gateway");
      case U_HTTP_ENTRY(HTTP_UNAVAILABLE,                     plen, "Service Unavailable");
      case U_HTTP_ENTRY(HTTP_GATEWAY_TIMEOUT,                 plen, "Gateway Time-out");
      case U_HTTP_ENTRY(HTTP_VERSION,                         plen, "HTTP Version Not Supported");
   // case U_HTTP_ENTRY(506,                                  plen, "Variant also varies");
   // case U_HTTP_ENTRY(507,                                  plen, "Insufficient Storage");
   // case U_HTTP_ENTRY(510,                                  plen, "Not Extended");
      case U_HTTP_ENTRY(HTTP_NETWORK_AUTHENTICATION_REQUIRED, plen, "Network authentication required");

      U_HTTP_ENTRY(default, plen, "Code unknown");
      }

   U_RETURN(descr);
}

#undef U_HTTP_ENTRY

U_NO_EXPORT UString UHTTP::getHTMLDirectoryList()
{
   U_TRACE_NO_PARAM(0, "UHTTP::getHTMLDirectoryList()")

   U_INTERNAL_ASSERT(file->pathname.isNullTerminated())

   bool is_dir;
   UDirWalk dirwalk;
   uint32_t i, pos, n;
   UVector<UString> vec(2048);
   UString buffer(4000U), item, size, basename, entry(4000U), value_encoded(U_CAPACITY), readme_txt;

   int32_t len     = file->getPathRelativLen();
   const char* ptr = file->getPathRelativ();
   bool broot      = (len == 1 && ptr[0] == '/');

   if (broot)
      {
      (void) buffer.assign(U_CONSTANT_TO_PARAM(
         "<html><head><title>Index of /</title></head>"
         "<body><h1>Index of directory: /</h1><hr>"
         "<table><tr>"
         "<td></td>"
         "<td></td>"
         "</tr>"));
      }
   else
      {
      buffer.snprintf(U_CONSTANT_TO_PARAM(
         "<html><head><title>Index of %.*s</title></head>"
         "<body><h1>Index of directory: %.*s</h1><hr>"
         "<table><tr>"
         "<td><a href=\"/%.*s/..?_nav_\"><img width=\"20\" height=\"21\" align=\"absbottom\" border=\"0\" src=\"/icons/dir.png\"> Up one level</a></td>"
         "<td></td>"
         "<td></td>"
         "</tr>"), len, ptr, len, ptr, len, ptr);

      if (dirwalk.setDirectory(U_FILE_TO_STRING(*file)) == false) goto end;
      }

   n = dirwalk.walk(vec, U_ALPHABETIC_SORT);

   pos = buffer.size();

   for (i = 0; i < n; ++i)
      {
      item      = vec[i];
      file_data = (*cache_file)[item];

      if (file_data == 0) continue; // NB: this can happen (servlet for example...)

      is_dir = S_ISDIR(file_data->mode);

      if (u_isUrlEncodeNeeded(U_STRING_TO_PARAM(item)))
         {
         Url::encode(item, value_encoded);

         item = value_encoded;
         }

      size     = UStringExt::printSize(file_data->size);
      basename = UStringExt::basename(item);

      entry.snprintf(U_CONSTANT_TO_PARAM(
         "<tr>"
            "<td><strong>"
               "<a href=\"/%v?_nav_\"><img width=\"20\" height=\"21\" align=\"absbottom\" border=\"0\" src=\"/icons/%s\"> %v</a>"
            "</strong></td>"
            "<td align=\"right\" valign=\"bottom\">%v</td>"
            "<td align=\"right\" valign=\"bottom\">%#3D</td>"
         "</tr>"),
         item.rep, (is_dir ? "dir.png" : "generic.gif"), basename.rep,
         size.rep,
         file_data->mtime);

      if (is_dir)
         {
         (void) buffer.insert(pos, entry);

         pos += entry.size();
         }
      else
         {
         if (size &&
             basename.equal(U_CONSTANT_TO_PARAM("README.txt")))
            {
            readme_txt = UFile::contentOf(U_STRING_FROM_CONSTANT("README.txt"));
            }

         (void) buffer.append(entry);
         }
      }

end:
   (void) buffer.append(U_CONSTANT_TO_PARAM("</table><hr>"));

   if (readme_txt)
      {
      (void) buffer.append(U_CONSTANT_TO_PARAM("<pre>"));
      (void) buffer.append(readme_txt);
      (void) buffer.append(U_CONSTANT_TO_PARAM("</pre><hr>"));
      }

   (void) buffer.append(U_CONSTANT_TO_PARAM("<address>ULib Server</address></body></html>"));

   U_INTERNAL_DUMP("buffer(%u) = %V", buffer.size(), buffer.rep)

   U_RETURN_STRING(buffer);
}

// retrieve information on specific HTML form elements
// (such as checkboxes, radio buttons, and text fields, or uploaded files)

__pure int UHTTP::getFormFirstNumericValue(int _min, int _max)
{
   U_TRACE(0, "UHTTP::getFormFirstNumericValue(%d,%d)", _min, _max)

   int value = _min;

   if (U_http_info.query_len)
      {
      const char* ptr = U_http_info.query;

      U_INTERNAL_DUMP("query = %.*S", U_HTTP_QUERY_TO_TRACE)

      ptr = (const char*) memchr(ptr, '=', U_http_info.query_len);

      if (ptr &&
          u__isdigit(*++ptr))
         {
         value = u_strtoul(ptr, U_http_info.query + U_http_info.query_len);

              if (value < _min) value = _min;
         else if (value > _max) value = _max;
         }
      }

   U_RETURN(value);
}

void UHTTP::getFormValue(UString& buffer, uint32_t n)
{
   U_TRACE(0, "UHTTP::getFormValue(%V,%u)", buffer.rep, n)

   U_INTERNAL_ASSERT_POINTER(form_name_value)

   if (n >= form_name_value->size()) buffer.clear();
   else                       (void) buffer.replace((*form_name_value)[n]);
}

void UHTTP::getFormValue(UString& buffer, const char* name, uint32_t len)
{
   U_TRACE(0, "UHTTP::getFormValue(%V,%.*S,%u)", buffer.rep, len, name, len)

   U_INTERNAL_ASSERT_POINTER(form_name_value)

   uint32_t index = form_name_value->find(name, len);

   if (index == U_NOT_FOUND) buffer.clear();
   else
      {
#  ifdef DEBUG
      if ((index+1) >= form_name_value->UVector<UStringRep*>::size())
         {
         U_ERROR("UHTTP::getFormValue() name(%u) = %.*S index = %u", len, len, name, index);
         }
#  endif

      (void) buffer.replace((*form_name_value)[index+1]);
      }
}

UString UHTTP::getFormValue(const char* name, uint32_t len, uint32_t start, uint32_t end)
{
   U_TRACE(0, "UHTTP::getFormValue(%.*S,%u,%u,%u)", len, name, len, start, end)

   U_INTERNAL_ASSERT(start <= end)
   U_INTERNAL_ASSERT_POINTER(form_name_value)

   UString buffer;
   uint32_t index = form_name_value->findRange(name, len, start, end);

   if (index != U_NOT_FOUND)
      {
#  ifdef DEBUG
      if ((index+1) >= form_name_value->UVector<UStringRep*>::size())
         {
         U_ERROR("UHTTP::getFormValue() name(%u) = %.*S start = %u end = %u index = %u", len, len, name, start, end, index);
         }
#  endif

      (void) buffer.replace((*form_name_value)[index+1]);
      }

   U_RETURN_STRING(buffer);
}

void UHTTP::getFormValue(UString& buffer, const char* name, uint32_t len, uint32_t start, uint32_t pos, uint32_t end)
{
   U_TRACE(0, "UHTTP::getFormValue(%V,%.*S,%u,%u,%u,%u)", buffer.rep, len, name, len, start, pos, end)

   U_INTERNAL_ASSERT(start <= end)
   U_INTERNAL_ASSERT_POINTER(form_name_value)

   if (pos >= end) buffer.clear();
   else
      {
#  ifdef DEBUG
      if (( pos    >= form_name_value->UVector<UStringRep*>::size()) ||
          ((pos-1) >= form_name_value->UVector<UStringRep*>::size()))
         {
         U_ERROR("UHTTP::getFormValue() name(%u) = %.*S start = %u pos = %u end = %u", len, len, name, start, pos, end);
         }
#  endif

      UStringRep* r = form_name_value->UVector<UStringRep*>::at(pos-1);

      if (r->equal(name, len) == false) getFormValue(buffer, name, len);
      else
         {
         (void) buffer.replace((*form_name_value)[pos]);

#     ifdef DEBUG
         UString tmp = getFormValue(name, len, start, end);

         if (buffer != tmp)
            {
            U_DEBUG("UHTTP::getFormValue(%p,%.*S,%u,%u,%u,%u) = %V differ from getFormValue(%.*S,%u,%u,%u) = %V",
                        &buffer, len, name, len, start, pos, end, buffer.rep, len, name, len, start, end, tmp.rep)
            }
#     endif
         }
      }
}

uint32_t UHTTP::processForm()
{
   U_TRACE_NO_PARAM(0, "UHTTP::processForm()")

   U_ASSERT(tmpdir->empty())
   U_ASSERT(qcontent->empty())
   U_ASSERT(formMulti->isEmpty())
   U_ASSERT(form_name_value->empty())

   UString tmp;

   if (isGETorHEAD()) tmp = UString(U_HTTP_QUERY_TO_PARAM);
   else
      {
      U_ASSERT(isPOST())

      // -------------------------------------------------------------------------
      // POST
      // -------------------------------------------------------------------------
      // Content-Type: application/x-www-form-urlencoded OR multipart/form-data...
      // -------------------------------------------------------------------------

      if (U_HTTP_CTYPE_MEMEQ("application/x-www-form-urlencoded")) tmp = *UClientImage_Base::body;
      else
         {
         // multipart/form-data (FILE UPLOAD)

         if (*UClientImage_Base::body &&
             U_HTTP_CTYPE_MEMEQ("multipart/form-data"))
            {
            // create temporary directory with files uploaded...

            tmpdir->snprintf(U_CONSTANT_TO_PARAM("%s/formXXXXXX"), u_tmpdir);

            if (UFile::mkdtemp(*tmpdir) &&
                formMulti->init(*UClientImage_Base::body))
               {
               UMimeEntity* item;
               const char* ptr = tmpdir->data();
               uint32_t sz = tmpdir->size(), len;
               UString content, name, filename, basename, content_disposition;

               U_SRV_LOG("temporary directory created: %.*s", sz, ptr);

               for (uint32_t i = 0, n = formMulti->getNumBodyPart(); i < n; ++i)
                  {
                  item                = (*formMulti)[i];
                  content             = item->getContent();
                  content_disposition = item->getContentDisposition();

                  // Content-Disposition: form-data; name="input_file"; filename="/tmp/4dcd39e8-2a84-4242-b7bc-ca74922d26e1"

                  if (UMimeHeader::getNames(content_disposition, name, filename))
                     {
                     // NB: we can't reuse the same string (filename) to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...

                     basename = UStringExt::basename(filename);

                     pathname->setBuffer(sz + 1 + basename.size());

                     pathname->snprintf(U_CONSTANT_TO_PARAM("%.*s/%v"), sz, ptr, basename.rep);

                     (void) UFile::writeTo(*pathname, content);

                     content = *pathname;

#                 ifdef DEBUG
                     basename.clear();
                     filename.clear();
#                 endif
                     }

                  form_name_value->push_back(name);
                  form_name_value->push_back(content);
                  }

               len = form_name_value->size();

               U_RETURN(len);
               }
            }

         U_RETURN(0);
         }
      }

   if (tmp)
      {
      uint32_t len = UStringExt::getNameValueFromData(tmp, *form_name_value, U_CONSTANT_TO_PARAM("&"));

      U_ASSERT_EQUALS(len, form_name_value->size())

      if (len) *qcontent = tmp;

      U_RETURN(len);
      }

   U_RETURN(0);
}

// --------------------------------------------------------------------------------------------------------------------------------------
// start HTTP main error message
// --------------------------------------------------------------------------------------------------------------------------------------

void UHTTP::setStatusDescription()
{
   U_TRACE_NO_PARAM(0, "UHTTP::setStatusDescription()")

   if (response_code != U_http_info.nResponseCode)
      {
      switch ((response_code = U_http_info.nResponseCode))
         {
         // 1xx indicates an informational message only
         case HTTP_SWITCH_PROT:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 101 Switching Protocols\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 101 Switching Protocols\r\n");
            }
         break;

         // 2xx indicates success of some kind
         case HTTP_OK:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 200 OK\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 200 OK\r\n");
            }
         break;
         case HTTP_NO_CONTENT:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 204 No Content\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 204 No Content\r\n");
            }
         break;
         case HTTP_PARTIAL:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 206 Partial Content\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 206 Partial Content\r\n");
            }
         break;

         // 3xx Redirection - Further action must be taken in order to complete the request
         case HTTP_MOVED_TEMP:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 302 Moved Temporarily\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 302 Moved Temporarily\r\n");
            }
         break;
         case HTTP_NOT_MODIFIED:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 304 Not Modified\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 304 Not Modified\r\n");
            }
         break;

         // 4xx indicates an error on the client's part
         case HTTP_BAD_REQUEST:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 400 Bad Request\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 400 Bad Request\r\n");
            }
         break;
         case HTTP_UNAUTHORIZED:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 401 Authorization Required\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 401 Authorization Required\r\n");
            }
         break;
         case HTTP_FORBIDDEN:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 403 Forbidden\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 403 Forbidden\r\n");
            }
         break;
         case HTTP_NOT_FOUND:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 404 Not Found\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 404 Not Found\r\n");
            }
         break;
         case HTTP_LENGTH_REQUIRED:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 411 Length Required\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 411 Length Required\r\n");
            }
         break;
         case HTTP_ENTITY_TOO_LARGE:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 413 Request Entity Too Large\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 413 Request Entity Too Large\r\n");
            }
         break;

         // 5xx indicates an error on the server's part
         case HTTP_INTERNAL_ERROR:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 500 Internal Server Error\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 500 Internal Server Error\r\n");
            }
         break;
         case HTTP_NOT_IMPLEMENTED:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 501 Not Implemented\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 501 Not Implemented\r\n");
            }
         break;
         case HTTP_VERSION:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 505 HTTP Version Not Supported\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 505 HTTP Version Not Supported\r\n");
            }
         break;
#     ifdef U_SERVER_CAPTIVE_PORTAL
         case HTTP_NETWORK_AUTHENTICATION_REQUIRED:
            {
            UClientImage_Base::iov_vec[0].iov_base =       (caddr_t) "HTTP/1.1 511 Network authentication required\r\n";  
            UClientImage_Base::iov_vec[0].iov_len  = U_CONSTANT_SIZE("HTTP/1.1 511 Network authentication required\r\n");
            }
         break;
#     endif

         default:
            {
            uint32_t sz;
            const char* status = getStatusDescription(&sz);

            UClientImage_Base::iov_vec[0].iov_base = response_buffer;
            UClientImage_Base::iov_vec[0].iov_len  = 9+u__snprintf(response_buffer+9, sizeof(response_buffer)-9, U_CONSTANT_TO_PARAM("%u %.*s\r\n"), response_code, sz, status);
            }
         }
      }

   U_INTERNAL_ASSERT_MAJOR(UClientImage_Base::iov_vec[0].iov_len, 0)

   U_INTERNAL_DUMP("UClientImage_Base::iov_vec[0] = %.*S", UClientImage_Base::iov_vec[0].iov_len, UClientImage_Base::iov_vec[0].iov_base)
}

void UHTTP::handlerResponse()
{
   U_TRACE_NO_PARAM(0, "UHTTP::handlerResponse()")

   U_INTERNAL_DUMP("U_http_info.nResponseCode = %d", U_http_info.nResponseCode)

   U_INTERNAL_ASSERT_MAJOR(U_http_info.nResponseCode, 0)

#ifdef DEBUG // NB: All 1xx (informational), 204 (no content), and 304 (not modified) responses MUST not include a body...
   if ((U_http_info.nResponseCode >= 100  &&
        U_http_info.nResponseCode <  200) ||
        U_http_info.nResponseCode == HTTP_NOT_MODIFIED)
      {
      U_ASSERT(ext->empty())
      }
#endif

   UClientImage_Base::setRequestProcessed();

#ifndef U_HTTP2_DISABLE
   if (U_http_version == '2')
      {
      UHTTP2::handlerResponse();

#  ifdef DEBUG
      UHTTP2::decodeHeadersResponse();
#  endif
      }
   else
#endif
   {
   UClientImage_Base::setHeaderForResponse(6+29+2+12+2); // Date: Wed, 20 Jun 2012 11:43:17 GMT\r\nServer: ULib\r\n

   if (U_http_info.nResponseCode == HTTP_NOT_IMPLEMENTED ||
       U_http_info.nResponseCode == HTTP_OPTIONS_RESPONSE)
      {
      U_INTERNAL_DUMP("U_http_method_not_implemented = %b", U_http_method_not_implemented)

      if (U_http_method_not_implemented)
         {
         (void) UClientImage_Base::wbuffer->assign(U_CONSTANT_TO_PARAM("Your browser request a method that this server has not implemented"));
         }
      else
         {
         (void) UClientImage_Base::wbuffer->assign(U_CONSTANT_TO_PARAM("Allow: "
            "GET, HEAD, POST, PUT, DELETE, OPTIONS, "     // request methods
            "TRACE, CONNECT, "                            // pathological
            "COPY, MOVE, LOCK, UNLOCK, MKCOL, PROPFIND, " // webdav
            "PATCH, PURGE, "                              // rfc-5789
            "MERGE, REPORT, CHECKOUT, MKACTIVITY, "       // subversion
            "NOTIFY, MSEARCH, SUBSCRIBE, UNSUBSCRIBE"     // upnp
            "\r\nContent-Length: 0\r\n\r\n"));

         if (U_http_info.nResponseCode == HTTP_OPTIONS_RESPONSE) U_http_info.nResponseCode = HTTP_OK;
         }

      UClientImage_Base::setCloseConnection();

      UClientImage_Base::setRequestNoCache();

      setStatusDescription();

      return;
      }

   // NB: all other responses must include an entity body or a Content-Length header field defined with a value of zero (0)

   char* ptr;
   char* base;
   const char* ptr2;
   uint32_t sz1 = set_cookie->size(), sz2 = ext->size();

   if (sz2) ptr2 = ext->data();
   else
      {
      ptr2 =                 "Content-Length: 0\r\n\r\n";
       sz2 = U_CONSTANT_SIZE("Content-Length: 0\r\n\r\n");

      if (U_http_info.nResponseCode == HTTP_OK)
         {
         if (isGETorHEAD()) U_http_info.nResponseCode = HTTP_NO_CONTENT;

         // A server implements an HSTS policy by supplying a header over an HTTPS connection (HSTS headers over HTTP are ignored)

#     if defined(USE_LIBSSL) && defined(U_HTTP_STRICT_TRANSPORT_SECURITY)
         if (uri_strict_transport_security_mask == (void*)1L)
            {
            U_INTERNAL_ASSERT(UServer_Base::bssl)

            ptr2 =                 "Strict-Transport-Security: max-age=31536000; includeSubDomains; preload\r\nContent-Length: 0\r\n\r\n";
             sz2 = U_CONSTANT_SIZE("Strict-Transport-Security: max-age=31536000; includeSubDomains; preload\r\nContent-Length: 0\r\n\r\n");
            }
#     endif
         }
      }

   setStatusDescription();

   UClientImage_Base::wbuffer->setBuffer(200U + sz1 + sz2);

   base = ptr = UClientImage_Base::wbuffer->data();

   if (sz1)
      {
      UClientImage_Base::setRequestNoCache();

      U_MEMCPY(ptr, set_cookie->data(), sz1);
               ptr +=                   sz1;

      set_cookie->setEmpty();
      }

   U_INTERNAL_DUMP("U_ClientImage_close = %b", U_ClientImage_close)

   if (U_ClientImage_close == false)
      {
      U_INTERNAL_DUMP("U_http_version = %C U_http_keep_alive = %b", U_http_version, U_http_keep_alive)

      // HTTP/1.0 compliance: if Keep-Alive not requested we force close

      if (U_http_version == '0')
         {
#     if defined(USE_LIBSSL) && defined(U_HTTP_STRICT_TRANSPORT_SECURITY)
         if (UServer_Base::bssl                              &&
             U_http_info.startHeader == 16                   &&
             uri_strict_transport_security_mask != (void*)1L &&
             U_HTTP_USER_AGENT_STREQ("SSL Labs (https://www.ssllabs.com/about/assessment.html)"))
            {
            U_MEMCPY(ptr,          "Strict-Transport-Security: max-age=31536000; includeSubDomains; preload\r\n",
                   U_CONSTANT_SIZE("Strict-Transport-Security: max-age=31536000; includeSubDomains; preload\r\n"));
            ptr += U_CONSTANT_SIZE("Strict-Transport-Security: max-age=31536000; includeSubDomains; preload\r\n");
            }
         else
#     endif
         {
         if (U_http_keep_alive == false) UClientImage_Base::setCloseConnection();
         else
            {
            u_put_unalignedp64(ptr,    U_MULTICHAR_CONSTANT64('C','o','n','n','e','c','t','i'));
            u_put_unalignedp64(ptr+8,  U_MULTICHAR_CONSTANT64('o','n',':',' ','K','e','e','p'));
            u_put_unalignedp64(ptr+16, U_MULTICHAR_CONSTANT64('-','A','l','i','v','e','\r','\n'));

            ptr += U_CONSTANT_SIZE("Connection: Keep-Alive\r\n");

            if (UServer_Base::getReqTimeout())
               {
               /**
                * Keep-Alive Timeout
                *
                * To indicate that the session is being kept alive for a maximum of x requests and a per-request timeout of x seconds
                *
                * Syntax:      Integer number
                * Description: Specifies the maximum idle time between requests from a Keep-Alive connection. If no new request is received during
                *              this period of time, the connection will be closed
                *
                * Tips: [Security & Performance] We recommend you to set the value just long enough to handle all requests for a single page view.
                *       It is unnecessary to keep connection alive for an extended period of time. A smaller value can reduce idle connections,
                *       increase capacity to service more users and guard against DoS attacks. 2-5 seconds is a reasonable range for most applications
                */

               ptr += u__snprintf(ptr, 100,
                                  U_CONSTANT_TO_PARAM("Keep-Alive: max=%u, timeout=%d\r\n"),
                                  UNotifier::max_connection - UNotifier::min_connection, UServer_Base::getReqTimeout());
               }
            }
         }
         }
      }

   U_MEMCPY(ptr, ptr2, sz2);

   UClientImage_Base::wbuffer->size_adjust((ptr - base) + sz2);
   }

   U_INTERNAL_DUMP("UClientImage_Base::wbuffer(%u) = %#V", UClientImage_Base::wbuffer->size(), UClientImage_Base::wbuffer->rep)
   U_INTERNAL_DUMP("UClientImage_Base::body(%u) = %V",     UClientImage_Base::body->size(),    UClientImage_Base::body->rep)
}

void UHTTP::setResponse(const UString& content_type, UString* pbody)
{
   U_TRACE(0, "UHTTP::setResponse(%V,%p)", content_type.rep, pbody)

   U_INTERNAL_ASSERT_POINTER(UClientImage_Base::body)
   U_INTERNAL_ASSERT_MAJOR(U_http_info.nResponseCode, 0)

   U_INTERNAL_ASSERT(u_endsWith(U_STRING_TO_PARAM(content_type), U_CONSTANT_TO_PARAM(U_CRLF)))

   ext->setBuffer(U_CAPACITY);

   char* start = ext->data();
   char* ptr   = start;
   uint32_t sz = content_type.size();

   u_put_unalignedp64(ptr,    U_MULTICHAR_CONSTANT64('C','o','n','t','e','n','t','-'));
   u_put_unalignedp32(ptr+8,  U_MULTICHAR_CONSTANT32('T','y','p','e'));
   u_put_unalignedp16(ptr+12, U_MULTICHAR_CONSTANT16(':',' '));

   ptr += U_CONSTANT_SIZE("Content-Type: ");

   U_MEMCPY(ptr, content_type.data(), sz);

   ptr += sz;

   u_put_unalignedp64(ptr,   U_MULTICHAR_CONSTANT64('C','o','n','t','e','n','t','-'));
   u_put_unalignedp64(ptr+8, U_MULTICHAR_CONSTANT64('L','e','n','g','t','h',':',' '));

   ptr += U_CONSTANT_SIZE("Content-Length: ");

   if (pbody == 0)
      {
      *ptr++ = '0';

      UClientImage_Base::body->clear(); // clean body to avoid writev() in response...

      goto end;
      }

#ifdef USE_LIBZ
   if (UStringExt::isGzip(*pbody))
      {
      if (U_http_is_accept_gzip == false) *pbody = UStringExt::gunzip(*pbody);

      ptr = u_num2str32(pbody->size(), ptr);

      if (U_http_is_accept_gzip)
         {
         u_put_unalignedp64(ptr,    U_MULTICHAR_CONSTANT64('\r','\n','C','o','n','t','e','n'));
         u_put_unalignedp64(ptr+8,  U_MULTICHAR_CONSTANT64('t', '-','E','n','c','o','d','i'));
         u_put_unalignedp64(ptr+16, U_MULTICHAR_CONSTANT64('n', 'g',':',' ','g','z','i','p'));

         ptr += U_CONSTANT_SIZE("\r\nContent-Encoding: gzip");
         }
      }
   else
#endif
   {
   ptr = u_num2str32(pbody->size(), ptr);
   }

   *UClientImage_Base::body = *pbody;

end:
   u_put_unalignedp32(ptr, U_MULTICHAR_CONSTANT32('\r','\n','\r','\n'));
                      ptr += U_CONSTANT_SIZE(U_CRLF2);

   ext->size_adjust(ptr - start);

   handlerResponse();
}

#define U_STR_FMR_BODY "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n" \
                       "<html><head>\r\n" \
                       "<title>%d %.*s</title>\r\n" \
                       "</head><body>\r\n" \
                       "<h1>%.*s</h1>\r\n" \
                       "<p>%.*s</p>\r\n" \
                       "<hr>\r\n" \
                       "<address>ULib Server</address>\r\n" \
                       "</body></html>\r\n"

/**
 * HTTP/1.0
 * ------------------------------------------------------------------------------------------------------------------
 * 302 Moved Temporarily
 *
 * The requested resource resides temporarily under a different URL. Since the redirection may be altered on occasion,
 * the client should continue to use the Request-URI for future requests. The URL must be given by the Location field
 * in the response. Unless it was a HEAD request, the Entity-Body of the response should contain a short note with a
 * hyperlink to the new URI(s).
 * ------------------------------------------------------------------------------------------------------------------
 *
 * HTTP/1.1
 * ------------------------------------------------------------------------------------------------------------------
 * 302 Found [Elsewhere]
 *
 * The requested resource resides temporarily under a different URI. Since the redirection might be altered on occasion,
 * the client SHOULD continue to use the Request-URI for future requests. This response is only cacheable if indicated
 * by a Cache-Control or Expires header field. The temporary URI SHOULD be given by the Location field in the response.
 * Unless the request method was HEAD, the entity of the response SHOULD contain a short hypertext note with a hyperlink
 * to the new URI(s).
 *
 * 307 Temporary Redirect
 *
 * The requested resource resides temporarily under a different URI. Since the redirection MAY be altered on occasion,
 * the client SHOULD continue to use the Request-URI for future requests. This response is only cacheable if indicated by
 * a Cache-Control or Expires header field. The temporary URI SHOULD be given by the Location field in the response. Unless
 * the request method was HEAD, the entity of the response SHOULD contain a short hypertext note with a hyperlink to the new
 * URI(s), since many pre-HTTP/1.1 user agents do not understand the 307 status. Therefore, the note SHOULD contain the
 * information necessary for a user to repeat the original request on the new URI.
 * ------------------------------------------------------------------------------------------------------------------
 *
 * http://sebastians-pamphlets.com/the-anatomy-of-http-redirects-301-302-307/
 */

void UHTTP::setRedirectResponse(int mode, const char* ptr_location, uint32_t len_location)
{
   U_TRACE(0, "UHTTP::setRedirectResponse(%d,%.*S,%u)", mode, len_location, ptr_location, len_location)

   U_ASSERT_EQUALS(u_find(ptr_location,len_location,"\n",1), 0)

   U_http_info.nResponseCode = ((mode & NETWORK_AUTHENTICATION_REQUIRED) != 0
                                      ? HTTP_NETWORK_AUTHENTICATION_REQUIRED
                                      : HTTP_MOVED_TEMP); // NB: firefox ask confirmation to user with response 307 (HTTP_TEMP_REDIR)...

   UClientImage_Base::resetPipelineAndSetCloseConnection();

   UString tmp(100U + len_location);

   tmp.snprintf(U_CONSTANT_TO_PARAM(U_CTYPE_HTML "\r\n%s%.*s\r\n"),
                     ((mode & REFRESH)                         != 0 ||
                      (mode & NETWORK_AUTHENTICATION_REQUIRED) != 0
                           ? "Refresh: 1; url="
                           : "Location: "),
                     len_location, ptr_location);

   if (*ext)
      {
      U_INTERNAL_ASSERT(u_endsWith(U_STRING_TO_PARAM(*ext), U_CONSTANT_TO_PARAM(U_CRLF)))

      (void) tmp.append(*ext);

#  ifdef DEBUG
      if ((mode & PARAM_DEPENDENCY) != 0) ext->clear(); // NB: ext has a dependency on UClientImage_Base::wbuffer...
#  endif
      }

   if ((mode & NO_BODY) != 0) setResponse(tmp, 0);
   else
      {
      char msg[4096];
      uint32_t sz, len;

      if ((mode & NETWORK_AUTHENTICATION_REQUIRED) != 0)
         {
         len = u__snprintf(msg, sizeof(msg), U_CONSTANT_TO_PARAM("You need to <a href=\"%.*s\">authenticate with the local network</a> in order to get access"), len_location, ptr_location);
         }
      else
         {
         len = u__snprintf(msg, sizeof(msg), U_CONSTANT_TO_PARAM("The document has moved <a href=\"%.*s\">here</a>"), len_location, ptr_location);
         }

      UString body(500U + len_location);
      const char* status = getStatusDescription(&sz);

      body.snprintf(U_CONSTANT_TO_PARAM(U_STR_FMR_BODY),
                    U_http_info.nResponseCode, sz, status,
                                               sz, status,
                    len, msg);

      setResponse(tmp, &body);
      }
}

void UHTTP::setErrorResponse(const UString& content_type, int code, const char* fmt, uint32_t fmt_size, bool flag)
{
   U_TRACE(0, "UHTTP::setErrorResponse(%V,%d,%.*S,%u,%b)", content_type.rep, code, fmt_size, fmt, fmt_size, flag)

   U_INTERNAL_ASSERT(U_IS_HTTP_ERROR(code))

   UClientImage_Base::setCloseConnection();

   U_INTERNAL_DUMP("U_http_is_request_nostat = %b U_HTTP_URI_QUERY_LEN = %u", U_http_is_request_nostat, U_HTTP_URI_QUERY_LEN)

   if (flag == false             &&
       (U_http_is_request_nostat ||
        U_HTTP_URI_QUERY_LEN == 0))
      {
      code = HTTP_BAD_REQUEST;

      fmt      =                 "Your browser sent a request that this server could not understand";
      fmt_size = U_CONSTANT_SIZE("Your browser sent a request that this server could not understand");
      }

   UString body(1000U);

   body.snprintf(U_CONSTANT_TO_PARAM("ErrorDocument/%u.html"), U_http_info.nResponseCode = code);

   UHTTP::UFileCacheData* ptr_file_data = cache_file->at(body);

   if (ptr_file_data &&
       ptr_file_data->array != 0)
      {
      body = (*ptr_file_data->array)[0];
      }
   else
      {
      uint32_t sz;
      const char* status = getStatusDescription(&sz);

      if (flag)
         {
         body.snprintf(U_CONSTANT_TO_PARAM(U_STR_FMR_BODY),
                       code, sz, status,
                             sz, status,
                       fmt_size, fmt);
         }
      else
         {
         // NB: we encoding to avoid cross-site scripting (XSS)...

         char output[512];
         char buffer[4096];

         uint32_t len = u__snprintf(buffer, sizeof(buffer), fmt, fmt_size,
                                    u_xml_encode((const unsigned char*)U_http_info.uri, U_min(100, U_HTTP_URI_QUERY_LEN), (unsigned char*)output), output);

         body.snprintf(U_CONSTANT_TO_PARAM(U_STR_FMR_BODY),
                       code, sz, status,
                             sz, status,
                       len, buffer);
         }
      }

   setResponse(content_type, &body);
}

void UHTTP::setUnAuthorized()
{
   U_TRACE_NO_PARAM(0, "UHTTP::setUnAuthorized()")

   UString tmp(100U);

   (void) tmp.assign(U_CONSTANT_TO_PARAM(U_CTYPE_HTML "\r\nWWW-Authenticate: "));

   U_INTERNAL_DUMP("digest_authentication = %b", digest_authentication)

   if (digest_authentication)        tmp.snprintf_add(U_CONSTANT_TO_PARAM("Digest qop=\"auth\", nonce=\"%ld\", algorithm=MD5,"), u_now->tv_sec);
   else                       (void) tmp.append(U_CONSTANT_TO_PARAM("Basic"));

   (void) tmp.append(U_CONSTANT_TO_PARAM(" realm=\"" U_HTTP_REALM "\"\r\n"));

   UClientImage_Base::setRequestForbidden();

   setErrorResponse(tmp, HTTP_UNAUTHORIZED,
                    U_CONSTANT_TO_PARAM("This server could not verify that you are authorized to access the document requested. Either you supplied the "
                                        "wrong credentials (e.g., bad password), or your browser doesn't understand how to supply the credentials required"), true);
}

// --------------------------------------------------------------------------------------------------------------------------------------
// end HTTP main error message
// --------------------------------------------------------------------------------------------------------------------------------------

void UHTTP::setDynamicResponse()
{
   U_TRACE_NO_PARAM(0, "UHTTP::setDynamicResponse()")

   U_INTERNAL_DUMP("U_http_info.endHeader = %u U_http_content_type_len = %u mime_index(%d) = %C",
                    U_http_info.endHeader,     U_http_content_type_len,     mime_index, mime_index)

   U_INTERNAL_DUMP("UClientImage_Base::wbuffer(%u) = %V", UClientImage_Base::wbuffer->size(), UClientImage_Base::wbuffer->rep)

   U_INTERNAL_ASSERT(*UClientImage_Base::wbuffer)
   U_INTERNAL_ASSERT_MAJOR(U_http_info.nResponseCode, 0)

   int ratio;
   char* ptr;
   char* ptr1;
   UString compress;
   const char* pEndHeader;
   uint32_t sz = 0, size, clength = UClientImage_Base::wbuffer->size();

   ext->setBuffer(U_CAPACITY);

   ptr = ext->data();

#ifndef USE_LIBZ
   bool bcompress = false;
#else
   bool bcompress = (U_http_is_accept_gzip &&
                     UClientImage_Base::wbuffer->size() > (U_http_info.endHeader + U_MIN_SIZE_FOR_DEFLATE));
#endif

   U_INTERNAL_DUMP("bcompress = %b", bcompress)

   if (U_http_info.endHeader)
      {
      U_INTERNAL_ASSERT(clength >= U_http_info.endHeader)

      clength -= U_http_info.endHeader;

      if (clength == 0) // no response
         {
         UClientImage_Base::body->clear(); // clean body to avoid writev() in response...

         UClientImage_Base::wbuffer->clear();

         goto no_response;
         }

      pEndHeader = UClientImage_Base::wbuffer->data();

      if (bcompress == false) goto no_compress;
      }
   else
      {
      pEndHeader = 0;

      if (bcompress == false)
         {
         *UClientImage_Base::body = *UClientImage_Base::wbuffer; 

         goto end;
         }
      }

   compress = UStringExt::deflate(UClientImage_Base::wbuffer->c_pointer(U_http_info.endHeader), clength, 1);

   size = compress.size();

   if (size < clength)
      {
      ratio = (size * 100U) / clength;

      U_INTERNAL_DUMP("ratio = %d (%d%%)", ratio, 100 - ratio)

      // NB: we accept new data only if ratio compression is better than 15%...

      if (ratio >= 85)
         {
no_compress:
         (void) UClientImage_Base::body->replace(pEndHeader + U_http_info.endHeader, clength);

         goto end;
         }

      clength = size;
           sz = U_CONSTANT_SIZE("Content-Encoding: gzip\r\n");

      u_put_unalignedp64(ptr,    U_MULTICHAR_CONSTANT64('C','o','n','t','e','n','t','-'));
      u_put_unalignedp64(ptr+8,  U_MULTICHAR_CONSTANT64('E','n','c','o','d','i','n','g'));
      u_put_unalignedp64(ptr+16, U_MULTICHAR_CONSTANT64(':',' ','g','z','i','p','\r','\n'));

      *UClientImage_Base::body = compress;

      U_SRV_LOG("CGI response: %u bytes - (%d%%) compression ratio", size, 100 - ratio);
      }

end:
   if (U_http_content_type_len != 1)
      {
      // NB: we assume that we don't have a content-type HTTP header...

#  if defined(DEBUG) && defined(USE_LIBMAGIC)
      if (clength > 4)
         {
         const char* p = UClientImage_Base::wbuffer->c_pointer(U_http_info.endHeader);

         if (u_isText((const unsigned char*)p, clength))
            {
            UString tmp = UMagic::getType(p, clength);

            U_INTERNAL_ASSERT_EQUALS(memcmp(tmp.data(), U_CONSTANT_TO_PARAM("text")), 0)
            }
         }
#  endif

      ptr1 = ptr+sz;

      u_put_unalignedp64(ptr1,    U_MULTICHAR_CONSTANT64('C','o','n','t','e','n','t','-'));
      u_put_unalignedp64(ptr1+8,  U_MULTICHAR_CONSTANT64('T','y','p','e',':',' ','t','e'));
      u_put_unalignedp16(ptr1+16, U_MULTICHAR_CONSTANT16('x','t'));

      ptr1 += U_CONSTANT_SIZE("Content-Type: text");

      if (u_is_html(mime_index))
         {
         u_put_unalignedp64(ptr1,    U_MULTICHAR_CONSTANT64('/','h','t','m','l',';',' ','c'));
         u_put_unalignedp64(ptr1+8,  U_MULTICHAR_CONSTANT64('h','a','r','s','e','t','=','U'));
         u_put_unalignedp32(ptr1+16, U_MULTICHAR_CONSTANT32('T','F','-','8'));
         u_put_unalignedp16(ptr1+20, U_MULTICHAR_CONSTANT16('\r','\n'));

         sz += U_CONSTANT_SIZE("Content-Type: " U_CTYPE_HTML "\r\n");
         }
      else
         {
#     if defined(U_LINUX) && defined(ENABLE_THREAD) && defined(U_LOG_DISABLE) && !defined(USE_LIBZ)
         u_put_unalignedp64(ptr1, U_MULTICHAR_CONSTANT64('/','p','l','a','i','n','\r','\n'));

         sz += U_CONSTANT_SIZE("Content-Type: text/plain\r\n");
#     else
         u_put_unalignedp64(ptr1,    U_MULTICHAR_CONSTANT64('/','p','l','a','i','n',';',' '));
         u_put_unalignedp64(ptr1+8,  U_MULTICHAR_CONSTANT64('c','h','a','r','s','e','t','='));
         u_put_unalignedp32(ptr1+16, U_MULTICHAR_CONSTANT32('U','T','F','-'));
                            ptr1[20] = '8';
         u_put_unalignedp16(ptr1+21, U_MULTICHAR_CONSTANT16('\r','\n'));

         sz += U_CONSTANT_SIZE("Content-Type: " U_CTYPE_TEXT_WITH_CHARSET "\r\n");
#     endif
         }
      }

   ptr1 = ptr+sz;

   u_put_unalignedp64(ptr1,   U_MULTICHAR_CONSTANT64('C','o','n','t','e','n','t','-'));
   u_put_unalignedp64(ptr1+8, U_MULTICHAR_CONSTANT64('L','e','n','g','t','h',':',' '));

   ptr1 = u_num2str32(clength, ptr1 + U_CONSTANT_SIZE("Content-Length: "));

   if (pEndHeader == 0)
      {
      u_put_unalignedp32(ptr1, U_MULTICHAR_CONSTANT32('\r','\n','\r','\n'));
                         ptr1 += 4;
      }
   else
      {
      u_put_unalignedp16(ptr1, U_MULTICHAR_CONSTANT16('\r','\n'));
                         ptr1 += 2;

      U_INTERNAL_ASSERT(u_endsWith(pEndHeader, U_http_info.endHeader, U_CONSTANT_TO_PARAM(U_CRLF2)))

      U_MEMCPY(ptr1, pEndHeader, U_http_info.endHeader);
               ptr1 +=           U_http_info.endHeader;
      }

   ext->size_adjust(ptr1);

no_response:
   handlerResponse();
}

U_NO_EXPORT bool UHTTP::processAuthorization()
{
   U_TRACE_NO_PARAM(0, "UHTTP::processAuthorization()")

   bool result = false, bpass = false;
   const char* ptr = getHeaderValuePtr(U_CONSTANT_TO_PARAM("Authorization"), false);

   if (ptr)
      {
      UTokenizer t;
      const char* psuffix;
      uint32_t sz, pos = 0;
      UHTTP::UFileCacheData* ptr_file_data;
      UString content, tmp, user(100U), buffer(100U), fpasswd;

      U_INTERNAL_DUMP("digest_authentication = %b", digest_authentication)

      if (digest_authentication)
         {
         if (u_get_unalignedp32(ptr) != U_MULTICHAR_CONSTANT32('D','i','g','e')) goto end;

         ptr += U_CONSTANT_SIZE("Digest ");
         }
      else
         {
         if (u_get_unalignedp32(ptr) != U_MULTICHAR_CONSTANT32('B','a','s','i')) goto end;

         ptr += U_CONSTANT_SIZE("Basic ");
         }

      tmp = UClientImage_Base::request->substr(UClientImage_Base::request->distance(ptr));

      t.setData(tmp);
      t.setDelimiter(U_CRLF2);

      if (t.next(content, (bool*)0) == false) goto end;

      ptr = UClientImage_Base::getRequestUri(sz);

      psuffix = u_getsuffix(ptr, sz);

      if (psuffix)
         {
         U_INTERNAL_ASSERT_EQUALS(psuffix[0], '.')

         pos = (ptr + sz) - psuffix;
         }

      buffer.snprintf(U_CONSTANT_TO_PARAM("..%.*s.ht%s"), sz-pos, ptr, digest_authentication ? "digest" : "passwd");

      ptr_file_data = cache_file->at(U_STRING_TO_PARAM(buffer));

      if (ptr_file_data) fpasswd = ptr_file_data->array->operator[](0);
      else
         {
         if (digest_authentication)
            {
            if (htdigest) fpasswd = *htdigest;
            }
         else
            {
            if (htpasswd) fpasswd = *htpasswd;
            }
         }

      if (fpasswd.empty()) goto end;

      bpass = true;

      if (digest_authentication)
         {
         /*
          * A user requests page protected by digest auth:
          *
          * - The server sends back a 401 and a WWW-Authenticate header with the value of digest along with a nonce value and a realm value
          * - The user concatenates his credentials with the nonce and realm and uses that as input to MD5 to produce one has (HA1)
          * - The user concatenates the method and the URI to create a second MD5 hash (HA2)
          * - The user then sends an Authorize header with the realm, nonce, URI, and the response which is the MD5 of the two previous hashes combined
          *
          * Authorization: Digest username="s.casazza", realm="Protected Area", nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093", uri="/",
          *                response="a74c1cb52877766bb0781d12b653d1a7", qop=auth, nc=00000001, cnonce="73ed2d9694b46324", algorithm=MD5
          */

         UVector<UString> name_value;
         UString name, value, realm, nonce, _uri, response, qop, nc, cnonce;

         for (int32_t i = 0, n = UStringExt::getNameValueFromData(content, name_value, U_CONSTANT_TO_PARAM(", \t")); i < n; i += 2)
            {
            name  = name_value[i];
            value = name_value[i+1];

            U_INTERNAL_DUMP("name = %V value = %V", name.rep, value.rep)

            switch (name.c_char(0))
               {
               case 'u':
                  {
                  if (name.equal(U_CONSTANT_TO_PARAM("username")))
                     {
                     U_ASSERT(user.empty())

                     user = value;
                     }
                  else if (name.equal(U_CONSTANT_TO_PARAM("uri")))
                     {
                     U_ASSERT(_uri.empty())

                     if (sz > value.size()   ||
                         memcmp(ptr, value.data(), sz))
                        {
                        goto end;
                        }
               
                     _uri = value;
                     }
                  }
               break;

               case 'r':
                  {
                  if (name.equal(U_CONSTANT_TO_PARAM("realm")))
                     {
                     U_ASSERT(realm.empty())

                     realm = value;
                     }
                  else if (name.equal(U_CONSTANT_TO_PARAM("response")))
                     {
                     U_ASSERT(response.empty())

                     if (value.size() != 32) goto end;

                     response = value;
                     }
                  }
               break;

               case 'n':
                  {
                  if (name.equal(U_CONSTANT_TO_PARAM("nonce")))
                     {
                     U_ASSERT(nonce.empty())

                     // XXX: Due to a bug in MSIE (version=??), we do not check for authentication timeout...

                     if ((u_now->tv_sec - value.strtoul()) > 3600) goto end;

                     nonce = value;
                     }
                  else if (name.equal(U_CONSTANT_TO_PARAM("nc")))
                     {
                     U_ASSERT(nc.empty())

                     nc = value;
                     }
                  }
               break;

               case 'q':
                  {
                  if (name.equal(U_CONSTANT_TO_PARAM("qop")))
                     {
                     U_ASSERT(qop.empty())

                     qop = value;
                     }
                  }
               break;

               case 'c':
                  {
                  if (name.equal(U_CONSTANT_TO_PARAM("cnonce")))
                     {
                     U_ASSERT(cnonce.empty())

                     cnonce = value;
                     }
                  }
               break;

               case 'a':
                  {
                  if (name.equal(U_CONSTANT_TO_PARAM("algorithm")))
                     {
                     if (value.equal("MD5") == false) goto end;
                     }
                  }
               break;
               }
            }

         UString  a2(4 + 1 + _uri.size()), //     method : uri
                 ha2(33U),                 // MD5(method : uri)
                 ha1,                      // MD5(user : realm : password)
                  a3(200U),
                 ha3(33U);

         // MD5(user : realm : password)

         buffer.snprintf(U_CONSTANT_TO_PARAM("%v:%v:"), user.rep, realm.rep);

         // s.casazza:Protected Area:b9ee2af50be37...........\n

         pos = fpasswd.find(buffer);

         if (pos != U_NOT_FOUND)
            {
            pos += buffer.size();
            ha1  = fpasswd.substr(pos, 32);

            U_INTERNAL_ASSERT_EQUALS(fpasswd.c_char(pos + 32), '\n')
            }

         // MD5(method : uri)

         a2.snprintf(U_CONSTANT_TO_PARAM("%.*s:%v"), U_HTTP_METHOD_TO_TRACE, _uri.rep);

         UServices::generateDigest(U_HASH_MD5, 0, a2, ha2, false);

         // --------------------------------------------------------------------------
         // MD5(HA1 : nonce : nc : cnonce : qop : HA2)
         // --------------------------------------------------------------------------
         // NB: wget supports only Basic auth as the only auth type over HTTP proxy...
         // --------------------------------------------------------------------------
         // if (qop.empty()) (void) qop.assign(U_CONSTANT_TO_PARAM("auth"));
         // --------------------------------------------------------------------------

         a3.snprintf(U_CONSTANT_TO_PARAM("%v:%v:%v:%v:%v:%v"), ha1.rep, nonce.rep, nc.rep, cnonce.rep, qop.rep, ha2.rep);

         UServices::generateDigest(U_HASH_MD5, 0, a3, ha3, false);

         result = (ha3 == response);
         }
      else
         {
         // Authorization: Basic cy5jYXNhenphOnN0ZWZhbm8x==

         UBase64::decode(content, buffer);

         if (buffer)
            {
            t.setData(buffer);
            t.setDelimiter(":");

            UString password(100U);

            if (t.next(user,     (bool*)0) &&
                t.next(password, (bool*)0))
               {
               UString line(100U), output(100U);

               UServices::generateDigest(U_HASH_SHA1, 0, password, output, true);

               line.snprintf(U_CONSTANT_TO_PARAM("%v:{SHA}%v\n"), user.rep, output.rep);

               // s.casazza:{SHA}Lkii1ZE7k.....\n

               if (fpasswd.find(line) != U_NOT_FOUND) result = true;
               }
            }
         }
end:
      U_SRV_LOG("%srequest authorization for user %V %s", result ? "" : "WARNING: ", user.rep, result ? "success" : "failed");
      }

   if (result == false)
      {
      // NB: we cannot authorize someone if it is not present above document root almost one passwd file... 

      if (bpass || htdigest || htpasswd) setUnAuthorized();
      else                                  setForbidden();

      U_RETURN(false);
      }

   U_RETURN(true);
}

__pure bool UHTTP::isSOAPRequest()
{
   U_TRACE_NO_PARAM(0, "UHTTP::isSOAPRequest()")

   // NB: soap is NOT a static page, so to avoid stat() syscall we use alias mechanism...

   U_INTERNAL_DUMP("U_http_is_request_nostat = %b", U_http_is_request_nostat)

#ifdef U_ALIAS
   if (isPOST()                                                             &&
       U_http_is_request_nostat                                             &&
       (UClientImage_Base::request_uri->equal(U_CONSTANT_TO_PARAM("/soap")) ||
        U_HTTP_CTYPE_STREQ("application/soap+xml")))
      {
      U_RETURN(true);
      }
#endif

   U_RETURN(false);
}

__pure bool UHTTP::isTSARequest()
{
   U_TRACE_NO_PARAM(0, "UHTTP::isTSARequest()")

   // NB: tsa is NOT a static page, so to avoid stat() syscall we use alias mechanism...

   U_INTERNAL_DUMP("U_http_is_request_nostat = %b", U_http_is_request_nostat)

#ifdef U_ALIAS
   if (isPOST()                                                            &&
       U_http_is_request_nostat                                            &&
       (UClientImage_Base::request_uri->equal(U_CONSTANT_TO_PARAM("/tsa")) ||
        U_HTTP_CTYPE_STREQ("application/timestamp-query")))
      {
      // NB: process the HTTP tsa request with fork....

      if (UServer_Base::startParallelization() == false) U_RETURN(true); // child of parallelization
      }
#endif

   U_RETURN(false);
}

bool UHTTP::isProxyRequest()
{
   U_TRACE_NO_PARAM(0, "UHTTP::isProxyRequest()")

   U_INTERNAL_DUMP("U_http_is_request_nostat = %b", U_http_is_request_nostat)

   if (UClientImage_Base::isRequestNotFound())
      {
      // check a possibly proxy service for the HTTP request

      service = UModProxyService::findService();

      if (service)
         {
#     ifdef USE_LIBSSL
         if (service->isRequestCertificate() && // NB: check if it is required a certificate for this service...
             UServer_Base::pClientImage->askForClientCertificate() == false)
            {
            service = 0;

            UModProxyService::setMsgError(UModProxyService::BAD_REQUEST);
            }
#     endif

         U_INTERNAL_DUMP("service->server = %V", service->server.rep)

         // NB: process the HTTP PROXY request with fork....

         if (UServer_Base::startParallelization() == false) U_RETURN(true); // child of parallelization
         }
      }

   U_RETURN(false);
}

__pure bool UHTTP::isFCGIRequest()
{
   U_TRACE_NO_PARAM(0, "UHTTP::isFCGIRequest()")

   U_INTERNAL_DUMP("U_http_is_request_nostat = %b", U_http_is_request_nostat)

   U_INTERNAL_ASSERT_POINTER(fcgi_uri_mask)

   if (UClientImage_Base::isRequestNotFound())
      {
      uint32_t sz;
      const char* ptr = UClientImage_Base::getRequestUri(sz);

      if (UServices::dosMatchWithOR(ptr, sz, U_STRING_TO_PARAM(*fcgi_uri_mask), 0)) U_RETURN(true);
      }

   U_RETURN(false);
}

__pure bool UHTTP::isSCGIRequest()
{
   U_TRACE_NO_PARAM(0, "UHTTP::isSCGIRequest()")

   U_INTERNAL_DUMP("U_http_is_request_nostat = %b", U_http_is_request_nostat)

   U_INTERNAL_ASSERT_POINTER(scgi_uri_mask)

   if (UClientImage_Base::isRequestNotFound())
      {
      uint32_t sz;
      const char* ptr = UClientImage_Base::getRequestUri(sz);

      if (UServices::dosMatchWithOR(ptr, sz, U_STRING_TO_PARAM(*scgi_uri_mask), 0)) U_RETURN(true);
      }

   U_RETURN(false);
}

#ifdef USE_LIBSSL
bool UHTTP::checkUriProtected()
{
   U_TRACE_NO_PARAM(0, "UHTTP::checkUriProtected()")

   U_INTERNAL_ASSERT_POINTER(uri_protected_mask)

   if (vallow_IP)
      {
      bool ok = UClientImage_Base::isAllowed(*vallow_IP);

      if (ok &&
          U_http_ip_client_len)
         {
         ok = UIPAllow::isAllowed(UServer_Base::client_address, *vallow_IP);
         }

      if (ok == false)
         {
         setForbidden();

         U_SRV_LOG("URI_PROTECTED: request %.*S denied by access list", U_HTTP_URI_TO_TRACE);

         U_RETURN(false);
         }
      }

   // check if it's OK via authentication (digest|basic)

   if (processAuthorization()) U_RETURN(true);

   U_RETURN(false);
}

__pure bool UHTTP::isUriRequestProtected()
{
   U_TRACE_NO_PARAM(0, "UHTTP::isUriRequestProtected()")

   // check if the uri is protected

   if (uri_protected_mask)
      {
      uint32_t sz;
      const char* ptr = UClientImage_Base::getRequestUri(sz);

      if (UServices::dosMatchWithOR(ptr, sz, U_STRING_TO_PARAM(*uri_protected_mask), 0)) U_RETURN(true);
      }

   U_RETURN(false);
}
#endif

__pure bool UHTTP::isUriRequestNeedCertificate()
{
   U_TRACE_NO_PARAM(0, "UHTTP::isUriRequestNeedCertificate()")

   // check if the uri request need a certificate

#ifdef USE_LIBSSL
   if (UServer_Base::bssl &&
       uri_request_cert_mask)
      {
      uint32_t sz;
      const char* ptr = UClientImage_Base::getRequestUri(sz);

      if (UServices::dosMatchWithOR(ptr, sz, U_STRING_TO_PARAM(*uri_request_cert_mask), 0)) U_RETURN(true);
      }
#endif

   U_RETURN(false);
}

#ifdef U_HTTP_STRICT_TRANSPORT_SECURITY
__pure bool UHTTP::isUriRequestStrictTransportSecurity()
{
   U_TRACE_NO_PARAM(0, "UHTTP::isUriRequestStrictTransportSecurity()")

   // check if the uri use HTTP Strict Transport Security to force client to use secure connections only

   if (uri_strict_transport_security_mask)
      {
#  ifdef USE_LIBSSL
      if (UServer_Base::bssl == false)
#  endif
      {
      uint32_t sz;
      const char* ptr = UClientImage_Base::getRequestUri(sz);

      if (UServices::dosMatchWithOR(ptr, sz, U_STRING_TO_PARAM(*uri_strict_transport_security_mask), 0)) U_RETURN(true);
      }
      }

   U_RETURN(false);
}
#endif

// NB: the tables must be ordered alphabetically because we use binary search algorithm...

void UHTTP::manageRequest(service_info*  GET_table, uint32_t n1,
                          service_info* POST_table, uint32_t n2)
{
   U_TRACE(0, "UHTTP::manageRequest(%p,%u,%p,%u)", GET_table, n1, POST_table, n2)

   int32_t high;
   service_info* table;

   if (isGETorHEAD())
      {
      high  = n1;
      table = GET_table;
      }
   else if (isPOSTorPUTorPATCH())
      {
      high  = n2;
      table = POST_table;
      }
   else
      {
      U_http_info.nResponseCode = HTTP_BAD_METHOD;

      return;
      }

   if (high == 0)
      {
not_found:
      U_http_info.nResponseCode = HTTP_BAD_REQUEST;

      return;
      }

   service_info* key;
   uint32_t target_len;
   const char* psuffix;
   int32_t cmp = -1, probe, low = -1;
   const char* target = UClientImage_Base::getRequestUri(target_len);

   // NB: skip '/'...

   if (target[0] != '/') goto not_found;

   target     += 1;
   target_len -= 1;

   psuffix = u_getsuffix(target, target_len);

   if (psuffix)
      {
      U_INTERNAL_ASSERT_EQUALS(psuffix[0], '.')

      target_len -= (target + target_len) - psuffix;
      }

   U_INTERNAL_DUMP("target(%u) = %.*S", target_len, target_len, target)

   while ((high - low) > 1)
      {
      probe = ((low + high) & 0xFFFFFFFFL) >> 1;

      U_INTERNAL_DUMP("low = %d high = %d probe = %d", low, high, probe)

      key = table + probe;

      U_INTERNAL_DUMP("key(%u) = %.*S", key->len, key->len, key->name)

      // NB: skip 'GET_' or 'POST_'...

      cmp = memcmp(key->name, target, U_min(key->len, target_len));

      if (cmp == 0) cmp = (key->len - target_len);
      
           if (cmp  > 0) high = probe;
      else if (cmp == 0) goto found;
      else               low = probe;
      }

   if (low == -1 ||
       (key = table + low, memcmp(key->name, target, U_min(key->len, target_len))) != 0)
      {
      goto not_found;
      }

   probe = low;

found:
   table[probe].function();
}

UString UHTTP::getHeaderMimeType(const char* content, uint32_t size, const char* content_type, time_t expire, bool content_length_changeable)
{
   U_TRACE(0, "UHTTP::getHeaderMimeType(%.*S,%u,%S,%ld,%b)", size, content, size, content_type, expire, content_length_changeable)

   U_INTERNAL_ASSERT_POINTER(file)
   U_INTERNAL_ASSERT_POINTER(content_type)

   UString header(U_CAPACITY);

   // check magic byte

   if (                   content &&
       u_get_unalignedp16(content) == U_MULTICHAR_CONSTANT16('\x1F','\x8B'))
      {
      if (file_data)
         {
         U_INTERNAL_DUMP("mime_index(%d) = %C file_data->mime_index(%d) = %C", mime_index, mime_index, file_data->mime_index, file_data->mime_index)

         mime_index = file_data->mime_index = U_gz;
         }

      if (u_get_unalignedp64(content_type)    == U_MULTICHAR_CONSTANT64('a','p','p','l','i','c','a','t') &&
          u_get_unalignedp64(content_type+8)  == U_MULTICHAR_CONSTANT64('i','o','n','/','x','-','g','z') &&
          u_get_unalignedp16(content_type+16) == U_MULTICHAR_CONSTANT16('i','p'))
         {
         content_type = file->getMimeType();
         }

      U_INTERNAL_ASSERT_DIFFERS(memcmp(content_type, U_CONSTANT_TO_PARAM("application/x-gzip")), 0)

      (void) header.assign(U_CONSTANT_TO_PARAM("Content-Encoding: gzip\r\n"));
      }

   /**
    * http://code.google.com/speed/page-speed/docs/caching.html
    *
    * HTTP/1.1 provides the following caching response headers:
    *
    * Expires and Cache-Control: max-age. These specify the freshness lifetime of a resource, that is, the time period during which the browser
    * can use the cached resource without checking to see if a new version is available from the web server. They are "strong caching headers"
    * that apply unconditionally; that is, once they're set and the resource is downloaded, the browser will not issue any GET requests for the
    * resource until the expiry date or maximum age is reached.
    *
    * Last-Modified and ETag. These specify some characteristic about the resource that the browser checks to determine if the files are the same.
    * In the Last-Modified header, this is always a date. In the ETag header, this can be any value that uniquely identifies a resource (file
    * versions or content hashes are typical). Last-Modified is a "weak" caching header in that the browser applies a heuristic to determine
    * whether to fetch the item from cache or not. (The heuristics are different among different browsers.) However, these headers allow the
    * browser to efficiently update its cached resources by issuing conditional GET requests when the user explicitly reloads the page. Conditional
    * GETs don't return the full response unless the resource has changed at the server, and thus have lower latency than full GETs.
    *
    * It is important to specify one of Expires or Cache-Control max-age, and one of Last-Modified or ETag, for all cacheable resources. It is
    * redundant to specify both Expires and Cache-Control: max-age, or to specify both Last-Modified and ETag. 
    *
    * Recommendations: set caching headers aggressively for all static resources.
    *
    * For all cacheable resources, we recommend the following settings:
    *
    * Set Expires to a minimum of one month, and preferably up to one year, in the future. (We prefer Expires over Cache-Control: max-age because
    * it is is more widely supported.) Do not set it to more than one year in the future, as that violates the RFC guidelines. If you know exactly
    * when a resource is going to change, setting a shorter expiration is okay. But if you think it "might change soon" but don't know when, you
    * should set a long expiration and use URL fingerprinting (described below). Setting caching aggressively does not "pollute" browser caches:
    * as far as we know, all browsers clear their caches according to a Least Recently Used algorithm; we are not aware of any browsers that wait
    * until resources expire before purging them.
    * Set the Last-Modified date to the last time the resource was changed. If the Last-Modified date is sufficiently far enough in the past,
    * chances are the browser won't refetch it.
    *
    * Use fingerprinting to dynamically enable caching. For resources that change occasionally, you can have the browser cache the resource until
    * it changes on the server, at which point the server tells the browser that a new version is available. You accomplish this by embedding a
    * fingerprint of the resource in its URL (i.e. the file path). When the resource changes, so does its fingerprint, and in turn, so does its URL.
    * As soon as the URL changes, the browser is forced to re-fetch the resource. Fingerprinting allows you to set expiry dates long into the future
    * even for resources that change more frequently than that. Of course, this technique requires that all of the pages that reference the resource
    * know about the fingerprinted URL, which may or may not be feasible, depending on how your pages are coded.
    *
    * Set the Vary header correctly for Internet Explorer. Internet Explorer does not cache any resources that are served with the Vary header and any
    * fields but Accept-Encoding and User-Agent. To ensure these resources are cached by IE, make sure to strip out any other fields from the Vary
    * header, or remove the Vary header altogether if possible
    *
    * The rule is any resource that is cachable should not have a Vary: User-Agent header.
    *
    * Avoid URLs that cause cache collisions in Firefox. The Firefox disk cache hash functions can generate collisions for URLs that differ only
    * slightly, namely only on 8-character boundaries. When resources hash to the same key, only one of the resources is persisted to disk cache;
    * the remaining resources with the same key have to be re-fetched across browser restarts. Thus, if you are using fingerprinting or are otherwise
    * programmatically generating file URLs, to maximize cache hit rate, avoid the Firefox hash collision issue by ensuring that your application
    * generates URLs that differ on more than 8-character boundaries. 
    *
    * Use the Cache control: public directive to enable HTTPS caching for Firefox. Some versions of Firefox require that the Cache control: public
    * header to be set in order for resources sent over SSL to be cached on disk, even if the other caching headers are explicitly set. Although this
    * header is normally used to enable caching by proxy servers (as described below), proxies cannot cache any content sent over HTTPS, so it is
    * always safe to set this header for HTTPS resources
    */

   header.snprintf_add(U_CONSTANT_TO_PARAM("Content-Type: %s\r\n"), content_type);

   U_INTERNAL_DUMP("mime_index(%d) = %C", mime_index, mime_index)

   if (content_length_changeable == false)
      {
      if (u__isdigit(mime_index)) goto next; // NB: check for dynamic page...

      if (expire) header.snprintf_add(U_CONSTANT_TO_PARAM("Expires: %#8D\r\n"), expire);
                  header.snprintf_add(U_CONSTANT_TO_PARAM("Last-Modified: %#8D\r\n"), file->st_mtime);
      }

   U_INTERNAL_DUMP("u_is_css(%d) = %b u_is_js(%d) = %b", mime_index, u_is_css(mime_index), mime_index, u_is_js(mime_index))

#ifdef DEBUG
   if (u_is_css(mime_index))
      {
      U_INTERNAL_ASSERT(u_endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(".css")))
      U_INTERNAL_ASSERT_EQUALS(memcmp(content_type, U_CONSTANT_TO_PARAM("text/css")), 0)

   // (void) header.append("Content-Style-Type: text/css\r\n");
      }
   else if (u_is_js(mime_index))
      {
      U_INTERNAL_ASSERT(u_endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(".js")))
      U_INTERNAL_ASSERT_EQUALS(memcmp(content_type, U_CONSTANT_TO_PARAM("application/javascript")), 0)

   // (void) header.append("Content-Script-Type: application/javascript\r\n");
      }
#endif

   U_INTERNAL_DUMP("enable_caching_by_proxy_servers = %b", enable_caching_by_proxy_servers)

   if (enable_caching_by_proxy_servers)
      {
      (void) header.append(U_CONSTANT_TO_PARAM("Cache-Control: public\r\n"
                                               "Vary: Accept-Encoding\r\n"
                                               "Accept-Ranges: bytes\r\n"));
      }

   // A server implements an HSTS policy by supplying a header over an HTTPS connection (HSTS headers over HTTP are ignored)

#if defined(USE_LIBSSL) && defined(U_HTTP_STRICT_TRANSPORT_SECURITY)
   if (uri_strict_transport_security_mask == (void*)1L)
      {
      U_INTERNAL_ASSERT(UServer_Base::bssl)

      (void) header.append(U_CONSTANT_TO_PARAM("Strict-Transport-Security: max-age=31536000; includeSubDomains; preload\r\n"));
      }
#endif

next:
   if (content_length_changeable)
      {
      if (size)   header.snprintf_add(U_CONSTANT_TO_PARAM("Content-Length:    %u\r\n\r\n"), size);
      else (void) header.append(      U_CONSTANT_TO_PARAM("Content-Length:    %u\r\n\r\n"));
      }
   else
      {
      if (size)   header.snprintf_add(U_CONSTANT_TO_PARAM("Content-Length: %u\r\n\r\n"), size);
      else (void) header.append(      U_CONSTANT_TO_PARAM("Content-Length: %u\r\n\r\n"));
      }

   (void) header.shrink();

   U_RETURN_STRING(header);
}

U_NO_EXPORT void UHTTP::putDataInCache(const UString& fmt, UString& content)
{
   U_TRACE(0, "UHTTP::putDataInCache(%V,%V)", fmt.rep, content.rep)

   U_INTERNAL_ASSERT_POINTER(file)
   U_INTERNAL_ASSERT_POINTER(file_data)
   U_INTERNAL_ASSERT(fmt.isNullTerminated())
   U_INTERNAL_ASSERT_MAJOR(file_data->size, 0)
   U_INTERNAL_ASSERT_EQUALS(file_data->mime_index, mime_index)

   uint32_t size;
   int ratio = 100;
   bool gzip = false;
   const char* motivation = 0;
   UString header(U_CAPACITY);

   U_NEW(UVector<UString>, file_data->array, UVector<UString>(4U));

   file_data->array->push_back(content);

   header.snprintf(U_STRING_TO_PARAM(fmt), file_data->size);

   (void) header.shrink();

   file_data->array->push_back(header);

   if (u_is_img(mime_index))
      {
      U_INTERNAL_ASSERT(u_endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(".gif")) ||
                        u_endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(".png")) ||
                        u_endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(".ico")) ||
                        u_endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(".jpg")) ||
                        u_endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(".jpeg")))

#if defined(USE_PAGE_SPEED) && defined(DEBUG)
           if (u_is_gif(mime_index)) page_speed->optimize_gif(content);
      else if (u_is_png(mime_index)) page_speed->optimize_png(content);
      else                           page_speed->optimize_jpg(content);

      if (content.size() < file_data->size) U_SRV_LOG("WARNING: found not optimized image: %V", pathname->rep);
#endif

      goto next2;
      }

   if (u_is_js( mime_index) ||
       u_is_css(mime_index))
      {
      // NB: check if it is already minified...

      if (u_is_js(mime_index))
         {
         U_INTERNAL_ASSERT(u_endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(".js")))

         if (UStringExt::endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(  ".min.js")) ||
             UStringExt::endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(".nomin.js")))
            {
            goto next1;
            }
         }
      else
         {
         U_INTERNAL_ASSERT(u_is_css(mime_index))
         U_INTERNAL_ASSERT(u_endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(".css")))

         if (UStringExt::endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(  ".min.css")) ||
             UStringExt::endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(".nomin.css")))
            {
            goto next1;
            }
         }

      content = UStringExt::minifyCssJs(content); // minifies CSS/JS by removing comments and whitespaces...

      if (content.empty()) goto end;
      }
#ifdef USE_PAGE_SPEED
   else if (u_is_html(mime_index))
      {
      U_INTERNAL_ASSERT(u_endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(".htm")) ||
                        u_endsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM(".html")))

      U_INTERNAL_ASSERT(pathname->isNullTerminated())

      page_speed->minify_html(pathname->data(), content);
      }
#endif

next1:
   U_INTERNAL_DUMP("UServer_Base::min_size_for_sendfile = %u", UServer_Base::min_size_for_sendfile)

        if (file_data->size >= UServer_Base::min_size_for_sendfile) motivation = " (size exceeded)";  // NB: for major size we assume is better to use sendfile()
   else if (file_data->size <= U_MIN_SIZE_FOR_DEFLATE)              motivation = " (size too small)";
#ifdef USE_LIBZ
   else if (u_is_ssi(mime_index) == false &&
            u_is_gz( mime_index) == false)
      {
      /**
       * http://zoompf.com/blog/2012/02/lose-the-wait-http-compression
       *
       * Unfortunately, the HTTP/1.1 RFC does a poor job when describing the allowable compression schemes for the Accept-Encoding
       * and Content-Encoding headers. It defines Content-Encoding: gzip to mean that the response body is composed of the GZIP
       * data format (GZIP headers, deflated data, and a checksum). It also defines Content-Encoding: deflate but, despite its name,
       * this does not mean the response body is a raw block of DEFLATE compressed data.
       *
       * According to RFC-2616, Content-Encoding: deflate means the response body is:
       * [the] "zlib" format defined in RFC 1950 in combination with the "deflate" compression mechanism described in RFC 1951.
       *
       * So, DEFLATE, and Content-Encoding: deflate, actually means the response body is composed of the zlib format (zlib header,
       * deflated data, and a checksum).
       *
       * This "deflate the identifier doesn't mean raw DEFLATE compressed data" idea was rather confusing.
       *
       * Browsers receive Content-Encoding: deflate had to handle two possible situations: the response body is raw DEFLATE data,
       * or the response body is zlib wrapped DEFLATE. So, how well do modern browser handle raw DEFLATE or zlib wrapped DEFLATE
       * responses? Verve Studios put together a test suite and tested a huge number of browsers. The results are not good. All
       * those fractional results in the table means the browser handled raw-DEFLATE or zlib-wrapped-DEFLATE inconsistently,
       * which is really another way of saying "It's broken and doesn't work reliably". This seems to be a tricky bug that
       * browser creators keep re-introducing into their products. Safari 5.0.2? No problem. Safari 5.0.3? Complete failure.
       * Safari 5.0.4? No problem. Safari 5.0.5? Inconsistent and broken.
       *
       * Sending raw DEFLATE data is just not a good idea. As Mark says "[it's] simply more reliable to only use GZIP"
       */

      gzip    = true;
      content = UStringExt::deflate(content, 2); // zopfli...
      }
#endif

next2:
   size = content.size();

   if (size < file_data->size)
      {
      ratio = (size * 100U) / file_data->size;

      U_INTERNAL_DUMP("ratio = %d (%d%%)", ratio, 100 - ratio)

      // NB: we accept new data only if ratio compression is better than 15%...

      if (ratio < 85 &&
          motivation == 0)
         {
         file_data->array->push_back(content);

         header.setBuffer(U_CAPACITY);

         if (gzip) (void) header.assign(U_CONSTANT_TO_PARAM("Content-Encoding: gzip\r\n"));
                          header.snprintf_add(U_STRING_TO_PARAM(fmt), size);

         (void) header.shrink();

         file_data->array->push_back(header);
         }
      }

end:
   U_SRV_LOG("File cached: %V - %u bytes - (%d%%) compression ratio%s", pathname->rep, file_data->size, 100 - ratio, (motivation ? motivation : ""));
}

bool UHTTP::callInitForAllUSP(UStringRep* key, void* value)
{
   U_TRACE(0, "UHTTP::callInitForAllUSP(%V,%p)", key, value)

   U_INTERNAL_ASSERT_POINTER(value)

   UHTTP::UFileCacheData* cptr = (UHTTP::UFileCacheData*)value;

   if (cptr->ptr           &&
       cptr->link == false &&
       cptr->mime_index == U_usp)
      {
      UServletPage* usp_page = (UServletPage*)cptr->ptr;

      U_INTERNAL_DUMP("usp_page->runDynamicPage = %p", usp_page->runDynamicPage)

#  ifdef DEBUG
      if (usp_page->runDynamicPage)
#  endif
      usp_page->runDynamicPage(U_DPAGE_INIT);
      }

   U_RETURN(true);
}

bool UHTTP::callEndForAllUSP(UStringRep* key, void* value)
{
   U_TRACE(0, "UHTTP::callEndForAllUSP(%V,%p)", key, value)

   U_INTERNAL_ASSERT_POINTER(value)
   U_INTERNAL_ASSERT(bcallInitForAllUSP)

   UHTTP::UFileCacheData* cptr = (UHTTP::UFileCacheData*)value;

   if (cptr->ptr           &&
       cptr->link == false &&
       cptr->mime_index == U_usp)
      {
      UServletPage* usp_page = (UServletPage*)cptr->ptr;

      U_INTERNAL_DUMP("usp_page->runDynamicPage = %p", usp_page->runDynamicPage)

#  ifdef DEBUG
      if (usp_page->runDynamicPage)
#  endif
      usp_page->runDynamicPage(U_DPAGE_DESTROY);
      }

   U_RETURN(true);
}

bool UHTTP::callSigHUPForAllUSP(UStringRep* key, void* value)
{
   U_TRACE(0, "UHTTP::callSigHUPForAllUSP(%V,%p)", key, value)

   U_INTERNAL_ASSERT_POINTER(value)
   U_INTERNAL_ASSERT(bcallInitForAllUSP)

   UHTTP::UFileCacheData* cptr = (UHTTP::UFileCacheData*)value;

   if (cptr->ptr           &&
       cptr->link == false &&
       cptr->mime_index == U_usp)
      {
      UServletPage* usp_page = (UServletPage*)cptr->ptr;

      U_INTERNAL_DUMP("usp_page->runDynamicPage = %p", usp_page->runDynamicPage)

#  ifdef DEBUG
      if (usp_page->runDynamicPage)
#  endif
      usp_page->runDynamicPage(U_DPAGE_SIGHUP);
      }

   U_RETURN(true);
}

bool UHTTP::callAfterForkForAllUSP(UStringRep* key, void* value)
{
   U_TRACE(0, "UHTTP::callAfterForkForAllUSP(%V,%p)", key, value)

   U_INTERNAL_ASSERT_POINTER(value)
   U_INTERNAL_ASSERT(bcallInitForAllUSP)

   UHTTP::UFileCacheData* cptr = (UHTTP::UFileCacheData*)value;

   if (cptr->ptr           &&
       cptr->link == false &&
       cptr->mime_index == U_usp)
      {
      UServletPage* usp_page = (UServletPage*)cptr->ptr;

      U_INTERNAL_DUMP("usp_page->runDynamicPage = %p", usp_page->runDynamicPage)

#  ifdef DEBUG
      if (usp_page->runDynamicPage)
#  endif
      usp_page->runDynamicPage(U_DPAGE_FORK);
      }

   U_RETURN(true);
}

U_NO_EXPORT bool UHTTP::checkIfUSPLink(UStringRep* key, void* value)
{
   U_TRACE(0+256, "UHTTP::checkIfUSPLink(%V,%p)", key, value)

   U_INTERNAL_ASSERT_POINTER(value)
   U_INTERNAL_ASSERT_POINTER(file_data)

   UHTTP::UFileCacheData* cptr = (UHTTP::UFileCacheData*)value;

   U_INTERNAL_DUMP("cptr->link = %b cptr->mime_index(%d) = %C file_data->mime_index(%d)  = %C",
                    cptr->link,     cptr->mime_index, cptr->mime_index, file_data->mime_index, file_data->mime_index)

   if (cptr->mime_index == U_usp)
      {
      U_INTERNAL_ASSERT_POINTER(cptr->ptr)

      UServletPage* usp_page1 = (UServletPage*)cptr->ptr;
      UServletPage* usp_page2 = (UServletPage*)file_data->ptr;

      U_INTERNAL_DUMP("usp_page1->runDynamicPage = %p", usp_page1->runDynamicPage)
      U_INTERNAL_DUMP("usp_page2->runDynamicPage = %p", usp_page2->runDynamicPage)

      U_INTERNAL_ASSERT_POINTER(usp_page1->runDynamicPage)
      U_INTERNAL_ASSERT_POINTER(usp_page2->runDynamicPage)

      if (usp_page1->runDynamicPage ==
          usp_page2->runDynamicPage)
         {
         delete usp_page2;

         file_data->ptr  = usp_page1;
         file_data->link = true;

         U_RETURN(false);
         }
      }

   U_RETURN(true);
}

U_NO_EXPORT bool UHTTP::checkIfUSP(UStringRep* key, void* value)
{
   U_TRACE(0+256, "UHTTP::checkIfUSP(%V,%p)", key, value)

   U_INTERNAL_ASSERT_POINTER(value)

   UHTTP::UFileCacheData* cptr = (UHTTP::UFileCacheData*)value;

   U_INTERNAL_DUMP("cptr->link = %b cptr->mime_index(%d) = %C", cptr->link, cptr->mime_index, cptr->mime_index)

   if (cptr->ptr           &&
       cptr->link == false &&
       cptr->mime_index == U_usp)
      {
#  ifdef DEBUG
      UServletPage* usp_page = (UServletPage*)cptr->ptr;

      U_INTERNAL_DUMP("usp_page->runDynamicPage = %p", usp_page->runDynamicPage)

      U_INTERNAL_ASSERT_POINTER(usp_page->runDynamicPage)
#  endif

      if (UStringExt::endsWith(U_STRING_TO_PARAM(*key), usp_page_key, usp_page_key_len))
         {
         usp_page_ptr = (UServletPage*)cptr->ptr;

         U_SRV_LOG("getUSP(%.*S) has found the USP service (URI): %V", usp_page_key_len, usp_page_key, key);

         U_RETURN(false);
         }
      }

   U_RETURN(true);
}

UHTTP::UServletPage* UHTTP::getUSP(const char* key, uint32_t key_len)
{
   U_TRACE(0, "UHTTP::getUSP(%.*S,%u)", key_len, key, key_len)

   U_INTERNAL_ASSERT_POINTER(key)
   U_INTERNAL_ASSERT_MAJOR(key_len,0)

   usp_page_ptr     = 0;
   usp_page_key     = key;
   usp_page_key_len = key_len;

   cache_file->callForAllEntry(checkIfUSP);

   U_RETURN_POINTER(usp_page_ptr, UHTTP::UServletPage);
}

U_NO_EXPORT bool UHTTP::compileUSP(const char* path, uint32_t len)
{
   U_TRACE(0, "UHTTP::compileUSP(%.*S,%u)", len, path, len)

   static int fd_stderr;

   UString command(200U);

   command.snprintf(U_CONSTANT_TO_PARAM("usp_compile.sh %.*s %s"), len, path, U_LIB_SUFFIX);

   UCommand cmd(command);

   U_ASSERT(cmd.checkForExecute())

   if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/usp_compile.err");

   bool ok = cmd.executeAndWait(0, -1, fd_stderr);

#ifndef U_LOG_DISABLE
   if (UServer_Base::isLog())
      {
      UServer_Base::logCommandMsgError(cmd.getCommand(), false);

      if (ok == false) ULog::log(U_CONSTANT_TO_PARAM("%sWARNING: USP compile failed: %.*S"), UServer_Base::mod_name[0], path, len);
      }
#endif

   U_RETURN(ok);
}

void UHTTP::checkFileForCache()
{
   U_TRACE_NO_PARAM(0, "UHTTP::checkFileForCache()")

   U_INTERNAL_ASSERT_POINTER(pathname)

   file->setPath(*pathname);

   UString file_name = UStringExt::basename(file->getPath());

#ifdef DEBUG
   if (file->isSuffixSwap() || // NB: vi tmp...
       UServices::dosMatchWithOR(file_name, U_CONSTANT_TO_PARAM("stack.userver_*|mempool.userver_*|trace.userver_*|*usp_translator*"), 0))
      {
      return;
      }
#endif

   if (file->stat()) // NB: file->stat() get also the size of the file...
      {
      U_INTERNAL_DUMP("nocache_file_mask = %p U_http_is_nocache_file = %b", nocache_file_mask, U_http_is_nocache_file)

      if (U_http_is_nocache_file ||
          (nocache_file_mask     &&
           UServices::dosMatchWithOR(file_name, U_STRING_TO_PARAM(*nocache_file_mask), 0)))
         {
         return;
         }

      manageDataForCache(file_name);
      }
}

U_NO_EXPORT void UHTTP::manageDataForCache(const UString& file_name)
{
   U_TRACE(1, "UHTTP::manageDataForCache(%V)", file_name.rep)

   uint32_t suffix_len;
   const char* suffix_ptr;

#ifdef DEBUG
   U_INTERNAL_ASSERT_POINTER(file)
   U_INTERNAL_ASSERT_POINTER(pathname)
   U_INTERNAL_ASSERT_POINTER(cache_file)

   U_INTERNAL_DUMP("pathname = %V file = %.*S rpathname = %V", pathname->rep, U_FILE_TO_TRACE(*file), rpathname->rep)

   if (pathname->equal(U_FILE_TO_PARAM(*file)) == false) // NB: can happen with inotify...
      {
      U_DEBUG("UHTTP::manageDataForCache(%V) pathname(%u) = %.*S file(%u) = %.*S", file_name.rep, pathname->size(), pathname->rep, file->getPathRelativLen(), U_FILE_TO_TRACE(*file))
      }
#endif

   U_NEW(UHTTP::UFileCacheData, file_data, UHTTP::UFileCacheData);

   // NB: copy the attributes from file...

   file_data->size  = file->st_size;
   file_data->mode  = file->st_mode;
   file_data->mtime = file->st_mtime;

   U_INTERNAL_DUMP("file_data->size = %u", file_data->size)

   if (file->dir())
      {
      U_INTERNAL_ASSERT(S_ISDIR(file_data->mode))

      goto end;
      }

#ifdef U_LINUX
   if (rpathname->empty() && // NB: check if we are called from here...
       (file->lstat(), file->slink()))
      {
      *rpathname = UFile::getRealPath(pathname->data(), true);

      U_ASSERT_DIFFERS(*rpathname, *pathname)

      if (rpathname->first_char() == '/')
         {
         // NB: we have something that point outside of document root, it cannot be a link to a cache entry...

         file->st_size  = file_data->size;
         file->st_mode  = file_data->mode;
         file->st_mtime = file_data->mtime;
         }
      else
         {
         UHTTP::UFileCacheData* file_data_link = cache_file->at(U_STRING_TO_PARAM(*rpathname));

         if (file_data_link) file_data->mime_index = file_data_link->mime_index;
         else
            {
            UHTTP::UFileCacheData* file_data_save;

            // NB: we call ourselves...

            UString save = *pathname;
                           *pathname = *rpathname;

            file_data_save = file_data;
                             file_data = 0;

            checkFileForCache();

            if (file_data == 0)
               {
               file_data = file_data_save;

               goto error;
               }

            *pathname = save;

            file_data_link = (u__isdigit(file_data->mime_index) && // NB: check for dynamic page...
                              (file_data->mime_index == U_usp   || // '0' => USP (ULib Servlet Page)
#                            ifdef HAVE_LIBTCC
                               file_data->mime_index == U_csp   || // '1' => CSP (C    Servlet Page)
#                            endif
                               file_data->mime_index == U_cgi)     // '2' => cgi-bin
                                 ? (UHTTP::UFileCacheData*)file_data->ptr
                                 :                         file_data);

            file_data_save->mime_index = file_data->mime_index;

            file_data = file_data_save;
            }

         U_INTERNAL_ASSERT_POINTER(file_data_link)

         file_data->ptr  = file_data_link;
         file_data->link = true;

         U_SRV_LOG("Symbolic link found: %V => %V", pathname->rep, rpathname->rep);

         (void) rpathname->clear();

         goto end;
         }
      }
#endif

   suffix_len = (*suffix = file->getSuffix()).size();
   suffix_ptr = (suffix_len ? suffix->data() : 0);

   // manage authorization data...

   if (suffix_len == 8                                                                            &&
       (u_get_unalignedp64(suffix_ptr) == U_MULTICHAR_CONSTANT64('h','t','p','a','s','s','w','d') ||
        u_get_unalignedp64(suffix_ptr) == U_MULTICHAR_CONSTANT64('h','t','d','i','g','e','s','t')))
      {
      U_NEW(UVector<UString>, file_data->array, UVector<UString>(1U));

      file_data->array->push_back(file->getContent(true, false, true));

      U_SRV_LOG("File cached: %V - %u bytes", pathname->rep, file_data->size);

      goto end;
      }

   if (cache_file_mask &&
       UServices::dosMatchWithOR(file_name, U_STRING_TO_PARAM(*cache_file_mask), 0))
      {
      if (file_data->size == 0)
         {
         U_SRV_LOG("WARNING: found empty file: %V", pathname->rep);
         }
      else if (file->open())
         {
         UString content = file->getContent(true, false, true);

         /**
          * NB: if we want to maintain the file open...
          *
          * file_data->fd = file->fd; (void) file->memmap(PROT_READ, &content);
          */

         if (content) putDataInCache(getHeaderMimeType(content.data(), 0, setMimeIndex(suffix_ptr), U_TIME_FOR_EXPIRE), content);
         }

      goto end;
      }

   // NB: when a pathfile ends with "*.[so|usp|c|dll]" it is assumed to be a dynamic page...

   if (suffix_len                                                                       &&
       (UServices::dosMatch(      file_name, U_CONSTANT_TO_PARAM("*.0.so"), 0) == false && // MACOSX
        UServices::dosMatchWithOR(file_name, U_CONSTANT_TO_PARAM("*.so|*.usp|*.c|*.dll"), 0)))
      {
      uint32_t len;
      const char* ptr;
      char buffer[U_PATH_MAX];
      char run_dynamic_page[128];

      bool usp_dll = false,
           usp_src = false;

      if (suffix_len == 3                                                   &&
          u_get_unalignedp16(suffix_ptr) == U_MULTICHAR_CONSTANT16('u','s') &&
          suffix_ptr[2] == 'p')
         {
         usp_src = true;
         }

      if ( usp_src ||
          (usp_dll = suffix->equal(U_CONSTANT_TO_PARAM(U_LIB_SUFFIX))))
         {
         bool exist;
         struct stat st;
#     ifndef U_LOG_DISABLE
         const char* link;
#     endif
         UServletPage* usp_page;

         ptr = file->getPathRelativ();
         len = file->getPathRelativLen() - suffix_len;

         U_INTERNAL_DUMP("ptr(%u) = %.*S", len, len, ptr)

         U_INTERNAL_ASSERT_MAJOR(len, 0)
         U_INTERNAL_ASSERT(u__strlen(U_LIB_SUFFIX, __PRETTY_FUNCTION__) >= 2)

         (void) u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%.*s%s"), len, ptr, usp_dll ? "usp" : U_LIB_SUFFIX);

         exist = (U_SYSCALL(stat, "%S,%p", buffer, &st) == 0);

         if ((exist && getUSP(ptr, len-1))                            || // NB: check to avoid duplication...
             (((usp_dll && ( exist && st.st_mtime > file->st_mtime))  ||
               (usp_src && (!exist || st.st_mtime < file->st_mtime))) &&
              compileUSP(ptr, len-1) == false))
            {
            goto check;
            }

         // NB: dlopen() fail if the name of the module is not prefixed with "./"...

         (void) u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("./%.*s%s"), len, ptr, U_LIB_SUFFIX);

         U_NEW(UHTTP::UServletPage, usp_page, UHTTP::UServletPage);

         if (usp_page->UDynamic::load(buffer) == false)
            {
fail:       U_SRV_LOG("WARNING: USP load failed: %S", buffer);

            delete usp_page;

check:      if (usp_src) goto end;

            goto error;
            }

         (void) u__snprintf(run_dynamic_page, sizeof(run_dynamic_page), U_CONSTANT_TO_PARAM("runDynamicPage_%.*s"), file_name.size() - suffix_len - 1, file_name.data());

         usp_page->runDynamicPage = (vPFi)(*usp_page)[run_dynamic_page];

         if (usp_page->runDynamicPage == 0)
            {
            usp_page->UDynamic::close();

            goto fail;
            }

#     if defined(DEBUG) && !defined(U_STATIC_ONLY)
         if (usp_src)
            {
            (void) pathname->shrink();

            cache_file->insert(*pathname, file_data);

            U_NEW(UHTTP::UFileCacheData, file_data, UHTTP::UFileCacheData);

            file_data->mtime = u_now->tv_sec;
            }
#     endif

         file_data->ptr        = usp_page;
         file_data->mime_index = U_usp;

#     ifdef U_LOG_DISABLE
                 cache_file->callForAllEntry(checkIfUSPLink);
#     else
         link = (cache_file->callForAllEntry(checkIfUSPLink), file_data->link) ? " (link)" : "";
#     endif

         (void) pathname->replace(buffer+2, len-1);

         U_SRV_LOG("USP found: %S%s, USP service registered (URI): %V", buffer, link, pathname->rep);

         if (bcallInitForAllUSP)
            {
            callInitForAllUSP(     pathname->rep, file_data);
            callAfterForkForAllUSP(pathname->rep, file_data);
            }
         }
#  ifdef HAVE_LIBTCC
      else if (suffix_len == 1 &&
               suffix_ptr[0] == 'c')
         {
         UString program = file->getContent();

         UCServletPage* csp_page;

         U_NEW(UHTTP::UCServletPage, csp_page, UHTTP::UCServletPage);

         if (program.empty()            == false &&
             csp_page->compile(program) == false)
            {
            U_SRV_LOG("WARNING: CSP load failed: %.*S", U_FILE_TO_TRACE(*file));

            delete csp_page;

            goto error;
            }

         U_INTERNAL_ASSERT_POINTER(csp_page->prog_main)

         file_data->ptr        = csp_page;
         file_data->mime_index = U_csp;

         len = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%.*s"), U_FILE_TO_TRACE(*file));

         (void) pathname->replace(buffer, len - U_CONSTANT_SIZE(".c"));

         U_SRV_LOG("CSP found: %S, CSP service registered (URI): %V", buffer, pathname->rep);
         }
#  endif

      goto end;
      }

   if (u_dosmatch(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM("*cgi-bin/*"), 0))
      {
      // NB: when a pathfile ends by "cgi-bin/*.[sh|php|pl|py|rb|*]" it is assumed to be a cgi script...

      uint32_t pos = U_STRING_FIND(*pathname, 0, "cgi-bin/");

      U_INTERNAL_ASSERT_DIFFERS(pos, U_NOT_FOUND)

      pos += U_CONSTANT_SIZE("cgi-bin");

      if (pathname->c_char(pos+2) != '.') // NB: the directory "cgi-bin" often contains some "functions file" (that starts with '.')...
         {
         UHTTP::ucgi* cgi = U_MALLOC_TYPE(UHTTP::ucgi);

         cgi->interpreter      = 0;
         cgi->environment_type = U_CGI;

         pathname->copy(cgi->dir);
                        cgi->dir[pos] = '\0';

         U_INTERNAL_DUMP("cgi_dir = %S cgi_doc = %S", cgi->dir, cgi->dir + u__strlen(cgi->dir, __PRETTY_FUNCTION__) + 1)

         if (suffix_len == 2)
            {
            if (u_get_unalignedp16(suffix_ptr) == U_MULTICHAR_CONSTANT16('s','h'))
               {
               cgi->interpreter      = U_PATH_SHELL;
               cgi->environment_type = U_SHELL;
               }
            else if (u_get_unalignedp16(suffix_ptr) == U_MULTICHAR_CONSTANT16('r','b')) cgi->interpreter = "ruby";
            else if (u_get_unalignedp16(suffix_ptr) == U_MULTICHAR_CONSTANT16('p','l')) cgi->interpreter = "perl";
            else if (u_get_unalignedp16(suffix_ptr) == U_MULTICHAR_CONSTANT16('p','y'))
               {
               cgi->interpreter      = "python";
               cgi->environment_type = U_WSCGI;
               }
            }
         else if (suffix_len == 3                                                   &&
                  u_get_unalignedp16(suffix_ptr) == U_MULTICHAR_CONSTANT16('p','h') &&
                  suffix_ptr[2] == 'p')
            {
            cgi->interpreter      = "php-cgi";
            cgi->environment_type = U_PHP;
            }
         else if (suffix_len == 4 &&
                  u_get_unalignedp32(suffix_ptr) == U_MULTICHAR_CONSTANT32('b','a','s','h'))
            {
            cgi->environment_type = U_SHELL;
            }

         U_INTERNAL_DUMP("cgi->environment_type = %d cgi->interpreter = %S", cgi->environment_type, cgi->interpreter)

         file_data->ptr        = cgi;
         file_data->mime_index = U_cgi;

         U_SRV_LOG("cgi-bin found: %.*S, interpreter registered: %S", U_FILE_TO_TRACE(*file), cgi->interpreter);

         goto end;
         }

      goto error;
      }

   if (suffix_len)
      {
      const char* ctype = u_get_mimetype(suffix_ptr, &file_data->mime_index);

      U_INTERNAL_DUMP("u_is_cacheable(%d) = %b ctype = %S", file_data->mime_index, u_is_cacheable(file_data->mime_index), ctype)

      if (u_is_cacheable(file_data->mime_index))
         {
         UString content = file->getContent(true, false, true);

         if (content.empty())
            {
            U_SRV_LOG("WARNING: found empty file: %V", pathname->rep);
            }
         else
            {
            mime_index = file_data->mime_index;

            putDataInCache(getHeaderMimeType(content.data(), 0, ctype, U_TIME_FOR_EXPIRE), content);
            }
         }
      }

end:
   U_INTERNAL_DUMP("file_data->mime_index(%d) = %C", file_data->mime_index, file_data->mime_index)

   (void) pathname->shrink();

   cache_file->insert(*pathname, file_data); // NB: we don't need to call u_construct<UHTTP::UFileCacheData>()...

   return;

error:
   delete file_data;
          file_data = 0;
}

void UHTTP::renewFileDataInCache()
{
   U_TRACE(0, "UHTTP::renewFileDataInCache()")

   U_ASSERT_EQUALS(file_data, cache_file->elem())

   // NB: we need to do this before call eraseAfterFind()...

   int fd                = file_data->fd;
   const UStringRep* key = cache_file->key();

   if (fd != -1) UFile::close(fd);

   U_INTERNAL_DUMP("file_data->fd = %d cache_file->key = %V", file_data->fd, key)

   pathname->setBuffer(key->size());

   pathname->snprintf(U_CONSTANT_TO_PARAM("%v"), key);

   U_DEBUG("renewFileDataInCache() called for file: %V - inotify %s enabled, expired=%b", pathname->rep,
               UServer_Base::handler_inotify ? "is" : "NOT", (u_now->tv_sec > file_data->expire))

   cache_file->eraseAfterFind();

   checkFileForCache();

   if (fd != -1     &&
       file->st_ino && // stat() ok...
       file->open())
      {
      file_data->fd = file->fd;
      }

   U_INTERNAL_DUMP("file_data->array = %p", file_data->array)
}

UString UHTTP::getDataFromCache(int idx)
{
   U_TRACE(0, "UHTTP::getDataFromCache(%d)", idx)

   U_INTERNAL_ASSERT_POINTER(file_data)

   U_INTERNAL_DUMP("u_now->tv_sec     = %#3D", u_now->tv_sec)
   U_INTERNAL_DUMP("file_data->expire = %#3D", file_data->expire)

   UString result;

   if (u_now->tv_sec > file_data->expire) renewFileDataInCache();

   if (file_data->array)
      {
      U_INTERNAL_ASSERT_MINOR(idx, 4)

      result = file_data->array->operator[](idx);
      }

   U_RETURN_STRING(result);
}

U_NO_EXPORT bool UHTTP::processFileCache()
{
   U_TRACE_NO_PARAM(0, "UHTTP::processFileCache()")

   U_INTERNAL_ASSERT_POINTER(file_data)
   U_ASSERT(UClientImage_Base::isRequestInFileCache())

   if (checkGetRequestIfModified() == false)
      {
      UClientImage_Base::setRequestFileCacheProcessed();

      U_RETURN(true);
      }

#ifdef USE_LIBZ
   U_INTERNAL_DUMP("U_http_is_accept_gzip = %b", U_http_is_accept_gzip)

   if (U_http_is_accept_gzip &&
       isDataCompressFromCache())
      {
      U_http_flag |= HTTP_IS_RESPONSE_GZIP;

      U_INTERNAL_DUMP("U_http_is_response_gzip = %b", U_http_is_response_gzip)

      *ext = getHeaderCompressFromCache();

      *UClientImage_Base::body = getBodyCompressFromCache();

      UClientImage_Base::setRequestFileCacheProcessed();

      U_RETURN(true);
      }
#endif

   uint32_t sz;
   *ext = getHeaderFromCache();

   // --------------------------------------------------------------------------------------------------------------
   // The Range: header is used with a GET/HEAD request.
   // For example assume that will return only a portion (let's say the first 32 bytes) of the requested resource...
   // --------------------------------------------------------------------------------------------------------------
   // Range: bytes=0-31
   // --------------------------------------------------------------------------------------------------------------

   range_size  = sz = file_data->size;
   range_start = 0;

   if (U_http_range_len &&
       checkGetRequestIfRange())
      {
      if (checkGetRequestForRange(getBodyFromCache()) != U_PARTIAL)
         {
         UClientImage_Base::setRequestFileCacheProcessed();

         U_RETURN(true);
         }

      // NB: range_start is modified only if we have as response U_PARTIAL from checkGetRequestForRange()...

      sz -= range_start;

      U_http_info.nResponseCode = HTTP_PARTIAL;
      }

   U_INTERNAL_DUMP("sz = %u UServer_Base::min_size_for_sendfile = %u", sz, UServer_Base::min_size_for_sendfile)

   if (sz < UServer_Base::min_size_for_sendfile)
      {
      UClientImage_Base::setRequestFileCacheProcessed();

      *UClientImage_Base::body = getBodyFromCache().substr(range_start, range_size);

      U_RETURN(true);
      }

   U_http_flag |= HTTP_IS_SENDFILE;

   U_INTERNAL_DUMP("U_http_sendfile = %b", U_http_sendfile)

   U_RETURN(false);
}

U_NO_EXPORT bool UHTTP::checkPath(uint32_t len)
{
   U_TRACE(0, "UHTTP::checkPath(%u)", len)

   pathname->size_adjust_force(u_cwd_len + len); // NB: pathname can be referenced by file obj...

   checkPath();

   if (UClientImage_Base::isRequestNotFound()) U_RETURN(false);

   U_RETURN(true);
}

void UHTTP::setPathName()
{
   U_TRACE_NO_PARAM(0, "UHTTP::setPathName()")

   U_INTERNAL_ASSERT(pathname->empty())
   U_ASSERT(pathname->capacity() >= u_cwd_len + U_http_info.uri_len)

   char* ptr = pathname->data();

   U_MEMCPY(ptr,                     u_cwd,           u_cwd_len);
   U_MEMCPY(ptr+u_cwd_len, U_http_info.uri, U_http_info.uri_len);

   pathname->size_adjust_force(u_cwd_len + U_http_info.uri_len); // NB: pathname can be referenced by file obj...
}

U_NO_EXPORT void UHTTP::checkPath()
{
   U_TRACE_NO_PARAM(0, "UHTTP::checkPath()")

   U_INTERNAL_DUMP("     pathname(%3u) = %V", pathname->size(), pathname->rep)
   U_INTERNAL_DUMP("document_root(%3u) = %V", UServer_Base::document_root_size, UServer_Base::document_root->rep)

   U_INTERNAL_ASSERT(pathname->isNullTerminated())
   U_INTERNAL_ASSERT_POINTER(UServer_Base::document_root_ptr)

   uint32_t len    = pathname->size();
   const char* ptr = pathname->data();

   if (u_canonicalize_pathname((char*)ptr))
      {
      len = u__strlen(ptr, __PRETTY_FUNCTION__);

      pathname->size_adjust_force(len); // NB: pathname can be referenced by file obj...
      }

   if (len < UServer_Base::document_root_size         ||
         ptr[UServer_Base::document_root_size] != '/' ||
       memcmp(ptr, UServer_Base::document_root_ptr, UServer_Base::document_root_size) != 0)
      {
#  ifndef U_SERVER_CAPTIVE_PORTAL
      setForbidden();
#  endif

      return;
      }

#ifdef USE_RUBY
   if (ruby_on_rails == false)
#endif
   {
   if (len == u_cwd_len)
      {
      // NB: we have the special case: '/' alias '.'...

      U_INTERNAL_ASSERT_EQUALS(U_http_info.uri[0], '/')

      file->setRoot();

      UClientImage_Base::setRequestNeedProcessing();

      return;
      }
   }

   file->setPath(*pathname);

   U_INTERNAL_DUMP("file = %.*S", U_FILE_TO_TRACE(*file))

   checkFileInCache1(U_FILE_TO_PARAM(*file));

   if (file_data == 0)
      {
      U_INTERNAL_DUMP("U_http_is_request_nostat = %b", U_http_is_request_nostat)

      if (U_http_is_request_nostat)
         {
         U_ClientImage_request = 0; // set to UClientImage_Base::NOT_FOUND...

         return;
         }

      // we don't wont to process this kind of request (usually aliased)...

      ptr = file->getPathRelativ();
      len = file->getPathRelativLen();

      if (len >= U_PATH_MAX               ||
          u_isFileName(ptr, len) == false ||
          UStringExt::endsWith(ptr, len, U_CONSTANT_TO_PARAM("nocontent")))
         {
nocontent:
         UClientImage_Base::setCloseConnection();

         U_http_info.nResponseCode = HTTP_NO_CONTENT;

         setResponse();

         return;
         }

      if (U_STREQ(ptr, len, "checkZombies")) // check for zombies...
         {
         UServer_Base::removeZombies();

         goto nocontent;
         }

#  ifdef U_SERVER_CAPTIVE_PORTAL
      return;
#  endif

#  ifdef USE_RUBY
      if (ruby_on_rails)
         {
         file_data  = file_not_in_cache_data;
         mime_index = file_data->mime_index = U_ruby;

         UClientImage_Base::setRequestInFileCache();

         return;
         }
#  endif
#  ifdef USE_PHP
      if (npathinfo)
         {
         U_INTERNAL_DUMP("SCRIPT_NAME=%.*S PATH_INFO=%.*S",
                                               npathinfo, U_http_info.uri,
                         U_http_info.uri_len - npathinfo, U_http_info.uri + npathinfo)

         file_data  = file_not_in_cache_data;
         mime_index = file_data->mime_index = U_php;

         UClientImage_Base::setRequestInFileCache();

         return;
         }
#  endif

#  if defined(HAVE_SYS_INOTIFY_H) && defined(U_HTTP_INOTIFY_SUPPORT) && !defined(U_SERVER_CAPTIVE_PORTAL)
      bool bstat = false;

      if (db_not_found)
         {
         U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

         U_INTERNAL_DUMP("U_http_is_nocache_file = %b", U_http_is_nocache_file)

         if (U_http_is_nocache_file == false)
            {
            db_not_found->lock();

            db_not_found->UCDB::setKey(ptr, len);

#        ifndef USE_HARDWARE_CRC32
            db_not_found->UCDB::setHash(ptr, len);
#        else
            db_not_found->UCDB::setHash(cache_file->hash);
#        endif

            if (db_not_found->_fetch())
               {
               db_not_found->unlock();

               return;
               }

            if (file->stat() == false)
               {
               db_not_found->UCDB::setData(U_CLIENT_ADDRESS_TO_PARAM);

               int ko = db_not_found->_store(RDB_INSERT, false);

               db_not_found->unlock();

               if (ko) U_WARNING("Insert data on db %.*S failed with error %d", U_FILE_TO_TRACE(*db_not_found), ko);

               return;
               }

            db_not_found->unlock();

            bstat = true;
            }
         }

      if (bstat ||
          file->stat())
#  else
      if (file->stat())
#  endif
         {
         *suffix = file->getSuffix();

         if (*suffix)
            {
            if (suffix->equal(U_CONSTANT_TO_PARAM(U_LIB_SUFFIX))) goto nocontent;

            if (U_HTTP_QUERY_STREQ("_nav_") &&
                suffix->equal(U_CONSTANT_TO_PARAM("usp")))
               {
               UClientImage_Base::setRequestNeedProcessing();
               }
            }

         (void) pathname->replace(U_FILE_TO_PARAM(*file));

         U_INTERNAL_DUMP("U_http_is_nocache_file = %b", U_http_is_nocache_file)

         if (U_http_is_nocache_file == false)
            {
            U_DEBUG("Found file not in cache: %V - inotify %s enabled", pathname->rep, UServer_Base::handler_inotify ? "is" : "NOT")

            manageDataForCache(UStringExt::basename(file->getPath()));

            U_INTERNAL_ASSERT_POINTER(file_data)
            }
         }
      }

   if (file_data) UClientImage_Base::setRequestInFileCache();
}

// REWRITE RULE

#if defined(U_ALIAS) && defined(USE_LIBPCRE)
UVector<UHTTP::RewriteRule*>* UHTTP::vRewriteRule;

U_NO_EXPORT void UHTTP::processRewriteRule()
{
   U_TRACE_NO_PARAM(0, "UHTTP::processRewriteRule()")

   uint32_t pos, len;
   UHTTP::RewriteRule* rule;
   UString _uri(U_HTTP_URI_TO_PARAM), new_uri;

   for (uint32_t i = 0, n = vRewriteRule->size(); i < n; ++i)
      {
      rule    = (*vRewriteRule)[i];
      new_uri = rule->key.replace(_uri, rule->replacement);

      if (rule->key.matched())
         {
         pos = new_uri.find('?');
         len = (pos == U_NOT_FOUND ? new_uri.size() : pos);

         pathname->setBuffer(u_cwd_len + len);

         pathname->snprintf(U_CONSTANT_TO_PARAM("%w%.*s"), len, new_uri.data());

         U_SRV_LOG("REWRITE_RULE_NF: URI request changed to: %V", new_uri.rep);

         checkPath();

         if (UClientImage_Base::isRequestNeedProcessing())
            {
            UClientImage_Base::request_uri->clear();

            (void) alias->replace(new_uri);

            U_http_info.uri     = alias->data();
            U_http_info.uri_len = len;

            U_INTERNAL_DUMP("uri = %.*S", U_HTTP_URI_TO_TRACE)

            if (pos != U_NOT_FOUND)
               {
               const char* ptr = alias->c_pointer(len+1);

               U_http_info.query     = ptr;
               U_http_info.query_len = alias->remain(ptr);

               U_INTERNAL_DUMP("query = %.*S", U_HTTP_QUERY_TO_TRACE)
               }
            }

         break;
         }
      }
}
#endif
 
/**
 * Set up CGI environment variables. The following table describes common CGI environment variables that the server
 * creates (some of these are not available with some servers):
 * 
 * CGI server variable Description
 * -------------------------------------------------------------------------------------------------------------------------------------------
 * REQUEST_METHOD    - The HTTP request method, such as 'GET' or 'POST'.
 *                     This cannot ever be an empty string, and so is always required.
 * SCRIPT_NAME       - The initial portion of the request URL 'path' that corresponds to the application object, so that
 *                     the application knows its virtual 'location'. This may be an empty string, if the application
 *                     corresponds to the 'root' of the server. If non-empty, must start with /.
 * PATH_INFO         - The remainder of the request URL 'path', designating the virtual 'location' of the request
 *                     target within the application. This may be an empty string, if the request URL targets the
 *                     application root and does not have a trailing slash. This value may be percent-encoded when I
 *                     originating from a URL. Scripts can be accessed by their virtual pathname, followed by this
 *                     extra information at the end of this path. This extra information is sent as PATH_INFO. If
 *                     non-empty, must start with /.
 * QUERY_STRING      - The portion of the request URL that follows the ?, if any.
 *                     May be empty, but is always required!
 * SERVER_NAME       - Server's hostname, DNS alias, or IP address as it appears in self-referencing URLs.
 *                     When SERVER_NAME and SERVER_PORT are combined with SCRIPT_NAME and PATH_INFO, these variables can be
 *                     used to complete the URL. Note, however, that HTTP_HOST, if present, should be used in preference to
 *                     SERVER_NAME for reconstructing the request URL. SERVER_NAME and SERVER_PORT can never be empty strings,
 *                     and so are always required.
 * SERVER_PORT       - Port number to which the request was sent.
 * 
 * HTTP_Variables    - Variables corresponding to the client-supplied HTTP request headers (i.e., variables whose names begin with HTTP_).
 *                     The presence or absence of these variables should correspond with the presence or absence of the appropriate HTTP
 *                     header in the request. The environment must not contain the keys HTTP_CONTENT_TYPE or HTTP_CONTENT_LENGTH
 *                     (use the versions without HTTP_).
 * 
 * REMOTE_HOST       - Hostname making the request. If the server does not have this information, it sets REMOTE_ADDR and does not set REMOTE_HOST.
 * REMOTE_ADDR       - IP address of the remote host making the request.
 * REMOTE_IDENT      - If the HTTP server supports RFC 931 identification, this variable is set to the remote username
 *                     retrieved from the server. Use this variable for logging only.
 * AUTH_TYPE         - If the server supports user authentication, and the script is protected, the protocol-specific
 *                     authentication method used to validate the user.
 * REMOTE_USER       - If the server supports user authentication, and the script is protected, the username the user has
 *                     authenticated as. (Also available as AUTH_USER)
 * CONTENT_TYPE      - For queries that have attached information, such as HTTP POST and PUT, this is the content type of the data.
 * CONTENT_LENGTH    - Length of the content as given by the client. If given, must consist of digits only.
 * PATH_TRANSLATED   - Translated version of PATH_INFO after any virtual-to-physical mapping.
 * SERVER_PROTOCOL   - Name and revision of the information protocol this request came in with. Format: protocol/revision.
 * SERVER_SOFTWARE   - Name and version of the information server software answering the request (and running the gateway). Format: name/version.
 * GATEWAY_INTERFACE - CGI specification revision with which this server complies. Format: CGI/revision.
 * 
 * CERT_ISSUER       - Issuer field of the client certificate (O=MS, OU=IAS, CN=user name, C=USA).
 * CERT_SUBJECT      - Subject field of the client certificate.
 * CERT_SERIALNUMBER - Serial number field of the client certificate.
 * -------------------------------------------------------------------------------------------------------------------------------------------
 * 
 * The following table describes common CGI environment variables the browser creates and passes in the request header:
 * 
 * CGI client variable      Description
 * -------------------------------------------------------------------------------------------------------------------------------------------
 * HTTP_REFERER           - The referring document that linked to or submitted form data.
 * HTTP_USER_AGENT        - The browser that the client is currently using to send the request. Format: software/version library/version.
 * HTTP_IF_MODIFIED_SINCE - The last time the page was modified. The browser determines whether to set this variable, usually in response
 *                          to the server having sent the LAST_MODIFIED HTTP header. It can be used to take advantage of browser-side caching.
 * -------------------------------------------------------------------------------------------------------------------------------------------
 * 
 * Example:
 * -------------------------------------------------------------------------------------------------------------------------------------------
 * PATH="/lib64/rc/sbin:/lib64/rc/bin:/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin"
 * UNIQUE_ID="b032zQoeAYMAAA-@BZwAAAAA"
 * REMOTE_ADDR=127.0.0.1
 * SCRIPT_NAME=/cgi-bin/printenv
 * SERVER_NAME=localhost
 * SERVER_PORT=80
 * REQUEST_URI="/cgi-bin/printenv"
 * REMOTE_PORT="35653"
 * SERVER_ADDR="127.0.0.1"
 * SERVER_ADMIN="root@localhost"
 * QUERY_STRING=
 * DOCUMENT_ROOT="/var/www/localhost/htdocs"
 * REQUEST_METHOD=GET
 * SERVER_SOFTWARE=Apache
 * SERVER_PROTOCOL=HTTP/1.1
 * SCRIPT_FILENAME="/var/www/localhost/cgi-bin/printenv"
 * SERVER_SIGNATURE="<address>Apache Server at localhost Port 80</address>\n"
 * GATEWAY_INTERFACE=CGI/1.1
 * 
 * HTTP_HOST="localhost"
 * HTTP_COOKIE="_saml_idp=dXJuOm1hY2U6dGVzdC5zXRo; _redirect_user_idp=urn%3Amace%3Atest.shib%3Afederation%3Alocalhost;"
 * HTTP_ACCEPT="text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png"
 * HTTP_KEEP_ALIVE="300"
 * HTTP_USER_AGENT="Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.8.1.14) Gecko/20080421 Firefox/2.0.0.14"
 * HTTP_CONNECTION="keep-alive"
 * HTTP_ACCEPT_CHARSET="ISO-8859-1,utf-8;q=0.7,*;q=0.7"
 * HTTP_ACCEPT_ENCODING="gzip, deflate"
 * HTTP_ACCEPT_LANGUAGE="en-us,en;q=0.5"
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */

U_NO_EXPORT bool UHTTP::addHTTPVariables(UStringRep* key, void* value)
{
   U_TRACE(0+256, "UHTTP::addHTTPVariables(%V,%V)", key, value)

   U_INTERNAL_ASSERT_POINTER(value)
   U_INTERNAL_ASSERT_POINTER(string_HTTP_Variables)

   uint32_t      key_sz  =                  key->size(),
               value_sz  = ((UStringRep*)value)->size();
   const char*   key_ptr = ((UStringRep*)  key)->data();
   const char* value_ptr = ((UStringRep*)value)->data();

// if (u_isBinary((const unsigned char*)value_ptr, value_sz) == false)
      {
#  ifndef U_HTTP2_DISABLE
      /**
       * +-------+-----------------------------+---------------+
       * | 1     | :authority                  |               |
       * | 2     | :method                     | GET           |
       * | 3     | :method                     | POST          |
       * | 4     | :path                       | /             |
       * | 5     | :path                       | /index.html   |
       * | 6     | :scheme                     | http          |
       * | 7     | :scheme                     | https         |
       * | 8     | :status                     | 200           |
       * | 9     | :status                     | 204           |
       * | 10    | :status                     | 206           |
       * | 11    | :status                     | 304           |
       * | 12    | :status                     | 400           |
       * | 13    | :status                     | 404           |
       * | 14    | :status                     | 500           |
       * | ...   | ...                         | ...           |
       * +-------+-----------------------------+---------------+
       */

      if (U_http_version == '2'                       &&
          (*key_ptr == ':'                            ||
           U_STREQ(key_ptr, key_sz, "cookie")         ||
           U_STREQ(key_ptr, key_sz, "accept")         ||
           U_STREQ(key_ptr, key_sz, "referer")        ||
           U_STREQ(key_ptr, key_sz, "user-agent")     ||
           U_STREQ(key_ptr, key_sz, "content-type")   ||
           U_STREQ(key_ptr, key_sz, "content-length") ||
           U_STREQ(key_ptr, key_sz, "accept-language")))
         {
         U_RETURN(true);
         }
#  endif

      UString buffer(20U + key_sz + value_sz),
              str = UStringExt::substitute(UStringExt::toupper(key_ptr, key_sz), '-', '_');

      buffer.snprintf(U_CONSTANT_TO_PARAM("'HTTP_%.*s=%.*s'\n"), key_sz, str.data(), value_sz, value_ptr);

      (void) string_HTTP_Variables->append(buffer);
      }

   U_RETURN(true);
}

bool UHTTP::setEnvironmentForLanguageProcessing(int type, void* env, vPFpvpcpc func)
{
   U_TRACE(0, "UHTTP::setEnvironmentForLanguageProcessing(%d,%p,%p)", type, env, func)

   if (getCGIEnvironment(*UClientImage_Base::environment, type) == false) U_RETURN(false);

   char** envp  = 0;
   int32_t nenv = UCommand::setEnvironment(*UClientImage_Base::environment, envp);

   for (int i = 0; envp[i]; ++i)
      {
      U_INTERNAL_DUMP("envp[%d] = %S", i, envp[i])

      char* ptr = strchr(envp[i], '=');

      if (ptr)
         {
         *ptr++ = '\0';

         func(env, envp[i], ptr);
         }
      }

   UCommand::freeEnvironment(envp, nenv);

   U_RETURN(true);
}

bool UHTTP::getCGIEnvironment(UString& environment, int type)
{
   U_TRACE(0, "UHTTP::getCGIEnvironment(%V,%d)", environment.rep, type)

   UString buffer(2000U + u_cwd_len + U_http_info.endHeader + U_http_info.query_len);

   // The portion of the request URL that follows the ?, if any. May be empty, but is always required!

   if (U_http_info.query_len == 0 ||
       u_isBinary((const unsigned char*)U_HTTP_QUERY_TO_PARAM) == false)
      {
      buffer.snprintf_add(U_CONSTANT_TO_PARAM("QUERY_STRING=%.*s\n"), U_HTTP_QUERY_TO_TRACE);
      }

   if (U_http_content_type_len &&
       u_isPrintable(U_HTTP_CTYPE_TO_PARAM, false))
      {
      buffer.snprintf_add(U_CONSTANT_TO_PARAM("'CONTENT_TYPE=%.*s'\n"), U_HTTP_CTYPE_TO_TRACE);
      }

   uint32_t sz     = U_http_info.uri_len;
   const char* ptr = U_http_info.uri;

   if ((type & U_RAKE) == 0)
      {
#  ifdef U_ALIAS
      bool brequest = false;

      if (*UClientImage_Base::request_uri)
         {
         // The interpreted pathname of the original requested document (relative to the document root)

         sz  = UClientImage_Base::request_uri->size();
         ptr = UClientImage_Base::request_uri->data();

         if (U_http_info.query_len &&
             (type & U_SHELL) == 0)
            {
            brequest = true;

            buffer.snprintf_add(U_CONSTANT_TO_PARAM("REQUEST_URI=%.*s?%.*s\n"), sz, ptr, U_HTTP_QUERY_TO_TRACE);
            }
         }

      if (brequest == false)
#  endif
      buffer.snprintf_add(U_CONSTANT_TO_PARAM("REQUEST_URI=%.*s\n"), sz, ptr);
      }

   // The "CONTENT_LENGTH" header must always be present, even if its value is "0"

   buffer.snprintf_add(U_CONSTANT_TO_PARAM("CONTENT_LENGTH=%u\n"
                       "REQUEST_METHOD=%.*s\n"),
                       UClientImage_Base::body->size(),
                       U_HTTP_METHOD_TO_TRACE);

   if ((type & U_PHP) == 0) buffer.snprintf_add(U_CONSTANT_TO_PARAM("SCRIPT_NAME=%.*s\n"), sz, ptr);
   else
      {
      /**
       * see: http://woozle.org/~neale/papers/php-cgi.html
       *
       * PHP_SELF    The filename of the currently executing script, relative to the document root
       *
       * SCRIPT_NAME The initial portion of the request URL path that corresponds to the application object, so that the application knows
       *             its virtual location. This may be an empty string, if the application corresponds to the root of the server
       * 
       * PATH_INFO   The remainder of the request URL path, designating the virtual location of the request target within the application.
       *             This may be an empty string, if the request URL targets the application root and does not have a trailing slash. This value may
       *             be percent-encoded when I originating from a URL
       * 
       * One of SCRIPT_NAME or PATH_INFO must be set. PATH_INFO should be / if SCRIPT_NAME is empty. SCRIPT_NAME never should be /, but instead be empty
       * 
       * http(s)://${SERVER_NAME}:${SERVER_PORT}${SCRIPT_NAME}${PATH_INFO} will always be an accessible URL that points to the current script
       * 
       * -----------------------------------
       * Mount Point:
       * -----------------------------------
       * URL: /something
       * SCRIPT_NAME:
       * PATH_INFO: /something
       * -----------------------------------
       * Mount Point: /application
       * -----------------------------------
       * URL: /application
       * SCRIPT_NAME: /application
       * PATH_INFO:
       * 
       * URL: /application/
       * SCRIPT_NAME: /application
       * PATH_INFO: /
       * 
       * URL: /application/something
       * SCRIPT_NAME: /application
       * PATH_INFO: /something
       * --------------------------------------------------------------------------------------------------------------------
       * see: http://dev.phpldapadmin.org/pla/issues/34
       * --------------------------------------------------------------------------------------------------------------------
       * There is a redirect loop when using Fast CGI. The problem is that in this case $_SERVER['SCRIPT_NAME']
       * is not filled with the running PHP script but the CGI wrapper. So PLA will redirect to index.php over and over again
       * --------------------------------------------------------------------------------------------------------------------
       */

      uint32_t start = (php_mount_point &&
                        UStringExt::startsWith(ptr, sz, U_STRING_TO_PARAM(*php_mount_point))
                              ? php_mount_point->size()
                              : 0);

      U_INTERNAL_DUMP("start = %u", start)

      buffer.snprintf_add(U_CONSTANT_TO_PARAM("PHP_SELF=%.*s\n"
                          "REDIRECT_STATUS=1\n"
                          "SCRIPT_FILENAME=%w%.*s\n"
                          "SCRIPT_NAME=%.*s\n"
                          "PATH_INFO=%.*s\n"),
                          sz, ptr, sz, ptr,
                          sz + npathinfo - start, ptr + start,
                          sz - npathinfo,        ptr + npathinfo);
      }

   (void) buffer.append(*UServer_Base::cenvironment); // SERVER_(NAME|PORT)

   UMimeHeader requestHeader;
   UHashMap<UString>* prequestHeader = 0;

   U_INTERNAL_DUMP("U_http_info.endHeader = %u", U_http_info.endHeader)

   if (U_http_info.endHeader) // NB: we can have HTTP 1.0 request without headers...
      {
      requestHeader.setIgnoreCase(true);

      if (requestHeader.parse(UClientImage_Base::request->c_pointer(U_http_info.startHeader), U_http_info.endHeader - U_CONSTANT_SIZE(U_CRLF2) - U_http_info.startHeader))
         {
         // The environment must not contain the keys HTTP_CONTENT_TYPE or HTTP_CONTENT_LENGTH (we use the versions without HTTP_)

         requestHeader.removeHeader(U_CONSTANT_TO_PARAM("Content-Type"));
         requestHeader.removeHeader(U_CONSTANT_TO_PARAM("Content-Length"));

         if (requestHeader.empty() == false) prequestHeader = &(requestHeader.table);
         }
      }

   // The hostname of your server from header's request.
   // The difference between HTTP_HOST and U_HTTP_VHOST is that
   // HTTP_HOST can include the «:PORT» text, and U_HTTP_VHOST only the name

   if (U_http_host_len)
      {
                        buffer.snprintf_add(U_CONSTANT_TO_PARAM("HTTP_HOST=%.*s\n"),    U_HTTP_HOST_TO_TRACE);
#  ifdef U_ALIAS
      if (virtual_host) buffer.snprintf_add(U_CONSTANT_TO_PARAM("VIRTUAL_HOST=%.*s\n"), U_HTTP_VHOST_TO_TRACE);
#  endif

      if (prequestHeader)
         {
         requestHeader.removeHeader(U_CONSTANT_TO_PARAM("Host"));

         if (requestHeader.empty()) prequestHeader = 0;
         }
      }

   // ------------------------------------------------------------------------------------------------------------------
   // The visitor's cookie, if one is set
   // ------------------------------------------------------------------------------------------------------------------
   // Cookie: _saml_idp=dXJuOm1hY2U6dGVzdC5zXRo, _redirect_user_idp=urn%3Amace%3Atest.shib%3Afederation%3Alocalhost; ...
   // ------------------------------------------------------------------------------------------------------------------

   if (U_http_info.cookie_len)
      {
      UString cookie, data;

      if (getCookie(&cookie, &data))
         {
         (void) buffer.append(U_CONSTANT_TO_PARAM("'ULIB_SESSION="));
         (void) buffer.append(data.empty() ? data_session->keyid : data);
         (void) buffer.append(U_CONSTANT_TO_PARAM("'\n"));
         }

      if (cookie)
         {
         (void) buffer.append(U_CONSTANT_TO_PARAM("'HTTP_COOKIE="));
         (void) buffer.append(cookie);
         (void) buffer.append(U_CONSTANT_TO_PARAM("'\n"));
         }

      if (prequestHeader)
         {
         requestHeader.removeHeader(U_CONSTANT_TO_PARAM("Cookie"));

         if (requestHeader.empty()) prequestHeader = 0;
         }
      }

   // The URL of the page that called the script

   if (U_http_info.referer_len)
      {
      buffer.snprintf_add(U_CONSTANT_TO_PARAM("'HTTP_REFERER=%.*s'\n"), U_HTTP_REFERER_TO_TRACE);

      if (prequestHeader)
         {
         requestHeader.removeHeader(U_CONSTANT_TO_PARAM("Referer"));

         if (requestHeader.empty()) prequestHeader = 0;
         }
      }

   // The browser type of the visitor

   if (U_http_info.user_agent_len)
      {
      if (u_isPrintable(U_HTTP_USER_AGENT_TO_PARAM, false)) buffer.snprintf_add(U_CONSTANT_TO_PARAM("'HTTP_USER_AGENT=%.*s'\n"), U_HTTP_USER_AGENT_TO_TRACE);

      if (prequestHeader)
         {
         requestHeader.removeHeader(U_CONSTANT_TO_PARAM("User-Agent"));

         if (requestHeader.empty()) prequestHeader = 0;
         }
      }

   if (U_http_accept_len)
      {
      buffer.snprintf_add(U_CONSTANT_TO_PARAM("'HTTP_ACCEPT=%.*s'\n"), U_HTTP_ACCEPT_TO_TRACE);

      if (prequestHeader)
         {
         requestHeader.removeHeader(U_CONSTANT_TO_PARAM("Accept"));

         if (requestHeader.empty()) prequestHeader = 0;
         }
      }

   if (U_http_accept_language_len)
      {
      buffer.snprintf_add(U_CONSTANT_TO_PARAM("'HTTP_ACCEPT_LANGUAGE=%.*s'\n"), U_HTTP_ACCEPT_LANGUAGE_TO_TRACE);

      if (prequestHeader)
         {
         requestHeader.removeHeader(U_CONSTANT_TO_PARAM("Accept-Language"));

         if (requestHeader.empty()) prequestHeader = 0;
         }
      }

#ifndef U_HTTP2_DISABLE
   if (U_http_version == '2')
      {
      U_INTERNAL_ASSERT_EQUALS(prequestHeader, 0)

      prequestHeader = &(UHTTP2::pConnection->itable);
      }
#endif

   if (prequestHeader)
      {
      prequestHeader->callForAllEntry(addHTTPVariables);

      (void) buffer.append(*string_HTTP_Variables);

      string_HTTP_Variables->clear();
      }

#ifdef USE_LIBSSL
   if (UServer_Base::bssl)
      {
           if ((type & U_RAKE)  != 0) (void) buffer.append(U_CONSTANT_TO_PARAM("rack.url_scheme=https\n"));
      else if ((type & U_WSCGI) != 0) (void) buffer.append(U_CONSTANT_TO_PARAM("wsgi.url_scheme=https\n"));
      else                            (void) buffer.append(U_CONSTANT_TO_PARAM("HTTPS=on\n")); // "on" if the script is being called through a secure server

      if ((type & U_SHELL) != 0)
         {
         X509* x509 = ((USSLSocket*)UServer_Base::csocket)->getPeerCertificate();

         if (x509)
            {
            UString issuer  = UCertificate::getIssuer(x509),
                    subject = UCertificate::getSubject(x509);

            buffer.snprintf_add(U_CONSTANT_TO_PARAM("'SSL_CLIENT_I_DN=%v'\n"
                                "'SSL_CLIENT_S_DN=%v'\n"
                                "SSL_CLIENT_CERT_SERIAL=%ld\n"), issuer.rep, subject.rep, UCertificate::getSerialNumber(x509));
            }
         }
      }
   else
#endif
   {
        if ((type & U_RAKE)  != 0) (void) buffer.append(U_CONSTANT_TO_PARAM("rack.url_scheme=http\n"));
   else if ((type & U_WSCGI) != 0) (void) buffer.append(U_CONSTANT_TO_PARAM("wsgi.url_scheme=http\n"));
   }

   if ((type & U_RAKE) == 0)
      {
           if ((type & U_CGI)   != 0) (void) buffer.append(U_CONSTANT_TO_PARAM("GATEWAY_INTERFACE=CGI/1.1\n"));
      else if ((type & U_WSCGI) != 0) (void) buffer.append(U_CONSTANT_TO_PARAM("SCGI=1\n"));

      buffer.snprintf_add(U_CONSTANT_TO_PARAM("SERVER_PROTOCOL=HTTP/1.%c\n"), U_http_version ? U_http_version : '0');

      (void) buffer.append(*UServer_Base::senvironment);

      if ((type & U_SHELL) != 0)
         {
         uint32_t agent  = getUserAgent();
         int remote_port = UServer_Base::csocket->remotePortNumber();

         // "REMOTE_HOST=%.*s\n"  // The hostname of the visitor (if your server has reverse-name-lookups on; otherwise this is IP address again)
         // "REMOTE_USER=%.*s\n"  // The visitor's username (for .htaccess-protected pages)
         // "SERVER_ADMIN=%.*s\n" // The email address for your server's webmaster

         buffer.snprintf_add(U_CONSTANT_TO_PARAM(
             "REMOTE_PORT=%d\n"
             "REMOTE_ADDR=%.*s\n"
             "SESSION_ID=%.*s:%u\n"
             "REQUEST_ID=%.*s:%d:%u\n"
             "PWD=%w"),
             remote_port,
             U_CLIENT_ADDRESS_TO_TRACE,
             U_CLIENT_ADDRESS_TO_TRACE,              agent,
             U_CLIENT_ADDRESS_TO_TRACE, remote_port, agent);

         if (file_data &&
             u_is_cgi(file_data->mime_index))
            {
            (void) buffer.push('/');

            (void) buffer.append(((UHTTP::ucgi*)file_data->ptr)->dir);
            }

         (void) buffer.append(U_CONSTANT_TO_PARAM("\nPATH=/usr/local/bin:/usr/bin:/bin\n"));

         if (*geoip) (void) buffer.append(*geoip);
         }
      }

   if (buffer.isBinary())
      {
#  ifdef DEBUG
      (void) UFile::writeToTmp(U_STRING_TO_PARAM(buffer), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("getCGIEnvironment.bin.%P"), 0);
#  endif

      setBadRequest();

      U_RETURN(false);
      }

   (void) environment.append(buffer);

   U_RETURN(true);
}

U_NO_EXPORT void UHTTP::setCGIShellScript(UString& command)
{
   U_TRACE(0, "UHTTP::setCGIShellScript(%V)", command.rep)

   U_INTERNAL_ASSERT_POINTER(form_name_value)

   // ULib facility: check if present form data and convert it in parameters for shell script...

   char c;
   UString item;
   const char* ptr;
   uint32_t i, sz, n = processForm();

   for (i = 1; i < n; i += 2)
      {
      item = (*form_name_value)[i];

      // check for binary data (not valid)...

      if (item.empty() ||
          item.isBinary())
         {
         c    = '\'';
         sz   = 0;
         ptr  = 0;

         if (item)
            {
#        ifdef DEBUG
            char buffer[4096];

            U_INTERNAL_DUMP("form item:\n"
                            "--------------------------------------\n"
                            "%s", u_memoryDump(buffer, (unsigned char*)U_STRING_TO_PARAM(item)))
#        endif

            U_SRV_LOG("WARNING: Found binary data in form item: %V", (*form_name_value)[i-1].rep);
            }
         }
      else
         {
         sz  = item.size();
         ptr = item.data();

         // we find how to escape the param...

         c = (memchr(ptr, '"', sz) ? '\'' : '"');
         }

      (void) command.reserve(sz + 4U);

      command.snprintf_add(U_CONSTANT_TO_PARAM(" %c%.*s%c "), c, sz, ptr, c);
      }
}

bool UHTTP::manageSendfile(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHTTP::manageSendfile(%.*S,%u)", len, ptr, len)

   pathname->setBuffer(u_cwd_len + 1 + len);

   if (ptr[0] == '/') pathname->snprintf(U_CONSTANT_TO_PARAM(   "%.*s"), len, ptr);
   else               pathname->snprintf(U_CONSTANT_TO_PARAM("%w/%.*s"), len, ptr);

   if (u_canonicalize_pathname(pathname->data())) pathname->size_adjust_force(); // NB: pathname is referenced...

   file->setPath(*pathname);

   if (file->stat()    &&
       file->st_size   &&
       file->regular() &&
       file->open())
      {
      UClientImage_Base::setRequestNoCache();

      if (*ext)
         {
         U_INTERNAL_ASSERT(u_endsWith(U_STRING_TO_PARAM(*ext), U_CONSTANT_TO_PARAM(U_CRLF)))

         ext->setBuffer(U_CAPACITY);
         }

      UClientImage_Base::body->clear();
      UClientImage_Base::wbuffer->clear();

      if (UStringExt::startsWith(U_FILE_TO_PARAM(*file), U_CONSTANT_TO_PARAM("/tmp/"))) file->_unlink();

      file_data       = file_not_in_cache_data;
      file_data->fd   = file->fd;
      file_data->size = file->st_size;

      processGetRequest();

      if (file_data->fd != UServer_Base::pClientImage->sfd) file->close();
      else
         {
         U_ClientImage_pclose(UServer_Base::pClientImage) = U_CLOSE;
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

U_NO_EXPORT bool UHTTP::splitCGIOutput(const char*& ptr1, const char* ptr2)
{
   U_TRACE(0, "UHTTP::splitCGIOutput(%p,%p)", ptr1, ptr2)

   uint32_t pos = UClientImage_Base::wbuffer->distance(ptr1);

   if (pos == 0) ext->setBuffer(U_CAPACITY);
   else         *ext = UClientImage_Base::wbuffer->substr(0U, pos);

   U_INTERNAL_DUMP("U_http_info.endHeader = %u", U_http_info.endHeader)

   ptr1 = (const char*) memchr(ptr2, '\r', U_http_info.endHeader - pos);

   if (ptr1)
      {
      pos = UClientImage_Base::wbuffer->distance(ptr1) + U_CONSTANT_SIZE(U_CRLF); // NB: we cut \r\n

      U_INTERNAL_DUMP("pos = %u U_http_info.endHeader = %u", pos, U_http_info.endHeader)

      U_INTERNAL_ASSERT_MINOR(pos, U_http_info.endHeader)

      uint32_t diff = U_http_info.endHeader - pos;

      U_INTERNAL_DUMP("diff = %u", diff)

      if (diff > U_CONSTANT_SIZE(U_CRLF2))
         {
         // NB: we cut one couple of '\r\n'

         (void) ext->append(UClientImage_Base::wbuffer->c_pointer(pos), diff - U_CONSTANT_SIZE(U_CRLF));
         }

      U_INTERNAL_DUMP("value = %.*S ext = %V", (uint32_t)((ptrdiff_t)ptr1 - (ptrdiff_t)ptr2), ptr2, ext->rep)

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UHTTP::processCGIOutput(bool cgi_sh_script, bool bheaders)
{
   U_TRACE(0, "UHTTP::processCGIOutput(%b,%b)", cgi_sh_script, bheaders)

   /**
    * CGI Script Output
    *
    * The script sends its output to stdout. This output can either be a document generated by the script, or instructions to the server
    * for retrieving the desired output.
    *
    * Script naming conventions
    *
    * Normally, scripts produce output which is interpreted and sent back to the client. An advantage of this is that the scripts do not
    * need to send a full HTTP/1.x header for every request.
    *
    * Some scripts may want to avoid the extra overhead of the server parsing their output, and talk directly to the client. In order to
    * distinguish these scripts from the other scripts, CGI requires that the script name begins with nph- if a script does not want the
    * server to parse its header. In this case, it is the script's responsibility to return a valid HTTP response to the client.
    *
    * Parsed headers
    *
    * The output of scripts begins with a small header. This header consists of text lines, in the same format as an HTTP header,
    * terminated by a blank line (a line with only a linefeed or CR/LF). Any headers which are not server directives are sent directly
    * back to the client. Currently, this specification defines three server directives:
    *
    * Status:       This is used to give the server an HTTP status line to send to the client. The format is nnn xxxxx, where nnn is
    *               the 3-digit status code, and xxxxx is the reason string, such as "Forbidden"
    *
    * Content-type: This is the MIME type of the document you are returning
    *
    * Location:     This is used to specify to the server that you are returning a reference to a document rather than an actual document.
    *               If the argument to this is a URL, the server will issue a redirect to the client
    */

   const char* ptr;
   const char* ptr1;
   const char* base;
   bool http_response;
   const char* endptr;
   uint32_t pos, sz, diff;

   if (UClientImage_Base::wbuffer->empty() ||
       UClientImage_Base::wbuffer->isWhiteSpace())
      {
      goto error;
      }

   sz           = UClientImage_Base::wbuffer->size();
   ptr = endptr = UClientImage_Base::wbuffer->data();

   U_INTERNAL_DUMP("U_http_info.endHeader = %u U_line_terminator_len = %d UClientImage_Base::wbuffer(%u) = %.*S", U_http_info.endHeader, U_line_terminator_len, sz, sz, ptr)

#ifdef DEBUG
// (void) UFile::writeToTmp(ptr, sz, O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("processCGIOutput.%P"), 0);
#endif

   if (bheaders == false)
      {
      // NB: we can have HTML content without HTTP headers...

      if (u_isHTML(ptr))
         {
         mime_index = U_html;

         U_http_info.endHeader = 0;

         goto nheader;
         }

rescan:
      U_http_info.endHeader = u_findEndHeader(ptr, sz); // NB: U_http_info.endHeader comprende anche la blank line...

      U_INTERNAL_DUMP("U_http_info.endHeader = %u U_line_terminator_len = %d", U_http_info.endHeader, U_line_terminator_len)

      if (U_http_info.endHeader == U_NOT_FOUND)
         {
         // NB: we assume to have some content not HTML without HTTP headers...

nheader: U_http_info.endHeader = 0;

         goto end;
         }

      if (u_get_unalignedp16(ptr+U_http_info.endHeader-2) == U_MULTICHAR_CONSTANT16('\n','\n'))
         {
         UString tmp                 = UStringExt::dos2unix(UClientImage_Base::wbuffer->substr(0U, U_http_info.endHeader), true) +
                                                            UClientImage_Base::wbuffer->substr(    U_http_info.endHeader);
         *UClientImage_Base::wbuffer = tmp;

         sz  = UClientImage_Base::wbuffer->size();
         ptr = UClientImage_Base::wbuffer->data();

         goto rescan;
         }
      }

   U_INTERNAL_DUMP("U_http_info.endHeader = %u sz = %u U_http_info.nResponseCode = %u", U_http_info.endHeader, sz, U_http_info.nResponseCode)

   U_INTERNAL_ASSERT_EQUALS(U_line_terminator_len, 2)
   U_INTERNAL_ASSERT_RANGE(1, U_http_info.endHeader, sz)

   http_response = false;

   endptr = UClientImage_Base::wbuffer->c_pointer(U_http_info.endHeader);

loop:
   U_INTERNAL_ASSERT_MINOR(ptr, endptr)

   U_INTERNAL_DUMP("ptr = %.20S", ptr)

   switch (u_get_unalignedp32(ptr))
      {
      case U_MULTICHAR_CONSTANT32('X','-','S','e'):
      case U_MULTICHAR_CONSTANT32('X','-','A','c'):
         {
         if (http_response == false)
            {
            U_INTERNAL_DUMP("check 'X-Sendfile: ...' or 'X-Accel-Redirect: ...'")

            ptr1 = 0;

            /**
             * X-Sendfile is a special, non-standard HTTP header. At first you might think it is no big deal, but think again.
             * It can be enabled in any CGI, FastCGI or SCGI backend. Basically its job is to instruct the web server to ignore
             * the content of the response and replace it by whatever is specified in the header. The main advantage of this is
             * that it will be server the one serving the file, making use of all its optimizations. It is useful for processing
             * script-output of e.g. php, perl, ruby or any cgi. This is particularly useful because it hands the load to server,
             * all the response headers from the backend are forwarded, the whole application uses a lot less resources and performs
             * several times faster not having to worry about a task best suited for a web server. You retain the ability to check for
             * special privileges or dynamically deciding anything contemplated by your backend logic, you speed up things a lot while
             * having more resources freed, and you can even specify the delivery of files outside of the web server's document root path.
             * Of course, this is to be done solely in controlled environments. In short, it offers a huge performance gain at absolutely
             * no cost. Note that the X-Sendfile feature also supports X-Accel-Redirect header, a similar feature offered by other web
             * servers. This is to allow the migration of applications supporting it without having to make major code rewrites
             */

                 if (u_get_unalignedp64(ptr+4) == U_MULTICHAR_CONSTANT64('n','d','f','i','l','e',':',' ')) ptr1 = ptr + U_CONSTANT_SIZE("X-Sendfile: ");
            else if (u_get_unalignedp64(ptr+4) == U_MULTICHAR_CONSTANT64('c','e','l','-','R','e','d','i')) ptr1 = ptr + U_CONSTANT_SIZE("X-Accel-Redirect: ");

            if (ptr1)
               {
               if (splitCGIOutput(ptr, ptr1) &&
                   manageSendfile(ptr1, ptr - ptr1))
                  {
                  U_SRV_LOG("Header X-Sendfile found in CGI output: serving file %V", pathname->rep);

                  U_RETURN(true);
                  }

               goto error;
               }
            }
         }
      break;

      case U_MULTICHAR_CONSTANT32('L','o','c','a'):
         {
         // we check if it is used to specify to the server that you are returning a reference to a document...

         U_INTERNAL_DUMP("check 'Location: ...'")

         if (u_get_unalignedp32(ptr+4) == U_MULTICHAR_CONSTANT32('t','i','o','n'))
            {
            ptr1 = ptr + U_CONSTANT_SIZE("Location: ");

            if (splitCGIOutput(ptr, ptr1))
               {
               setRedirectResponse(PARAM_DEPENDENCY, ptr1, ptr - ptr1);

               U_RETURN(true);
               }

            goto error;
            }
         }
      break;

      case U_MULTICHAR_CONSTANT32('S','t','a','t'):
         {
         if (http_response == false)
            {
            // ------------------------------------------------------------------------------------------------------------------------------------------
            // we check if it is used to give the server an HTTP status line to send to the client...
            // ------------------------------------------------------------------------------------------------------------------------------------------
            // Ex: "Status: 503 Service Unavailable\r\nX-Powered-By: PHP/5.2.6-pl7-gentoo\r\nContent-Type: text/html;charset=utf-8\r\n\r\n<!DOCTYPE html"
            // ------------------------------------------------------------------------------------------------------------------------------------------

            U_INTERNAL_DUMP("check 'Status: ...'")

            if (u_get_unalignedp32(ptr+4) == U_MULTICHAR_CONSTANT32('u','s',':',' '))
               {
               const char* start;

               ptr1 = ptr + U_CONSTANT_SIZE("Status: ");

               U_INTERNAL_ASSERT(u__isdigit(ptr1[0]))

               for (start = ptr1; u__isdigit(*ptr1); ++ptr1) {}

               U_http_info.nResponseCode = u_strtoul(start, ptr1);

               U_INTERNAL_DUMP("U_http_info.nResponseCode = %d", U_http_info.nResponseCode)

               ptr1 = (const char*) memchr(ptr1, '\n', endptr - ptr1);

               if (ptr1 == 0) goto error;

               diff = (ptr1 - ptr) + 1; // NB: we cut also \n...

               U_INTERNAL_ASSERT_MINOR(diff, 128)
               U_INTERNAL_ASSERT(diff <= U_http_info.endHeader)

               sz                    -= diff;
               U_http_info.endHeader -= diff;

               U_INTERNAL_DUMP("diff = %u U_http_info.endHeader = %u", diff, U_http_info.endHeader)

               pos = UClientImage_Base::wbuffer->distance(ptr);

               if (pos  == 0 &&
                   diff == sz)
                  {
                  // NB: we assume to have no content without HTTP headers...

                  UClientImage_Base::body->clear();

                  setResponse();

                  U_RETURN(true);
                  }

               UClientImage_Base::wbuffer->erase(pos, diff);

               ptr    = UClientImage_Base::wbuffer->data();
               endptr = UClientImage_Base::wbuffer->c_pointer(U_http_info.endHeader);

               U_INTERNAL_DUMP("wbuffer(%u) = %V", sz, UClientImage_Base::wbuffer->rep)

               U_ASSERT_EQUALS(sz, UClientImage_Base::wbuffer->size())

               goto loop;
               }
            }
         }
      break;

      case U_MULTICHAR_CONSTANT32('S','e','t','-'):
         {
         if (cgi_sh_script)
            {
            // ULib facility: we check for request: 'TODO timed session cookie'...

            U_INTERNAL_DUMP("check 'Set-Cookie: TODO['")

            if (u_get_unalignedp64(ptr+ 4) == U_MULTICHAR_CONSTANT64('C','o','o','k','i','e',':',' ') &&
                u_get_unalignedp32(ptr+12) == U_MULTICHAR_CONSTANT32('T','O','D','O'))
               {
               uint32_t pos1,
                        pos2 = U_CONSTANT_SIZE("Set-Cookie: "),
                        len, n2, n1 = U_CONSTANT_SIZE("TODO[");

               ptr += pos2;
               pos1 = UClientImage_Base::wbuffer->distance(ptr);

               ptr += n1;
               ptr1 = ptr;

               ptr = (const char*) memchr(ptr1, ']', endptr - ptr1);

               if (ptr == 0) goto error;

               len = ptr - ptr1;

               n1 += len + 1; // ']'

               U_INTERNAL_DUMP("ptr = %.20S", ptr)

               setCookie(UClientImage_Base::wbuffer->substr(ptr1, len));

               n2   = set_cookie->size() - 2 - pos2,
               diff = n2 - n1;

               sz                    += diff;
               U_http_info.endHeader += diff;

               U_INTERNAL_DUMP("diff = %u U_http_info.endHeader = %u", diff, U_http_info.endHeader)

               U_INTERNAL_ASSERT_MINOR(diff, 512)

               (void) UClientImage_Base::wbuffer->replace(pos1, n1, *set_cookie, pos2, n2);

               set_cookie->setEmpty();

               U_INTERNAL_DUMP("wbuffer(%u) = %#V", sz, UClientImage_Base::wbuffer->rep)

               U_ASSERT_EQUALS(sz, UClientImage_Base::wbuffer->size())

               // check if we have for more parsing...

               if ((pos1 += n2 + 2) < U_http_info.endHeader)
                  {
                  ptr    = UClientImage_Base::wbuffer->c_pointer(pos1);
                  endptr = UClientImage_Base::wbuffer->c_pointer(U_http_info.endHeader);

                  goto loop;
                  }
               }
            }
         }
      break;

      case U_MULTICHAR_CONSTANT32('H','T','T','P'): // response line: HTTP/1.n nnn <ssss>
         {
         if (scanfHeaderResponse(ptr, endptr - ptr)) // we check for script's responsibility to return a valid HTTP response to the client...
            {
            http_response = true;
            cgi_sh_script = false;

            if ((ptr += U_http_info.startHeader) < endptr) goto loop;
            }
         }
      break;

      case U_MULTICHAR_CONSTANT32('C','o','n','n'):
         {
         // NB: we check if it is used to specify that the server must close the connection...

         U_INTERNAL_DUMP("check 'Connection: close'")

         if (u_get_unalignedp64(ptr+ 4) == U_MULTICHAR_CONSTANT64('e','c','t','i','o','n',':',' ') &&
             u_get_unalignedp32(ptr+12) == U_MULTICHAR_CONSTANT32('c','l','o','s'))
            {
            sz                    -= U_CONSTANT_SIZE("Connection: close\r\n");
            U_http_info.endHeader -= U_CONSTANT_SIZE("Connection: close\r\n");

            pos = UClientImage_Base::wbuffer->distance(ptr);

            UClientImage_Base::wbuffer->erase(pos, U_CONSTANT_SIZE("Connection: close\r\n"));

            ptr    = UClientImage_Base::wbuffer->data();
            endptr = UClientImage_Base::wbuffer->c_pointer(U_http_info.endHeader);

            U_INTERNAL_DUMP("wbuffer(%u) = %V", sz, UClientImage_Base::wbuffer->rep)

            U_ASSERT_EQUALS(sz, UClientImage_Base::wbuffer->size())

            UClientImage_Base::setCloseConnection();

            goto loop;
            }
         }
      break;

      case U_MULTICHAR_CONSTANT32('C','o','n','t'):
         {
         if (http_response == false)
            {
            U_INTERNAL_DUMP("check 'Content-...: ...'")

            base = ptr;
                   ptr += 4;

            if (u_get_unalignedp64(ptr) == U_MULTICHAR_CONSTANT64('e','n','t','-','T','y','p','e'))
               {
               U_http_content_type_len = 1;

               ptr += U_CONSTANT_SIZE("ent-Type: ");

               if (u_get_unalignedp32(ptr) != U_MULTICHAR_CONSTANT32('t','e','x','t'))
                  {
                  U_http_flag &= ~HTTP_IS_ACCEPT_GZIP;

                  U_INTERNAL_DUMP("U_http_is_accept_gzip = %b", U_http_is_accept_gzip)
                  }
               }
            else if (u_get_unalignedp64(ptr) == U_MULTICHAR_CONSTANT64('e','n','t','-','E','n','c','o'))
               {
               U_http_flag &= ~HTTP_IS_ACCEPT_GZIP;

               U_INTERNAL_DUMP("U_http_is_accept_gzip = %b", U_http_is_accept_gzip)

               ptr += U_CONSTANT_SIZE("ent-Encoding: ");
               }
            else if (u_get_unalignedp64(ptr) == U_MULTICHAR_CONSTANT64('e','n','t','-','L','e','n','g'))
               {
               ptr += U_CONSTANT_SIZE("ent-Length: ");

               U_INTERNAL_DUMP("Content-Length: = %ld", ::strtol(ptr, 0, 10))

               ptr1 = (const char*) memchr(ptr, '\n', endptr - ptr);

               if (ptr1 == 0) goto error;

               diff = (ptr1 - base) + 1; // NB: we cut also '\n'...

               U_INTERNAL_ASSERT_MINOR(diff, 32)
               U_INTERNAL_ASSERT_MINOR(diff, U_http_info.endHeader)

               sz                    -= diff;
               U_http_info.endHeader -= diff;

               U_INTERNAL_DUMP("diff = %u U_http_info.endHeader = %u", diff, U_http_info.endHeader)

               UClientImage_Base::wbuffer->erase(UClientImage_Base::wbuffer->distance(base), diff);

               ptr    = UClientImage_Base::wbuffer->data();
               endptr = UClientImage_Base::wbuffer->c_pointer(U_http_info.endHeader);

               U_INTERNAL_DUMP("wbuffer(%u) = %V", sz, UClientImage_Base::wbuffer->rep)

               U_ASSERT_EQUALS(sz, UClientImage_Base::wbuffer->size())

               goto loop;
               }
            }
         }
      break;
      }

   ptr = (const char*) memchr(ptr, '\n', endptr - ptr);

   if (  ptr &&
       ++ptr < endptr)
      {
      goto loop;
      }

   if (http_response)
      {
      UClientImage_Base::body->clear();

      UClientImage_Base::setNoHeaderForResponse();

      U_RETURN(true);
      }

   U_INTERNAL_DUMP("sz = %u U_http_info.endHeader = %u UClientImage_Base::wbuffer(%u) = %V",
                    sz,     U_http_info.endHeader,     UClientImage_Base::wbuffer->size(), UClientImage_Base::wbuffer->rep)

   U_ASSERT_EQUALS(sz, UClientImage_Base::wbuffer->size())

   if (sz == U_http_info.endHeader)
      {
      // NB: we assume to have no content with some HTTP headers...

      UClientImage_Base::body->clear();

      // NB: we use the var 'set_cookie' for opportunism within handlerResponse()...

      (void) set_cookie->append(UClientImage_Base::wbuffer->data(), sz - U_CONSTANT_SIZE(U_CRLF));

      setResponse();

      U_RETURN(true);
      }

   if (u_isHTML(endptr)) mime_index = U_html;

end:
   setDynamicResponse();

   U_RETURN(true);

error:
   U_SRV_LOG("WARNING: UHTTP::processCGIOutput(%b,%b) failed", cgi_sh_script, bheaders);

   U_RETURN(false);
}

bool UHTTP::processCGIRequest(UCommand* cmd, UHTTP::ucgi* cgi)
{
   U_TRACE(0, "UHTTP::processCGIRequest(%p,%p)", cmd, cgi)

   static int fd_stderr;

   // process the CGI or script request

   U_INTERNAL_DUMP("U_http_method_type = %B URI = %.*S U_http_info.nResponseCode = %d", U_http_method_type, U_HTTP_URI_TO_TRACE, U_http_info.nResponseCode)

   U_ASSERT(cmd->checkForExecute())
   U_INTERNAL_ASSERT(*UClientImage_Base::environment)

   cmd->setEnvironment(UClientImage_Base::environment);

   /**
    * When a url ends by "cgi-bin/" it is assumed to be a cgi script.
    * The server changes directory to the location of the script and
    * executes it after setting QUERY_STRING and other environment variables
    */

   if (cgi) (void) UFile::chdir(cgi->dir, true);

   // execute script...

   if (cgi_timeout) cmd->setTimeout(cgi_timeout);

   if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/processCGIRequest.err");

   bool result = cmd->execute(UClientImage_Base::body->empty() ? 0 : UClientImage_Base::body, UClientImage_Base::wbuffer, -1, fd_stderr);

   if (cgi) (void) UFile::chdir(0, true);

#ifndef U_LOG_DISABLE
   UServer_Base::logCommandMsgError(cmd->getCommand(), false);
#endif

   cmd->reset(UClientImage_Base::environment);

   cmd->environment.clear();

   if (result == false ||
       UClientImage_Base::wbuffer->empty())
      {
      if (UCommand::isTimeout())
         {
         U_http_info.nResponseCode = HTTP_GATEWAY_TIMEOUT;

         setResponse();
         }
      else
         {
         // NB: exit_value consists of the least significant 8 bits of the status argument that the child specified in the call to exit()...

         if (UCommand::exit_value > 128       &&
             cgi                              &&
             cgi->environment_type == U_SHELL &&
             U_IS_HTTP_ERROR(UCommand::exit_value + 256))
            {
            U_http_info.nResponseCode = UCommand::exit_value + 256;

            setResponse();
            }
         else
            {
            setInternalError();
            }
         }

      U_RETURN(false);
      }

   U_RETURN(true);
}

bool UHTTP::checkContentLength(uint32_t length, uint32_t pos)
{
   U_TRACE(0, "UHTTP::checkContentLength(%u,%u)", length, pos)

   const char* ptr;

   if (pos != U_NOT_FOUND) ptr = ext->c_pointer(pos);  
   else
      {
      pos = U_STRING_FIND(*ext, 0, "Content-Length");

      U_INTERNAL_ASSERT_DIFFERS(pos, U_NOT_FOUND)

      ptr = ext->c_pointer(pos += U_CONSTANT_SIZE("Content-Length") + 1);
      }

   if (u__isblank(*ptr)) // NB: weighttp fail if we don't put at least one space...
      {
      ++ptr;
      ++pos;
      }

   uint32_t end = ext->findWhiteSpace(pos);

   U_INTERNAL_ASSERT_DIFFERS(end, U_NOT_FOUND)

   uint32_t clength = u_strtoul(ptr, ext->c_pointer(end));

   U_INTERNAL_DUMP("ptr = %.20S clength = %u", ptr, clength)

   if (clength != length)
      {
      char bp[12];

      uint32_t sz_len1 = end - pos,
               sz_len2 = u_num2str32(length, bp) - bp;
                         
      U_INTERNAL_DUMP("sz_len1 = %u sz_len2 = %u", sz_len1, sz_len2)

      while (u__isspace(*ptr))
         {
         if (sz_len1 == sz_len2) break;

         ++ptr;
         --sz_len1;
         }

      pos = ext->distance(ptr);

      (void) ext->replace(pos, sz_len1, bp, sz_len2);

      U_INTERNAL_DUMP("ext(%u) = %#V", ext->size(), ext->rep)

      U_RETURN(true);
      }

   U_RETURN(false);
}

typedef struct { long start, end; } HTTPRange;

/**
 * The Range: header is used with a GET request.
 *
 * For example assume that will return only a portion (let's say the first 32 bytes) of the requested resource...
 * Range: bytes=0-31
 *
 * If @end is non-negative, then @start and @end represent the bounds
 * of the range, counting from %0. (Eg, the first 500 bytes would be
 * represented as @start = %0 and @end = %499.)
 *
 * If @end is %-1 and @start is non-negative, then this represents a
 * range starting at @start and ending with the last byte of the
 * requested resource body. (Eg, all but the first 500 bytes would be
 * @start = %500, and @end = %-1.)
 *
 * If @end is %-1 and @start is negative, then it represents a "suffix
 * range", referring to the last -@start bytes of the resource body.
 * (Eg, the last 500 bytes would be @start = %-500 and @end = %-1.)
 *
 * The If-Range: header allows a client to "short-circuit" the request (conditional GET).
 * Informally, its meaning is `if the entity is unchanged, send me the part(s) that I am
 * missing; otherwise, send me the entire new entity'.
 *
 * If-Range: ( entity-tag | HTTP-date )
 *
 * If the client has no entity tag for an entity, but does have a Last-Modified date, it
 * MAY use that date in an If-Range header. (The server can distinguish between a valid
 * HTTP-date and any form of entity-tag by examining no more than two characters.) The If-Range
 * header SHOULD only be used together with a Range header, and MUST be ignored if the request
 * does not include a Range header, or if the server does not support the sub-range operation 
 */

U_NO_EXPORT bool UHTTP::checkGetRequestIfRange()
{
   U_TRACE_NO_PARAM(0, "UHTTP::checkGetRequestIfRange()")

   const char* ptr = getHeaderValuePtr(U_CONSTANT_TO_PARAM("If-Range"), false);

   if (ptr)
      {
      if (*ptr == '"') // entity-tag
         {
         uint32_t sz = etag->size();

         if (sz &&
             etag->equal(ptr, sz) == false)
            {
            U_RETURN(false);
            }
         }
      else // HTTP-date
         {
         time_t since = UTimeDate::getSecondFromTime(ptr, true);

         U_INTERNAL_DUMP("since = %ld", since)
         U_INTERNAL_DUMP("mtime = %ld", file->st_mtime)

         if (file->st_mtime > since) U_RETURN(false);
         }
      }

   UClientImage_Base::setRequestNoCache();

   U_RETURN(true);
}

U_NO_EXPORT __pure int UHTTP::sortRange(const void* a, const void* b)
{
   U_TRACE(0, "UHTTP::sortRange(%p,%p)", a, b)

#ifdef U_STDCPP_ENABLE
   /**
    * The comparison function must follow a strict-weak-ordering
    *
    * 1) For all x, it is not the case that x < x (irreflexivity)
    * 2) For all x, y, if x < y then it is not the case that y < x (asymmetry)
    * 3) For all x, y, and z, if x < y and y < z then x < z (transitivity)
    * 4) For all x, y, and z, if x is incomparable with y, and y is incomparable with z, then x is incomparable with z (transitivity of incomparability)
    */

   return ((((HTTPRange*)a)->start - ((HTTPRange*)b)->start) < 0);
#else
   return (*(HTTPRange**)a)->start - (*(HTTPRange**)b)->start;
#endif
}

U_NO_EXPORT void UHTTP::setResponseForRange(uint32_t _start, uint32_t _end, uint32_t _header)
{
   U_TRACE(0, "UHTTP::setResponseForRange(%u,%u,%u)", _start, _end, _header)

   // Single range

   U_INTERNAL_ASSERT(_start <= _end)
   U_INTERNAL_ASSERT_RANGE(_start, _end, range_size-1)

   UString tmp(100U);

   tmp.snprintf(U_CONSTANT_TO_PARAM("Content-Range: bytes %u-%u/%u\r\n"), _start, _end, range_size);

   range_size = _end - _start + 1;

   if (*ext) (void) checkContentLength(_header + range_size, U_NOT_FOUND);

   (void) ext->insert(0, tmp);

   U_INTERNAL_DUMP("ext = %V", ext->rep)
}

// return U_YES     - ok
// return U_PARTIAL - ok
// return U_NOT     - error

U_NO_EXPORT int UHTTP::checkGetRequestForRange(const UString& data)
{
   U_TRACE(0, "UHTTP::checkGetRequestForRange(%V)", data.rep)

   U_INTERNAL_ASSERT_MAJOR(U_http_range_len, 0)

   char* pend;
   uint32_t i, n;
   HTTPRange* cur;
   long cur_start, cur_end;
   UVector<HTTPRange*> array;
   UVector<UString> range_list;
   UString range(U_http_info.range, U_http_range_len), item;

   for (i = 0, n = range_list.split(U_http_info.range, U_http_range_len, ','); i < n; ++i)
      {
      item = range_list[i];

      const char* spec = item.data();

      U_INTERNAL_DUMP("spec = %.*S", 10, spec)

      if (*spec == '-')
         {
         cur_start = ::strtol(spec, &pend, 10) + range_size;
         cur_end   = range_size - 1;
         }
      else
         {
         cur_start = ::strtol(spec, &pend, 10);

         if (*pend == '-') ++pend;

         U_INTERNAL_DUMP("item.remain(pend) = %u", item.remain(pend))

         cur_end = (item.remain(pend) ? ::strtol(pend, &pend, 10) : range_size - 1);
         }

      U_INTERNAL_DUMP("cur_start = %ld cur_end = %ld", cur_start, cur_end)

      if (cur_end >= (long)range_size) cur_end = range_size - 1;

      if (cur_start <= cur_end)
         {
         U_INTERNAL_ASSERT_RANGE(cur_start, cur_end, range_size-1)

         cur = new HTTPRange;

         cur->end   = cur_end;
         cur->start = cur_start;

         array.push(cur);
         }
      }

   n = array.size();

   if (n > 1)
      {
      array.sort(sortRange);

      for (i = 1, n = array.size(); i < n; ++i)
         {
         cur  = array[i];

         HTTPRange* prev = array[i-1];

         U_INTERNAL_DUMP("prev->start = %u prev->end = %u", prev->start, prev->end)
         U_INTERNAL_DUMP(" cur->start = %u  cur->end = %u",  cur->start,  cur->end)

         if (cur->start <= prev->end)
            {
            prev->end = U_max(prev->end, cur->end);

            array.erase(i);
            }
         }

      n = array.size();
      }

   if (n == 0)
      {
      U_http_info.nResponseCode = HTTP_REQ_RANGE_NOT_OK;

      U_RETURN(U_NOT);
      }

   if (n == 1) // Single range
      {
      cur = array[0];

      setResponseForRange((range_start = cur->start), cur->end, 0);

      U_RETURN(U_PARTIAL);
      }

   /**
    * Multiple ranges, so we build a multipart/byteranges response
    * ------------------------------------------------------------
    * GET /index.html HTTP/1.1
    * Host: www.unirel.com
    * User-Agent: curl/7.21.0 (x86_64-pc-linux-gnu) libcurl/7.21.0 GnuTLS/2.10.0 zlib/1.2.5
    * Range: bytes=100-199,500-599
    * ------------------------------------------------------------
    * HTTP/1.1 206 Partial Content
    * Date: Fri, 09 Jul 2010 10:27:52 GMT
    * Server: Apache/2.0.49 (Linux/SuSE)
    * Last-Modified: Fri, 06 Nov 2009 17:59:33 GMT
    * Accept-Ranges: bytes
    * Content-Length: 431
    * Content-Type: multipart/byteranges; boundary=48af1db00244c25fa
    *
    * --48af1db00244c25fa
    * Content-type: text/html; charset=ISO-8859-1
    * Content-range: bytes 100-199/598
    *
    * ............
    * --48af1db00244c25fa
    * Content-type: text/html; charset=ISO-8859-1
    * Content-range: bytes 500-597/598
    *
    * ............
    * --48af1db00244c25fa--
    */

   char buffer[64];
   char* ptr = buffer;

   u_put_unalignedp64(ptr,   U_MULTICHAR_CONSTANT64('C','o','n','t','e','n','t','-'));
   u_put_unalignedp64(ptr+8, U_MULTICHAR_CONSTANT64('L','e','n','g','t','h',':',' '));

   ptr += U_CONSTANT_SIZE("Content-Length: ");

   UMimeMultipartMsg response(U_CONSTANT_TO_PARAM("byteranges"), UMimeMultipartMsg::NONE,
                              buffer, U_CONSTANT_SIZE("Content-Length: ") + u_num2str32(range_size, ptr) - ptr, false);

   for (i = 0; i < n; ++i)
      {
      cur = array[i];

      uint32_t start = cur->start,
                _end = cur->end;

      U_INTERNAL_ASSERT(start <= _end)
      U_INTERNAL_ASSERT_RANGE(start,_end,range_size-1)

      ptr = buffer;

      u_put_unalignedp64(ptr,    U_MULTICHAR_CONSTANT64('C','o','n','t','e','n','t','-'));
      u_put_unalignedp64(ptr+8,  U_MULTICHAR_CONSTANT64('R','a','n','g','e',':',' ','b'));
      u_put_unalignedp32(ptr+16, U_MULTICHAR_CONSTANT32('y','t','e','s'));

       ptr += U_CONSTANT_SIZE("Content-Range: bytes");
      *ptr  = ' ';
       ptr  = u_num2str32(start, ptr+1);
      *ptr  = '-';
       ptr  = u_num2str32(_end, ptr+1);
      *ptr  = '/';
       ptr  = u_num2str32(range_size, ptr+1);

      response.add(UMimeMultipartMsg::section(data.substr(start, _end - start + 1),
                                              U_CTYPE_HTML, U_CONSTANT_SIZE(U_CTYPE_HTML),
                                              UMimeMultipartMsg::NONE, "", "",
                                              buffer, ptr - buffer));
      }

   uint32_t content_length = response.message(*ext, false);

   (void) checkContentLength(content_length, U_NOT_FOUND);

#ifdef DEBUG
   (void) UFile::writeToTmp(U_STRING_TO_PARAM(*ext), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("byteranges.%P"), 0);
#endif

   U_http_info.nResponseCode = HTTP_PARTIAL;

   U_RETURN(U_YES);
}

#define U_NO_If_Unmodified_Since // I think it's not very much used...

U_NO_EXPORT bool UHTTP::checkGetRequestIfModified()
{
   U_TRACE_NO_PARAM(0, "UHTTP::checkGetRequestIfModified()")

   /**
    * The If-Modified-Since: header is used with a GET request. If the requested resource has been modified since the given date, ignore the header
    * and return the resource as you normally would. Otherwise, return a "304 Not Modified" response, including the Date: header and no message body, like
    * 
    * HTTP/1.1 304 Not Modified
    * Date: Fri, 31 Dec 1999 23:59:59 GMT
    * [blank line here]
    */

   if (U_http_info.if_modified_since)
      {
      U_INTERNAL_DUMP("since = %u",  U_http_info.if_modified_since)
      U_INTERNAL_DUMP("mtime = %ld", file->st_mtime)

      if (file->st_mtime <= (long)U_http_info.if_modified_since)
         {
         U_http_info.nResponseCode = HTTP_NOT_MODIFIED;

         U_RETURN(false);
         }
      }
#ifndef U_NO_If_Unmodified_Since
   else
      {
      /**
       * The If-Unmodified-Since: header is similar, but can be used with any method. If the requested resource has not been modified since
       * the given date, ignore the header and return the resource as you normally would. Otherwise, return a "412 Precondition Failed" response, like
       *
       * HTTP/1.1 412 Precondition Failed
       * Date: Fri, 31 Dec 1999 23:59:59 GMT
       * [blank line here]
       */

      const char* ptr = getHeaderValuePtr(U_CONSTANT_TO_PARAM("If-Unmodified-Since"), false);

      if (ptr)
         {
         time_t since = UTimeDate::getSecondFromTime(ptr, true);

         U_INTERNAL_DUMP("since = %ld", since)
         U_INTERNAL_DUMP("mtime = %ld", file->st_mtime)

         if (file->st_mtime > since)
            {
            UClientImage_Base::setCloseConnection();

            U_http_info.nResponseCode = HTTP_PRECON_FAILED;

            U_RETURN(false);
            }
         }
      }
#endif

   U_RETURN(true);
}

U_NO_EXPORT void UHTTP::processGetRequest()
{
   U_TRACE_NO_PARAM(0, "UHTTP::processGetRequest()")

   U_INTERNAL_ASSERT_POINTER(file_data)
   U_INTERNAL_ASSERT_MAJOR(file_data->size, 0)
   U_INTERNAL_ASSERT_DIFFERS(file_data->fd, -1)
   U_INTERNAL_ASSERT_EQUALS(file->fd, file_data->fd)
   U_INTERNAL_ASSERT_EQUALS(file->st_size, file_data->size)

   UString mmap;
   time_t expire;
   const char* ctype;

   // NB: we check if we need to send the body with sendfile()...

   U_INTERNAL_DUMP("U_http_sendfile = %b", U_http_sendfile)

   if (U_http_sendfile)
      {
      U_INTERNAL_ASSERT(range_size >= UServer_Base::min_size_for_sendfile)

      goto sendfile;
      }

   if (file->memmap(PROT_READ, &mmap) == false) goto error;

   if (file_data == file_not_in_cache_data)
      {
      ctype      = file->getMimeType();
      expire     = 0L;
      mime_index = '9'; // NB: '9' => we declare a dynamic page to avoid 'Last-Modified: ...' in header response...
      }
   else
      {
      ctype  = setMimeIndex(0);
      expire = U_TIME_FOR_EXPIRE;
      }

   (void) ext->append(getHeaderMimeType(file->map, file->st_size, ctype, expire));

   range_size  = file->st_size;
   range_start = 0;

   U_http_info.nResponseCode = HTTP_OK;

   if (U_http_range_len &&
       checkGetRequestIfRange())
      {
      // The Range: header is used with a GET request.
      // For example assume that will return only a portion (let's say the first 32 bytes) of the requested resource...
      //
      // Range: bytes=0-31

      if (checkGetRequestForRange(mmap) != U_PARTIAL)
         {
         handlerResponse();

         return;
         }

      U_http_info.nResponseCode = HTTP_PARTIAL;

      goto build_response;
      }

   // NB: check for Flash pseudo-streaming
   // --------------------------------------------------------------------
   // Adobe Flash Player can start playing from any part of a FLV movie
   // by sending the HTTP request below ('123' is the bytes offset):
   //
   // GET /movie.flv?start=123
   //
   // HTTP servers that support Flash Player requests must send the binary 
   // FLV Header ("FLV\x1\x1\0\0\0\x9\0\0\0\x9") before the requested data
   // --------------------------------------------------------------------

   if (u_is_flv(file_data->mime_index) &&
       U_HTTP_QUERY_MEMEQ("start="))
      {
      U_INTERNAL_ASSERT_DIFFERS(U_http_info.nResponseCode, HTTP_PARTIAL)

      range_start = atol(U_http_info.query + U_CONSTANT_SIZE("start="));

      U_SRV_LOG("Request for flash pseudo-streaming: video = %.*S start = %u", U_FILE_TO_TRACE(*file), range_start);

      if (range_start >= range_size) range_start = 0;
      else
         {
         // build response...

         U_http_info.nResponseCode = HTTP_PARTIAL;

         setResponseForRange(range_start, range_size-1, U_CONSTANT_SIZE(U_FLV_HEAD));

         handlerResponse();

         (void) UClientImage_Base::wbuffer->append(U_CONSTANT_TO_PARAM(U_FLV_HEAD));

         goto next;
         }
      }

build_response:
   handlerResponse();

next:
   U_INTERNAL_DUMP("range_start = %u range_size = %u UServer_Base::min_size_for_sendfile = %u", range_start, range_size, UServer_Base::min_size_for_sendfile)

   U_ASSERT(UClientImage_Base::body->empty())

   // NB: we check if we need to send the body with sendfile()...

   if (range_size >= UServer_Base::min_size_for_sendfile)
      {
      U_http_flag |= HTTP_IS_SENDFILE;

      U_INTERNAL_DUMP("U_http_sendfile = %b", U_http_sendfile)

sendfile:
      UClientImage_Base::setSendfile(file->fd, range_start, range_size);

      return;
      }

   if (U_http_info.nResponseCode == HTTP_PARTIAL &&
       file->memmap(PROT_READ, &mmap, range_start, range_size) == false)
      {
error:
      setServiceUnavailable();

      return;
      }

   *UClientImage_Base::body = mmap;
}

// -------------------------------------------------------------------------------------------------------------------------------------
// COMMON LOG FORMAT
// -------------------------------------------------------------------------------------------------------------------------------------
// The Common Log Format, also known as the NCSA Common log format, is a standardized text file format used by web servers
// when generating server log files. Because the format is standardized, the files may be analyzed by a variety of web analysis programs.
//
// Each line in a file stored in the Common Log Format has the following syntax:
// host ident authuser date request status bytes
// -------------------------------------------------------------------------------------------------------------------------------------

#ifndef U_LOG_DISABLE
void UHTTP::initApacheLikeLog()
{
   U_TRACE_NO_PARAM(0, "UHTTP::initApacheLikeLog()")

   iov_vec[1].iov_base = (caddr_t)       " - - [";
   iov_vec[1].iov_len  = U_CONSTANT_SIZE(" - - [");
   iov_vec[2].iov_base = (caddr_t)ULog::date.date2; // %d/%b/%Y:%T %z - 21/May/2012:16:29:41 +0200 
   iov_vec[2].iov_len  = 26;
   iov_vec[3].iov_base = (caddr_t)       "] \"";
   iov_vec[3].iov_len  = U_CONSTANT_SIZE("] \"");
   // request
   iov_vec[5].iov_base = (caddr_t)iov_buffer; // response_code, body_len
   // referer
   iov_vec[7].iov_base = (caddr_t)       "\" \"";
   iov_vec[7].iov_len  = U_CONSTANT_SIZE("\" \"");
   // agent
   iov_vec[9].iov_base = (caddr_t)       "\"\n";
   iov_vec[9].iov_len  = U_CONSTANT_SIZE("\"\n");

   U_INTERNAL_DUMP("iov_vec = %p iov_vec[2] = %.*S", iov_vec, iov_vec[2].iov_len, iov_vec[2].iov_base)
}

void UHTTP::prepareApacheLikeLog()
{
   U_TRACE_NO_PARAM(0, "UHTTP::prepareApacheLikeLog()")

   U_INTERNAL_ASSERT_EQUALS(iov_vec[0].iov_len, 0)

   const char* request;
   uint32_t request_len;

#ifndef U_CACHE_REQUEST_DISABLE
     agent_offset =
   request_offset =
   referer_offset = 0;
   const char* prequest =
#endif
   request = UClientImage_Base::request->data(); 

   iov_vec[0].iov_base = (caddr_t) UServer_Base::client_address;
   iov_vec[0].iov_len  =           UServer_Base::client_address_len;

   resetApacheLikeLog();

   iov_vec[4].iov_len  = 1;
   iov_vec[4].iov_base = (caddr_t) "-";
   iov_vec[5].iov_len  = 0;

   if (U_http_info.startHeader)
      {
      U_INTERNAL_DUMP("U_http_info.startHeader = %u", U_http_info.startHeader)

      request_len = U_http_info.startHeader;

      if (u__isspace(request[request_len]) == false &&
          u__isspace(request[request_len-1]))
         {
         while (u__isspace(request[--request_len])) {}

         ++request_len;
         }
      }
   else
      {
#  ifndef U_HTTP2_DISABLE
      if (U_http_version == '2')
         {
         uint32_t sz;
         const char* ptr = UClientImage_Base::getRequestUri(sz);

         request = UClientImage_Base::cbuffer;

         request_len = u__snprintf(UClientImage_Base::cbuffer, sizeof(UClientImage_Base::cbuffer), U_CONSTANT_TO_PARAM("%.*s %.*s "), U_HTTP_METHOD_TO_TRACE, sz, ptr);

         if (request_len > (sizeof(UClientImage_Base::cbuffer) - U_CONSTANT_SIZE("HTTP/2.0"))) request_len = U_NOT_FOUND;
         else
            {
            U_MEMCPY((void*)(request + request_len), "HTTP/2.0", U_CONSTANT_SIZE("HTTP/2.0"));
            }
         }
      else
#  endif
      request_len = U_STRING_FIND(*UClientImage_Base::request, 0, "HTTP/");

      U_INTERNAL_DUMP("request_len = %u", request_len)

      if (request_len == U_NOT_FOUND) goto next;

      request_len += U_CONSTANT_SIZE("HTTP/1.1");
      }

   U_INTERNAL_DUMP("request(%u) = %.*S", request_len, request_len, request)

   while (u__isspace(*request))
      {
      ++request;
      --request_len;
      }

   if (u_isPrintable(request, U_min(1000, request_len), false))
      {
      iov_vec[4].iov_base = (caddr_t) request;
      iov_vec[4].iov_len  = U_min(1000, request_len);

#  ifndef U_CACHE_REQUEST_DISABLE
      request_offset = request - prequest;

      U_INTERNAL_DUMP("request_offset = %u", request_offset)
#  endif
      }

next:
   if (U_http_info.referer_len &&
       u_isPrintable(U_http_info.referer, U_min(1000, U_http_info.referer_len), false))
      {
      iov_vec[6].iov_base = (caddr_t) U_http_info.referer;
      iov_vec[6].iov_len  = U_min(1000, U_http_info.referer_len);

#  ifndef U_CACHE_REQUEST_DISABLE
      if (U_http_info.referer > prequest)
         {
         referer_offset = U_http_info.referer - prequest;

         U_INTERNAL_DUMP("referer_offset = %u", referer_offset)
         }
#  endif
      }

   if (U_http_info.user_agent_len &&
       u_isPrintable(U_http_info.user_agent, U_min(1000, U_http_info.user_agent_len), false))
      {
      iov_vec[8].iov_base = (caddr_t) U_http_info.user_agent;
      iov_vec[8].iov_len  = U_min(1000, U_http_info.user_agent_len);

#  ifndef U_CACHE_REQUEST_DISABLE
      if (U_http_info.user_agent > prequest)
         {
         agent_offset = U_http_info.user_agent - prequest;

         U_INTERNAL_DUMP("agent_offset = %u", agent_offset)
         }
#  endif
      }
}
#endif

// STREAM

#ifdef U_STDCPP_ENABLE
U_EXPORT istream& operator>>(istream& is, UHTTP::UFileCacheData& d)
{
   U_TRACE(0, "UFileCacheData::operator>>(%p,%p)", &is, &d)

   d.wd    =
   d.fd    = -1;
   d.ptr   = 0;     // data
   d.mode  = 0;     // file type
   d.link  = false; // true => ptr point to another entry
   d.array = 0;

   if (is.good())
      {
      int c;
      streambuf* sb = is.rdbuf();

      (void) sb->sbumpc(); // skip '{'

      do { c = sb->sbumpc(); } while (c != EOF && u__isspace(c)); // skip white-space

           if (c == '}') is.setstate(ios::badbit);
      else if (c == EOF) is.setstate(ios::eofbit);
      else if (c == '>')
         {
         UString link(200U);

         link.get(is);

         U_INTERNAL_DUMP("link = %V", link.rep)

         U_INTERNAL_ASSERT(link)

         UHTTP::UFileCacheData* ptr_file_data = UHTTP::cache_file->at(U_STRING_TO_PARAM(link));

         if (ptr_file_data)
            {
            d.link       = true;
            d.ptr        = ptr_file_data;
            d.mime_index = ptr_file_data->mime_index;
            d.size       = ptr_file_data->size;
            d.mtime      = ptr_file_data->mtime;
            d.expire     = ptr_file_data->expire;

            U_INTERNAL_DUMP("d.ptr = %p d.mime_index(%d) = %C d.size = %u", d.ptr, d.mime_index, d.mime_index, d.size)

            if (d.size >= UServer_Base::min_size_for_sendfile)
               {
               // NB: we can't use sendfile() for this...

               U_SRV_LOG("WARNING: we must change the value of MIN_SIZE_FOR_SENDFILE option (%u=>%u)", UServer_Base::min_size_for_sendfile, d.size+1);

               UServer_Base::min_size_for_sendfile = d.size+1;
               }
            }
         }
      else
         {
         sb->sputbackc(c);

         is >> d.mime_index // index file mime type
            >> d.size       // size content
            >> d.mtime      // time of last modification
            >> d.expire;    // expire time of the entry

         U_INTERNAL_DUMP("d.mime_index(%d) = %C d.size = %u", d.mime_index, d.mime_index, d.size)

         U_INTERNAL_ASSERT_MINOR(d.size, 64 * 1024)

         do { c = sb->sbumpc(); } while (c != EOF && u__isspace(c)); // skip white-space

         if (c         == '(' &&
             is.peek() != ')')
            {
            sb->sputbackc(c);

            UVector<UString> vec(4U);

            is >> vec;

            // content, header, gzip(content, header)

            if (vec.empty() == false)
               {
               U_NEW(UVector<UString>, d.array, UVector<UString>(4U));

               UString encoded, decoded;

               // content

               encoded = vec[0],
               decoded.setBuffer(d.size);

               if (d.mime_index == U_js  || // NB: sometimes we have some escape character in javascript file... (Ex: webif pageload.js)
                   d.mime_index == U_png ||
                   d.mime_index == U_gif ||
                   d.mime_index == U_jpg ||
                   d.mime_index == U_ico)
                  {
                  UBase64::decode(encoded, decoded);
                  }
               else
                  {
                  UEscape::decode(encoded, decoded);
                  }

#           ifdef DEBUG
               if (decoded.size() != d.size)
                  {
                  (void) UFile::writeToTmp(U_STRING_TO_PARAM(decoded), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("decoded.differ.%P"), 0);

                  U_INTERNAL_ASSERT_MSG(false, "decoded differ")
                  }
#           endif

               d.array->push_back(decoded);

               // header

               encoded = vec[1];
               decoded.setBuffer(encoded.size());

               UEscape::decode(encoded, decoded);

               d.array->push_back(decoded);

               encoded = vec[2];

               if (encoded)
                  {
                  // gzip(content)

                  decoded.setBuffer(encoded.size());

                  UBase64::decode(encoded, decoded);

                  d.array->push_back(decoded);

                  // gzip(header)

                  encoded = vec[3];
                  decoded.setBuffer(encoded.size());

                  UEscape::decode(encoded, decoded);

                  d.array->push_back(decoded);
                  }

               U_ASSERT(d.array->check_memory())
               }
            }
         }

      // skip '}'

      do { c = sb->sbumpc(); } while (c != '}' && c != EOF);

      if (c == EOF) is.setstate(ios::eofbit);
      }

   return is;
}

U_EXPORT ostream& operator<<(ostream& os, const UHTTP::UFileCacheData& d)
{
   U_TRACE(0, "UFileCacheData::operator<<(%p,%p)", &os, &d)

   os.put('{');

   if (d.ptr  == 0     &&
       d.link == false &&
       S_ISDIR(d.mode) == 0)
      {
      os.put(' ');
      os << d.mime_index; // index file mime type
      os.put(' ');
      os << d.size;       // size content
      os.put(' ');
      os << d.mtime;      // time of last modification
      os.put(' ');
      os << d.expire;     // expire time of the entry
      os.put(' ');
      os.put('(');

      if (d.array && // content, header, gzip(content, header)
          d.size < (64 * 1024))
         {
         U_INTERNAL_ASSERT_EQUALS(d.ptr, 0)

         UString str;
         uint32_t pos;
         char buffer[100 * 1024];

         str = d.array->at(0); // content

         pos = (d.mime_index == U_js  || // NB: sometimes we have some escape character in javascript file... (Ex: webif pageload.js)
                d.mime_index == U_png ||
                d.mime_index == U_gif ||
                d.mime_index == U_jpg ||
                d.mime_index == U_ico
                     ? u_base64_encode((const unsigned char*)U_STRING_TO_PARAM(str), (unsigned char*)buffer)
                     : u_escape_encode((const unsigned char*)U_STRING_TO_PARAM(str),                 buffer, sizeof(buffer)));

         os.put('\n');
         os.write(buffer, pos);
         os.put('\n');

         str = d.array->at(1); // header

         pos = u_escape_encode((const unsigned char*)U_STRING_TO_PARAM(str), buffer, sizeof(buffer));

         os.put('\n');
         os.write(buffer, pos);
         os.put('\n');

         if (d.array->size() == 2) os.write(U_CONSTANT_TO_PARAM("\n\"\"\n\"\"\n"));
         else
            {
            str = d.array->at(2); // gzip(content)

            pos = u_base64_encode((const unsigned char*)U_STRING_TO_PARAM(str), (unsigned char*)buffer);

            os.put('\n');
            os.write(buffer, pos);
            os.put('\n');

            str = d.array->at(3); // gzip(header)

            pos = u_escape_encode((const unsigned char*)U_STRING_TO_PARAM(str), buffer, sizeof(buffer));

            os.put('\n');
            os.write(buffer, pos);
            os.put('\n');
            }
         }

      os.put(')');
      }

   os.put(' ');
   os.put('}');
   os.put('\n');

   return os;
}

// DEBUG

#  ifdef DEBUG
U_EXPORT const char* UHTTP::UServletPage::dump(bool reset) const
{
   UDynamic::dump(false);

   *UObjectIO::os << '\n'
                  << "runDynamicPage" << (void*)runDynamicPage << '\n';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

U_EXPORT const char* UHTTP::UCServletPage::dump(bool reset) const
{
   *UObjectIO::os << "size      " << size             << '\n'
                  << "relocated " << (void*)relocated << '\n'
                  << "prog_main " << (void*)prog_main << '\n';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

U_EXPORT const char* UHTTP::UFileCacheData::dump(bool reset) const
{
   *UObjectIO::os << "wd                      " << wd            << '\n'
                  << "size                    " << size          << '\n'
                  << "mode                    " << mode          << '\n'
                  << "expire                  " << expire        << '\n'
                  << "mtime                   " << mtime         << '\n'
                  << "array (UVector<UString> " << (void*)&array << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

#  ifdef USE_PAGE_SPEED
U_EXPORT const char* UHTTP::UPageSpeed::dump(bool reset) const
{
   UDynamic::dump(false);

   *UObjectIO::os << '\n'
                  << "minify_html   " << (void*)minify_html  << '\n'
                  << "optimize_gif  " << (void*)optimize_gif << '\n'
                  << "optimize_png  " << (void*)optimize_png << '\n'
                  << "optimize_jpg  " << (void*)optimize_jpg << '\n';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#  ifdef USE_LIBV8
U_EXPORT const char* UHTTP::UV8JavaScript::dump(bool reset) const
{
   UDynamic::dump(false);

   *UObjectIO::os << '\n'
                  << "runv8         " << (void*)runv8  << '\n';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#  ifdef USE_RUBY
U_EXPORT const char* UHTTP::URUBY::dump(bool reset) const
{
   UDynamic::dump(false);

   *UObjectIO::os << '\n'
                  << "runRUBY              " << (void*)runRUBY     << '\n'
                  << "ruby_on_rails        " << ruby_on_rails      << '\n'
                  << "ruby_libdir (UString " << (void*)ruby_libdir << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#  ifdef USE_PYTHON
U_EXPORT const char* UHTTP::UPYTHON::dump(bool reset) const
{
   UDynamic::dump(false);

   *UObjectIO::os << '\n'
                  << "runPYTHON                   " << (void*)runPYTHON          << '\n'
                  << "py_project_app     (UString " << (void*)py_project_app     << ")\n"
                  << "py_project_root    (UString " << (void*)py_project_root    << ")\n"
                  << "py_virtualenv_path (UString " << (void*)py_virtualenv_path << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#  ifdef USE_PHP
U_EXPORT const char* UHTTP::UPHP::dump(bool reset) const
{
   UDynamic::dump(false);

   *UObjectIO::os << '\n'
                  << "runPHP        " << (void*)runPHP << '\n';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#if defined(U_ALIAS) && defined(USE_LIBPCRE) // REWRITE RULE
U_EXPORT const char* UHTTP::RewriteRule::dump(bool reset) const
{
   *UObjectIO::os << "key         (UPCRE   " << (void*)&key         << ")\n"
                  << "replacement (UString " << (void*)&replacement << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#  endif
#endif
