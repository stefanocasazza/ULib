// wi_auth.cpp - dynamic page translation (wi_auth.usp => wi_auth.cpp)
   
#include <ulib/net/server/usp_macro.h>
   
static void usp_end_wi_auth();
   static void usp_init_wi_auth();
   static void usp_sighup_wi_auth();
   
   static long start_op;
   
   #include <ulib/examples/wi_auth_declaration.h>  
   
extern "C" {
extern U_EXPORT int runDynamicPage_wi_auth(UClientImage_Base* client_image);
       U_EXPORT int runDynamicPage_wi_auth(UClientImage_Base* client_image)
{
   U_TRACE(0, "::runDynamicPage_wi_auth(%p)", client_image)
   
   
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
      if (client_image == (void*)-1) { usp_init_wi_auth(); U_RETURN(0); }
   
      if (client_image == (void*)-3) { usp_end_wi_auth(); U_RETURN(0); }
   
      if (client_image == (void*)-4) { usp_sighup_wi_auth(); U_RETURN(0); }
   
      if (client_image >= (void*)-5) U_RETURN(0);
   
      U_http_info.endHeader = 0;
      }
      
   static UHTTP::service_info GET_table[] = { // NB: the table must be ordered alphabetically for binary search...
      GET_ENTRY(admin),
      GET_ENTRY(admin_edit_ap),
      GET_ENTRY(admin_export_statistics_login_as_csv),
      GET_ENTRY(admin_export_statistics_registration_as_csv),
      GET_ENTRY(admin_export_view_using_historical_as_csv),
      GET_ENTRY(admin_historical_statistics_login),
      GET_ENTRY(admin_login_nodog),
      GET_ENTRY(admin_printlog),
      GET_ENTRY(admin_recovery),
      GET_ENTRY(admin_status_network),
      GET_ENTRY(admin_status_nodog),
      GET_ENTRY(admin_view_statistics_login),
      GET_ENTRY(admin_view_statistics_registration),
      GET_ENTRY(admin_view_user),
      GET_ENTRY(admin_view_using),
      GET_ENTRY(admin_view_using_historical),
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
      /*
      GET_ENTRY(unifi),
      GET_ENTRY(unifi_login_request),
      */
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
      POST_ENTRY(uploader)
   };
   
   start_op = u_now->tv_sec;
   
   UHTTP::manageRequest( GET_table, sizeof( GET_table) / sizeof( GET_table[0]),
                        POST_table, sizeof(POST_table) / sizeof(POST_table[0]));
   
      UClientImage_Base::setRequestNoCache();
   
   U_RETURN(200);
} }