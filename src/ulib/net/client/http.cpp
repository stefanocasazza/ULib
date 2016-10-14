// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    http.cpp - client HTTP
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>
#include <ulib/process.h>
#include <ulib/utility/base64.h>
#include <ulib/net/client/http.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/server.h>

#ifdef USE_LIBMAGIC
#  include <ulib/magic/magic.h>
#endif

/**
 * An Example with HTTP/1.0 
 * ==========================================================================================
 * server{meng}% telnet www.cs.panam.edu 80
 * Trying 129.113.132.240...
 * Connected to server.cs.panam.edu.
 * Escape character is '^]'.
 * GET /index.html HTTP/1.0
 * From: meng@panam.edu
 * User-Agent: Test/1.1
 *
 * HTTP/1.1 200 OK
 * Date: Tue, 13 Apr 1999 19:30:29 GMT
 * Server: Apache/1.3.2 (Unix)
 * Last-Modified: Wed, 03 Feb 1999 22:06:29 GMT
 * ETag: "5beed-b41-36b8c865"
 * Accept-Ranges: bytes
 * Content-Length: 2881
 * Connection: close
 * Content-Type: text/html
 *
 * <!DOCTYPE HTML SYSTEM "html.dtd">
 * <HTML>
 * <HEAD>
 * <TITLE>Department of Computer Science UT-Pan American</TITLE>
 * </HEAD>
 * <BODY>
 * <IMG SRC="/LocalIcons/utpa_g_t.gif"  HSPACE=15 VSPACE=0 BORDER=0 ALIGN=left >
 * <!--width=150 height=120--!>
 * <H2>
 * <BR>
 * Department Of Computer Science
 * </H2>
 * <BR  CLEAR=LEFT>
 * <HR>
 * <H2>
 * <center>
 * Welcome</center>
 * </H2>
 * <h3>
 * <hr>
 * ... 
 * </BODY>
 * </HTML>
 * Connection closed by foreign host.
 * ==========================================================================================
 * HTTP 1.1 is superset of HTTP 1.0. HTTP 1.1 added a few more requirements on both the server
 * side and the client side. On the client side:
 * ---------------------------------------------------
 * 1) include the "Host: ..." header with each request
 * ---------------------------------------------------
 * Starting with HTTP 1.1, one server at one IP address can be multi-homed, i.e. the home of
 * several Web domains. For example, "www.host1.com" and "www.host2.com" can live on the same
 * server. That's why the Host field is required. Example:
 * GET /path/file.html HTTP/1.1
 * Host: www.host1.com:80
 * [blank line here]
 * -------------------------------------
 * 2) accept responses with chunked data
 * -------------------------------------
 * If a server wants to start sending a response before knowing its total length (like with long
 * script output), it might use the simple chunked transfer-encoding, which breaks the complete
 * response into smaller chunks and sends them in series. You can identify such a response because
 * it contains the "Transfer-Encoding: chunked" header. All HTTP 1.1 clients must be able to receive
 * chunked messages. A chunked message body contains a series of chunks, followed by a line with a
 * single "0" (zero), followed by optional footers (just like headers), and a blank line. Each chunk
 * consists of two parts: a line with the size of the chunk data, in hex, possibly followed by a
 * semicolon and extra parameters you can ignore (none are currently standard), and ending with CRLF.
 * the data itself, followed by CRLF. An example:
 *  HTTP/1.1 200 OK
 *  Date: Fri, 31 Dec 1999 23:59:59 GMT
 *  Content-Type: text/plain
 *  Transfer-Encoding: chunked
 *  [blank line here]
 *  1a; ignore-stuff-here
 *  abcdefghijklmnopqrstuvwxyz
 *  10
 *  1234567890abcdef
 *  0
 *  some-footer: some-value
 *  another-footer: another-value
 *  [blank line here]
 * Note the blank line after the last footer. The length of the text data is 42 bytes (1a + 10, in hex),
 * and the data itself is abcdefghijklmnopqrstuvwxyz1234567890abcdef. The footers should be treated like
 * headers, as if they were at the top of the response. The chunks can contain any binary data, and may
 * be much larger than the examples here. The size-line parameters are rarely used, but you should at
 * least ignore them correctly. Footers are also rare, but might be appropriate for things like checksums
 * or digital signatures.
 * -----------------------------------------------------------------------------------------------------
 * 3) either support persistent connections, or include the "Connection: close" header with each request
 * -----------------------------------------------------------------------------------------------------
 * In HTTP 1.0 and before, TCP connections are closed after each request and response, so each resource to
 * be retrieved requires its own connection. Persistent connections are the default in HTTP 1.1, so nothing
 * special is required to use them. Just open a connection and send several requests in series (called pipelining),
 * and read the responses in the same order as the requests were sent. If you do this, be very careful to read
 * the correct length of each response, to separate them correctly. If a client includes the "Connection: close"
 * header in the request, then the connection will be closed after the corresponding response. Use this if you
 * don't support persistent connections, or if you know a request will be the last on its connection. Similarly,
 * if a response contains this header, then the server will close the connection following that response, and
 * the client shouldn't send any more requests through that connection. A server might close the connection
 * before all responses are sent, so a client must keep track of requests and resend them as needed. When
 * resending, don't pipeline the requests until you know the connection is persistent. Don't pipeline at all
 * if you know the server won't support persistent connections (like if it uses HTTP 1.0, based on a previous response).
 * -----------------------------------
 * 4) handle the 100 Continue response
 * -----------------------------------
 * During the course of an HTTP 1.1 client sending a request to a server, the server might respond with an interim
 * "100 Continue" response. This means the server has received the first part of the request, and can be used to
 * aid communication over slow links. In any case, all HTT 1.1 clients must handle the 100 response correctly
 * (perhaps by just ignoring it). The "100 Continue" response is structured like any HTTP response, i.e. consists
 * of a status line, optional headers, and a blank line. Unlike other responses, it is always followed by another
 * complete, final response. Example:
 *  HTTP/1.0 100 Continue
 *  [blank line here]
 *  HTTP/1.0 200 OK
 *  Date: Fri, 31 Dec 1999 23:59:59 GMT
 *  Content-Type: text/plain
 *  Content-Length: 42
 *  some-footer: some-value
 *  another-footer: another-value
 *  
 *  abcdefghijklmnoprstuvwxyz1234567890abcdef
 * To handle this, a simple HTTP 1.1 client might read one response from the socket; if the status code is 100,
 * discard the first response and read the next one instead.
 * -----------------------------------
 * To comply with HTTP 1.1, servers must:
 * --------------------------------------
 * 1) require the Host: header from HTTP 1.1 clients
 * 2) accept absolute URL's in a request
 * 3) accept requests with chunked data
 * 4) either support persistent connections, or include the "Connection: close" header with each response
 * 5) use the "100 Continue" response appropriately
 * 6) include the Date: header in each response
 * 7) handle requests with If-Modified-Since: or If-Unmodified-Since: headers
 * 8) support at least the GET and HEAD methods
 * 9) support HTTP 1.0 requests
 * ==========================================================================================
 * An example of transaction under HTTP 1.1:
 * ==========================================================================================
 * server{meng}% telnet www.cs.panam.edu 80
 * Trying 129.113.132.240...
 * Connected to server.cs.panam.edu.
 * Escape character is '^]'.
 *  GET /index.html HTTP/1.1
 *  Host: www.cs.panam.edu:80
 *  From: meng@panam.edu
 *  User-Agent: test/1.1
 *  Connection: close
 *  
 *  HTTP/1.1 200 OK
 *  Date: Tue, 13 Apr 1999 20:57:45 GMT
 *  Server: Apache/1.3.2 (Unix)
 *  Last-Modified: Wed, 03 Feb 1999 22:06:29 GMT
 *  ETag: "5beed-b41-36b8c865"
 *  Accept-Ranges: bytes
 *  Content-Length: 2881
 *  Connection: close
 *  Content-Type: text/html
 *  
 *  <!-------------------- Welcome.html ------------------------------!>
 *  <!-- Depertment of Computer Science Home Page --!>
 *  <!DOCTYPE HTML SYSTEM "html.dtd">
 *  <HTML>
 *  <HEAD>
 *  <TITLE>Department of Computer Science UT-Pan American</TITLE>
 *  </HEAD>
 *  <BODY>
 *  <IMG SRC="/LocalIcons/utpa_g_t.gif"  HSPACE=15 VSPACE=0 BORDER=0 ALIGN=left >
 *  <!--width=150 height=120--!>
 *  <H2>
 *  <BR>
 *  Department Of Computer Science
 *  </H2>
 *  ...
 *  </BODY>
 *  </HTML>
 * Connection closed by foreign host.
 * ==========================================================================================
 */

#define U_MAX_REDIRECTS 10 // HTTP 1.0 used to suggest 5

bool             UHttpClient_Base::server_context_flag;
struct uhttpinfo UHttpClient_Base::u_http_info_save;

UHttpClient_Base::UHttpClient_Base(UFileConfig* _cfg) : UClient_Base(_cfg)
{
   U_TRACE_REGISTER_OBJECT(0, UHttpClient_Base, "%p", _cfg)

   method_num       = 0;
   bFollowRedirects = true;
   bproxy           = false;

   u_init_http_method_list();

   U_NEW(UMimeHeader,  requestHeader, UMimeHeader);
   U_NEW(UMimeHeader, responseHeader, UMimeHeader);
}

void UHttpClient_Base::reset()
{
   U_TRACE_NO_PARAM(0, "UHttpClient_Base::reset()")

   body.clear();

    requestHeader->clear();
   responseHeader->clear();

   UClient_Base::reset();
   UClient_Base::server.clear();
}

// =====================================================================================
// In response to a HTTP_UNAUTHORISED response from the HTTP server,
// this function will attempt to generate an Authentication header to satisfy the server
// =====================================================================================

UString UHttpClient_Base::getBasicAuthorizationHeader()
{
   U_TRACE_NO_PARAM(0, "UHttpClient_Base::getBasicAuthorizationHeader()")

   UString headerValue(300U), tmp(100U), data(100U);

   // ---------------------------------------------------------------------------------------------------------------------------
   // According to RFC 2617 HTTP Authentication: Basic and Digest Access Authentication
   // ---------------------------------------------------------------------------------------------------------------------------
   // For "Basic" authentication, the user and password are concatentated with a colon separator before being encoded in base64.
   // According to RFC 2068 (HTTP/1.1) the Username and Password are defined as TEXT productions and are therefore supposed to be
   // encoded in ISO-8859-1 before being Base64-encoded
   // ---------------------------------------------------------------------------------------------------------------------------

   tmp.snprintf(U_CONSTANT_TO_PARAM("%v:%v"), user.rep, password.rep);

   UBase64::encode(tmp, data);

   // Authorization: Basic cy5jYXNhenphOnN0ZWZhbm8x

   headerValue.snprintf(U_CONSTANT_TO_PARAM("Basic %v"), data.rep);

   U_RETURN_STRING(headerValue);
}

bool UHttpClient_Base::createAuthorizationHeader(bool bProxy)
{
   U_TRACE(0, "UHttpClient_Base::createAuthorizationHeader(%b)", bProxy)

   if (    user.empty() ||
       password.empty())
      {
      // If the registered Authenticator cannot supply a user/password then we cannot continue.
      // This is signalled by returning false to the sendRequest() function

      U_RETURN(false);
      }

   uint32_t keylen;
   const char* key;

   if (bProxy)
      {
      key    =                 "Proxy-Authenticate";
      keylen = U_CONSTANT_SIZE("Proxy-Authenticate");
      }
   else
      {
      key    =                 "WWW-Authenticate";
      keylen = U_CONSTANT_SIZE("WWW-Authenticate");
      }

   UString authResponse = responseHeader->getHeader(key, keylen);

   if (authResponse.empty())
      {
      U_DUMP("%.*S header missing from HTTP response: %d", keylen, key, U_http_info.nResponseCode)

      U_RETURN(false);
      }

   // ---------------------------------------------------------------------------------------------------------------------------
   // The authentication header is constructed like a tagged attribute list:
   // ---------------------------------------------------------------------------------------------------------------------------
   // WWW-Authenticate: Basic  realm="SokEvo"
   // WWW-Authenticate: Digest realm="Autenticazione su LDAP/SSL", nonce="GkPcSTxaBAA=666065cb86c557d75991c7b3fa362e7f881abb93",
   //                   algorithm=MD5, qop="auth"
   // ---------------------------------------------------------------------------------------------------------------------------

   UVector<UString> name_value;

   uint32_t n = name_value.split(authResponse, ",= ");

   if (n < 3)
      {
      U_WARNING("%.*S header: %V value is invalid", keylen, key, authResponse.rep);

      U_RETURN(false);
      }

   UString scheme = name_value[0], headerValue(300U);

   // ---------------------------------------------------------------------------------------------------------------------------
   // According to RFC 2617 HTTP Authentication: Basic and Digest Access Authentication
   // ---------------------------------------------------------------------------------------------------------------------------
   // For "Basic" authentication, the user and password are concatentated with a colon separator before being encoded in base64.
   // According to RFC 2068 (HTTP/1.1) the Username and Password are defined as TEXT productions and are therefore supposed to be
   // encoded in ISO-8859-1 before being Base64-encoded
   // ---------------------------------------------------------------------------------------------------------------------------

   if (scheme.equal(U_CONSTANT_TO_PARAM("Basic"))) headerValue = getBasicAuthorizationHeader();
   else
      {
      // WWW-Authenticate: Digest realm="Autenticazione su LDAP/SSL", nonce="86c557d75991c7b3fa362e7f881abb93", algorithm=MD5, qop="auth"

      U_ASSERT(scheme.equal(U_CONSTANT_TO_PARAM("Digest")))

      uint32_t i = 1;
      UString name, value, realm, nonce, algorithm, qop; // "Quality of Protection" (qop)

      while (i < n)
         {
         name  = name_value[i++];
         value = name_value[i++];

         U_INTERNAL_DUMP("name = %V value = %V", name.rep, value.rep)

         switch (name.c_char(0))
            {
            case 'q':
               {
               if (name.equal(U_CONSTANT_TO_PARAM("qop")))
                  {
                  U_ASSERT(qop.empty())

                  qop = value;
                  }
               }
            break;

            case 'r':
               {
               if (name.equal(U_CONSTANT_TO_PARAM("realm")))
                  {
                  U_ASSERT(realm.empty())

                  realm = value;
                  }
               }
            break;

            case 'n':
               {
               if (name.equal(U_CONSTANT_TO_PARAM("nonce")))
                  {
                  U_ASSERT(nonce.empty())

                  nonce = value;
                  }
               }
            break;

            case 'a':
               {
               if (name.equal(U_CONSTANT_TO_PARAM("algorithm")))
                  {
                  algorithm = value;

                  if (algorithm.equal("MD5") == false) U_RETURN(false);
                  }
               }
            break;
            }
         }

      if (  qop.empty() ||
          realm.empty() ||
          nonce.empty())
         {
         // We cannot continue. This is signalled by returning false to the sendRequest() function.

         U_RETURN(false);
         }

      static uint32_t nc; //  nonce: counter incremented by client
                          // cnonce: client generated random nonce (u_now)

      UString a1(100U), ha1(33U),                     // MD5(user : realm : password)
              a2(4 + 1 + UClient_Base::uri.size()),   //     method : uri
              ha2(33U),                               // MD5(method : uri)
              a3(200U), _response(33U);               // MD5(HA1 : nonce : nc : cnonce : qop : HA2)

      // MD5(user : realm : password)

      a1.snprintf(U_CONSTANT_TO_PARAM("%v:%v:%v"), user.rep, realm.rep, password.rep);

      UServices::generateDigest(U_HASH_MD5, 0, a1, ha1, false);

      // MD5(method : uri)

      a2.snprintf(U_CONSTANT_TO_PARAM("%.*s:%v"), U_HTTP_METHOD_NUM_TO_TRACE(method_num), UClient_Base::uri.rep);

      UServices::generateDigest(U_HASH_MD5, 0, a2, ha2, false);

      // MD5(HA1 : nonce : nc : cnonce : qop : HA2)

      a3.snprintf(U_CONSTANT_TO_PARAM("%v:%v:%08u:%ld:%v:%v"), ha1.rep, nonce.rep, ++nc, u_now->tv_sec, qop.rep, ha2.rep);

      UServices::generateDigest(U_HASH_MD5, 0, a3, _response, false);

      // Authorization: Digest username="s.casazza", realm="Protected Area", nonce="1222108408", uri="/ok", cnonce="dad0f85801e27b987d6dc59338c7bf99",
      //                       nc=00000001, response="240312fba053f6d687d10c90928f4af2", qop="auth", algorithm="MD5"

      headerValue.snprintf(U_CONSTANT_TO_PARAM("Digest username=\"%v\", realm=%v, nonce=%v, uri=\"%v\", cnonce=\"%ld\", nc=%08u, response=\"%v\", qop=%v"),
                           user.rep, &realm.rep, nonce.rep, UClient_Base::uri.rep, u_now->tv_sec, nc, _response.rep, qop.rep);

      if (algorithm) (void) headerValue.append(U_CONSTANT_TO_PARAM(", algorithm=\"MD5\""));
      }

   // Only basic and digest authentication is supported at present. By failing to create an authentication header,
   // and returning false, we signal to the caller that the authenticate response should be treated as an error

   if (headerValue)
      {
      setAuthorizationHeader(bProxy, headerValue);

      U_RETURN(true);
      }

   U_RETURN(false);
}

// ------------------------------------------------------------
// category of response:
// ------------------------------------------------------------
// -1 indicates errors of some kind
//  0 redirects the client to another URL with another socket
//  1 redirects the client to another URL using the same socket
//  2 indicates an success, no redirects, ok to read body
// ------------------------------------------------------------

int UHttpClient_Base::checkResponse(int& redirectCount)
{
   U_TRACE(0, "UHttpClient_Base::checkResponse(%d)", redirectCount)

   U_DUMP("HTTP status = (%d, %S)", U_http_info.nResponseCode, UHTTP::getStatusDescription())

   // check if you can use the same socket connection

   bool connection_close = (UClient_Base::isClosed() || responseHeader->isClose());

   // General category of response:
   // ------------------------------------------------------------
   // 1xx indicates an informational message only
   // 2xx indicates success of some kind
   // 3xx redirects the client to another URL
   // 4xx indicates an error on the client's part
   // 5xx indicates an error on the server's part
   // ------------------------------------------------------------

   bool bAuth  = (U_http_info.nResponseCode == HTTP_UNAUTHORIZED), // 401
        bProxy = (U_http_info.nResponseCode == HTTP_PROXY_AUTH);   // 407

   if (bAuth ||
       bProxy)
      {
      // If we haven't already done so, attempt to create an Authentication header. If this fails
      // (due to application not passing the credentials), then we treat it as an error.
      // If we already have one then the server is rejecting it so we have an error anyway

      if ((bAuth  && requestHeader->containsHeader(U_CONSTANT_TO_PARAM("Authorization")))       ||
          (bProxy && requestHeader->containsHeader(U_CONSTANT_TO_PARAM("Proxy-Authorization"))) ||
          (redirectCount == -1 || createAuthorizationHeader(bProxy) == false))
         {
         U_RETURN(-2);
         }

      redirectCount = -1; // NB: we register that we have already made a tentative... 

      // NB: in general is not safe to assume can we can use the same socket connection...

      U_RETURN(0); // connection_close ? 0 : 1
      }

   if (bFollowRedirects)
      {
      UString refresh = responseHeader->getRefresh();

      if (refresh.empty()           == false           ||
          U_http_info.nResponseCode == HTTP_MOVED_PERM || // 301
          U_http_info.nResponseCode == HTTP_MOVED_TEMP)   // 302
         {
         // 3xx redirects the client to another URL

         if (++redirectCount > U_MAX_REDIRECTS)
            {
            U_INTERNAL_DUMP("REDIRECTION LIMIT REACHED...")

            U_RETURN(-1);
            }

         UString newLocation = responseHeader->getLocation();

         if (newLocation.empty() &&
                 refresh.empty() == false)
            {
            uint32_t pos = U_STRING_FIND(refresh, 0, "url=");

            if (pos != U_NOT_FOUND)
               {
               newLocation = refresh.substr(pos + U_CONSTANT_SIZE("url="));

               if (newLocation.isQuoted()) newLocation.rep->unQuote();
               }
            }

         if (newLocation.empty())
            {
            U_INTERNAL_DUMP("LOCATION HEADER MISSING FROM HTTP REDIRECT RESPONSE")

            U_RETURN(-1);
            }

         if (newLocation.find(*UString::str_localhost) != U_NOT_FOUND)
            {
            U_INTERNAL_DUMP("LOCATION HEADER POINT TO LOCALHOST CAUSING DEADLOCK")

            U_RETURN(-1);
            }

         // New locations will possibly need different authentication, so we should reset
         // our origin authentication header (if any)

         requestHeader->removeHeader(U_CONSTANT_TO_PARAM("Authorization"));

         // Store cookie if present "Set-Cookie:"

         setcookie = responseHeader->getHeader(U_CONSTANT_TO_PARAM("Set-Cookie"));

         U_INTERNAL_DUMP("Set-Cookie: %V", setcookie.rep)

         if (bproxy && 
             setcookie)
            {
            U_INTERNAL_DUMP("SET-COOKIE HEADER PRESENT FROM HTTP REDIRECT RESPONSE")

            U_RETURN(2); // no redirection, read body
            }

#     ifdef DEBUG
         UClient_Base::reset();

         newLocation.duplicate(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#     endif

         UClient_Base::iovcnt = 0;

         // Combine the new location with our URL

         if (UClient_Base::setUrl(newLocation) == false && // NB: not modified...
             connection_close                  == false)
            {
            U_RETURN(1); // you can use the same socket connection for the redirect
            }

         U_RETURN(0); // redirects the client to another URL with another socket
         }
      }

   U_RETURN(2); // indicates success, no redirects, ok to read body
}

// QUEUE MODE

bool UHttpClient_Base::putRequestOnQueue() // In general, if sendRequest() failed after we want to call this function...
{
   U_TRACE(0, "UHttpClient_Base::putRequestOnQueue()")

   char _buffer[U_PATH_MAX];

   (void) u__snprintf(_buffer, sizeof(_buffer), U_CONSTANT_TO_PARAM("%v/%v.%4D"), UString::str_CLIENT_QUEUE_DIR->rep, UClient_Base::host_port.rep);

   int fd = UFile::creat(_buffer);

   if (fd == -1) U_RETURN(false);

   if (UClient_Base::iovcnt == 0)
      {
      // we need to compose the request to the HTTP server...

      method_num = 0; // GET

      composeRequest();
      }

   (void) UFile::writev(fd, UClient_Base::iov, UClient_Base::iovcnt);

   UFile::close(fd);

   U_RETURN(true);
}

// it creates a copy of itself, return pid child if parent...

#define U_MAX_ATTEMPTS 10

int UHttpClient_Base::sendRequestAsync(const UString& _url, bool bqueue, const char* log_msg, int log_fd)
{
   U_TRACE(0, "UHttpClient_Base::sendRequestAsync(%V,%b,%S,%d)", _url.rep, bqueue, log_msg, log_fd)

   U_INTERNAL_ASSERT_EQUALS(UClient_Base::socket->isSSLActive(), false)

   pid_t pid = UServer_Base::startNewChild();

   if (pid > 0) U_RETURN(pid); // parent

   // child

   int num_attempts = 1;

   U_INTERNAL_ASSERT_EQUALS(UClient_Base::queue_dir, 0)

   if (UClient_Base::connectServer(_url))
      {
      // we need to compose the request to the HTTP server...

      composeRequest(U_CONSTANT_TO_PARAM("application/x-www-form-urlencoded"));

      UClient_Base::adjustTimeOut();

      UTimeVal to_sleep(10);
loop:
      if (UClient_Base::isOpen() == false) UClient_Base::socket->_socket();

      if ((UClient_Base::isConnected() ||
           UClient_Base::connect())    &&
          sendRequestEngine())
         {
         goto next;
         }

      if (++num_attempts < U_MAX_ATTEMPTS)
         {
         to_sleep.nanosleep();

         goto loop;
         }

next:
      if (log_msg)
         {
         const char* str = (num_attempts < U_MAX_ATTEMPTS ? "success" : "FAILED");

         if (log_fd == -1) ULog::log(        log_msg, strlen(log_msg), str, num_attempts);
         else              ULog::log(log_fd, log_msg, strlen(log_msg), str, num_attempts);
         }
      }

   // QUEUE MODE

   if (bqueue &&
       num_attempts >= U_MAX_ATTEMPTS)
      {
      (void) putRequestOnQueue();
      }

   if (pid == 0) UServer_Base::endNewChild();

   U_RETURN(0);
}

//=============================================================================
// Send the http request to the remote host.
//
// The request is made up of a request line followed by request header fields. Ex:
//
// GET filename HTTP/1.1
// Host: hostname[:port]
// Connection: close
//
// The response from the server will contain a number of header
// fields followed by the requested data.
//
// Note: HTTP Redirection
// ----------------------
// By default we will follow HTTP redirects. These are communicated
// to us by a 3xx HTTP response code and the presence of a "Location" header
// field. A 3xx response code without a Location header is an error.
// Redirection may be an iterative process, so it continues until
// we receive a 200 OK response or the maximum number of redirects is exceeded.
//
// We do not process Location headers when accompanying a 200 OK response
//=============================================================================

void UHttpClient_Base::parseRequest(uint32_t n)
{
   U_TRACE(0, "UHttpClient_Base::parseRequest(%u)", n)

   U_INTERNAL_DUMP("last_request = %V", last_request.rep)

   U_INTERNAL_ASSERT(last_request)

   uint32_t startHeader = last_request.find(U_CRLF, 0, 2) + 2;

   U_ASSERT_RANGE(11, startHeader, last_request.size())

   const char* ptr = last_request.data();

   UClient_Base::iovcnt = n;

   UClient_Base::iov[0].iov_base = (caddr_t)ptr;
   UClient_Base::iov[0].iov_len  = startHeader;
   UClient_Base::iov[1].iov_base = 0;
   UClient_Base::iov[1].iov_len  = 0;
   UClient_Base::iov[2].iov_base = (caddr_t)ptr        + startHeader;
   UClient_Base::iov[2].iov_len  = last_request.size() - startHeader;
}

UString UHttpClient_Base::wrapRequest(UString* req, const UString& host_port, uint32_t method_num, const char* _uri,
                                      uint32_t uri_len, const char* extension, const char* content_type, uint32_t content_type_len)
{
   U_TRACE(0, "UHttpClient_Base::wrapRequest(%p,%V,%u,%.*S,%u,%S,%.*S,%u)",req,host_port.rep,method_num,uri_len,_uri,uri_len,extension,content_type_len,content_type,content_type_len)

   U_INTERNAL_ASSERT_MAJOR(uri_len, 0)
   U_INTERNAL_ASSERT_MAJOR(U_http_method_list[0].len, 0)
   U_INTERNAL_ASSERT_MINOR(method_num, U_NUM_ELEMENTS(U_http_method_list))

   // Add the MIME-type headers to the request for HTTP server

   UString tmp(800U + uri_len + (req ? req->size() : 0));

#ifdef USE_LIBZ
#  define U_WRAPREQ \
      "%.*s %.*s HTTP/1.1\r\n" \
      "Host: %v\r\n" \
      "Accept-Encoding: gzip\r\n" \
      "User-Agent: " PACKAGE_NAME "/" PACKAGE_VERSION "\r\n" \
      "%s"
#else
#  define U_WRAPREQ \
      "%.*s %.*s HTTP/1.1\r\n" \
      "Host: %v\r\n" \
      "User-Agent: " PACKAGE_NAME "/" PACKAGE_VERSION "\r\n" \
      "%s"
#endif

   tmp.snprintf(U_CONSTANT_TO_PARAM(U_WRAPREQ), U_HTTP_METHOD_NUM_TO_TRACE(method_num), uri_len, _uri, host_port.rep, extension);

   if (req)
      {
      U_INTERNAL_ASSERT(*req)
      U_INTERNAL_ASSERT_POINTER(content_type)
      U_INTERNAL_ASSERT_MAJOR(content_type_len, 0)

      tmp.snprintf_add(U_CONSTANT_TO_PARAM("Content-Type: %.*s\r\n"
                       "Content-Length: %u\r\n"
                       "\r\n"),
                       content_type_len, content_type,
                       req->size());

      (void) tmp.append(*req);
      }

   U_RETURN_STRING(tmp);
}

void UHttpClient_Base::composeRequest(const char* content_type, uint32_t content_type_len)
{
   U_TRACE(0, "UHttpClient_Base::composeRequest(%.*S,%u)", content_type_len, content_type, content_type_len)

   U_INTERNAL_ASSERT(UClient_Base::uri)

   U_INTERNAL_DUMP("method_num = %u", method_num)

   if (method_num == 0) // GET
      {
      last_request = wrapRequest(0, UClient_Base::host_port, method_num, U_STRING_TO_PARAM(UClient_Base::uri), "\r\n");

      parseRequest(3);
      }
   else
      {
      U_INTERNAL_ASSERT_POINTER(content_type)
      U_INTERNAL_ASSERT_MAJOR(content_type_len, 0)

      uint32_t sz = body.size();

      U_INTERNAL_ASSERT_MAJOR(sz, 0)

      UClient_Base::iov[3].iov_base = (caddr_t)body.data();
      UClient_Base::iov[3].iov_len  = sz;

      (void) last_request.reserve(UClient_Base::uri.size() + UClient_Base::server.size() + 300U);

      last_request.snprintf(U_CONSTANT_TO_PARAM("%.*s %v HTTP/1.1\r\n"
                            "Host: %v:%u\r\n"
                            "User-Agent: ULib/1.4.2\r\n"
                            "Content-Length: %d\r\n"
                            "Content-Type: %.*s\r\n"
                            "\r\n"),
                            U_HTTP_METHOD_NUM_TO_TRACE(method_num),
                            UClient_Base::uri.rep, UClient_Base::server.rep, UClient_Base::port, sz, content_type_len, content_type);

      parseRequest(4);
      }
}

bool UHttpClient_Base::sendRequestEngine()
{
   U_TRACE_NO_PARAM(0, "UHttpClient_Base::sendRequestEngine()")

   U_INTERNAL_ASSERT_RANGE(1,UClient_Base::iovcnt,6)

   UString headers;
   int result = -1, redirectCount = 0, sendCount = 0;

   while (true)
      {
      if (result == 0 || // redirects the client to another URL with another socket
          result == 1)   // you can use the same socket connection for the redirect
         {
         if (UClient_Base::iovcnt == 0) composeRequest(); // we need to compose the request to the HTTP server...
         }

      // check if there are some headers (Ex. Authentication) to insert in the request...

      if (requestHeader->empty() == false)
         {
         headers = requestHeader->getHeaders();

         UClient_Base::iov[1].iov_base = (caddr_t)headers.data();
         UClient_Base::iov[1].iov_len  =          headers.size();
         }

                 body.clear();
      responseHeader->clear();

      UClient_Base::response.setEmpty();

      result = (UClient_Base::sendRequestAndReadResponse() &&
                responseHeader->readHeader(UClient_Base::socket, UClient_Base::response) // read the HTTP response header
                     ? checkResponse(redirectCount)
                     : -1);

      if (result ==  1) continue;       // redirection, use the same socket connection...
      if (result == -2) U_RETURN(true); // pass HTTP_UNAUTHORISED response to the HTTP client...
      if (result ==  2)                 // no redirection, read body...
         {
         U_DUMP("SERVER RETURNED HTTP RESPONSE: %d", U_http_info.nResponseCode)

         U_http_info.clength = responseHeader->getHeader(U_CONSTANT_TO_PARAM("Content-Length")).strtoul();

         U_INTERNAL_DUMP("U_http_info.clength = %u", U_http_info.clength)

         if (U_http_info.clength == 0)
            {
            bool is_chunked = responseHeader->isChunked();

            if (is_chunked) U_http_flag |=  HTTP_IS_DATA_CHUNKED;
            else            U_http_flag &= ~HTTP_IS_DATA_CHUNKED;

            U_INTERNAL_DUMP("is_chunked = %b U_http_data_chunked = %b", is_chunked, U_http_data_chunked)

            if (is_chunked == false) U_RETURN(true);
            }

         if (UHTTP::readBodyResponse(UClient_Base::socket, &response, body)) U_RETURN(true);

         if (U_http_info.nResponseCode == HTTP_CLIENT_TIMEOUT ||
             U_http_info.nResponseCode == HTTP_ENTITY_TOO_LARGE)
            {
            U_RETURN(false);
            }
         }

      if (result < 0       &&
          (++sendCount > 2 ||
           UClient_Base::isConnected() == false))
         {
         if (UClient_Base::queue_dir) U_RETURN(true); // QUEUE MODE

         U_RETURN(false);
         }

      UClient_Base::close();
      }
}

bool UHttpClient_Base::sendRequest()
{
   U_TRACE_NO_PARAM(0, "UHttpClient_Base::sendRequest()")

   U_INTERNAL_ASSERT_RANGE(0,UClient_Base::iovcnt,6)

   U_INTERNAL_DUMP("server_context_flag = %b U_ClientImage_close = %b", server_context_flag, U_ClientImage_close)

   bool ok;
   uint64_t flag_save = 0;

   if (server_context_flag)
      {
      u_http_info_save = U_http_info;
             flag_save = u_clientimage_info.flag.u;
      }

   if (UClient_Base::iovcnt == 0)
      {
      // we need to compose the request to the HTTP server...

      method_num = 0; // GET

      composeRequest();
      }

#ifdef USE_LIBSSL
   if (isPasswordAuthentication() &&
       UClient_Base::socket->isSSLActive())
      {
      setAuthorizationHeader(false, getBasicAuthorizationHeader());
      }
#endif

   ok = sendRequestEngine();

   if (server_context_flag)
      {
      u_clientimage_info.flag.u = flag_save;

                    u_http_info_save.nResponseCode = U_http_info.nResponseCode;
      U_http_info = u_http_info_save;
      }

   U_INTERNAL_DUMP("server_context_flag = %b U_ClientImage_close = %b", server_context_flag, U_ClientImage_close)

   U_RETURN(ok);
}

bool UHttpClient_Base::sendRequest(const UString& req)
{
   U_TRACE(0, "UHttpClient_Base::sendRequest(%V)", req.rep)

   U_INTERNAL_ASSERT(req)

   last_request = req;

   parseRequest();

   bool result = false;
   char http_method_num_save = U_http_method_num;
                               U_http_method_num = 0;

   if (UHTTP::scanfHeaderRequest((const char*)UClient_Base::iov[0].iov_base, UClient_Base::iov[0].iov_len))
      {
      U_INTERNAL_DUMP("U_http_method_type = %B", U_http_method_type)

      U_INTERNAL_ASSERT(UHTTP::isGETorPOST())

      method_num = (U_http_method_type == HTTP_GET ? 0 : 2); // GET|POST

      (void) UClient_Base::uri.assign(U_HTTP_URI_QUERY_TO_PARAM);

      if (sendRequest()) result = true;
      }

   U_http_method_num = http_method_num_save;

   U_RETURN(result);
}

bool UHttpClient_Base::sendRequest(int method, const char* content_type, uint32_t content_type_len, const char* data, uint32_t data_len, const char* _uri, uint32_t uri_len)
{
   U_TRACE(0, "UHttpClient_Base::sendRequest(%d,%.*S,%u,%.*S,%u,%.*S,%u)", method, content_type_len, content_type, content_type_len, data_len, data, data_len, uri_len, _uri, uri_len)

   if ( uri_len) (void) UClient_Base::setUrl(_uri, uri_len);
   if (data_len) (void) body.assign(data, data_len);

   method_num = method;

   composeRequest(content_type, content_type_len);

   // send method requested to server and get response

   bool ok = sendRequest();

   // reset reference to request...

   UClient_Base::reset();

    requestHeader->clear();
   responseHeader->clear();

   U_RETURN(ok);
}

bool UHttpClient_Base::sendPost(const UString& _url, const UString& _body, const char* content_type, uint32_t content_type_len)
{
   U_TRACE(0, "UHttpClient_Base::sendPost(%V,%V,%.*S,%u)", _url.rep, _body.rep, content_type_len, content_type, content_type_len)

   if (UClient_Base::connectServer(_url) == false)
      {
      body = UClient_Base::response; // NB: contains the error message...

      U_RETURN(false);
      }

   // send POST(2) request to server and get response

   if (sendRequest(2, content_type, content_type_len, U_STRING_TO_PARAM(_body), 0, 0)) U_RETURN(true);

   U_RETURN(false);
}

bool UHttpClient_Base::upload(const UString& _url, UFile& file, const char* filename, uint32_t filename_len, int method)
{
   U_TRACE(0, "UHttpClient_Base::upload(%V,%.*S,%.*S,%u,%d)", _url.rep, U_FILE_TO_TRACE(file), filename_len, filename, filename_len, method)

   if (UClient_Base::connectServer(_url) == false) body = UClient_Base::response; // NB: it contains the error message...
   else
      {
      UString content = file.getContent();

      uint32_t sz = content.size();

      if (sz)
         {
         U_INTERNAL_ASSERT(UClient_Base::uri)

#     ifdef USE_LIBMAGIC
         if (UMagic::magic == 0) (void) UMagic::init();
#     endif

         /**
          * The PUT method, though not as widely used as the POST method is perhaps the more efficient way of uploading files to a server.
          * This is because in a POST upload the files need to be combined together into a multipart message and this message has to be
          * decoded at the server. In contrast, the PUT method allows you to simply write the contents of the file to the socket connection
          * that is established with the server.
          *
          * When using the POST method, all the files are combined together into a single multipart/form-data type object. This MIME message
          * when transferred to the server, has to be decoded by the server side handler. The decoding process may consume significant amounts
          * of memory and CPU cycles for very large files.
          *
          * The fundamental difference between the POST and PUT requests is reflected in the different meaning of the Request-URI. The URI in a
          * POST request identifies the resource that will handle the enclosed entity. That resource might be a data-accepting process, a gateway
          * to some other protocol, or a separate entity that accepts annotations. In contrast, the URI in a PUT request identifies the entity
          * enclosed with the request
          */

         if (method == 3) // 3 => PUT
            {
            uint32_t nup = (filename_len == 0 ? 0
                                              : U_CONSTANT_SIZE("/upload/"));

            if (nup == 0)
               {
               filename     = UClient_Base::uri.data();
               filename_len = UClient_Base::uri.size();
               }

            (void) last_request.reserve(filename_len + UClient_Base::server.size() + 300U);

            last_request.snprintf(U_CONSTANT_TO_PARAM("PUT %.*s%.*s HTTP/1.1\r\n"
                                  "Host: %v:%u\r\n"
                                  "User-Agent: ULib/1.4.2\r\n"
                                  "Content-Length: %u\r\n"
                                  "Content-Type: %s\r\n"
                                  "\r\n"),
                                  nup, "/upload/",
                                  filename_len, filename,
                                  UClient_Base::server.rep, UClient_Base::port,
                                  sz, file.getMimeType());

            UClient_Base::iov[3].iov_base = (caddr_t)content.data();
            UClient_Base::iov[3].iov_len  = sz;

            parseRequest(4);
            }
         else
            {
            U_INTERNAL_ASSERT_EQUALS(method, 2) // 2 => POST

            if (filename_len == 0)
               {
               filename     = file.getPathRelativ();
               filename_len = file.getPathRelativLen();
               }

            UString _body(filename_len + 300U);

            _body.snprintf(U_CONSTANT_TO_PARAM("------------------------------b34551106891\r\n"
                           "Content-Disposition: form-data; name=\"file\"; filename=\"%.*s\"\r\n"
                           "Content-Type: %s\r\n"
                           "\r\n"),
                           filename_len, filename,
                           file.getMimeType());

            UClient_Base::iov[3].iov_base = (caddr_t)_body.data();
            UClient_Base::iov[3].iov_len  = _body.size();
            UClient_Base::iov[4].iov_base = (caddr_t)content.data();
            UClient_Base::iov[4].iov_len  = sz;
            UClient_Base::iov[5].iov_base = (caddr_t)       "\r\n"
                                                            "------------------------------b34551106891--\r\n";
            UClient_Base::iov[5].iov_len  = U_CONSTANT_SIZE("\r\n"
                                                            "------------------------------b34551106891--\r\n");

            (void) last_request.reserve(UClient_Base::uri.size() + UClient_Base::server.size() + 300U);

            last_request.snprintf(U_CONSTANT_TO_PARAM("POST %v HTTP/1.1\r\n"
                                  "Host: %v:%u\r\n"
                                  "User-Agent: ULib/1.4.2\r\n"
                                  "Content-Length: %u\r\n"
                                  "Content-Type: multipart/form-data; boundary=----------------------------b34551106891\r\n"
                                  "\r\n"),
                                  UClient_Base::uri.rep, UClient_Base::server.rep, UClient_Base::port, _body.size() + sz + UClient_Base::iov[5].iov_len);

            parseRequest(6);
            }

         // send upload request to server and get response

         if (sendRequest()) U_RETURN(true);
         }
      }

   U_RETURN(false);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UHttpClient_Base::dump(bool _reset) const
{
   UClient_Base::dump(false);

   *UObjectIO::os << '\n'
                  << "bproxy                              " << bproxy                << '\n'
                  << "method_num                          " << method_num            << ")\n"
                  << "bFollowRedirects                    " << bFollowRedirects      << '\n'
                  << "body           (UString             " << (void*)&body          << ")\n"
                  << "user           (UString             " << (void*)&user          << ")\n"
                  << "password       (UString             " << (void*)&password      << ")\n"
                  << "setcookie      (UString             " << (void*)&setcookie     << ")\n"
                  << "last_request   (UString             " << (void*)&last_request  << ")\n"
                  << "requestHeader  (UMimeHeader         " << (void*)requestHeader  << ")\n"
                  << "responseHeader (UMimeHeader         " << (void*)responseHeader << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
