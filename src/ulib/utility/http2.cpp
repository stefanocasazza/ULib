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
uint32_t                      UHTTP2::hash_static_table[61];
const char*                   UHTTP2::upgrade_settings;
UHTTP2::Stream*               UHTTP2::pStream;
UVector<UString>*             UHTTP2::vext;
UHTTP2::FrameHeader           UHTTP2::frame;
UHTTP2::Connection*           UHTTP2::vConnection;
UHTTP2::Connection*           UHTTP2::pConnection;
UHTTP2::HpackHeaderTableEntry UHTTP2::hpack_static_table[61];

// ===================================
//            SETTINGS FRAME
// ===================================
// frame size = 18 (3*6)
// settings frame
// no flags
// stream id = 0
// ===================================
// max_concurrent_streams = 100
//    initial_window_size = 2147483646
//         max_frame_size = 16777215
// ===================================
// 9 + 18 = 27 bytes
// ===================================

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

UHTTP2::Connection::Connection() : itable(53, setIndexStaticTable)
{
   U_TRACE_REGISTER_OBJECT(0, Connection, "", 0)

   state  = CONN_STATE_IDLE;

   max_open_stream_id      =
   max_processed_stream_id = 0;

   (void) memset(&streams, 0, sizeof(Stream) * 100);
}

void UHTTP2::ctor()
{
   U_TRACE_NO_PARAM(0+256, "UHTTP2::ctor()")

   UString::str_allocate(STR_ALLOCATE_HTTP2);

   hpack_static_table[0].name   = UString::str_authority;
    hash_static_table[0]        = UString::str_authority->hash();
   hpack_static_table[1].name   = UString::str_method;
    hash_static_table[1]        = UString::str_method->hash();
   hpack_static_table[1].value  = UString::str_method_get;
   hpack_static_table[2].name   = UString::str_method;
   hpack_static_table[2].value  = UString::str_method_post;
   hpack_static_table[3].name   = UString::str_path;
    hash_static_table[3]        = UString::str_path->hash();
   hpack_static_table[3].value  = UString::str_path_root;
   hpack_static_table[4].name   = UString::str_path;
   hpack_static_table[4].value  = UString::str_path_index;
   hpack_static_table[5].name   = UString::str_scheme;
    hash_static_table[5]        = UString::str_scheme->hash();
   hpack_static_table[5].value  = UString::str_scheme_http;
   hpack_static_table[6].name   = UString::str_scheme;
   hpack_static_table[6].value  = UString::str_scheme_https;
   hpack_static_table[7].name   = UString::str_status;
    hash_static_table[7]        = UString::str_status->hash();
   hpack_static_table[7].value  = UString::str_status_200;
   hpack_static_table[8].name   = UString::str_status;
   hpack_static_table[8].value  = UString::str_status_204;
   hpack_static_table[9].name   = UString::str_status;
   hpack_static_table[9].value  = UString::str_status_206;
   hpack_static_table[10].name  = UString::str_status;
   hpack_static_table[10].value = UString::str_status_304;
   hpack_static_table[11].name  = UString::str_status;
   hpack_static_table[11].value = UString::str_status_400;
   hpack_static_table[12].name  = UString::str_status;
   hpack_static_table[12].value = UString::str_status_404;
   hpack_static_table[13].name  = UString::str_status;
   hpack_static_table[13].value = UString::str_status_500;
   hpack_static_table[14].name  = UString::str_accept_charset;
    hash_static_table[14]       = UString::str_accept_charset->hash();
   hpack_static_table[15].name  = UString::str_accept_encoding;
    hash_static_table[15]       = UString::str_accept_encoding->hash();
   hpack_static_table[15].value = UString::str_accept_encoding_value;
   hpack_static_table[16].name  = UString::str_accept_language;
    hash_static_table[16]       = UString::str_accept_language->hash();
   hpack_static_table[17].name  = UString::str_accept_ranges;
    hash_static_table[17]       = UString::str_accept_ranges->hash();
   hpack_static_table[18].name  = UString::str_accept;
    hash_static_table[18]       = UString::str_accept->hash();
   hpack_static_table[19].name  = UString::str_access_control_allow_origin;
    hash_static_table[19]       = UString::str_access_control_allow_origin->hash();
   hpack_static_table[20].name  = UString::str_age;
    hash_static_table[20]       = UString::str_age->hash();
   hpack_static_table[21].name  = UString::str_allow;
    hash_static_table[21]       = UString::str_allow->hash();
   hpack_static_table[22].name  = UString::str_authorization;
    hash_static_table[22]       = UString::str_authorization->hash();
   hpack_static_table[23].name  = UString::str_cache_control;
    hash_static_table[23]       = UString::str_cache_control->hash();
   hpack_static_table[24].name  = UString::str_content_disposition;
    hash_static_table[24]       = UString::str_content_disposition->hash();
   hpack_static_table[25].name  = UString::str_content_encoding;
    hash_static_table[25]       = UString::str_content_encoding->hash();
   hpack_static_table[26].name  = UString::str_content_language;
    hash_static_table[26]       = UString::str_content_language->hash();
   hpack_static_table[27].name  = UString::str_content_length;
    hash_static_table[27]       = UString::str_content_length->hash();
   hpack_static_table[28].name  = UString::str_content_location;
    hash_static_table[28]       = UString::str_content_location->hash();
   hpack_static_table[29].name  = UString::str_content_range;
    hash_static_table[29]       = UString::str_content_range->hash();
   hpack_static_table[30].name  = UString::str_content_type;
    hash_static_table[30]       = UString::str_content_type->hash();
   hpack_static_table[31].name  = UString::str_cookie;
    hash_static_table[31]       = UString::str_cookie->hash();
   hpack_static_table[32].name  = UString::str_date;
    hash_static_table[32]       = UString::str_date->hash();
   hpack_static_table[33].name  = UString::str_etag;
    hash_static_table[33]       = UString::str_etag->hash();
   hpack_static_table[34].name  = UString::str_expect;
    hash_static_table[34]       = UString::str_expect->hash();
   hpack_static_table[35].name  = UString::str_expires;
    hash_static_table[35]       = UString::str_expires->hash();
   hpack_static_table[36].name  = UString::str_from;
    hash_static_table[36]       = UString::str_from->hash();
   hpack_static_table[37].name  = UString::str_host;
    hash_static_table[37]       = UString::str_host->hash();
   hpack_static_table[38].name  = UString::str_if_match;
    hash_static_table[38]       = UString::str_if_match->hash();
   hpack_static_table[39].name  = UString::str_if_modified_since;
    hash_static_table[39]       = UString::str_if_modified_since->hash();
   hpack_static_table[40].name  = UString::str_if_none_match;
    hash_static_table[40]       = UString::str_if_none_match->hash();
   hpack_static_table[41].name  = UString::str_if_range;
    hash_static_table[41]       = UString::str_if_range->hash();
   hpack_static_table[42].name  = UString::str_if_unmodified_since;
    hash_static_table[42]       = UString::str_if_unmodified_since->hash();
   hpack_static_table[43].name  = UString::str_last_modified;
    hash_static_table[43]       = UString::str_last_modified->hash();
   hpack_static_table[44].name  = UString::str_link;
    hash_static_table[44]       = UString::str_link->hash();
   hpack_static_table[45].name  = UString::str_location;
    hash_static_table[45]       = UString::str_location->hash();
   hpack_static_table[46].name  = UString::str_max_forwards;
    hash_static_table[46]       = UString::str_max_forwards->hash();
   hpack_static_table[47].name  = UString::str_proxy_authenticate;
    hash_static_table[47]       = UString::str_proxy_authenticate->hash();
   hpack_static_table[48].name  = UString::str_proxy_authorization;
    hash_static_table[48]       = UString::str_proxy_authorization->hash();
   hpack_static_table[49].name  = UString::str_range;
    hash_static_table[49]       = UString::str_range->hash();
   hpack_static_table[50].name  = UString::str_referer;
    hash_static_table[50]       = UString::str_referer->hash();
   hpack_static_table[51].name  = UString::str_refresh;
    hash_static_table[51]       = UString::str_refresh->hash();
   hpack_static_table[52].name  = UString::str_retry_after;
    hash_static_table[52]       = UString::str_retry_after->hash();
   hpack_static_table[53].name  = UString::str_server;
    hash_static_table[53]       = UString::str_server->hash();
   hpack_static_table[54].name  = UString::str_set_cookie;
    hash_static_table[54]       = UString::str_set_cookie->hash();
   hpack_static_table[55].name  = UString::str_strict_transport_security;
    hash_static_table[55]       = UString::str_strict_transport_security->hash();
   hpack_static_table[56].name  = UString::str_transfer_encoding;
    hash_static_table[56]       = UString::str_transfer_encoding->hash();
   hpack_static_table[57].name  = UString::str_user_agent;
    hash_static_table[57]       = UString::str_user_agent->hash();
   hpack_static_table[58].name  = UString::str_vary;
    hash_static_table[58]       = UString::str_vary->hash();
   hpack_static_table[59].name  = UString::str_via;
    hash_static_table[59]       = UString::str_via->hash();
   hpack_static_table[60].name  = UString::str_www_authenticate;
    hash_static_table[60]       = UString::str_www_authenticate->hash();

   vext = U_NEW(UVector<UString>(10));
}

void UHTTP2::dtor()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::dtor()")

   delete   vext;
   delete[] vConnection;
}

bool UHTTP2::updateSetting(unsigned char* ptr, uint32_t len)
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

      for (*dst++ |= prefix_max; value >= 256; value >>= 8) *dst++ = 0x80 | value;

      *dst++ = value;
      }

   return dst;
}

unsigned char* UHTTP2::hpackDecodeInt(unsigned char* src, unsigned char* src_end, int32_t* pvalue, uint8_t prefix_max)
{
   U_TRACE(0, "UHTTP2::hpackDecodeInt(%p,%p,%p,%u)", src, src_end, pvalue, prefix_max)

   U_INTERNAL_ASSERT_DIFFERS(src, src_end)

   *pvalue = (uint8_t)*src++ & prefix_max;

   if (*pvalue == prefix_max)
      {
      // we only allow at most 4 octets (excluding prefix) to be used as int (== 2**(4*7) == 2**28)

      if ((src_end - src) > 4)
           src_end = src  + 4;

      *pvalue = prefix_max;

      for (int32_t mult = 1; src < src_end; mult *= 128)
         {
         *pvalue += (*src & 127) * mult;

         if ((*src++ & 128) == 0) return src;
         }

      *pvalue = -1;
      }

   return src;
}

unsigned char* UHTTP2::hpackEncodeString(unsigned char* dst, const char* src, uint32_t len)
{
   U_TRACE(0+256, "UHTTP2::hpackEncodeString(%p,%.*S,%u)", dst, len, src, len)

   U_INTERNAL_ASSERT_MAJOR(len, 0)

   if (len < 29)
      {
      // encode as-is

asis: *dst = '\0';
       dst = hpackEncodeInt(dst, len, (1<<7)-1);

      u__memcpy(dst, src, len, __PRETTY_FUNCTION__);
      }
   else
      {
      uint64_t bits = 0;
      int bits_left = 40;
      const char* src_end = src + len;

      UString buffer(len + 1024U);

      unsigned char* _dst       = (unsigned char*)buffer.data();
      unsigned char* _dst_end   = _dst + len;
      unsigned char* _dst_start = _dst;

      // try to encode in huffman

      do {
         const HuffSym* sym = huff_sym_table + *(unsigned char*)src++;

      // U_INTERNAL_DUMP("sym->nbits = %u sym->code = %u bits_left = %d", sym->nbits, sym->code, bits_left)

         bits |= (uint64_t)sym->code << (bits_left - sym->nbits);

         bits_left -= sym->nbits;

      // U_INTERNAL_DUMP("bits = %llu bits_left = %d", bits, bits_left)

         while (bits_left <= 32)
            {
            *_dst++ = bits >> 32;

         // U_INTERNAL_DUMP("_dst = %u bits >> 32 = %u", _dst[-1], bits >> 32)

            bits <<= 8;
            bits_left += 8;

            U_INTERNAL_ASSERT_MINOR(_dst, _dst_end)
            }
         }
      while (src < src_end);

   // U_INTERNAL_DUMP("bits = %llu bits_left = %d", bits, bits_left)

      if (bits_left != 40)
         {
         bits |= (1UL << bits_left) - 1;

         *_dst++ = bits >> 32;

      // U_INTERNAL_DUMP("_dst = %u bits >> 32 = %u", _dst[-1], bits >> 32)
         }

#  ifdef DEBUG
      int32_t u_printf_string_max_length_save = u_printf_string_max_length;
                                                u_printf_string_max_length = buffer.distance((const char*)_dst) * 3;

      U_INTERNAL_DUMP("_dst_end = %p _dst = %p %#.*S", _dst_end, _dst, buffer.distance((const char*)_dst), _dst_start)

      u_printf_string_max_length = u_printf_string_max_length_save;
#  endif

      if (_dst > _dst_end) goto asis; // encode as-is

      *dst = '\x80';
       dst = hpackEncodeInt(dst, (len = _dst-_dst_start), (1<<7)-1);

      u__memcpy(dst, _dst_start, len, __PRETTY_FUNCTION__);
      }

   return dst+len;
}

unsigned char* UHTTP2::hpackDecodeString(unsigned char* src, unsigned char* src_end, UString* pvalue)
{
   U_TRACE(0+256, "UHTTP2::hpackDecodeString(%p,%p,%p)", src, src_end, pvalue)

   int32_t len;
   bool is_huffman = ((*src & 0x80) != 0);

   src = hpackDecodeInt(src, src_end, &len, (1<<7)-1);

   if (len <= 0)
      {
err:  pvalue->clear();

      return src;
      }

   U_INTERNAL_DUMP("is_huffman = %b len = %u src = %#.*S", is_huffman, len, len, src)

   if (is_huffman == false)
      {
      if ((src + len) > src_end) goto err;

      (void) pvalue->replace((const char*)src, len);

      return (src+len);
      }

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

   return src_end;
}

void UHTTP2::decodeHeaders(unsigned char* ptr, unsigned char* endptr)
{
   U_TRACE(0, "UHTTP2::decodeHeaders(%p,%p)", ptr, endptr)

   uint32_t sz;
   int32_t index;
   unsigned char c;
   const char* ptr1;
   UString name, value;
   UHTTP2::HpackHeaderTableEntry* entry;
   bool value_is_indexed, continue100 = false;
   UHashMap<UString>* ptable = &(pConnection->itable);

   while (ptr < endptr)
      {
      value_is_indexed = ((c = *ptr) >= 128);

      U_INTERNAL_DUMP("c = %u value_is_indexed = %b", c, value_is_indexed)

      if (value_is_indexed) // indexed header field representation
         {
         ptr = hpackDecodeInt(ptr, endptr, &index, (1<<7)-1);
         }
      else if (c > 64) // literal header field with incremental handling
         {
         ptr = hpackDecodeInt(ptr, endptr, &index, (1<<6)-1);
         }
      else if (c == 64) // literal header field with incremental handling
         {
         ++ptr;

         index = 0;
         }
      else if (c < 32) // literal header field without indexing or never indexed
         {
         if ((c & 0x0f) != 0) ptr = hpackDecodeInt(ptr, endptr, &index, (1<<4)-1);
         else
            {
            ++ptr;

            index = 0;
            }
         }
      else // size update
         {
         ptr = hpackDecodeInt(ptr, endptr, &index, (1<<5)-1);

         if (index > pConnection->hpack_max_capacity) goto error;

         pConnection->hpack_max_capacity = index;

         continue;
         }

      U_INTERNAL_DUMP("index = %d", index)

      if (index <= 0)
         {
         if (index == 0) // non-existing name
            {
            ptr = hpackDecodeString(ptr, endptr, &name);

            if (name)
               {
               ptr = hpackDecodeString(ptr, endptr, &value);

               if (value)
                  {
                  // add the decoded header to the header table

                  ptable->hash = name.hash();

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

      name = *UString::str_host;

      goto host;

case_1: // authority (a.k.a. the Host header)

      name = *UString::str_authority;

host: U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      sz = value.size();

      if (sz == 0) goto error;

      UHTTP::setHostname(value.data(), sz);

      ptable->hash = hash_static_table[37]; // host

      goto insert;

case_2_3: // GET - POST

      if (value_is_indexed) U_http_method_type = (index == 2 ? HTTP_GET : (U_http_method_num = 2, HTTP_POST));
      else
         {
         ptr = hpackDecodeString(ptr, endptr, &value);

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
            }
         }

      continue;

case_4_5: // / - /index.html

      name = *UString::str_path;

      ptable->hash = hash_static_table[3]; // path

      // determine the value (if necessary)

      if (value_is_indexed)
         {
         value = *(index == 4 ? UString::str_path_root : UString::str_path_index);

         U_http_info.uri     = value.data();
         U_http_info.uri_len = value.size();
         }
      else
         {
         ptr = hpackDecodeString(ptr, endptr, &value);

         sz = value.size();

         if (sz == 0) goto error;

         U_http_info.query = (const char*) memchr((U_http_info.uri = value.data()), '?', (U_http_info.uri_len = sz));

         if (U_http_info.query)
            {
            U_http_info.query_len = U_http_info.uri_len - (U_http_info.query++ - U_http_info.uri);

            U_http_info.uri_len -= U_http_info.query_len;

            U_INTERNAL_DUMP("query = %.*S", U_HTTP_QUERY_TO_TRACE)
            }
         }

      U_INTERNAL_DUMP("URI = %.*S", U_HTTP_URI_TO_TRACE)

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

      name = *UString::str_accept_encoding;

      U_INTERNAL_ASSERT(value_is_indexed)

      value = *UString::str_accept_encoding_value;

      U_http_is_accept_gzip = '1';

      U_INTERNAL_DUMP("U_http_is_accept_gzip = %C", U_http_is_accept_gzip)

      ptable->hash = hash_static_table[15]; // accept_encoding

      goto insert;

case_17: // accept-language

      name = *UString::str_accept_language;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      U_http_accept_language_len = value.size();

      if (U_http_accept_language_len == 0) goto error;

      U_http_info.accept_language = value.data();

      U_INTERNAL_DUMP("Accept-Language: = %.*S", U_HTTP_ACCEPT_LANGUAGE_TO_TRACE)

      ptable->hash = hash_static_table[16]; // accept_language

      goto insert;

case_19: // accept

      name = *UString::str_accept;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      U_http_accept_len = value.size();

      if (U_http_accept_len == 0) goto error;

      U_http_info.accept = value.data();

      U_INTERNAL_DUMP("Accept: = %.*S", U_HTTP_ACCEPT_TO_TRACE)

      ptable->hash = hash_static_table[18]; // accept

      goto insert;

case_28: // content_length

      name = *UString::str_content_length;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      if (value.empty()) goto error;

      U_http_info.clength = (uint32_t) strtoul(value.data(), 0, 10);

      U_INTERNAL_DUMP("Content-Length: = %.*S U_http_info.clength = %u", U_STRING_TO_TRACE(value), U_http_info.clength)

      ptable->hash = hash_static_table[27]; // content_length 

      goto insert;

case_31: // content_type

      name = *UString::str_content_type;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      U_http_content_type_len = value.size();

      if (U_http_content_type_len == 0) goto error;

      U_http_info.content_type = value.data();

      U_INTERNAL_DUMP("Content-Type(%u): = %.*S", U_http_content_type_len, U_HTTP_CTYPE_TO_TRACE)

      ptable->hash = hash_static_table[30]; // content_type 

      goto insert;

case_32: // cookie

      name = *UString::str_cookie;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      U_http_info.cookie_len = value.size();

      if (U_http_info.cookie_len == 0) goto error;

      U_http_info.cookie = value.data();

      U_INTERNAL_DUMP("Cookie(%u): = %.*S", U_http_info.cookie_len, U_HTTP_COOKIE_TO_TRACE)

      ptable->hash = hash_static_table[31]; // cookie 

      goto insert;

case_35: // expect

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      if (value.empty()) goto error;

      // NB: check for 'Expect: 100-continue' (as curl does)...

      continue100 = value.equal(U_CONSTANT_TO_PARAM("100-continue"));

      if (continue100 == false)
         {
         name = *UString::str_expect;

         ptable->hash = hash_static_table[34]; // expect 

         goto insert;
         }

      continue;

case_40: // if-modified-since

      name = *UString::str_if_modified_since;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      if (value.empty()) goto error;

      U_http_info.if_modified_since = UTimeDate::getSecondFromTime(value.data(), true);

      U_INTERNAL_DUMP("If-Modified-Since = %u", U_http_info.if_modified_since)

      ptable->hash = hash_static_table[39]; // if_modified_since 

      goto insert;

case_50: // range 

      name = *UString::str_range;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      U_http_range_len = value.size() - U_CONSTANT_SIZE("bytes=");

      if (U_http_range_len <= 0) goto error;

      U_http_info.range = value.data() + U_CONSTANT_SIZE("bytes=");

      U_INTERNAL_DUMP("Range = %.*S", U_HTTP_RANGE_TO_TRACE)

      ptable->hash = hash_static_table[49]; // range 

      goto insert;

case_51: // referer 

      name = *UString::str_referer;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      U_http_info.referer_len = value.size();

      if (U_http_info.referer_len == 0) goto error;

      U_http_info.referer = value.data();

      U_INTERNAL_DUMP("Referer(%u): = %.*S", U_http_info.referer_len, U_HTTP_REFERER_TO_TRACE)

      ptable->hash = hash_static_table[50]; // referer 

      goto insert;

case_57: // transfer-encoding

      U_SRV_LOG("WARNING: Transfer-Encoding is not supported in HTTP/2");

      goto error;

case_58: // user-agent

      name = *UString::str_user_agent;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      U_http_info.user_agent_len = value.size();

      if (U_http_info.user_agent_len == 0) goto error;

      U_http_info.user_agent = value.data();

      U_INTERNAL_DUMP("User-Agent: = %.*S", U_HTTP_USER_AGENT_TO_TRACE)

      ptable->hash = hash_static_table[57]; // user_agent

      goto insert;

cdefault:
      entry = hpack_static_table + --index;

      ptable->hash = hash_static_table[index];

                             name = *(entry->name);
      if (value_is_indexed) value = *(entry->value); // determine the value (if necessary)
      else  
         {
         ptr = hpackDecodeString(ptr, endptr, &value);

         if (value.empty()) goto error;
         }

insert:
      ptable->insert(name, value); // add the decoded header to the header table

      U_INTERNAL_DUMP("name = %V value = %V", name.rep, value.rep)
      }

   U_INTERNAL_DUMP("U_http_method_type = %B U_http_method_num = %d", U_http_method_type, U_http_method_num)

   if (U_http_info.clength)
      {
      U_INTERNAL_DUMP("U_http_info.clength = %u UHTTP::limit_request_body = %u", U_http_info.clength, UHTTP::limit_request_body)

      if (U_http_info.clength > UHTTP::limit_request_body)
         {
         UClientImage_Base::setCloseConnection();

         U_http_info.nResponseCode = HTTP_ENTITY_TOO_LARGE;

         UHTTP::setResponse(0, 0);

         return;
         }

      if (continue100)
         {
         char buffer[HTTP2_FRAME_HEADER_SIZE+5] = { 0, 0, 5,               // frame size
                                                    HEADERS,               // header frame
                                                    FLAG_END_HEADERS,      // end header flags
                                                    0, 0, 0, 0,            // stream id
                                                    8, 3, '1', '0', '0' }; // use literal header field without indexing - indexed name

         *(uint32_t*)(buffer+5) = htonl(frame.stream_id);

         if (USocketExt::write(UServer_Base::csocket, buffer, sizeof(buffer), UServer_Base::timeoutMS) != sizeof(buffer))
            {
            nerror = FLOW_CONTROL_ERROR;

            return;
            }
         }

      (void) UClientImage_Base::body->reserve(U_http_info.clength);
      }
}

void UHTTP2::openStream()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::openStream()")

   U_INTERNAL_ASSERT_POINTER(pStream)
   U_INTERNAL_ASSERT(pConnection->max_open_stream_id <= frame.stream_id)

       pStream->out_window =
   pConnection->out_window =                   settings.initial_window_size; //   sender flow control window
       pStream->inp_window =
   pConnection->inp_window = pConnection->peer_settings.initial_window_size; // receiver flow control window

   pStream->id                     =
   pConnection->max_open_stream_id = frame.stream_id;

   pConnection->hpack_max_capacity = settings.header_table_size;

   pStream->state = ((frame.flags & FLAG_END_STREAM) != 0 ? STREAM_STATE_HALF_CLOSED : STREAM_STATE_OPEN);

   U_INTERNAL_DUMP("pStream->id = %d pStream->state = %d pStream->inp_window = %d", pStream->id, pStream->state, pStream->inp_window)
}

void UHTTP2::readFrame()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::readFrame()")

   U_INTERNAL_ASSERT_POINTER(pStream)
   U_INTERNAL_ASSERT_EQUALS(nerror, NO_ERROR)

   uint32_t sz;
    int32_t len;
   const char* ptr;

loop:
   sz = UClientImage_Base::rbuffer->size();

   U_INTERNAL_DUMP("UClientImage_Base::rbuffer->size() = %u UClientImage_Base::rstart = %u", sz, UClientImage_Base::rstart)

   if (sz == UClientImage_Base::rstart)
      {
      U_INTERNAL_DUMP("settings_ack = %b U_http2_settings_len = %b", settings_ack, U_http2_settings_len)

      if (settings_ack &&
          U_http2_settings_len)
         {
         U_DEBUG("HTTP2 upgrade: User-Agent = %.*S", U_HTTP_USER_AGENT_TO_TRACE);

         openStream();

         return;
         }

      resetDataRead();

      if (nerror != NO_ERROR) goto err;

      sz = UClientImage_Base::rbuffer->size();
      }
#ifdef DEBUG
   else
      {
      U_INTERNAL_ASSERT((sz-UClientImage_Base::rstart) >= HTTP2_FRAME_HEADER_SIZE)
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
             frame.stream_id, frame.type, getFrameTypeDescription(frame.type), frame.flags, frame.length, ptr + HTTP2_FRAME_HEADER_SIZE)

   U_INTERNAL_ASSERT_MINOR(frame.length, (int32_t)settings.max_frame_size)

   len = UClientImage_Base::rbuffer->size() - (UClientImage_Base::rstart += HTTP2_FRAME_HEADER_SIZE) - frame.length;

   U_INTERNAL_DUMP("UClientImage_Base::rbuffer->size() = %u UClientImage_Base::rstart = %u frame.length = %u len = %d", sz, UClientImage_Base::rstart, frame.length, len)

   if (len < 0)
      {
      len = -len;

      if (UClientImage_Base::rstart)
         {
         UClientImage_Base::rbuffer->moveToBeginDataInBuffer(UClientImage_Base::rstart); 
                                                             UClientImage_Base::rstart = 0;
         }

      if (USocketExt::read(UServer_Base::csocket, *UClientImage_Base::rbuffer, len, UServer_Base::timeoutMS, UHTTP::request_read_timeout) == false)
         {
         nerror = ERROR_INCOMPLETE;

         goto err;
         }
      }

   frame.payload = (unsigned char*)UClientImage_Base::rbuffer->c_pointer(UClientImage_Base::rstart);
                                                                         UClientImage_Base::rstart += (uint32_t)frame.length;

   if (frame.type == WINDOW_UPDATE)
      {
      uint32_t window_size_increment = ntohl(*(uint32_t*)frame.payload) & 0x7fffffff;

      U_INTERNAL_DUMP("window_size_increment = %u", window_size_increment)

#  ifdef DEBUG
      if (frame.length != 4 ||
          window_size_increment == 0)
         {
         nerror = PROTOCOL_ERROR;

         goto err;
         }
#  endif

      if (frame.stream_id)     pStream->out_window += window_size_increment;
      else                 pConnection->out_window += window_size_increment;

      goto loop;
      }

   if (frame.stream_id == 0)
      {
      if (frame.type == SETTINGS)
         {
         if ((frame.flags & FLAG_ACK) != 0)
            {
#        ifdef DEBUG
            if (frame.length)
               {
               nerror = FRAME_SIZE_ERROR;

               goto err;
               }
#        endif

            settings_ack = true;

            if (UClientImage_Base::rbuffer->size() == UClientImage_Base::rstart) return;
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
         openStream();

         manageHeaders();

         if (nerror != NO_ERROR) goto err;

         return;
         }

      U_INTERNAL_DUMP("pStream->id = %d", pStream->id)

      if (pStream->id != frame.stream_id)
         {
         U_INTERNAL_DUMP("pConnection->max_open_stream_id = %d", pConnection->max_open_stream_id)

         if (frame.stream_id <= pConnection->max_open_stream_id)
            {
            for (Stream* pStreamEnd = (pStream = pConnection->streams) + 100; pStream < pStreamEnd; ++pStream)
               {
               if (pStream->id == frame.stream_id) goto next;
               }
            }

         nerror = FLOW_CONTROL_ERROR;

         goto err;
         }

next: if ((frame.flags & FLAG_END_STREAM) != 0) pStream->state = STREAM_STATE_HALF_CLOSED;

      if (frame.type == DATA)
         {
         manageData();

         if (nerror != NO_ERROR) goto err;

         if (pStream->state != STREAM_STATE_HALF_CLOSED) goto loop; // we must wait for other DATA frames for the same stream...

         return;
         }

      if (frame.type == RST_STREAM)
         {
         U_INTERNAL_ASSERT_EQUALS(pStream->state, STREAM_STATE_HALF_CLOSED)

         pStream->state = STREAM_STATE_CLOSED;

         uint32_t error = ntohl(*(uint32_t*)frame.payload);

         U_INTERNAL_DUMP("error = %u", error)
         }
      }

   return;

err:
   UClientImage_Base::rstart = 0;

   UClientImage_Base::rbuffer->setEmpty();
}

void UHTTP2::manageHeaders()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::manageHeaders()")

   int32_t padlen;
   unsigned char* ptr;
   unsigned char* endptr = (ptr = frame.payload) + frame.length;

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

      (void) bufferHeaders.append((const char*)frame.payload, frame.length);

      if ((frame.flags & FLAG_END_HEADERS) == 0) goto wait_CONTINUATION;

      ptr = (unsigned char*)bufferHeaders.data();

      decodeHeaders(ptr, ptr + bufferHeaders.size()); // parse header block fragment
      }
}

void UHTTP2::resetDataRead()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::resetDataRead()")

   UClientImage_Base::rstart = 0;

   UClientImage_Base::rbuffer->setEmpty();

   if (USocketExt::read(UServer_Base::csocket, *UClientImage_Base::rbuffer, U_SINGLE_READ, UServer_Base::timeoutMS, UHTTP::request_read_timeout) == false) nerror = PROTOCOL_ERROR;
}

void UHTTP2::manageData()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::manageData()")

   // Send Window Update if current window size is not sufficient

   U_INTERNAL_DUMP("pStream->id = %d pStream->state = %d pConnection->inp_window = %d", pStream->id, pStream->state, pConnection->inp_window)

   if (pConnection->inp_window > frame.length) pConnection->inp_window -= frame.length;
   else
      {
      char buffer[HTTP2_FRAME_HEADER_SIZE+4] = { 0, 0, 4,       // frame size
                                                 WINDOW_UPDATE, // header frame
                                                 FLAG_NONE,     // flags
                                                 0, 0, 0, 0,    // stream id
                                                 0, 0, 0, 0 };

      char* ptr = buffer;

      pConnection->inp_window = pConnection->peer_settings.initial_window_size + U_http_info.clength;

      U_INTERNAL_DUMP("pConnection->inp_window = %d", pConnection->inp_window)

      *(uint32_t*)(ptr+9) = htonl(pConnection->inp_window);

   // ptr[9] &= ~(0x01 << 7);

      if (USocketExt::write(UServer_Base::csocket, buffer, HTTP2_FRAME_HEADER_SIZE+4, UServer_Base::timeoutMS) != HTTP2_FRAME_HEADER_SIZE+4)
         {
         nerror = FLOW_CONTROL_ERROR;

         return;
         }
      }

   unsigned char* ptr = frame.payload;

   if ((frame.flags & FLAG_PADDED) != 0)
      {
      int32_t padlen = *ptr;

#  ifdef DEBUG
      U_INTERNAL_DUMP("frame.length = %u padlen = %d", frame.length, padlen)

      if (frame.length < padlen)
         {
         nerror = PROTOCOL_ERROR;

         return;
         }
#  endif

      unsigned char* endptr = ptr + frame.length;

      uint32_t sz = (endptr - padlen) - ++ptr;

      U_INTERNAL_ASSERT(sz <= U_http_info.clength)

      (void) UClientImage_Base::body->append((const char*)ptr, sz);
      }
   else
      {
      U_INTERNAL_ASSERT((uint32_t)frame.length <= U_http_info.clength)

      (void) UClientImage_Base::body->append((const char*)ptr, frame.length);
      }

   if (pStream->state == STREAM_STATE_RESERVED)
      {
      U_INTERNAL_DUMP("U_http_info.clength = %u UClientImage_Base::body->size() = %u", U_http_info.clength, UClientImage_Base::body->size())

      if (U_http_info.clength == UClientImage_Base::body->size()) pStream->state = STREAM_STATE_HALF_CLOSED;
      else
         {
         U_INTERNAL_DUMP("UClientImage_Base::rbuffer->size() = %u UClientImage_Base::rstart = %u", UClientImage_Base::rbuffer->size(), UClientImage_Base::rstart)

         if (UClientImage_Base::rbuffer->size() == UClientImage_Base::rstart) resetDataRead();
         }
      }
}

void UHTTP2::sendError()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::sendError()")

   U_INTERNAL_ASSERT_DIFFERS(nerror, NO_ERROR)
   U_INTERNAL_ASSERT(UServer_Base::csocket->isOpen())

   char buffer[HTTP2_FRAME_HEADER_SIZE+8] = { 0, 0, 4,    // frame size
                                              RST_STREAM, // header frame
                                              FLAG_NONE,  // flags
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

   // *(uint32_t*)(ptr+ 5) = 0;
      *(uint32_t*)(ptr+ 9) = htonl(pConnection->max_processed_stream_id);
      *(uint32_t*)(ptr+13) = htonl(nerror);

      if (USocketExt::write(UServer_Base::csocket, buffer, HTTP2_FRAME_HEADER_SIZE+8, UServer_Base::timeoutMS) != HTTP2_FRAME_HEADER_SIZE+8) U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;
      }
}

void UHTTP2::handlerResponse()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::handlerResponse()")

   U_INTERNAL_DUMP("ext(%u) = %#V", UHTTP::ext->size(), UHTTP::ext->rep)

   uint32_t sz  = HTTP2_FRAME_HEADER_SIZE+1,
            sz1 = UHTTP::set_cookie->size(),
            sz2 = UHTTP::ext->size();

   UClientImage_Base::wbuffer->setBuffer(200U + sz1 + sz2);

            char* ptr = UClientImage_Base::wbuffer->data();
   unsigned char* dst = (unsigned char*)ptr+HTTP2_FRAME_HEADER_SIZE+1;

   if (U_http_info.nResponseCode == HTTP_NOT_IMPLEMENTED ||
       U_http_info.nResponseCode == HTTP_OPTIONS_RESPONSE)
      {
      UClientImage_Base::setCloseConnection();

      ptr[HTTP2_FRAME_HEADER_SIZE] = 0x80 | 8;

      *dst = 0x40;
       dst = hpackEncodeInt(dst, 22, (1<<6)-1);
       dst = hpackEncodeString(dst,
               U_CONSTANT_TO_PARAM("GET, HEAD, POST, PUT, DELETE, OPTIONS, "     // request methods
                                   "TRACE, CONNECT, "                            // pathological
                                   "COPY, MOVE, LOCK, UNLOCK, MKCOL, PROPFIND, " // webdav
                                   "PATCH, PURGE, "                              // rfc-5789
                                   "MERGE, REPORT, CHECKOUT, MKACTIVITY, "       // subversion
                                   "NOTIFY, MSEARCH, SUBSCRIBE, UNSUBSCRIBE"));  // upnp
     
      U_INTERNAL_DUMP("allow(%u) = %#169S", dst-(unsigned char*)ptr-HTTP2_FRAME_HEADER_SIZE+1, ptr+HTTP2_FRAME_HEADER_SIZE+1)
      }
   else
      {
      if (sz2 == 0 &&
          U_http_info.nResponseCode == HTTP_OK)
         {
         ptr[HTTP2_FRAME_HEADER_SIZE] = 0x80 | 9; // HTTP_NO_CONTENT
         }
      else
         {
         switch (U_http_info.nResponseCode)
            {
            case HTTP_OK:             ptr[HTTP2_FRAME_HEADER_SIZE] = 0x80 |  8; break;
            case HTTP_NO_CONTENT:     ptr[HTTP2_FRAME_HEADER_SIZE] = 0x80 |  9; break;
            case HTTP_PARTIAL:        ptr[HTTP2_FRAME_HEADER_SIZE] = 0x80 | 10; break;
            case HTTP_NOT_MODIFIED:   ptr[HTTP2_FRAME_HEADER_SIZE] = 0x80 | 11; break;
            case HTTP_BAD_REQUEST:    ptr[HTTP2_FRAME_HEADER_SIZE] = 0x80 | 12; break;
            case HTTP_NOT_FOUND:      ptr[HTTP2_FRAME_HEADER_SIZE] = 0x80 | 13; break;
            case HTTP_INTERNAL_ERROR: ptr[HTTP2_FRAME_HEADER_SIZE] = 0x80 | 14; break;

            default: // use literal header field without indexing - indexed name
               {
               u_put_unalignedp32(ptr+HTTP2_FRAME_HEADER_SIZE,  U_MULTICHAR_CONSTANT32('\010','\003','0'+(U_http_info.nResponseCode / 100),'\0'));
                      U_NUM2STR16(ptr+HTTP2_FRAME_HEADER_SIZE+3,                                          U_http_info.nResponseCode % 100);

                sz += 4;
               dst += 4;
               }
            }
         }
      }

   /**
    * -------------------------------------------------
    * literal header field with indexing (indexed name)
    * -------------------------------------------------
    * server: ULib
    * date: Wed, 20 Jun 2012 11:43:17 GMT
    * -------------------------------------------------
    * *dst = 0x40;
    *  dst = hpackEncodeInt(dst, 54, (1<<6)-1);
    *  dst = hpackEncodeString(dst, U_CONSTANT_TO_PARAM("ULib"));
    * *dst = 0x40;
    *  dst = hpackEncodeInt(dst, 33, (1<<6)-1);
    * -------------------------------------------------
    */

   u_put_unalignedp64(dst, U_MULTICHAR_CONSTANT64('v','\004','U','L','i','b','a','\0'));

#if defined(U_LINUX) && defined(ENABLE_THREAD) && !defined(U_LOG_ENABLE) && !defined(USE_LIBZ)
   U_INTERNAL_ASSERT_POINTER(u_pthread_time)
   U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::iov_vec[1].iov_base, ULog::ptr_shared_date->date3)
#else
   U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::iov_vec[1].iov_base, ULog::date.date3)

   ULog::updateDate3();
#endif

   dst = hpackEncodeString(dst+7, ((char*)UClientImage_Base::iov_vec[1].iov_base)+6, 29); // Date: Wed, 20 Jun 2012 11:43:17 GMT\r\nServer: ULib\r\nConnection: close\r\n

   if (sz1)
      {
      UClientImage_Base::setRequestNoCache();

     *dst = 0x40;
      dst = hpackEncodeInt(dst, 55, (1<<6)-1);
      dst = hpackEncodeString(dst, UHTTP::set_cookie->data(), sz1);

      UHTTP::set_cookie->setEmpty();
      }

   if (sz2)
      {
      const char* keyp;
      uint32_t pos, index;
      UString key, value, row;

      for (uint32_t i = 0, n = vext->split(*UHTTP::ext, U_CRLF); i < n; ++i)
         {
         row = (*vext)[i];
         pos = row.find_first_of(':');

         key   = row.substr(0U, pos);
         value = row.substr(pos+2);

         U_INTERNAL_DUMP("key(%u) = %#V", key.size(), key.rep)
         U_INTERNAL_DUMP("value(%u) = %#V", value.size(), value.rep)

         U_INTERNAL_ASSERT_EQUALS(u_isBinary((unsigned char*)U_STRING_TO_PARAM(value)), false)

         index = U_NOT_FOUND;
          keyp = key.data();

         switch (u_get_unalignedp32(keyp))
            {
            case U_MULTICHAR_CONSTANT32('C','o','n','t'):
               {
               keyp += 7;

               switch (u_get_unalignedp32(keyp))
                  {
                  case U_MULTICHAR_CONSTANT32('-','D','i','s'): index = 25; break; // content-disposition
                  case U_MULTICHAR_CONSTANT32('-','E','n','c'): index = 26; break; // content-encoding
                  case U_MULTICHAR_CONSTANT32('-','L','a','n'): index = 27; break; // content-language
                  case U_MULTICHAR_CONSTANT32('-','L','o','c'): index = 29; break; // content-location
                  case U_MULTICHAR_CONSTANT32('-','R','a','n'): index = 30; break; // content-range
                  case U_MULTICHAR_CONSTANT32('-','T','y','p'): index = 31; break; // content-type 

                  default:
                     {
                     U_INTERNAL_ASSERT_EQUALS(key, U_STRING_FROM_CONSTANT("Content-Length"))

                     continue;
                     }
                  }
               }
            break;

            case U_MULTICHAR_CONSTANT32('A','c','c','e'):
               {
               keyp += 6;

               if (u_get_unalignedp32(keyp) == U_MULTICHAR_CONSTANT32('-','R','a','n')) index = 18; // accept-ranges
               else
                  {
                  U_INTERNAL_ASSERT_EQUALS(key, U_STRING_FROM_CONSTANT("Access-Control-Allow-Origin"))

                  index = 20; // access-control-allow-origin
                  }
               }
            break;

            case U_MULTICHAR_CONSTANT32('C','a','c','h'): index = 24; break; // cache-control
            case U_MULTICHAR_CONSTANT32('E','t','a','g'): index = 34; break; // etag
            case U_MULTICHAR_CONSTANT32('E','x','p','i'): index = 36; break; // expires
            case U_MULTICHAR_CONSTANT32('L','a','s','t'): index = 44; break; // last-modified
            case U_MULTICHAR_CONSTANT32('L','i','n','k'): index = 45; break; // link
            case U_MULTICHAR_CONSTANT32('L','o','c','a'): index = 46; break; // location
            case U_MULTICHAR_CONSTANT32('P','r','o','x'): index = 48; break; // proxy-authenticate
            case U_MULTICHAR_CONSTANT32('R','e','f','r'): index = 52; break; // refresh
            case U_MULTICHAR_CONSTANT32('R','e','t','r'): index = 53; break; // retry-after
            case U_MULTICHAR_CONSTANT32('S','t','r','i'): index = 56; break; // strict-transport-security
            case U_MULTICHAR_CONSTANT32('V','a','r','y'): index = 59; break; // vary 
            case U_MULTICHAR_CONSTANT32('W','W','W','-'): index = 61; break; // www-authenticate 

            default:
               {
                    if (key.equalnocase(U_CONSTANT_TO_PARAM("Via"))) index = 60; // via
               else if (key.equalnocase(U_CONSTANT_TO_PARAM("Age"))) index = 21; // age
               }
            break;
            }

         *dst = 0x40;

         if (index != U_NOT_FOUND) dst = hpackEncodeInt(     dst, index, (1<<6)-1);
         else                      dst = hpackEncodeString(++dst, U_STRING_TO_PARAM(key));

         dst = hpackEncodeString(dst, U_STRING_TO_PARAM(value));
         }

      vext->clear();
      }
   else
      {
      /**
       * -------------------------------------------------
       * literal header field with indexing (indexed name)
       * -------------------------------------------------
       * content-length: 0
       * -------------------------------------------------
       * *dst = 0x40;
       *  dst = hpackEncodeInt(dst, 28, (1<<6)-1);
       *  dst = hpackEncodeString(dst, U_CONSTANT_TO_PARAM("0"));
       * -------------------------------------------------
       */

      u_put_unalignedp32(dst, U_MULTICHAR_CONSTANT32('\\','\001','0','\0'));
                         dst += 3;
      }

   sz  = dst-(unsigned char*)ptr;
   sz1 = pConnection->peer_settings.max_frame_size;

   U_INTERNAL_DUMP("sz = %u pConnection->peer_settings.max_frame_size = %u", sz, sz1)

   if (sz <= sz1)
      {
      *(uint32_t*) ptr    = htonl((sz-HTTP2_FRAME_HEADER_SIZE) << 8);
                   ptr[3] = HEADERS;
                   ptr[4] = FLAG_END_HEADERS;
      *(uint32_t*)(ptr+5) = htonl(pStream->id);
      }
   else
      {
      *(uint32_t*) ptr    = htonl((sz1-HTTP2_FRAME_HEADER_SIZE) << 8);
                   ptr[3] = HEADERS;
                   ptr[4] = 0;
      *(uint32_t*)(ptr+5) = htonl(pStream->id);

      U_DUMP("frame header response { length = %d stream_id = %d type = (%d, %s) flags = %d } = %#.*S", ntohl(*(uint32_t*)ptr & 0x00ffffff) >> 8,
                  ntohl(*(uint32_t*)(ptr+5) & 0x7fffffff), ptr[3], getFrameTypeDescription(ptr[3]), ptr[4], ntohl(*(uint32_t*)ptr & 0x00ffffff) >> 8, ptr + HTTP2_FRAME_HEADER_SIZE)

      ptr = UClientImage_Base::wbuffer->c_pointer(sz1);
      sz2 = sz-sz1;

      (void) U_SYSCALL(memmove, "%p,%p,%u", ptr+HTTP2_FRAME_HEADER_SIZE, ptr, sz2);

       sz += HTTP2_FRAME_HEADER_SIZE;
      dst += HTTP2_FRAME_HEADER_SIZE;

      *(uint32_t*) ptr    = htonl(sz2 << 8);
                   ptr[3] = CONTINUATION;
                   ptr[4] = FLAG_END_HEADERS;
      *(uint32_t*)(ptr+5) = htonl(pStream->id);
      }

   U_DUMP("frame header response { length = %d stream_id = %d type = (%d, %s) flags = %d } = %#.*S", ntohl(*(uint32_t*)ptr & 0x00ffffff) >> 8,
           ntohl(*(uint32_t*)(ptr+5) & 0x7fffffff), ptr[3], getFrameTypeDescription(ptr[3]), ptr[4], ntohl(*(uint32_t*)ptr & 0x00ffffff) >> 8, ptr + HTTP2_FRAME_HEADER_SIZE)

   sz2 = UClientImage_Base::body->size();

   if (sz2)
      {
      *(uint32_t*) dst    = htonl(sz2 << 8);
                   dst[3] = DATA;
                   dst[4] = FLAG_END_STREAM;
      *(uint32_t*)(dst+5) = htonl(pStream->id);

      U_DUMP("frame data response { length = %d stream_id = %d type = (%d, %s) flags = %d }", ntohl(*(uint32_t*)dst & 0x00ffffff) >> 8,
                  ntohl(*(uint32_t*)(dst+5) & 0x7fffffff), dst[3], getFrameTypeDescription(dst[3]), dst[4])

      sz += HTTP2_FRAME_HEADER_SIZE;
      }

   UClientImage_Base::wbuffer->size_adjust(sz);

   UClientImage_Base::setNoHeaderForResponse();
}

int UHTTP2::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::handlerRequest()")

   U_INTERNAL_ASSERT_EQUALS(U_http_version, '2')

   uint32_t sz = (UServer_Base::pClientImage - UServer_Base::vClientImage);

   U_INTERNAL_DUMP("sz = %u UNotifier::max_connection = %u", sz, UNotifier::max_connection)

   U_INTERNAL_ASSERT_MINOR(sz, UNotifier::max_connection)

   pStream = (pConnection = (vConnection + sz))->streams;

   pConnection->state =   CONN_STATE_OPEN;
       pStream->state = STREAM_STATE_IDLE;

   pConnection->max_open_stream_id      =
   pConnection->max_processed_stream_id = 0;

   pConnection->peer_settings                     = settings;
   pConnection->peer_settings.initial_window_size = 65535; // initial flow-control window size

   U_INTERNAL_DUMP("HTTP2-Settings(%u): = %.*S U_http_method_type = %d %B", U_http2_settings_len, U_http2_settings_len, upgrade_settings, U_http_method_type, U_http_method_type)

   if (U_http2_settings_len)
      {
      if (USocketExt::write(UServer_Base::csocket, U_CONSTANT_TO_PARAM(HTTP2_CONNECTION_UPGRADE_AND_SETTING_BIN), UServer_Base::timeoutMS) !=
                                                       U_CONSTANT_SIZE(HTTP2_CONNECTION_UPGRADE_AND_SETTING_BIN))
         {
         U_RETURN(U_PLUGIN_HANDLER_ERROR);
         }

      UString buffer_settings(U_CAPACITY);

      UBase64::decodeUrl(upgrade_settings, U_http2_settings_len, buffer_settings);

      if (buffer_settings.empty() ||
          updateSetting((unsigned char*)U_STRING_TO_PARAM(buffer_settings)) == false)
         {
         U_RETURN(U_PLUGIN_HANDLER_ERROR);
         }

      U_INTERNAL_ASSERT_MAJOR(U_http_info.startHeader, 2)
      U_INTERNAL_ASSERT_RANGE(1,UClientImage_Base::uri_offset,64)

      sz = U_http_info.startHeader - UClientImage_Base::uri_offset + U_CONSTANT_SIZE(" HTTP/1.1\r\n");

      if (sz > sizeof(UClientImage_Base::cbuffer)) U_RETURN(U_PLUGIN_HANDLER_ERROR);

      U_http_info.uri = UClientImage_Base::cbuffer;

      (void) u__memcpy(UClientImage_Base::cbuffer, UClientImage_Base::request->c_pointer(UClientImage_Base::uri_offset), sz, __PRETTY_FUNCTION__);

      U_INTERNAL_DUMP("U_http_info.uri(%u) = %.*S", U_http_info.uri_len, U_http_info.uri_len, U_http_info.uri)
      }

   U_INTERNAL_ASSERT(UClientImage_Base::request->same(*UClientImage_Base::rbuffer))

   UClientImage_Base::request->clear();

   // maybe we have read more data than necessary...

   sz = UClientImage_Base::rbuffer->size();

   U_INTERNAL_ASSERT_MAJOR(sz, 0)

   if (sz > U_http_info.endHeader) UClientImage_Base::rstart = U_http_info.endHeader;
   else
      {
      // we wait for HTTP2_CONNECTION_PREFACE...

      UClientImage_Base::rbuffer->setEmptyForce();

      if (UNotifier::waitForRead(UServer_Base::csocket->iSockDesc, U_TIMEOUT_MS) != 1 ||
          USocketExt::read(UServer_Base::csocket, *UClientImage_Base::rbuffer, U_SINGLE_READ, 0) == false)
         {
         U_RETURN(U_PLUGIN_HANDLER_ERROR);
         }

      UClientImage_Base::rstart = 0;

      sz = UClientImage_Base::rbuffer->size();
      }

   const char* ptr = UClientImage_Base::rbuffer->c_pointer(UClientImage_Base::rstart);

   if (u_get_unalignedp64(ptr)    != U_MULTICHAR_CONSTANT64( 'P', 'R','I',' ', '*', ' ', 'H', 'T') ||
       u_get_unalignedp64(ptr+8)  != U_MULTICHAR_CONSTANT64( 'T', 'P','/','2', '.', '0','\r','\n') ||
       u_get_unalignedp64(ptr+16) != U_MULTICHAR_CONSTANT64('\r','\n','S','M','\r','\n','\r','\n'))
      {
      U_RETURN(U_PLUGIN_HANDLER_ERROR);
      }

   if (U_http2_settings_len)
      {
      /**
       * The HTTP/1.1 request that is sent prior to upgrade is assigned a stream identifier of 1 with default priority values.
       * Stream 1 is implicitly "half-closed" from the client toward the server, since the request is completed as an HTTP/1.1
       * request. After commencing the HTTP/2 connection, stream 1 is used for the response
       */

      pStream->id                     = 
      pConnection->max_open_stream_id = 1;

      pStream->state = STREAM_STATE_HALF_CLOSED;
      }
   else if (USocketExt::write(UServer_Base::csocket, U_CONSTANT_TO_PARAM(HTTP2_SETTINGS_BIN), UServer_Base::timeoutMS) !=
                                                         U_CONSTANT_SIZE(HTTP2_SETTINGS_BIN))
      {
      U_RETURN(U_PLUGIN_HANDLER_ERROR);
      }

   settings_ack = false;

   UClientImage_Base::rstart += U_CONSTANT_SIZE(HTTP2_CONNECTION_PREFACE);

loop:
   readFrame();

   if (nerror == NO_ERROR)
      {
      U_INTERNAL_DUMP("settings_ack = %b U_http_method_type = %d", settings_ack, U_http_method_type)

      if (settings_ack == false) goto loop; // we wait for SETTINGS ack...

      if (U_http_method_type == 0 ||
          U_http_method_type == HTTP_OPTIONS)
         {
         U_http_info.nResponseCode = (U_http_method_type ? HTTP_OPTIONS_RESPONSE
                                                         : HTTP_NOT_IMPLEMENTED);

         UHTTP::setResponse(0, 0);

         U_RETURN(U_PLUGIN_HANDLER_FINISHED);
         }

      if (UClientImage_Base::isRequestNotFound()) return UHTTP::manageRequest();

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }

   if (UServer_Base::csocket->isOpen()) sendError();

   U_RETURN(U_PLUGIN_HANDLER_ERROR);
}

bool UHTTP2::readBodyRequest()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::readBodyRequest()")

   U_INTERNAL_DUMP("U_http_info.clength = %u UClientImage_Base::body->size() = %u", U_http_info.clength, UClientImage_Base::body->size())

   if (U_http_info.clength > UClientImage_Base::body->size())
      {
      U_INTERNAL_ASSERT_POINTER(pStream)

      U_INTERNAL_DUMP("pStream->id = %d pStream->state = %d", pStream->id, pStream->state)

      pStream->state = STREAM_STATE_RESERVED;

      readFrame();

      if (pStream->state != STREAM_STATE_HALF_CLOSED) U_RETURN(false);
      }

   U_ASSERT_EQUALS(U_http_info.clength, UClientImage_Base::body->size()) 

   U_RETURN(true);
}

#ifdef DEBUG
#ifdef ENTRY
#undef ENTRY
#endif
#define ENTRY(n) n: descr = #n; break

const char* UHTTP2::getFrameTypeDescription(char type)
{
   U_TRACE(0, "UHTTP2::getFrameTypeDescription(%d)", type)

   const char* descr;

   switch (type)
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

#ifdef U_STDCPP_ENABLE
U_EXPORT const char* UHTTP2::Connection::dump(bool reset) const
{
   *UObjectIO::os << "state                     " << state                   << '\n'
                  << "out_window                " << out_window              << '\n'
                  << "inp_window                " << inp_window              << '\n'
                  << "hpack_max_capacity        " << hpack_max_capacity      << '\n'
                  << "max_open_stream_id        " << max_open_stream_id      << '\n'
                  << "max_processed_stream_id   " << max_processed_stream_id << '\n'
                  << "itable (UHashMap<UString> " << (void*)&itable          << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
#endif
