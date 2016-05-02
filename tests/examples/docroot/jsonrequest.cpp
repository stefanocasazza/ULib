// jsonrequest.cpp - dynamic page translation (jsonrequest.usp => jsonrequest.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT void runDynamicPage_jsonrequest(int param);
       U_EXPORT void runDynamicPage_jsonrequest(int param)
{
   U_TRACE(0, "::runDynamicPage_jsonrequest(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
      U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false);
      U_http_info.endHeader = 41;
      (void) UClientImage_Base::wbuffer->insert(0, 
   U_CONSTANT_TO_PARAM("Content-Type: application/jsonrequest\r\n\r\n"));
   
   U_INTERNAL_DUMP("U_HTTP_CTYPE = %.*S", U_HTTP_CTYPE_TO_TRACE)
   
   if (U_HTTP_CTYPE_STREQ("application/jsonrequest"))
      {
      UValue json;
   
      if (json.parse(*UClientImage_Base::body)) USP_PUTS_STRING(json.output());
      else                                      USP_PUTS_CONSTANT("{}");
      }
   
      U_http_content_type_len = 1;
   
   UClientImage_Base::setRequestNoCache();
   
   
} }