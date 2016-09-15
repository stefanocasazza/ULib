// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_fcgi.cpp - Perform simple fastcgi request forwarding
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
#include <ulib/net/server/plugin/mod_fcgi.h>

#ifndef _MSWINDOWS_
#  include <ulib/net/unixsocket.h>
#endif

/**
 * spawn-fcgi is used to spawn remote and local FastCGI processes.
 *
 * While it is obviously needed to spawn remote FastCGI backends (the web server
 * can only spawn local ones), it is recommended to spawn local backends with spawn-fcgi, too.
 *
 * Reasons why you may want to use spawn-fcgi instead of something else:
 *
 * Privilege separation without needing a suid-binary or running a server as root.
 *
 * You can restart your web server and the FastCGI applications without restarting the others.
 *
 * You can run them in different chroot()s.
 *
 * Running your FastCGI applications doesn't depend on the web server you are running,
 * which allows for easier testing of other web servers
 */

// ---------------------------------------------------------------------------------------------------------------
// START Fast CGI stuff
// ---------------------------------------------------------------------------------------------------------------

// Number of bytes in a FCGI_Header. Future versions of the protocol will not reduce this number
#define FCGI_HEADER_LEN 8

// Value for version component of FCGI_Header
#define FCGI_VERSION_1 1

// Values for type component of FCGI_Header
#define FCGI_BEGIN_REQUEST       1
#define FCGI_ABORT_REQUEST       2
#define FCGI_END_REQUEST         3
#define FCGI_PARAMS              4
#define FCGI_STDIN               5
#define FCGI_STDOUT              6
#define FCGI_STDERR              7
#define FCGI_DATA                8
#define FCGI_GET_VALUES          9
#define FCGI_GET_VALUES_RESULT  10
#define FCGI_UNKNOWN_TYPE       11
#define FCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)

// Value for requestId component of FCGI_Header
#define FCGI_NULL_REQUEST_ID 0

typedef struct {
   u_char  version;
   u_char  type;
   u_short request_id;
   u_short content_length;
   u_char  padding_length;
   u_char  reserved;
} FCGI_Header;

// Mask for flags component of FCGI_BeginRequestBody
#define FCGI_KEEP_CONN 1

// Values for role component of FCGI_BeginRequestBody
#define FCGI_RESPONDER  1
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER     3

typedef struct {
   u_short role;
   u_char  flags;
   u_char  reserved[5];
} FCGI_BeginRequestBody;

// Values for protocolStatus component of FCGI_EndRequestBody
#define FCGI_REQUEST_COMPLETE 0
#define FCGI_CANT_MPX_CONN    1
#define FCGI_OVERLOADED       2
#define FCGI_UNKNOWN_ROLE     3

typedef struct {
   u_int  app_status;
   u_char protocol_status;
   u_char reserved[3];
} FCGI_EndRequestBody;

typedef struct {
   FCGI_Header           header;
   FCGI_BeginRequestBody body;
} FCGI_BeginRequestRecord;

typedef struct {
   FCGI_Header         header;
   FCGI_EndRequestBody body;
} FCGI_EndRequestRecord;

// Initialize a fast CGI header record

static FCGI_BeginRequestRecord beginRecord;

void UFCGIPlugIn::set_FCGIBeginRequest()
{
   U_TRACE_NO_PARAM(0, "UFCGIPlugIn::set_FCGIBeginRequest()")

   beginRecord.header.version    = FCGI_VERSION_1;
// beginRecord.header.request_id = htons((uint16_t)u_pid);

   beginRecord.body.role         = htons(FCGI_RESPONDER);
   beginRecord.body.flags        = fcgi_keep_conn;
}

void UFCGIPlugIn::fill_FCGIBeginRequest(u_char type, u_short content_length)
{
   U_TRACE(0, "UFCGIPlugIn::fill_FCGIBeginRequest(%C,%d)", type, content_length)

// beginRecord.header.version        = FCGI_VERSION_1;
   beginRecord.header.type           = type;
   beginRecord.header.request_id     = htons((uint16_t)u_pid);
   beginRecord.header.content_length = htons(content_length);
// beginRecord.header.padding_length = padding_length;
// beginRecord.header.reserved       = 0;
}

// ---------------------------------------------------------------------------------------------------------------
// END Fast CGI stuff
// ---------------------------------------------------------------------------------------------------------------

U_CREAT_FUNC(server_plugin_fcgi, UFCGIPlugIn)

bool          UFCGIPlugIn::fcgi_keep_conn;
char          UFCGIPlugIn::environment_type;
UClient_Base* UFCGIPlugIn::connection;

UFCGIPlugIn::UFCGIPlugIn()
{
   U_TRACE_REGISTER_OBJECT(0, UFCGIPlugIn, "")
}

UFCGIPlugIn::~UFCGIPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, UFCGIPlugIn)

   if (connection) delete connection;
}

// Server-wide hooks

int UFCGIPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "UFCGIPlugIn::handlerConfig(%p)", &cfg)

   // ------------------------------------------------------------------------------------------
   // FCGI_URI_MASK  mask (DOS regexp) of uri type that send request to FCGI (*.php)
   //
   // NAME_SOCKET    file name for the fcgi socket
   //
   // SERVER         host name or ip address for the fcgi host
   // PORT           port number             for the fcgi host
   //
   // RES_TIMEOUT    timeout for response from server FCGI
   // FCGI_KEEP_CONN If not zero, the server FCGI does not close the connection after
   //                responding to request; the plugin retains responsibility for the connection.
   //
   // LOG_FILE       location for file log (use server log if exist)
   // ------------------------------------------------------------------------------------------

   if (cfg.loadTable())
      {
      UClient_Base::cfg = &cfg;

      U_NEW(UClient_Base, connection, UClient_Base(&cfg));

      UString x = cfg.at(U_CONSTANT_TO_PARAM("FCGI_URI_MASK"));

      U_INTERNAL_ASSERT_EQUALS(UHTTP::fcgi_uri_mask,0)

      if (x) U_NEW(UString, UHTTP::fcgi_uri_mask, UString(x));

      fcgi_keep_conn = cfg.readBoolean(U_CONSTANT_TO_PARAM("CGI_KEEP_CONN"));

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

int UFCGIPlugIn::handlerInit()
{
   U_TRACE_NO_PARAM(1, "UFCGIPlugIn::handlerInit()")

   if (connection &&
       UHTTP::fcgi_uri_mask)
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
         U_SRV_LOG("connection to the fastcgi-backend %V accepted", connection->host_port.rep);

         set_FCGIBeginRequest();

#     ifndef U_ALIAS
         U_ERROR("Sorry, I can't run fastcgi plugin because alias URI support is missing, please recompile ULib");
#     else
         // NB: FCGI is NOT a static page...

         if (UHTTP::valias == 0) U_NEW(UVector<UString>, UHTTP::valias, UVector<UString>(2U));

         UHTTP::valias->push_back(*UHTTP::fcgi_uri_mask);
         UHTTP::valias->push_back(*UString::str_nostat);

         environment_type = (UHTTP::fcgi_uri_mask->equal(U_CONSTANT_TO_PARAM("*.php")) ? U_PHP : U_CGI);

         U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
#     endif
         }

      delete connection;
             connection = 0;
      }

   U_RETURN(U_PLUGIN_HANDLER_ERROR);
}

// Connection-wide hooks

int UFCGIPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "UFCGIPlugIn::handlerRequest()")

   if (connection &&
       UHTTP::isFCGIRequest())
      {
      fill_FCGIBeginRequest(FCGI_BEGIN_REQUEST, sizeof(FCGI_BeginRequestBody));

      FCGI_Header* h;
      char* equalPtr;
      char* envp[128];
      uint32_t clength, pos, size;
      unsigned char  headerBuff[8];
      unsigned char* headerBuffPtr;
      UString request(U_CAPACITY), params(U_CAPACITY);
      int nameLen, valueLen, headerLen, byte_to_read, i, n;

      (void) request.append((const char*)&beginRecord, sizeof(FCGI_BeginRequestRecord));

      // Set environment for the FCGI application server

      UString environment(U_CAPACITY);

      if (UHTTP::getCGIEnvironment(environment, environment_type) == false) U_RETURN(U_PLUGIN_HANDLER_ERROR);

      n = u_split(U_STRING_TO_PARAM(environment), envp, 0);

      U_INTERNAL_ASSERT_MINOR(n, 128)

      U_DUMP_ATTRS(envp)

      for (i = 0; i < n; ++i)
         {
         equalPtr = strchr(envp[i], '=');

         if (equalPtr)
            {
             nameLen = (equalPtr - envp[i]);
            valueLen = u__strlen(++equalPtr, __PRETTY_FUNCTION__);

            if (valueLen > 0)
               {  
               U_INTERNAL_ASSERT_MAJOR(nameLen, 0)

               // Builds a name-value pair header from the name length and the value length

               headerBuffPtr = headerBuff;

               if (nameLen < 0x80) *headerBuffPtr++ = (unsigned char) nameLen;
               else
                  {
                  *headerBuffPtr++ = (unsigned char) ((nameLen >> 24) | 0x80);
                  *headerBuffPtr++ = (unsigned char)  (nameLen >> 16);
                  *headerBuffPtr++ = (unsigned char)  (nameLen >>  8);
                  *headerBuffPtr++ = (unsigned char)   nameLen;
                  }

               if (valueLen < 0x80) *headerBuffPtr++ = (unsigned char) valueLen;
               else
                  {
                  *headerBuffPtr++ = (unsigned char) ((valueLen >> 24) | 0x80);
                  *headerBuffPtr++ = (unsigned char)  (valueLen >> 16);
                  *headerBuffPtr++ = (unsigned char)  (valueLen >>  8);
                  *headerBuffPtr++ = (unsigned char)   valueLen;
                  }

               headerLen = headerBuffPtr - headerBuff;

               U_INTERNAL_ASSERT_MAJOR(valueLen, 0)

               (void) params.append((const char*)headerBuff, headerLen);
               (void) params.append(envp[i], nameLen);
               (void) params.append(equalPtr, valueLen);
               }
            }
         }

      fill_FCGIBeginRequest(FCGI_PARAMS, params.size());

      (void) request.append((const char*)&beginRecord, FCGI_HEADER_LEN);
      (void) request.append(params);

      // maybe we have some data to put on stdin of cgi process (POST)

      U_INTERNAL_DUMP("UClientImage_Base::body(%u) = %V", UClientImage_Base::body->size(), UClientImage_Base::body->rep)

      size = UClientImage_Base::body->size();

      if (size)
         {
         U_INTERNAL_ASSERT(UHTTP::isPOST())

         fill_FCGIBeginRequest(FCGI_PARAMS, 0);

         (void) request.append((const char*)&beginRecord, FCGI_HEADER_LEN);
         }

      fill_FCGIBeginRequest(FCGI_STDIN, size);

      (void) request.append((const char*)&beginRecord, FCGI_HEADER_LEN);

      if (size)
         {
         (void) request.append(*UClientImage_Base::body);

         fill_FCGIBeginRequest(FCGI_STDIN, 0);

         (void) request.append((const char*)&beginRecord, FCGI_HEADER_LEN);
         }

      // Send request and read fast cgi header+record

      connection->prepareRequest(request);

      if (connection->sendRequestAndReadResponse() == false)
         {
         UHTTP::setInternalError();

         U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
         }

      if (fcgi_keep_conn == false)
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

      pos = 0;

      while (true)
         {
         U_INTERNAL_DUMP("response.c_pointer(%u) = %#.*S", pos, 16, connection->response.c_pointer(pos))

         U_INTERNAL_ASSERT((connection->response.size() - pos) >= FCGI_HEADER_LEN)

         h = (FCGI_Header*)connection->response.c_pointer(pos);

         U_INTERNAL_DUMP("version = %C request_id = %u", h->version, ntohs(h->request_id))

         U_INTERNAL_ASSERT_EQUALS(h->version,    FCGI_VERSION_1)
         U_INTERNAL_ASSERT_EQUALS(h->request_id, htons((uint16_t)u_pid))

         h->content_length = ntohs(h->content_length);

         clength      = h->content_length + h->padding_length;
         byte_to_read = pos + FCGI_HEADER_LEN + clength - connection->response.size();

         U_INTERNAL_DUMP("pos = %u clength = %u response.size() = %u byte_to_read = %d", pos, clength, connection->response.size(), byte_to_read)

         if (byte_to_read > 0 &&
             connection->readResponse(byte_to_read) == false)
            {
            break;
            }

         // NB: connection->response can be resized...

         h = (FCGI_Header*)connection->response.c_pointer(pos);

         // Record fully read

         pos += FCGI_HEADER_LEN;

         // Process this fcgi record

         U_INTERNAL_DUMP("h->type = %C", h->type)

         switch (h->type)
            {
            case FCGI_STDOUT:
               if (clength) (void) UClientImage_Base::wbuffer->append(connection->response.substr(pos, clength));
            break;

            case FCGI_STDERR:
               (void) UFile::writeToTmp(connection->response.c_pointer(pos), clength, O_RDWR | O_APPEND, U_CONSTANT_TO_PARAM("server_plugin_fcgi.err"), 0);
            break;

            case FCGI_END_REQUEST:
               {
               FCGI_EndRequestBody* body = (FCGI_EndRequestBody*)connection->response.c_pointer(pos);

               U_INTERNAL_DUMP("protocol_status = %C app_status = %u", body->protocol_status, ntohl(body->app_status))

               if (body->protocol_status == FCGI_REQUEST_COMPLETE)
                  {
                  U_INTERNAL_ASSERT_EQUALS(pos + clength, connection->response.size())

                  if (UHTTP::processCGIOutput(false, false)) UClientImage_Base::setRequestProcessed();
                  else                                       UHTTP::setInternalError();

                  goto end;
                  }
               }
            // NB: lack of break is intentional...

            // not implemented

            case FCGI_UNKNOWN_TYPE:
            case FCGI_GET_VALUES_RESULT:
            default:
               {
               UHTTP::setInternalError();

               goto end;
               }
            }

         pos += clength;

         U_INTERNAL_DUMP("pos = %u response.size() = %u", pos, connection->response.size())

         if ((connection->response.size() - pos) < FCGI_HEADER_LEN &&
             connection->readResponse(U_SINGLE_READ) == false)
            {
            break;
            }
         }

end:  connection->clearData();

      if (fcgi_keep_conn == false &&
          connection->isConnected())
         {
         connection->close();
         }

      U_RETURN(U_PLUGIN_HANDLER_GO_ON | U_PLUGIN_HANDLER_PROCESSED);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UFCGIPlugIn::dump(bool reset) const
{
   *UObjectIO::os << "fcgi_keep_conn              " << fcgi_keep_conn         << '\n'
                  << "connection    (UClient_Base " << (void*)connection      << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
