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

   uint32_t sz;
   const char* ptr;

#ifdef U_ECHO_RESPONSE_FOR_TEST
   ptr =                 U_ECHO_RESPONSE_FOR_TEST;
    sz = U_CONSTANT_SIZE(U_ECHO_RESPONSE_FOR_TEST);
#else
   UString tmp = UClientImage_Base::request->substr(0U, U_http_info.endHeader) + *UHTTP::body;

   ptr = tmp.data();
    sz = tmp.size();
#endif
   (void) memcpy(UClientImage_Base::wbuffer->data(), ptr, sz); 

   UClientImage_Base::wbuffer->size_adjust_constant(sz);

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED);
}
