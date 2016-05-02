// calcajax.cpp - dynamic page translation (calcajax.usp => calcajax.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT void runDynamicPage_calcajax(int param);
       U_EXPORT void runDynamicPage_calcajax(int param)
{
   U_TRACE(0, "::runDynamicPage_calcajax(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html>\n <head>\n  <title>Calculator</title>\n  <script src=\"/js/calcajax.js\" type=\"text/javascript\"></script>\n </head>\n <body bgcolor=#ffffcc>\n  <h1>Tommi's Calculator</h1>\n\n  <form>\n   <input type=\"text\"   value=\"0\" id=\"arg1\"> <br>\n   <input type=\"text\"   value=\"0\" id=\"arg2\"> <br>\n   <input type=\"button\" value=\"+\" onClick=\"calc('p')\">\n   <input type=\"button\" value=\"-\" onClick=\"calc('-')\">\n   <input type=\"button\" value=\"*\" onClick=\"calc('*')\">\n   <input type=\"button\" value=\"/\" onClick=\"calc('/')\">\n  </form>\n\n  <div id=\"result\" style=\"display:none\">\n  </div>\n\n </body>\n</html>")
   );
      
} }