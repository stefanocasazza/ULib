// upload_progress.cpp - dynamic page translation (upload_progress.usp => upload_progress.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
static void usp_init_upload_progress()
   {
   U_TRACE(5, "::usp_init_upload_progress()")
   
   if (UServer_Base::isPreForked())
      {
      UHTTP::ptr_upload_progress = (UHTTP::upload_progress*) UServer_Base::getPointerToDataShare(UHTTP::ptr_upload_progress);
   
      U_INTERNAL_ASSERT_EQUALS(UHTTP::ptr_upload_progress->byte_read, 0)
      }
   }  
   
extern "C" {
extern U_EXPORT int runDynamicPage_upload_progress(UClientImage_Base* client_image);
       U_EXPORT int runDynamicPage_upload_progress(UClientImage_Base* client_image)
{
   U_TRACE(0, "::runDynamicPage_upload_progress(%p)", client_image)
   
   
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
      if (client_image == (void*)-1) { usp_init_upload_progress(); U_RETURN(0); }
   
      if (client_image >= (void*)-5) U_RETURN(0);
   
      (void) UClientImage_Base::wbuffer->append(
         U_CONSTANT_TO_PARAM("Content-Type: application/json\r\nCache Control: max-age=0\r\nExpires: Thu, 19 Nov 1981 08:52:00 GMT\r\n\r\n"));
   
      u_http_info.endHeader = UClientImage_Base::wbuffer->size();
      }
      
   (void) UClientImage_Base::wbuffer->append(UHTTP::getUploadProgress());
   
      U_http_content_type_len = 1;
   
      UClientImage_Base::setRequestNoCache();
   
   U_RETURN(200);
} }