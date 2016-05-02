// remove.cpp - dynamic page translation (remove.usp => remove.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT void runDynamicPage_remove(int param);
       U_EXPORT void runDynamicPage_remove(int param)
{
   U_TRACE(0, "::runDynamicPage_remove(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
   UHTTP::removeDataSession();
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html><body>SESSION COOKIE REMOVED</body></html>")
   );
      
} }