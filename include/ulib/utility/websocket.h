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

#include <ulib/db/rdb.h>
#include <ulib/net/server/server.h>

#define U_WS_MESSAGE_TYPE_INVALID  -1
#define U_WS_MESSAGE_TYPE_TEXT      0
#define U_WS_MESSAGE_TYPE_BROTLI    6
#define U_WS_MESSAGE_TYPE_BINARY  128
#define U_WS_MESSAGE_TYPE_CLOSE   255
#define U_WS_MESSAGE_TYPE_PING    256
#define U_WS_MESSAGE_TYPE_PONG    257

#define U_WS_OPCODE_CONTINUATION 0x0
#define U_WS_OPCODE_TEXT         0x1
#define U_WS_OPCODE_BINARY       0x2
#define U_WS_OPCODE_BROTLI       0x6
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

class UHTTP;
class USocket;
class UCommand;
class UDataStorage;
class UProxyPlugIn;
class UWebSocketClient;
class UWebSocketPlugIn;

template <class T> class URDBObjectHandler;

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

   static void checkForInitialData();
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

   // DB

   typedef struct uwrec {
      uint32_t csocket;
      uint32_t user_id;
   // .................
   } uwrec;

   static bool insertOnDb(in_addr_t client, uint32_t csocket, uint32_t user_id)
      {
      U_TRACE(0, "UWebSocket::insertOnDb(%u,%u,%u)", client, csocket, user_id)

      U_INTERNAL_ASSERT_POINTER(db)

      uwrec new_rec = { csocket, user_id };

      return db->insertDataStorage(&new_rec, sizeof(uwrec), client, RDB_INSERT);
      }

   static bool insertOnDb(uint32_t user_id) { return insertOnDb(UServer_Base::getClientAddress(), UServer_Base::csocket->getFd(), user_id); }

   static bool removeFromDb(in_addr_t client)
      {
      U_TRACE(0, "UWebSocket::removeFromDb(%u)", client)

      U_INTERNAL_ASSERT_POINTER(db)

      return db->remove((const char*)&client, sizeof(in_addr_t));
      }

   static bool setCloseOnDb(in_addr_t client)
      {
      U_TRACE(0, "UWebSocket::setCloseOnDb(%u)", client)

      U_INTERNAL_ASSERT_POINTER(db)

      if (db->getDataStorage(client))
         {
         db->lockRecord();

         rec->csocket = U_NOT_FOUND;

         db->unlockRecord();

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static void getWebSocketRecFromBuffer(const char* data, uint32_t datalen)
      {
      U_TRACE(0, "UWebSocket::getWebSocketRecFromBuffer(%.*S,%u)", datalen, data, datalen)

      rec = (uwrec*)u_buffer;

      u_buffer_len = sizeof(uwrec);

      const char* ptr = data;

      rec->csocket = u_strtoulp(&ptr);
      rec->user_id = u_strtoulp(&ptr);

      U_INTERNAL_DUMP("rec->csocket = %u rec->user_id = %u", rec->csocket, rec->user_id)

      U_INTERNAL_ASSERT_EQUALS(ptr, data+datalen+1)
      }

   static void printWebSocketRecToBuffer(const char* data, uint32_t datalen)
      {
      U_TRACE(0, "UWebSocket::printWebSocketRecToBuffer(%.*S,%u)", datalen, data, datalen)

      U_INTERNAL_ASSERT_EQUALS(datalen, sizeof(uwrec))

      rec = (uwrec*)data;

      u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE, U_CONSTANT_TO_PARAM("%u %u"), rec->csocket, rec->user_id);
      }

   static UString* message;
   static int message_type;

private:
   static uwrec* rec;
   static int fd_stderr;
   static vPF on_message;
   static UCommand* command;
   static UString* penvironment;
   static vPFu on_message_param;
   static uint32_t max_message_size;
   static URDBObjectHandler<UDataStorage*>* db;

   static int status_code;
   static UString* rbuffer;
   static const char* upgrade_settings;
   static WebSocketFrameData control_frame;
   static WebSocketFrameData message_frame;

   static void initDb();
   static void handlerRequest();
   static bool sendAccept(USocket* socket);
   static int handleDataFraming(USocket* socket);
   static RETSIGTYPE handlerForSigTERM(int signo);
   static bool sendControlFrame(USocket* socket, int opcode, const unsigned char* payload, uint32_t payload_length);

   U_DISALLOW_COPY_AND_ASSIGN(UWebSocket)

   friend class UHTTP;
   friend class UProxyPlugIn;
   friend class UWebSocketClient;
   friend class UWebSocketPlugIn;
   friend class UClientImage_Base;
};
#endif
