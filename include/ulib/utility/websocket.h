// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    websocket.h - WebSocket utility
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_WEBSOCKET_H
#define ULIB_WEBSOCKET_H 1

#include <ulib/string.h>

#define U_WS_MESSAGE_TYPE_INVALID  -1
#define U_WS_MESSAGE_TYPE_TEXT      0
#define U_WS_MESSAGE_TYPE_BINARY  128
#define U_WS_MESSAGE_TYPE_CLOSE   255
#define U_WS_MESSAGE_TYPE_PING    256
#define U_WS_MESSAGE_TYPE_PONG    257

#define U_WS_OPCODE_CONTINUATION 0x0
#define U_WS_OPCODE_TEXT         0x1
#define U_WS_OPCODE_BINARY       0x2
#define U_WS_OPCODE_CLOSE        0x8
#define U_WS_OPCODE_PING         0x9
#define U_WS_OPCODE_PONG         0xA

#define U_WS_STATUS_CODE_OK                1000
#define U_WS_STATUS_CODE_GOING_AWAY        1001
#define U_WS_STATUS_CODE_PROTOCOL_ERROR    1002
#define U_WS_STATUS_CODE_INVALID_TYPE      1003
#define U_WS_STATUS_CODE_RESERVED1         1004 /* Protocol 8: frame too large */
#define U_WS_STATUS_CODE_RESERVED2         1005
#define U_WS_STATUS_CODE_RESERVED3         1006
#define U_WS_STATUS_CODE_INVALID_UTF8      1007
#define U_WS_STATUS_CODE_POLICY_VIOLATION  1008
#define U_WS_STATUS_CODE_MESSAGE_TOO_LARGE 1009
#define U_WS_STATUS_CODE_EXTENSION_ERROR   1010
#define U_WS_STATUS_CODE_INTERNAL_ERROR    1011
#define U_WS_STATUS_CODE_RESERVED4         1015

class USocket;

class U_EXPORT UWebSocket {
public:

   // SERVICES

   typedef struct _WebSocketFrameData {
      unsigned char* application_data;
      uint32_t       application_data_offset;
      unsigned char  fin;
      unsigned char  opcode;
      unsigned int   utf8_state;
   } WebSocketFrameData;

   static UString* rbuffer;
   static UString* message;
   static uint32_t max_message_size;
   static const char* upgrade_settings;
   static int message_type, status_code;
   static WebSocketFrameData control_frame;
   static WebSocketFrameData message_frame;

   static void checkForInitialData();
   static bool sendAccept(USocket* socket);
   static int  handleDataFraming(USocket* socket);
   static bool sendData(USocket* socket, int type, const char* data, uint32_t len);

   static bool sendData(USocket* socket, int type, const UString& data) { return sendData(socket, type, U_STRING_TO_PARAM(data)); }

   static bool sendClose(USocket* socket)
      {
      U_TRACE(0, "UWebSocket::sendClose(%p)", socket)

      // Send server-side closing handshake

      U_INTERNAL_DUMP("status_code = %d", status_code)

      unsigned char status_code_buffer[2] = { (unsigned char)((status_code >> 8) & 0xFF),
                                              (unsigned char)( status_code       & 0xFF) };

      if (sendControlFrame(socket, U_WS_OPCODE_CLOSE, status_code_buffer, sizeof(status_code_buffer))) U_RETURN(true);

      U_RETURN(false);
      }

private:
   static bool sendControlFrame(USocket* socket, int opcode, const unsigned char* payload, uint32_t payload_length);

   U_DISALLOW_COPY_AND_ASSIGN(UWebSocket)
};

#endif
