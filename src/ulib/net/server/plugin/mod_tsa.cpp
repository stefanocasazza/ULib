// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_tsa.cpp - this is a plugin tsa for userver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/command.h>
#include <ulib/file_config.h>
#include <ulib/utility/uhttp.h>
#include <ulib/net/server/server.h>
#include <ulib/net/server/plugin/mod_tsa.h>

U_CREAT_FUNC(server_plugin_tsa, UTsaPlugIn)

UCommand* UTsaPlugIn::command;

UTsaPlugIn::UTsaPlugIn()
{
   U_TRACE_REGISTER_OBJECT(0, UTsaPlugIn, "")
}

UTsaPlugIn::~UTsaPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, UTsaPlugIn)

   if (command) delete command;
}

// Server-wide hooks

int UTsaPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "UTsaPlugIn::handlerConfig(%p)", &cfg)

   // Perform registration of userver method
   // -----------------------------------------------
   // COMMAND                      command to execute
   // ENVIRONMENT  environment for command to execute
   // -----------------------------------------------

   if (cfg.loadTable())
      {
      command = UServer_Base::loadConfigCommand();

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

int UTsaPlugIn::handlerInit()
{
   U_TRACE_NO_PARAM(0, "UTsaPlugIn::handlerInit()")

   if (command)
      {
      // NB: tsa is NOT a static page, so to avoid stat() syscall we use alias mechanism...

#  ifndef U_ALIAS
      U_SRV_LOG("WARNING: Sorry, I can't enable TSA plugin because alias URI support is missing, please recompile ULib");
#  else
      if (UHTTP::valias == 0) U_NEW(UVector<UString>, UHTTP::valias, UVector<UString>(2U));

      UHTTP::valias->push_back(*UString::str_tsa);
      UHTTP::valias->push_back(*UString::str_nostat);

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
#  endif
      }

   U_RETURN(U_PLUGIN_HANDLER_ERROR);
}

// Connection-wide hooks

int UTsaPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "UTsaPlugIn::handlerRequest()")

   if (UHTTP::isTSARequest())
      {
      // process TSA request

      UString body;

      if (command->execute(UClientImage_Base::body, &body))
         {
         U_http_info.nResponseCode = HTTP_OK;

         UHTTP::setResponse(*UString::str_ctype_tsa, &body);
         }
      else
         {
         UHTTP::setInternalError();
         }

#  ifndef U_LOG_DISABLE
      UServer_Base::logCommandMsgError(command->getCommand(), true);
#  endif

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UTsaPlugIn::dump(bool reset) const
{
   *UObjectIO::os << "command (UCommand " << (void*)command << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
