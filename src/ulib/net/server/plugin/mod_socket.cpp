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

#include <ulib/db/rdb.h>
#include <ulib/command.h>
#include <ulib/utility/uhttp.h>
#include <ulib/utility/services.h>
#include <ulib/utility/websocket.h>
#include <ulib/net/server/server.h>
#include <ulib/net/server/plugin/mod_socket.h>

U_CREAT_FUNC(server_plugin_socket, UWebSocketPlugIn)

bool UWebSocketPlugIn::enable_db;

UWebSocketPlugIn::UWebSocketPlugIn()
{
   U_TRACE_CTOR(0, UWebSocketPlugIn, "")
}

UWebSocketPlugIn::~UWebSocketPlugIn()
{
   U_TRACE_DTOR(0, UWebSocketPlugIn)

   if (UWebSocket::command)
      {
      U_DELETE(UWebSocket::command)
      U_DELETE(UWebSocket::penvironment)
      }

   if (UWebSocket::rbuffer)
      {
      U_DELETE(UWebSocket::rbuffer)
      U_DELETE(UWebSocket::message)
      }

   if (UWebSocket::db)
      {
      UWebSocket::db->close();

      U_DELETE(UWebSocket::db)
      }
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
   // ENABLE_DB        enable db management for web socket
   // MAX_MESSAGE_SIZE maximum size (in bytes) of a message to accept; default is approximately 4GB
   // ----------------------------------------------------------------------------------------------

   if (cfg.loadTable())
      {
      UWebSocket::command = UServer_Base::loadConfigCommand();

      U_INTERNAL_ASSERT_EQUALS(UWebSocket::penvironment, U_NULLPTR)

      U_NEW_STRING(UWebSocket::penvironment, UString(U_CAPACITY));

      UWebSocket::max_message_size = cfg.readLong(U_CONSTANT_TO_PARAM("MAX_MESSAGE_SIZE"), U_STRING_MAX_SIZE);

      UWebSocket::fd_stderr = UServices::getDevNull("/tmp/UWebSocketPlugIn.err");

      enable_db = cfg.readBoolean(U_CONSTANT_TO_PARAM("ENABLE_DB"));

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

   if (UWebSocket::command == U_NULLPTR)
      {
      if (UHTTP::getUSP(U_CONSTANT_TO_PARAM("modsocket")) == false) U_RETURN(U_PLUGIN_HANDLER_ERROR);

      U_INTERNAL_DUMP("modsocket->runDynamicPage = %p", UHTTP::usp->runDynamicPage)

      U_INTERNAL_ASSERT_POINTER(UHTTP::usp->runDynamicPage)
      U_INTERNAL_ASSERT_POINTER(UHTTP::usp->runDynamicPageParam)

      UWebSocket::on_message       = UHTTP::usp->runDynamicPage;
      UWebSocket::on_message_param = UHTTP::usp->runDynamicPageParam;
      }

   if (enable_db) UWebSocket::initDb();

   U_RETURN(U_PLUGIN_HANDLER_OK);
}
