// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    websocket.h - simple websocket client
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_WEBSOCKET_CLIENT_H
#define ULIB_WEBSOCKET_CLIENT_H 1

#include <ulib/net/client/client.h>
#include <ulib/ssl/net/sslsocket.h>
#include <ulib/utility/websocket.h>

/**
 * @class UWebSocketClient
 */

class U_EXPORT UWebSocketClient {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   /**
    * Constructor
    */

   UWebSocketClient()
      {
      U_TRACE_CTOR(0, UWebSocketClient, "")

      U_NEW(UClient<USSLSocket>, client, UClient<USSLSocket>(U_NULLPTR));
      }

   ~UWebSocketClient()
      {
      U_TRACE_DTOR(0, UWebSocketClient)

      U_INTERNAL_ASSERT_POINTER(client)

      U_DELETE(client)
      }

   // SERVICES

   bool readMessage()
      {
      U_TRACE_NO_PARAM(0, "UWebSocketClient::readMessage()")

      U_INTERNAL_ASSERT_POINTER(client)

      UWebSocket::rbuffer->setEmpty();

      if (UWebSocket::handleDataFraming(UWebSocket::rbuffer, client->UClient_Base::socket) == U_WS_STATUS_CODE_OK) U_RETURN(true);

      U_RETURN(false);
      }

   bool sendMessage(const UString& msg, int type = U_WS_MESSAGE_TYPE_TEXT) // U_WS_MESSAGE_TYPE_BROTLI
      {
      U_TRACE(0, "UWebSocketClient::sendMessage(%V,%d)", msg.rep, type)

      U_INTERNAL_ASSERT_POINTER(client)

      return UWebSocket::sendData(false, client->UClient_Base::socket, type, msg);
      }

   void close()
      {
      U_TRACE_NO_PARAM(0, "UWebSocketClient::close()")

      U_INTERNAL_ASSERT_POINTER(client)

      (void) UWebSocket::sendClose(false, client->UClient_Base::socket);

      client->UClient_Base::close();
      }

   bool connectServer(const UString& url);

   UClient<USSLSocket>* getClient() const { return client; }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UClient<USSLSocket>* client;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UWebSocketClient)
};
#endif
