// modsocket.cpp - dynamic page translation (modsocket.usp => modsocket.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT void runDynamicPage_modsocket(int param);
       U_EXPORT void runDynamicPage_modsocket(int param)
{
   U_TRACE(0, "::runDynamicPage_modsocket(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   U_http_info.endHeader = 0;
   
   if (UWebSocket::sendData(UWebSocket::message_type, (const unsigned char*)U_STRING_TO_PARAM(*UClientImage_Base::wbuffer)) == false) U_http_info.nResponseCode = HTTP_INTERNAL_ERROR;
   
   
} }