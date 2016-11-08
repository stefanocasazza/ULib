// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    cache.h - A structure for fixed-size cache (Bernstein)
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_CACHE_H
#define ULIB_CACHE_H 1

#include <ulib/file.h>

/**
 * @class UCache
 *
 * @brief UCache is a structure for fixed-size cache.
 *
 * The first part of cache is a hash tables with (hsize / sizeof(uint32_t)) pointers to consecutive bucket linked lists.
 * +--------------------+--------------------------+------------+
 * | p0 p1 ... phsize-1 | entry0 entry1 ... entryn | free space |
 * +--------------------+--------------------------+------------+
 * The internal data structure of cache is the following structure:
 *
 * x[    0....hsize-1]  hsize / sizeof(uint32_t) head links.
 * x[hsize....writer-1] consecutive entries, newest entry on the right.
 * x[writer...oldest-1] free space for new entries.
 * x[oldest...unused-1] consecutive entries, oldest entry on the left.
 * x[unused...size-1]   unused.
 *
 * Each hash bucket is a linked list containing the following items:
 * the head link, the newest entry, the second-newest entry, etc.
 * Each link is a 4-byte number giving the xor of the positions of the adjacent items in the list.
 * Entries are always inserted immediately after the head and removed at the tail.
 * Each entry contains the following information: struct cache_hash_table_entry + key + data
 */

#define U_MAX_TTL         365L * U_ONE_DAY_IN_SECOND // 365 gg (1 year)
#define U_MAX_KEYLEN     1000U
#define U_MAX_DATALEN 1000000U

class U_EXPORT UCache {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UCache()
      {
      U_TRACE_REGISTER_OBJECT(0, UCache, "", 0)

      fd = -1;

      x     = 0;
      ttl   = 0;
      info  = 0;
      start = 0;

#  ifdef DEBUG
      dir_template_mtime = 0;
#  endif
      }

   ~UCache();

   // OPEN/CREAT a cache file

   bool open(const UString& path, uint32_t size,               const UString* environment = 0);
   bool open(const UString& path, const UString& dir_template, const UString* environment = 0, bool brdonly = false);

   // OPERATION

   uint32_t getHSize(uint32_t size) const
      {
      U_TRACE(0, "UCache::getHSize(%u)", size)

      // sizeof(uint32_t) <= hsize <= size / sizeof(cache_hash_table_entry) (hsize is a power of 2)

      uint32_t hsize = sizeof(uint32_t);

      while (hsize <= (size / sizeof(UCache::cache_hash_table_entry))) hsize <<= 1U;

      U_RETURN(hsize);
      }

   void add(       const UString& key, const UString& data,    uint32_t _ttl = 0);
   void addContent(const UString& key, const UString& content, uint32_t _ttl = 0); // NB: +null terminator...

   UString get(       const char* key, uint32_t len);
   UString getContent(const char* key, uint32_t len); // NB: -null terminator...

   UString getContent(const UString& key) { return getContent(U_STRING_TO_PARAM(key)); }

   void loadContentOf(const UString& directory, const char* filter = 0, uint32_t filter_len = 0);

   // operator []

   UString operator[](const UString& key) { return getContent(key); }

   // SERVICES

   uint32_t getTTL() const
      {
      U_TRACE_NO_PARAM(0, "UCache::getTTL()")

      U_RETURN(ttl);
      }

   uint32_t getTime() const
      {
      U_TRACE_NO_PARAM(0, "UCache::getTime()")

      U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

      uint32_t now = (uint32_t)(u_now->tv_sec - start);

      U_RETURN(now);
      }

   // STREAM

#ifdef U_STDCPP_ENABLE
   void print(ostream& os, uint32_t& pos) const;

   friend U_EXPORT istream& operator>>(istream& is,       UCache& c);
   friend U_EXPORT ostream& operator<<(ostream& os, const UCache& c);

   // DEBUG

# ifdef DEBUG
   const char* dump(bool reset) const;
# endif
#endif

protected:
   typedef struct cache_info {
      uint32_t size;    // size of cache
      uint32_t hsize;   // size of hash table
      uint32_t writer;  // pointer to free space
      uint32_t oldest;  // pointer to oldest entries
      uint32_t unused;  // pointer to unused space
      } cache_info;

   typedef struct cache_hash_table_entry {
      uint32_t link;
      uint32_t keylen;
      uint32_t datalen;
      uint32_t time_expire;
   // ------> keylen  array of char...
   // ------> datalen array of char...
   } cache_hash_table_entry;

   int fd;
   char* x;          // cache      pointer
   cache_info* info; // cache info pointer
   time_t start;     // time of reference
   uint32_t ttl;     // time to live (expire entry)
#ifdef DEBUG
   UString dir_template;
   time_t  dir_template_mtime;
#endif

   uint32_t hash(const char* key, uint32_t keylen)
      {
      U_TRACE(0, "UCache::hash(%.*S,%u)", keylen, key, keylen)

      uint32_t keyhash = u_cdb_hash((unsigned char*)key, keylen, -1) * sizeof(uint32_t) % info->hsize;

      U_RETURN(keyhash);
      }

   uint32_t getLink(uint32_t pos) const
      {
      U_TRACE(0, "UCache::getLink(%u)", pos)

      U_INTERNAL_ASSERT(pos <= (info->size - sizeof(uint32_t)))

      uint32_t value = u_get_unalignedp32(x + pos);

      U_INTERNAL_DUMP("value = %u info->size = %u", value, info->size)

      U_RETURN(value);
      }

    cache_hash_table_entry* setHead(uint32_t pos, uint32_t value)
      {
      U_TRACE(0, "UCache::setHead(%u,%u)", pos, value)

      U_INTERNAL_ASSERT_MINOR(pos,info->hsize)
      U_INTERNAL_ASSERT(value <= (info->size - sizeof(uint32_t)))

      char* ptr = x + pos;

      U_INTERNAL_DUMP("ptr = %p *ptr = %u", ptr, u_get_unalignedp32(ptr))

      u_put_unalignedp32(ptr, value);

      ptr = x + value;

      U_RETURN_POINTER(ptr, cache_hash_table_entry);
      }

   cache_hash_table_entry* entry(uint32_t pos) const
      {
      U_TRACE(0, "UCache::entry(%u)", pos)

      U_INTERNAL_DUMP("info->size = %u", info->size)

      U_INTERNAL_ASSERT(pos <= (info->size - sizeof(uint32_t)))

      cache_hash_table_entry* e = (cache_hash_table_entry*)(x + pos);

      U_RETURN_POINTER(e, cache_hash_table_entry);
      }

   void replace(uint32_t pos, uint32_t value)
      {
      U_TRACE(0, "UCache::replace(%u,%u)", pos, value)

      U_INTERNAL_ASSERT(pos   <= (info->size - sizeof(uint32_t)))
      U_INTERNAL_ASSERT(value <= (info->size - sizeof(uint32_t)))

      char* ptr = x + pos;

      U_INTERNAL_DUMP("*ptr = %u", u_get_unalignedp32(ptr))

      value ^= u_get_unalignedp32(ptr);

      u_put_unalignedp32(ptr, value);

      U_INTERNAL_DUMP("*ptr = %u", value)
      }

   char* add(const char* key, uint32_t keylen, uint32_t datalen, uint32_t ttl);

private:
   void init(UFile& _x, uint32_t size, bool bexist, bool brdonly) U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(UCache)
};

#endif
