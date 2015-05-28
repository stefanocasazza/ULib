// jsonrequest.cpp - dynamic page translation (jsonrequest.usp => jsonrequest.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT int runDynamicPage_jsonrequest(UClientImage_Base* client_image);
       U_EXPORT int runDynamicPage_jsonrequest(UClientImage_Base* client_image)
{
   U_TRACE(0, "::runDynamicPage_jsonrequest(%p)", client_image)
   
   
   // ------------------------------
   // special argument value:
   // ------------------------------
   //  0 -> call it as service
   // -1 -> init
   // -2 -> reset
   // -3 -> destroy
   // -4 -> call it for sigHUP
   // -5 -> call it after fork
   // ------------------------------
   
   if (client_image)
      {
      if (client_image >= (void*)-5) U_RETURN(0);
   
      (void) UClientImage_Base::wbuffer->append(
         U_CONSTANT_TO_PARAM("Content-Type: application/jsonrequest\r\n\r\n"));
   
      U_http_info.endHeader = UClientImage_Base::wbuffer->size();
      }
      
   U_INTERNAL_DUMP("U_HTTP_CTYPE = %.*S", U_HTTP_CTYPE_TO_TRACE)
   
   if (U_HTTP_CTYPE_STREQ("application/jsonrequest"))
      {
      UValue json;
   
      if (json.parse(*UClientImage_Base::body)) USP_PUTS_STRING(json.output());
      else                                      USP_PUTS_CONSTANT("{}");
      }
   
      U_http_content_type_len = 1;
   
      UClientImage_Base::setRequestNoCache();
   
   U_RETURN(200);
} }
