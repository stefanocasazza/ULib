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

#define U_OPTIONS ""
#define U_PURPOSE "program for dynamic page translation ([xxx].usp => [xxx].cpp)"

#include <ulib/application.h>

#define USP_SESSION_INIT \
"\n\t\nstatic void usp_init_%.*s()\n" \
"{\n" \
"\tU_TRACE(5, \"::usp_init_%.*s()\")\n" \
"\t\n" \
"%s" \
"%s" \
"\n\tif (UHTTP::db_session == U_NULLPTR) UHTTP::initSession();\n" \
"}"

#define USP_TEMPLATE \
"// %.*s.cpp - dynamic page translation (%.*s.usp => %.*s.cpp)\n" \
"\t\n" \
"#include <ulib/net/server/usp_macro.h>\n" \
"\t\n" \
"%v" \
"\t\n" \
"\t\n" \
"extern \"C\" {\n" \
"extern U_EXPORT void runDynamicPage_%.*s(int param);\n" \
"       U_EXPORT void runDynamicPage_%.*s(int param)\n" \
"{\n" \
"\tU_TRACE(0, \"::runDynamicPage_%.*s(%%d)\", param)\n" \
"\t\n" \
"%s" \
"\t\n" \
"\tif (param)\n" \
"\t\t{\n" \
"%s" \
"%s" \
"%s" \
"%s" \
"%s" \
"%s" \
"\t\t}\n" \
"\t\n" \
"%v" \
"%v" \
"\t\n" \
"%v" \
"%v" \
"%v" \
"%s" \
"\t\n" \
"} }\n"

class Application : public UApplication {
public:

   Application()
      {
      U_TRACE_NO_PARAM(5, "Application::Application()")
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

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUSP_%s_VAR_GET(%u,%.*s);\n\t"), name, i, size, ptr);
         output1.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUSP_%s_VAR_PUT(%u,%.*s);\n\t"), name, i, size, ptr);

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

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUString %v = %s(%u);\n\t"), id.rep, name, i+binc);

         if (pos != U_NOT_FOUND)
            {
            output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tif (%v.empty()) %v = U_STRING_FROM_CONSTANT(%.*s);\n\t"), id.rep, id.rep, tmp.size()-pos-2, tmp.c_pointer(pos+1));
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
       * <!--#number ... -->
       * <!--#puts ... -->
       * <!--#xmlputs ... -->
       * <!--#cout ... -->
       * <!--#print ... -->
       * <!--#printfor ... -->
       */

      U_INTERNAL_DUMP("token = %V", token.rep)

      const char* directive = token.c_pointer(2); // "-#"...

      U_INTERNAL_DUMP("directive(10) = %.*S", 10, directive)

      if (bfirst_pass)
         {
         bool bdefine  = (strncmp(directive, U_CONSTANT_TO_PARAM("define"))  == 0),
              binclude = (strncmp(directive, U_CONSTANT_TO_PARAM("include")) == 0);

         U_INTERNAL_DUMP("bdefine = %b binclude = %b", bdefine, binclude)

         if (bdefine ||
             binclude)
            {
            UString block(100U);
            UVector<UString> vec;

            block.snprintf(U_CONSTANT_TO_PARAM("<!-%v-->"), token.rep);

            for (const char* ptr = t.getPointer(); u__isspace(*ptr); ++ptr) block.push_back(*ptr);

            vec.push_back(block);

            if (binclude)
               {
               setDirectiveItem(directive, U_CONSTANT_SIZE("include"));

               UString content = UFile::contentOf(token);

               if (content) vec.push_back(vdefine.empty() ? content : UStringExt::substitute(content, vdefine));
               else
                  {
                  vec.push_back(UString::getStringNull());

                  U_WARNING("load of include usp %V failed", token.rep);
                  }
               }
            else
               {
               U_INTERNAL_ASSERT(bdefine)

               setDirectiveItem(directive, U_CONSTANT_SIZE("define"));

               vec.push_back(UString::getStringNull());

               (void) vec.split(U_STRING_TO_PARAM(token));

               U_ASSERT_EQUALS(vec.size(), 4)

               vdefine.push_back(vec[2]);
               vdefine.push_back(vec[3]);
               }

#        ifdef DEBUG
            token.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#        endif

            t.setData((usp = UStringExt::substitute(usp, vec)));
            }

         return;
         }

      if (strncmp(directive, U_CONSTANT_TO_PARAM("declaration")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_ASSERT(declaration.empty()) // NB: <!--#declaration ... --> must be at the beginning and uniq...
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("declaration"));

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
            (void) output0.append(U_CONSTANT_TO_PARAM("\n\tif (UHTTP::getDataSession() == false) UHTTP::setSessionCookie();\n\t"));
            (void) output1.append(U_CONSTANT_TO_PARAM("\n\tUHTTP::putDataSession();\n\t"));
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
            (void) output0.append(U_CONSTANT_TO_PARAM("\n\t(void) UHTTP::getDataStorage();\n\t"));
            (void) output1.append(U_CONSTANT_TO_PARAM("\n\tUHTTP::putDataStorage();\n\t"));
            }
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("args")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("args"));

         (void) output0.append(U_CONSTANT_TO_PARAM("\t\n\t\tif (UHTTP::isGETorPOST()) (void) UHTTP::processForm();\n"));

         if (token) manageDirectiveArgsOrCpath("USP_FORM_VALUE", false);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("cpath")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("cpath"));

         manageDirectiveArgsOrCpath("UHTTP::getPathComponent", true);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("header")) == 0)
         {
         U_ASSERT(vcode.empty())
         U_ASSERT(http_header.empty())
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("header"));

         http_header = token;
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("vcode")) == 0) // validation code
         {
         U_ASSERT(vcode.empty()) // NB: <!--#vcode ... --> must be before other code and uniq...
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("vcode"));

         token = UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t"));

         vcode.setBuffer(20U + token.size());

         vcode.snprintf(U_CONSTANT_TO_PARAM("\n\t%v\n\t\n"), token.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("pcode")) == 0) // parallelization code (long running task)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("pcode"));

         token = UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t"));

         (void) output0.reserve(20U + token.size());

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t%v\n\t\n"), token.rep);

         (void) vcode.append(U_CONSTANT_TO_PARAM("\tif (UServer_Base::startParallelization()) { U_http_info.nResponseCode = HTTP_CONTINUE; return; }\n\t\n"));
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("lcode")) == 0) // load balance code
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("lcode"));

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

         token = UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t"));

         (void) output0.reserve(20U + token.size());

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t%v\n\t\n"), token.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("number")) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("number"));

         (void) output0.reserve(100U + token.size());

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUStringExt::appendNumber32(*UClientImage_Base::wbuffer, (%v));\n\t"), token.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("puts")) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("puts"));

         (void) output0.reserve(100U + token.size());

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t(void) UClientImage_Base::wbuffer->append((%v));\n\t"), token.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("xmlputs")) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         setDirectiveItem(directive, U_CONSTANT_SIZE("xmlputs"));

         (void) output0.reserve(100U + token.size());

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUSP_XML_PUTS((%v));\n\t"), token.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("cout")) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         bvar = true;

         setDirectiveItem(directive, U_CONSTANT_SIZE("cout"));

         (void) output0.reserve(200U + token.size());

         output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tusp_sz = UObject2String((%v), usp_buffer, sizeof(usp_buffer));"
                                                  "\n\t(void) UClientImage_Base::wbuffer->append(usp_buffer, usp_sz);\n\t"), token.rep);
         }
      else if (strncmp(directive, U_CONSTANT_TO_PARAM("print")) == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(bfirst_pass, false)

         bvar = true;

         bool bfor = (strncmp(directive + U_CONSTANT_SIZE("print"), U_CONSTANT_TO_PARAM("for")) == 0);

         setDirectiveItem(directive, (bfor ? U_CONSTANT_SIZE("printfor") : U_CONSTANT_SIZE("print")));

         (void) output0.reserve(200U + token.size());

         UVector<UString> vec(token, ';');

         if (bfor)
            {
            U_ASSERT_EQUALS(vec.size(), 5)

            output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tfor (%v; %v; %v) { usp_sz = u__snprintf(usp_buffer, sizeof(usp_buffer), %v, %v);"
                                                     "(void) UClientImage_Base::wbuffer->append(usp_buffer, usp_sz); }\n\t"),
                                                     vec[0].rep, vec[1].rep, vec[2].rep, vec[3].rep, vec[4].rep);
            }
         else
            {
            U_ASSERT_EQUALS(vec.size(), 2)

            output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\tusp_sz = u__snprintf(usp_buffer, sizeof(usp_buffer), %v, %v);"
                                                     "(void) UClientImage_Base::wbuffer->append(usp_buffer, usp_sz);\n\t"), vec[0].rep, vec[1].rep);
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

#  ifdef DEBUG
      token.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#  endif

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

            output0.snprintf_add(U_CONSTANT_TO_PARAM("\n\t(void) UClientImage_Base::wbuffer->append(\n\t\tU_CONSTANT_TO_PARAM(%v)\n\t);\n\t"), tmp.rep);
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

   void firstPass()
      {
      U_TRACE_NO_PARAM(5, "Application::firstPass()")

      // NB: we check for <!--#define ... --> and <!--#include ... -->

      bfirst_pass = true;

      processUSP();
      }

   bool execPreProcessing()
      {
      U_TRACE_NO_PARAM(5, "Application::execPreProcessing()")

#  ifndef DEBUG
      bpreprocessing_failed = false;
#  endif

      if (U_STRING_FIND(usp, 0, "\n#ifdef DEBUG") != U_NOT_FOUND)
         {
         UFileConfig cfg(UStringExt::substitute(usp, U_CONSTANT_TO_PARAM("#include"), U_CONSTANT_TO_PARAM("//#include")), true);

         if (cfg.processData(false)) usp = UStringExt::substitute(cfg.getData(), U_CONSTANT_TO_PARAM("//#include"), U_CONSTANT_TO_PARAM("#include"));
         else
            {
#        ifndef DEBUG
            bpreprocessing_failed = true;
#        endif

            U_RETURN(false);
            }
         }

      U_RETURN(true);
      }

   void secondPass()
      {
      U_TRACE_NO_PARAM(5, "Application::secondPass()")

      bvar         = 
      bsession     =
      bstorage     =
      bfirst_pass  =
           is_html =
      test_if_html = false;

      processUSP();
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      if (argv[1] == U_NULLPTR) U_ERROR("filename usp not specified");

      UString filename(argv[1]);

      usp = UFile::contentOf(filename);

      if (usp.empty()) U_ERROR("load of %V failed", filename.rep);

      t.setGroup(U_CONSTANT_TO_PARAM("<!--->"));

      firstPass();

      if (usp.empty()) U_ERROR("first pass for %V failed", filename.rep);

      if (execPreProcessing() == false) U_WARNING("preprocessing of %V failed", filename.rep);

      secondPass();

      U_INTERNAL_DUMP("declaration = %V", declaration.rep)

      bool binit,   // usp_init (Server-wide hooks)...
           bend,    // usp_end
           bsighup, // usp_sighup
           bfork;   // usp_fork

            char  ptr1[100] = { '\0' };
            char  ptr2[100] = { '\0' };
            char  ptr3[100] = { '\0' };
            char  ptr4[100] = { '\0' };
            char  ptr5[100] = { '\0' };
      const char* ptr6      = "";
      const char* ptr7      = (u_get_unalignedp32(usp.data()) == U_MULTICHAR_CONSTANT32('<','!','-','-') && usp.c_char(4) == '#' && u__isspace(usp.c_char(5)) // <!--# --> (comment)
                                 ? "\n\tUClientImage_Base::setRequestNoCache();\n\t\n"
                                 : "");

      UString basename = UStringExt::basename(filename);

      uint32_t    basename_sz  = basename.size() - U_CONSTANT_SIZE(".usp");
      const char* basename_ptr = basename.data();

      if (declaration)
         {
         binit   = (U_STRING_FIND(declaration, 0, "static void usp_init_")   != U_NOT_FOUND);
         bend    = (U_STRING_FIND(declaration, 0, "static void usp_end_")    != U_NOT_FOUND);
         bsighup = (U_STRING_FIND(declaration, 0, "static void usp_sighup_") != U_NOT_FOUND);
         bfork   = (U_STRING_FIND(declaration, 0, "static void usp_fork_")   != U_NOT_FOUND);

         if (bfork)   (void) u__snprintf(ptr5, 100, U_CONSTANT_TO_PARAM("\n\t\tif (param == U_DPAGE_FORK) { usp_fork_%.*s(); return; }\n\t"),     basename_sz, basename_ptr);
         if (bsighup) (void) u__snprintf(ptr4, 100, U_CONSTANT_TO_PARAM("\n\t\tif (param == U_DPAGE_SIGHUP) { usp_sighup_%.*s(); return; }\n\t"), basename_sz, basename_ptr);

         if (bend)
            {
#        ifndef DEBUG
            if (bpreprocessing_failed) bend = false;
            else
#        endif
            (void) u__snprintf(ptr3, 100, U_CONSTANT_TO_PARAM("\n\t\tif (param == U_DPAGE_DESTROY) { usp_end_%.*s(); return; }\n\t"), basename_sz, basename_ptr);
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

         declaration.snprintf_add(U_CONSTANT_TO_PARAM(USP_SESSION_INIT),
                         basename_sz, basename_ptr,
                         basename_sz, basename_ptr,
                         (bsession ? "\n\tif (UHTTP::data_session == U_NULLPTR)   U_NEW(UDataSession, UHTTP::data_session, UDataSession);\n\t" : ""),
                         (bstorage ? "\n\tif (UHTTP::data_storage == U_NULLPTR) { U_NEW(UDataSession, UHTTP::data_storage, UDataSession(*UString::str_storage_keyid)); }\n\t" : ""));
         }

      if (binit) (void) u__snprintf(ptr1, 100, U_CONSTANT_TO_PARAM("\n\t\tif (param == U_DPAGE_INIT) { usp_init_%.*s(); return; }\n\t"), basename_sz, basename_ptr);

      if (binit   == false ||
          bend    == false ||
          bsighup == false ||
          bfork   == false)
         {
         ptr6 = (bfork ? "\n\t\tif (param >  U_DPAGE_FORK) return;\n"
                       : "\n\t\tif (param >= U_DPAGE_FORK) return;\n");
         }

      // NB: we check for HTML without HTTP headers...

      UString output2;

      if (http_header.empty())
         {
         if (is_html) (void) http_header.append(U_CONSTANT_TO_PARAM("\n\tUHTTP::mime_index = U_html;\n\t"));

         (void) http_header.append(U_CONSTANT_TO_PARAM("\n\tU_http_info.endHeader = 0;\n"));
         }
      else
         {
         // NB: we use insert because the possibility of UHTTP::callService() (see chat.usp)...

         if (U_STRING_FIND(http_header, 0, "Content-Type") != U_NOT_FOUND) (void) output2.assign(U_CONSTANT_TO_PARAM("\n\t\tU_http_content_type_len = 1;\n\t"));

         http_header = UStringExt::dos2unix(http_header, true);

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

      UString result(1024U + sizeof(USP_TEMPLATE) + declaration.size() + http_header.size() + output0.size() + output1.size() + output2.size());

      result.snprintf(U_CONSTANT_TO_PARAM(USP_TEMPLATE),
                      basename_sz, basename_ptr,
                      basename_sz, basename_ptr,
                      basename_sz, basename_ptr,
                      declaration.rep,
                      basename_sz, basename_ptr,
                      basename_sz, basename_ptr,
                      basename_sz, basename_ptr,
                      bvar ? "\n\tuint32_t usp_sz = 0;"
                             "\n\tchar usp_buffer[10 * 4096];\n"
                           : "",
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
   UString usp, token, output0, output1, declaration, vcode, http_header;
   bool test_if_html, is_html, bpreprocessing_failed, bfirst_pass, bvar, bsession, bstorage;

   U_DISALLOW_COPY_AND_ASSIGN(Application)
};

U_MAIN
