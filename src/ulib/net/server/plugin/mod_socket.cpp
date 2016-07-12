// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_socket.cpp - this is a plugin web socket for userver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/command.h>
#include <ulib/file_config.h>
#include <ulib/utility/uhttp.h>
#include <ulib/utility/services.h>
#include <ulib/utility/websocket.h>
#include <ulib/net/server/server.h>
#include <ulib/net/server/plugin/mod_socket.h>

U_CREAT_FUNC(server_plugin_socket, UWebSocketPlugIn)

vPFi      UWebSocketPlugIn::on_message;
UCommand* UWebSocketPlugIn::command;

UWebSocketPlugIn::UWebSocketPlugIn()
{
   U_TRACE_REGISTER_OBJECT(0, UWebSocketPlugIn, "")
}

UWebSocketPlugIn::~UWebSocketPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, UWebSocketPlugIn)

   if (command)             delete command;
   if (UWebSocket::rbuffer) delete UWebSocket::rbuffer;
}

RETSIGTYPE UWebSocketPlugIn::handlerForSigTERM(int signo)
{
   U_TRACE(0, "[SIGTERM] UWebSocketPlugIn::handlerForSigTERM(%d)", signo)

   UInterrupt::sendOurselves(SIGTERM);
}

// Server-wide hooks

int UWebSocketPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "UWebSocketPlugIn::handlerConfig(%p)", &cfg)

   // Perform registration of web socket method
   // ----------------------------------------------------------------------------------------------
   // COMMAND                          command (alternative to USP websocket) to execute
   // ENVIRONMENT      environment for command (alternative to USP websocket) to execute
   //
   // MAX_MESSAGE_SIZE Maximum size (in bytes) of a message to accept; default is approximately 4GB
   // ----------------------------------------------------------------------------------------------

   if (cfg.loadTable())
      {
      command = UServer_Base::loadConfigCommand();

      UWebSocket::max_message_size = cfg.readLong(U_CONSTANT_TO_PARAM("MAX_MESSAGE_SIZE"), U_STRING_MAX_SIZE);

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

int UWebSocketPlugIn::handlerRun()
{
   U_TRACE_NO_PARAM(0, "UWebSocketPlugIn::handlerRun()")

   U_INTERNAL_ASSERT_EQUALS(UWebSocket::rbuffer, 0)

   U_NEW(UString, UWebSocket::rbuffer, UString(U_CAPACITY));

   UHTTP::UServletPage* usp = UHTTP::getUSP(U_CONSTANT_TO_PARAM("/modsocket"));

   if (usp)
      {
      U_INTERNAL_DUMP("usp->runDynamicPage = %p", usp->runDynamicPage)

      U_INTERNAL_ASSERT_POINTER(usp->runDynamicPage)

      on_message = usp->runDynamicPage;

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   if (command) U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);

   U_RETURN(U_PLUGIN_HANDLER_ERROR);
}

// Connection-wide hooks

int UWebSocketPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "UWebSocketPlugIn::handlerRequest()")

   if (U_http_websocket_len)
      {
      int fdmax = 0;
      fd_set fd_set_read, read_set;
      bool bcommand = (command && on_message == 0);

      if (bcommand)
         {
         // Set environment for the command application server

         static int fd_stderr;

         if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/UWebSocketPlugIn.err");

         UString environment(U_CAPACITY);

         (void) environment.append(command->getStringEnvironment());

         if (UHTTP::getCGIEnvironment(environment, U_GENERIC) == false) U_RETURN(U_PLUGIN_HANDLER_ERROR);

         command->setEnvironment(&environment);

         if (command->execute((UString*)-1, (UString*)-1, -1, fd_stderr))
            {
#        ifndef U_LOG_DISABLE
            UServer_Base::logCommandMsgError(command->getCommand(), true);
#        endif

            UInterrupt::setHandlerForSignal(SIGTERM, (sighandler_t)UWebSocketPlugIn::handlerForSigTERM); // sync signal
            }

         FD_ZERO(&fd_set_read);
         FD_SET(UProcess::filedes[2], &fd_set_read);
         FD_SET(UServer_Base::csocket->iSockDesc, &fd_set_read);

         fdmax = U_max(UServer_Base::csocket->iSockDesc, UProcess::filedes[2]) + 1;
         }

      UWebSocket::checkForInitialData(); // check if we have read more data than necessary...

      if (bcommand == false) goto handle_data;

loop: read_set = fd_set_read;

      if (U_SYSCALL(select, "%d,%p,%p,%p,%p", fdmax, &read_set, 0, 0, 0) > 0)
         {
         if (FD_ISSET(UProcess::filedes[2], &read_set))
            {
            UWebSocket::rbuffer->setEmpty();

            if (UServices::read(UProcess::filedes[2], *UWebSocket::rbuffer) &&
                UWebSocket::sendData(UWebSocket::message_type, (const unsigned char*)U_STRING_TO_PARAM(*UWebSocket::rbuffer)))
               {
               UWebSocket::rbuffer->setEmpty();

               goto loop;
               }
            }
         else if (FD_ISSET(UServer_Base::csocket->iSockDesc, &read_set))
            {
handle_data:
            if (UWebSocket::handleDataFraming(UServer_Base::csocket) == STATUS_CODE_OK &&
                (bcommand == false ? (on_message(0), U_http_info.nResponseCode != HTTP_INTERNAL_ERROR)
                                   : UNotifier::write(UProcess::filedes[1], U_STRING_TO_PARAM(*UClientImage_Base::wbuffer))))
               {
               UWebSocket::rbuffer->setEmpty();

               if (bcommand == false) goto handle_data;

               goto loop;
               }
            }
         }

      // Send server-side closing handshake

      if (UServer_Base::csocket->isOpen())
         {
         (void) UWebSocket::sendClose();

         UClientImage_Base::close();
         }

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UWebSocketPlugIn::dump(bool reset) const
{
   *UObjectIO::os << "on_message        " << (void*)on_message << '\n'
                  << "command (UCommand " << (void*)command    << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
