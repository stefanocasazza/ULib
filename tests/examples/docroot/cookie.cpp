// cookie.cpp - dynamic page translation (cookie.usp => cookie.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
static void usp_init_cookie()
{
   U_TRACE(5, "::usp_init_cookie()")
   
   if (UHTTP::data_session == 0)   U_NEW(UDataSession, UHTTP::data_storage, UDataSession);
   
   if (UHTTP::db_session == 0) UHTTP::initSession();
}  
   
extern "C" {
extern U_EXPORT void runDynamicPage_cookie(int param);
       U_EXPORT void runDynamicPage_cookie(int param)
{
   U_TRACE(0, "::runDynamicPage_cookie(%d)", param)
   
   uint32_t usp_sz = 0;
   char usp_buffer[10 * 4096];
   
   if (param)
      {
      if (param == U_DPAGE_INIT) { usp_init_cookie(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
      if (UHTTP::isGETorPOST()) (void) UHTTP::processForm();
   
   UString name = USP_FORM_VALUE(0);
   
   UString delete_cookie = USP_FORM_VALUE(1);
   
   UString ses_name;
   
   USP_SESSION_VAR_GET(0,ses_name);
   
   if (name.empty())     name = ses_name;
   else              ses_name =     name;
   
   if (delete_cookie) UHTTP::removeDataSession();
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html>\n <head>\n  <title>Hello World-application for Ulib</title>\n </head>\n\n <body bgcolor=\"#FFFFFF\">\n\n  <h1>Hello ")
   );
   
   (void) UClientImage_Base::wbuffer->append((name.empty() ? "World" : name.data()));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</h1>\n\n  <form>\n   What's your name?\n   <input type=\"text\" name=\"name\" value=\"")
   );
   
   (void) UClientImage_Base::wbuffer->append((name));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("\"> <br>\n   <input type=\"submit\">\n   <input type=\"submit\" name=\"clearcookie\" value=\"delete cookie\">\n  </form>\n\n  <a href=\"cookie\">reload</a>\n\n </body>\n</html>")
   );
   
   USP_SESSION_VAR_PUT(0,ses_name);
   
   UClientImage_Base::setRequestNoCache();
   
   
} }