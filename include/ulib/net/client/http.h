// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    http.h - client HTTP
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_HTTP_CLIENT_H
#define U_HTTP_CLIENT_H 1

#include <ulib/mime/entity.h>
#include <ulib/net/client/client.h>

/**
 * @class UHttpClient
 * 
 * @brief Creates and manages a client connection with a HTTP server.
 *
 * <p>HTTP stands for Hyper Text Transfer Protocol, which is defined by
 * RFCs published by the <a href="http://www.ietf.org/">Internet Engineering
 * Task Force</a>.</p>
 *
 * A UHttpClient instance can make only one HTTP request (although
 * this one request may involve several message exchanges if HTTP redirection
 * or authorization is involved). However, HTTP 1.1 introduces
 * persistent connections which can make more effective use of TCP/IP.
 */

class Application;
class WiAuthNodog;
class UServer_Base;
class UProxyPlugIn;
class UNoCatPlugIn;
class USOAPClient_Base;

class U_EXPORT UHttpClient_Base : public UClient_Base {
public:

   void reset();

   // Returns a modifiable sequence of MIME-type headers that will be used to form a request to the HTTP server

   UMimeHeader* getRequestHeader() { return requestHeader; }

   /**
    * Sets a request MIME header value. If a MIME header with the specified
    * name already exists, its value is replaced with the supplied value
    * 
    * @param name the name by which the property is known
    * @param value the value to be associated with the named property
    */

   void setHeader(const UString& name, const UString& value) { requestHeader->setHeader(name, value); }

   void setHeaderHostPort(const UString& h)  { setHeader(*UString::str_host, h); }
   void setHeaderUserAgent(const UString& u) { setHeader(*UString::str_user_agent, u); }

   void removeHeader(const UString& name) { requestHeader->removeHeader(name); }

   // Returns the MIME header that were received in response from the HTTP server

   UMimeHeader* getResponseHeader() { return responseHeader; }

   /**
    * Sets a flag indicating if HTTP redirects will be followed.
    * 
    * @param bFollow @c true for redirect requests to be followed (the default);
    *                @c false prevents this behaviour
    */

   bool getFollowRedirects() const                              { return bFollowRedirects; }
   void setFollowRedirects(bool bFollow, bool _bproxy = false)  { bFollowRedirects = bFollow; bproxy = _bproxy; }

   // In response to a HTTP_UNAUTHORISED response from the HTTP server,
   // obtain a userid and password for the scheme/realm returned from the HTTP server

   bool isPasswordAuthentication() const
      {
      U_TRACE(0, "UHttpClient_Base::isPasswordAuthentication()")

      bool result = (user && password);

      U_RETURN(result);
      }

   void setRequestPasswordAuthentication(const UString& _user, const UString& _password)
      {
      U_TRACE(0, "UHttpClient_Base::setRequestPasswordAuthentication(%.*S,%.*S)", U_STRING_TO_TRACE(_user), U_STRING_TO_TRACE(_password))

      user     = _user;
      password = _password;
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
   // We do not process Location headers when accompanying a 200 OK response.
   //=============================================================================

   bool sendRequest();
   bool sendRequest(const UString& req);

   // ASYNC MODE (it creates a copy of itself, return pid child if parent)

   int sendRequestAsync(const UString& url, bool bqueue = true, const char* log_msg = 0, int log_fd = -1);

   // QUEUE MODE

   bool putRequestOnQueue();

   bool upload(const UString& url, UFile& file, const char* filename = 0, uint32_t filename_len = 0);
   bool sendPost(const UString& url, const UString& pbody, const char* content_type = "application/x-www-form-urlencoded");

   UString getContent() const     { return body; }
   UString getSetCookie() const   { return setcookie; }
   UString getLastRequest() const { return last_request; }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const;
#endif

protected:
   UMimeHeader* requestHeader;
   UMimeHeader* responseHeader;
   UString method, body, user, password, setcookie, last_request;
   bool bFollowRedirects, bproxy;

   static struct uhttpinfo u_http_info_save;

   void parseRequest();
   void composeRequest();
   bool sendRequestEngine();

   // Add the MIME-type headers to the request for HTTP server

   static UString wrapRequest(UString* req, const UString& host_port,
                              const char* method,    uint32_t method_len,
                              const char* uri,       uint32_t    uri_len,
                              const char* extension, const char* content_type = 0);

   // In response to a HTTP_UNAUTHORISED response from the HTTP server, this function will attempt
   // to generate an Authentication header to satisfy the server.

   bool createAuthorizationHeader();
   int  checkResponse(int& redirectCount);

   // COSTRUTTORI

   UHttpClient_Base(UFileConfig* _cfg) : UClient_Base(_cfg)
      {
      U_TRACE_REGISTER_OBJECT(0, UHttpClient_Base, "%p", _cfg)

      requestHeader  = U_NEW(UMimeHeader);
      responseHeader = U_NEW(UMimeHeader);

      bproxy           = false;
      bFollowRedirects = true;
      }

   ~UHttpClient_Base()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UHttpClient_Base)

      delete requestHeader;
      delete responseHeader;
      }

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UHttpClient_Base(const UHttpClient_Base&) = delete;
   UHttpClient_Base& operator=(const UHttpClient_Base&) = delete;
#else
   UHttpClient_Base(const UHttpClient_Base&) : UClient_Base(0) {}
   UHttpClient_Base& operator=(const UHttpClient_Base&)        { return *this; }
#endif

   friend class USSLSocket;
   friend class Application;
   friend class WiAuthNodog;
   friend class UServer_Base;
   friend class UProxyPlugIn;
   friend class UNoCatPlugIn;
   friend class USOAPClient_Base;
};

template <class Socket> class U_EXPORT UHttpClient : public UHttpClient_Base {
public:

   // COSTRUTTORI

   UHttpClient(UFileConfig* _cfg) : UHttpClient_Base(_cfg)
      {
      U_TRACE_REGISTER_OBJECT(0, UHttpClient, "%p", _cfg)

      UClient_Base::socket = U_NEW(Socket(UClient_Base::bIPv6));
      }

   ~UHttpClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UHttpClient)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return UHttpClient_Base::dump(_reset); }
#endif

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UHttpClient(const UHttpClient&) = delete;
   UHttpClient& operator=(const UHttpClient&) = delete;
#else
   UHttpClient(const UHttpClient&) : UHttpClient_Base(0) {}
   UHttpClient& operator=(const UHttpClient&)            { return *this; }
#endif
};

#ifdef USE_LIBSSL // specializzazione con USSLSocket

template <> class U_EXPORT UHttpClient<USSLSocket> : public UHttpClient_Base {
public:

   UHttpClient(UFileConfig* _cfg) : UHttpClient_Base(_cfg)
      {
      U_TRACE_REGISTER_OBJECT(0, UHttpClient<USSLSocket>, "%p", _cfg)

      UClient_Base::setSSLContext();
      }

   ~UHttpClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UHttpClient<USSLSocket>)
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool _reset) const { return UHttpClient_Base::dump(_reset); }
#endif

protected:

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UHttpClient<USSLSocket>(const UHttpClient<USSLSocket>&) = delete;
   UHttpClient<USSLSocket>& operator=(const UHttpClient<USSLSocket>&) = delete;
#else
   UHttpClient<USSLSocket>(const UHttpClient<USSLSocket>&) : UHttpClient_Base(0) {}
   UHttpClient<USSLSocket>& operator=(const UHttpClient<USSLSocket>&)            { return *this; }
#endif
};

#endif
#endif
