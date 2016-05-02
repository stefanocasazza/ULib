// docalc.cpp - dynamic page translation (docalc.usp => docalc.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT void runDynamicPage_docalc(int param);
       U_EXPORT void runDynamicPage_docalc(int param)
{
   U_TRACE(0, "::runDynamicPage_docalc(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   U_http_info.endHeader = 0;
   
      if (UHTTP::isGETorPOST()) (void) UHTTP::processForm();
   
   UString arg1 = USP_FORM_VALUE(0);
   
   UString arg2 = USP_FORM_VALUE(1);
   
   UString opval = USP_FORM_VALUE(2);
   
   double val1   = arg1.strtod(),
          val2   = arg2.strtod(),
          result = 0.0;
   
        if (opval == "p") result = val1 + val2;
   else if (opval == "-") result = val1 - val2;
   else if (opval == "*") result = val1 * val2;
   else if (opval == "/") result = val1 / val2;
   
   if (opval) USP_PRINTF("<hr>\n%v %v %v = %g", arg1.rep, opval.rep, arg2.rep, result);
   
   UClientImage_Base::setRequestNoCache();
   
   
} }