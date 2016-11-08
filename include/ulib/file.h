// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    file.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_FILE_H
#define ULIB_FILE_H

#include <ulib/string.h>

#ifdef _MSWINDOWS_
#define st_ino u_inode
#elif defined(HAVE_ASM_MMAN_H)
#  include <asm/mman.h>
#endif

// struct stat {
//    dev_t     st_dev;      /* device */
//    ino_t     st_ino;      /* inode */
//    mode_t    st_mode;     /* protection */
//    nlink_t   st_nlink;    /* number of hard links */
//    uid_t     st_uid;      /* user ID of owner */
//    gid_t     st_gid;      /* group ID of owner */
//    dev_t     st_rdev;     /* device type (if inode device) */
//    off_t     st_size;     /* total size, in bytes */
//    blksize_t st_blksize;  /* blocksize for filesystem I/O */
//    blkcnt_t  st_blocks;   /* number of blocks allocated */
//    time_t    st_atime;    /* time of last access */
//    time_t    st_mtime;    /* time of last modification */
//    time_t    st_ctime;    /* time of last change */
// };

// File-permission-bit symbols

#define U_PERM__rw_r__r__ 0644
#define U_PERM__r________ 0400
#define U_PERM__r__r__r__ 0444
#define U_PERM__r_xr_xr_x 0555

#define PERM_FILE       (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH) // rw_rw_r__
#define PERM_DIRECTORY   S_IRWXU                                          // rwx

// NB: the UString pathname maybe not writeable so path_relativ[path_relativ_len] maybe != '\0' (not null-terminate)...

#define U_FILE_TO_PARAM(file)  (file).getPathRelativ(),(file).getPathRelativLen()
#define U_FILE_TO_TRACE(file)  (file).getPathRelativLen(),(file).getPathRelativ()
#define U_FILE_TO_STRING(file) (file).getPath().substr((file).getPath().distance((file).getPathRelativ()),(file).getPathRelativLen())

class URDB;
class UHTTP;
class UDirWalk;
class UStringExt;
class UClientImage_Base;

template <class T> class URDBObjectHandler;

class U_EXPORT UFile : public stat {
public:

   // NB: the object can be used as (struct stat) because UMemoryError is allocate after...

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   // check for op chdir()

#ifdef DEBUG
   static int      num_file_object;
   static void inc_num_file_object(UFile* pthis);
   static void dec_num_file_object(int fd);
   static void chk_num_file_object();
#else
#  define inc_num_file_object(pthis)
#  define dec_num_file_object(fd)
#  define chk_num_file_object()
#endif

   void reset()
      {
      U_TRACE_NO_PARAM(0, "UFile::reset()")

      fd       = -1;
      map      = (char*)MAP_FAILED;
      st_size  = 0;
      map_size = 0;
      }

   UFile()
      {
      U_TRACE_REGISTER_OBJECT(0, UFile, "", 0)

      fd           = -1;
      map          = (char*)MAP_FAILED;
      path_relativ = 0;

      path_relativ_len = map_size = 0;

      inc_num_file_object(this);
      }

   UFile(const UString& path, const UString* environment = 0) : pathname(path) 
      {
      U_TRACE_REGISTER_OBJECT(0, UFile, "%V,%p", path.rep, environment)

      inc_num_file_object(this);

      setPathRelativ(environment);
      }

#ifdef U_COVERITY_FALSE_POSITIVE
   virtual
#endif
   ~UFile()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UFile)

      dec_num_file_object(fd);
      }

   // PATH

   void setRoot();
   void setPath(const UString& path, const UString* environment = 0);

   bool isRoot() const
      {
      U_TRACE_NO_PARAM(0, "UFile::isRoot()")

      U_INTERNAL_DUMP("u_cwd           = %S", u_cwd)
      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

      if (path_relativ_len == 1 &&
          path_relativ[0]  == '/')
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   bool isPath() const
      {
      U_TRACE_NO_PARAM(0, "UFile::isPath()")

      U_CHECK_MEMORY

      if (pathname) U_RETURN(true);

      U_RETURN(false);
      }

   bool isPath(const char* _pathname, uint32_t len) const
      {
      U_TRACE(0, "UFile::isPath(%.*S,%u)", len, _pathname, len)

      U_INTERNAL_DUMP("u_cwd(%u)        = %.*S", u_cwd_len, u_cwd_len, u_cwd)
      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

      if ((path_relativ_len == len) && (memcmp(path_relativ, _pathname, len) == 0)) U_RETURN(true);

      U_RETURN(false);
      }

   bool isSuffixSwap() const // NB: vi tmp...
      {
      U_TRACE_NO_PARAM(0, "UFile::isSuffixSwap()")

      U_INTERNAL_ASSERT_POINTER(path_relativ)

      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

      const char* suffix = u_getsuffix(path_relativ, path_relativ_len);

      if (suffix &&
          u_isSuffixSwap(suffix))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   // NB: the string can be not writable so path_relativ[path_relativ_len] can be != '\0'...

   UString& getPath() { return pathname; }
   UString  getName() const;
   UString  getDirName() const;
   UString  getSuffix() const;
   char*    getPathRelativ() const    { return (char*)path_relativ; }
   int32_t  getPathRelativLen() const { return        path_relativ_len; }

   bool isName(const UString& name) const { return name.equal(getName()); }

   bool isNameDosMatch(const char* mask, uint32_t mask_len) const;

   static uint32_t setPathFromFile(const UFile& file, char* buffer_path, const char* suffix, uint32_t len);

   // OPEN - CLOSE

   static int  open(const char* _pathname, int flags,                    mode_t mode);
   static int creat(const char* _pathname, int flags = O_TRUNC | O_RDWR, mode_t mode = PERM_FILE)
      {
      U_TRACE(0, "UFile::creat(%S,%d,%d)", _pathname, flags, mode)

      return open(_pathname, O_CREAT | flags, mode);
      }

   static void close(int _fd)
      {
      U_TRACE(1, "UFile::close(%d)", _fd)

      U_INTERNAL_ASSERT_DIFFERS(_fd, -1)

#  ifdef U_COVERITY_FALSE_POSITIVE
      if (_fd > 0)
#  endif
      (void) U_SYSCALL(close, "%d", _fd);
      }

   bool open(                       int flags = O_RDONLY);
   bool open(const char* _pathname, int flags = O_RDONLY)
      {
      U_TRACE(0, "UFile::open(%S,%d)", _pathname, flags)

      UString path(_pathname, u__strlen(_pathname, __PRETTY_FUNCTION__));

      setPath(path);

      return open(flags);
      }

   bool open(const UString& path, int flags = O_RDONLY)
      {
      U_TRACE(0, "UFile::open(%V,%d)", path.rep, flags)

      setPath(path);

      return open(flags);
      }

   bool creat(                     int flags = O_TRUNC | O_RDWR, mode_t mode = PERM_FILE);
   bool creat(const UString& path, int flags = O_TRUNC | O_RDWR, mode_t mode = PERM_FILE);

   void reopen(int flags)
      {
      U_TRACE(0, "UFile::reopen(%d)", flags)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(path_relativ)

      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

      close();

      U_INTERNAL_ASSERT(pathname.isNullTerminated())

      fd = open(pathname.data(), flags, PERM_FILE);
      }

   bool isOpen()
      {
      U_TRACE_NO_PARAM(0, "UFile::isOpen()")

      U_CHECK_MEMORY

      if (fd != -1) U_RETURN(true);

      U_RETURN(false);
      }

   void close()
      {
      U_TRACE_NO_PARAM(0, "UFile::close()")

      U_CHECK_MEMORY

      UFile::close(fd);

      fd = -1;
      }

   void setFd(int _fd)
      {
      U_TRACE(0, "UFile::setFd(%d)", _fd)

      U_CHECK_MEMORY

      fd = _fd;
      }

   int getFd() const
      {
      U_TRACE_NO_PARAM(0, "UFile::getFd()")

      U_CHECK_MEMORY

      U_RETURN(fd);
      }

   // ACCESS

   bool access(int mode = R_OK | X_OK)
      {
      U_TRACE(1, "UFile::access(%d)", mode)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(path_relativ)

      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

      if (U_SYSCALL(access, "%S,%d", U_PATH_CONV(path_relativ), mode) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   bool stat();

   bool slink() const
      {
      U_TRACE_NO_PARAM(0, "UFile::slink()")

      U_CHECK_MEMORY

#  ifndef _MSWINDOWS_
      if (S_ISLNK(st_mode)) U_RETURN(true);
#  endif

      U_RETURN(false);
      }

#ifndef _MSWINDOWS_
   bool lstat()
      {
      U_TRACE_NO_PARAM(1, "UFile::lstat()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(path_relativ)

      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

      if (U_SYSCALL(lstat, "%S,%p", U_PATH_CONV(path_relativ), (struct stat*)this) == 0) U_RETURN(true);

      U_RETURN(false);
      }
#endif

   void fstat()
      {
      U_TRACE_NO_PARAM(1, "UFile::fstat()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_DIFFERS(fd, -1)

#  if defined(DEBUG) && !defined(U_LINUX) && !defined(O_TMPFILE)
      U_INTERNAL_ASSERT_POINTER(path_relativ)

      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)
#  endif

#  ifdef U_COVERITY_FALSE_POSITIVE
      if (fd > 0)
#  endif
      (void) U_SYSCALL(fstat, "%d,%p", fd, (struct stat*)this);
      }

   off_t lseek(uint32_t offset, int whence)
      {
      U_TRACE(1, "UFile::lseek(%u,%d)", offset, whence)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_DIFFERS(fd, -1)

#  ifdef U_COVERITY_FALSE_POSITIVE
      if (fd <= 0) U_RETURN(0);
#  endif
      off_t result = U_SYSCALL(lseek, "%d,%u,%d", fd, offset, whence);

      U_RETURN(result);
      }

   void readSize()
      {
      U_TRACE_NO_PARAM(1, "UFile::readSize()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_DIFFERS(fd, -1)

#  if defined(DEBUG) && !defined(U_LINUX) && !defined(O_TMPFILE)
      U_INTERNAL_ASSERT_POINTER(path_relativ)

      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)
#  endif

      st_size = lseek(U_SEEK_BEGIN, SEEK_END);

      U_INTERNAL_ASSERT(st_size >= U_SEEK_BEGIN)
      }

   bool ftruncate(uint32_t n);

   off_t size(bool bstat = false);

   off_t getSize() const
      {
      U_TRACE_NO_PARAM(0, "UFile::getSize()")

      U_CHECK_MEMORY

      U_RETURN(st_size);
      }

   static off_t getSize(int _fd)
      {
      U_TRACE(1, "UFile::getSize(%d)", _fd)

      U_INTERNAL_ASSERT_DIFFERS(_fd, -1)

      off_t result = U_SYSCALL(lseek, "%d,%u,%d", _fd, U_SEEK_BEGIN, SEEK_END);

      U_RETURN(result);
      }

   static off_t getSize(const char* _pathname)
      {
      U_TRACE(1, "UFile::getSize(%S)", _pathname)

      struct stat st;

      if (U_SYSCALL(stat, "%S,%p", U_PATH_CONV(_pathname), &st) == 0) U_RETURN(st.st_size);

      U_RETURN(0);
      }

   bool empty() const
      {
      U_TRACE_NO_PARAM(0, "UFile::empty()")

      U_CHECK_MEMORY

      if (st_size == 0) U_RETURN(true);

      U_RETURN(false);
      }

   bool regular() const
      {
      U_TRACE_NO_PARAM(0, "UFile::regular()")

      U_CHECK_MEMORY

      if (S_ISREG(st_mode)) U_RETURN(true);

      U_RETURN(false);
      }

   bool dir() const
      {
      U_TRACE_NO_PARAM(0, "UFile::dir()")

      U_CHECK_MEMORY

      if (S_ISDIR(st_mode)) U_RETURN(true);

      U_RETURN(false);
      }

   bool socket() const
      {
      U_TRACE_NO_PARAM(0, "UFile::socket()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_DIFFERS(fd, -1)

      if (S_ISSOCK(st_mode)) U_RETURN(true);

      U_RETURN(false);
      }

   // Etag (HTTP/1.1)

   UString etag() const
      {
      U_TRACE_NO_PARAM(0, "UFile::etag()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(st_size, 0)
      U_INTERNAL_ASSERT_DIFFERS(st_ino, 0)

      UString _etag(100U);

      // NB: The only format constraints are that the string must be quoted...

      _etag.snprintf(U_CONSTANT_TO_PARAM("\"%x-%x-%x\""), st_ino, st_size, st_mtime);

      U_RETURN_STRING(_etag);
      }

   uint64_t inode()
      {
      U_TRACE_NO_PARAM(0, "UFile::inode()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_MAJOR(st_size, 0)
      U_INTERNAL_ASSERT_DIFFERS(st_ino, 0)

      U_RETURN(st_ino);
      }

   /**
    * Mount Point
    *
    * Either the system root directory or a directory for which the st_dev field of structure stat differs from that of its parent directory
    */

   bool isMountPoint(dev_t parent_id)
      {
      U_TRACE(0, "UFile::isMountPoint(%ld)", parent_id)

      fstat();

      U_RETURN(st_dev != parent_id);
      }

   // MODIFIER

   bool modified()
      {
      U_TRACE_NO_PARAM(0, "UFile::modified()")

      time_t mtime = st_mtime;

      fstat();

      U_INTERNAL_DUMP("mtime = %ld, st_mtime = %ld", (long)mtime, (long)st_mtime)

      if (mtime != st_mtime) U_RETURN(true);

      U_RETURN(false);
      }

   static bool isBlocking(int _fd, int& flags) // actual state is blocking...?
      {
      U_TRACE(1, "UFile::isBlocking(%d,%d)", _fd, flags)

      U_INTERNAL_ASSERT_DIFFERS(_fd, -1)

      if (flags == -1) flags = U_SYSCALL(fcntl, "%d,%d,%d", _fd, F_GETFL, 0);

      U_INTERNAL_DUMP("O_NONBLOCK = %B, flags = %B", O_NONBLOCK, flags)

      bool blocking = ((flags & O_NONBLOCK) != O_NONBLOCK);

      U_RETURN(blocking);
      }

   static int setBlocking(int fd, int flags, bool block);

   // mkdir

   static bool _mkdir(const char* path, mode_t mode = PERM_DIRECTORY)
      {
      U_TRACE(1, "UFile::_mkdir(%S,%d)", path, mode)

      if (U_SYSCALL(mkdir, "%S,%d", U_PATH_CONV(path), mode) != -1 || errno == EEXIST) U_RETURN(true);

      U_RETURN(false);
      }

   // unlink

   static bool _unlink(const char* _pathname)
      {
      U_TRACE(1, "UFile::_unlink(%S)", _pathname)

      if (U_SYSCALL(unlink, "%S", U_PATH_CONV(_pathname)) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   bool _unlink()
      {
      U_TRACE_NO_PARAM(0, "UFile::_unlink()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(path_relativ)

      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)

      if (UFile::_unlink(path_relativ)) U_RETURN(true);

      U_RETURN(false);
      }

   // rename

          bool _rename(                     const char* newpath);
   static bool _rename(const char* oldpath, const char* newpath)
      {
      U_TRACE(1, "UFile::_rename(%S,%S)", oldpath, newpath)

      if (U_SYSCALL(rename, "%S,%S", oldpath, newpath) != -1) U_RETURN(true);

      U_RETURN(false);
      }

   void fsync()
      {
      U_TRACE_NO_PARAM(1, "UFile::fsync()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_DIFFERS(fd, -1)

#  if defined(DEBUG) && !defined(U_LINUX) && !defined(O_TMPFILE)
      U_INTERNAL_ASSERT_POINTER(path_relativ)

      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)
#  endif

#  ifdef U_COVERITY_FALSE_POSITIVE
      if (fd > 0)
#  endif
      (void) U_SYSCALL(fsync, "%d", fd);
      }

   bool fallocate(uint32_t n)
      {
      U_TRACE(0, "UFile::fallocate(%u)", n)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_DIFFERS(fd, -1)

#  if defined(DEBUG) && !defined(U_LINUX) && !defined(O_TMPFILE)
      U_INTERNAL_ASSERT_POINTER(path_relativ)

      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)
#  endif

      if (fallocate(fd, n))
         {
         st_size = n;

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool fallocate(int fd, uint32_t n);
   static bool chdir(const char* path, bool flag_save = false);

   // LOCKING

   bool lock(short l_type = F_WRLCK, uint32_t start = 0, uint32_t len = 0) const
      {
      U_TRACE(0, "UFile::lock(%d,%u,%u)", l_type, start, len)

      // set the lock, waiting if necessary

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_DIFFERS(fd, -1)

#  if defined(DEBUG) && !defined(U_LINUX) && !defined(O_TMPFILE)
      U_INTERNAL_ASSERT_POINTER(path_relativ)

      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)
#  endif

      return lock(fd, l_type, start, len);
      }

   bool unlock(uint32_t start = 0, uint32_t len = 0) const
      {
      U_TRACE(0, "UFile::unlock(%u,%u)", start, len)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_DIFFERS(fd, -1)

#  if defined(DEBUG) && !defined(U_LINUX) && !defined(O_TMPFILE)
      U_INTERNAL_ASSERT_POINTER(path_relativ)

      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)
#  endif

      return lock(fd, F_UNLCK, start, len);
      }

   static bool   lock(int fd, short l_type = F_WRLCK, uint32_t start = 0, uint32_t len = 0);
   static bool unlock(int fd,                         uint32_t start = 0, uint32_t len = 0) { return lock(fd, F_UNLCK, start, len); }

   // MEMORY MAPPED I/O (Basically, you can tell the OS that some file is the backing store for a certain portion of the process memory)

   bool isMapped() const
      {
      U_TRACE_NO_PARAM(0, "UFile::isMapped()")

      U_CHECK_MEMORY

      U_RETURN(map != MAP_FAILED);
      }

   char* getMap() const { return map; }

          void munmap();
   static void munmap(void* _map, uint32_t length)
      {
      U_TRACE(1, "UFile::munmap(%p,%u)", _map, length)

      U_INTERNAL_ASSERT_DIFFERS(_map, MAP_FAILED)

      (void) U_SYSCALL(munmap, "%p,%u", _map, length);
      }

   static void msync(char* ptr, char* page, int flags = MS_ASYNC | MS_INVALIDATE); // flushes changes made to memory mapped file back to disk

   // mremap() expands (or shrinks) an existing memory mapping, potentially moving it at the same time
   // (controlled by the flags argument and the available virtual address space)

   static char* mremap(void* old_address, uint32_t old_size, uint32_t new_size, int flags = 0) // MREMAP_MAYMOVE == 1
      {
      U_TRACE(1, "UFile::mremap(%p,%u,%u,%d)", old_address, old_size, new_size, flags)

      void* result =
#  if defined(__NetBSD__) || defined(__UNIKERNEL__)
      U_SYSCALL(mremap, "%p,%u,%p,%u,%d", old_address, old_size, 0, new_size, 0);
#  else
      U_SYSCALL(mremap, "%p,%u,%u,%d",    old_address, old_size,    new_size, flags);
#  endif

      U_RETURN((char*)result);
      }

   bool memmap(int prot = PROT_READ, UString* str = 0, uint32_t offset = 0, uint32_t length = 0);

   UString  getContent(                   bool brdonly = true,  bool bstat = false, bool bmap = false);
   UString _getContent(bool bsize = true, bool brdonly = false,                     bool bmap = false);

   static UString contentOf(const UString& _pathname, int flags = O_RDONLY, bool bstat = false, const UString* environment = 0);

   static char* mmap(uint32_t* plength, int _fd = -1, int prot = PROT_READ | PROT_WRITE, int flags = MAP_SHARED | MAP_ANONYMOUS, uint32_t offset = 0);

   static char* shm_open(  const char* name, uint32_t length); // create/open POSIX shared memory object
   static void  shm_unlink(const char *name);                  //      unlink POSIX shared memory object

   // MIME TYPE

   const char* getMimeType(const char* suffix = 0, int* pmime_index = 0);

   // PREAD - PWRITE

   static bool pread(int _fd, void* buf, uint32_t count, uint32_t offset)
      {
      U_TRACE(1, "UFile::pread(%d,%p,%u,%u)", _fd, buf, count, offset)

      if (U_SYSCALL(pread, "%d,%p,%u,%u", _fd, buf, count, offset) == (ssize_t)count) U_RETURN(true);

      U_RETURN(false);
      }

   static bool pwrite(int _fd, const void* buf, uint32_t count, uint32_t offset)
      {
      U_TRACE(1, "UFile::pwrite(%d,%p,%u,%u)", _fd, buf, count, offset)

      if (U_SYSCALL(pwrite, "%d,%p,%u,%u", _fd, buf, count, offset) == (ssize_t)count) U_RETURN(true);

      U_RETURN(false);
      }

   bool pread(       void* buf, uint32_t count, uint32_t offset);
   bool pwrite(const void* buf, uint32_t count, uint32_t offset);

   // SERVICES

   static bool access(const char* path, int mode = R_OK | X_OK)
      {
      U_TRACE(1, "UFile::access(%S,%d)", path, mode)

      if (U_SYSCALL(access, "%S,%d", U_PATH_CONV(path), mode) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool stat(const char* path, struct stat* st)
      {
      U_TRACE(1, "UFile::stat(%S,%p)", path, st)

      if (U_SYSCALL(stat, "%S,%p", U_PATH_CONV(path), st) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   static bool write(int _fd, const void* buf, uint32_t count)
      {
      U_TRACE(1, "UFile::write(%d,%p,%u)", _fd, buf, count)

      if (U_SYSCALL(write, "%d,%p,%u", _fd, buf, count) == (ssize_t)count) U_RETURN(true);

      U_RETURN(false);
      }

   static int writev(int _fd, const struct iovec* iov, int n)
      {
      U_TRACE(1, "UFile::writev(%d,%p,%d)", _fd, iov, n) // problem with sanitize address

      int result = U_SYSCALL(writev, "%d,%p,%d", _fd, iov, n);

      U_RETURN(result);
      }

   int writev(const struct iovec* iov, int n) const
      {
      U_TRACE(0, "UFile::writev(%p,%d)", iov, n)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_DIFFERS(fd, -1)

#  if defined(DEBUG) && !defined(U_LINUX) && !defined(O_TMPFILE)
      U_INTERNAL_ASSERT_POINTER(path_relativ)

      U_INTERNAL_DUMP("path_relativ(%u) = %.*S", path_relativ_len, path_relativ_len, path_relativ)
#  endif

#  ifdef U_COVERITY_FALSE_POSITIVE
      if (fd <= 0) U_RETURN(0);
#  endif
      int result = UFile::writev(fd, iov, n);

      U_RETURN(result);
      }

   void printf(const char* format, uint32_t fmt_size, ...);

   bool write(const char* data,  uint32_t sz, int flags = O_RDWR | O_TRUNC, bool bmkdirs = false);
   bool write(const struct iovec* iov, int n, int flags = O_RDWR | O_TRUNC, bool bmkdirs = false);
   bool write(const UString& data,            int flags = O_RDWR | O_TRUNC, bool bmkdirs = false) { return write(U_STRING_TO_PARAM(data), flags, bmkdirs); }

   static long    setSysParam(  const char* name, long value, bool force = false);
   static long    getSysParam(  const char* name);
   static UString getSysContent(const char* name);

   static bool writeToTmp(const char* data,  uint32_t sz, int flags, const char* fmt, uint32_t fmt_size, ...);
   static bool writeToTmp(const struct iovec* iov, int n, int flags, const char* fmt, uint32_t fmt_size, ...);

   static bool writeTo(const UString& path, const char* data,  uint32_t sz, int flags = O_RDWR | O_TRUNC, bool bmkdirs = false);
   static bool writeTo(const UString& path, const struct iovec* iov, int n, int flags = O_RDWR | O_TRUNC, bool bmkdirs = false);

   static bool writeTo(const UString& path, const UString& data, int flags = O_RDWR | O_TRUNC, bool bmkdirs = false)
      { return writeTo(path, U_STRING_TO_PARAM(data), flags, bmkdirs); }

   // symlink creates a symbolic link named newpath which contains the string oldpath (make one pathname be an alias for another)

   static bool symlink(const char* oldpath, const char* newpath)
      {
      U_TRACE(1, "UFile::symlink(%S,%S)", oldpath, newpath)

#  ifndef _MSWINDOWS_
      if (U_SYSCALL(symlink, "%S,%S", oldpath, newpath) != -1) U_RETURN(true);
#  endif

      U_RETURN(false);
      }

   bool symlink(const char* newpath)
      {
      U_TRACE(0, "UFile::symlink(%S)", newpath)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(pathname.isNullTerminated())

#  ifndef _MSWINDOWS_
      if (symlink(pathname.data(), newpath)) U_RETURN(true);
#  endif

      U_RETURN(false);
      }

   // make a FIFO special file (a named pipe)

   static bool mkfifo(const char* _pathname, mode_t mode = S_IRUSR | S_IWUSR)
      {
      U_TRACE(1, "UFile::mkfifo(%S,%d)", _pathname, mode)

#  ifndef _MSWINDOWS_
      if (U_SYSCALL(mkfifo, "%S,%d", _pathname, mode) == 0) U_RETURN(true);
#  endif

      U_RETURN(false);
      }

   // Expands all symbolic links and resolves references to '/./', '/../' and extra '/' characters in the null
   // terminated string named by path and stores the canonicalized absolute pathname in the return string.
   // The resulting string will have no symbolic link, '/./' or '/../' components

   static UString getRealPath(const char* path, bool brelativ = false);

   // TEMP OP

   static int mkTemp(); // create a unique temporary file

   // --------------------------------------------------------------------------------------------------------------
   // mkdtemp - create a unique temporary directory
   // --------------------------------------------------------------------------------------------------------------
   // The mkdtemp() function generates a uniquely-named temporary directory from template. The last six characters
   // of template must be XXXXXX and these are replaced with a string that makes the directory name unique. The
   // directory is then created with permissions 0700. Since it will be modified, template must not be a string
   // constant, but should be declared as a character array
   // --------------------------------------------------------------------------------------------------------------

   static bool mkdtemp(UString& _template);

   // DIR OP

   static bool rmdir(const UString& dir, bool remove_all = false);

   // If path includes more than one pathname component, remove it, then strip the last component
   // and remove the resulting directory, etc., until all components have been removed.
   // The pathname component must be empty...

   static bool rmdirs(const UString& dir, bool remove_all = false);

   // Creates all directories in this path. This method returns true if all directories in this path are created

   static bool mkdirs(const char* path, mode_t mode = PERM_DIRECTORY);

   // MEMORY POOL

   static uint32_t getPageMask(uint32_t length) // NB: munmap() length of MAP_HUGETLB memory must be hugepage aligned...
      {
      U_TRACE(0, "UFile::getPageMask(%u)", length)

#  if defined(U_LINUX) && defined(U_MEMALLOC_WITH_HUGE_PAGE) && (defined(MAP_HUGE_1GB) || defined(MAP_HUGE_2MB)) // (since Linux 3.8)
      if (nr_hugepages)
         {
         U_INTERNAL_DUMP("nr_hugepages = %ld rlimit_memfree = %u", nr_hugepages, rlimit_memfree)

         U_INTERNAL_ASSERT_EQUALS(rlimit_memfree, U_2M)

#     ifdef MAP_HUGE_1GB
         if (length >= U_1G) U_RETURN(U_1G_MASK);
#     endif
#     ifdef MAP_HUGE_2MB
         U_RETURN(U_2M_MASK);
#     endif
         }
# endif

      U_RETURN(U_PAGEMASK);
      }

   static bool checkPageAlignment(uint32_t length) // NB: munmap() length of MAP_HUGETLB memory must be hugepage aligned...
      {
      U_TRACE(0, "UFile::checkPageAlignment(%u)", length)

#  if defined(U_LINUX) && defined(U_MEMALLOC_WITH_HUGE_PAGE) && (defined(MAP_HUGE_1GB) || defined(MAP_HUGE_2MB)) // (since Linux 3.8)
      if (nr_hugepages)
         {
         U_INTERNAL_DUMP("nr_hugepages = %ld rlimit_memfree = %u", nr_hugepages, rlimit_memfree)

         U_INTERNAL_ASSERT_EQUALS(rlimit_memfree, U_2M)

#     ifdef MAP_HUGE_1GB
         if (length >= U_1G)
            {
            if ((length & U_1G_MASK) == 0) U_RETURN(true);

            U_RETURN(false);
            }
#     endif
#     ifdef MAP_HUGE_2MB
         if ((length & U_2M_MASK) == 0) U_RETURN(true);

         U_RETURN(false);
#     endif
         }
# endif
      if ((length & U_PAGEMASK) == 0) U_RETURN(true);

      U_RETURN(false);
      }

   static uint32_t getSizeAligned(uint32_t length) // NB: munmap() length of MAP_HUGETLB memory must be hugepage aligned...
      {
      U_TRACE(0, "UFile::getSizeAligned(%u)", length)

      uint32_t pmask = getPageMask(length),
               sz = (length + pmask) & ~pmask;

      U_ASSERT(checkPageAlignment(sz))

      U_RETURN(sz);
      }

   static bool isAllocableFromPool(uint32_t sz)
      {
      U_TRACE(0, "UFile::isAllocableFromPool(%u)", sz)

#  ifdef ENABLE_MEMPOOL
      U_INTERNAL_DUMP("nfree = %u pfree = %p", nfree, pfree)

      if (sz > U_CAPACITY &&
          nfree > (getSizeAligned(sz) + rlimit_memfree))
         {
         U_RETURN(true);
         }
#  endif

      U_RETURN(false);
      }

   static bool isLastAllocation(void* ptr, size_t sz)
      {
      U_TRACE(0, "UFile::isLastAllocation(%p,%lu)", ptr, sz)

#  ifdef ENABLE_MEMPOOL
      U_INTERNAL_ASSERT(sz >= U_MAX_SIZE_PREALLOCATE)

      U_INTERNAL_DUMP("nfree = %u pfree = %p", nfree, pfree)

      if (pfree)
         {
         U_INTERNAL_ASSERT_MAJOR(nfree, rlimit_memfree)

         if ((pfree - sz) == ptr)
            {
            U_ASSERT(UFile::checkPageAlignment(sz)) // NB: munmap() length of MAP_HUGETLB memory must be hugepage aligned...

            U_RETURN(true);
            }
         }
#  endif

      U_RETURN(false);
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   uint32_t path_relativ_len, map_size; // size to mmap(), may be larger than the size of the file...
   UString pathname;
   char* map;
   const char* path_relativ;            // the string can be not writeable...
   int fd;

   static char*    cwd_save;
   static uint32_t cwd_save_len;

   static char* pfree;
   static long nr_hugepages;
   static uint32_t nfree, rlimit_memfree, rlimit_memalloc;

   void substitute(UFile& file);
   bool creatForWrite(int flags, bool bmkdirs);
   void setPathRelativ(const UString* environment = 0);
   void setPath(const UFile& file, char* buffer_path, const char* suffix, uint32_t len);

   static void ftw_tree_up();
   static void ftw_tree_push();
   static void ftw_vector_push();

   static char* mmap_anon_huge(uint32_t* plength, int flags);

private:
#ifdef _MSWINDOWS_
   uint64_t u_inode;
#endif

   U_DISALLOW_ASSIGN(UFile)

   friend class ULib;
   friend class URDB;
   friend class UHTTP;
   friend class UString;
   friend class UDirWalk;
   friend class UStringRep;
   friend class UStringExt;
   friend class UMemoryPool;
   friend class UClientImage_Base;

   template <class T> friend class URDBObjectHandler;
};

#endif
