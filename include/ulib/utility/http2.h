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

#define HTTP2_CONNECTION_PREFACE "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n" // (24 bytes)

class UHTTP;
class UHashMap<UString>;
class UClientImage_Base;

class U_EXPORT UHTTP2 {
public:

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
      STREAM_STATE_IDLE        = 0x000,
      STREAM_STATE_RESERVED    = 0x001,
      STREAM_STATE_OPEN        = 0x002,
      STREAM_STATE_HALF_CLOSED = 0x004,
      STREAM_STATE_CLOSED      = 0x008
   };

   enum ConnectionState {
      CONN_STATE_IDLE          = 0x000,
      CONN_STATE_OPEN          = 0x001,
      CONN_STATE_IS_CLOSING    = 0x002
   };

   struct Stream {
      int32_t id;
      int32_t state;
   // internal
   // int32_t out_window;
   // int32_t inp_window;
   // priority
   // uint32_t priority_dependency; // 0 if not set
   //     char priority_weight;     // 0 if not set
   //     bool priority_exclusive;
   };

   static void ctor();
   static void dtor();

   class Connection {
   public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UHashMap<UString> itable; // headers request
   Settings peer_settings; // settings
   ConnectionState state; // state
   // internal
// int32_t out_window; //   sender flow control window
   int32_t inp_window; // receiver flow control window
   int32_t hpack_max_capacity; // the value set by SETTINGS_HEADER_TABLE_SIZE
   // streams
   int32_t max_open_stream_id;
   int32_t max_processed_stream_id;
   Stream streams[100];

   // COSTRUTTORI

    Connection();
   ~Connection()
      {
      U_TRACE_UNREGISTER_OBJECT(0, Connection)
      }

   // SERVICES

   static void preallocate(uint32_t max_connection)
      {
      U_TRACE(0+256, "UHTTP2::Connection::preallocate(%u)", max_connection)

      U_INTERNAL_ASSERT_EQUALS(vConnection, 0)

      U_INTERNAL_DUMP("sizeof(Connection) = %u sizeof(Stream) = %u", sizeof(Connection), sizeof(Stream))

      vConnection = new Connection[max_connection];
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const U_EXPORT;
#endif

   private:
#ifdef U_COMPILER_DELETE_MEMBERS
   Connection(const Connection&) = delete;
   Connection& operator=(const Connection&) = delete;
#else
   Connection(const Connection&)            {}
   Connection& operator=(const Connection&) { return *this; }
#endif
   };

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
      unsigned char* payload;
      int32_t length;    // The length field of this frame, excluding frame header
      int32_t stream_id; // The stream identifier (aka, stream ID)
      uint8_t type;      // The type of this frame
      uint8_t flags;
   };

   struct HpackHeaderTableEntry {
      const UString* name;
      const UString* value;
   };

   static int nerror;
   static Stream* pStream;
   static FrameHeader frame;
   static bool settings_ack;
   static Connection* vConnection;
   static Connection* pConnection;
   static const Settings settings;
   static const char* upgrade_settings;

   static uint32_t               hash_static_table[61];
   static HpackHeaderTableEntry hpack_static_table[61];

   // SERVICES

   static void readFrame();
   static void sendError();
   static void openStream();
   static void manageData();
   static void manageHeaders();
   static void resetDataRead();
   static int  handlerRequest();
   static void handlerResponse();
   static bool readBodyRequest();
   static bool updateSetting(unsigned char*  ptr, uint32_t len);
   static void decodeHeaders(unsigned char*  ptr, unsigned char*  endptr);

#ifdef DEBUG
   static const char* getFrameTypeDescription(char type);
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

   static unsigned char* hpackEncodeString(unsigned char* dst,                         const char* src, uint32_t len);
   static unsigned char* hpackDecodeString(unsigned char* src, unsigned char* src_end, UString* pvalue);

   static unsigned char* hpackEncodeInt(   unsigned char* dst,                         uint32_t   value, uint8_t prefix_max);
   static unsigned char* hpackDecodeInt(   unsigned char* src, unsigned char* src_end,  int32_t* pvalue, uint8_t prefix_max);

private:
   static UVector<UString>* vext;

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
