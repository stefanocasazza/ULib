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

U_EXPORT int UObjectDB::fd = -1;
U_EXPORT int UObjectDB::level_active;

U_NO_EXPORT iovec UObjectDB::liov[8] = {
   { U_NULLPTR, 0 },
   { (caddr_t) UObjectDB::buffer1, 0 },
   { (caddr_t) UObjectDB::buffer2, 0 },
   { U_NULLPTR, 0 },
   { (caddr_t) UObjectDB::buffer3, 0 },
   { (caddr_t) U_CONSTANT_TO_PARAM(U_LF2) },
   { U_NULLPTR, 0 },
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
U_NO_EXPORT void*       UObjectDB::_ptr_object;
U_NO_EXPORT uint32_t    UObjectDB::file_size;
U_NO_EXPORT bPFpcpv     UObjectDB::checkObject;
U_NO_EXPORT const char* UObjectDB::_name_class;

typedef bool (*vPFpObjectDumpable)(UObjectDumpable*);

class U_NO_EXPORT UHashMapObjectDumpable {
public:
   UObjectDumpable* objDumper;

   UHashMapObjectDumpable()
      {
      U_INTERNAL_TRACE("UHashMapObjectDumpable::UHashMapObjectDumpable()")

      objDumper = U_NULLPTR;

      next = U_NULLPTR;
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

#ifdef USE_LIBMIMALLOC
      table = (UHashMapObjectDumpable**) mi_calloc((table_size = U_GET_NEXT_PRIME_NUMBER(size)), sizeof(UHashMapObjectDumpable*));
#else
      table = (UHashMapObjectDumpable**)    calloc((table_size = U_GET_NEXT_PRIME_NUMBER(size)), sizeof(UHashMapObjectDumpable*));
#endif
      }

   static void lookup(void* ptr_object)
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

#  ifdef USE_LIBMIMALLOC
      mi_free(old_table);
#  else
      free(old_table);
#  endif
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
         U_DEBUG("UHashMapObjectDumpable::insert() - ptr_object = %p base_class = %s derived_class = %s", dumper->ptr_object, node->objDumper->name_class, dumper->name_class);
         */

#     ifdef USE_LIBMIMALLOC
         (void) mi_free((void*)node->objDumper->name_file);
         (void) mi_free((void*)node->objDumper->name_function);
#     else
         (void) free((void*)node->objDumper->name_file);
         (void) free((void*)node->objDumper->name_function);
#     endif

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

   static void erase(void* ptr_object)
      {
      U_INTERNAL_TRACE("UHashMapObjectDumpable::erase(%p)", ptr_object)

      lookup(ptr_object);

      if (node == U_NULLPTR)
         {
      // U_DEBUG("UObjectDB::unregisterObject(%p) = false", ptr_object);

         return;
         }

      UHashMapObjectDumpable* prev = U_NULLPTR;

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

#  ifdef USE_LIBMIMALLOC
      (void) mi_free((void*)node->objDumper->name_file);
      (void) mi_free((void*)node->objDumper->name_function);
#  else
      (void) free((void*)node->objDumper->name_file);
      (void) free((void*)node->objDumper->name_function);
#  endif

      delete node->objDumper;
      delete node;

      --num;

      U_INTERNAL_PRINT("num = %u", num)
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

void UObjectDB::init(bool flag)
{
   U_INTERNAL_TRACE("UObjectDB::init(%b)", flag)

   char* env = getenv("UOBJDUMP");

   if ( env &&
       *env)
      {
      if (u__isquote(*env)) ++env; // normalizzazione...

      // format: <level_active> <max_size_log> <table_size>
      //                1           500k           100

      char suffix;
      char name[U_PATH_MAX];
      uint32_t table_size = 0;

      (void) sscanf(env, "%d%u%c%u", &level_active, &file_size, &suffix, &table_size);

      if (file_size) U_NUMBER_SUFFIX(file_size, suffix);

      (void) u__snprintf(name, U_PATH_MAX, U_CONSTANT_TO_PARAM("%s/object.%N.%P"), u_trace_folder);

      /* NB: O_RDWR is needed for mmap(MAP_SHARED)... */

      fd = open(name, O_CREAT | O_TRUNC | O_RDWR | O_BINARY | O_APPEND, 0666);

      if (fd == -1)
         {
         U_WARNING("Failed to create file %S%R - current working directory: %.*S - UTRACE_FOLDER: %S", name, 0, u_cwd_len, u_cwd, u_trace_folder);

         file_size = 0;

         return;
         }

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

            file_mem = (char*) mmap(U_NULLPTR, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

            if (file_mem == MAP_FAILED)
               {
               file_mem  = U_NULLPTR;

               (void) ftruncate(fd, (file_size = 0));
               }

            file_ptr   = file_mem;
            file_limit = file_mem + file_size;
            }
         }

      if (flag)
         {
         UHashMapObjectDumpable::init(table_size);

         u_atexit(&UObjectDB::close); // register function of close dump at exit...
         }
      }
}

U_NO_EXPORT void UObjectDB::_write(struct iovec* iov, int _n)
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
   U_TRACE_NO_PARAM(0, "UObjectDB::close()")

   int lfd = fd;
             fd = -1;

   U_INTERNAL_DUMP("fd = %d", lfd)

   if (lfd != -1)
      {
      u_trace_suspend = 1;

      dumpObjects();

      U_INTERNAL_DUMP("file_size = %u", file_size)

      if (file_size)
         {
         ptrdiff_t write_size = file_ptr - file_mem;

         U_INTERNAL_DUMP("write_size = %u", write_size)

         U_INTERNAL_ASSERT_MINOR(write_size, (ptrdiff_t)file_size)

      // (void)  msync(file_mem, write_size, MS_SYNC | MS_INVALIDATE);
         (void) munmap(file_mem, file_size);

         (void) ftruncate(lfd, write_size);
      // (void) fsync(lfd);

         file_size = 0;
         }

      (void) ::close(lfd);
      }
}

void UObjectDB::initFork()
{
   U_INTERNAL_TRACE("UObjectDB::initFork()")

   if (fd != -1)
      {
      U_INTERNAL_ASSERT_RANGE(0,fd,1024)

      if (file_size)
         {
         (void) munmap(file_mem, file_size);

         file_size = 0;
         }

      (void) ::close(fd);

      init(false);
      }
}

void UObjectDB::registerObject(UObjectDumpable* dumper)
{
   U_INTERNAL_TRACE("UObjectDB::registerObject(%p)", dumper)

   U_INTERNAL_ASSERT_POINTER(u_name_file)
   U_INTERNAL_ASSERT_POINTER(u_name_function)

   dumper->num_line      = u_num_line;
#ifdef USE_LIBMIMALLOC
   dumper->name_file     = mi_strdup(u_name_file);
   dumper->name_function = mi_strdup(u_name_function);
#else
   dumper->name_file     = strdup(u_name_file);
   dumper->name_function = strdup(u_name_function);
#endif

   UHashMapObjectDumpable::insert(dumper);
}

void UObjectDB::unregisterObject(void* ptr_object)
{
   U_INTERNAL_TRACE("UObjectDB::unregisterObject(%p)", ptr_object)

   UHashMapObjectDumpable::erase(ptr_object);
}

// dump

void UObjectDB::dumpObject(UObjectDumpable* dumper)
{
   U_TRACE(0, "UObjectDB::dumpObject(%p)", dumper)

   U_INTERNAL_DUMP("dumper->level = %d level_active = %u", dumper->level, level_active)

   U_INTERNAL_ASSERT(dumper->level >= level_active)

   liov[0].iov_len  =   strlen(dumper->name_class);
   liov[0].iov_base = (caddr_t)dumper->name_class;

   liov[1].iov_len = sprintf(buffer1, " %p size %u level %d", // cnt %09u",
                             dumper->ptr_object, dumper->sz_obj, dumper->level); //, dumper->cnt);

   liov[2].iov_len = sprintf(buffer2, "\n%s(%u)\n", dumper->name_file, dumper->num_line);

   U_SYSCALL_FREE((void*)dumper->name_file);

   liov[3].iov_len  =   strlen(dumper->name_function);
   liov[3].iov_base = (caddr_t)dumper->name_function;

// U_DUMP_IOVEC(liov,4)

   U_INTERNAL_ASSERT_EQUALS(liov[1].iov_len, strlen(buffer1))
   U_INTERNAL_ASSERT_EQUALS(liov[2].iov_len, strlen(buffer2))

   void* _this = (dumper->psentinel ? *(dumper->psentinel) : (void*)U_CHECK_MEMORY_SENTINEL);

   if (_this == (void*)U_CHECK_MEMORY_SENTINEL) liov[4].iov_len = 0;
   else
      {
      liov[4].iov_len = sprintf(buffer3, "\nERROR ON MEMORY [sentinel = " U_CHECK_MEMORY_SENTINEL_STR " _this = %p - %s]\n", _this, (_this ? "ABW" : "FMR"));

      U_INTERNAL_ASSERT_EQUALS(liov[4].iov_len, strlen(buffer3))
      }

   liov[6].iov_base = (caddr_t) dumper->dump();
   liov[6].iov_len  = (UObjectIO::buffer_output_len ? UObjectIO::buffer_output_len : u__strlen((const char*)liov[6].iov_base, __PRETTY_FUNCTION__));

   U_DUMP_IOVEC(liov,7)
}

// dump single object...

U_NO_EXPORT bool __pure UObjectDB::checkIfObject(const char* name_class, void* ptr_object)
{
   U_VAR_UNUSED(name_class)

   U_INTERNAL_TRACE("UObjectDB::checkIfObject(%S,%p)", name_class, ptr_object)

   if (ptr_object == _ptr_object) return true;

   return false;
}

U_NO_EXPORT bool UObjectDB::printObjLive(UObjectDumpable* dumper)
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

uint32_t UObjectDB::dumpObject(char* buffer, uint32_t buffer_size, void* ptr_object)
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

uint32_t          UObjectDB::n;
UObjectDumpable** UObjectDB::vec_obj_live;

U_NO_EXPORT bool UObjectDB::addObjLive(UObjectDumpable* dumper)
{
   U_TRACE(0, "UObjectDB::addObjLive(%p)", dumper)

   if (n < 8192)
      {
      vec_obj_live[n++] = dumper;

      U_RETURN(true);
      }

   U_RETURN(false);
}

U_NO_EXPORT int __pure UObjectDB::compareDumper(const void* dumper1, const void* dumper2)
{
   U_TRACE(0, "UObjectDB::compareDumper(%p,%p)", dumper1, dumper2)

   int cmp = ((*(UObjectDumpable**)dumper1)->cnt <
              (*(UObjectDumpable**)dumper2)->cnt ? -1 : 1);

   U_RETURN(cmp);
}

void UObjectDB::dumpObjects()
{
   U_TRACE_NO_PARAM(0, "UObjectDB::dumpObjects()")

   UObjectDumpable* obj_live[8192];

   vec_obj_live = &obj_live[n = 0];

   UHashMapObjectDumpable::callForAllEntry(UObjectDB::addObjLive);

   if (n)
      {
      qsort(obj_live, n, sizeof(UObjectDumpable*), compareDumper);

   // u_trace_suspend = 0;

      U_INTERNAL_DUMP("n = %u", n)

      U_INTERNAL_ASSERT(n <= UHashMapObjectDumpable::num)

      for (uint32_t i = 0; i < n; ++i)
         {
         UObjectDumpable* dumper = obj_live[i];

         if (dumper->level >= level_active)
            {
            UObjectIO::buffer_output_len = 0;

            dumpObject(dumper);

            _write(liov, 8);

            U_SYSCALL_FREE((void*)dumper->name_function);
            }
         }
      }
}
