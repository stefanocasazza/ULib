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
#include <ulib/utility/base64.h>
#include <ulib/utility/hpack_huffman_table.h> // huff_sym_table, huff_decode_table

#ifndef UINT32_MAX
#define UINT32_MAX (4294967295U)
#endif

#define HTTP2_MAX_WINDOW_SIZE (2147483647U-1)
#define HTTP2_HEADER_TABLE_OFFSET 62
#define HTTP2_DEFAULT_WINDOW_SIZE 65535 

int                           UHTTP2::nerror;
bool                          UHTTP2::continue100;
bool                          UHTTP2::settings_ack;
uint32_t                      UHTTP2::hash_static_table[61];
const char*                   UHTTP2::upgrade_settings;
UHTTP2::Stream*               UHTTP2::pStream;
UHTTP2::FrameHeader           UHTTP2::frame;
UHTTP2::Connection*           UHTTP2::vConnection;
UHTTP2::Connection*           UHTTP2::pConnection;
UHTTP2::HpackHeaderTableEntry UHTTP2::hpack_static_table[61];

#ifdef DEBUG
bool UHTTP2::bdecodeHeadersDebug;
#endif

// ===================================
//            SETTINGS FRAME
// ===================================
// frame size = 18 (3*6)
// settings frame
// no flags
// stream id = 0
// ===================================
// max_concurrent_streams =        100
//    initial_window_size = 2147483647-1
//         max_frame_size =   16777215
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
   /* enable_push = */ 0,
   /* max_concurrent_streams = */ 100,
   /* initial_window_size = */ 2147483647-1,
   /* max_frame_size = */ 16777215,
   /* max_header_list_size */ UINT32_MAX
};

#ifdef DEBUG
UHTTP2::Connection::Connection() : itable(53, setIndexStaticTable), dtable(53, setIndexStaticTable)
#else
UHTTP2::Connection::Connection() : itable(53, setIndexStaticTable)
#endif
{
   U_TRACE_REGISTER_OBJECT(0, Connection, "", 0)

   state                   = CONN_STATE_IDLE; 
   inp_window              =
   out_window              =
   max_open_stream_id      =
   max_processed_stream_id = 0;

   (void) memset(&idyntbl, 0, sizeof(HpackDynamicTable));
   (void) memset(&odyntbl, 0, sizeof(HpackDynamicTable));
   (void) memset(&streams, 0, sizeof(Stream) * 100);
#ifdef DEBUG
   (void) memset(&ddyntbl, 0, sizeof(HpackDynamicTable));
#endif
}

void UHTTP2::ctor()
{
   U_TRACE_NO_PARAM(0+256, "UHTTP2::ctor()")

   UString::str_allocate(STR_ALLOCATE_HTTP2);

   hpack_static_table[0].name   = UString::str_authority;
    hash_static_table[0]        = UString::str_authority->hashIgnoreCase();
   hpack_static_table[1].name   = UString::str_method;
    hash_static_table[1]        = UString::str_method->hashIgnoreCase();
   hpack_static_table[1].value  = UString::str_method_get;
   hpack_static_table[2].name   = UString::str_method;
   hpack_static_table[2].value  = UString::str_method_post;
   hpack_static_table[3].name   = UString::str_path;
    hash_static_table[3]        = UString::str_path->hashIgnoreCase();
   hpack_static_table[3].value  = UString::str_path_root;
   hpack_static_table[4].name   = UString::str_path;
   hpack_static_table[4].value  = UString::str_path_index;
   hpack_static_table[5].name   = UString::str_scheme;
    hash_static_table[5]        = UString::str_scheme->hashIgnoreCase();
   hpack_static_table[5].value  = UString::str_http;
   hpack_static_table[6].name   = UString::str_scheme;
   hpack_static_table[6].value  = UString::str_scheme_https;
   hpack_static_table[7].name   = UString::str_status;
    hash_static_table[7]        = UString::str_status->hashIgnoreCase();
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
    hash_static_table[14]       = UString::str_accept_charset->hashIgnoreCase();
   hpack_static_table[15].name  = UString::str_accept_encoding;
    hash_static_table[15]       = UString::str_accept_encoding->hashIgnoreCase();
   hpack_static_table[15].value = UString::str_accept_encoding_value;
   hpack_static_table[16].name  = UString::str_accept_language;
    hash_static_table[16]       = UString::str_accept_language->hashIgnoreCase();
   hpack_static_table[17].name  = UString::str_accept_ranges;
    hash_static_table[17]       = UString::str_accept_ranges->hashIgnoreCase();
   hpack_static_table[18].name  = UString::str_accept;
    hash_static_table[18]       = UString::str_accept->hashIgnoreCase();
   hpack_static_table[19].name  = UString::str_access_control_allow_origin;
    hash_static_table[19]       = UString::str_access_control_allow_origin->hashIgnoreCase();
   hpack_static_table[20].name  = UString::str_age;
    hash_static_table[20]       = UString::str_age->hashIgnoreCase();
   hpack_static_table[21].name  = UString::str_allow;
    hash_static_table[21]       = UString::str_allow->hashIgnoreCase();
   hpack_static_table[22].name  = UString::str_authorization;
    hash_static_table[22]       = UString::str_authorization->hashIgnoreCase();
   hpack_static_table[23].name  = UString::str_cache_control;
    hash_static_table[23]       = UString::str_cache_control->hashIgnoreCase();
   hpack_static_table[24].name  = UString::str_content_disposition;
    hash_static_table[24]       = UString::str_content_disposition->hashIgnoreCase();
   hpack_static_table[25].name  = UString::str_content_encoding;
    hash_static_table[25]       = UString::str_content_encoding->hashIgnoreCase();
   hpack_static_table[26].name  = UString::str_content_language;
    hash_static_table[26]       = UString::str_content_language->hashIgnoreCase();
   hpack_static_table[27].name  = UString::str_content_length;
    hash_static_table[27]       = UString::str_content_length->hashIgnoreCase();
   hpack_static_table[28].name  = UString::str_content_location;
    hash_static_table[28]       = UString::str_content_location->hashIgnoreCase();
   hpack_static_table[29].name  = UString::str_content_range;
    hash_static_table[29]       = UString::str_content_range->hashIgnoreCase();
   hpack_static_table[30].name  = UString::str_content_type;
    hash_static_table[30]       = UString::str_content_type->hashIgnoreCase();
   hpack_static_table[31].name  = UString::str_cookie;
    hash_static_table[31]       = UString::str_cookie->hashIgnoreCase();
   hpack_static_table[32].name  = UString::str_date;
    hash_static_table[32]       = UString::str_date->hashIgnoreCase();
   hpack_static_table[33].name  = UString::str_etag;
    hash_static_table[33]       = UString::str_etag->hashIgnoreCase();
   hpack_static_table[34].name  = UString::str_expect;
    hash_static_table[34]       = UString::str_expect->hashIgnoreCase();
   hpack_static_table[35].name  = UString::str_expires;
    hash_static_table[35]       = UString::str_expires->hashIgnoreCase();
   hpack_static_table[36].name  = UString::str_from;
    hash_static_table[36]       = UString::str_from->hashIgnoreCase();
   hpack_static_table[37].name  = UString::str_host;
    hash_static_table[37]       = UString::str_host->hashIgnoreCase();
   hpack_static_table[38].name  = UString::str_if_match;
    hash_static_table[38]       = UString::str_if_match->hashIgnoreCase();
   hpack_static_table[39].name  = UString::str_if_modified_since;
    hash_static_table[39]       = UString::str_if_modified_since->hashIgnoreCase();
   hpack_static_table[40].name  = UString::str_if_none_match;
    hash_static_table[40]       = UString::str_if_none_match->hashIgnoreCase();
   hpack_static_table[41].name  = UString::str_if_range;
    hash_static_table[41]       = UString::str_if_range->hashIgnoreCase();
   hpack_static_table[42].name  = UString::str_if_unmodified_since;
    hash_static_table[42]       = UString::str_if_unmodified_since->hashIgnoreCase();
   hpack_static_table[43].name  = UString::str_last_modified;
    hash_static_table[43]       = UString::str_last_modified->hashIgnoreCase();
   hpack_static_table[44].name  = UString::str_link;
    hash_static_table[44]       = UString::str_link->hashIgnoreCase();
   hpack_static_table[45].name  = UString::str_location;
    hash_static_table[45]       = UString::str_location->hashIgnoreCase();
   hpack_static_table[46].name  = UString::str_max_forwards;
    hash_static_table[46]       = UString::str_max_forwards->hashIgnoreCase();
   hpack_static_table[47].name  = UString::str_proxy_authenticate;
    hash_static_table[47]       = UString::str_proxy_authenticate->hashIgnoreCase();
   hpack_static_table[48].name  = UString::str_proxy_authorization;
    hash_static_table[48]       = UString::str_proxy_authorization->hashIgnoreCase();
   hpack_static_table[49].name  = UString::str_range;
    hash_static_table[49]       = UString::str_range->hashIgnoreCase();
   hpack_static_table[50].name  = UString::str_referer;
    hash_static_table[50]       = UString::str_referer->hashIgnoreCase();
   hpack_static_table[51].name  = UString::str_refresh;
    hash_static_table[51]       = UString::str_refresh->hashIgnoreCase();
   hpack_static_table[52].name  = UString::str_retry_after;
    hash_static_table[52]       = UString::str_retry_after->hashIgnoreCase();
   hpack_static_table[53].name  = UString::str_server;
    hash_static_table[53]       = UString::str_server->hashIgnoreCase();
   hpack_static_table[54].name  = UString::str_set_cookie;
    hash_static_table[54]       = UString::str_set_cookie->hashIgnoreCase();
   hpack_static_table[55].name  = UString::str_strict_transport_security;
    hash_static_table[55]       = UString::str_strict_transport_security->hashIgnoreCase();
   hpack_static_table[56].name  = UString::str_transfer_encoding;
    hash_static_table[56]       = UString::str_transfer_encoding->hashIgnoreCase();
   hpack_static_table[57].name  = UString::str_user_agent;
    hash_static_table[57]       = UString::str_user_agent->hashIgnoreCase();
   hpack_static_table[58].name  = UString::str_vary;
    hash_static_table[58]       = UString::str_vary->hashIgnoreCase();
   hpack_static_table[59].name  = UString::str_via;
    hash_static_table[59]       = UString::str_via->hashIgnoreCase();
   hpack_static_table[60].name  = UString::str_www_authenticate;
    hash_static_table[60]       = UString::str_www_authenticate->hashIgnoreCase();
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

   if (len) U_RETURN(false);

   U_RETURN(true);
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

         if ((*src++ & 128) == 0) U_RETURN_POINTER(src, unsigned char);
         }

      *pvalue = -1;
      }

   U_INTERNAL_DUMP("*pvalue = %d", *pvalue)

   U_RETURN_POINTER(src, unsigned char);
}

unsigned char* UHTTP2::hpackEncodeInt(unsigned char* dst, uint32_t value, uint8_t prefix_max)
{
   U_TRACE(0+256, "UHTTP2::hpackEncodeInt(%p,%u,%u)", dst, value, prefix_max)

#ifdef DEBUG
    int32_t _value;
   uint32_t  value1 = value;
   unsigned char* src = dst;
#endif

   if (value <= prefix_max) *dst++ |= value;
   else
      {
      value -= prefix_max;

      U_INTERNAL_ASSERT(value <= 0x0fffffff)

      for (*dst++ |= prefix_max; value >= 256; value >>= 8) *dst++ = 0x80 | value;

      *dst++ = value;
      }

#ifdef DEBUG
   (void) hpackDecodeInt(src, src+(dst-src), &_value, prefix_max);

   U_INTERNAL_ASSERT_EQUALS(value1, (uint32_t)_value)
#endif

   U_RETURN_POINTER(dst, unsigned char);
}

unsigned char* UHTTP2::hpackEncodeString(unsigned char* dst, const char* src, uint32_t len)
{
   U_TRACE(0+256, "UHTTP2::hpackEncodeString(%p,%.*S,%u)", dst, len, src, len)

   U_INTERNAL_ASSERT_MAJOR(len, 0)

#ifdef DEBUG
   uint32_t       len1 = len;
   const    char* src1 = src;
   unsigned char* dst1 = dst;
#endif

   if (len < 29)
      {
      // encode as-is

asis: *dst = '\0';
       dst = hpackEncodeInt(dst, len, (1<<7)-1);

      U_MEMCPY(dst, src, len);
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

      U_MEMCPY(dst, _dst_start, len);
      }

#ifdef DEBUG
   UString value;

   (void) hpackDecodeString(dst1, dst+len, &value);

   U_INTERNAL_ASSERT(value.equal(src1, len1))
#endif

   U_RETURN_POINTER(dst+len, unsigned char);
}

unsigned char* UHTTP2::hpackDecodeString(unsigned char* src, unsigned char* src_end, UString* pvalue)
{
   U_TRACE(0, "UHTTP2::hpackDecodeString(%p,%p,%p)", src, src_end, pvalue)

   int32_t len;
   bool is_huffman = ((*src & 0x80) != 0);

   src = hpackDecodeInt(src, src_end, &len, (1<<7)-1);

   if (len <= 0)
      {
err:  pvalue->clear();

      U_INTERNAL_DUMP("len = %u", len)

      U_RETURN_POINTER(src, unsigned char);
      }

   U_INTERNAL_DUMP("is_huffman = %b len = %u src = %#.*S", is_huffman, len, len, src)

   if (is_huffman == false)
      {
      if ((src + len) > src_end) goto err;

      (void) pvalue->replace((const char*)src, len);

      U_INTERNAL_DUMP("len = %u", len)

      U_RETURN_POINTER(src+len, unsigned char);
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

   U_RETURN_POINTER(src_end, unsigned char);
}

void UHTTP2::addHpackDynTblEntry(HpackDynamicTable* table, const UString& name, const UString& value)
{
   U_TRACE(1, "UHTTP2::addHpackDynTblEntry(%p,%V,%V)", table, name.rep, value.rep)

   U_INTERNAL_ASSERT(name)

   uint32_t size_add = name.size() + value.size() + HTTP2_HEADER_TABLE_ENTRY_SIZE_OFFSET;

   U_INTERNAL_DUMP("num_entries = %u entry_capacity = %u entry_start_index = %u hpack_size = %u size_add = %u hpack_capacity = %u hpack_max_capacity = %u",
                     table->num_entries, table->entry_capacity, table->entry_start_index, table->hpack_size, size_add, table->hpack_capacity, table->hpack_max_capacity)

   // adjust the size

   while (table->num_entries != 0 &&
          (table->hpack_size + size_add) > table->hpack_capacity)
      {
      evictHpackDynTblEntry(table);
      }

   if (table->num_entries == 0)
      {
      U_INTERNAL_ASSERT_EQUALS(table->hpack_size, 0)

      if (size_add > table->hpack_capacity)
         {
         U_DEBUG("HTTP decompressor literal with index not inserted due to size (%u > %u)", size_add, table->hpack_capacity)

         U_SRV_LOG("WARNING: HTTP decompressor literal with index not inserted due to size (%u > %u)", size_add, table->hpack_capacity);

         U_INTERNAL_DUMP("HTTP decompressor literal with index not inserted due to size (%u > %u)", size_add, table->hpack_capacity)

         return;
         }
      }

   table->hpack_size += size_add;

   // grow the entries if full

   if (table->num_entries == table->entry_capacity)
      {
      uint32_t new_capacity = table->num_entries * 2;

      if (new_capacity < 16) new_capacity = 16;

      HpackHeaderTableEntry* new_entries = (HpackHeaderTableEntry*) UMemoryPool::_malloc(&new_capacity, sizeof(HpackHeaderTableEntry));

      if (table->num_entries != 0)
         {
         uint32_t src_index = table->entry_start_index,
                  dst_index = 0;

         do {
            new_entries[dst_index] = table->entries[src_index];

            if (++src_index == table->entry_capacity) src_index = 0;
            }
         while (++dst_index != table->num_entries);
         }

      (void) U_SYSCALL(memset, "%p,%d,%u", new_entries + table->num_entries, 0, sizeof(HpackHeaderTableEntry) * (new_capacity - table->num_entries));

      if (table->entry_capacity) UMemoryPool::_free(table->entries, table->entry_capacity, sizeof(HpackHeaderTableEntry));

      table->entries           = new_entries;
      table->entry_capacity    = new_capacity;
      table->entry_start_index = 0;
      }

   table->num_entries++;

   U_INTERNAL_ASSERT_MINOR(table->num_entries, 65536)

   table->entry_start_index = (table->entry_start_index - 1 + table->entry_capacity) % table->entry_capacity;

   UHTTP2::HpackHeaderTableEntry* entry = table->entries + table->entry_start_index;

   U_NEW(UString, entry->name,  UString(name));
   U_NEW(UString, entry->value, UString(value));

   U_INTERNAL_DUMP("num_entries = %u entry_capacity = %u entry_start_index = %u hpack_size = %u hpack_capacity = %u hpack_max_capacity = %u",
                     table->num_entries, table->entry_capacity, table->entry_start_index, table->hpack_size, table->hpack_capacity, table->hpack_max_capacity)
}

void UHTTP2::decodeHeaders(unsigned char* ptr, unsigned char* endptr)
{
   U_TRACE(0, "UHTTP2::decodeHeaders(%#.*S)", endptr-ptr, ptr)

   U_INTERNAL_ASSERT_EQUALS(U_http_version, '2')

   // ------------------------------
   // U_http_info.uri
   // ....
   // U_http_info.nResponseCode
   // ....
   // ------------------------------
   U_HTTP_INFO_RESET(0);

   U_http_version = '2';

   UHashMap<UString>* table = &(pConnection->itable);

   decodeHeaders(table, &(pConnection->idyntbl), ptr, endptr);

   if (nerror == NO_ERROR)
      {
      U_INTERNAL_DUMP("Host            = %.*S", U_HTTP_HOST_TO_TRACE)
      U_INTERNAL_DUMP("Range           = %.*S", U_HTTP_RANGE_TO_TRACE)
      U_INTERNAL_DUMP("Accept          = %.*S", U_HTTP_ACCEPT_TO_TRACE)
      U_INTERNAL_DUMP("Cookie          = %.*S", U_HTTP_COOKIE_TO_TRACE)
      U_INTERNAL_DUMP("Referer         = %.*S", U_HTTP_REFERER_TO_TRACE)
      U_INTERNAL_DUMP("User-Agent      = %.*S", U_HTTP_USER_AGENT_TO_TRACE)
      U_INTERNAL_DUMP("Content-Type    = %.*S", U_HTTP_CTYPE_TO_TRACE)
      U_INTERNAL_DUMP("Accept-Language = %.*S", U_HTTP_ACCEPT_LANGUAGE_TO_TRACE)

      U_INTERNAL_DUMP("U_http_method_type = %B U_http_method_num = %d", U_http_method_type, U_http_method_num)

      if (U_http_info.clength)
         {
         U_INTERNAL_DUMP("U_http_info.clength = %u UHTTP::limit_request_body = %u", U_http_info.clength, UHTTP::limit_request_body)

         if (U_http_info.clength > UHTTP::limit_request_body)
            {
            nerror = ERROR_NOCLOSE;

            U_http_info.nResponseCode = HTTP_ENTITY_TOO_LARGE;

            UHTTP::setResponse();

            return;
            }

         if (continue100)
            {
            continue100 = false;

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

      U_INTERNAL_DUMP("U_http_is_accept_gzip = %b", U_http_is_accept_gzip)

      if (U_http_is_accept_gzip == false)
         {
         table->hash = hash_static_table[15]; // accept-encoding

         UString value = table->at(UString::str_accept_encoding->rep);

         if (value) setEncoding(value);
         }

      U_INTERNAL_DUMP("U_http_info.uri_len = %u", U_http_info.uri_len)

      if (U_http_info.uri_len == 0)
         {
         table->hash = hash_static_table[3]; // path

         UString value = table->at(UString::str_path->rep);

         setURI(U_STRING_TO_PARAM(value));
         }
      }
}

void UHTTP2::decodeHeaders(UHashMap<UString>* table, HpackDynamicTable* dyntbl, unsigned char* ptr, unsigned char* endptr)
{
   U_TRACE(0+256, "UHTTP2::decodeHeaders(%p,%p,%#.*S)", table, dyntbl, endptr-ptr, ptr)

   U_INTERNAL_ASSERT_EQUALS(U_http_version, '2')

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

   uint32_t sz;
   int32_t index;
   const char* ptr1;
   UString name, value;
   UHTTP2::HpackHeaderTableEntry* entry;
   bool value_is_indexed = false, binsert_dynamic_table = false;

   table->clear();

   while (ptr < endptr)
      {
#  ifdef DEBUG
       name.clear();
      value.clear();
#  endif

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)
      U_INTERNAL_ASSERT_EQUALS(binsert_dynamic_table, false)

      index = *ptr;

      U_INTERNAL_DUMP("index = (%u 0x%X) %B", index, index, index)

      if (index & 0x80) // (128: 10000000) - indexed header field representation
         {
         value_is_indexed = true;

         ptr = hpackDecodeInt(ptr, endptr, &index, (1<<7)-1); // (127: 01111111)

         U_INTERNAL_DUMP("index = %d value_is_indexed = %b binsert_dynamic_table = %b", index, value_is_indexed, binsert_dynamic_table)

         U_INTERNAL_ASSERT_MAJOR(index, 0)

         goto check2;
         }

      /**
       * Literal Header Field Representation
       *
       * header field names are provided either as a literal or by reference to an existing table entry, either from the static table or the dynamic table
       */

      if (index & 0x40) // (64: 01000000) - literal header field with incremental indexing
         {
         binsert_dynamic_table = true; // incremental indexing implicitly adds a new entry into the dynamic table

         ptr = hpackDecodeInt(ptr, endptr, &index, (1<<6)-1); // (63: 00111111)

         U_INTERNAL_DUMP("index = %d value_is_indexed = %b binsert_dynamic_table = %b", index, value_is_indexed, binsert_dynamic_table)

         U_INTERNAL_ASSERT(index >= 0)

         goto check1;
         }

      if (index & 0x20) // (32: 00100000) - context update
         {
         ptr = hpackDecodeInt(ptr, endptr, &index, (1<<5)-1); // (31: 00011111)

         U_INTERNAL_DUMP("hpack_capacity = %u hpack_max_capacity = %u index = %u", dyntbl->hpack_capacity, dyntbl->hpack_max_capacity, index)

         if (index != (int32_t)dyntbl->hpack_capacity)
            {
            if (index > (int32_t)dyntbl->hpack_max_capacity)
               {
               nerror = COMPRESSION_ERROR;

               return;
               }

            setHpackDynTblCapacity(dyntbl, index);
            }

         continue;
         }

      /**
       * Literal Header Field without Indexing
       *
       * a literal header field without indexing representation results in appending a header field to the decoded header list without altering the dynamic table
       */

      ptr = hpackDecodeInt(ptr, endptr, &index, (1<<4)-1); // (15: 00001111)

      U_INTERNAL_DUMP("index = %d value_is_indexed = %b binsert_dynamic_table = %b", index, value_is_indexed, binsert_dynamic_table)

      U_INTERNAL_ASSERT(index >= 0)

check1:
      if (index == 0) // not-existing name
         {
         U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

         ptr = hpackDecodeString(ptr, endptr, &name);

         if (name)
            {
            ptr = hpackDecodeString(ptr, endptr, &value);

            if (value)
               {
               table->hash = name.hashIgnoreCase();

               goto next;
               }
            }

         nerror = COMPRESSION_ERROR;

         return;
         }

check2:
      if (index >= HTTP2_HEADER_TABLE_OFFSET)
         {
         index -= HTTP2_HEADER_TABLE_OFFSET;

         U_INTERNAL_DUMP("index = %d dyntbl->num_entries = %u", index, dyntbl->num_entries)

         value_is_indexed      =
         binsert_dynamic_table = false;

         if (index < (int32_t)dyntbl->num_entries)
            {
            entry = getHpackDynTblEntry(dyntbl, index);

             name = *(entry->name);
            value = *(entry->value);

            table->hash = name.hashIgnoreCase();

            goto insert;
            }

         U_DUMP_CONTAINER(*dyntbl)
         U_DUMP_OBJECT_TO_TMP(*dyntbl, dtable.hpack)

#     if defined(U_STDCPP_ENABLE) && defined(DEBUG)
         continue;
#     endif

         nerror = COMPRESSION_ERROR;

         return;
         }

      // existing name

      U_INTERNAL_DUMP("dispatch_table[%d] = %p &&cdefault = %p", index, dispatch_table[index], &&cdefault)

      goto *((char*)&&cdefault + dispatch_table[index]);

case_38: // host

      name = *UString::str_host;

      goto host;

case_1: // authority (a.k.a. the Host header)

      name = *UString::str_authority;

host: U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)
      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      sz = value.size();

      if (sz == 0)
         {
         nerror = COMPRESSION_ERROR;

         return;
         }

      UHTTP::setHostname(value.data(), sz);

      table->hash = hash_static_table[37]; // host

      goto next;

case_2_3: // GET - POST

      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)

      if (value_is_indexed)
         {
         value_is_indexed = false;

         U_http_method_type = (index == 2 ? HTTP_GET : (U_http_method_num = 2, HTTP_POST));

         U_INTERNAL_DUMP("name = %V value = %V", hpack_static_table[index-1].name->rep, hpack_static_table[index-1].value->rep)
         }
      else
         {
         ptr = hpackDecodeString(ptr, endptr, &value);

         ptr1 = value.data();

         if (*ptr1 == '\0')
            {
            nerror = COMPRESSION_ERROR;

            return;
            }

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

         U_INTERNAL_DUMP("name = %V value = %V", hpack_static_table[index-1].name->rep, value.rep)
         }

      continue;

case_4_5: // / - /index.html

      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)

      name = *UString::str_path;

      // determine the value (if necessary)

      if (value_is_indexed)
         {
         value_is_indexed = false;

         value = *(index == 4 ? UString::str_path_root : UString::str_path_index);

         U_http_info.uri     = value.data();
         U_http_info.uri_len = value.size();

         U_INTERNAL_DUMP("URI = %.*S", U_HTTP_URI_TO_TRACE)
         }
      else
         {
         ptr = hpackDecodeString(ptr, endptr, &value);

         sz = value.size();

         if (sz == 0)
            {
            nerror = COMPRESSION_ERROR;

            return;
            }

         setURI(value.data(), sz);
         }
         
      table->hash = hash_static_table[3]; // path

      goto next;

case_6: // http

      U_INTERNAL_ASSERT(value_is_indexed)
      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)
      U_ASSERT_EQUALS(UServer_Base::csocket->isSSLActive(), false)

      U_INTERNAL_DUMP("name = %V value = %V", hpack_static_table[6-1].name->rep, hpack_static_table[6-1].value->rep)

      value_is_indexed = false;

      continue;

case_7: // https

      U_INTERNAL_ASSERT(value_is_indexed)
      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)
      U_ASSERT(((USSLSocket*)UServer_Base::csocket)->isSSL())

      U_INTERNAL_DUMP("name = %V value = %V", hpack_static_table[7-1].name->rep, hpack_static_table[7-1].value->rep)

      value_is_indexed = false;

      continue;

case_16: // accept-encoding: gzip, deflate

      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)

      name = *UString::str_accept_encoding;

      // determine the value (if necessary)

      if (value_is_indexed)
         {
         value_is_indexed = false;

         value = *UString::str_accept_encoding_value;
         }
      else
         {
         ptr = hpackDecodeString(ptr, endptr, &value);

         if (value.empty())
            {
            nerror = COMPRESSION_ERROR;

            return;
            }
         }

      setEncoding(value);

      table->hash = hash_static_table[15]; // accept_encoding

      goto next;

case_17: // accept-language

      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)

      name = *UString::str_accept_language;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      U_http_accept_language_len = value.size();

      if (U_http_accept_language_len == 0)
         {
         nerror = COMPRESSION_ERROR;

         return;
         }

      U_http_info.accept_language = value.data();

      U_INTERNAL_DUMP("Accept-Language: = %.*S", U_HTTP_ACCEPT_LANGUAGE_TO_TRACE)

      table->hash = hash_static_table[16]; // accept_language

      goto next;

case_19: // accept

      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)

      name = *UString::str_accept;

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      U_http_accept_len = value.size();

      if (U_http_accept_len == 0)
         {
         nerror = COMPRESSION_ERROR;

         return;
         }

      U_http_info.accept = value.data();

      U_INTERNAL_DUMP("Accept: = %.*S", U_HTTP_ACCEPT_TO_TRACE)

      table->hash = hash_static_table[18]; // accept

      goto next;

case_28: // content_length

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      name = *UString::str_content_length;

      ptr = hpackDecodeString(ptr, endptr, &value);

      if (value.empty())
         {
         nerror = COMPRESSION_ERROR;

         return;
         }

#  ifdef DEBUG
      if (bdecodeHeadersDebug == false)
#  endif
      {
      U_http_info.clength = (uint32_t) strtoul(value.data(), 0, 10);

      U_INTERNAL_DUMP("Content-Length: = %.*S U_http_info.clength = %u", U_STRING_TO_TRACE(value), U_http_info.clength)
      }

      table->hash = hash_static_table[27]; // content_length 

      goto next;

case_31: // content_type

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      name = *UString::str_content_type;

      ptr = hpackDecodeString(ptr, endptr, &value);

#  ifdef DEBUG
      if (bdecodeHeadersDebug == false)
#  endif
      {
      U_http_content_type_len = value.size();

      if (U_http_content_type_len == 0)
         {
         nerror = COMPRESSION_ERROR;

         return;
         }

      U_http_info.content_type = value.data();

      U_INTERNAL_DUMP("Content-Type(%u): = %.*S", U_http_content_type_len, U_HTTP_CTYPE_TO_TRACE)
      }

      table->hash = hash_static_table[30]; // content_type 

      goto next;

case_32: // cookie

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)
      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)

      name = *UString::str_cookie;

      ptr = hpackDecodeString(ptr, endptr, &value);

      sz = value.size();

      if (sz == 0)
         {
         nerror = COMPRESSION_ERROR;

         return;
         }

      if (U_http_info.cookie_len) value = UString(U_CAPACITY, "%.*s; %v", U_HTTP_COOKIE_TO_TRACE, value.rep);

      U_http_info.cookie     = value.data();
      U_http_info.cookie_len = value.size();

      U_INTERNAL_DUMP("Cookie(%u): = %.*S", U_http_info.cookie_len, U_HTTP_COOKIE_TO_TRACE)

      table->hash = hash_static_table[31]; // cookie 

      goto next;

case_35: // expect

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)
      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)

      ptr = hpackDecodeString(ptr, endptr, &value);

      if (value.empty())
         {
         nerror = COMPRESSION_ERROR;

         return;
         }

      U_INTERNAL_DUMP("name = %V value = %V", hpack_static_table[35-1].name->rep, value.rep)

      // NB: check for 'Expect: 100-continue' (as curl does)...

      continue100 = value.equal(U_CONSTANT_TO_PARAM("100-continue"));

      if (continue100 == false)
         {
         name = *UString::str_expect;

         table->hash = hash_static_table[34]; // expect 

         goto next;
         }

      continue;

case_40: // if-modified-since

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)
      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)

      name = *UString::str_if_modified_since;

      ptr = hpackDecodeString(ptr, endptr, &value);

      if (value.empty())
         {
         nerror = COMPRESSION_ERROR;

         return;
         }

      U_http_info.if_modified_since = UTimeDate::getSecondFromTime(value.data(), true);

      U_INTERNAL_DUMP("If-Modified-Since = %u", U_http_info.if_modified_since)

      table->hash = hash_static_table[39]; // if_modified_since 

      goto next;

case_50: // range 

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)
      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)

      name = *UString::str_range;

      ptr = hpackDecodeString(ptr, endptr, &value);

      U_http_range_len = value.size() - U_CONSTANT_SIZE("bytes=");

      if (U_http_range_len <= 0)
         {
         nerror = COMPRESSION_ERROR;

         return;
         }

      U_http_info.range = value.data() + U_CONSTANT_SIZE("bytes=");

      U_INTERNAL_DUMP("Range = %.*S", U_HTTP_RANGE_TO_TRACE)

      table->hash = hash_static_table[49]; // range 

      goto next;

case_51: // referer 

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)
      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)

      name = *UString::str_referer;

      ptr = hpackDecodeString(ptr, endptr, &value);

      U_http_info.referer_len = value.size();

      if (U_http_info.referer_len == 0)
         {
         nerror = COMPRESSION_ERROR;

         return;
         }

      U_http_info.referer = value.data();

      U_INTERNAL_DUMP("Referer(%u): = %.*S", U_http_info.referer_len, U_HTTP_REFERER_TO_TRACE)

      table->hash = hash_static_table[50]; // referer 

      goto next;

case_57: // transfer-encoding

      U_DEBUG("Transfer-Encoding is not supported in HTTP/2")

      U_SRV_LOG("WARNING: Transfer-Encoding is not supported in HTTP/2");

      U_INTERNAL_DUMP("Transfer-Encoding is not supported in HTTP/2")

      nerror = COMPRESSION_ERROR;

      return;

case_58: // user-agent

      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)
      U_INTERNAL_ASSERT_EQUALS(bdecodeHeadersDebug, false)

      name = *UString::str_user_agent;

      ptr = hpackDecodeString(ptr, endptr, &value);

      U_http_info.user_agent_len = value.size();

      if (U_http_info.user_agent_len == 0)
         {
         nerror = COMPRESSION_ERROR;

         return;
         }

      U_http_info.user_agent = value.data();

      U_INTERNAL_DUMP("User-Agent: = %.*S", U_HTTP_USER_AGENT_TO_TRACE)

      table->hash = hash_static_table[57]; // user_agent

      goto next;

cdefault:
      entry = hpack_static_table + --index;

      name = *(entry->name);

      table->hash = hash_static_table[index];

      U_INTERNAL_DUMP("table->hash = %u name = %V", table->hash, name.rep)

      if (table->hash == 0)
         {
         value_is_indexed = false;

         if (binsert_dynamic_table)
            {
            /*
            binsert_dynamic_table = false;
            addHpackDynTblEntry(dyntbl, name, UString::getStringNull());
            */

            nerror = COMPRESSION_ERROR;

            return;
            }

         continue;
         }

      // determine the value (if necessary)

      if (value_is_indexed)
         {
         value_is_indexed = false;

         if (entry->value) value = *(entry->value);
         else
            {
            U_INTERNAL_ASSERT_EQUALS(binsert_dynamic_table, false)

            nerror = COMPRESSION_ERROR;

            return;
            }
         }
      else
         {
         ptr = hpackDecodeString(ptr, endptr, &value);

         if (value.empty())
            {
            if (binsert_dynamic_table)
               {
               /*
               binsert_dynamic_table = false;
               addHpackDynTblEntry(dyntbl, name, value);
               */

               nerror = COMPRESSION_ERROR;

               return;
               }

            continue;
            }
         }

next:
      U_INTERNAL_ASSERT(name)
      U_INTERNAL_ASSERT(value)
      U_INTERNAL_ASSERT(table->hash)
      U_INTERNAL_ASSERT_EQUALS(value_is_indexed, false)

      if (binsert_dynamic_table)
         {
         binsert_dynamic_table = false;

         addHpackDynTblEntry(dyntbl, name, value);
         }

insert:
      U_INTERNAL_ASSERT(name)
      U_INTERNAL_ASSERT(value)
      U_INTERNAL_ASSERT(table->hash)

      table->insert(name, value); // add the decoded header to the header table

      U_INTERNAL_DUMP("name = %V value = %V", name.rep, value.rep)
      }

   U_DUMP_CONTAINER(*table)

   U_INTERNAL_DUMP("num_entries = %u entry_capacity = %u entry_start_index = %u hpack_size = %u hpack_capacity = %u hpack_max_capacity = %u",
                     dyntbl->num_entries, dyntbl->entry_capacity, dyntbl->entry_start_index, dyntbl->hpack_size, dyntbl->hpack_capacity, dyntbl->hpack_max_capacity)
}

void UHTTP2::openStream()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::openStream()")

   U_INTERNAL_ASSERT_POINTER(pStream)

   pStream->id = frame.stream_id;

   U_INTERNAL_DUMP("pConnection->max_processed_stream_id = %u pStream->id = %u", pConnection->max_processed_stream_id, pStream->id)

   if (pConnection->max_processed_stream_id < pStream->id) pConnection->max_processed_stream_id = pStream->id;

#ifdef DEBUG
   pConnection->ddyntbl.hpack_capacity     =
   pConnection->ddyntbl.hpack_max_capacity =
#endif
   pConnection->odyntbl.hpack_capacity     =
   pConnection->idyntbl.hpack_capacity     =
   pConnection->odyntbl.hpack_max_capacity =
   pConnection->idyntbl.hpack_max_capacity = settings.header_table_size;

   pStream->state = ((frame.flags & FLAG_END_STREAM) != 0 ? STREAM_STATE_HALF_CLOSED : STREAM_STATE_OPEN);

   U_DUMP("pStream->state = (%d, %s)", pStream->state, getStreamStatusDescription())
}

void UHTTP2::readFrame()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::readFrame()")

   U_INTERNAL_ASSERT_POINTER(pStream)
   U_INTERNAL_ASSERT_EQUALS(nerror, NO_ERROR)

   int32_t len;
   uint32_t sz;
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
         U_DEBUG("HTTP2 upgrade: User-Agent = %.*S", U_HTTP_USER_AGENT_TO_TRACE)

         U_INTERNAL_ASSERT(pConnection->max_open_stream_id <= frame.stream_id)

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

      goto err;
      }
#endif

   frame.length = ntohl(*(uint32_t*)ptr & 0x00ffffff) >> 8;

   U_INTERNAL_DUMP("frame.length = %#.4S", &frame.length)

   frame.flags     = ptr[4];
   frame.stream_id = ntohl(*(uint32_t*)(ptr+5) & 0x7fffffff);

   U_DUMP("frame { length = %d stream_id = %d type = (%d, %s) flags = %d %B } = %#.*S", frame.length,
           frame.stream_id, frame.type, getFrameTypeDescription(frame.type), frame.flags, frame.flags, frame.length, ptr + HTTP2_FRAME_HEADER_SIZE)

   U_INTERNAL_ASSERT_MINOR(frame.length, settings.max_frame_size)

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

   frame.payload = (unsigned char*) UClientImage_Base::rbuffer->c_pointer(UClientImage_Base::rstart);
                                                                          UClientImage_Base::rstart += (uint32_t)frame.length;

   if (frame.type == WINDOW_UPDATE)
      {
      uint32_t window_size_increment = ntohl(*(uint32_t*)frame.payload) & 0x7fffffff;

      U_INTERNAL_DUMP("pConnection->out_window = %d window_size_increment = %u", pConnection->out_window, window_size_increment)

#  ifdef DEBUG
      if (frame.length != 4 ||
          window_size_increment == 0)
         {
         nerror = FRAME_SIZE_ERROR;

         goto err;
         }
#  endif

      pConnection->out_window += window_size_increment;

#  ifdef DEBUG
      if ((uint32_t)pConnection->out_window > HTTP2_MAX_WINDOW_SIZE)
         {
         nerror = FLOW_CONTROL_ERROR;

         goto err;
         }
#  endif

      goto loop;
      }

   if (frame.stream_id == 0)
      {
      if (frame.type == GOAWAY)
         {
         uint32_t error = ntohl(*(uint32_t*)(frame.payload+4));

         const char* descr = getFrameErrorCodeDescription(error);

         U_DEBUG("Received GOAWAY frame with error (%u, %s)", error, descr)

         U_SRV_LOG("WARNING: Received GOAWAY frame with error (%u, %s)", error, descr);

         U_INTERNAL_DUMP("GOAWAY: Last-Stream-ID = %u", ntohl(*(uint32_t*)frame.payload) & 0x7fffffff)

         goto loop;
         }

      if (frame.type == PING)
         {
         if (frame.length != 8)
            {
            nerror = PROTOCOL_ERROR;

            goto err;
            }

         if (frame.flags == 0)
            {
            U_DEBUG("Received PING frame with data (%#.8S)", frame.payload)

            U_SRV_LOG("WARNING: Received PING frame with data (%#.8S)", frame.payload);

            char buffer[HTTP2_FRAME_HEADER_SIZE+8] = { 0, 0, 8,    // frame size
                                                       PING,       // ping frame
                                                       FLAG_ACK,   // ack flag
                                                       0, 0, 0, 0, // stream id
                                                       0, 0, 0, 0, 0, 0, 0, 0 };

            if (*(uint64_t*)frame.payload) U_MEMCPY(buffer+HTTP2_FRAME_HEADER_SIZE, frame.payload, 8);

            if (USocketExt::write(UServer_Base::csocket, buffer, sizeof(buffer), UServer_Base::timeoutMS) != sizeof(buffer))
               {
               nerror = FLOW_CONTROL_ERROR;

               goto err;
               }
            }

         goto loop;
         }

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

            pConnection->out_window = settings.initial_window_size; // sender flow control window

            U_INTERNAL_DUMP("pConnection->out_window = %d pConnection->inp_window = %d", pConnection->out_window, pConnection->inp_window)

            if (UClientImage_Base::rbuffer->size() == UClientImage_Base::rstart) return;
            }
         else
            {
            uint32_t old_initial_window_size = pConnection->peer_settings.initial_window_size;

            U_INTERNAL_DUMP("old_initial_window_size = %u settings_ack = %b", old_initial_window_size, settings_ack)

            if (updateSetting(frame.payload, frame.length) == false ||
                USocketExt::write(UServer_Base::csocket, U_CONSTANT_TO_PARAM(HTTP2_SETTINGS_ACK), UServer_Base::timeoutMS) != U_CONSTANT_SIZE(HTTP2_SETTINGS_ACK))
               {
               nerror = PROTOCOL_ERROR;

               goto err;
               }

            // receiver flow control window

            if (pConnection->peer_settings.initial_window_size) pConnection->inp_window += pConnection->peer_settings.initial_window_size - old_initial_window_size;
            else
               {
               if (settings_ack == false)
                  {
                  U_INTERNAL_ASSERT_EQUALS(old_initial_window_size, 0)

                  pConnection->inp_window = HTTP2_DEFAULT_WINDOW_SIZE;
                  }
               }

            U_INTERNAL_DUMP("pConnection->inp_window = %d", pConnection->inp_window)

            HpackDynamicTable* dyntbl = &(pConnection->odyntbl);

            uint32_t value = pConnection->peer_settings.header_table_size;

            if (value != dyntbl->hpack_capacity)
               {
               dyntbl->hpack_max_capacity = value;

               setHpackDynTblCapacity(dyntbl, value);
               }
            }

         goto loop;
         }

      nerror = PROTOCOL_ERROR;

      goto err;
      }

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

      if ((frame.flags & FLAG_END_STREAM) != 0) pStream->state = STREAM_STATE_HALF_CLOSED;

      return;
      }

   U_INTERNAL_DUMP("pStream->id = %u", pStream->id)

   if (frame.type == RST_STREAM)
      {
      nerror = ERROR_NOCLOSE;

      uint32_t error = ntohl(*(uint32_t*)frame.payload);

      const char* descr = getFrameErrorCodeDescription(error);

      U_DEBUG("Received RST_STREAM frame for stream %u with error (%u, %s)", frame.stream_id, error, descr)

      U_SRV_LOG("WARNING: Received RST_STREAM frame for stream %u with error (%u, %s)", frame.stream_id, error, descr);

      if (pStream->id == frame.stream_id ||
          setStream())
         {
         U_INTERNAL_ASSERT_EQUALS(pStream->state, STREAM_STATE_HALF_CLOSED)

         pStream->state = STREAM_STATE_CLOSED;
         }

      return;
      }

   if (pStream->id != frame.stream_id &&
       setStream() == false)
      {
      nerror = PROTOCOL_ERROR;

      goto err;
      }

   if ((frame.flags & FLAG_END_STREAM) != 0) pStream->state = STREAM_STATE_HALF_CLOSED;

   if (frame.type == DATA)
      {
      manageData();

      if (nerror != NO_ERROR) goto err;

      if (pStream->state != STREAM_STATE_HALF_CLOSED) goto loop; // we must wait for other DATA frames for the same stream...

      return;
      }

   nerror = PROTOCOL_ERROR;

err:
   UClientImage_Base::rstart = 0;

   UClientImage_Base::rbuffer->setEmptyForce();
}

void UHTTP2::manageHeaders()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::manageHeaders()")

   uint32_t padlen;
   unsigned char* ptr;
   unsigned char* endptr = (ptr = frame.payload) + frame.length;

   /**
    * TODO
    *
    * char     priority_weight;     // 0 if not set
    * bool     priority_exclusive;
    * uint32_t priority_dependency; // 0 if not set
    */

   if ((frame.flags & FLAG_PADDED) == 0) padlen = 0;
   else
      {
      padlen = *ptr++;

      U_INTERNAL_DUMP("padlen = %u", padlen)

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
      /**
       * priority_weight     = 0;
       * priority_exclusive  = false;
       * priority_dependency = 0;
       */
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

      U_INTERNAL_DUMP("u4 = %d", ntohl(*(uint32_t*)ptr))

      /**
       * uint32_t u4 =  ntohl(*(uint32_t*)ptr);
       *                                  ptr += 4;
       * priority_weight     = (uint16_t)*ptr++ + 1;
       * priority_exclusive  = (u4 >> 31) != 0;
       * priority_dependency =  u4 & 0x7fffffff;
       */

      ptr += 5;
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
          (uint32_t)frame.stream_id != pStream->id)
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

void UHTTP2::manageData()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::manageData()")

   // Send Window Update if current window size is not sufficient

   U_DUMP("pStream->id = %u pStream->state = (%d, %s) pConnection->inp_window = %d frame.length = %u",
           pStream->id,     pStream->state, getStreamStatusDescription(), pConnection->inp_window, frame.length)

   if (pConnection->inp_window <= (int32_t)frame.length)
      {
      char buffer[HTTP2_FRAME_HEADER_SIZE+4] = { 0, 0, 4,       // frame size
                                                 WINDOW_UPDATE, // header frame
                                                 FLAG_NONE,     // flags
                                                 0, 0, 0, 0,    // stream id
                                                 0, 0, 0, 0 };

      char* ptr = buffer;

      *(uint32_t*)(ptr+9)  = htonl(HTTP2_MAX_WINDOW_SIZE - pConnection->inp_window);
   //              ptr[9] &= ~(0x01 << 7);

      if (USocketExt::write(UServer_Base::csocket, buffer, sizeof(buffer), UServer_Base::timeoutMS) != sizeof(buffer))
         {
         nerror = FLOW_CONTROL_ERROR;

         return;
         }

      pConnection->inp_window = HTTP2_MAX_WINDOW_SIZE;
      }

   pConnection->inp_window -= frame.length;

   unsigned char* ptr = frame.payload;

   if ((frame.flags & FLAG_PADDED) != 0)
      {
      uint32_t padlen = *ptr;

#  ifdef DEBUG
      U_INTERNAL_DUMP("frame.length = %u padlen = %u", frame.length, padlen)

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

void UHTTP2::sendGoAway()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::sendGoAway()")

   U_INTERNAL_ASSERT_POINTER(pConnection)
   U_INTERNAL_ASSERT(UServer_Base::csocket->isOpen())

   char buffer[HTTP2_FRAME_HEADER_SIZE+8] = { 0, 0, 8,    // frame size
                                              GOAWAY,     // header frame
                                              FLAG_NONE,  // flags
                                              0, 0, 0, 0, // stream id
                                              0, 0, 0, 0,
                                              0, 0, 0, 0 };
   char* ptr = buffer;

// *(uint32_t*)(ptr+ 5) = 0;
   *(uint32_t*)(ptr+ 9) = htonl(pConnection->max_processed_stream_id);
   *(uint32_t*)(ptr+13) = htonl(nerror);

   const char* descr = getFrameErrorCodeDescription(nerror);

   U_DEBUG("Sent GOAWAY frame with error (%u, %s)", nerror, descr)

   U_SRV_LOG("WARNING: Sent GOAWAY frame with error (%u, %s)", nerror, descr);

   pConnection->state = CONN_STATE_IS_CLOSING;

   (void) USocketExt::write(UServer_Base::csocket, buffer, sizeof(buffer), UServer_Base::timeoutMS);
}

__pure int32_t UHTTP2::getIndexStaticTable(const UString& key)
{
   U_TRACE(0, "UHTTP2::getIndexStaticTable(%V)", key.rep)

   int32_t index = 0;

   const char* keyp = key.data();

   switch (u_get_unalignedp32(keyp))
      {
      case U_MULTICHAR_CONSTANT32('C','o','n','t'):
         {
         keyp += 7;

         switch (u_get_unalignedp32(keyp))
            {
            case U_MULTICHAR_CONSTANT32('-','D','i','s'): index =  25; break; // content-disposition
            case U_MULTICHAR_CONSTANT32('-','E','n','c'): index = -26; break; // content-encoding
            case U_MULTICHAR_CONSTANT32('-','L','a','n'): index =  27; break; // content-language
            case U_MULTICHAR_CONSTANT32('-','L','e','n'): index =  28; break; // content-length
            case U_MULTICHAR_CONSTANT32('-','L','o','c'): index =  29; break; // content-location
            case U_MULTICHAR_CONSTANT32('-','R','a','n'): index =  30; break; // content-range
            case U_MULTICHAR_CONSTANT32('-','T','y','p'): index =  31; break; // content-type 

            default:
               {
               U_ASSERT(key == U_STRING_FROM_CONSTANT("Content-Style-Type") ||
                        key == U_STRING_FROM_CONSTANT("Content-Script-Type"))

               index = -1;
               }
            }
         }
      break;

      case U_MULTICHAR_CONSTANT32('A','c','c','e'):
         {
         keyp += 6;

         if (u_get_unalignedp32(keyp) == U_MULTICHAR_CONSTANT32('-','R','a','n')) index = -18; // accept-ranges
         else
            {
            U_INTERNAL_ASSERT_EQUALS(key, U_STRING_FROM_CONSTANT("Access-Control-Allow-Origin"))

            index = 20; // access-control-allow-origin
            }
         }
      break;

      case U_MULTICHAR_CONSTANT32('C','a','c','h'): index = -24; break; // cache-control
      case U_MULTICHAR_CONSTANT32('E','t','a','g'): index =  34; break; // etag
      case U_MULTICHAR_CONSTANT32('E','x','p','i'): index =  36; break; // expires
      case U_MULTICHAR_CONSTANT32('L','a','s','t'): index =  44; break; // last-modified
      case U_MULTICHAR_CONSTANT32('L','i','n','k'): index =  45; break; // link
      case U_MULTICHAR_CONSTANT32('L','o','c','a'): index =  46; break; // location
      case U_MULTICHAR_CONSTANT32('P','r','o','x'): index =  48; break; // proxy-authenticate
      case U_MULTICHAR_CONSTANT32('R','e','f','r'): index =  52; break; // refresh
      case U_MULTICHAR_CONSTANT32('R','e','t','r'): index =  53; break; // retry-after
      case U_MULTICHAR_CONSTANT32('S','t','r','i'): index = -56; break; // strict-transport-security
      case U_MULTICHAR_CONSTANT32('V','a','r','y'): index = -59; break; // vary
      case U_MULTICHAR_CONSTANT32('W','W','W','-'): index =  61; break; // www-authenticate 

      default:
         {
              if (key.equalnocase(U_CONSTANT_TO_PARAM("Via"))) index = 60; // via
         else if (key.equalnocase(U_CONSTANT_TO_PARAM("Age"))) index = 21; // age
         }
      break;
      }

   U_RETURN(index);
}

unsigned char* UHTTP2::hpackEncodeHeader(unsigned char* dst, HpackDynamicTable* dyntbl, UString& key, const UString& value, UVector<UString>* pvec)
{
   U_TRACE(0, "UHTTP2::hpackEncodeHeader(%p,%p,%V,%V,%p)", dst, dyntbl, key.rep, value.rep, pvec)

   U_INTERNAL_ASSERT_EQUALS(u_isBinary((unsigned char*)U_STRING_TO_PARAM(key)), false)
   U_INTERNAL_ASSERT_EQUALS(u_isBinary((unsigned char*)U_STRING_TO_PARAM(value)), false)

   int32_t index = getIndexStaticTable(key);

   if (index < 0) // literal header field with incremental indexing
      {
      U_INTERNAL_ASSERT_DIFFERS(index, -1)

      if (pvec)
         {
         pvec->push(key);
         pvec->push(value);

         return dst;
         }

      *dst = 0x40;
       dst = hpackEncodeInt(dst, -index, (1<<6)-1);

      addHpackDynTblEntry(dyntbl, key, value);
      }
   else
      {
      // literal header field without indexing

      *dst = 0x10;

      if (index) dst = hpackEncodeInt(dst, index, (1<<4)-1);
      else
         {
         if (u__isupper(key.c_char(0))) key = UStringExt::tolower(key);

         dst = hpackEncodeString(++dst, U_STRING_TO_PARAM(key));
         }
      }

   return hpackEncodeString(dst, U_STRING_TO_PARAM(value));
}

void UHTTP2::handlerResponse()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::handlerResponse()")

            char* ptr;
   unsigned char* dst;
   UVector<UString> vdyntbl;
   HpackDynamicTable* dyntbl = &(pConnection->odyntbl);
   bool bclear_body = false, bfirst = (dyntbl->num_entries == 0);

   uint32_t sz0 = pConnection->peer_settings.max_frame_size,
            sz1 = UHTTP::set_cookie->size(),
            sz2 = UHTTP::ext->size(),
            sz3 = UClientImage_Base::body->size(),
            sz  = 300U + sz1 + sz2;

   if (sz3)
      {
      U_INTERNAL_DUMP("UClientImage_Base::body->size() = %u pConnection->peer_settings.max_frame_size = %u pConnection->out_window = %d", sz3, sz0, pConnection->out_window)

      pConnection->out_window -= sz3;

      if (pConnection->out_window <= 0)
         {
         // TODO: we must wait for a Window Update frame if current window size is not sufficient (we need something like a pending data frame to write)...

         U_DEBUG("Current window size %d is not sufficient for data size %u", pConnection->out_window+sz3, sz3)

         U_SRV_LOG("WARNING: Current window size %d is not sufficient for data size %u", pConnection->out_window+sz3, sz3);
         }

      if (sz3 > sz0)
         {
         sz += sz3;

         bclear_body = true;  
         }
      }

   UClientImage_Base::wbuffer->setBuffer(sz);

   ptr = UClientImage_Base::wbuffer->data();
   dst = (unsigned char*)ptr + (sz = (HTTP2_FRAME_HEADER_SIZE+1));

   U_INTERNAL_DUMP("ext(%u) = %#V", sz2, UHTTP::ext->rep)

   if (U_http_info.nResponseCode == HTTP_NOT_IMPLEMENTED ||
       U_http_info.nResponseCode == HTTP_OPTIONS_RESPONSE)
      {
      UClientImage_Base::setCloseConnection();

      ptr[HTTP2_FRAME_HEADER_SIZE] = 0x80 | 8; // HTTP_OK

      /**
       * --------------------------------------------------------
       * literal header field without indexing - indexed name
       * -------------------------------------------------------- 
       * *dst = 0x10;
       *  dst = hpackEncodeInt(dst, 22, (1<<4)-1);
       * -------------------------------------------------------- 
       */

      u_put_unalignedp16(dst, U_MULTICHAR_CONSTANT16(0x1f,0x07));
                         dst += 2;

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
               u_put_unalignedp32(ptr+HTTP2_FRAME_HEADER_SIZE, U_MULTICHAR_CONSTANT32(0x08,0x03,0x30+(U_http_info.nResponseCode / 100),'\0'));
                      U_NUM2STR16(ptr+HTTP2_FRAME_HEADER_SIZE+3,                                      U_http_info.nResponseCode % 100);

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
    */

#if defined(U_LINUX) && defined(ENABLE_THREAD) && defined(U_LOG_DISABLE) && !defined(USE_LIBZ)
   U_INTERNAL_ASSERT_POINTER(u_pthread_time)
   U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::iov_vec[1].iov_base, ULog::ptr_shared_date->date3)
#else
   U_INTERNAL_ASSERT_EQUALS(UClientImage_Base::iov_vec[1].iov_base, ULog::date.date3)

   ULog::updateDate3();
#endif

   UString date((void*)(((char*)UClientImage_Base::iov_vec[1].iov_base)+6), 29);

   U_INTERNAL_DUMP("num_entries = %u entry_capacity = %u entry_start_index = %u hpack_size = %u hpack_capacity = %u hpack_max_capacity = %u",
                     dyntbl->num_entries, dyntbl->entry_capacity, dyntbl->entry_start_index, dyntbl->hpack_size, dyntbl->hpack_capacity, dyntbl->hpack_max_capacity)

   if (bfirst)
      {
      /**
       * -------------------------------------------------
       * *dst = 0x40;
       *  dst = hpackEncodeInt(dst, 54, (1<<6)-1);
       *  dst = hpackEncodeString(dst, U_CONSTANT_TO_PARAM("ULib"));
       * -------------------------------------------------
       */

      u_put_unalignedp64(dst, U_MULTICHAR_CONSTANT64(0x76,0x04,0x55,0x4C,0x69,0x62,0x61,'\0'));

      /**
       * -------------------------------------------------
       * *dst = 0x40;
       *  dst = hpackEncodeInt(dst, 33, (1<<6)-1);
       * -------------------------------------------------
       */

      dst = hpackEncodeString(dst+7, date.data(), 29); // Date: Wed, 20 Jun 2012 11:43:17 GMT\r\nServer: ULib\r\nConnection: close\r\n

      addHpackDynTblEntry(dyntbl, *UString::str_server, *UString::str_ULib);
      addHpackDynTblEntry(dyntbl, *UString::str_date,   date);
      }

   if (sz1)
      {
      /**
       * --------------------------------------------------------
       * literal header field without indexing - indexed name
       * -------------------------------------------------------- 
       * *dst = 0x10;
       *  dst = hpackEncodeInt(dst, 55, (1<<4)-1);
       * -------------------------------------------------------- 
       */

      u_put_unalignedp16(dst, U_MULTICHAR_CONSTANT16(0x1f,0x28));
                         dst += 2;

      dst = hpackEncodeString(dst, UHTTP::set_cookie->data(), sz1);

      UHTTP::set_cookie->setEmpty();

      UClientImage_Base::setRequestNoCache();
      }

   if (sz2)
      {
      UString row, key;
      UVector<UString> vext(20);

      for (uint32_t i = 0, n = vext.split(*UHTTP::ext, U_CRLF); i < n; ++i)
         {
         row = vext[i];

         uint32_t pos = row.find_first_of(':');

         key = row.substr(0U, pos);
         dst = hpackEncodeHeader(dst, dyntbl, key, row.substr(pos+2), bfirst ? 0 : &vdyntbl); 
         }
      }
   else // content-length: 0
      {
      /**
       * --------------------------------------------------------
       * literal header field without indexing - indexed name
       * --------------------------------------------------------
       * *dst = 0x10;
       *  dst = hpackEncodeInt(dst, 28, (1<<4)-1);
       *  dst = hpackEncodeString(dst, U_CONSTANT_TO_PARAM("0"));
       * --------------------------------------------------------
       */

      u_put_unalignedp32(dst, U_MULTICHAR_CONSTANT32(0x1f,0x0d,0x01,0x30));
                         dst += 4;
      }

   if (bfirst == false)
      {
      UString key;
      bool bdate   = false,
           bserver = false;

      U_DUMP_CONTAINER(*dyntbl)

      for (uint32_t i = dyntbl->entry_start_index, e = dyntbl->num_entries; e != 0; --e)
         {
         HpackHeaderTableEntry* entry = dyntbl->entries + i;

         const UString* pname  = entry->name;
         const UString* pvalue = entry->value;

         uint32_t index = i - dyntbl->entry_start_index;

         U_INTERNAL_DUMP("entry[%u]->name = %V entry[%u]->value = %V", index, pname->rep, index, pvalue->rep)

         if (bdate == false &&
             pname->equal(*UString::str_date))
            {
            bdate = true;

            const char* ptr1 =    date.c_pointer(U_CONSTANT_SIZE("Wed, 20 Jun 2012 "));
            const char* ptr2 = pvalue->c_pointer(U_CONSTANT_SIZE("Wed, 20 Jun 2012 "));

            if (u_get_unalignedp64(ptr1) == u_get_unalignedp64(ptr2))
               {
               *dst = 0x80;
                dst = hpackEncodeInt(dst, HTTP2_HEADER_TABLE_OFFSET+index, (1<<7)-1);
               }
            else
               {
               /**
                * Literal Header Field without Indexing
                */

               *dst = 0x10;
                dst = hpackEncodeInt(dst, 33, (1<<4)-1);
                dst = hpackEncodeString(dst, date.data(), 29); // Date: Wed, 20 Jun 2012 11:43:17 GMT\r\nServer: ULib\r\nConnection: close\r\n
               }
            }
         else if (bserver == false &&
                  pname->equal(*UString::str_server))
            {
            bserver = true;

            *dst = 0x80;
             dst = hpackEncodeInt(dst, HTTP2_HEADER_TABLE_OFFSET+index, (1<<7)-1);
            }
         else
            {
            U_DUMP_CONTAINER(vdyntbl)

            for (int32_t j = 0, n = vdyntbl.size(); j < n; j += 2)
               {
               key = vdyntbl[j];

               U_INTERNAL_DUMP("key = %V", key.rep)

               if (pname->equal(key))
                  {
#              ifdef DEBUG
                  UString value = vdyntbl[j+1];

                  U_INTERNAL_DUMP("value = %V", value.rep)

                  U_INTERNAL_ASSERT_EQUALS(*pvalue, value)
                  U_ASSERT_EQUALS(*(getHpackDynTblEntry(dyntbl, index)->name),    key)
                  U_ASSERT_EQUALS(*(getHpackDynTblEntry(dyntbl, index)->value), value)
#              endif

                  vdyntbl.erase(j, j+2);

                  *dst = 0x80;
                   dst = hpackEncodeInt(dst, HTTP2_HEADER_TABLE_OFFSET+index, (1<<7)-1);

                  break;
                  }
               }
            }

         if (++i == dyntbl->entry_capacity) i = 0;
         }

      U_DUMP_CONTAINER(vdyntbl)

      for (int32_t i = 0, n = vdyntbl.size(); i < n; i += 2)
         {
         key = vdyntbl[i];
         dst = hpackEncodeHeader(dst, dyntbl, key, vdyntbl[i+1]); 
         }
      }

   U_INTERNAL_ASSERT_EQUALS(ptr, UClientImage_Base::wbuffer->data())

   sz = (dst - (unsigned char*)ptr) - HTTP2_FRAME_HEADER_SIZE;

   U_INTERNAL_DUMP("sz = %u pConnection->peer_settings.max_frame_size = %u", sz, sz0)

   if (sz <= sz0)
      {
      *(uint32_t*) ptr    = htonl(sz << 8);
                   ptr[3] = HEADERS;
                   ptr[4] = FLAG_END_HEADERS | (sz3 == 0); // FLAG_END_STREAM (1)
      *(uint32_t*)(ptr+5) = htonl(pStream->id);

      U_DUMP("frame header response { length = %d stream_id = %d type = (%d, %s) flags = %d } = %#.*S", ntohl(*(uint32_t*)ptr & 0x00ffffff) >> 8,
                  ntohl(*(uint32_t*)(ptr+5) & 0x7fffffff), ptr[3], getFrameTypeDescription(ptr[3]), ptr[4], ntohl(*(uint32_t*)ptr & 0x00ffffff) >> 8, ptr + HTTP2_FRAME_HEADER_SIZE)
      }
   else
      {
      sz1 = sz-sz0;

      U_INTERNAL_ASSERT_MINOR(sz1, sz0)

      *(uint32_t*) ptr    = htonl(sz0 << 8);
                   ptr[3] = HEADERS;
                   ptr[4] = 0;
      *(uint32_t*)(ptr+5) = htonl(pStream->id);

      U_DUMP("frame header response { length = %d stream_id = %d type = (%d, %s) flags = %d } = %#.*S", ntohl(*(uint32_t*)ptr & 0x00ffffff) >> 8,
                  ntohl(*(uint32_t*)(ptr+5) & 0x7fffffff), ptr[3], getFrameTypeDescription(ptr[3]), ptr[4], ntohl(*(uint32_t*)ptr & 0x00ffffff) >> 8, ptr + HTTP2_FRAME_HEADER_SIZE)

      // NB: we must insert a continuation frame in the raw headers data...

      char* ptr0 = ptr + sz0;

#  ifdef U_APEX_ENABLE
      (void) U_SYSCALL(apex_memmove, "%p,%p,%u", ptr0+HTTP2_FRAME_HEADER_SIZE, ptr0, sz1); // void* dest, const void* src, ...
#  else
      (void) U_SYSCALL(     memmove, "%p,%p,%u", ptr0+HTTP2_FRAME_HEADER_SIZE, ptr0, sz1); // void* dest, const void* src, ...
#  endif

      *(uint32_t*) ptr0   = htonl(sz1 << 8);
                   ptr0[3] = CONTINUATION;
                   ptr0[4] = FLAG_END_HEADERS | (sz3 == 0); // FLAG_END_STREAM (1)
      *(uint32_t*)(ptr0+5) = htonl(pStream->id);

       sz += HTTP2_FRAME_HEADER_SIZE;
      dst += HTTP2_FRAME_HEADER_SIZE;

      U_DUMP("frame header response { length = %d stream_id = %d type = (%d, %s) flags = %d } = %#.*S", ntohl(*(uint32_t*)ptr & 0x00ffffff) >> 8,
                  ntohl(*(uint32_t*)(ptr0+5) & 0x7fffffff), ptr0[3], getFrameTypeDescription(ptr0[3]), ptr0[4], ntohl(*(uint32_t*)ptr0 & 0x00ffffff) >> 8, ptr0 + HTTP2_FRAME_HEADER_SIZE)
      }

   sz += HTTP2_FRAME_HEADER_SIZE;

   if (sz3)
      {
      char* ptr0;

      U_INTERNAL_DUMP("sz3 = %u pConnection->peer_settings.max_frame_size = %u bclear_body = %b", sz3, sz0, bclear_body)

      if (sz3 <= sz0)
         {
         U_INTERNAL_ASSERT_EQUALS(bclear_body, false)

         goto next;
         }

      U_INTERNAL_ASSERT(bclear_body)

      ptr0 = UClientImage_Base::body->data();

      do {
         // NB: we must add a data frame plus the raw data from UClientImage_Base::body...

         *(uint32_t*) dst    = htonl(sz0 << 8);
                      dst[3] = DATA;
                      dst[4] = 0;
         *(uint32_t*)(dst+5) = htonl(pStream->id);

         U_DUMP("frame data response { length = %d stream_id = %d type = (%d, %s) flags = %d }", ntohl(*(uint32_t*)dst & 0x00ffffff) >> 8,
                     ntohl(*(uint32_t*)(dst+5) & 0x7fffffff), dst[3], getFrameTypeDescription(dst[3]), dst[4])

         dst += HTTP2_FRAME_HEADER_SIZE;

         U_MEMCPY(dst, ptr0,   sz0);
                       ptr0 += sz0;
                        dst += sz0;
         }
      while ((sz3 -= sz0) > sz0);

      U_INTERNAL_DUMP("sz3 = %u", sz3)

      if (sz3)
         {
         sz = (dst - (unsigned char*)ptr) + sz3;

         U_MEMCPY(dst+HTTP2_FRAME_HEADER_SIZE, ptr0, sz3);

next:    sz += HTTP2_FRAME_HEADER_SIZE;

         *(uint32_t*) dst    = htonl(sz3 << 8);
                      dst[3] = DATA;
                      dst[4] = FLAG_END_STREAM;
         *(uint32_t*)(dst+5) = htonl(pStream->id);
         }

      if (bclear_body) UClientImage_Base::body->clear();

      U_DUMP("frame data response { length = %d stream_id = %d type = (%d, %s) flags = %d }", ntohl(*(uint32_t*)dst & 0x00ffffff) >> 8,
                  ntohl(*(uint32_t*)(dst+5) & 0x7fffffff), dst[3], getFrameTypeDescription(dst[3]), dst[4])
      }

   UClientImage_Base::wbuffer->size_adjust(sz);

   UClientImage_Base::setNoHeaderForResponse();
}

void UHTTP2::clearHpackDynTbl(HpackDynamicTable* table)
{
   U_TRACE(0, "UHTTP2::clearHpackDynTbl(%p)", table)

   if (table->num_entries)
      {
      U_INTERNAL_DUMP("num_entries = %u entry_capacity = %u entry_start_index = %u hpack_size = %u hpack_capacity = %u hpack_max_capacity = %u",
                        table->num_entries, table->entry_capacity, table->entry_start_index, table->hpack_size, table->hpack_capacity, table->hpack_max_capacity)

      uint32_t index = table->entry_start_index;

      do {
         HpackHeaderTableEntry* entry = table->entries + index;

         delete entry->name;
         delete entry->value;

         if (++index == table->entry_capacity) index = 0;

         table->num_entries--;
         }
      while (table->num_entries != 0);

      UMemoryPool::_free(table->entries, table->entry_capacity, sizeof(HpackHeaderTableEntry));

      table->entry_capacity    =
      table->entry_start_index =
      table->hpack_size        = 0;
      table->entries           = 0;

      U_INTERNAL_DUMP("num_entries = %u entry_capacity = %u entry_start_index = %u hpack_size = %u hpack_capacity = %u hpack_max_capacity = %u",
                        table->num_entries, table->entry_capacity, table->entry_start_index, table->hpack_size, table->hpack_capacity, table->hpack_max_capacity)
      }
}

void UHTTP2::reset(uint32_t idx, bool bsocket_open)
{
   U_TRACE(0, "UHTTP2::reset(%u,%b)", idx, bsocket_open)

   U_INTERNAL_ASSERT_EQUALS(U_http_version, '2')
   U_INTERNAL_ASSERT_MINOR(idx, UNotifier::max_connection)

   pConnection = vConnection + idx;

   U_DUMP("pConnection->state = (%d, %s)", pConnection->state, getConnectionStatusDescription())

   pConnection->itable.clear();

   clearHpackDynTbl(&(pConnection->idyntbl));
   clearHpackDynTbl(&(pConnection->odyntbl));

#ifdef DEBUG
   pConnection->dtable.clear();

   clearHpackDynTbl(&(pConnection->ddyntbl));
#endif

   if (bsocket_open) sendGoAway();
   else
      {
      pConnection->state = CONN_STATE_IDLE;
      }

   U_INTERNAL_DUMP("U_http2_settings_len = %u", U_http2_settings_len)

   U_http_version       =
   U_http2_settings_len = 0;
}

int UHTTP2::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::handlerRequest()")

   U_INTERNAL_ASSERT_EQUALS(U_http_version, '2')

   const char* ptr;
   uint32_t sz = (UServer_Base::pClientImage - UServer_Base::vClientImage);

   U_INTERNAL_DUMP("idx = %u UNotifier::max_connection = %u", sz, UNotifier::max_connection)

   U_INTERNAL_ASSERT_MINOR(sz, UNotifier::max_connection)

   pStream = (pConnection = (vConnection + sz))->streams;

   U_DUMP("pConnection->state = (%d, %s) pStream->state = (%d, %s)", pConnection->state, getConnectionStatusDescription(), pStream->state, getStreamStatusDescription())

   if (pConnection->state == CONN_STATE_OPEN)
      {
      settings_ack = true;

      goto loop;
      }

   pConnection->inp_window              =
   pConnection->out_window              =
   pConnection->max_open_stream_id      =
   pConnection->max_processed_stream_id = 0;

   pConnection->state =   CONN_STATE_OPEN;
       pStream->state = STREAM_STATE_IDLE;

   pConnection->peer_settings                     = settings;
   pConnection->peer_settings.initial_window_size = 0;

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

      U_INTERNAL_ASSERT_EQUALS(settings_ack, false)

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

      U_MEMCPY(UClientImage_Base::cbuffer, UClientImage_Base::request->c_pointer(UClientImage_Base::uri_offset), sz);

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

   ptr = UClientImage_Base::rbuffer->c_pointer(UClientImage_Base::rstart);

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

   UClientImage_Base::rstart += U_CONSTANT_SIZE(HTTP2_CONNECTION_PREFACE);

   settings_ack = false;

loop:
   readFrame();

   U_INTERNAL_DUMP("nerror = %d", nerror)

   if (nerror == NO_ERROR)
      {
      U_DUMP("settings_ack = %b U_http_method_type = %d pStream->state = (%d, %s)", settings_ack, U_http_method_type, pStream->state, getStreamStatusDescription())

      if (settings_ack == false || // we wait for SETTINGS ack...
          pStream->state == STREAM_STATE_IDLE)
         {
         goto loop;
         }

      if (frame.type != GOAWAY)
         {
         if (U_http_method_type == 0 ||
             U_http_method_type == HTTP_OPTIONS)
            {
            U_http_info.nResponseCode = (U_http_method_type ? HTTP_OPTIONS_RESPONSE
                                                            : HTTP_NOT_IMPLEMENTED);

            UHTTP::setResponse();

            U_RETURN(U_PLUGIN_HANDLER_FINISHED);
            }

         if (UClientImage_Base::isRequestNotFound()) return UHTTP::manageRequest();
         }

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }

   if (nerror == ERROR_NOCLOSE)
      {
      nerror = NO_ERROR;

      UClientImage_Base::setRequestProcessed();

      U_RETURN(U_PLUGIN_HANDLER_FINISHED);
      }

   if (UServer_Base::csocket->isOpen())
      {
      sendGoAway();

      UClientImage_Base::close();
      }

   nerror = NO_ERROR;

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

#define U_HTTP2_ENTRY(n) n: descr = #n; break

const char* UHTTP2::getFrameErrorCodeDescription(uint32_t error)
{
   U_TRACE(0, "UHTTP2::getFrameErrorCodeDescription(%u)", error)

   const char* descr;

   switch (error)
      {
      case U_HTTP2_ENTRY(NO_ERROR);          //  0: The endpoint NOT detected an error
      case U_HTTP2_ENTRY(PROTOCOL_ERROR);    //  1: The endpoint detected an unspecific protocol error. This error is for use when a more specific error code is not available
      case U_HTTP2_ENTRY(INTERNAL_ERROR);    //  2: The endpoint encountered an unexpected internal error
      case U_HTTP2_ENTRY(FLOW_CONTROL_ERROR);   //  3: The endpoint detected that its peer violated the flow-control protocol
      case U_HTTP2_ENTRY(SETTINGS_TIMEOUT);     //  4: The endpoint sent a SETTINGS frame but did not receive a response in a timely manner
      case U_HTTP2_ENTRY(STREAM_CLOSED);        //  5: The endpoint received a frame after a stream was half-closed
      case U_HTTP2_ENTRY(FRAME_SIZE_ERROR);     //  6: The endpoint received a frame with an invalid size
      case U_HTTP2_ENTRY(REFUSED_STREAM);    //  7: The endpoint refused the stream prior to performing any application processing
      case U_HTTP2_ENTRY(CANCEL);               //  8: Used by the endpoint to indicate that the stream is no longer needed
      case U_HTTP2_ENTRY(COMPRESSION_ERROR); //  9: The endpoint is unable to maintain the header compression context for the connection
      case U_HTTP2_ENTRY(CONNECT_ERROR);        // 10: The connection established in response to a CONNECT request (Section 8.3) was reset or abnormally closed
      case U_HTTP2_ENTRY(ENHANCE_YOUR_CALM); // 11: The endpoint detected that its peer is exhibiting a behavior that might be generating excessive load
      case U_HTTP2_ENTRY(INADEQUATE_SECURITY); // 12: The underlying transport has properties that do not meet minimum security requirements
      case U_HTTP2_ENTRY(HTTP_1_1_REQUIRED); // 13: The endpoint requires that HTTP/1.1 be used instead of HTTP/2

      default: descr = "Error type unknown";
      }

   U_RETURN(descr);
}

#ifdef DEBUG
const char* UHTTP2::getFrameTypeDescription(char type)
{
   U_TRACE(0, "UHTTP2::getFrameTypeDescription(%d)", type)

   const char* descr;

   switch (type)
      {
      case U_HTTP2_ENTRY(DATA);
      case U_HTTP2_ENTRY(HEADERS);
      case U_HTTP2_ENTRY(PRIORITY);
      case U_HTTP2_ENTRY(RST_STREAM);
      case U_HTTP2_ENTRY(SETTINGS);
      case U_HTTP2_ENTRY(PUSH_PROMISE);
      case U_HTTP2_ENTRY(PING);
      case U_HTTP2_ENTRY(GOAWAY);
      case U_HTTP2_ENTRY(WINDOW_UPDATE);
      case U_HTTP2_ENTRY(CONTINUATION);

      default: descr = "Frame type unknown";
      }

   U_RETURN(descr);
}

const char* UHTTP2::getStreamStatusDescription()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::getStreamStatusDescription()")

   const char* descr;

   switch (pStream->state)
      {
      case U_HTTP2_ENTRY(STREAM_STATE_IDLE);
      case U_HTTP2_ENTRY(STREAM_STATE_RESERVED);
      case U_HTTP2_ENTRY(STREAM_STATE_OPEN);
      case U_HTTP2_ENTRY(STREAM_STATE_HALF_CLOSED);
      case U_HTTP2_ENTRY(STREAM_STATE_CLOSED);

      default: descr = "Stream status unknown";
      }

   U_RETURN(descr);
}

const char* UHTTP2::getConnectionStatusDescription()
{
   U_TRACE_NO_PARAM(0, "UHTTP2::getConnectionStatusDescription()")

   const char* descr;

   switch (pConnection->state)
      {
      case U_HTTP2_ENTRY(CONN_STATE_IDLE);
      case U_HTTP2_ENTRY(CONN_STATE_OPEN);
      case U_HTTP2_ENTRY(CONN_STATE_IS_CLOSING);

      default: descr = "Connection status unknown";
      }

   U_RETURN(descr);
}

#undef U_HTTP2_ENTRY

#ifdef U_STDCPP_ENABLE
ostream& operator<<(ostream& _os, const UHTTP2::HpackDynamicTable& v)
{
   U_TRACE(0+256, "HpackDynamicTable::operator<<(%p,%p)", &_os, &v)

   _os.put('[');
   _os.put('\n');

   for (uint32_t index = v.entry_start_index, e = v.num_entries; e != 0; --e)
      {
      UHTTP2::HpackHeaderTableEntry* entry = v.entries + index;

      entry->name->write(_os);

      _os.put('\t');

      entry->value->write(_os);

      _os.put('\n');

      if (++index == v.entry_capacity) index = 0;
      }

   _os.put(']');

   return _os;
}
#endif

U_EXPORT const char* UHTTP2::Connection::dump(bool reset) const
{
   *UObjectIO::os << "state                     " << state                   << '\n'
                  << "out_window                " << out_window              << '\n'
                  << "inp_window                " << inp_window              << '\n'
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
