// upload.cpp - dynamic page translation (upload.usp => upload.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
   
extern "C" {
extern U_EXPORT void runDynamicPage_upload(int param);
       U_EXPORT void runDynamicPage_upload(int param)
{
   U_TRACE(0, "::runDynamicPage_upload(%d)", param)
   
   
   if (param)
      {
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
      if (UHTTP::isGETorPOST()) (void) UHTTP::processForm();
   
   UString filepath = USP_FORM_VALUE(0);
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html>\n <body>\n\n  ")
   );
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("  <form method=\"post\" enctype=\"multipart/form-data\">\n   <input type=\"file\" name=\"upload\">\n   <input type=\"submit\">\n  </form>\n\n")
   );
   
   if (UHTTP::isPOST() &&
          UHTTP::form_name_value->size() == 2)
      {
      UString md5sum(33U), dump(10000U),
              content  = UFile::contentOf(filepath);
   
      uint32_t size   = content.size();
      const char* ptr = content.data();
   
      UServices::generateDigest(U_HASH_MD5, 0, (unsigned char*)ptr, size, md5sum, false);
   
      dump.snprintf("%M", ptr, U_min(size,1024));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("    <p>\n\t file received:<br>\n\t ")
   );
   
   (void) UClientImage_Base::wbuffer->append((UStringExt::basename(filepath)));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM(" (")
   );
   
   UStringExt::appendNumber32(*UClientImage_Base::wbuffer, (size));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM(" bytes)<br>\n    md5sum: ")
   );
   
   (void) UClientImage_Base::wbuffer->append((md5sum));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<br>\n    data:\n\t <pre>")
   );
   
   USP_XML_PUTS((dump));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</pre>\n\t </p>\n\n")
   );
   
   }
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM(" </body>\n</html>")
   );
   
   UClientImage_Base::setRequestNoCache();
   
   
} }