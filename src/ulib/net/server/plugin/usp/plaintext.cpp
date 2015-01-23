// plaintext.cpp - dynamic page translation (plaintext.usp => plaintext.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT int runDynamicPage_plaintext(UClientImage_Base* client_image);
       U_EXPORT int runDynamicPage_plaintext(UClientImage_Base* client_image)
{
   U_TRACE(0, "::runDynamicPage_plaintext(%p)", client_image)
   
   
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
   
      u_http_info.endHeader = 0;
      }
      
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("Hello, World!")
   );
   
   U_RETURN(200);
} }