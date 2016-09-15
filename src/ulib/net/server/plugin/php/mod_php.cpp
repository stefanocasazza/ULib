// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_php.cpp - this is a wrapper to embed the PHP interpreter
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/command.h>
#include <ulib/utility/uhttp.h>
#include <ulib/net/server/server.h>

#undef PACKAGE_URL
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef PACKAGE_BUGREPORT

#ifdef restrict
#undef restrict
#endif
#ifdef HAVE_SNPRINTF
#undef HAVE_SNPRINTF
#endif

#include <php_embed.h>
//#include <php/Zend/zend_stream.h>

#ifdef snprintf
#undef snprintf
#endif

extern "C" {

static void UPHP_set_environment(void* env, char* name, char* value)
{
   U_TRACE(0, "UPHP_set_environment(%p,%S,%S)", env, name, value)

   php_register_variable_safe(name, value, strlen(value), track_vars_array TSRMLS_CC);
}

static void register_server_variables(zval* track_vars_array TSRMLS_DC)
{
   U_TRACE(0, "PHP::register_server_variables(%p)", track_vars_array)

   php_import_environment_variables(track_vars_array TSRMLS_CC);

   (void) UHTTP::setEnvironmentForLanguageProcessing(U_PHP, 0, UPHP_set_environment);
}

static int send_headers(sapi_headers_struct* sapi_headers)
{
   U_TRACE(0, "PHP::send_headers(%p)", sapi_headers)

   if (SG(request_info).no_headers == 1) U_http_info.endHeader = 0;
   else
      {
      zend_llist_position pos;
      sapi_header_struct* h = (sapi_header_struct*) zend_llist_get_first_ex(&sapi_headers->headers, &pos);

      while (h)
         {
         (void) UClientImage_Base::wbuffer->append(h->header, h->header_len);
         (void) UClientImage_Base::wbuffer->append(U_CONSTANT_TO_PARAM(U_CRLF));

         h = (sapi_header_struct*) zend_llist_get_next_ex(&sapi_headers->headers, &pos);
         }

      (void) UClientImage_Base::wbuffer->append(U_CONSTANT_TO_PARAM(U_CRLF));

      U_http_info.endHeader = UClientImage_Base::wbuffer->size();
      }

   U_http_info.nResponseCode = SG(sapi_headers).http_response_code;

   if (U_IS_HTTP_VALID_RESPONSE(U_http_info.nResponseCode) == false) U_http_info.nResponseCode = HTTP_OK; 

   U_DUMP("HTTP status = (%d %S)", U_http_info.nResponseCode, UHTTP::getStatusDescription())

   return SAPI_HEADER_SENT_SUCCESSFULLY;
}

static int ub_write(const char* str, unsigned int strlen TSRMLS_DC) // this is the stdout output of a PHP script
{
   U_TRACE(0, "PHP::ub_write(%.*S,%u)", strlen, str, strlen)

   (void) UClientImage_Base::wbuffer->append(str, strlen);

   return strlen;
}

static char* read_cookies()
{
   U_TRACE_NO_PARAM(0, "PHP::read_cookies()")

   if (U_http_info.cookie_len) return estrndup(U_http_info.cookie, U_http_info.cookie_len);

   return 0;
}

static void log_message(char* message)
{
   U_TRACE(0, "PHP::log_message(%S)", message)

   U_SRV_LOG("%s", message);
}

extern U_EXPORT bool initPHP()
       U_EXPORT bool initPHP()
{
   U_TRACE(0, "::initPHP()")

   /**
    * char str[512];
    * zval ret_value;
    * int exit_status;
    * zend_first_try
    * {
    * PG(during_request_startup) = 0;
    * snprintf(str, sizeof(str), "include (\"%s\");", UHTTP::file->getPathRelativ());
    * zend_eval_string(str, &ret_value, str TSRMLS_CC);
    * exit_status = Z_LVAL(ret_value);
    * } zend_catch
    * {
    * exit_status = EG(exit_status);
    * }
    * zend_end_try();
    * return exit_status;
    */

   U_SET_MODULE_NAME(php);

   // -------------------------------------------------------------
   // extern EMBED_SAPI_API sapi_module_struct php_embed_module = {
   // -------------------------------------------------------------
   // "embed",                      /* name */
   // "PHP Embedded Library",       /* pretty name */
   // php_embed_startup,            /* startup */
   // php_module_shutdown_wrapper,  /* shutdown */
   // NULL,                         /* activate */
   // php_embed_deactivate,         /* deactivate */
   // php_embed_ub_write,           /* unbuffered write */
   // php_embed_flush,              /* flush */
   // NULL,                         /* get uid */
   // NULL,                         /* getenv */
   // php_error,                    /* error handler */
   // NULL,                         /* header handler */
   // NULL,                         /* send headers handler */
   // php_embed_send_header,        /* send header handler */
   // NULL,                         /* read POST data */
   // php_embed_read_cookies,       /* read Cookies */
   // php_embed_register_variables, /* register server variables */
   // php_embed_log_message,        /* Log message */
   // NULL,                         /* Get request time */
   // NULL,                         /* Child terminate */
   // -------------------------------------------------------------
   // STANDARD_SAPI_MODULE_PROPERTIES
   // -------------------------------------------------------------
   // char* php_ini_path_override;
   // void (*block_interruptions)(void);
   // void (*unblock_interruptions)(void);
   // void (*default_post_reader)(TSRMLS_D);
   // void (*treat_data)(int arg, char* str, zval* destArray TSRMLS_DC);
   // char* executable_location;
   // int php_ini_ignore;
   // int php_ini_ignore_cwd; /* don't look for php.ini in the current directory */
   // int (*get_fd)(int* fd TSRMLS_DC);
   // int (*force_http_10)(TSRMLS_D);
   // int (*get_target_uid)(uid_t* TSRMLS_DC);
   // int (*get_target_gid)(gid_t* TSRMLS_DC);
   // unsigned int (*input_filter)(int arg, char* var, char** val, unsigned int val_len, unsigned int* new_val_len TSRMLS_DC);
   // void (*ini_defaults)(HashTable *configuration_hash);
   // int phpinfo_as_text;
   // char* ini_entries;
   // const zend_function_entry* additional_functions;
   // unsigned int (*input_filter_init)(TSRMLS_D);
   // -------------------------------------------------------------
   // };
   // -------------------------------------------------------------

   // set up the callbacks
   php_embed_module.ub_write                  = ub_write;
   php_embed_module.log_message               = log_message;
   php_embed_module.send_headers              = send_headers;
   php_embed_module.read_cookies              = read_cookies;
   php_embed_module.register_server_variables = register_server_variables;

   sapi_startup(&php_embed_module);

   // applying custom options
   // ....
   //

   (void) php_embed_module.startup(&php_embed_module);

   U_SRV_LOG("PHP(%s) initialized", PHP_VERSION);

end:
   U_RESET_MODULE_NAME;

   U_RETURN(true);
}

extern U_EXPORT bool runPHP();
       U_EXPORT bool runPHP()
{
   U_TRACE(0, "::runPHP()")

   /**
    * char str[512];
    * zval ret_value;
    * int exit_status;
    * zend_first_try
    * {
    * PG(during_request_startup) = 0;
    * snprintf(str, sizeof(str), "include (\"%s\");", UHTTP::file->getPathRelativ());
    * zend_eval_string(str, &ret_value, str TSRMLS_CC);
    * exit_status = Z_LVAL(ret_value);
    * } zend_catch
    * {
    * exit_status = EG(exit_status);
    * }
    * zend_end_try();
    * return exit_status;
    */

   bool esito = true;

   U_SET_MODULE_NAME(php);

   zend_file_handle file_handle;

   file_handle.type          = ZEND_HANDLE_FILENAME;
   file_handle.filename      = UHTTP::file->getPathRelativ();
   file_handle.free_filename = 0;
   file_handle.opened_path   = 0;

   if (php_request_startup(TSRMLS_C))
      {
      esito = false;

      goto end;
      }

   php_execute_script(&file_handle TSRMLS_CC);

   php_request_shutdown(0);

   UClientImage_Base::environment->setEmpty();

end:
   U_RESET_MODULE_NAME;

   U_RETURN(esito);
}

extern U_EXPORT void endPHP();
       U_EXPORT void endPHP()
{
   U_TRACE_NO_PARAM(0, "endPHP()")

   TSRMLS_FETCH();
   php_module_shutdown(TSRMLS_C);
   sapi_shutdown();

   if (php_embed_module.ini_entries)
      {
#  ifdef DEBUG
      (void) UFile::writeToTmp(php_embed_module.ini_entries, strlen(php_embed_module.ini_entries), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("php_embed_module.ini"), 0);
#  endif

      free(php_embed_module.ini_entries);

      php_embed_module.ini_entries = 0;
      }
}
/**
 * extern U_EXPORT void UPHP_set_var(const char* varname, const char* varval);
 *        U_EXPORT void UPHP_set_var(const char* varname, const char* varval)
 * {
 *    zval* var;
 *    MAKE_STD_ZVAL(var);
 *    ZVAL_STRING(var, varval, 1);
 *    zend_hash_update(&EG(symbol_table), varname, strlen(varname) + 1, &var, sizeof(zval*), 0);
 * }
 * extern U_EXPORT const char* UPHP_get_var(const char* varname);
 *        U_EXPORT const char* UPHP_get_var(const char* varname)
 * {
 *    zval** data = 0;
 *    const char* ret = NULL;
 *    if (zend_hash_find(&EG(symbol_table), varname, strlen(varname) + 1, (void**)&data) == FAILURE)
 *       {
 *       printf("Name not found in $GLOBALS\n");
 *       return "";
 *       }
 *    if (data == 0)
 *       {
 *       printf("Value is NULL (not possible for symbol_table?)\n");
 *       return "";
 *       }
 *    ret = Z_STRVAL_PP(data);
 *    return ret;
 * }
 */
}
