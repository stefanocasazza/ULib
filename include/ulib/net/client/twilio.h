// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    twilio.h - simple Twilio client
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_TWILIO_H
#define ULIB_TWILIO_H 1

#include <ulib/json/value.h>
#include <ulib/net/client/http.h>
#include <ulib/ssl/net/sslsocket.h>

/*
#define TWILIO_LOOKUPS_URL       "https://lookups.twilio.com"
#define TWILIO_PRICING_URL       "https://pricing.twilio.com"
#define TWILIO_MONITOR_URL       "https://monitor.twilio.com"
#define TWILIO_TRUNKING_URL      "https://trunking.twilio.com"
#define TWILIO_TASKROUTER_URL    "https://taskrouter.twilio.com"
#define TWILIO_IP_MESSAGING_URL  "https://ip-messaging.twilio.com"
*/

#ifndef TWILIO_API_URL
#define TWILIO_API_URL "https://api.twilio.com"
#endif

#ifndef TWILIO_API_VERSION
#define TWILIO_API_VERSION "2010-04-01"
#endif

/**
 * @class UTwilioClient
 *
 * @brief UTwilioClient is a wrapper to Twilio API
 *
 * @c https://twilio-php.readthedocs.org/en/latest
 */

class U_EXPORT UTwilioClient {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   /**
    * Constructor
    *
    * @param string sid:   Your Account SID
    * @param string token: Your Auth Token from `your dashboard <https://www.twilio.com/user/account>`
    */

   UTwilioClient(const UString& sid, const UString& token) : uri(U_CAPACITY)
      {
      U_TRACE_REGISTER_OBJECT(0, UTwilioClient, "%V,%V", sid.rep, token.rep)

      U_NEW(UHttpClient<USSLSocket>, client, UHttpClient<USSLSocket>(0));

      client->setRequestPasswordAuthentication(sid, token);
      }

   ~UTwilioClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTwilioClient)

      U_INTERNAL_ASSERT_POINTER(client)

      delete client;
      }

   // SERVICES

   UString getContent() const                 { return client->body; }
   bool    parseResponse(UValue* pjson) const { return pjson->parse(getContent()); }

   UHttpClient<USSLSocket>* getClient() const { return client; }

   // make a call

   bool makeCall(const char* to, uint32_t to_len, const char* from, uint32_t from_len, const char* url, uint32_t url_len)
      {
      U_TRACE(0, "UTwilioClient::makeCall(%.*S,%u,%.*S,%u,%.*S,%u)", to_len, to, to_len, from_len, from, from_len, url_len, url, url_len)

      U_INTERNAL_ASSERT_POINTER(client)

      UString buffer(U_CAPACITY);

      buffer.snprintf(U_CONSTANT_TO_PARAM("To %.*s\nFrom %.*s\nUrl %.*s"), to_len, to, from_len, from, url_len, url);

      return sendRequest(2, U_CONSTANT_TO_PARAM("Calls"), buffer); 
      }

   // get completed calls XML

   bool getCompletedCalls()
      {
      U_TRACE_NO_PARAM(0, "UTwilioClient::getCompletedCalls()")

      U_INTERNAL_ASSERT_POINTER(client)

      return sendRequest(0, U_CONSTANT_TO_PARAM("Calls"), U_STRING_FROM_CONSTANT("Status Completed")); 
      }

   // send SMS

   bool sendSMS(const char* to, uint32_t to_len, const char* from, uint32_t from_len, const char* body, uint32_t body_len)
      {
      U_TRACE(0, "UTwilioClient::sendSMS(%.*S,%u,%.*S,%u,%.*S,%u)", to_len, to, to_len, from_len, from, from_len, body_len, body, body_len)

      U_INTERNAL_ASSERT_POINTER(client)

      UString buffer(U_CAPACITY);

      buffer.snprintf(U_CONSTANT_TO_PARAM("To %.*s\nFrom %.*s\nBody %.*s"), to_len, to, from_len, from, body_len, body);

      return sendRequest(2, U_CONSTANT_TO_PARAM("SMS/Messages"), buffer); 
      }

   // generic request

   bool sendRequest(int method, const char* path, uint32_t path_len, const UString& data);

   bool sendRequest(int method, const UString& path, const UString& data) { return sendRequest(method, U_STRING_TO_PARAM(path), data); }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString uri;
   UHttpClient<USSLSocket>* client;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UTwilioClient)
};
#endif
