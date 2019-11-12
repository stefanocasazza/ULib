// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rdb.h - A Reliable DataBase library (Felix von Leitner)
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_RDB_H
#define ULIB_RDB_H 1

#include <ulib/db/cdb.h>
#include <ulib/utility/lock.h>
#include <ulib/utility/string_ext.h>
#include <ulib/utility/data_session.h>

#ifdef _MSWINDOWS_
#  include <ws2tcpip.h>
#else
#  include <netinet/in.h>
#endif

/**
 * @class URDB
 *
 * @brief URDB is a fast, Reliable, simple class for creating and reading DataBases
 *
 * The idea behind URDB is to take UCDB and put a journal over it.
 * Then provide an abstraction layer that looks like ndbm and writes updates to the journal.
 * Read operations are answered by consulting the cache (build with journal) and the cdb file.
 * The result should be a reasonably small yet crash-proof read-write database
 */

class URDBServer;
class UHttpPlugIn;
class Application;
class UServer_Base;
class UDataStorage;
class URDBClient_Base;
class URDBClientImage;

// The interface is very similar to the gdbm one

#  define CACHE_HASHTAB_LEN 769

#  define RDB_off(prdb)      ((URDB::cache_struct*)(((URDB*)prdb)->journal.map))->off
#  define RDB_capacity(prdb) (uint32_t)(((URDB*)prdb)->journal.st_size - RDB_off(prdb))
#  define RDB_eof(prdb)      (((URDB*)prdb)->journal.map+(ptrdiff_t)((URDB*)prdb)->journal.st_size)
#  define RDB_allocate(prdb) (uint32_t*)(((URDB*)prdb)->journal.map+(ptrdiff_t)RDB_off(prdb))

#  define RDB_sync(prdb)      ((URDB::cache_struct*)(((URDB*)prdb)->journal.map))->sync
#  define RDB_nrecord(prdb)   ((URDB::cache_struct*)(((URDB*)prdb)->journal.map))->nrecord
#  define RDB_reference(prdb) ((URDB::cache_struct*)(((URDB*)prdb)->journal.map))->reference
#  define RDB_hashtab(prdb)  (((URDB::cache_struct*)(((URDB*)prdb)->journal.map))->hashtab)

#  define RDB_ptr(prdb)      (((URDB*)prdb)->journal.map+sizeof(URDB::cache_struct))
#  define RDB_start(prdb)    (RDB_ptr(prdb)-(CACHE_HASHTAB_LEN*sizeof(uint32_t)))
#  define RDB_node(prdb)     ((URDB::cache_node*)(((URDB*)prdb)->journal.map+prdb->node))

#  define RDB_node_key_pr(prdb)  u_get_unaligned32(RDB_node(prdb)->key.dptr)
#  define RDB_node_key_sz(prdb)  u_get_unaligned32(RDB_node(prdb)->key.dsize)
#  define RDB_node_data_pr(prdb) u_get_unaligned32(RDB_node(prdb)->data.dptr)
#  define RDB_node_data_sz(prdb) u_get_unaligned32(RDB_node(prdb)->data.dsize)

#  define RDB_node_key(prdb)     (((URDB*)prdb)->journal.map+RDB_node_key_pr(prdb))
#  define RDB_node_data(prdb)    (((URDB*)prdb)->journal.map+RDB_node_data_pr(prdb))

#  define RDB_ptr_node(prdb,offset)      ((URDB::cache_node*)(((URDB*)prdb)->journal.map+offset))
#  define RDB_cache_node(node,attribute) (u_get_unaligned32(((URDB::cache_node*)node)->attribute))

class U_EXPORT URDB : public UCDB {
public:

   URDB(int _ignore_case = false) : UCDB(_ignore_case)
      {
      U_TRACE_CTOR(0, URDB, "%d", _ignore_case)

      plock = U_NULLPTR;
      pnode = U_NULLPTR;
       node = 0;

      key1.dptr  = U_NULLPTR;
      key1.dsize = 0;
      }

   URDB(const UString& pathdb, int _ignore_case) : UCDB(pathdb, _ignore_case)
      {
      U_TRACE_CTOR(0, URDB, "%V,%d", pathdb.rep, _ignore_case)

      plock = U_NULLPTR;
      pnode = U_NULLPTR;
       node = 0;

      key1.dptr  = U_NULLPTR;
      key1.dsize = 0;
      }

   // coverity[VIRTUAL_DTOR]
#ifdef U_COVERITY_FALSE_POSITIVE
   virtual
#endif
   ~URDB()
      {
      U_TRACE_DTOR(0, URDB)

      if (preclock)
         {
         delete[] preclock;
                  preclock = U_NULLPTR;
         }
      }

   // Open a Reliable DataBase

   bool open(                       uint32_t log_size = 1024 * 1024, bool btruncate = false, bool cdb_brdonly = true, bool breference = true, sem_t* psem = &nolock);
   bool open(const UString& pathdb, uint32_t log_size = 1024 * 1024, bool btruncate = false, bool cdb_brdonly = true, bool breference = true, sem_t* psem = &nolock)
      {
      U_TRACE(0, "URDB::open(%V,%u,%b,%b,%b)", pathdb.rep, log_size, btruncate, cdb_brdonly, breference)

      UFile::setPath(pathdb);

      return URDB::open(log_size, btruncate, cdb_brdonly, breference, psem);
      }

   void reset();

   uint32_t size() const
      {
      U_TRACE_NO_PARAM(0, "URDB::size()")

      U_INTERNAL_DUMP("UCDB::nrecord = %u RDB_nrecord = %u", UCDB::nrecord, RDB_nrecord(this))

      U_RETURN(UCDB::nrecord + RDB_nrecord(this));
      }

   // Close a Reliable DataBase

   void close(bool breference = true);

   // Combines the old cdb file and the diffs in a new cdb file.
   // Close the database and deletes the obsolete journal file if everything worked out

   bool closeReorganize();

   // ---------------------------------------------------------------------
   // Write a key/value pair to a reliable database
   // ---------------------------------------------------------------------
   // RETURN VALUE
   // ---------------------------------------------------------------------
   //  0: Everything was OK
   // -1: flag was RDB_INSERT and this key already existed
   // -3: there is not enough (virtual) memory available on writing journal
   // ---------------------------------------------------------------------

#  define RDB_INSERT              0 // Insertion of new entries only
#  define RDB_REPLACE             1 // Allow replacing existing entries
#  define RDB_INSERT_WITH_PADDING 2 // Allow replacing existing entries and for insertion of new entries padding data with space

   int store(const UString& k, const char* d, uint32_t dlen,    int _flag) { return store(U_STRING_TO_PARAM(k), d, dlen,              _flag); }
   int store(const char*    k, uint32_t klen, const UString& d, int _flag) { return store(k, klen,              U_STRING_TO_PARAM(d), _flag); }
   int store(const UString& k, const UString& d,                int _flag) { return store(U_STRING_TO_PARAM(k), U_STRING_TO_PARAM(d), _flag); }

   int store(const char* _key, uint32_t keylen, const char* _data, uint32_t datalen, int _flag);

   // ---------------------------------------------------------------------
   // Mark a key/value as deleted
   // ---------------------------------------------------------------------
   // RETURN VALUE
   // ---------------------------------------------------------------------
   //  0: Everything was OK
   // -1: The entry was not in the database
   // -2: The entry was already marked deleted in the hash-tree
   // -3: there is not enough (virtual) memory available on writing journal
   // ---------------------------------------------------------------------

   int remove(const UString& _key)
      {
      U_TRACE(0, "URDB::remove(%V)", _key.rep)

      UCDB::setKey(_key);

      return remove();
      }

   int remove(const char* _key, uint32_t keylen)
      {
      U_TRACE(0, "URDB::remove(%.*S,%u)", keylen, _key, keylen)

      UCDB::setKey(_key, keylen);

      return remove();
      }

   // ----------------------------------------------------------------------
   // Substitute a key/value with a new key/value (remove+store)
   // ----------------------------------------------------------------------
   // RETURN VALUE
   // ----------------------------------------------------------------------
   //  0: Everything was OK
   // -1: The entry was not in the database
   // -2: The entry was marked deleted in the hash-tree
   // -3: there is not enough (virtual) memory available on writing journal
   // -4: flag was RDB_INSERT and the new key already existed
   // ----------------------------------------------------------------------

   int substitute(const UString& _key, const UString& new_key, const UString& _data, int _flag = RDB_INSERT)
      {
      U_TRACE(0, "URDB::substitute(%V,%V,%V,%d)", _key.rep, new_key.rep, _data.rep, _flag)

      UCDB::setKey(_key);
      UCDB::setData(_data);
      UCDB::datum key2 = { (void*) new_key.data(), new_key.size() };

      return substitute(&key2, _flag);
      }

   bool fetch();
   bool find(const char* key, uint32_t keylen);

   bool find(const UString& _key) { return find(U_STRING_TO_PARAM(_key)); }

   UFile&   getJournal()            { return journal; }
   uint32_t getJournalSize()        { return journal.st_size; }
   uint32_t getCapacity() const     { return RDB_capacity(this); }
   uint32_t getDataSize() const     { return RDB_node_data_sz(this); }
   void*    getDataPointer() const  { return RDB_node_data(this); }

   UString at(UStringRep* _key)
      {
      U_TRACE(0, "URDB::at(%V)", _key)

      UCDB::setKey(_key);

      return at();
      }

   UString at(const UString& _key)
      {
      U_TRACE(0, "URDB::at(%V)", _key.rep)

      UCDB::setKey(_key);

      return at();
      }

   UString at(const char* _key, uint32_t keylen)
      {
      U_TRACE(0, "URDB::at(%.*S,%u)", keylen, _key, keylen)

      UCDB::setKey(_key, keylen);

      return at();
      }

   // operator []

   UString operator[](UStringRep* _key)    { return at(_key); }
   UString operator[](const UString& _key) { return at(_key); }

   // flushes changes made to the log file back to disk

   void msync();
   void fsync() { journal.fsync(); }

   // LOCK

   void   lockRecord();
   void unlockRecord()
      {
      U_TRACE_NO_PARAM(0, "URDB::unlockRecord()")

      U_INTERNAL_ASSERT_POINTER(plock)

      plock->unlock();
      }

   static void initRecordLock();

   void   lock() { if (_lock.sem) _lock.lock(); }
   void unlock() { if (_lock.sem) _lock.unlock(); }

   // TRANSACTION

   bool  beginTransaction();
   void  abortTransaction();
   void commitTransaction();

   // Call function for all entry

   void getKeys(UVector<UString>& vec);

   void callForAllEntry(      iPFprpr function, UVector<UString>* v  = U_NULLPTR);
   void callForAllEntryDelete(iPFprpr function);
   void callForAllEntrySorted(iPFprpr function, qcompare compare_obj = U_NULLPTR);

   // PRINT

   UString print();
   UString printSorted();

   // STREAM

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT ostream& operator<<(ostream& os, URDB& rdb);

   // DEBUG

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   ULock _lock;
   ULock* plock;
   UFile journal;

   // ----------------------------------------------------------------------------------------------------------------
   // CACHE for CDB:
   // ----------------------------------------------------------------------------------------------------------------
   // This code implements a chained hash table where we use binary search trees instead of linked lists as chains.
   // Let's call this a hash tree. This code uses blobs, not 0-terminated strings. While these routines can be used
   // as associative hash, they are meant as a cache for a larger, disk-base database or mmap'ed file. And I provide
   // functions to mark a record as deleted, so that this can be used to cache deltas to a constant database like CDB
   // ----------------------------------------------------------------------------------------------------------------
   // NB: offsets relative to the starting address of the mapping should be employed...
   // ----------------------------------------------------------------------------------------------------------------

   typedef struct rdb_datum {
      uint32_t dptr, dsize;
   } rdb_datum;

   typedef struct rdb_cache_node {
      rdb_datum key, data;
      uint32_t left, right; // Two cache_node 'pointer' of the binary search tree behind every entry of the hash table
   } cache_node;

   typedef struct rdb_cache_struct {
      uint32_t off;                        // RDB_off
      uint32_t sync;                       // RDB_sync
      uint32_t nrecord;                    // RDB_nrecord
      uint32_t reference;                  // RDB_reference
      uint32_t hashtab[CACHE_HASHTAB_LEN]; // RDB_hashtab
      // -----> data storage...            // RDB_ptr
   } cache_struct;

   // Manage shared cache

   UString at();

   int  remove();
   bool _fetch();
   bool isDeleted();
   bool reorganize(); // Combines the old cdb file and the diffs in a new cdb file
   int  store(int flag);
   bool compactionJournal();
   int _store(int flag, bool exist);
   int  substitute(UCDB::datum* new_key, int flag);

   void resetReference()
      {
      U_TRACE_NO_PARAM(0, "URDB::resetReference()")

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("RDB_reference = %u", RDB_reference(this))

      RDB_reference(this) = 1;
      }

   bool cdbLookup() // NB: set the value of struct UCDB::data...
      {
      U_TRACE_NO_PARAM(0, "URDB::cdbLookup()")

      U_CHECK_MEMORY

      if (UFile::st_size &&
          UCDB::find())
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   // utility especially created for RDB net interface class

   static char* parseLine(const char* ptr, UCDB::datum* key, UCDB::datum* data);

          char* parseLine(const char* ptr) { return parseLine(ptr, &key, &data); }

   static sem_t nolock;
   static ULock* preclock;

private:
   uint32_t* pnode;
   uint32_t   node; // RDB_node
   UCDB::datum key1;

   static uint32_t nerror;

   inline void setNodeLeft() U_NO_EXPORT;
   inline void setNodeRight() U_NO_EXPORT;

   void copy1(URDB* prdb, uint32_t offset) U_NO_EXPORT;
   void call1(UCDB* pcdb, uint32_t offset) U_NO_EXPORT;
   void print1(UCDB* pcdb, uint32_t offset) U_NO_EXPORT;
   void getKeys1(UCDB* pcdb, uint32_t offset) U_NO_EXPORT;
   void makeAdd1(UCDB* pcdb, uint32_t offset) U_NO_EXPORT;

   bool logJournal(int op) U_NO_EXPORT;
   bool resizeJournal(uint32_t oversize) U_NO_EXPORT;
   void call(UCDB* pcdb, vPFpvu function1, vPFpvpc function2) U_NO_EXPORT;
   void callForEntryNotInCache(UCDB* pcdb, vPFpvpc function2) U_NO_EXPORT;
   bool writev(const struct iovec* iov, int n, uint32_t size) U_NO_EXPORT;

   static void htAlloc(URDB* prdb) U_NO_EXPORT;       // Alloc one node for the hash tree
   static bool htLookup(URDB* prdb) U_NO_EXPORT;      // Search one key/data pair in the cache
   static void htInsert(URDB* prdb) U_NO_EXPORT;      // Insert one key/data pair in the cache
   static void htRemoveAlloc(URDB* prdb) U_NO_EXPORT; // remove one node allocated for the hash tree

   U_DISALLOW_COPY_AND_ASSIGN(URDB)

   friend class UHTTP;
   friend class URDBServer;
   friend class UHttpPlugIn;
   friend class Application;
   friend class UServer_Base;
   friend class URDBClient_Base;
   friend class URDBClientImage;

   template <class T> friend class URDBObjectHandler;
};

template <> class U_EXPORT URDBObjectHandler<UDataStorage*> : public URDB {
public:

   URDBObjectHandler(const UString& pathdb, int _ignore_case, void* ptr, bool _bdirect = false);

   // coverity[VIRTUAL_DTOR]
#ifdef U_COVERITY_FALSE_POSITIVE
   virtual
#endif
   ~URDBObjectHandler()
      {
      U_TRACE_DTOR(0, URDBObjectHandler<UDataStorage*>)

#  ifdef DEBUG
      if (bdirect == false &&
          pDataStorage     &&
          pDataStorage->recdata)
         {
         uint32_t sz = pDataStorage->size();

         U_INTERNAL_DUMP("pDataStorage->recdata(%u) = %p", sz, pDataStorage->recdata)

         U_INTERNAL_ASSERT_MAJOR(sz, 0)

         UMemoryPool::_free(pDataStorage->recdata, sz);
         }
#  endif
      }

   // SERVICES

   void close();

   bool getDataStorage();
   bool getDataStorage(const char* s, uint32_t n);

   bool getDataStorage(in_addr_t client)
      {
      U_TRACE(0, "URDBObjectHandler<UDataStorage*>::getDataStorage(%u)", client)

      U_CHECK_MEMORY

      U_cdb_no_hash(this) = true;

      bool result = getDataStorage((const char*)&client, sizeof(in_addr_t));

      U_cdb_no_hash(this) = false;

      U_RETURN(result);
      }

   bool getDataStorage(const UString& _key)
      {
      U_TRACE(0, "URDBObjectHandler<UDataStorage*>::getDataStorage(%V)", _key.rep)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(pDataStorage)

      pDataStorage->setKeyIdDataSession(_key);

      return getDataStorage();
      }

   bool putDataStorage(const UString& _key)
      {
      U_TRACE(0, "URDBObjectHandler<UDataStorage*>::putDataStorage(%V)", _key.rep)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(pDataStorage)
      U_INTERNAL_ASSERT_EQUALS(_key, pDataStorage->keyid)

      return putDataStorage(RDB_INSERT_WITH_PADDING);
      }

   bool putDataStorage(int op = RDB_INSERT_WITH_PADDING);

   bool insertDataStorage(int op)
      {
      U_TRACE(0, "URDBObjectHandler<UDataStorage*>::insertDataStorage(%d)", op)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(pDataStorage)
      U_ASSERT(pDataStorage->isDataSession())

      data_buffer = pDataStorage->toBuffer();
      data_len    = UDataStorage::buffer_len;

      return _insertDataStorage(U_STRING_TO_PARAM(pDataStorage->keyid), op);
      }

   bool insertDataStorage(const char* s, uint32_t n, int _flag = RDB_INSERT) // SSL session cache...
      {
      U_TRACE(0, "URDBObjectHandler<UDataStorage*>::insertDataStorage(%.*S,%u,%d)", n, s, n, _flag)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(pDataStorage)

      pDataStorage->setKeyIdDataSession(s, n);

      return insertDataStorage(_flag);
      }

   bool insertDataStorage(const UString& _key, int _flag = RDB_INSERT_WITH_PADDING)
      {
      U_TRACE(0, "URDBObjectHandler<UDataStorage*>::insertDataStorage(%V,%d)", _key.rep, _flag)

      return insertDataStorage(U_STRING_TO_PARAM(_key), _flag);
      }

   void setPointerToDataStorage()
      {
      U_TRACE_NO_PARAM(0, "URDBObjectHandler<UDataStorage*>::setPointerToDataStorage()")

      U_INTERNAL_ASSERT(bdirect)
      U_INTERNAL_ASSERT_POINTER(pDataStorage)

      *(char**)pDataStorage = recval.data();
      }

   bool insertDataStorage(void* drec, uint32_t dlen, const char* s, uint32_t n, int op)
      {
      U_TRACE(0, "URDBObjectHandler<UDataStorage*>::insertDataStorage(%p,%u,%.*S,%u,%d)", drec, dlen, n, s, n, op)

      data_buffer = (char*)drec;
      data_len    = dlen;

      if (_insertDataStorage(s, n, op))
         {
         setPointerToDataStorage();

         U_ASSERT(recval.equal(data_buffer, dlen))

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool insertDataStorage(void* drec, uint32_t dlen, in_addr_t client, int op)
      {
      U_TRACE(0, "URDBObjectHandler<UDataStorage*>::insertDataStorage(%p,%u,%u,%d)", drec, dlen, client, op)

      U_CHECK_MEMORY

      U_cdb_no_hash(this) = true;

      bool result = insertDataStorage(drec, dlen, (const char*)&client, sizeof(in_addr_t), op);

      U_cdb_no_hash(this) = false;

      U_RETURN(result);
      }

   UString getKeyID() const { return pDataStorage->keyid; }
   void  resetKeyID() const {        pDataStorage->keyid.clear(); }

   bool          isRecordFound() const     { return brecfound; }
   UString       getRecordValue() const    { return recval; }
   UDataStorage* getPointerToDataStorage() { return pDataStorage; }

   void setPointerToDataStorage(UDataStorage* ptr)
      {
      U_TRACE(0, "URDBObjectHandler<UDataStorage*>::setPointerToDataStorage(%p)", ptr)

      pDataStorage = ptr;

      U_INTERNAL_DUMP("pDataStorage->keyid(%u) = %V", ptr->keyid.size(), ptr->keyid.rep)

#  ifdef DEBUG
      if (bdirect == false &&
          ptr              &&
          ptr->recdata)
         {
         uint32_t sz = ptr->size();

         U_INTERNAL_DUMP("pDataStorage->recdata(%u) = %p", sz, ptr->recdata)

         U_INTERNAL_ASSERT_MAJOR(sz, 0)
         }
#  endif
      }

   // Call function for all entry

   void callForAllEntry(iPFprpr function, vPF function_no_lock = U_NULLPTR, qcompare compare_obj = U_NULLPTR);

   void callForAllEntryWithSetEntry(vPFprpr function, vPF function_no_lock = U_NULLPTR, qcompare compare_obj = U_NULLPTR)
      { bsetEntry = true; callForAllEntry((iPFprpr)function, function_no_lock, compare_obj); bsetEntry = false; }

   void callForAllEntryWithVector(iPFprpr function, vPF function_no_lock = (vPF)-1, qcompare compare_obj = U_NULLPTR) { callForAllEntry(function, function_no_lock, compare_obj); }

#ifdef DEBUG
   const char* dump(bool _reset) const;
#endif

protected:
   UString recval;
   UDataStorage* pDataStorage;
   bool brecfound, bdirect;

   static bool bsetEntry;
   static char* data_buffer;
   static uint32_t data_len;
   static iPFpvpv ds_function_to_call;
   static URDBObjectHandler<UDataStorage*>* pthis;

   bool _insertDataStorage(const char* s, uint32_t n, int op);

          void setEntry(     UStringRep* key, UStringRep* data);
   static int callEntryCheck(UStringRep* key, UStringRep* data);

private:
   U_DISALLOW_COPY_AND_ASSIGN(URDBObjectHandler<UDataStorage*>)

   friend class UServer_Base;
};

template <class T> class U_EXPORT URDBObjectHandler<T*> : public URDBObjectHandler<UDataStorage*> {
public:

   URDBObjectHandler(const UString& pathdb, int _ignore_case, const T* ptr) : URDBObjectHandler<UDataStorage*>(pathdb, _ignore_case, ptr)
      {
      U_TRACE_CTOR(0, URDBObjectHandler<T*>, "%V,%d,%p", pathdb.rep, _ignore_case, ptr)
      }

   ~URDBObjectHandler()
      {
      U_TRACE_DTOR(0, URDBObjectHandler<T*>)
      }

   // SERVICES

#ifdef DEBUG
   const char* dump(bool _reset) const { return URDBObjectHandler<UDataStorage*>::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(URDBObjectHandler<T*>)
};

#endif
