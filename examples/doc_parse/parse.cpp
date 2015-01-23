// parse.cpp

#include <ulib/command.h>
#include <ulib/zip/zip.h>
#include <ulib/ssl/crl.h>
#include <ulib/ui/dialog.h>
#include <ulib/ssl/pkcs10.h>
#include <ulib/ssl/certificate.h>
#include <ulib/utility/services.h>
#include <ulib/ssl/mime/mime_pkcs7.h>

#ifdef HAVE_SSL_TS
#  include <ulib/ssl/timestamp.h>
#endif

#undef  PACKAGE
#define PACKAGE "doc_parse"
#undef  ARGS
#define ARGS "document"

#define U_OPTIONS \
"purpose 'parse tbote document...'\n" \
"option e extract  1 'tag document to extract' ''\n" \
"option s css      1 'css url' ''\n" \
"option H htmlview 0 'html view flag' ''\n" \
"option X treeview 0 'tree view with Xdialog flag' ''\n" \
"option p inner_p7 0 'inner pkcs7 view flag' ''\n"

#include <ulib/application.h>

#define U_MAX_TAB 100

//#define U_PRINT_TAB

#ifdef U_PRINT_TAB
static char tab[U_MAX_TAB];
#endif

#define U_PRINT(str) print(str,U_CONSTANT_SIZE(str))

#define U_HTML_TEMPLATE_START \
"<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n" \
"<html>\n" \
"<head>\n" \
"  <title>doc_parse HTML View</title>\n" \

// NB: Two cursor properties are necessary because IE and Netscape use different identifiers...

#define U_HTML_CSS \
"<style type=\"text/css\">\n" \
"body {\n" \
"  font: 10pt Verdana,sans-serif;\n" \
"  color: navy;\n" \
"}\n" \
"table {\n" \
"  font: 70% Verdana,sans-serif;\n" \
"  color: navy;\n" \
"}\n" \
".trigger {\n" \
"  cursor: pointer;\n" \
"  cursor: hand;\n" \
"}\n" \
".branch {\n" \
"  display: none;\n" \
"  margin-left: 16px;\n" \
"}\n" \
"</style>\n"

#define U_HTML_JAVASCRIPT \
"<script language=\"JavaScript\" type=\"text/javascript\">\n" \
"var openImg   = new Image();\n" \
"openImg.src   = \"/icons/open.gif\";\n" \
"var closedImg = new Image();\n" \
"closedImg.src = \"/icons/closed.gif\";\n" \
"function showBranch(branch) {\n" \
"  var objBranch = document.getElementById(branch).style;\n" \
"  if (objBranch.display == \"block\") objBranch.display = \"none\";\n" \
"  else                                objBranch.display = \"block\";\n" \
"}\n" \
"function showAllBranch() {\n" \
"  var i, objBranch;\n" \
"  objBranch = document.getElementsByTagName('span');\n" \
"  for(i in objBranch) {\n" \
"    if (objBranch[i].style.display == \"block\") objBranch[i].style.display = \"none\";\n" \
"    else                                         objBranch[i].style.display = \"block\";\n" \
"  }\n" \
"}\n" \
"function swapAllFolder() {\n" \
"  var i, objImg;\n" \
"  objImg = document.getElementsByTagName('img');\n" \
"  for(i in objImg) {\n" \
"    if (objImg[i].src.indexOf('/icons/closed.gif') > -1) objImg[i].src =   openImg.src;\n" \
"    else                                                 objImg[i].src = closedImg.src;\n" \
"  }\n" \
"}\n" \
"function swapFolder(img) {\n" \
"  objImg = document.getElementById(img);\n" \
"  if (objImg.src.indexOf('/icons/closed.gif') > -1) objImg.src =   openImg.src;\n" \
"  else                                              objImg.src = closedImg.src;\n" \
"}\n" \
"</script>\n"

#define U_HTML_TEMPLATE_END \
"</head>\n" \
"<body onload=\"showAllBranch();swapAllFolder()\">\n\n"

#define U_HTML_FOLDER_TEMPLATE \
"<div class=\"trigger\" onclick = \"showBranch('%s'); swapFolder('%s')\">\n" \
"<img src=\"/icons/closed.gif\" border=\"0\" id=\"%s\" name=\"%s\"> %.*s<br></div>\n" \
"<span class=\"branch\" id=\"%s\">\n" \
"\n"

#define U_HTML_ELEMENT_TEMPLATE \
"<img src=\"%s\"><a href=\"?filename=%s&tag=%s\"> %.*s</a><br>\n"

static UString* row;
static UString* pfile;
static UString* stype;
static UString* output;
static UDialog* dialog;
static UString* content;
static UString* extract;
static UString* css_url;
static const char* icon;
static UCommand* xsltproc;
static char* u_buffer_ptr;
static const char* filename;
static UString* xsltproc_cmd;
static UString* xsltproc_out;
static UVector<UString>* vec;
static const char* extract_tag;
static bool binary, treeview, htmlview, inner_p7;
static uint32_t old_buffer_len, num_tab, old_tab, num_tag;
static char buffer[4096], old_buffer[4096], prefix[1 + U_MAX_TAB * 2] = { '1', '\0' }, old_prefix[1 + U_MAX_TAB * 2];

class Application : public UApplication {
public:

   ~Application()
      {
      U_TRACE(5, "Application::~Application()")

      if (row)          delete row;
      if (vec)          delete vec;
      if (pfile)        delete pfile;
      if (stype)        delete stype;
      if (dialog)       delete dialog;
      if (output)       delete output;
      if (extract)      delete extract;
      if (xsltproc)     delete xsltproc;
      if (xsltproc_cmd) delete xsltproc_cmd;
      if (xsltproc_out) delete xsltproc_out;
//                      delete content;
      }

   // HTML representation functions

   static void setElementData()
      {
      U_TRACE(5, "Application::setElementData()")

      icon = "/icons/doc.gif";

      U_INTERNAL_DUMP("stype = %.*S", U_STRING_TO_TRACE(*stype))

      if (*stype)
         {
         const char* type = stype->data();

         if      (strncmp(type, U_CONSTANT_TO_PARAM("image/")) == 0)                       icon = "/icons/img.png";
         else if (strncmp(type, U_CONSTANT_TO_PARAM("application/x-")) == 0)               icon = "/icons/x.gif";
         else if (strncmp(type, U_CONSTANT_TO_PARAM("application/pdf")) == 0)              icon = "/icons/pdf.gif";
         else if (strncmp(type, U_CONSTANT_TO_PARAM("application/timestamp-reply")) == 0)  icon = "/icons/timestamp.gif";
         else if (strncmp(type, U_CONSTANT_TO_PARAM("text/xml")) == 0)
            {
            icon = "/icons/xml.gif";

            if (U_STRING_FIND(*content, 0, "<?xml-stylesheet type=\"text/xsl\"") != U_NOT_FOUND)
               {
               U_INTERNAL_DUMP("FOUND XML WITH stylesheet")

               static int fd_stderr;

               if (xsltproc_cmd == 0)
                  {
                  if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/xsltproc.err");

                  row          = U_NEW(UString);
                  vec          = U_NEW(UVector<UString>);
                  xsltproc     = U_NEW(UCommand);
                  xsltproc_cmd = U_NEW(UString("xsltproc -"));
                  xsltproc_out = U_NEW(UString(U_CAPACITY));

                  xsltproc->set(*xsltproc_cmd, (char**)0);
                  }

               if (xsltproc->getCommand()) (void) xsltproc->execute(content, xsltproc_out, -1, fd_stderr);
               else
                  {
                  U_WARNING("command xsltproc not found...");
                  }
               }
            }
         }

      // NB: we need to remember the previous element to discriminate if current element is structured (folder)...

      u__strcpy(old_prefix, prefix);

      U_MEMCPY(old_buffer, u_buffer_ptr, (old_buffer_len = u_buffer_len));
      }

   static void printHTMLElement()
      {
      U_TRACE(5, "Application::printHTMLElement()")

      uint32_t len = u__snprintf(buffer, sizeof(buffer), U_HTML_ELEMENT_TEMPLATE, icon, filename, old_prefix, old_buffer_len, old_buffer);

      std::cout.write(buffer, len);

      if ( xsltproc_out &&
          *xsltproc_out)
         {
         uint32_t i = 1, n = vec->split(*xsltproc_out, '\n') - 1;

         for (; i < n; ++i)
            {
            *row = (*vec)[i];

            std::cout.write(row->data(), row->size());
            std::cout.put('\n');
            }

         std::cout.write(U_CONSTANT_TO_PARAM("</br>\n"));

         row->clear();
         vec->clear();
         xsltproc_out->clear();
         }
      }

   static void printHTMLFolder()
      {
      U_TRACE(5, "Application::printHTMLFolder()")

      U_INTERNAL_ASSERT_MAJOR(old_buffer_len,0)

      char branch[10], folder[10];

      (void) u__snprintf(branch, sizeof(branch), "branch%02u", num_tag),
      (void) u__snprintf(folder, sizeof(folder), "folder%02u", num_tag++);

      uint32_t len = u__snprintf(buffer, sizeof(buffer), U_HTML_FOLDER_TEMPLATE, branch, folder, folder, folder, old_buffer_len, old_buffer, branch);

      std::cout.write(buffer, len);
      }

   static void printHTMLClose(uint32_t n)
      {
      U_TRACE(5, "Application::printHTMLClose(%u)", n)

      for (uint32_t i = 0; i < n; ++i) std::cout.write(U_CONSTANT_TO_PARAM("</span>\n"));
      }

   // representation function

   static void print()
      {
      U_TRACE(5, "Application::print()")

      char tag[10];
      uint32_t nt = 0;

      if (treeview ||
          extract_tag)
         {
         nt = u__snprintf(tag, sizeof(tag), " tag%03u \"", num_tag++);

         if (extract_tag &&
             strncmp(extract_tag, tag + sizeof("tag"), 3) == 0)
            {
         // std::cout.write(content->data(), content->size());

            u_buffer_len = 0;

            dialog->setSize(60, 80);

            if (dialog->fselect(*pfile, "Please choose where to extract the selected part")) (void) UFile::writeTo(*pfile, *content);

            U_EXIT(0);
            }
         }

      U_INTERNAL_DUMP("num_tab = %u old_tab = %u", num_tab, old_tab)

      U_INTERNAL_ASSERT_MINOR(num_tab,U_MAX_TAB)

      // convert TAB to PREFIX (tag for extraction)...

      int skip         = (num_tab == 0),
          change_level = 0;

      if (skip ||
          num_tab > old_tab)
         {
         if (skip == false) // num_tab > old_tab
            {
            change_level = 1;

            (void) strcat(prefix, ".1");

            old_tab = num_tab;
            }
         }
      else // num_tab <= old_tab
         {
         if (num_tab < old_tab)
            {
            change_level = num_tab - old_tab;

            U_INTERNAL_DUMP("prefix = %S", prefix)

            uint32_t num_points = 0;
            char* end           = prefix + sizeof(prefix);

            for (char* ptr = prefix + 2; ptr < end; ++ptr)
               {
               if (*ptr == '.' &&
                   ++num_points == num_tab)
                  {
                  *ptr = '\0';

                  break;
                  }
               }

            old_tab = num_tab;
            }

         if (num_tab == old_tab)
            {
            char* ptr = strrchr(prefix, '.');

            int n = atoi(++ptr);

            (void) sprintf(ptr, "%d", ++n);
            }
         }

      U_INTERNAL_DUMP("change_level = %d prefix = %S old_buffer = %.*S u_buffer_ptr = %.*S",
                       change_level, prefix, old_buffer_len, old_buffer, u_buffer_len, u_buffer_ptr)

      if (extract)
         {
         if (extract->equal(prefix))
            {
            std::cout.write(content->data(), content->size());

            U_EXIT(0);
            }
         }
      else if (treeview || htmlview)
         {
         // convert '"' -> '\''

         char* end = u_buffer_ptr + u_buffer_len;

         for (char* ptr = u_buffer_ptr; ptr < end; ++ptr) if (*ptr == '"') *ptr = '\'';

         if (treeview)
            {
            char depth[10];
            uint32_t nd = u__snprintf(depth, sizeof(depth), "\" on %u", num_tab);

            (void) output->append(tag, nt);
            (void) output->append(u_buffer_ptr, u_buffer_len);
            (void) output->append(depth, nd);
            }
         else // htmlview
            {
            if (skip ||
                change_level == 1) // num_tab > old_tab
               {
               if (skip == false) printHTMLFolder();
               }
            else // num_tab <= old_tab
               {
               printHTMLElement();

               if (change_level < 0) // num_tab < old_tab
                  {
                  printHTMLClose(-change_level);
                  }
               }

            // NB: we need to remember the previous element to discriminate if current element is structured (folder)...

            setElementData();
            }
         }
      else
         {
#     ifdef U_PRINT_TAB
         struct iovec iov[3] = { { (caddr_t)tab, num_tab },
                                 { (caddr_t)u_buffer_ptr, u_buffer_len },
                                 { (caddr_t)"\n", 1 } };

         UFile::writev(STDOUT_FILENO, iov+skip, 3-skip);
#     else
         std::cout << prefix << ' ';
         std::cout.write(u_buffer_ptr, u_buffer_len);
         std::cout << '\n';
#     endif
         }

      stype->clear();
      pfile->clear();

      u_buffer_len = 0;

      ++num_tab;
      }

   static void print(const char* s, uint32_t len)
      {
      U_TRACE(5, "Application::print(%.*S,%u)", len, s, len)

      u_buffer_ptr = (char*)s;
      u_buffer_len = len;

      print();
      }

   static void print(const UString& description) { print(description.data(), description.size()); }

   static void vprint(const char* format, ...)
      {
      U_TRACE(5, "Application::vprint(%S)", format)

      va_list argp;
      va_start(argp, format);

      u_buffer_len = u__vsnprintf((u_buffer_ptr = u_buffer), U_BUFFER_SIZE, format, argp);

      va_end(argp);

      print();
      }

   static void manageDescription(UMimeEntity* uMimeEntity, uint32_t i)
      {
      U_TRACE(5, "Application::manageDescription(%p,%u)", uMimeEntity, i)

      UString x(20U), description(U_CAPACITY);

      if (i == 0) (void) x.assign("MIME");
      else               x.snprintf("Part %d", i);

      *stype = uMimeEntity->shortContentType();

      description.snprintf("%.*s: Content-type='%.*s'", U_STRING_TO_TRACE(x), U_STRING_TO_TRACE(*stype));

      if (uMimeEntity->isBodyMessage()) (void) description.append(U_CONSTANT_TO_PARAM(" - BODY MESSAGE"));
      else
         {
         *pfile = uMimeEntity->getFileName();

         if (*pfile) description.snprintf_add(" - Filename='%.*s'", U_STRING_TO_TRACE(*pfile));
         }

      print(description);
      }

   static void parseMime(UMimeEntity& entity)
      {
      U_TRACE(5, "Application::parseMime(%p)", &entity)

      if (entity.isRFC822())
         {
         U_PRINT("MIME: rfc822");

         UMimeMessage tmp(entity);
         UMimeEntity& rfc822 = tmp.getRFC822();

         parseMime(rfc822);
         }
      else if (entity.isPKCS7())
         {
         U_PRINT("MIME: smime signed");

         UMimePKCS7 tmp(entity);

         *content = tmp.getContent();
         binary   = content->isBinary();

         parse();
         }
      else if (entity.isMultipart())
         {
         UMimeEntity* uMimeEntity;
         UMimeMultipart tmp(entity);
         uint32_t i = 0, bodyPartCount = tmp.getNumBodyPart();

         vprint("MIME: multipart - %d parts", bodyPartCount);

         content->hold(); // NB: si incrementa la reference della stringa...

         while (i < bodyPartCount)
            {
            uMimeEntity = tmp[i];

            *content = (uMimeEntity->isMultipart() ? uMimeEntity->getData()
                                                   : uMimeEntity->getContent());

            manageDescription(uMimeEntity, ++i);

#        ifdef HAVE_SSL_TS
            if (*stype == U_STRING_FROM_CONSTANT("application/timestamp-reply") && UTimeStamp::isTimeStampResponse(*content))
#        else
            if (*stype == U_STRING_FROM_CONSTANT("application/timestamp-reply"))
#        endif
               {
               U_PRINT("TIMESTAMP_RESPONSE");
               }
            else
               {
               binary = content->isBinary();

               parse();
               }

            --num_tab;
            }

      // content->release(); // NB: si decrementa la reference della stringa...
         }
      else
         {
         (void) manageDescription(&entity, 0);
         }

      --num_tab;
      }

   static void parse()
      {
      U_TRACE(5, "Application::parse()")

      U_INTERNAL_DUMP("binary = %b content = %.*S", binary, U_STRING_TO_TRACE(*content))

      if (binary)
         {
         UZIP zip(*content);

         if (zip.isValid() &&
             zip.readContent())
            {
            UString namefile;
            uint32_t count = zip.getFilesCount();

            vprint("ZIP: %d parts", count);

            for (uint32_t i = 0; i < count; ++i)
               {
               namefile = zip.getFilenameAt(i);

               *content = zip.getFileContentAt(i);
               binary   = content->isBinary();

               vprint("Part %d: Filename='%.*s'", i+1, U_STRING_TO_TRACE(namefile));

               parse();

               --num_tab;
               }

            --num_tab;

            return;
            }
         }
      else if (content->empty())
         {
         U_PRINT("EMPTY");

         --num_tab;

         return;
         }
      else if (content->isWhiteSpace())
         {
         U_PRINT("WHITE_SPACES");

         --num_tab;

         return;
         }
      else
         {
         UMimeEntity entity(*content);

         if (entity.isParsingOk())
            {
            parseMime(entity);

            return;
            }
         }

      if (inner_p7)
         {
         UPKCS7 p7(*content, (binary ? "DER" : "PEM"));

         if (p7.isValid())
            {
            U_PRINT("PKCS7");

            *content = p7.getContent();
            binary   = content->isBinary();

            parse();

            --num_tab;

            return;
            }
         }
      }

   static bool loadDocument()
      {
      U_TRACE(5, "Application::loadDocument()")

      *content = UFile::contentOf(filename);

      if (content->empty())
         {
         U_ERROR("EMPTY OR UNEXISTANT FILE");

         U_RETURN(false);
         }

      binary = content->isBinary();

      U_RETURN(true);
      }

   static bool checkIfNeedParsing()
      {
      U_TRACE(5, "Application::checkIfNeedParsing()")

      UPKCS7 p7(*content, (binary ? "DER" : "PEM"));

      if (p7.isValid())
         {
#     ifdef HAVE_SSL_TS
         if (UTimeStamp::isTimeStampToken(p7.getPKCS7()))
            {
            U_PRINT("TIMESTAMP_TOKEN");

            U_RETURN(false);
            }
#     endif

         U_PRINT("PKCS7");

         *content = p7.getContent();
         binary   = content->isBinary();

         if (binary == false) U_RETURN(true);
         }
      else if (UCertificate(*content).isValid()) U_PRINT("CERTIFICATE");
      else if (     UPKCS10(*content).isValid()) U_PRINT("PKCS10");
      else if (        UCrl(*content).isValid()) U_PRINT("CRL");
      else if (binary)                           U_PRINT("UNKNOW BINARY OBJECT");
      else                                       U_RETURN(true);

      U_RETURN(false);
      }

   void run(int argc, char* argv[], char* env[])
      {
      U_TRACE(5, "Application::run(%d,%p,%p)", argc, argv, env)

      UApplication::run(argc, argv, env);

      // manage options

      if (UApplication::isOptions())
         {
         UString tmp = U_STRING_FROM_CONSTANT("1");

         inner_p7 = (opt['p'] == tmp);
         treeview = (opt['X'] == tmp);
         htmlview = (opt['H'] == tmp);

         tmp = opt['e'];

         if (tmp) extract = U_NEW(UString(tmp));

         tmp = opt['s'];

         if (tmp) css_url = U_NEW(UString(tmp));
         }

      // manage arg operation

      filename = argv[optind];

      if (filename == 0) U_ERROR("document not specified");

      U_INTERNAL_DUMP("htmlview = %b treeview = %b inner_p7 = %b", htmlview, treeview, inner_p7)

      content = U_NEW(UString);

      if (loadDocument() &&
          checkIfNeedParsing())
         {
         if (treeview)
            {
            dialog = U_NEW(UDialog(0, 24, 80));

            if (UDialog::isXdialog() == false) U_ERROR("I don't find Xdialog");

            output = U_NEW(UString(U_CAPACITY));
            }
         else if (htmlview)
            {
            std::cout.write(U_CONSTANT_TO_PARAM(U_HTML_TEMPLATE_START));

            if (css_url)
               {
               std::cout.write(U_CONSTANT_TO_PARAM("<link type=\"text/css\" href=\""));
               std::cout.write(U_STRING_TO_PARAM(*css_url));
               std::cout.write(U_CONSTANT_TO_PARAM("\" rel=\"stylesheet\">\n"));
               }
            else
               {
               std::cout.write(U_CONSTANT_TO_PARAM(U_HTML_CSS));
               }

            std::cout.write(U_CONSTANT_TO_PARAM(U_HTML_JAVASCRIPT));

            std::cout.write(U_CONSTANT_TO_PARAM(U_HTML_TEMPLATE_END));
            }

#     ifdef U_PRINT_TAB
         (void) U_SYSCALL(memset, "%p,%d,%u", tab, '\t', U_MAX_TAB);
#     endif

         pfile = U_NEW(UString);
         stype = U_NEW(UString);

#     ifdef DEBUG
      // UStringRep::check_dead_of_source_string_with_child_alive = false;
#     endif

         parse();

         if (old_tab == 0)
            {
            U_PRINT("UNKNOW NOT BINARY OBJECT");
            }
         else if (htmlview)
            {
            printHTMLElement();
            printHTMLClose(old_tab);

            std::cout.write(U_CONSTANT_TO_PARAM("\n</body>\n</html>"));
            }
         else if (treeview)
            {
            dialog->setArgument(*output);
            dialog->setOptions(U_STRING_FROM_CONSTANT("--icon ./logo.png"));

            if (dialog->tree(filename, 0, 0, 0, 0, 0, 0, "TBote Viewer") != -1)
               {
               dialog->setSize(10, 70);

               if (dialog->yesno("Do you want to extract the selected part of the file ?"))
                  {
                  num_tag     = 0;
                  extract_tag = dialog->output.c_pointer(U_CONSTANT_SIZE("tag"));

                  if (loadDocument()) parse();
                  }
               }
            }
         }
      }
};

U_MAIN
