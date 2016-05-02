// json.cpp - dynamic page translation (json.usp => json.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
static UString* pkey;
   static UString* pvalue;
   static void usp_init_json()
   {
    U_TRACE(5, "::usp_init_json()")
    U_NEW(UString, pkey, U_STRING_FROM_CONSTANT("message"));
    U_NEW(UString, pvalue, U_STRING_FROM_CONSTANT("Hello, World!"));
   }
   static void usp_end_json()
   {
    U_TRACE(5, "::usp_end_json()")
    delete pkey;
    delete pvalue;
   }  
   
extern "C" {
extern U_EXPORT void runDynamicPage_json(int param);
       U_EXPORT void runDynamicPage_json(int param)
{
   U_TRACE(0, "::runDynamicPage_json(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_INIT) { usp_init_json(); return; }
   
      if (param == U_DPAGE_DESTROY) { usp_end_json(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
      U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false);
      U_http_info.endHeader = 34;
      (void) UClientImage_Base::wbuffer->insert(0, 
   U_CONSTANT_TO_PARAM("Content-Type: application/json\r\n\r\n"));
   
   UValue json(*pkey, *pvalue);
   USP_JSON_PUTS(json);
   
      U_http_content_type_len = 1;
   
   UClientImage_Base::setRequestNoCache();
   
   
} }