// wi_auth2.cpp - dynamic page translation (wi_auth2.usp => wi_auth2.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
static void usp_end_wi_auth2();
static void usp_init_wi_auth2();
static void usp_fork_wi_auth2();
#include "wi_auth_declaration2.h"
   
extern "C" {
extern U_EXPORT void runDynamicPage_wi_auth2(int param);
       U_EXPORT void runDynamicPage_wi_auth2(int param)
{
   U_TRACE(0, "::runDynamicPage_wi_auth2(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_INIT) { usp_init_wi_auth2(); return; }
      if (param == U_DPAGE_DESTROY) { usp_end_wi_auth2(); return; }
      if (param == U_DPAGE_FORK) { usp_fork_wi_auth2(); return; }
      return;
      }
   
   U_http_info.endHeader = 0;
   static UHTTP::service_info GET_table[] = { // NB: the table must be ordered alphabetically for binary search...
      GET_ENTRY(anagrafica),
      GET_ENTRY(checkCaptive),
      GET_ENTRY(clean),
      GET_ENTRY(cleanSession),
      GET_ENTRY(get_config),
      GET_ENTRY(logout),
      GET_ENTRY(start_ap),
      GET_ENTRY(welcome)
   };
   
   static UHTTP::service_info POST_table[] = { // NB: the table must be ordered alphabetically for binary search...
      POST_ENTRY(info),
      POST_ENTRY(login),
      POST_ENTRY(notify),
      POST_ENTRY(strict_notify)
   };
   
   UHTTP::manageRequest(GET_table, U_NUM_ELEMENTS(GET_table), POST_table, U_NUM_ELEMENTS(POST_table));
   
   
} }