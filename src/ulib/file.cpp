// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    file.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>
#include <ulib/tokenizer.h>
#include <ulib/utility/services.h>
#include <ulib/utility/dir_walk.h>
#include <ulib/utility/string_ext.h>

#ifdef USE_LIBMAGIC
#  include <ulib/magic/magic.h>
#endif

char*    UFile::cwd_save;
char*    UFile::pfree;
long     UFile::nr_hugepages;
uint32_t UFile::nfree;
uint32_t UFile::cwd_save_len;

#if defined(U_LINUX) && (defined(MAP_HUGE_1GB) || defined(MAP_HUGE_2MB)) // (since Linux 3.8)
uint32_t UFile::rlimit_memfree  = U_2M;
#else
uint32_t UFile::rlimit_memfree  = 16U*1024U;
#endif
uint32_t UFile::rlimit_memalloc = 256U*1024U*1024U;

#ifdef DEBUG
int UFile::num_file_object;

void UFile::inc_num_file_object(UFile* pthis)
{
   U_TRACE(0+256, "UFile::inc_num_file_object(%p)", pthis)

   ++num_file_object;

// U_INTERNAL_DUMP("this         = %p", pthis)
// U_INTERNAL_DUMP("&st_dev      = %p", &(pthis->st_dev))
// U_INTERNAL_DUMP("&st_ctime    = %p", &(pthis->st_ctime))
// U_INTERNAL_DUMP("memory._this = %p", pthis->memory._this)

   U_INTERNAL_ASSERT_EQUALS((void*)pthis, (void*)&(pthis->st_dev))
}

void UFile::dec_num_file_object(int fd)
{
   --num_file_object;

   if (fd != -1) U_WARNING("File descriptor %d not closed...", fd);
}

void UFile::chk_num_file_object()
{
   if (num_file_object) U_WARNING("UFile::chdir() with num file object = %d", num_file_object);
}
#endif

#ifndef MREMAP_MAYMOVE
#define MREMAP_MAYMOVE 1
#endif

void UFile::setPathRelativ(const UString* environment)
{
   U_TRACE(0, "UFile::setPathRelativ(%p)", environment)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT(pathname)

   reset();

   const char* ptr = (pathname.isNullTerminated() ? pathname.data() : pathname.c_str());

   char c = *ptr;

   if (c == '~' ||
       c == '$')
      {
      UString x = UStringExt::expandPath(pathname, environment);

      if (x)
         {
         pathname = x;

         ptr = pathname.data();
         }
      }

   // NB: the string can be not writable...

   path_relativ_len = pathname.size();
   path_relativ     = u_getPathRelativ(ptr, &path_relativ_len);

   U_INTERNAL_ASSERT_MAJOR(path_relativ_len, 0)

   // we don't need this... (I think)

   /*
   if (pathname.writeable() &&
       pathname.size() != path_relativ_len)
      {
      path_relativ[path_relativ_len] = '\0';
      }
   */

   U_INTERNAL_DUMP("u_cwd(%u) = %S", u_cwd_len, u_cwd)
   U_INTERNAL_DUMP("pathname(%u) = %V", pathname.size(), pathname.rep)
   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)
}

void UFile::setRoot()
{
   U_TRACE_NO_PARAM(0, "UFile::setRoot()")

   reset();

   pathname.setConstant(U_CONSTANT_TO_PARAM("/"));

   st_mode          = S_IFDIR|0755;
   path_relativ     = pathname.data();
   path_relativ_len = 1;

   U_INTERNAL_DUMP("u_cwd(%u) = %S", u_cwd_len, u_cwd)
   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)
}

// gcc - call is unlikely and code size would grow

void UFile::setPath(const UString& path, const UString* environment)
{
   U_TRACE(0, "UFile::setPath(%V,%p)", path.rep, environment)

   pathname = path;

   setPathRelativ(environment);
}

bool UFile::open(int flags)
{
   U_TRACE(0, "UFile::open(%d)", flags)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_EQUALS(fd, -1)
   U_INTERNAL_ASSERT_POINTER(path_relativ)
   U_INTERNAL_ASSERT_MAJOR(path_relativ_len, 0)

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

   fd = UFile::open(path_relativ, flags, PERM_FILE);

   if (fd != -1) U_RETURN(true);

   U_RETURN(false);
}

int UFile::open(const char* _pathname, int flags, mode_t mode)
{
   U_TRACE(1, "UFile::open(%S,%d,%d)", _pathname, flags, mode)

   U_INTERNAL_ASSERT_POINTER(_pathname)
   U_INTERNAL_ASSERT_MAJOR(u__strlen(_pathname, __PRETTY_FUNCTION__), 0)

   // NB: we centralize here O_BINARY...

   int _fd = U_SYSCALL(open, "%S,%d,%d", U_PATH_CONV(_pathname), flags | O_CLOEXEC | O_BINARY, mode);

   U_RETURN(_fd);
}

bool UFile::creat(int flags, mode_t mode)
{
   U_TRACE(0, "UFile::creat(%d,%d)", flags, mode)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_EQUALS(fd, -1)
   U_INTERNAL_ASSERT_POINTER(path_relativ)

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

   fd = UFile::open(path_relativ, O_CREAT | flags, mode);

   if (fd != -1) U_RETURN(true);

   U_RETURN(false);
}

bool UFile::creat(const UString& path, int flags, mode_t mode)
{
   U_TRACE(0, "UFile::creat(%V,%d,%d)", path.rep, flags, mode)

   setPath(path);

   return creat(flags, mode);
}

bool UFile::stat()
{
   U_TRACE_NO_PARAM(1, "UFile::stat()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(path_relativ)

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

   st_ino = 0;

   bool result = (U_SYSCALL(stat, "%S,%p", U_PATH_CONV(path_relativ), (struct stat*)this) == 0);

   U_RETURN(result);
}

bool UFile::chdir(const char* path, bool flag_save)
{
   U_TRACE(1, "UFile::chdir(%S,%b)", path, flag_save)

   chk_num_file_object();

   U_INTERNAL_ASSERT_POINTER(cwd_save)

   U_INTERNAL_DUMP("u_cwd(%u) = %S", u_cwd_len, u_cwd)
   U_INTERNAL_DUMP("cwd_save(%u) = %S", cwd_save_len, cwd_save)

   if (path)
      {
      if (strcmp(path, u_cwd) == 0) U_RETURN(true);

#  ifndef _MSWINDOWS_
      U_INTERNAL_ASSERT(IS_DIR_SEPARATOR(u_cwd[0]))
#  endif

      if (flag_save)
         {
         cwd_save_len = u_cwd_len;

         u__strcpy(cwd_save, u_cwd);
         }
      }
   else
      {
      U_INTERNAL_ASSERT(flag_save)
      U_INTERNAL_ASSERT_MAJOR(cwd_save_len, 0)

      path = cwd_save;
      }

   bool result = (U_SYSCALL(chdir, "%S", U_PATH_CONV(path)) != -1);

   if (result)
      {
      if (path == cwd_save) // NB: => chdir(0, true)...
         {
         U_INTERNAL_ASSERT(flag_save)

         u_cwd_len = cwd_save_len;

         u__strcpy(u_cwd, cwd_save);

         cwd_save_len = 0;
         }
      else if (IS_DIR_SEPARATOR(path[0]) == false) u_getcwd();
      else
         {
         u_cwd_len = u__strlen(path, __PRETTY_FUNCTION__);

         U_INTERNAL_ASSERT_MINOR(u_cwd_len, U_PATH_MAX)

         u__strcpy(u_cwd, path);
         }
      }

   U_INTERNAL_DUMP("u_cwd(%u) = %S", u_cwd_len, u_cwd)
   U_INTERNAL_DUMP("cwd_save(%u) = %S", cwd_save_len, cwd_save)

   U_RETURN(result);
}

uint32_t UFile::setPathFromFile(const UFile& file, char* buffer_path, const char* suffix, uint32_t len)
{
   U_TRACE(1, "UFile::setPathFromFile(%p,%p,%.*S,%u)", &file, buffer_path, len, suffix, len)

   U_INTERNAL_DUMP("file.path_relativ(%u) = %.*S", file.path_relativ_len, file.path_relativ_len, file.path_relativ)

   U_MEMCPY(buffer_path,                         file.path_relativ, file.path_relativ_len);
   U_MEMCPY(buffer_path + file.path_relativ_len,            suffix,                   len);

   uint32_t new_path_relativ_len = file.path_relativ_len + len;

   U_INTERNAL_ASSERT_MINOR(new_path_relativ_len,(int32_t)MAX_FILENAME_LEN)

   buffer_path[new_path_relativ_len] = '\0';

   U_INTERNAL_DUMP("buffer_path(%u) = %S", new_path_relativ_len, buffer_path)

   U_RETURN(new_path_relativ_len);
}

void UFile::setPath(const UFile& file, char* buffer_path, const char* suffix, uint32_t len)
{
   U_TRACE(1, "UFile::setPath(%p,%p,%.*S,%u)", &file, buffer_path, len, suffix, len)

   U_INTERNAL_DUMP("u_cwd(%u) = %S", u_cwd_len, u_cwd)
   U_INTERNAL_DUMP("cwd_save(%u) = %S", cwd_save_len, cwd_save)

   reset();

   if (IS_DIR_SEPARATOR(file.path_relativ[0]) == false && cwd_save_len) (void) U_SYSCALL(chdir, "%S", U_PATH_CONV(cwd_save)); // for IR...

   path_relativ_len = file.path_relativ_len + len;

   if (buffer_path == 0)
      {
      pathname.setBuffer(path_relativ_len);
      pathname.size_adjust(path_relativ_len);

      buffer_path = pathname.data();
      }

   path_relativ = buffer_path;

   (void) setPathFromFile(file, buffer_path, suffix, len);

   U_INTERNAL_DUMP("path_relativ = %.*S", path_relativ_len, path_relativ)
}

UString UFile::getName() const
{
   U_TRACE_NO_PARAM(0, "UFile::getName()")

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

   U_INTERNAL_ASSERT_POINTER(path_relativ)
   U_INTERNAL_ASSERT_MAJOR(path_relativ_len, 0)

   uint32_t pos;
   UString result;
   const char* base = 0;
   ptrdiff_t name_len = path_relativ_len;

   for (const char* ptr = path_relativ, *end = path_relativ + path_relativ_len; ptr < end; ++ptr) if (*ptr == '/') base = ptr + 1;

   if (base) name_len -= (base - path_relativ);

   pos = pathname.size() - name_len;

   U_INTERNAL_DUMP("name = %.*S", name_len, pathname.c_pointer(pos))

   U_ASSERT(UStringExt::endsWith(pathname, pathname.c_pointer(pos), name_len))

   result = pathname.substr(pos);

   U_RETURN_STRING(result);
}

bool UFile::isNameDosMatch(const char* mask, uint32_t mask_len) const
{
   U_TRACE(0, "UFile::isNameDosMatch(%.*S,%u)", mask_len, mask, mask_len)

   UString basename = getName();

   bool result = UServices::dosMatchWithOR(U_STRING_TO_PARAM(basename), mask, mask_len, 0);

   U_RETURN(result);
}

UString UFile::getDirName() const
{
   U_TRACE_NO_PARAM(0, "UFile::getDirName()")

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

   U_INTERNAL_ASSERT_POINTER(path_relativ)
   U_INTERNAL_ASSERT_MAJOR(path_relativ_len, 0)

   UString result = UStringExt::dirname(path_relativ, path_relativ_len);

   U_INTERNAL_ASSERT(result.isNullTerminated())

   U_RETURN_STRING(result);
}

off_t UFile::size(bool bstat)
{
   U_TRACE(0, "UFile::size(%b)", bstat)

   U_INTERNAL_ASSERT_EQUALS(st_size, 0)

   if (bstat == false) readSize();
   else
      {
      fstat();

#  ifdef _MSWINDOWS_
      st_ino = u_get_inode(fd);
#  endif

      U_INTERNAL_DUMP("st_ino = %llu", st_ino)

      if (S_ISDIR(st_mode)) U_RETURN(0);
      }

   U_RETURN(st_size);
}

// MEMORY MAPPED I/O

char* UFile::shm_open(const char* name, uint32_t length)
{
   U_TRACE(1, "UFile::shm_open(%S,%u)", name, length)

   U_INTERNAL_ASSERT_POINTER(name)
   U_INTERNAL_ASSERT_MAJOR(length, 0)

   // create/open POSIX shared memory object

   int _fd;

   // open file in read-write mode and create it if its not there, create the shared object with permissions for only the user to read and write

#ifdef HAVE_SHM_OPEN
   _fd = U_SYSCALL(shm_open, "%S,%d,%d", name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
#else
   char shm_buffer_path[MAX_FILENAME_LEN];

   (void) u__snprintf(shm_buffer_path, sizeof(shm_buffer_path), U_CONSTANT_TO_PARAM("/tmp%s"), name);

   _fd = U_SYSCALL(open, "%S,%d,%d", shm_buffer_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
#endif

   if (LIKELY(_fd > 0))
      {
      (void) U_SYSCALL(ftruncate, "%d,%u", _fd, length); // set the size of the shared memory object

      char* _ptr = (char*) U_SYSCALL(mmap, "%d,%u,%d,%d,%d,%u", 0, length, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0);

      (void) U_SYSCALL(close, "%d", _fd);

      return _ptr;
      }

   return 0;
}

// unlink POSIX shared memory object

void UFile::shm_unlink(const char* name)
{
   U_TRACE(1, "UFile::shm_unlink(%S)", name)

   U_INTERNAL_ASSERT_POINTER(name)

#ifdef HAVE_SHM_OPEN
   (void) U_SYSCALL(shm_unlink, "%S", name);
#endif
}

// On linux platforms maps (but not reserves) 256+ Megabytes of virtual address space

char* UFile::mmap_anon_huge(uint32_t* plength, int flags)
{
   U_TRACE(1, "UFile::mmap_anon_huge(%p,%d,%u)", plength, flags)

#ifdef U_LINUX
   if (nr_hugepages)
      {
      U_INTERNAL_DUMP("nr_hugepages = %ld rlimit_memfree = %u", nr_hugepages, rlimit_memfree)

      U_INTERNAL_ASSERT_EQUALS(rlimit_memfree, U_2M)

#  ifdef MAP_HUGE_1GB /* (since Linux 3.8) */
      if (*plength >= U_1G)
         {
         uint32_t length = (*plength + U_1G_MASK) & ~U_1G_MASK; // NB: munmap() length of MAP_HUGETLB memory must be hugepage aligned...

         U_INTERNAL_ASSERT_EQUALS(length & U_1G_MASK, 0)

         U_DEBUG("We are going to allocate (%u GB - %u bytes) MAP_HUGE_1GB - nfree = %u flags = %B", length / U_1G, length, nfree, flags | U_MAP_ANON_HUGE | MAP_HUGE_1GB)

         char* ptr = (char*) U_SYSCALL(mmap, "%d,%u,%d,%d,%d,%u", U_MAP_ANON_HUGE_ADDR, length, PROT_READ | PROT_WRITE, flags | U_MAP_ANON_HUGE | MAP_HUGE_1GB, -1, 0);

         if (ptr != (char*)MAP_FAILED)
            {
            *plength = length;

            return ptr;
            }
         }
#  endif
#  ifdef MAP_HUGE_2MB /* (since Linux 3.8) */
      uint32_t length = (*plength + U_2M_MASK) & ~U_2M_MASK; // NB: munmap() length of MAP_HUGETLB memory must be hugepage aligned...

      U_INTERNAL_ASSERT_EQUALS(length & U_2M_MASK, 0)

      U_DEBUG("We are going to allocate (%u MB - %u bytes) MAP_HUGE_2MB - nfree = %u flags = %B", length / (1024U*1024U), length, nfree, flags | U_MAP_ANON_HUGE | MAP_HUGE_2MB)

      char* ptr = (char*) U_SYSCALL(mmap, "%d,%u,%d,%d,%d,%u", U_MAP_ANON_HUGE_ADDR, length, PROT_READ | PROT_WRITE, flags | U_MAP_ANON_HUGE | MAP_HUGE_2MB, -1, 0);

      if (ptr != (char*)MAP_FAILED)
         {
         *plength = length;

         return ptr;
         }

      if (*plength < U_1G)
         {
         unsigned long vsz, rss;

         u_get_memusage(&vsz, &rss);

         U_WARNING("Cannot allocate %u bytes (%u MB) of memory MAP_HUGE_2MB - "
                   "address space usage: %.2f MBytes - "
                             "rss usage: %.2f MBytes",
                   *plength, *plength / (1024U*1024U), (double)vsz / (1024.0 * 1024.0),
                                                       (double)rss / (1024.0 * 1024.0));

         nr_hugepages = 0;
         }
#  endif
      }
#endif

   *plength = (*plength + U_PAGEMASK) & ~U_PAGEMASK;

   U_INTERNAL_ASSERT_EQUALS(*plength & U_PAGEMASK, 0)

   U_DEBUG("We are going to allocate (%u KB - %u bytes) - nfree = %u flags = %B", *plength / 1024U, *plength, nfree, flags)

   return (char*) U_SYSCALL(mmap, "%d,%u,%d,%d,%d,%u", 0, *plength, PROT_READ | PROT_WRITE, flags, -1, 0);
}

char* UFile::mmap(uint32_t* plength, int _fd, int prot, int flags, uint32_t offset)
{
   U_TRACE(1, "UFile::mmap(%p,%d,%d,%d,%u)", plength, _fd, prot, flags, offset)

   U_INTERNAL_ASSERT_POINTER(plength)

#ifdef U_LINUX
# ifndef HAVE_ARCH64
   U_INTERNAL_ASSERT_RANGE(1U, *plength, 3U * 1024U * 1024U * 1024U) // limit of linux system on 32bit
# endif
   if (_fd != -1)
#endif
   return (char*) U_SYSCALL(mmap, "%d,%u,%d,%d,%d,%u", 0, *plength, prot, flags, _fd, offset);

   U_INTERNAL_ASSERT_EQUALS(prot, PROT_READ | PROT_WRITE)

   if ((flags & MAP_SHARED) != 0)
      {
      U_INTERNAL_ASSERT_DIFFERS(flags & MAP_ANONYMOUS, 0)

      return mmap_anon_huge(plength, flags);
      }

   U_INTERNAL_ASSERT_DIFFERS(flags & MAP_PRIVATE, 0)

   char* _ptr;
   bool _abort = false;

   if (*plength >= rlimit_memalloc) // NB: we try to avoid strong swap pressure...
      {
#ifndef U_SERVER_CAPTIVE_PORTAL
try_from_file_system:
#endif
      *plength = (*plength + U_PAGEMASK) & ~U_PAGEMASK;

      U_INTERNAL_ASSERT_EQUALS(*plength & U_PAGEMASK, 0)

      int fd = mkTemp();

      if (fd != -1)
         {
         U_DEBUG("We are going to allocate from file system (%u KB - %u bytes)", *plength / 1024, *plength)

         _ptr = (fallocate(fd, *plength)
                     ? (char*)U_SYSCALL(mmap, "%d,%u,%d,%d,%d,%u", 0, *plength, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_NORESERVE, fd, 0)
                     : (char*)MAP_FAILED);

         close(fd);

         if (_ptr != (char*)MAP_FAILED) return _ptr;
         }

      if (_abort)
         {
         unsigned long vsz, rss;

         u_get_memusage(&vsz, &rss);

         U_ERROR("Cannot allocate %u bytes (%u KB) of memory - "
                 "address space usage: %.2f MBytes - "
                           "rss usage: %.2f MBytes",
                  *plength, *plength / 1024, (double)vsz / (1024.0 * 1024.0),
                                             (double)rss / (1024.0 * 1024.0));
         }
      }

#ifdef U_SERVER_CAPTIVE_PORTAL
   _ptr = (char*) U_SYSCALL(malloc, "%u", *plength);

   return _ptr;
#else
   U_INTERNAL_DUMP("plength = %u nfree = %u pfree = %p", *plength, nfree, pfree)

   if (pfree == 0)
      {
#  ifdef DEBUG
      unsigned long vsz, rss;

      u_get_memusage(&vsz, &rss);

      U_DEBUG("We are going to allocate %u MB - "
                 "address space usage: %.2f MBytes - "
                           "rss usage: %.2f MBytes",
                        rlimit_memalloc / (1024 * 1024),
                            (double)vsz / (1024.0 * 1024.0),
                            (double)rss / (1024.0 * 1024.0))
#  endif

      nfree = rlimit_memalloc;
#  ifdef U_MEMALLOC_WITH_HUGE_PAGE
      pfree = mmap_anon_huge(&nfree, MAP_PRIVATE | U_MAP_ANON);
#  else
      pfree = (char*) U_SYSCALL(mmap, "%d,%u,%d,%d,%d,%u", 0, nfree, PROT_READ | PROT_WRITE, MAP_PRIVATE | U_MAP_ANON, -1, 0);
#  endif

      if (pfree == (char*)MAP_FAILED)
         {
         nfree = 0;
         pfree = 0;
         }
      }

   if (*plength > nfree)
      {
      _ptr = mmap_anon_huge(plength, MAP_PRIVATE | U_MAP_ANON);

      if (_ptr == (char*)MAP_FAILED)
         {
         _abort = true;

         goto try_from_file_system;
         }

      return _ptr;
      }

# ifdef U_MEMALLOC_WITH_HUGE_PAGE
   if (nr_hugepages)
      {
#  ifdef MAP_HUGE_1GB
      if (*plength >= U_1G)
         {
         *plength = (*plength + U_1G_MASK) & ~U_1G_MASK;

         U_INTERNAL_ASSERT_EQUALS(*plength & U_1G_MASK, 0)
         }
      else
#  endif
#  ifdef MAP_HUGE_2MB
      {
      *plength = (*plength + U_2M_MASK) & ~U_2M_MASK;

      U_INTERNAL_ASSERT_EQUALS(*plength & U_2M_MASK, 0)
      }
#  endif
      }
   else
# endif
   {
   *plength = (*plength + U_PAGEMASK) & ~U_PAGEMASK;

   U_INTERNAL_ASSERT_EQUALS(*plength & U_PAGEMASK, 0)
   }

   _ptr   = pfree;
   nfree -= *plength;

   if (nfree > rlimit_memfree) pfree += *plength;
   else
      {
      pfree     = 0;
      *plength += nfree;
      }

   U_INTERNAL_DUMP("plength = %u nfree = %u pfree = %p", *plength, nfree, pfree)

   return _ptr;
#endif
}

bool UFile::memmap(int prot, UString* str, uint32_t offset, uint32_t length)
{
   U_TRACE(0, "UFile::memmap(%d,%p,%u,%u)", prot, str, offset, length)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(path_relativ)

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

   U_INTERNAL_ASSERT_DIFFERS(fd, -1)
   U_INTERNAL_ASSERT_MAJOR(st_size, 0)

#ifdef _MSWINDOWS_
   U_INTERNAL_ASSERT((off_t)length <= st_size) // NB: don't allow mappings beyond EOF since Windows can't handle that POSIX like...
#endif

   if (length == 0) length = st_size;

   uint32_t resto = 0;

   if (offset)
      {
      resto = offset % PAGESIZE;

      offset -= resto;
      length += resto;
      }

   U_INTERNAL_DUMP("resto = %u", resto)

#ifdef HAVE_ARCH64
   U_INTERNAL_ASSERT_MINOR_MSG(length, U_STRING_MAX_SIZE, "we can't manage file size bigger than 4G...") // limit of UString
#endif

   U_INTERNAL_ASSERT_EQUALS((offset % PAGESIZE), 0) // offset should be a multiple of the page size as returned by getpagesize(2)

   if (map != (char*)MAP_FAILED)
      {
      munmap(map, map_size);

      map_size = 0;
      }

   int flags = MAP_SHARED;

#if defined(U_LINUX) && defined(MAP_POPULATE) // (since Linux 2.5.46)
   if (prot == PROT_READ) flags |= MAP_POPULATE;
#endif

   map = (char*) U_SYSCALL(mmap, "%d,%u,%d,%d,%d,%u", 0, length, prot, flags, fd, offset);

   if (map != (char*)MAP_FAILED)
      {
      map_size = length;

      if (str)
         {
         str->mmap(map + resto, length - resto);

#     if defined(U_LINUX) && defined(MADV_SEQUENTIAL)
         if (prot == PROT_READ &&
             length > (32 * PAGESIZE))
            {
            (void) U_SYSCALL(madvise, "%p,%u,%d", (void*)map, length, MADV_SEQUENTIAL);
            }
#     endif
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

void UFile::munmap()
{
   U_TRACE_NO_PARAM(0, "UFile::munmap()")

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(path_relativ)

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

   U_INTERNAL_ASSERT_MAJOR(map_size,0UL)
   U_INTERNAL_ASSERT_DIFFERS(map,(char*)MAP_FAILED)

   UFile::munmap(map, map_size);

   map      = (char*)MAP_FAILED;
   map_size = 0;
}

void UFile::msync(char* ptr, char* page, int flags)
{
   U_TRACE(1, "UFile::msync(%p,%p,%d)", ptr, page, flags)

   U_INTERNAL_ASSERT(ptr >= page)

   uint32_t resto = (long)page & U_PAGEMASK;

   U_INTERNAL_DUMP("resto = %u", resto)

   char* addr      = page - resto;
   uint32_t length =  ptr - addr;

   U_INTERNAL_ASSERT_EQUALS((long)addr & U_PAGEMASK, 0) // addr should be a multiple of the page size as returned by getpagesize(2)

   (void) U_SYSCALL(msync, "%p,%u,%d", addr, length, flags);
}

UString UFile::_getContent(bool bsize, bool brdonly, bool bmap)
{
   U_TRACE(0, "UFile::_getContent(%b,%b,%b)", bsize, brdonly, bmap)

   U_INTERNAL_DUMP("fd = %d map = %p map_size = %u st_size = %I", fd, map, map_size, st_size)

   U_INTERNAL_ASSERT_DIFFERS(fd, -1)

#if defined(DEBUG) && !defined(U_LINUX) && !defined(O_TMPFILE)
   U_INTERNAL_ASSERT_POINTER(path_relativ)

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)
#endif

# ifdef U_COVERITY_FALSE_POSITIVE
   if (fd <= 0) return UString::getStringNull();
# endif

   if (bsize) readSize();

   if (st_size)
      {
      if (bmap ||
          st_size > (off_t)(4L * PAGESIZE))
         {
         int                   prot  = PROT_READ;
         if (brdonly == false) prot |= PROT_WRITE;

         UString fileContent;

         (void) memmap(prot, &fileContent, 0, st_size);

         U_RETURN_STRING(fileContent);
         }

      UString fileContent(st_size);

      char* ptr = fileContent.data();

      ssize_t value = U_SYSCALL(pread, "%d,%p,%u,%u", fd, ptr, st_size, 0);

      if (value < 0L) value = 0L;

      ptr[value] = '\0'; // NB: in this way we can use the UString method data()...

      fileContent.size_adjust(value);

      U_RETURN_STRING(fileContent);
      }

   return UString::getStringNull();
}

UString UFile::getContent(bool brdonly, bool bstat, bool bmap)
{
   U_TRACE(0, "UFile::getContent(%b,%b,%b)", brdonly, bstat, bmap)

   if (isOpen()                          == false &&
       open(brdonly ? O_RDONLY : O_RDWR) == false)
      {
      return UString::getStringNull();
      }

   UString fileContent;

   if (st_size ||
       size(bstat))
      {
      fileContent = _getContent(false, brdonly, bmap);
      }

   UFile::close();

   U_RETURN_STRING(fileContent);
}

UString UFile::contentOf(const UString& _pathname, int flags, bool bstat, const UString* environment)
{
   U_TRACE(0, "UFile::contentOf(%V,%d,%b,%p)", _pathname.rep, flags, bstat, environment)

   U_INTERNAL_ASSERT(_pathname)

   UFile file;
   UString content;

   file.reset();
   file.setPath(_pathname, environment);

   if (file.open(flags)) content = file.getContent((((flags & O_RDWR) | (flags & O_WRONLY)) == 0), bstat);

   U_RETURN_STRING(content);
}

void UFile::printf(const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0, "UFile::printf(%.*S,%u)", fmt_size, format, fmt_size)

   char buffer[8196];
   uint32_t bytes_to_write;

   va_list argp;
   va_start(argp, fmt_size);

   bytes_to_write = u__vsnprintf(buffer, sizeof(buffer)-1, format, fmt_size, argp);

   va_end(argp);

   buffer[bytes_to_write++] = '\n';

   ssize_t bytes_written = U_SYSCALL(write, "%d,%p,%u", fd, buffer, bytes_to_write);

   if (bytes_written != (ssize_t)bytes_to_write)
      {
      U_WARNING("write data of size %u is %s", bytes_to_write, (bytes_written == -1 ? "failed" : "partial"));
      }
}

bool UFile::creatForWrite(int flags, bool bmkdirs)
{
   U_TRACE(1, "UFile::creatForWrite(%d,%b)", flags, bmkdirs)

   U_INTERNAL_ASSERT_POINTER(path_relativ)

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

   bool esito = isOpen();

   if (esito == false)
      {
      esito = creat(flags, PERM_FILE);

      if (esito == false &&
          bmkdirs)
         {
         // Make any missing parent directories for each directory argument

         char* ptr = (char*) strrchr(path_relativ, '/');

         U_INTERNAL_DUMP("ptr = %S", ptr)

         if (ptr)
            {
            char buffer[U_PATH_MAX];

            uint32_t len = ptr - path_relativ;

            U_MEMCPY(buffer, path_relativ, len);

            buffer[len] = '\0';

            if (mkdirs(buffer)) esito = creat(flags, PERM_FILE);
            }
         }
      }

   U_RETURN(esito);
}

bool UFile::write(const char* data, uint32_t sz, int flags, bool bmkdirs)
{
   U_TRACE(0, "UFile::write(%.*S,%u,%d,%b)", sz, data, sz, flags, bmkdirs)

   bool esito = false;

   if (creatForWrite(flags, bmkdirs) &&
       sz)
      {
      if (sz <= PAGESIZE) esito = UFile::write(fd, data, sz);
      else
         {
         uint32_t offset = ((flags & O_APPEND) != 0 ? size() : 0);

         esito = fallocate(offset + sz);

         if (esito == false)
            {
            readSize();

            U_WARNING("No more space on disk for requested size %u - acquired only %u bytes", offset + sz, st_size);

            sz = (st_size > offset ? st_size - offset : 0);
            }

         if (sz &&
             memmap(PROT_READ | PROT_WRITE, 0, offset, st_size))
            {
            U_MEMCPY(map + offset, data, sz);

            munmap();
            }
         }
      }

   U_RETURN(esito);
}

bool UFile::write(const struct iovec* iov, int n, int flags, bool bmkdirs)
{
   U_TRACE(0, "UFile::write(%p,%d,%d,%b)", iov, n, flags, bmkdirs)

   if (creatForWrite(flags, bmkdirs) &&
       writev(iov, n))
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UFile::writeTo(const UString& path, const char* data, uint32_t sz, int flags, bool bmkdirs)
{
   U_TRACE(0, "UFile::writeTo(%V,%.*S,%u,%d,%b)", path.rep, sz, data, sz, flags, bmkdirs)

   UFile tmp(path);

   bool result = tmp.write(data, sz, flags, bmkdirs);

   if (tmp.isOpen()) tmp.close();

   U_RETURN(result);
}

bool UFile::writeTo(const UString& path, const struct iovec* iov, int n, int flags, bool bmkdirs)
{
   U_TRACE(0, "UFile::writeTo(%V,%p,%d,%d,%b)", path.rep, iov, n, flags, bmkdirs)

   UFile tmp(path);

   bool result = tmp.write(iov, n, flags, bmkdirs);

   if (tmp.isOpen()) tmp.close();

   U_RETURN(result);
}

bool UFile::writeToTmp(const char* data, uint32_t sz, int flags, const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0+256, "UFile::writeToTmp(%.*S,%u,%d,%.*S,%u)", sz, data, sz, flags, fmt_size, format, fmt_size)

   bool result = false;

   if (sz)
      {
      UString path((unsigned char*)U_CONSTANT_TO_PARAM("/tmp/"), 200U);

      va_list argp;
      va_start(argp, fmt_size);

      path.vsnprintf_add(format, fmt_size, argp);

      va_end(argp);

      result = writeTo(path, data, sz, flags, false);
      }

   U_RETURN(result);
}

bool UFile::writeToTmp(const struct iovec* iov, int n, int flags, const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0+256, "UFile::writeToTmp(%p,%d,%d,%.*S,%u)", iov, n, flags, fmt_size, format, fmt_size)

   bool result = false;

   if (n)
      {
      UString path((unsigned char*)U_CONSTANT_TO_PARAM("/tmp/"), 200U);

      va_list argp;
      va_start(argp, fmt_size);

      path.vsnprintf_add(format, fmt_size, argp);

      va_end(argp);

      result = writeTo(path, iov, n, flags, false);
      }

   U_RETURN(result);
}

bool UFile::lock(int fd, short l_type, uint32_t start, uint32_t len)
{
   U_TRACE(1, "UFile::lock(%d,%d,%u,%u)", fd, l_type, start, len)

   U_INTERNAL_ASSERT_DIFFERS(fd, -1)

   /**
    * Advisory file segment locking data type - information passed to system by user
    */

#if defined(__NetBSD__) || defined(__UNIKERNEL__) || defined(__OSX__)
   /**
    * struct flock {
    *    off_t l_start;  // starting offset
    *    off_t l_len;    // len = 0 means until end of file
    *    pid_t l_pid;    // lock owner
    *    short l_type;   // lock type: read/write, etc.
    *    short l_whence; // type of l_start
    * };
    */

   struct flock flock = { start, len, u_pid, SEEK_SET, l_type };
#else
   /**
    * struct flock {
    *    short l_type;   // Type of lock: F_RDLCK, F_WRLCK, F_UNLCK
    *    short l_whence; // How to interpret l_start: SEEK_SET, SEEK_CUR, SEEK_END
    *    off_t l_start;  // Starting offset for lock
    *    off_t l_len;    // Number of bytes to lock
    *    pid_t l_pid;    // PID of process blocking our lock (F_GETLK only)
    * };
    */

   struct flock flock = { l_type, SEEK_SET, start, len, u_pid };
#endif

   /**
    *  F_SETLK: Acquire a lock (when l_type is F_RDLCK or F_WRLCK) or release a lock (when l_type is F_UNLCK) on the
    *           bytes specified by the l_whence, l_start, and l_len fields of lock. If a conflicting lock is held by another
    *           process, this call returns -1 and sets errno to EACCES or EAGAIN.
    *
    * F_SETLKW: As for F_SETLK, but if a conflicting lock is held on the file, then wait for that lock to be released.
    *           If a signal is caught while waiting, then the call is interrupted and (after the signal handler has returned)
    *           returns immediately (with return value -1 and errno set to EINTR).
    * ---------------------------------------------------------------------------------------------------------------------
    * #ifndef F_GETLKP
    * #define F_GETLKP  F_GETLK 
    * #define F_SETLKP  F_SETLK
    * #define F_SETLKPW F_SETLKW
    * #endif
    *
    * F_GETLKP  - test whether a lock is able to be applied
    * F_SETLKP  - attempt to set a file-private lock
    * F_SETLKPW - attempt to set a file-private lock and block until able to do so
    * ---------------------------------------------------------------------------------------------------------------------
    */

   if (U_SYSCALL(fcntl, "%d,%d,%p", fd, F_SETLK, &flock) != -1) U_RETURN(true); // F_SETLKW

   U_RETURN(false);
}

bool UFile::ftruncate(uint32_t n)
{
   U_TRACE(1, "UFile::ftruncate(%u)", n)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(path_relativ)

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

   U_INTERNAL_ASSERT_DIFFERS(fd, -1)
#if defined(__CYGWIN__) || defined(_MSWINDOWS_)
   U_INTERNAL_ASSERT_EQUALS(map, (char*)MAP_FAILED)
#endif

#ifdef U_COVERITY_FALSE_POSITIVE
   if (fd <= 0) U_RETURN(false);
#endif

   if (map != (char*)MAP_FAILED &&
       map_size < (uint32_t)n)
      {
      uint32_t _map_size = n * 2;
         char* _map      = UFile::mremap(map, map_size, _map_size, MREMAP_MAYMOVE);

      if (_map == (char*)MAP_FAILED) U_RETURN(false);

      map      = _map;
      map_size = _map_size;
      }

   if (U_SYSCALL(ftruncate, "%d,%u", fd, n) == 0)
      {
      st_size = n;

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UFile::fallocate(int fd, uint32_t n)
{
   U_TRACE(1, "UFile::fallocate(%d,%u)", fd, n)

   U_INTERNAL_ASSERT_DIFFERS(fd, -1)

#ifdef U_COVERITY_FALSE_POSITIVE
   if (fd <= 0) U_RETURN(false);
#endif

#ifdef FALLOCATE_IS_SUPPORTED
   if (U_SYSCALL(fallocate, "%d,%d,%u,%u", fd, 0, 0, n) == 0) U_RETURN(true);

   U_INTERNAL_DUMP("errno = %d", errno)

   if (errno != EOPNOTSUPP) U_RETURN(false);
#endif

   if (U_SYSCALL(ftruncate, "%d,%u", fd, n) == 0) U_RETURN(true);

   U_RETURN(false);
}

UString UFile::getSysContent(const char* name)
{
   U_TRACE(0, "UFile::getSysContent(%S)", name)

   UString fileContent(U_CAPACITY);

   int fd = open(name, O_RDONLY, PERM_FILE);

   if (fd != -1)
      {
      UServices::readEOF(fd, fileContent);

      U_ASSERT_EQUALS(UServices::read(fd, fileContent), false)

      close(fd);
      }

   U_RETURN_STRING(fileContent);
}

long UFile::getSysParam(const char* name)
{
   U_TRACE(0, "UFile::getSysParam(%S)", name)

   long value = -1;
   int fd = open(name, O_RDONLY, PERM_FILE);

   if (fd != -1)
      {
      char buffer[32];

      ssize_t bytes_read = U_SYSCALL(read, "%d,%p,%u", fd, buffer, sizeof(buffer)-1);

      if (bytes_read > 0)
         {
         U_INTERNAL_ASSERT_MINOR((uint32_t)bytes_read, sizeof(buffer))

         buffer[bytes_read] = '\0';

         value = strtol(buffer, 0, 10);
         }

      close(fd);
      }

   U_RETURN(value);
}

long UFile::setSysParam(const char* name, long value, bool force)
{
   U_TRACE(0, "UFile::setSysParam(%S,%ld,%b)", name, value, force)

   long old_value = -1;
   int fd = open(name, O_RDWR, PERM_FILE);

   if (fd != -1)
      {
      char buffer[32];

      ssize_t bytes_read = U_SYSCALL(read, "%d,%p,%u", fd, buffer, sizeof(buffer)-1);

      if (bytes_read > 0)
         {
         U_INTERNAL_ASSERT_MINOR((uint32_t)bytes_read, sizeof(buffer))

         buffer[bytes_read] = '\0';

         old_value = strtol(buffer, 0, 10);

         if (force ||
             old_value < value)
            {
            char* ptr = buffer;

#        if SIZEOF_LONG == 4
            if (pwrite(fd, buffer, u_num2str32s(value, ptr) - ptr, 0) > 0) old_value = value;
#        elif !defined(U_COVERITY_FALSE_POSITIVE) // (INTEGER_OVERFLOW)
            if (pwrite(fd, buffer, u_num2str64s(value, ptr) - ptr, 0) > 0) old_value = value;
#        endif
            }
         }

      close(fd);
      }

   U_RETURN(old_value);
}

bool UFile::pread(void* buf, uint32_t count, uint32_t offset)
{
   U_TRACE(0, "UFile::pread(%p,%u,%u)", buf, count, offset)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_DIFFERS(fd, -1)

#if defined(DEBUG) && !defined(U_LINUX) && !defined(O_TMPFILE)
   U_INTERNAL_ASSERT_POINTER(path_relativ)

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)
#endif

#ifdef U_COVERITY_FALSE_POSITIVE
   if (fd <= 0) U_RETURN(false);
#endif

   if (pwrite(fd, buf, count, offset)) U_RETURN(true);

   U_RETURN(false);
}

bool UFile::pwrite(const void* _buf, uint32_t count, uint32_t offset)
{
   U_TRACE(0, "UFile::pwrite(%p,%u,%u)", _buf, count, offset)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_DIFFERS(fd, -1)

#if defined(DEBUG) && !defined(U_LINUX) && !defined(O_TMPFILE)
   U_INTERNAL_ASSERT_POINTER(path_relativ)

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)
#endif

#ifdef U_COVERITY_FALSE_POSITIVE
   if (fd <= 0) U_RETURN(false);
#endif

   if (pwrite(fd, _buf, count, offset)) U_RETURN(true);

   U_RETURN(false);
}

int UFile::setBlocking(int _fd, int flags, bool block)
{
   U_TRACE(1, "UFile::setBlocking(%d,%d,%b)", _fd, flags, block)

   U_INTERNAL_ASSERT_DIFFERS(_fd, -1)

   /**
    * ------------------------------------------------------
    * #define O_RDONLY           00
    * #define O_WRONLY           01
    * #define O_RDWR             02
    * #define O_ACCMODE        0003
    * #define O_CREAT          0100 // not fcntl
    * #define O_EXCL           0200 // not fcntl
    * #define O_NOCTTY         0400 // not fcntl
    * #define O_TRUNC         01000 // not fcntl
    * #define O_APPEND        02000
    * #define O_NONBLOCK      04000
    * #define O_SYNC         010000
    * #define O_ASYNC        020000
    * #define O_DIRECT       040000 // Direct disk access
    * #define O_DIRECTORY   0200000 // Must be a directory
    * #define O_NOFOLLOW    0400000 // Do not follow links
    * #define O_NOATIME    01000000 // Do not set atime
    * #define O_CLOEXEC    02000000 // Set close_on_exec
    * ------------------------------------------------------
    * #define O_FSYNC  O_SYNC
    * #define O_NDELAY O_NONBLOCK
    * ------------------------------------------------------
    */

   bool blocking = isBlocking(_fd, flags); // actual state is blocking...?

   // ----------------------------------------------------------------------------------------
   // flags & ~O_NONBLOCK: read() and write()     block ( normal case)
   // flags |  O_NONBLOCK: read() and write() NOT block (special case)
   // ----------------------------------------------------------------------------------------

   if (block != blocking)
      {
      flags = (blocking ? (flags |  O_NONBLOCK) 
                        : (flags & ~O_NONBLOCK));

      U_INTERNAL_DUMP("flags = %B", flags)

      (void) U_SYSCALL(fcntl, "%d,%d,%d", _fd, F_SETFL, flags);
      }

   U_RETURN(flags);
}

// resolv simbolic link and/or reference to "./" and "../"

UString UFile::getRealPath(const char* path, bool brelativ)
{
   U_TRACE(1, "UFile::getRealPath(%S,%b)", path, brelativ)

   UString buffer(U_PATH_MAX);

   char* result = U_SYSCALL(realpath, "%S,%p", path, buffer.data());

   if (result)
      {
      buffer.size_adjust();

      if (brelativ == false) U_RETURN_STRING(buffer);

      uint32_t len = buffer.size();
      void* ptr    = u_getPathRelativ(buffer.data(), &len);

      UString x((void*)ptr, len);

      U_RETURN_STRING(x);
      }

   return UString::getStringNull();
}

// create a unique temporary file

int UFile::mkTemp()
{
   U_TRACE(1, "UFile::mkTemp()")

   /**
    * O_TMPFILE is a new open(2)/openat(2) flag that makes easier the creation of secure temporary files. Files opened with the O_TMPFILE
    * flag are created but they are not visible in the filesystem. And as soon as they are closed, they get deleted - just as a file you
    * would have opened and unlinked
    *
    * http://kernelnewbies.org/Linux_3.11#head-8be09d59438b31c2a724547838f234cb33c40357
    */

#if defined(U_LINUX) && defined(O_TMPFILE)
   int fd = U_SYSCALL(open, "%S,%d,%d", u_tmpdir, O_TMPFILE | O_RDWR, PERM_FILE);
#else
   char _pathname[U_PATH_MAX];

   // The last six characters of template must be XXXXXX and these are replaced with a string that makes the filename unique

   (void) u__snprintf(_pathname, sizeof(_pathname), U_CONSTANT_TO_PARAM("%s/tmpXXXXXX"), u_tmpdir);

   mode_t old_mode = U_SYSCALL(umask, "%d", 077);  // Create file with restrictive permissions

   errno = 0; // mkstemp may not set it on error

   int fd = U_SYSCALL(mkstemp, "%S", U_PATH_CONV(_pathname));

   (void) U_SYSCALL(unlink, "%S", U_PATH_CONV(_pathname));

   (void) U_SYSCALL(umask, "%d", old_mode);
#endif

#ifdef DEBUG
   if (fd != -1)
      {
      U_ASSERT(  lock(fd) &&
               unlock(fd))
      }
#endif

   U_RETURN(fd);
}

// mkdtemp - create a unique temporary directory

bool UFile::mkdtemp(UString& _template)
{
   U_TRACE(1, "UFile::mkdtemp(%V)", _template.rep)

   errno = 0; // mkdtemp may not set it on error

   char* modified = U_SYSCALL(mkdtemp, "%S", U_PATH_CONV((char*)_template.c_str()));

   if (modified)
      {
      // NB: c_str() in replace use a new string...

      if (modified != _template.data()) (void) _template.assign(modified);

      U_RETURN(true);
      }

   U_RETURN(false);
}

// Make any missing parent directories for each directory argument

bool UFile::mkdirs(const char* path, mode_t mode)
{
   U_TRACE(1, "UFile::mkdirs(%S,%d)", path, mode)

   if (_mkdir(path, mode)) U_RETURN(true);

   U_INTERNAL_DUMP("errno = %d", errno)

   if (errno == ENOENT)
      {
      char* ptr = (char*) strrchr(path, '/');

      U_INTERNAL_DUMP("ptr = %S", ptr)

      if (ptr)
         {
         char buffer[U_PATH_MAX];

         uint32_t len = ptr - path;

         U_MEMCPY(buffer, path, len);

         buffer[len] = '\0';

         bool result =  mkdirs(buffer, mode) &&
                       _mkdir(   path, mode);

         U_RETURN(result);
         }
      }

   U_RETURN(false);
}

bool UFile::rmdir(const UString& path, bool remove_all)
{
   U_TRACE(1, "UFile::rmdir(%V,%b)", path.rep, remove_all)

   const char* ptr = path.data();

   U_INTERNAL_ASSERT(path.isNullTerminated())

   if (U_SYSCALL(rmdir, "%S", U_PATH_CONV(ptr)) == -1)
      {
      if (remove_all &&
#  ifdef _MSWINDOWS_
          (errno == ENOTEMPTY || errno == EACCES))
#  else
          (errno == ENOTEMPTY))
#  endif
         {
         bool result;
         UString file;
         UDirWalk dirwalk(path);
         UVector<UString> vec(256);

         for (uint32_t i = 0, n = dirwalk.walk(vec); i < n; ++i)
            {
            file = vec[i];

            U_ASSERT_DIFFERS(file, path)
            U_INTERNAL_ASSERT(file.isNullTerminated())

            if (UFile::_unlink(file.data()) == false &&
#        ifdef _MSWINDOWS_
                (errno == EISDIR || errno == EPERM || errno == EACCES))
#        else
                (errno == EISDIR || errno == EPERM))
#        endif
               {
               if (UFile::rmdir(file, true) == false) U_RETURN(false);
               }
            }

         result = UFile::rmdir(path, false);

         U_RETURN(result);
         }

      U_RETURN(false);
      }

   U_RETURN(true);
}

// If path includes more than one pathname component, remove it, then strip the last component
// and remove the resulting directory, etc., until all components have been removed.
// The pathname component must be empty...

bool UFile::rmdirs(const UString& path, bool remove_all)
{
   U_TRACE(1, "UFile::rmdirs(%V,%b)", path.rep, remove_all)

   bool result = rmdir(path, remove_all);

   if (result)
      {
      UString newpath = UStringExt::dirname(path);

      if (newpath != *UString::str_point) result = rmdirs(newpath.copy());
      }

   U_RETURN(result);
}

bool UFile::_rename(const char* newpath)
{
   U_TRACE(0, "UFile::_rename(%S)", newpath)

   U_CHECK_MEMORY

   U_INTERNAL_ASSERT_POINTER(path_relativ)

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

   bool result = UFile::_rename(path_relativ, newpath);

   if (result)
      {
      path_relativ     =  (char*) newpath;
      path_relativ_len = u__strlen(newpath, __PRETTY_FUNCTION__);

      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

      U_RETURN(true);
      }

   U_RETURN(false);
}

void UFile::substitute(UFile& file)
{
   U_TRACE(1, "UFile::substitute(%p)", &file)

   U_INTERNAL_ASSERT_POINTER(cwd_save)

   U_INTERNAL_DUMP("u_cwd(%u) = %S", u_cwd_len, u_cwd)
   U_INTERNAL_DUMP("cwd_save(%u) = %S", cwd_save_len, cwd_save)
   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)
   U_INTERNAL_DUMP("file.path_relativ = %.*S", file.path_relativ_len, file.path_relativ)

   U_INTERNAL_ASSERT_EQUALS(strcmp(path_relativ, file.path_relativ), 0)

   if (cwd_save_len) (void) U_SYSCALL(chdir, "%S", U_PATH_CONV(u_cwd));

   if (fd != -1)
      {
      UFile::fsync();
      UFile::close();
      }

   if (map != (char*)MAP_FAILED) munmap();

   fd       = file.fd;
   map      = file.map;
   st_size  = file.st_size;
   map_size = file.map_size;

   file.reset();

   U_INTERNAL_DUMP("fd = %d map = %p map_size = %u st_size = %I", fd, map, map_size, st_size)

   if (fd != -1) UFile::fsync();
}

UString UFile::getSuffix() const
{
   U_TRACE_NO_PARAM(0, "UFile::getSuffix()")

   U_INTERNAL_ASSERT_POINTER(path_relativ)

   U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

   UString suffix;
   const char* ptr = u_getsuffix(path_relativ, path_relativ_len);

   if (ptr)
      {
      U_INTERNAL_ASSERT_EQUALS(ptr[0], '.')
      U_INTERNAL_ASSERT_EQUALS(strchr(ptr, '/'), 0)

      (void) suffix.assign(ptr+1, (path_relativ + path_relativ_len) - (1 + ptr)); // 1 => '.'
      }

   U_RETURN_STRING(suffix);
}

// MIME TYPE

const char* UFile::getMimeType(const char* suffix, int* pmime_index)
{
   U_TRACE(0, "UFile::getMimeType(%S,%p)", suffix, pmime_index)

   const char* content_type;

   if (suffix) content_type = u_get_mimetype(suffix, pmime_index);
   else
      {
      suffix = u_getsuffix(path_relativ, path_relativ_len);

      U_INTERNAL_DUMP("suffix = %.*S", suffix ? (path_relativ_len-(suffix-path_relativ)-1) : 0, suffix+1)

      content_type = (suffix ? u_get_mimetype(suffix+1, pmime_index) : 0);
      }

#ifdef DEBUG
   if (pmime_index) U_INTERNAL_DUMP("mime_index(%d) = %C", *pmime_index, *pmime_index)
#endif

#ifdef USE_LIBMAGIC
   if (pmime_index                       &&
       map != (char*)MAP_FAILED          &&                 
       u_is_js (  *pmime_index) == false &&
       u_is_css(  *pmime_index) == false &&
       u__isdigit(*pmime_index) == false) // NB: check for dynamic page...
      {
      const char* ctype = UMagic::getType(map, map_size).data();

      if (ctype) content_type = ctype;
      }
#endif

   if (content_type == 0)
      {
#  ifdef USE_LIBMAGIC
      if (map != (char*)MAP_FAILED) content_type = UMagic::getType(map, map_size).data();

      if (content_type == 0)
#  endif
          content_type = "application/octet-stream";
      }

   U_INTERNAL_ASSERT_POINTER(content_type)

   U_RETURN(content_type);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UFile::dump(bool _reset) const
{
   *UObjectIO::os << "fd                        " << fd                  << '\n'
                  << "map                       " << (void*)map          << '\n'
                  << "st_size                   " << st_size             << '\n'
                  << "path_relativ              " << '"';

   if (path_relativ)
      {
      *UObjectIO::os << path_relativ;
      }

   *UObjectIO::os << "\"\n"
                  << "path_relativ_len          " << path_relativ_len     << '\n'
                  << "pathname (UString         " << (void*)&pathname     << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
