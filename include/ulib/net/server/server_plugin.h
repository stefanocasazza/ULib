// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    server_plugin.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_SERVER_PLUGIN_H
#define U_SERVER_PLUGIN_H 1

#include <ulib/dynamic/plugin.h>

/**
 * The plugin interface is an integral part of UServer which provides a flexible way to add specific
 * functionality to UServer. Plugins allow you to enhance the functionality of UServer without changing
 * the core of the server. They can be loaded at startup time and can change virtually some aspect of
 * the behaviour of the server
 *
 * UServer has 7 hooks which are used in different states of the execution of the request:
 * ----------------------------------------------------------------------------------------------------
 *  * Server-wide hooks:
 *  ````````````````````
 *  1) handlerConfig: called when the server finished to process its configuration
 *  2) handlerInit:   called when the server start    to process its init
 *  3) handlerRun:    called when the server finished to process its init, and before start to run
 *  4) handlerFork:   called when the server have forked a child
 *  5) handlerStop:   called when the server shut down
 *
 *  * Connection-wide hooks: called in `UClientImage_Base::handlerRead()`
 *  ````````````````````````
 *  6) handlerREAD:
 *  7) handlerRequest:
 * ----------------------------------------------------------------------------------------------------
 *
 * RETURNS:
 *  U_PLUGIN_HANDLER_OK        ok
 *  U_PLUGIN_HANDLER_ERROR     error
 *  U_PLUGIN_HANDLER_AGAIN     the request is not ready (NONBLOCKING)
 *  U_PLUGIN_HANDLER_PROCESSED the request has been processed
 *  U_PLUGIN_HANDLER_FINISHED  the request processing is complete
 */

enum PluginHandlerReturn {
   U_PLUGIN_HANDLER_OK        = 0x000,
   U_PLUGIN_HANDLER_ERROR     = 0x001,
   U_PLUGIN_HANDLER_AGAIN     = 0x002,
   U_PLUGIN_HANDLER_PROCESSED = 0x004,
   U_PLUGIN_HANDLER_FINISHED  = 0x008
};

class UFileConfig;

class UServerPlugIn {
public:

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

       UServerPlugIn() {}
   virtual ~UServerPlugIn() {}

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg)
      {
      U_TRACE(0, "UServerPlugIn::handlerConfig(%p)", &cfg)

      U_RETURN(U_PLUGIN_HANDLER_OK);
      }

   virtual int handlerInit()
      {
      U_TRACE_NO_PARAM(0, "UServerPlugIn::handlerInit()")

      U_RETURN(U_PLUGIN_HANDLER_OK);
      }

   virtual int handlerRun()
      {
      U_TRACE_NO_PARAM(0, "UServerPlugIn::handlerRun()")

      U_RETURN(U_PLUGIN_HANDLER_OK);
      }

   virtual int handlerFork()
      {
      U_TRACE_NO_PARAM(0, "UServerPlugIn::handlerFork()")

      U_RETURN(U_PLUGIN_HANDLER_OK);
      }

   virtual int handlerStop()
      {
      U_TRACE_NO_PARAM(0, "UServerPlugIn::handlerStop()")

      U_RETURN(U_PLUGIN_HANDLER_OK);
      }

   // Connection-wide hooks

   virtual int handlerREAD()
      {
      U_TRACE_NO_PARAM(0, "UServerPlugIn::handlerREAD()")

      U_RETURN(U_PLUGIN_HANDLER_OK);
      }

   virtual int handlerRequest()
      {
      U_TRACE_NO_PARAM(0, "UServerPlugIn::handlerRequest()")

      U_RETURN(U_PLUGIN_HANDLER_OK);
      }

   // SigHUP hook

   virtual int handlerSigHUP()
      {
      U_TRACE_NO_PARAM(0, "UServerPlugIn::handlerSigHUP()")

      U_RETURN(U_PLUGIN_HANDLER_OK);
      }

#ifdef DEBUG
   const char* dump(bool reset) const { return ""; }
#endif

private:
   U_DISALLOW_ASSIGN(UServerPlugIn)
};

#endif
