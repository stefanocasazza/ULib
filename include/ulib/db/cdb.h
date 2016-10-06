// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    cdb.h - A structure for constant databases (Bernstein)
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_CDB_H
#define ULIB_CDB_H 1

#include <ulib/file.h>
#include <ulib/container/hash_map.h>

/**
 * @class UCDB
 *
 * @brief UCDB is a fast, reliable, simple class for creating and reading constant databases.
 *
 * Its database structure provides several features:
 * Fast lookups: A successful lookup in a large database normally takes just two disk accesses.
 * An unsuccessful lookup takes only one.
 * Low overhead: A database uses 4096 bytes, plus 16 bytes per record, plus the space for keys and data.
 * A cdb is an associative array: it maps strings (keys) to strings (data).
 * A cdb contains 512 pointers to linearly probed open hash tables.
 * The hash tables contain pointers to (key,data) pairs. A cdb is stored in a single file on disk:
 * +----------------+------------+-------+-------+-----+---------+
 * | p0 p1 ... p511 | records... | hash0 | hash1 | ... | hash511 |
 * +----------------+------------+-------+-------+-----+---------+
 * Each of the 512 initial pointers states a position and a length. The position is the starting byte
 * position of the hash table. The length is the number of slots in the hash table.
 * Records are stored sequentially, without special alignment. A record states a key length, a data
 * length, the key, and the data. Each hash table slot states a hash value and a byte position. If the
 * byte position is 0, the slot is empty. Otherwise, the slot points to a record whose key has that hash value.
 * Positions, lengths, and hash values are 32-bit quantities, stored in 4 bytes. Thus a cdb must fit into 4 gigabytes.
 * A record is located as follows. Compute the hash value of the key in the record.
 * The hash value modulo 512 is the number of a hash table.
 * The hash value divided by 512, modulo the length of that table, is a slot number.
 * Probe that slot, the next higher slot, and so on, until you find the record or run into an empty slot
 */

#define CDB_NUM_HASH_TABLE_POINTER 512

class URDB;
class UHTTP;

typedef int  (*iPFprpr) (UStringRep*, UStringRep*);
typedef void (*vPFprpr) (UStringRep*, UStringRep*);

#define U_cdb_ignore_case(obj)         (obj)->UCDB::flag[0]
#define U_cdb_shared(obj)              (obj)->UCDB::flag[1]
#define U_cdb_result_call(obj)         (obj)->UCDB::flag[2]
#define U_cdb_add_entry_to_vector(obj) (obj)->UCDB::flag[3]

class U_EXPORT UCDB : public UFile {
public:

   typedef struct datum {
      void* dptr;
      uint32_t dsize;
   } datum;

   typedef struct cdb_hash_table_pointer {
      uint32_t pos;   // starting byte position of the hash table
      uint32_t slots; //        number of slots in the hash table
   } cdb_hash_table_pointer;

   typedef struct cdb_record_header {
      uint32_t klen; //  key length
      uint32_t dlen; // data length
   } cdb_record_header;

   typedef struct cdb_hash_table_slot {
      uint32_t hash; // hash value of the key
      uint32_t pos;  // starting byte position of the record (0 -> slot empty)
   } cdb_hash_table_slot;

   UCDB(int ignore_case = 0)
      {
      U_TRACE_REGISTER_OBJECT(0, UCDB, "%d", ignore_case)

      init_internal(ignore_case);
      }

   UCDB(const UString& path, int ignore_case) : UFile(path)
      {
      U_TRACE_REGISTER_OBJECT(0, UCDB, "%V,%d", path.rep, ignore_case)

      init_internal(ignore_case);
      }

#ifdef U_COVERITY_FALSE_POSITIVE
   virtual
#endif
   ~UCDB()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UCDB)
      }

   // Open a Constant DataBase

   bool open(bool brdonly = true);

   bool open(const UString& pathdb, bool brdonly = true)
      {
      U_TRACE(0, "UCDB::open(%V)", pathdb.rep)

      UFile::setPath(pathdb);

      return UCDB::open(brdonly);
      }

   bool ignoreCase() const  { return ignoreCase(this); }

   void setKey(UStringRep* _key)                 { key.dptr = (void*) _key->data(); key.dsize = _key->size(); }
   void setKey(const UString&  _key)             { key.dptr = (void*) _key.data();  key.dsize = _key.size(); }
   void setKey(const void* dptr, uint32_t dsize) { key.dptr = (void*) dptr;         key.dsize = dsize; }

   void setData(const UString& _data)             { data.dptr = (void*)_data.data(); data.dsize = _data.size(); }
   void setData(const void* dptr, uint32_t dsize) { data.dptr = (void*) dptr;        data.dsize = dsize; }

   bool find(const UString& _key)
      {
      U_TRACE(0, "UCDB::find(%V)", _key.rep)

      setKey(_key);

      cdb_hash();

      return find();
      }

   bool findNext(); // handles repeated keys...

   // Get methods

   uint32_t size() const
      {
      U_TRACE_NO_PARAM(0, "UCDB::size()")

      U_RETURN(nrecord);
      }

   UString elem()
      {
      U_TRACE_NO_PARAM(0, "UCDB::elem()")

      UString str((const char*)data.dptr, data.dsize);

      U_RETURN_STRING(str);
      }

   // Set methods

   void setSize(uint32_t sz)
      {
      U_TRACE(0, "UCDB::setSize(%u)", sz)

      nrecord = sz;
      }

   // operator []

   UString operator[](const UString& _key)
      {
      U_TRACE(0, "UCDB::operator[](%V)", _key.rep)

      setKey(_key);

      return at();
      }

   UString operator[](UStringRep* _key)
      {
      U_TRACE(0, "UCDB::operator[](%V)", _key)

      setKey(_key);

      return at();
      }

   // Call function for all entry

   char* getPattern() { return pattern; }

   void addEntryToVector() { U_cdb_add_entry_to_vector(this) = true; }

   iPFprpr getFunctionToCall()             { return function_to_call; }
   void    setFunctionToCall(iPFprpr func) { function_to_call  = func; }

   UVector<UString>* getVector()                      { return ptr_vector; }
   void              setVector(UVector<UString>* ptr) { ptr_vector = ptr; }

   void callForAllEntrySorted(     iPFprpr function);
   void callForAllEntryWithPattern(iPFprpr function, UString* pattern);

   iPFprpr getFilterToFunctionToCall()                 { return filter_function_to_call; }
   void  resetFilterToFunctionToCall()                 { filter_function_to_call = functionCall; }
   void    setFilterToFunctionToCall(iPFprpr function) { filter_function_to_call = function; }

   uint32_t getValuesWithKeyNask(UVector<UString>& vec_values, const UString& mask_key, uint32_t* size = 0);

   // Save memory hash table as Constant DataBase

   static uint32_t sizeFor(uint32_t _nrecord)
      {
      U_TRACE(0, "UCDB::sizeFor(%u)", _nrecord)

      uint32_t size = CDB_NUM_HASH_TABLE_POINTER * sizeof(cdb_hash_table_pointer) +
                      _nrecord * (sizeof(cdb_record_header) + sizeof(cdb_hash_table_slot));

      U_RETURN(size);
      }

          bool writeTo(                     UHashMap<void*>* t, uint32_t tbl_space, pvPFpvpb f = 0) { return UCDB::writeTo(*this, t, tbl_space, f); }
   static bool writeTo(const UString& path, UHashMap<void*>* t, uint32_t tbl_space, pvPFpvpb f = 0) { return UCDB(path, t->ignoreCase()).writeTo(t, tbl_space, f); }

   // STREAM

   UString print();

#ifdef U_STDCPP_ENABLE
   friend U_EXPORT istream& operator>>(istream& is, UCDB& cdb);
   friend U_EXPORT ostream& operator<<(ostream& os, UCDB& cdb);

   // DEBUG

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   datum key;                  // initialized in find()
   datum data;                 // initialized if findNext() returns 1
   cdb_record_header* hr;      // initialized if findNext() returns 1
   cdb_hash_table_slot* slot;  // initialized in find()
   cdb_hash_table_pointer* hp; // initialized in find()

   // internal

   char* pattern;
   UString* pbuffer;
   UVector<UString>* ptr_vector;
   iPFprpr function_to_call, filter_function_to_call;

   // when mmap not available we use this storage...

   cdb_hash_table_pointer hp_buf;
   cdb_record_header      hr_buf;
   cdb_hash_table_slot  slot_buf;

   uint32_t loop,    // number of hash slots searched under key
            nslot,   // initialized in find()
            khash,   // initialized in find()
            nrecord, // initialized in makeStart()
            offset,
            start_hash_table_slot;

   unsigned char flag[4];

   bool find();
   UString at();
   void init_internal(int ignore_case);

   static bool ignoreCase(const UCDB* pcdb) { return (U_cdb_ignore_case(pcdb) != 0); }

   uint32_t cdb_hash(const char* t, uint32_t tlen)
      {
      U_TRACE(0, "UCDB::cdb_hash(%.*S,%u)", tlen, t, tlen)

      int flags = (U_cdb_ignore_case(this) == 0xff ? -1 : U_cdb_ignore_case(this));

      U_INTERNAL_DUMP("flags = %d U_cdb_ignore_case(this) = %d", flags, U_cdb_ignore_case(this))

      uint32_t result = u_cdb_hash((unsigned char*)t, tlen, flags);

      U_RETURN(result);
      }

   void cdb_hash() { khash = cdb_hash((const char*)key.dptr, key.dsize); }

   void setHash(uint32_t _hash)               { khash = _hash; }
   void setHash(const char* t, uint32_t tlen) { khash = cdb_hash(t, tlen); }

   // START-END of record data

   char* start() const { return (UFile::map + CDB_NUM_HASH_TABLE_POINTER * sizeof(cdb_hash_table_pointer)); }
   char*   end() const { return (UFile::map + start_hash_table_slot); }

   // Call function for all entry

   void callForAllEntry(vPFpvpc function);

   static int functionCall(UStringRep* key, UStringRep* data) { return 1; }

   // Save memory hash table as Constant DataBase

   static bool writeTo(UCDB& cdb, UHashMap<void*>* table, uint32_t tbl_space, pvPFpvpb f = 0);

   // FOR RDB

   void makeStart()
      {
      U_TRACE_NO_PARAM(0, "UCDB::makeStart()")

      U_INTERNAL_ASSERT_DIFFERS(map, MAP_FAILED)

      nrecord = start_hash_table_slot = 0;

      hr = (UCDB::cdb_record_header*) start();
      }

   uint32_t makeFinish(bool reset);

   void call1();
   void call1(const char*  key_ptr, uint32_t  key_size,
              const char* data_ptr, uint32_t data_size);

   static void call2(UCDB* pcdb, char* src);
   static void print2(UCDB* pcdb, char* src);
   static void getKeys2(UCDB* pcdb, char* src);
   static void makeAdd2(UCDB* pcdb, char* src);

#ifdef DEBUG
   void checkForAllEntry() U_NO_EXPORT;
#endif

private:
   inline bool match(uint32_t pos) U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UCDB)

   friend class URDB;
   friend class UHTTP;

   template <class T> friend class URDBObjectHandler;
};

#endif
