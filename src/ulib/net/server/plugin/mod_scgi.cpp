// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_scgi.cpp - Perform simple scgi request forwarding
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file_config.h>
#include <ulib/utility/uhttp.h>
#include <ulib/net/tcpsocket.h>
#include <ulib/utility/services.h>
#include <ulib/net/client/client.h>
#include <ulib/net/server/server.h>
#include <ulib/net/server/plugin/mod_scgi.h>

#ifndef _MSWINDOWS_
#  include <ulib/net/unixsocket.h>
#endif

U_CREAT_FUNC(server_plugin_scgi, USCGIPlugIn)

bool          USCGIPlugIn::scgi_keep_conn;
UClient_Base* USCGIPlugIn::connection;

USCGIPlugIn::USCGIPlugIn()
{
   U_TRACE_REGISTER_OBJECT(0, USCGIPlugIn, "")
}

USCGIPlugIn::~USCGIPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, USCGIPlugIn)

   if (connection) delete connection;
}

// Server-wide hooks

int USCGIPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "USCGIPlugIn::handlerConfig(%p)", &cfg)

   // ------------------------------------------------------------------------------------------
   // SCGI_URI_MASK  mask (DOS regexp) of uri type that send request to SCGI (*.php)
   //
   // NAME_SOCKET    file name for the scgi socket
   //
   // SERVER         host name or ip address for the scgi host
   // PORT           port number             for the scgi host
   //
   // RES_TIMEOUT    timeout for response from server SCGI
   // SCGI_KEEP_CONN If not zero, the server SCGI does not close the connection after
   //                responding to request; the plugin retains responsibility for the connection.
   //
   // LOG_FILE       location for file log (use server log if exist)
   // ------------------------------------------------------------------------------------------

   if (cfg.loadTable())
      {
      UClient_Base::cfg = &cfg;

      U_NEW(UClient_Base, connection, UClient_Base(&cfg));

      UString x = cfg.at(U_CONSTANT_TO_PARAM("SCGI_URI_MASK"));

      U_INTERNAL_ASSERT_EQUALS(UHTTP::scgi_uri_mask, 0)

      if (x) U_NEW(UString, UHTTP::scgi_uri_mask, UString(x));

      scgi_keep_conn = cfg.readBoolean(U_CONSTANT_TO_PARAM("SCGI_KEEP_CONN"));

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

int USCGIPlugIn::handlerInit()
{
   U_TRACE_NO_PARAM(1, "USCGIPlugIn::handlerInit()")

   if (connection &&
       UHTTP::scgi_uri_mask)
      {
#  ifdef _MSWINDOWS_
      U_INTERNAL_ASSERT_DIFFERS(connection->port, 0)

      U_NEW(UTCPSocket, connection->socket, UTCPSocket(connection->bIPv6));
#  else
      if (connection->port) U_NEW(UTCPSocket,  connection->socket, UTCPSocket(connection->bIPv6));
      else                  U_NEW(UUnixSocket, connection->socket, UUnixSocket);
#  endif

      if (connection->connect())
         {
         U_SRV_LOG("connection to the scgi-backend %V accepted", connection->host_port.rep);

#     ifndef U_ALIAS
         U_ERROR("Sorry, I can't run scgi plugin because alias URI support is missing, please recompile ULib");
#     else
         // NB: SCGI is NOT a static page...

         if (UHTTP::valias == 0) U_NEW(UVector<UString>, UHTTP::valias, UVector<UString>(2U));

         UHTTP::valias->push_back(*UHTTP::scgi_uri_mask);
         UHTTP::valias->push_back(*UString::str_nostat);

         U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
#     endif
         }

      delete connection;
             connection = 0;
      }

   U_RETURN(U_PLUGIN_HANDLER_ERROR);
}

// Connection-wide hooks

int USCGIPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "USCGIPlugIn::handlerRequest()")

   if (connection &&
       UHTTP::isSCGIRequest())
      {
      // Set environment for the SCGI application server

      char* equalPtr;
      char* envp[128];
      UString environment(U_CAPACITY);

      if (UHTTP::getCGIEnvironment(environment, U_WSCGI) == false) U_RETURN(U_PLUGIN_HANDLER_ERROR);

      int n = u_split(U_STRING_TO_PARAM(environment), envp, 0);

      U_INTERNAL_ASSERT_MINOR(n, 128)

#  ifdef DEBUG
      uint32_t hlength = 0; // calculate the total length of the headers
#  endif

      for (int i = 0; i < n; ++i)
         {
         equalPtr = strchr(envp[i], '=');

         if (equalPtr)
            {
            U_INTERNAL_ASSERT_MAJOR(equalPtr-envp[i], 0)
            U_INTERNAL_ASSERT_MAJOR(u__strlen(equalPtr+1, __PRETTY_FUNCTION__), 0)

#        ifdef DEBUG
            hlength += (equalPtr - envp[i]) + u__strlen(equalPtr, __PRETTY_FUNCTION__) + 1;
#        endif

            *equalPtr = '\0';
            }
         }

      n = environment.size();

      U_INTERNAL_ASSERT_EQUALS((int)hlength, n)

      // send header data as netstring -> [len]":"[string]","

      UString request(10U + n);

      request.snprintf(U_CONSTANT_TO_PARAM("%u:%v,"), environment.size(), environment.rep);

      (void) request.append(*UClientImage_Base::body);

      connection->prepareRequest(request);

      if (connection->sendRequestAndReadResponse() == false)
         {
         UHTTP::setInternalError();

         goto end;
         }

      if (scgi_keep_conn == false)
         {
         /**
          * The shutdown() tells the receiver the server is done sending data. No
          * more data is going to be send. More importantly, it doesn't close the
          * socket. At the socket layer, this sends a TCP/IP FIN packet to the receiver
          */

         if (connection->shutdown(SHUT_WR) == false)
            {
            UHTTP::setInternalError();

            goto end;
            }
         }

      *UClientImage_Base::wbuffer = connection->getResponse();

      if (UHTTP::processCGIOutput(false, false)) UClientImage_Base::setRequestProcessed();
      else                                       UHTTP::setInternalError();

end:  connection->clearData();

      if (scgi_keep_conn == false &&
          connection->isConnected())
         {
         connection->close();
         }

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* USCGIPlugIn::dump(bool reset) const
{
   *UObjectIO::os << "scgi_keep_conn              " << scgi_keep_conn         <<  '\n'
                  << "connection    (UClient_Base " << (void*)connection      << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
