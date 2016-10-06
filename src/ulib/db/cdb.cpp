// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    cdb.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/db/cdb.h>
#include <ulib/utility/services.h>

void UCDB::init_internal(int ignore_case)
{
   U_TRACE(0, "UCDB::init_internal(%d)", ignore_case)

   (void) U_SYSCALL(memset, "%p,%d,%d",  &key, 0, sizeof(datum));
   (void) U_SYSCALL(memset, "%p,%d,%d", &data, 0, sizeof(datum));

   hr   = 0;
   slot = 0;
   hp   = 0;

   pattern                 = 0;
   pbuffer                 = 0;
   ptr_vector              = 0;
   function_to_call        = 0;
   filter_function_to_call = functionCall;

   // when mmap not available we use this storage...

   (void) U_SYSCALL(memset, "%p,%d,%d",   &hp_buf, 0, sizeof(hp_buf));
   (void) U_SYSCALL(memset, "%p,%d,%d",   &hr_buf, 0, sizeof(cdb_record_header));
   (void) U_SYSCALL(memset, "%p,%d,%d", &slot_buf, 0, sizeof(cdb_hash_table_slot));

   loop = nslot = khash = nrecord = offset = start_hash_table_slot = 0;

   union uucdb_flag {
      unsigned char* pc;
      uint32_t*      pi;
   };

   union uucdb_flag u = { (unsigned char*)&flag };

   *u.pi = 0x00000000;

   U_cdb_ignore_case(this) = ignore_case;

   flag[1] = flag[2] = flag[3] = 0;
}

bool UCDB::open(bool brdonly)
{
   U_TRACE(0, "UCDB::open(%b)", brdonly)

   nrecord = start_hash_table_slot = 0;

   if (UFile::isOpen() ||
       UFile::open(brdonly ? O_RDONLY : O_CREAT | O_RDWR))
      {
      UFile::readSize();

      if (UFile::st_size)
         {
         (void) UFile::memmap(PROT_READ | (brdonly ? 0 : PROT_WRITE));

         if (UFile::map == MAP_FAILED)
            {
            data.dptr = 0;

            hp   =   &hp_buf;
            hr   =   &hr_buf;
            slot = &slot_buf;

            (void) UFile::pread(&start_hash_table_slot, sizeof(uint32_t), 0);
            }
         else
            {
            UFile::close();

            start_hash_table_slot = *(uint32_t*)UFile::map;
            }

         nrecord = (UFile::st_size - start_hash_table_slot) / sizeof(cdb_hash_table_slot);
         }

      U_INTERNAL_DUMP("nrecord = %u", nrecord)

#  ifdef DEBUG
      if (UFile::st_size) checkForAllEntry();
#  endif

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UCDB::find()
{
   U_TRACE_NO_PARAM(0, "UCDB::find()")

   U_INTERNAL_ASSERT_MAJOR(UFile::st_size, 0)

   // A record is located as follows. Compute the hash value of the key in the record.
   // The hash value modulo CDB_NUM_HASH_TABLE_POINTER is the number of a hash table

   offset = (khash % CDB_NUM_HASH_TABLE_POINTER) * sizeof(cdb_hash_table_pointer);

   if (UFile::map == MAP_FAILED) (void) UFile::pread(&hp_buf, sizeof(cdb_hash_table_pointer), offset);
   else                          hp = (cdb_hash_table_pointer*) (UFile::map + offset);

   U_INTERNAL_DUMP("hp[%d] = { %u, %u }", offset / sizeof(cdb_hash_table_pointer), hp->pos, hp->slots)

   if (hp->slots)
      {
      // The hash value divided by CDB_NUM_HASH_TABLE_POINTER, modulo the length of that table, is a slot number

      nslot  = (khash / CDB_NUM_HASH_TABLE_POINTER) % hp->slots;
      offset = hp->pos + (nslot * sizeof(cdb_hash_table_slot));

      if (UFile::map == MAP_FAILED) (void) UFile::pread(&slot_buf, sizeof(cdb_hash_table_slot), offset);
      else                          slot = (cdb_hash_table_slot*) (UFile::map + offset);

      U_INTERNAL_DUMP("slot[%d] = { %u, %u }", nslot, u_get_unaligned32(slot->hash), u_get_unaligned32(slot->pos))

      // Each hash table slot states a hash value and a byte position.
      // If the byte position is 0, the slot is empty.
      // Otherwise, the slot points to a record whose key has that hash value

      if (u_get_unaligned32(slot->pos))
         {
         loop = 0;

         return findNext();
         }
      }

   U_RETURN(false);
}

U_NO_EXPORT inline bool UCDB::match(uint32_t pos)
{
   U_TRACE(0, "UCDB::match(%u)", pos)

   U_CHECK_MEMORY

   U_INTERNAL_DUMP("UFile::map = %p", UFile::map)

   if (UFile::map == MAP_FAILED)
      {
      char _buf[32];
      uint32_t len = key.dsize;
      char* pkey = (char*) key.dptr;

      pos += sizeof(cdb_record_header);

      while (len)
         {
         uint32_t n = U_min(len, sizeof(_buf));

         (void) UFile::pread(_buf, n, pos);

         if (u_equal(pkey, _buf, n, ignoreCase())) U_RETURN(false);

         pkey += n;
         pos  += n;
         len  -= n;
         }

      if (data.dptr) U_SYSCALL_VOID(free, "%p", data.dptr); // free old data...

      data.dsize = u_get_unaligned32(hr->dlen);
      data.dptr  = U_SYSCALL(malloc, "%lu", data.dsize);

      (void) UFile::pread(data.dptr, data.dsize, pos);

      U_RETURN(true);
      }

   data.dsize = u_get_unaligned32(hr->dlen);

   if (u_equal(key.dptr, ++hr, key.dsize, ignoreCase()) == 0)
      {
      data.dptr = (char*)hr + key.dsize;

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UCDB::findNext()
{
   U_TRACE_NO_PARAM(0, "UCDB::findNext()")

   uint32_t pos;

   // Probe that slot, the next higher slot, and so on, until you find the record or run into an empty slot

   while (++loop <= hp->slots)
      {
      U_INTERNAL_DUMP("loop = %u", loop)

      if (loop > 1)
         {
         // handles repeated keys...

         if (++nslot == hp->slots)
            {
            nslot = 0;

            if (UFile::map == MAP_FAILED) (void) UFile::pread(&slot_buf, sizeof(cdb_hash_table_slot), hp->pos);
            else                          slot = (cdb_hash_table_slot*) (UFile::map + hp->pos);
            }
         else
            {
            if (UFile::map != MAP_FAILED) ++slot;
            else
               {
               offset += sizeof(cdb_hash_table_slot);

               (void) UFile::pread(&slot_buf, sizeof(cdb_hash_table_slot), offset);
               }
            }
         }

      // Each hash table slot states a hash value and a byte position. If the
      // byte position is 0, the slot is empty. Otherwise, the slot points to
      // a record whose key has that hash value

      pos = u_get_unaligned32(slot->pos);

      U_INTERNAL_DUMP("slot[%d] = { %u, %u }", nslot, u_get_unaligned32(slot->hash), pos)

      if (pos == 0) break;

      if (u_get_unaligned32(slot->hash) == khash)
         {
         // Records are stored sequentially, without special alignment.
         // A record states a key length, a data length, the key, and the data

         if (UFile::map == MAP_FAILED) (void) UFile::pread(&hr_buf, sizeof(cdb_record_header), pos);
         else                          hr = (cdb_record_header*)(UFile::map + pos);

         U_INTERNAL_DUMP("hr = { %u, %u }", u_get_unaligned32(hr->klen), u_get_unaligned32(hr->dlen))

         if (u_get_unaligned32(hr->klen) == key.dsize && match(pos)) U_RETURN(true);
         }
      }

   U_RETURN(false);
}

UString UCDB::at()
{
   U_TRACE_NO_PARAM(0, "UCDB::at()")

   cdb_hash();

   if (find()) return elem();

   return UString::getStringNull();
}

// FOR RDB

uint32_t UCDB::makeFinish(bool _reset)
{
   U_TRACE(1+256, "UCDB::makeFinish(%b)", _reset)

   U_INTERNAL_ASSERT_DIFFERS(UFile::map, MAP_FAILED)

   // Each of the CDB_NUM_HASH_TABLE_POINTER initial pointers states a position and a length.
   // The position is the starting byte position of the hash table.
   // The length is the number of slots in the hash table

   char* eod = (char*)hr; // END OF DATA (eod) -> start of hash table slot...

   start_hash_table_slot = eod - UFile::map;

   uint32_t pos = start_hash_table_slot;

   U_INTERNAL_DUMP("nrecord = %u", nrecord)

   if (nrecord > 0)
      {
      uint32_t i;

      struct cdb_tmp {
         uint32_t hash;
         uint32_t pos;
         uint32_t index;
      };

      cdb_tmp*  tmp = (cdb_tmp*) UMemoryPool::_malloc(nrecord, sizeof(cdb_tmp));
      cdb_tmp* ptmp = tmp;

      cdb_hash_table_slot* pslot;

      hp = (cdb_hash_table_pointer*) UFile::map;

      (void) U_SYSCALL(memset, "%p,%d,%u", hp, 0, CDB_NUM_HASH_TABLE_POINTER * sizeof(cdb_hash_table_pointer));

      for (char* ptr = start(); ptr < eod; ++ptmp)
         {
         ptmp->pos = (ptr - UFile::map);

         hr = (cdb_record_header*)ptr;

         ptr += sizeof(UCDB::cdb_record_header);

         // The hash value modulo CDB_NUM_HASH_TABLE_POINTER is the number of a hash table

         uint32_t klen = u_get_unaligned32(hr->klen);
         ptmp->hash    = cdb_hash(ptr, klen);
         ptmp->index   = ptmp->hash % CDB_NUM_HASH_TABLE_POINTER;

         /*
         U_INTERNAL_DUMP("pos   = %u", ptmp->pos)
         U_INTERNAL_DUMP("index = %u", ptmp->index)
         */

         U_INTERNAL_ASSERT_MINOR(ptmp->index, CDB_NUM_HASH_TABLE_POINTER)

         hp[ptmp->index].slots++;

         ptr += klen + u_get_unaligned32(hr->dlen);
         }

      for (i = 0; i < CDB_NUM_HASH_TABLE_POINTER; ++i)
         {
         hp[i].pos = pos;

         pos += hp[i].slots * sizeof(cdb_hash_table_slot);

         /*
#     ifdef DEBUG
         if (hp[i].slots) U_INTERNAL_DUMP("hp[%3d] = { %u, %u }", i, hp[i].pos, hp[i].slots)
#     endif
         */

         U_INTERNAL_ASSERT(pos <= (uint32_t)st_size)
         }

      U_INTERNAL_DUMP("nrecord = %u num_hash_slot = %u", nrecord, (pos - start_hash_table_slot) / sizeof(cdb_hash_table_slot))

      if (_reset) (void) U_SYSCALL(memset, "%p,%d,%u", eod, 0, pos - start_hash_table_slot);

      for (i = 0; i < nrecord; ++i)
         {
         slot = (cdb_hash_table_slot*)(UFile::map + hp[tmp[i].index].pos);

      // U_INTERNAL_DUMP("slot = %u", (char*)slot - UFile::map)

         // The hash value divided by CDB_NUM_HASH_TABLE_POINTER, modulo the length of that table, is a slot number

         nslot = (tmp[i].hash / CDB_NUM_HASH_TABLE_POINTER) % hp[tmp[i].index].slots;

      // U_INTERNAL_DUMP("nslot = %u", nslot)

         // handles repeated keys...

         while (true)
            {
            pslot = slot + nslot;

            if (u_get_unaligned32(pslot->pos) == 0) break;

            if (++nslot == hp[tmp[i].index].slots) nslot = 0;
            }

         u_put_unaligned32(pslot->hash, tmp[i].hash);
         u_put_unaligned32(pslot->pos,  tmp[i].pos);

      // U_INTERNAL_DUMP("slot[%u] = { %u, %u }", nslot, tmp[i].hash, tmp[i].pos)
         }

      UMemoryPool::_free(tmp, nrecord, sizeof(cdb_tmp));
      }

   U_RETURN(pos);
}

// Call function for all entry

void UCDB::callForAllEntry(vPFpvpc function)
{
   U_TRACE(0, "UCDB::callForAllEntry(%p)", function)

   U_INTERNAL_DUMP("nrecord = %u", nrecord)

   U_INTERNAL_ASSERT_MAJOR(UFile::st_size,0)
   U_INTERNAL_ASSERT_DIFFERS(UFile::map, MAP_FAILED)

   char* ptr  = start();
   char* _end = UCDB::end();

   U_INTERNAL_DUMP("ptr = %p end = %p", ptr, _end)

   U_INTERNAL_ASSERT_MINOR(ptr,_end)

   while (ptr < _end)
      {
      hr = (UCDB::cdb_record_header*) ptr;

      U_INTERNAL_DUMP("hr(%p) = { %u, %u } key = %.*S", hr, u_get_unaligned32(hr->klen), u_get_unaligned32(hr->dlen),
                                                            u_get_unaligned32(hr->klen), ptr+sizeof(UCDB::cdb_record_header))

      function(this, ptr);

      ptr += sizeof(UCDB::cdb_record_header) + u_get_unaligned32(hr->klen) + u_get_unaligned32(hr->dlen);
      }
}

void UCDB::callForAllEntrySorted(iPFprpr function)
{
   U_TRACE(0, "UCDB::callForAllEntrySorted(%p)", function)

   U_INTERNAL_DUMP("nrecord = %u", nrecord)

   U_INTERNAL_ASSERT_MAJOR(UFile::st_size,0)
   U_INTERNAL_ASSERT_DIFFERS(UFile::map, MAP_FAILED)

   UStringRep* r;
   uint32_t n = size();
   UVector<UString> vkey(n);

   ptr_vector = &vkey;

   callForAllEntry((vPFpvpc)getKeys2);

   U_ASSERT_EQUALS(n, vkey.size())

   if (n > 1) vkey.sort(ignoreCase());

   function_to_call = function;

   for (uint32_t i = 0; i < n; ++i)
      {
      r = vkey.UVector<UStringRep*>::at(i);

      setKey(r);

      cdb_hash();

      if (find())
         {
         call1();

         if (U_cdb_result_call(this) == 0) break;
         }
      }

   function_to_call = 0;
}

void UCDB::callForAllEntryWithPattern(iPFprpr function, UString* _pattern)
{
   U_TRACE(1, "UCDB::callForAllEntryWithPattern(%p,%p)", function, _pattern)

   U_INTERNAL_ASSERT_MAJOR(UFile::st_size,0)
   U_INTERNAL_ASSERT_DIFFERS(UFile::map, MAP_FAILED)

   char* ptr  = start();
   char* _end = end();

   U_INTERNAL_DUMP("ptr = %p _end = %p", ptr, _end)

   U_INTERNAL_ASSERT_MINOR(ptr,_end)

   char* tmp;
   char* pattern_data = 0;
   uint32_t klen, dlen, pattern_size = 0;

   if (_pattern)
      {
      pattern_data = _pattern->data();
      pattern_size = _pattern->size();

      tmp = ptr + sizeof(UCDB::cdb_record_header);

      pattern = (char*) u_find(tmp, _end - tmp, pattern_data, pattern_size);

      if (pattern == 0) goto fine;
      }

   function_to_call = function;

   while (true)
      {
      klen = u_get_unaligned32(((UCDB::cdb_record_header*)ptr)->klen); //  key length
      dlen = u_get_unaligned32(((UCDB::cdb_record_header*)ptr)->dlen); // data length

      U_INTERNAL_DUMP("hr(%p) = { %u, %u }", ptr, klen, dlen)

      tmp = ptr + sizeof(UCDB::cdb_record_header) + klen + dlen;

      if (_pattern &&
          tmp <= pattern) goto end_loop;

      call1(ptr + sizeof(UCDB::cdb_record_header), klen, tmp - dlen, dlen);

      if (tmp >= _end ||
          U_cdb_result_call(this) == 0)
         {
         break;
         }

      if (_pattern)
         {
         ptr = tmp + sizeof(UCDB::cdb_record_header);

         pattern = (char*) u_find(ptr, _end - ptr, pattern_data, pattern_size);

         if (pattern == 0) break;

         U_INTERNAL_ASSERT_MINOR(pattern, _end)
         }

end_loop:
      ptr = tmp;
      }

fine:
   U_INTERNAL_ASSERT(tmp <= _end)

   function_to_call = 0;
}

void UCDB::call1()
{
   U_TRACE_NO_PARAM(0, "UCDB::call1()")

   U_INTERNAL_ASSERT_POINTER(function_to_call)

   UStringRep* skey;
   UStringRep* sdata;

   U_NEW(UStringRep, skey,  UStringRep((const char*) key.dptr,  key.dsize));
   U_NEW(UStringRep, sdata, UStringRep((const char*)data.dptr, data.dsize));

   U_INTERNAL_DUMP("skey = %#V sdata = %#V)", skey, sdata)

   if (filter_function_to_call(skey, sdata))
      {
      U_cdb_result_call(this) = function_to_call(skey, sdata);

      U_INTERNAL_DUMP("U_cdb_result_call = %d U_cdb_add_entry_to_vector = %b", U_cdb_result_call(this), U_cdb_add_entry_to_vector(this))

      if (U_cdb_result_call(this) == 2 || // NB: for remove...
          U_cdb_add_entry_to_vector(this))
         {
         U_INTERNAL_ASSERT_POINTER(ptr_vector)

         ptr_vector->UVector<void*>::push(skey);

         if (U_cdb_add_entry_to_vector(this) == false) goto next;

         U_cdb_add_entry_to_vector(this) = false;

         ptr_vector->UVector<void*>::push(sdata);

         return;
         }
      }

       skey->release();
next: sdata->release();
}

void UCDB::call1(const char*  key_ptr, uint32_t  key_size,
                 const char* data_ptr, uint32_t data_size)
{
   U_TRACE(0, "UCDB::call1(%.*S,%u,%.*S,%u)",  key_size,  key_ptr,  key_size,
                                              data_size, data_ptr, data_size)

   setKey(  key_ptr,  key_size);
   setData(data_ptr, data_size);

   call1();
}

void UCDB::call2(UCDB* pcdb, char* src)
{
   U_TRACE(0, "UCDB::call2(%p,%p)", pcdb, src)

   UCDB::cdb_record_header* ptr_hr = (UCDB::cdb_record_header*)src;

   uint32_t klen = u_get_unaligned32(ptr_hr->klen), // key length
            dlen = u_get_unaligned32(ptr_hr->dlen); // data length

   U_INTERNAL_DUMP("hr(%p) = { %u, %u }", ptr_hr, klen, dlen)

   src += sizeof(UCDB::cdb_record_header);

   pcdb->call1(src,        klen,
               src + klen, dlen);
}

void UCDB::getKeys2(UCDB* pcdb, char* src)
{
   U_TRACE(0, "UCDB::getKeys2(%p,%p)", pcdb, src)

   UStringRep* skey;
   UCDB::cdb_record_header* ptr_hr = (UCDB::cdb_record_header*)src;

   uint32_t klen = u_get_unaligned32(ptr_hr->klen); // key length
#ifdef DEBUG
   uint32_t dlen = u_get_unaligned32(ptr_hr->dlen); // data length

   U_INTERNAL_DUMP("hr(%p) = { %u, %u }", ptr_hr, klen, dlen)
#endif

   src += sizeof(UCDB::cdb_record_header);

   U_NEW(UStringRep, skey, UStringRep(src, klen));

   if (pcdb->filter_function_to_call(skey, 0) == 0) skey->release();
   else                                             pcdb->ptr_vector->UVector<void*>::push(skey);
}

void UCDB::print2(UCDB* pcdb, char* src)
{
   U_TRACE(0, "UCDB::print2(%p,%p)", pcdb, src)

   U_INTERNAL_ASSERT_POINTER(pcdb)
   U_INTERNAL_ASSERT_POINTER(pcdb->pbuffer)

   UCDB::cdb_record_header* ptr_hr = (UCDB::cdb_record_header*)src;

   uint32_t klen = u_get_unaligned32(ptr_hr->klen), //  key length
            dlen = u_get_unaligned32(ptr_hr->dlen); // data length

   pcdb->pbuffer->printKeyValue(src + sizeof(UCDB::cdb_record_header), klen, src + sizeof(UCDB::cdb_record_header) + klen, dlen);
}

void UCDB::makeAdd2(UCDB* pcdb, char* src)
{
   U_TRACE(0, "UCDB::makeAdd2(%p,%p)", pcdb, src)

   UCDB::cdb_record_header* s_hr = (UCDB::cdb_record_header*)src;
   UCDB::cdb_record_header* d_hr = pcdb->hr;

   U_INTERNAL_DUMP("src = { %#.*S %#.*S }", u_get_unaligned32(s_hr->klen), src + sizeof(UCDB::cdb_record_header),
                                            u_get_unaligned32(s_hr->dlen), src + sizeof(UCDB::cdb_record_header) +
                                            u_get_unaligned32(s_hr->klen))

   uint32_t sz = sizeof(UCDB::cdb_record_header) + u_get_unaligned32(s_hr->klen) + u_get_unaligned32(s_hr->dlen);

   U_MEMCPY(d_hr, s_hr, sz);

   U_INTERNAL_DUMP("hr(%p) = { %u, %u }", d_hr, u_get_unaligned32(d_hr->klen), u_get_unaligned32(d_hr->dlen))

   pcdb->hr = (UCDB::cdb_record_header*)((char*)d_hr + sz);

   pcdb->nrecord++;
}

uint32_t UCDB::getValuesWithKeyNask(UVector<UString>& vec_values, const UString& mask_key, uint32_t* _size)
{
   U_TRACE(0, "UCDB::getValuesWithKeyNask(%p,%V,%p)", &vec_values, mask_key.rep, _size)

   U_INTERNAL_ASSERT_MAJOR(UFile::st_size,0)
   U_INTERNAL_ASSERT_DIFFERS(UFile::map, MAP_FAILED)

   char* ptr  = start();
   char* _end = UCDB::end();

   U_INTERNAL_DUMP("ptr = %p end = %p", ptr, _end)

   U_INTERNAL_ASSERT_MINOR(ptr,_end)

   char* tmp;
   UStringRep* rep;
   uint32_t n = vec_values.size();
   int flags = (ignoreCase() ? FNM_CASEFOLD : 0);

   if (_size) *_size = 0;

   while (true)
      {
      uint32_t klen = u_get_unaligned32(((UCDB::cdb_record_header*)ptr)->klen), //  key length
               dlen = u_get_unaligned32(((UCDB::cdb_record_header*)ptr)->dlen); // data length

      U_INTERNAL_DUMP("hr(%p) = { %u, %u }", ptr, klen, dlen)

      tmp = ptr + sizeof(UCDB::cdb_record_header) + klen + dlen;

      if (UServices::dosMatchWithOR(ptr + sizeof(UCDB::cdb_record_header), klen, U_STRING_TO_PARAM(mask_key), flags))
         {
         U_INTERNAL_DUMP("key = %#.*S data = %#.*S)", klen, ptr + sizeof(UCDB::cdb_record_header), dlen, tmp - dlen)

         U_NEW(UStringRep, rep, UStringRep(tmp - dlen, dlen));

         vec_values.UVector<void*>::push(rep);

         if (_size) *_size += dlen;
         }

      if (tmp >= _end) break;

      ptr = tmp;
      }

   U_INTERNAL_ASSERT(tmp <= _end)

   n = vec_values.size() - n;

   U_RETURN(n);
}

// PRINT DATABASE

UString UCDB::print()
{
   U_TRACE_NO_PARAM(0, "UCDB::print()")

   if (UFile::st_size)
      {
      UString buffer(UFile::st_size);

      pbuffer = &buffer;

      callForAllEntry((vPFpvpc)print2);

      U_RETURN_STRING(buffer);
      }

   return UString::getStringNull();
}

// Save memory hash table as constant database

bool UCDB::writeTo(UCDB& cdb, UHashMap<void*>* table, uint32_t tbl_space, pvPFpvpb func)
{
   U_TRACE(1, "UCDB::writeTo(%p,%p,%u,%p)", &cdb, table, tbl_space, func)

   cdb.nrecord = (func ? 0
                       : table->size());

   bool result = cdb.creat(O_RDWR) &&
                 cdb.ftruncate(sizeFor(cdb.nrecord) + tbl_space);

   if (result)
      {
      result = cdb.memmap(PROT_READ | PROT_WRITE);

      if (result == false) U_RETURN(false);

      bool bdelete;
      UStringRep* value;
      const UStringRep* _key;
      char* ptr = cdb.start(); // init of DATA
      UCDB::cdb_record_header* _hr;

      U_INTERNAL_DUMP("table->_length = %u", table->_length)

      UHashMapNode* node;
      UHashMapNode** pnode;
      UHashMapNode** tbl = table->table;

      uint32_t klen; //  key length
      uint32_t dlen; // data length

      for (uint32_t index = 0, capacity = table->_capacity; index < capacity; ++index)
         {
         if (tbl[index])
            {
            node  = tbl[index];
            pnode = tbl + index;

            do {
               if (func == 0) value = (UStringRep*) node->elem;
               else
                  {
                  value = (UStringRep*) func((void*)node->elem, &bdelete);

                  U_INTERNAL_DUMP("bdelete = %b", bdelete)

                  if (bdelete) // ask for to delete node of table...
                     {
                     *pnode = node->next; // lo si toglie dalla lista collisioni...

                     /**
                      * NB: it must be do in the function
                      * ---------------------------------
                      * elem = (T*) node->elem;
                      *
                      * u_destroy<T>(elem);
                      * ---------------------------------
                      */

                     delete node;

                     table->_length--;
                     }
                  }

               if (value)
                  {
                  _key = node->key;

                  klen =  _key->size(); //  key length
                  dlen = value->size(); // data length

                  _hr = (UCDB::cdb_record_header*) ptr;

                  u_put_unaligned32(_hr->klen, klen);
                  u_put_unaligned32(_hr->dlen, dlen);

                  U_INTERNAL_DUMP("hr = { %u, %u }", klen, dlen)

                  ptr += sizeof(UCDB::cdb_record_header);

                  U_MEMCPY(ptr, _key->data(), klen);

                  U_INTERNAL_DUMP("key = %.*S", klen, ptr)

                  ptr += klen;

                  U_MEMCPY(ptr, value->data(), dlen);

                  U_INTERNAL_DUMP("data = %.*S", dlen, ptr)

                  ptr += dlen;

                  if (func)
                     {
                     cdb.nrecord++;

                     value->release();
                     }
                  }

               // check if asked to delete node of the table...

               if (func    == 0 ||
                   bdelete == false) pnode = &(*pnode)->next;
               }
            while ((node = *pnode));
            }
         }

      U_INTERNAL_DUMP("table->_length = %u", table->_length)

      cdb.hr = (UCDB::cdb_record_header*) ptr; // end of DATA

      uint32_t pos = cdb.makeFinish(true);

      U_INTERNAL_ASSERT(pos <= (uint32_t)cdb.st_size)

      if (pos < (uint32_t)cdb.st_size)
         {
                  cdb.munmap();
         result = cdb.ftruncate(pos);
         }

      cdb.UFile::close();
      }

   U_RETURN(result);
}

#ifdef DEBUG
U_NO_EXPORT void UCDB::checkForAllEntry()
{
   U_TRACE_NO_PARAM(0+256, "UCDB::checkForAllEntry()")

   U_INTERNAL_DUMP("nrecord = %u", nrecord)

   U_INTERNAL_ASSERT_MAJOR(UFile::st_size, 0)
   U_INTERNAL_ASSERT_DIFFERS(UFile::map, MAP_FAILED)

   char* ptr;
   char* _eof = UFile::map + (ptrdiff_t)UFile::st_size;
   slot       = (cdb_hash_table_slot*) end();

   while ((char*)slot < _eof)
      {
      uint32_t pos = u_get_unaligned32(slot->pos);

      if (pos)
         {
         ptr = UFile::map + pos;
          hr = (cdb_record_header*)ptr;

         U_INTERNAL_DUMP("hr(%p) = { %u, %u }", hr, u_get_unaligned32(hr->klen), u_get_unaligned32(hr->dlen))

         khash     = u_get_unaligned32(slot->hash);
         key.dsize = u_get_unaligned32(hr->klen);
         key.dptr  = ptr + sizeof(cdb_record_header);

         U_INTERNAL_DUMP("key = %.*S khash = %u", key.dsize, key.dptr, khash)

         if (key.dsize == 0) U_ERROR("UCDB::checkForAllEntry() - null key size - db(%.*S)", U_FILE_TO_TRACE(*this));
         }

      slot++;
      }
}
#endif

// STREAM

#ifdef U_STDCPP_ENABLE
U_EXPORT istream& operator>>(istream& is, UCDB& cdb)
{
   U_TRACE(0+256, "UCDB::operator>>(%p,%p)", &is, &cdb)

   cdb.makeStart();

   char c;
   char* ptr = (char*) cdb.hr; // init of DATA

   uint32_t klen; //  key length
   uint32_t dlen; // data length

   UCDB::cdb_record_header* hr;

   while (is >> c)
      {
      U_INTERNAL_DUMP("c = %C", c)

      if (c == '#')
         {
         for (int ch = is.get(); (ch != '\n' && ch != EOF); ch = is.get()) {}

         continue;
         }

      if (c != '+') break;

      hr = (UCDB::cdb_record_header*)ptr;

      is >> klen;
      is.get(); // skip ','
      is >> dlen;
      is.get(); // skip ':'

      U_INTERNAL_DUMP("hr = { %u, %u }", klen, dlen)

      u_put_unaligned32(hr->klen, klen);
      u_put_unaligned32(hr->dlen, dlen);

      ptr += sizeof(UCDB::cdb_record_header);

#  ifndef U_COVERITY_FALSE_POSITIVE /* TAINTED_SCALAR */
      is.read(ptr, klen);
#  endif

      U_INTERNAL_DUMP("key = %.*S", klen, ptr)

      is.get(); // skip '-'
      is.get(); // skip '>'

      ptr += klen;

#  ifndef U_COVERITY_FALSE_POSITIVE /* TAINTED_SCALAR */
      is.read(ptr, dlen);
#  endif

      U_INTERNAL_DUMP("data = %.*S", dlen, ptr)

      ptr += dlen;

      cdb.nrecord++;

      is.get(); // skip '\n'
      }

   cdb.hr = (UCDB::cdb_record_header*) ptr; // end of DATA

   uint32_t pos = cdb.makeFinish(true);

          cdb.munmap();
   (void) cdb.ftruncate(pos);

   return is;
}

U_EXPORT ostream& operator<<(ostream& os, UCDB& cdb)
{
   U_TRACE(0+256, "UCDB::operator<<(%p,%p)", &os, &cdb)

   UString text = cdb.print();

   (void) os.write(text.data(), text.size());

   os.put('\n');

   return os;
}

// DEBUG

#  ifdef DEBUG
const char* UCDB::dump(bool _reset) const
{
   UFile::dump(false);

   *UObjectIO::os << '\n'
                  << "hp                        " << (void*)hp      << '\n'
                  << "hr                        " << (void*)hr      << '\n'
                  << "key                       " << "{ "           << key.dptr
                                                  << ' '            << key.dsize
                                                                    << " }\n"
                  << "data                      " << "{ "           << data.dptr
                                                  << ' '            << data.dsize
                                                                    << " }\n"
                  << "slot                      " << (void*)slot    << '\n'
                  << "loop                      " << loop           << '\n'
                  << "nslot                     " << nslot          << '\n'
                  << "khash                     " << khash          << '\n'
                  << "offset                    " << offset         << '\n'
                  << "nrecord                   " << nrecord        << '\n'
                  << "start_hash_table_slot     " << start_hash_table_slot;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
