// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mongodb.h - Simple MongoDB client 
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_MONGODB_H
#define ULIB_MONGODB_H 1

#include <ulib/container/vector.h>

#ifndef USE_MONGODB
typedef int bson_t;
#else
#  include <mongoc.h>
#endif

/**
 * @class UMongoDBClient
 *
 * @brief UMongoDBClient is a wrapper to MongoDB client API
 */

class U_EXPORT UMongoDBClient {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

    UMongoDBClient() : uri(100)
      {
      U_TRACE_REGISTER_OBJECT(0, UMongoDBClient, "", 0)

#  ifdef USE_MONGODB
      puri = 0;
      client = 0;
      collection = 0;

      U_SYSCALL_VOID_NO_PARAM(mongoc_init);
#  endif
      }

   ~UMongoDBClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UMongoDBClient)

#  ifdef USE_MONGODB
      if (puri)       U_SYSCALL_VOID(mongoc_uri_destroy, "%p", puri);
      if (client)     U_SYSCALL_VOID(mongoc_client_destroy, "%p", client);
      if (collection) U_SYSCALL_VOID(mongoc_collection_destroy, "%p", collection);

      U_SYSCALL_VOID_NO_PARAM(mongoc_cleanup);
#  endif
      }

   // SERVICES

   UVector<UString> vitem;

   bool findOne(uint32_t value)
      {
      U_TRACE(0, "UMongoDBClient::findOne(%u)", value)

#  ifndef USE_MONGODB
      U_RETURN(false);
#  else
      U_INTERNAL_ASSERT_POINTER(client)
      U_INTERNAL_ASSERT_POINTER(collection)

      bson_t* query = (bson_t*) U_SYSCALL_NO_PARAM(bson_new);  

      BSON_APPEND_INT32(query, "_id", value);

      bool result = find(query);

      U_SYSCALL_VOID(bson_destroy, "%p", query);

      U_RETURN(result);
#  endif
      }

   bool findAll()
      {
      U_TRACE(0, "UMongoDBClient::findAll()")

#  ifndef USE_MONGODB
      U_RETURN(false);
#  else
      U_INTERNAL_ASSERT_POINTER(client)
      U_INTERNAL_ASSERT_POINTER(collection)

      bson_t* query = (bson_t*) U_SYSCALL_NO_PARAM(bson_new);  

      bool result = find(query);

      U_SYSCALL_VOID(bson_destroy, "%p", query);

      U_RETURN(result);
#  endif
      }

#ifndef USE_MONGODB
   bool connect(const char* host = 0, unsigned int _port = 27017) { return false; }
   bool selectCollection(const char* db, const char* name_collection) { return false; }
   bool update(uint32_t old_value, const char* key, uint32_t new_value) { return false; }
# if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return ""; }
# endif
#else
   bool insert(bson_t* doc);

   bool findOne(const char* json, uint32_t len);

   bool        update(bson_t* query, bson_t* update);
   bool findAndModify(bson_t* query, bson_t* update);

   bool selectCollection(const char* db, const char* name_collection);

   bool update(uint32_t old_value, const char* key, uint32_t new_value);

   bool connect(const char* host = 0, unsigned int _port = 27017); // connect to MongoDB server

# if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
# endif
#endif

protected:
   UString uri;
#ifdef USE_MONGODB
   mongoc_uri_t* puri;
   mongoc_client_t* client;
   mongoc_cursor_t* cursor;
   mongoc_collection_t* collection;

   bool find(bson_t* query);
#endif

private:
#ifdef U_COMPILER_DELETE_MEMBERS
   UMongoDBClient(const UMongoDBClient&) = delete;
   UMongoDBClient& operator=(const UMongoDBClient&) = delete;
#else
   UMongoDBClient(const UMongoDBClient&) {}
   UMongoDBClient& operator=(const UMongoDBClient&) { return *this; }
#endif
};

#endif
