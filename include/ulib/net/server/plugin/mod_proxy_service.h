// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_proxy_service.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_PROXY_SERVICE_H
#define U_MOD_PROXY_SERVICE_H 1

#include <ulib/internal/common.h>

#ifdef USE_LIBPCRE
#  include <ulib/pcre/pcre.h>
#endif

class UHTTP;
class UCommand;
class UFileConfig;

class U_EXPORT UModProxyService {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   enum Error {
      INTERNAL_ERROR            = 1, // NB: we need to start from 1 because we use a vector...
      BAD_REQUEST               = 2,
      NOT_FOUND                 = 3,
      FORBIDDEN                 = 4,
      ERROR_PARSE_REQUEST       = 5, // user case start...
      ERROR_A_BAD_HEADER        = 6,
      ERROR_A_X509_MISSING_CRT  = 7,
      ERROR_A_X509_REJECTED     = 8,
      ERROR_A_X509_TAMPERED     = 9,
      ERROR_A_X509_NOBASICAUTH  = 10
   };

    UModProxyService();
   ~UModProxyService();

   int     getPort() const           { return port; }
   UString getUser() const           { return user; }
   UString getServer() const;
   UString getPassword() const       { return password; }

   bool isWebSocket() const          { return websocket; }
   bool isReplaceResponse() const    { return (vreplace_response.empty() == false); }
   bool isFollowRedirects() const    { return follow_redirects; }
   bool isResponseForClient() const  { return response_client; }
   bool isRequestCertificate() const { return request_cert; }

   bool isAuthorization() const
      {
      U_TRACE_NO_PARAM(0, "UModProxyService::isAuthorization()")

      if (    user.empty() == false &&
          password.empty() == false)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   UString replaceResponse(const UString& msg);

   // SERVICES

   UCommand* command;
   UString environment;

   static void setMsgError(int err);
   static bool loadConfig(UFileConfig& cfg);
   static bool setServerAddress(const UString& dir, const char* addr, uint32_t addr_len);

   static UModProxyService* findService() __pure;
   static UModProxyService* findService(const char* host, uint32_t host_len, const char* uri, uint32_t uri_len) __pure;

   static UModProxyService* findService(const UString& host, const UString& uri) { return findService(U_STRING_TO_PARAM(host), U_STRING_TO_PARAM(uri)); }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UVector<UString> vreplace_response;
   UVector<UIPAllow*>* vremote_address;
   UString host_mask, server, user, password;
#ifdef USE_LIBPCRE
   UPCRE   uri_mask;
#else
   UString uri_mask;
#endif
   int port, method_mask;
   bool request_cert, follow_redirects, response_client, websocket;

private:
   U_DISALLOW_ASSIGN(UModProxyService)

   friend class UHTTP;
};

#endif
