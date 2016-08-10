// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    tdb.h - (TDB) Trivial database (Samba utility)
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_TDB_H
#define ULIB_TDB_H 1

#include <ulib/string.h>

#include <tdb.h>

/**
 * @class UTDB
 *
 * @brief UTDB is a wrapper to Trivial DataBases (TDB) API
 *
 * TDB is a simple database API. It was inspired by the realisation that in Samba we have several ad-hoc bits of code that
 * essentially implement small databases for sharing structures between parts of Samba. The interface is based on gdbm.
 * gdbm couldn't be use as we needed to be able to have multiple writers to the databases at one time
 */

union _uudata {
    U_DATA d1;
  TDB_DATA d2;
};

class U_EXPORT UTDB {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   static bool disable_mmap, disable_lock;

   struct tdb_context* context;

   UTDB()
      {
      U_TRACE_REGISTER_OBJECT(0, UTDB, "", 0)

      context = 0;
      }

   ~UTDB()
      {
      U_TRACE_UNREGISTER_OBJECT(1, UTDB)

      if (context) close();
      }

   // Create a database

   bool create(char* name, int hash_size = 0,
               int tdb_flags = TDB_CLEAR_IF_FIRST | \
                              (disable_mmap ? TDB_NOMMAP : 0) | \
                              (disable_lock ? TDB_NOLOCK : 0),
               int open_flags = O_RDWR | O_CREAT | O_TRUNC, mode_t mode = 0600)
      {
      U_TRACE(1, "UTDB::create(%S,%d,%d,%d,%d)", name, hash_size, tdb_flags, open_flags, mode)

      if (context) (void) U_SYSCALL(tdb_close, "%p", context);

      context = (struct tdb_context*) U_SYSCALL(tdb_open, "%S,%d,%d,%d,%d", name, hash_size, tdb_flags, open_flags, mode);

      if (context) U_RETURN(true);

      U_RETURN(false);
      }

   // Open a database

   bool open(char* name, int hash_size = 0,
             int tdb_flags = (disable_mmap ? TDB_NOMMAP : 0) | \
                             (disable_lock ? TDB_NOLOCK : 0),
             int open_flags = O_RDWR, mode_t mode = 0600)
      {
      U_TRACE(1, "UTDB::open(%S,%d,%d,%d,%d)", name, hash_size, tdb_flags, open_flags, mode)

      if (context) (void) U_SYSCALL(tdb_close, "%p", context);

      context = (struct tdb_context*) U_SYSCALL(tdb_open, "%S,%d,%d,%d,%d", name, hash_size, tdb_flags, open_flags, mode);

      if (context) U_RETURN(true);

      U_RETURN(false);
      }

   // Close a database

   void close()
      {
      U_TRACE_NO_PARAM(1, "UTDB::close()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(context)

      (void) U_SYSCALL(tdb_close, "%p", context);
                                        context = 0;
      }

   // Store an element in the database

   bool store(const char* _key, uint32_t keylen, const char* _data, uint32_t datalen, int _flag = TDB_INSERT) // TDB_REPLACE
      {
      U_TRACE(1, "UTDB::store(%.*S,%u,%.*S,%u,%d)", keylen, _key, keylen, datalen, _data, datalen, _flag)

      _uudata key, dbuf;

       key.d1.dptr  = (unsigned char*)_key;
       key.d1.dsize = keylen;
      dbuf.d1.dptr  = (unsigned char*)_data;
      dbuf.d1.dsize = datalen;

      if (U_SYSCALL(tdb_store, "%p,%J,%J,%d", context, key.d2, dbuf.d2, _flag)) U_RETURN(false);

      U_RETURN(true);
      }

   bool store(const UString& k, const char* d, uint32_t dlen,    int _flag) { return store(U_STRING_TO_PARAM(k), d, dlen,              _flag); }
   bool store(const char*    k, uint32_t klen, const UString& d, int _flag) { return store(k, klen,              U_STRING_TO_PARAM(d), _flag); }
   bool store(const UString& k, const UString& d,                int _flag) { return store(U_STRING_TO_PARAM(k), U_STRING_TO_PARAM(d), _flag); }

   // Delete an entry in the database given a key

   bool remove(const char* _key, uint32_t keylen)
      {
      U_TRACE(1, "UTDB::remove(%.*S,%u)", keylen, _key, keylen)

      _uudata key;

      key.d1.dptr  = (unsigned char*)_key;
      key.d1.dsize = keylen;

      if (U_SYSCALL(tdb_delete, "%p,%J", context, key.d2)) U_RETURN(false);

      U_RETURN(true);
      }

   bool remove(const UString& _key) { return remove(U_STRING_TO_PARAM(_key)); }

   // Fetch an entry in the database given a key

   UString at(const char* _key, uint32_t keylen)
      {
      U_TRACE(1, "UTDB::at(%.*S,%u)", keylen, _key, keylen)

      _uudata key;

      key.d1.dptr  = (unsigned char*)_key;
      key.d1.dsize = keylen;

      TDB_DATA dbuf = (TDB_DATA) U_SYSCALL(tdb_fetch, "%p,%J", context, key.d2);

      if (dbuf.dptr)
         {
         UString str((const char*)dbuf.dptr, dbuf.dsize);

         str.rep->_capacity = U_TO_FREE;

         U_RETURN_STRING(str);
         }

      return UString::getStringNull();
      }

   UString at(UStringRep* _key)
      {
      U_TRACE(0, "UTDB::at(%V)", _key)

      return at(U_STRING_TO_PARAM(*_key));
      }

   UString at(const UString& _key)
      {
      U_TRACE(0, "UTDB::at(%V)", _key.rep)

      return at(U_STRING_TO_PARAM(_key));
      }

   // Substitute a key/value with a new key/value (remove+store)

   bool substitute(const UString& _key, const UString& new_key, const UString& _data)
      {
      U_TRACE(1, "UTDB::substitute(%V,%V,%V)", _key.rep, new_key.rep, _data.rep)

      _uudata key;

      key.d1.dptr  = (unsigned char*)_key.data();
      key.d1.dsize = _key.size();

      if (U_SYSCALL(tdb_delete, "%p,%J", context, key.d2) == 0)
         {
         _uudata dbuf;

          key.d1.dptr  = (unsigned char*)new_key.data();
          key.d1.dsize = new_key.size();
         dbuf.d1.dptr  = (unsigned char*)_data.data();
         dbuf.d1.dsize = _data.size();

         if (U_SYSCALL(tdb_store, "%p,%J,%J,%d", context, key.d2, dbuf.d2, TDB_INSERT) == 0) U_RETURN(true);
         }

      U_RETURN(false);
      }

   // operator []

   UString operator[](UStringRep* _key)    { return at(_key); }
   UString operator[](const UString& _key) { return at(_key); }

   bool find(const char* _key, uint32_t keylen)
      {
      U_TRACE(1, "UTDB::find(%.*S,%u)", keylen, _key, keylen)

      _uudata key;

      key.d1.dptr  = (unsigned char*)_key;
      key.d1.dsize = keylen;

      if (U_SYSCALL(tdb_exists, "%p,%J", context, key.d2)) U_RETURN(true);

      U_RETURN(false);
      }

   bool find(const UString& _key) { return find(U_STRING_TO_PARAM(_key)); }

   // PRINT

   UString print();
   UString printSorted();

   bool getKeys(UVector<UString>& vec);

   // STREAM

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT istream& operator>>(istream& is, UTDB& tdb);
   friend U_EXPORT ostream& operator<<(ostream& os, UTDB& tdb);

   // DEBUG

#  ifdef DEBUG
   const char* dump(bool reset) const;
#  endif
#endif

private:
   static int   _print(TDB_CONTEXT* tdb, TDB_DATA key, TDB_DATA dbuf, void* ptr) U_NO_EXPORT;
   static int _getKeys(TDB_CONTEXT* tdb, TDB_DATA key, TDB_DATA dbuf, void* ptr) U_NO_EXPORT;
   
   U_DISALLOW_COPY_AND_ASSIGN(UTDB)
};

#endif
