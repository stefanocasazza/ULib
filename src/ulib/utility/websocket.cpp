// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    websocket.cpp - web socket utility 
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/command.h>
#include <ulib/utility/uhttp.h>
#include <ulib/utility/services.h>
#include <ulib/utility/websocket.h>
#include <ulib/net/server/server.h>

#define U_WS_DATA_FRAMING_MASK               0
#define U_WS_DATA_FRAMING_START              1
#define U_WS_DATA_FRAMING_PAYLOAD_LENGTH     2
#define U_WS_DATA_FRAMING_PAYLOAD_LENGTH_EXT 3
#define U_WS_DATA_FRAMING_EXTENSION_DATA     4
#define U_WS_DATA_FRAMING_APPLICATION_DATA   5
#define U_WS_DATA_FRAMING_CLOSE              6

#define U_WS_FRAME_GET_FIN(BYTE)         (((BYTE) >> 7) & 0x01)
#define U_WS_FRAME_GET_RSV1(BYTE)        (((BYTE) >> 6) & 0x01)
#define U_WS_FRAME_GET_RSV2(BYTE)        (((BYTE) >> 5) & 0x01)
#define U_WS_FRAME_GET_RSV3(BYTE)        (((BYTE) >> 4) & 0x01)
#define U_WS_FRAME_GET_OPCODE(BYTE)      ( (BYTE)       & 0x0F)
#define U_WS_FRAME_GET_MASK(BYTE)        (((BYTE) >> 7) & 0x01)
#define U_WS_FRAME_GET_PAYLOAD_LEN(BYTE) ( (BYTE)       & 0x7F)

#define U_WS_FRAME_SET_FIN(BYTE)         (((BYTE) & 0x01) << 7)
#define U_WS_FRAME_SET_OPCODE(BYTE)       ((BYTE) & 0x0F)
#define U_WS_FRAME_SET_MASK(BYTE)        (((BYTE) & 0x01) << 7)
#define U_WS_FRAME_SET_LENGTH(X64, IDX)  (unsigned char)(((uint64_t)(X64) >> ((IDX)*8)) & 0xFF)

#define U_WS_GUID     "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define U_WS_GUID_LEN 36

int         UWebSocket::fd_stderr;
int         UWebSocket::status_code;
int         UWebSocket::message_type;
int         UWebSocket::timeoutMS;
vPF         UWebSocket::on_message;
vPFu        UWebSocket::on_message_param;
UString*    UWebSocket::rbuffer;
UString*    UWebSocket::message;
UString*    UWebSocket::penvironment;
uint32_t    UWebSocket::max_message_size;
UCommand*   UWebSocket::command;
const char* UWebSocket::upgrade_settings;

UWebSocket::uwrec*                UWebSocket::rec;
URDBObjectHandler<UDataStorage*>* UWebSocket::db;

UWebSocket::WebSocketFrameData UWebSocket::control_frame = { U_NULLPTR, 0, 1, 8, 0 };
UWebSocket::WebSocketFrameData UWebSocket::message_frame = { U_NULLPTR, 0, 1, 0, 0 };

RETSIGTYPE UWebSocket::handlerForSigTERM(int signo)
{
   U_TRACE(0, "[SIGTERM] UWebSocket::handlerForSigTERM(%d)", signo)

   UInterrupt::sendOurselves(SIGTERM);
}

bool UWebSocket::checkForInitialData()
{
   U_TRACE_NO_PARAM(0, "UWebSocket::checkForInitialData()")

   U_INTERNAL_ASSERT_POINTER(rbuffer)
   U_INTERNAL_ASSERT_MAJOR(UClientImage_Base::size_request, 0)

   uint32_t sz = UClientImage_Base::rbuffer->size();

   U_INTERNAL_DUMP("UClientImage_Base::size_request = %u UClientImage_Base::rbuffer(%u) = %V", UClientImage_Base::size_request, sz, UClientImage_Base::rbuffer->rep)

   if (UClientImage_Base::size_request < sz)
      {
      // we have read more data than necessary...

      U_ASSERT(rbuffer->empty())

      (void) rbuffer->assign(UClientImage_Base::rbuffer->c_pointer(UClientImage_Base::size_request), sz - UClientImage_Base::size_request);

      U_INTERNAL_DUMP("rbuffer(%u) = %V", rbuffer->size(), rbuffer->rep)

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UWebSocket::sendAccept(USocket* socket)
{
   U_TRACE(0, "UWebSocket::sendAccept(%p)", socket)

   U_INTERNAL_ASSERT_MAJOR(U_http_websocket_len, 0)

   // In order to establish a websocket connection, a client (a web browser) sends a HTTP GET request with a number of HTTP headers. Among those
   // headers there is the Sec-WebSocket-Key header, which contains a handshake key. According to the WebSocket protocol, the server should:
   //
   // 1) Concatenate the handshake key with the magic guid {258EAFA5-E914-47DA-95CA-C5AB0DC85B11}
   // 2) Take the SHA1 hash of the concatenation result
   // 3) Send the base64 equivalent of the hash in HTTP response to the client

   unsigned char challenge[128];

   U_MEMCPY(challenge,                      upgrade_settings, U_http_websocket_len);
   U_MEMCPY(challenge+U_http_websocket_len, U_WS_GUID, U_WS_GUID_LEN);

   // SHA1(challenge)

   UString accept(U_CAPACITY), buffer(U_CAPACITY);

   UServices::generateDigest(U_HASH_SHA1, 0, challenge, U_http_websocket_len + U_WS_GUID_LEN, accept, true);

   buffer.snprintf(U_CONSTANT_TO_PARAM("HTTP/1.1 101 Switching Protocols\r\n"
                                        "Upgrade: websocket\r\n"
                                        "Connection: Upgrade\r\n"
                                        "Sec-WebSocket-Accept: %v\r\n\r\n"), accept.rep);

   if (USocketExt::write(socket, buffer, UServer_Base::timeoutMS))
      {
      status_code  = U_WS_STATUS_CODE_INTERNAL_ERROR;
      message_type = U_WS_MESSAGE_TYPE_INVALID;

      if (max_message_size == 0) max_message_size = U_STRING_MAX_SIZE;

      U_RETURN(true);
      }

   U_RETURN(false);
}

/**
 * @see: http://tools.ietf.org/html/rfc6455#section-5.2 Base Framing Protocol
 *
 * So, WebSockets presents a sequence of infinitely long byte streams with a termination indicator (the FIN bit in the frame header) and
 * not a message based interface as you might initially believe. Given that a general purpose protocol handler can only work in terms of
 * partial frames, we effectively have a stream based protocol with lots of added complexity to provide the illusion of a message based
 * protocol that can actually only ever be dealt with as a stream of bytes
 *
 * +-+-+-+-+-------+-+-------------+-------------------------------+
 * 0                   1                   2                   3   |
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 |
 * +-+-+-+-+-------+-+-------------+-------------------------------+
 * |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 * |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 * |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 * | |1|2|3|       |K|             |                               |
 * +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 * |     Extended payload length continued, if payload len == 127  |
 * + - - - - - - - - - - - - - - - +-------------------------------+
 * |                               | Masking-key, if MASK set to 1 |
 * +-------------------------------+-------------------------------+
 * | Masking-key (continued)       |          Payload Data         |
 * +-------------------------------- - - - - - - - - - - - - - - - +
 * :                     Payload Data continued ...                :
 * + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 * |                     Payload Data continued ...                |
 * +---------------------------------------------------------------+
 */

int UWebSocket::handleDataFraming(UString* pbuffer, USocket* socket)
{
   U_TRACE(0, "UWebSocket::handleDataFraming(%V,%p)", pbuffer->rep, socket)

   U_INTERNAL_ASSERT_POINTER(pbuffer)

   unsigned char* block;
   uint32_t block_offset, ncount = 0, block_size;
   WebSocketFrameData* frame = &UWebSocket::control_frame;
   unsigned char fin = 0, opcode = 0xFF, mask[4] = { 0, 0, 0, 0 };
   int32_t extension_bytes_remaining = 0, payload_length = 0, mask_offset = 0;
   int framing_state = U_WS_DATA_FRAMING_START, payload_length_bytes_remaining = 0, mask_index = 0, masking = 0;

loop:
   U_INTERNAL_DUMP("timeoutMS = %d buffer(%u) = %V", timeoutMS, pbuffer->size(), pbuffer->rep)

   if (pbuffer->empty() &&
       USocketExt::read(socket, *pbuffer, U_SINGLE_READ, timeoutMS) == false)
      {
      status_code = U_WS_STATUS_CODE_INTERNAL_ERROR;

      U_RETURN(U_WS_STATUS_CODE_INTERNAL_ERROR);
      }

              block        = (unsigned char*) pbuffer->data();
   ncount += (block_size   =                  pbuffer->size());
              block_offset = 0;

   do {
      U_INTERNAL_DUMP("framing_state = %d", framing_state)

      switch (framing_state)
         {
         case U_WS_DATA_FRAMING_START: // 1
            {
            // Since we don't currently support any extensions, the reserve bits must be 0

            if ((U_WS_FRAME_GET_RSV1(block[block_offset]) != 0) ||
                (U_WS_FRAME_GET_RSV2(block[block_offset]) != 0) ||
                (U_WS_FRAME_GET_RSV3(block[block_offset]) != 0))
               {
               // framing_state = U_WS_DATA_FRAMING_CLOSE; // 6

               status_code = U_WS_STATUS_CODE_PROTOCOL_ERROR;

               U_RETURN(U_WS_STATUS_CODE_PROTOCOL_ERROR);
               }

            fin    = U_WS_FRAME_GET_FIN(   block[block_offset]);
            opcode = U_WS_FRAME_GET_OPCODE(block[block_offset++]);

            U_INTERNAL_DUMP("fin = %d opcode = %X", fin, opcode)

            framing_state = U_WS_DATA_FRAMING_PAYLOAD_LENGTH; // 2

            if (opcode >= 0x8) // Control frame
               {
               if (fin == 0)
                  {
                  // framing_state = U_WS_DATA_FRAMING_CLOSE; // 6

                  status_code = U_WS_STATUS_CODE_PROTOCOL_ERROR;

                  U_RETURN(U_WS_STATUS_CODE_PROTOCOL_ERROR);
                  }

               frame             = &UWebSocket::control_frame;
               frame->opcode     = opcode;
               frame->utf8_state = 0;
               }
            else // Message frame
               {
               frame = &UWebSocket::message_frame;

               if (opcode)
                  {
                  if (frame->fin == 0)
                     {
                     // framing_state = U_WS_DATA_FRAMING_CLOSE; // 6

                     status_code = U_WS_STATUS_CODE_PROTOCOL_ERROR;

                     U_RETURN(U_WS_STATUS_CODE_PROTOCOL_ERROR);
                     }

                  frame->opcode     = opcode;
                  frame->utf8_state = 0;
                  }
               else if (frame->fin ||
                        ((opcode = frame->opcode) == 0))
                  {
                  // framing_state = U_WS_DATA_FRAMING_CLOSE; // 6

                  status_code = U_WS_STATUS_CODE_PROTOCOL_ERROR;

                  U_RETURN(U_WS_STATUS_CODE_PROTOCOL_ERROR);
                  }

               frame->fin = fin;
               }

            payload_length                 = 0;
            payload_length_bytes_remaining = 0;

            if (block_offset >= block_size) goto next;

            U_INTERNAL_DUMP("framing_state = %d", framing_state)
            }

         /* FALL THRU */

         case U_WS_DATA_FRAMING_PAYLOAD_LENGTH: // 2
            {
            payload_length = U_WS_FRAME_GET_PAYLOAD_LEN(block[block_offset]);
            masking        = U_WS_FRAME_GET_MASK(       block[block_offset++]);

            U_INTERNAL_DUMP("masking = %d payload_length = %d", masking, payload_length)

            if (payload_length == 126)
               {
               payload_length                 = 0;
               payload_length_bytes_remaining = 2;
               }
            else if (payload_length == 127)
               {
               payload_length                 = 0;
               payload_length_bytes_remaining = 8;
               }
            else
               {
               payload_length_bytes_remaining = 0;
               }

            if ((masking == 0)   || // Client-side mask is required
                ((opcode >= 0x8) && // Control opcodes cannot have a payload larger than 125 bytes
                (payload_length_bytes_remaining != 0)))
               {
               // framing_state = U_WS_DATA_FRAMING_CLOSE; // 6

               status_code = U_WS_STATUS_CODE_PROTOCOL_ERROR;

               U_RETURN(U_WS_STATUS_CODE_PROTOCOL_ERROR);
               }

            framing_state = U_WS_DATA_FRAMING_PAYLOAD_LENGTH_EXT; // 3

            if (block_offset >= block_size) goto next;

            U_INTERNAL_DUMP("framing_state = %d", framing_state)
            }

         /* FALL THRU */

         case U_WS_DATA_FRAMING_PAYLOAD_LENGTH_EXT: // 3
            {
            while ((payload_length_bytes_remaining > 0) &&
                   (block_offset < block_size))
               {
               payload_length *= 256;
               payload_length += block[block_offset++];

               payload_length_bytes_remaining--;
               }

            if (payload_length_bytes_remaining == 0)
               {
               if ((          payload_length < 0) ||
                   ((uint32_t)payload_length > max_message_size))
                  {
                  U_SRV_LOG_WITH_ADDR("Got frame with payload greater than maximum frame buffer size: (%u > %u) from", payload_length, max_message_size);

                  // framing_state = U_WS_DATA_FRAMING_CLOSE; // 6

                  status_code = U_WS_STATUS_CODE_MESSAGE_TOO_LARGE; // Invalid payload length

                  U_RETURN(U_WS_STATUS_CODE_MESSAGE_TOO_LARGE);
                  }

               if (masking == 0)
                  {
                  framing_state = U_WS_DATA_FRAMING_EXTENSION_DATA; // 4

                  break;
                  }

               framing_state = U_WS_DATA_FRAMING_MASK; // 0
               }

            if (block_offset >= block_size) goto next;

            U_INTERNAL_DUMP("framing_state = %d", framing_state)
            }

         /* FALL THRU */

         case U_WS_DATA_FRAMING_MASK: // 0
            {
            U_INTERNAL_DUMP("mask_index = %d", mask_index)

            while ((mask_index < 4) && (block_offset < block_size)) mask[mask_index++] = block[block_offset++];

            U_INTERNAL_DUMP("mask_index = %d", mask_index)

            if (mask_index != 4) break;

            mask_index    = 0;
            mask_offset   = 0;
            framing_state = U_WS_DATA_FRAMING_EXTENSION_DATA; // 4

            if ((mask[0] == 0) &&
                (mask[1] == 0) &&
                (mask[2] == 0) &&
                (mask[3] == 0))
               {
               masking = 0;
               }

            U_INTERNAL_DUMP("masking = %d framing_state = %d", masking, framing_state)
            }

         /* FALL THRU */

         case U_WS_DATA_FRAMING_EXTENSION_DATA: // 4
            {
            // Deal with extension data when we support them -- FIXME

            U_INTERNAL_DUMP("extension_bytes_remaining = %d", extension_bytes_remaining)

            if (extension_bytes_remaining == 0)
               {
               if (payload_length > 0)
                  {
                  (void) message->setBuffer(frame->application_data_offset + payload_length);

                  frame->application_data = (unsigned char*) message->data();
                  }

               framing_state = U_WS_DATA_FRAMING_APPLICATION_DATA; // 5
               }

            U_INTERNAL_DUMP("framing_state = %d", framing_state)
            }

         /* FALL THRU */

         case U_WS_DATA_FRAMING_APPLICATION_DATA: // 5
            {
            int32_t block_length      = block_size - block_offset,
                    block_data_length = (payload_length > block_length ? block_length
                                                                       : payload_length);

            uint32_t       application_data_offset = frame->application_data_offset;
            unsigned char* application_data        = frame->application_data;

            if (masking)
               {
               int32_t i;

               if (opcode == U_WS_OPCODE_TEXT)
                  {
                  unsigned int utf8_state = frame->utf8_state;

                  for (i = 0; i < block_data_length; ++i)
                     {
                     unsigned char c = block[block_offset++] ^ mask[mask_offset++ & 3];

                     utf8_state = u_validate_utf8[utf8_state + c];

                     if (utf8_state == 1)
                        {
                        payload_length = block_data_length;

                        break;
                        }

                     application_data[application_data_offset++] = c;
                     }

                  frame->utf8_state = utf8_state;
                  }
               else
                  {
                  // Need to optimize the unmasking -- FIXME

                  for (i = 0; i < block_data_length; ++i)
                     {
                     application_data[application_data_offset++] = block[block_offset++] ^ mask[mask_offset++ & 3];
                     }
                  }
               }
            else if (block_data_length > 0)
               {
               U_MEMCPY(&application_data[application_data_offset], &block[block_offset], block_data_length);
               
               if (opcode == U_WS_OPCODE_TEXT)
                  {
                  unsigned int utf8_state = frame->utf8_state;
                  int32_t i, application_data_end = application_data_offset + block_data_length;

                  for (i = application_data_offset; i < application_data_end; i++)
                     {
                     utf8_state = u_validate_utf8[utf8_state + application_data[i]];

                     if (utf8_state == 1)
                        {
                        payload_length = block_data_length;

                        break;
                        }
                     }

                  frame->utf8_state = utf8_state;
                  }

                          block_offset += block_data_length;
               application_data_offset += block_data_length;
               }

            payload_length -= block_data_length;

            if (payload_length == 0)
               {
               message_type = U_WS_MESSAGE_TYPE_INVALID;

               switch (opcode)
                  {
                  case U_WS_OPCODE_TEXT:
                     {
                     if ((fin &&
                         (frame->utf8_state != 0)) ||
                         (frame->utf8_state == 1))
                        {
                        // framing_state = U_WS_DATA_FRAMING_CLOSE; // 6

                        status_code = U_WS_STATUS_CODE_INVALID_UTF8;

                        U_RETURN(U_WS_STATUS_CODE_INVALID_UTF8);
                        }

                     message_type = U_WS_MESSAGE_TYPE_TEXT;
                     }
                  break;

                  case U_WS_OPCODE_BINARY: message_type = U_WS_MESSAGE_TYPE_BINARY; break;
                  case U_WS_OPCODE_BROTLI: message_type = U_WS_MESSAGE_TYPE_BROTLI; break;

                  case U_WS_OPCODE_CLOSE:
                     {
                     // framing_state = U_WS_DATA_FRAMING_CLOSE; // 6

                     status_code = U_WS_STATUS_CODE_GOING_AWAY;

                     U_RETURN(U_WS_STATUS_CODE_GOING_AWAY);
                     }

                  case U_WS_OPCODE_PING:
                     {
                     // implies only client initiates ping pong... so any pong is always server -> client
                     if (sendControlFrame(true, socket, U_WS_OPCODE_PONG, application_data, application_data_offset) == false)
                        {
                        U_RETURN(U_WS_STATUS_CODE_PROTOCOL_ERROR);
                        }
                     }
                  break;

                  case U_WS_OPCODE_PONG: break;

                  default:
                     {
                     // framing_state = U_WS_DATA_FRAMING_CLOSE; // 6

                     status_code = U_WS_STATUS_CODE_PROTOCOL_ERROR;

                     U_RETURN(U_WS_STATUS_CODE_PROTOCOL_ERROR);
                     }
                  }

               if (fin)
                  {
                  U_INTERNAL_DUMP("framing_state = %d message_type = %d status_code = %d", framing_state, message_type, status_code)

                  if (message_type != U_WS_MESSAGE_TYPE_INVALID)
                     {
                     U_INTERNAL_ASSERT_EQUALS(framing_state, U_WS_DATA_FRAMING_APPLICATION_DATA)

                     message->size_adjust_force(application_data_offset);

                     U_SRV_LOG_WITH_ADDR("received websocket data (%u+%u bytes) %V from",
                                             ncount - message->size(),
                                                      message->size(), message->rep)

                     status_code = U_WS_STATUS_CODE_OK;

#                 ifdef USE_LIBBROTLI
                     if (message_type == U_WS_MESSAGE_TYPE_BROTLI &&
                         UStringExt::isBrotli(*message))
                        {
                        *message = UStringExt::unbrotli(*message);
                        }
#                 endif

                     U_RETURN(U_WS_STATUS_CODE_OK);
                     }

                  frame->application_data        = U_NULLPTR;
                         application_data_offset = 0;
                  }

               framing_state = U_WS_DATA_FRAMING_START; // 1
               }

            frame->application_data_offset = application_data_offset;
            }
         break;

      // case U_WS_DATA_FRAMING_CLOSE: block_offset = block_size; break;

         default:
            {
            // framing_state = U_WS_DATA_FRAMING_CLOSE; // 6

            status_code = U_WS_STATUS_CODE_PROTOCOL_ERROR;

            U_RETURN(U_WS_STATUS_CODE_PROTOCOL_ERROR);
            }
         }
      }
   while (block_offset < block_size);

next:
   U_INTERNAL_ASSERT(block_offset >= block_size)
   U_INTERNAL_ASSERT_DIFFERS(framing_state, U_WS_DATA_FRAMING_CLOSE) // 6

   pbuffer->setEmpty();

   goto loop;
}

bool UWebSocket::sendData(const bool isServer, USocket* socket, int type, const char* data, uint32_t len)
{
   U_TRACE(0, "UWebSocket::sendData(%b,%p,%d,%.*S,%u)", isServer, socket, type, len, data, len)

   /*
   if (UNLIKELY(len > 0xffffffff))
   {
      status_code = U_WS_STATUS_CODE_MESSAGE_TOO_LARGE;

      U_RETURN(false);
   }
   */

   uint8_t opcode, masking_key[4];

   U_DUMP("len = %lu", len);
   // 0xffff == 65535
   uint32_t header_length = (len > 125U ? 2U : 0) + (len > 0xffff ? 8U : 0);

   U_DUMP("header_length = %lu", header_length);

   if (isServer) header_length += 2U;
   else
   {
      header_length += 6U;
      *((uint32_t*)masking_key) = u_get_num_random();
   }

   uint32_t ncount = header_length + len;

   // 1 MB
   static UString buffer(1048576U);
   buffer.setEmpty();
   buffer.reserve(ncount);

   unsigned char* header = (unsigned char*)buffer.data();

   switch (type)
      {
      case U_WS_MESSAGE_TYPE_TEXT:
      case U_WS_MESSAGE_TYPE_INVALID:
         opcode = U_WS_OPCODE_TEXT;
      break;

      case U_WS_MESSAGE_TYPE_BROTLI:
         {
         opcode = U_WS_OPCODE_TEXT;

#     ifdef USE_LIBBROTLI
         UString compressed;

         if (compressed = UStringExt::brotli(data, len, (U_PARALLELIZATION_CHILD ? BROTLI_MAX_QUALITY : UHTTP::brotli_level_for_dynamic_content)))
            {
            opcode = U_WS_OPCODE_BROTLI;

            len  = compressed.size();
            data = compressed.data();

            U_SRV_LOG("websocket compressed request: %u bytes - (%u%%) brotli compression ratio", len, 100-UStringExt::ratio);
            }
#     endif
         }
      break;

      case U_WS_MESSAGE_TYPE_PING:   opcode = U_WS_OPCODE_PING;   break;
      case U_WS_MESSAGE_TYPE_PONG:   opcode = U_WS_OPCODE_PONG;   break;
      case U_WS_MESSAGE_TYPE_BINARY: opcode = U_WS_OPCODE_BINARY; break;

      case U_WS_MESSAGE_TYPE_CLOSE:
      default:
         opcode = U_WS_OPCODE_CLOSE;
      break;
      }

   header[0] = (opcode | 0x80);

   // possible client header lengths
   //    2  4  12
   // possible server header lengths
   //    6  8  16

   switch (header_length)
   {
      // server + len < 125
      case 2:
      {
         header[1] = len;
         break;
      }
      // server + (len >  125 && len <= 0xffff) // 125 && 65535
      case 4:
      {
         header[1] = 126;
         u_put_unalignedp16(header+2, htons((uint16_t)len));
         break;
      }
      case 12:
      {
         header[1] = 127;
         u_put_unalignedp64(header+2, htonl(len));
         break;
      }
      // client + len < 125
      case 6:
      {
         header[1] = (len | 0x80);
         u_put_unalignedp32(header+2, *((uint32_t*)masking_key));
         break;
      }
      // client + (len >  125 && len <= 0xffff) // 125 && 65535
      case 8:
      {
         header[1] = (126 | 0x80);

         u_put_unalignedp16(header+2, htons(len));
         u_put_unalignedp32(header+4, *((uint32_t*)masking_key));
         break;
      }
      case 16:
      {
         header[1] = (127 | 0x80);

         u_put_unalignedp64(header+2, htonl(len));
         u_put_unalignedp32(header+10, *((uint32_t*)masking_key));
         break;
      }

      default: break; // never reached
   }

   switch (header_length)
   {
      // server
      case 2:
      case 4:
      case 12:
      {
         memcpy(header + header_length, data, len);
         break;
      }
      // client
      case 6:
      case 8:
      case 16:
      {  
         // we should SIMD this
         for (uint32_t i = 0; i < len; ++i)
         {
            header[header_length + i] = (data[i] ^ masking_key[i % 4]) & 0xff;
         }
         break;
      }

      default: break; // never reached
   }

   U_SRV_LOG_WITH_ADDR("send websocket data (%u+%u bytes) %.*S to", header_length, len, len, data)

   if (USocketExt::write(socket, (const char*)header, ncount, UServer_Base::timeoutMS) == ncount) U_RETURN(true);

   U_RETURN(false);
}

bool UWebSocket::sendControlFrame(const bool isServer, USocket* socket, int opcode, const unsigned char* payload, uint32_t payload_length)
{
   U_TRACE(0, "UWebSocket::sendControlFrame(%b,%p,%d,%.*S,%u)", isServer, socket, opcode, payload_length, payload, payload_length)

   uint32_t ncount = (isServer ? 2U : 6U) + payload_length;

   UString tmp(ncount);
   unsigned char* header = (unsigned char*)tmp.data();

   header[0] = (opcode | 0x80);

   if (isServer)
   {
      header[1] = payload_length;

      for (uint32_t i = 0; i < payload_length; ++i)
      {
         header[2+i] = payload[i];
      }
   }
   else
   {
      header[1] = (payload_length | 0x80);
      
      uint8_t masking_key[4];
      *((uint32_t*)masking_key) = u_get_num_random();
      
      u_put_unalignedp32(header+2, *((uint32_t*)masking_key));

      for (uint32_t i = 0; i < payload_length; ++i)
      {
         header[6+i] = (payload[i] ^ masking_key[i % 4]) & 0xff;
      }
   }

   if (USocketExt::write(socket, (const char*)header, ncount, UServer_Base::timeoutMS) == ncount)
      {
      U_SRV_LOG_WITH_ADDR("send control frame(%d) (6+%u bytes) %.*S to", opcode, payload_length, payload_length, payload)

      U_RETURN(true);
      }

   status_code = U_WS_STATUS_CODE_PROTOCOL_ERROR;

   U_RETURN(false);
}

void UWebSocket::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "UWebSocket::handlerRequest()")

   U_INTERNAL_ASSERT_POINTER(rbuffer)
   U_INTERNAL_ASSERT_MAJOR(U_http_websocket_len, 0)

   int fdmax = 0; // NB: to avoid 'warning: fdmax may be used uninitialized in this function'...
   fd_set fd_set_read, read_set;

   rbuffer->setEmpty();

   (void) checkForInitialData(); // check if we have read more data than necessary...

   if (command == U_NULLPTR)
      {
      on_message_param(U_DPAGE_OPEN);

      goto data;
      }

   // Set environment for the command application server

   penvironment->setBuffer(U_CAPACITY);

   (void) penvironment->append(command->getStringEnvironment());

   if (UHTTP::getCGIEnvironment(*penvironment, U_GENERIC) == false) return;

   command->setEnvironment(penvironment);

   if (command->execute((UString*)-1, (UString*)-1, -1, fd_stderr))
      {
      U_SRV_LOG_CMD_MSG_ERR(*command, true);

      UInterrupt::setHandlerForSignal(SIGTERM, (sighandler_t)handlerForSigTERM); // sync signal
      }

   FD_ZERO(&fd_set_read);
   FD_SET(UProcess::filedes[2], &fd_set_read);
   FD_SET(UServer_Base::csocket->iSockDesc, &fd_set_read);

   fdmax = U_max(UServer_Base::csocket->iSockDesc, UProcess::filedes[2]) + 1;

loop:
   read_set = fd_set_read;

   if (U_FF_SYSCALL(select, "%d,%p,%p,%p,%p", fdmax, &read_set, U_NULLPTR, U_NULLPTR, U_NULLPTR) > 0)
      {
      if (FD_ISSET(UProcess::filedes[2], &read_set))
         {
         rbuffer->setEmpty();

         if (UServices::read(UProcess::filedes[2], *rbuffer) &&
             sendData(true, UServer_Base::csocket, message_type, *rbuffer))
            {
            rbuffer->setEmpty();

            goto loop;
            }
         }
      else if (FD_ISSET(UServer_Base::csocket->iSockDesc, &read_set))
         {
data:    if (handleDataFraming(rbuffer, UServer_Base::csocket) == U_WS_STATUS_CODE_OK)
            {
            if (command == U_NULLPTR)
               {
               on_message();

               if (U_http_info.nResponseCode != HTTP_INTERNAL_ERROR)
                  {
                  rbuffer->setEmpty();

                  goto data;
                  }
               }
            else if (UNotifier::write(UProcess::filedes[1], U_STRING_TO_PARAM(*UClientImage_Base::wbuffer)))
               {
               rbuffer->setEmpty();

               goto loop;
               }
            }
         }
      }

   // Send server-side closing handshake

   if (UServer_Base::csocket->isOpen() &&
       (status_code == U_WS_STATUS_CODE_GOING_AWAY || sendClose(true, UServer_Base::csocket)))
      {
      UClientImage_Base::close();
      }
   else
      {
      UClientImage_Base::setRequestProcessed();
      }

   if (command == U_NULLPTR) on_message_param(U_DPAGE_CLOSE);
}

// DB

void UWebSocket::initDb()
{
   U_TRACE_NO_PARAM(0, "UWebSocket::initDb()")

   U_INTERNAL_ASSERT_EQUALS(db, U_NULLPTR)

   U_NEW(URDBObjectHandler<UDataStorage*>, db, URDBObjectHandler<UDataStorage*>(U_STRING_FROM_CONSTANT("../db/WebSocket"), -1, &rec, true));

   // POSIX shared memory object: interprocess - can be used by unrelated processes (userver_tcp and userver_ssl)

   if (db->open(128 * U_1M, false, true, true, U_SHM_LOCK_WEBSOCK)) // NB: we don't want truncate (we have only the journal)...
      {
      U_SRV_LOG("db WebSocket initialization success: size(%u)", db->size());

      URDB::initRecordLock();

      db->reset(); // Initialize the db to contain no entries
      }
   else
      {
      U_SRV_LOG("WARNING: db WebSocket initialization failed");

      U_DELETE(db)

      db = U_NULLPTR;
      }
}
