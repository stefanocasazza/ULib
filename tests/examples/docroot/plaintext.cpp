// plaintext.cpp - dynamic page translation (plaintext.usp => plaintext.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT void runDynamicPage_plaintext(int param);
       U_EXPORT void runDynamicPage_plaintext(int param)
{
   U_TRACE(0, "::runDynamicPage_plaintext(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   U_http_info.endHeader = 0;
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("Hello, World!")
   );
      
} }