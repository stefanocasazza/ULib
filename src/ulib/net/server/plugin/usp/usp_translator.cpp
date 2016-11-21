// =======================================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    usp_translator.cpp - the translator .usp => .cpp for plugin dynamic page for UServer
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
#define ARGS "[filename]"

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
"\n\tif (UHTTP::db_session == 0) UHTTP::initSession();\n" \
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
"%s" \
"\t\t}\n" \
"\t\n" \
"%v" \
"%s" \
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

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      if (argv[1] == 0) U_ERROR("Filename not specified");

      UString filename(argv[1], strlen(argv[1]));

      UString usp = UFile::contentOf(filename);

      if (usp.empty()) U_ERROR(" %V not valid", filename.rep);

#  ifndef DEBUG
      bool bpreprocessing_failed = false;
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

            U_WARNING("Preprocessing %V failed", filename.rep);
            }
         }

      const char* ptr;
      uint32_t i, n, size;
      UString token, declaration, http_header(U_CAPACITY), buffer(U_CAPACITY), bufname(100U), output(U_CAPACITY), output1(U_CAPACITY), xoutput(U_CAPACITY);
      bool bgroup, binit = false, bend = false, bsighup = false, bfork = false, bcomment = false, bvar = false, bform = false, test_if_html = false, is_html = false,
           bsession = false, bstorage = false, bparallelization = false;

      // Anything that is not enclosed in <!-- ... --> tags is assumed to be HTML

      UTokenizer t(usp);
      t.setGroup(U_CONSTANT_TO_PARAM("<!--->"));

      while (true)
         {
         uint32_t distance = t.getDistance(),
                       pos = U_STRING_FIND(usp, distance, "<!--#");

         if (pos)
            {
            if (pos == U_NOT_FOUND)
               {
               pos = usp.size();

               t.setDistance(pos);

               while (usp.c_char(pos-1) == '\n') --pos; // no trailing \n...
               }
            else
               {
               bcomment = true;

               t.setDistance(pos);
               }

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

               (void) buffer.reserve(tmp.size() + 100U);

               U_ASSERT(tmp.isQuoted())

               buffer.snprintf(U_CONSTANT_TO_PARAM("\n\t(void) UClientImage_Base::wbuffer->append(\n\t\tU_CONSTANT_TO_PARAM(%v)\n\t);\n\t"), tmp.rep);

               (void) output.append(buffer);
               }
            }

         if (t.next(token, &bgroup) == false) break;

         U_INTERNAL_DUMP("token = %V", token.rep)

         U_INTERNAL_ASSERT(bgroup)

         const char* directive = token.c_pointer(2); // "-#"...

         U_INTERNAL_DUMP("directive(10) = %.*S", 10, directive)

         if (strncmp(directive, U_CONSTANT_TO_PARAM("declaration")) == 0)
            {
            U_ASSERT(declaration.empty()) // NB: <!--#declaration ... --> must be at the beginning...

            n = token.size() - U_CONSTANT_SIZE("declaration") - 2;

            declaration = UStringExt::trim(directive + U_CONSTANT_SIZE("declaration"), n);

            binit   = (U_STRING_FIND(declaration, 0, "static void usp_init_")   != U_NOT_FOUND); // usp_init (Server-wide hooks)...
            bend    = (U_STRING_FIND(declaration, 0, "static void usp_end_")    != U_NOT_FOUND); // usp_end
            bsighup = (U_STRING_FIND(declaration, 0, "static void usp_sighup_") != U_NOT_FOUND); // usp_sighup
            bfork   = (U_STRING_FIND(declaration, 0, "static void usp_fork_")   != U_NOT_FOUND); // usp_fork

            declaration = UStringExt::substitute(declaration, '\n', U_CONSTANT_TO_PARAM("\n\t"));
            }
         else if (strncmp(directive, U_CONSTANT_TO_PARAM("header")) == 0)
            {
            U_ASSERT(http_header.empty())

            n = token.size() - U_CONSTANT_SIZE("header") - 2;

            http_header = UStringExt::trim(directive + U_CONSTANT_SIZE("header"), n);
            }
         else if (strncmp(directive, U_CONSTANT_TO_PARAM("session")) == 0)
            {
            bsession = true;

            n = token.size() - U_CONSTANT_SIZE("session") - 2;

            token = UStringExt::trim(directive + U_CONSTANT_SIZE("session"), n);

            if (token.empty())
               {
               (void) output.append( U_CONSTANT_TO_PARAM("\n\tif (UHTTP::getDataSession() == false) UHTTP::setSessionCookie();\n\t"));
               (void) output1.append(U_CONSTANT_TO_PARAM("\n\tUHTTP::putDataSession();\n\t"));
               }
            else
               {
               UString tmp, name;
               UVector<UString> vec(token, "\t\n;");

               bvar = true;

               (void) output.append(U_CONSTANT_TO_PARAM("\n\t"));
               (void) output.append(token);
               (void) output.append(U_CONSTANT_TO_PARAM("\n\t\n"));

               for (i = 0, n = vec.size(); i < n; ++i)
                  {
#              ifdef DEBUG
                  name.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#              endif
                  tmp = UStringExt::trim(vec[i]);
                  ptr = tmp.data();

                  do { ++ptr; } while (u__isspace(*ptr) == false);
                  do { ++ptr; } while (u__isspace(*ptr) == true);

                  name = tmp.substr(tmp.distance(ptr));
                  pos  = name.find('(');

                  size = (pos == U_NOT_FOUND ? name.size() : pos);

                  buffer.snprintf(U_CONSTANT_TO_PARAM("\n\tUSP_SESSION_VAR_GET(%u,%.*s);\n\t"), i, size, ptr);

                  (void) output.append(buffer);

                  output1.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUSP_SESSION_VAR_PUT(%u,%.*s);\n\t"), i, size, ptr);
                  }
               }
            }
         else if (strncmp(directive, U_CONSTANT_TO_PARAM("storage")) == 0)
            {
            bstorage = true;

            n = token.size() - U_CONSTANT_SIZE("storage") - 2;

            token = UStringExt::trim(directive + U_CONSTANT_SIZE("storage"), n);

            if (token.empty())
               {
               (void) output.append( U_CONSTANT_TO_PARAM("\n\t(void) UHTTP::getDataStorage();\n\t"));
               (void) output1.append(U_CONSTANT_TO_PARAM("\n\tUHTTP::putDataStorage();\n\t"));
               }
            else
               {
               UString tmp, name;
               UVector<UString> vec(token, "\t\n;");

               bvar = true;

               (void) output.append(U_CONSTANT_TO_PARAM("\n\t"));
               (void) output.append(token);
               (void) output.append(U_CONSTANT_TO_PARAM("\n\t\n"));

               for (i = 0, n = vec.size(); i < n; ++i)
                  {
#              ifdef DEBUG
                  name.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#              endif
                  tmp = UStringExt::trim(vec[i]);
                  ptr = tmp.data();

                  do { ++ptr; } while (u__isspace(*ptr) == false);
                  do { ++ptr; } while (u__isspace(*ptr) == true);

                  name = tmp.substr(tmp.distance(ptr));
                  pos  = name.find('(');

                  size = (pos == U_NOT_FOUND ? name.size() : pos);

                  buffer.snprintf(U_CONSTANT_TO_PARAM("\n\tUSP_STORAGE_VAR_GET(%u,%.*s);\n\t"), i, size, ptr);

                  (void) output.append(buffer);

                  output1.snprintf_add(U_CONSTANT_TO_PARAM("\n\tUSP_STORAGE_VAR_PUT(%u,%.*s);\n\t"), i, size, ptr);
                  }
               }
            }
         else if (strncmp(directive, U_CONSTANT_TO_PARAM("args")) == 0)
            {
            bform = true;

            n = token.size() - U_CONSTANT_SIZE("args") - 2;

            token = UStringExt::trim(directive + U_CONSTANT_SIZE("args"), n);

            UString tmp, name;
            UVector<UString> vec(token, "\t\n;");

            (void) buffer.reserve(token.size() * 100U);

            for (i = 0, n = vec.size(); i < n; ++i)
               {
#           ifdef DEBUG
               name.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#           endif
               tmp  = UStringExt::trim(vec[i]);
               pos  = tmp.find('(');
               name = (pos == U_NOT_FOUND ? tmp : tmp.substr(0U, pos));

               buffer.snprintf(U_CONSTANT_TO_PARAM("\n\tUString %v = USP_FORM_VALUE(%u);\n\t"), name.rep, i);

               if (pos != U_NOT_FOUND)
                  {
                  buffer.snprintf_add(U_CONSTANT_TO_PARAM("\n\tif (%v.empty()) %v = U_STRING_FROM_CONSTANT(%.*s);\n\t"),
                                      name.rep, name.rep, tmp.size() - pos - 2, tmp.c_pointer(pos + 1));  
                  }

               (void) output.append(buffer);
               }
            }
         else if (strncmp(directive, U_CONSTANT_TO_PARAM("number")) == 0)
            {
            n = token.size() - U_CONSTANT_SIZE("number") - 2;

            token = UStringExt::trim(directive + U_CONSTANT_SIZE("number"), n);

            (void) buffer.reserve(token.size() + 100U);

            buffer.snprintf(U_CONSTANT_TO_PARAM("\n\tUStringExt::appendNumber32(*UClientImage_Base::wbuffer, (%v));\n\t"), token.rep);

            (void) output.append(buffer);
            }
         else if (strncmp(directive, U_CONSTANT_TO_PARAM("puts")) == 0)
            {
            n = token.size() - U_CONSTANT_SIZE("puts") - 2;

            token = UStringExt::trim(directive + U_CONSTANT_SIZE("puts"), n);

            (void) buffer.reserve(token.size() + 100U);

            buffer.snprintf(U_CONSTANT_TO_PARAM("\n\t(void) UClientImage_Base::wbuffer->append((%v));\n\t"), token.rep);

            (void) output.append(buffer);
            }
         else if (strncmp(directive, U_CONSTANT_TO_PARAM("xmlputs")) == 0)
            {
            n = token.size() - U_CONSTANT_SIZE("xmlputs") - 2;

            token = UStringExt::trim(directive + U_CONSTANT_SIZE("xmlputs"), n);

            buffer.snprintf(U_CONSTANT_TO_PARAM("\n\tUSP_XML_PUTS((%v));\n\t"), token.rep);

            (void) output.append(buffer);
            }
         else if (strncmp(directive, U_CONSTANT_TO_PARAM("print")) == 0)
            {
            bvar = true;

            bool bfor = (strncmp(directive + U_CONSTANT_SIZE("print"), U_CONSTANT_TO_PARAM("for")) == 0);

            if (bfor)
               {
               n = token.size() - U_CONSTANT_SIZE("printfor") - 2;

               token = UStringExt::trim(directive + U_CONSTANT_SIZE("printfor"), n);
               }
            else
               {
               n = token.size() - U_CONSTANT_SIZE("print") - 2;

               token = UStringExt::trim(directive + U_CONSTANT_SIZE("print"), n);
               }

            UVector<UString> vec(token, ';');

            (void) buffer.reserve(token.size() + 200U);

            if (bfor)
               {
               buffer.snprintf(U_CONSTANT_TO_PARAM("\n\tfor (%v; %v; %v) { usp_sz = u__snprintf(usp_buffer, sizeof(usp_buffer), %v, %v);"
                               "(void) UClientImage_Base::wbuffer->append(usp_buffer, usp_sz); }\n\t"), vec[0].rep, vec[1].rep, vec[2].rep, vec[3].rep, vec[4].rep);
               }
            else
               {
               buffer.snprintf(U_CONSTANT_TO_PARAM("\n\tusp_sz = u__snprintf(usp_buffer, sizeof(usp_buffer), %v, %v);"
                               "(void) UClientImage_Base::wbuffer->append(usp_buffer, usp_sz);\n\t"), vec[0].rep, vec[1].rep);
               }

            (void) output.append(buffer);
            }
         else if (strncmp(directive, U_CONSTANT_TO_PARAM("cout")) == 0)
            {
            bvar = true;

            n = token.size() - U_CONSTANT_SIZE("cout") - 2;

            token = UStringExt::trim(directive + U_CONSTANT_SIZE("cout"), n);

            (void) buffer.reserve(token.size() + 200U);

            buffer.snprintf(U_CONSTANT_TO_PARAM("\n\tusp_sz = UObject2String((%v), usp_buffer, sizeof(usp_buffer));"
                            "\n\t(void) UClientImage_Base::wbuffer->append(usp_buffer, usp_sz);\n\t"), token.rep);

            (void) output.append(buffer);
            }
         else
            {
            n = 0;

            if (strncmp(directive, U_CONSTANT_TO_PARAM("pcode")) == 0)
               {
               bparallelization = true;

               n = token.size() - U_CONSTANT_SIZE("pcode") - 2;

               token = UStringExt::trim(directive + U_CONSTANT_SIZE("pcode"), n);
               }
            else if (strncmp(directive, U_CONSTANT_TO_PARAM("code")) == 0)
               {
               n = token.size() - U_CONSTANT_SIZE("code") - 2;

               token = UStringExt::trim(directive + U_CONSTANT_SIZE("code"), n);
               }
            else if (strncmp(directive, U_CONSTANT_TO_PARAM("xcode")) == 0)
               {
               bvar = true;

               token = UStringExt::trim(directive + U_CONSTANT_SIZE("xcode"), token.size() - U_CONSTANT_SIZE("xcode") - 2);

               (void) xoutput.append(U_CONSTANT_TO_PARAM("\n\t"));
               (void) xoutput.append(UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t")));
               (void) xoutput.append(U_CONSTANT_TO_PARAM("\n\t\n"));
               }

            if (n)
               {
               (void) output.append(U_CONSTANT_TO_PARAM("\n\t"));
               (void) output.append(UStringExt::substitute(token, '\n', U_CONSTANT_TO_PARAM("\n\t")));
               (void) output.append(U_CONSTANT_TO_PARAM("\n\t\n"));
               }
            }

         // no trailing \n...

         for (ptr = t.getPointer(); u__islterm(*ptr); ++ptr) {}

         t.setPointer(ptr);
         }

      UString basename = UStringExt::basename(filename);

      ptr  = basename.data();
      size = basename.size() - U_CONSTANT_SIZE(".usp");

      bufname.snprintf(U_CONSTANT_TO_PARAM("%.*s.cpp"), size, ptr);

      if (binit == false &&
          (bsession || bstorage))
         {
         binit = true;

         (void) buffer.reserve(500U);

         buffer.snprintf(U_CONSTANT_TO_PARAM(USP_SESSION_INIT),
                         size, ptr,
                         size, ptr,
                         (bsession ? "\n\tif (UHTTP::data_session == 0)   U_NEW(UDataSession, UHTTP::data_session, UDataSession);\n\t" : ""),
                         (bstorage ? "\n\tif (UHTTP::data_storage == 0) { U_NEW(UDataSession, UHTTP::data_storage, UDataSession(*UString::str_storage_keyid)); }\n\t" : ""));

         (void) declaration.append(buffer);
         }

      // NB: we check for HTML without HTTP headers...

      UString x(U_CAPACITY);

      if (http_header.empty())
         {
         if (is_html) (void) x.append(U_CONSTANT_TO_PARAM("\n\tUHTTP::mime_index = U_html;\n\t"));

         (void) x.append(U_CONSTANT_TO_PARAM("\n\tU_http_info.endHeader = 0;\n"));
         }
      else
         {
         // NB: we use insert because the possibility of UHTTP::callService() (see chat.usp)...

         if (U_STRING_FIND(http_header, 0, "Content-Type") != U_NOT_FOUND) (void) xoutput.append(U_CONSTANT_TO_PARAM("\n\t\tU_http_content_type_len = 1;\n\t"));

         UString header = UStringExt::dos2unix(http_header, true);

         (void) header.append(U_CONSTANT_TO_PARAM("\r\n\r\n"));

         n = header.size();

         U_INTERNAL_DUMP("n = %u", size)

         UString encoded(n * 4);

         UEscape::encode(header, encoded);

         U_ASSERT(encoded.isQuoted())

         UString tmp(encoded.size() + 200U);

         tmp.snprintf(U_CONSTANT_TO_PARAM("\n\t\tU_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false)"
                      "\n\t\tU_http_info.endHeader = %u;"
                      "\n\t\t(void) UClientImage_Base::wbuffer->insert(0, \n\tU_CONSTANT_TO_PARAM(%v));\n"), n, encoded.rep);

         (void) x.append(tmp);
         }

      http_header = x;

            char  ptr1[100] = { '\0' };
            char  ptr2[100] = { '\0' };
            char  ptr3[100] = { '\0' };
            char  ptr4[100] = { '\0' };
            char  ptr5[100] = { '\0' };
      const char* ptr6      = "";
      const char* ptr7      = "";
      const char* ptr8      = (bcomment ? "\n\tUClientImage_Base::setRequestNoCache();\n\t\n" : "");

      if (binit)   (void) u__snprintf(ptr1, 100, U_CONSTANT_TO_PARAM("\n\t\tif (param == U_DPAGE_INIT) { usp_init_%.*s(); return; }\n\t"), size, ptr);
      if (bsighup) (void) u__snprintf(ptr4, 100, U_CONSTANT_TO_PARAM("\n\t\tif (param == U_DPAGE_SIGHUP) { usp_sighup_%.*s(); return; }\n\t"), size, ptr);
      if (bfork)   (void) u__snprintf(ptr5, 100, U_CONSTANT_TO_PARAM("\n\t\tif (param == U_DPAGE_FORK) { usp_fork_%.*s(); return; }\n\t"), size, ptr);

      if (bend)
         {
#     ifndef DEBUG
         if (bpreprocessing_failed) bend = false;
         else
#     endif
         (void) u__snprintf(ptr3, 100, U_CONSTANT_TO_PARAM("\n\t\tif (param == U_DPAGE_DESTROY) { usp_end_%.*s(); return; }\n\t"), size, ptr);
         }

      if (binit   == false ||
          bend    == false ||
          bsighup == false ||
          bfork   == false)
         {
         ptr6 = (bfork ? "\n\t\tif (param >  U_DPAGE_FORK) return;\n"
                       : "\n\t\tif (param >= U_DPAGE_FORK) return;\n");
         }

      if (bparallelization) ptr7 = "\t\n\t\tif (UServer_Base::startParallelization(UServer_Base::num_client_for_parallelization)) return;\n\t\n";

      UString result(1024U + sizeof(USP_TEMPLATE) + declaration.size() + http_header.size() + output.size() + output1.size() + xoutput.size());

      result.snprintf(U_CONSTANT_TO_PARAM(USP_TEMPLATE),
                      size, ptr,
                      size, ptr,
                      size, ptr,
                      declaration.rep,
                      size, ptr,
                      size, ptr,
                      size, ptr,
                      bvar ? "\n\tuint32_t usp_sz = 0;"
                             "\n\tchar usp_buffer[10 * 4096];\n" : "",
                      ptr1,
                      ptr2,
                      ptr3,
                      ptr4,
                      ptr5,
                      ptr6,
                      ptr7,
                      http_header.rep,
                      bform ? "\t\n\t\tif (UHTTP::isGETorPOST()) (void) UHTTP::processForm();\n" : "",
                      output.rep,
                      output1.rep,
                      xoutput.rep,
                      ptr8);

      (void) UFile::writeTo(bufname, UStringExt::removeEmptyLine(result));
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(Application)
};

U_MAIN
