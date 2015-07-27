// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    http2.cpp - HTTP/2 utility 
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/http2.h>
#include <ulib/utility/uhttp.h>
#include <ulib/utility/base64.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/hpack_huffman_table.h> // huff_sym_table, huff_decode_table

#define HTTP2_FRAME_HEADER_SIZE    9 // The number of bytes of the frame header
#define HTTP2_HEADER_TABLE_OFFSET 62

int                           UHTTP2::nerror;
bool                          UHTTP2::settings_ack;
void*                         UHTTP2::pConnectionEnd;
uint32_t                      UHTTP2::hash_static_table[61];
const char*                   UHTTP2::upgrade_settings;
UHTTP2::Stream*               UHTTP2::pStream;
UHTTP2::FrameHeader           UHTTP2::frame;
UHTTP2::Connection*           UHTTP2::pConnection;
UHTTP2::HpackHeaderTableEntry UHTTP2::hpack_static_table[61];

// ================================
//       SETTINGS FRAME
// ================================
// frame size = 18 (3*6)
// settings frame
// no flags
// stream id = 0
// ================================
// max_concurrent_streams = 100
// initial_window_size = 2147483646
// max_frame_size = 16777215
// ================================
// 9 + 18 = 27 bytes
// ================================

#define HTTP2_SETTINGS_BIN       \
   "\x00\x00\x12"                \
   "\x04"                        \
   "\x00"                        \
   "\x00\x00\x00\x00"            \
   "\x00\x03" "\x00\x00\x00\x64" \
   "\x00\x04" "\x7f\xff\xff\xfe" \
   "\x00\x05" "\x00\xff\xff\xff"

#define HTTP2_CONNECTION_UPGRADE_AND_SETTING_BIN \
   "HTTP/1.1 101 Switching Protocols\r\n"        \
   "Connection: Upgrade\r\n"                     \
   "Upgrade: h2c\r\n\r\n"                        \
   "\x00\x00\x12"                                \
   "\x04"                                        \
   "\x00"                                        \
   "\x00\x00\x00\x00"                            \
   "\x00\x03" "\x00\x00\x00\x64"                 \
   "\x00\x04" "\x7f\xff\xff\xfe"                 \
   "\x00\x05" "\x00\xff\xff\xff"

// ================================
//       SETTINGS ACK FRAME
// ================================
// frame size = 0
// settings frame
// flags ACK
// stream id = 0
// ================================
// (9 bytes)
// ================================
#define HTTP2_SETTINGS_ACK \
   "\x00\x00\x00"          \
   "\x04"                  \
   "\x01"                  \
   "\x00\x00\x00\x00"

const UHTTP2::Settings UHTTP2::settings = {
   /* header_table_size = */ 4096,
   /* enable_push = */ 1,
   /* max_concurrent_streams = */ 100,
   /* initial_window_size = */ 2147483646, // (2e31-1) - disables flow control for that receiver
   /* max_frame_size = */ 16777215,
   /* max_header_list_size */ UINT32_MAX
};

UString* UHTTP2::str_authority;
UString* UHTTP2::str_method;
UString* UHTTP2::str_method_get;
UString* UHTTP2::str_method_post;
UString* UHTTP2::str_path;
UString* UHTTP2::str_path_root;
UString* UHTTP2::str_path_index;
UString* UHTTP2::str_scheme;
UString* UHTTP2::str_scheme_http;
UString* UHTTP2::str_scheme_https;
UString* UHTTP2::str_status;
UString* UHTTP2::str_status_200;
UString* UHTTP2::str_status_204;
UString* UHTTP2::str_status_206;
UString* UHTTP2::str_status_304;
UString* UHTTP2::str_status_400;
UString* UHTTP2::str_status_404;
UString* UHTTP2::str_status_500;
UString* UHTTP2::str_accept_charset;
UString* UHTTP2::str_accept_encoding;
UString* UHTTP2::str_accept_encoding_value;
UString* UHTTP2::str_accept_language;
UString* UHTTP2::str_accept_ranges;
UString* UHTTP2::str_accept;
UString* UHTTP2::str_access_control_allow_origin;
UString* UHTTP2::str_age;
UString* UHTTP2::str_allow;
UString* UHTTP2::str_authorization;
UString* UHTTP2::str_cache_control;
UString* UHTTP2::str_content_disposition;
UString* UHTTP2::str_content_encoding;
UString* UHTTP2::str_content_language;
UString* UHTTP2::str_content_length;
UString* UHTTP2::str_content_location;
UString* UHTTP2::str_content_range;
UString* UHTTP2::str_content_type;
UString* UHTTP2::str_cookie;
UString* UHTTP2::str_date;
UString* UHTTP2::str_etag;
UString* UHTTP2::str_expect;
UString* UHTTP2::str_expires;
UString* UHTTP2::str_from;
UString* UHTTP2::str_host;
UString* UHTTP2::str_if_match;
UString* UHTTP2::str_if_modified_since;
UString* UHTTP2::str_if_none_match;
UString* UHTTP2::str_if_range;
UString* UHTTP2::str_if_unmodified_since;
UString* UHTTP2::str_last_modified;
UString* UHTTP2::str_link;
UString* UHTTP2::str_location;
UString* UHTTP2::str_max_forwards;
UString* UHTTP2::str_proxy_authenticate;
UString* UHTTP2::str_proxy_authorization;
UString* UHTTP2::str_range;
UString* UHTTP2::str_referer;
UString* UHTTP2::str_refresh;
UString* UHTTP2::str_retry_after;
UString* UHTTP2::str_server;
UString* UHTTP2::str_set_cookie;
UString* UHTTP2::str_strict_transport_security;
UString* UHTTP2::str_transfer_encoding;
UString* UHTTP2::str_user_agent;
UString* UHTTP2::str_vary;
UString* UHTTP2::str_via;
UString* UHTTP2::str_www_authenticate;

void UHTTP2::ctor()
{
   U_TRACE(0, "UHTTP2::ctor()")

   static ustringrep stringrep_storage[] = {
      { U_STRINGREP_FROM_CONSTANT(":authority") },
      { U_STRINGREP_FROM_CONSTANT(":method") },
      { U_STRINGREP_FROM_CONSTANT("GET") },
      { U_STRINGREP_FROM_CONSTANT("POST") },
      { U_STRINGREP_FROM_CONSTANT(":path") },
      { U_STRINGREP_FROM_CONSTANT("/") },
      { U_STRINGREP_FROM_CONSTANT("/index.html") },
      { U_STRINGREP_FROM_CONSTANT(":scheme") },
      { U_STRINGREP_FROM_CONSTANT("http") },
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
      { U_STRINGREP_FROM_CONSTANT("host") },
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
      { U_STRINGREP_FROM_CONSTANT("www-authenticate") }
   };

   U_NEW_ULIB_OBJECT(str_authority,                   U_STRING_FROM_STRINGREP_STORAGE(0));
   U_NEW_ULIB_OBJECT(str_method,                      U_STRING_FROM_STRINGREP_STORAGE(1));
   U_NEW_ULIB_OBJECT(str_method_get,                  U_STRING_FROM_STRINGREP_STORAGE(2));
   U_NEW_ULIB_OBJECT(str_method_post,                 U_STRING_FROM_STRINGREP_STORAGE(3));
   U_NEW_ULIB_OBJECT(str_path,                        U_STRING_FROM_STRINGREP_STORAGE(4));
   U_NEW_ULIB_OBJECT(str_path_root,                   U_STRING_FROM_STRINGREP_STORAGE(5));
   U_NEW_ULIB_OBJECT(str_path_index,                  U_STRING_FROM_STRINGREP_STORAGE(6));
   U_NEW_ULIB_OBJECT(str_scheme,                      U_STRING_FROM_STRINGREP_STORAGE(7));
   U_NEW_ULIB_OBJECT(str_scheme_http,                 U_STRING_FROM_STRINGREP_STORAGE(8));
   U_NEW_ULIB_OBJECT(str_scheme_https,                U_STRING_FROM_STRINGREP_STORAGE(9));
   U_NEW_ULIB_OBJECT(str_status,                      U_STRING_FROM_STRINGREP_STORAGE(10));
   U_NEW_ULIB_OBJECT(str_status_200,                  U_STRING_FROM_STRINGREP_STORAGE(11));
   U_NEW_ULIB_OBJECT(str_status_204,                  U_STRING_FROM_STRINGREP_STORAGE(12));
   U_NEW_ULIB_OBJECT(str_status_206,                  U_STRING_FROM_STRINGREP_STORAGE(13));
   U_NEW_ULIB_OBJECT(str_status_304,                  U_STRING_FROM_STRINGREP_STORAGE(14));
   U_NEW_ULIB_OBJECT(str_status_400,                  U_STRING_FROM_STRINGREP_STORAGE(15));
   U_NEW_ULIB_OBJECT(str_status_404,                  U_STRING_FROM_STRINGREP_STORAGE(16));
   U_NEW_ULIB_OBJECT(str_status_500,                  U_STRING_FROM_STRINGREP_STORAGE(17));
   U_NEW_ULIB_OBJECT(str_accept_charset,              U_STRING_FROM_STRINGREP_STORAGE(18));
   U_NEW_ULIB_OBJECT(str_accept_encoding,             U_STRING_FROM_STRINGREP_STORAGE(19));
   U_NEW_ULIB_OBJECT(str_accept_encoding_value,       U_STRING_FROM_STRINGREP_STORAGE(20));
   U_NEW_ULIB_OBJECT(str_accept_language,             U_STRING_FROM_STRINGREP_STORAGE(21));
   U_NEW_ULIB_OBJECT(str_accept_ranges,               U_STRING_FROM_STRINGREP_STORAGE(22));
   U_NEW_ULIB_OBJECT(str_accept,                      U_STRING_FROM_STRINGREP_STORAGE(23));
   U_NEW_ULIB_OBJECT(str_access_control_allow_origin, U_STRING_FROM_STRINGREP_STORAGE(24));
   U_NEW_ULIB_OBJECT(str_age,                         U_STRING_FROM_STRINGREP_STORAGE(25));
   U_NEW_ULIB_OBJECT(str_allow,                       U_STRING_FROM_STRINGREP_STORAGE(26));
   U_NEW_ULIB_OBJECT(str_authorization,               U_STRING_FROM_STRINGREP_STORAGE(27));
   U_NEW_ULIB_OBJECT(str_cache_control,               U_STRING_FROM_STRINGREP_STORAGE(28));
   U_NEW_ULIB_OBJECT(str_content_disposition,         U_STRING_FROM_STRINGREP_STORAGE(29));
   U_NEW_ULIB_OBJECT(str_content_encoding,            U_STRING_FROM_STRINGREP_STORAGE(30));
   U_NEW_ULIB_OBJECT(str_content_language,            U_STRING_FROM_STRINGREP_STORAGE(31));
   U_NEW_ULIB_OBJECT(str_content_length,              U_STRING_FROM_STRINGREP_STORAGE(32));
   U_NEW_ULIB_OBJECT(str_content_location,            U_STRING_FROM_STRINGREP_STORAGE(33));
   U_NEW_ULIB_OBJECT(str_content_range,               U_STRING_FROM_STRINGREP_STORAGE(34));
   U_NEW_ULIB_OBJECT(str_content_type,                U_STRING_FROM_STRINGREP_STORAGE(35));
   U_NEW_ULIB_OBJECT(str_cookie,                      U_STRING_FROM_STRINGREP_STORAGE(36));
   U_NEW_ULIB_OBJECT(str_date,                        U_STRING_FROM_STRINGREP_STORAGE(37));
   U_NEW_ULIB_OBJECT(str_etag,                        U_STRING_FROM_STRINGREP_STORAGE(38));
   U_NEW_ULIB_OBJECT(str_expect,                      U_STRING_FROM_STRINGREP_STORAGE(39));
   U_NEW_ULIB_OBJECT(str_expires,                     U_STRING_FROM_STRINGREP_STORAGE(40));
   U_NEW_ULIB_OBJECT(str_from,                        U_STRING_FROM_STRINGREP_STORAGE(41));
   U_NEW_ULIB_OBJECT(str_host,                        U_STRING_FROM_STRINGREP_STORAGE(42));
   U_NEW_ULIB_OBJECT(str_if_match,                    U_STRING_FROM_STRINGREP_STORAGE(43));
   U_NEW_ULIB_OBJECT(str_if_modified_since,           U_STRING_FROM_STRINGREP_STORAGE(44));
   U_NEW_ULIB_OBJECT(str_if_none_match,               U_STRING_FROM_STRINGREP_STORAGE(45));
   U_NEW_ULIB_OBJECT(str_if_range,                    U_STRING_FROM_STRINGREP_STORAGE(46));
   U_NEW_ULIB_OBJECT(str_if_unmodified_since,         U_STRING_FROM_STRINGREP_STORAGE(47));
   U_NEW_ULIB_OBJECT(str_last_modified,               U_STRING_FROM_STRINGREP_STORAGE(48));
   U_NEW_ULIB_OBJECT(str_link,                        U_STRING_FROM_STRINGREP_STORAGE(49));
   U_NEW_ULIB_OBJECT(str_location,                    U_STRING_FROM_STRINGREP_STORAGE(50));
   U_NEW_ULIB_OBJECT(str_max_forwards,                U_STRING_FROM_STRINGREP_STORAGE(51));
   U_NEW_ULIB_OBJECT(str_proxy_authenticate,          U_STRING_FROM_STRINGREP_STORAGE(52));
   U_NEW_ULIB_OBJECT(str_proxy_authorization,         U_STRING_FROM_STRINGREP_STORAGE(53));
   U_NEW_ULIB_OBJECT(str_range,                       U_STRING_FROM_STRINGREP_STORAGE(54));
   U_NEW_ULIB_OBJECT(str_referer,                     U_STRING_FROM_STRINGREP_STORAGE(55));
   U_NEW_ULIB_OBJECT(str_refresh,                     U_STRING_FROM_STRINGREP_STORAGE(56));
   U_NEW_ULIB_OBJECT(str_retry_after,                 U_STRING_FROM_STRINGREP_STORAGE(57));
   U_NEW_ULIB_OBJECT(str_server,                      U_STRING_FROM_STRINGREP_STORAGE(58));
   U_NEW_ULIB_OBJECT(str_set_cookie,                  U_STRING_FROM_STRINGREP_STORAGE(59));
   U_NEW_ULIB_OBJECT(str_strict_transport_security,   U_STRING_FROM_STRINGREP_STORAGE(60));
   U_NEW_ULIB_OBJECT(str_transfer_encoding,           U_STRING_FROM_STRINGREP_STORAGE(61));
   U_NEW_ULIB_OBJECT(str_user_agent,                  U_STRING_FROM_STRINGREP_STORAGE(62));
   U_NEW_ULIB_OBJECT(str_vary,                        U_STRING_FROM_STRINGREP_STORAGE(63));
   U_NEW_ULIB_OBJECT(str_via,                         U_STRING_FROM_STRINGREP_STORAGE(64));
   U_NEW_ULIB_OBJECT(str_www_authenticate,            U_STRING_FROM_STRINGREP_STORAGE(65));

   hpack_static_table[0].name   = str_authority;
    hash_static_table[0]        = str_authority->hash();
   hpack_static_table[1].name   = str_method;
    hash_static_table[1]        = str_method->hash();
   hpack_static_table[1].value  = str_method_get;
   hpack_static_table[2].name   = str_method;
   hpack_static_table[2].value  = str_method_post;
   hpack_static_table[3].name   = str_path;
    hash_static_table[3]        = str_path->hash();
   hpack_static_table[3].value  = str_path_root;
   hpack_static_table[4].name   = str_path;
   hpack_static_table[4].value  = str_path_index;
   hpack_static_table[5].name   = str_scheme;
    hash_static_table[5]        = str_scheme->hash();
   hpack_static_table[5].value  = str_scheme_http;
   hpack_static_table[6].name   = str_scheme;
   hpack_static_table[6].value  = str_scheme_https;
   hpack_static_table[7].name   = str_status;
    hash_static_table[7]        = str_status->hash();
   hpack_static_table[7].value  = str_status_200;
   hpack_static_table[8].name   = str_status;
   hpack_static_table[8].value  = str_status_204;
   hpack_static_table[9].name   = str_status;
   hpack_static_table[9].value  = str_status_206;
   hpack_static_table[10].name  = str_status;
   hpack_static_table[10].value = str_status_304;
   hpack_static_table[11].name  = str_status;
   hpack_static_table[11].value = str_status_400;
   hpack_static_table[12].name  = str_status;
   hpack_static_table[12].value = str_status_404;
   hpack_static_table[13].name  = str_status;
   hpack_static_table[13].value = str_status_500;
   hpack_static_table[14].name  = str_accept_charset;
    hash_static_table[14]       = str_accept_charset->hash();
   hpack_static_table[15].name  = str_accept_encoding;
    hash_static_table[15]       = str_accept_encoding->hash();
   hpack_static_table[15].value = str_accept_encoding_value;
   hpack_static_table[16].name  = str_accept_language;
    hash_static_table[16]       = str_accept_language->hash();
   hpack_static_table[17].name  = str_accept_ranges;
    hash_static_table[17]       = str_accept_ranges->hash();
   hpack_static_table[18].name  = str_accept;
    hash_static_table[18]       = str_accept->hash();
   hpack_static_table[19].name  = str_access_control_allow_origin;
    hash_static_table[19]       = str_access_control_allow_origin->hash();
   hpack_static_table[20].name  = str_age;
    hash_static_table[20]       = str_age->hash();
   hpack_static_table[21].name  = str_allow;
    hash_static_table[21]       = str_allow->hash();
   hpack_static_table[22].name  = str_authorization;
    hash_static_table[22]       = str_authorization->hash();
   hpack_static_table[23].name  = str_cache_control;
    hash_static_table[23]       = str_cache_control->hash();
   hpack_static_table[24].name  = str_content_disposition;
    hash_static_table[24]       = str_content_disposition->hash();
   hpack_static_table[25].name  = str_content_encoding;
    hash_static_table[25]       = str_content_encoding->hash();
   hpack_static_table[26].name  = str_content_language;
    hash_static_table[26]       = str_content_language->hash();
   hpack_static_table[27].name  = str_content_length;
    hash_static_table[27]       = str_content_length->hash();
   hpack_static_table[28].name  = str_content_location;
    hash_static_table[28]       = str_content_location->hash();
   hpack_static_table[29].name  = str_content_range;
    hash_static_table[29]       = str_content_range->hash();
   hpack_static_table[30].name  = str_content_type;
    hash_static_table[30]       = str_content_type->hash();
   hpack_static_table[31].name  = str_cookie;
    hash_static_table[31]       = str_cookie->hash();
   hpack_static_table[32].name  = str_date;
    hash_static_table[32]       = str_date->hash();
   hpack_static_table[33].name  = str_etag;
    hash_static_table[33]       = str_etag->hash();
   hpack_static_table[34].name  = str_expect;
    hash_static_table[34]       = str_expect->hash();
   hpack_static_table[35].name  = str_expires;
    hash_static_table[35]       = str_expires->hash();
   hpack_static_table[36].name  = str_from;
    hash_static_table[36]       = str_from->hash();
   hpack_static_table[37].name  = str_host;
    hash_static_table[37]       = str_host->hash();
   hpack_static_table[38].name  = str_if_match;
    hash_static_table[38]       = str_if_match->hash();
   hpack_static_table[39].name  = str_if_modified_since;
    hash_static_table[39]       = str_if_modified_since->hash();
   hpack_static_table[40].name  = str_if_none_match;
    hash_static_table[40]       = str_if_none_match->hash();
   hpack_static_table[41].name  = str_if_range;
    hash_static_table[41]       = str_if_range->hash();
   hpack_static_table[42].name  = str_if_unmodified_since;
    hash_static_table[42]       = str_if_unmodified_since->hash();
   hpack_static_table[43].name  = str_last_modified;
    hash_static_table[43]       = str_last_modified->hash();
   hpack_static_table[44].name  = str_link;
    hash_static_table[44]       = str_link->hash();
   hpack_static_table[45].name  = str_location;
    hash_static_table[45]       = str_location->hash();
   hpack_static_table[46].name  = str_max_forwards;
    hash_static_table[46]       = str_max_forwards->hash();
   hpack_static_table[47].name  = str_proxy_authenticate;
    hash_static_table[47]       = str_proxy_authenticate->hash();
   hpack_static_table[48].name  = str_proxy_authorization;
    hash_static_table[48]       = str_proxy_authorization->hash();
   hpack_static_table[49].name  = str_range;
    hash_static_table[49]       = str_range->hash();
   hpack_static_table[50].name  = str_referer;
    hash_static_table[50]       = str_referer->hash();
   hpack_static_table[51].name  = str_refresh;
    hash_static_table[51]       = str_refresh->hash();
   hpack_static_table[52].name  = str_retry_after;
    hash_static_table[52]       = str_retry_after->hash();
   hpack_static_table[53].name  = str_server;
    hash_static_table[53]       = str_server->hash();
   hpack_static_table[54].name  = str_set_cookie;
    hash_static_table[54]       = str_set_cookie->hash();
   hpack_static_table[55].name  = str_strict_transport_security;
    hash_static_table[55]       = str_strict_transport_security->hash();
   hpack_static_table[56].name  = str_transfer_encoding;
    hash_static_table[56]       = str_transfer_encoding->hash();
   hpack_static_table[57].name  = str_user_agent;
    hash_static_table[57]       = str_user_agent->hash();
   hpack_static_table[58].name  = str_vary;
    hash_static_table[58]       = str_vary->hash();
   hpack_static_table[59].name  = str_via;
    hash_static_table[59]       = str_via->hash();
   hpack_static_table[60].name  = str_www_authenticate;
    hash_static_table[60]       = str_www_authenticate->hash();
}

void UHTTP2::dtor()
{
   U_TRACE(0, "UHTTP2::dtor()")
}

void UHTTP2::sendError()
{
   U_TRACE(0, "UHTTP2::sendError()")

   U_INTERNAL_ASSERT_DIFFERS(nerror, NO_ERROR)

   char buffer[HTTP2_FRAME_HEADER_SIZE+8] = { 0, 0, 4,    // frame size
                                              RST_STREAM, // header frame
                                              0,          // end header flags
                                              0, 0, 0, 0, // stream id
                                              0, 0, 0, 0,
                                              0, 0, 0, 0 };
   char* ptr = buffer;

   if (frame.stream_id)
      {
      *(uint32_t*)(ptr+5) = htonl(frame.stream_id);
      *(uint32_t*)(ptr+9) = htonl(nerror);

      if (USocketExt::write(UServer_Base::csocket, buffer, HTTP2_FRAME_HEADER_SIZE+4, UServer_Base::timeoutMS) != HTTP2_FRAME_HEADER_SIZE+4) U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;
      }
   else
      {
      ptr[2] = 8;
      ptr[3] = GOAWAY;

      U_INTERNAL_ASSERT_POINTER(pConnection)

      pConnection->state = CONN_STATE_IS_CLOSING;

      *(uint32_t*)(ptr+ 5) = 0;
      *(uint32_t*)(ptr+ 9) = htonl(pConnection->max_processed_stream_id);
      *(uint32_t*)(ptr+13) = htonl(nerror);

      if (USocketExt::write(UServer_Base::csocket, buffer, HTTP2_FRAME_HEADER_SIZE+8, UServer_Base::timeoutMS) != HTTP2_FRAME_HEADER_SIZE+8) U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;
      }
}

bool UHTTP2::updateSetting(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHTTP2::updateSetting(%#.*S,%u)", len, ptr, len)

   U_INTERNAL_ASSERT_POINTER(pConnection)

   for (; len >= 6; len -= 6, ptr += 6)
      {
      uint32_t value = ntohl(*(uint32_t*)(ptr+2));

      switch (ntohs(*(uint16_t*)ptr))
         {
         case HEADER_TABLE_SIZE:      pConnection->peer_settings.header_table_size      = value; break;
         case ENABLE_PUSH:            pConnection->peer_settings.enable_push            = value; break;
         case MAX_CONCURRENT_STREAMS: pConnection->peer_settings.max_concurrent_streams = value; break;
         case INITIAL_WINDOW_SIZE:    pConnection->peer_settings.initial_window_size    = value; break;
         case MAX_FRAME_SIZE:         pConnection->peer_settings.max_frame_size         = value; break;
         case MAX_HEADER_LIST_SIZE:   pConnection->peer_settings.max_header_list_size   = value; break;

         default: break; // ignore unknown (5.5)
         }
      }

   U_INTERNAL_DUMP("header_table_size = %d enable_push = %d max_concurrent_streams = %d initial_window_size = %d max_frame_size = %d max_header_list_size = %d",
                     pConnection->peer_settings.header_table_size,   pConnection->peer_settings.enable_push,    pConnection->peer_settings.max_concurrent_streams,
                     pConnection->peer_settings.initial_window_size, pConnection->peer_settings.max_frame_size, pConnection->peer_settings.max_header_list_size)

   if (len == 0) U_RETURN(true);

   U_RETURN(false);
}

unsigned char* UHTTP2::hpackEncodeInt(unsigned char* dst, uint32_t value, uint8_t prefix_max)
{
   U_TRACE(0, "UHTTP2::hpackEncodeInt(%p,%u,%u)", dst, value, prefix_max)

   if (value <= prefix_max) *dst++ |= value;
   else
      {
      value -= prefix_max;

      U_INTERNAL_ASSERT(value <= 0x0fffffff)

      for (*dst++ |= value; value >= 256; value >>= 8) *dst++ = 0x80 | value;

      *dst++ = value;
      }

   return dst;
}

uint32_t UHTTP2::hpackDecodeInt(const unsigned char* src, const unsigned char* src_end, int32_t* pvalue, uint8_t prefix_max)
{
   U_TRACE(0, "UHTTP2::hpackDecodeInt(%p,%p,%p,%u)", src, src_end, pvalue, prefix_max)

   *pvalue = (uint8_t)*src++ & prefix_max;

   if (*pvalue != prefix_max) U_RETURN(1);

   // we only allow at most 4 octets (excluding prefix) to be used as int (== 2**(4*7) == 2**28)

   if ((src_end - src) > 4)
        src_end = src  + 4;

   *pvalue = prefix_max;

   for (int32_t mult = 1; src < src_end; mult *= 128)
      {
      *pvalue += (*src & 127) * mult;

      if ((*src++ & 128) == 0) goto end;
      }

   *pvalue = -1;

end:
   U_RETURN(1 + 4-(src_end-src));
}

uint32_t UHTTP2::hpackEncodeString(unsigned char* dst, const char* src, uint32_t len)
{
   U_TRACE(0+256, "UHTTP2::hpackEncodeString(%p,%.*S,%u)", dst, len, src, len)

   U_INTERNAL_ASSERT_MAJOR(len, 0)

   uint64_t bits = 0;
   int bits_left = 40;
   unsigned char* ptr;
   const char* src_end = src + len;

   UString buffer(len + 1024U);

   unsigned char* _dst       = (unsigned char*)buffer.data();
   unsigned char* _dst_end   = _dst + len;
   unsigned char* _dst_start = _dst;

   do {
      const HuffSym* sym = huff_sym_table + *src++;

      bits |= (uint64_t)sym->code << (bits_left - sym->nbits);

      bits_left -= sym->nbits;

      while (bits_left <= 32)
         {
         *_dst++ = bits >> 32;

         bits <<= 8;
         bits_left += 8;
         }
      }
   while (src < src_end);

   if (bits_left != 40)
      {
      bits |= (1UL << bits_left) - 1;

      *_dst++ = bits >> 32;
      }

   if (_dst >= _dst_end) // encode as-is
      {
      *dst = '\0';
       ptr = hpackEncodeInt(dst, len, (1<<7)-1);

      u__memcpy(ptr, src, len, __PRETTY_FUNCTION__);

      U_RETURN(ptr - dst + len);
      }

    len = _dst - _dst_start;
   *dst = '\x80';
    ptr = hpackEncodeInt(dst, len, (1<<7)-1);

   u__memcpy(ptr, _dst, len, __PRETTY_FUNCTION__);

   U_RETURN(ptr - dst + len);
}

uint32_t UHTTP2::hpackDecodeString(const unsigned char* src, const unsigned char* src_end, UString* pvalue)
{
   U_TRACE(0+256, "UHTTP2::hpackDecodeString(%p,%p,%p)", src, src_end, pvalue)

   int32_t len;
   bool is_huffman = ((*src & 0x80) != 0);
   uint32_t nmove = hpackDecodeInt(src, src_end, &len, (1<<7)-1);

   if (len <= 0)
      {
err:  pvalue->clear();

      U_RETURN(nmove);
      }

   src += nmove;

   U_INTERNAL_DUMP("is_huffman = %b", is_huffman)

   if (is_huffman == false)
      {
      if ((src + len) > src_end) goto err;

      (void) pvalue->replace((const char*)src, len);
      }
   else     
      {
      uint8_t state = 0;
      bool maybe_eos = true;
      UString result(len * 2); // max compression ratio is >= 0.5
      char* dst = result.data();
      const UHTTP2::HuffDecode* entry;

      for (src_end = src + len; src < src_end; ++src)
         {
         entry = huff_decode_table[state] + (*src >> 4);

         if ((entry->flags & HUFF_FAIL) != 0) goto err;
         if ((entry->flags & HUFF_SYM)  != 0) *dst++ = entry->sym;

         entry = huff_decode_table[entry->state] + (*src & 0x0f);

         if ((entry->flags & HUFF_FAIL) != 0) goto err;
         if ((entry->flags & HUFF_SYM)  != 0) *dst++ = entry->sym;

         state     =  entry->state;
         maybe_eos = (entry->flags & HUFF_ACCEPTED) != 0;
         }

      if (maybe_eos) result.size_adjust(dst);

      *pvalue = result;
      }

   U_RETURN(nmove + len);
}

void UHTTP2::decodeHeaders(const char* ptr, const char* endptr)
{
   U_TRACE(0, "UHTTP2::decodeHeaders(%p,%p)", ptr, endptr)

   uint32_t sz;
   int32_t index;
   unsigned char c;
   const char* ptr1;
   UString name, value;
   UHTTP2::HpackHeaderTableEntry* entry;
   bool value_is_indexed, continue100 = false;
   UHashMap<UString>& ptable = pConnection->itable;

   while (ptr < endptr)
      {
      value_is_indexed = ((c = *(unsigned char*)ptr) >= 128);

      U_INTERNAL_DUMP("c = %u value_is_indexed = %b", c, value_is_indexed)

      if (value_is_indexed) // indexed header field representation
         {
         ptr += hpackDecodeInt((const unsigned char*)ptr, (const unsigned char*)endptr, &index, (1<<7)-1);
         }
      else if (c >= 64) // literal header field with incremental handling
         {
         ptr += (c == 64 ? (index = 0, 1) : hpackDecodeInt((const unsigned char*)ptr, (const unsigned char*)endptr, &index, (1<<6)-1));
         }
      else if (c < 32) // literal header field without indexing or never indexed
         {
         ptr += ((c & 0x0f) == 0 ? (index = 0, 1) : hpackDecodeInt((const unsigned char*)ptr, (const unsigned char*)endptr, &index, (1<<4)-1));
         }
      else // size update
         {
         ptr += hpackDecodeInt((const unsigned char*)ptr, (const unsigned char*)endptr, &index, (1<<5)-1);

         if (index > pConnection->hpack_max_capacity) goto error;

         pConnection->hpack_max_capacity = index;

         continue;
         }

      U_INTERNAL_DUMP("index = %d", index)

      if (index <= 0)
         {
         if (index == 0) // non-existing name
            {
            ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &name);

            if (name)
               {
               ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

               if (value)
                  {
                  // add the decoded header to the header table

                  ptable.hash = name.hash();

                  goto insert;
                  }
               }
            }

error:   nerror = COMPRESSION_ERROR;

         return;
         }

      // determine the header

      U_INTERNAL_ASSERT_MINOR(index, HTTP2_HEADER_TABLE_OFFSET) // existing name (and value?)

      static const int dispatch_table[HTTP2_HEADER_TABLE_OFFSET] = {
         0,/* 0 */
         (int)((char*)&&case_1-(char*)&&cdefault),   /* 1 :authority */
         (int)((char*)&&case_2_3-(char*)&&cdefault), /* 2 :method */
         (int)((char*)&&case_2_3-(char*)&&cdefault), /* 3 :method */
         (int)((char*)&&case_4_5-(char*)&&cdefault), /* 4 :path */
         (int)((char*)&&case_4_5-(char*)&&cdefault), /* 5 :path */
         (int)((char*)&&case_6-(char*)&&cdefault),   /* 6 :scheme */
         (int)((char*)&&case_7-(char*)&&cdefault),   /* 7 :scheme */
         0,/*  8 :status 200 */
         0,/*  9 :status 204 */
         0,/* 10 :status 206 */
         0,/* 11 :status 304 */
         0,/* 12 :status 400 */
         0,/* 13 :status 404 */
         0,/* 14 :status 500 */
         0,/* 15 accept-charset */
         (int)((char*)&&case_16-(char*)&&cdefault), /* 16 accept-encoding */
         (int)((char*)&&case_17-(char*)&&cdefault), /* 17 accept-language */
         0,/* 18 accept-ranges */
         (int)((char*)&&case_19-(char*)&&cdefault), /* 19 accept */
         0,/* 20 access-control-allow-origin */
         0,/* 21 age */
         0,/* 22 allow */
         0,/* 23 authorization */
         0,/* 24 cache-control */
         0,/* 25 content-disposition */
         0,/* 26 content-encoding */
         0,/* 27 content-language */
         (int)((char*)&&case_28-(char*)&&cdefault), /* 28 content-length */
         0,/* 29 content-location */
         0,/* 30 content-range */
         (int)((char*)&&case_31-(char*)&&cdefault), /* 31 content-type */
         (int)((char*)&&case_32-(char*)&&cdefault), /* 32 cookie */
         0,/* 33 date */
         0,/* 34 etag */
         (int)((char*)&&case_35-(char*)&&cdefault), /* 35 expect */
         0,/* 36 expires */
         0,/* 37 from */
         (int)((char*)&&case_38-(char*)&&cdefault), /* 38 host */
         0,/* 39 if-match */
         (int)((char*)&&case_40-(char*)&&cdefault), /* 40 if-modified-since */
         0,/* 41 if-none-match */
         0,/* 42 if-range */
         0,/* 43 if-unmodified-since */
         0,/* 44 last-modified */
         0,/* 45 link*/
         0,/* 46 location */
         0,/* 47 max-forwards*/
         0,/* 48 proxy-authenticate */
         0,/* 49 proxy-authorization */
         (int)((char*)&&case_50-(char*)&&cdefault), /* 50 range */
         (int)((char*)&&case_51-(char*)&&cdefault), /* 51 referer */
         0,/* 52 refresh */
         0,/* 53 retry-after */
         0,/* 54 server */
         0,/* 55 set-cookie */
         0,/* 56 strict-transport-security */
         (int)((char*)&&case_57-(char*)&&cdefault), /* 57 transfer-encoding */
         (int)((char*)&&case_58-(char*)&&cdefault), /* 58 user-agent */
         0,/* 59 vary */
         0,/* 60 via */
         0 /* 61 www-authenticate */
      };

      U_INTERNAL_PRINT("dispatch_table[%d] = %p &&cdefault = %p", index, dispatch_table[index], &&cdefault)

      goto *((char*)&&cdefault + dispatch_table[index]);

case_38: // host

      name = *str_host;

      goto host;

case_1: // authority (a.k.a. the Host header)

      name = *str_authority;

host: U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

      sz = value.size();

      if (sz == 0) goto error;

      UHTTP::setHostname(value.data(), sz);

      ptable.hash = hash_static_table[37]; // host

      goto insert;

case_2_3: // GET - POST

      if (value_is_indexed) U_http_method_type = (index == 2 ? HTTP_GET : HTTP_POST);
      else
         {
         ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

         ptr1 = value.data();

         if (*ptr1 == '\0') goto error;

         switch (u_get_unalignedp32(ptr1))
            {
            case U_MULTICHAR_CONSTANT32('g','e','t','\0'):
            case U_MULTICHAR_CONSTANT32('G','E','T','\0'): U_http_method_type = HTTP_GET;                                 break;
            case U_MULTICHAR_CONSTANT32('h','e','a','d'):
            case U_MULTICHAR_CONSTANT32('H','E','A','D'):  U_http_method_type = HTTP_HEAD;        U_http_method_num =  1; break;
            case U_MULTICHAR_CONSTANT32('p','o','s','t'):
            case U_MULTICHAR_CONSTANT32('P','O','S','T'):  U_http_method_type = HTTP_POST;        U_http_method_num =  2; break;
            case U_MULTICHAR_CONSTANT32('p','u','t','\0'):
            case U_MULTICHAR_CONSTANT32('P','U','T','\0'): U_http_method_type = HTTP_PUT;         U_http_method_num =  3; break;
            case U_MULTICHAR_CONSTANT32('d','e','l','e'):
            case U_MULTICHAR_CONSTANT32('D','E','L','E'):  U_http_method_type = HTTP_DELETE;      U_http_method_num =  4; break;
            case U_MULTICHAR_CONSTANT32('o','p','t','i'):
            case U_MULTICHAR_CONSTANT32('O','P','T','I'):  U_http_method_type = HTTP_OPTIONS;     U_http_method_num =  5; break;
            // NOT IMPLEMENTED
            case U_MULTICHAR_CONSTANT32('T','R','A','C'):  U_http_method_type = HTTP_TRACE;       U_http_method_num =  6; break;
            case U_MULTICHAR_CONSTANT32('C','O','N','N'):  U_http_method_type = HTTP_CONNECT;     U_http_method_num =  7; break;
            case U_MULTICHAR_CONSTANT32('C','O','P','Y'):  U_http_method_type = HTTP_COPY;        U_http_method_num =  8; break;
            case U_MULTICHAR_CONSTANT32('M','O','V','E'):  U_http_method_type = HTTP_MOVE;        U_http_method_num =  9; break;
            case U_MULTICHAR_CONSTANT32('L','O','C','K'):  U_http_method_type = HTTP_LOCK;        U_http_method_num = 10; break;
            case U_MULTICHAR_CONSTANT32('U','N','L','O'):  U_http_method_type = HTTP_UNLOCK;      U_http_method_num = 11; break;
            case U_MULTICHAR_CONSTANT32('M','K','C','O'):  U_http_method_type = HTTP_MKCOL;       U_http_method_num = 12; break;
            case U_MULTICHAR_CONSTANT32('S','E','A','R'):  U_http_method_type = HTTP_SEARCH;      U_http_method_num = 13; break;
            case U_MULTICHAR_CONSTANT32('P','R','O','P'):  U_http_method_type = HTTP_PROPFIND;    U_http_method_num = 14; break;
            case U_MULTICHAR_CONSTANT32('P','A','T','C'):  U_http_method_type = HTTP_PATCH;       U_http_method_num = 16; break;
            case U_MULTICHAR_CONSTANT32('P','U','R','G'):  U_http_method_type = HTTP_PURGE;       U_http_method_num = 17; break;
            case U_MULTICHAR_CONSTANT32('M','E','R','G'):  U_http_method_type = HTTP_MERGE;       U_http_method_num = 18; break;
            case U_MULTICHAR_CONSTANT32('R','E','P','O'):  U_http_method_type = HTTP_REPORT;      U_http_method_num = 19; break;
            case U_MULTICHAR_CONSTANT32('C','H','E','C'):  U_http_method_type = HTTP_CHECKOUT;    U_http_method_num = 20; break;
            case U_MULTICHAR_CONSTANT32('M','K','A','C'):  U_http_method_type = HTTP_MKACTIVITY;  U_http_method_num = 21; break;
            case U_MULTICHAR_CONSTANT32('N','O','T','I'):  U_http_method_type = HTTP_NOTIFY;      U_http_method_num = 22; break;
            case U_MULTICHAR_CONSTANT32('M','S','E','A'):  U_http_method_type = HTTP_MSEARCH;     U_http_method_num = 23; break;
            case U_MULTICHAR_CONSTANT32('S','U','B','S'):  U_http_method_type = HTTP_SUBSCRIBE;   U_http_method_num = 24; break;
            case U_MULTICHAR_CONSTANT32('U','N','S','U'):  U_http_method_type = HTTP_UNSUBSCRIBE; U_http_method_num = 25; break;

            default: goto error;
            }
         }

      continue;

case_4_5: // / - /index.html

      name = *str_path;

      ptable.hash = hash_static_table[3]; // path

      // determine the value (if necessary)

      if (value_is_indexed) value = *(index == 4 ? str_path_root : str_path_index);
      else
         {
         ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

         sz = value.size();

         if (sz == 0) goto error;

         U_http_info.query = (const char*) memchr((U_http_info.uri = value.data()), '?', (U_http_info.uri_len = sz));

         if (U_http_info.query)
            {
            U_http_info.query_len = U_http_info.uri_len - (U_http_info.query++ - U_http_info.uri);

            U_http_info.uri_len -= U_http_info.query_len;

            U_INTERNAL_DUMP("query = %.*S", U_HTTP_QUERY_TO_TRACE)
            }

         U_INTERNAL_DUMP("URI = %.*S", U_HTTP_URI_TO_TRACE)
         }

      goto insert;

case_6: // http

      U_INTERNAL_ASSERT(value_is_indexed)
      U_ASSERT_EQUALS(UServer_Base::csocket->isSSLActive(), false)

      continue;

case_7: // https

      U_INTERNAL_ASSERT(value_is_indexed)
      U_ASSERT(((USSLSocket*)UServer_Base::csocket)->isSSL())

      continue;

case_16: // accept-encoding: gzip, deflate

      name = *str_accept_encoding;

      U_INTERNAL_ASSERT(value_is_indexed)

      value = *str_accept_encoding_value;

      U_http_is_accept_gzip = '1';

      U_INTERNAL_DUMP("U_http_is_accept_gzip = %C", U_http_is_accept_gzip)

      ptable.hash = hash_static_table[15]; // accept_encoding

      goto insert;

case_17: // accept-language

      name = *str_accept_language;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

      U_http_accept_language_len = value.size();

      if (U_http_accept_language_len == 0) goto error;

      U_http_info.accept_language = value.data();

      U_INTERNAL_DUMP("Accept-Language: = %.*S", U_HTTP_ACCEPT_LANGUAGE_TO_TRACE)

      ptable.hash = hash_static_table[16]; // accept_language

      goto insert;

case_19: // accept

      name = *str_accept;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

      U_http_accept_len = value.size();

      if (U_http_accept_len == 0) goto error;

      U_http_info.accept = value.data();

      U_INTERNAL_DUMP("Accept: = %.*S", U_HTTP_ACCEPT_TO_TRACE)

      ptable.hash = hash_static_table[18]; // accept

      goto insert;

case_28: // content_length

      name = *str_content_length;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

      if (value.empty()) goto error;

      U_http_info.clength = (uint32_t) strtoul(value.data(), 0, 0);

      U_INTERNAL_DUMP("Content-Length: = %.*S U_http_info.clength = %u", U_STRING_TO_TRACE(value), U_http_info.clength)

      ptable.hash = hash_static_table[27]; // content_length 

      goto insert;

case_31: // content_type

      name = *str_content_type;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

      U_http_content_type_len = value.size();

      if (U_http_content_type_len == 0) goto error;

      U_http_info.content_type = value.data();

      U_INTERNAL_DUMP("Content-Type(%u): = %.*S", U_http_content_type_len, U_HTTP_CTYPE_TO_TRACE)

      ptable.hash = hash_static_table[30]; // content_type 

      goto insert;

case_32: // cookie

      name = *str_cookie;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

      U_http_info.cookie_len = value.size();

      if (U_http_info.cookie_len == 0) goto error;

      U_http_info.cookie = value.data();

      U_INTERNAL_DUMP("Cookie(%u): = %.*S", U_http_info.cookie_len, U_HTTP_COOKIE_TO_TRACE)

      ptable.hash = hash_static_table[31]; // cookie 

      goto insert;

case_35: // expect

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

      if (value.empty()) goto error;

      // NB: check for 'Expect: 100-continue' (as curl does)...

      continue100 = value.equal(U_CONSTANT_TO_PARAM("100-continue"));

      if (continue100 == false)
         {
         name = *str_expect;

         ptable.hash = hash_static_table[34]; // expect 

         goto insert;
         }

      continue;

case_40: // if-modified-since

      name = *str_if_modified_since;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

      if (value.empty()) goto error;

      U_http_info.if_modified_since = UTimeDate::getSecondFromTime(value.data(), true);

      U_INTERNAL_DUMP("If-Modified-Since = %u", U_http_info.if_modified_since)

      ptable.hash = hash_static_table[39]; // if_modified_since 

      goto insert;

case_50: // range 

      name = *str_range;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

      U_http_range_len = value.size() - U_CONSTANT_SIZE("bytes=");

      if (U_http_range_len <= 0) goto error;

      U_http_info.range = value.data() + U_CONSTANT_SIZE("bytes=");

      U_INTERNAL_DUMP("Range = %.*S", U_HTTP_RANGE_TO_TRACE)

      ptable.hash = hash_static_table[49]; // range 

      goto insert;

case_51: // referer 

      name = *str_referer;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

      U_http_info.referer_len = value.size();

      if (U_http_info.referer_len == 0) goto error;

      U_http_info.referer = value.data();

      U_INTERNAL_DUMP("Referer(%u): = %.*S", U_http_info.referer_len, U_HTTP_REFERER_TO_TRACE)

      ptable.hash = hash_static_table[50]; // referer 

      goto insert;

case_57: // transfer-encoding

      U_SRV_LOG("WARNING: Transfer-Encoding is not supported in HTTP/2");

      goto error;

case_58: // user-agent

      name = *str_user_agent;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

      U_http_info.user_agent_len = value.size();

      if (U_http_info.user_agent_len == 0) goto error;

      U_http_info.user_agent = value.data();

      U_INTERNAL_DUMP("User-Agent: = %.*S", U_HTTP_USER_AGENT_TO_TRACE)

      ptable.hash = hash_static_table[57]; // user_agent

      goto insert;

cdefault:
      entry = hpack_static_table + --index;

      ptable.hash = hash_static_table[index];

                             name = *(entry->name);
      if (value_is_indexed) value = *(entry->value); // determine the value (if necessary)
      else  
         {
         ptr += hpackDecodeString((const unsigned char*)ptr, (const unsigned char*)endptr, &value);

         if (value.empty()) goto error;
         }

insert:
      ptable.insert(name, value); // add the decoded header to the header table

      U_INTERNAL_DUMP("name = %V value = %V", name.rep, value.rep)
      }

   U_INTERNAL_DUMP("U_http_method_type = %B U_http_method_num = %d", U_http_method_type, U_http_method_num)

   if (U_http_info.clength)
      {
      U_INTERNAL_DUMP("U_http_info.clength = %u UHTTP::limit_request_body = %u", U_http_info.clength, UHTTP::limit_request_body)

      if (U_http_info.clength > UHTTP::limit_request_body)
         {
         U_http_info.nResponseCode = HTTP_ENTITY_TOO_LARGE;

         UClientImage_Base::setCloseConnection();

         UHTTP::setResponse(0, 0);

         return;
         }

      char buffer[HTTP2_FRAME_HEADER_SIZE+5] = { 0, 0, 5,               // frame size
                                                 1,                     // header frame
                                                 4,                     // end header flags
                                                 0, 0, 0, 0,            // stream id
                                                 8, 3, '1', '0', '0' }; // use literal header field without indexing - indexed name

      *(uint32_t*)(buffer+5) = htonl(frame.stream_id);

      if (USocketExt::write(UServer_Base::csocket, buffer, sizeof(buffer), UServer_Base::timeoutMS) != sizeof(buffer))
         {
         nerror = FLOW_CONTROL_ERROR;

         return;
         }

      (void) UClientImage_Base::body->reserve(U_http_info.clength);
      }
}

void UHTTP2::readFrame()
{
   U_TRACE(0, "UHTTP2::readFrame()")

   U_INTERNAL_ASSERT_POINTER(pStream)

   uint32_t sz;
    int32_t len;
   const char* ptr;

loop:
   sz = UClientImage_Base::rbuffer->size();

   U_INTERNAL_DUMP("UClientImage_Base::rbuffer->size() = %u UClientImage_Base::rstart = %u", sz, UClientImage_Base::rstart)

   if (sz == UClientImage_Base::rstart)
      {
      UClientImage_Base::rstart = 0;

      UClientImage_Base::rbuffer->setEmpty();

      if (USocketExt::read(UServer_Base::csocket, *UClientImage_Base::rbuffer, U_SINGLE_READ, UServer_Base::timeoutMS, UHTTP::request_read_timeout) == false)
         {
         nerror = PROTOCOL_ERROR;

         return;
         }

      sz = UClientImage_Base::rbuffer->size();
      }
#ifdef DEBUG
   else
      {
      U_INTERNAL_ASSERT_MAJOR(sz-UClientImage_Base::rstart, HTTP2_FRAME_HEADER_SIZE)
      }
#endif

   ptr = UClientImage_Base::rbuffer->c_pointer(UClientImage_Base::rstart);

   U_INTERNAL_DUMP("ptr = %#.4S", ptr) // "\000\000\f\004" (big endian: 0x11223344)

#ifdef DEBUG
   if ((frame.type = ptr[3]) > CONTINUATION)
      {
      nerror = PROTOCOL_ERROR;

      return;
      }
#endif

   frame.length = ntohl(*(uint32_t*)ptr & 0x00ffffff) >> 8;

   U_INTERNAL_DUMP("frame.length = %#.4S", &frame.length)

   frame.flags     = ptr[4];
   frame.stream_id = ntohl(*(uint32_t*)(ptr+5) & 0x7fffffff);

   U_DUMP("frame { length = %d stream_id = %d type = (%d, %s) flags = %d } = %#.*S", frame.length,
                frame.stream_id, frame.type, getFrameTypeDescription(), frame.flags, frame.length, ptr + HTTP2_FRAME_HEADER_SIZE)

   U_INTERNAL_ASSERT_MINOR(frame.length, (int32_t)settings.max_frame_size)

   len = UClientImage_Base::rbuffer->size() - (UClientImage_Base::rstart += HTTP2_FRAME_HEADER_SIZE) - frame.length;

   U_INTERNAL_DUMP("UClientImage_Base::rbuffer->size() = %u UClientImage_Base::rstart = %u frame.length = %u len = %d", sz, UClientImage_Base::rstart, frame.length, len)

   if (len >= 0  ||
       USocketExt::read(UServer_Base::csocket, *UClientImage_Base::rbuffer, -len, UServer_Base::timeoutMS, UHTTP::request_read_timeout))
      {
      frame.payload = UClientImage_Base::rbuffer->c_pointer(UClientImage_Base::rstart);
                                                            UClientImage_Base::rstart += (uint32_t)frame.length;

      if (frame.type == WINDOW_UPDATE)
         {
         uint32_t window_size_increment = ntohl(*(uint32_t*)frame.payload) & 0x7fffffff;

         U_INTERNAL_DUMP("window_size_increment = %u", window_size_increment)

#     ifdef DEBUG
         if (frame.length != 4 ||
             window_size_increment == 0)
            {
            nerror = PROTOCOL_ERROR;

            goto err;
            }
#     endif

         if (frame.stream_id)     pStream->output_window += window_size_increment;
         else                 pConnection->output_window += window_size_increment;

         goto loop;
         }

      if (frame.stream_id == 0)
         {
         if (frame.type == SETTINGS)
            {
            if ((frame.flags & FLAG_ACK) != 0)
               {
#           ifdef DEBUG
               if (frame.length)
                  {
                  nerror = FRAME_SIZE_ERROR;

                  goto err;
                  }
#           endif

               settings_ack = true;
               }
            else
               {
               if (updateSetting(frame.payload, frame.length) == false ||
                   USocketExt::write(UServer_Base::csocket, U_CONSTANT_TO_PARAM(HTTP2_SETTINGS_ACK), UServer_Base::timeoutMS) != U_CONSTANT_SIZE(HTTP2_SETTINGS_ACK))
                  {
                  nerror = PROTOCOL_ERROR;

                  goto err;
                  }
               }

            goto loop;
            }
         }
      else
         {
         if (frame.type == PRIORITY)
            {
            // TODO

            goto loop;
            }

         if (frame.type == HEADERS)
            {
            // open the stream

            U_INTERNAL_ASSERT_POINTER(pStream)
            U_INTERNAL_ASSERT(pConnection->max_open_stream_id <= frame.stream_id)

                pStream->input_window  =
            pConnection->input_window  = settings.initial_window_size;
                pStream->output_window =
            pConnection->output_window = pConnection->peer_settings.initial_window_size;

            pStream->id                     =
            pConnection->max_open_stream_id = frame.stream_id;
            pConnection->hpack_max_capacity = settings.header_table_size;

            pStream->state = ((frame.flags & FLAG_END_STREAM) != 0 ? STREAM_STATE_HALF_CLOSED : STREAM_STATE_OPEN);

            manageHeaders();

            goto end;
            }

         if (pStream->id != frame.stream_id)
            {
            if (frame.stream_id <= pConnection->max_open_stream_id)
               {
               for (pStream = pConnection->streams; pStream < pConnectionEnd; ++pStream)
                  {
                  if (pStream->id == frame.stream_id) goto next;
                  }
               }

            nerror = FLOW_CONTROL_ERROR;

            goto err;
            }

next:    if ((frame.flags & FLAG_END_STREAM) != 0) pStream->state = STREAM_STATE_HALF_CLOSED;

         if (frame.type == DATA)
            {
            manageData();

            if (pStream->state != STREAM_STATE_HALF_CLOSED) goto loop; // we must wait for other DATA frames for the same stream...

            goto end;
            }

         if (frame.type == RST_STREAM)
            {
            U_INTERNAL_ASSERT_EQUALS(pStream->state, STREAM_STATE_HALF_CLOSED)

            pStream->state = STREAM_STATE_CLOSED;

            uint32_t error = ntohl(*(uint32_t*)frame.payload);

            U_INTERNAL_DUMP("error = %u", error)
            }
         }

end:  nerror = NO_ERROR;

      return;
      }

   nerror = ERROR_INCOMPLETE;

err:
   UClientImage_Base::rstart = 0;

   UClientImage_Base::rbuffer->setEmpty();
}

void UHTTP2::manageHeaders()
{
   U_TRACE(0, "UHTTP2::manageHeaders()")

   int32_t padlen;
   const char* ptr;
   const char* endptr = (ptr = frame.payload) + frame.length;

   if ((frame.flags & FLAG_PADDED) == 0) padlen = 0;
   else
      {
      padlen = *ptr++;

#  ifdef DEBUG
      if (frame.length < padlen)
         {
         nerror = PROTOCOL_ERROR;

         return;
         }
#  endif

      endptr -= padlen;
      }

   if ((frame.flags & FLAG_PRIORITY) == 0)
      {
      pStream->priority_exclusive  = false;
      pStream->priority_dependency = 0;
      pStream->priority_weight     = 0;
      }
   else
      {
#  ifdef DEBUG
      if ((frame.length - padlen) < 5)
         {
         nerror = PROTOCOL_ERROR;

         return;
         }
#  endif

      uint32_t u4 = ntohl(*(uint32_t*)ptr);
                                      ptr += 4;

      pStream->priority_exclusive  = (u4 >> 31) != 0;
      pStream->priority_dependency =  u4 & 0x7fffffff;
      pStream->priority_weight     = (uint16_t)*ptr++ + 1;
      }

   if ((frame.flags & FLAG_END_HEADERS) != 0) decodeHeaders(ptr, endptr); // parse header block fragment
   else
      {
      // we must wait for CONTINUATION frames for the same stream...

      U_INTERNAL_ASSERT_EQUALS(frame.flags & FLAG_END_HEADERS, 0)

      uint32_t sz = (endptr - ptr);

      UString bufferHeaders(ptr, sz, sz * 2);

wait_CONTINUATION:
      readFrame();

#  ifdef DEBUG
      if (nerror != NO_ERROR         ||
          frame.type != CONTINUATION ||
          frame.stream_id != pStream->id)
         {
         nerror = PROTOCOL_ERROR;

         return;
         }
#  endif

      (void) bufferHeaders.append(frame.payload, frame.length);

      if ((frame.flags & FLAG_END_HEADERS) == 0) goto wait_CONTINUATION;

      ptr = bufferHeaders.data();

      decodeHeaders(ptr, ptr + bufferHeaders.size()); // parse header block fragment
      }
}

void UHTTP2::manageData()
{
   U_TRACE(0, "UHTTP2::manageData()")

   if ((frame.flags & FLAG_PADDED) == 0)
      {
      U_INTERNAL_ASSERT((uint32_t)frame.length <= U_http_info.clength)

      (void) UClientImage_Base::body->append(frame.payload, frame.length);

      return;
      }

   const char* ptr    = frame.payload;;
   int32_t padlen     = *ptr;
   const char* endptr =  ptr + frame.length;

#ifdef DEBUG
   if (frame.length < padlen)
      {
      nerror = PROTOCOL_ERROR;

      return;
      }
#endif

   uint32_t sz = (endptr - padlen) - ++ptr;

   U_INTERNAL_ASSERT(sz <= U_http_info.clength)

   (void) UClientImage_Base::body->append(ptr, sz);
}

bool UHTTP2::manageSetting()
{
   U_TRACE(0, "UHTTP2::manageSetting()")

   U_INTERNAL_ASSERT_EQUALS(U_http_version, '2')

   U_INTERNAL_DUMP("HTTP2-Settings: = %.*S U_http_method_type = %B", U_http2_settings_len, UHTTP2::upgrade_settings, U_http_method_type)

   if (U_http2_settings_len &&
       USocketExt::write(UServer_Base::csocket, U_CONSTANT_TO_PARAM(HTTP2_CONNECTION_UPGRADE_AND_SETTING_BIN), UServer_Base::timeoutMS) !=
                                                    U_CONSTANT_SIZE(HTTP2_CONNECTION_UPGRADE_AND_SETTING_BIN))
      {
      U_RETURN(false);
      }

   U_INTERNAL_ASSERT(UClientImage_Base::request->same(*UClientImage_Base::rbuffer))

   UClientImage_Base::request->clear();

   // maybe we have read more data than necessary...

   uint32_t sz = UClientImage_Base::rbuffer->size();

   U_INTERNAL_ASSERT_MAJOR(sz, 0)

   if (sz > U_http_info.endHeader) UClientImage_Base::rstart = U_http_info.endHeader;
   else
      {
      // we wait for HTTP2_CONNECTION_PREFACE...

      UClientImage_Base::rbuffer->setEmptyForce();

      if (UNotifier::waitForRead(UServer_Base::csocket->iSockDesc, U_TIMEOUT_MS) != 1 ||
          USocketExt::read(UServer_Base::csocket, *UClientImage_Base::rbuffer, U_SINGLE_READ, 0) == false)
         {
         U_RETURN(false);
         }

      UClientImage_Base::rstart = 0;

      sz = UClientImage_Base::rbuffer->size();
      }

   const char* ptr = UClientImage_Base::rbuffer->c_pointer(UClientImage_Base::rstart);

   if (u_get_unalignedp64(ptr)    != U_MULTICHAR_CONSTANT64( 'P', 'R','I',' ', '*', ' ', 'H', 'T') ||
       u_get_unalignedp64(ptr+8)  != U_MULTICHAR_CONSTANT64( 'T', 'P','/','2', '.', '0','\r','\n') ||
       u_get_unalignedp64(ptr+16) != U_MULTICHAR_CONSTANT64('\r','\n','S','M','\r','\n','\r','\n'))
      {
      U_RETURN(false);
      }

   pConnection    = (Connection*)UServer_Base::pClientImage->connection;
   pConnectionEnd = (char*)pConnection + sizeof(Connection);

   pConnection->state         = CONN_STATE_OPEN;
   pConnection->peer_settings = settings;

   pConnection->max_open_stream_id      =
   pConnection->num_responding_streams  =
   pConnection->max_processed_stream_id = 0;

   pStream = pConnection->streams;

   pStream->state = STREAM_STATE_IDLE;

   if (U_http2_settings_len == 0)
      {
      if (USocketExt::write(UServer_Base::csocket, U_CONSTANT_TO_PARAM(HTTP2_SETTINGS_BIN), UServer_Base::timeoutMS) != U_CONSTANT_SIZE(HTTP2_SETTINGS_BIN)) U_RETURN(false);
      }
   else
      {
      UString buffer(U_CAPACITY);

      UBase64::decodeUrl(upgrade_settings, U_http2_settings_len, buffer);

      if (buffer.empty() ||
          updateSetting(U_STRING_TO_PARAM(buffer)) == false)
         {
         U_RETURN(false);
         }
      }

   settings_ack = false;

   UClientImage_Base::rstart += U_CONSTANT_SIZE(HTTP2_CONNECTION_PREFACE);

loop:
   readFrame();

   if (nerror == NO_ERROR)
      {
      U_INTERNAL_DUMP("settings_ack = %b", settings_ack)

      if (settings_ack == false) goto loop; // we wait for SETTINGS ack...

      U_RETURN(true);
      }

   sendError();

   U_RETURN(false);
}

#ifdef ENTRY
#undef ENTRY
#endif
#define ENTRY(n) n: descr = #n; break

#ifdef DEBUG
const char* UHTTP2::getFrameTypeDescription()
{
   U_TRACE(0, "UHTTP2::getFrameTypeDescription()")

   const char* descr;

   switch (frame.type)
      {
      case ENTRY(DATA);
      case ENTRY(HEADERS);
      case ENTRY(PRIORITY);
      case ENTRY(RST_STREAM);
      case ENTRY(SETTINGS);
      case ENTRY(PUSH_PROMISE);
      case ENTRY(PING);
      case ENTRY(GOAWAY);
      case ENTRY(WINDOW_UPDATE);
      case ENTRY(CONTINUATION);

      default: descr = "Frame type unknown";
      }

   U_RETURN(descr);
}
#endif
