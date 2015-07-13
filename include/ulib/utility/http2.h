// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    http2.h - HTTP/2 utility
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_HTTP2_H
#define ULIB_HTTP2_H 1

#include <ulib/net/ipaddress.h>
#include <ulib/container/hash_map.h>

#define HTTP2_CONNECTION_UPGRADE \
      "HTTP/1.1 101 Switching Protocols\r\n" \
      "Connection: Upgrade\r\n" \
      "Upgrade: h2c\r\n\r\n"

#define HTTP2_CONNECTION_PREFACE "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n" // (24 bytes)

class UHTTP;
class UClientImage_Base;
class UHashMap<UString>;

class U_EXPORT UHTTP2 {
public:

   static void ctor();
   static void dtor();
   static bool manageSetting();

protected:
   enum FrameTypesId {
      DATA          = 0x00,
      HEADERS       = 0x01,
      PRIORITY      = 0x02,
      RST_STREAM    = 0x03,
      SETTINGS      = 0x04,
      PUSH_PROMISE  = 0x05,
      PING          = 0x06,
      GOAWAY        = 0x07,
      WINDOW_UPDATE = 0x08,
      CONTINUATION  = 0x09
   };

   enum FrameFlagsId {
      FLAG_NONE        = 0x00,
      FLAG_ACK         = 0x01,
      FLAG_END_STREAM  = 0x01,
      FLAG_END_HEADERS = 0x04,
      FLAG_PADDED      = 0x08,
      FLAG_PRIORITY    = 0x20
   };

   enum FrameErrorCode { // The status codes for the RST_STREAM and GOAWAY frames
      NO_ERROR            = 0x00,
      PROTOCOL_ERROR      = 0x01,
      INTERNAL_ERROR      = 0x02,
      FLOW_CONTROL_ERROR  = 0x03,
      SETTINGS_TIMEOUT    = 0x04,
      STREAM_CLOSED       = 0x05,
      FRAME_SIZE_ERROR    = 0x06,
      REFUSED_STREAM      = 0x07,
      CANCEL              = 0x08,
      COMPRESSION_ERROR   = 0x09,
      CONNECT_ERROR       = 0x0a,
      ENHANCE_YOUR_CALM   = 0x0b,
      INADEQUATE_SECURITY = 0x0c,
      HTTP_1_1_REQUIRED   = 0x0d,
      ERROR_INCOMPLETE    = 0xff
   };

   /**
    * All frames begin with a fixed 9-octet header followed by a variable-length payload
    *
    *  0                   1                   2                   3
    *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    * |                 Length (24)                   |
    * +---------------+---------------+---------------+
    * |   Type (8)    |   Flags (8)   |
    * +-+-+-----------+---------------+-------------------------------+
    * |R|                 Stream Identifier (31)                      |
    * +=+=============================================================+
    * |                   Frame Payload (0...)                      ...
    * +---------------------------------------------------------------+
    */

   struct FrameHeader {
      const char* payload;
      int32_t length;    // The length field of this frame, excluding frame header
      int32_t stream_id; // The stream identifier (aka, stream ID)
      uint8_t type;      // The type of this frame
      uint8_t flags;
   };

   enum SettingsId {
      HEADER_TABLE_SIZE      = 1,
      ENABLE_PUSH            = 2,
      MAX_CONCURRENT_STREAMS = 3,
      INITIAL_WINDOW_SIZE    = 4,
      MAX_FRAME_SIZE         = 5,
      MAX_HEADER_LIST_SIZE   = 6
   };

   struct Settings {
      uint32_t header_table_size;
      uint32_t enable_push;
      uint32_t max_concurrent_streams;
      uint32_t initial_window_size;
      uint32_t max_frame_size;
      uint32_t max_header_list_size;
   };

   enum StreamState {
      STREAM_STATE_RECV_PSUEDO_HEADERS = 0x001,
      STREAM_STATE_RECV_HEADERS        = 0x002,
      STREAM_STATE_RECV_BODY           = 0x004,
      STREAM_STATE_REQ_PENDING         = 0x008,
      STREAM_STATE_SEND_HEADERS        = 0x010,
      STREAM_STATE_SEND_BODY           = 0x020,
      STREAM_STATE_END_STREAM          = 0x040,
      STREAM_STATE_HALF_CLOSED         = 0x080
   };

   enum ConnectionState {
      CONN_STATE_OPEN,
      CONN_STATE_IS_CLOSING
   };

   struct Stream {
      int id;
      int state;
      int  input_window;
      int output_window;
      // priority
      uint32_t priority_dependency; // 0 if not set
          char priority_weight;     // 0 if not set
          bool priority_exclusive;
   };

   struct Connection {
      // headers
      UHashMap<UString>* itable;
      UHashMap<UString>* otable;
      // streams
      Stream open_streams[100];
      // settings
      Settings peer_settings;
      // streams
      int max_open_stream_id;
      uint32_t num_responding_streams;
      uint32_t max_processed_stream_id;
      // internal
      ConnectionState state;
      uint32_t  input_window;
      uint32_t output_window;
       int32_t hpack_max_capacity; // the value set by SETTINGS_HEADER_TABLE_SIZE
   };

   struct HpackHeaderTableEntry {
      UString* name;
      UString* value;
   };

   static UString* str_authority;
   static UString* str_method;
   static UString* str_method_get;
   static UString* str_method_post;
   static UString* str_path;
   static UString* str_path_root;
   static UString* str_path_index;
   static UString* str_scheme;
   static UString* str_scheme_http;
   static UString* str_scheme_https;
   static UString* str_status;
   static UString* str_status_200;
   static UString* str_status_204;
   static UString* str_status_206;
   static UString* str_status_304;
   static UString* str_status_400;
   static UString* str_status_404;
   static UString* str_status_500;
   static UString* str_accept_charset;
   static UString* str_accept_encoding;
   static UString* str_accept_encoding_value;
   static UString* str_accept_language;
   static UString* str_accept_ranges;
   static UString* str_accept;
   static UString* str_access_control_allow_origin;
   static UString* str_age;
   static UString* str_allow;
   static UString* str_authorization;
   static UString* str_cache_control;
   static UString* str_content_disposition;
   static UString* str_content_encoding;
   static UString* str_content_language;
   static UString* str_content_length;
   static UString* str_content_location;
   static UString* str_content_range;
   static UString* str_content_type;
   static UString* str_cookie;
   static UString* str_date;
   static UString* str_etag;
   static UString* str_expect;
   static UString* str_expires;
   static UString* str_from;
   static UString* str_host;
   static UString* str_if_match;
   static UString* str_if_modified_since;
   static UString* str_if_none_match;
   static UString* str_if_range;
   static UString* str_if_unmodified_since;
   static UString* str_last_modified;
   static UString* str_link;
   static UString* str_location;
   static UString* str_max_forwards;
   static UString* str_proxy_authenticate;
   static UString* str_proxy_authorization;
   static UString* str_range;
   static UString* str_referer;
   static UString* str_refresh;
   static UString* str_retry_after;
   static UString* str_server;
   static UString* str_set_cookie;
   static UString* str_strict_transport_security;
   static UString* str_transfer_encoding;
   static UString* str_user_agent;
   static UString* str_vary;
   static UString* str_via;
   static UString* str_www_authenticate;

   static int nerror;
   static Stream* pStream;
   static FrameHeader frame;
   static void* pConnectionEnd;
   static Connection* pConnection;
   static const char* upgrade_settings;
   static const Settings SETTINGS_HOST;
   static const Settings SETTINGS_DEFAULT;

   static uint32_t hash_static_table[61];
   static HpackHeaderTableEntry hpack_static_table[61];

   // SERVICES

   static void   readFrame();
   static void decodeFrame();

   static void manageData();
   static void managePriority();
   static void manageWindowUpdate();

   static void setStream();
   static void setConnection();
   static void sendError(int err);
   static void resetReadBuffer(uint32_t length);
   static bool updateSetting(const char*  ptr, uint32_t len);
   static void decodeHeaders(const char*  ptr, const char*  endptr);
   static bool manageHeaders(const char** ptr, const char** endptr);

#ifdef DEBUG
   static const char* getFrameTypeDescription();
#endif      

   static bool setIndexStaticTable(UHashMap<void*>* ptable, const char* key, uint32_t length)
      {
      U_TRACE(0, "UHTTP2::setIndexStaticTable(%p,%.*S,%u)", ptable, length, key, length)

      ptable->index = ptable->hash % ptable->_capacity;

      U_RETURN(false);
      }

   static void setLengthAndType(char* ptr, uint32_t length, FrameTypesId type)
      {
      U_TRACE(0, "UHTTP2::setLengthAndType(%p,%u,%d)", ptr, length, type)

      U_INTERNAL_ASSERT(type <= CONTINUATION)

      *(uint32_t*)ptr    = htonl(length & 0x00ffffff) >> 8;
                  ptr[3] = type;

      U_INTERNAL_DUMP("length = %#.4S", ptr) // "\000\000\004\003" (big endian: 0x11223344)
      }

   // HPACK

   enum HuffDecodeFlag {
      HUFF_ACCEPTED =  1,        // FSA accepts this state as the end of huffman encoding sequence
      HUFF_SYM      = (1 << 1),  // This state emits symbol
      HUFF_FAIL     = (1 << 2)   // If state machine reaches this state, decoding fails
   };

   struct HuffDecode {
      /**
       * huffman decoding state, which is actually the node ID of internal huffman tree.
       * We have 257 leaf nodes, but they are identical to root node other than emitting
       * a symbol, so we have 256 internal nodes [1..255], inclusive
       */
      uint8_t state;
      uint8_t flags; // bitwise OR of zero or more of the HuffDecodeFlag
      uint8_t sym;   // symbol if HUFF_SYM flag set
   };

   struct HuffSym {
      uint32_t nbits; // The number of bits in this code
      uint32_t code;  // Huffman code aligned to LSB
   };

   static const HuffSym    huff_sym_table[];
   static const HuffDecode huff_decode_table[][16];

   static uint32_t hpackDecodeInt(   const unsigned char* src, const unsigned char* src_end, int32_t* pvalue, uint8_t prefix_max);
   static uint32_t hpackDecodeString(const unsigned char* src, const unsigned char* src_end, UString* pvalue);

private:
   friend class UHTTP;
   friend class UClientImage_Base;

#ifdef U_COMPILER_DELETE_MEMBERS
   UHTTP2(const UHTTP2&) = delete;
   UHTTP2& operator=(const UHTTP2&) = delete;
#else
   UHTTP2(const UHTTP2&)            {}
   UHTTP2& operator=(const UHTTP2&) { return *this; }
#endif      
};

#endif
