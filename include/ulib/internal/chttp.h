/* ============================================================================
 *
 * LIBRARY
 *    ULib - c++ library
 *
 * FILENAME
 *    chttp.h - HTTP definition for C binding
 *
 * AUTHOR
 *    Stefano Casazza
 *
 * ============================================================================ */

#ifndef ULIB_CHTTP_H
#define ULIB_CHTTP_H 1

/**
 * -------------------------------------------------------------------------------------------------------
 *  _     _   _
 * | |__ | |_| |_ _ __
 * | '_ \| __| __| '_ \
 * | | | | |_| |_| |_) |
 * |_| |_|\__|\__| .__/
 *                 |_|
 *
 * ---------------------------------------------------------------------------------------------------------
 * HTTP message handler
 *
 * The status code is a three-digit integer, and the first digit identifies the general category of response
 * ---------------------------------------------------------------------------------------------------------
 */

/* 1xx indicates an informational message only */
#define HTTP_CONTINUE                        100
#define HTTP_SWITCH_PROT                     101
#define HTTP_STATUS_102                      102 /* "Processing" */

/* 2xx indicates success of some kind */
#define HTTP_OK                              200
#define HTTP_CREATED                         201
#define HTTP_ACCEPTED                        202
#define HTTP_NOT_AUTHORITATIVE               203
#define HTTP_NO_CONTENT                      204
#define HTTP_RESET                           205
#define HTTP_PARTIAL                         206
#define HTTP_STATUS_207                      207 /* "Multi-Status" */
#define HTTP_OPTIONS_RESPONSE                222 /* only internal use, not standard */

/* 3xx redirects the client to another URL */
#define HTTP_MULT_CHOICE                     300
#define HTTP_MOVED_PERM                      301
#define HTTP_MOVED_TEMP                      302
#define HTTP_FOUND                           302
#define HTTP_SEE_OTHER                       303
#define HTTP_NOT_MODIFIED                    304
#define HTTP_USE_PROXY                       305
#define HTTP_TEMP_REDIR                      307

/* 4xx indicates an error on the client's part */
#define HTTP_BAD_REQUEST                     400
#define HTTP_UNAUTHORIZED                    401
#define HTTP_PAYMENT_REQUIRED                402
#define HTTP_FORBIDDEN                       403
#define HTTP_NOT_FOUND                       404
#define HTTP_BAD_METHOD                      405
#define HTTP_NOT_ACCEPTABLE                  406
#define HTTP_PROXY_AUTH                      407
#define HTTP_CLIENT_TIMEOUT                  408
#define HTTP_CONFLICT                        409
#define HTTP_GONE                            410
#define HTTP_LENGTH_REQUIRED                 411
#define HTTP_PRECON_FAILED                   412
#define HTTP_ENTITY_TOO_LARGE                413
#define HTTP_REQ_TOO_LONG                    414
#define HTTP_UNSUPPORTED_TYPE                415
#define HTTP_REQ_RANGE_NOT_OK                416
#define HTTP_EXPECTATION_FAILED              417
#define HTTP_STATUS_418                      418 /* "I'm a teapot" */
#define HTTP_UNPROCESSABLE_ENTITY            422
#define HTTP_STATUS_423                      423 /* "Locked" */
#define HTTP_STATUS_424                      424 /* "Failed Dependency" */
#define HTTP_STATUS_425                      425 /* "Unordered Collection" */
#define HTTP_STATUS_426                      426 /* "Upgrade Required" */
#define HTTP_PRECONDITION_REQUIRED           428
#define HTTP_TOO_MANY_REQUESTS               429
#define HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE 431

/* 5xx indicates an error on the server's part */
#define HTTP_INTERNAL_ERROR                  500
#define HTTP_NOT_IMPLEMENTED                 501
#define HTTP_BAD_GATEWAY                     502
#define HTTP_UNAVAILABLE                     503
#define HTTP_GATEWAY_TIMEOUT                 504
#define HTTP_VERSION                         505
#define HTTP_STATUS_506                      506 /* "Variant Also Negotiates" */
#define HTTP_STATUS_507                      507 /* "Insufficient Storage" */
#define HTTP_STATUS_509                      509 /* "Bandwidth Limit Exceeded" */
#define HTTP_STATUS_510                      510 /* "Not Extended" */
#define HTTP_NETWORK_AUTHENTICATION_REQUIRED 511

#define U_IS_HTTP_INFO(x)           (((x)>=100)&&((x)<200)) /* is the status code informational */
#define U_IS_HTTP_SUCCESS(x)        (((x)>=200)&&((x)<300)) /* is the status code OK ? */
#define U_IS_HTTP_REDIRECT(x)       (((x)>=300)&&((x)<400)) /* is the status code a redirect */
#define U_IS_HTTP_ERROR(x)          (((x)>=400)&&((x)<600)) /* is the status code a error (client or server) */
#define U_IS_HTTP_CLIENT_ERROR(x)   (((x)>=400)&&((x)<500)) /* is the status code a client error */
#define U_IS_HTTP_SERVER_ERROR(x)   (((x)>=500)&&((x)<600)) /* is the status code a server error */
#define U_IS_HTTP_VALID_RESPONSE(x) (((x)>=100)&&((x)<600)) /* is the status code a (potentially) valid response code ? */
 
/* should the status code drop the connection ? */
#define U_STATUS_DROPS_CONNECTION(x)              \
(((x) == HTTP_MOVED_TEMP)                      || \
 ((x) == HTTP_BAD_REQUEST)                     || \
 ((x) == HTTP_CLIENT_TIMEOUT)                  || \
 ((x) == HTTP_LENGTH_REQUIRED)                 || \
 ((x) == HTTP_PRECON_FAILED)                   || \
 ((x) == HTTP_ENTITY_TOO_LARGE)                || \
 ((x) == HTTP_REQ_TOO_LONG)                    || \
 ((x) == HTTP_INTERNAL_ERROR)                  || \
 ((x) == HTTP_UNAVAILABLE)                     || \
 ((x) == HTTP_NETWORK_AUTHENTICATION_REQUIRED) || \
 ((x) == HTTP_NOT_IMPLEMENTED))

/**
 * HTTP header representation
 *
 * sizeof(struct uhttpinfo) 64bit == 144
 */

typedef struct uhttpinfo {
   const char* uri;
   const char* query;
   const char* host;
   const char* range;
   const char* cookie;
   const char* accept;
   const char* referer;
   const char* ip_client;
   const char* user_agent;
   const char* content_type;
   const char* accept_language;

   /* RESET == 52 */
   uint16_t nResponseCode, cookie_len, referer_len, user_agent_len;
   uint32_t if_modified_since, startHeader, endHeader, clength, uri_len, query_len, method_type;
   unsigned char flag[16];
} uhttpinfo;

enum HTTPMethodType {
/* request methods */
   HTTP_GET         = 0x00000001,
   HTTP_HEAD        = 0x00000002,
   HTTP_POST        = 0x00000004,
   HTTP_PUT         = 0x00000008,
   HTTP_DELETE      = 0x00000010,
   HTTP_OPTIONS     = 0x00000020,
/* pathological */
   HTTP_TRACE       = 0x00000040,
   HTTP_CONNECT     = 0x00000080,
/* webdav */
   HTTP_COPY        = 0x00000100,
   HTTP_MOVE        = 0x00000200,
   HTTP_LOCK        = 0x00000400,
   HTTP_UNLOCK      = 0x00000800,
   HTTP_MKCOL       = 0x00001000,
   HTTP_SEARCH      = 0x00002000,
   HTTP_PROPFIND    = 0x00004000,
   HTTP_PROPPATCH   = 0x00008000,
/* rfc-5789 */
   HTTP_PATCH       = 0x00010000,
   HTTP_PURGE       = 0x00020000,
/* subversion */
   HTTP_MERGE       = 0x00040000,
   HTTP_REPORT      = 0x00080000,
   HTTP_CHECKOUT    = 0x00100000,
   HTTP_MKACTIVITY  = 0x00200000,
/* upnp */
   HTTP_NOTIFY      = 0x00400000,
   HTTP_MSEARCH     = 0x00800000,
   HTTP_SUBSCRIBE   = 0x01000000,
   HTTP_UNSUBSCRIBE = 0x02000000
};

typedef struct uhttpmethodtype {
   const char* name;
   uint32_t len;
} uhttpmethodtype;

typedef struct uclientimage_info {
   union uucflag64 flag;
   struct uhttpmethodtype http_method_list[26];
   struct uhttpinfo http_info;
} uclientimage_info;

#ifdef __cplusplus
extern "C" {
#endif
extern U_EXPORT uclientimage_info u_clientimage_info;
#ifdef __cplusplus
}
#endif

#define U_PARALLELIZATION_CHILD  1
#define U_PARALLELIZATION_PARENT 2

#define U_http_info        u_clientimage_info.http_info
#define U_clientimage_flag u_clientimage_info.flag
#define U_http_method_list u_clientimage_info.http_method_list
#define U_http_method_type u_clientimage_info.http_info.method_type

#define U_line_terminator_len                    u_clientimage_info.flag.c[0]

#define U_ClientImage_state                      u_clientimage_info.flag.c[1]
#define U_ClientImage_close                      u_clientimage_info.flag.c[2]
#define U_ClientImage_request                    u_clientimage_info.flag.c[3]
#define U_ClientImage_pipeline                   u_clientimage_info.flag.c[4]
#define U_ClientImage_data_missing               u_clientimage_info.flag.c[5]
#define U_ClientImage_parallelization            u_clientimage_info.flag.c[6]
#define U_ClientImage_advise_for_parallelization u_clientimage_info.flag.c[7]

#define U_http_version                 u_clientimage_info.http_info.flag[ 0]
#define U_http_method_num              u_clientimage_info.http_info.flag[ 1]
#define U_http_host_len                u_clientimage_info.http_info.flag[ 2]
#define U_http_host_vlen               u_clientimage_info.http_info.flag[ 3]
#define U_http_range_len               u_clientimage_info.http_info.flag[ 4]
#define U_http_accept_len              u_clientimage_info.http_info.flag[ 5]
#define U_http_websocket_len           u_clientimage_info.http_info.flag[ 6]
#define U_http2_settings_len           u_clientimage_info.http_info.flag[ 7]
#define U_http_ip_client_len           u_clientimage_info.http_info.flag[ 8]
#define U_http_content_type_len        u_clientimage_info.http_info.flag[ 9]
#define U_http_accept_language_len     u_clientimage_info.http_info.flag[10]
#define U_http_len_user1               u_clientimage_info.http_info.flag[12]
#define U_http_len_user2               u_clientimage_info.http_info.flag[13]
#define U_http_len_user3               u_clientimage_info.http_info.flag[14]
#define U_http_len_user4               u_clientimage_info.http_info.flag[15]

#define U_http_flag                    u_clientimage_info.http_info.flag[11]
#define U_http_flag_save               UHttpClient_Base::u_http_info_save.flag[11]

enum HttpRequestType {
   HTTP_IS_SENDFILE            = 0x0001,
   HTTP_IS_KEEP_ALIVE          = 0x0002,
   HTTP_IS_DATA_CHUNKED        = 0x0004,
   HTTP_IS_ACCEPT_GZIP         = 0x0008,
   HTTP_IS_NOCACHE_FILE        = 0x0010,
   HTTP_IS_RESPONSE_GZIP       = 0x0020,
   HTTP_IS_REQUEST_NOSTAT      = 0x0040,
   HTTP_METHOD_NOT_IMPLEMENTED = 0x0080
};

#define U_http_sendfile               ((U_http_flag & HTTP_IS_SENDFILE)            != 0)
#define U_http_keep_alive             ((U_http_flag & HTTP_IS_KEEP_ALIVE)          != 0)
#define U_http_data_chunked           ((U_http_flag & HTTP_IS_DATA_CHUNKED)        != 0)
#define U_http_is_nocache_file        ((U_http_flag & HTTP_IS_NOCACHE_FILE)        != 0)
#define U_http_is_response_gzip       ((U_http_flag & HTTP_IS_RESPONSE_GZIP)       != 0)
#define U_http_is_request_nostat      ((U_http_flag & HTTP_IS_REQUEST_NOSTAT)      != 0)
#define U_http_method_not_implemented ((U_http_flag & HTTP_METHOD_NOT_IMPLEMENTED) != 0)

#define U_http_is_accept_gzip         ((U_http_flag      & HTTP_IS_ACCEPT_GZIP)    != 0)
#define U_http_is_accept_gzip_save    ((U_http_flag_save & HTTP_IS_ACCEPT_GZIP)    != 0)

#define U_HTTP_INFO_INIT(c)  (void) U_SYSCALL(memset, "%p,%d,%u", &(u_clientimage_info.http_info),               c, sizeof(uhttpinfo))
#define U_HTTP_INFO_RESET(c) (void) U_SYSCALL(memset, "%p,%d,%u", &(u_clientimage_info.http_info.nResponseCode), c, 52)

#define U_HTTP_URI_TO_PARAM u_clientimage_info.http_info.uri, u_clientimage_info.http_info.uri_len
#define U_HTTP_URI_TO_TRACE u_clientimage_info.http_info.uri_len, u_clientimage_info.http_info.uri

#define U_HTTP_QUERY_TO_PARAM u_clientimage_info.http_info.query, u_clientimage_info.http_info.query_len
#define U_HTTP_QUERY_TO_TRACE u_clientimage_info.http_info.query_len, u_clientimage_info.http_info.query

#define U_HTTP_URI_QUERY_LEN (u_clientimage_info.http_info.uri_len + u_clientimage_info.http_info.query_len + (u_clientimage_info.http_info.query_len ? 1 : 0))

#define U_HTTP_URI_QUERY_TO_PARAM u_clientimage_info.http_info.uri, U_HTTP_URI_QUERY_LEN 
#define U_HTTP_URI_QUERY_TO_TRACE U_HTTP_URI_QUERY_LEN, u_clientimage_info.http_info.uri

#define U_HTTP_CTYPE_TO_PARAM u_clientimage_info.http_info.content_type, U_http_content_type_len
#define U_HTTP_CTYPE_TO_TRACE U_http_content_type_len, u_clientimage_info.http_info.content_type

#define U_HTTP_RANGE_TO_PARAM u_clientimage_info.http_info.range, U_http_range_len
#define U_HTTP_RANGE_TO_TRACE U_http_range_len, u_clientimage_info.http_info.range

#define U_HTTP_COOKIE_TO_PARAM u_clientimage_info.http_info.cookie, u_clientimage_info.http_info.cookie_len
#define U_HTTP_COOKIE_TO_TRACE u_clientimage_info.http_info.cookie_len, u_clientimage_info.http_info.cookie

#define U_HTTP_REFERER_TO_PARAM u_clientimage_info.http_info.referer, u_clientimage_info.http_info.referer_len
#define U_HTTP_REFERER_TO_TRACE u_clientimage_info.http_info.referer_len, u_clientimage_info.http_info.referer

#define U_HTTP_IP_CLIENT_TO_PARAM u_clientimage_info.http_info.ip_client, U_http_ip_client_len
#define U_HTTP_IP_CLIENT_TO_TRACE U_http_ip_client_len, u_clientimage_info.http_info.ip_client

#define U_HTTP_USER_AGENT_TO_PARAM u_clientimage_info.http_info.user_agent, u_clientimage_info.http_info.user_agent_len
#define U_HTTP_USER_AGENT_TO_TRACE u_clientimage_info.http_info.user_agent_len, u_clientimage_info.http_info.user_agent

#define U_HTTP_ACCEPT_TO_PARAM u_clientimage_info.http_info.accept, U_http_accept_len
#define U_HTTP_ACCEPT_TO_TRACE U_http_accept_len, u_clientimage_info.http_info.accept

#define U_HTTP_ACCEPT_LANGUAGE_TO_PARAM u_clientimage_info.http_info.accept_language, U_http_accept_language_len
#define U_HTTP_ACCEPT_LANGUAGE_TO_TRACE U_http_accept_language_len, u_clientimage_info.http_info.accept_language

#define U_HTTP_METHOD_TO_PARAM U_HTTP_METHOD_NUM_TO_PARAM(U_http_method_num)
#define U_HTTP_METHOD_TO_TRACE U_HTTP_METHOD_NUM_TO_TRACE(U_http_method_num)

#define U_HTTP_METHOD_NUM_TO_PARAM(num) u_clientimage_info.http_method_list[num].name, u_clientimage_info.http_method_list[num].len
#define U_HTTP_METHOD_NUM_TO_TRACE(num) u_clientimage_info.http_method_list[num].len,  u_clientimage_info.http_method_list[num].name

#define U_HTTP_HOST_STREQ(str) (U_http_host_len ? U_STREQ(u_clientimage_info.http_info.host, U_http_host_len, str) : false)

#define U_HTTP_REFERER_STREQ(str) (u_clientimage_info.http_info.referer_len ? U_STREQ(u_clientimage_info.http_info.referer, u_clientimage_info.http_info.referer_len, str) : false)

#define U_HTTP_USER_AGENT_STREQ(str) (u_clientimage_info.http_info.user_agent_len ? U_STREQ(u_clientimage_info.http_info.user_agent, u_clientimage_info.http_info.user_agent_len, str) : false)

#define U_HTTP_URI_MEMEQ(str)   memcmp(u_clientimage_info.http_info.uri, U_CONSTANT_TO_PARAM(str)) == 0
#define U_HTTP_URI_STREQ(str)  U_STREQ(u_clientimage_info.http_info.uri, u_clientimage_info.http_info.uri_len, str)

#define U_HTTP_CTYPE_MEMEQ(str) (U_http_content_type_len ?  memcmp(u_clientimage_info.http_info.content_type, U_CONSTANT_TO_PARAM(str)) == 0 : false)
#define U_HTTP_CTYPE_STREQ(str) (U_http_content_type_len ? U_STREQ(u_clientimage_info.http_info.content_type, U_http_content_type_len, str)  : false)

#define U_HTTP_QUERY_MEMEQ(str) (u_clientimage_info.http_info.query_len ?  memcmp(u_clientimage_info.http_info.query, U_CONSTANT_TO_PARAM(str)) == 0 : false)
#define U_HTTP_QUERY_STREQ(str) (u_clientimage_info.http_info.query_len ? U_STREQ(u_clientimage_info.http_info.query, u_clientimage_info.http_info.query_len, str) : false)

/**
 * The hostname of your server from header's request.
 * The difference between U_HTTP_HOST_.. and U_HTTP_VHOST_.. is that
 * U_HTTP_HOST_.. can include the :PORT text, and U_HTTP_VHOST_.. only the name
 */

#define U_HTTP_HOST_TO_PARAM  u_clientimage_info.http_info.host, U_http_host_len
#define U_HTTP_HOST_TO_TRACE  U_http_host_len, u_clientimage_info.http_info.host

#define U_HTTP_VHOST_TO_PARAM u_clientimage_info.http_info.host, U_http_host_vlen
#define U_HTTP_VHOST_TO_TRACE U_http_host_vlen, u_clientimage_info.http_info.host

#endif
