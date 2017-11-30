// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_proxy.cpp - this is a plugin proxy for userver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/command.h>
#include <ulib/mime/entity.h>
#include <ulib/utility/escape.h>
#include <ulib/utility/websocket.h>
#include <ulib/net/server/server.h>
#include <ulib/net/server/client_image.h>
#include <ulib/net/server/plugin/mod_proxy.h>
#include <ulib/net/server/plugin/mod_proxy_service.h>

/*
#ifdef HAVE_LIBNETFILTER_CONNTRACK_LIBNETFILTER_CONNTRACK_H
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#endif
*/

UHttpClient<UTCPSocket>* UProxyPlugIn::client_http;

U_CREAT_FUNC(server_plugin_proxy, UProxyPlugIn)

UProxyPlugIn::UProxyPlugIn()
{
   U_TRACE_REGISTER_OBJECT(0, UProxyPlugIn, "")
}

UProxyPlugIn::~UProxyPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, UProxyPlugIn)

   if (client_http) delete client_http;
}

// Server-wide hooks

int UProxyPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "UProxyPlugIn::handlerConfig(%p)", &cfg)

   if (UModProxyService::loadConfig(cfg)) U_RETURN(U_PLUGIN_HANDLER_PROCESSED);

   U_RETURN(U_PLUGIN_HANDLER_OK);
}

int UProxyPlugIn::handlerInit()
{
   U_TRACE_NO_PARAM(0, "UProxyPlugIn::handlerInit()")

   if (UHTTP::vservice == U_NULLPTR ||
       UHTTP::vservice->empty())
      {
      U_RETURN(U_PLUGIN_HANDLER_ERROR);
      }

/*
#ifdef LINUX_NETFILTER
#endif
*/

   U_NEW(UHttpClient<UTCPSocket>, client_http, UHttpClient<UTCPSocket>((UFileConfig*)U_NULLPTR));

   U_RETURN(U_PLUGIN_HANDLER_OK);
}

// Connection-wide hooks

int UProxyPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "UProxyPlugIn::handlerRequest()")

   if (UHTTP::isProxyRequest() == false) U_RETURN(U_PLUGIN_HANDLER_OK);

   // NB: process the HTTP PROXY request with fork....

   if (UServer_Base::startParallelization()) U_RETURN(U_PLUGIN_HANDLER_PROCESSED); // parent

   bool output_to_client = false,
        output_to_server = false;

   if (UHTTP::service->command) // check if it is required an action...
      {
      U_ASSERT(UClientImage_Base::environment->empty())

      if (UHTTP::getCGIEnvironment(*UClientImage_Base::environment, U_GENERIC))
         {
         if (UHTTP::service->environment) UClientImage_Base::environment->append(UHTTP::service->environment);

         if (UHTTP::processCGIRequest(UHTTP::service->command) &&
             UHTTP::processCGIOutput(false, false))
            {
            if (UHTTP::service->isResponseForClient()) output_to_client = true; // send output as response to client...
            else                                       output_to_server = true; // send output as request  to server...
            }

         UClientImage_Base::environment->setEmpty();
         }

      if (output_to_client == false &&
          output_to_server == false)
         {
         if (U_http_info.nResponseCode == 0 ||
             U_http_info.nResponseCode == HTTP_OK)
            {
            UHTTP::setInternalError();
            }

         U_RETURN(U_PLUGIN_HANDLER_PROCESSED);
         }
      }

   U_INTERNAL_DUMP("output_to_server = %b output_to_client = %b", output_to_server, output_to_client)

   if (output_to_server) // check if the kind of request is like HTTP protocol (client/server)...
      {
      U_INTERNAL_ASSERT(*UClientImage_Base::wbuffer)

      if (UHTTP::scanfHeaderResponse(U_STRING_TO_PARAM(*UClientImage_Base::wbuffer)) == false)
         {
         UModProxyService::setMsgError(UModProxyService::INTERNAL_ERROR);

         U_RETURN(U_PLUGIN_HANDLER_PROCESSED);
         }

      U_INTERNAL_DUMP("uri = %.*S", U_HTTP_URI_TO_TRACE)
      }

   if (output_to_client) UClientImage_Base::setRequestProcessed();
   else
      {
      // before connect to server check if server and/or port to connect has changed...

      if (client_http->setHostPort(UHTTP::service->getServer(), UHTTP::service->getPort()) &&
          client_http->UClient_Base::isConnected())
         {
         client_http->UClient_Base::close();
         }

      // --------------------------------------------------------------------------------------------------------------------
      // A WebSocket is a long-lived connection, lasting hours or days. If each WebSocket proxy holds the original thread,
      // won't that consume all of the workers very quickly? It looks as if my server, with 16 workers, will be unable to
      // handle either the 17th WebSocket proxy or any other HTTP request. That's not really practical in a production system.
      // A WebSocket server could potentially handle hundreds or even thousands of simultaneous connections, which would mean
      // the same number of proxies in server...
      // --------------------------------------------------------------------------------------------------------------------

      if (UHTTP::service->isWebSocket())
         {
         UWebSocket::checkForInitialData(); // check if we have read more data than necessary...

         while (UWebSocket::handleDataFraming(UServer_Base::csocket) == STATUS_CODE_OK                                                            &&
                (client_http->UClient_Base::prepareRequest(*UClientImage_Base::wbuffer), client_http->UClient_Base::sendRequestAndReadResponse()) &&
                UWebSocket::sendData(UWebSocket::message_type, (const unsigned char*)U_STRING_TO_PARAM(client_http->UClient_Base::response)))
            {
            client_http->UClient_Base::clearData();

            UClientImage_Base::wbuffer->setEmpty();
            }

         U_RETURN(U_PLUGIN_HANDLER_ERROR);
         }

                                             client_http->setFollowRedirects(UHTTP::service->isFollowRedirects(), true);
      if (UHTTP::service->isAuthorization()) client_http->setRequestPasswordAuthentication(UHTTP::service->getUser(), UHTTP::service->getPassword());

      // connect to server, send request and get response

      if (output_to_server == false) *UClientImage_Base::wbuffer = *UClientImage_Base::request;

      bool result = client_http->sendRequest(*UClientImage_Base::wbuffer);

      *UClientImage_Base::wbuffer = client_http->getResponse();

      if (result == false)
         {
         UClientImage_Base::setRequestProcessed();

#     ifndef U_LOG_DISABLE
         if (UServer_Base::isLog()) UServer_Base::log->logResponse(*UClientImage_Base::wbuffer, U_CONSTANT_TO_PARAM(""), 0);
#     endif
         }
      else
         {
         UClientImage_Base::setNoHeaderForResponse();

         U_INTERNAL_DUMP("U_http_data_chunked = %b U_ClientImage_close = %b client_http->data_chunked = %b", U_http_data_chunked, U_ClientImage_close, client_http->data_chunked)

         if (client_http->data_chunked == false)
            {
            if (UHTTP::service->isReplaceResponse()) *UClientImage_Base::wbuffer = UHTTP::service->replaceResponse(*UClientImage_Base::wbuffer); 

            UClientImage_Base::setRequestProcessed();
            }
         else
            {
            // NB: in this case we broke the transparency of the response to avoid a duplication of effort to read chunked data...

            UString body         = client_http->getContent(),
                    content_type = client_http->getResponseHeader()->getContentType();

            if (body &&
                content_type)
               {
               if (UHTTP::service->isReplaceResponse()) body = UHTTP::service->replaceResponse(body); 

               content_type.rep->_length += 2; // NB: we add "\r\n"...

               UHTTP::setDynamicResponse(body, UString::getStringNull(), content_type);
               }
            }
         }

      client_http->reset(); // reset reference to request...
      }

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UProxyPlugIn::dump(bool reset) const
{
   *UObjectIO::os << "client_http (UHttpClient<UTCPSocket> " << (void*)client_http << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
