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

#include <ulib/utility/uhttp.h>
#include <ulib/utility/services.h>
#include <ulib/utility/websocket.h>
#include <ulib/net/server/server.h>

#define OPCODE_CONTINUATION 0x0
#define OPCODE_TEXT         0x1
#define OPCODE_BINARY       0x2
#define OPCODE_CLOSE        0x8
#define OPCODE_PING         0x9
#define OPCODE_PONG         0xA

#define DATA_FRAMING_MASK               0
#define DATA_FRAMING_START              1
#define DATA_FRAMING_PAYLOAD_LENGTH     2
#define DATA_FRAMING_PAYLOAD_LENGTH_EXT 3
#define DATA_FRAMING_EXTENSION_DATA     4
#define DATA_FRAMING_APPLICATION_DATA   5
#define DATA_FRAMING_CLOSE              6

#define FRAME_GET_FIN(BYTE)         (((BYTE) >> 7) & 0x01)
#define FRAME_GET_RSV1(BYTE)        (((BYTE) >> 6) & 0x01)
#define FRAME_GET_RSV2(BYTE)        (((BYTE) >> 5) & 0x01)
#define FRAME_GET_RSV3(BYTE)        (((BYTE) >> 4) & 0x01)
#define FRAME_GET_OPCODE(BYTE)      ( (BYTE)       & 0x0F)
#define FRAME_GET_MASK(BYTE)        (((BYTE) >> 7) & 0x01)
#define FRAME_GET_PAYLOAD_LEN(BYTE) ( (BYTE)       & 0x7F)

#define FRAME_SET_FIN(BYTE)         (((BYTE) & 0x01) << 7)
#define FRAME_SET_OPCODE(BYTE)       ((BYTE) & 0x0F)
#define FRAME_SET_MASK(BYTE)        (((BYTE) & 0x01) << 7)
#define FRAME_SET_LENGTH(X64, IDX)  (unsigned char)(((uint64_t)(X64) >> ((IDX)*8)) & 0xFF)

#define WEBSOCKET_GUID     "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define WEBSOCKET_GUID_LEN 36

int         UWebSocket::status_code;
int         UWebSocket::message_type;
UString*    UWebSocket::rbuffer;
uint32_t    UWebSocket::max_message_size;
const char* UWebSocket::upgrade_settings;

UWebSocket::WebSocketFrameData UWebSocket::control_frame = { 0, 0, 1, 8, 0 };
UWebSocket::WebSocketFrameData UWebSocket::message_frame = { 0, 0, 1, 0, 0 };

bool UWebSocket::sendAccept()
{
   U_TRACE_NO_PARAM(0, "UWebSocket::sendAccept()")

   U_INTERNAL_ASSERT_MAJOR(U_http_websocket_len, 0)

   // In order to establish a websocket connection, a client (a web browser) sends a HTTP GET request with a number of HTTP headers. Among those
   // headers there is the Sec-WebSocket-Key header, which contains a handshake key. According to the WebSocket protocol, the server should:
   //
   // 1) Concatenate the handshake key with the magic guid {258EAFA5-E914-47DA-95CA-C5AB0DC85B11}
   // 2) Take the SHA1 hash of the concatenation result
   // 3) Send the base64 equivalent of the hash in HTTP response to the client

   unsigned char challenge[128];

   U_MEMCPY(challenge,                  upgrade_settings, U_http_websocket_len);
   U_MEMCPY(challenge+U_http_websocket_len, WEBSOCKET_GUID, WEBSOCKET_GUID_LEN);

   // SHA1(challenge)

   UString accept(U_CAPACITY);

   UServices::generateDigest(U_HASH_SHA1, 0, challenge, U_http_websocket_len + WEBSOCKET_GUID_LEN, accept, true);

   UClientImage_Base::wbuffer->snprintf(U_CONSTANT_TO_PARAM("HTTP/1.1 101 Switching Protocols\r\n"
                                        "Upgrade: websocket\r\n"
                                        "Connection: Upgrade\r\n"
                                        "Sec-WebSocket-Accept: %v\r\n\r\n"), accept.rep);

   if (USocketExt::write(UServer_Base::csocket, *UClientImage_Base::wbuffer, UServer_Base::timeoutMS))
      {
      status_code  = STATUS_CODE_INTERNAL_ERROR;
      message_type = MESSAGE_TYPE_INVALID;

      if (max_message_size == 0) max_message_size = U_STRING_MAX_SIZE;

      U_RETURN(true);
      }

   U_RETURN(false);
}

void UWebSocket::checkForInitialData()
{
   U_TRACE_NO_PARAM(0, "UWebSocket::checkForInitialData()")

   U_INTERNAL_ASSERT_POINTER(rbuffer)
   U_INTERNAL_ASSERT_MAJOR(UClientImage_Base::size_request, 0)

   uint32_t sz = UClientImage_Base::rbuffer->size();

   U_INTERNAL_DUMP("UClientImage_Base::rbuffer(%u) = %V", sz, UClientImage_Base::rbuffer->rep)

   if (UClientImage_Base::size_request < sz)
      {
      // we have read more data than necessary...

      const char* ptr = UClientImage_Base::rbuffer->c_pointer(UClientImage_Base::size_request);

      (void) rbuffer->append(ptr, sz - UClientImage_Base::size_request);

      U_INTERNAL_DUMP("rbuffer(%u) = %V", rbuffer->size(), rbuffer->rep)
      }
}

/**
 * So, WebSockets presents a sequence of infinitely long byte streams
 * with a termination indicator (the FIN bit in the frame header) and
 * not a message based interface as you might initially believe. Given
 * that a general purpose protocol handler can only work in terms of
 * partial frames, we effectively have a stream based protocol with
 * lots of added complexity to provide the illusion of a message based
 * protocol that can actually only ever be dealt with as a stream of bytes
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

int UWebSocket::handleDataFraming(USocket* socket)
{
   U_TRACE(0, "UWebSocket::handleDataFraming(%p)", socket)

   unsigned char* block;
   uint32_t block_offset, ncount = 0, block_size;
   WebSocketFrameData* frame = &UWebSocket::control_frame;
   unsigned char fin = 0, opcode = 0xFF, mask[4] = { 0, 0, 0, 0 };
   int32_t extension_bytes_remaining = 0, payload_length = 0, mask_offset = 0;
   int framing_state = DATA_FRAMING_START, payload_length_bytes_remaining = 0, mask_index = 0, masking = 0;

loop:
   if (rbuffer->empty() &&
       USocketExt::read(socket, *rbuffer, U_SINGLE_READ, UServer_Base::timeoutMS) == false)
      {
      U_RETURN(STATUS_CODE_INTERNAL_ERROR);
      }

              block        = (unsigned char*) rbuffer->data();
   ncount += (block_size   =                  rbuffer->size());
              block_offset = 0;

   do {
      U_INTERNAL_DUMP("framing_state = %d", framing_state)

      switch (framing_state)
         {
         case DATA_FRAMING_START: // 1
            {
            // Since we don't currently support any extensions, the reserve bits must be 0

            if ((FRAME_GET_RSV1(block[block_offset]) != 0) ||
                (FRAME_GET_RSV2(block[block_offset]) != 0) ||
                (FRAME_GET_RSV3(block[block_offset]) != 0))
               {
               // framing_state = DATA_FRAMING_CLOSE; // 6

               U_RETURN(status_code = STATUS_CODE_PROTOCOL_ERROR);
               }

            fin    = FRAME_GET_FIN(   block[block_offset]);
            opcode = FRAME_GET_OPCODE(block[block_offset++]);

            U_INTERNAL_DUMP("fin = %d opcode = %X", fin, opcode)

            framing_state = DATA_FRAMING_PAYLOAD_LENGTH; // 2

            if (opcode >= 0x8) // Control frame
               {
               if (fin == 0)
                  {
                  // framing_state = DATA_FRAMING_CLOSE; // 6

                  U_RETURN(status_code = STATUS_CODE_PROTOCOL_ERROR);
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
                     // framing_state = DATA_FRAMING_CLOSE; // 6

                     U_RETURN(status_code = STATUS_CODE_PROTOCOL_ERROR);
                     }

                  frame->opcode     = opcode;
                  frame->utf8_state = 0;
                  }
               else if (frame->fin ||
                        ((opcode = frame->opcode) == 0))
                  {
                  // framing_state = DATA_FRAMING_CLOSE; // 6

                  U_RETURN(status_code = STATUS_CODE_PROTOCOL_ERROR);
                  }

               frame->fin = fin;
               }

            payload_length                 = 0;
            payload_length_bytes_remaining = 0;

            if (block_offset >= block_size) break; // Only break if we need more data
            }

         U_INTERNAL_DUMP("framing_state = %d", framing_state)

         case DATA_FRAMING_PAYLOAD_LENGTH: // 2
            {
            payload_length = FRAME_GET_PAYLOAD_LEN(block[block_offset]);
            masking        = FRAME_GET_MASK(       block[block_offset++]);

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
               // framing_state = DATA_FRAMING_CLOSE; // 6

               U_RETURN(status_code = STATUS_CODE_PROTOCOL_ERROR);
               }

            framing_state = DATA_FRAMING_PAYLOAD_LENGTH_EXT; // 3

            if (block_offset >= block_size) break; // Only break if we need more data
            }

         U_INTERNAL_DUMP("framing_state = %d", framing_state)

         case DATA_FRAMING_PAYLOAD_LENGTH_EXT: // 3
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

                  // framing_state = DATA_FRAMING_CLOSE; // 6

                  U_RETURN(status_code = STATUS_CODE_MESSAGE_TOO_LARGE); // Invalid payload length
                  }

               if (masking == 0)
                  {
                  framing_state = DATA_FRAMING_EXTENSION_DATA; // 4

                  break;
                  }

               framing_state = DATA_FRAMING_MASK; // 0
               }

            if (block_offset >= block_size) break; // Only break if we need more data
            }

         U_INTERNAL_DUMP("framing_state = %d", framing_state)

         case DATA_FRAMING_MASK: // 0
            {
            U_INTERNAL_DUMP("mask_index = %d", mask_index)

            while ((mask_index < 4) && (block_offset < block_size)) mask[mask_index++] = block[block_offset++];

            U_INTERNAL_DUMP("mask_index = %d", mask_index)

            if (mask_index != 4) break;

            mask_index    = 0;
            mask_offset   = 0;
            framing_state = DATA_FRAMING_EXTENSION_DATA; // 4

            if ((mask[0] == 0) &&
                (mask[1] == 0) &&
                (mask[2] == 0) &&
                (mask[3] == 0))
               {
               masking = 0;
               }

            U_INTERNAL_DUMP("masking = %d", masking)
            }

         // Fall through

         U_INTERNAL_DUMP("framing_state = %d", framing_state)

         case DATA_FRAMING_EXTENSION_DATA: // 4
            {
            // Deal with extension data when we support them -- FIXME

            U_INTERNAL_DUMP("extension_bytes_remaining = %d", extension_bytes_remaining)

            if (extension_bytes_remaining == 0)
               {
               if (payload_length > 0)
                  {
                  (void) UClientImage_Base::wbuffer->reserve(frame->application_data_offset + payload_length);

                  frame->application_data = (unsigned char*) UClientImage_Base::wbuffer->data();
                  }

               framing_state = DATA_FRAMING_APPLICATION_DATA; // 5
               }
            }

         // Fall through

         U_INTERNAL_DUMP("framing_state = %d", framing_state)

         case DATA_FRAMING_APPLICATION_DATA: // 5
            {
            int32_t block_length      = block_size - block_offset,
                    block_data_length = (payload_length > block_length ? block_length
                                                                       : payload_length);

            uint32_t       application_data_offset = frame->application_data_offset;
            unsigned char* application_data        = frame->application_data;

            if (masking)
               {
               int32_t i;

               if (opcode == OPCODE_TEXT)
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
               
               if (opcode == OPCODE_TEXT)
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
               message_type = MESSAGE_TYPE_INVALID;

               switch (opcode)
                  {
                  case OPCODE_TEXT:
                     {
                     if ((fin &&
                         (frame->utf8_state != 0)) ||
                         (frame->utf8_state == 1))
                        {
                        // framing_state = DATA_FRAMING_CLOSE; // 6

                        U_RETURN(status_code = STATUS_CODE_INVALID_UTF8);
                        }

                     message_type = MESSAGE_TYPE_TEXT;
                     }
                  break;

                  case OPCODE_BINARY: message_type = MESSAGE_TYPE_BINARY; break;

                  case OPCODE_CLOSE:
                     {
                     // framing_state = DATA_FRAMING_CLOSE; // 6

                     U_RETURN(status_code = STATUS_CODE_OK);
                     }

                  case OPCODE_PING:
                     {
                     (void) sendData(MESSAGE_TYPE_PONG, application_data, application_data_offset);
                     }
                  break;

                  case OPCODE_PONG: break;

                  default:
                     {
                     // framing_state = DATA_FRAMING_CLOSE; // 6

                     U_RETURN(status_code = STATUS_CODE_PROTOCOL_ERROR);
                     }
                  }

               if (fin)
                  {
                  U_INTERNAL_DUMP("framing_state = %d message_type = %d status_code = %d", framing_state, message_type, status_code)

                  if (message_type != MESSAGE_TYPE_INVALID)
                     {
                     U_INTERNAL_ASSERT_EQUALS(framing_state, DATA_FRAMING_APPLICATION_DATA)

                     UClientImage_Base::wbuffer->size_adjust_force(application_data_offset);

                     U_SRV_LOG_WITH_ADDR("received websocket data (%u+%u bytes) %V from",
                                             ncount - UClientImage_Base::wbuffer->size(),
                                                      UClientImage_Base::wbuffer->size(), UClientImage_Base::wbuffer->rep)

                     U_RETURN(status_code = STATUS_CODE_OK);
                     }

                  frame->application_data        = 0;
                         application_data_offset = 0;
                  }

               framing_state = DATA_FRAMING_START; // 1
               }

            frame->application_data_offset = application_data_offset;
            }
         break;

      // case DATA_FRAMING_CLOSE: block_offset = block_size; break;

         default:
            {
            // framing_state = DATA_FRAMING_CLOSE; // 6

            U_RETURN(status_code = STATUS_CODE_PROTOCOL_ERROR);
            }
         }
      }
   while (block_offset < block_size);

   U_INTERNAL_ASSERT(block_offset >= block_size)
   U_INTERNAL_ASSERT_DIFFERS(framing_state, DATA_FRAMING_CLOSE) // 6

   rbuffer->setEmpty();

   goto loop;
}

bool UWebSocket::sendData(int type, const unsigned char* buffer, uint32_t buffer_size)
{
   U_TRACE(0, "UWebSocket::sendData(%d,%p,%u)", type, buffer, buffer_size)

   uint32_t pos = 0;
   unsigned char opcode, header[32];
   uint32_t payload_length = (buffer ? buffer_size : 0);

   switch (type)
      {
      case MESSAGE_TYPE_TEXT:
      case MESSAGE_TYPE_INVALID:
         opcode = OPCODE_TEXT;
      break;

      case MESSAGE_TYPE_PING:   opcode = OPCODE_PING;   break;
      case MESSAGE_TYPE_PONG:   opcode = OPCODE_PONG;   break;
      case MESSAGE_TYPE_BINARY: opcode = OPCODE_BINARY; break;

      case MESSAGE_TYPE_CLOSE:
      default:
         opcode = OPCODE_CLOSE;
      break;
      }

   header[pos++] = FRAME_SET_FIN(1) | FRAME_SET_OPCODE(opcode);

   if (payload_length < 126) header[pos++] = FRAME_SET_MASK(0) | FRAME_SET_LENGTH(payload_length, 0);
   else
      {
      if (payload_length < 65536) header[pos++] = FRAME_SET_MASK(0) | 126;
      else
         {
         header[pos++] = FRAME_SET_MASK(0) | 127;
         header[pos++] = 0; // FRAME_SET_LENGTH(payload_length, 7);
         header[pos++] = 0; // FRAME_SET_LENGTH(payload_length, 6);
         header[pos++] = 0; // FRAME_SET_LENGTH(payload_length, 5);
         header[pos++] = 0; // FRAME_SET_LENGTH(payload_length, 4);
         header[pos++] = FRAME_SET_LENGTH(payload_length, 3);
         header[pos++] = FRAME_SET_LENGTH(payload_length, 2);
         }

      header[pos++] = FRAME_SET_LENGTH(payload_length, 1);
      header[pos++] = FRAME_SET_LENGTH(payload_length, 0);
      }

   U_SRV_LOG_WITH_ADDR("send websocket data (%u+%u bytes) %.*S to", pos, buffer_size, buffer_size, buffer)

   struct iovec iov[2] = { { (caddr_t)header, pos },
                           { (caddr_t)buffer, payload_length } };

   int iBytesWrite = (payload_length
            ? (pos += payload_length, USocketExt::writev(UServer_Base::csocket, iov, 2,              pos, UServer_Base::timeoutMS))
            :                         USocketExt::write( UServer_Base::csocket, (const char*)header, pos, UServer_Base::timeoutMS));

   if (iBytesWrite == (int)pos) U_RETURN(true);

   U_RETURN(false);
}
