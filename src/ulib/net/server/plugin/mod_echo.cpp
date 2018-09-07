// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_echo.cpp - this is plugin ECHO for userver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/uhttp.h>
#include <ulib/net/server/client_image.h>
#include <ulib/net/server/server_plugin.h>
#include <ulib/net/server/plugin/mod_echo.h>

U_CREAT_FUNC(server_plugin_echo, UEchoPlugIn)

/*
#define U_ECHO_RESPONSE_FOR_TEST \
   "HTTP/1.0 200 OK\r\n" \
   "Server: ULib\r\n" \
   "Connection: close\r\n" \
   "Content-Type: text/html\r\n" \
   "Content-Length: 22\r\n\r\n" \
   "<h1>Hello stefano</h1>"
*/

UEchoPlugIn::~UEchoPlugIn()
{
   U_TRACE_DTOR(0, UEchoPlugIn)
}

// Server-wide hooks

// Connection-wide hooks

int UEchoPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "UEchoPlugIn::handlerRequest()")

   U_ASSERT(UClientImage_Base::body->empty())

   UClientImage_Base::bnoheader = true;

   UClientImage_Base::setRequestProcessed();

#ifndef U_ECHO_RESPONSE_FOR_TEST
   UClientImage_Base::wbuffer->replace(*UClientImage_Base::request);
#else
   (void) UClientImage_Base::wbuffer->assign(U_CONSTANT_TO_PARAM(U_ECHO_RESPONSE_FOR_TEST));
#endif

   // *UClientImage_Base::body = *UHTTP::body;

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED);
}
