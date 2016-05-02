// benchmarking.cpp - dynamic page translation (benchmarking.usp => benchmarking.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT void runDynamicPage_benchmarking(int param);
       U_EXPORT void runDynamicPage_benchmarking(int param)
{
   U_TRACE(0, "::runDynamicPage_benchmarking(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
      if (UHTTP::isGETorPOST()) (void) UHTTP::processForm();
   
   UString name = USP_FORM_VALUE(0);
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<h1>Hello ")
   );
   
   USP_XML_PUTS((name));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</h1>")
   );
   
   UClientImage_Base::setRequestNoCache();
   
   
} }