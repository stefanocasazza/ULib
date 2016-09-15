// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    elasticsearch.h - simple ElasticSearch client 
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_ELASTICSEARCH_H
#define ULIB_ELASTICSEARCH_H 1

#include <ulib/json/value.h>
#include <ulib/net/tcpsocket.h>
#include <ulib/net/client/http.h>

/**
 * @class UElasticSearchClient
 *
 * @brief UElasticSearchClient is a wrapper to ElasticSearch server API
 */

class U_EXPORT UElasticSearchClient {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UElasticSearchClient() : uri(U_CAPACITY)
      {
      U_TRACE_REGISTER_OBJECT(0, UElasticSearchClient, "", 0)

      client = 0;
      }

   ~UElasticSearchClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UElasticSearchClient)

      if (client) delete client;
      }

   // Connect to ElasticSearch server

   bool connect(const char* host = 0, unsigned int _port = 9200);

   // SERVICES

   UString getContent() const                 { return client->body; }
   bool    parseResponse(UValue* pjson) const { return pjson->parse(getContent()); }

   UHttpClient<UTCPSocket>* getClient() const { return client; }

   // Search API of ES. Specify the doc type

   bool search(const char* _index, uint32_t index_len, const char* type, uint32_t type_len, const char* query, uint32_t query_len)
      {
      U_TRACE(0, "UElasticSearchClient::search(%.*S,%u,%.*S,%u,%.*S,%u)", index_len, _index, index_len, type_len, type, type_len, query_len, query, query_len)

      U_INTERNAL_ASSERT_POINTER(client)
      U_INTERNAL_ASSERT_MAJOR(query_len, 0)

      uri.snprintf(U_CONSTANT_TO_PARAM("/%.*s/%.*s/_search"), index_len, _index, type_len, type);

      // send POST(2) request to server and get response

      if (client->sendRequest(2, U_CONSTANT_TO_PARAM("application/json"), query, query_len, U_STRING_TO_PARAM(uri))) U_RETURN(true);

      U_RETURN(false);
      }

   bool search(const UString& _index, const UString& type, const UString& query)
      { return search(U_STRING_TO_PARAM(_index), U_STRING_TO_PARAM(type), U_STRING_TO_PARAM(query)); }

   // Index a document with maybe automatic id creation

   bool index(const char* _index, uint32_t index_len, const char* type, uint32_t type_len, const char* id, uint32_t id_len, const char* data, uint32_t data_len)
      {
      U_TRACE(0, "UElasticSearchClient::index(%.*S,%u,%.*S,%u,%.*S,%u,%.*S,%u)", index_len, _index, index_len, type_len, type, type_len,
                                                                                 id_len, id, id_len, data_len, data, data_len)

      U_INTERNAL_ASSERT_POINTER(data)
      U_INTERNAL_ASSERT_POINTER(client)
      U_INTERNAL_ASSERT_MAJOR(data_len, 0)

      uri.snprintf(U_CONSTANT_TO_PARAM("/%.*s/%.*s/%.*s"), index_len, _index, type_len, type, id_len, id);

      // send PUT(3) request to server and get response

      if (client->sendRequest(3, U_CONSTANT_TO_PARAM("application/json"), data, data_len, U_STRING_TO_PARAM(uri))) U_RETURN(true);

      U_RETURN(false);
      }

   bool index(const UString& _index, const UString& type, const UString& id, const UString& data)
      { return index(U_STRING_TO_PARAM(_index), U_STRING_TO_PARAM(type), U_STRING_TO_PARAM(id), U_STRING_TO_PARAM(data)); }

   // Request the document by index/type/id

   bool getDocument(const char* _index, uint32_t index_len, const char* type, uint32_t type_len, const char* id, uint32_t id_len)
      {
      U_TRACE(0, "UElasticSearchClient::getDocument(%.*S,%u,%.*S,%u,%.*S,%u)", index_len, _index, index_len, type_len, type, type_len, id_len, id, id_len)

      U_INTERNAL_ASSERT_POINTER(client)

      uri.snprintf(U_CONSTANT_TO_PARAM("/%.*s/%.*s/%.*s"), index_len, _index, type_len, type, id_len, id);

      // send GET(0) request to server and get response

      if (client->sendRequest(0, U_CONSTANT_TO_PARAM("application/json"), 0, 0, U_STRING_TO_PARAM(uri))) U_RETURN(true);

      U_RETURN(false);
      }

   bool getDocument(const UString& _index, const UString& type, const UString& id) { return getDocument(U_STRING_TO_PARAM(_index), U_STRING_TO_PARAM(type), U_STRING_TO_PARAM(id)); }

   // Request the document by query key:value

   bool getDocument(const char* _index, uint32_t index_len, const char* type, uint32_t type_len, const char* key, uint32_t key_len, const char* value, uint32_t value_len)
      {
      U_TRACE(0, "UElasticSearchClient::getDocument(%.*S,%u,%.*S,%u,%.*S,%u,%.*S,%u)", index_len, _index, index_len, type_len, type, type_len,
                                                                                       key_len, key, key_len, value_len, value, value_len)

      U_INTERNAL_ASSERT_POINTER(client)

      uri.snprintf(U_CONSTANT_TO_PARAM("/%.*s/%.*s/_search"), index_len, _index, type_len, type);

      UString query(100U + key_len + value_len);

      query.snprintf(U_CONSTANT_TO_PARAM("{\"query\":{\"match\":{\"%.*s\":\"%.*s\"}}}"), key_len, key, value_len, value);

      return sendPOST(uri, query);
      }

   bool getDocument(const UString& _index, const UString& type, const UString& key, const UString& value)
      { return getDocument(U_STRING_TO_PARAM(_index), U_STRING_TO_PARAM(type), U_STRING_TO_PARAM(key), U_STRING_TO_PARAM(value)); }

   // Update a document field

   bool update(const char* _index, uint32_t index_len, const char* type, uint32_t type_len,
               const char* id,     uint32_t    id_len, const char* key,  uint32_t  key_len, const char* value, uint32_t value_len)
      {
      U_TRACE(0, "UElasticSearchClient::update(%.*S,%u,%.*S,%u,%.*S,%u,%.*S,%u,%.*S,%u)",
               index_len, _index, index_len, type_len, type, type_len, id_len, id, id_len, key_len, key, key_len, value_len, value, value_len)

      uri.snprintf(U_CONSTANT_TO_PARAM("/%.*s/%.*s/%.*s/_update"), index_len, _index, type_len, type, id_len, id);

      UString data(100U + key_len + value_len);

      data.snprintf(U_CONSTANT_TO_PARAM("{\"doc\":{\"%.*s\":\"%.*s\"}}"), key_len, key, value_len, value);

      return sendPOST(uri, data);
      }

   bool update(const UString& _index, const UString& type, const UString& id, const UString& key, const UString& value)
      { return update(U_STRING_TO_PARAM(_index), U_STRING_TO_PARAM(type), U_STRING_TO_PARAM(id), U_STRING_TO_PARAM(key), U_STRING_TO_PARAM(value)); }

   // Delete the document by index/type/id

   bool deleteDocument(const char* _index, uint32_t index_len, const char* type, uint32_t type_len, const char* id, uint32_t id_len)
      {
      U_TRACE(0, "UElasticSearchClient::deleteDocument(%.*S,%u,%.*S,%u,%.*S,%u)", index_len, _index, index_len, type_len, type, type_len, id_len, id, id_len)

      U_INTERNAL_ASSERT_POINTER(client)

      uri.snprintf(U_CONSTANT_TO_PARAM("/%.*s/%.*s/%.*s"), index_len, _index, type_len, type, id_len, id);

      // send DELETE(4) request to server and get response

      if (client->sendRequest(4, U_CONSTANT_TO_PARAM("application/json"), 0, 0, U_STRING_TO_PARAM(uri))) U_RETURN(true);

      U_RETURN(false);
      }

   bool deleteDocument(const UString& _index, const UString& type, const UString& id)
      { return deleteDocument(U_STRING_TO_PARAM(_index), U_STRING_TO_PARAM(type), U_STRING_TO_PARAM(id)); }

   // generic POST

   bool sendPOST(const char* _uri, uint32_t uri_len, const char* data, uint32_t data_len)
      {
      U_TRACE(0, "UElasticSearchClient::sendPOST(%.*S,%u,%.*S,%u)", uri_len, _uri, uri_len, data_len, data, data_len)

      U_INTERNAL_ASSERT_POINTER(client)

      // send POST(2) request to server and get response

      if (client->sendRequest(2, U_CONSTANT_TO_PARAM("application/json"), data, data_len, _uri, uri_len)) U_RETURN(true);

      U_RETURN(false);
      }

   bool sendPOST(const UString& _uri, const UString& data) { return sendPOST(U_STRING_TO_PARAM(_uri), U_STRING_TO_PARAM(data)); }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString uri;
   UHttpClient<UTCPSocket>* client;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UElasticSearchClient)
};
#endif
