// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_rpc.cpp - this is a plugin rpc for userver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/rpc/rpc.h>
#include <ulib/net/server/server.h>
#include <ulib/net/rpc/rpc_object.h>
#include <ulib/net/rpc/rpc_parser.h>
#include <ulib/net/server/plugin/mod_rpc.h>

U_CREAT_FUNC(server_plugin_rpc, URpcPlugIn)

bool        URpcPlugIn::is_rpc_msg;
URPCParser* URpcPlugIn::rpc_parser;

URpcPlugIn::~URpcPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, URpcPlugIn)

   if (rpc_parser)
      {
      delete rpc_parser;
      delete URPCMethod::encoder;
      delete URPCObject::dispatcher;
      }
}

// Server-wide hooks

int URpcPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "URpcPlugIn::handlerConfig(%p)", &cfg)

   // Perform registration of server RPC method

   U_NEW(URPCParser, rpc_parser, URPCParser);

   URPCObject::loadGenericMethod(&cfg);

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

__pure int URpcPlugIn::handlerInit()
{
   U_TRACE_NO_PARAM(0, "URpcPlugIn::handlerInit()")

   if (rpc_parser) U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);

   U_RETURN(U_PLUGIN_HANDLER_ERROR);
}

// Connection-wide hooks

int URpcPlugIn::handlerREAD()
{
   U_TRACE_NO_PARAM(0, "URpcPlugIn::handlerREAD()")

   if (rpc_parser)
      {
      is_rpc_msg = URPC::readRequest(UServer_Base::csocket); // NB: URPC::resetInfo() it is already called by clearData()...

      if (is_rpc_msg) U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_FINISHED);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

int URpcPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "URpcPlugIn::handlerRequest()")

   if (is_rpc_msg)
      {
      // process the RPC request

      U_INTERNAL_ASSERT_POINTER(rpc_parser)
      U_INTERNAL_ASSERT(*UClientImage_Base::request)

      bool bSendingFault;
      UString method = UClientImage_Base::request->substr(0U, U_TOKEN_NM);

      *UClientImage_Base::wbuffer = rpc_parser->processMessage(method, *URPCObject::dispatcher, bSendingFault);

      U_SRV_LOG_WITH_ADDR("method %V process %s for", method.rep, (bSendingFault ? "failed" : "passed"));

#  ifndef U_LOG_DISABLE
      if (UServer_Base::isLog()) (void) UClientImage_Base::request_uri->assign(method);
#  endif

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_FINISHED);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* URpcPlugIn::dump(bool reset) const
{
   *UObjectIO::os << "is_rpc_msg             " << is_rpc_msg        << '\n'
                  << "rpc_parser (URPCParser " << (void*)rpc_parser << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
