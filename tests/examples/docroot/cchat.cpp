// cchat.cpp - dynamic page translation (cchat.usp => cchat.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
static void usp_init_cchat()
{
   U_TRACE(5, "::usp_init_cchat()")
   
   if (UHTTP::data_storage == 0) { U_NEW(UDataSession, UHTTP::data_storage, UDataSession(*UString::str_storage_keyid)); }
   
   if (UHTTP::db_session == 0) UHTTP::initSession();
}  
   
extern "C" {
extern U_EXPORT void runDynamicPage_cchat(int param);
       U_EXPORT void runDynamicPage_cchat(int param)
{
   U_TRACE(0, "::runDynamicPage_cchat(%d)", param)
   
   uint32_t usp_sz = 0;
   char usp_buffer[10 * 4096];
   
   if (param)
      {
      if (param == U_DPAGE_INIT) { usp_init_cchat(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
      U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::wbuffer->findEndHeader(),false);
      U_http_info.endHeader = 139;
      (void) UClientImage_Base::wbuffer->insert(0, 
   U_CONSTANT_TO_PARAM("Pragma: no-cache\r\nExpires: Thu, 19 Nov 1981 08:52:00 GMT\r\nCache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0\r\n\r\n"));
   
      if (UHTTP::isGETorPOST()) (void) UHTTP::processForm();
   
   UVector<UString> chat_message;
   
   USP_STORAGE_VAR_GET(0,chat_message);
   
   UString person = USP_FORM_VALUE(0);
   
   UString message = USP_FORM_VALUE(1);
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<table>\n")
   );
   
   if (message)
      {
      message = UStringExt::substitute(message, U_CONSTANT_TO_PARAM(":("), U_CONSTANT_TO_PARAM("<img src=\"/images/sad.png\">"));
      message = UStringExt::substitute(message, U_CONSTANT_TO_PARAM(";)"), U_CONSTANT_TO_PARAM("<img src=\"/images/wink.png\">"));
      message = UStringExt::substitute(message, U_CONSTANT_TO_PARAM(":)"), U_CONSTANT_TO_PARAM("<img src=\"/images/smile.png\">"));
   
      chat_message.push_back(person);
      chat_message.push_back(message);
   
      if (chat_message.size() > 20) chat_message.erase(0, 2);
      }
   
   for (uint32_t i = 0, n = chat_message.size(); i < n; i += 2)
      {
      person  = chat_message[i];
      message = chat_message[i+1];
   
      USP_PRINTF("<tr><td class=\"person\">%v</td>"
                     "<td class=\"message\">%v</td>"
                 "</tr>", person.rep, message.rep);
      }
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</table>")
   );
   
   USP_STORAGE_VAR_PUT(0,chat_message);
   
   UClientImage_Base::setRequestNoCache();
   
   
} }