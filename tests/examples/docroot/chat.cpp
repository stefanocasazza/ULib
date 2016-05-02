// chat.cpp - dynamic page translation (chat.usp => chat.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT void runDynamicPage_chat(int param);
       U_EXPORT void runDynamicPage_chat(int param)
{
   U_TRACE(0, "::runDynamicPage_chat(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html>\n <head>\n  <title>USP application chat</title>\n  <link type=\"text/css\" href=\"/css/chat.css\" rel=\"stylesheet\">\n  <script src=\"/js/chat.js\" type=\"text/javascript\"></script>\n </head>\n <body>\n  <h1>Ajaxchat</h1>\n  <form>\n   <table>\n    <tr>\n     <td>\n      your Name\n     </td>\n     <td>\n      <input type=\"text\" name=\"person\" id=\"person\" size=\"20\">\n     </td>\n    </tr>\n    <tr>\n     <td>\n      your Message\n     </td>\n     <td>\n      <input type=\"text\" name=\"message\" id=\"message\" size=\"80\"><br>\n     </td>\n    </tr>\n   </table>\n   <input type=\"button\" value=\"talk\" onClick=\"sendMessage()\">\n   <img src=\"/images/sad.png\"   onClick='addText(\" :( \")'>\n   <img src=\"/images/wink.png\"  onClick='addText(\" ;) \")'>\n   <img src=\"/images/smile.png\" onClick='addText(\" :) \")'>\n  </form>\n  <script type=\"text/javascript\">\n   document.getElementById(\"person\").focus();\n  </script>\n  <hr>\n  <div id=\"chat\">\n")
   );
   
   (void) UHTTP::callService(U_STRING_FROM_CONSTANT("servlet/cchat"));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("  </div>\n </body>\n</html>")
   );
   
   UClientImage_Base::setRequestNoCache();
   
   
} }