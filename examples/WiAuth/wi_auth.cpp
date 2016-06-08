// wi_auth.cpp - dynamic page translation (wi_auth.usp => wi_auth.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
static void usp_end_wi_auth();
   static void usp_init_wi_auth();
   static void usp_sighup_wi_auth();
   
   static long start_op;
   
   #include <ulib/examples/wi_auth_declaration.h>  
   
extern "C" {
extern U_EXPORT void runDynamicPage_wi_auth(int param);
       U_EXPORT void runDynamicPage_wi_auth(int param)
{
   U_TRACE(0, "::runDynamicPage_wi_auth(%d)", param)
   
   
   if (param)
      {
      if (param == U_DPAGE_INIT) { usp_init_wi_auth(); return; }
   
      if (param == U_DPAGE_DESTROY) { usp_end_wi_auth(); return; }
   
      if (param == U_DPAGE_SIGHUP) { usp_sighup_wi_auth(); return; }
   
      if (param >= U_DPAGE_FORK) return;
      }
   
   U_http_info.endHeader = 0;
   
   static UHTTP::service_info GET_table[] = { // NB: the table must be ordered alphabetically for binary search...
      GET_ENTRY(admin),
      GET_ENTRY(admin_continuing_status_ap),
      GET_ENTRY(admin_current_status_ap),
      GET_ENTRY(admin_edit_ap),
      GET_ENTRY(admin_export_statistics_login_as_csv),
      GET_ENTRY(admin_export_statistics_registration_as_csv),
      GET_ENTRY(admin_export_view_using_historical_as_csv),
      GET_ENTRY(admin_historical_statistics_login),
      GET_ENTRY(admin_login_nodog),
      GET_ENTRY(admin_login_nodog_historical),
      GET_ENTRY(admin_login_nodog_historical_view_data),
      GET_ENTRY(admin_printlog),
      GET_ENTRY(admin_recovery),
      GET_ENTRY(admin_status_network),
      GET_ENTRY(admin_status_nodog),
      GET_ENTRY(admin_status_nodog_and_user),
      GET_ENTRY(admin_status_nodog_and_user_as_csv),
      GET_ENTRY(admin_view_statistics_login),
      GET_ENTRY(admin_view_statistics_registration),
      GET_ENTRY(admin_view_user),
      GET_ENTRY(admin_view_using),
      GET_ENTRY(admin_view_using_historical),
      GET_ENTRY(calendar),
      GET_ENTRY(error_ap),
      GET_ENTRY(fake_login_validate),
      GET_ENTRY(gen_activation),
      GET_ENTRY(get_ap_check_firewall),
      GET_ENTRY(get_ap_check_zombies),
      GET_ENTRY(get_ap_name),
      GET_ENTRY(get_ap_uptime),
      GET_ENTRY(get_config),
      GET_ENTRY(get_users_info),
      GET_ENTRY(help_wifi),
      GET_ENTRY(info),
      GET_ENTRY(logged),
      GET_ENTRY(logged_login_request),
      GET_ENTRY(login),
      GET_ENTRY(login_request),
      GET_ENTRY(login_request_IdP),
      GET_ENTRY(login_request_by_MAC),
      GET_ENTRY(login_validate),
      GET_ENTRY(logout_page),
      GET_ENTRY(password),
      GET_ENTRY(polling_attivazione),
      GET_ENTRY(polling_password),
      GET_ENTRY(postlogin),
      GET_ENTRY(recovery),
      GET_ENTRY(registrazione),
      GET_ENTRY(reset_counter_ap),
      GET_ENTRY(reset_policy),
      GET_ENTRY(resync),
      GET_ENTRY(start_ap),
      GET_ENTRY(stato_utente),
      GET_ENTRY(status_ap),
      GET_ENTRY(status_ap_no_label),
      GET_ENTRY(tavarnelle),
      GET_ENTRY(webif_ap)
   };
   
   static UHTTP::service_info POST_table[] = { // NB: the table must be ordered alphabetically for binary search...
      POST_ENTRY(admin_edit_ap),
      POST_ENTRY(admin_execute_recovery),
      POST_ENTRY(admin_recovery),
      POST_ENTRY(admin_view_user),
      POST_ENTRY(info),
      POST_ENTRY(login_request),
      POST_ENTRY(login_request_IdP),
      POST_ENTRY(password),
      POST_ENTRY(registrazione),
      POST_ENTRY(roaming),
      POST_ENTRY(tavarnelle),
      POST_ENTRY(uploader)
   };
   
   start_op = u_now->tv_sec;
   
   U_http_info.nResponseCode = 0;
   
   UHTTP::manageRequest( GET_table, sizeof( GET_table) / sizeof( GET_table[0]),
                        POST_table, sizeof(POST_table) / sizeof(POST_table[0]));
   
   U_INTERNAL_DUMP("U_http_info.nResponseCode = %d", U_http_info.nResponseCode)
   
   // NB: it is used by server_plugin_ssi to continue processing with a shell script...
   
   if (U_http_info.nResponseCode == 0) (void) UClientImage_Base::environment->append(U_CONSTANT_TO_PARAM("HTTP_RESPONSE_CODE=0\n"));
   
   UClientImage_Base::setRequestNoCache();
   
   
} }