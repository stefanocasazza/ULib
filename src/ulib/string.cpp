// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    string.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>
#include <ulib/json/value.h>
#include <ulib/utility/escape.h>
#include <ulib/internal/chttp.h>

UString*    UString::string_null        = ULib::uustringnull.p2;
UStringRep* UStringRep::string_rep_null = ULib::uustringrepnull.p2;

// OPTMIZE APPEND (BUFFERED)
char* UString::appbuf;
char* UString::ptrbuf;

const UString* UString::str_host;
const UString* UString::str_chunked;
const UString* UString::str_without_mac;
const UString* UString::str_localhost;
const UString* UString::str_http;
const UString* UString::str_msg_rfc;
const UString* UString::str_txt_plain;
const UString* UString::str_address;
const UString* UString::str_CLIENT_QUEUE_DIR;
const UString* UString::str_point;
const UString* UString::str_true;
const UString* UString::str_false;
const UString* UString::str_response;
const UString* UString::str_zero;
const UString* UString::str_nostat;
const UString* UString::str_tsa;
const UString* UString::str_soap;
// SOAP
const UString* UString::str_ns;
const UString* UString::str_boolean;
const UString* UString::str_byte;
const UString* UString::str_unsignedByte;
const UString* UString::str_short;
const UString* UString::str_unsignedShort;
const UString* UString::str_int;
const UString* UString::str_unsignedInt;
const UString* UString::str_long;
const UString* UString::str_unsignedLong;
const UString* UString::str_float;
const UString* UString::str_double;
const UString* UString::str_string;
const UString* UString::str_base64Binary;
// IMAP
const UString* UString::str_recent;
const UString* UString::str_unseen;
const UString* UString::str_uidnext;
const UString* UString::str_uidvalidity;
// PROXY SERVICE
const UString* UString::str_FOLLOW_REDIRECTS;
const UString* UString::str_CLIENT_CERTIFICATE;
const UString* UString::str_REMOTE_ADDRESS_IP;
const UString* UString::str_WEBSOCKET;
// NOCAT
const UString* UString::str_without_label;
const UString* UString::str_allowed_members_default;
// SSI
const UString* UString::str_cgi;
const UString* UString::str_var;
// HTTP
const UString* UString::str_origin;
const UString* UString::str_indexhtml;
const UString* UString::str_ctype_tsa;
const UString* UString::str_ctype_txt;
const UString* UString::str_ctype_html;
const UString* UString::str_ctype_soap;
const UString* UString::str_ulib_header;
const UString* UString::str_storage_keyid;
const UString* UString::str_websocket_key;
const UString* UString::str_websocket_prot;
// QUERY PARSER
const UString* UString::str_p1;
const UString* UString::str_p2;
const UString* UString::str_or;
const UString* UString::str_and;
const UString* UString::str_not;
// ORM
const UString* UString::str_port;
const UString* UString::str_root;
const UString* UString::str_UTF8;
const UString* UString::str_UTF16;
const UString* UString::str_dbname;
const UString* UString::str_timeout;
const UString* UString::str_compress;
const UString* UString::str_character_set;
// ORM MYSQL
const UString* UString::str_mysql_name;
const UString* UString::str_secure_auth;
const UString* UString::str_auto_reconnect;
// ORM PGSQL
const UString* UString::str_pgsql_name;
// ORM SQLITE
const UString* UString::str_sqlite_name;
const UString* UString::str_dbdir;
const UString* UString::str_memory;
#ifndef U_HTTP2_DISABLE
const UString* UString::str_authority;
const UString* UString::str_method;
const UString* UString::str_method_get;
const UString* UString::str_method_post;
const UString* UString::str_path;
const UString* UString::str_path_root;
const UString* UString::str_path_index;
const UString* UString::str_scheme;
const UString* UString::str_scheme_https;
const UString* UString::str_status;
const UString* UString::str_status_200;
const UString* UString::str_status_204;
const UString* UString::str_status_206;
const UString* UString::str_status_304;
const UString* UString::str_status_400;
const UString* UString::str_status_404;
const UString* UString::str_status_500;
const UString* UString::str_accept_charset;
const UString* UString::str_accept_encoding;
const UString* UString::str_accept_encoding_value;
const UString* UString::str_accept_language;
const UString* UString::str_accept_ranges;
const UString* UString::str_accept;
const UString* UString::str_access_control_allow_origin;
const UString* UString::str_age;
const UString* UString::str_allow;
const UString* UString::str_authorization;
const UString* UString::str_cache_control;
const UString* UString::str_content_disposition;
const UString* UString::str_content_encoding;
const UString* UString::str_content_language;
const UString* UString::str_content_length;
const UString* UString::str_content_location;
const UString* UString::str_content_range;
const UString* UString::str_content_type;
const UString* UString::str_cookie;
const UString* UString::str_date;
const UString* UString::str_etag;
const UString* UString::str_expect;
const UString* UString::str_expires;
const UString* UString::str_from;
const UString* UString::str_if_match;
const UString* UString::str_if_modified_since;
const UString* UString::str_if_none_match;
const UString* UString::str_if_range;
const UString* UString::str_if_unmodified_since;
const UString* UString::str_last_modified;
const UString* UString::str_link;
const UString* UString::str_location;
const UString* UString::str_max_forwards;
const UString* UString::str_proxy_authenticate;
const UString* UString::str_proxy_authorization;
const UString* UString::str_range;
const UString* UString::str_referer;
const UString* UString::str_refresh;
const UString* UString::str_retry_after;
const UString* UString::str_server;
const UString* UString::str_set_cookie;
const UString* UString::str_strict_transport_security;
const UString* UString::str_transfer_encoding;
const UString* UString::str_user_agent;
const UString* UString::str_vary;
const UString* UString::str_via;
const UString* UString::str_www_authenticate;
const UString* UString::str_ULib;
#endif

#ifdef U_HTTP2_DISABLE
static ustringrep stringrep_storage[71] = {
#else
static ustringrep stringrep_storage[136] = {
#endif
   { U_STRINGREP_FROM_CONSTANT("host") },
   { U_STRINGREP_FROM_CONSTANT("chunked") },
   { U_STRINGREP_FROM_CONSTANT("00:00:00:00:00:00") },
   { U_STRINGREP_FROM_CONSTANT("localhost") },
   { U_STRINGREP_FROM_CONSTANT("http") },
   { U_STRINGREP_FROM_CONSTANT("message/rfc822") },
   { U_STRINGREP_FROM_CONSTANT("text/plain") },
   { U_STRINGREP_FROM_CONSTANT("stefano.casazza@gmail.com") },
   { U_STRINGREP_FROM_CONSTANT("/tmp/uclient") },
   { U_STRINGREP_FROM_CONSTANT(".") },
   { U_STRINGREP_FROM_CONSTANT("true") },
   { U_STRINGREP_FROM_CONSTANT("false") },
   { U_STRINGREP_FROM_CONSTANT("response") },
   { U_STRINGREP_FROM_CONSTANT("0") },
   { U_STRINGREP_FROM_CONSTANT("/nostat") },
   { U_STRINGREP_FROM_CONSTANT("/tsa") },
   { U_STRINGREP_FROM_CONSTANT("/soap") },
   { U_STRINGREP_FROM_CONSTANT("") },
   // SOAP
   { U_STRINGREP_FROM_CONSTANT("ns") },
   { U_STRINGREP_FROM_CONSTANT("boolean") },
   { U_STRINGREP_FROM_CONSTANT("byte") },
   { U_STRINGREP_FROM_CONSTANT("unsignedByte") },
   { U_STRINGREP_FROM_CONSTANT("short") },
   { U_STRINGREP_FROM_CONSTANT("unsignedShort") },
   { U_STRINGREP_FROM_CONSTANT("int") },
   { U_STRINGREP_FROM_CONSTANT("unsignedInt") },
   { U_STRINGREP_FROM_CONSTANT("long") },
   { U_STRINGREP_FROM_CONSTANT("unsignedLong") },
   { U_STRINGREP_FROM_CONSTANT("float") },
   { U_STRINGREP_FROM_CONSTANT("double") },
   { U_STRINGREP_FROM_CONSTANT("string") },
   { U_STRINGREP_FROM_CONSTANT("base64Binary") },
   // IMAP
   { U_STRINGREP_FROM_CONSTANT("RECENT") },
   { U_STRINGREP_FROM_CONSTANT("UNSEEN") },
   { U_STRINGREP_FROM_CONSTANT("UIDNEXT") },
   { U_STRINGREP_FROM_CONSTANT("UIDVALIDITY") },
   // SSI
   { U_STRINGREP_FROM_CONSTANT("cgi") },
   { U_STRINGREP_FROM_CONSTANT("var") },
   // NOCAT
   { U_STRINGREP_FROM_CONSTANT("without_label") },
   { U_STRINGREP_FROM_CONSTANT("/etc/nodog.allowed") },
   // HTTP
   { U_STRINGREP_FROM_CONSTANT("index.html") },
   { U_STRINGREP_FROM_CONSTANT("application/timestamp-reply\r\n") },
   { U_STRINGREP_FROM_CONSTANT(U_CTYPE_TEXT_WITH_CHARSET U_CRLF) },
   { U_STRINGREP_FROM_CONSTANT(U_CTYPE_HTML U_CRLF) },
   { U_STRINGREP_FROM_CONSTANT("application/soap+xml; charset=\"utf-8\"\r\n") },
   { U_STRINGREP_FROM_CONSTANT("Origin") },
   { U_STRINGREP_FROM_CONSTANT("X-Powered-By: ULib/" ULIB_VERSION "\r\n") },
   { U_STRINGREP_FROM_CONSTANT("StiD") },
   { U_STRINGREP_FROM_CONSTANT("Sec-WebSocket-Key") },
   { U_STRINGREP_FROM_CONSTANT("Sec-WebSocket-Protocol") },
   // QUERY PARSER
   { U_STRINGREP_FROM_CONSTANT("(") },
   { U_STRINGREP_FROM_CONSTANT(")") },
   { U_STRINGREP_FROM_CONSTANT("OR") },
   { U_STRINGREP_FROM_CONSTANT("AND") },
   { U_STRINGREP_FROM_CONSTANT("NOT") },
   // ORM
   { U_STRINGREP_FROM_CONSTANT("port") },
   { U_STRINGREP_FROM_CONSTANT("root") },
   { U_STRINGREP_FROM_CONSTANT("utf8") },
   { U_STRINGREP_FROM_CONSTANT("utf16") },
   { U_STRINGREP_FROM_CONSTANT("dbname") },
   { U_STRINGREP_FROM_CONSTANT("timeout") },
   { U_STRINGREP_FROM_CONSTANT("compress") },
   { U_STRINGREP_FROM_CONSTANT("character-set") },
   // ORM SQLITE
   { U_STRINGREP_FROM_CONSTANT("sqlite") },
   { U_STRINGREP_FROM_CONSTANT("dbdir") },
   { U_STRINGREP_FROM_CONSTANT(":memory:") },
   // ORM MYSQL
   { U_STRINGREP_FROM_CONSTANT("mysql") },
   { U_STRINGREP_FROM_CONSTANT("secure-auth") },
   { U_STRINGREP_FROM_CONSTANT("auto-reconnect") },
   // ORM PGSQL
   { U_STRINGREP_FROM_CONSTANT("pgsql") },
#ifndef U_HTTP2_DISABLE
   { U_STRINGREP_FROM_CONSTANT(":authority") },
   { U_STRINGREP_FROM_CONSTANT(":method") },
   { U_STRINGREP_FROM_CONSTANT("GET") },
   { U_STRINGREP_FROM_CONSTANT("POST") },
   { U_STRINGREP_FROM_CONSTANT(":path") },
   { U_STRINGREP_FROM_CONSTANT("/") },
   { U_STRINGREP_FROM_CONSTANT("/index.html") },
   { U_STRINGREP_FROM_CONSTANT(":scheme") },
   { U_STRINGREP_FROM_CONSTANT("https") },
   { U_STRINGREP_FROM_CONSTANT(":status") },
   { U_STRINGREP_FROM_CONSTANT("200") },
   { U_STRINGREP_FROM_CONSTANT("204") },
   { U_STRINGREP_FROM_CONSTANT("206") },
   { U_STRINGREP_FROM_CONSTANT("304") },
   { U_STRINGREP_FROM_CONSTANT("400") },
   { U_STRINGREP_FROM_CONSTANT("404") },
   { U_STRINGREP_FROM_CONSTANT("500") },
   { U_STRINGREP_FROM_CONSTANT("accept-charset") },
   { U_STRINGREP_FROM_CONSTANT("accept-encoding") },
   { U_STRINGREP_FROM_CONSTANT("gzip, deflate") },
   { U_STRINGREP_FROM_CONSTANT("accept-language") },
   { U_STRINGREP_FROM_CONSTANT("accept-ranges") },
   { U_STRINGREP_FROM_CONSTANT("accept") },
   { U_STRINGREP_FROM_CONSTANT("access-control-allow-origin") },
   { U_STRINGREP_FROM_CONSTANT("age") },
   { U_STRINGREP_FROM_CONSTANT("allow") },
   { U_STRINGREP_FROM_CONSTANT("authorization") },
   { U_STRINGREP_FROM_CONSTANT("cache-control") },
   { U_STRINGREP_FROM_CONSTANT("content-disposition") },
   { U_STRINGREP_FROM_CONSTANT("content-encoding") },
   { U_STRINGREP_FROM_CONSTANT("content-language") },
   { U_STRINGREP_FROM_CONSTANT("content-length") },
   { U_STRINGREP_FROM_CONSTANT("content-location") },
   { U_STRINGREP_FROM_CONSTANT("content-range") },
   { U_STRINGREP_FROM_CONSTANT("content-type") },
   { U_STRINGREP_FROM_CONSTANT("cookie") },
   { U_STRINGREP_FROM_CONSTANT("date") },
   { U_STRINGREP_FROM_CONSTANT("etag") },
   { U_STRINGREP_FROM_CONSTANT("expect") },
   { U_STRINGREP_FROM_CONSTANT("expires") },
   { U_STRINGREP_FROM_CONSTANT("from") },
   { U_STRINGREP_FROM_CONSTANT("if-match") },
   { U_STRINGREP_FROM_CONSTANT("if-modified-since") },
   { U_STRINGREP_FROM_CONSTANT("if-none-match") },
   { U_STRINGREP_FROM_CONSTANT("if-range") },
   { U_STRINGREP_FROM_CONSTANT("if-unmodified-since") },
   { U_STRINGREP_FROM_CONSTANT("last-modified") },
   { U_STRINGREP_FROM_CONSTANT("link") },
   { U_STRINGREP_FROM_CONSTANT("location") },
   { U_STRINGREP_FROM_CONSTANT("max-forwards") },
   { U_STRINGREP_FROM_CONSTANT("proxy-authenticate") },
   { U_STRINGREP_FROM_CONSTANT("proxy-authorization") },
   { U_STRINGREP_FROM_CONSTANT("range") },
   { U_STRINGREP_FROM_CONSTANT("referer") },
   { U_STRINGREP_FROM_CONSTANT("refresh") },
   { U_STRINGREP_FROM_CONSTANT("retry-after") },
   { U_STRINGREP_FROM_CONSTANT("server") },
   { U_STRINGREP_FROM_CONSTANT("set-cookie") },
   { U_STRINGREP_FROM_CONSTANT("strict-transport-security") },
   { U_STRINGREP_FROM_CONSTANT("transfer-encoding") },
   { U_STRINGREP_FROM_CONSTANT("user-agent") },
   { U_STRINGREP_FROM_CONSTANT("vary") },
   { U_STRINGREP_FROM_CONSTANT("via") },
   { U_STRINGREP_FROM_CONSTANT("www-authenticate") },
   { U_STRINGREP_FROM_CONSTANT("ULib") }
#endif
};

void UString::str_allocate(int which)
{
   U_TRACE(0+256, "UString::str_allocate(%d)", which)

   if (which == 0)
      {
      U_INTERNAL_ASSERT_EQUALS(str_host, 0)
      U_INTERNAL_ASSERT_EQUALS(str_chunked, 0)
      U_INTERNAL_ASSERT_EQUALS(str_without_mac, 0)
      U_INTERNAL_ASSERT_EQUALS(str_localhost, 0)
      U_INTERNAL_ASSERT_EQUALS(str_http, 0)
      U_INTERNAL_ASSERT_EQUALS(str_msg_rfc, 0)
      U_INTERNAL_ASSERT_EQUALS(str_txt_plain, 0)
      U_INTERNAL_ASSERT_EQUALS(str_address, 0)
      U_INTERNAL_ASSERT_EQUALS(str_CLIENT_QUEUE_DIR, 0)
      U_INTERNAL_ASSERT_EQUALS(str_point, 0)
      U_INTERNAL_ASSERT_EQUALS(str_true, 0)
      U_INTERNAL_ASSERT_EQUALS(str_false, 0)
      U_INTERNAL_ASSERT_EQUALS(str_response, 0)
      U_INTERNAL_ASSERT_EQUALS(str_zero, 0)
      U_INTERNAL_ASSERT_EQUALS(str_nostat, 0)
      U_INTERNAL_ASSERT_EQUALS(str_tsa, 0)
      U_INTERNAL_ASSERT_EQUALS(str_soap, 0)
      U_INTERNAL_ASSERT_EQUALS(UHashMap<void*>::pkey, 0)

      U_NEW_ULIB_OBJECT(UString, str_host,             UString(stringrep_storage+0));
      U_NEW_ULIB_OBJECT(UString, str_chunked,          UString(stringrep_storage+1));
      U_NEW_ULIB_OBJECT(UString, str_without_mac,      UString(stringrep_storage+2));
      U_NEW_ULIB_OBJECT(UString, str_localhost,        UString(stringrep_storage+3));
      U_NEW_ULIB_OBJECT(UString, str_http,             UString(stringrep_storage+4));
      U_NEW_ULIB_OBJECT(UString, str_msg_rfc,          UString(stringrep_storage+5));
      U_NEW_ULIB_OBJECT(UString, str_txt_plain,        UString(stringrep_storage+6));
      U_NEW_ULIB_OBJECT(UString, str_address,          UString(stringrep_storage+7));
      U_NEW_ULIB_OBJECT(UString, str_CLIENT_QUEUE_DIR, UString(stringrep_storage+8));
      U_NEW_ULIB_OBJECT(UString, str_point,            UString(stringrep_storage+9));
      U_NEW_ULIB_OBJECT(UString, str_true,             UString(stringrep_storage+10));
      U_NEW_ULIB_OBJECT(UString, str_false,            UString(stringrep_storage+11));
      U_NEW_ULIB_OBJECT(UString, str_response,         UString(stringrep_storage+12));
      U_NEW_ULIB_OBJECT(UString, str_zero,             UString(stringrep_storage+13));
      U_NEW_ULIB_OBJECT(UString, str_nostat,           UString(stringrep_storage+14));
      U_NEW_ULIB_OBJECT(UString, str_tsa,              UString(stringrep_storage+15));
      U_NEW_ULIB_OBJECT(UString, str_soap,             UString(stringrep_storage+16));

      uustringrep key1 = { stringrep_storage+17 };

      UHashMap<void*>::pkey = key1.p2;

      U_INTERNAL_ASSERT(UHashMap<void*>::pkey->invariant())

      U_INTERNAL_ASSERT_EQUALS(*str_without_mac,      "00:00:00:00:00:00")
      U_INTERNAL_ASSERT_EQUALS(*str_CLIENT_QUEUE_DIR, "/tmp/uclient")
      }
   else if ((which & STR_ALLOCATE_SOAP) != 0)
      {
      U_INTERNAL_ASSERT_EQUALS(str_ns, 0)
      U_INTERNAL_ASSERT_EQUALS(str_boolean, 0)
      U_INTERNAL_ASSERT_EQUALS(str_byte, 0)
      U_INTERNAL_ASSERT_EQUALS(str_unsignedByte, 0)
      U_INTERNAL_ASSERT_EQUALS(str_short, 0)
      U_INTERNAL_ASSERT_EQUALS(str_unsignedShort, 0)
      U_INTERNAL_ASSERT_EQUALS(str_int, 0)
      U_INTERNAL_ASSERT_EQUALS(str_unsignedInt, 0)
      U_INTERNAL_ASSERT_EQUALS(str_long, 0)
      U_INTERNAL_ASSERT_EQUALS(str_unsignedLong, 0)
      U_INTERNAL_ASSERT_EQUALS(str_float, 0)
      U_INTERNAL_ASSERT_EQUALS(str_double, 0)
      U_INTERNAL_ASSERT_EQUALS(str_string, 0)
      U_INTERNAL_ASSERT_EQUALS(str_base64Binary, 0)

      U_NEW_ULIB_OBJECT(UString, str_ns,            UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+0));
      U_NEW_ULIB_OBJECT(UString, str_boolean,       UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+1));
      U_NEW_ULIB_OBJECT(UString, str_byte,          UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+2));
      U_NEW_ULIB_OBJECT(UString, str_unsignedByte,  UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+3));
      U_NEW_ULIB_OBJECT(UString, str_short,         UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+4));
      U_NEW_ULIB_OBJECT(UString, str_unsignedShort, UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+5));
      U_NEW_ULIB_OBJECT(UString, str_int,           UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+6));
      U_NEW_ULIB_OBJECT(UString, str_unsignedInt,   UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+7));
      U_NEW_ULIB_OBJECT(UString, str_long,          UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+8));
      U_NEW_ULIB_OBJECT(UString, str_unsignedLong,  UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+9));
      U_NEW_ULIB_OBJECT(UString, str_float,         UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+10));
      U_NEW_ULIB_OBJECT(UString, str_double,        UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+11));
      U_NEW_ULIB_OBJECT(UString, str_string,        UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+12));
      U_NEW_ULIB_OBJECT(UString, str_base64Binary,  UString(stringrep_storage+STR_ALLOCATE_INDEX_SOAP+13));
      }
   else if ((which & STR_ALLOCATE_IMAP) != 0)
      {
      U_INTERNAL_ASSERT_EQUALS(str_recent, 0)
      U_INTERNAL_ASSERT_EQUALS(str_unseen, 0)
      U_INTERNAL_ASSERT_EQUALS(str_uidnext, 0)
      U_INTERNAL_ASSERT_EQUALS(str_uidvalidity, 0)

      U_NEW_ULIB_OBJECT(UString, str_recent,      UString(stringrep_storage+STR_ALLOCATE_INDEX_IMAP+0));
      U_NEW_ULIB_OBJECT(UString, str_unseen,      UString(stringrep_storage+STR_ALLOCATE_INDEX_IMAP+1));
      U_NEW_ULIB_OBJECT(UString, str_uidnext,     UString(stringrep_storage+STR_ALLOCATE_INDEX_IMAP+2));
      U_NEW_ULIB_OBJECT(UString, str_uidvalidity, UString(stringrep_storage+STR_ALLOCATE_INDEX_IMAP+3));
      }
   else if ((which & STR_ALLOCATE_SSI) != 0)
      {
      U_INTERNAL_ASSERT_EQUALS(str_cgi, 0)
      U_INTERNAL_ASSERT_EQUALS(str_var, 0)

      U_NEW_ULIB_OBJECT(UString, str_cgi, UString(stringrep_storage+STR_ALLOCATE_INDEX_SSI+0));
      U_NEW_ULIB_OBJECT(UString, str_var, UString(stringrep_storage+STR_ALLOCATE_INDEX_SSI+1));
      }
   else if ((which & STR_ALLOCATE_NOCAT) != 0)
      {
      U_INTERNAL_ASSERT_EQUALS(str_without_label, 0)
      U_INTERNAL_ASSERT_EQUALS(str_allowed_members_default, 0)

      U_NEW_ULIB_OBJECT(UString, str_without_label,           UString(stringrep_storage+STR_ALLOCATE_INDEX_NOCAT+0));
      U_NEW_ULIB_OBJECT(UString, str_allowed_members_default, UString(stringrep_storage+STR_ALLOCATE_INDEX_NOCAT+1));
      }
   else if ((which & STR_ALLOCATE_HTTP) != 0)
      {
      U_INTERNAL_ASSERT_EQUALS(str_indexhtml, 0)
      U_INTERNAL_ASSERT_EQUALS(str_ctype_tsa, 0)
      U_INTERNAL_ASSERT_EQUALS(str_ctype_txt, 0)
      U_INTERNAL_ASSERT_EQUALS(str_ctype_html, 0)
      U_INTERNAL_ASSERT_EQUALS(str_ctype_soap, 0)
      U_INTERNAL_ASSERT_EQUALS(str_origin, 0)
      U_INTERNAL_ASSERT_EQUALS(str_ulib_header, 0)
      U_INTERNAL_ASSERT_EQUALS(str_storage_keyid, 0)
      U_INTERNAL_ASSERT_EQUALS(str_websocket_key, 0)
      U_INTERNAL_ASSERT_EQUALS(str_websocket_prot, 0)

      U_NEW_ULIB_OBJECT(UString, str_indexhtml,      UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP+0));
      U_NEW_ULIB_OBJECT(UString, str_ctype_tsa,      UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP+1));
      U_NEW_ULIB_OBJECT(UString, str_ctype_txt,      UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP+2));
      U_NEW_ULIB_OBJECT(UString, str_ctype_html,     UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP+3));
      U_NEW_ULIB_OBJECT(UString, str_ctype_soap,     UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP+4));
      U_NEW_ULIB_OBJECT(UString, str_origin,         UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP+5));
      U_NEW_ULIB_OBJECT(UString, str_ulib_header,    UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP+6));
      U_NEW_ULIB_OBJECT(UString, str_storage_keyid,  UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP+7));
      U_NEW_ULIB_OBJECT(UString, str_websocket_key,  UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP+8));
      U_NEW_ULIB_OBJECT(UString, str_websocket_prot, UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP+9));
      }
   else if ((which & STR_ALLOCATE_QUERY_PARSER) != 0)
      {
      U_INTERNAL_ASSERT_EQUALS(str_p1, 0)
      U_INTERNAL_ASSERT_EQUALS(str_p2, 0)
      U_INTERNAL_ASSERT_EQUALS(str_or, 0)
      U_INTERNAL_ASSERT_EQUALS(str_and, 0)
      U_INTERNAL_ASSERT_EQUALS(str_not, 0)

      U_NEW_ULIB_OBJECT(UString, str_p1,  UString(stringrep_storage+STR_ALLOCATE_INDEX_QUERY_PARSER+0));
      U_NEW_ULIB_OBJECT(UString, str_p2,  UString(stringrep_storage+STR_ALLOCATE_INDEX_QUERY_PARSER+1));
      U_NEW_ULIB_OBJECT(UString, str_or,  UString(stringrep_storage+STR_ALLOCATE_INDEX_QUERY_PARSER+2));
      U_NEW_ULIB_OBJECT(UString, str_and, UString(stringrep_storage+STR_ALLOCATE_INDEX_QUERY_PARSER+3));
      U_NEW_ULIB_OBJECT(UString, str_not, UString(stringrep_storage+STR_ALLOCATE_INDEX_QUERY_PARSER+4));
      }
   else if ((which & STR_ALLOCATE_ORM) != 0)
      {
      U_INTERNAL_ASSERT_EQUALS(str_port, 0)
      U_INTERNAL_ASSERT_EQUALS(str_root, 0)
      U_INTERNAL_ASSERT_EQUALS(str_UTF8, 0)
      U_INTERNAL_ASSERT_EQUALS(str_UTF16, 0)
      U_INTERNAL_ASSERT_EQUALS(str_dbname, 0)
      U_INTERNAL_ASSERT_EQUALS(str_timeout, 0)
      U_INTERNAL_ASSERT_EQUALS(str_compress, 0)
      U_INTERNAL_ASSERT_EQUALS(str_character_set, 0)
      U_INTERNAL_ASSERT_EQUALS(str_sqlite_name, 0)
      U_INTERNAL_ASSERT_EQUALS(str_dbdir, 0)
      U_INTERNAL_ASSERT_EQUALS(str_memory, 0)
      U_INTERNAL_ASSERT_EQUALS(str_mysql_name, 0)
      U_INTERNAL_ASSERT_EQUALS(str_secure_auth, 0)
      U_INTERNAL_ASSERT_EQUALS(str_auto_reconnect, 0)
      U_INTERNAL_ASSERT_EQUALS(str_pgsql_name, 0)

      U_NEW_ULIB_OBJECT(UString, str_port,           UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+0));
      U_NEW_ULIB_OBJECT(UString, str_root,           UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+1));
      U_NEW_ULIB_OBJECT(UString, str_UTF8,           UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+2));
      U_NEW_ULIB_OBJECT(UString, str_UTF16,          UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+3));
      U_NEW_ULIB_OBJECT(UString, str_dbname,         UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+4));
      U_NEW_ULIB_OBJECT(UString, str_timeout,        UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+5));
      U_NEW_ULIB_OBJECT(UString, str_compress,       UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+6));
      U_NEW_ULIB_OBJECT(UString, str_character_set,  UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+7));
      U_NEW_ULIB_OBJECT(UString, str_sqlite_name,    UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+8));
      U_NEW_ULIB_OBJECT(UString, str_dbdir,          UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+9));
      U_NEW_ULIB_OBJECT(UString, str_memory,         UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+10));
      U_NEW_ULIB_OBJECT(UString, str_mysql_name,     UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+11));
      U_NEW_ULIB_OBJECT(UString, str_secure_auth,    UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+12));
      U_NEW_ULIB_OBJECT(UString, str_auto_reconnect, UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+13));
      U_NEW_ULIB_OBJECT(UString, str_pgsql_name,     UString(stringrep_storage+STR_ALLOCATE_INDEX_ORM+14));
      }
#ifndef U_HTTP2_DISABLE
   else if ((which & STR_ALLOCATE_HTTP2) != 0)
      {
      U_INTERNAL_ASSERT_EQUALS(str_authority, 0)
      U_INTERNAL_ASSERT_EQUALS(U_NUM_ELEMENTS(stringrep_storage), 136)

      U_NEW_ULIB_OBJECT(UString, str_authority,                   UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+0));
      U_NEW_ULIB_OBJECT(UString, str_method,                      UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+1));
      U_NEW_ULIB_OBJECT(UString, str_method_get,                  UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+2));
      U_NEW_ULIB_OBJECT(UString, str_method_post,                 UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+3));
      U_NEW_ULIB_OBJECT(UString, str_path,                        UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+4));
      U_NEW_ULIB_OBJECT(UString, str_path_root,                   UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+5));
      U_NEW_ULIB_OBJECT(UString, str_path_index,                  UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+6));
      U_NEW_ULIB_OBJECT(UString, str_scheme,                      UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+7));
      U_NEW_ULIB_OBJECT(UString, str_scheme_https,                UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+8));
      U_NEW_ULIB_OBJECT(UString, str_status,                      UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+9));
      U_NEW_ULIB_OBJECT(UString, str_status_200,                  UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+10));
      U_NEW_ULIB_OBJECT(UString, str_status_204,                  UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+11));
      U_NEW_ULIB_OBJECT(UString, str_status_206,                  UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+12));
      U_NEW_ULIB_OBJECT(UString, str_status_304,                  UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+13));
      U_NEW_ULIB_OBJECT(UString, str_status_400,                  UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+14));
      U_NEW_ULIB_OBJECT(UString, str_status_404,                  UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+15));
      U_NEW_ULIB_OBJECT(UString, str_status_500,                  UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+16));
      U_NEW_ULIB_OBJECT(UString, str_accept_charset,              UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+17));
      U_NEW_ULIB_OBJECT(UString, str_accept_encoding,             UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+18));
      U_NEW_ULIB_OBJECT(UString, str_accept_encoding_value,       UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+19));
      U_NEW_ULIB_OBJECT(UString, str_accept_language,             UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+20));
      U_NEW_ULIB_OBJECT(UString, str_accept_ranges,               UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+21));
      U_NEW_ULIB_OBJECT(UString, str_accept,                      UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+22));
      U_NEW_ULIB_OBJECT(UString, str_access_control_allow_origin, UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+23));
      U_NEW_ULIB_OBJECT(UString, str_age,                         UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+24));
      U_NEW_ULIB_OBJECT(UString, str_allow,                       UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+25));
      U_NEW_ULIB_OBJECT(UString, str_authorization,               UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+26));
      U_NEW_ULIB_OBJECT(UString, str_cache_control,               UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+27));
      U_NEW_ULIB_OBJECT(UString, str_content_disposition,         UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+28));
      U_NEW_ULIB_OBJECT(UString, str_content_encoding,            UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+29));
      U_NEW_ULIB_OBJECT(UString, str_content_language,            UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+30));
      U_NEW_ULIB_OBJECT(UString, str_content_length,              UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+31));
      U_NEW_ULIB_OBJECT(UString, str_content_location,            UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+32));
      U_NEW_ULIB_OBJECT(UString, str_content_range,               UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+33));
      U_NEW_ULIB_OBJECT(UString, str_content_type,                UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+34));
      U_NEW_ULIB_OBJECT(UString, str_cookie,                      UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+35));
      U_NEW_ULIB_OBJECT(UString, str_date,                        UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+36));
      U_NEW_ULIB_OBJECT(UString, str_etag,                        UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+37));
      U_NEW_ULIB_OBJECT(UString, str_expect,                      UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+38));
      U_NEW_ULIB_OBJECT(UString, str_expires,                     UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+39));
      U_NEW_ULIB_OBJECT(UString, str_from,                        UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+40));
      U_NEW_ULIB_OBJECT(UString, str_if_match,                    UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+41));
      U_NEW_ULIB_OBJECT(UString, str_if_modified_since,           UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+42));
      U_NEW_ULIB_OBJECT(UString, str_if_none_match,               UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+43));
      U_NEW_ULIB_OBJECT(UString, str_if_range,                    UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+44));
      U_NEW_ULIB_OBJECT(UString, str_if_unmodified_since,         UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+45));
      U_NEW_ULIB_OBJECT(UString, str_last_modified,               UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+46));
      U_NEW_ULIB_OBJECT(UString, str_link,                        UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+47));
      U_NEW_ULIB_OBJECT(UString, str_location,                    UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+48));
      U_NEW_ULIB_OBJECT(UString, str_max_forwards,                UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+49));
      U_NEW_ULIB_OBJECT(UString, str_proxy_authenticate,          UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+50));
      U_NEW_ULIB_OBJECT(UString, str_proxy_authorization,         UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+51));
      U_NEW_ULIB_OBJECT(UString, str_range,                       UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+52));
      U_NEW_ULIB_OBJECT(UString, str_referer,                     UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+53));
      U_NEW_ULIB_OBJECT(UString, str_refresh,                     UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+54));
      U_NEW_ULIB_OBJECT(UString, str_retry_after,                 UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+55));
      U_NEW_ULIB_OBJECT(UString, str_server,                      UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+56));
      U_NEW_ULIB_OBJECT(UString, str_set_cookie,                  UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+57));
      U_NEW_ULIB_OBJECT(UString, str_strict_transport_security,   UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+58));
      U_NEW_ULIB_OBJECT(UString, str_transfer_encoding,           UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+59));
      U_NEW_ULIB_OBJECT(UString, str_user_agent,                  UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+60));
      U_NEW_ULIB_OBJECT(UString, str_vary,                        UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+61));
      U_NEW_ULIB_OBJECT(UString, str_via,                         UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+62));
      U_NEW_ULIB_OBJECT(UString, str_www_authenticate,            UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+63));
      U_NEW_ULIB_OBJECT(UString, str_ULib,                        UString(stringrep_storage+STR_ALLOCATE_INDEX_HTTP2+64));
      }
#else
   U_INTERNAL_ASSERT_EQUALS(U_NUM_ELEMENTS(stringrep_storage), 71)
#endif
}

UStringRep* UStringRep::create(uint32_t length, uint32_t need, const char* ptr)
{
   U_TRACE(1, "UStringRep::create(%u,%u,%p)", length, need, ptr)

   U_INTERNAL_ASSERT_MAJOR(need, 0)

   char* _ptr;
   UStringRep* r;

   // NB: we don't use new (ctor) because we want an allocation with more space for string data...

#ifndef ENABLE_MEMPOOL
      r = (UStringRep*) U_SYSCALL(malloc, "%u", need+(1+sizeof(UStringRep)));
   _ptr = (char*)(r + 1);
#else
   if (need > U_CAPACITY)
      {
      _ptr = UFile::mmap(&need, -1, PROT_READ | PROT_WRITE, MAP_PRIVATE | U_MAP_ANON, 0);

      if (_ptr == MAP_FAILED)
         {
         string_rep_null->references++;

         U_RETURN_POINTER(string_rep_null, UStringRep);
         }

      r = U_MALLOC_TYPE(UStringRep);
      }
   else
      {
#  ifdef DEBUG
      UMemoryPool::obj_class = "UStringRep";
      UMemoryPool::func_call = __PRETTY_FUNCTION__;
#  endif

      // -------------------------------------------------------------------------------------------------------------------------------
      // see: http://www.codeproject.com/Articles/702065/C-Struct-Hack
      //
      // NB: we need an array of char[_capacity], plus a terminating null char element, plus enough for the UStringRep data structure...
      // -------------------------------------------------------------------------------------------------------------------------------

      if (need > (U_STACK_TYPE_8-(1+sizeof(UStringRep))))
         {
         need = U_STACK_TYPE_9-(1+sizeof(UStringRep));

         r = (UStringRep*) UMemoryPool::pop(9);
         }
      else
         {
         int stack_index;
         uint32_t sz = need + (1+sizeof(UStringRep));

         if (sz <= U_STACK_TYPE_4) // 128
            {
            need        = U_STACK_TYPE_4-(1+sizeof(UStringRep));
            stack_index = 4;
            }
         else if (sz <= U_STACK_TYPE_5) // 256
            {
            need        = U_STACK_TYPE_5-(1+sizeof(UStringRep));
            stack_index = 5;
            }
         else if (sz <= U_STACK_TYPE_6) // 512
            {
            need        = U_STACK_TYPE_6-(1+sizeof(UStringRep));
            stack_index = 6;
            }
         else if (sz <= U_STACK_TYPE_7) // 1024
            {
            need        = U_STACK_TYPE_7-(1+sizeof(UStringRep));
            stack_index = 7;
            }
         else
            {
            U_INTERNAL_ASSERT(need <= U_STACK_TYPE_8) // 2048

            need        = U_STACK_TYPE_8-(1+sizeof(UStringRep));
            stack_index = 8;
            }

         U_INTERNAL_DUMP("sz = %u need = %u stack_index = %u", sz, need, stack_index)

         r = (UStringRep*) UMemoryPool::pop(stack_index);
         }

      _ptr = (char*)(r + 1);

#  ifdef DEBUG
      UMemoryPool::obj_class = UMemoryPool::func_call = 0;
#  endif
      }
#endif

#ifdef DEBUG
   U_SET_LOCATION_INFO;
   U_REGISTER_OBJECT_PTR(0,UStringRep,r,&(r->memory._this))
   r->memory._this = (void*)U_CHECK_MEMORY_SENTINEL;
#endif

   r->set(length, need, _ptr);

   if (length &&
       ptr)
      {
      U_MEMCPY((void*)_ptr, ptr, length);

      _ptr[length] = '\0';
      }

   U_INTERNAL_ASSERT(r->invariant())

   U_RETURN_POINTER(r, UStringRep);
}

bool UString::shrink()
{
   U_TRACE_NO_PARAM(0, "UString::shrink()")

#ifdef ENABLE_MEMPOOL
   uint32_t _length = rep->_length, sz = _length+(1+sizeof(UStringRep)); // NB: we need an array of char[_length], plus a terminating null char, plus the UStringRep data structure...

   U_INTERNAL_DUMP("rep->_capacity = %u _length = %u sz = %u", rep->_capacity, _length, sz)

   U_INTERNAL_ASSERT_MAJOR(rep->_capacity, 0) // mode: 0 -> const

   if (sz <= U_STACK_TYPE_8) // 2048
      {
      int stack_index;
      uint32_t _capacity;

      if (sz <= U_STACK_TYPE_4) // 128
         {
         _capacity   = U_STACK_TYPE_4-(1+sizeof(UStringRep));
         stack_index = 4;
         }
      else if (sz <= U_STACK_TYPE_5) // 256
         {
         _capacity   = U_STACK_TYPE_5-(1+sizeof(UStringRep));
         stack_index = 5;
         }
      else if (sz <= U_STACK_TYPE_6) // 512
         {
         _capacity   = U_STACK_TYPE_6-(1+sizeof(UStringRep));
         stack_index = 6;
         }
      else if (sz <= U_STACK_TYPE_7) // 1024
         {
         _capacity   = U_STACK_TYPE_7-(1+sizeof(UStringRep));
         stack_index = 7;
         }
      else // 2048
         {
         _capacity   = U_STACK_TYPE_8-(1+sizeof(UStringRep));
         stack_index = 8;
         }

      U_INTERNAL_DUMP("_capacity = %u stack_index = %u", _capacity, stack_index)

      if (_capacity < rep->_capacity)
         {
         UStringRep* r = (UStringRep*) UMemoryPool::pop(stack_index);
         char* ptr     = (char*)(r + 1);

#     ifdef DEBUG
         U_SET_LOCATION_INFO;
         U_REGISTER_OBJECT_PTR(0,UStringRep,r,&(r->memory._this))
         r->memory._this = (void*)U_CHECK_MEMORY_SENTINEL;
#     endif

         r->set(_length, _capacity, ptr);

         U_MEMCPY((void*)ptr, rep->str, _length);

         ptr[_length] = '\0';

         U_INTERNAL_ASSERT(r->invariant())

         _set(r);

         U_INTERNAL_ASSERT(invariant())

         U_RETURN(true);
         }
      }
#endif

   U_RETURN(false);
}

static const int MultiplyDeBruijnBitPosition2[32] = { 0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9 };

void UStringRep::_release()
{
   U_TRACE_NO_PARAM(0, "UStringRep::_release()")

   U_INTERNAL_DUMP("_capacity = %u str(%u) = %V", _capacity, _length, this)

   U_INTERNAL_ASSERT_EQUALS(references, 0)
   U_INTERNAL_ASSERT_DIFFERS(this, string_rep_null)

#if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
   if (parent)
# ifdef U_SUBSTR_INC_REF
   parent->release(); // NB: only the death of substring de-reference the source...
# else
   {
   U_INTERNAL_ASSERT_EQUALS(child, 0)

   U_INTERNAL_DUMP("parent->child = %d", parent->child)

// U_INTERNAL_ASSERT_RANGE(1, parent->child, max_child)

   if (parent->child >= 1 &&
       parent->child <= max_child)
      {
      parent->child--;

      U_INTERNAL_DUMP("this = %p parent = %p parent->references = %u parent->child = %d", this, parent, parent->references, parent->child)
      }
   else
      {
      U_WARNING("parent->child has value(%d) out of range [1-%d]", parent->child, max_child);
      }
   }
   else // source...
   {
   if (child)
      {
#  ifdef U_STDCPP_ENABLE
      if (UObjectDB::fd > 0)
         {
         parent_destroy = this;

         U_DUMP_OBJECT_WITH_CHECK("DEAD OF SOURCE STRING WITH CHILD ALIVE - child of this", checkIfChild)
         }
      else
#  endif
      {
      char buffer[4096];

      uint32_t len = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("DEAD OF SOURCE STRING WITH CHILD ALIVE: child(%u) source(%u) = %V"), child, _length, this);

      if (check_dead_of_source_string_with_child_alive)
         {
         U_INTERNAL_ASSERT_MSG(false, buffer)
         }
      else
         {
         U_WARNING("%.*s", len, buffer);
         }
      }
      }
   }
# endif
# ifdef DEBUG
   U_UNREGISTER_OBJECT(0, this)
# endif
#endif

#ifndef ENABLE_MEMPOOL
   U_SYSCALL_VOID(free, "%p", (void*)this);
#else
   U_INTERNAL_DUMP("_capacity = %d (_capacity <= U_CAPACITY) = %b", (int32_t)_capacity, (_capacity <= U_CAPACITY))

   if (_capacity <= U_CAPACITY)
      {
      if (_capacity == 0) UMemoryPool::push(this, U_SIZE_TO_STACK_INDEX(sizeof(UStringRep))); // NB: no room for data, which mean constant string...
      else
         {
         // NB: we need an array of char[_capacity], plus a terminating null char element, plus enough for the UStringRep data structure...

         uint32_t sz = _capacity + (1 + sizeof(UStringRep));

         /**
          * power of 2:
          * -----------
          * 2^7    128
          * 2^8    256
          * 2^9    512
          * 2^10  1024
          * 2^11  2048
          * 2^12  4096
          */

         U_INTERNAL_ASSERT_EQUALS(sz & (sz-1), 0) // must be a power of 2

         UMemoryPool::push(this, MultiplyDeBruijnBitPosition2[(sz * 0x077CB531U) >> 27] - 3);
         }
      }
   else
      {
      if (_capacity != U_NOT_FOUND)
         {
#     if defined(USE_LIBTDB) || defined(USE_MONGODB)
         if (_capacity == U_TO_FREE) { U_SYSCALL_VOID(free, "%p", (void*)str); }
         else
#     endif
         UMemoryPool::deallocate((void*)str, _capacity);
         }
      else
         {
         ptrdiff_t resto = (ptrdiff_t)str % PAGESIZE;

         U_INTERNAL_DUMP("resto = %u _length = %u", resto, _length)

         if (resto)
            {
                str -= resto;
            _length += resto;
            }

         (void) U_SYSCALL(munmap, "%p,%lu", (void*)str, _length);
         }

      U_FREE_TYPE(this, UStringRep); // NB: in debug mode the memory area is zeroed...
      }
#endif
}

#ifdef DEBUG // substring capture event 'DEAD OF SOURCE STRING WITH CHILD ALIVE'...
int32_t     UStringRep::max_child;
UStringRep* UStringRep::parent_destroy;
UStringRep* UStringRep::string_rep_share;
bool        UStringRep::check_dead_of_source_string_with_child_alive = true;

bool UStringRep::checkIfReferences(const char* name_class, const void* ptr_object)
{
   U_TRACE(0, "UStringRep::checkIfReferences(%S,%p)", name_class, ptr_object)

   if (strncmp(name_class, U_CONSTANT_TO_PARAM("UString")) == 0)
      {
      U_INTERNAL_DUMP("references = %u", ((UString*)ptr_object)->rep->references)

      if (((UString*)ptr_object)->rep == string_rep_share) U_RETURN(true);
      }

   U_RETURN(false);
}

bool UStringRep::checkIfChild(const char* name_class, const void* ptr_object)
{
   U_TRACE(0, "UStringRep::checkIfChild(%S,%p)", name_class, ptr_object)

   if (strncmp(name_class, U_CONSTANT_TO_PARAM("UStringRep")) == 0)
      {
      U_INTERNAL_DUMP("parent = %p", ((UStringRep*)ptr_object)->parent)

      if (((UStringRep*)ptr_object)->parent == parent_destroy) U_RETURN(true);
      }

   U_RETURN(false);
}
#endif

UStringRep* UStringRep::substr(const char* t, uint32_t tlen) const
{
   U_TRACE(0+256, "UStringRep::substr(%.*S,%u)", tlen, t, tlen)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(tlen <= _length)

   UStringRep* r;

   if (tlen == 0)
      {
      r = string_rep_null;

      r->references++;
      }
   else
      {
      U_INTERNAL_ASSERT_RANGE(str, t, pend())

      U_NEW(UStringRep, r, UStringRep(t, tlen));

#  if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
      UStringRep* p = (UStringRep*)this;

      while (p->parent)
         {
         p = p->parent;

         U_INTERNAL_ASSERT(p->invariant())
         }

      r->parent = p;

#    ifdef U_SUBSTR_INC_REF
      p->references++; // substring increment reference of source string
#    else
      p->child++;      // substring capture event 'DEAD OF SOURCE STRING WITH CHILD ALIVE'...

      max_child = U_max(max_child, p->child);
#    endif

      U_INTERNAL_DUMP("r->parent = %p max_child = %d", r->parent, max_child)
#  endif
      }

   U_RETURN_POINTER(r, UStringRep);
}

__pure bool UStringRep::isSubStringOf(UStringRep* rep) const
{
   U_TRACE(0, "UStringRep::isSubStringOf(%V)", rep)

   U_CHECK_MEMORY

   if (this != rep           &&
       _capacity == 0        && // mode: 0 -> const
       data() >= rep->data() &&
       pend() <= rep->pend())
      {
#  if defined(U_SUBSTR_INC_REF) || defined(DEBUG)
      U_INTERNAL_ASSERT_EQUALS(parent, rep)
      U_INTERNAL_ASSERT_MAJOR(rep->child, 0)
#  endif

      U_RETURN(true);
      }

   U_RETURN(false);
}

void UStringRep::copy(char* s, uint32_t n, uint32_t pos) const
{
   U_TRACE(0, "UStringRep::copy(%p,%u,%u)", s, n, pos)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(pos <= _length)

   if (n > (_length - pos)) n = (_length - pos);

   U_INTERNAL_ASSERT_MAJOR(n, 0)

   U_MEMCPY(s, str + pos, n);

   s[n] = '\0';
}

void UStringRep::trim()
{
   U_TRACE_NO_PARAM(0, "UStringRep::trim()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_EQUALS(_capacity, 0)

   // skip white space from start

   while (_length && u__isspace(*str))
      {
      ++str;
      --_length;
      }

   U_INTERNAL_DUMP("_length = %u", _length)

   // skip white space from end

   while (_length && u__isspace(str[_length-1])) --_length;
}

__pure int UStringRep::compare(const UStringRep* rep, uint32_t depth) const
{
   U_TRACE(0, "UStringRep::compare(%p,%u)", rep, depth)

   U_CHECK_MEMORY

   int r;
   uint32_t min = U_min(_length, rep->_length);

   U_INTERNAL_DUMP("min = %u", min)

   if (depth > min) goto next;

   r = memcmp(str + depth, rep->str + depth, min - depth);

   U_INTERNAL_DUMP("str[%u] = %.*S", depth, min - depth, str + depth)

   if (r == 0)
next:
   r = (_length - rep->_length);

   U_RETURN(r);
}

__pure uint32_t UStringRep::findWhiteSpace(uint32_t pos) const
{
   U_TRACE(0, "UStringRep::findWhiteSpace(%u)", pos)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(pos <= _length)

   for (; pos < _length; ++pos)
      {
      if (u__isspace(str[pos])) U_RETURN(pos);
      }

   U_RETURN(U_NOT_FOUND);
}

__pure bool UStringRep::isEndHeader(uint32_t pos) const
{
   U_TRACE(0, "UStringRep::isEndHeader(%u)", pos)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_MINOR(pos, _length)

   const char* ptr  = str + pos;
   uint32_t _remain = (_length - pos);

   if (_remain >= 4 &&
       u_get_unalignedp32(ptr) == U_MULTICHAR_CONSTANT32('\r','\n','\r','\n'))
      {
   // U_line_terminator_len = 2;

      U_INTERNAL_ASSERT(u__islterm(*ptr))

      U_RETURN(true);
      }

   if (_remain >= 2 &&
       u_get_unalignedp16(ptr) == U_MULTICHAR_CONSTANT16('\n','\n'))
      {
   // U_line_terminator_len = 1;

      U_INTERNAL_ASSERT(u__islterm(*ptr))

      U_RETURN(true);
      }

   U_RETURN(false);
}

__pure bool UStringRep::findEndHeader(uint32_t pos) const
{
   U_TRACE(0, "UStringRep::findEndHeader(%u)", pos)

   U_CHECK_MEMORY

   if (_length)
      {
      U_INTERNAL_ASSERT_MINOR(pos, _length)

      const char* ptr  = str + pos;
      uint32_t _remain = (_length - pos);

      if (u_findEndHeader1(ptr, _remain) != U_NOT_FOUND) U_RETURN(true); // find sequence of U_CRLF2
      }

   U_RETURN(false);
}

UString::UString(const UString& str, uint32_t pos, uint32_t n)
{
   U_TRACE_REGISTER_OBJECT_WITHOUT_CHECK_MEMORY(0, UString, "%p,%u,%u", &str, pos, n)

   U_INTERNAL_ASSERT(pos <= str.size())

   uint32_t sz = str.rep->fold(pos, n);

   if (sz) rep = UStringRep::create(sz, sz, str.rep->str + pos);
   else    _copy(UStringRep::string_rep_null);

   U_INTERNAL_ASSERT(invariant())
}

UString::UString(uint32_t sz, const char* format, uint32_t fmt_size, ...) // ctor with var arg
{
   U_TRACE_REGISTER_OBJECT_WITHOUT_CHECK_MEMORY(0, UString, "%u,%.*S,%u", sz, fmt_size, format, fmt_size)

   U_INTERNAL_ASSERT_POINTER(format)

   va_list argp;
   va_start(argp, fmt_size);

   rep = UStringRep::create(0U, sz, 0);

   UString::vsnprintf(format, fmt_size, argp); 

   va_end(argp);
}

UString UString::copy() const
{
   U_TRACE_NO_PARAM(0, "UString::copy()")

   if (rep->_length)
      {
      uint32_t sz = rep->_length;

      UString copia((void*)rep->str, sz);

      U_RETURN_STRING(copia);
      }

   return getStringNull();
}

// SERVICES

UString::UString(uint32_t n, unsigned char c)
{
   U_TRACE_REGISTER_OBJECT_WITHOUT_CHECK_MEMORY(0, UString, "%u,%C", n, c)

   rep = UStringRep::create(n, n, 0);

   (void) memset((void*)rep->str, c, n);

   U_INTERNAL_ASSERT(invariant())
}

UString::UString(uint32_t len, uint32_t sz, char* ptr) // NB: for UStringExt::deflate()...
{
   U_TRACE_REGISTER_OBJECT_WITHOUT_CHECK_MEMORY(0, UString, "%u,%u,%p", len, sz, ptr)

   U_INTERNAL_ASSERT_MAJOR(sz, U_CAPACITY)
   U_ASSERT(UFile::checkPageAlignment(sz))

   rep = U_MALLOC_TYPE(UStringRep);

#ifdef DEBUG
   U_SET_LOCATION_INFO;
   U_REGISTER_OBJECT_PTR(0,UStringRep,rep,&(rep->memory._this))
   rep->memory._this = (void*)U_CHECK_MEMORY_SENTINEL;
#endif

   rep->set(len, sz, ptr);

   U_INTERNAL_ASSERT(invariant())
}

UString& UString::assign(const char* s, uint32_t n)
{
   U_TRACE(0, "UString::assign(%.*S,%u)", n, s, n)

   if (rep->references ||
       rep->_capacity < n)
      {
      if (n)
         {
         UStringRep* r;

         U_NEW(UStringRep, r, UStringRep(s, n));

         _set(r);
         }
      else
         {
         _assign(UStringRep::string_rep_null);
         }
      }
   else
      {
      char* ptr = (char*)rep->str;

      U_INTERNAL_ASSERT_MAJOR(n, 0)
      U_INTERNAL_ASSERT_DIFFERS(ptr, s)

      U_MEMCPY(ptr, s, n);

      U_ASSERT(rep->uniq())

      ptr[(rep->_length = n)] = '\0';
      }

   U_INTERNAL_ASSERT(invariant())

   return *this;
}

void UString::setBuffer(uint32_t n)
{
   U_TRACE(0, "UString::setBuffer(%u)", n)

   U_INTERNAL_ASSERT_RANGE(1, n, max_size())

   U_INTERNAL_DUMP("rep = %p rep->parent = %p rep->references = %u rep->child = %d rep->_capacity = %u",
                    rep,     rep->parent,     rep->references,     rep->child,     rep->_capacity)

   if (rep->references == 0 &&
       n <= rep->_capacity)
      {
      ((char*)rep->str)[(rep->_length = 0)] = '\0';
      }
   else
      {
      if (n < U_CAPACITY) n = U_CAPACITY;

      _set(UStringRep::create(0U, n, 0));
      }

   U_INTERNAL_ASSERT(invariant())
}

void UString::moveToBeginDataInBuffer(uint32_t n)
{
   U_TRACE(1, "UString::moveToBeginDataInBuffer(%u)", n)

   U_INTERNAL_ASSERT_MAJOR(rep->_length, n)
   U_INTERNAL_ASSERT_RANGE(1, n, max_size())
   U_INTERNAL_ASSERT_MAJOR(rep->_capacity, n)

#if defined(DEBUG) && !defined(U_SUBSTR_INC_REF)
   U_INTERNAL_ASSERT(rep->references == 0)
#endif

   rep->_length -= n;

#ifdef U_APEX_ENABLE
   (void) U_SYSCALL(apex_memmove, "%p,%p,%u", (void*)rep->str, rep->str + n, rep->_length);
#else
   (void) U_SYSCALL(     memmove, "%p,%p,%u", (void*)rep->str, rep->str + n, rep->_length);
#endif

   U_INTERNAL_ASSERT(invariant())
}

void UString::_reserve(UString& buffer, uint32_t n)
{
   U_TRACE(0, "UString::_reserve(%V,%u)", buffer.rep, n)

   UStringRep* rep = buffer.rep;

   U_INTERNAL_DUMP("rep = %p rep->parent = %p rep->references = %u rep->child = %d rep->_length = %u rep->_capacity = %u",
                    rep,     rep->parent,     rep->references,     rep->child,     rep->_length,     rep->_capacity)

   U_ASSERT(rep->space() < n)
   U_INTERNAL_ASSERT(n <= max_size())

   uint32_t need = rep->_length + n;

        if (need < U_CAPACITY) need = U_CAPACITY;
   else if (need > U_CAPACITY)
      {
      if (need < 2*1024*1024) need  = (need * 2) + (PAGESIZE * 2);
                              need += PAGESIZE; // NB: to avoid duplication on realloc...
      }

   buffer._set(UStringRep::create(rep->_length, need, rep->str));

   U_INTERNAL_ASSERT(buffer.invariant())
   U_INTERNAL_ASSERT(buffer.space() >= n)
}

// manage UString as memory mapped area...

void UString::mmap(const char* map, uint32_t len)
{
   U_TRACE(0, "UString::mmap(%.*S,%u)", len, map, len)

   U_INTERNAL_ASSERT_DIFFERS(map, MAP_FAILED)

   if (isMmap())
      {
      U_ASSERT(uniq())

      rep->str     = map;
      rep->_length = len;
      }
   else
      {
      UStringRep* r;

      U_NEW(UStringRep, r, UStringRep(map, len));

      _set(r);

      rep->_capacity = U_NOT_FOUND;
      }

   U_INTERNAL_ASSERT(invariant())
}

char* UString::__replace(uint32_t pos, uint32_t n1, uint32_t n2)
{
   U_TRACE(0, "UString::__replace(%u,%u,%u)", pos, n1, n2)

   U_INTERNAL_ASSERT_DIFFERS(n2, U_NOT_FOUND)

   uint32_t sz = size();

   U_INTERNAL_ASSERT(pos <= sz)

   uint32_t sz1 = rep->fold(pos, n1),
            n   = sz + n2  - sz1;

   U_INTERNAL_DUMP("sz1 = %u, n = %u", sz1, n)

   if (n == 0)
      {
      _assign(UStringRep::string_rep_null);

      return 0;
      }

   int32_t how_much = sz - pos - sz1;

   U_INTERNAL_DUMP("how_much = %d", how_much)

   U_INTERNAL_ASSERT(how_much >= 0)

         char* str = (char*)rep->str;
   const char* src = str + pos + sz1;

   uint32_t __capacity = rep->_capacity;

   if (__capacity == U_NOT_FOUND) __capacity = 0;

   if (rep->references ||
       n > __capacity)
      {
      U_INTERNAL_DUMP("__capacity = %u, n = %u", __capacity, n)

      if (__capacity < n) __capacity = n;

      UStringRep* r = UStringRep::create(n, __capacity, 0);

      if (pos)      U_MEMCPY((void*)r->str,            str, pos);
      if (how_much) U_MEMCPY((char*)r->str + pos + n2, src, how_much);

      _set(r);

      str = (char*)r->str;
      }
   else if (how_much > 0 &&
            n1 != n2)
      {
#  ifdef U_APEX_ENABLE
      (void) U_SYSCALL(apex_memmove, "%p,%p,%u", str + pos + n2, src, how_much);
#  else
      (void) U_SYSCALL(     memmove, "%p,%p,%u", str + pos + n2, src, how_much);
#  endif
      }

   U_ASSERT(uniq())

   str[(rep->_length = n)] = '\0';

   return str + pos;
}

void UString::unQuote()
{
   U_TRACE_NO_PARAM(0, "UString::unQuote()")

   U_ASSERT(uniq())

   uint32_t len = rep->_length;

        if (len            <= 2) clear();
   else if (rep->_capacity == 0) rep->unQuote();
   else
      {
      len -= 2;

      char* ptr = (char*) rep->str;

#  ifdef U_APEX_ENABLE
      (void) U_SYSCALL(apex_memmove, "%p,%p,%u", ptr, ptr + 1, len);
#  else
      (void) U_SYSCALL(     memmove, "%p,%p,%u", ptr, ptr + 1, len);
#  endif

      ptr[(rep->_length = len)] = '\0';
      }
}

char* UString::__append(uint32_t n)
{
   U_TRACE(0, "UString::__append(%u)", n)

   UStringRep* r;
   char* str = (char*)rep->str;
   uint32_t sz = rep->_length, need = sz + n;

   U_INTERNAL_DUMP("need = %u", need)

   U_INTERNAL_ASSERT_MAJOR(need, 0)

   if (rep->references ||
       need > rep->_capacity)
      {
      r = UStringRep::create(sz, (need < U_CAPACITY ? U_CAPACITY : (need * 2) + (PAGESIZE * 2)), str);

      _set(r);

      str = (char*)r->str;
      }

   U_ASSERT(uniq())

   str[(rep->_length = need)] = '\0';

   return str + sz;
}

UString& UString::append(uint32_t n, char c)
{
   U_TRACE(1, "UString::append(%u,%C)", n, c)

   if (n)
      {
      char* ptr    = __append(n);
            ptr[0] = c;

      if (--n) (void) U_SYSCALL(memset, "%p,%d,%u", ptr+1, c, n);
      }

   U_INTERNAL_ASSERT(invariant())

   return *this;
}

void UString::duplicate() const
{
   U_TRACE_NO_PARAM(0, "UString::duplicate()")

   uint32_t sz = size();

   if (sz) ((UString*)this)->_set(UStringRep::create(sz, sz, rep->str));
   else
      {
      ((UString*)this)->_set(UStringRep::create(0, 100U, 0));

      *(((UString*)this)->UString::rep->data()) = '\0';
      }

   U_INTERNAL_ASSERT(invariant())
   U_INTERNAL_ASSERT(isNullTerminated())
}

void UString::setNullTerminated() const
{
   U_TRACE_NO_PARAM(0, "UString::setNullTerminated()")

   // A file is mapped in multiples of the page size. For a file that is not a multiple of the page size,
   // the remaining memory is zeroed when mapped, and writes to that region are not written out to the file

   if (writeable() == false ||
       (isMmap() && (rep->_length % PAGESIZE) == 0))
      {
      duplicate();
      }
   else
      {
      rep->setNullTerminated();
      }

   U_ASSERT_EQUALS(u__strlen(rep->str, __PRETTY_FUNCTION__), rep->_length)
}

void UString::resize(uint32_t n, unsigned char c)
{
   U_TRACE(0, "UString::resize(%u,%C)", n, c)

   U_INTERNAL_ASSERT(n <= max_size())

   uint32_t sz = size();

   if      (n > sz) (void) append(n - sz, c);
   else if (n < sz) erase(n);
   else             size_adjust(n);

   U_INTERNAL_ASSERT(invariant())
}

// The `find' function searches string for a specified string (possibly a single character) and returns
// its starting position. You can supply the parameter pos to specify the position where search must begin

__pure uint32_t UString::find(const char* s, uint32_t pos, uint32_t s_len, uint32_t how_much) const
{
   U_TRACE(0, "UString::find(%.*S,%u,%u,%u)", s_len, s, pos, s_len, how_much)

   U_INTERNAL_ASSERT_MAJOR(s_len, 0)

// An empty string consists of no characters, therefore it should be found at every point in a UString, except beyond the end...
// if (s_len == 0) U_RETURN(pos <= size() ? pos : U_NOT_FOUND);

   uint32_t n = rep->fold(pos, how_much);

   U_INTERNAL_DUMP("rep->_length = %u", rep->_length)

   U_INTERNAL_ASSERT(n <= rep->_length)

   const char* str = rep->str;
   const char* ptr = (const char*) u_find(str + pos, n, s, s_len);

   n = (ptr ? ptr - str : U_NOT_FOUND);

   U_RETURN(n);
}

__pure uint32_t UString::findnocase(const char* s, uint32_t pos, uint32_t s_len, uint32_t how_much) const
{
   U_TRACE(0, "UString::findnocase(%.*S,%u,%u,%u)", s_len, s, pos, s_len, how_much)

   U_INTERNAL_ASSERT_MAJOR(s_len, 1)

   uint32_t n     = rep->fold(pos, how_much);
    int32_t __end = n - s_len + 1;

   if (__end > 0)
      {
      const char* str = rep->str + pos;

      for (int32_t xpos = 0; xpos < __end; ++xpos)
         {
         if (u__strncasecmp(str + xpos, s, s_len) == 0) U_RETURN(pos+xpos);
         }
      }

   U_RETURN(U_NOT_FOUND);
}

// rfind() instead of starting at the beginning of the string and searching for the text's first occurence, starts its search at the end and returns the last occurence

__pure uint32_t UString::rfind(unsigned char c, uint32_t pos) const
{
   U_TRACE(0, "UString::rfind(%C,%u)", c, pos)

   uint32_t sz = size();

   if (sz)
      {
      uint32_t xpos = sz - 1;

      if (xpos > pos) xpos = pos;

      for (++xpos; xpos-- > 0; )
         {
         if (rep->str[xpos] == c) U_RETURN(xpos);
         }
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UString::rfind(const char* s, uint32_t pos, uint32_t n) const
{
   U_TRACE(0, "UString::rfind(%.*S,%u,%u)", n, s, pos, n)

   uint32_t sz = size();

   if (n <= sz)
      {
      pos = U_min(sz - n, pos);

      do {
         if (memcmp(rep->str + pos, s, n) == 0) U_RETURN(pos);
         }
      while (pos-- > 0);
      }

   U_RETURN(U_NOT_FOUND);
}

// Instead of searching for the entire string, find_first_of() returns as soon as a single common element is found between the strings being compared.
// And yes, this means that the find_first_of() that take a single char are exactly the same as the find() functions with the same parameters...

__pure uint32_t UString::find_first_of(const char* s, uint32_t pos, uint32_t n) const
{
   U_TRACE(0, "UString::find_first_of(%.*S,%u,%u)", n, s, pos, n)

   if (n)
      {
      for (; pos < size(); ++pos)
         {
         if (memchr(s, rep->str[pos], n)) U_RETURN(pos);
         }
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UString::find_last_of(const char* s, uint32_t pos, uint32_t n) const
{
   U_TRACE(0, "UString::find_last_of(%.*S,%u,%u)", n, s, pos, n)

   uint32_t sz = size();

   if (sz && n)
      {
      if (--sz > pos) sz = pos;

      do {
         if (memchr(s, rep->str[sz], n)) U_RETURN(sz);
         }
      while (sz-- != 0);
      }

   U_RETURN(U_NOT_FOUND);
}

// Now these functions, instead of returning an index to the first common element, returns an index to the first non-common element...

__pure uint32_t UString::find_first_not_of(const char* s, uint32_t pos, uint32_t n) const
{
   U_TRACE(0, "UString::find_first_not_of(%.*S,%u,%u)", n, s, pos, n)

   if (n)
      {
      uint32_t sz   = size(),
               xpos = pos;

      for (; xpos < sz; ++xpos)
         {
         if (memchr(s, rep->str[xpos], n) == 0) U_RETURN(xpos);
         }
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UString::find_first_not_of(unsigned char c, uint32_t pos) const
{
   U_TRACE(0, "UString::find_first_not_of(%C,%u)", c, pos)

   uint32_t sz   = size(),
            xpos = pos;

   const char* str = rep->str;

   for (; xpos < sz; ++xpos)
      {
      if (str[xpos] != c) U_RETURN(xpos);
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UString::find_last_not_of(const char* s, uint32_t pos, uint32_t n) const
{
   U_TRACE(0, "UString::find_last_not_of(%.*S,%u,%u)", n, s, pos, n)

   uint32_t sz = size();

   if (n &&
       sz)
      {
      if (--sz > pos) sz = pos;

      do {
         if (memchr(s, rep->str[sz], n) == 0) U_RETURN(sz);
         }
      while (sz-- != 0);
      }

   U_RETURN(U_NOT_FOUND);
}

__pure uint32_t UString::find_last_not_of(unsigned char c, uint32_t pos) const
{
   U_TRACE(0, "UString::find_last_not_of(%C,%u)", c, pos)

   uint32_t sz = size();

   if (sz)
      {
      uint32_t xpos = sz - 1;

      if (xpos > pos) xpos = pos;

      const char* str = rep->str;

      for (++xpos; xpos-- > 0; )
         {
         if (str[xpos] != c) U_RETURN(xpos);
         }
      }

   U_RETURN(U_NOT_FOUND);
}

// EXTENSION

__pure bool UStringRep::strtob() const
{
   U_TRACE_NO_PARAM(0, "UStringRep::strtob()")

   if (_length)
      {
      switch (u_get_unalignedp16(str))
         {
         case U_MULTICHAR_CONSTANT16('1',0):
         case U_MULTICHAR_CONSTANT16('o','n'):
         case U_MULTICHAR_CONSTANT16('O','n'):
         case U_MULTICHAR_CONSTANT16('O','N'): U_RETURN(true);
         }

      switch (u_get_unalignedp32(str))
         {
         case U_MULTICHAR_CONSTANT32('y','e','s',0):
         case U_MULTICHAR_CONSTANT32('Y','e','s',0):
         case U_MULTICHAR_CONSTANT32('Y','E','S',0):
         case U_MULTICHAR_CONSTANT32('t','r','u','e'):
         case U_MULTICHAR_CONSTANT32('T','r','u','e'):
         case U_MULTICHAR_CONSTANT32('T','R','U','E'): U_RETURN(true); 
         }
      }

   U_RETURN(false);
}

#define U_MANAGE_CHECK_FOR_SUFFIX \
   if (check_for_suffix == false) while (u__isdigit(*(endptr-1)) == false) --endptr; \
   else \
      { \
      suffix = *(endptr-1); \
                        \
      if (suffix != 'M' && \
          suffix != 'G' && \
          u__toupper(suffix) != 'K') \
         { \
         suffix = 0; \
         } \
      else \
         { \
         --endptr; \
                   \
         U_INTERNAL_ASSERT(u__isdigit(*(endptr-1))) \
         } \
      }

__pure long UStringRep::strtol(bool check_for_suffix) const
{
   U_TRACE(0, "UString::strtol(%b)", check_for_suffix)

   if (_length)
      {
      char suffix        = 0;
      const char* s      = str;
      const char* endptr = str + _length;

      while (u__isspace(*s)) ++s;

      U_MANAGE_CHECK_FOR_SUFFIX

      long value = u_strtol(s, endptr);

#  ifdef DEBUG
      {
      long tmp = ::strtol(s, 0, 10);

      if (value != tmp)
         {
         U_WARNING("value(%V) = %ld differs from ::strtol() = %ld", this, value, tmp);
         }
      }
#  endif

      if (suffix)
         {
         U_NUMBER_SUFFIX(value, suffix);
         }

      U_RETURN(value);
      }

   U_RETURN(0);
}

__pure unsigned long UStringRep::strtoul(bool check_for_suffix) const
{
   U_TRACE(0, "UString::strtoul(%b)", check_for_suffix)

   if (_length)
      {
      char suffix        = 0;
      const char* s      = str;
      const char* endptr = str + _length;

      while (u__isspace(*s)) ++s;

      U_MANAGE_CHECK_FOR_SUFFIX

      unsigned long value = u_strtoul(s, endptr);

#  ifdef DEBUG
      {
      unsigned long tmp = ::strtoul(s, 0, 10);

      if (value != tmp)
         {
         U_WARNING("value(%V) = %lu differs from ::strtoul() = %lu", this, value, tmp);
         }
      }
#  endif

      if (suffix)
         {
         U_NUMBER_SUFFIX(value, suffix);
         }

      U_RETURN(value);
      }

   U_RETURN(0);
}

__pure int64_t UStringRep::strtoll(bool check_for_suffix) const
{
   U_TRACE(0, "UString::strtoll(%b)", check_for_suffix)

   if (_length)
      {
      char suffix        = 0;
      const char* s      = str;
      const char* endptr = str + _length;

      while (u__isspace(*s)) ++s;

      U_MANAGE_CHECK_FOR_SUFFIX

      int64_t value = u_strtoll(s, endptr);

#  if defined(DEBUG) && defined(HAVE_STRTOULL)
      {
      int64_t tmp = ::strtoll(s, 0, 10);

      if (value != tmp)
         {
         U_WARNING("value(%V) = %lld differs from ::strtol() = %lld", this, value, tmp);
         }
      }
#  endif

      if (suffix)
         {
         U_NUMBER_SUFFIX(value, suffix);
         }

      U_RETURN(value);
      }

   U_RETURN(0);
}

__pure uint64_t UStringRep::strtoull(bool check_for_suffix) const
{
   U_TRACE(0, "UString::strtoull(%b)", check_for_suffix)

   if (_length)
      {
      char suffix        = 0;
      const char* s      = str;
      const char* endptr = str + _length;

      while (u__isspace(*s)) ++s;

      U_MANAGE_CHECK_FOR_SUFFIX

      uint64_t value = u_strtoull(s, endptr);

#  if defined(DEBUG) && defined(HAVE_STRTOULL)
      {
      uint64_t tmp = ::strtoull(s, 0, 10);

      if (value != tmp)
         {
         U_WARNING("value(%V) = %llu differs from ::strtol() = %llu", this, value, tmp);
         }
      }
#  endif

      if (suffix)
         {
         U_NUMBER_SUFFIX(value, suffix);
         }

      U_RETURN(value);
      }

   U_RETURN(0);
}

#undef U_MANAGE_CHECK_FOR_SUFFIX

#ifdef HAVE_STRTOF
// extern "C" { float strtof(const char* nptr, char** endptr); }
float UStringRep::strtof() const
{
   U_TRACE_NO_PARAM(0, "UStringRep::strtof()")

   if (_length)
      {
      char* eos = (char*)str + _length;

      if (isNullTerminated() == false && writeable()) *eos = '\0';

#  ifndef DEBUG
      float result = ::strtof(str, 0);
#  else
      char* endptr;
      float result = ::strtof(str, &endptr);

      U_INTERNAL_ASSERT(endptr <= eos)
#  endif

      U_INTERNAL_DUMP("errno = %d", errno)

      U_RETURN(result);
      }

   U_RETURN(0);
}
#endif

#ifdef HAVE_STRTOLD
// extern "C" { long double strtold(const char* nptr, char** endptr); }
long double UStringRep::strtold() const
{
   U_TRACE_NO_PARAM(0, "UStringRep::strtold()")

   if (_length)
      {
      char* eos = (char*)str + _length;

      if (isNullTerminated() == false && writeable()) *eos = '\0';

#  ifndef DEBUG
      long double result = ::strtold(str, 0);
#  else
      char* endptr;
      long double result = ::strtold(str, &endptr);

      U_INTERNAL_ASSERT_MINOR(endptr, eos)
#  endif

      U_INTERNAL_DUMP("errno = %d", errno)

      U_RETURN(result);
      }

   U_RETURN(0);
}
#endif

// UTF8 <--> ISO Latin 1

UStringRep* UStringRep::fromUTF8(const unsigned char* s, uint32_t n)
{
   U_TRACE(0, "UStringRep::fromUTF8(%.*S,%u)", n, s, n)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_MAJOR(n, 0)

   int c, c1, c2;
   UStringRep* r = UStringRep::create(n, n, 0);

   char* p                   = (char*)r->str;
   const unsigned char* _end = s + n;

   while (s < _end)
      {
      if (  s < (_end - 1)        &&
          (*s     & 0xE0) == 0xC0 &&
          (*(s+1) & 0xC0) == 0x80)
         {
         c1 = *s++ & 0x1F;
         c2 = *s++ & 0x3F;
         c  = (c1 << 6) + c2;

         U_INTERNAL_DUMP("c = %d %C", c, (char)c)
         }
      else
         {
         c = *s++;
         }

      *p++ = (unsigned char)c;
      }

   r->_length = p - r->str;

   U_INTERNAL_ASSERT(r->invariant())

   U_RETURN_POINTER(r, UStringRep);
}

UStringRep* UStringRep::toUTF8(const unsigned char* s, uint32_t n)
{
   U_TRACE(0, "UStringRep::toUTF8(%.*S,%u)", n, s, n)

   U_INTERNAL_ASSERT_POINTER(s)
   U_INTERNAL_ASSERT_MAJOR(n, 0)

   UStringRep* r = UStringRep::create(n, n * 2, 0);

   char* p                   = (char*)r->str;
   const unsigned char* _end = s + n;

   while (s < _end)
      {
      int c = *s++;

      if (c >= 0x80)
         {
         *p++ = ((c >> 6) & 0x1F) | 0xC0;
         *p++ = ( c       & 0x3F) | 0x80;

         continue;
         }

      *p++ = (unsigned char)c;
      }

   r->_length = p - r->str;

   U_INTERNAL_ASSERT(r->invariant())

   U_RETURN_POINTER(r, UStringRep);
}

double UString::strtod() const
{
   U_TRACE_NO_PARAM(0, "UString::strtod()")

   UValue json;

   if (json.parse(*this)) return json.getDouble();

   U_RETURN(0);
}

void UString::printKeyValue(const char* key, uint32_t keylen, const char* _data, uint32_t datalen)
{
   U_TRACE(0, "UString::printKeyValue(%.*S,%u,%.*S,%u,%d)", keylen, key, keylen, datalen, _data, datalen)

   uint32_t n = 5 + 18 + keylen + datalen; 

   if (rep->space() < n) _reserve(*this, n);

   char* ptr = (char*)rep->str + rep->_length;

   ptr += u__snprintf(ptr, 40, U_CONSTANT_TO_PARAM("+%u,%u:"), keylen, datalen);

   U_MEMCPY(ptr, key, keylen);
            ptr +=    keylen;

   U_MEMCPY(ptr, "->", U_CONSTANT_SIZE("->"));
            ptr +=     U_CONSTANT_SIZE("->");

   U_MEMCPY(ptr, _data, datalen);
            ptr +=      datalen;

   u_put_unalignedp16(ptr, U_MULTICHAR_CONSTANT16('\n','\0'));

   rep->_length = (ptr - rep->str) + 1;

   U_INTERNAL_ASSERT(invariant())
}

void UString::setFromData(const char** p, uint32_t sz, unsigned char delim)
{
   U_TRACE(0, "UString::setFromData(%.*S,%u,%C)", sz, *p, sz, delim)

   U_INTERNAL_ASSERT_MAJOR(sz, 0)
   U_INTERNAL_ASSERT_EQUALS(rep->_length, 0)

   const char* ptr   = *p;
   unsigned char c   = *ptr;
   const char* _pend =  ptr + sz;

   U_INTERNAL_DUMP("c = %C", c)

   U_INTERNAL_ASSERT_EQUALS(u__isspace(c), false)

   if (c == '@' &&
       UVector<void*>::istream_loading == false)
      {
      // get content pointed by string 'meta' (that start with '@')

      if (u_get_unalignedp32(ptr+1) == U_MULTICHAR_CONSTANT32('F','I','L','E'))
         {
         UFile file;
         char pathname[U_PATH_MAX];

         ptr += U_CONSTANT_SIZE("@FILE:");

         if (*ptr == '"') ++ptr; // check if string is quoted...

         U_INTERNAL_ASSERT_EQUALS(u__isspace(*ptr), false)

         for (char* path = pathname; ptr < _pend; ++path, ++ptr)
            {
            c = *ptr;

            if (c == '"' ||
                u__isspace(c))
               {
               if (c == '"') ++ptr;

               *path = '\0';

               break;
               }

            *path = c;
            }

         *p = ptr;

         if (file.open(pathname)) *this = file.getContent();
         else
            {
            U_WARNING("Open file %S specified in configuration failed", pathname);
            }

         U_INTERNAL_DUMP("size = %u, str = %V", size(), rep)

         U_INTERNAL_ASSERT(invariant())

         return;
         }

      U_INTERNAL_ASSERT_EQUALS(memcmp(ptr, U_CONSTANT_TO_PARAM("@STRING:")), 0)

      ptr += U_CONSTANT_SIZE("@STRING:");

      const char* start;

      if (*ptr == '"') // check if string is quoted...
         {
         ptr = u_find_char((start = (ptr+1)), _pend, '"'); // find char '"' not quoted

         if (ptr == _pend)
            {
            (void) append(ptr, _pend - ptr);

            *p = _pend;

            U_INTERNAL_DUMP("size = %u, str = %V", size(), rep)

            U_INTERNAL_ASSERT(invariant())

            return;
            }

         U_INTERNAL_ASSERT_EQUALS(*ptr, '"')

         sz = ptr++ - start;
         }
      else
         {
         for (start = ptr; ptr < _pend; ++ptr)
            {
            c = *ptr;

            if (u__isspace(c)) break;
            }

         sz = ptr - start;
         }

      setBuffer(sz * 4);

      UEscape::decode(start, sz, *this);

      U_INTERNAL_ASSERT_MAJOR(rep->_length, 0)

      *p = ptr;

      U_INTERNAL_DUMP("size = %u, str = %V", size(), rep)

      U_INTERNAL_ASSERT(invariant())

      return;
      }

loop:
   if ( delim == c     ||
       (delim != '"'   &&
        u__isspace(c)) ||
       (delim == '\0'  &&
        (c == '}'      ||
         c == ']')))
      {
      ++ptr;

      goto end;
      }

   if (c == '\\')
      {
      c = *++ptr;

      U_INTERNAL_DUMP("c (after '\\') = %C", c)

      if (c != delim)
         {
         if (c == '\n')
            {
            // compress multiple white-space in a single new-line...

            U_INTERNAL_DUMP("ptr+1 = %.*S", 20, ptr+1)

            while (ptr < _pend)
               {
               if (u__isspace(ptr[1]) == false) break;

               ++ptr;
               }

            U_INTERNAL_DUMP("ptr+1 = %.*S", 20, ptr+1)
            }
         else if (strchr("nrtbfvae", c))
            {
            _append('\\');
            }
         }
      }

   _append(c);

   if (++ptr <= _pend)
      {
      c = *ptr;

      U_INTERNAL_DUMP("c = %C", c)

      goto loop;
      }

end:
   _append();

   *p = ptr;

   if (empty()) _assign(UStringRep::string_rep_null);
   else
      {
      if (shrink() == false) setNullTerminated();
      }

   U_INTERNAL_DUMP("size = %u, str = %V", size(), rep)

   U_INTERNAL_ASSERT(invariant())
}

// STREAM

#ifdef U_STDCPP_ENABLE
void UString::get(istream& is)
{
   U_TRACE(0, "UString::get(%p)", &is) // problem with sanitize address

   if (is.peek() != '"') is >> *this;
   else
      {
      (void) is.get(); // skip '"'

      (void) getline(is, '"');
      }
}

void UStringRep::write(ostream& os) const
{
   U_TRACE(0, "UStringRep::write(%p)", &os)

   bool need_quote = (_length == 0);

   if (need_quote == false)
      {
      for (const unsigned char* s = (const unsigned char*)str, *_end = s + _length; s < _end; ++s)
         {
         unsigned char c = *s;

         if (c == '"'  ||
             c == '\\' ||
             u__isspace(c))
            {
            need_quote = true;

            break;
            }
         }
      }

   if (need_quote == false) os.write(str, _length);
   else
      {
      os.put('"');

      char* p;
      char* s    = (char*)str;
      char* _end = s + _length;

      while (s < _end)
         {
         p = (char*) memchr(s, '"', _end - s);

         if (p == 0)
            {
            os.write(s, _end - s);

            break;
            }

         os.write(s, p - s);

         if (*(p-1) == '\\') os.put('\\');
                             os.put('\\');
                             os.put('"');

         s = p + 1;
         }

      os.put('"');
      }
}

U_EXPORT istream& operator>>(istream& in, UString& str)
{
   U_TRACE(0+256, "UString::operator>>(%p,%p)", &in, &str)

   uint32_t extracted = 0;

   if (in.good())
      {
      streambuf* sb = in.rdbuf();

      int c = sb->sbumpc();

      if (in.flags() & ios::skipws)
         {
         while ((c != EOF) &&
                u__isspace(c))
            {
            c = sb->sbumpc();
            }
         }

      if (c != EOF)
         {
         if (str)
            {
            if (str.uniq()) str.setEmpty();
            else            str._set(UStringRep::create(0U, U_CAPACITY, 0)); // NB: we need this because we use the same object for all input stream of vector (see vector.h:830)...
            }

         streamsize w = in.width();

         uint32_t n = (w > 0 ? (uint32_t)w
                             : str.max_size());

         while (extracted < n &&
                u__isspace(c) == false)
            {
            str._append(c);

            ++extracted;

            c = sb->sbumpc();

         // U_INTERNAL_DUMP("c = %C, EOF = %C", c, EOF)

            if (c == EOF) break;
            }

         str._append();
         }

      if (c == EOF) in.setstate(ios::eofbit);
      else          sb->sputbackc(c);

      in.width(0);

      U_INTERNAL_DUMP("size = %u, str = %V", str.size(), str.rep)
      }

        if (extracted == 0) in.setstate(ios::failbit);
   else if (str.shrink() == false) str.setNullTerminated();

   U_INTERNAL_ASSERT(str.invariant())

   return in;
}

istream& UString::getline(istream& in, unsigned char delim)
{
   U_TRACE(0+256, "UString::getline(%p,%C)", &in, delim)

   int c          = EOF;
   bool extracted = false;

   if (in.good())
      {
      if (size())
         {
         if (uniq()) setEmpty();
         else        _set(UStringRep::create(0U, U_CAPACITY, 0)); // NB: we need this because we use the same object for all input stream of vector (see vector.h:830)...
         }

      streambuf* sb = in.rdbuf();

      while (true)
         {
         c = sb->sbumpc();

         U_INTERNAL_DUMP("c = %C", c)

         if (c == '\\')
            {
            c = sb->sbumpc();

            U_INTERNAL_DUMP("c = %C", c)

            if (c == delim)
               {
               _append(delim);

               continue;
               }

            if (strchr("nrt\nbfvae", c))
               {
               if (c != '\n') _append('\\');
               else
                  {
                  // compress multiple white-space in a single new-line...

                  do { c = sb->sbumpc(); } while (c != EOF && u__isspace(c));

                  if (c != EOF)
                     {
                     sb->sputbackc(c);

                     c = '\n';
                     }
                  }
               }
            }

         if (c == EOF)
            {
            in.setstate(ios::eofbit);

            break;
            }

         if (c == delim) break;

         _append(c);
         }

      _append();

      U_INTERNAL_DUMP("size = %u, str = %V", size(), rep)

      extracted = (empty() == false);

      if (extracted &&
          shrink() == false)
         {
         setNullTerminated();
         }
      }

   if (c         != delim &&
       extracted == false)
      {
      in.setstate(ios::failbit);
      }

   U_INTERNAL_ASSERT(invariant())

   return in;
}

U_EXPORT ostream& operator<<(ostream& out, const UString& str)
{
   U_TRACE(0, "UString::operator<<(%p,%p)", &out, &str)

   if (out.good())
      {
      const char* s = str.data();

      streamsize res,
                 w   = out.width(),
                 len = (streamsize)str.size();

      if (w <= len) res = out.rdbuf()->sputn(s, len);
      else
         {
         int plen = (int)(w - len);

         ios::fmtflags fmt = (out.flags() & ios::adjustfield);

         if (fmt == ios::left)
            {
            res = out.rdbuf()->sputn(s, len);

            // Padding last

            for (int i = 0; i < plen; ++i) (void) out.rdbuf()->sputc(' ');
            }
         else
            {
            // Padding first

            for (int i = 0; i < plen; ++i) (void) out.rdbuf()->sputc(' ');

            res = out.rdbuf()->sputn(s, len);
            }
         }

      U_INTERNAL_DUMP("len = %u, res = %u, w = %u", len, res, w)

      out.width(0);

      if (res != len) out.setstate(ios::failbit);
      }

   return out;
}
#endif

// operator +

U_EXPORT UString operator+(const UString& lhs, const UString& rhs)
{
   uint32_t sz1 = lhs.size(),
            sz2 = rhs.size();

   UString str(sz1 + sz2);

   (void) str.append(lhs.data(), sz1);
   (void) str.append(rhs.data(), sz2);

   return str;
}

U_EXPORT UString operator+(const UString& lhs, const char* rhs)
{
   uint32_t sz1 = lhs.size(),
            sz2 = u__strlen(rhs, __PRETTY_FUNCTION__);

   UString str(sz1 + sz2);

   (void) str.append(lhs.data(), sz1);
   (void) str.append(rhs,        sz2);

   return str;
}

U_EXPORT UString operator+(const char* lhs, const UString& rhs)
{
   uint32_t sz2 = rhs.size(),
            sz1 = u__strlen(lhs, __PRETTY_FUNCTION__);

   UString str(sz1 + sz2);

   (void) str.append(lhs,        sz1);
   (void) str.append(rhs.data(), sz2);

   return str;
}

U_EXPORT UString operator+(const UString& lhs, char rhs)
{
   uint32_t sz = lhs.size();

   UString str(sz + 1U);

   (void) str.append(lhs.data(), sz);
   (void) str.append(1U, rhs);

   return str;
}

U_EXPORT UString operator+(char lhs, const UString& rhs)
{
   uint32_t sz = rhs.size();

   UString str(sz + 1U);

   (void) str.append(1U, lhs);
   (void) str.append(rhs.data(), sz);

   return str;
}

#ifdef DEBUG
const char* UStringRep::dump(bool reset) const
{
#ifdef U_STDCPP_ENABLE
   *UObjectIO::os << "length     " << _length       << '\n'
                  << "capacity   " << _capacity     << '\n'
                  << "references " << references    << '\n'
                  << "parent     " << (void*)parent << '\n'
                  << "child      " << child         << '\n'
                  << "str        " << (void*)str    << ' ';

   char buffer[1024];

   UObjectIO::os->write(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%V"), this));

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }
#endif

   return 0;
}

bool UStringRep::invariant() const
{
   if (_capacity &&
       _capacity < _length)
      {
      U_WARNING("Error on rep string: (overflow)\n"
                "--------------------------------------------------\n%s", UStringRep::dump(true));

      return false;
      }

   if ((int32_t)references < 0)
      {
      U_WARNING("Error on rep string: (leak reference)\n"
                "--------------------------------------------------\n%s", UStringRep::dump(true));

      return false;
      }

   if (this == string_rep_null)
      {
      U_CHECK_MEMORY

      if (_length)
         {
         U_WARNING("Error on string_rep_null: (not empty)\n"
                   "--------------------------------------------------\n%s", UStringRep::dump(true));

         return false;
         }

      return true;
      }

   return string_rep_null->invariant();
}

bool UString::invariant() const
{
   if (rep == 0)
      {
      U_WARNING("Error on string: (rep = null pointer)");

      return false;
      }

   return rep->invariant();
}

void UString::vsnprintf_check(const char* format) const
{
   bool ok_writeable  = writeable(),
        ok_isNull     = (isNull() == false),
        ok_references = (rep->references == 0),
        ok_format     = (rep->_capacity > u__strlen(format, __PRETTY_FUNCTION__));

   if (ok_writeable == false ||
       ok_isNull    == false ||
       ok_format    == false)
      {
      // -----------------------------------------------------------------------------------------------------------------------------------------
      // Ex: userver_tcp: ERROR: UString::vsnprintf_check() this = 0xa79bbd18 parent = (nil) references = 2126 child = 0 _capacity = 0 str(0) = ""
      //                  format = "%v:" - ok_writeable = false ok_isNull = false ok_references = false ok_format = false
      // -----------------------------------------------------------------------------------------------------------------------------------------

      U_ERROR("UString::vsnprintf_check() this = %p parent = %p rep = %p references = %u child = %d _capacity = %u str(%u) = %V format = %S - "
              "ok_writeable = %b ok_isNull = %b ok_references = %b ok_format = %b",
               this, rep->parent, rep, rep->references, rep->child, rep->_capacity, rep->_length, rep, format, ok_writeable, ok_isNull, ok_references, ok_format);
      }
   else if (ok_references == false)
      {
      U_WARNING("UString::vsnprintf_check() this = %p parent = %p rep = %p references = %u child = %d _capacity = %u str(%u) = %V format = %S - "
                "ok_writeable = %b ok_isNull = %b ok_references = %b ok_format = %b",
                this, rep->parent, rep, rep->references, rep->child, rep->_capacity, rep->_length, rep, format, ok_writeable, ok_isNull, ok_references, ok_format);
      }
}
#endif

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UString::dump(bool reset) const
{
   U_CHECK_MEMORY_OBJECT(rep)

   *UObjectIO::os << "rep (UStringRep " << (void*)rep << ")";

   if (rep == rep->string_rep_null) UObjectIO::os->write(U_CONSTANT_TO_PARAM(" == UStringRep::string_rep_null"));

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
