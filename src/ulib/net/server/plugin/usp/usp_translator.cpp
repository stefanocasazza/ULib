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

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE_NO_PARAM(5, "Application::Application()")

      bvar                  = 
      bsession              =
      bstorage              =
      bfirst_pass           =
           is_html          =
      test_if_html          =
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

   void manageDirectiveSessionOrStorage(const char* name)
      {
      U_TRACE(5, "Application::manageDirectiveSessionOrStorage(%S)", name)

      U_INTERNAL_ASSERT(token)

      UString id, tmp;
      const char* ptr;
      uint32_t pos, size;
      UVector<UString> vec(token, "\t\n;");

      bvar = true;

      (void) output0.reserve(20U + token.size());

      output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t%v\n\t\n"), token.rep);

      for (uint32_t i = 0, n = vec.size(); i < n; ++i)
         {
         ptr = (tmp = UStringExt::trim(vec[i])).data();

         do { ++ptr; } while (u__isspace(*ptr) == false);
         do { ++ptr; } while (u__isspace(*ptr) == true);

          pos = (id = tmp.substr(tmp.distance(ptr))).find('(');
         size = (pos == U_NOT_FOUND ? id.size() : pos);

         (void) output0.reserve(50U + size);
         (void) output1.reserve(50U + size);

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUSP_%s_VAR_GET(%u,%.*s);\n"), name, i, size, ptr);
         output1.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUSP_%s_VAR_PUT(%u,%.*s);\n"), name, i, size, ptr);

#     ifdef DEBUG
         id.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#     endif
         }
      }

   void manageDirectiveArgsOrCpath(const char* name, bool binc)
      {
      U_TRACE(5, "Application::manageDirectiveArgsOrCpath(%S,%b)", name, binc)

      U_INTERNAL_ASSERT(token)

      uint32_t pos;
      UString id, tmp;
      UVector<UString> vec(token, "\t\n;");

      (void) output0.reserve(200U + token.size());

      for (uint32_t i = 0, n = vec.size(); i < n; ++i)
         {
         tmp = UStringExt::trim(vec[i]);
         pos = tmp.find('(');
          id = (pos == U_NOT_FOUND ? tmp : tmp.substr(0U, pos));

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUString %v = %s(%u);\n"), id.rep, name, i+binc);

         if (pos != U_NOT_FOUND)
            {
            output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tif (%v.empty()) %v = U_STRING_FROM_CONSTANT(%.*s);\n"), id.rep, id.rep, tmp.size()-pos-2, tmp.c_pointer(pos+1));
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
       * <!--#session ... -->
       * <!--#storage ... -->
       * <!--#args ... -->
       * <!--#cpath ... -->
       * <!--#header ... -->
       * <!--#code ... -->
       * <!--#vcode ... -->
       * <!--#pcode ... -->
       * <!--#lcode ... -->
       * <!--#ssecode ... -->
       * <!--#sseloop ... -->
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

         U_INTERNAL_ASSERT(token)

         declaration = UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t"));
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("session")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         bsession = true;

         setDirectiveItem(directive, U_CONSTANT_SIZE("session"));

         if (token) manageDirectiveSessionOrStorage("SESSION");
         else
            {
            (void) output0.append(U_CONSTANT_TO_PARAM("\n\tif (UHTTP::getDataSession() == false) UHTTP::setSessionCookie();\n"));
            (void) output1.append(U_CONSTANT_TO_PARAM("\n\tUHTTP::putDataSession();\n"));
            }
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("storage")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         bstorage = true;

         setDirectiveItem(directive, U_CONSTANT_SIZE("storage"));

         if (token) manageDirectiveSessionOrStorage("STORAGE");
         else
            {
            (void) output0.append(U_CONSTANT_TO_PARAM("\n\t(void) UHTTP::getDataStorage();\n"));
            (void) output1.append(U_CONSTANT_TO_PARAM("\n\tUHTTP::putDataStorage();\n"));
            }
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("args")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("args"));

         (void) output0.append(U_CONSTANT_TO_PARAM("\t\n\tif (UHTTP::isGETorPOST()) (void) UHTTP::processForm();\n"));

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

         U_INTERNAL_ASSERT(token)

         token = UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t"));

         vcode.setBuffer(20U + token.size());

         vcode.snprintf(U_CONSTANT_TO_PARAM("\n\t%v\n\t\n"), token.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("pcode")) == 0) // parallelization code (long running task)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("pcode"));

         U_INTERNAL_ASSERT(token)

         token = UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t"));

         (void) output0.reserve(20U + token.size());

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t%v\n\t\n"), token.rep);

         (void) vcode.append(U_CONSTANT_TO_PARAM("\tif (UServer_Base::startParallelization()) { U_http_info.nResponseCode = HTTP_CONTINUE; return; }\n\t\n"));
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("lcode")) == 0) // load balance code
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("lcode"));

         U_INTERNAL_ASSERT(token)

         token = UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t"));

         (void) output0.reserve(20U + token.size());

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t%v\n\t\n"), token.rep);

         (void) vcode.append(U_CONSTANT_TO_PARAM("\tif (UServer_Base::startParallelization()) { U_http_info.nResponseCode = HTTP_CONTINUE; return; }\n\t\n"));

#     ifdef USE_LOAD_BALANCE
         (void) vcode.append(U_CONSTANT_TO_PARAM("\tif (UHTTP::manageRequestOnRemoteServer()) return;\n\t\n"));
#     endif
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("code")) == 0) // generic code
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("code"));

         U_INTERNAL_ASSERT(token)

         token = UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t"));

         (void) output0.reserve(20U + token.size());

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t%v\n\t\n"), token.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("ssecode")) == 0) // SSE code
         {
         U_ASSERT(output2.empty())
         U_ASSERT(http_header.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("ssecode"));

         U_INTERNAL_ASSERT(token)

         token = UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t\t"));

         /**
          * we must insert:
          *
          * <!--#header
          * Content-Type: text/event-stream 
          * Cache-Control: no-cache
          * -->
          */

         (void) http_header.reserve(1000U + token.size());

         (void) http_header.snprintf(U_CONSTANT_TO_PARAM(
               "\n\tif (U_http_accept_len != U_CONSTANT_SIZE(\"text/event-stream\") ||"
               " u_get_unalignedp64(U_http_info.accept) != U_MULTICHAR_CONSTANT64('t','e','x','t','/','e','v','e')) UHTTP::setBadRequest();"
               "\n\telse"
               "\n\t\t{"
               "\n\t\tUHTTP::bsse = true;"
               "\n\t\t(void) UServer_Base::csocket->shutdown(SHUT_RD);"
               "\n\t\tU_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false)"
               "\n\t\tU_http_info.endHeader = 61;"
               "\n\t\t(void) UClientImage_Base::wbuffer->insert(0, U_CONSTANT_TO_PARAM(\"Content-Type: text/event-stream\\r\\nCache-Control: no-cache\\r\\n\\r\\n\"));"
               "\n\t\tU_http_content_type_len = 1;"
               "\n\t\t%v"
               "\n\t\t}"), token.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("sseloop")) == 0) // SSE loop
         {
         U_ASSERT(sseloop.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("sseloop"));

         U_INTERNAL_ASSERT(token)

         uint32_t sz      = token.size();
         const char* data = token.data();

         uint32_t sse_time_to_sleep = 1;

         if (*data == ':')
            {
            ++data;

            sse_time_to_sleep = u_strtoulp(&data);

            sz = token.remain(data);
            }

         char buffer[200];

         (void) vars.append(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("\n\tUTimeVal sse_time_to_sleep(%u, 0L);\n"), sse_time_to_sleep));

         sseloop = UStringExt::substitute(data, sz, '\n', U_CONSTANT_TO_PARAM("\n\t"));

         (void) output2.reserve(200U + sseloop.size());

         output2.snprintf(U_CONSTANT_TO_PARAM(
                        "\n\treturn;\n\t\n"
                        "sseloop:"
                        "\n\tsse_time_to_sleep.nanosleep();"
                        "\n\t%v"
                        "\n\tgoto sseloop;"), sseloop.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("header")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_ASSERT(http_header.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("header"));

         // NB: we use insert because the possibility of UHTTP::callService() (see chat.usp)...

         if (U_STRING_FIND(token, 0, "Content-Type") != U_NOT_FOUND) (void) output2.assign(U_CONSTANT_TO_PARAM("\n\tU_http_content_type_len = 1;\n\t\n"));

         http_header = UStringExt::dos2unix(token, true);

         (void) http_header.append(U_CONSTANT_TO_PARAM("\r\n\r\n"));

         uint32_t n = http_header.size();

         UString encoded(n * 4);

         UEscape::encode(http_header, encoded);

         U_ASSERT(encoded.isQuoted())

         (void) http_header.reserve(200U + encoded.size());

         (void) http_header.snprintf(U_CONSTANT_TO_PARAM("\n\tU_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false)"
                                                         "\n\tU_http_info.endHeader = %u;"
                                                         "\n\t(void) UClientImage_Base::wbuffer->insert(0, U_CONSTANT_TO_PARAM(%v));\n\t\n"), n, encoded.rep);
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

         U_INTERNAL_ASSERT(token)

         (void) output0.reserve(100U + token.size());

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t(void) UClientImage_Base::wbuffer->append((%v));\n"), token.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("xmlputs")) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("xmlputs"));

         U_INTERNAL_ASSERT(token)

         (void) output0.reserve(100U + token.size());

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUSP_XML_PUTS((%v));\n"), token.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("cout")) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         bvar = true;

         setDirectiveItem(directive, U_CONSTANT_SIZE("cout"));

         U_INTERNAL_ASSERT(token)

         (void) output0.reserve(200U + token.size());

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tusp_sz = UObject2String((%v), usp_buffer, sizeof(usp_buffer));"
                                                  "\n\t(void) UClientImage_Base::wbuffer->append(usp_buffer, usp_sz);\n"), token.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("print")) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         bvar = true;

         bool bfor = (strncmp(directive + U_CONSTANT_SIZE("print"), U_CONSTANT_TO_PARAM("for")) == 0);

         setDirectiveItem(directive, (bfor ? U_CONSTANT_SIZE("printfor") : U_CONSTANT_SIZE("print")));

         U_INTERNAL_ASSERT(token)

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

      if (t.next(token, &bgroup))
         {
         U_INTERNAL_ASSERT(bgroup)

         processDirective();

         if (usp) 
            {
            // no trailing \n...

            for (ptr = t.getPointer(); u__islterm(*ptr); ++ptr) {}

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

      processUSP();

#  ifdef DEBUG
      token.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#  endif

      if (vdefine.empty() == false) usp = UStringExt::substitute(usp, vdefine);

      if (execPreProcessing() == false) U_WARNING("preprocessing of %V failed", filename.rep);

      bfirst_pass = false;

      processUSP();

      U_INTERNAL_DUMP("declaration = %V", declaration.rep)

      bool binit,   // usp_init (Server-wide hooks)...
           bend,    // usp_end
           bsighup, // usp_sighup
           bfork;   // usp_fork

      const char* ptr0      = (sseloop ? "\n\t\tif (UHTTP::bsse) goto sseloop;\n" : "");
            char  ptr1[100] = { '\0' };
            char  ptr2[100] = { '\0' };
            char  ptr3[100] = { '\0' };
            char  ptr4[100] = { '\0' };
            char  ptr5[100] = { '\0' };
      const char* ptr6      = "";
      const char* ptr7      = "";
      
#  ifndef U_CACHE_REQUEST_DISABLE
      if (usp.c_char(4) == '#'      &&
          u__isspace(usp.c_char(5)) &&
          u_get_unalignedp32(usp.data()) == U_MULTICHAR_CONSTANT32('<','!','-','-')) // <!--# --> (comment)
         {
         ptr7 = "\n\tUClientImage_Base::setRequestNoCache();\n\t\n";
         }
#  endif

      UString basename = UStringExt::basename(filename);

      uint32_t    basename_sz  = basename.size() - U_CONSTANT_SIZE(".usp");
      const char* basename_ptr = basename.data();

      if (declaration)
         {
         binit   = (U_STRING_FIND(declaration, 0, "static void usp_init_")   != U_NOT_FOUND);
         bend    = (U_STRING_FIND(declaration, 0, "static void usp_end_")    != U_NOT_FOUND);
         bsighup = (U_STRING_FIND(declaration, 0, "static void usp_sighup_") != U_NOT_FOUND);
         bfork   = (U_STRING_FIND(declaration, 0, "static void usp_fork_")   != U_NOT_FOUND);

         if (bfork)   (void) u__snprintf(ptr5, 100, U_CONSTANT_TO_PARAM("\n\t\tif (param == U_DPAGE_FORK) { usp_fork_%.*s(); return; }\n"),     basename_sz, basename_ptr);
         if (bsighup) (void) u__snprintf(ptr4, 100, U_CONSTANT_TO_PARAM("\n\t\tif (param == U_DPAGE_SIGHUP) { usp_sighup_%.*s(); return; }\n"), basename_sz, basename_ptr);

         if (bend)
            {
#        ifndef DEBUG
            if (bpreprocessing_failed) bend = false;
            else
#        endif
            (void) u__snprintf(ptr3, 100, U_CONSTANT_TO_PARAM("\n\t\tif (param == U_DPAGE_DESTROY) { usp_end_%.*s(); return; }\n"), basename_sz, basename_ptr);
            }
         }
      else
         {
         binit   =
         bend    =
         bsighup =
         bfork   = false;
         }

      U_INTERNAL_DUMP("binit = %b bend = %b bsighup = %b bfork = %b", binit, bend, bsighup, bfork)

      if (binit == false &&
          (bsession || bstorage))
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
               (bsession ? "\n\tif (UHTTP::data_session == U_NULLPTR) U_NEW(UDataSession, UHTTP::data_session, UDataSession);\n" : ""),
               (bstorage ? "\n\tif (UHTTP::data_storage == U_NULLPTR) { U_NEW(UDataSession, UHTTP::data_storage, UDataSession(*UString::str_storage_keyid)); }\n" : ""));
         }

      if (binit) (void) u__snprintf(ptr1, 100, U_CONSTANT_TO_PARAM("\n\t\tif (param == U_DPAGE_INIT) { usp_init_%.*s(); return; }\n"), basename_sz, basename_ptr);

      if (binit   == false ||
          bend    == false ||
          bsighup == false ||
          bfork   == false)
         {
         ptr6 = (bfork ? "\n\t\tif (param >  U_DPAGE_FORK) return;\n"
                       : "\n\t\tif (param >= U_DPAGE_FORK) return;\n");
         }

      if (bvar)
         {
         (void) vars.append(U_CONSTANT_TO_PARAM("\n\tuint32_t usp_sz = 0;"
                                                "\n\tchar usp_buffer[10 * 4096];\n"));
         }

      // NB: we check for HTML without HTTP headers...

      if (http_header.empty())
         {
         if (is_html) (void) http_header.append(U_CONSTANT_TO_PARAM("\n\tUHTTP::mime_index = U_html;\n"));

         (void) http_header.append(U_CONSTANT_TO_PARAM("\n\tU_http_info.endHeader = 0;\n"));
         }

      UString result(1024U + declaration.size() + http_header.size() + output0.size() + output1.size() + output2.size() + sseloop.size() + vars.size());

      result.snprintf(U_CONSTANT_TO_PARAM(
            "// %.*s.cpp - dynamic page translation (%.*s.usp => %.*s.cpp)\n"
            "\t\n"
            "#include <ulib/net/server/usp_macro.h>\n"
            "\t\n"
            "%v"
            "\n\t\n"
            "extern \"C\" {\n"
            "extern U_EXPORT void runDynamicPage_%.*s(int param);\n"
            "       U_EXPORT void runDynamicPage_%.*s(int param)\n"
            "{\n"
            "\tU_TRACE(0, \"::runDynamicPage_%.*s(%%d)\", param)\n"
            "\t\n"
            "%v"
            "\t\n"
            "\tif (param)\n"
            "\t\t{\n"
            "%s"
            "%s"
            "%s"
            "%s"
            "%s"
            "%s"
            "%s"
            "\t\t}\n"
            "\t\n"
            "%v"
            "%v"
            "\t\n"
            "%v"
            "%v"
            "%v"
            "%s"
            "\t\n"
            "} }\n"),
            basename_sz, basename_ptr,
            basename_sz, basename_ptr,
            basename_sz, basename_ptr,
            declaration.rep,
            basename_sz, basename_ptr,
            basename_sz, basename_ptr,
            basename_sz, basename_ptr,
            vars.rep,
            ptr0,
            ptr1,
            ptr2,
            ptr3,
            ptr4,
            ptr5,
            ptr6,
            vcode.rep,
            http_header.rep,
            output0.rep,
            output1.rep,
            output2.rep,
            ptr7);

      UString name(200U);

      name.snprintf(U_CONSTANT_TO_PARAM("%.*s.cpp"), basename_sz, basename_ptr);

      (void) UFile::writeTo(name, UStringExt::removeEmptyLine(result));
      }

private:
   UTokenizer t;
   UVector<UString> vdefine;
   UString pinclude, usp, token, output0, output1, output2, declaration, vcode, http_header, sseloop, vars;
   bool bvar, bsession, bstorage, bfirst_pass, is_html, test_if_html, bpreprocessing_failed;

   U_DISALLOW_COPY_AND_ASSIGN(Application)
};

U_MAIN
