/* mremap.c */

#include <ulib/base/base.h>

#include <errno.h>

#ifndef PAGE_MASK
#define PAGE_MASK 0xFFFFF000
#endif
#ifndef MREMAP_MAYMOVE
#define MREMAP_MAYMOVE 1
#endif

/**
 * Expand (or shrink) an existing mapping, potentially moving it at the 
 * same time (controlled by the MREMAP_MAYMOVE flag and available VM space) 
 */ 

#ifdef __UNIKERNEL__
extern U_EXPORT void* mremap(void* addr, size_t old_len, void* new_addr, size_t new_len, int _flags);
       U_EXPORT void* mremap(void* addr, size_t old_len, void* new_addr, size_t new_len, int _flags)
#else
extern U_EXPORT void* mremap(void* addr, size_t old_len,                 size_t new_len, int _flags);
       U_EXPORT void* mremap(void* addr, size_t old_len,                 size_t new_len, int _flags)
#endif
{
   if (((unsigned long)addr & (~PAGE_MASK)))
      {
      errno = EINVAL;

      return (void*)-1;
      }

   if (!(_flags & MREMAP_MAYMOVE))
      {
      errno = ENOSYS;

      return (void*)-1;
      }

   /**
    * old_len = PAGE_ALIGN(old_len);
    * new_len = PAGE_ALIGN(new_len);
    */

   /* Always allow a shrinking remap: that just unmaps the unnecessary pages.. */ 

   if (old_len > new_len)
      {
      munmap((char*)addr+new_len, old_len - new_len);

      return addr;
      }

   errno = ENOSYS;

   return (void*)-1;
}
