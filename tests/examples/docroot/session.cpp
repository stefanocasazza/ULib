// session.cpp - dynamic page translation (session.usp => session.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
static void usp_init_session()
{
   U_TRACE(5, "::usp_init_session()")
   
   if (UHTTP::data_session == 0)   U_NEW(UDataSession, UHTTP::data_storage, UDataSession);
   
   if (UHTTP::db_session == 0) UHTTP::initSession();
}  
   
extern "C" {
extern U_EXPORT void runDynamicPage_session(int param);
       U_EXPORT void runDynamicPage_session(int param)
{
   U_TRACE(0, "::runDynamicPage_session(%d)", param)
   
   uint32_t usp_sz = 0;
   char usp_buffer[10 * 4096];
   
   if (param)
      {
      if (param == U_DPAGE_INIT) { usp_init_session(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
   UHTTP::mime_index = U_html;
   
   U_http_info.endHeader = 0;
   
   unsigned access_count(0);
   
   USP_SESSION_VAR_GET(0,access_count);
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("<html>\n <head><title>Example of USP with HTTP session</title></head>\n <body bgcolor=\"#FDF5E6\">\n\n   <h1 align=\"center\">Welcome")
   );
   
   (void) UClientImage_Base::wbuffer->append(((UHTTP::isNewSession() ?  ", Newcomer" : " Back")));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</h1>\n\t<h2>Information on Your Session:</h2>\n\t<table border=1 align=center>\n\t<tr bgcolor=\"#ffad00\">\n\t  <th>Info Type</th><th>Value</th>\n\t</tr>\n\t<tr>\n\t  <td>keyIdDataSession</td>\n\t  <td>")
   );
   
   (void) UClientImage_Base::wbuffer->append((UHTTP::getKeyIdDataSession()));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</td>\n\t</tr>\n\t<tr>\n\t  <td>Creation Time</td>\n\t  <td>")
   );
   
   (void) UClientImage_Base::wbuffer->append((UHTTP::getSessionCreationTime()));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</td>\n\t</tr>\n\t<tr>\n\t  <td>Time of Last Access</td>\n\t  <td>")
   );
   
   (void) UClientImage_Base::wbuffer->append((UHTTP::getSessionLastAccessedTime()));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</td>\n\t</tr>\n\t<tr>\n\t  <td>Number of Previous Accesses</td>\n\t  <td>")
   );
   
   UStringExt::appendNumber32(*UClientImage_Base::wbuffer, (access_count++));
   
   (void) UClientImage_Base::wbuffer->append(
      U_CONSTANT_TO_PARAM("</td>\n\t</tr>\n\t</table>\n\t<br><a href=\"session\">reload</a>\n</body> \n</html>")
   );
   
   USP_SESSION_VAR_PUT(0,access_count);
   
   UClientImage_Base::setRequestNoCache();
   
   
} }