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

#define MESSAGE_TYPE_INVALID  -1
#define MESSAGE_TYPE_TEXT      0
#define MESSAGE_TYPE_BINARY  128
#define MESSAGE_TYPE_CLOSE   255
#define MESSAGE_TYPE_PING    256
#define MESSAGE_TYPE_PONG    257

#define STATUS_CODE_OK                1000
#define STATUS_CODE_GOING_AWAY        1001
#define STATUS_CODE_PROTOCOL_ERROR    1002
#define STATUS_CODE_INVALID_TYPE      1003
#define STATUS_CODE_RESERVED1         1004 /* Protocol 8: frame too large */
#define STATUS_CODE_RESERVED2         1005
#define STATUS_CODE_RESERVED3         1006
#define STATUS_CODE_INVALID_UTF8      1007
#define STATUS_CODE_POLICY_VIOLATION  1008
#define STATUS_CODE_MESSAGE_TOO_LARGE 1009
#define STATUS_CODE_EXTENSION_ERROR   1010
#define STATUS_CODE_INTERNAL_ERROR    1011
#define STATUS_CODE_RESERVED4         1015

class USocket;

class U_EXPORT UWebSocket {
public:

   // SERVICES

   static UString* rbuffer;
   static uint32_t max_message_size;
   static const char* upgrade_settings;
   static int message_type, status_code;

   typedef struct _WebSocketFrameData {
      unsigned char* application_data;
      uint32_t       application_data_offset;
      unsigned char  fin;
      unsigned char  opcode;
      unsigned int   utf8_state;
   } WebSocketFrameData;

   static WebSocketFrameData control_frame;
   static WebSocketFrameData message_frame;

   static bool sendAccept();
   static void checkForInitialData();
   static int  handleDataFraming(USocket* socket);
   static bool sendData(int type, const unsigned char* buffer, uint32_t buffer_size);

   static bool sendClose()
      {
      U_TRACE_NO_PARAM(0, "UWebSocket::sendClose()")

      // Send server-side closing handshake

      U_INTERNAL_DUMP("status_code = %d", status_code)

      unsigned char status_code_buffer[2] = { (unsigned char)((status_code >> 8) & 0xFF),
                                              (unsigned char)( status_code       & 0xFF) };

      bool result = sendData(MESSAGE_TYPE_CLOSE, status_code_buffer, sizeof(status_code_buffer));

      U_RETURN(result);
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UWebSocket)
};

#endif
