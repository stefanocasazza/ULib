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
#include <ulib/utility/uhttp.h>
#include <ulib/utility/hexdump.h>

#define HTTP2_FRAME_HEADER_SIZE               9 // The number of bytes of the frame header
#define HTTP2_HEADER_TABLE_ENTRY_SIZE_OFFSET 32

#define HTTP2_CONNECTION_PREFACE "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n" // (24 bytes)

class UHTTP;
class UClientImage_Base;

class U_EXPORT UHTTP2 {
public:

   struct Stream {
      uint32_t id,
               state;
   };

   enum StreamState {
      STREAM_STATE_IDLE        = 0x000,
      STREAM_STATE_RESERVED    = 0x001,
      STREAM_STATE_OPEN        = 0x002,
      STREAM_STATE_HALF_CLOSED = 0x004,
      STREAM_STATE_CLOSED      = 0x008
   };

   struct Settings {
      uint32_t header_table_size,
               enable_push,
               max_concurrent_streams,
               initial_window_size,
               max_frame_size,
               max_header_list_size;
   };

   enum SettingsId {
      HEADER_TABLE_SIZE      = 1,
      ENABLE_PUSH            = 2,
      MAX_CONCURRENT_STREAMS = 3,
      INITIAL_WINDOW_SIZE    = 4,
      MAX_FRAME_SIZE         = 5,
      MAX_HEADER_LIST_SIZE   = 6
   };

   struct HpackHeaderTableEntry {
      const UString* name;
      const UString* value;
   };

   struct HpackDynamicTable {
      uint32_t num_entries,
               entry_capacity,
               entry_start_index;
      // size and capacities are 32+name_len+value_len (as defined by hpack spec)
      uint32_t hpack_size,
               hpack_capacity,        // the value set by SETTINGS_HEADER_TABLE_SIZE _and_ dynamic table size update
               hpack_max_capacity;    // the value set by SETTINGS_HEADER_TABLE_SIZE
      HpackHeaderTableEntry* entries; // ring buffer
   };

   enum ConnectionState {
      CONN_STATE_IDLE       = 0x000,
      CONN_STATE_OPEN       = 0x001,
      CONN_STATE_IS_CLOSING = 0x002
   };

   class Connection {
   public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   int32_t out_window,                 //   sender flow control window (its size is a measure of the buffering capacity of the   sender)
           inp_window;                 // receiver flow control window (its size is a measure of the buffering capacity of the receiver)
   ConnectionState state;              // state
   Settings peer_settings;             // settings
   UHashMap<UString> itable;           // headers request
   HpackDynamicTable idyntbl, odyntbl; // hpack dynamic table (request, response)
   // streams
   uint32_t max_open_stream_id,
            max_processed_stream_id;
   Stream streams[100];
#ifdef DEBUG
   UHashMap<UString> dtable;
   HpackDynamicTable ddyntbl;
#endif

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
   U_DISALLOW_COPY_AND_ASSIGN(Connection)
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
      FLAG_ACK         = 0x01, // 1
      FLAG_END_STREAM  = 0x01, // 1
      FLAG_END_HEADERS = 0x04, // 100
      FLAG_PADDED      = 0x08, // 1000
      FLAG_PRIORITY    = 0x20  // 100000
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
      ERROR_NOCLOSE       = 0xfe,
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
      uint32_t length,    // The length field of this frame, excluding frame header
               stream_id; // The stream identifier (aka, stream ID)
      uint8_t  type,      // The type of this frame
               flags;
   };

   static int nerror;
   static Stream* pStream;
   static FrameHeader frame;
   static Connection* vConnection;
   static Connection* pConnection;
   static const Settings settings;
   static const char* upgrade_settings;
   static bool settings_ack, continue100;

   static uint32_t               hash_static_table[61];
   static HpackHeaderTableEntry hpack_static_table[61];

   // SERVICES

   static void ctor();
   static void dtor()
      {
      U_TRACE_NO_PARAM(0, "UHTTP2::dtor()")

      delete[] vConnection;
      }

   static void resetDataRead()
      {
      U_TRACE_NO_PARAM(0, "UHTTP2::resetDataRead()")

      UClientImage_Base::rstart = 0;

      UClientImage_Base::rbuffer->setEmptyForce();

      if (USocketExt::read(UServer_Base::csocket, *UClientImage_Base::rbuffer, U_SINGLE_READ, UServer_Base::timeoutMS, UHTTP::request_read_timeout) == false) nerror = PROTOCOL_ERROR;
      }

   static void setEncoding(const UString& x)
      {
      U_TRACE(0, "UHTTP2::setEncoding(%V)", x.rep)

      U_INTERNAL_ASSERT_EQUALS(U_http_is_accept_gzip, false)

      U_INTERNAL_DUMP("Accept-Encoding: = %V", x.rep)

      U_INTERNAL_ASSERT(u_find(x.data(), 30, U_CONSTANT_TO_PARAM("gzip")))

      U_http_flag |= HTTP_IS_ACCEPT_GZIP;

      U_INTERNAL_DUMP("U_http_is_accept_gzip = %b", U_http_is_accept_gzip)
      }

   static void setURI(const char* ptr, uint32_t len)
      {
      U_TRACE(0, "UHTTP2::setURI(%.*S,%u)", len, ptr, len)

      U_INTERNAL_ASSERT_EQUALS(U_http_info.uri_len, 0)

      U_http_info.query = (const char*) memchr((U_http_info.uri = ptr), '?', (U_http_info.uri_len = len));

      U_INTERNAL_DUMP("URI = %.*S", U_HTTP_URI_TO_TRACE)

      if (U_http_info.query)
         {
         U_http_info.query_len = U_http_info.uri_len - (U_http_info.query++ - U_http_info.uri);

         U_http_info.uri_len -= U_http_info.query_len;

         U_INTERNAL_DUMP("query = %.*S", U_HTTP_QUERY_TO_TRACE)
         }
      }

   static bool setStream()
      {
      U_TRACE_NO_PARAM(0, "UHTTP2::setStream()")

      U_INTERNAL_DUMP("pStream->id = %u frame.stream_id = %u pConnection->max_open_stream_id = %u", pStream->id, frame.stream_id, pConnection->max_open_stream_id)

      U_INTERNAL_ASSERT_DIFFERS(pStream->id, frame.stream_id)

      U_INTERNAL_DUMP("pConnection->max_open_stream_id = %u", pConnection->max_open_stream_id)

      if ((uint32_t)frame.stream_id <= pConnection->max_open_stream_id)
         {
         for (Stream* pStreamEnd = (pStream = pConnection->streams) + 100; pStream < pStreamEnd; ++pStream)
            {
            if (pStream->id == frame.stream_id) U_RETURN(true);
            }
         }

      U_RETURN(false);
      }

   static void readFrame();
   static void openStream();
   static void manageData();
   static void sendGoAway();
   static void manageHeaders();
   static int  handlerRequest();
   static void handlerResponse();
   static bool readBodyRequest();
   static void reset(uint32_t idx, bool bsocket_open);
   static bool updateSetting(unsigned char* ptr, uint32_t len);

   static const char* getFrameErrorCodeDescription(uint32_t error);

#ifdef DEBUG
   static bool bdecodeHeadersDebug;

   static bool findHeader(uint32_t index)
      {
      U_TRACE(0, "UHTTP2::findHeader(%u)", index)

      UHashMap<UString>* table = &(pConnection->dtable);

      table->hash = hash_static_table[index];

      table->lookup(hpack_static_table[index].name->rep);

      if (table->node) U_RETURN(true);

      U_RETURN(false);
      }

# ifdef U_STDCPP_ENABLE
   friend ostream& operator<<(ostream& _os, const HpackDynamicTable& v);
# endif

   static void decodeHeadersResponse()
      {
      U_TRACE_NO_PARAM(0, "UHTTP2::decodeHeadersResponse()")

      bdecodeHeadersDebug = true;

      const char* start = UClientImage_Base::wbuffer->c_pointer(HTTP2_FRAME_HEADER_SIZE);

      decodeHeaders(&(pConnection->dtable),
                    &(pConnection->ddyntbl), (unsigned char*)start, (unsigned char*)(start + (ntohl(*(uint32_t*)UClientImage_Base::wbuffer->data() & 0x00ffffff) >> 8)));

      bdecodeHeadersDebug = false;

   // U_DUMP_OBJECT_TO_TMP(pConnection->dtable, "response.hpack")

      U_ASSERT(findHeader(53)) // UString::str_server
      U_ASSERT(findHeader(32)) // UString::str_date
      U_ASSERT(findHeader(27)) // UString::str_content_length
      }

   static void saveHpackData(const char* ptr, uint32_t len, bool breq)
      {
      U_TRACE(0+256, "UHTTP2::saveHpackData(%.*S,%u,%b)", len, ptr, len, breq)

      /**
       * We save the Header Block Fragment of the frame to inspect it with inflatehd (https://github.com/tatsuhiro-t/nghttp2)
       *
       * ./inflatehd < inflatehd.json => { "cases": [ { "wire": "8285" } ] }
       */

   // UString tmp(U_CAPACITY);
   // UHexDump::encode(ptr, len, tmp);
   // (void) UFile::writeToTmp(U_STRING_TO_PARAM(tmp), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("%s.hpack.%P"), breq ? "request" : "response");
      }

   static const char* getStreamStatusDescription();
   static const char* getConnectionStatusDescription();
   static const char* getFrameTypeDescription(char type);
#endif

   static void decodeHeaders(                                                       unsigned char* ptr, unsigned char* endptr);
   static void decodeHeaders(UHashMap<UString>* itable, HpackDynamicTable* idyntbl, unsigned char* ptr, unsigned char* endptr);

   /**
    * static void eraseHeader(uint32_t index)
    * {
    * U_TRACE(0, "UHTTP2::eraseHeader(%u)", index)
    *
    * UHashMap<UString>* table = &(pConnection->itable);
    *
    * table->hash = hash_static_table[index];
    *
    * table->lookup(hpack_static_table[index].name->rep);
    *
    * if (table->node) table->eraseAfterFind();
    * }
    *
    * static void setFrameLengthAndType(char* ptr, uint32_t length, FrameTypesId type)
    * {
    * U_TRACE(0, "UHTTP2::setFrameLengthAndType(%p,%u,%d)", ptr, length, type)
    *
    * U_INTERNAL_ASSERT(type <= CONTINUATION)
    *
    * *(uint32_t*)ptr    = htonl(length & 0x00ffffff) >> 8;
    *             ptr[3] = type;
    *
    * U_INTERNAL_DUMP("length = %#.4S", ptr) // "\000\000\004\003" (big endian: 0x11223344)
    * }
    */

   static int32_t getIndexStaticTable(const UString& key) __pure;

   static bool setIndexStaticTable(UHashMap<void*>* table, const char* key, uint32_t length)
      {
      U_TRACE(0, "UHTTP2::setIndexStaticTable(%p,%.*S,%u)", table, length, key, length)

      U_INTERNAL_DUMP("table->hash = %u", table->hash)

      table->index = table->hash % table->_capacity;

      U_RETURN(true); // NB: ignore case...
      }

   // HPACK (HTTP headers compression)

   static HpackHeaderTableEntry* getHpackDynTblEntry(HpackDynamicTable* table, uint32_t index)
      {
      U_TRACE(0, "UHTTP2::getHpackDynTblEntry(%p,%u)", table, index)

      U_INTERNAL_DUMP("table->entry_start_index = %u table->entry_capacity = %u", table->entry_start_index, table->entry_capacity)

      uint32_t entry_index = (table->entry_start_index + index) % table->entry_capacity;

      HpackHeaderTableEntry* entry = table->entries + entry_index;

      U_INTERNAL_ASSERT_POINTER(entry->name)

      U_INTERNAL_DUMP("entry->name = %V entry->value = %V", entry->name->rep, entry->value->rep)

      U_RETURN_POINTER(entry, HpackHeaderTableEntry);
      }

   static void evictHpackDynTblEntry(HpackDynamicTable* table)
      {
      U_TRACE(0, "UHTTP2::evictHpackDynTblEntry(%p)", table)

      U_INTERNAL_ASSERT_MAJOR(table->num_entries, 0)

      table->num_entries--;

      HpackHeaderTableEntry* entry = getHpackDynTblEntry(table, table->num_entries);

      U_DEBUG("HTTP compressor header table index %u: (%V %V) removed for size %u", table->num_entries, entry->name->rep, entry->value->rep, table->hpack_size)

      U_INTERNAL_DUMP("HTTP compressor header table index %u: (%V %V) removed for size %u", table->num_entries, entry->name->rep, entry->value->rep, table->hpack_size)

      table->hpack_size -= entry->name->size() + entry->value->size() + HTTP2_HEADER_TABLE_ENTRY_SIZE_OFFSET;

      delete entry->name;
      delete entry->value;
      }

   static void setHpackDynTblCapacity(HpackDynamicTable* dyntbl, uint32_t value)
      {
      U_TRACE(0, "UHTTP2::setHpackDynTblCapacity(%p,%u)", dyntbl, value)

      U_INTERNAL_DUMP("hpack_capacity = %u hpack_max_capacity = %u", dyntbl->hpack_capacity, dyntbl->hpack_max_capacity)

      U_INTERNAL_ASSERT_DIFFERS(dyntbl->hpack_capacity, value)

      dyntbl->hpack_capacity = value;

      // adjust the size

      while (dyntbl->num_entries != 0 &&
             dyntbl->hpack_size > dyntbl->hpack_capacity)
         {
         evictHpackDynTblEntry(dyntbl);
         }
      }

   static void clearHpackDynTbl(     HpackDynamicTable* table);
   static void   addHpackDynTblEntry(HpackDynamicTable* table, const UString& name, const UString& value);

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
      uint8_t state,
              flags, // bitwise OR of zero or more of the HuffDecodeFlag
              sym;   // symbol if HUFF_SYM flag set
   };

   struct HuffSym {
      uint32_t nbits, // The number of bits in this code
               code;  // Huffman code aligned to LSB
   };

   static const HuffSym    huff_sym_table[];
   static const HuffDecode huff_decode_table[][16];

   static unsigned char* hpackDecodeInt(   unsigned char* src, unsigned char* src_end, int32_t* pvalue, uint8_t prefix_max);
   static unsigned char* hpackDecodeString(unsigned char* src, unsigned char* src_end, UString* pvalue);

   static unsigned char* hpackEncodeInt(   unsigned char* dst, uint32_t value, uint8_t prefix_max);
   static unsigned char* hpackEncodeString(unsigned char* dst, const char* src, uint32_t len);
   static unsigned char* hpackEncodeHeader(unsigned char* dst, HpackDynamicTable* dyntbl, UString& key, const UString& value, UVector<UString>* pvec = 0);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHTTP2)

   friend class UHTTP;
   friend class UClientImage_Base;
};

#endif
