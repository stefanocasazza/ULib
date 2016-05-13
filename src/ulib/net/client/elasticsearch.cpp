// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    elasticsearch.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/client/elasticsearch.h>

// Connect to ElasticSearch server

bool UElasticSearchClient::connect(const char* phost, unsigned int _port)
{
   U_TRACE(0, "UElasticSearchClient::connect(%S,%u)", phost, _port)

   UString host;

   if (phost) (void) host.assign(phost);
   else
      {
      const char* env_elasticsearch_host = (const char*) U_SYSCALL(getenv, "%S", "ELASTICSEARCH_HOST");

      if (env_elasticsearch_host == 0) U_RETURN(false);

      (void) host.assign(env_elasticsearch_host, u__strlen(env_elasticsearch_host, __PRETTY_FUNCTION__));

      const char* env_elasticsearch_port = (const char*) U_SYSCALL(getenv, "%S", "ELASTICSEARCH_PORT");

      if (env_elasticsearch_port) _port = atoi(env_elasticsearch_port);
      }

   if (client == 0) U_NEW(UHttpClient<UTCPSocket>, client, UHttpClient<UTCPSocket>(0));

   if (client->setHostPort(host, _port) &&
       client->connect())
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UElasticSearchClient::dump(bool _reset) const
{
   *UObjectIO::os << "uri    (UString                 " << (void*)&uri   << ")\n"
                  << "client (UHttpClient<UTCPSocket> " << (void*)client << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
