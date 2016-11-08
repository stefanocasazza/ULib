// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    memory_pool.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>

// --------------------------------------------------------------------------------------
// U_NUM_ENTRY_MEM_BLOCK: number of blocks to preallocate for 'type' stack

#define U_NUM_ENTRY_MEM_BLOCK 32

// U_SIZE_MEM_BLOCK: total space to preallocate for the various 'type' stack defined (~256k bytes)

#define U_SIZE_MEM_BLOCK (U_STACK_TYPE_0  * U_NUM_ENTRY_MEM_BLOCK + \
                          U_STACK_TYPE_1  * U_NUM_ENTRY_MEM_BLOCK + \
                          U_STACK_TYPE_2  * U_NUM_ENTRY_MEM_BLOCK + \
                          U_STACK_TYPE_3  * U_NUM_ENTRY_MEM_BLOCK + \
                          U_STACK_TYPE_4  * U_NUM_ENTRY_MEM_BLOCK + \
                          U_STACK_TYPE_5  * U_NUM_ENTRY_MEM_BLOCK + \
                          U_STACK_TYPE_6  * U_NUM_ENTRY_MEM_BLOCK + \
                          U_STACK_TYPE_7  * U_NUM_ENTRY_MEM_BLOCK + \
                          U_STACK_TYPE_8  * U_NUM_ENTRY_MEM_BLOCK + \
                          U_STACK_TYPE_9  * U_NUM_ENTRY_MEM_BLOCK)
// --------------------------------------------------------------------------------------

#ifdef DEBUG
const char* UMemoryPool::obj_class;
const char* UMemoryPool::func_call;
#ifdef U_STDCPP_ENABLE
#  include <fstream>
#endif
#endif

const uint32_t UMemoryPool::U_STACK_INDEX_TO_SIZE[U_NUM_STACK_TYPE] = { U_STACK_TYPE_0, U_STACK_TYPE_1, U_STACK_TYPE_2, U_STACK_TYPE_3, U_STACK_TYPE_4,
                                                                        U_STACK_TYPE_5, U_STACK_TYPE_6, U_STACK_TYPE_7, U_STACK_TYPE_8, U_MAX_SIZE_PREALLOCATE };

typedef struct ustackmemorypool {
#ifdef DEBUG
   void* _this;
#endif
   void** pointer_block;
   uint32_t type, len, space;
   uint32_t index;
#ifdef DEBUG
   uint32_t depth, max_depth,
            pop_cnt, push_cnt,
            num_call_allocateMemoryBlocks;
#endif
} ustackmemorypool;

/*
           --   --        --   --   --        --   -- 
  10      |xx| |  |  --  |  | |  | |  |      |  | |  | -> space
   9      |xx| |  | |xx| |xx| |  | |xx|  --  |  | |xx|
   8  --  |xx| |  | |xx| |xx| |xx| |xx| |  | |xx| |xx|
   7 |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| -> len
   6 |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx|
   5 |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx|
   4 |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx|
   3 |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx|
   2 |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx|
   1 |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx| |xx|
      --   --   --   --   --   --   --   --   --   -- 
       8   24   32   56  128  256  512  1024 2048 4096 -> type
       0    1    2    3    4    5    6     7    8    9 -> index
*/

class U_NO_EXPORT UStackMemoryPool {
public:

   // Check for memory error
   U_MEMORY_TEST

   void** pointer_block;
   uint32_t type, len, space;
   uint32_t index;
#ifdef DEBUG
   uint32_t depth, max_depth,
            pop_cnt, push_cnt,
            num_call_allocateMemoryBlocks;
#endif

   void deallocatePointerBlock()
      {
      U_TRACE_NO_PARAM(0, "UStackMemoryPool::deallocatePointerBlock()")

#  if defined(ENABLE_MEMPOOL)
      U_INTERNAL_DUMP("&mem_pointer_block[  0] = %p", &mem_pointer_block[0])
      U_INTERNAL_DUMP("     pointer_block      = %p",      pointer_block)
      U_INTERNAL_DUMP("&mem_pointer_block[%3u] = %p",                    U_NUM_STACK_TYPE * U_NUM_ENTRY_MEM_BLOCK * 2,
                                                      &mem_pointer_block[U_NUM_STACK_TYPE * U_NUM_ENTRY_MEM_BLOCK * 2])

      if (pointer_block < &mem_pointer_block[0] ||
          pointer_block > &mem_pointer_block[U_NUM_STACK_TYPE * U_NUM_ENTRY_MEM_BLOCK * 2])
         {
         UMemoryPool::deallocate(pointer_block, space * sizeof(void*));
         }
#  endif
      }

   void growPointerBlock(uint32_t new_space)
      {
      U_TRACE(0, "UStackMemoryPool::growPointerBlock(%u)", new_space)

      U_INTERNAL_ASSERT_MAJOR(index, 0)

      // NB: we alloc the space for total number pointer (block allocate previous + a new set of blocks) relativ to type 'dimension' current stack...

      uint32_t size = new_space * sizeof(void*);

      void** new_block = (void**) UFile::mmap(&size, -1, PROT_READ | PROT_WRITE, MAP_PRIVATE | U_MAP_ANON, 0);

      new_space = (size / sizeof(void*));

      if (len) U_MEMCPY(new_block, pointer_block, len * sizeof(void*));

      deallocatePointerBlock();

      space         = new_space;
      pointer_block = new_block;

      U_INTERNAL_DUMP("index = %u type = %u len = %u space = %u depth = %u max_depth = %u pop_cnt = %u push_cnt = %u num_call_allocateMemoryBlocks = %u",
                       index,     type,     len,     space,     depth,     max_depth,     pop_cnt,     push_cnt,     num_call_allocateMemoryBlocks)
      }

   void allocateMemoryBlocks(uint32_t n)
      {
      U_TRACE(0, "UStackMemoryPool::allocateMemoryBlocks(%u)", n)

      U_INTERNAL_ASSERT_MAJOR(n, len)

      uint32_t num_entry = (n - len),
               size      = num_entry * type;

      U_INTERNAL_DUMP("num_entry = %u size = %u", num_entry, size)

      if (index == 0)
         {
         U_INTERNAL_ASSERT_EQUALS(len, 0)

         pointer_block = (void**) UFile::mmap(&size, -1, PROT_READ | PROT_WRITE, MAP_PRIVATE |  U_MAP_ANON, 0);
           len = space = (size / type);

#     if defined(DEBUG) && defined(ENABLE_MEMPOOL)
         (void) U_SYSCALL(memset, "%p,%d,%u", pointer_block, 0, size); // NB: for check duplicate entry...
#     endif
         }
      else
         {
         char* pblock = (char*) UMemoryPool::_malloc(&num_entry, type);

         uint32_t new_len = len + num_entry;

         U_INTERNAL_DUMP("num_entry = %u new_len = %u", num_entry, new_len)

         char* eblock = pblock + (num_entry * type);

         if (space <= new_len) growPointerBlock(space + num_entry); // NB: this call can change len...

         do {
                                           eblock -= type;
            pointer_block[len++] = (void*) eblock;
            }
         while (--num_entry);

         U_INTERNAL_ASSERT_EQUALS(pblock, eblock)
         }

#  ifdef DEBUG
      ++num_call_allocateMemoryBlocks;
#  endif

      U_INTERNAL_DUMP("index = %u type = %u len = %u space = %u depth = %u max_depth = %u pop_cnt = %u push_cnt = %u num_call_allocateMemoryBlocks = %u",
                       index,     type,     len,     space,     depth,     max_depth,     pop_cnt,     push_cnt,     num_call_allocateMemoryBlocks)
      }

   void* pop()
      {
      U_INTERNAL_ASSERT_MINOR(index, U_NUM_STACK_TYPE) // 10

      if (len == 0) allocateMemoryBlocks(space);

      void** pblock = pointer_block + --len;

      void* ptr = (index == 0 ? (void*)pblock : *pblock);

#  ifdef DEBUG
      U_INTERNAL_ASSERT_EQUALS(((long)ptr & (sizeof(long)-1)), 0) // memory aligned

      if (++depth > max_depth) max_depth = depth;

      ++pop_cnt;
#  endif

      return ptr;
      }

   void push(void* ptr)
      {
      U_INTERNAL_ASSERT_MAJOR(index, 0)
      U_INTERNAL_ASSERT_MINOR(index, U_NUM_STACK_TYPE) // 10
      U_INTERNAL_ASSERT_EQUALS(index, U_SIZE_TO_STACK_INDEX(type))
      U_INTERNAL_ASSERT_EQUALS(type, UMemoryPool::U_STACK_INDEX_TO_SIZE[index])
      U_INTERNAL_ASSERT_EQUALS(((long)ptr & (sizeof(long)-1)), 0) // memory aligned

      if (len == space) growPointerBlock(space << 1); // NB: this call can change len...

#  if defined(DEBUG) && defined(ENABLE_MEMPOOL)
      if (ptr < &mem_block[0] ||
          ptr > &mem_block[U_SIZE_MEM_BLOCK])
         {
         (void) memset((void*)ptr, 0, type); // NB: in debug mode the memory area is zeroed to enhance showing bugs...
         }
#  endif

      pointer_block[len++] = (void*)ptr;

#  ifdef DEBUG
      if (depth) --depth;

      ++push_cnt;
#  endif
      }

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   static void paint(ostream& os); // print info
#endif

#if defined(ENABLE_MEMPOOL)
   static char  mem_block[U_SIZE_MEM_BLOCK];
   static void* mem_pointer_block[U_NUM_STACK_TYPE * U_NUM_ENTRY_MEM_BLOCK * 2];

   static ustackmemorypool mem_stack[U_NUM_STACK_TYPE]; // 10
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UStackMemoryPool)
};

#ifdef ENABLE_MEMPOOL
void UMemoryPool::allocateMemoryBlocks(int stack_index, uint32_t n)
{
   U_TRACE(0+256, "UMemoryPool::allocateMemoryBlocks(%d,%u)", stack_index, n)

   U_INTERNAL_ASSERT_MAJOR(stack_index, 0)
   U_INTERNAL_ASSERT_MINOR(stack_index, U_NUM_STACK_TYPE) // 10

   UStackMemoryPool* pstack = (UStackMemoryPool*)(UStackMemoryPool::mem_stack+stack_index);

   U_INTERNAL_DUMP("stack[%u]: type = %4u len = %5u space = %5u depth = %4u max_depth = %4u pop_cnt = %5u push_cnt = %5u allocateMemoryBlocks = %u",
                        stack_index, pstack->type, pstack->len, pstack->space,
                        pstack->depth, pstack->max_depth,
                        pstack->pop_cnt, pstack->push_cnt,
                        pstack->num_call_allocateMemoryBlocks);

   if (n > pstack->len) pstack->allocateMemoryBlocks(n);
}

void UMemoryPool::allocateMemoryBlocks(const char* ptr)
{
   U_TRACE(0+256, "UMemoryPool::allocateMemoryBlocks(%S)", ptr)

   U_INTERNAL_ASSERT_POINTER(ptr)

   void* addr;
   uint32_t i;
   char* endptr;
   UStackMemoryPool* pstack;
   int memblock[U_NUM_STACK_TYPE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

   // NB: start from 1... (Ex: 768,768,0,1536,2085,0,0,0,121:268435456,274432)

   for (i = 1; i < U_NUM_STACK_TYPE; ++i)
      {
      memblock[i] = strtol(ptr, &endptr, 10);

      if (endptr == ptr) break;

      ptr = endptr + 1;
      }

   if (*endptr == ':')
      {
      int value = strtol(ptr, &endptr, 10);

      if (value) UFile::rlimit_memalloc = value;

      value = strtol(endptr + 1, &endptr, 10);

      if (value) UFile::rlimit_memfree = value;
      }

#if defined(U_LINUX) && defined(MAP_HUGE_2MB)
   if (UFile::rlimit_memfree == U_2M)
      {
#  ifdef DEBUG
      char buffer[256];

      if (u_err_buffer == 0) u_err_buffer = buffer;
#  endif

      // cat /proc/meminfo | grep Huge

      UFile::nr_hugepages = UFile::setSysParam("/proc/sys/vm/nr_hugepages", 64);

      U_DEBUG("Creation of 64 huge pages %s", UFile::nr_hugepages ? "success" : "FAILED")
      }
#endif

   for (i = 1; i < U_NUM_STACK_TYPE; ++i)
      {
           if (memblock[i] > 0) allocateMemoryBlocks(i, memblock[i]);
      else if (memblock[i] < 0)
         {
         U_INTERNAL_ASSERT_MINOR(i, U_NUM_STACK_TYPE-1)
         U_INTERNAL_ASSERT_EQUALS(U_STACK_INDEX_TO_SIZE[i] * 2, U_STACK_INDEX_TO_SIZE[i+1])

         pstack = (UStackMemoryPool*)(UStackMemoryPool::mem_stack+i);

         memblock[i] += (memblock[i] & 1); // NB: must be even...

         if (i < (U_NUM_STACK_TYPE-1))
            {
            do {
               addr = (pstack->pop(),
                       pstack->pop());

               pstack = (UStackMemoryPool*)(UStackMemoryPool::mem_stack+i+1);

               pstack->push(addr);

               memblock[i] += 2;
               }
            while (memblock[i] < 0);

            U_INTERNAL_ASSERT_EQUALS(memblock[i], 0)
            }
         }
      }
}

void UMemoryPool::push(void* ptr, int stack_index)
{
   U_TRACE(0+256, "UMemoryPool::push(%p,%d)", ptr, stack_index) // problem with sanitize address

   U_INTERNAL_ASSERT_POINTER(ptr)
   U_INTERNAL_ASSERT_MINOR(stack_index, U_NUM_STACK_TYPE) // 10

   if (stack_index) ((UStackMemoryPool*)(UStackMemoryPool::mem_stack+stack_index))->push(ptr);
}

void UMemoryPool::_free(void* ptr, uint32_t num, uint32_t type_size)
{
   U_TRACE(1, "UMemoryPool::_free(%p,%u,%u)", ptr, num, type_size) // problem with sanitize address

   U_INTERNAL_ASSERT_POINTER(ptr)
   U_INTERNAL_ASSERT_MAJOR(num, 0)

   uint32_t length = (num * type_size);

   U_INTERNAL_DUMP("length = %u", length)

   if (length <= U_MAX_SIZE_PREALLOCATE)       push(ptr, U_SIZE_TO_STACK_INDEX(length));
   else                                  deallocate(ptr, UFile::getSizeAligned(length));
}

void* UMemoryPool::pop(int stack_index)
{
   U_TRACE(0+256, "UMemoryPool::pop(%d)", stack_index)

   U_INTERNAL_ASSERT_MINOR(stack_index, U_NUM_STACK_TYPE) // 10

   UStackMemoryPool* pstack = (UStackMemoryPool*)(UStackMemoryPool::mem_stack+stack_index);

#ifdef DEBUG
   if (pstack->index &&
       pstack->len == 0)
      {
      U_WARNING("We are going to call allocateMemoryBlocks() (pid %P) - object = %S func = %S"
                " index = %u type = %u len = %u space = %u depth = %u max_depth = %u num_call_allocateMemoryBlocks = %u pop_cnt = %u push_cnt = %u",
                  obj_class, func_call, pstack->index, pstack->type, pstack->len, pstack->space, pstack->depth,
                  pstack->max_depth, pstack->num_call_allocateMemoryBlocks, pstack->pop_cnt, pstack->push_cnt);
      }
#endif

   return pstack->pop();
}

#  ifdef DEBUG
bool UMemoryPool::check(void* ptr)
{
   U_TRACE(0, "UMemoryPool::check(%p)", ptr)

   UStackMemoryPool* pstack;

   for (uint32_t stack_index = 0; stack_index < U_NUM_STACK_TYPE; ++stack_index)
      {
      pstack = (UStackMemoryPool*)(UStackMemoryPool::mem_stack+stack_index);

      U_INTERNAL_DUMP("stack[%u]: type = %4u len = %5u space = %5u depth = %4u max_depth = %4u pop_cnt = %5u push_cnt = %5u allocateMemoryBlocks = %u",
                           stack_index, pstack->type, pstack->len, pstack->space,
                           pstack->depth, pstack->max_depth,
                           pstack->pop_cnt, pstack->push_cnt,
                           pstack->num_call_allocateMemoryBlocks);

      U_CHECK_MEMORY_OBJECT(pstack)

      for (uint32_t i = 0; i < pstack->len; ++i)
         {
         if (ptr == pstack->pointer_block[i])
            {
            U_ERROR("Duplicate entry on memory pool: stack[%u].index = %u stack[%u].len = %u stack[%u].space = %u stack[%u].pointer_block[%u] = %p",
                       stack_index,    pstack->index,
                       stack_index,    pstack->len,
                       stack_index,    pstack->space,
                       stack_index, i, pstack->pointer_block[i]);
            }
         }
      }

   U_RETURN(true);
}
#  endif
#endif

void* UMemoryPool::_malloc(uint32_t num, uint32_t type_size, bool bzero)
{
   U_TRACE(0, "UMemoryPool::_malloc(%u,%u,%b)", num, type_size, bzero)

   U_INTERNAL_ASSERT_MAJOR(num, 0)

   void* ptr;
   uint32_t length = (num * type_size);

   U_INTERNAL_DUMP("length = %u", length)

#ifndef ENABLE_MEMPOOL
# ifndef HAVE_ARCH64
   U_INTERNAL_ASSERT_RANGE(4, length, 1U * 1024U * 1024U * 1024U) // NB: over 1G is very suspect on 32bit...
# endif
   ptr = U_SYSCALL(malloc, "%u", length);
#else
   if (length <= U_MAX_SIZE_PREALLOCATE)
      {
      int stack_index = U_SIZE_TO_STACK_INDEX(length);

      ptr    = pop(stack_index);
      length = U_STACK_INDEX_TO_SIZE[stack_index];
      }
   else
      {
      ptr = UFile::mmap(&length, -1, PROT_READ | PROT_WRITE, MAP_PRIVATE | U_MAP_ANON, 0);

      U_INTERNAL_DUMP("length = %u", length)
      }
#endif

   if (bzero) (void) U_SYSCALL(memset, "%p,%d,%u", ptr, 0, length);

   U_RETURN(ptr);
}

void* UMemoryPool::_malloc(uint32_t* pnum, uint32_t type_size, bool bzero)
{
   U_TRACE(1, "UMemoryPool::_malloc(%p,%u,%b)", pnum, type_size, bzero)

   U_INTERNAL_ASSERT_POINTER(pnum)

   void* ptr;
   uint32_t length = (*pnum * type_size);

   U_INTERNAL_DUMP("length = %u", length)

#ifndef ENABLE_MEMPOOL
# ifndef HAVE_ARCH64
   U_INTERNAL_ASSERT_MINOR(length, 1U * 1024U * 1024U * 1024U) // NB: over 1G is very suspect on 32bit...
# endif
   ptr = U_SYSCALL(malloc, "%u", length);
#else
   if (length > U_MAX_SIZE_PREALLOCATE) ptr = UFile::mmap(&length, -1, PROT_READ | PROT_WRITE, MAP_PRIVATE | U_MAP_ANON, 0);
   else
      {
      int stack_index = U_SIZE_TO_STACK_INDEX(length);

      ptr    = pop(stack_index);
      length = U_STACK_INDEX_TO_SIZE[stack_index];
      }

   *pnum = length / type_size;

   U_INTERNAL_DUMP("*pnum = %u length = %u", *pnum, length)
#endif

   if (bzero) (void) U_SYSCALL(memset, "%p,%d,%u", ptr, 0, length);

   U_RETURN(ptr);
}

#if defined(ENABLE_MEMPOOL) && !defined(U_SERVER_CAPTIVE_PORTAL)
void UMemoryPool::deallocate(void* ptr, uint32_t length)
{
   U_TRACE(1, "UMemoryPool::deallocate(%p,%u)", ptr, length)

   if (UFile::isLastAllocation(ptr, length))
      {
      UFile::pfree  = (char*)ptr;
      UFile::nfree += length;

      U_INTERNAL_DUMP("UFile::nfree = %u UFile::pfree = %p", UFile::nfree, UFile::pfree)

      return;
      }

#if defined(U_LINUX) && defined(HAVE_ARCH64)
# if defined(MAP_HUGE_1GB) || defined(MAP_HUGE_2MB) // (since Linux 3.8)
   U_INTERNAL_DUMP("UFile::nr_hugepages = %ld", UFile::nr_hugepages)

   if (UFile::nr_hugepages == 0) // NB: MADV_DONTNEED cannot be applied to locked pages, Huge TLB pages, or VM_PFNMAP pages...
# endif
   {
   (void) U_SYSCALL(madvise, "%p,%lu,%d", (void*)ptr, length, MADV_DONTNEED); // causes the kernel to reclaim the indicated pages immediately and drop their contents

   return;
   }
#endif
   /**
    * munmap() is expensive. A series of page table entries must be cleaned up, and the VMA must be unlinked. By contrast, madvise(MADV_DONTNEED) only needs to set
    * a flag in the VMA and has the further benefit that no system call is required to reallocate the memory. That operation informs the kernel that the pages can
    * be (destructively) discarded from memory; if the process tries to access the pages again, they will either be faulted in from the underlying file, for a file
    * mapping, or re-created as zero-filled pages, for the anonymous mappings that are employed by user-space allocators. Of course, re-creating the pages zero filled
    * is normally exactly the desired behavior for a user-space memory allocator. (The only potential downside is that process address space is not freed, but this tends
    * not to matter on 64-bit systems)
    */

   (void) U_SYSCALL(munmap, "%p,%lu", (void*)ptr, length);
}
#endif

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
void UStackMemoryPool::paint(ostream& os) // paint info
{
   U_TRACE(0, "UStackMemoryPool::paint(%p)", &os)

# if defined(ENABLE_MEMPOOL)
   char buffer[256];
   uint32_t max_space = 0;
   UStackMemoryPool* pstack;

   for (int stack_index = 0; stack_index < U_NUM_STACK_TYPE; ++stack_index)
      {
      pstack = (UStackMemoryPool*)(UStackMemoryPool::mem_stack+stack_index);

      if (pstack->space > max_space) max_space = pstack->space;

      (void) snprintf(buffer, sizeof(buffer),
                      "stack[%u]: type = %4u len = %5u space = %5u allocateMemoryBlocks = %2u depth = %5u max_depth = %5u pop_cnt = %9u push_cnt = %9u\n",
                      stack_index,
                      pstack->type, pstack->len, pstack->space, pstack->num_call_allocateMemoryBlocks,
                      pstack->depth, pstack->max_depth, pstack->pop_cnt, pstack->push_cnt);

      os << buffer;
      }

   /*
   (void) snprintf(buffer, sizeof(buffer),
#     ifdef HAVE_ARCH64
          U_CONSTANT_TO_PARAM("\n        8   24   32   56  128  256  512  1024 2048 4096\n"
#     else
          U_CONSTANT_TO_PARAM("\n        4   16   36   48  128  256  512  1024 2048 4096\n"
#     endif
          "       %s   %s   %s   %s   %s   %s   %s   %s   %s   %s\n"),
          (mem_stack[0].space == max_space ? "--" : "  "),
          (mem_stack[1].space == max_space ? "--" : "  "),
          (mem_stack[2].space == max_space ? "--" : "  "),
          (mem_stack[3].space == max_space ? "--" : "  "),
          (mem_stack[4].space == max_space ? "--" : "  "),
          (mem_stack[5].space == max_space ? "--" : "  "),
          (mem_stack[6].space == max_space ? "--" : "  "),
          (mem_stack[7].space == max_space ? "--" : "  "),
          (mem_stack[8].space == max_space ? "--" : "  "),
          (mem_stack[9].space == max_space ? "--" : "  "));

   os << buffer;

   for (int32_t i = max_space-1; i >= 0; --i)
      {
      (void) snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%5u %c%s%c %c%s%c %c%s%c %c%s%c %c%s%c %c%s%c %c%s%c %c%s%c %c%s%c %c%s%c\n"), i+1,
          (mem_stack[0].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[0].space == (uint32_t)i ? "--" : (mem_stack[0].len > (uint32_t)i ? "xx" : "  ")),
          (mem_stack[0].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[1].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[1].space == (uint32_t)i ? "--" : (mem_stack[1].len > (uint32_t)i ? "xx" : "  ")),
          (mem_stack[1].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[2].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[2].space == (uint32_t)i ? "--" : (mem_stack[2].len > (uint32_t)i ? "xx" : "  ")),
          (mem_stack[2].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[3].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[3].space == (uint32_t)i ? "--" : (mem_stack[3].len > (uint32_t)i ? "xx" : "  ")),
          (mem_stack[3].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[4].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[4].space == (uint32_t)i ? "--" : (mem_stack[4].len > (uint32_t)i ? "xx" : "  ")),
          (mem_stack[4].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[5].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[5].space == (uint32_t)i ? "--" : (mem_stack[5].len > (uint32_t)i ? "xx" : "  ")),
          (mem_stack[5].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[6].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[6].space == (uint32_t)i ? "--" : (mem_stack[6].len > (uint32_t)i ? "xx" : "  ")),
          (mem_stack[6].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[7].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[7].space == (uint32_t)i ? "--" : (mem_stack[7].len > (uint32_t)i ? "xx" : "  ")),
          (mem_stack[7].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[8].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[8].space == (uint32_t)i ? "--" : (mem_stack[8].len > (uint32_t)i ? "xx" : "  ")),
          (mem_stack[8].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[9].space >  (uint32_t)i ?  '|' :  ' '),
          (mem_stack[9].space == (uint32_t)i ? "--" : (mem_stack[9].len > (uint32_t)i ? "xx" : "  ")),
          (mem_stack[9].space >  (uint32_t)i ?  '|' :  ' '));

      os << buffer;
      }

   (void) snprintf(buffer, sizeof(buffer),
          U_CONSTANT_TO_PARAM("       --   --   --   --   --   --   --   --   --   --\n"
#     ifdef HAVE_ARCH64
          "        8   24   32   56  128  256  512  1024 2048 4096"
#     else
          "        4   16   36   48  128  256  512  1024 2048 4096\n"
#     endif
          "\n"));

   os << buffer << endl;
   */
# endif
}

void UMemoryPool::printInfo(ostream& os)
{
   U_TRACE(0+256, "UMemoryPool::printInfo(%p)", &os)

   UStackMemoryPool::paint(os);
}

void UMemoryPool::writeInfoTo(const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0+256, "UMemoryPool::writeInfoTo(%.*S,%u)", fmt_size, format, fmt_size)

# if defined(ENABLE_MEMPOOL) && !defined(_MSWINDOWS_)
   char name[256];

   va_list argp;
   va_start(argp, fmt_size);

   (void) u__vsnprintf(name, sizeof(name), format, fmt_size, argp);

   va_end(argp);

   std::ofstream of(name);

   if (!of) return;

   UStackMemoryPool::paint(of);
# endif
}
#endif

#if defined(ENABLE_MEMPOOL)
char UStackMemoryPool::mem_block[U_SIZE_MEM_BLOCK];
// ***************************************************************** memory_pool.dat
// U_SPACE + U_TYPE *   0, U_SPACE + U_TYPE *   1, U_SPACE + U_TYPE *   2,
// U_SPACE + U_TYPE *   3, U_SPACE + U_TYPE *   4, U_SPACE + U_TYPE *   5,
// U_SPACE + U_TYPE *   6, U_SPACE + U_TYPE *   7, U_SPACE + U_TYPE *   8,
// .................................................................
// U_SPACE + U_TYPE * 126, U_SPACE + U_TYPE * 127,
// ***************************************************************** memory_pool.dat
void* UStackMemoryPool::mem_pointer_block[U_NUM_STACK_TYPE * U_NUM_ENTRY_MEM_BLOCK * 2] = {
#undef  U_TYPE
#define U_TYPE U_STACK_TYPE_0
#undef  U_SPACE
#define U_SPACE mem_block
#include "./memory_pool.dat"
#undef  U_TYPE
#define U_TYPE U_STACK_TYPE_1
#undef  U_SPACE
#define U_SPACE (mem_block + U_STACK_TYPE_0 * U_NUM_ENTRY_MEM_BLOCK)
#include "./memory_pool.dat"
#undef  U_TYPE
#define U_TYPE U_STACK_TYPE_2
#undef  U_SPACE
#define U_SPACE (mem_block + U_STACK_TYPE_0 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_1 * U_NUM_ENTRY_MEM_BLOCK)
#include "./memory_pool.dat"
#undef  U_TYPE
#define U_TYPE U_STACK_TYPE_3
#undef  U_SPACE
#define U_SPACE (mem_block + U_STACK_TYPE_0 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_1 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_2 * U_NUM_ENTRY_MEM_BLOCK)
#include "./memory_pool.dat"
#undef  U_TYPE
#define U_TYPE U_STACK_TYPE_4
#undef  U_SPACE
#define U_SPACE (mem_block + U_STACK_TYPE_0 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_1 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_2 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_3 * U_NUM_ENTRY_MEM_BLOCK)
#include "./memory_pool.dat"
#undef  U_TYPE
#define U_TYPE U_STACK_TYPE_5
#undef  U_SPACE
#define U_SPACE (mem_block + U_STACK_TYPE_0 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_1 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_2 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_3 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_4 * U_NUM_ENTRY_MEM_BLOCK)
#include "./memory_pool.dat"
#undef  U_TYPE
#define U_TYPE U_STACK_TYPE_6
#undef  U_SPACE
#define U_SPACE (mem_block + U_STACK_TYPE_0 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_1 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_2 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_3 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_4 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_5 * U_NUM_ENTRY_MEM_BLOCK)
#include "./memory_pool.dat"
#undef  U_TYPE
#define U_TYPE U_STACK_TYPE_7
#undef  U_SPACE
#define U_SPACE (mem_block + U_STACK_TYPE_0 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_1 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_2 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_3 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_4 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_5 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_6 * U_NUM_ENTRY_MEM_BLOCK)
#include "./memory_pool.dat"
#undef  U_TYPE
#define U_TYPE U_STACK_TYPE_8
#undef  U_SPACE
#define U_SPACE (mem_block + U_STACK_TYPE_0 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_1 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_2 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_3 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_4 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_5 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_6 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_7 * U_NUM_ENTRY_MEM_BLOCK)
#include "./memory_pool.dat"
#undef  U_TYPE
#define U_TYPE U_STACK_TYPE_9
#undef  U_SPACE
#define U_SPACE (mem_block + U_STACK_TYPE_0 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_1 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_2 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_3 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_4 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_5 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_6 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_7 * U_NUM_ENTRY_MEM_BLOCK + \
                             U_STACK_TYPE_8 * U_NUM_ENTRY_MEM_BLOCK)
#include "./memory_pool.dat"
};

// struttura e classe che encapsula lo stack dinamico di puntatori a blocchi preallocati per un 'type' dimensione definito
//
// typedef struct ustackmemorypool {
// #ifdef DEBUG
//    void* _this;
// #endif
//    void** pointer_block;
//    uint32_t type, len, space;
//    uint32_t index;
// #ifdef DEBUG
//    uint32_t depth, max_depth,
//             pop_cnt, push_cnt,
//             num_call_allocateMemoryBlocks;
// #endif
// } ustackmemorypool;

ustackmemorypool UStackMemoryPool::mem_stack[U_NUM_STACK_TYPE] = { { // 10
#ifdef DEBUG
   (void*)U_CHECK_MEMORY_SENTINEL,
#endif
   mem_pointer_block + 0 * U_NUM_ENTRY_MEM_BLOCK * 2,
   U_STACK_TYPE_0, U_NUM_ENTRY_MEM_BLOCK, U_NUM_ENTRY_MEM_BLOCK * 2,
   0
#ifdef DEBUG
   , 0, 0, 0, 0, 0
#endif
   }, {
#ifdef DEBUG
   (void*)U_CHECK_MEMORY_SENTINEL,
#endif
   mem_pointer_block + 1 * U_NUM_ENTRY_MEM_BLOCK * 2,
   U_STACK_TYPE_1, U_NUM_ENTRY_MEM_BLOCK, U_NUM_ENTRY_MEM_BLOCK * 2,
   1
#ifdef DEBUG
   , 0, 0, 0, 0, 0
#endif
   }, {
#ifdef DEBUG
   (void*)U_CHECK_MEMORY_SENTINEL,
#endif
   mem_pointer_block + 2 * U_NUM_ENTRY_MEM_BLOCK * 2,
   U_STACK_TYPE_2, U_NUM_ENTRY_MEM_BLOCK, U_NUM_ENTRY_MEM_BLOCK * 2,
   2
#ifdef DEBUG
   , 0, 0, 0, 0, 0
#endif
   }, {
#ifdef DEBUG
   (void*)U_CHECK_MEMORY_SENTINEL,
#endif
   mem_pointer_block + 3 * U_NUM_ENTRY_MEM_BLOCK * 2,
   U_STACK_TYPE_3, U_NUM_ENTRY_MEM_BLOCK, U_NUM_ENTRY_MEM_BLOCK * 2,
   3
#ifdef DEBUG
   , 0, 0, 0, 0, 0
#endif
   }, {
#ifdef DEBUG
   (void*)U_CHECK_MEMORY_SENTINEL,
#endif
   mem_pointer_block + 4 * U_NUM_ENTRY_MEM_BLOCK * 2,
   U_STACK_TYPE_4, U_NUM_ENTRY_MEM_BLOCK, U_NUM_ENTRY_MEM_BLOCK * 2,
   4
#ifdef DEBUG
   , 0, 0, 0, 0, 0
#endif
   }, {
#ifdef DEBUG
   (void*)U_CHECK_MEMORY_SENTINEL,
#endif
   mem_pointer_block + 5 * U_NUM_ENTRY_MEM_BLOCK * 2,
   U_STACK_TYPE_5, U_NUM_ENTRY_MEM_BLOCK, U_NUM_ENTRY_MEM_BLOCK * 2,
   5
#ifdef DEBUG
   , 0, 0, 0, 0, 0
#endif
   }, {
#ifdef DEBUG
   (void*)U_CHECK_MEMORY_SENTINEL,
#endif
   mem_pointer_block + 6 * U_NUM_ENTRY_MEM_BLOCK * 2,
   U_STACK_TYPE_6, U_NUM_ENTRY_MEM_BLOCK, U_NUM_ENTRY_MEM_BLOCK * 2,
   6
#ifdef DEBUG
   , 0, 0, 0, 0, 0
#endif
   }, {
#ifdef DEBUG
   (void*)U_CHECK_MEMORY_SENTINEL,
#endif
   mem_pointer_block + 7 * U_NUM_ENTRY_MEM_BLOCK * 2,
   U_STACK_TYPE_7, U_NUM_ENTRY_MEM_BLOCK, U_NUM_ENTRY_MEM_BLOCK * 2,
   7
#ifdef DEBUG
   , 0, 0, 0, 0, 0
#endif
   }, {
#ifdef DEBUG
   (void*)U_CHECK_MEMORY_SENTINEL,
#endif
   mem_pointer_block + 8 * U_NUM_ENTRY_MEM_BLOCK * 2,
   U_STACK_TYPE_8, U_NUM_ENTRY_MEM_BLOCK, U_NUM_ENTRY_MEM_BLOCK * 2,
   8
#ifdef DEBUG
   , 0, 0, 0, 0, 0
#endif
   }, {
#ifdef DEBUG
   (void*)U_CHECK_MEMORY_SENTINEL,
#endif
   mem_pointer_block + 9 * U_NUM_ENTRY_MEM_BLOCK * 2,
   U_STACK_TYPE_9, U_NUM_ENTRY_MEM_BLOCK, U_NUM_ENTRY_MEM_BLOCK * 2,
   9
#ifdef DEBUG
   , 0, 0, 0, 0, 0
#endif
   }
};
#endif
