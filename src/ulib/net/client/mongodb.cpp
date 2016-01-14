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

   uri.snprintf("mongodb://%v:%u", host.rep, _port);

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

bool UMongoDBClient::insert(bson_t* doc)
{
   U_TRACE(0, "UMongoDBClient::insert(%p)", doc)

   U_INTERNAL_ASSERT_POINTER(client)
   U_INTERNAL_ASSERT_POINTER(collection)

   bson_error_t error;

   if (U_SYSCALL(mongoc_collection_insert, "%p,%p,%p,%p,%p,%b,%b,%b,%p,%p", collection, MONGOC_INSERT_NONE, doc, 0, &error)) U_RETURN(true);

   U_WARNING("mongoc_collection_insert(): %", error.message);

   U_RETURN(false);
}

bool UMongoDBClient::find(bson_t* query)
{
   U_TRACE(0, "UMongoDBClient::find(%p)", query)

   U_INTERNAL_ASSERT_POINTER(client)
   U_INTERNAL_ASSERT_POINTER(collection)

   cursor = (mongoc_cursor_t*) U_SYSCALL(mongoc_collection_find, "%p,%d,%u,%u,%u,%p,%p,%p", collection, MONGOC_QUERY_NONE, 0, 0, 0, query, 0, 0);

   if (cursor)
      {
      char* str;
      size_t length;
      const bson_t* doc;
      bson_error_t error;

#  ifdef DEBUG
      str = bson_as_json(query, &length);

      U_INTERNAL_DUMP("query = %.*S", length, str);

      bson_free(str);
#  endif

      vitem.clear();

      while (U_SYSCALL(mongoc_cursor_next, "%p,%p", cursor, &doc))
         {
         str = U_SYSCALL(bson_as_json, "%p,%p", doc, &length);

         UString x((const char*)str, length);

         x.rep->_capacity = U_TO_FREE;

         U_INTERNAL_DUMP("x = %V", x.rep);

         vitem.push(x);
         }

      if (U_SYSCALL(mongoc_cursor_error, "%p,%p", cursor, &error)) U_WARNING("mongoc_cursor_error(): %S", error.message);

      U_SYSCALL_VOID(mongoc_cursor_destroy, "%p", cursor);

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

   if (bson == 0)
      {
      U_WARNING("bson_new_from_json(): %S", error.message);
      }
   else
      {
      bool result = find(bson);

      U_SYSCALL_VOID(bson_destroy, "%p", bson);

      U_RETURN(result);
      }

   U_RETURN(false);
}

bool UMongoDBClient::update(bson_t* query, bson_t* _update)
{
   U_TRACE(0, "UMongoDBClient::update(%p,%p)", query, _update)

   U_INTERNAL_ASSERT_POINTER(client)
   U_INTERNAL_ASSERT_POINTER(collection)

   bson_error_t error;

   if (U_SYSCALL(mongoc_collection_update, "%p,%d,%p,%p,%p,%p", collection, MONGOC_UPDATE_NONE, query, _update, 0, &error) == false)
      {
      U_WARNING("mongoc_collection_update(): %S", error.message);

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

bool UMongoDBClient::findAndModify(bson_t* query, bson_t* update)
{
   U_TRACE(0, "UMongoDBClient::findAndModify(%p,%p)", query, update)

   U_INTERNAL_ASSERT_POINTER(client)
   U_INTERNAL_ASSERT_POINTER(collection)

   bson_t reply;
   bson_error_t error;

   if (U_SYSCALL(mongoc_collection_find_and_modify, "%p,%p,%p,%p,%p,%b,%b,%b,%p,%p", collection, query, 0, update, 0, false, false, true, &reply, &error) == false)
      {
      U_WARNING("mongoc_collection_find_and_modify(): %S", error.message);
      }
   else
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
