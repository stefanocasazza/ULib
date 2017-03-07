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

#ifdef USE_LOAD_BALANCE
#  include <ulib/net/client/http.h>
#endif

#define HTTP2_FRAME_HEADER_SIZE               9 // The number of bytes of the frame header
#define HTTP2_DEFAULT_WINDOW_SIZE         65535 
#define HTTP2_HEADER_TABLE_OFFSET            62
#define HTTP2_MAX_CONCURRENT_STREAMS        128
#define HTTP2_HEADER_TABLE_ENTRY_SIZE_OFFSET 32

#define HTTP2_CONNECTION_PREFACE "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n" // (24 bytes)

class UHTTP;
class UClientImage_Base;

class U_EXPORT UHTTP2 {
public:

   enum ConnectionState {
      CONN_STATE_IDLE       = 0x000,
      CONN_STATE_OPEN       = 0x001,
      CONN_STATE_IS_CLOSING = 0x002
   };

   enum StreamState {
      STREAM_STATE_IDLE        = 0x000,
      STREAM_STATE_OPEN        = 0x001,
      STREAM_STATE_HALF_CLOSED = 0x002,
      STREAM_STATE_CLOSED      = 0x004,
      STREAM_STATE_RESERVED    = 0x008
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
      uint32_t header_table_size,
               enable_push,
               max_concurrent_streams,
               initial_window_size,
               max_frame_size,
               max_header_list_size;
   };

   struct HpackHeaderTableEntry {
      UStringRep* name;
      UStringRep* value;
   };

   struct HpackDynamicTable { // (Last In First Out) 
      uint32_t num_entries,
               entry_capacity,
               entry_start_index;
      // size and capacities are 32+name_len+value_len (as defined by hpack spec)
      uint32_t hpack_size,
               hpack_capacity,        // the value set by SETTINGS_HEADER_TABLE_SIZE _and_ dynamic table size update
               hpack_max_capacity;    // the value set by SETTINGS_HEADER_TABLE_SIZE
      HpackHeaderTableEntry* entries; // ring buffer
   };

   struct Stream {
      UString headers, body;
      uint32_t id, state, clength;
   };

   class Connection {
   public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   int32_t inp_window,                 // receive flow control window (its size is a measure of the buffering capacity of the   sender)
           out_window;                 //    send flow control window (its size is a measure of the buffering capacity of the receiver)
   ConnectionState state;              // state
   Settings peer_settings;             // settings
   UHashMap<UString> itable;           // headers request
   HpackDynamicTable idyntbl, odyntbl; // hpack dynamic table (request, response)
   // streams
   uint32_t max_processed_stream_id;
   Stream streams[HTTP2_MAX_CONCURRENT_STREAMS];
   bool bnghttp2;
#ifdef DEBUG
   UHashMap<UString> dtable;
   HpackDynamicTable ddyntbl;
#endif

    Connection();
   ~Connection()
      {
      U_TRACE_UNREGISTER_OBJECT(0, Connection)
      }

   void reset()
      {
      U_TRACE_NO_PARAM(0, "UHTTP2::Connection::reset()")

      inp_window              =
      out_window              = HTTP2_DEFAULT_WINDOW_SIZE;
      peer_settings           = settings;
      max_processed_stream_id = 0;
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

#ifdef DEBUG
   static void testHpackDynTbl();
#endif

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
      ERROR_INCOMPLETE    = 0x0e
   };

   enum HuffDecodeFlag {
      HUFF_ACCEPTED =  1,       // FSA accepts this state as the end of huffman encoding sequence
      HUFF_SYM      = (1 << 1), // This state emits symbol
      HUFF_FAIL     = (1 << 2)  // If state machine reaches this state, decoding fails
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

   struct HuffSym {
      uint32_t nbits, // The number of bits in this code
               code;  // Huffman code aligned to LSB
   };

   /**
    * huffman decoding state, which is actually the node ID of internal huffman tree.
    * We have 257 leaf nodes, but they are identical to root node other than emitting
    * a symbol, so we have 256 internal nodes [1..255], inclusive
    */

   struct HuffDecode {
      uint8_t state,
              flags, // bitwise OR of zero or more of the HuffDecodeFlag
              sym;   // symbol if HUFF_SYM flag set
   };

   static Stream* pStream;
   static FrameHeader frame;
   static Stream* pStreamEnd;
   static int nerror, hpack_errno;
   static Connection* vConnection;
   static Connection* pConnection;
   static const Settings settings;
   static uint32_t wait_for_continuation;
   static bool bcontinue100, bsetting_ack, bsetting_send;

   static uint8_t  priority_weight;     // 0 if not set
   static bool     priority_exclusive;
   static uint32_t priority_dependency; // 0 if not set

   static const HuffSym    huff_sym_table[];
   static const HuffDecode huff_decode_table[][16];

   static uint32_t               hash_static_table[61];
   static HpackHeaderTableEntry hpack_static_table[61];

   // SERVICES

   static void ctor();
   static void dtor()
      {
      U_TRACE_NO_PARAM(0, "UHTTP2::dtor()")

      delete[] vConnection;
      }

   static void sendPing();
   static void readFrame();
   static void setStream();
   static void manageData();
   static bool initRequest();
   static void writeResponse();
   static void decodeHeaders();
   static void manageHeaders();
   static void handlerRequest();
   static void handlerResponse();
   static void sendResetStream();
   static void sendWindowUpdate();
   static void sendGoAway(USocket* psocket);

   static void updateSetting(unsigned char* ptr, uint32_t len);
   static void writeData(struct iovec* iov, bool bdata, bool flag);
   static void handlerDelete(UClientImage_Base* pclient, bool& bsocket_open);

   static void startRequest()
      {
      U_TRACE_NO_PARAM(0, "UHTTP2::startRequest()")

      UClientImage_Base::endRequest();

      UHTTP::startRequest();

      U_http_version = '2';
      }

   static void resetDataRead()
      {
      U_TRACE_NO_PARAM(0, "UHTTP2::resetDataRead()")

      UClientImage_Base::rstart = 0;

      UClientImage_Base::rbuffer->setEmptyForce();
      }

   static void clear()
      {
      U_TRACE_NO_PARAM(0+256, "UHTTP2::clear()")

      for (pStream = pConnection->streams; pStream <= pStreamEnd; ++pStream)
         {
         pStream->body.clear();
         pStream->headers.clear();

         pStream->id      =
         pStream->state   =
         pStream->clength = 0;
         }

#  ifdef DEBUG
      for (pStreamEnd = (pConnection->streams+HTTP2_MAX_CONCURRENT_STREAMS); pStream < pStreamEnd; ++pStream)
         {
      // U_INTERNAL_DUMP("pStream index = %u", pStream - pConnection->streams)

         U_ASSERT(pStream->body.empty())
         U_ASSERT(pStream->headers.empty())

         U_INTERNAL_ASSERT_EQUALS(pStream->clength, 0)
         }
#  endif
      }

   static void readPriority(unsigned char* ptr)
      {
      U_TRACE(0, "UHTTP2::readPriority(%p)", ptr)

      priority_weight = ptr[4]+1;

      uint32_t u4 = u_parse_unalignedp32(ptr);

      priority_exclusive  = u4 >> 31;
      priority_dependency = u4 & 0x7fffffff;

      U_INTERNAL_DUMP("priority_weight = %u priority_exclusive = %b priority_dependency = %u frame.stream_id = %u",
                       priority_weight,     priority_exclusive,     priority_dependency,     frame.stream_id)

      if (priority_dependency == frame.stream_id) nerror = PROTOCOL_ERROR;
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

   static void setURI(const UString& uri) { setURI(U_STRING_TO_PARAM(uri)); }

   static void updateSetting(const UString& data) { updateSetting((unsigned char*)U_STRING_TO_PARAM(data)); }

   static bool writev(struct iovec* iov, int iovcnt, uint32_t count)
      {
      U_TRACE(0, "UHTTP2::writev(%p,%d,%u)", iov, iovcnt, count)

      U_DUMP_IOVEC(iov,iovcnt)

      int iBytesWrite =
#  if defined(USE_LIBSSL) || defined(_MSWINDOWS_)
      USocketExt::writev( UServer_Base::csocket, iov, iovcnt, count, 0);
#  else
      USocketExt::_writev(UServer_Base::csocket, iov, iovcnt, count, 0);
#  endif

      if (iBytesWrite == (int)count) U_RETURN(true);

      nerror = CONNECT_ERROR;

      U_RETURN(false);
      }

   static void checkStreamState()
      {
      U_TRACE_NO_PARAM(0, "UHTTP2::checkStreamState()")

      if (pStream->state < STREAM_STATE_HALF_CLOSED)
         {
         if ((frame.flags & FLAG_END_STREAM) != 0) pStream->state = STREAM_STATE_HALF_CLOSED;
         else
            {
            if (frame.type == HEADERS) nerror = PROTOCOL_ERROR;
            }
         }
      else
         {
         if (frame.type != RST_STREAM &&
             frame.type != WINDOW_UPDATE)
            {
            nerror = STREAM_CLOSED;
            }
         }
      }

   /**
    * +-------+-----------------------------+---------------+
    * | 1     | :authority                  |               |
    * | 2     | :method                     | GET           |
    * | 3     | :method                     | POST          |
    * | 4     | :path                       | /             |
    * | 5     | :path                       | /index.html   |
    * | 6     | :scheme                     | http          |
    * | 7     | :scheme                     | https         |
    * | 8     | :status                     | 200           |
    * | 9     | :status                     | 204           |
    * | 10    | :status                     | 206           |
    * | 11    | :status                     | 304           |
    * | 12    | :status                     | 400           |
    * | 13    | :status                     | 404           |
    * | 14    | :status                     | 500           |
    * | ...   | ...                         | ...           |
    * +-------+-----------------------------+---------------+
    */

   static bool isHeaderToErase(UStringRep* key)
      {
      U_TRACE(0, "UHTTP2::isHeaderToErase(%V)", key)

      if (key == UString::str_path->rep       ||
          key == UString::str_method->rep     ||
          key == UString::str_authority->rep  ||
          key == UString::str_user_agent->rep ||
          key == UString::str_accept_encoding->rep)
         {
         U_RETURN(false);
         }

      U_RETURN(true);
      }

   static bool isHeaderToCopy(UStringRep* key)
      {
      U_TRACE(0, "UHTTP2::isHeaderToCopy(%V)", key)

      if (isHeaderToErase(key) == false           &&
          key != UString::str_cookie->rep         &&
          key != UString::str_accept->rep         &&
          key != UString::str_referer->rep        &&
          key != UString::str_content_type->rep   &&
          key != UString::str_content_length->rep &&
          key != UString::str_accept_language->rep)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }
      
   static bool eraseHeaders(UStringRep* key, void* elem) // callWithDeleteForAllEntry()...
      {
      U_TRACE(0, "UHTTP2::eraseHeaders(%V,%p)", key, elem)

      if (isHeaderToErase(key)) U_RETURN(true);

      U_RETURN(false);
      }

   static bool copyHeaders(UStringRep* key, void* elem)
      {
      U_TRACE(0, "UHTTP2::copyHeaders(%V,%p)", key, elem)

      if (isHeaderToCopy(key)) UClientImage_Base::request->snprintf_add(U_CONSTANT_TO_PARAM("%v: %v\r\n"), key, (const UStringRep*)elem);

      U_RETURN(true);
      }

   static void downgradeRequest(); // HTTP2 => HTTP1

#ifdef USE_LOAD_BALANCE
   static bool bproxy;

   static void wrapRequest();
#endif

   static bool setIndexStaticTable(UHashMap<void*>* table, const char* key, uint32_t length)
      {
      U_TRACE(0, "UHTTP2::setIndexStaticTable(%p,%.*S,%u)", table, length, key, length)

   // if (bhash) table->hash = u_hash_ignore_case((unsigned char*)key, length);

      U_INTERNAL_DUMP("table->hash = %u", table->hash)

      U_INTERNAL_ASSERT_MAJOR(table->hash, 0)

      table->index = table->hash % table->_capacity;

      U_RETURN(true); // NB: ignore case...
      }

   // HPACK (HTTP headers compression)

   static bool isHpackError()
      {
      U_TRACE_NO_PARAM(0, "UHTTP2::isHpackError()")

      U_INTERNAL_DUMP("hpack_errno = %d", hpack_errno)

      if (hpack_errno)
         {
         nerror = COMPRESSION_ERROR;

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool isHpackError(int32_t index)
      {
      U_TRACE(0, "UHTTP2::isHpackError(%d)", index)

      U_INTERNAL_DUMP("index = %d hpack_errno = %d", index, hpack_errno)

      if (index == -1 ||
          hpack_errno)
         {
#     ifdef DEBUG
         if (hpack_errno == 0) hpack_errno = -2; // The decoding buffer ends before the decoded HPACK block
#     endif

         nerror = COMPRESSION_ERROR;

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static void setHpackDynTblCapacity(HpackDynamicTable* dyntbl, uint32_t value)
      {
      U_TRACE(0, "UHTTP2::setHpackDynTblCapacity(%p,%u)", dyntbl, value)

      U_INTERNAL_DUMP("hpack_capacity = %u hpack_max_capacity = %u", dyntbl->hpack_capacity, dyntbl->hpack_max_capacity)

      U_INTERNAL_ASSERT_DIFFERS(dyntbl->hpack_capacity, value)

      dyntbl->hpack_capacity = value;

      // adjust the size

      while (dyntbl->num_entries > 0 &&
             dyntbl->hpack_size > dyntbl->hpack_capacity)
         {
         evictHpackDynTblFirstEntry(dyntbl);
         }
      }

   static void setHpackInputDynTblCapacity(uint32_t value)
      {
      U_TRACE(0, "UHTTP2::setHpackInputDynTblCapacity(%u)", value)

      HpackDynamicTable* dyntbl = &(pConnection->idyntbl);

      U_INTERNAL_DUMP("hpack_capacity = %u hpack_max_capacity = %u", dyntbl->hpack_capacity, dyntbl->hpack_max_capacity)

      if (value != dyntbl->hpack_capacity)
         {
         dyntbl->hpack_max_capacity = value;

         setHpackDynTblCapacity(dyntbl, value);
         }
      }

   static unsigned char* setHpackOutputDynTblCapacity(unsigned char* dst, uint32_t& value)
      {
      U_TRACE(0, "UHTTP2::setHpackOutputDynTblCapacity(%p,%u)", dst, value)

      HpackDynamicTable* dyntbl = &(pConnection->odyntbl);

      U_INTERNAL_DUMP("hpack_capacity = %u hpack_max_capacity = %u", dyntbl->hpack_capacity, dyntbl->hpack_max_capacity)

      if (value > 4096) value = 4096;

      if (value != dyntbl->hpack_capacity)
         {
         setHpackDynTblCapacity(dyntbl, value);

         dst = hpackEncodeInt(dst, value, (1<<5)-1, 0x20);
         }

      U_RETURN_POINTER(dst, unsigned char);
      }

   static HpackHeaderTableEntry* getHpackDynTblEntry(HpackDynamicTable* dyntbl, uint32_t index)
      {
      U_TRACE(0, "UHTTP2::getHpackDynTblEntry(%p,%u)", dyntbl, index)

      U_INTERNAL_DUMP("dyntbl->entry_start_index = %u dyntbl->entry_capacity = %u dyntbl->num_entries = %u", dyntbl->entry_start_index, dyntbl->entry_capacity, dyntbl->num_entries)

      U_INTERNAL_ASSERT_MAJOR(dyntbl->num_entries, 0)
      U_INTERNAL_ASSERT_MINOR(index, dyntbl->num_entries)

      HpackHeaderTableEntry* entry = dyntbl->entries + ((dyntbl->entry_start_index+index) % dyntbl->entry_capacity);

      U_INTERNAL_ASSERT_POINTER(entry->name)
      U_INTERNAL_ASSERT_POINTER(entry->value)

      U_INTERNAL_DUMP("entry->name = %V entry->value = %V", entry->name, entry->value)

      U_RETURN_POINTER(entry, HpackHeaderTableEntry);
      }

   static void evictHpackDynTblEntry(HpackDynamicTable* dyntbl, HpackHeaderTableEntry* entry)
      {
      U_TRACE(0, "UHTTP2::evictHpackDynTblEntry(%p,%p)", dyntbl, entry)

      dyntbl->num_entries--;

      dyntbl->hpack_size -= entry->name->size() + entry->value->size() + HTTP2_HEADER_TABLE_ENTRY_SIZE_OFFSET;

      U_INTERNAL_ASSERT_POINTER(entry->name)
      U_INTERNAL_ASSERT_POINTER(entry->value)

      // NB: we decreases the reference string...

      U_INTERNAL_DUMP("entry->name = %V entry->value = %V", entry->name, entry->value)

      entry->name->release();
      entry->value->release();

      entry->name  =
      entry->value = 0;
      }

   static void evictHpackDynTblFirstEntry(HpackDynamicTable* dyntbl)
      {
      U_TRACE(0, "UHTTP2::evictHpackDynTblFirstEntry(%p)", dyntbl)

      evictHpackDynTblEntry(dyntbl, getHpackDynTblEntry(dyntbl, dyntbl->num_entries-1));
      }

   static void evictHpackDynTblLastEntry(HpackDynamicTable* dyntbl)
      {
      U_TRACE(0, "UHTTP2::evictHpackDynTblLastEntry(%p)", dyntbl)

      evictHpackDynTblEntry(dyntbl, getHpackDynTblEntry(dyntbl, 0));

      dyntbl->entry_start_index = (dyntbl->entry_start_index+1+dyntbl->entry_capacity) % dyntbl->entry_capacity;
      }

   static void clearHpackDynTbl(     HpackDynamicTable* dyntbl);
   static void   addHpackDynTblEntry(HpackDynamicTable* dyntbl, const UString& name, const UString& value);
   static void evictHpackDynTblEntry(HpackDynamicTable* dyntbl, HpackHeaderTableEntry* entry, uint32_t index);

   static void addHpackOutputDynTblEntry(const UString& name, const UString& value) { addHpackDynTblEntry(&(pConnection->odyntbl), name, value); }

   static unsigned char* hpackDecodeInt(   unsigned char* src, unsigned char* src_end, int32_t& value, uint8_t prefix_max);
   static unsigned char* hpackDecodeString(unsigned char* src, unsigned char* src_end, UString& value);

   static unsigned char* hpackEncodeInt(unsigned char* dst, uint32_t value, uint8_t prefix_max, uint8_t pattern)
      {
      U_TRACE(0+256, "UHTTP2::hpackEncodeInt(%p,%u,%u,%u)", dst, value, prefix_max, pattern)

      /*
#  ifdef DEBUG
      unsigned char* src  = dst;
      uint32_t value_save = value;
#  endif
      */

      if (value < prefix_max) *dst++ = (uint8_t)(value | pattern);
      else
         {
         *dst++ = (pattern | prefix_max);

         for (value -= prefix_max; value >= 0x80; value >>= 7) *dst++ = 0x80 | (value & 0x7f);

         *dst++ = (uint8_t)value;
         }

      /*
#  ifdef DEBUG
      int32_t index;
      (void) hpackDecodeInt(src, src+(dst-src), index, prefix_max);
      U_INTERNAL_ASSERT_EQUALS((uint32_t)index, value_save)
#  endif
      */

      U_RETURN_POINTER(dst, unsigned char);
      }

   static unsigned char* setHpackEncodeStringLen(unsigned char* dst, uint32_t sz)
      {
      U_TRACE(0, "UHTTP2::setHpackEncodeStringLen(%p,%u)", dst, sz)

      uint32_t l = sz >> 3;

      if (sz & 0x07) ++l;

      dst = hpackEncodeInt(dst, l, (1<<7)-1, 0x80);

      U_RETURN_POINTER(dst, unsigned char);
      }

   // Header field grammar validation (RFC 7230 Section 3.2)

   static bool isHeaderName( const char* s, uint32_t n) __pure;
   static bool isHeaderValue(const char* s, uint32_t n) __pure;

   static bool isHeaderName( const UString& s) { return isHeaderName( U_STRING_TO_PARAM(s)); }
   static bool isHeaderValue(const UString& s) { return isHeaderValue(U_STRING_TO_PARAM(s)); } 

   static    const char* getFrameErrorCodeDescription(uint32_t error);
   static unsigned char* hpackEncodeHeader(unsigned char* dst, const UString& key, const UString& value);

   static void decodeHeaders(UHashMap<UString>* itable, HpackDynamicTable* dyntbl, unsigned char* ptr, unsigned char* endptr);

   static unsigned char* hpackEncodeString(unsigned char* dst, const char* src, uint32_t len, bool bhuffman);
   static unsigned char* hpackEncodeString(unsigned char* dst, const UString& value,          bool bhuffman) { return hpackEncodeString(dst, U_STRING_TO_PARAM(value), bhuffman); }

#ifdef DEBUG
   typedef struct { const char* str; int value; const char* desc; } HpackError;

   static unsigned char* index_ptr;
   static HpackError hpack_error[8];
   static bool btest, bdecodeHeadersDebug, bhash;

   static const char* hpack_strerror()
      {
      U_TRACE_NO_PARAM(0, "UHTTP2::hpack_strerror()")

      U_INTERNAL_DUMP("hpack_errno = %d hpack_error[%d] = %S", hpack_errno, -hpack_errno-2, hpack_error[-hpack_errno-2].desc)

      return hpack_error[-hpack_errno-2].desc;
      }

   static bool findHeader(uint32_t index)
      {
      U_TRACE(0, "UHTTP2::findHeader(%u)", index)

      UHashMap<UString>* table = &(pConnection->dtable);

      table->hash = hash_static_table[index];

      table->lookup(hpack_static_table[index].name);

      if (table->node) U_RETURN(true);

      U_RETURN(false);
      }

   static const char* getStreamStatusDescription();
   static const char* getConnectionStatusDescription();
   static const char* getFrameTypeDescription(char type);

   static void printHpackDynamicTable(HpackDynamicTable* dyntbl, ostream& os);

   static void printHpackInputDynTable()  { printHpackDynamicTable(&(pConnection->idyntbl), cout); }
   static void printHpackOutputDynTable() { printHpackDynamicTable(&(pConnection->odyntbl), cout); }

# ifdef U_STDCPP_ENABLE
   friend ostream& operator<<(ostream& os, HpackDynamicTable& v)
      {
      U_TRACE(0+256, "HpackDynamicTable::operator<<(%p,%p)", &os, &v)

      printHpackDynamicTable(&v, os);

      return os;
      }
# endif

   /**
    * static void eraseHeader(uint32_t index)
    * {
    * U_TRACE(0, "UHTTP2::eraseHeader(%u)", index)
    *
    * UHashMap<UString>* table = &(pConnection->itable);
    *
    * table->hash = hash_static_table[index];
    *
    * table->lookup(hpack_static_table[index].name);
    *
    * if (table->node) table->eraseAfterFind();
    * }
    *
    * static void decodeHeadersResponse(unsigned char* ptr, uint32_t length)
    * {
    * U_TRACE(0, "UHTTP2::decodeHeadersResponse(%p,%u)", ptr, length)
    *
    * bdecodeHeadersDebug = true;
    *
    * decodeHeaders(&(pConnection->dtable),
    *               &(pConnection->ddyntbl), ptr, ptr+length));
    *
    * bdecodeHeadersDebug = false;
    *
    * U_DUMP_OBJECT_TO_TMP(pConnection->dtable,  response.dtable)
    * U_DUMP_OBJECT_TO_TMP(pConnection->ddyntbl, response.ddyntbl)
    *
    * U_ASSERT(findHeader(53)) // UString::str_server
    * U_ASSERT(findHeader(32)) // UString::str_date
    * U_ASSERT(findHeader(27)) // UString::str_content_length
    *
    * pConnection->dtable.clear();
    * clearHpackDynTbl(&(pConnection->ddyntbl));
    * }
    *
    * We save the Header Block Fragment of the frame to inspect it with inflatehd (https://github.com/tatsuhiro-t/nghttp2)
    *
    * ./inflatehd < inflatehd.json => { "cases": [ { "wire": "8285" } ] }
    *
    * static void saveHpackData(const char* ptr, uint32_t len, bool breq)
    * {
    * U_TRACE(0+256, "UHTTP2::saveHpackData(%.*S,%u,%b)", len, ptr, len, breq)
    *
    * UString tmp(U_CAPACITY);
    *
    * UHexDump::encode(ptr, len, tmp);
    *
    * (void) UFile::writeToTmp(U_STRING_TO_PARAM(tmp), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("%s.hpack.%P"), breq ? "request" : "response");
    * }
    */
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHTTP2)

   friend class UHTTP;
   friend class Application;
   friend class UClientImage_Base;
};
#endif
