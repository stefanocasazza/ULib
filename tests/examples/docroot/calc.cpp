// calc.cpp - dynamic page translation (calc.usp => calc.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT void runDynamicPage_calc(int param);
       U_EXPORT void runDynamicPage_calc(int param)
{
   U_TRACE(0, "::runDynamicPage_calc(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
      if (UHTTP::isGETorPOST()) (void) UHTTP::processForm();
   
   UString arg1 = USP_FORM_VALUE(0);
   
   if (arg1.empty()) arg1 = U_STRING_FROM_CONSTANT("0");
   
   UString arg2 = USP_FORM_VALUE(1);
   
   if (arg2.empty()) arg2 = U_STRING_FROM_CONSTANT("0");
   
   UString opval = USP_FORM_VALUE(2);
   
   UString op    = USP_FORM_NAME(2);
   double val1   = arg1.strtod(),
          val2   = arg2.strtod(),
          result = 0.0;
   
        if (op == "plus")  result = val1 + val2;
   else if (op == "minus") result = val1 - val2;
   else if (op == "mul")   result = val1 * val2;
   else if (op == "div")   result = val1 / val2;
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html>\n <head>\n  <title>Calculator</title>\n </head>\n <body bgcolor=#ffffcc>\n  <h1>Tommi's Calculator</h1>\n\n  <form>\n   <input type=\"text\"   name=\"arg1\"  value=\"")
   );
   
   (void) UClientImage_Base::wbuffer->append((arg1));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("\" > <br>\n   <input type=\"text\"   name=\"arg2\"  value=\"")
   );
   
   (void) UClientImage_Base::wbuffer->append((arg2));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("\" > <br>\n   <input type=\"submit\" name=\"plus\"  value=\"+\">\n   <input type=\"submit\" name=\"minus\" value=\"-\">\n   <input type=\"submit\" name=\"mul\"   value=\"*\">\n   <input type=\"submit\" name=\"div\"   value=\"/\">\n  </form>\n\n")
   );
   
   if (op)
      {
      USP_PRINTF("<hr>\n%v %v %v = %g", arg1.rep, opval.rep, arg2.rep, result);
      }
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM(" </body>\n</html>")
   );
   
   UClientImage_Base::setRequestNoCache();
   
   
} }