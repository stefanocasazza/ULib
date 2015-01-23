// json.cpp - dynamic page translation (json.usp => json.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
#ifdef AS_cpoll_cppsp_DO
   #undef AS_cpoll_cppsp_DO
   #endif
   #ifdef AS_cpoll_cppsp_DO
   #  include <json/json.h>
   #else
   static UString* pkey;
   static UString* pvalue;
   #endif
   
   static void usp_init_json()
   {
      U_TRACE(5, "::usp_init_json()")
   
   #ifndef AS_cpoll_cppsp_DO
      pkey   = U_NEW(U_STRING_FROM_CONSTANT("message"));
      pvalue = U_NEW(U_STRING_FROM_CONSTANT("Hello, World!"));
   #endif
   }
   
   static void usp_end_json()
   {
      U_TRACE(5, "::usp_end_json()")
   
   #ifndef AS_cpoll_cppsp_DO
      delete pkey;
      delete pvalue;
   #endif
   }  
   
extern "C" {
extern U_EXPORT int runDynamicPage_json(UClientImage_Base* client_image);
       U_EXPORT int runDynamicPage_json(UClientImage_Base* client_image)
{
   U_TRACE(0, "::runDynamicPage_json(%p)", client_image)
   
   
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
      if (client_image == (void*)-1) { usp_init_json(); U_RETURN(0); }
   
      if (client_image == (void*)-3) { usp_end_json(); U_RETURN(0); }
   
      if (client_image >= (void*)-5) U_RETURN(0);
   
      (void) UClientImage_Base::wbuffer->append(
         U_CONSTANT_TO_PARAM("Content-Type: application/json; charset=UTF-8\r\n\r\n"));
   
      u_http_info.endHeader = UClientImage_Base::wbuffer->size();
      }
      
   #ifndef AS_cpoll_cppsp_DO
   UValue json(*pkey, *pvalue);
   USP_JSON_PUTS(json);
   #else
   json_object* hello = json_object_new_object();
   json_object_object_add(hello, "message", json_object_new_string("Hello, World!"));
   const char* hello_str = json_object_to_json_string(hello);
   USP_PUTS_STRING(hello_str);
   json_object_put(hello);
   #endif
   
      U_http_content_type_len = 1;
   
      UClientImage_Base::setRequestNoCache();
   
   U_RETURN(200);
} }