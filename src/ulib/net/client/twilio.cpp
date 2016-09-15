// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    twilio.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/url.h>
#include <ulib/net/client/twilio.h>

bool UTwilioClient::sendRequest(int method, const char* path, uint32_t path_len, const UString& data)
{
   U_TRACE(0, "UTwilioClient::sendRequest(%d,%.*S,%u,%V)", method, path_len, path, path_len, data.rep)

   U_INTERNAL_ASSERT_POINTER(client)

   uri.snprintf(U_CONSTANT_TO_PARAM(TWILIO_API_URL "/" TWILIO_API_VERSION "/Accounts/%.*s/%.*s"), U_STRING_TO_TRACE(client->user), path_len, path);

   if (method == 4) // DELETE
      {
      // send DELETE(4) request to server and get response

      if (client->sendRequest(4, 0, 0, 0, 0, U_STRING_TO_PARAM(uri))) U_RETURN(true);
      }
   else if (method == 3) // PUT
      {
      UFile file(data);

      // upload file to server and get response

      if (client->upload(uri, file, 0, 0, 3)) U_RETURN(true);
      }
   else
      {
      UVector<UString> name_value;

      if (name_value.split(data))
         {
         if (method == 0) // GET
            {
            Url url(uri);

            if (url.setQuery(name_value))
               {
               UString x = url.get();

               // send GET(0) request to server and get response

               if (client->sendRequest(0, 0, 0, 0, 0, U_STRING_TO_PARAM(x))) U_RETURN(true);
               }
            }
         else if (method == 2) // POST
            {
            UString body = Url::getQueryBody(name_value);

            // send POST(2) request to server and get response

            if (client->sendRequest(2, U_CONSTANT_TO_PARAM("application/x-www-form-urlencoded"), U_STRING_TO_PARAM(body), U_STRING_TO_PARAM(uri))) U_RETURN(true);
            }
         }
      }

   U_RETURN(false);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UTwilioClient::dump(bool _reset) const
{
   *UObjectIO::os << "uri    (UString                 " << (void*)&uri   << ")\n"
                  << "client (UHttpClient<USSLSocket> " << (void*)client << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
