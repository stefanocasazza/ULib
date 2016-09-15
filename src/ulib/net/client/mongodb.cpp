// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mongodb.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/net/client/mongodb.h>

bool UMongoDBClient::connect(const char* _uri)
{
   U_TRACE(0, "UMongoDBClient::connect(%S)", _uri)

   client = (mongoc_client_t*) U_SYSCALL(mongoc_client_new, "%S", _uri);

   if (client) U_RETURN(true);

   U_RETURN(false);
}

bool UMongoDBClient::connect(const char* phost, unsigned int _port)
{
   U_TRACE(0, "UMongoDBClient::connect(%S,%u)", phost, _port)

   UString host;

   if (phost) (void) host.assign(phost);
   else
      {
      const char* env_mongodb_host = (const char*) U_SYSCALL(getenv, "%S", "MONGODB_HOST");

      if (env_mongodb_host == 0) U_RETURN(false);

      (void) host.assign(env_mongodb_host, u__strlen(env_mongodb_host, __PRETTY_FUNCTION__));

      const char* env_mongodb_port = (const char*) U_SYSCALL(getenv, "%S", "MONGODB_PORT");

      if (env_mongodb_port) _port = atoi(env_mongodb_port);
      }

   uri.snprintf(U_CONSTANT_TO_PARAM("mongodb://%v:%u"), host.rep, _port ? _port : 27017);

   puri = (mongoc_uri_t*) U_SYSCALL(mongoc_uri_new, "%S", uri.data());

   if (puri)
      {
      client = (mongoc_client_t*) U_SYSCALL(mongoc_client_new_from_uri, "%p", puri);

      if (client) U_RETURN(true);
      }

   U_RETURN(false);
}

bool UMongoDBClient::selectCollection(const char* db, const char* name_collection)
{
   U_TRACE(0, "UMongoDBClient::selectCollection(%S,%S)", db, name_collection)

   U_INTERNAL_ASSERT_POINTER(client)

   if (collection) U_SYSCALL_VOID(mongoc_collection_destroy, "%p", collection);

   collection = (mongoc_collection_t*) U_SYSCALL(mongoc_client_get_collection, "%p,%S,%S", client, db, name_collection);

   if (collection) U_RETURN(true);

   U_RETURN(false);
}

void UMongoDBClient::readFromCursor()
{
   U_TRACE_NO_PARAM(0, "UMongoDBClient::readFromCursor()")

   size_t length;
   const bson_t* doc;
   bson_error_t error;

   vitem.clear();

   while (U_SYSCALL(mongoc_cursor_next, "%p,%p", cursor, &doc))
      {
      char* str = U_SYSCALL(bson_as_json, "%p,%p", doc, &length);

      UString x((const char*)str, length);

      x.rep->_capacity = U_TO_FREE;

      U_INTERNAL_DUMP("x = %V", x.rep);

      vitem.push(x);
      }

   if (U_SYSCALL(mongoc_cursor_error, "%p,%p", cursor, &error)) U_WARNING("mongoc_cursor_error(): %d.%d,%S", error.domain, error.code, error.message);

   U_SYSCALL_VOID(mongoc_cursor_destroy, "%p", cursor);
}

bool UMongoDBClient::find(bson_t* query, bson_t* projection, mongoc_query_flags_t flags, mongoc_read_prefs_t* read_prefs)
{
   U_TRACE(0, "UMongoDBClient::find(%p,%p,%d,%p)", query, projection, flags, read_prefs)

   U_INTERNAL_ASSERT_POINTER(client)
   U_INTERNAL_ASSERT_POINTER(collection)

   /**
    * Parameters
    *
    * collection  A mongoc_collection_t
    * flags       A mongoc_query_flags_t
    * skip        A uint32_t of number of documents to skip or 0
    * limit       A uint32_t of max number of documents to return or 0
    * batch_size  A uint32_t containing batch size of document result sets or 0 for default. Default is 100
    * query       A bson_t containing the query and options to execute
    * fields      A bson_t containing fields to return or NULL
    * read_prefs  A mongoc_read_prefs_t or NULL for default read preferences
    */

   cursor = (mongoc_cursor_t*) U_SYSCALL(mongoc_collection_find, "%p,%d,%u,%u,%u,%p,%p,%p", collection, flags, 0, 0, 0, query, projection, read_prefs);

   if (cursor)
      {
      readFromCursor();

      U_RETURN(true);
      }

   U_RETURN(false);
}

// Execute an 'aggregation' query

bool UMongoDBClient::findAggregation(bson_t* pipeline, bson_t* options, mongoc_query_flags_t flags, mongoc_read_prefs_t* read_prefs)
{
   U_TRACE(0, "UMongoDBClient::findAggregation(%p,%p,%d,%p)", pipeline, options, flags, read_prefs)

   U_INTERNAL_ASSERT_POINTER(client)
   U_INTERNAL_ASSERT_POINTER(collection)

   cursor = (mongoc_cursor_t*) U_SYSCALL(mongoc_collection_aggregate, "%p,%d,%p,%p,%p", collection, flags, pipeline, options, read_prefs);

   if (cursor)
      {
      readFromCursor();

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UMongoDBClient::findOne(const char* json, uint32_t len)
{
   U_TRACE(0, "UMongoDBClient::findOne(%.*S,%u)", len, json, len)

   U_INTERNAL_ASSERT_POINTER(client)
   U_INTERNAL_ASSERT_POINTER(collection)

   bson_error_t error;
   bson_t* bson = (bson_t*) U_SYSCALL(bson_new_from_json, "%p,%u,%p", (const uint8_t*)json, len, &error);

   if (bson)
      {
      bool result = find(bson, 0);

      U_SYSCALL_VOID(bson_destroy, "%p", bson);

      U_RETURN(result);
      }

   U_WARNING("bson_new_from_json(): %d.%d,%S", error.domain, error.code, error.message);

   U_RETURN(false);
}

bool UMongoDBClient::insert(bson_t* doc)
{
   U_TRACE(0, "UMongoDBClient::insert(%p)", doc)

   U_INTERNAL_ASSERT_POINTER(client)
   U_INTERNAL_ASSERT_POINTER(collection)

   bson_error_t error;

   if (U_SYSCALL(mongoc_collection_insert, "%p,%p,%p,%p,%p,%b,%b,%b,%p,%p", collection, MONGOC_INSERT_NONE, doc, 0, &error) == 0)
      {
      U_WARNING("mongoc_collection_insert(): %d.%d,%S", error.domain, error.code, error.message);

      U_RETURN(false);
      }

   U_RETURN(true);
}

bool UMongoDBClient::update(bson_t* query, bson_t* _update)
{
   U_TRACE(0, "UMongoDBClient::update(%p,%p)", query, _update)

   U_INTERNAL_ASSERT_POINTER(client)
   U_INTERNAL_ASSERT_POINTER(collection)

   bson_error_t error;

   if (U_SYSCALL(mongoc_collection_update, "%p,%d,%p,%p,%p,%p", collection, MONGOC_UPDATE_NONE, query, _update, 0, &error) == 0)
      {
      U_WARNING("mongoc_collection_update(): %d.%d,%S", error.domain, error.code, error.message);

      U_RETURN(false);
      }

   U_RETURN(true);
}

bool UMongoDBClient::update(uint32_t old_value, const char* key, uint32_t new_value)
{
   U_TRACE(0, "UMongoDBClient::update(%u,%S,%u)", old_value, key, new_value)

   U_INTERNAL_ASSERT_POINTER(client)
   U_INTERNAL_ASSERT_POINTER(collection)

   bson_t* query   = (bson_t*) U_SYSCALL_NO_PARAM(bson_new);
   bson_t* _update = BCON_NEW ("$set", "{", key, BCON_INT32( new_value ), "}");

   BSON_APPEND_INT32(query, "_id", old_value);

   bool result = update(query, _update);

   U_SYSCALL_VOID(bson_destroy, "%p", query);
   U_SYSCALL_VOID(bson_destroy, "%p", _update);

   U_RETURN(result);
}

bool UMongoDBClient::findAndModify(bson_t* query, bson_t* _update)
{
   U_TRACE(0, "UMongoDBClient::findAndModify(%p,%p)", query, _update)

   U_INTERNAL_ASSERT_POINTER(client)
   U_INTERNAL_ASSERT_POINTER(collection)

   bson_t reply;
   bson_error_t error;

   if (U_SYSCALL(mongoc_collection_find_and_modify, "%p,%p,%p,%p,%p,%b,%b,%b,%p,%p", collection, query, 0, _update, 0, false, false, true, &reply, &error))
      {
      UString x;
      size_t length;
      char* str = U_SYSCALL(bson_as_json, "%p,%p", &reply, &length);

      x.setConstant((const char*)str, length);

      x.rep->_capacity = U_TO_FREE;

      U_INTERNAL_DUMP("x = %V", x.rep);

      vitem.clear();
      vitem.push(x);

      U_RETURN(true);
      }

   U_WARNING("mongoc_collection_find_and_modify(): %d.%d,%S", error.domain, error.code, error.message);

   U_RETURN(false);
}

// BULK

void UMongoDBClient::updateOneBulk(mongoc_bulk_operation_t* bulk, uint32_t old_value, const char* key, uint32_t new_value)
{
   U_TRACE(0, "UMongoDBClient::updateOneBulk(%p,%u,%S,%u)", bulk, old_value, key, new_value)

   U_INTERNAL_ASSERT_POINTER(client)
   U_INTERNAL_ASSERT_POINTER(collection)

   bson_t* query   = (bson_t*) U_SYSCALL_NO_PARAM(bson_new);
   bson_t* _update = BCON_NEW ("$set", "{", key, BCON_INT32( new_value ), "}");

   BSON_APPEND_INT32(query, "_id", old_value);

   U_SYSCALL_VOID(mongoc_bulk_operation_update_one, "%p,%p,%p,%b", bulk, query, _update, false);

   U_SYSCALL_VOID(bson_destroy, "%p", query);
   U_SYSCALL_VOID(bson_destroy, "%p", _update);
}

bool UMongoDBClient::executeBulk(mongoc_bulk_operation_t* bulk)
{
   U_TRACE(0, "UMongoDBClient::executeBulk(%p)", bulk)

   U_INTERNAL_ASSERT_POINTER(client)
   U_INTERNAL_ASSERT_POINTER(collection)

   bool ok;
   bson_t reply;
   bson_error_t error;

   if ((ok = (U_SYSCALL(mongoc_bulk_operation_execute, "%p,%p,%p", bulk, &reply, &error) != 0)))
      {
      UString x;
      size_t length;
      char* str = U_SYSCALL(bson_as_json, "%p,%p", &reply, &length);

      x.setConstant((const char*)str, length);

      x.rep->_capacity = U_TO_FREE;

      U_INTERNAL_DUMP("x = %V", x.rep);

      vitem.clear();
      vitem.push(x);
      }

   U_SYSCALL_VOID(bson_destroy, "%p", &reply);
   U_SYSCALL_VOID(mongoc_bulk_operation_destroy, "%p", bulk);

   if (ok) U_RETURN(true);

   U_WARNING("mongoc_bulk_operation_execute(): %d.%d,%S", error.domain, error.code, error.message);

   U_RETURN(false);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UMongoDBClient::dump(bool _reset) const
{
   *UObjectIO::os << "puri           " << (void*)puri       << '\n'
                  << "client         " << (void*)client     << '\n'
                  << "collection     " << (void*)collection << '\n'
                  << "uri   (UString " << (void*)&uri       << ")\n"
                  << "vitem (UVector " << (void*)&vitem     << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
