// hello.cpp - dynamic page translation (hello.usp => hello.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT void runDynamicPage_hello(int param);
       U_EXPORT void runDynamicPage_hello(int param)
{
   U_TRACE(0, "::runDynamicPage_hello(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
      if (UHTTP::isGETorPOST()) (void) UHTTP::processForm();
   
   UString name = USP_FORM_VALUE(0);
   
   if (name.empty()) name = U_STRING_FROM_CONSTANT("World");
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html>\n <head>\n  <title>Hello World-application for Ulib</title>\n </head>\n\n <body bgcolor=\"#FFFFFF\">\n  <img src=\"/images/ulib_logo.jpg\" align=\"right\">\n\n   <h1>Hello ")
   );
   
   (void) UClientImage_Base::wbuffer->append((name));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</h1>\n\n  <form>\n   What's your name?\n   <input type=\"text\" name=\"name\" value=\"")
   );
   
   (void) UClientImage_Base::wbuffer->append((name));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("\" ><br>\n   <input type=\"submit\">\n  </form>\n\n </body>\n</html>")
   );
   
   UClientImage_Base::setRequestNoCache();
   
   
} }