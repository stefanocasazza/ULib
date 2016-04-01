// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    tdb.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/db/tdb.h>
#include <ulib/container/vector.h>

bool UTDB::disable_lock;
bool UTDB::disable_mmap;

U_NO_EXPORT int UTDB::_getKeys(TDB_CONTEXT* tdb, TDB_DATA key, TDB_DATA dbuf, void* ptr)
{
   U_TRACE(0, "UTDB::_getKeys(%p,%J,%J,%p)", tdb, key, dbuf, ptr)

   U_INTERNAL_ASSERT_POINTER(ptr)

   UString str((void*)key.dptr, key.dsize);

   ((UVector<UString>*)ptr)->push(str);

   U_RETURN(0); // A non-zero return value from fn() indicates that the traversal should stop
}

U_NO_EXPORT int UTDB::_print(TDB_CONTEXT* tdb, TDB_DATA key, TDB_DATA dbuf, void* ptr)
{
   U_TRACE(0, "UTDB::_print(%p,%J,%J,%p)", tdb, key, dbuf, ptr)

   U_INTERNAL_ASSERT_POINTER(ptr)

   ((UString*)ptr)->printKeyValue((const char*)key.dptr, key.dsize, (const char*)dbuf.dptr, dbuf.dsize);

   U_RETURN(0); // A non-zero return value from fn() indicates that the traversal should stop
}

bool UTDB::getKeys(UVector<UString>& vec)
{
   U_TRACE(1, "UTDB::getKeys(%p)", &vec)

   U_CHECK_MEMORY

#ifdef U_TDB_TRAVERSE_READ
   if (U_SYSCALL(tdb_traverse_read, "%p,%p,%p", context, UTDB::_getKeys, &vec) != -1) U_RETURN(true);
#else
   if (U_SYSCALL(tdb_traverse,      "%p,%p,%p", context, UTDB::_getKeys, &vec) != -1) U_RETURN(true);
#endif

   U_RETURN(false);
}

UString UTDB::print()
{
   U_TRACE_NO_PARAM(1, "UTDB::print()")

   U_CHECK_MEMORY

   UString str(U_CAPACITY);

#ifdef U_TDB_TRAVERSE_READ
   if (U_SYSCALL(tdb_traverse_read, "%p,%p,%p", context, UTDB::_print, &str) != -1) U_RETURN_STRING(str);
#else
   if (U_SYSCALL(tdb_traverse,      "%p,%p,%p", context, UTDB::_print, &str) != -1) U_RETURN_STRING(str);
#endif

   return UString::getStringNull();
}

UString UTDB::printSorted()
{
   U_TRACE_NO_PARAM(1, "UTDB::printSorted()")

   U_CHECK_MEMORY

   UVector<UString> vkey(512);

   getKeys(vkey);

   _uudata key;
   TDB_DATA dbuf;
   UStringRep* r;
   uint32_t n = vkey.size();
   UString buffer(U_CAPACITY);

   if (n > 1) vkey.sort();

   for (uint32_t i = 0; i < n; ++i)
      {
      r = vkey.UVector<UStringRep*>::at(i);

      key.d1.dptr  = (unsigned char*)r->data();
      key.d1.dsize = r->size();

      dbuf = (TDB_DATA) U_SYSCALL(tdb_fetch, "%p,%J", context, key.d2);

      if (dbuf.dptr)
         {
         buffer.printKeyValue((const char*)key.d1.dptr, key.d1.dsize, (const char*)dbuf.dptr, dbuf.dsize);

         U_SYSCALL_VOID(free, "%p", dbuf.dptr);
         }
      }

   U_RETURN_STRING(buffer);
}

// STREAM

#ifdef U_STDCPP_ENABLE
U_EXPORT istream& operator>>(istream& is, UTDB& tdb)
{
   U_TRACE(1+256, "UTDB::operator>>(%p,%p)", &is, &tdb)

   char c;
   _uudata key, dbuf;
   char buffer1[4096], buffer2[4096];

    key.d1.dptr  = (unsigned char*)buffer1;
    key.d1.dsize = 0;
   dbuf.d1.dptr  = (unsigned char*)buffer2;
   dbuf.d1.dsize = 0;

   while (is >> c)
      {
      U_INTERNAL_DUMP("c = %C", c)

      if (c == '#')
         {
         for (int ch = is.get(); (ch != '\n' && ch != EOF); ch = is.get()) {}

         continue;
         }

      if (c != '+') break;

      is >> key.d1.dsize;
      is.get(); // skip ','
      is >> dbuf.d1.dsize;
      is.get(); // skip ':'

      U_INTERNAL_ASSERT_MINOR(key.d1.dsize, sizeof(buffer1))

      is.read(buffer1, key.d1.dsize);

      U_INTERNAL_DUMP("key = %J", key.d1)

      is.get(); // skip '-'
      is.get(); // skip '>'

      U_INTERNAL_ASSERT_MINOR(dbuf.d1.dsize, sizeof(buffer2))

      is.read(buffer2, dbuf.d1.dsize);

      U_INTERNAL_DUMP("data = %J", dbuf.d1)

      is.get(); // skip '\n'

      if (U_SYSCALL(tdb_store, "%p,%J,%J,%d", tdb.context, key.d2, dbuf.d2, TDB_REPLACE)) break;
      }

   return is;
}

U_EXPORT ostream& operator<<(ostream& os, UTDB& tdb)
{
   U_TRACE(0+256, "UTDB::operator<<(%p,%p)", &os, &tdb)

   UString text = tdb.print();

   (void) os.write(text.data(), text.size());

   os.put('\n');

   return os;
}

// DEBUG

#  ifdef DEBUG
const char* UTDB::dump(bool _reset) const
{
   *UObjectIO::os << "context " << (void*)context;

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
