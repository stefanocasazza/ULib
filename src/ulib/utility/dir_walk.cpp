// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    dir_walk.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/file.h>
#include <ulib/container/tree.h>
#include <ulib/utility/dir_walk.h>
#include <ulib/utility/services.h>
#include <ulib/container/hash_map.h>

/**
 * FTW - walks through the directory tree starting from the indicated directory.
 *       For each found entry in the tree, it calls foundFile()
 *
 * struct dirent {
 *    ino_t          d_ino;       // inode number
 *    off_t          d_off;       // offset to the next dirent
 *    unsigned short d_reclen;    // length of this record
 *    unsigned char  d_type;      // type of file; not supported by all file system types
 *    char           d_name[256]; // filename
 * };
 */

#ifndef DT_DIR
#define DT_DIR 4
#endif
#ifndef DT_REG
#define DT_REG 8
#endif
#ifndef DT_LNK
#define DT_LNK 10
#endif
#ifndef DT_UNKNOWN
#define DT_UNKNOWN 0
#endif

#ifdef DIRENT_HAVE_D_TYPE
#  define U_DT_TYPE dp->d_type
#else
#  define U_DT_TYPE DT_UNKNOWN
#endif

vPF               UDirWalk::call_if_up;
vPF               UDirWalk::call_internal;
int               UDirWalk::filter_flags;
bool              UDirWalk::brecurse;     // recurse subdirectories ?
bool              UDirWalk::bfollowlinks; // recurse subdirectories when are symbolic link ?
bool              UDirWalk::tree_root;
bool              UDirWalk::call_if_directory;
uint32_t          UDirWalk::max;
uint32_t          UDirWalk::filter_len;
qcompare          UDirWalk::sort_by;
UString*          UDirWalk::suffix_file_type;
UDirWalk*         UDirWalk::pthis;
const char*       UDirWalk::filter;
UTree<UString>*   UDirWalk::ptree;
UVector<UString>* UDirWalk::pvector;
UHashMap<UFile*>* UDirWalk::cache_file_for_compare;

void UDirWalk::ctor(const UString* dir, const char* _filter, uint32_t _filter_len, int _filter_flags)
{
   U_TRACE(0, "UDirWalk::ctor(%p,%.*S,%u,%d)", dir, _filter_len, _filter, _filter_len, _filter_flags)

   max               = 128U * 1024U;
   depth             = -1; // starting recursion depth
   pthis             = this;
   sort_by           = 0;
   call_if_up        = 0;
   call_internal     = 0;
   suffix_file_type  = 0;
   call_if_directory =
   brecurse          =
   is_directory      = false;

   if (dir) (void) setDirectory(*dir, _filter, _filter_len, _filter_flags);
   else
      {
      pathname[0]             = '.';
      pathname[(pathlen = 1)] = '\0';

      setFilter(_filter, _filter_len, _filter_flags);
      }
}

bool UDirWalk::setDirectory(const UString& dir, const char* _filter, uint32_t _filter_len, int _filter_flags)
{
   U_TRACE(0, "UDirWalk::setDirectory(%V,%.*S,%u,%d)", dir.rep, _filter_len, _filter, _filter_len, _filter_flags)

   pthis->pathlen = dir.size();

   const char* pdir = u_getPathRelativ(dir.data(), &(pthis->pathlen));

   U_INTERNAL_ASSERT_MAJOR(pthis->pathlen, 0)

   U_MEMCPY(pthis->pathname, pdir, pthis->pathlen);

   pthis->pathname[pthis->pathlen] = '\0';

   if (UFile::access(pthis->pathname) == false)
      {
      pthis->pathlen = 0;

      U_RETURN(false);
      }

   setFilter(_filter, _filter_len, _filter_flags);

   U_RETURN(true);
}

U_NO_EXPORT void UDirWalk::prepareForCallingRecurse(char* d_name, uint32_t d_namlen, unsigned char d_type)
{
   U_TRACE(0, "UDirWalk::prepareForCallingRecurse(%.*S,%u,%d)", d_namlen, d_name, d_namlen, d_type)

   U_INTERNAL_ASSERT_EQUALS(pthis, this)

   if (d_type == DT_REG ||
       d_type == DT_DIR ||
       d_type == DT_LNK ||
       d_type == DT_UNKNOWN)
      {
      U_MEMCPY(pathname + pathlen, d_name, d_namlen);

      pathlen += d_namlen;

      U_INTERNAL_ASSERT_MINOR(pathlen, sizeof(pathname))

      pathname[pathlen] = '\0';

      is_directory = (d_type == DT_DIR);

      if (brecurse          &&
          (is_directory     ||
           d_type == DT_LNK ||
           d_type == DT_UNKNOWN))
         {
         recurse();
         }
      else
         {
         foundFile();
         }

      pathlen -= d_namlen;
      }
}

U_NO_EXPORT bool UDirWalk::isFile()
{
   U_TRACE_NO_PARAM(0, "UDirWalk::isFile()")

   U_INTERNAL_ASSERT_POINTER(pthis)

   const char* ptr = u_getsuffix(pathname+1, pathlen-1);

   if (ptr++)
      {
      if (u_get_mimetype(ptr, 0)) U_RETURN(true);

      if (suffix_file_type)
         {
         uint32_t len = pathlen - (ptr - pathname);
          
         U_INTERNAL_DUMP("suffix(%u) = %.*S", len, len, ptr)

         if (UServices::dosMatchWithOR(ptr, len, U_STRING_TO_PARAM(*suffix_file_type))) U_RETURN(true);
         }
      }

   U_RETURN(false);
}

void UDirWalk::recurse()
{
   U_TRACE_NO_PARAM(1+256, "UDirWalk::recurse()")

   U_INTERNAL_ASSERT_EQUALS(pthis, this)

   DIR* dirp = 0;

   ++depth; // if this has been called, then we're one level lower

   U_INTERNAL_DUMP("depth = %d pathlen = %u pathname(%u) = %S", depth, pathlen, u__strlen(pathname, __PRETTY_FUNCTION__), pathname)

   U_INTERNAL_ASSERT_EQUALS(u__strlen(pathname, __PRETTY_FUNCTION__), pathlen)

   if (depth == 0) dirp = (DIR*) U_SYSCALL(opendir, "%S", "."); // NB: if pathname it is not '.' we have already make chdir()... 
   else
      {
      if (isFile()) // NB: we check if this item is a file so we don't need to try opendir()...
         {
#ifndef _MSWINDOWS_
found_file:
#endif
         is_directory = false;

         foundFile();

         goto end;
         }

      dirp = (DIR*) U_SYSCALL(opendir, "%S", U_PATH_CONV(pathname));
      }

   is_directory = (dirp != 0);

   if (is_directory == false ||
       call_if_directory)
      {
      foundFile();
      }

   if (is_directory)
      {
#  ifndef _MSWINDOWS_ 
      if (bfollowlinks == false)
         {
         struct stat st;

         if (U_SYSCALL(lstat, "%S,%p", U_PATH_CONV(pathname), &st) == 0 &&
             S_ISLNK(st.st_mode))
            {
            if (call_if_directory) // NB: to avoid duplication...
               {
               is_directory = false;

               goto end;
               }

            goto found_file;
            }
         }
#  endif

      dir_s qdir;
      dirent_s* ds;
      struct dirent* dp;

      qdir.num            = 0;
      pathname[pathlen++] = '/';

      if (sort_by)
         {
         U_INTERNAL_ASSERT_MAJOR(max, 0)

         qdir.max    = max;
         qdir.dp     = (dirent_s*) UMemoryPool::_malloc(&qdir.max, sizeof(dirent_s));

         qdir.szfree = qdir.max * 128;
         qdir.free   = (char*) UMemoryPool::_malloc(&qdir.szfree);
         qdir.nfree  =                               qdir.szfree;
         qdir.pfree  = 0;
         }

      // -----------------------------------------------
      // NB: these are NOT always the first two entry !!
      // -----------------------------------------------
      // (void) readdir(dirp); // skip '.'
      // (void) readdir(dirp); // skip '..'
      // -----------------------------------------------

      while ((dp = readdir(dirp))) // NB: we don't use the macro U_SYSCALL to avoid warning on stderr...
         {
         uint32_t d_namlen = NAMLEN(dp);

         U_INTERNAL_DUMP("d_namlen = %u d_name = %.*s filter(%u) = %.*S filter_flags = %d sort_by = %p",
                          d_namlen,     d_namlen, dp->d_name, filter_len, filter_len, filter, filter_flags, sort_by)

         if (U_ISDOTS(dp->d_name)) continue;

         if (filter_len == 0 ||
             UServices::dosMatchWithOR(dp->d_name, d_namlen, filter, filter_len, filter_flags))
            {
            if (sort_by == 0) prepareForCallingRecurse(dp->d_name, d_namlen, U_DT_TYPE);
            else
               {
               // NB: check if we must do reallocation...

               if (qdir.num >= qdir.max)
                  {
                  uint32_t  old_max   = qdir.max;
                  dirent_s* old_block = qdir.dp;

                  qdir.max <<= 1;

                  U_INTERNAL_DUMP("Reallocating dirent (%u => %u)", old_max, qdir.max)

                  qdir.dp = (dirent_s*) UMemoryPool::_malloc(&qdir.max, sizeof(dirent_s));

                  U_MEMCPY(qdir.dp, old_block, old_max * sizeof(dirent_s));

                  UMemoryPool::_free(old_block, old_max, sizeof(dirent_s));
                  }

               if (d_namlen > qdir.nfree)
                  {
                  char*    old_block = qdir.free;
                  uint32_t old_free  = qdir.szfree;

                  qdir.szfree <<= 1;

                  qdir.free  = (char*) UMemoryPool::_malloc(&qdir.szfree);
                  qdir.nfree = (qdir.szfree - qdir.pfree);

                  U_INTERNAL_DUMP("Reallocating dirname (%u => %u) nfree = %u", old_free, qdir.szfree, qdir.nfree)

                  U_MEMCPY(qdir.free, old_block, qdir.pfree);

                  UMemoryPool::_free(old_block, old_free);
                  }

               ds         = qdir.dp + qdir.num++;
               ds->d_ino  = dp->d_ino;
               ds->d_type = U_DT_TYPE;

               U_MEMCPY(qdir.free + (ds->d_name = qdir.pfree), dp->d_name, (ds->d_namlen = d_namlen));

               qdir.pfree += d_namlen;
               qdir.nfree -= d_namlen;

               U_INTERNAL_DUMP("readdir: %lu %.*s %d", ds->d_ino, ds->d_namlen, qdir.free + ds->d_name, ds->d_type);
               }
            }
         }

      U_INTERNAL_DUMP("qdir.num = %u", qdir.num)

      if (qdir.num)
         {
         U_SYSCALL_VOID(qsort, "%p,%u,%d,%p", qdir.dp, qdir.num, sizeof(dirent_s), sort_by);

         for (uint32_t i = 0; i < qdir.num; ++i)
            {
            ds = qdir.dp + i;

            prepareForCallingRecurse(qdir.free + ds->d_name, ds->d_namlen, ds->d_type);
            }
         }

      if (sort_by)
         {
         UMemoryPool::_free(qdir.free, qdir.szfree);
         UMemoryPool::_free(qdir.dp,   qdir.max, sizeof(dirent_s));
         }

      --pathlen;

      if (call_if_up) call_if_up(); // for UTree<UString>::load() ...
      }

end:
   --depth; // we're returning to the parent's depth now

   U_INTERNAL_DUMP("depth = %d", depth)

   if (dirp) (void) U_SYSCALL(closedir, "%p", dirp);
}

void UDirWalk::walk()
{
   U_TRACE_NO_PARAM(0, "UDirWalk::walk()")

   U_INTERNAL_DUMP("pathname = %S", pathname)
   U_INTERNAL_DUMP("u_cwd(%u) = %.*S", u_cwd_len, u_cwd_len, u_cwd)

   U_INTERNAL_ASSERT_EQUALS(pthis, this)

   if (pathlen     == 1 &&
       pathname[0] == '.')
      {
      recurse();

      pathname[0]             = '.';
      pathname[(pathlen = 1)] = '\0';
      }
   else
      {
      char cwd_save[U_PATH_MAX];

      // NB: we need our own backup of current directory (see IR)...

      U_MEMCPY(cwd_save, u_cwd, u_cwd_len);

      cwd_save[u_cwd_len] = '\0';

      if (UFile::chdir(pathname, false))
         {
         char pathname_save[U_PATH_MAX];
         uint32_t pathlen_save = pathlen;

         U_MEMCPY(pathname_save, pathname, pathlen);

         recurse();

         (void) UFile::chdir(cwd_save, false);

         U_MEMCPY(pathname, pathname_save, (pathlen = pathlen_save));

         pathname[pathlen] = '\0';
         }
      }

   U_INTERNAL_DUMP("pathname = %S", pathname)
   U_INTERNAL_DUMP("u_cwd(%u) = %.*S", u_cwd_len, u_cwd_len, u_cwd)
}

U_NO_EXPORT void UDirWalk::vectorPush()
{
   U_TRACE_NO_PARAM(0, "UDirWalk::vectorPush()")

   U_INTERNAL_ASSERT_POINTER(pthis)
   U_INTERNAL_ASSERT_POINTER(pvector)

   U_INTERNAL_DUMP("depth = %d pathlen = %u pathname(%u) = %S",
                     pthis->depth, pthis->pathlen, u__strlen(pthis->pathname, __PRETTY_FUNCTION__), pthis->pathname)

   uint32_t len    = pthis->pathlen;
   const char* ptr = pthis->pathname;

   if (ptr[0] == '.')
      {
      if (len == 1) return;

      if (IS_DIR_SEPARATOR(ptr[1]))
         {
         U_INTERNAL_ASSERT_MAJOR(len, 2)

         ptr += 2;
         len -= 2;
         }
      }

   UString str((void*)ptr, len);

   pvector->push(str);
}

U_NO_EXPORT int UDirWalk::cmp_modify(const void* a, const void* b)
{
   U_TRACE(0, "UDirWalk::cmp_modify(%p,%p)", a, b)

   U_INTERNAL_ASSERT_POINTER(cache_file_for_compare)

   UFile* ra = (*cache_file_for_compare)[*(UStringRep**)a];

   if (ra == 0)
      {
      UString key(*(UStringRep**)a);

      U_NEW(UFile, ra, UFile(key));

      (void) ra->stat();

      cache_file_for_compare->insertAfterFind(key, ra);
      }

   UFile* rb = (*cache_file_for_compare)[*(UStringRep**)b];

   if (rb == 0)
      {
      UString key(*(UStringRep**)b);

      U_NEW(UFile, rb, UFile(key));

      (void) rb->stat();

      cache_file_for_compare->insertAfterFind(key, rb);
      }

   U_INTERNAL_DUMP("ra = %.*S", U_FILE_TO_TRACE(*ra))
   U_INTERNAL_DUMP("rb = %.*S", U_FILE_TO_TRACE(*rb))

   int diff = (ra->st_mtime - ra->st_mtime);

   U_RETURN(diff);
}

uint32_t UDirWalk::walk(UVector<UString>& vec, qcompare compare_obj)
{
   U_TRACE(0, "UDirWalk::walk(%p,%p)", &vec, compare_obj)

   U_INTERNAL_ASSERT_EQUALS(pthis, this)

   pvector = &vec;

   call_internal = vectorPush;

   walk();

   uint32_t n = vec.size();

   if (compare_obj &&
       n > 1)
      {
      if (compare_obj == U_ALPHABETIC_SORT) vec.sort();
      else
         {
         if (cache_file_for_compare == 0) U_NEW(UHashMap<UFile*>, cache_file_for_compare, UHashMap<UFile*>);

         uint32_t sz       = n + (15 * (n / 100)) + 32,
                  capacity = cache_file_for_compare->capacity();

         if (capacity < sz)
            {
            if (capacity) cache_file_for_compare->deallocate();

            cache_file_for_compare->allocate(sz);
            }

         vec.UVector<void*>::sort(compare_obj);

         cache_file_for_compare->clear();
         }
      }

   U_RETURN(n);
}

U_NO_EXPORT void UDirWalk::treePush()
{
   U_TRACE_NO_PARAM(0, "UDirWalk::treePush()")

   U_INTERNAL_DUMP("is_directory = %b", pthis->is_directory)

   U_INTERNAL_ASSERT_POINTER(ptree)

   if (tree_root) tree_root = false;
   else
      {
      UString str((void*)pthis->pathname, pthis->pathlen);

      UTree<UString>* _ptree = ptree->push(str);

      if (pthis->is_directory) ptree = _ptree;
      }
}

U_NO_EXPORT void UDirWalk::treeUp()
{
   U_TRACE_NO_PARAM(0, "UDirWalk::treeUp()")

   U_INTERNAL_ASSERT_POINTER(ptree)

   ptree = ptree->parent();
}

uint32_t UDirWalk::walk(UTree<UString>& tree)
{
   U_TRACE(0, "UDirWalk::walk(%p)", &tree)

   U_INTERNAL_ASSERT_EQUALS(pthis, this)

   ptree     = &tree;
   tree_root = true;

   setRecurseSubDirs(true, true);

   call_if_up    = treeUp;
   call_internal = treePush;

   UString str((void*)pathname, pathlen);

   tree.setRoot(str);

   walk();

   uint32_t result = tree.size();

   U_RETURN(result);
}

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UDirWalk::dump(bool reset) const
{
   *UObjectIO::os << "max                       " << max                     << '\n'
                  << "pathlen                   " << pathlen                 << '\n'
                  << "sort_by                   " << (void*)sort_by          << '\n'
                  << "brecurse                  " << brecurse                << '\n'
                  << "call_if_up                " << (void*)call_if_up       << '\n'
                  << "filter_len                " << filter_len              << '\n'
                  << "bfollowlinks              " << bfollowlinks            << '\n'
                  << "filter_flags              " << filter_flags            << '\n'
                  << "is_directory              " << is_directory            << '\n'
                  << "call_if_directory         " << call_if_directory       << '\n'
                  << "suffix_file_type (UString " << (void*)suffix_file_type << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
