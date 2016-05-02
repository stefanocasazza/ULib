// strings.cpp - dynamic page translation (strings.usp => strings.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
static void usp_init_strings()
{
   U_TRACE(5, "::usp_init_strings()")
   
   if (UHTTP::data_session == 0)   U_NEW(UDataSession, UHTTP::data_storage, UDataSession);
   
   if (UHTTP::db_session == 0) UHTTP::initSession();
}  
   
extern "C" {
extern U_EXPORT void runDynamicPage_strings(int param);
       U_EXPORT void runDynamicPage_strings(int param)
{
   U_TRACE(0, "::runDynamicPage_strings(%d)", param)
   
   uint32_t usp_sz = 0;
   char usp_buffer[10 * 4096];
   
   if (param)
      {
      if (param == U_DPAGE_INIT) { usp_init_strings(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
      if (UHTTP::isGETorPOST()) (void) UHTTP::processForm();
   
   UVector<UString> strings;
   
   USP_SESSION_VAR_GET(0,strings);
   
   uint32_t remove = U_NOT_FOUND;
   
   if (USP_FORM_VALUE_FROM_NAME("add").empty()) remove = USP_FORM_VALUE_FROM_NAME("remove").strtol();
   else                                strings.push_back(USP_FORM_VALUE_FROM_NAME("string"));
   
   if (remove < strings.size()) strings.erase(remove);
   
   uint32_t sz = strings.size();
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html>\n <head><title>Example USP</title></head>\n <body>\n  <h1>Example USP</h1>\n\n   <p>This servlet enters strings into a table.</p>\n\n   <form method=get>\n    <p>Enter a string:\n    <input type=text size=64 name=\"string\"></input>\n    <input type=submit       name=\"add\" value=\"Go!\"></input>\n    </p>\n   </form>\n\n\t<p><table border=1>\n\t<tr><th colspan=2>Strings entered so far: ")
   );
   
   UStringExt::appendNumber32(*UClientImage_Base::wbuffer, (sz));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</th></tr>\n")
   );
   
   UString item;
   
      for (uint32_t i = 0; i < sz; ++i)
         {
         item = strings[i];
   
         USP_PRINTF("<tr><td>%v</td><td><a href=\"?remove=%u\">remove</a></td></tr>", item.rep, i);
         }
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("\t</table></p>\n</body> \n</html>")
   );
   
   USP_SESSION_VAR_PUT(0,strings);
   
   UClientImage_Base::setRequestNoCache();
   
   
} }