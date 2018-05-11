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

int       UWebSocketPlugIn::fd_stderr;
vPFi      UWebSocketPlugIn::on_message;
UString*  UWebSocketPlugIn::penvironment;
UCommand* UWebSocketPlugIn::command;

UWebSocketPlugIn::UWebSocketPlugIn()
{
   U_TRACE_CTOR(0, UWebSocketPlugIn, "")
}

UWebSocketPlugIn::~UWebSocketPlugIn()
{
   U_TRACE_DTOR(0, UWebSocketPlugIn)

   if (command)
      {
      U_DELETE(command)
      U_DELETE(penvironment)
      }

   if (UWebSocket::rbuffer)
      {
      U_DELETE(UWebSocket::rbuffer)
      U_DELETE(UWebSocket::message)
      }
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

   // ----------------------------------------------------------------------------------------------
   // Perform registration of web socket method
   // ----------------------------------------------------------------------------------------------
   // COMMAND                     command (alternative to USP modsocket) to execute
   // ENVIRONMENT environment for command (alternative to USP modsocket) to execute
   //
   // MAX_MESSAGE_SIZE Maximum size (in bytes) of a message to accept; default is approximately 4GB
   // ----------------------------------------------------------------------------------------------

   if (cfg.loadTable())
      {
      command = UServer_Base::loadConfigCommand();

      U_NEW_STRING(penvironment, UString(U_CAPACITY));

      UWebSocket::max_message_size = cfg.readLong(U_CONSTANT_TO_PARAM("MAX_MESSAGE_SIZE"), U_STRING_MAX_SIZE);

      fd_stderr = UServices::getDevNull("/tmp/UWebSocketPlugIn.err");

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED);
      }

   U_RETURN(U_PLUGIN_HANDLER_OK);
}

int UWebSocketPlugIn::handlerRun()
{
   U_TRACE_NO_PARAM(0, "UWebSocketPlugIn::handlerRun()")

   U_INTERNAL_ASSERT_EQUALS(UWebSocket::rbuffer, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(UWebSocket::message, U_NULLPTR)

   U_NEW_STRING(UWebSocket::rbuffer, UString(U_CAPACITY));
   U_NEW_STRING(UWebSocket::message, UString(U_CAPACITY));

   if (command == U_NULLPTR)
      {
      if (UHTTP::getUSP(U_CONSTANT_TO_PARAM("modsocket")) == false) U_RETURN(U_PLUGIN_HANDLER_ERROR);

      U_INTERNAL_DUMP("modsocket->runDynamicPage = %p", UHTTP::usp->runDynamicPage)

      U_INTERNAL_ASSERT_POINTER(UHTTP::usp->runDynamicPage)

      on_message = UHTTP::usp->runDynamicPage;
      }

   U_RETURN(U_PLUGIN_HANDLER_OK);
}

// Connection-wide hooks

int UWebSocketPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "UWebSocketPlugIn::handlerRequest()")

   if (U_http_websocket_len)
      {
      int fdmax = 0; // NB: to avoid 'warning: fdmax may be used uninitialized in this function'...
      fd_set fd_set_read, read_set;

      UWebSocket::checkForInitialData(); // check if we have read more data than necessary...

      if (command == U_NULLPTR)
         {
         on_message(U_DPAGE_OPEN);

         goto data;
         }

      // Set environment for the command application server

      penvironment->setBuffer(U_CAPACITY);

      (void) penvironment->append(command->getStringEnvironment());

      if (UHTTP::getCGIEnvironment(*penvironment, U_GENERIC) == false) U_RETURN(U_PLUGIN_HANDLER_ERROR);

      command->setEnvironment(penvironment);

      if (command->execute((UString*)-1, (UString*)-1, -1, fd_stderr))
         {
         U_SRV_LOG_CMD_MSG_ERR(*command, true);

         UInterrupt::setHandlerForSignal(SIGTERM, (sighandler_t)UWebSocketPlugIn::handlerForSigTERM); // sync signal
         }

      FD_ZERO(&fd_set_read);
      FD_SET(UProcess::filedes[2], &fd_set_read);
      FD_SET(UServer_Base::csocket->iSockDesc, &fd_set_read);

      fdmax = U_max(UServer_Base::csocket->iSockDesc, UProcess::filedes[2]) + 1;

loop: read_set = fd_set_read;

      if (U_SYSCALL(select, "%d,%p,%p,%p,%p", fdmax, &read_set, U_NULLPTR, U_NULLPTR, U_NULLPTR) > 0)
         {
         if (FD_ISSET(UProcess::filedes[2], &read_set))
            {
            UWebSocket::rbuffer->setEmpty();

            if (UServices::read(UProcess::filedes[2], *UWebSocket::rbuffer) &&
                UWebSocket::sendData(UServer_Base::csocket, UWebSocket::message_type, *UWebSocket::rbuffer))
               {
               UWebSocket::rbuffer->setEmpty();

               goto loop;
               }
            }
         else if (FD_ISSET(UServer_Base::csocket->iSockDesc, &read_set))
            {
data:       if (UWebSocket::handleDataFraming(UServer_Base::csocket) == U_WS_STATUS_CODE_OK)
               {
               if (command == U_NULLPTR)
                  {
                  on_message(0);

                  if (U_http_info.nResponseCode != HTTP_INTERNAL_ERROR)
                     {
                     UWebSocket::rbuffer->setEmpty();

                     goto data;
                     }
                  }
               else if (UNotifier::write(UProcess::filedes[1], U_STRING_TO_PARAM(*UClientImage_Base::wbuffer)))
                  {
                  UWebSocket::rbuffer->setEmpty();

                  goto loop;
                  }
               }
            }
         }

      // Send server-side closing handshake

      if (UServer_Base::csocket->isOpen() &&
          UWebSocket::sendClose(UServer_Base::csocket))
         {
         UClientImage_Base::close();
         }
      else
         {
         UClientImage_Base::setRequestProcessed();
         }

      if (command == U_NULLPTR) on_message(U_DPAGE_CLOSE);

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED);
      }

   U_RETURN(U_PLUGIN_HANDLER_OK);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UWebSocketPlugIn::dump(bool reset) const
{
   *UObjectIO::os << "fd_stderr              " << fd_stderr           << '\n'
                  << "on_message             " << (void*)on_message   << '\n'
                  << "command      (UCommand " << (void*)command      << ")\n"
                  << "penvironment (UString  " << (void*)penvironment << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
