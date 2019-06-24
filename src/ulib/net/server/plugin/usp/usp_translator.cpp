// =======================================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    usp_translator.cpp - the translator .usp => .cpp for dynamic page for UServer
//
// = AUTHOR
//    Stefano Casazza
//
// =======================================================================================

#include <ulib/tokenizer.h>
#include <ulib/net/socket.h>
#include <ulib/file_config.h>
#include <ulib/utility/escape.h>
#include <ulib/utility/string_ext.h>

#include <limits.h>

#undef  PACKAGE
#define PACKAGE "usp_translator"
#undef  ARGS
#define ARGS "[filename usp]"

#define U_PURPOSE "program for dynamic page translation ([xxx].usp => [xxx].cpp)"
#define U_OPTIONS \
"option I include 1 \"path of local include directory\" \"\"\n"

#include <ulib/application.h>

#ifdef USE_FSTACK
#  include <ff_api.h>
#  include <ff_epoll.h>

extern "C" {
extern U_EXPORT int ff_epoll_create(int size)                                                                          { return 0; /* epoll_create(size); */ }
extern U_EXPORT int ff_epoll_ctl(int epfd, int op, int fd, struct epoll_event* event)                                  { return 0; /* epoll_ctl(epfd, op, fd, event); */ }
extern U_EXPORT int ff_epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)                    { return 0; /* epoll_wait(epfd, events, maxevents, timeout); */ }
extern U_EXPORT int ff_close(int fd)                                                                                   { return 0; /* close(fd); */ }
extern U_EXPORT int ff_shutdown(int s, int how)                                                                        { return 0; /* shutdown(s, how); */ }
extern U_EXPORT int ff_listen(int s, int backlog)                                                                      { return 0; /* listen(s, backlog); */ }
extern U_EXPORT int ff_socket(int domain, int type, int protocol)                                                      { return 0; /* socket(domain, type, protocol); */ }
extern U_EXPORT int ff_poll(struct pollfd fds[], nfds_t nfds, int timeout)                                             { return 0; /* poll(fds, nfds, timeout); */ }
extern U_EXPORT int ff_accept(int s, struct linux_sockaddr* addr, socklen_t* addrlen)                                  { return 0; /* accept(s, (sockaddr*)addr, addrlen); */ }
extern U_EXPORT int ff_bind(int s, const struct linux_sockaddr* addr, socklen_t addrlen)                               { return 0; /* bind(s, (sockaddr*)addr, addrlen); */ }
extern U_EXPORT int ff_getpeername(int s, struct linux_sockaddr* name, socklen_t* namelen)                             { return 0; /* getpeername(s, (sockaddr*)name, namelen); */ }
extern U_EXPORT int ff_getsockname(int s, struct linux_sockaddr* name, socklen_t* namelen)                             { return 0; /* getsockname(s, (sockaddr*)name, namelen); */ }
extern U_EXPORT int ff_connect(int s, const struct linux_sockaddr* name, socklen_t namelen)                            { return 0; /* connect(s, (sockaddr*)name, namelen); */ }
extern U_EXPORT int ff_getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen)                      { return 0; /* getsockopt(s, level, optname, optval, optlen); */ }
extern U_EXPORT int ff_setsockopt(int s, int level, int optname, const void* optval, socklen_t optlen)                 { return 0; /* setsockopt(s, level, optname, optval, optlen); */ }
extern U_EXPORT int ff_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout) { return 0; /* select(nfds, readfds, writefds, exceptfds, timeout); */ }

extern U_EXPORT ssize_t ff_recv(int s, void* buf, size_t len, int flags)       { return 0; /* recv(s, buf, len, flags); */ }
extern U_EXPORT ssize_t ff_recvmsg(int s, struct msghdr* msg, int flags)       { return 0; /* recvmsg(s, msg, flags); */ }
extern U_EXPORT ssize_t ff_write(int fd, const void* buf, size_t nbytes)       { return 0; /* write(fd, buf, nbytes); */ }
extern U_EXPORT ssize_t ff_writev(int fd, const struct iovec* iov, int iovcnt) { return 0; /* writev(fd, iov, iovcnt); */ }
extern U_EXPORT ssize_t ff_send(int s, const void* buf, size_t len, int flags) { return 0; /* send(s, buf, len, flags); */ }

extern U_EXPORT ssize_t ff_recvfrom(int s, void* buf, size_t len, int flags, struct linux_sockaddr* from, socklen_t* fromlen)
{ return 0; /* recvfrom(s, buf, len, flags, (sockaddr*)from, fromlen); */ }

extern U_EXPORT ssize_t ff_sendto(int s, const void* buf, size_t len, int flags, const struct linux_sockaddr* to, socklen_t tolen)
{ return 0; /* sendto(s, buf, len, flags, (sockaddr*)to, tolen); */ }

extern U_EXPORT int ff_fcntl(int fd, int cmd, ...)               { return 0; /* fcntl(fd, cmd, argp); */ }
extern U_EXPORT int ff_ioctl(int fd, unsigned long request, ...) { return 0; /* ioctl(fd, request, argp); */ }
}
#endif

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE_NO_PARAM(5, "Application::Application()")

      bvar                  = 
      breturn               =
      bsession              =
      bstorage              =
      bfirst_pass           =
           is_html          =
      test_if_html          =
      bhttp_header_empty    =
      bpreprocessing_failed = false;
      }

   ~Application()
      {
      U_TRACE_NO_PARAM(5, "Application::~Application()")
      }

   void setDirectiveItem(const char* directive, uint32_t tag_len)
      {
      U_TRACE(5, "Application::setDirectiveItem(%.10S,%u)", directive, tag_len)

      uint32_t n = token.size() - tag_len - 2;

      token = UStringExt::trim(directive+tag_len, n);

      U_INTERNAL_DUMP("token = %V", token.rep)
      }

   void manageDirectiveSessionOrStorage(const char* name, uint32_t name_len)
      {
      U_TRACE(5, "Application::manageDirectiveSessionOrStorage(%.*S,%u)", name_len, name, name_len)

      (void) output0.reserve(500U);
      (void) output1.reserve(500U);

      output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tif (usp_b%.*s) {\n"), name_len, name);
      output1.snprintf_add(U_CONSTANT_TO_PARAM("\n\tif (usp_b%.*s) {\n"), name_len, name);

      if (token.empty()) output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUHTTP::putData%.*s();\n"), name_len, name);
      else
         {
         UString id, tmp;
         const char* ptr;
         uint32_t pos, size;

         UVector<UString> vec(token, "\t\n;");

         bvar = true;

         for (uint32_t i = 0, n = vec.size(); i < n; ++i)
            {
            ptr = (tmp = UStringExt::trim(vec[i])).data();

            do { ++ptr; } while (u__isspace(*ptr) == false);
            do { ++ptr; } while (u__isspace(*ptr) == true);

             pos = (id = tmp.substr(tmp.distance(ptr))).find('(');
            size = (pos == U_NOT_FOUND ? id.size() : pos);

            (void) output0.reserve(50U + size);
            (void) output1.reserve(50U + size);

            output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUSP_%.*s_VAR_GET(%u,%.*s);\n"), name_len, name, i, size, ptr);
            output1.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUSP_%.*s_VAR_PUT(%u,%.*s);\n"), name_len, name, i, size, ptr);

            (void) static_vars.reserve(50U + size);

            static_vars.snprintf_add(U_CONSTANT_TO_PARAM("static %v;\n"), tmp.rep);

#        ifdef DEBUG
            id.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#        endif
            }
         }

      output0.snprintf_add(U_CONSTANT_TO_PARAM(                         "\n\t}\n\tusp_b%.*s = true;\n"),                 name_len, name);
      output1.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUHTTP::putData%.*s();\n\t}\n\tusp_b%.*s = true;\n"), name_len, name, name_len, name);
      }

   void manageDirectiveArgsOrCpath(const char* name, bool binc)
      {
      U_TRACE(5, "Application::manageDirectiveArgsOrCpath(%S,%b)", name, binc)

      U_INTERNAL_ASSERT(token)

      uint32_t pos;
      UString id, tmp;
      UVector<UString> vec(token, "\t\n;");

      (void) output0.reserve(500U + token.size());

      for (uint32_t i = 0, n = vec.size(); i < n; ++i)
         {
         tmp = UStringExt::trim(vec[i]);
         pos = tmp.find('(');
          id = (pos == U_NOT_FOUND ? tmp : tmp.substr(0U, pos));

         if (id.first_char() != '&') output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUString %v = %s(%u);\n"), id.rep, name, i+binc);
         else                        output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t%.*s = %s(%u);\n"), id.size()-1, id.c_pointer(1), name, i+binc);

         if (pos != U_NOT_FOUND)
            {
            uint32_t sz = tmp.size()-pos-2;
            const char* ptr = tmp.c_pointer(pos+1);
            const char* quote = (*ptr == '"' ? "" : "\"");

            U_INTERNAL_ASSERT_MAJOR(sz, 0)

            output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tif (%v.empty()) %v = U_STRING_FROM_CONSTANT(%s%.*s%s);\n"), id.rep, id.rep, quote, sz, ptr, quote);
            }

#     ifdef DEBUG
         id.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#     endif
         }
      }

   void processDirective()
      {
      U_TRACE_NO_PARAM(5, "Application::processDirective()")

      /**
       * token list:
       *
       * <!--# ... --> (comment)
       * <!--#define ... -->
       * <!--#include ... -->
       * <!--#declaration ... -->
       * <!--#login ... -->
       * <!--#session ... -->
       * <!--#storage ... -->
       * <!--#args ... -->
       * <!--#cpath ... -->
       * <!--#header ... -->
       * <!--#code ... -->
       * <!--#vcode ... -->
       * <!--#pcode ... -->
       * <!--#lcode ... -->
       * <!--#sse ... -->
       * <!--#number ... -->
       * <!--#puts ... -->
       * <!--#xmlputs ... -->
       * <!--#cout ... -->
       * <!--#print ... -->
       * <!--#printfor ... -->
       */

      U_INTERNAL_DUMP("token = %V", token.rep)

      const char* directive = token.c_pointer(2); // "-#"...

      U_INTERNAL_DUMP("directive(10) = %.10S", directive)

      if (bfirst_pass)
         {
         bool bdefine  = (strncmp(directive, U_CONSTANT_TO_PARAM("define"))  == 0),
              binclude = (strncmp(directive, U_CONSTANT_TO_PARAM("include")) == 0);

         U_INTERNAL_DUMP("bdefine = %b binclude = %b", bdefine, binclude)

         if (bdefine ||
             binclude)
            {
            const char* ptr = usp.data();
            UString content, x(U_CAPACITY);
            uint32_t pos2 = t.getDistance(),
                     pos1 = pos2 - token.size() - U_CONSTANT_SIZE("<!--->");

            U_INTERNAL_DUMP("pos1(%u) = %.10S pos2(%u) = %.10S", pos1, usp.c_pointer(pos1), pos2, t.getPointer())

            U_INTERNAL_ASSERT_MAJOR(pos2, 0)

            if (pos1) (void) x.append(ptr, pos1);

            ptr += pos2;

            if (bdefine)
               {
               setDirectiveItem(directive, U_CONSTANT_SIZE("define"));

               U_INTERNAL_ASSERT(token)

               const char* ptr1 = token.data();

               do { ++ptr1; } while (u__isspace(*ptr1) == false);

               uint32_t pos = token.distance(ptr1);

               UString    id(token, 0, pos),
                       value(token,    pos);

               vdefine.push_back(id);
               vdefine.push_back(value);
               }
            else
               {
               setDirectiveItem(directive, U_CONSTANT_SIZE("include"));

               U_INTERNAL_ASSERT(token)

               content = UFile::contentOf(token, pinclude);

               if (content.empty())
                  {
                  U_ERROR("load of include usp %V failed: path = %V", token.rep, pinclude.rep);
                  }

               (void) x.append(content);
               }

            (void) x.append(ptr, usp.remain(ptr));

#        ifdef DEBUG
            token.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#        endif

            t.setData((usp = x));

            t.setDistance(pos1);
            }

         return;
         }

      if (strncmp(directive, U_CONSTANT_TO_PARAM("declaration")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_ASSERT(declaration.empty()) // NB: <!--#declaration ... --> must be at the beginning and uniq...
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("declaration"));

         if (token) declaration = token;
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("login")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         (void) output0.append(U_CONSTANT_TO_PARAM(
               "\n\tU_http_info.endHeader = 0;"
               "\n\tif (UHTTP::getLoginCookie() ||"
               "\n\t    UHTTP::getPostLoginUserPasswd())"
               "\n\t\t{"
               "\n\t\tUHTTP::usp->runDynamicPageParam(U_DPAGE_AUTH);"
               "\n\t\t}"
               "\n\tif (UHTTP::loginCookie->empty())"
               "\n\t\t{"
               "\n\t\tUHTTP::UServletPage* usp_save = UHTTP::usp;"
               "\n\t\tif (UHTTP::getUSP(U_CONSTANT_TO_PARAM(\"login_form\")))"
               "\n\t\t\t{"
               "\n\t\t\tUHTTP::usp->runDynamicPageParam(U_DPAGE_AUTH);"
               "\n\t\t\tUHTTP::usp = usp_save;"
               "\n\t\t\treturn;"
               "\n\t\t\t}"
               "\n\t\tUHTTP::UFileCacheData* login_form_html = UHTTP::getFileCachePointer(U_CONSTANT_TO_PARAM(\"login_form.html\"));"
               "\n\t\tif (login_form_html)"
               "\n\t\t\t{"
               "\n\t\t\tUHTTP::setResponseFromFileCache(login_form_html);"
               "\n\t\t\tU_http_info.nResponseCode = HTTP_NO_CONTENT; // NB: to escape management after usp exit..."
               "\n\t\t\treturn;"
               "\n\t\t\t}"
               "\n\t\t(void) UClientImage_Base::wbuffer->append(U_CONSTANT_TO_PARAM("
               "\n\"<form method=\\\"post\\\">\\n\""
               "\n\" <p>Login</p>\\n\""
               "\n\" <p>username <input name=\\\"user\\\" type=\\\"text\\\" class=\\\"inputbox\\\" title=\\\"Enter your username\\\"></p>\\n\""
               "\n\" <p>password <input name=\\\"pass\\\" type=\\\"text\\\" class=\\\"inputbox\\\" title=\\\"Enter your password\\\"></p>\\n\""
               "\n\" <p><input type=\\\"submit\\\" value=\\\"login\\\"></p>\\n\""
               "\n\"</form>\"));"
               "\n\t\treturn;"
               "\n\t\t}\n\t\t"));
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("session")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         bsession = true;

         (void) output0.append(U_CONSTANT_TO_PARAM("\n\tusp_bSESSION = (UHTTP::getDataSession() ? true : (UHTTP::setSessionCookie(), false));\n"));

         setDirectiveItem(directive, U_CONSTANT_SIZE("session"));

         manageDirectiveSessionOrStorage(U_CONSTANT_TO_PARAM("SESSION"));
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("storage")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         bstorage = true;

         (void) output0.append(U_CONSTANT_TO_PARAM("\n\tusp_bSTORAGE = true;\n\t(void) UHTTP::getDataStorage();\n"));

         setDirectiveItem(directive, U_CONSTANT_SIZE("storage"));

         manageDirectiveSessionOrStorage(U_CONSTANT_TO_PARAM("STORAGE"));
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("args")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("args"));

         if (token &&
             token.first_char() == ':')
            {
            char buffer[128];
            const char* data = token.c_pointer(1);
            uint32_t sz = (*data == 'G' ? 3 : 4); // 3 => GET | 4 => POST

            (void) output0.append(buffer, u__snprintf(buffer, U_CONSTANT_SIZE(buffer), U_CONSTANT_TO_PARAM("\t\n\tif (UHTTP::is%.*s()) (void) UHTTP::processForm();\n"), sz, data));

            (void) token.erase(0, 1+sz+1);
            }
         else
            {
            (void) output0.append(U_CONSTANT_TO_PARAM("\t\n\tif (UHTTP::isGETorPOST()) (void) UHTTP::processForm();\n"));
            }

         if (token) manageDirectiveArgsOrCpath("USP_FORM_VALUE", false);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("cpath")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("cpath"));

         manageDirectiveArgsOrCpath("UHTTP::getPathComponent", true);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("vcode")) == 0) // validation code
         {
         U_ASSERT(vcode.empty()) // NB: <!--#vcode ... --> must be before other code and uniq...
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("vcode"));

         if (token)
            {
            token = UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t"));

            vcode.setBuffer(20U + token.size());

            vcode.snprintf(U_CONSTANT_TO_PARAM("\n\t%v\n\t\n"), token.rep);
            }
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("pcode")) == 0) // parallelization code (long running task)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("pcode"));

         if (token)
            {
            token = UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t"));

            (void) output0.reserve(20U + token.size());

            output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t%v\n\t\n"), token.rep);

            (void) vcode.append(U_CONSTANT_TO_PARAM("\tif (UServer_Base::startParallelization()) { U_http_info.nResponseCode = HTTP_CONTINUE; return; }\n\t\n"));
            }
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("lcode")) == 0) // load balance code
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("lcode"));

         if (token)
            {
            token = UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t"));

            (void) output0.reserve(20U + token.size());

            output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t%v\n\t\n"), token.rep);

            (void) vcode.append(U_CONSTANT_TO_PARAM("\tif (UServer_Base::startParallelization()) { U_http_info.nResponseCode = HTTP_CONTINUE; return; }\n\t\n"));

#        ifdef USE_LOAD_BALANCE
            (void) vcode.append(U_CONSTANT_TO_PARAM("\tif (UHTTP::manageRequestOnRemoteServer()) return;\n\t\n"));
#        endif
            }
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("code")) == 0) // generic code
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("code"));

         if (token)
            {
            if (breturn == false) breturn = (U_STRING_FIND(token, 0, "return") != U_NOT_FOUND);

            token = UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t"));

            (void) output0.reserve(20U + token.size());

            output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t%v\n\t\n"), token.rep);
            }
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("sse")) == 0) // SSE code
         {
         U_ASSERT(sseloop.empty())
         U_ASSERT(http_header.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         (void) sseloop.reserve(100U);
         (void) http_header.reserve(300U + basename_sz + basename_sz);

         (void) http_header.append(U_CONSTANT_TO_PARAM(
               "\n\tif (U_http_accept_len != U_CONSTANT_SIZE(\"text/event-stream\") ||"
               "\n\t    u_get_unalignedp64(U_http_info.accept) != U_MULTICHAR_CONSTANT64('t','e','x','t','/','e','v','e'))"
               "\n\t\t{"
               "\n\t\tUHTTP::setBadRequest();"
               "\n\t\treturn;"
               "\n\t\t}"));

         setDirectiveItem(directive, U_CONSTANT_SIZE("sse"));

         if (token.empty())
            {
            (void) declaration.reserve(100U + basename_sz);

            declaration.snprintf(U_CONSTANT_TO_PARAM("\n#ifdef USE_LIBSSL\nstatic UString sse_%.*s() { return UString::getStringNull(); }\n#endif"), basename_sz, basename_ptr);

            (void) sseloop.append(U_CONSTANT_TO_PARAM("\n\tif (param == U_DPAGE_SSE) { UHTTP::readSSE(-1); return; }\n"));

            http_header.snprintf(U_CONSTANT_TO_PARAM(
                  "\n#ifdef USE_LIBSSL"
                  "\n\tif (UServer_Base::bssl) UHTTP::sse_func = sse_%.*s;"
                  "\n\telse"
                  "\n#endif"
                  "\n\tUHTTP::sse_func = (UHTTP::strPF)(void*)1L;"), basename_sz, basename_ptr);
            }
         else
            {
            const char* data = token.data();
            uint32_t sz = token.size(), sse_timeout = 1000;

            if (*data == ':')
               {
               ++data;

               sse_timeout *= u_strtoulp(&data);

               sz = token.remain(data);
               }

            UString sse_code = UStringExt::substitute(data, sz, '\n', U_CONSTANT_TO_PARAM("\n\t"));

            (void) declaration.reserve(100U + basename_sz + basename_sz + sse_code.size());

            declaration.snprintf_add(U_CONSTANT_TO_PARAM(
                  "\nstatic UString sse_%.*s()"
                  "\n{"
                  "\n\tU_TRACE(5, \"::sse_%.*s()\")"
                  "\n\tUString sse_data;"
                  "\n\t%v"
                  "\n\tU_RETURN_STRING(sse_data);"
                  "\n}"),
                  basename_sz, basename_ptr,
                  basename_sz, basename_ptr, sse_code.rep);

            sseloop.snprintf(U_CONSTANT_TO_PARAM("\n\tif (param == U_DPAGE_SSE) { UHTTP::readSSE(%u); return; }\n"), sse_timeout);

            http_header.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUHTTP::sse_func = sse_%.*s;\n"), basename_sz, basename_ptr);
            }
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("header")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_ASSERT(http_header.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("header"));

         (void) output1.reserve(200U);

         if (token.empty())
            {
            bhttp_header_empty = true;

#        if !defined(U_SERVER_CAPTIVE_PORTAL) || !defined(ENABLE_THREAD) || !defined(U_CACHE_REQUEST_DISABLE)
            (void) output1.append(U_CONSTANT_TO_PARAM("\n\tU_http_info.endHeader = U_NOT_FOUND;\n"));
#        endif
            }
         else
            {
            // NB: we use insert because the possibility of UHTTP::callService() (see chat.usp)...

            bool bheader = (U_STRING_FIND(token, 0, "Content-Type") != U_NOT_FOUND);

            http_header = UStringExt::dos2unix(token, true);

            (void) http_header.append(U_CONSTANT_TO_PARAM("\r\n\r\n"));

            uint32_t n = http_header.size();

            UString encoded(n * 4);

            UEscape::encode(http_header, encoded);

            U_ASSERT(encoded.isQuoted())

            (void) http_header.reserve(200U + encoded.size());

            (void) http_header.snprintf(U_CONSTANT_TO_PARAM("\n\tU_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false)"
                                                            "\n\t(void) UClientImage_Base::wbuffer->insert(0, U_CONSTANT_TO_PARAM(%v));\n\t\n"), encoded.rep);

            (void) output1.snprintf_add(U_CONSTANT_TO_PARAM("\n\tU_http_info.endHeader = %.*s%u;\n"), bheader ? U_CONSTANT_SIZE("(uint32_t)-") : 0, "(uint32_t)-", n);
            }
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("number")) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("number"));

         U_INTERNAL_ASSERT(token)

         (void) output0.reserve(100U + token.size());

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUStringExt::appendNumber32(*UClientImage_Base::wbuffer, (%v));\n"), token.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("puts")) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("puts"));

         if (token)
            {
            (void) output0.reserve(100U + token.size());

            output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tif (%v) (void) UClientImage_Base::wbuffer->append((%v));\n"), token.rep, token.rep);
            }
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("xmlputs")) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("xmlputs"));

         if (token)
            {
            (void) output0.reserve(100U + token.size());

            output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUSP_XML_PUTS((%v));\n"), token.rep);
            }
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("cout")) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         bvar = true;

         setDirectiveItem(directive, U_CONSTANT_SIZE("cout"));

         if (token)
            {
            (void) output0.reserve(200U + token.size());

            output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tusp_sz = UObject2String((%v), usp_buffer, sizeof(usp_buffer));"
                                                     "\n\t(void) UClientImage_Base::wbuffer->append(usp_buffer, usp_sz);\n"), token.rep);
            }
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("print")) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         bvar = true;

         bool bfor = (strncmp(directive + U_CONSTANT_SIZE("print"), U_CONSTANT_TO_PARAM("for")) == 0);

         setDirectiveItem(directive, (bfor ? U_CONSTANT_SIZE("printfor") : U_CONSTANT_SIZE("print")));

         if (token)
            {
            (void) output0.reserve(200U + token.size());

            UVector<UString> vec(token, ';');

            if (bfor)
               {
               U_ASSERT_EQUALS(vec.size(), 5)

               output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tfor (%v; %v; %v) { usp_sz = u__snprintf(usp_buffer, sizeof(usp_buffer), %v, %v);"
                                                        "(void) UClientImage_Base::wbuffer->append(usp_buffer, usp_sz); }\n"),
                                                        vec[0].rep, vec[1].rep, vec[2].rep, vec[3].rep, vec[4].rep);
               }
            else
               {
               U_ASSERT_EQUALS(vec.size(), 2)

               output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tusp_sz = u__snprintf(usp_buffer, sizeof(usp_buffer), %v, %v);"
                                                        "(void) UClientImage_Base::wbuffer->append(usp_buffer, usp_sz);\n"), vec[0].rep, vec[1].rep);
               }
            }
         }
      }

   void processUSP()
      {
      U_TRACE_NO_PARAM(5, "Application::processUSP()")

      // NB: Anything that is not enclosed in <!-- ... --> tags is assumed to be HTML

      bool bgroup;
      const char* ptr;
      uint32_t distance, pos, size;

      t.setData(usp);

loop: distance = t.getDistance();

      pos = U_STRING_FIND(usp, distance, "<!--#");

      if (pos != U_NOT_FOUND) t.setDistance(pos);
      else
         {
         pos = usp.size();

         t.setDistance(pos);

         while (usp.c_char(pos-1) == '\n') --pos; // no trailing \n...
         }

      if (bfirst_pass == false)
         {
         size = (pos > distance ? pos - distance : 0);

         U_INTERNAL_DUMP("size = %u", size)

         if (size)
            {
            token = usp.substr(distance, size);
     
            if (token.isWhiteSpace() == false)
               {
               // plain html block

               if (test_if_html == false)
                  {
                  test_if_html = true;

                  if (u_isHTML(token.data())) is_html = true;
                  }

               UString tmp(token.size() * 4);

               UEscape::encode(token, tmp);

               U_ASSERT(tmp.isQuoted())

               (void) output0.reserve(100U + tmp.size());

               output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t(void) UClientImage_Base::wbuffer->append(\n\t\tU_CONSTANT_TO_PARAM(%v)\n\t);\n"), tmp.rep);
               }
            }
         }

      if (t.next(token, &bgroup))
         {
         U_INTERNAL_ASSERT(bgroup)

         processDirective();

         if (usp &&
             t.atEnd() == false)
            {
            // no trailing \n...

            for (ptr = t.getPointer(); u__islterm(*ptr); ++ptr) {}

            if (ptr > t.getEnd()) ptr = t.getEnd();

            t.setPointer(ptr);

            goto loop;
            }
         }
      }

   bool execPreProcessing()
      {
      U_TRACE_NO_PARAM(5, "Application::execPreProcessing()")

      if (U_STRING_FIND(usp, 0, "\n#ifdef DEBUG") != U_NOT_FOUND)
         {
         UFileConfig cfg(UStringExt::substitute(usp, U_CONSTANT_TO_PARAM("#include"), U_CONSTANT_TO_PARAM("//#include")), true);

         if (cfg.processData(false)) usp = UStringExt::substitute(cfg.getData(), U_CONSTANT_TO_PARAM("//#include"), U_CONSTANT_TO_PARAM("#include"));
         else
            {
            bpreprocessing_failed = true;

            U_RETURN(false);
            }
         }

      U_RETURN(true);
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage options

      if (UApplication::isOptions()) pinclude = opt['I'];

      // manage arg

      if (argv[optind] == U_NULLPTR) U_ERROR("filename usp not specified");

      UString filename(argv[optind]);

      usp = UFile::contentOf(filename);

      if (usp.empty()) U_ERROR("load of %V failed", filename.rep);

      t.setGroup(U_CONSTANT_TO_PARAM("<!--->"));

      bfirst_pass = true; // NB: we check for <!--#define ... --> and <!--#include ... -->

      UString basename = UStringExt::basename(filename);

      basename_sz  = basename.size() - U_CONSTANT_SIZE(".usp");
      basename_ptr = basename.data();

      processUSP();

#  ifdef DEBUG
      token.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#  endif

      if (vdefine.empty() == false) usp = UStringExt::substitute(usp, vdefine);

      usp = UStringExt::eraseIds(usp);

      if (execPreProcessing() == false) U_WARNING("preprocessing of %V failed", filename.rep);

      bfirst_pass = false;

      processUSP();

      U_INTERNAL_DUMP("static_vars = %V declaration = %V", static_vars.rep, declaration.rep)

      /**
       * Server-wide hooks
       *
       * enum DynamicPageType {
       * U_DPAGE_SSE     = 0,
       * U_DPAGE_CONFIG  = 1,
       * U_DPAGE_INIT    = 2,
       * U_DPAGE_RESET   = 3,
       * U_DPAGE_DESTROY = 4,
       * U_DPAGE_SIGHUP  = 5,
       * U_DPAGE_FORK    = 6,
       * U_DPAGE_OPEN    = 7,
       * U_DPAGE_CLOSE   = 8,
       * U_DPAGE_ERROR   = 9,
       * U_DPAGE_AUTH    = 10 };
       */

      bool bcfg,    // usp_config
           binit,   // usp_init
           breset,  // usp_reset
           bend,    // usp_end
           bsighup, // usp_sighup
           bfork,   // usp_fork
           bopen,   // usp_open
           bclose,  // usp_close
           berror,  // usp_error
           bauth;   // usp_auth

      char ptr1[100]  = { '\0' };
      char ptr2[100]  = { '\0' };
      char ptr3[100]  = { '\0' };
      char ptr4[100]  = { '\0' };
      char ptr5[100]  = { '\0' };
      char ptr6[100]  = { '\0' };
      char ptr7[100]  = { '\0' };
      char ptr8[100]  = { '\0' };
      char ptr9[100]  = { '\0' };
      char ptr10[100] = { '\0' };

#  ifndef U_CACHE_REQUEST_DISABLE
      if (usp.c_char(4) == '#'      &&
          u__isspace(usp.c_char(5)) &&
          u_get_unalignedp32(usp.data()) == U_MULTICHAR_CONSTANT32('<','!','-','-')) // <!--# --> (comment)
         {
         (void) output1.append(U_CONSTANT_TO_PARAM("\n\tUClientImage_Base::setRequestNoCache();\n\t\n"));
         }
#  endif

      if (declaration)
         {
         bcfg    = (U_STRING_FIND(declaration, 0, "static void usp_config_") != U_NOT_FOUND);
         binit   = (U_STRING_FIND(declaration, 0, "static void usp_init_")   != U_NOT_FOUND);
         breset  = (U_STRING_FIND(declaration, 0, "static void usp_reset_")  != U_NOT_FOUND);
         bend    = (U_STRING_FIND(declaration, 0, "static void usp_end_")    != U_NOT_FOUND);
         bsighup = (U_STRING_FIND(declaration, 0, "static void usp_sighup_") != U_NOT_FOUND);
         bfork   = (U_STRING_FIND(declaration, 0, "static void usp_fork_")   != U_NOT_FOUND);
         bopen   = (U_STRING_FIND(declaration, 0, "static void usp_open_")   != U_NOT_FOUND);
         bclose  = (U_STRING_FIND(declaration, 0, "static void usp_close_")  != U_NOT_FOUND);
         berror  = (U_STRING_FIND(declaration, 0, "static void usp_error_")  != U_NOT_FOUND);
         bauth   = (U_STRING_FIND(declaration, 0, "static void usp_auth_")   != U_NOT_FOUND);

         if (breset) (void) u__snprintf(ptr2, 100, U_CONSTANT_TO_PARAM("\n\tif (param == U_DPAGE_RESET) { usp_reset_%.*s(); return; }\n"), basename_sz, basename_ptr);

         if (bend)
            {
#        ifndef DEBUG
            if (bpreprocessing_failed) bend = false;
            else
#        endif
            (void) u__snprintf(ptr3, 100, U_CONSTANT_TO_PARAM("\n\tif (param == U_DPAGE_DESTROY) { usp_end_%.*s(); return; }\n"), basename_sz, basename_ptr);
            }

         if (bsighup) (void) u__snprintf(ptr4,  100, U_CONSTANT_TO_PARAM("\n\tif (param == U_DPAGE_SIGHUP) { usp_sighup_%.*s(); return; }\n"), basename_sz, basename_ptr);
         if (bfork)   (void) u__snprintf(ptr5,  100, U_CONSTANT_TO_PARAM("\n\tif (param == U_DPAGE_FORK) { usp_fork_%.*s(); return; }\n"),     basename_sz, basename_ptr);
         if (bopen)   (void) u__snprintf(ptr6,  100, U_CONSTANT_TO_PARAM("\n\tif (param == U_DPAGE_OPEN) { usp_open_%.*s(); return; }\n"),     basename_sz, basename_ptr);
         if (bclose)  (void) u__snprintf(ptr7,  100, U_CONSTANT_TO_PARAM("\n\tif (param == U_DPAGE_CLOSE) { usp_close_%.*s(); return; }\n"),   basename_sz, basename_ptr);
         if (berror)  (void) u__snprintf(ptr8,  100, U_CONSTANT_TO_PARAM("\n\tif (param == U_DPAGE_ERROR) { usp_error_%.*s(); return; }\n"),   basename_sz, basename_ptr);
         if (bcfg)    (void) u__snprintf(ptr9,  100, U_CONSTANT_TO_PARAM("\n\tif (param == U_DPAGE_CONFIG){ usp_config_%.*s(); return; }\n"),  basename_sz, basename_ptr);
         if (bauth)   (void) u__snprintf(ptr10, 100, U_CONSTANT_TO_PARAM("\n\tif (param == U_DPAGE_AUTH) { usp_auth_%.*s(); return; }\n"),     basename_sz, basename_ptr);
         }
      else
         {
         bcfg    =
         binit   =
         breset  =
         bend    =
         bsighup =
         bfork   =
         bopen   =
         bclose  =
         bauth   = false;
         }

      bool bdatamod = (bsession || bstorage);

      U_INTERNAL_DUMP("bcfg = %b binit = %b breset = %b bend = %b bsighup = %b bfork = %b bopen = %b bclose = %b bdatamod = %b bauth = %b",
                       bcfg,     binit,     breset,     bend,     bsighup,     bfork,     bopen,     bclose,     bdatamod,     bauth)

      if (bdatamod)
         {
         if (bsession)
            {
            (void) declaration.append(U_CONSTANT_TO_PARAM(
               "\n\t\nstatic bool usp_bSESSION;\n"
               "\n\t\n"
               "#define USP_SESSION_VAR_GET(index,varname) \\\n"
               "   { \\\n"
               "   UString varname##_value; \\\n"
               "   if (UHTTP::getDataSession(index, varname##_value) && \\\n"
               "       (usp_sz = varname##_value.size())) \\\n"
               "      { \\\n"
               "      UString2Object(varname##_value.data(), usp_sz, varname); \\\n"
               "      } \\\n"
               "   U_INTERNAL_DUMP(\"%s(%u) = %V\", #varname, usp_sz, varname##_value.rep) \\\n"
               "   }"
               "\n\t\n"
               "#define USP_SESSION_VAR_PUT(index,varname) \\\n"
               "   { \\\n"
               "   usp_sz = UObject2String(varname, usp_buffer, sizeof(usp_buffer)); \\\n"
               "   if (usp_sz) \\\n"
               "      { \\\n"
               "      UString varname##_value((void*)usp_buffer, usp_sz); \\\n"
               "      UHTTP::data_session->putValueVar(index, varname##_value); \\\n"
               "      U_INTERNAL_DUMP(\"%s(%u) = %V\", #varname, usp_sz, varname##_value.rep) \\\n"
               "      } \\\n"
               "   else \\\n"
               "      { \\\n"
               "      UHTTP::data_session->putValueVar(index, UString::getStringNull()); \\\n"
               "      } \\\n"
               "   }\n"));
            }

         if (bstorage)
            {
            (void) declaration.append(U_CONSTANT_TO_PARAM(
               "\n\t\nstatic bool usp_bSTORAGE;\n"
               "\n\t\n"
               "#define USP_STORAGE_VAR_GET(index,varname) \\\n"
               "   { \\\n"
               "   UString varname##_value; \\\n"
               "   if (UHTTP::getDataStorage(index, varname##_value) && \\\n"
               "       (usp_sz = varname##_value.size())) \\\n"
               "      { \\\n"
               "      UString2Object(varname##_value.data(), usp_sz, varname); \\\n"
               "      } \\\n"
               "   U_INTERNAL_DUMP(\"%s(%u) = %V\", #varname, usp_sz, varname##_value.rep) \\\n"
               "   }"
               "\n\t\n"
               "#define USP_STORAGE_VAR_PUT(index,varname) \\\n"
               "   { \\\n"
               "   usp_sz = UObject2String(varname, usp_buffer, sizeof(usp_buffer)); \\\n"
               "   if (usp_sz) \\\n"
               "      { \\\n"
               "      UString varname##_value((void*)usp_buffer, usp_sz); \\\n"
               "      UHTTP::data_storage->putValueVar(index, varname##_value); \\\n"
               "      U_INTERNAL_DUMP(\"%s(%u) = %V\", #varname, usp_sz, varname##_value.rep) \\\n"
               "      } \\\n"
               "   else \\\n"
               "      { \\\n"
               "      UHTTP::data_storage->putValueVar(index, UString::getStringNull()); \\\n"
               "      } \\\n"
               "   }\n"));
            }

         if (binit == false)
            {
            binit = true;

            (void) declaration.reserve(500U);

            declaration.snprintf_add(U_CONSTANT_TO_PARAM(
                  "\n\t\nstatic void usp_init_%.*s()\n"
                  "{\n"
                  "\tU_TRACE(5, \"::usp_init_%.*s()\")\n"
                  "\t\n"
                  "%s"
                  "%s"
                  "\n\tif (UHTTP::db_session == U_NULLPTR) UHTTP::initSession();\n"
                  "}"),
                  basename_sz, basename_ptr,
                  basename_sz, basename_ptr,
                  (bsession ? "\n\tif (UHTTP::data_session == U_NULLPTR) U_NEW(UDataSession, UHTTP::data_session, UDataSession)\n" : ""),
                  (bstorage ? "\n\tif (UHTTP::data_storage == U_NULLPTR) { U_NEW(UDataSession, UHTTP::data_storage, UDataSession(*UString::str_storage_keyid)) }\n" : ""));
            }
         }

      if (binit) (void) u__snprintf(ptr1, 100, U_CONSTANT_TO_PARAM("\n\tif (param == U_DPAGE_INIT) { usp_init_%.*s(); return; }\n"), basename_sz, basename_ptr);

      if (bvar)
         {
         (void) vars.append(U_CONSTANT_TO_PARAM("\n\tuint32_t usp_sz = 0;"
                                                "\n\tchar usp_buffer[10 * 4096];\n"));
         }

      // NB: we check for HTML without HTTP headers...

      if (http_header.empty())
         {
         if (is_html) (void) output1.append(U_CONSTANT_TO_PARAM("\n\tUHTTP::mime_index = U_html;\n"));

         if (bhttp_header_empty == false) (void) output1.append(U_CONSTANT_TO_PARAM("\n\tU_http_info.endHeader = 0;\n"));
         }

      // NB: we have checked for presence of 'return' inside the code...

      if (breturn)
         {
         (void) declaration.reserve(200U + vars.size() + output0.size());

         declaration.snprintf_add(U_CONSTANT_TO_PARAM(
               "\n\t\nstatic void usp_body_%.*s()\n"
               "{\n"
               "\tU_TRACE(5, \"::usp_body_%.*s()\")\n"
               "\t\n"
               "%v"
               "%v"
               "\n}"),
               basename_sz, basename_ptr,
               basename_sz, basename_ptr,
               vars.rep,
               output0.rep);

         output0.snprintf(U_CONSTANT_TO_PARAM("\n\tusp_body_%.*s();\n"), basename_sz, basename_ptr);
         }

      UString result(1024U + static_vars.size() + declaration.size() + http_header.size() + vars.size() + output0.size() + output1.size());

      result.snprintf(U_CONSTANT_TO_PARAM(
            "// %.*s.cpp - dynamic page translation (%.*s.usp => %.*s.cpp)\n"
            "\t\n"
            "#include <ulib/net/server/usp_macro.h>\n"
            "\t\n"
            "%v"
            "\n\t\n"
            "%v"
            "\n\t\n"
            "extern \"C\" {\n"
            "extern U_EXPORT void runDynamicPageParam_%.*s(uint32_t param);\n"
            "       U_EXPORT void runDynamicPageParam_%.*s(uint32_t param)\n"
            "{\n"
            "\tU_TRACE(0, \"::runDynamicPageParam_%.*s(%%u)\", param)\n"
            "\t\n"
            "%v"
            "%s"
            "%s"
            "%s"
            "%s"
            "%s"
            "%s"
            "%s"
            "%s"
            "%s"
            "%s"
            "\treturn;\n"
            "} }\n"
            "\t\n"
            "extern \"C\" {\n"
            "extern U_EXPORT void runDynamicPage_%.*s();\n"
            "       U_EXPORT void runDynamicPage_%.*s()\n"
            "{\n"
            "\tU_TRACE_NO_PARAM(0, \"::runDynamicPage_%.*s()\")\n"
            "\t\n"
            "%v"
            "%v"
            "%v"
            "%v"
            "%v"
            "} }\n"),
            basename_sz, basename_ptr,
            basename_sz, basename_ptr,
            basename_sz, basename_ptr,
            static_vars.rep,
            declaration.rep,
            basename_sz, basename_ptr,
            basename_sz, basename_ptr,
            basename_sz, basename_ptr,
            sseloop.rep,
            ptr1,
            ptr2,
            ptr3,
            ptr4,
            ptr5,
            ptr6,
            ptr7,
            ptr8,
            ptr9,
            ptr10,
            basename_sz, basename_ptr,
            basename_sz, basename_ptr,
            basename_sz, basename_ptr,
            vars.rep,
            vcode.rep,
            http_header.rep,
            output0.rep,
            output1.rep);

      UString name(200U);

      name.snprintf(U_CONSTANT_TO_PARAM("%.*s.cpp"), basename_sz, basename_ptr);

      (void) UFile::writeTo(name, UStringExt::removeEmptyLine(result));
      }

private:
   UTokenizer t;
   UVector<UString> vdefine;
   UString pinclude, usp, token, output0, output1, static_vars, declaration, vcode, http_header, sseloop, vars;
   const char* basename_ptr;
   uint32_t basename_sz;
   bool bvar, breturn, bsession, bstorage, bfirst_pass, is_html, test_if_html, bhttp_header_empty, bpreprocessing_failed;

   U_DISALLOW_COPY_AND_ASSIGN(Application)
};

U_MAIN
