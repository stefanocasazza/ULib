// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_skeleton.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_SKELETON_H
#define U_MOD_SKELETON_H 1

#include <ulib/net/server/server_plugin.h>

/**
 * The plugin interface is an integral part of UServer which provides a flexible way to add specific functionality to UServer.
 * Plugins allow you to enhance the functionality of UServer without changing the core of the server. They can be loaded at
 * startup time and can change virtually some aspect of the behaviour of the server.
 *
 * UServer has 7 hooks which are used in different states of the execution of the request:
 * --------------------------------------------------------------------------------------------
 *  * Server-wide hooks:
 *  ````````````````````
 *  1) handlerConfig: called when the server finished to process its configuration
 *  2) handlerInit:   called when the server start    to process its init
 *  3) handlerRun:    called when the server finished to process its init, and before start to run
 *  4) handlerFork:   called when the server have forked a child
 *  5) handlerStop:   called when the server shut down
 *
 *  * Connection-wide hooks:
 *  ````````````````````````
 *  6) handlerREAD:
 *  7) handlerRequest:
 *   called in `UClientImage_Base::handlerRead()`
 * --------------------------------------------------------------------------------------------
 *
 * RETURNS:
 *  U_PLUGIN_HANDLER_GO_ON     ok
 *  U_PLUGIN_HANDLER_ERROR     error
 *  U_PLUGIN_HANDLER_AGAIN     the request is not ready (NONBLOCKING)
 *  U_PLUGIN_HANDLER_FINISHED  the request processing is complete
 *  U_PLUGIN_HANDLER_PROCESSED the request has been processed
 */

class U_EXPORT USkeletonPlugIn : public UServerPlugIn {
public:

            USkeletonPlugIn() : UServerPlugIn() {}
   virtual ~USkeletonPlugIn()                   {}

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks
   virtual int handlerConfig(UFileConfig& cfg) U_DECL_FINAL;
   virtual int handlerInit() U_DECL_FINAL;
   virtual int handlerRun() U_DECL_FINAL;
   virtual int handlerFork() U_DECL_FINAL;
   virtual int handlerStop() U_DECL_FINAL;

   // Connection-wide hooks
   virtual int handlerREAD() U_DECL_FINAL;
   virtual int handlerRequest() U_DECL_FINAL;

private:
   U_DISALLOW_COPY_AND_ASSIGN(USkeletonPlugIn)
};

#endif
