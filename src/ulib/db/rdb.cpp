// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    rdb.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/db/rdb.h>
#include <ulib/net/server/server.h>

uint32_t URDB::nerror;

#define U_FOR_EACH_ENTRY1(pcdb,function1)                      \
                                                               \
   U_cdb_result_call(pcdb) = 1;                                \
                                                               \
   /* 1) first we read the entry in the cache... */            \
                                                               \
   for (uint32_t _offset, i = 0; i < CACHE_HASHTAB_LEN; ++i)   \
      {                                                        \
      if ((_offset = RDB_hashtab(this)[i]))                    \
         {                                                     \
         U_INTERNAL_DUMP("slot = %u _offset = %u", i, _offset) \
                                                               \
         function1(pcdb, _offset);                             \
                                                               \
         if (U_cdb_result_call(pcdb) == 0) break;              \
         if (U_cdb_result_call(pcdb) == 2) (void) remove();    \
         }                                                     \
      }

#define U_FOR_EACH_ENTRY2(pcdb,function2)                                                              \
                                                                                                       \
   /* 2) ...after we scan the constant database and we search the entry NOT present in the cache... */ \
                                                                                                       \
   if (UFile::st_size) callForEntryNotInCache(pcdb, (vPFpvpc)function2)

#define U_FOR_EACH_ENTRY(pcdb,function1,function2) { U_FOR_EACH_ENTRY1(pcdb,function1) \
                                                     U_FOR_EACH_ENTRY2(pcdb,function2); }

// Search one key/data pair in the cache.
// To save code, I use only a single function to do the lookup in the hash table and binary search tree

U_NO_EXPORT inline void URDB::setNodeLeft()
{
   U_TRACE_NO_PARAM(0, "URDB::setNodeLeft()")

   U_CHECK_MEMORY

   pnode = &(RDB_node(this)->left);

   // NB: the reference at memory in the cache data must point to memory mapped...

   U_INTERNAL_DUMP("pnode = %p node = %u", pnode, node)

   U_INTERNAL_ASSERT_RANGE(RDB_hashtab(this), pnode, RDB_allocate(this))
}

U_NO_EXPORT inline void URDB::setNodeRight()
{
   U_TRACE_NO_PARAM(0, "URDB::setNodeRight()")

   U_CHECK_MEMORY

   pnode = &(RDB_node(this)->right);

   // NB: the reference at memory in the cache data must point to memory mapped...

   U_INTERNAL_DUMP("pnode = %p node = %u", pnode, node)

   U_INTERNAL_ASSERT_RANGE(RDB_hashtab(this), pnode, RDB_allocate(this))
}

U_NO_EXPORT bool URDB::htLookup(URDB* prdb)
{
   U_TRACE(0, "URDB::htLookup(%p)", prdb)

#ifdef DEBUG
   if (prdb->UCDB::key.dsize == 0) U_ERROR("URDB::htLookup(%p) - key search have size null - db(%.*S,%u recs)", prdb, U_FILE_TO_TRACE(*prdb), RDB_nrecord(prdb));
#endif

   U_INTERNAL_ASSERT_POINTER(prdb->UCDB::key.dptr)

   // Because the insertion routine has to know where to insert the cache_node, this code has to assign
   // a pointer to the empty pointer to manipulate in that case, so we have to do a nasty indirection...

   prdb->pnode = RDB_hashtab(prdb) + (prdb->UCDB::khash % CACHE_HASHTAB_LEN);

   U_INTERNAL_DUMP("pnode = %p slot = %u", prdb->pnode, prdb->UCDB::khash % CACHE_HASHTAB_LEN)

   uint32_t len;

   while (true)
      {
#  ifdef DEBUG
      if ((char*)prdb->pnode < RDB_start(prdb) ||
                 prdb->pnode > RDB_allocate(prdb))
         {
         ++nerror;

         U_WARNING("URDB::htLookup(%p) - pointer node (%p) OUT OF RANGE (%p,%p)", prdb, prdb->pnode, RDB_start(prdb), RDB_allocate(prdb));

         goto invalid_entry;
         }
#  endif

      prdb->node = u_get_unalignedp32(prdb->pnode);

      U_INTERNAL_DUMP("pnode = %p node = %u", prdb->pnode, prdb->node)

      if (prdb->node == 0) break;

#  ifdef DEBUG
      if ((char*)prdb->pnode < RDB_start(prdb) ||
                 prdb->pnode > RDB_allocate(prdb))
         {
         ++nerror;

         U_WARNING("URDB::htLookup(%p) - pointer node (%p) OUT OF RANGE (%p,%p)", prdb, prdb->pnode, RDB_start(prdb), RDB_allocate(prdb));

         goto invalid_entry;
         }
#  endif

      len = RDB_node_key_sz(prdb);

#  ifdef DEBUG
      if (len == 0)
         {
         ++nerror;

         U_WARNING("URDB::htLookup(%p) - null key size at offset %u, pnode %p", prdb, prdb->node, prdb->pnode);

invalid_entry:
         prdb->node = 0;

         u_put_unalignedp32(prdb->pnode, 0);

         break;
         }
#  endif

      int result = u_equal(prdb->UCDB::key.dptr, RDB_node_key(prdb), U_min(prdb->UCDB::key.dsize, len), UCDB::ignoreCase(prdb));

      // RDB_node => ((URDB::cache_node*)(journal.map+node))

      U_INTERNAL_DUMP("result = %d len = %d", result, len)

      if (result < 0) prdb->setNodeLeft();
      else
         {
         if (result == 0 &&
             len    == prdb->UCDB::key.dsize)
            {
            U_RETURN(true);
            }

         prdb->setNodeRight();
         }
      }

   U_RETURN(false);
}

// Alloc one node for the hash tree

U_NO_EXPORT void URDB::htAlloc(URDB* prdb)
{
   U_TRACE(0, "URDB::htAlloc(%p)", prdb)

   U_INTERNAL_ASSERT_POINTER(prdb->UCDB::key.dptr)
   U_INTERNAL_ASSERT_MAJOR(prdb->UCDB::key.dsize, 0)

   U_INTERNAL_DUMP("RDB_capacity = %u", RDB_capacity(prdb))

#ifdef DEBUG
   if (RDB_capacity(prdb) < sizeof(URDB::cache_node))
      {
      U_ERROR("URDB::htAlloc() - capacity(%u) < size node(%u) - db(%.*S,%u recs)", RDB_capacity(prdb), sizeof(URDB::cache_node), U_FILE_TO_TRACE(*prdb), RDB_nrecord(prdb));
      }
#endif

   prdb->node = RDB_off(prdb);

   u_put_unalignedp32(prdb->pnode, prdb->node);

   RDB_off(prdb) += sizeof(URDB::cache_node);

   (void) U_SYSCALL(memset, "%p,%d,%d", prdb->journal.map + prdb->node, 0, sizeof(URDB::cache_node));
}

// remove one node allocated for the hash tree

U_NO_EXPORT void URDB::htRemoveAlloc(URDB* prdb)
{
   U_TRACE(0, "URDB::htRemoveAlloc(%p)", prdb)

   U_INTERNAL_ASSERT_POINTER(prdb->UCDB::key.dptr)
   U_INTERNAL_ASSERT_MAJOR(prdb->UCDB::key.dsize, 0)

#ifdef DEBUG
   if (RDB_capacity(prdb) < sizeof(URDB::cache_node))
      {
      U_ERROR("URDB::htRemoveAlloc() - capacity(%u) < size node(%u) - db(%.*S,%u recs)", RDB_capacity(prdb), sizeof(URDB::cache_node), U_FILE_TO_TRACE(*prdb), RDB_nrecord(prdb));
      }
#endif

   RDB_off(prdb) -= sizeof(URDB::cache_node);

   U_INTERNAL_ASSERT_EQUALS(RDB_off(prdb), u_get_unalignedp32(prdb->pnode))

   (void) U_SYSCALL(memset, "%p,%d,%d", prdb->journal.map + prdb->node, 0, sizeof(URDB::cache_node));

   u_put_unalignedp32(prdb->pnode, 0);
}

// Insert one key/data pair in the cache

U_NO_EXPORT void URDB::htInsert(URDB* prdb)
{
   U_TRACE(0, "URDB::htInsert(%p)", prdb)

   // NB: the reference at memory in the cache data must point to memory mapped...

   U_INTERNAL_DUMP("key  = { %p, %u }", prdb->UCDB::key.dptr,  prdb->UCDB::key.dsize)
   U_INTERNAL_DUMP("data = { %p, %u }", prdb->UCDB::data.dptr, prdb->UCDB::data.dsize)

#ifdef DEBUG
                                U_INTERNAL_ASSERT_RANGE(RDB_ptr(prdb), prdb->UCDB::key.dptr,  RDB_allocate(prdb))
   if (prdb->UCDB::data.dptr) { U_INTERNAL_ASSERT_RANGE(RDB_ptr(prdb), prdb->UCDB::data.dptr, RDB_allocate(prdb)) }
#endif

   // NB: the reference at memory in the cache data must point to memory mapped...

   U_INTERNAL_DUMP("pnode = %p node = %u", prdb->pnode, prdb->node)

   U_INTERNAL_ASSERT_RANGE(sizeof(URDB::cache_struct), prdb->node, prdb->journal.st_size - sizeof(URDB::cache_node))
   U_INTERNAL_ASSERT_RANGE(RDB_hashtab(prdb),          prdb->pnode, RDB_allocate(prdb))

   uint32_t offset1 =                           (char*)prdb->UCDB::key.dptr  - prdb->journal.map,
            offset2 = (prdb->UCDB::data.dptr ? ((char*)prdb->UCDB::data.dptr - prdb->journal.map) : 0);

   u_put_unaligned32(RDB_node(prdb)->key.dptr,   offset1);
   u_put_unaligned32(RDB_node(prdb)->key.dsize,  prdb->UCDB::key.dsize);
   u_put_unaligned32(RDB_node(prdb)->data.dptr,  offset2);
   u_put_unaligned32(RDB_node(prdb)->data.dsize, prdb->UCDB::data.dsize);
}

U_NO_EXPORT bool URDB::resizeJournal(uint32_t oversize)
{
   U_TRACE(0, "URDB::resizeJournal(%u)", oversize)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("RDB_reference = %u", RDB_reference(this))

   if (RDB_reference(this) <= 1)
      {
      uint32_t _size = (journal.st_size / 2);

      if (oversize < _size) oversize = _size;

      U_INTERNAL_DUMP("oversize = %u", oversize)

      uint32_t _offset = (char*)pnode - journal.map;

      U_INTERNAL_DUMP("pnode = %p node = %u offset = %u", pnode, node, _offset)

#  if defined(__CYGWIN__) || defined(_MSWINDOWS_)
      journal.munmap(); // for ftruncate()...
#  endif

      if (journal.ftruncate(journal.st_size + oversize))
         {
#     if defined(__CYGWIN__) || defined(_MSWINDOWS_)
         (void) journal.memmap(PROT_READ | PROT_WRITE);
#     endif

         pnode = (uint32_t*)(journal.map + _offset);

         U_INTERNAL_DUMP("pnode = %p node = %u offset = %u", pnode, node, _offset)

         U_INTERNAL_ASSERT_RANGE(RDB_hashtab(this), pnode, RDB_allocate(this))

         U_RETURN(true);
         }

      // save entry

      key1              = UCDB::key;
      UCDB::datum data1 = UCDB::data;
      uint32_t save     = UCDB::khash;

      if (reorganize())
         {
         // set old entry

         UCDB::key   = key1;
         UCDB::data  = data1;
         UCDB::khash = save;

         (void) htLookup(this);

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

U_NO_EXPORT bool URDB::writev(const struct iovec* _iov, int n, uint32_t _size)
{
   U_TRACE(0, "URDB::writev(%p,%d,%u)", _iov, n, _size)

   U_INTERNAL_ASSERT_MAJOR(_size,0)

   U_INTERNAL_DUMP("RDB_off = %u", RDB_off(this))

   uint32_t sz = RDB_off(this) + _size + (sizeof(URDB::cache_node) * 32);

   if (sz > journal.st_size &&
       resizeJournal(sz - journal.st_size) == false)
      {
      U_RETURN(false);
      }

   char* journal_ptr = journal.map + RDB_off(this);

   for (int i = 0; i < n; ++i)
      {
      U_MEMCPY(journal_ptr, _iov[i].iov_base, _iov[i].iov_len);

      // NB: one time writed the data on journal we change the reference at memory
      //     at data end keys so they pointing to journal memory mapped...

      if (n < 5) // remove(), store()
         {
         if      (i == 1) UCDB::key.dptr  = journal_ptr; // remove(), store()
         else if (i == 2) UCDB::data.dptr = journal_ptr; // store()
         }
      else // substitute()
         {
         if      (i == 1)      key1.dptr  = journal_ptr;
         if      (i == 3) UCDB::key.dptr  = journal_ptr;
         else if (i == 4) UCDB::data.dptr = journal_ptr;
         }

      journal_ptr += _iov[i].iov_len;
      }

   RDB_off(this) = (journal_ptr - journal.map);

   U_INTERNAL_DUMP("RDB_off = %u", RDB_off(this))

   U_RETURN(true);
}

bool URDB::logJournal(int op)
{
   U_TRACE(0, "URDB::logJournal(%d)", op)

   // Records are stored without special alignment. A record
   // states a key length, a data length, the key, and the data

   if (op == 0) // remove
      {
      UCDB::cdb_record_header hrec = { UCDB::key.dsize, U_NOT_FOUND };

      U_INTERNAL_DUMP("hrec = { %u, %u }", hrec.klen, hrec.dlen)

      struct iovec _iov[2] = { { (caddr_t)&hrec, sizeof(UCDB::cdb_record_header) },
                               { (caddr_t)UCDB::key.dptr, UCDB::key.dsize } };

      if (writev(_iov, 2, sizeof(UCDB::cdb_record_header) + UCDB::key.dsize) == false) U_RETURN(false);
      }
   else if (op == 1) // store
      {
      UCDB::cdb_record_header hrec = { UCDB::key.dsize, UCDB::data.dsize };

      U_INTERNAL_DUMP("hrec = { %u, %u }", hrec.klen, hrec.dlen)

      struct iovec _iov[3] = { { (caddr_t)&hrec, sizeof(UCDB::cdb_record_header) },
                               { (caddr_t)UCDB::key.dptr,  UCDB::key.dsize },
                               { (caddr_t)UCDB::data.dptr, UCDB::data.dsize } };

      if (writev(_iov, 3, sizeof(UCDB::cdb_record_header) + UCDB::key.dsize + UCDB::data.dsize) == false) U_RETURN(false);
      }
   else // substitute
      {
      U_INTERNAL_ASSERT_EQUALS(op,2)
      U_INTERNAL_ASSERT_POINTER(UCDB::key.dptr)
      U_INTERNAL_ASSERT_POINTER(key1.dptr)
      U_INTERNAL_ASSERT_POINTER(UCDB::data.dptr)
      U_INTERNAL_ASSERT_MAJOR(UCDB::key.dsize,0)
      U_INTERNAL_ASSERT_MAJOR(key1.dsize,0)
      U_INTERNAL_ASSERT_MAJOR(UCDB::data.dsize,0)

      UCDB::cdb_record_header hrec1 = {      key1.dsize, U_NOT_FOUND },
                              hrec  = { UCDB::key.dsize, UCDB::data.dsize };

      U_INTERNAL_DUMP("hrec1 = { %u, %u } hrec = { %u, %u }", hrec1.klen, hrec1.dlen, hrec.klen, hrec.dlen)

      struct iovec _iov[5] = { { (caddr_t)&hrec1, sizeof(UCDB::cdb_record_header) },
                               { (caddr_t)key1.dptr, key1.dsize },
                               { (caddr_t)&hrec, sizeof(UCDB::cdb_record_header) },
                               { (caddr_t)UCDB::key.dptr,  UCDB::key.dsize },
                               { (caddr_t)UCDB::data.dptr, UCDB::data.dsize } };

      if (writev(_iov, 5, (sizeof(UCDB::cdb_record_header) * 2) + key1.dsize + UCDB::key.dsize + UCDB::data.dsize) == false) U_RETURN(false);
      }

   U_RETURN(true);
}

U_NO_EXPORT void URDB::copy1(URDB* prdb, uint32_t _offset) // entry present on cache...
{
   U_TRACE(0, "URDB::copy1(%p,%u)", prdb, _offset)

   URDB::cache_node* n = RDB_ptr_node(this, _offset);

#ifdef DEBUG
   if (    (char*)n < RDB_start(this) ||
       (uint32_t*)n > RDB_allocate(this))
      {
      ++nerror;

      U_WARNING("URDB::copy1(%p,%u) - pointer node (%p) OUT OF RANGE (%p,%p)", prdb, _offset, n, RDB_start(this), RDB_allocate(this));

      return;
      }
#endif

   if (RDB_cache_node(n,left))  copy1(prdb, RDB_cache_node(n,left));
   if (RDB_cache_node(n,right)) copy1(prdb, RDB_cache_node(n,right));

   // check if node is mark for deleted (dptr == NULL)...

   uint32_t offset_data = RDB_cache_node(n,data.dptr);

   U_INTERNAL_DUMP("offset_data = %p", offset_data)

   if (offset_data)
      {
      uint32_t size_key  = RDB_cache_node(n, key.dsize),
               size_data = RDB_cache_node(n,data.dsize);

      union uucdb_record_header {
         uint32_t*                p;
         UCDB::cdb_record_header* prec;
      };

      union uucdb_record_header u = { &size_key };

      U_INTERNAL_DUMP("prec = { %u, %u }", u.prec->klen, u.prec->dlen)

      const char* ptr_key = (const char*)((ptrdiff_t)RDB_cache_node(n, key.dptr) + (ptrdiff_t)journal.map);

      prdb->UCDB::data.dsize = size_data;

      prdb->UCDB::setKey(ptr_key, size_key);

      prdb->UCDB::cdb_hash();

      // Search one key/data pair in the cache

#  ifdef DEBUG
      if (htLookup(prdb))
         {
         ++nerror;

         U_WARNING("URDB::copy1(%p,%u) - data node at offset is already present in copy, key: %#.*S", prdb, _offset, size_key, ptr_key);

         return;
         }

      U_INTERNAL_ASSERT_EQUALS(prdb->node, 0)
#  else
      (void) htLookup(prdb);
#  endif

      htAlloc(prdb);

      char* journal_ptr = prdb->journal.map + RDB_off(prdb);

      U_MEMCPY(journal_ptr, u.prec, sizeof(UCDB::cdb_record_header));

      // i == 0

      journal_ptr += sizeof(UCDB::cdb_record_header);

      U_MEMCPY(journal_ptr, ptr_key, size_key);

      prdb->UCDB::key.dptr = journal_ptr; // i == 1

      journal_ptr += size_key;

      U_MEMCPY(journal_ptr, journal.map + offset_data, size_data);

      prdb->UCDB::data.dptr = journal_ptr; // i == 2

      journal_ptr += size_data;

      RDB_off(prdb) = (journal_ptr - prdb->journal.map);

      // NB: the reference at memory in the cache data must point to memory mapped...

      htInsert(prdb); // Insertion of new entry in the cache

      RDB_nrecord(prdb)++;
      }
#ifdef DEBUG
   else if (RDB_cache_node(n,data.dsize) != U_NOT_FOUND)
      {
      ++nerror;

      U_WARNING("URDB::copy1(%p,%u) - data node at offset is invalid, key: %#.*S", prdb, _offset,
                  RDB_cache_node(n,key.dsize), (const char*)((ptrdiff_t)RDB_cache_node(n,key.dptr) + (ptrdiff_t)journal.map));

      u_put_unaligned32(n->data.dsize, U_NOT_FOUND);
      }
#  endif
}

void URDB::setShared(sem_t* psem, char* spinlock)
{
   U_TRACE(0, "URDB::setShared(%p,%p)", psem, spinlock)

   U_CHECK_MEMORY

   if (psem == 0)
      {
      char somename[256];

      // For portable use, a shared memory object should be identified by a name of the form /somename; that is,
      // a null-terminated string of up to NAME_MAX (i.e., 255) characters consisting of an initial slash,
      // followed by one or more characters, none of which are slashes

      UString basename = UFile::getName();

      (void) u__snprintf(somename, sizeof(somename), U_CONSTANT_TO_PARAM("/%v"), basename.rep);

      psem     = (sem_t*) UFile::shm_open(somename, sizeof(sem_t) + 1);
      spinlock = (char*)psem + sizeof(sem_t) + 1;
      }

   _lock.init(psem, spinlock);

   U_cdb_shared(this) = true;
}

bool URDB::compactionJournal()
{
   U_TRACE_NO_PARAM(0, "URDB::compactionJournal()")

   U_CHECK_MEMORY

   U_ASSERT_EQUALS(UFile::isOpen(), false) // NB: no cdb file, Ex: ssl session cache...

   U_INTERNAL_DUMP("RDB_off = %u RDB_reference = %u", RDB_off(this), RDB_reference(this))

   bool result = true;
   URDB rdb(UCDB::ignoreCase());
   char rdb_buffer_path[MAX_FILENAME_LEN];

   rdb.journal.setPath(*(const UFile*)this, rdb_buffer_path, U_CONSTANT_TO_PARAM(".tmp"));

   if (rdb.journal.creat(O_RDWR) &&
       rdb.journal.ftruncate(journal.st_size))
      {
      if (rdb.journal.memmap(PROT_READ | PROT_WRITE) == false) U_RETURN(false);

      rdb.UCDB::nrecord   = 0;

      RDB_off(&rdb)       = sizeof(URDB::cache_struct);
      RDB_reference(&rdb) = 1;

      U_INTERNAL_DUMP("RDB_off = %u RDB_sync = %u capacity = %u nrecord = %u RDB_reference = %u",
                       RDB_off(&rdb), RDB_sync(&rdb), RDB_capacity(&rdb), RDB_nrecord(&rdb), RDB_reference(&rdb))

      lock();

#  ifdef DEBUG
      nerror = 0;
#  endif

      U_FOR_EACH_ENTRY1(&rdb, copy1)

#  if defined(_MSWINDOWS_) || defined(__CYGWIN__)
      journal.UFile::munmap(); // for rename()...
#    ifdef   _MSWINDOWS_
      rdb.journal.UFile::close();
#    endif
#  endif

      if (rdb.journal._rename(journal.UFile::path_relativ) == false) U_RETURN(false);

#  if defined(_MSWINDOWS_) || defined(__CYGWIN__)
#    ifdef   _MSWINDOWS_
      result = rdb.journal.UFile::open(journal.UFile::path_relativ);
#    endif
      result = rdb.journal.memmap(PROT_READ | PROT_WRITE);
#  endif

#  ifdef DEBUG
      uint32_t sz1 =     getCapacity(),
               sz2 = rdb.getCapacity();

      U_DEBUG("URDB::compactionJournal() - nrecords (%u => %u) capacity (%.2fM (%u bytes) => %.2fM (%u bytes)) nerror=%u",
                        size(), rdb.size(),
                        (double)sz1 / (1024.0 * 1024.0), sz1,
                        (double)sz2 / (1024.0 * 1024.0), sz2, nerror)

      nerror = 0;
#  endif

      journal.UFile::substitute(rdb.journal);

      unlock();

      U_INTERNAL_DUMP("RDB_off = %u RDB_sync = %u capacity = %u nrecord = %u RDB_reference = %u",
                       RDB_off(this), RDB_sync(this), RDB_capacity(this), RDB_nrecord(this), RDB_reference(this))

#  ifdef DEBUG
      if (RDB_capacity(this) < sizeof(URDB::cache_node))
         {
         ++nerror;

         U_WARNING("URDB::compactionJournal() - capacity(%u) < size node(%u)", RDB_capacity(this), sizeof(URDB::cache_node));
         }
#  endif

      U_RETURN(result);
      }

   U_RETURN(false);
}

// open a Reliable DataBase

bool URDB::open(uint32_t log_size, bool btruncate, bool cdb_brdonly, bool breference)
{
   U_TRACE(0, "URDB::open(%u,%b,%b,%b)", log_size, btruncate, cdb_brdonly, breference)

   bool result = false;

   (void) UCDB::open(cdb_brdonly);

   journal.setPath(*(const UFile*)this, 0, U_CONSTANT_TO_PARAM(".jnl"));

   int            flags  = O_RDWR;
   if (btruncate) flags |= O_TRUNC; // NB: we can have only the journal (HTTP session, SSL session, ...)

   if (journal.creat(flags))
      {
      lock();

      uint32_t journal_sz     = journal.size(),
               journal_sz_new = (journal_sz >= log_size ? journal_sz : log_size);

      if (journal_sz_new == 0) journal_sz_new = (UFile::st_size ? UFile::st_size : 1 * 1024 * 1024); // 1M 

      if (journal_sz     == journal_sz_new ||
          journal.ftruncate(journal_sz_new))
         {
#     if !defined(__CYGWIN__) && !defined(_MSWINDOWS_)
         if (journal_sz_new < 32 * 1024 * 1024) journal_sz_new = 32 * 1024 * 1024; // oversize mmap for optimize resizeJournal() with ftruncate()
#     endif

         if (journal.memmap(PROT_READ | PROT_WRITE, 0, 0, journal_sz_new))
            {
            if (RDB_off(this) == 0) RDB_off(this) = sizeof(URDB::cache_struct);

            U_INTERNAL_DUMP("RDB_off = %u RDB_sync = %u capacity = %u nrecord = %u RDB_reference = %u",
                             RDB_off(this), RDB_sync(this), RDB_capacity(this), RDB_nrecord(this), RDB_reference(this))

#        ifdef DEBUG
            if (RDB_capacity(this) < sizeof(URDB::cache_node))
               {
               ++nerror;

               U_WARNING("URDB::open(%u,%b,%b,%b) - capacity(%u) < size node(%u)",
                          log_size, btruncate, cdb_brdonly, breference, RDB_capacity(this), sizeof(URDB::cache_node));
               }
#        endif

            if (breference) RDB_reference(this)++;

            result = true;
            }
         }

      unlock();
      }

   U_RETURN(result);
}

void URDB::close(bool breference)
{
   U_TRACE(0, "URDB::close(%b)", breference)

   U_CHECK_MEMORY

   if (UFile::map_size) UFile::munmap(); // Constant DB

   if (breference == false) journal.munmap();
   else
      {
      lock();

      RDB_reference(this)--;

      uint32_t reference = RDB_reference(this);

      U_INTERNAL_DUMP("RDB_reference = %u", RDB_reference(this))

      uint32_t sz = RDB_sync(this) = RDB_off(this);

      journal.munmap();

      if (reference == 0) (void) journal.ftruncate(sz);

      unlock();
      }

   if (journal.isOpen()) journal.close();

   journal.reset();
}

void URDB::reset()
{
   U_TRACE_NO_PARAM(1, "URDB::reset()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_DIFFERS(journal.map, MAP_FAILED)

   resetReference();

   RDB_off(this)     = sizeof(URDB::cache_struct);
   RDB_sync(this)    = 0;
   RDB_nrecord(this) = 0;

   // Initialize the cache to contain no entries

   (void) U_SYSCALL(memset, "%p,%d,%d", RDB_hashtab(this), 0, sizeof(uint32_t) * CACHE_HASHTAB_LEN);
}

bool URDB::beginTransaction()
{
   U_TRACE_NO_PARAM(0, "URDB::beginTransaction()")

   lock();

   return reorganize();
}

void URDB::commitTransaction()
{
   U_TRACE_NO_PARAM(0, "URDB::commitTransaction()")

   msync();
   fsync();

   unlock();
}

void URDB::abortTransaction()
{
   U_TRACE_NO_PARAM(0, "URDB::abortTransaction()")

   U_INTERNAL_DUMP("RDB_reference = %u", RDB_reference(this))

   U_INTERNAL_ASSERT_EQUALS(RDB_reference(this), 1)

   reset();

   unlock();
}

void URDB::msync()
{
   U_TRACE_NO_PARAM(0, "URDB::msync()")

   U_CHECK_MEMORY

   lock();

   U_INTERNAL_DUMP("RDB_off = %u RDB_sync = %u", RDB_off(this), RDB_sync(this))

   UFile::msync(journal.map + RDB_off(this), journal.map + RDB_sync(this));

   RDB_sync(this) = RDB_off(this);

   U_INTERNAL_DUMP("RDB_off = %u RDB_sync = %u", RDB_off(this), RDB_sync(this))

   unlock();
}

// Close a Reliable DataBase

bool URDB::closeReorganize()
{
   U_TRACE_NO_PARAM(0, "URDB::closeReorganize()")

   resetReference();

   if (reorganize())
      {
      URDB::close();

      (void) journal._unlink();

      U_RETURN(true);
      }

   U_RETURN(false);
}

// Call function for all entry

U_NO_EXPORT void URDB::callForEntryNotInCache(UCDB* pcdb, vPFpvpc function2)
{
   U_TRACE(0, "URDB::callForEntryNotInCache(%p,%p)", pcdb, function2)

   U_INTERNAL_DUMP("nrecord = %u", UCDB::nrecord)

   U_INTERNAL_ASSERT_MAJOR(UFile::st_size,0)
   U_INTERNAL_ASSERT_DIFFERS(UFile::map, MAP_FAILED)

   char* ptr;
   char* _eof = UFile::map + (ptrdiff_t)UFile::st_size;
   UCDB::slot = (UCDB::cdb_hash_table_slot*) UCDB::end();

   U_cdb_result_call(pcdb) = 1;

   while ((char*)slot < _eof)
      {
      uint32_t pos = u_get_unaligned32(slot->pos);

      if (pos)
         {
         ptr      = UFile::map + pos;
         UCDB::hr = (UCDB::cdb_record_header*) ptr;

         U_INTERNAL_DUMP("hr(%p) = { %u, %u }", UCDB::hr, u_get_unaligned32(UCDB::hr->klen), u_get_unaligned32(UCDB::hr->dlen))

         UCDB::khash     = u_get_unaligned32(slot->hash);
         UCDB::key.dsize = u_get_unaligned32(UCDB::hr->klen);
         UCDB::key.dptr  = ptr + sizeof(UCDB::cdb_record_header);

         U_INTERNAL_DUMP("key = %.*S khash = %u", UCDB::key.dsize, UCDB::key.dptr, khash)

         if (htLookup(this) == false) // NB: entry NOT present in the cache...
            {
            function2(pcdb, ptr);

            if (U_cdb_result_call(pcdb) == 0) break;
            }
         }

      UCDB::slot++;
      }
}

// entry present in the cache...

U_NO_EXPORT void URDB::call1(UCDB* pcdb, uint32_t _offset)
{
   U_TRACE(0, "URDB::call1(%p,%u)", pcdb, _offset)

   URDB::cache_node* n = RDB_ptr_node(this, _offset);

   if (RDB_cache_node(n,left))  call1(pcdb, RDB_cache_node(n,left));
   if (RDB_cache_node(n,right)) call1(pcdb, RDB_cache_node(n,right));

   // check if node is mark for deleted (dptr == NULL)...

   uint32_t offset_data = RDB_cache_node(n,data.dptr);

   U_INTERNAL_DUMP("offset_data = %p", offset_data)

   if (offset_data)
      {
      pcdb->UCDB::call1((const char*)((ptrdiff_t)RDB_cache_node(n,key.dptr) + (ptrdiff_t)journal.map),
                        RDB_cache_node(n,key.dsize),
                        (const char*)((ptrdiff_t)offset_data                + (ptrdiff_t)journal.map),
                        RDB_cache_node(n,data.dsize));
      }
#ifdef DEBUG
   else if (RDB_cache_node(n,data.dsize) != U_NOT_FOUND)
      {
      ++nerror;

      U_WARNING("URDB::call1(%p,%u) - data node at offset is invalid, key: %#.*S", pcdb, offset,
                  RDB_cache_node(n,key.dsize), (const char*)((ptrdiff_t)RDB_cache_node(n,key.dptr) + (ptrdiff_t)journal.map));

      u_put_unaligned32(n->data.dsize, U_NOT_FOUND);
      }
#endif
}

U_NO_EXPORT void URDB::makeAdd1(UCDB* pcdb, uint32_t _offset) // entry presenti nella cache...
{
   U_TRACE(0, "URDB::makeAdd1(%p,%u)", pcdb, _offset)

   URDB::cache_node* n = RDB_ptr_node(this, _offset);

   if (RDB_cache_node(n,left))  makeAdd1(pcdb, RDB_cache_node(n,left));
   if (RDB_cache_node(n,right)) makeAdd1(pcdb, RDB_cache_node(n,right));

   // check if node is mark for deleted (dptr == NULL)...

   uint32_t offset_data = RDB_cache_node(n,data.dptr);

   U_INTERNAL_DUMP("offset_data = %p", offset_data)

   if (offset_data)
      {
      uint32_t size_key  = RDB_cache_node(n, key.dsize),
               size_data = RDB_cache_node(n,data.dsize);

      UCDB::cdb_record_header* ptr_hr = pcdb->hr;

      u_put_unaligned32(ptr_hr->klen, size_key);
      u_put_unaligned32(ptr_hr->dlen, size_data);

      U_INTERNAL_DUMP("hr(%p) = { %u, %u }", ptr_hr, u_get_unaligned32(ptr_hr->klen), u_get_unaligned32(ptr_hr->dlen))

      ++ptr_hr;

      char* ptr = (char*)ptr_hr;

      U_MEMCPY(ptr, journal.map + RDB_cache_node(n,key.dptr), size_key);

      ptr += size_key;

      U_MEMCPY(ptr, journal.map + offset_data, size_data);

      ptr += size_data;

      pcdb->hr = (UCDB::cdb_record_header*)ptr;

      pcdb->nrecord++;
      }
   else if (RDB_cache_node(n,data.dsize) != U_NOT_FOUND)
      {
#  ifdef DEBUG
      ++nerror;

      U_WARNING("URDB::makeAdd1(%p,%u) - data node at offset is invalid, key: %#.*S", pcdb, offset,
                  RDB_cache_node(n,key.dsize), (const char*)((ptrdiff_t)RDB_cache_node(n,key.dptr) + (ptrdiff_t)journal.map));
#  endif

      u_put_unaligned32(n->data.dsize, U_NOT_FOUND);
      }
}

U_NO_EXPORT void URDB::print1(UCDB* pcdb, uint32_t _offset) // entry present in the cache...
{
   U_TRACE(0, "URDB::print1(%p,%u)", pcdb, _offset)

   URDB::cache_node* n = RDB_ptr_node(this, _offset);

   if (RDB_cache_node(n,left))  print1(pcdb, RDB_cache_node(n,left));
   if (RDB_cache_node(n,right)) print1(pcdb, RDB_cache_node(n,right));

   // check if node is mark for deleted (dptr == NULL)...

   uint32_t offset_data = RDB_cache_node(n,data.dptr);

   U_INTERNAL_DUMP("offset_data = %p", offset_data)

   if (offset_data)
      {
      U_INTERNAL_ASSERT_POINTER(pcdb)
      U_INTERNAL_ASSERT_POINTER(pcdb->pbuffer)

      uint32_t klen = RDB_cache_node(n, key.dsize),
               dlen = RDB_cache_node(n,data.dsize);

      pcdb->pbuffer->printKeyValue((const char*)((ptrdiff_t)journal.map + (ptrdiff_t)RDB_cache_node(n, key.dptr)), klen,
                                   (const char*)((ptrdiff_t)journal.map + (ptrdiff_t)offset_data),                 dlen);
      }
   else if (RDB_cache_node(n,data.dsize) != U_NOT_FOUND)
      {
#  ifdef DEBUG
      ++nerror;

      U_WARNING("URDB::print1(%p,%u) - data node at offset is invalid, key: %#.*S", pcdb, offset,
                  RDB_cache_node(n,key.dsize), (const char*)((ptrdiff_t)RDB_cache_node(n,key.dptr) + (ptrdiff_t)journal.map));
#  endif

      u_put_unaligned32(n->data.dsize, U_NOT_FOUND);
      }
}

U_NO_EXPORT void URDB::getKeys1(UCDB* pcdb, uint32_t _offset) // entry present in the cache...
{
   U_TRACE(0, "URDB::getKeys1(%p,%u)", pcdb, _offset)

   URDB::cache_node* n = RDB_ptr_node(this, _offset);

   if (RDB_cache_node(n,left))  getKeys1(pcdb, RDB_cache_node(n,left));
   if (RDB_cache_node(n,right)) getKeys1(pcdb, RDB_cache_node(n,right));

   // check if node is mark for deleted (dptr == NULL)...

   uint32_t offset_data = RDB_cache_node(n,data.dptr);

   if (offset_data)
      {
      UStringRep* skey;

      U_NEW(UStringRep, skey, UStringRep((const char*)((ptrdiff_t)journal.map+(ptrdiff_t)RDB_cache_node(n,key.dptr)), RDB_cache_node(n,key.dsize)));

      if (UCDB::filter_function_to_call(skey, 0) == 0) skey->release();
      else                                             pcdb->UCDB::ptr_vector->UVector<void*>::push(skey);
      }
   else if (RDB_cache_node(n,data.dsize) != U_NOT_FOUND)
      {
#  ifdef DEBUG
      ++nerror;

      U_WARNING("URDB::getKeys1(%p,%u) - data node at offset is invalid, key: %#.*S", pcdb, offset,
                  RDB_cache_node(n,key.dsize), (const char*)((ptrdiff_t)RDB_cache_node(n,key.dptr) + (ptrdiff_t)journal.map));
#  endif

      u_put_unaligned32(n->data.dsize, U_NOT_FOUND);
      }
}

// Combines the old cdb file and the diffs in a new cdb file

bool URDB::reorganize()
{
   U_TRACE_NO_PARAM(0, "URDB::reorganize()")

   U_CHECK_MEMORY

   bool result = true;

   lock();

   U_INTERNAL_DUMP("RDB_off = %u", RDB_off(this))

   if (RDB_off(this) > sizeof(URDB::cache_struct))
      {
      U_INTERNAL_DUMP("RDB_reference = %u", RDB_reference(this))

      U_INTERNAL_ASSERT_EQUALS(RDB_reference(this), 1)

      UCDB cdb(UCDB::ignoreCase());
      char cdb_buffer_path[MAX_FILENAME_LEN];

      cdb.setPath(*(const UFile*)this, cdb_buffer_path, U_CONSTANT_TO_PARAM(".tmp"));

      result = cdb.creat(O_RDWR) &&
               cdb.ftruncate(UFile::st_size + journal.st_size + UCDB::sizeFor(4096));

      if (result)
         {
         if (cdb.memmap(PROT_READ | PROT_WRITE) == false) U_RETURN(false);

         cdb.makeStart();

         U_FOR_EACH_ENTRY(&cdb, makeAdd1, UCDB::makeAdd2)

         uint32_t pos = cdb.makeFinish(false);

         U_INTERNAL_ASSERT(pos <= cdb.st_size)

#     if defined(__CYGWIN__) || defined(_MSWINDOWS_)
         cdb.munmap(); // for ftruncate()...
#     endif

         if (cdb.ftruncate(pos) == false) U_RETURN(false);

#     if defined(_MSWINDOWS_) || defined(__CYGWIN__)
         UFile::munmap(); // for rename()...
#       ifdef   _MSWINDOWS_
         cdb.UFile::close();
#       endif
#     endif

         if (cdb._rename(UFile::path_relativ) == false) U_RETURN(false);

         reset();

#     if defined(_MSWINDOWS_) || defined(__CYGWIN__)
#       ifdef   _MSWINDOWS_
         result = cdb.UFile::open(UFile::path_relativ);
                  cdb.st_size = pos;
#       endif
         result = cdb.memmap(); // read only...
#     endif

         cdb.UFile::close();

         UFile::substitute(cdb);

         UCDB::nrecord               = cdb.nrecord;
         UCDB::start_hash_table_slot = cdb.start_hash_table_slot;

         U_INTERNAL_DUMP("UCDB::nrecord = %u RDB_nrecord = %u", UCDB::nrecord, RDB_nrecord(this))
         }
      }

   unlock();

   U_RETURN(result);
}

char* URDB::parseLine(const char* ptr, UCDB::datum* _key, UCDB::datum* _data)
{
   U_TRACE(0, "URDB::parseLine(%p,%p,%p)", ptr, _key, _data)

   U_INTERNAL_DUMP("*ptr = %C", *ptr)

   U_INTERNAL_ASSERT_EQUALS(*ptr, '+')

   _key->dsize = (*++ptr == '0' ? (++ptr, 0) : strtol(ptr, (char**)&ptr, 10));

   U_INTERNAL_ASSERT_EQUALS(*ptr,',')

   if (*++ptr == '-')
      {
      // special case: deleted key

      _data->dsize = U_NOT_FOUND;

      ptr += 2;

      U_INTERNAL_ASSERT_EQUALS(*(ptr-1),'1')
      }
   else
      {
      _data->dsize = strtol(ptr, (char**)&ptr, 10);
      }

   U_INTERNAL_ASSERT_EQUALS(*ptr,':')

   _key->dptr = (void*)++ptr;

   ptr += _key->dsize + 2;

   U_INTERNAL_ASSERT_EQUALS(*(ptr-2),'-')
   U_INTERNAL_ASSERT_EQUALS(*(ptr-1),'>')

   if (_data->dsize == U_NOT_FOUND)
      { 
      // special case: deleted key

      _data->dptr = 0;
      }
   else
      {
      _data->dptr = (void*)ptr;

      ptr += _data->dsize;
      }

   U_INTERNAL_DUMP("*ptr = %C key = %.*S data = %.*S", *ptr, _key->dsize, _key->dptr, _data->dsize, _data->dptr)

   if (*ptr == '\n') ++ptr;

   U_RETURN((char*)ptr);
}

// PRINT DATABASE

UString URDB::print()
{
   U_TRACE_NO_PARAM(0, "URDB::print()")

   U_CHECK_MEMORY

   uint32_t _size = UFile::st_size + RDB_off(this);

   U_INTERNAL_DUMP("size = %u", _size)

   if (_size)
      {
      UString buffer(_size);

      lock();

      UCDB::pbuffer = &buffer;

      U_FOR_EACH_ENTRY(this, print1, UCDB::print2)

      unlock();

      U_RETURN_STRING(buffer);
      }

   return UString::getStringNull();
}

void URDB::callForAllEntry(iPFprpr function, UVector<UString>* pvec)
{
   U_TRACE(0, "URDB::callForAllEntry(%p,%p)", function, pvec)

   lock();

   iPFprpr       function_prev = UCDB::getFunctionToCall();
   UVector<UString>* pvec_prev = UCDB::getVector();

   UCDB::setVector(pvec);
   UCDB::setFunctionToCall(function);

   U_FOR_EACH_ENTRY(this, call1, UCDB::call2)

   UCDB::setVector(pvec_prev);
   UCDB::setFunctionToCall(function_prev);

   unlock();
}

void URDB::callForAllEntryDelete(iPFprpr function)
{
   U_TRACE(0, "URDB::callForAllEntryDelete(%p)", function)

   UStringRep* _key;
   UVector<UString> vec(URDB::size() / 2);

   callForAllEntry(function, &vec);

   for (int32_t i = 0, n = vec.size(); i < n; i += 2)
      {
      _key = vec.UVector<UStringRep*>::at(i);

      UCDB::setKey(_key);

      (void) remove();
      }
}

void URDB::getKeys(UVector<UString>& vec)
{
   U_TRACE(0, "URDB::getKeys(%p)", &vec)

   lock();

   UVector<UString>* pvec_prev = UCDB::getVector();

   UCDB::setVector(&vec);

   U_FOR_EACH_ENTRY(this, getKeys1, UCDB::getKeys2)

   UCDB::setVector(pvec_prev);

   unlock();
}

void URDB::callForAllEntrySorted(iPFprpr function, qcompare compare_obj)
{
   U_TRACE(0, "URDB::callForAllEntrySorted(%p,%p)", function, compare_obj)

   uint32_t n = size();

   if (n)
      {
      uint32_t i;
      UVector<UString> vkey(n);

      getKeys(vkey);

      U_ASSERT_EQUALS(n, vkey.size())

      n = vkey.size();

      if (n > 1)
         {
         if (compare_obj == 0) vkey.sort(UCDB::ignoreCase());
         else                  vkey.UVector<void*>::sort(compare_obj);
         }

      lock();

      UCDB::setFunctionToCall(function);
      UCDB::resetFilterToFunctionToCall();

      for (i = 0; i < n; ++i)
         {
         UStringRep* r = vkey.UVector<UStringRep*>::at(i);

         UCDB::setKey(r);

         UCDB::cdb_hash();

         if (_fetch())
            {
            UCDB::call1();

            if (U_cdb_result_call(this) == 0) break;
            }
         }

      UCDB::setFunctionToCall(0);

      unlock();
      }
}

UString URDB::printSorted()
{
   U_TRACE_NO_PARAM(0, "URDB::printSorted()")

   U_CHECK_MEMORY

   uint32_t _size = UFile::st_size + RDB_off(this);

   U_INTERNAL_DUMP("_size = %u", _size)

   if (_size)
      {
      uint32_t n = size();

      if (n)
         {
         UVector<UString> vkey(n);

         getKeys(vkey);

         if (n > 1) vkey.sort(UCDB::ignoreCase());

         UString buffer(_size);

         for (uint32_t i = 0; i < n; ++i)
            {
            UStringRep* r = vkey.UVector<UStringRep*>::at(i);

            UCDB::setKey(r);

            UCDB::cdb_hash();

            if (_fetch()) buffer.printKeyValue((const char*)UCDB::key.dptr, UCDB::key.dsize, (const char*)UCDB::data.dptr, UCDB::data.dsize);
            }

         U_RETURN_STRING(buffer);
         }
      }

   return UString::getStringNull();
}

bool URDB::isDeleted()
{
   U_TRACE_NO_PARAM(0, "URDB::isDeleted()")

   U_CHECK_MEMORY

   bool result = (RDB_node_data_pr(this) == 0);

   if (result &&
       RDB_node_data_sz(this) != U_NOT_FOUND)
      {
#  ifdef DEBUG
      ++nerror;

      U_WARNING("URDB::isDeleted() - data node is invalid, key: %#.*S", RDB_node_key_sz(this), RDB_node_key_pr(this));
#  endif

      u_put_unaligned32(RDB_node(this)->data.dsize, U_NOT_FOUND);
      }

   U_RETURN(result);
}

// SERVICES

bool URDB::_fetch()
{
   U_TRACE_NO_PARAM(0, "URDB::_fetch()")

   bool result;

   // Search one key/data pair in the cache or in the cdb

   if (htLookup(this) == false)
      {
      U_INTERNAL_ASSERT_EQUALS(node, 0)

      result = cdbLookup();
      }
   else
      {
      if (isDeleted()) result = false;
      else
         {
         result           = true;
         UCDB::data.dptr  = RDB_node_data(this);
         UCDB::data.dsize = RDB_node_data_sz(this);
         }
      }

   U_RETURN(result);
}

// --------------------------------------------------------------------
// Fetch the value for a given key from the database.
// --------------------------------------------------------------------
// If the lookup failed, datum is NULL
// If the lookup succeeded, datum points to the value from the database
// --------------------------------------------------------------------

bool URDB::fetch()
{
   U_TRACE_NO_PARAM(0, "URDB::fetch()")

   bool result;

   lock();

   UCDB::cdb_hash();

   // Search one key/data pair in the cache or in the cdb

   result = _fetch();

   unlock();

   U_RETURN(result);
}

UString URDB::at()
{
   U_TRACE_NO_PARAM(0, "URDB::at()")

   UString result;

   lock();

   UCDB::cdb_hash();

   if (_fetch()) result = UCDB::elem();

   unlock();

   U_RETURN_STRING(result);
}

int URDB::_store(int _flag, bool exist)
{
   U_TRACE(0, "URDB::_store(%d,%b)", _flag, exist)

   int result = 0;

   if (exist)
      {
      if (_flag == RDB_INSERT) // Insertion of new entries only
         {
         if (isDeleted()) exist = false;
         else
            {
            U_RETURN(-1); // -1: flag was RDB_INSERT and this key already existed
            }
         }
      }
   else
      {
      UCDB::datum data_new = UCDB::data;

      U_INTERNAL_ASSERT_EQUALS(node, 0)

      exist = cdbLookup(); // Search one key/data pair in the cdb

      if (exist)
         {
         if (_flag == RDB_INSERT) // Insertion of new entries only
            {
            U_RETURN(-1); // -1: flag was RDB_INSERT and this key already existed
            }
         }
      else if (_flag == RDB_INSERT_WITH_PADDING)
         {
         char* ptr = (char*)data_new.dptr + data_new.dsize;

         u_put_unalignedp64(ptr,    U_MULTICHAR_CONSTANT64(' ',' ',' ',' ',' ',' ',' ',' '));
         u_put_unalignedp64(ptr+8,  U_MULTICHAR_CONSTANT64(' ',' ',' ',' ',' ',' ',' ',' '));
         u_put_unalignedp64(ptr+16, U_MULTICHAR_CONSTANT64(' ',' ',' ',' ',' ',' ',' ',' '));
         u_put_unalignedp64(ptr+24, U_MULTICHAR_CONSTANT64(' ',' ',' ',' ',' ',' ',' ',' '));

         data_new.dsize += 32;

#     ifdef DEBUG
         if (data_new.dptr == u_buffer &&
             data_new.dsize > U_BUFFER_SIZE)
            {
            U_ERROR("URDB::store(%d) - overflow on data buffer (%u > %u) - db(%.*S,%u recs)", _flag, data_new.dsize, U_BUFFER_SIZE, U_FILE_TO_TRACE(*this), RDB_nrecord(this));
            }
#     endif
         }

      htAlloc(this);

      UCDB::data = data_new;
      }

   U_INTERNAL_ASSERT_EQUALS(result, 0)

   if (logJournal(1) == false)
      {
      htRemoveAlloc(this);

      U_RETURN(-3); // -3: there is not enough (virtual) memory available on writing journal
      }

   // NB: the reference at memory in the cache data must point to memory mapped...

   htInsert(this); // Insertion of new entry in the cache

   if (exist == false) RDB_nrecord(this)++;

   U_INTERNAL_DUMP("nrecord = %u", RDB_nrecord(this))

   U_RETURN(result);
}

// ---------------------------------------------------------------------
// Write a key/value pair to a reliable database
// ---------------------------------------------------------------------
// RETURN VALUE
// ---------------------------------------------------------------------
//  0: Everything was OK
// -1: flag was RDB_INSERT and this key already existed
// -3: there is not enough (virtual) memory available on writing journal
// ---------------------------------------------------------------------

int URDB::store(int _flag)
{
   U_TRACE(0, "URDB::store(%d)", _flag)

   int result;

   lock();

   UCDB::cdb_hash();

   result = _store(_flag, htLookup(this));

   unlock();

   U_RETURN(result);
}

// ----------------------------------------------------------------------
// Mark a key/value as deleted
// ----------------------------------------------------------------------
// RETURN VALUE
// ----------------------------------------------------------------------
//  0: Everything was OK
// -1: The entry was not in the database
// -2: The entry was already marked deleted in the cache
// -3: there is not enough (virtual) memory available on writing journal
// ----------------------------------------------------------------------

int URDB::remove()
{
   U_TRACE_NO_PARAM(0, "URDB::remove()")

   int result = 0;
   bool record_cache_deleted = false;

   lock();

   UCDB::cdb_hash();

   // NB: Because the insertion routine has to know where to insert the cache_node, we need to call anyway htLookup()...  

   if (htLookup(this)) // Search one key/data pair in the cache
      {
      if (isDeleted())
         {
         result = -2; // -2: The entry was already marked deleted in the cache

         goto end;
         }

      record_cache_deleted = true;
      }
   else
      {
      // Search one key/data pair in the cdb

      if (cdbLookup()) htAlloc(this);
      else
         {
         result = -1; // -1: The entry was not in the database

         goto end;
         }
      }

   U_INTERNAL_ASSERT_EQUALS(result, 0)

   if (logJournal(0) == false)
      {
      result = -3; // -3: there is not enough (virtual) memory available on writing journal

      htRemoveAlloc(this);

      goto end;
      }

   // NB: the reference at memory in the cache data must point to memory mapped...

   UCDB::data.dptr  = 0;
   UCDB::data.dsize = U_NOT_FOUND;

   htInsert(this); // Insertion or update of new entry of the cache

   if (record_cache_deleted) RDB_nrecord(this)--;

   U_INTERNAL_DUMP("nrecord = %u", RDB_nrecord(this))

end:
   unlock();

   U_RETURN(result);
}

// inlining failed in call to 'URDB::remove(UString const&)': call is unlikely and code size would grow

int URDB::remove(const UString& _key)
{
   U_TRACE(0, "URDB::remove(%V)", _key.rep)

   UCDB::setKey(_key);

   int result = remove();

   U_RETURN(result);
}

// ---------------------------------------------------------------------
// Substitute a key/value with a new key/value (remove+store)
// ---------------------------------------------------------------------
// RETURN VALUE
// ---------------------------------------------------------------------
//  0: Everything was OK
// -1: The entry was not in the database
// -2: The entry was marked deleted in the cache 
// -3: there is not enough (virtual) memory available on writing journal
// -4: flag was RDB_INSERT and the new key already existed
// ---------------------------------------------------------------------

int URDB::substitute(UCDB::datum* key2, int _flag)
{
   U_TRACE(0, "URDB::substitute(%p,%d)", key2, _flag)

   int result        = 0;
   UCDB::datum data2 = UCDB::data;

   lock();

   UCDB::cdb_hash();

   key1 = UCDB::key;

   // search for remove

   if (htLookup(this)) // Search one key/data pair in the cache
      {
      if (isDeleted()) result = -2; // -2: The entry was already marked deleted in the cache
      }
   else
      {
      // Search one key/data pair in the cdb

      U_INTERNAL_ASSERT_EQUALS(node, 0)

      if (cdbLookup() == false) result = -1; // -1: The entry was not in the database
      }

   if (result == 0)
      {
      // save cache pointer

      uint32_t   node1 =  node;
      uint32_t* pnode1 = pnode;

      // search for store

      UCDB::key = *key2;

      UCDB::cdb_hash();

      if (htLookup(this)) // Search one key/data pair in the cache
         {
         if (_flag == RDB_INSERT && // Insertion of new entries only
             isDeleted() == false)
            {
            result = -4; // -4: flag was RDB_INSERT and this key already existed
            }
         }
      else
         {
         U_INTERNAL_ASSERT_EQUALS(node, 0)

         if (cdbLookup() &&       // Search one key/data pair in the cdb
             _flag == RDB_INSERT) // Insertion of new entries only
            {
            result = -4; // -4: flag was RDB_INSERT and this key already existed
            }
         else
            {
            U_INTERNAL_ASSERT_EQUALS(result,0)

            htAlloc(this);
            }
         }

      if (result == 0) // ok, substitute
         {
         UCDB::data = data2;

         if (logJournal(2) == false)
            {
            result = -3; // -3: disk full writing to the journal file

            htRemoveAlloc(this);

            goto end;
            }

         // NB: the reference at memory in the cache data must point to memory mapped...

         htInsert(this); // Insertion of new entry in the cache

         // remove of old entry

         UCDB::key        = key1;
         UCDB::data.dptr  = 0;
         UCDB::data.dsize = U_NOT_FOUND;

                   pnode = pnode1;
         if (node1) node =  node1;
         else       htAlloc(this);

         htInsert(this); // Insertion or update of new entry in the cache
         }
      }

end:
   unlock();

   U_RETURN(result);
}

// inlining failed in call to ...: call is unlikely and code size would grow

bool URDB::find(const char* _key, uint32_t keylen)
{
   U_TRACE(0, "URDB::find(%.*S,%u)", keylen, _key, keylen)

   bool result;

   lock();

   UCDB::setKey(_key, keylen);

   UCDB::cdb_hash();

   result = _fetch(); // Fetch the value for a given key from the database

   unlock();

   U_RETURN(result);
}

int URDB::store(const char* _key, uint32_t keylen, const char* _data, uint32_t datalen, int _flag)
{
   U_TRACE(0, "URDB::store(%.*S,%u,%.*S,%u,%d)", keylen, _key, keylen, datalen, _data, datalen, _flag)

   lock();

   UCDB::setKey(  _key,  keylen);
   UCDB::setData(_data, datalen);

   UCDB::cdb_hash();

   int result = _store(_flag, htLookup(this));

   unlock();

   U_RETURN(result);
}

int URDB::substitute(const UString& _key, const UString& new_key, const UString& _data, int _flag)
{
   U_TRACE(0, "URDB::substitute(%V,%V,%V,%d)", _key.rep, new_key.rep, _data.rep, _flag)

   UCDB::setKey(_key);
   UCDB::setData(_data);
   UCDB::datum key2 = { (void*) new_key.data(), new_key.size() };

   int result = substitute(&key2, _flag);

   U_RETURN(result);
}

bool URDBObjectHandler<UDataStorage*>::getDataStorage()
{
   U_TRACE(0, "URDBObjectHandler<UDataStorage*>::getDataStorage()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(pDataStorage)
   U_ASSERT(pDataStorage->isDataSession())

   pDataStorage->clear();

   recval    = URDB::at(pDataStorage->keyid);
   brecfound = (recval.empty() == false);

   if (brecfound)
      {
      pDataStorage->fromData(U_STRING_TO_PARAM(recval));

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool     URDBObjectHandler<UDataStorage*>::bsetEntry;
char*    URDBObjectHandler<UDataStorage*>::data_buffer;
uint32_t URDBObjectHandler<UDataStorage*>::data_len;
iPFpvpv  URDBObjectHandler<UDataStorage*>::ds_function_to_call;

bool URDBObjectHandler<UDataStorage*>::_insertDataStorage(int op)
{
   U_TRACE(0, "URDBObjectHandler<UDataStorage*>::_insertDataStorage(%d)", op)

   U_INTERNAL_ASSERT_POINTER(pDataStorage)
   U_ASSERT(pDataStorage->isDataSession())

   lock();

   UCDB::setKey(pDataStorage->keyid);
   UCDB::setData(data_buffer, data_len);

   UCDB::cdb_hash();

   int result = URDB::_store(op, htLookup(this));

   if (result == 0)
      {
      recval = UCDB::elem(); // NB: in this way now we have the direct reference to mmap memory for the record...

#  ifdef DEBUG
      char* ptr   = recval.data();
      uint32_t sz = recval.size();

      if (sz > 40)       sz = 40;
      if (sz > data_len) sz = data_len;

      if (strncmp(ptr, data_buffer, sz))
         {
         ++nerror;

         U_WARNING("URDB store: db(%.*S) op(%d) data(%u) - to: %.*S from: %.*S", U_FILE_TO_TRACE(*this), op, data_len, sz, ptr, sz, data_buffer);
         }
#  endif
      }

   unlock();

   u_buffer_len = 0;

   if (result)
      {
      U_WARNING("Store data with op %d on db %.*S failed with error %d", op, U_FILE_TO_TRACE(*this), result);

      U_RETURN(false);
      }

   U_RETURN(true);
}

bool URDBObjectHandler<UDataStorage*>::putDataStorage()
{
   U_TRACE_NO_PARAM(0, "URDBObjectHandler<UDataStorage*>::putDataStorage()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(pDataStorage)

   char* ptr   = recval.data();
   uint32_t sz = recval.size();

   data_buffer = pDataStorage->toBuffer();
   data_len    = UDataStorage::buffer_len;

   U_INTERNAL_DUMP("new_rec_size(%u) = %.*S old_rec_size(%u) = %.*S", data_len, data_len, data_buffer, sz, sz, ptr)

   if (data_len <= sz)
      {
      U_MEMCPY(ptr, data_buffer, data_len);

      sz -= data_len;

      if (sz) (void) memset(ptr + data_len, ' ', sz);

      u_buffer_len = 0;

      U_RETURN(true);
      }

   return _insertDataStorage(RDB_INSERT_WITH_PADDING);
}

void URDBObjectHandler<UDataStorage*>::setEntry(UStringRep* _key, UStringRep* _data)
{
   U_TRACE(0, "URDBObjectHandler<UDataStorage*>::setEntry(%V,%V)", _key, _data)

   U_INTERNAL_ASSERT_POINTER(pDataStorage)

   pDataStorage->clear();

   // NB: in this way now we have the direct reference to mmap memory for the data record and the key...

                recval._assign(_data);
   pDataStorage->keyid._assign(_key);

   pDataStorage->fromData(U_STRING_TO_PARAM(recval));
}

URDBObjectHandler<UDataStorage*>* URDBObjectHandler<UDataStorage*>::pthis;

int URDBObjectHandler<UDataStorage*>::callEntryCheck(UStringRep* _key, UStringRep* _data)
{
   U_TRACE(0, "URDBObjectHandler<UDataStorage*>::callEntryCheck(%V,%V)", _key, _data)

   U_INTERNAL_ASSERT_POINTER(pthis)
   U_INTERNAL_ASSERT_POINTER(ds_function_to_call)
   U_INTERNAL_ASSERT_POINTER(pthis->UCDB::filter_function_to_call)

   if (pthis->UCDB::filter_function_to_call(_key, _data))
      {
      if (bsetEntry)
         {
         pthis->setEntry(_key, _data);

         (void) ds_function_to_call(_key, _data);

         U_RETURN(1);
         }

      int result = ds_function_to_call(_key, _data);

      if (result == 4) // NB: call function later after set record value from db with setEntry()...
         {
         pthis->setEntry(_key, _data);

         result = ds_function_to_call(0, 0);

         if (result == 3) // NB: call function later without lock on db...
            {
            pthis->UCDB::addEntryToVector();

            U_RETURN(1);
            }
         }

      U_RETURN(result);
      }

   U_RETURN(1);
}

void URDBObjectHandler<UDataStorage*>::callForAllEntry(iPFprpr function, vPF function_no_lock, qcompare compare_obj)
{
   U_TRACE(0, "URDBObjectHandler<UDataStorage*>::callForAllEntry(%p,%p,%p)", function, function_no_lock, compare_obj)

   brecfound = false;

   uint32_t n = URDB::size();

   if (n == 0) return;

   iPFpvpv             ds_function_to_call_prev = ds_function_to_call;
   URDBObjectHandler<UDataStorage*>* pthis_prev = pthis;

   pthis               = this;
   brecfound           = true;
   ds_function_to_call = (iPFpvpv)function;

   if (compare_obj      == 0 &&
       function_no_lock == 0)
      {
      URDB::callForAllEntry(callEntryCheck, 0);
      }
   else
      {
      int i;
      UVector<UString> vec(n);

      if (compare_obj == 0)
         {
         U_INTERNAL_ASSERT_POINTER(function_no_lock)

         URDB::callForAllEntry(callEntryCheck, &vec);
         }
      else
         {
         UVector<UString> vkey(n);
         UVector<UString>* pvec_prev = 0;
         iPFprpr function_prev = UCDB::getFunctionToCall();

         getKeys(vkey);

         n = vkey.size();

         if (n > 1)
            {
            if (compare_obj == (qcompare)-1) vkey.sort(UCDB::ignoreCase());
            else                             vkey.UVector<void*>::sort(compare_obj);
            }

         lock();

         UCDB::resetFilterToFunctionToCall();
         UCDB::setFunctionToCall(callEntryCheck);

         if (function_no_lock)
            {
            pvec_prev = UCDB::getVector();
                        UCDB::setVector(&vec);
            }

         for (i = 0; i < (int)n; ++i)
            {
            UStringRep* r = vkey.UVector<UStringRep*>::at(i);

            UCDB::setKey(r);

            UCDB::cdb_hash();

            if (_fetch())
               {
               UCDB::call1();

               if (U_cdb_result_call(this) == 0) break;
               }
            }

         UCDB::setFunctionToCall(function_prev);

         if (function_no_lock) UCDB::setVector(pvec_prev);

         unlock();
         }

      if ((n = vec.size()))
         {
         for (i = 0; i < (int)n; i += 2)
            {
            UStringRep*  _key = vec.UVector<UStringRep*>::at(i);
            UStringRep* _data = vec.UVector<UStringRep*>::at(i+1);

            pthis->setEntry(_key, _data);

            int result = ds_function_to_call((void*)-1, (void*)function_no_lock);

            if (result == 2) // remove
               {
               UCDB::setKey(_key);

               (void) URDB::remove();
               }
            }
         }
      }

   pthis               =               pthis_prev;
   ds_function_to_call = ds_function_to_call_prev;
}

void URDBObjectHandler<UDataStorage*>::close()
{
   U_TRACE_NO_PARAM(0, "URDBObjectHandler<UDataStorage*>::close()")

   U_CHECK_MEMORY

   recval.clear();

   if (pDataStorage)
      {
      pDataStorage->clear();
      pDataStorage->resetDataSession();
      }

   if (UFile::st_size == 0) (void) URDB::close(true); // NB: we have only the journal...
   else
      {
      if (UServer_Base::bssl) (void) URDB::close(false);
      else
         {
         /*
         UString x = URDB::print();
         (void) UFile::writeToTmp(U_STRING_TO_PARAM(x), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("%.*s.end"), U_FILE_TO_TRACE(*this));
         */

         (void) URDB::closeReorganize();

         /*
         char buffer[1024];
         (void) u__snprintf(U_CONSTANT_TO_PARAM("/tmp/%.*s.init"), U_FILE_TO_TRACE(*this))
         (void) UFile::_unlink(buffer);
         */
         }
      }
}

// STREAM

#ifdef U_STDCPP_ENABLE
U_EXPORT ostream& operator<<(ostream& os, URDB& rdb)
{
   U_TRACE(0+256, "URDB::operator<<(%p,%p)", &os, &rdb)

   UString text = rdb.print();

   (void) os.write(text.data(), text.size());

   os.put('\n');

   return os;
}

// DEBUG

#  ifdef DEBUG
const char* URDB::dump(bool _reset) const
{
   UCDB::dump(false);

   *UObjectIO::os << "\n"
                  << "node                      " << node            << '\n'
                  << "pnode                     " << (void*)pnode    << '\n'
                  << "_lock   (ULock            " << (void*)&_lock   << ")\n"
                  << "journal (UFile            " << (void*)&journal << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* URDBObjectHandler<UDataStorage*>::dump(bool _reset) const
{
   URDB::dump(false);

   *UObjectIO::os << "\n"
                  << "brecfound                 " << brecfound                   << '\n'
                  << "pDataStorage              " << (void*)pDataStorage         << '\n'
                  << "ds_function_to_call       " << (void*)ds_function_to_call  << '\n'
                  << "recval (UString           " << (void*)&recval              << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
