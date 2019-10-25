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
typedef int mongoc_write_concern_t;
typedef int mongoc_bulk_operation_t;

extern "C" {
extern U_EXPORT void bson_destroy(bson_t* bson);
       U_EXPORT void bson_destroy(bson_t* bson) {}
extern U_EXPORT bson_t* bson_new();
       U_EXPORT bson_t* bson_new() { return U_NULLPTR; }
};
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
      U_TRACE_CTOR(0, UMongoDBClient, "")

#  ifdef USE_MONGODB
      puri = U_NULLPTR;
      client = U_NULLPTR;
      collection = U_NULLPTR;

      U_SYSCALL_VOID_NO_PARAM(mongoc_init);
#  endif
      }

   ~UMongoDBClient()
      {
      U_TRACE_DTOR(0, UMongoDBClient)

#  ifdef USE_MONGODB
      if (puri)       U_SYSCALL_VOID(mongoc_uri_destroy, "%p", puri);
      if (client)     U_SYSCALL_VOID(mongoc_client_destroy, "%p", client);
      if (collection) U_SYSCALL_VOID(mongoc_collection_destroy, "%p", collection);

      U_SYSCALL_VOID_NO_PARAM(mongoc_cleanup);
#  endif
      }

   // SERVICES

   UVector<UString> vitem;

   bool findOne(uint32_t value, bson_t* query)
      {
      U_TRACE(0, "UMongoDBClient::findOne(%u,%p)", value, query)

#  ifdef USE_MONGODB
      U_INTERNAL_ASSERT_POINTER(client)
      U_INTERNAL_ASSERT_POINTER(collection)

      U_SYSCALL_VOID(bson_init, "%p", query);

      BSON_APPEND_INT32(query, "_id", value);

      if (find(query)) U_RETURN(true);
#  endif

      U_RETURN(false);
      }

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
   bool connect(const char* _uri) { return false; }
   bool connect(const char* host, unsigned int _port) { return false; }
   bool executeBulk(mongoc_bulk_operation_t* bulk) { return false; }
   bool selectCollection(const char* db, const char* name_collection) { return false; }
   bool update(uint32_t old_value, const char* key, uint32_t new_value) { return false; }
   void updateOneBulk(mongoc_bulk_operation_t* bulk, uint32_t old_value, const char* key, uint32_t new_value) {}
   mongoc_bulk_operation_t* createBulk(bool ordered, const mongoc_write_concern_t* write_concern = U_NULLPTR) { return U_NULLPTR; }
# if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const { return ""; }
# endif
#else
   bool insert(bson_t* doc);
   bool remove(bson_t* selector);
   bool findOne(const char* json, uint32_t len);

   bool find(bson_t* query, bson_t* projection = U_NULLPTR,            mongoc_query_flags_t flags = MONGOC_QUERY_NONE, mongoc_read_prefs_t* read_prefs = U_NULLPTR);
   bool findAggregation(bson_t* pipeline, bson_t* options = U_NULLPTR, mongoc_query_flags_t flags = MONGOC_QUERY_NONE, mongoc_read_prefs_t* read_prefs = U_NULLPTR); // Execute an 'aggregation' query

   bool        update(bson_t* query, bson_t* update);
   bool findAndModify(bson_t* query, bson_t* update);

   bool selectCollection(const char* db, const char* name_collection);

   bool update(uint32_t old_value, const char* key, uint32_t new_value);

   // connect to MongoDB server

   bool connect(const char* uri);
   bool connect(const char* host, unsigned int _port);

   // BULK

   bool   executeBulk(mongoc_bulk_operation_t* bulk);
   void updateOneBulk(mongoc_bulk_operation_t* bulk, uint32_t old_value, const char* key, uint32_t new_value);

   mongoc_bulk_operation_t* createBulk(bool ordered, const mongoc_write_concern_t* write_concern = U_NULLPTR)
      {
      U_TRACE(0, "UMongoDBClient::createBulk(%b,%p)", ordered, write_concern)

#  ifndef USE_MONGODB
      U_RETURN_POINTER(0, mongoc_bulk_operation_t);
#  else
      U_INTERNAL_ASSERT_POINTER(client)
      U_INTERNAL_ASSERT_POINTER(collection)

#  if MONGOC_CHECK_VERSION(1, 9, 0)
      mongoc_bulk_operation_t* bulk = (mongoc_bulk_operation_t*) U_SYSCALL(mongoc_collection_create_bulk_operation_with_opts, "%p,%p", collection, U_NULLPTR);  
#  else
      mongoc_bulk_operation_t* bulk = (mongoc_bulk_operation_t*) U_SYSCALL(mongoc_collection_create_bulk_operation, "%p,%b,%p", collection, ordered, write_concern);  
#  endif

      U_RETURN_POINTER(bulk, mongoc_bulk_operation_t);
#  endif
      }

   void insertBulk(mongoc_bulk_operation_t* bulk, const bson_t* doc)
      {
      U_TRACE(0, "UMongoDBClient::insertBulk(%p,%p)", bulk, doc)

#  ifdef USE_MONGODB
      U_INTERNAL_ASSERT_POINTER(client)
      U_INTERNAL_ASSERT_POINTER(collection)

      U_SYSCALL_VOID(mongoc_bulk_operation_insert, "%p,%p", bulk, doc);
#  endif
      }

   void updateBulk(mongoc_bulk_operation_t* bulk, bson_t* query, bson_t* _update) // This function queues an update as part of a bulk operation
      {
      U_TRACE(0, "UMongoDBClient::updateBulk(%p,%p,%p)", bulk, query, _update)

#  ifdef USE_MONGODB
      U_INTERNAL_ASSERT_POINTER(client)
      U_INTERNAL_ASSERT_POINTER(collection)

      U_SYSCALL_VOID(mongoc_bulk_operation_update, "%p,%p,%p,%b", bulk, query, _update, false);

      U_SYSCALL_VOID(bson_destroy, "%p", query);
      U_SYSCALL_VOID(bson_destroy, "%p", _update);
#  endif
      }

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
#endif

   void readFromCursor();
   
private:
   U_DISALLOW_COPY_AND_ASSIGN(UMongoDBClient)
};

// by Victor Stewart

#if defined(U_STDCPP_ENABLE) && defined(HAVE_CXX20) && defined(USE_MONGODB) && defined(U_LINUX)
template <class StringType>
   static void bson_append_utf8(bson_t* doc, UCompileTimeStringType&& fieldname, StringType&& value)
      {
           if constexpr (std::is_same_v<StringType, UString>)               (void) bson_append_utf8(doc, U_CTV_TO_PARAM(fieldname), U_STRING_TO_PARAM(value));
      else if constexpr (UCompileTimeStringFormatter::is_ctv_v<StringType>) (void) bson_append_utf8(doc, U_CTV_TO_PARAM(fieldname),    U_CTV_TO_PARAM(value));
      }

static void bson_append_bool(     bson_t* doc, UCompileTimeStringType&& fieldname,    bool value) { (void) ::bson_append_bool(     doc, U_CTV_TO_PARAM(fieldname), value); }
static void bson_append_int32(    bson_t* doc, UCompileTimeStringType&& fieldname, int32_t value) { (void) ::bson_append_int32(    doc, U_CTV_TO_PARAM(fieldname), value); }
static void bson_append_date_time(bson_t* doc, UCompileTimeStringType&& fieldname, int64_t value) { (void) ::bson_append_date_time(doc, U_CTV_TO_PARAM(fieldname), value); }
#endif

#endif
