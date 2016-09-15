// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_ssi.cpp - this is a plugin SSI for userver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/url.h>
#include <ulib/date.h>
#include <ulib/command.h>
#include <ulib/tokenizer.h>
#include <ulib/file_config.h>
#include <ulib/utility/uhttp.h>
#include <ulib/utility/escape.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/xml_escape.h>
#include <ulib/utility/string_ext.h>
#include <ulib/net/server/plugin/mod_ssi.h>

/**
 * Server Side Include (SSI) commands are executed by the server as it parses your HTML file. Server side includes can be used to include
 * the value of various server environment variables within your HTML such as the local date and time. One might use a server side include
 * to add a signature file to an HTML file or company logo for example
 */

U_CREAT_FUNC(server_plugin_ssi, USSIPlugIn)

int      USSIPlugIn::alternative_response;
bool     USSIPlugIn::use_size_abbrev;
time_t   USSIPlugIn::last_modified;
UString* USSIPlugIn::errmsg;
UString* USSIPlugIn::timefmt;
UString* USSIPlugIn::docname;
UString* USSIPlugIn::body;
UString* USSIPlugIn::header;
UString* USSIPlugIn::environment;
UString* USSIPlugIn::alternative_include;

USSIPlugIn::USSIPlugIn()
{
   U_TRACE_REGISTER_OBJECT(0, USSIPlugIn, "")

   U_NEW(UString, errmsg,  UString);
   U_NEW(UString, timefmt, UString);
   U_NEW(UString, docname, UString);

   UString::str_allocate(STR_ALLOCATE_SSI);
}

USSIPlugIn::~USSIPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, USSIPlugIn)

   delete errmsg;
   delete timefmt;
   delete docname;

   if (body)
      {
      delete body;  
      delete header;
      delete alternative_include;
      }

   if (environment) delete environment;
}

void USSIPlugIn::setAlternativeRedirect(const char* fmt, ...)
{
   U_TRACE(0, "USSIPlugIn::setAlternativeRedirect(%S)", fmt)

   char format[4096];
   UString buffer(U_CAPACITY);

   va_list argp;
   va_start(argp, fmt);

   buffer.vsnprintf(format, u__snprintf(format, sizeof(format), U_CONSTANT_TO_PARAM(U_CTYPE_HTML "\r\nRefresh: 0; url=%s\r\n"), fmt), argp);

   va_end(argp);

   alternative_response      = 1;
   U_http_info.nResponseCode = HTTP_OK;

   UClientImage_Base::setCloseConnection();

   UHTTP::setResponse(buffer, 0);
}

void USSIPlugIn::setBadRequest()
{
   U_TRACE_NO_PARAM(0, "USSIPlugIn::setBadRequest()")

   alternative_response = 1;

   UHTTP::setBadRequest();
}

void USSIPlugIn::setAlternativeResponse()
{
   U_TRACE_NO_PARAM(0, "USSIPlugIn::setAlternativeResponse()")

   alternative_response = 1;

   U_http_info.nResponseCode = HTTP_NO_CONTENT;

   UClientImage_Base::setCloseConnection();

   UHTTP::setResponse();
}

void USSIPlugIn::setAlternativeResponse(UString& _body)
{
   U_TRACE(0, "USSIPlugIn::setAlternativeResponse(%V)", _body.rep)

   alternative_response = 1;

   UClientImage_Base::setCloseConnection();

   if (_body.empty())
      {
      U_http_info.nResponseCode = HTTP_NO_CONTENT;

      UHTTP::setResponse();
      }
   else
      {
      U_http_info.nResponseCode = HTTP_OK;

      UHTTP::setResponse(*(u_is_know(UHTTP::mime_index) ? UString::str_ctype_txt : UString::str_ctype_html), &_body);
      }
}

void USSIPlugIn::setAlternativeInclude(const char* title_txt, const UString& output)
{
   U_TRACE(0, "USSIPlugIn::setAlternativeInclude(%S,%V)", title_txt, output.rep)

   U_INTERNAL_ASSERT_POINTER(title_txt)

   *alternative_include = output;

   U_http_info.nResponseCode = HTTP_NO_CONTENT;

   UString buffer(U_CAPACITY);

   buffer.snprintf(U_CONSTANT_TO_PARAM("'TITLE_TXT=%s'\n"), title_txt);

   (void) UClientImage_Base::environment->append(buffer);

   UClientImage_Base::wbuffer->setBuffer(U_CAPACITY); // NB: to avoid append on output...
}

void USSIPlugIn::setAlternativeInclude(const UString& tmpl, uint32_t estimated_size, bool bprocess, const char* title_txt, const char* ssi_head, const char* body_style, ...)
{
   U_TRACE(0, "USSIPlugIn::setAlternativeInclude(%V,%u,%b,%S,%S,%S)", tmpl.rep, estimated_size, bprocess, title_txt, ssi_head, body_style)

   U_INTERNAL_ASSERT_POINTER(title_txt)
   U_INTERNAL_ASSERT(tmpl.isNullTerminated())

   estimated_size += tmpl.size() + 4096;

   alternative_include->setBuffer(estimated_size);

   va_list argp;
   va_start(argp, body_style);

   alternative_include->vsnprintf(U_STRING_TO_PARAM(tmpl), argp);

   va_end(argp);

   U_http_info.nResponseCode = HTTP_NO_CONTENT;

   UString buffer(U_CAPACITY);

                   buffer.snprintf(    U_CONSTANT_TO_PARAM("'TITLE_TXT=%s'\n"), title_txt);
   if (ssi_head)   buffer.snprintf_add(U_CONSTANT_TO_PARAM("'SSI_HEAD=%s'\n"),  ssi_head);
   if (body_style) buffer.snprintf_add(U_CONSTANT_TO_PARAM("BODY_STYLE=%s\n"),  body_style);

   (void) UClientImage_Base::environment->append(buffer);

   if (bprocess) *alternative_include = processSSIRequest(*alternative_include, 0);

   UClientImage_Base::wbuffer->setBuffer(U_CAPACITY); // NB: to avoid append on output...
}

void USSIPlugIn::setMessagePageWithVar(const UString& tmpl, const char* title_txt, const char* fmt, uint32_t fmt_size, ...)
{
   U_TRACE(0, "USSIPlugIn::setMessagePageWithVar(%V,%S,%.*S,%u)", tmpl.rep, title_txt, fmt_size, fmt, fmt_size)

   char format[4096];

   va_list argp;
   va_start(argp, fmt_size);

   (void) u__vsnprintf(format, sizeof(format), fmt, fmt_size, argp);

   va_end(argp);

   setMessagePage(tmpl, title_txt, format);
}

U_NO_EXPORT UString USSIPlugIn::getPathname(const UString& name, const UString& value, const UString& directory)
{
   U_TRACE(0, "USSIPlugIn::getPathname(%V,%V,%V)", name.rep, value.rep, directory.rep)

   /**
    * "include file" looks in the current directory (the name must not start with /, ., or ..) and "include virtual" starts
    * in the root directory of your kiosk (so the name must start with "/".) You might want to make a "/includes" directory
    * in your Kiosk and then you can say "include virtual=/includes/file.txt" from any page. The "virtual" and "file"
    * parameters are also used with "fsize" and "flastmod". With either method, you can only reference files that are within
    * your Kiosk directory (apart if "direct"...)
    */

   UString pathname;

        if (name.equal(U_CONSTANT_TO_PARAM("direct")))  pathname =                    value;
   else if (name.equal(U_CONSTANT_TO_PARAM("file")))    pathname = directory + '/'  + value;
   else if (name.equal(U_CONSTANT_TO_PARAM("virtual"))) pathname =             "./" + value;

   U_RETURN_STRING(pathname);
}

U_NO_EXPORT UString USSIPlugIn::getInclude(const UString& include, int include_level, bool bssi)
{
   U_TRACE(0, "USSIPlugIn::getInclude(%V,%d,%b)", include.rep, include_level, bssi)

   UString content = include;

   U_INTERNAL_DUMP("file = %.*S", U_FILE_TO_TRACE(*UHTTP::file))

   if (bssi ||
       UStringExt::endsWith(U_FILE_TO_PARAM(*UHTTP::file), U_CONSTANT_TO_PARAM(".shtml")))
      {
      if (include_level < 16) content = processSSIRequest(content, include_level + 1);
      else
         {
         U_SRV_LOG("WARNING: SSI #include level is too deep (%.*S)", U_FILE_TO_TRACE(*UHTTP::file));
         }
      }

   U_RETURN_STRING(content);
}

U_NO_EXPORT bool USSIPlugIn::callService(const UString& name, const UString& value)
{
   U_TRACE(0, "USSIPlugIn::callService(%V,%V)", name.rep, value.rep)

   if (name.equal(U_CONSTANT_TO_PARAM("cmd")))
      {
      static int fd_stderr;

      if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/SSI.err");

      *UClientImage_Base::wbuffer = UCommand::outputCommand(value, 0, -1, fd_stderr);

      U_RETURN(true);
      }

   U_ASSERT(name == *UString::str_cgi ||
            name.equal(U_CONSTANT_TO_PARAM("servlet")))

   UClientImage_Base::wbuffer->setBuffer(U_CAPACITY); // NB: we need this to avoid to accumulate output from services...

   bool result = UHTTP::callService(value);

#ifdef DEBUG // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
   UHTTP::file->getPath().clear();
#endif

   if (result == false      ||
       alternative_response ||
       U_ClientImage_parallelization == U_PARALLELIZATION_PARENT)
      {
      alternative_response = 1; // 1 => response already complete (nothing to do)

      U_RETURN(false);
      }

   U_RETURN(true);
}

U_NO_EXPORT UString USSIPlugIn::processSSIRequest(const UString& content, int include_level)
{
   U_TRACE(0, "USSIPlugIn::processSSIRequest(%V,%d)", content.rep, include_level)

   U_INTERNAL_ASSERT(content)

   UString tmp; // NB: must be here to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
   UVector<UString> name_value;
   uint32_t size, sz = content.size();
   enum {E_NONE, E_URL, E_ENTITY} encode;
   int if_level = 0, if_is_false_level = 0;
   bool bgroup, bfile, bvar, if_is_false = false, if_is_false_endif = false;
   UString token, name, value, pathname, include, output(U_CAPACITY), x, encoded,
           directory = UStringExt::dirname(UHTTP::file->getPath()).copy();

   UTokenizer t(content);
   t.setGroup(U_CONSTANT_TO_PARAM("<!--->"));

   enum { SSI_NONE, SSI_INCLUDE, SSI_EXEC, SSI_ECHO, SSI_CONFIG, SSI_FLASTMOD,
          SSI_FSIZE, SSI_PRINTENV, SSI_SET, SSI_IF, SSI_ELSE, SSI_ELIF, SSI_ENDIF };

   do {
      // Find next SSI tag

      uint32_t distance = t.getDistance(),
               pos      = U_STRING_FIND(content, distance, "<!--#");

      if (pos)
         {
         if (pos == U_NOT_FOUND) pos = sz;

         t.setDistance(pos);

         size = pos - distance;

         if (size &&
             if_is_false == false)
            {
            (void) output.append(content.substr(distance, size)); // plain html block
            }
         }

      if (t.next(token, &bgroup) == false) break;

      U_INTERNAL_ASSERT(bgroup)

      U_INTERNAL_DUMP("token = %V", token.rep)

      U_ASSERT(token.size() >= 2)

      int i = 2;

      const char* directive = token.c_pointer(2); // "-#"...

      U_INTERNAL_DUMP("directive = %.*S", 10, directive)

      /**
       * <!--#element attribute=value attribute=value ... -->
       *
       * echo         DONE
       *   var        DONE
       *   encoding   DONE
       * set          DONE
       *   var        DONE
       *   value      DONE
       * exec         DONE
       *   cmd        DONE
       *   cgi        DONE
       *   servlet    DONE
       * include      DONE
       *   file       DONE
       *   direct     DONE
       *   virtual    DONE
       * fsize        DONE
       *   file       DONE
       *   direct     DONE
       *   virtual    DONE
       * flastmod     DONE
       *   file       DONE
       *   direct     DONE
       *   virtual    DONE
       * config       DONE
       *   errmsg     DONE
       *   sizefmt    DONE
       *   timefmt    DONE
       * printenv     DONE
       *
       * if           DONE
       * elif         DONE
       * else         DONE
       * endif        DONE
       *
       * expressions
       * &&, ||       DONE
       * comp         DONE
       * ${...}       DONE
       * $...         DONE
       * '...'        DONE
       * ( ... )      DONE
       */

      // Check element

      int op = SSI_NONE;

      if (u_get_unalignedp16(directive) == U_MULTICHAR_CONSTANT16('i','f'))
         {
         U_INTERNAL_ASSERT_EQUALS(directive[2], ' ')

         op = SSI_IF;
         i += U_CONSTANT_SIZE("if ");
         }
      else if (u_get_unalignedp32(directive) == U_MULTICHAR_CONSTANT32('s','e','t',' '))
         {
         op = SSI_SET;
         i += U_CONSTANT_SIZE("set ");
         }
      else if (u_get_unalignedp32(directive) == U_MULTICHAR_CONSTANT32('e','l','s','e'))
         {
         U_INTERNAL_ASSERT_EQUALS(directive[4], ' ')

         op = SSI_ELSE;
         i += U_CONSTANT_SIZE("else ");
         }
      else if (u_get_unalignedp32(directive) == U_MULTICHAR_CONSTANT32('e','l','i','f'))
         {
         U_INTERNAL_ASSERT_EQUALS(directive[4], ' ')

         op = SSI_ELIF;
         i += U_CONSTANT_SIZE("elif ");
         }
      else if (u_get_unalignedp32(directive) == U_MULTICHAR_CONSTANT32('e','x','e','c'))
         {
         U_INTERNAL_ASSERT_EQUALS(directive[4], ' ')

         op = SSI_EXEC;
         i += U_CONSTANT_SIZE("exec ");
         }
      else if (u_get_unalignedp32(directive) == U_MULTICHAR_CONSTANT32('e','c','h','o'))
         {
         U_INTERNAL_ASSERT_EQUALS(directive[4], ' ')

         op = SSI_ECHO;
         i += U_CONSTANT_SIZE("echo ");
         }
      else if (u_get_unalignedp32(directive)   == U_MULTICHAR_CONSTANT32('e','n','d','i') &&
               u_get_unalignedp16(directive+4) == U_MULTICHAR_CONSTANT16('f',' '))
         {
         op = SSI_ENDIF;
         i += U_CONSTANT_SIZE("endif ");
         }
      else if (u_get_unalignedp32(directive)   == U_MULTICHAR_CONSTANT32('f','s','i','z') &&
               u_get_unalignedp16(directive+4) == U_MULTICHAR_CONSTANT16('e',' '))
         {
         op = SSI_FSIZE;
         i += U_CONSTANT_SIZE("fsize ");
         }
      else if (u_get_unalignedp32(directive)   == U_MULTICHAR_CONSTANT32('c','o','n','f') &&
               u_get_unalignedp16(directive+4) == U_MULTICHAR_CONSTANT16('i','g'))
         {
         U_INTERNAL_ASSERT_EQUALS(directive[6], ' ')

         op = SSI_CONFIG;
         i += U_CONSTANT_SIZE("config ");
         }
      else if (u_get_unalignedp64(directive) == U_MULTICHAR_CONSTANT64('i','n','c','l','u','d','e',' '))
         {
         op = SSI_INCLUDE;
         i += U_CONSTANT_SIZE("include ");
         }
      else if (u_get_unalignedp64(directive) == U_MULTICHAR_CONSTANT64('f','l','a','s','t','m','o','d'))
         {
         U_INTERNAL_ASSERT_EQUALS(directive[8], ' ')

         op = SSI_FLASTMOD;
         i += U_CONSTANT_SIZE("flastmod ");
         }
      else if (u_get_unalignedp64(directive) == U_MULTICHAR_CONSTANT64('p','r','i','n','t','e','n','v'))
         {
         U_INTERNAL_ASSERT_EQUALS(directive[8], ' ')

         op = SSI_PRINTENV;
         i += U_CONSTANT_SIZE("printenv ");
         }

      int n = token.size() - i;

      U_INTERNAL_DUMP("op = %d n = %u", op, n)

#  ifdef DEBUG // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
       name.clear();
      value.clear();
#  endif

      if (n)
         {
         name_value.clear();

         tmp = UStringExt::simplifyWhiteSpace(token.substr(i, n));

         n = UStringExt::getNameValueFromData(tmp, name_value, U_CONSTANT_TO_PARAM(" "));
         }

#  ifdef DEBUG // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
      token.clear();
#  endif

      U_INTERNAL_DUMP("if_is_false = %b if_is_false_level = %d if_level = %d if_is_false_endif = %b",
                       if_is_false,     if_is_false_level,     if_level,     if_is_false_endif)

      switch (op)
         {
         /**
          * <!--#if expr="${Sec_Nav}" -->
          *    <!--#include virtual="secondary_nav.txt" -->
          * <!--#elif expr="${Pri_Nav}" -->
          *    <!--#include virtual="primary_nav.txt" -->
          * <!--#endif -->
          */

         case SSI_IF:
         case SSI_ELIF:
            {
            if (n != 2) U_ERROR("SSI: syntax error for %S statement", op == SSI_IF ? "if" : "elif");

            name = name_value[0];

            if (name.equal(U_CONSTANT_TO_PARAM("expr")) == false)
               {
               U_ERROR("SSI: unknow attribute %V for %S statement", name.rep, op == SSI_IF ? "if" : "elif");
               }

            value = name_value[1];

            if (op == SSI_IF)
               {
               if ( if_is_false       == false &&
                   (if_is_false_level == 0 || (if_level < if_is_false_level)))
                  {
                  if_is_false = (UStringExt::evalExpression(value, *UClientImage_Base::environment).empty()
                                    ? (if_is_false_level = if_level, true)
                                    :                                false);
                  }
               }
            else
               {
               --if_level;

               if (if_level == if_is_false_level)
                  {
                  if (if_is_false &&
                      if_is_false_endif == false)
                     {
                     if_is_false = (UStringExt::evalExpression(value, *UClientImage_Base::environment).empty()
                                             ? (if_is_false_level = if_level, true)
                                             : false);
                     }
                  else
                     {
                     if_is_false_level = if_level;
                     if_is_false = if_is_false_endif = true;
                     }
                  }
               }

            ++if_level;
            }
         break;

         case SSI_ELSE:
            {
            --if_level;

            if (if_is_false)
               {
               if (if_level          == if_is_false_level &&
                   if_is_false_endif == false)
                  {
                  if_is_false = false;
                  }
               }
            else
               {
               if_is_false       = true;
               if_is_false_level = if_level;
               }

            ++if_level;
            }
         break;

         case SSI_ENDIF:
            {
            --if_level;

            if (if_level == if_is_false_level) if_is_false = if_is_false_endif = false;
            }
         break;
         }

      U_INTERNAL_DUMP("if_is_false = %b if_is_false_level = %d if_level = %d if_is_false_endif = %b",
                       if_is_false,     if_is_false_level,     if_level,     if_is_false_endif)

      if (if_is_false) continue;

      switch (op)
         {
         /**
          * <!--#printenv -->
          */
         case SSI_PRINTENV: (void) output.append(U_STRING_TO_PARAM(*UClientImage_Base::environment)); break;

         /**
          * <!--#config sizefmt="bytes" -->
          * <!--#config timefmt="%y %m %d" -->
          * <!--#config errmsg="SSI command failed!" -->
          */
         case SSI_CONFIG:
            {
            for (i = 0; i < n; i += 2)
               {
               name  = name_value[i];
               value = name_value[i+1];

                    if (name.equal(U_CONSTANT_TO_PARAM("errmsg")))  (*errmsg  = value).duplicate();
               else if (name.equal(U_CONSTANT_TO_PARAM("timefmt"))) (*timefmt = value).duplicate();
               else if (name.equal(U_CONSTANT_TO_PARAM("sizefmt"))) use_size_abbrev = value.equal(U_CONSTANT_TO_PARAM("abbrev"));
               }
            }
         break;

         /**
          * <!--#echo var=$BODY_STYLE -->
          * <!--#echo [encoding="..."] var="..." [encoding="..."] var="..." ... -->
          */
         case SSI_ECHO:
            {
            encode = E_NONE;

            for (i = 0; i < n; i += 2)
               {
               name  = name_value[i];
               value = name_value[i+1];

               if (name.equal(U_CONSTANT_TO_PARAM("encoding")))
                  {
                       if (value.equal(U_CONSTANT_TO_PARAM("none")))   encode = E_NONE;
                  else if (value.equal(U_CONSTANT_TO_PARAM("url")))    encode = E_URL;
                  else if (value.equal(U_CONSTANT_TO_PARAM("entity"))) encode = E_ENTITY;
                  }
               else if (name == *UString::str_var)
                  {
                  U_INTERNAL_ASSERT_MAJOR(u_user_name_len, 0)

                  /**
                   * DATE_GMT       The current date in Greenwich Mean Time.
                   * DATE_LOCAL     The current date in the local time zone.
                   * LAST_MODIFIED  The last modification date of the document requested by the user.
                   * USER_NAME      Contains the owner of the file which included it.
                   * DOCUMENT_NAME  The filename (excluding directories) of the document requested by the user.
                   * DOCUMENT_URI   The URL path of the document requested by the user. Note that in the case of
                   *                nested include files, this is not then URL for the current document
                   */

                  bool blocal = false;

                  if (          value.equal(U_CONSTANT_TO_PARAM("DATE_GMT")) ||
                      (blocal = value.equal(U_CONSTANT_TO_PARAM("DATE_LOCAL"))))
                     {
                     U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

                     x = UTimeDate::strftime(U_STRING_TO_PARAM(*timefmt), u_now->tv_sec, blocal);
                     }
                  else
                     {
                          if (value.equal(U_CONSTANT_TO_PARAM("LAST_MODIFIED"))) x = UTimeDate::strftime(U_STRING_TO_PARAM(*timefmt), last_modified);
                     else if (value.equal(U_CONSTANT_TO_PARAM("USER_NAME")))     (void) x.assign(u_user_name, u_user_name_len);
                     else if (value.equal(U_CONSTANT_TO_PARAM("DOCUMENT_URI")))  (void) x.assign(U_HTTP_URI_TO_PARAM);
                     else if (value.equal(U_CONSTANT_TO_PARAM("DOCUMENT_NAME"))) x = *docname;
                     else
                        {
                        x = UStringExt::expandEnvironmentVar(value, UClientImage_Base::environment);
                        }
                     }

                  if (x.empty()) continue;

                       if (encode == E_NONE) encoded = x;
                  else if (encode == E_URL)
                     {
                     encoded.setBuffer(x.size() * 3);

                     Url::encode(x, encoded);
                     }
                  else
                     {
                     U_INTERNAL_ASSERT_EQUALS(encode, E_ENTITY)

                     encoded.setBuffer(x.size());

                     UXMLEscape::encode(x, encoded);
                     }

                  (void) output.append(encoded);
                  }

               x.clear();
               }
            }
         break;

         /**
          * <!--#fsize file="script.pl" -->
          * <!--#flastmod virtual="index.html" -->
          */
         case SSI_FSIZE:
         case SSI_FLASTMOD:
            {
            if (n == 2)
               {
               name     = name_value[0];
               value    = name_value[1];
               pathname = getPathname(name, value, directory);
               }

            bfile = false;

            if (pathname)
               {
               UHTTP::file->setPath(pathname, UClientImage_Base::environment);

                    if (UHTTP::isFileInCache()) bfile = true;
               else if (UHTTP::file->open())
                  {
                  bfile = true;

                  UHTTP::file->fstat();
                  }

               pathname.clear();
               }

            if (bfile == false)
               {
               (void) output.append(*errmsg);
               }
            else if (op == SSI_FSIZE)
               {
               uint32_t file_size = UHTTP::file->getSize();

               if (use_size_abbrev == false) UStringExt::appendNumber32(output, file_size);
               else                          (void) output.append(UStringExt::printSize(file_size));
               }
            else
               {
               (void) output.append(UTimeDate::strftime(U_STRING_TO_PARAM(*timefmt), UHTTP::file->st_mtime)); // SSI_FLASTMOD
               }
            }
         break;

         /**
          * <!--#include file=footer.html -->
          */
         case SSI_INCLUDE:
            {
            if (n == 2)
               {
               name     = name_value[0];
               value    = name_value[1];
               pathname = getPathname(name, value, directory);
               }

            bfile = false;

            if (pathname)
               {
               // NB: we check if we are near to the end of ssi processing (include of services output file)...

               bfile = u_startsWith(U_STRING_TO_PARAM(pathname), U_CONSTANT_TO_PARAM("$SSI_FILE_")); // HEAD|BODY

               if (bfile &&
                   *alternative_include)
                  {
                  (void) output.append(*alternative_include);
                                        alternative_include->clear();

                  break;
                  }

               UHTTP::file->setPath(pathname, UClientImage_Base::environment);

               if (bfile == false         &&
                   UHTTP::isFileInCache() &&
                   UHTTP::isDataFromCache())
                  {
                  include = UHTTP::getBodyFromCache();
                  }
               else if (UHTTP::file->open())
                  {
                            UHTTP::file->fstat();
                  include = UHTTP::file->getContent();

                  // NB: we want to clean the temporary file generate by shell script services...

                  if (bfile                                                                                   &&
                      (UStringExt::endsWith(U_FILE_TO_PARAM(*UHTTP::file), U_CONSTANT_TO_PARAM(":head.html")) ||
                       UStringExt::endsWith(U_FILE_TO_PARAM(*UHTTP::file), U_CONSTANT_TO_PARAM(":body.html"))))
                     {
                     (void) UHTTP::file->_unlink();
                     }
                  }
               }

            if (include.empty()) x = *errmsg;
            else
               {
               x = getInclude(include, include_level, bfile);

               include.clear();
               }

            (void) output.append(x);

                                 x.clear();
                          pathname.clear();
            UHTTP::file->getPath().clear();
            }
         break;

         /**
          * <!--#exec cmd="ls -l" -->
          * <!--#exec cgi=/cgi-bin/foo.cgi -->
          * <!--#exec servlet=/servlet/mchat -->
          */
         case SSI_EXEC:
            {
            if (n == 2)
               {
               if (callService(name_value[0], name_value[1]) == false) return UString::getStringNull();

               (void) output.append(*UClientImage_Base::wbuffer);
               }
            }
         break;

         /**
          * <!--#set cmd="env -l" -->
          * <!--#set var="foo" value="bar" -->
          * <!--#set cgi=$VIRTUAL_HOST/cgi-bin/main.bash -->
          */
         case SSI_SET:
            {
            bvar = false;

            for (i = 0; i < n; i += 4)
               {
               if (name_value[i]   == *UString::str_var &&
                   name_value[i+2].equal(U_CONSTANT_TO_PARAM("value")))
                  {
                  bvar = true;

                  (void) UClientImage_Base::environment->append(name_value[i+1]);
                  (void) UClientImage_Base::environment->append(1U, '=');
                  (void) UClientImage_Base::environment->append(UStringExt::expandEnvironmentVar(name_value[i+3], UClientImage_Base::environment));
                  (void) UClientImage_Base::environment->append(1U, '\n');
                  }
               }

            if (bvar == false)
               {
               name = name_value[0];

               if (callService(name, name_value[1]) == false) return UString::getStringNull();

               if (name == *UString::str_cgi)
                  {
                  // NB: check if we are in cgi shell script DEBUG mode...

                  if (u_startsWith(U_STRING_TO_PARAM(*UClientImage_Base::wbuffer), U_CONSTANT_TO_PARAM("ENVIRONMENT:\n-")))
                     {
                     pos = U_STRING_FIND(*UClientImage_Base::wbuffer, 100, "'TITLE_TXT=");

                     UClientImage_Base::wbuffer->erase(0, pos);
                     }

                  // NB: we check if there is an alternative response to elaborate...

                  UString rheader = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("HTTP_RESPONSE_HEADER"), UClientImage_Base::wbuffer);

                  U_INTERNAL_DUMP("response_header(%u) = %V", rheader.size(), rheader.rep)

                  if (rheader)
                     {
                     // 2 => var environment 'HTTP_RESPONSE_HEADER' inserted in header response (var 'header')

                     alternative_response = 2;

                     uint32_t hsz = rheader.size();

                     UString _tmp(hsz);

                     UEscape::decode(rheader, _tmp);

                     // NB: we cannot use directly the body attribute because is bounded at the param 'content'...

                     UString rbody = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("HTTP_RESPONSE_BODY"), UClientImage_Base::wbuffer);

                     U_INTERNAL_DUMP("response_body(%u) = %V", rbody.size(), rbody.rep)

                     if (rbody.empty()) (void) header->insert(0, _tmp);
                     else
                        {
                        // 3 => var environment 'HTTP_RESPONSE_HEADER' set as header response (var 'header') and
                        //      var environment 'HTTP_RESPONSE_BODY'   set as body   response (var 'body')

                        alternative_response = 3;

                        *body = rbody;

                        // NB: with an alternative body response we cannot use the cached header response...

                        (void) header->replace(_tmp);

                        return UString::getStringNull();
                        }

                     UClientImage_Base::wbuffer->erase(0, U_CONSTANT_SIZE("HTTP_RESPONSE_HEADER=") + hsz + 3);
                     }

                  (void) UClientImage_Base::environment->append(*UClientImage_Base::wbuffer);
                  }
               }
            }
         break;
         }
      }
   while (t.atEnd() == false);

   U_INTERNAL_DUMP("if_is_false = %b if_is_false_level = %d if_level = %d if_is_false_endif = %b",
                    if_is_false,     if_is_false_level,     if_level,     if_is_false_endif)

   if (if_is_false || if_is_false_endif || if_level || if_is_false_level) U_ERROR("SSI: syntax error for conditional statement");

   U_RETURN_STRING(output);
}

// Server-wide hooks

int USSIPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "USSIPlugIn::handlerConfig(%p)", &cfg)

   // --------------------------------------------------------------------------------------------------------------
   // ENVIRONMENT             path of file configuration environment for SSI
   //
   // SSI_AUTOMATIC_ALIASING  special SSI HTML file that is recognized automatically as alias of all uri request
   //                         without suffix (generally cause navigation directory not working)
   // --------------------------------------------------------------------------------------------------------------

   if (cfg.loadTable())
      {
      UString x = cfg.at(U_CONSTANT_TO_PARAM("ENVIRONMENT"));

      if (x)
         {
         U_NEW(UString, environment, UString(UStringExt::prepareForEnvironmentVar(UFile::contentOf(x))));

         const char* home = U_SYSCALL(getenv, "%S", "HOME");

         if (home) environment->snprintf_add(U_CONSTANT_TO_PARAM("HOME=%s\n"), home);

         U_ASSERT_EQUALS(environment->isBinary(), false)
         }

#  ifdef U_ALIAS
      x = cfg.at(U_CONSTANT_TO_PARAM("SSI_AUTOMATIC_ALIASING"));

      if (x) UHTTP::setGlobalAlias(x); // NB: automatic alias of all uri request without suffix...
#  endif

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

int USSIPlugIn::handlerInit()
{
   U_TRACE_NO_PARAM(0, "USSIPlugIn::handlerInit()")

   U_INTERNAL_ASSERT_EQUALS(body, 0)
   U_INTERNAL_ASSERT_EQUALS(header, 0)
   U_INTERNAL_ASSERT_EQUALS(alternative_include, 0)

   U_NEW(UString, body,  UString);
   U_NEW(UString, header, UString);
   U_NEW(UString, alternative_include, UString);

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

// Connection-wide hooks

int USSIPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "USSIPlugIn::handlerRequest()")

   U_INTERNAL_DUMP("uri = %.*S", U_HTTP_URI_TO_TRACE)

   if (UClientImage_Base::isRequestNotFound()  == false &&
       UClientImage_Base::isRequestForbidden() == false)
      {
      bool bcache =          UHTTP::file_data &&
                    u_is_ssi(UHTTP::file_data->mime_index);

      U_DUMP("bcache = %b", bcache)

      if (bcache ||
          (UHTTP::setMimeIndex(), u_is_ssi(UHTTP::mime_index)))
         {
         // init

         U_ASSERT(UClientImage_Base::environment->empty())

         if (UHTTP::getCGIEnvironment(*UClientImage_Base::environment, U_SHELL) == false) U_RETURN(U_PLUGIN_HANDLER_ERROR);

         if (environment)       (void) UClientImage_Base::environment->append(*environment);
         if (UHTTP::isMobile()) (void) UClientImage_Base::environment->append(U_CONSTANT_TO_PARAM("HTTP_USER_AGENT_MOBILE=1\n"));

         (void)  errmsg->assign(U_CONSTANT_TO_PARAM("Error"));
         (void) timefmt->assign(U_CONSTANT_TO_PARAM("%A, %d-%b-%Y %T GMT"));

         last_modified        = UHTTP::file->st_mtime;
         use_size_abbrev      = true;
         alternative_response = 0;

         header->setBuffer(U_CAPACITY);

         UClientImage_Base::setRequestNoCache();

         *docname = UStringExt::basename(UHTTP::file->getPath()).copy();

         // read the SSI file

         if (bcache == false)
            {
            *body = UHTTP::file->getContent();

            if (body->empty())
               {
               U_DEBUG("USSIPlugIn::handlerRequest() SSI file empty: %.*S", U_FILE_TO_TRACE(*UHTTP::file))

               UClientImage_Base::environment->setEmpty();

               UHTTP::setInternalError();

               U_RETURN(U_PLUGIN_HANDLER_GO_ON);
               }
            }
         else
            {
            U_INTERNAL_DUMP("UClientImage_Base::body(%u) = %V", UClientImage_Base::body->size(), UClientImage_Base::body->rep)

            U_ASSERT(UHTTP::isDataFromCache())
            U_INTERNAL_ASSERT_POINTER(UHTTP::file_data->array)
            U_INTERNAL_ASSERT_EQUALS( UHTTP::file_data->array->size(), 2)

            (void) header->append(UHTTP::getHeaderFromCache()); // NB: after now 'file_data' can change...

            *body = (UHTTP::isGETorHEAD() &&
                     *UClientImage_Base::body
                         ? *UClientImage_Base::body
                         : UHTTP::getBodyFromCache());
            }

         // process the SSI file

         UString output = processSSIRequest(*body, 0);

         U_INTERNAL_DUMP("alternative_response = %d output(%u) = %V", alternative_response, output.size(), output.rep)

         if (alternative_response == 0)
            {
            uint32_t size = output.size();

            U_INTERNAL_ASSERT_MAJOR(size,0)

#        ifdef USE_LIBZ
            U_INTERNAL_DUMP("U_http_is_accept_gzip = %b", U_http_is_accept_gzip)

            if (U_http_is_accept_gzip &&
                size > U_MIN_SIZE_FOR_DEFLATE)
#        endif
               {
               output = UStringExt::deflate(output, 1);

               size = output.size();

               (void) header->insert(0, U_CONSTANT_TO_PARAM("Content-Encoding: gzip\r\n"));
               }

            *UHTTP::ext = *header;

            if (bcache) (void) UHTTP::checkContentLength(size, U_NOT_FOUND); // NB: adjusting the size of response...
            else
               {
               UHTTP::mime_index = U_unknow;

               (void) UHTTP::ext->append(UHTTP::getHeaderMimeType(0, size, U_CTYPE_HTML));
               }

            U_http_info.nResponseCode = HTTP_OK;

            *UClientImage_Base::body = output;

            UHTTP::handlerResponse();
            }
         else if (alternative_response > 1)
            {
            // -----------------------------------------------------------------------------------------------------
            // 1 => response already complete (nothing to do)
            // ------------------------------------------------------------------------------------------------------
            // 2 => var environment 'HTTP_RESPONSE_HEADER' inserted in header response (var 'header')
            // ------------------------------------------------------------------------------------------------------
            // 3 => var environment 'HTTP_RESPONSE_HEADER'      set as header response (var 'header') and
            //      var environment 'HTTP_RESPONSE_BODY'        set as body   response (var 'body')
            // ------------------------------------------------------------------------------------------------------

            U_INTERNAL_DUMP("header(%u) = %V", header->size(), header->rep)
            U_INTERNAL_DUMP("  body(%u) = %V",   body->size(),   body->rep)

            (void) UClientImage_Base::wbuffer->replace(*header);
            (void) UClientImage_Base::wbuffer->append(U_CONSTANT_TO_PARAM(U_CRLF));

            U_http_info.endHeader     = UClientImage_Base::wbuffer->size(); 
            U_http_info.nResponseCode = HTTP_OK;

            (void) UClientImage_Base::wbuffer->append(alternative_response == 2 ? output : *body);

            if (UHTTP::processCGIOutput(true, true) == false) UHTTP::setInternalError();
            }

         UClientImage_Base::environment->setEmpty();
         }

   // U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* USSIPlugIn::dump(bool reset) const
{
   *UObjectIO::os << "last_modified                " << last_modified               << '\n'
                  << "use_size_abbrev              " << use_size_abbrev             << '\n'
                  << "body                (UString " << (void*)body                 << ")\n"
                  << "errmsg              (UString " << (void*)errmsg               << ")\n"
                  << "header              (UString " << (void*)header               << ")\n"
                  << "timefmt             (UString " << (void*)timefmt              << ")\n"
                  << "docname             (UString " << (void*)docname              << ")\n"
                  << "environment         (UString " << (void*)environment          << ")\n"
                  << "alternative_include (UString " << (void*)alternative_include  << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
