// index.cpp - dynamic page translation (index.usp => index.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
   
static void usp_end_index();
static void usp_init_index();
static void usp_fork_index();
static void usp_auth_index();
static void usp_config_index();
#include "example_declaration.h"
   
extern "C" {
extern U_EXPORT void runDynamicPageParam_index(uint32_t param);
       U_EXPORT void runDynamicPageParam_index(uint32_t param)
{
   U_TRACE(0, "::runDynamicPageParam_index(%u)", param)
   
   if (param == U_DPAGE_INIT) { usp_init_index(); return; }
   if (param == U_DPAGE_DESTROY) { usp_end_index(); return; }
   if (param == U_DPAGE_FORK) { usp_fork_index(); return; }
   if (param == U_DPAGE_CONFIG){ usp_config_index(); return; }
   if (param == U_DPAGE_AUTH) { usp_auth_index(); return; }
   return;
} }
   
extern "C" {
extern U_EXPORT void runDynamicPage_index();
       U_EXPORT void runDynamicPage_index()
{
   U_TRACE_NO_PARAM(0, "::runDynamicPage_index()")
   
   U_http_info.endHeader = 0;
   if (UHTTP::getLoginCookie() ||
       UHTTP::getPostLoginUserPasswd())
      {
      UHTTP::usp->runDynamicPageParam(U_DPAGE_AUTH);
      }
   if (UHTTP::loginCookie->empty())
      {
      UHTTP::UServletPage* usp_save = UHTTP::usp;
      if (UHTTP::getUSP(U_CONSTANT_TO_PARAM("login_form")))
         {
         UHTTP::usp->runDynamicPageParam(U_DPAGE_AUTH);
         UHTTP::usp = usp_save;
         return;
         }
      UHTTP::UFileCacheData* login_form_html = UHTTP::getFileCachePointer(U_CONSTANT_TO_PARAM("login_form.html"));
      if (login_form_html)
         {
         UHTTP::setResponseFromFileCache(login_form_html);
         U_http_info.nResponseCode = HTTP_NO_CONTENT; // NB: to escape management after usp exit...
         return;
         }
      (void) UClientImage_Base::wbuffer->append(U_CONSTANT_TO_PARAM(
"<form method=\"post\">\n"
" <p>Login</p>\n"
" <p>username <input name=\"user\" type=\"text\" class=\"inputbox\" title=\"Enter your username\"></p>\n"
" <p>password <input name=\"pass\" type=\"text\" class=\"inputbox\" title=\"Enter your password\"></p>\n"
" <p><input type=\"submit\" value=\"login\"></p>\n"
"</form>"));
      return;
      }
      
   static UHTTP::service_info GET_table[] = { // NB: the table must be ordered alphabetically for binary search...
      GET_ENTRY(logout),
      GET_ENTRY(welcome)
   };
   
   static UHTTP::service_info POST_table[] = { // NB: the table must be ordered alphabetically for binary search...
      POST_ENTRY(service)
   };
   
   UHTTP::manageRequest(GET_table, U_NUM_ELEMENTS(GET_table), POST_table, U_NUM_ELEMENTS(POST_table));
   
   U_http_info.endHeader = 0;
} }