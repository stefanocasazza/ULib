// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    uwebsocket.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/client/websocket.h>

bool UWebSocketClient::connectServer(const UString& url)
{
   U_TRACE(0, "UWebSocketClient::connectServer(%V)", url.rep)

   U_INTERNAL_ASSERT_POINTER(client)

   if (client->UClient_Base::connectServer(url))
      {
      UMimeHeader responseHeader;
      UString buffer(U_CAPACITY);

      buffer.snprintf(U_CONSTANT_TO_PARAM(
                        "GET %v HTTP/1.1\r\n"
                        "Host: %v:%u\r\n"
                        "Upgrade: websocket\r\n"
                        "Connection: Upgrade\r\n"
                        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                        "Sec-WebSocket-Version: 13\r\n"
                        "\r\n"),
                        client->UClient_Base::uri.rep, client->UClient_Base::server.rep, client->UClient_Base::port);

      client->UClient_Base::prepareRequest(buffer);

      if (client->UClient_Base::sendRequestAndReadResponse() &&
          (responseHeader.setIgnoreCase(true), client->UClient_Base::processHeader(&responseHeader)))
         {
         /**
          * HTTP/1.1 101 Switching Protocols\r\n
          * Upgrade: websocket\r\n
          * Connection: Upgrade\r\n
          * Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n
          */

         if (U_http_info.nResponseCode != 101) U_RETURN(false); // invalid HTTP status response code

         if (responseHeader.getHeader(U_CONSTANT_TO_PARAM("Upgrade")).equalnocase(U_CONSTANT_TO_PARAM("websocket"))  == false) U_RETURN(false); // invalid HTTP upgrade header
         if (responseHeader.getHeader(U_CONSTANT_TO_PARAM("Connection")).equalnocase(U_CONSTANT_TO_PARAM("Upgrade")) == false) U_RETURN(false); // invalid HTTP connection header

         if (responseHeader.getHeader(U_CONSTANT_TO_PARAM("Sec-WebSocket-Accept")).equal(U_CONSTANT_TO_PARAM("s3pPLMBiTxaQ9kYGzzhZRbK+xOo="))) U_RETURN(true);
         }
      }

   U_RETURN(false);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UWebSocketClient::dump(bool _reset) const
{
   *UObjectIO::os << "client (UClient<USSLSocket> " << (void*)client << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
