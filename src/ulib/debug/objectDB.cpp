// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    objectDB.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

/*
#define DEBUG_DEBUG
*/

#include <ulib/internal/common.h>

#include <ulib/base/hash.h>
#include <ulib/base/utility.h>
#include <ulib/debug/error_memory.h>

U_EXPORT int  UObjectDB::fd = -1;
U_EXPORT int  UObjectDB::level_active;
U_EXPORT bool UObjectDB::flag_new_object;
U_EXPORT bool UObjectDB::flag_ulib_object;

U_NO_EXPORT iovec UObjectDB::liov[8] = {
   { 0, 0 },
   { (caddr_t) UObjectDB::buffer1, 0 },
   { (caddr_t) UObjectDB::buffer2, 0 },
   { 0, 0 },
   { (caddr_t) UObjectDB::buffer3, 0 },
   { (caddr_t) U_CONSTANT_TO_PARAM(U_LF2) },
   { 0, 0 },
   { (caddr_t) "\n---------------------------------------"
               "-----------------------------------------\n", 82 }
};

U_NO_EXPORT char        UObjectDB::buffer1[64];
U_NO_EXPORT char        UObjectDB::buffer2[256];
U_NO_EXPORT char        UObjectDB::buffer3[64];
U_NO_EXPORT char*       UObjectDB::file_ptr;
U_NO_EXPORT char*       UObjectDB::file_mem;
U_NO_EXPORT char*       UObjectDB::file_limit;
U_NO_EXPORT char*       UObjectDB::lbuf;
U_NO_EXPORT char*       UObjectDB::lend;
U_NO_EXPORT uint32_t    UObjectDB::file_size;
U_NO_EXPORT bPFpcpv     UObjectDB::checkObject;
U_NO_EXPORT const char* UObjectDB::_name_class;
U_NO_EXPORT const void* UObjectDB::_ptr_object;

typedef bool (*vPFpObjectDumpable)(const UObjectDumpable*);

class U_NO_EXPORT UHashMapObjectDumpable {
public:
   const UObjectDumpable* objDumper;

   UHashMapObjectDumpable()
      {
      U_INTERNAL_TRACE("UHashMapObjectDumpable::UHashMapObjectDumpable()")

      objDumper = 0;

      next = 0;
      hash = 0;
      }

   ~UHashMapObjectDumpable()
      {
      U_INTERNAL_TRACE("UHashMapObjectDumpable::~UHashMapObjectDumpable()")
      }

   // Services

   static UHashMapObjectDumpable* node;
   static UHashMapObjectDumpable** table;
   static uint32_t index, random, num, counter, table_size;

   static void init(uint32_t size)
      {
      U_INTERNAL_TRACE("UHashMapObjectDumpable::init(%u)", size)

      table = (UHashMapObjectDumpable**) calloc((table_size = U_GET_NEXT_PRIME_NUMBER(size)), sizeof(UHashMapObjectDumpable*));
      }

   static void lookup(const void* ptr_object)
      {
      U_INTERNAL_TRACE("UHashMapObjectDumpable::lookup(%p)", ptr_object)

      // This hash function is designed so that a changing just one bit in input 'i' will potentially affect the
      // every bit in hash(i), and the correlation between succesive hashes is (hopefully) extremely small (if not zero)

#  ifndef HAVE_ARCH64
      random = u_random(  (uint32_t)ptr_object);
#  else
      random = u_random64((uint64_t)ptr_object);
#  endif

      index = random % table_size;

      U_INTERNAL_PRINT("index = %u", index)

      U_INTERNAL_ASSERT_MINOR(index, table_size)

      for (node = table[index]; node; node = node->next)
         {
         if (node->objDumper->ptr_object == ptr_object) break;
         }

      U_INTERNAL_PRINT("node = %p", node)
      }

   static void resize()
      {
      U_INTERNAL_TRACE("UHashMapObjectDumpable::resize()")

      uint32_t                 old_table_size = table_size;
      UHashMapObjectDumpable** old_table      = table;

      init(old_table_size);

      // we insert the old elements

      UHashMapObjectDumpable* next;

      for (uint32_t i = 0; i < old_table_size; ++i)
         {
         if (old_table[i])
            {
            node = old_table[i];

            do {
               next  = node->next;
               index = (node->hash % table_size);

               U_INTERNAL_PRINT("hash = %u i = %u index = %u", node->hash, i, index)

               /**
                * list self-organizing (move-to-front), we place before
                * the element at the beginning of the list of collisions
                */

               node->next = table[index];
                            table[index] = node;
               }
            while ((node = next));
            }
         }

      free(old_table);
      }

   static void insert(UObjectDumpable* dumper)
      {
      U_INTERNAL_TRACE("UHashMapObjectDumpable::insert(%p)", dumper)

      if (num > table_size) resize();

      lookup(dumper->ptr_object);

      // This is for to cope with hierarchies of dumpable classes. In such cases we typically want only one dump,
      // corresponding to the most derived instance. To achieve this, the handle registered for the subobject
      // corresponding to the base class is overwritten (hence on destruction of the subobject its handle won't
      // exist anymore and we'll have to check for that)

      if (node)
         {
         /*
         U_DEBUG("UHashMapObjectDumpable::insert() - ptr_object = %p base_class = %s derived_class = %s",
                                 dumper->ptr_object, node->objDumper->name_class, dumper->name_class);
         */

         delete node->objDumper;
         }
      else
         {
         ++num;

         node = new UHashMapObjectDumpable;

         /**
          * list self-organizing (move-to-front), we place before
          * the element at the beginning of the list of collisions
          */

         node->hash = random;
         node->next = table[index];
                      table[index] = node;
         }

      dumper->cnt     = ++counter;
      node->objDumper = dumper;

      U_INTERNAL_PRINT("num = %u counter = %u", num, counter)
      }

   static bool erase(const void* ptr_object)
      {
      U_INTERNAL_TRACE("UHashMapObjectDumpable::erase(%p)", ptr_object)

      lookup(ptr_object);

      if (node == 0) return false;

      UHashMapObjectDumpable* prev = 0;

      for (UHashMapObjectDumpable* pnode = table[index]; pnode; pnode = pnode->next)
         {
         if (pnode == node)
            {
            U_INTERNAL_ASSERT_EQUALS(pnode->objDumper->ptr_object, ptr_object)

            /**
             * list self-organizing (move-to-front), we place before
             * the element at the beginning of the list of collisions
             */

            if (prev)
               {
               prev->next = pnode->next;
               node->next = table[index];
                            table[index] = pnode;
               }

            U_INTERNAL_ASSERT_EQUALS(node, table[index])

            break;
            }

         prev = pnode;
         }

      U_INTERNAL_PRINT("prev = %p", prev)

      /**
       * list self-organizing (move-to-front), we requires the
       * item to be deleted at the beginning of the list of collisions
       */

      U_INTERNAL_ASSERT_EQUALS(node, table[index])

      table[index] = node->next;

      delete node->objDumper;
      delete node;

      --num;

      U_INTERNAL_PRINT("num = %u", num)

      return true;
      }

   static void callForAllEntry(vPFpObjectDumpable function)
      {
      U_INTERNAL_TRACE("UHashMapObjectDumpable::callForAllEntry(%p)", function)

      U_INTERNAL_PRINT("num = %u", num)

      if (num)
         {
         int sum = 0, max = 0, min = 1024, width;

         for (index = 0; index < table_size; ++index)
            {
            if (table[index])
               {
               node = table[index];

               ++sum;

               width = -1;

               do {
                  ++width;

                  if (function(node->objDumper) == false) return;
                  }
               while ((node = node->next));

               if (max < width) max = width;
               if (min > width) min = width;
               }
            }

         U_INTERNAL_PRINT("collision(min,max) = (%d,%d) - distribution = %f", min, max, (double)num / (double)sum)
         }
      }

private:
   UHashMapObjectDumpable* next;
   uint32_t hash;

   U_DISALLOW_COPY_AND_ASSIGN(UHashMapObjectDumpable)
};

U_NO_EXPORT uint32_t     UHashMapObjectDumpable::num;
U_NO_EXPORT uint32_t     UHashMapObjectDumpable::index;
U_NO_EXPORT uint32_t     UHashMapObjectDumpable::random;
U_NO_EXPORT uint32_t     UHashMapObjectDumpable::counter;
U_NO_EXPORT uint32_t     UHashMapObjectDumpable::table_size;
UHashMapObjectDumpable*  UHashMapObjectDumpable::node;
UHashMapObjectDumpable** UHashMapObjectDumpable::table;

void UObjectDB::init(bool flag, bool info)
{
   U_INTERNAL_TRACE("UObjectDB::init(%d,%d)", flag, info)

   char* env = getenv("UOBJDUMP");

   if ( env &&
       *env)
      {
      if (u__isquote(*env)) ++env; // normalizzazione...

      // format: <level_active> <max_size_log> <table_size>
      //                1           500k           100

      char suffix;
      char name[128];
      uint32_t table_size = 0;

      (void) sscanf(env, "%d%u%c%u", &level_active, &file_size, &suffix, &table_size);

      if (file_size) U_NUMBER_SUFFIX(file_size, suffix);

      (void) u__snprintf(name, 128, U_CONSTANT_TO_PARAM("object.%N.%P"), 0);

      /* NB: O_RDWR is needed for mmap(MAP_SHARED)... */

      fd = open(name, O_CREAT | O_RDWR | O_BINARY, 0666);

      if (fd != -1)
         {
         /* we manage max size... */

         if (file_size)
            {
            if (ftruncate(fd, file_size))
               {
               U_WARNING("Out of space on file system, (required %u bytes)", file_size);

               file_size = 0;
               }
            else
               {
               /* NB: PROT_READ avoid some strange SIGSEGV... */

               file_mem = (char*) mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

               if (file_mem == MAP_FAILED)
                  {
                  file_mem  = 0;
                  file_size = 0;

                  (void) ftruncate(fd, file_size);
                  }

               file_ptr   = file_mem;
               file_limit = file_mem + file_size;
               }
            }
         }

      if (fd != -1)
         {
         if (flag)
            {
            UHashMapObjectDumpable::init(table_size);

            u_atexit(&UObjectDB::close); // register function of close dump at exit...
            }
         }
      }

   if (info)
      {
      if (fd == -1)
         {
         U_MESSAGE("OBJDUMP%W<%Woff%W>%W", YELLOW, RED, YELLOW, RESET);
         }
      else
         {
         U_MESSAGE("OBJDUMP%W<%Won%W>: Level<%W%d%W> MaxSize<%W%d%W>%W", YELLOW, GREEN, YELLOW, CYAN, level_active, YELLOW, CYAN, file_size, YELLOW, RESET);
         }
      }
}

U_NO_EXPORT void UObjectDB::_write(const struct iovec* iov, int _n)
{
   U_INTERNAL_TRACE("UObjectDB::_write(%p,%d)", iov, _n)

   if (file_size == 0) (void) writev(fd, iov, _n);
   else
      {
      for (int i = 0; i < _n; ++i)
         {
         if (iov[i].iov_len)
            {
            U_INTERNAL_ASSERT_RANGE(1,iov[i].iov_len,file_size)

            if ((file_ptr + iov[i].iov_len) > file_limit) file_ptr = file_mem;

            u__memcpy(file_ptr, iov[i].iov_base, iov[i].iov_len, __PRETTY_FUNCTION__);

            file_ptr += iov[i].iov_len;
            }
         }
      }
}

void UObjectDB::close()
{
   U_INTERNAL_TRACE("UObjectDB::close()")

   if (fd != -1)
      {
      dumpObjects();

      if (file_size)
         {
         ptrdiff_t write_size = file_ptr - file_mem;

         U_INTERNAL_ASSERT_MINOR(write_size, (ptrdiff_t)file_size)

      // (void) msync(file_mem, write_size, MS_SYNC);
      
         (void) munmap(file_mem, file_size);
         (void) ftruncate(fd, write_size);
         (void) fsync(fd);

         file_size = 0;
         }

      (void) ::close(fd);

      fd = -1;
      }
}

void UObjectDB::initFork()
{
   U_INTERNAL_TRACE("UObjectDB::initFork()")

   U_INTERNAL_ASSERT_RANGE(0,fd,1024)

   if (file_size)
      {
      (void) munmap(file_mem, file_size);

      file_size = 0;
      }

   (void) ::close(fd);

   init(false, true);
}

void UObjectDB::registerObject(UObjectDumpable* dumper)
{
   U_INTERNAL_TRACE("UObjectDB::registerObject(%p)", dumper)

   dumper->num_line      = u_num_line;
   dumper->name_file     = u_name_file;
   dumper->name_function = u_name_function;

   if (flag_ulib_object) dumper->level = -1;

   UHashMapObjectDumpable::insert(dumper);
}

void UObjectDB::unregisterObject(const void* ptr_object)
{
   U_INTERNAL_TRACE("UObjectDB::unregisterObject(%p)", ptr_object)

   if (UHashMapObjectDumpable::erase(ptr_object) == false)
      {
   // U_DEBUG("UObjectDB::unregisterObject(%p) = false", ptr_object);
      }
}

// dump

void UObjectDB::dumpObject(const UObjectDumpable* dumper)
{
   U_INTERNAL_TRACE("UObjectDB::dumpObject(%p)", dumper)

   U_INTERNAL_ASSERT(dumper->level >= level_active)

   liov[0].iov_len  =  u__strlen(dumper->name_class, __PRETTY_FUNCTION__);
   liov[0].iov_base = (caddr_t)  dumper->name_class;

   liov[1].iov_len  = sprintf(buffer1, " %p size %d level %d", // cnt %09d",
                              dumper->ptr_object, dumper->sz_obj, dumper->level); //, dumper->cnt);

   U_INTERNAL_ASSERT_EQUALS(liov[1].iov_len, strlen(buffer1))

   liov[2].iov_len  = sprintf(buffer2, "\n%s(%d)\n", dumper->name_file, dumper->num_line);

   U_INTERNAL_ASSERT_EQUALS(liov[2].iov_len, strlen(buffer2))

   liov[3].iov_len  = u__strlen(dumper->name_function, __PRETTY_FUNCTION__);
   liov[3].iov_base = (caddr_t) dumper->name_function;

   const void* _this = (dumper->psentinel ? *(dumper->psentinel)
                                          : (void*)U_CHECK_MEMORY_SENTINEL);

   if (_this == (void*)U_CHECK_MEMORY_SENTINEL) liov[4].iov_len = 0;
   else
      {
      liov[4].iov_len = sprintf(buffer3, "\nERROR ON MEMORY [sentinel = " U_CHECK_MEMORY_SENTINEL_STR " _this = %p - %s]\n", _this, (_this ? "ABW" : "FMR"));

      U_INTERNAL_ASSERT_EQUALS(liov[4].iov_len, strlen(buffer3))
      }

   liov[6].iov_base = (caddr_t) dumper->dump();
   liov[6].iov_len  = (UObjectIO::buffer_output_len ? UObjectIO::buffer_output_len : u__strlen((const char*)liov[6].iov_base, __PRETTY_FUNCTION__));

   U_INTERNAL_PRINT("UObjectIO::buffer_output(%u) = \"%.*s\" liov[6](%u) = \"%.*s\"",
                     UObjectIO::buffer_output_len, UObjectIO::buffer_output_len, UObjectIO::buffer_output, liov[6].iov_len, liov[6].iov_len, liov[6].iov_base)
}

// dump single object...

U_NO_EXPORT bool __pure UObjectDB::checkIfObject(const char* name_class, const void* ptr_object)
{
   U_VAR_UNUSED(name_class)

   U_INTERNAL_TRACE("UObjectDB::checkIfObject(%S,%p)", name_class, ptr_object)

   if (ptr_object == _ptr_object) return true;

   return false;
}

U_NO_EXPORT bool UObjectDB::printObjLive(const UObjectDumpable* dumper)
{
   U_INTERNAL_TRACE("UObjectDB::printObjLive(%p)", dumper)

   if (dumper->level >= level_active &&
       checkObject(dumper->name_class, dumper->ptr_object))
      {
      dumpObject(dumper);

      for (int i = 0; i < 8; ++i)
         {
         if ((lbuf + liov[i].iov_len) > lend) return false;

         if (liov[i].iov_len)
            {
            u__memcpy(lbuf, liov[i].iov_base, liov[i].iov_len, __PRETTY_FUNCTION__);

            lbuf += liov[i].iov_len;
            }
         }
      }

   return true;
}

U_EXPORT uint32_t UObjectDB::dumpObject(char* buffer, uint32_t buffer_size, bPFpcpv check_object)
{
   U_INTERNAL_TRACE("UObjectDB::dumpObject(%p,%u,%p)", buffer, buffer_size, check_object)

   lbuf        = buffer;
   lend        = buffer + buffer_size;
   checkObject = check_object;

   UHashMapObjectDumpable::callForAllEntry(UObjectDB::printObjLive);

   U_INTERNAL_ASSERT(lbuf >= buffer)
   U_INTERNAL_ASSERT_MINOR(lbuf, lend)

   return (lbuf - buffer);
}

uint32_t UObjectDB::dumpObject(char* buffer, uint32_t buffer_size, const void* ptr_object)
{
   U_INTERNAL_TRACE("UObjectDB::dumpObject(%p,%u,%p)", buffer, buffer_size, ptr_object)

   lbuf        = buffer;
   lend        = buffer + buffer_size;
   _ptr_object = ptr_object;
   checkObject = UObjectDB::checkIfObject;

   UHashMapObjectDumpable::callForAllEntry(UObjectDB::printObjLive);

   U_INTERNAL_ASSERT(lbuf >= buffer)
   U_INTERNAL_ASSERT_MINOR(lbuf, lend)

   return (lbuf - buffer);
}

// sorting object live for time creation...

uint32_t                UObjectDB::n;
const UObjectDumpable** UObjectDB::vec_obj_live;

U_NO_EXPORT bool UObjectDB::addObjLive(const UObjectDumpable* dumper)
{
   U_INTERNAL_TRACE("UObjectDB::addObjLive(%p)", dumper)

   if (n < 8192)
      {
      vec_obj_live[n++] = dumper;

      return true;
      }

   return false;
}

U_NO_EXPORT int __pure UObjectDB::compareDumper(const void* dumper1, const void* dumper2)
{
   U_INTERNAL_TRACE("UObjectDB::compareDumper(%p,%p)", dumper1, dumper2)

   int cmp = ((*(const UObjectDumpable**)dumper1)->cnt <
              (*(const UObjectDumpable**)dumper2)->cnt ? -1 : 1);

   return cmp;
}

void UObjectDB::dumpObjects()
{
   U_INTERNAL_TRACE("UObjectDB::dumpObjects()")

   const UObjectDumpable* obj_live[8192];

   vec_obj_live = &obj_live[n = 0];

   UHashMapObjectDumpable::callForAllEntry(UObjectDB::addObjLive);

   U_INTERNAL_ASSERT(n <= UHashMapObjectDumpable::num)

   qsort(obj_live, n, sizeof(const UObjectDumpable*), compareDumper);

   for (uint32_t i = 0; i < n; ++i)
      {
      const UObjectDumpable* dumper = obj_live[i];

      if (dumper->level >= level_active)
         {
         UObjectIO::buffer_output_len = 0;

         dumpObject(dumper);

         _write(liov, 8);
         }
      }
}
