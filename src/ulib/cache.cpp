// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    cache.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/cache.h>
#include <ulib/utility/dir_walk.h>
#include <ulib/utility/string_ext.h>

#define U_NO_TTL (uint32_t)-1

UCache::~UCache()
{
   U_TRACE_UNREGISTER_OBJECT(0, UCache)

   if (fd != -1)
      {
      UFile::close(fd);

      UFile::munmap(info, sizeof(UCache::cache_info) + info->size);
      }
}

U_NO_EXPORT void UCache::init(UFile& _x, uint32_t size, bool bexist, bool brdonly)
{
   U_TRACE(0, "UCache::init(%.*S,%u,%b,%b)", U_FILE_TO_TRACE(_x), size, bexist, brdonly)

   U_CHECK_MEMORY

   fd = _x.getFd();

   (void) _x.memmap(PROT_READ | (brdonly ? 0 : PROT_WRITE));

   char* ptr = _x.getMap();

   info = (cache_info*)ptr;
      x =              ptr + sizeof(UCache::cache_info);

   if (bexist) start = (_x.fstat(), _x.st_mtime);
   else
      {
      // 100 <= size <= 1000000000

      U_INTERNAL_ASSERT_RANGE(100U, size, 1000U * 1000U * 1000U)

      // hsize <= writer <= oldest <= unused <= size

      info->hsize = info->writer = getHSize((info->oldest = info->unused = info->size = (size - sizeof(UCache::cache_info))));

      U_INTERNAL_DUMP("hsize = %u writer = %u oldest = %u unused = %u size = %u", info->hsize, info->writer, info->oldest, info->unused, info->size)

      U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

      start = u_now->tv_sec;
      }
}

bool UCache::open(const UString& path, uint32_t size, const UString* environment)
{
   U_TRACE(0, "UCache::open(%V,%u,%p)", path.rep, size, environment)

   U_CHECK_MEMORY

   UFile _x(path, environment);

   if (_x.creat(O_RDWR))
      {
      init(_x, size, (_x.size() ? true : ((void)_x.ftruncate(size), false)), false);

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UCache::open(const UString& path, const UString& dir, const UString* environment, bool brdonly)
{
   U_TRACE(0, "UCache::open(%V,%V,%p,%b)", path.rep, dir.rep, environment, brdonly)

   U_CHECK_MEMORY

   UFile _x(path, environment),
         _y( dir, environment);

   if (_y.stat() &&
       _x.creat(O_RDWR))
      {
#  ifdef DEBUG
      dir_template       = _y.getPath();
      dir_template_mtime = _y.st_mtime;
#  endif

      bool exist = true;
      UDirWalk dirwalk(_y.getPath());
      UVector<UString> vec1(256), vec2;
      uint32_t i, n, size = 0, hsize = 0;

      if (( _x.size() == 0                          ||
           (_x.fstat(), _x.st_mtime < _y.st_mtime)) &&
          (UDirWalk::setFollowLinks(true), n = dirwalk.walk(vec1)))
         {
         exist = false;

         UString item, content;

         for (i = 0; i < n; ++i)
            {
            item = vec1[i];

            _y.setPath(item);

            content = _y.getContent();

            if (content)
               {
               vec2.push_back(content);

               size += sizeof(UCache::cache_hash_table_entry) + UStringExt::getBaseNameLen(item) + _y.getSize() + 1; // NB: 1 => (+null-terminator)...
               }
            }

          size += sizeof(UCache::cache_info);
         hsize  = getHSize(size);
         hsize  = getHSize(size + hsize);
          size +=                 hsize;

         (void) _x.ftruncate(size);
         }

      init(_x, size, exist, exist ? brdonly : false);

      if (exist == false)
         {
         U_INTERNAL_ASSERT_EQUALS(info->hsize, hsize)
         U_INTERNAL_ASSERT_EQUALS(info->size, size - sizeof(UCache::cache_info))

         for (i = 0, n = vec2.size(); i < n; ++i)
            {
            addContent(UStringExt::basename(vec1[i]), vec2[i]);

            U_ASSERT_EQUALS(getContent(UStringExt::basename(vec1[i])), vec2[i])
            }

         U_INTERNAL_DUMP("hsize = %u writer = %u oldest = %u unused = %u size = %u", info->hsize, info->writer, info->oldest, info->unused, info->size)

         U_INTERNAL_ASSERT_EQUALS(info->writer, info->size)

      // (void) U_SYSCALL(msync, "%p,%u,%d", (char*)info, sizeof(UCache::cache_info) + info->size, MS_SYNC); // flushes changes made to memory mapped file back to disk
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

char* UCache::add(const char* key, uint32_t keylen, uint32_t datalen, uint32_t _ttl)
{
   U_TRACE(0, "UCache::add(%.*S,%u,%u,%u)", keylen, key, keylen, datalen, _ttl)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(_ttl <= U_MAX_TTL)
   U_INTERNAL_ASSERT_RANGE(1U,  keylen, U_MAX_KEYLEN)
   U_INTERNAL_ASSERT_RANGE(1U, datalen, U_MAX_DATALEN)

   cache_hash_table_entry* e;
   uint32_t index, pos, entrylen = sizeof(UCache::cache_hash_table_entry) + keylen + datalen;

   U_INTERNAL_DUMP("writer = %u entrylen = %u oldest = %u unused = %u", info->writer, entrylen, info->oldest, info->unused)

   while ((info->writer + entrylen) > info->oldest)
      {
      if (info->oldest == info->unused)
         {
         if (info->writer <= info->hsize)
            {
            U_ERROR("Cache exhausted");

            U_RETURN((char*)0);
            }

         info->unused = info->writer;
         info->oldest = info->writer = info->hsize;
         }

      e = entry(info->oldest);

      U_INTERNAL_DUMP("entry = { %u, %u, %.*S, %u, %.*S, %#3D }", u_get_unaligned32(e->link),
                        u_get_unaligned32(e->keylen),
                        u_get_unaligned32(e->keylen),                                  (char*)(e+1),
                        u_get_unaligned32(e->datalen),
                        u_get_unaligned32(e->datalen), (u_get_unaligned32(e->keylen) + (char*)(e+1)),
                        u_get_unaligned32(e->time_expire))

      pos = u_get_unaligned32(e->link);

      replace(pos, info->oldest);

      info->oldest += sizeof(UCache::cache_hash_table_entry) + u_get_unaligned32(e->keylen) + u_get_unaligned32(e->datalen);

      U_INTERNAL_ASSERT(info->oldest <= info->unused)

      if (info->oldest == info->unused) info->unused = info->oldest = info->size;
      }

   index = hash(key, keylen);
     pos = getLink(index);

   e = setHead(index, info->writer);

   if (pos) replace(pos, index ^ info->writer);

   time_t expire = (_ttl ? getTime() + _ttl : U_NO_TTL);

   u_put_unaligned32(e->link,        pos ^ index);
   u_put_unaligned32(e->keylen,      keylen);
   u_put_unaligned32(e->datalen,     datalen);
   u_put_unaligned32(e->time_expire, expire);

   U_INTERNAL_DUMP("entry = { %u, %u, %.*S, %u, %.*S, %#3D }", u_get_unaligned32(e->link),
                        u_get_unaligned32(e->keylen),
                        u_get_unaligned32(e->keylen),                                  (char*)(e+1),
                        u_get_unaligned32(e->datalen),
                        u_get_unaligned32(e->datalen), (u_get_unaligned32(e->keylen) + (char*)(e+1)),
                        u_get_unaligned32(e->time_expire))

   char* p = x + info->writer + sizeof(UCache::cache_hash_table_entry);

   info->writer += entrylen;

   U_INTERNAL_DUMP("writer = %u entrylen = %u oldest = %u unused = %u", info->writer, entrylen, info->oldest, info->unused)

   U_RETURN(p);
}

void UCache::add(const UString& _key, const UString& _data, uint32_t _ttl)
{
   U_TRACE(0, "UCache::add(%V,%V,%u)", _key.rep, _data.rep, _ttl)

   const char*  key =  _key.data();
   const char* data = _data.data();
   uint32_t keylen  =  _key.size(),
            datalen = _data.size();

   char* ptr = add(key, keylen, datalen, _ttl);

   U_MEMCPY(ptr,           key,  keylen);
   U_MEMCPY(ptr + keylen, data, datalen);
}

void UCache::addContent(const UString& _key, const UString& content, uint32_t _ttl)
{
   U_TRACE(0, "UCache::addContent(%V,%V,%u)", _key.rep, content.rep, _ttl)

   U_INTERNAL_ASSERT(_key)
   U_INTERNAL_ASSERT(content)

   const char*  key =    _key.data();
   const char* data = content.data();
   uint32_t keylen  =    _key.size(),
            datalen = content.size() + 1; // NB: 1 => (+null-terminator)...

   char* ptr = add(key, keylen, datalen, _ttl);

   U_MEMCPY(ptr,           key,  keylen);
   U_MEMCPY(ptr + keylen, data, datalen);

   if (content.isNullTerminated() == false) ptr[keylen+datalen-1] = '\0';
}

UString UCache::get(const char* key, uint32_t keylen)
{
   U_TRACE(0, "UCache::get(%.*S,%u)", keylen, key, keylen)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_RANGE(1, keylen, U_MAX_KEYLEN)

   const char* p;
   uint32_t loop = 0,
            index = hash(key, keylen),
              pos = getLink(index), prevpos = index;

   while (pos)
      {
      cache_hash_table_entry* e = entry(pos);

      uint32_t time_expire = u_get_unaligned32(e->time_expire);

      U_INTERNAL_DUMP("entry = { %u, %u, %.*S, %u, %.*S, %#3D }", u_get_unaligned32(e->link),
                        u_get_unaligned32(e->keylen),
                        u_get_unaligned32(e->keylen),                                  (char*)(e+1),
                        u_get_unaligned32(e->datalen),
                        u_get_unaligned32(e->datalen), (u_get_unaligned32(e->keylen) + (char*)(e+1)),
                        time_expire)

      if (time_expire && // chek if entry is expired...
          u_get_unaligned32(e->keylen) == keylen)
         {
         U_INTERNAL_ASSERT((pos + sizeof(UCache::cache_hash_table_entry) + keylen) <= info->size)

         p = x + pos + sizeof(UCache::cache_hash_table_entry);

         if (memcmp(p, key, keylen) == 0)
            {
            if (time_expire != U_NO_TTL &&
                getTime() >= time_expire)
               {
               u_put_unaligned32(e->time_expire, 0); // set entry expired...

               break;
               }

            U_INTERNAL_ASSERT(u_get_unaligned32(e->datalen) <= (info->size - pos - sizeof(UCache::cache_hash_table_entry) - keylen))

            ttl = time_expire;

            UString str(p + keylen, u_get_unaligned32(e->datalen));

            U_RETURN_STRING(str);
            }
         }

      if (++loop > 100U) break; // to protect against hash flooding

      uint32_t nextpos = prevpos ^ getLink(pos);

      prevpos = pos;
      pos     = nextpos;
      }

   return UString::getStringNull();
}

UString UCache::getContent(const char* key, uint32_t keylen)
{
   U_TRACE(0, "UCache::getContent(%.*S,%u)", keylen, key, keylen)

#ifdef DEBUG
   struct stat st;
   UString buffer(U_PATH_MAX);

   buffer.snprintf(U_CONSTANT_TO_PARAM("%v/%.*s"), dir_template.rep, keylen, key);

   if (U_SYSCALL(stat, "%S,%p", buffer.data(), &st) == 0 &&
       st.st_mtime >= dir_template_mtime)
      {
      return UFile::contentOf(buffer);
      }
#endif

   UString content = get(key, keylen);

   if (content &&
       content.last_char() == '\0')
      {
      content.rep->_length -= 1; // NB: 1 => (-null-terminator)...

      U_INTERNAL_ASSERT(content.isNullTerminated())
      }

   U_RETURN_STRING(content);
}

void UCache::loadContentOf(const UString& dir, const char* filter, uint32_t filter_len)
{
   U_TRACE(1, "UCache::loadContentOf(%V,%.*S,%u)", dir.rep, filter_len, filter, filter_len)

   UString item, content;
   UVector<UString> vec(128);
   UDirWalk dirwalk(dir, filter, filter_len);

   for (uint32_t i = 0, n = dirwalk.walk(vec); i < n; ++i)
      {
      item    = vec[i];
      content = UFile::contentOf(item);

      if (content) addContent(UStringExt::basename(item), content);
      }

   U_INTERNAL_DUMP("hsize = %u writer = %u oldest = %u unused = %u size = %u", info->hsize, info->writer, info->oldest, info->unused, info->size)

   if (info->writer < info->size &&
       U_SYSCALL(ftruncate, "%d,%u", fd, info->writer + sizeof(UCache::cache_info)) == 0)
      {
      info->oldest = info->unused = info->size = info->writer;
      }
}

// STREAM

#ifdef U_STDCPP_ENABLE
U_EXPORT istream& operator>>(istream& is, UCache& cache)
{
   U_TRACE(0+256, "UCache::operator>>(%p,%p)", &is, &cache)

   char c;
   char* ptr;
   char key[U_MAX_KEYLEN];
   uint32_t keylen, datalen;

   while (is >> c)
      {
      U_INTERNAL_DUMP("c = %C", c)

      if (c == '#')
         {
         for (int ch = is.get(); (ch != '\n' && ch != EOF); ch = is.get()) {}

         continue;
         }

      if (c != '+') break;

      is >> keylen;

      is.get(); // skip ','
      is >> datalen;
      is.get(); // skip ':'

#  ifndef U_COVERITY_FALSE_POSITIVE /* TAINTED_SCALAR */
      is.read(key, keylen);
#  endif

      U_INTERNAL_DUMP("key = %.*S keylen = %u", keylen, key, keylen)

      U_INTERNAL_ASSERT_MINOR(keylen, U_MAX_KEYLEN)

      ptr = cache.add(key, keylen, datalen, 0);

#  ifndef U_COVERITY_FALSE_POSITIVE // coverity[TAINTED_SCALAR]
      U_MEMCPY(ptr, key, keylen);
#  endif

      is.get(); // skip '-'
      is.get(); // skip '>'

      ptr += keylen;

#  ifndef U_COVERITY_FALSE_POSITIVE /* TAINTED_SCALAR */
      is.read(ptr, datalen);
#  endif

      U_INTERNAL_DUMP("data = %.*S datalen = %u", datalen, ptr, datalen)

      is.get(); // skip '\n'
      }

   return is;
}

void UCache::print(ostream& os, uint32_t& pos) const
{
   U_INTERNAL_TRACE("UCache::print(%p,%u)", &os, pos)

   U_CHECK_MEMORY

   UCache::cache_hash_table_entry* e = (UCache::cache_hash_table_entry*)(x + pos);

   os.put('+');
   os << u_get_unaligned32(e->keylen);
   os.put(',');
   os << u_get_unaligned32(e->datalen);
   os.put(':');

   pos += sizeof(UCache::cache_hash_table_entry);
   os.write(x + pos, u_get_unaligned32(e->keylen));

   os.put('-');
   os.put('>');

   pos += u_get_unaligned32(e->keylen);
   os.write(x + pos, u_get_unaligned32(e->datalen));
   pos += u_get_unaligned32(e->datalen);

   os.put('\n');
}

U_EXPORT ostream& operator<<(ostream& os, const UCache& c)
{
   U_TRACE(0, "UCache::operator<<(%p,%p)", &os, &c)

   uint32_t pos = c.info->oldest;

   while (pos < c.info->unused) c.print(os, pos);

   pos = c.info->hsize;

   while (pos < c.info->writer) c.print(os, pos);

   os.put('\n');

   return os;
}

// DEBUG

#  ifdef DEBUG
const char* UCache::dump(bool _reset) const
{
   *UObjectIO::os << "x                     " << (void*)x             << '\n'
                  << "fd                    " << fd                   << '\n'
                  << "ttl                   " << ttl                  << '\n'
                  << "start                 " << (void*)start         << '\n'
                  << "size                  " << info->size           << '\n'
                  << "hsize                 " << info->hsize          << '\n'
                  << "writer                " << info->writer         << '\n'
                  << "oldest                " << info->oldest         << '\n'
                  << "unused                " << info->unused         << '\n'
                  << "dir_template_mtime    " << dir_template_mtime   << '\n'
                  << "dir_template (UString " << (void*)&dir_template << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#  endif
#endif
