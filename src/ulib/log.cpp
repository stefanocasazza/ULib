// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    log.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/log.h>
#include <ulib/date.h>
#include <ulib/utility/lock.h>
#include <ulib/internal/chttp.h>

#ifndef _MSWINDOWS_
#  ifdef __clang__
#  undef  NULL
#  define NULL 0
#  endif
#  define SYSLOG_NAMES
#  include <syslog.h>
#  include <sys/utsname.h>
#endif

#ifdef USE_LIBZ // check for crc32
#  include <ulib/base/coder/gzio.h>
#  include <ulib/utility/interrupt.h>
#  include <ulib/utility/string_ext.h>
#endif

#define U_MARK_END       "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" // 24
#define U_FMT_START_STOP "*** %s %N (%ubit, pid %P) [%U@%H] ***"

long              ULog::tv_sec_old_1;
long              ULog::tv_sec_old_2;
long              ULog::tv_sec_old_3;
ULog*             ULog::pthis;
uint32_t          ULog::log_data_sz;
uint32_t          ULog::prefix_len;
const char*       ULog::prefix;
struct iovec      ULog::iov_vec[5];
ULog::log_date    ULog::date;
ULog::log_date*   ULog::ptr_shared_date;
#if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
pthread_rwlock_t* ULog::prwlock;
#endif

ULog::ULog(const UString& path, uint32_t _size, const char* dir_log_gz) : UFile(path, 0)
{
   U_TRACE_REGISTER_OBJECT(0, ULog, "%V,%u,%S", path.rep, _size, dir_log_gz)

   lock         = 0;
   ptr_log_data = 0;
   log_file_sz  =
   log_gzip_sz  = 0;

   U_Log_start_stop_msg(this) = false;

#ifdef USE_LIBZ
     buf_path_compress = 0;
   index_path_compress = 0;
#endif

   if (UFile::getPath().equal(U_CONSTANT_TO_PARAM("syslog")))
      {
      U_Log_syslog(this) = true;

#  ifndef _MSWINDOWS_
      openlog(u_progname, LOG_PID, LOG_LOCAL0);
#  endif

      return;
      }

   if (UFile::creat(O_RDWR | O_APPEND, 0664) == false)
      {
#  ifndef U_COVERITY_FALSE_POSITIVE
      U_ERROR("Cannot creat log file %.*S", U_FILE_TO_TRACE(*this));
#  endif

      return;
      }

   /**
    * typedef struct log_data {
    *  uint32_t file_ptr;
    *  uint32_t file_page;
    *  uint32_t gzip_len;
    *  sem_t lock_shared;
    *  char spinlock_shared[1];
    *  // --------------> maybe unnamed array of char for gzip compression...
    * } log_data;
    */

   ptr_log_data = U_MALLOC_TYPE(log_data);

   ptr_log_data->file_ptr = 0;

   if (_size)
      {
      uint32_t file_size = UFile::size();

      bool bsize = (file_size != _size);

      if ((bsize && UFile::ftruncate(_size) == false) ||
          UFile::memmap(PROT_READ | PROT_WRITE) == false)
         {
         U_ERROR("Cannot init log file %.*S", U_FILE_TO_TRACE(*this));

         return;
         }

      if (bsize) ptr_log_data->file_ptr = file_size; // append mode
      else
         {
         // NB: we can have a previous crash without resizing the file or we are an other process (apache like log)...

         char* ptr = (char*) u_find(UFile::map, file_size, U_CONSTANT_TO_PARAM(U_MARK_END));

         if (ptr)
            {
            ptr_log_data->file_ptr = ptr - UFile::map;

            // NB: we can be an other process that manage this file (apache like log)...

            u_put_unalignedp64(ptr,    U_MULTICHAR_CONSTANT64('\n','\n','\n','\n','\n','\n','\n','\n'));
            u_put_unalignedp64(ptr+8,  U_MULTICHAR_CONSTANT64('\n','\n','\n','\n','\n','\n','\n','\n'));
            u_put_unalignedp64(ptr+16, U_MULTICHAR_CONSTANT64('\n','\n','\n','\n','\n','\n','\n','\n'));

            UFile::msync(ptr + U_CONSTANT_SIZE(U_MARK_END), UFile::map, MS_SYNC);
            }

         U_INTERNAL_ASSERT_MINOR(ptr_log_data->file_ptr, UFile::st_size)
         }

      log_file_sz = UFile::st_size;
      }

   U_INTERNAL_ASSERT(ptr_log_data->file_ptr <= UFile::st_size)

   U_NEW(ULock, lock, ULock);

   U_Log_syslog(this)      = false;
   ptr_log_data->gzip_len  = 0;
   ptr_log_data->file_page = ptr_log_data->file_ptr;

#ifdef USE_LIBZ
   char suffix[32];
   uint32_t len_suffix = u__snprintf(suffix, sizeof(suffix), U_CONSTANT_TO_PARAM(".%4D.gz"));

   U_NEW(UString, buf_path_compress, UString(MAX_FILENAME_LEN));

   char* ptr = buf_path_compress->data();

   if (dir_log_gz == 0)
      {
#  ifndef U_COVERITY_FALSE_POSITIVE // Uninitialized pointer read (UNINIT)
      (void) UFile::setPathFromFile(*this, ptr, suffix, len_suffix);
#  endif

      buf_path_compress->size_adjust();

      index_path_compress = (buf_path_compress->size() - len_suffix + 1);
      }
   else
      {
      UString name = UFile::getName();
      uint32_t len = u__strlen(dir_log_gz, __PRETTY_FUNCTION__), sz = name.size();

      U_MEMCPY(ptr, dir_log_gz, len);

       ptr  += len;
      *ptr++ = '/';

      buf_path_compress->size_adjust(len + 1 + sz + len_suffix);

      U_MEMCPY(ptr, name.data(), sz);
               ptr +=            sz;
      U_MEMCPY(ptr, suffix, len_suffix);

      index_path_compress = buf_path_compress->distance(ptr) + 1;
      }
#endif
}

ULog::~ULog()
{
   U_TRACE_UNREGISTER_OBJECT(0, ULog)

   if (lock) delete lock;

#ifdef USE_LIBZ
   if (buf_path_compress) delete buf_path_compress;
#endif
}

void ULog::initDate()
{
   U_TRACE_NO_PARAM(1, "ULog::initDate()")

   iov_vec[0].iov_len  = 17;
   iov_vec[1].iov_len  =
   iov_vec[4].iov_len  = 1;
   iov_vec[0].iov_base = (caddr_t)date.date1;
   iov_vec[1].iov_base = (caddr_t)" ";
   iov_vec[2].iov_base = (caddr_t)u_buffer;
   iov_vec[4].iov_base = (caddr_t)U_LF;

   u_gettimenow();

   tv_sec_old_1 =
   tv_sec_old_2 =
   tv_sec_old_3 = u_now->tv_sec;

   (void) u_strftime2(date.date1, 17,               U_CONSTANT_TO_PARAM("%d/%m/%y %T"),                                                        tv_sec_old_1 + u_now_adjust);
   (void) u_strftime2(date.date2, 26,               U_CONSTANT_TO_PARAM("%d/%b/%Y:%T %z"),                                                     tv_sec_old_2 + u_now_adjust);
   (void) u_strftime2(date.date3, 6+29+2+12+2+17+2, U_CONSTANT_TO_PARAM("Date: %a, %d %b %Y %T GMT\r\nServer: ULib\r\nConnection: close\r\n"), tv_sec_old_3);
}

void ULog::startup()
{
   U_TRACE_NO_PARAM(1, "ULog::startup()")
  
   initDate();

   log(U_CONSTANT_TO_PARAM(U_FMT_START_STOP), "STARTUP", sizeof(void*) * 8);

   log(U_CONSTANT_TO_PARAM("Building Environment: " PLATFORM_VAR " (" __DATE__ ")"), 0);

#ifndef _MSWINDOWS_
   struct utsname u;

   (void) U_SYSCALL(uname, "%p", &u);

   log(U_CONSTANT_TO_PARAM("Current Operating System: %s %s v%s %s"), u.sysname, u.machine, u.version, u.release);
#endif

#if __BYTE_ORDER != __LITTLE_ENDIAN
   log(U_CONSTANT_TO_PARAM("Big endian arch detected"), 0);
#endif
}

void ULog::init(const char* _prefix, uint32_t _prefix_len)
{
   U_TRACE(0, "ULog::init(%.*S,%u)", _prefix_len, _prefix, _prefix_len)

   U_INTERNAL_ASSERT_EQUALS(pthis, 0)
   U_INTERNAL_ASSERT_EQUALS(U_Log_syslog(this), false)

   pthis                      = this;
   prefix                     = _prefix;
   prefix_len                 = _prefix_len;
   U_Log_start_stop_msg(this) = true;

   startup();
}

void ULog::setPrefix(const char* _prefix, uint32_t _prefix_len)
{
   U_TRACE(0, "ULog::setPrefix(%.*S,%u)", _prefix_len, _prefix, _prefix_len)

   U_INTERNAL_ASSERT_EQUALS(pthis, 0)

   pthis = this;

   if (U_Log_syslog(this) == false)
      {
      prefix                     = _prefix;
      prefix_len                 = _prefix_len;
      U_Log_start_stop_msg(this) = true;

      startup();
      }
}

void ULog::updateDate1()
{
   U_TRACE_NO_PARAM(1, "ULog::updateDate1()")

   /**
    * 18/06/12 18:45:56
    * 012345678901234567890123456789
    */

#if defined(U_LINUX) && defined(ENABLE_THREAD)
   if (u_pthread_time)
      {
      (void) U_SYSCALL(pthread_rwlock_rdlock, "%p", prwlock);

      if (tv_sec_old_1 != u_now->tv_sec)
         {
         long tv_sec = u_now->tv_sec;

         U_INTERNAL_DUMP("tv_sec_old_1 = %lu u_now->tv_sec = %lu", tv_sec_old_1, tv_sec)

         if ((tv_sec - tv_sec_old_1) != 1 ||
             (tv_sec % U_ONE_HOUR_IN_SECOND) == 0)
            {
            tv_sec_old_1 = tv_sec;

            U_MEMCPY(date.date1, ptr_shared_date->date1, 17);
            }
         else
            {
            ++tv_sec_old_1;

            u_put_unalignedp16(date.date1+12,  U_MULTICHAR_CONSTANT16(ptr_shared_date->date1[12],ptr_shared_date->date1[13]));
            u_put_unalignedp16(date.date1+12+3,U_MULTICHAR_CONSTANT16(ptr_shared_date->date1[15],ptr_shared_date->date1[16]));
            }

         U_INTERNAL_ASSERT_EQUALS(tv_sec, tv_sec_old_1)
         }

      (void) U_SYSCALL(pthread_rwlock_unlock, "%p", prwlock);
      }
   else
#endif
   {
   U_INTERNAL_ASSERT_EQUALS(u_pthread_time, 0)

   u_gettimenow();

   if (tv_sec_old_1 != u_now->tv_sec)
      {
      long tv_sec = u_now->tv_sec;

      U_INTERNAL_DUMP("tv_sec_old_1 = %lu u_now->tv_sec = %lu", tv_sec_old_1, tv_sec)

      if ((tv_sec - tv_sec_old_1) != 1 ||
          (tv_sec % U_ONE_HOUR_IN_SECOND) == 0)
         {
         (void) u_strftime2(date.date1, 17, U_CONSTANT_TO_PARAM("%d/%m/%y %T"), (tv_sec_old_1 = tv_sec) + u_now_adjust);
         }
      else
         {
         ++tv_sec_old_1;

         UTimeDate::updateTime(date.date1+12);
         }

      U_INTERNAL_ASSERT_EQUALS(tv_sec, tv_sec_old_1)
      }
   }

   U_INTERNAL_DUMP("date.date1 = %.17S", date.date1)
}

void ULog::updateDate2()
{
   U_TRACE_NO_PARAM(1, "ULog::updateDate2()")

   /**
    * 04/Jun/2012:18:18:37 +0200
    * 012345678901234567890123456789
    */

#if defined(U_LINUX) && defined(ENABLE_THREAD)
   if (u_pthread_time)
      {
      (void) U_SYSCALL(pthread_rwlock_rdlock, "%p", prwlock);

      if (tv_sec_old_2 != u_now->tv_sec)
         {
         long tv_sec = u_now->tv_sec;

         U_INTERNAL_DUMP("tv_sec_old_2 = %lu u_now->tv_sec = %lu", tv_sec_old_2, tv_sec)

         if ((tv_sec - tv_sec_old_2) != 1 ||
             (tv_sec % U_ONE_HOUR_IN_SECOND) == 0)
            {
            tv_sec_old_2 = tv_sec;

            U_MEMCPY(date.date2, ptr_shared_date->date2, 26);
            }
         else
            {
            ++tv_sec_old_2;

            u_put_unalignedp16(date.date2+15,  U_MULTICHAR_CONSTANT16(ptr_shared_date->date2[15],ptr_shared_date->date2[16]));
            u_put_unalignedp16(date.date2+15+3,U_MULTICHAR_CONSTANT16(ptr_shared_date->date2[18],ptr_shared_date->date2[19]));
            }

         U_INTERNAL_ASSERT_EQUALS(tv_sec, tv_sec_old_2)
         }

      (void) U_SYSCALL(pthread_rwlock_unlock, "%p", prwlock);
      }
   else
#endif
   {
   U_INTERNAL_ASSERT_EQUALS(u_pthread_time, 0)

   u_gettimenow();

   if (tv_sec_old_2 != u_now->tv_sec)
      {
      long tv_sec = u_now->tv_sec;

      U_INTERNAL_DUMP("tv_sec_old_2 = %lu u_now->tv_sec = %lu", tv_sec_old_2, tv_sec)

      if ((tv_sec - tv_sec_old_2) != 1 ||
          (tv_sec % U_ONE_HOUR_IN_SECOND) == 0)
         {
         (void) u_strftime2(date.date2, 26-6, U_CONSTANT_TO_PARAM("%d/%b/%Y:%T"), (tv_sec_old_2 = tv_sec) + u_now_adjust);
         }
      else
         {
         ++tv_sec_old_2;

         UTimeDate::updateTime(date.date2+15);
         }

      U_INTERNAL_ASSERT_EQUALS(tv_sec, tv_sec_old_2)
      }
   }

   U_INTERNAL_DUMP("date.date2 = %.26S", date.date2)
}

void ULog::updateDate3()
{
   U_TRACE_NO_PARAM(1, "ULog::updateDate3()")

   /**
    * Date: Wed, 20 Jun 2012 11:43:17 GMT\r\nServer: ULib\r\n
    *       0123456789012345678901234567890123
    * 0123456789012345678901234567890123456789
    */

#if defined(U_LINUX) && defined(ENABLE_THREAD)
   if (u_pthread_time)
      {
      (void) U_SYSCALL(pthread_rwlock_rdlock, "%p", prwlock);

      if (tv_sec_old_3 != u_now->tv_sec)
         {
         long tv_sec = u_now->tv_sec;

         U_INTERNAL_DUMP("tv_sec_old_3 = %lu u_now->tv_sec = %lu", tv_sec_old_3, tv_sec)

         if ((tv_sec - tv_sec_old_3) != 1 ||
             (tv_sec % U_ONE_HOUR_IN_SECOND) == 0)
            {
            tv_sec_old_3 = tv_sec;

            U_MEMCPY(date.date3+6, ptr_shared_date->date3+6, 29-4);
            }
         else
            {
            ++tv_sec_old_3;

            u_put_unalignedp16(date.date3+26,  U_MULTICHAR_CONSTANT16(ptr_shared_date->date3[26],ptr_shared_date->date3[27]));
            u_put_unalignedp16(date.date3+26+3,U_MULTICHAR_CONSTANT16(ptr_shared_date->date3[29],ptr_shared_date->date3[30]));
            }

         U_INTERNAL_ASSERT_EQUALS(tv_sec, tv_sec_old_3)
         }

      (void) U_SYSCALL(pthread_rwlock_unlock, "%p", prwlock);
      }
   else
#endif
   {
   U_INTERNAL_ASSERT_EQUALS(u_pthread_time, 0)

   u_gettimenow();

   if (tv_sec_old_3 != u_now->tv_sec)
      {
      long tv_sec = u_now->tv_sec;

      U_INTERNAL_DUMP("tv_sec_old_3 = %lu u_now->tv_sec = %lu", tv_sec_old_3, tv_sec)

      if ((tv_sec - tv_sec_old_3) != 1 ||
          (tv_sec % U_ONE_HOUR_IN_SECOND) == 0)
         {
         (void) u_strftime2(date.date3+6, 29-4, U_CONSTANT_TO_PARAM("%a, %d %b %Y %T"), (tv_sec_old_3 = tv_sec)); // GMT can't change...
         }
      else
         {
         ++tv_sec_old_3;

         UTimeDate::updateTime(date.date3+26);
         }

      U_INTERNAL_ASSERT_EQUALS(tv_sec, tv_sec_old_3)
      }
   }

   U_INTERNAL_DUMP("date.date3+6 = %.29S", date.date3+6)
}

void ULog::setShared(log_data* ptr, uint32_t _size, bool breference)
{
   U_TRACE(0, "ULog::setShared(%p,%u,%b)", ptr, _size, breference)

   U_INTERNAL_ASSERT_POINTER(lock)
   U_INTERNAL_ASSERT_POINTER(ptr_log_data)
   U_INTERNAL_ASSERT_EQUALS(U_Log_syslog(this), false)

   if (ptr) log_gzip_sz = _size;
   else
      {
      if (_size == 0)
         {
         log_data_sz = sizeof(log_data);

         ptr = (log_data*) UFile::mmap(&log_data_sz);

         U_INTERNAL_ASSERT_DIFFERS(ptr, MAP_FAILED)
         }
      else
         {
         char somename[256];

         // -------------------------------------------------------------------------------------------------------------------
         // For portable use, a shared memory object should be identified by a name of the form /somename;
         // that is, a null-terminated string of up to NAME_MAX (i.e., 255) characters consisting of an
         // initial slash, followed by one or more characters, none of which are slashes
         // -------------------------------------------------------------------------------------------------------------------

         UString basename = UFile::getName();

         (void) u__snprintf(somename, sizeof(somename), U_CONSTANT_TO_PARAM("/%v"), basename.rep);

         // -------------------------------------------------------------------------------------------------------------------
         // ULog::log_data log_data_shared;
         // -> unnamed array of char for gzip compression (log rotate)...
         // -------------------------------------------------------------------------------------------------------------------
         // The zlib documentation states that destination buffer size must be at least 0.1% larger than avail_in plus 12 bytes
         // -------------------------------------------------------------------------------------------------------------------

         ptr = (log_data*) UFile::shm_open(somename, sizeof(log_data) + _size);

         U_INTERNAL_DUMP("ptr->file_ptr = %u", ptr->file_ptr)

         log_gzip_sz = _size;
         }
      }

   if (breference == false)
      {
      ptr->file_ptr  =
      ptr->file_page = 0;
      }
   else
      {
      ptr->file_ptr  = ptr_log_data->file_ptr;
      ptr->file_page = ptr_log_data->file_page;
      }

   U_FREE_TYPE(ptr_log_data, log_data);

   ptr_log_data           = ptr;
   ptr_log_data->gzip_len = 0;

   lock->init(&(ptr_log_data->lock_shared), ptr_log_data->spinlock_shared);

   U_INTERNAL_DUMP("ptr_log_data->file_ptr = %u UFile::st_size = %u log_gzip_sz = %u", ptr_log_data->file_ptr, UFile::st_size, log_gzip_sz)

   U_INTERNAL_ASSERT(ptr_log_data->file_ptr <= UFile::st_size)
}

void ULog::write(const struct iovec* iov, int n)
{
   U_TRACE(1+256, "ULog::write(%p,%d)", iov, n)

   U_INTERNAL_ASSERT_EQUALS(U_Log_syslog(this), false)

   if (log_file_sz == 0) (void) UFile::writev(iov, n);
   else
      {
      uint32_t file_ptr;

      lock->lock();

      file_ptr = ptr_log_data->file_ptr;

      U_INTERNAL_DUMP("UFile::map = %p ptr_log_data->file_ptr = %u log_file_sz = %u", UFile::map, file_ptr, log_file_sz)

      for (int i = 0; i < n; ++i)
         {
         int len = iov[i].iov_len;

         if (len == 0) continue;

         const char* ptr = (const char*)iov[i].iov_base;

      // U_INTERNAL_DUMP("iov[%d](%u) -> %.*S", i, len, len, ptr)

         if ((file_ptr+len) > log_file_sz) // if overwrite log file we compress it as gzip...
            {
#        ifdef USE_LIBZ
            U_INTERNAL_DUMP("UFile::st_size = %u log_gzip_sz = %u", UFile::st_size, log_gzip_sz)

            U_INTERNAL_ASSERT_MAJOR(file_ptr, 0)
            U_INTERNAL_ASSERT(file_ptr <= UFile::st_size)

            // NB: the shared area to compress log data may be not available at this time... (Ex: startup plugin u_server)

            if (file_ptr <= log_gzip_sz)
               {
               checkForLogRotateDataToWrite(); // check if there are previous data to write

               ptr_log_data->gzip_len = u_gz_deflate(UFile::map, file_ptr, (char*)ptr_log_data+sizeof(log_data), true);

               U_INTERNAL_DUMP("u_gz_deflate(%u) = %u", file_ptr, ptr_log_data->gzip_len)
               }
            else
               {
               U_INTERNAL_ASSERT_EQUALS(ptr_log_data->gzip_len, 0)

               UString data_to_write = UStringExt::deflate(UFile::map, file_ptr, 1);

               char* ptr1 = buf_path_compress->c_pointer(index_path_compress);

               ptr1[u__snprintf(ptr1, 17, U_CONSTANT_TO_PARAM("%4D"))] = '.';

               (void) UFile::writeTo(*buf_path_compress, data_to_write, O_RDWR | O_EXCL, false);
               }
#        endif

                          file_ptr  =
            ptr_log_data->file_page = 0;
            }

         if (len == 1) UFile::map[file_ptr++] = *ptr;
         else
            {
            U_MEMCPY(UFile::map + file_ptr, ptr, len);

            file_ptr += len;

         // U_INTERNAL_DUMP("memcpy(%u) => %.*S", len, len, UFile::map + file_ptr - len)
            }
         }

      U_INTERNAL_DUMP("ptr_log_data->file_ptr = %u", file_ptr)

      if ((log_file_sz-file_ptr) > U_CONSTANT_SIZE(U_MARK_END))
         {
         char* ptr = UFile::map + file_ptr;

         u_put_unalignedp64(ptr,    U_MULTICHAR_CONSTANT64('\n','\n','\n','\n','\n','\n','\n','\n'));
         u_put_unalignedp64(ptr+8,  U_MULTICHAR_CONSTANT64('\n','\n','\n','\n','\n','\n','\n','\n'));
         u_put_unalignedp64(ptr+16, U_MULTICHAR_CONSTANT64('\n','\n','\n','\n','\n','\n','\n','\n'));
         }

      ptr_log_data->file_ptr = file_ptr;

      lock->unlock();
      }
}

void ULog::write(const char* msg, uint32_t len)
{
   U_TRACE(0+256, "ULog::write(%.*S,%u)", len, msg, len)

   if (U_Log_syslog(pthis))
      {
#  ifndef _MSWINDOWS_
      U_SYSCALL_VOID(syslog, "%d,%S,%d,%p", LOG_INFO, "%.*s", (int)len, (char*)msg);
#  endif

      return;
      }

   U_INTERNAL_ASSERT_EQUALS(iov_vec[0].iov_len, 17)
   U_INTERNAL_ASSERT_EQUALS(iov_vec[1].iov_len,  1)
   U_INTERNAL_ASSERT_EQUALS(iov_vec[4].iov_len,  1)

   if (prefix_len)
      {
      U_INTERNAL_DUMP("u_buffer_len = %u", u_buffer_len)

      U_INTERNAL_ASSERT_MINOR(u_buffer_len, U_BUFFER_SIZE - 100)

      iov_vec[2].iov_base = (caddr_t) u_buffer + u_buffer_len;
      iov_vec[2].iov_len  = u__snprintf((char*)iov_vec[2].iov_base, U_BUFFER_SIZE - u_buffer_len, prefix, prefix_len, 0);
      }

   iov_vec[3].iov_len  = len;
   iov_vec[3].iov_base = (caddr_t) msg;

   updateDate1();

   pthis->write(iov_vec, 5);

   if (prefix_len) iov_vec[2].iov_len = 0;
}

void ULog::log(const char* fmt, uint32_t fmt_size, ...)
{
   U_TRACE(0, "ULog::log(%.*S,%u)", fmt_size, fmt, fmt_size)

   uint32_t len;
   char buffer[8196];

   va_list argp;
   va_start(argp, fmt_size);

   len = u__vsnprintf(buffer, sizeof(buffer), fmt, fmt_size, argp);

   va_end(argp);

   write(buffer, len);
}

void ULog::log(int _fd, const char* fmt, uint32_t fmt_size, ...)
{
   U_TRACE(1, "ULog::log(%d,%.*S,%u)", _fd, fmt_size, fmt, fmt_size)

   U_INTERNAL_ASSERT_DIFFERS(_fd, -1)

   uint32_t len;
   char buffer[8196];

   va_list argp;
   va_start(argp, fmt_size);

   len = u__vsnprintf(buffer, sizeof(buffer), fmt, fmt_size, argp);

   va_end(argp);

   U_INTERNAL_ASSERT_EQUALS(iov_vec[0].iov_len, 17)
   U_INTERNAL_ASSERT_EQUALS(iov_vec[1].iov_len,  1)
   U_INTERNAL_ASSERT_EQUALS(iov_vec[4].iov_len,  1)

   iov_vec[3].iov_len  = len;
   iov_vec[3].iov_base = (caddr_t)buffer;

   updateDate1();

   (void) U_SYSCALL(writev, "%d,%p,%d", _fd, iov_vec, 5);
}

void ULog::log(const struct iovec* iov, const char* name, const char* type, int ncount, const char* msg, uint32_t msg_len, const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0, "ULog::log(%p,%S,%S,%d,%.*S,%u,%.*S,%u)", iov, name, type, ncount, msg_len, msg, msg_len, fmt_size, format, fmt_size)

   U_INTERNAL_ASSERT_MAJOR(ncount, 0)

   char buffer1[2000], buffer2[8192];
   const char* ptr = (const char*)iov[2].iov_base;
   uint32_t len, u_printf_string_max_length_save = u_printf_string_max_length,
            sz = iov[2].iov_len, sz1 = iov[0].iov_len + iov[1].iov_len, sz_header = sz1 + sz;

   U_INTERNAL_DUMP("u_printf_string_max_length = %d iov[0].len = %d iov[1].len = %d iov[2].len = %d iov[3].len = %d",
                    u_printf_string_max_length,     iov[0].iov_len, iov[1].iov_len, iov[2].iov_len, iov[3].iov_len)

#ifndef U_HTTP2_DISABLE
   if (U_http_version != '2')
#endif
   {
   if (u_printf_string_max_length == -1)
      {
      uint32_t endHeader = (sz ? u_findEndHeader1(ptr, sz) : U_NOT_FOUND);

      if (endHeader == U_NOT_FOUND) u_printf_string_max_length = U_min(sz_header, sizeof(buffer1));
      else                          u_printf_string_max_length = sz1 + endHeader;

      U_INTERNAL_DUMP("endHeader = %u sz_header = %u", endHeader, sz_header)

      // NB: with partial response we can have u_printf_string_max_length == 0...

      U_INTERNAL_ASSERT(u_printf_string_max_length <= (int)sz_header)
      }
   }

   U_INTERNAL_DUMP("u_printf_string_max_length = %d sz1 = %u", u_printf_string_max_length, sz1)

   if (sz1)
      {
      int maxlen = (u_printf_string_max_length > 0 ? u_printf_string_max_length : 128) - sz1;

      if (maxlen  > 0 &&
          (int)sz > maxlen)
         {
         sz = maxlen;

         U_INTERNAL_ASSERT_POINTER(ptr)
         }

      int length = sizeof(buffer1)-sz-1;

      if (iov[0].iov_len)
         {
         length -= iov[0].iov_len;

              if (length > (int)iov[0].iov_len) length = iov[0].iov_len;
         else if (length <= 1)                  length = 128;

         U_INTERNAL_DUMP("length = %d maxlen = %d", length, maxlen)

         U_INTERNAL_ASSERT_MAJOR(length, 0)
         U_INTERNAL_ASSERT_MINOR(length, (int)sizeof(buffer1))

         U_MEMCPY(buffer1, (const char*)iov[0].iov_base, length);
         }

      if (iov[1].iov_len)
         {
         U_INTERNAL_ASSERT_MINOR(length+iov[1].iov_len, sizeof(buffer1))

         U_MEMCPY(buffer1+length, (const char*)iov[1].iov_base, iov[1].iov_len);
         }

      U_INTERNAL_ASSERT_MINOR(length+iov[1].iov_len+sz, sizeof(buffer1))

      if (sz)
         {
         U_INTERNAL_ASSERT_POINTER(ptr)

         U_MEMCPY(buffer1+length+iov[1].iov_len, ptr, sz);
         }

      sz += sz1;
      ptr = buffer1;
      }

   len = u__snprintf(buffer2, sizeof(buffer2), U_CONSTANT_TO_PARAM("%ssend %s (%u bytes) %.*s%.*S"), name, type, ncount, msg_len, msg, sz, ptr);

   va_list argp;
   va_start(argp, fmt_size);

   len += u__vsnprintf(buffer2+len, sizeof(buffer2)-len, format, fmt_size, argp);

   va_end(argp);

   write(buffer2, len);

   u_printf_string_max_length = u_printf_string_max_length_save;
}

void ULog::logResponse(const UString& data, const char* name, const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0, "ULog::logResponse(%V,%S,%.*S,%u)", data.rep, name, fmt_size, format, fmt_size)

   U_INTERNAL_ASSERT(data)

   char buffer[8192];
   const char* ptr  = data.data();
   uint32_t len, sz = data.size(), u_printf_string_max_length_save = u_printf_string_max_length;

   U_INTERNAL_DUMP("u_printf_string_max_length = %d", u_printf_string_max_length)

   if (u_printf_string_max_length == -1)
      {
      u_printf_string_max_length = u_findEndHeader1(ptr, sz);

      if ((uint32_t)u_printf_string_max_length == U_NOT_FOUND) u_printf_string_max_length = U_min(sz,2000);

      U_INTERNAL_ASSERT_MAJOR(u_printf_string_max_length, 0)
      }

   U_INTERNAL_DUMP("u_printf_string_max_length = %d", u_printf_string_max_length)

   len = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%sreceived response (%u bytes) %.*S"), name, sz, sz, ptr);

   va_list argp;
   va_start(argp, fmt_size);

   len += u__vsnprintf(buffer+len, sizeof(buffer)-len, format, fmt_size, argp);

   va_end(argp);

   write(buffer, len);

   u_printf_string_max_length = u_printf_string_max_length_save;
}

void ULog::logger(const char* ident, int priority, const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(1, "ULog::logger(%S,%d,%.*S,%u)", ident, priority, fmt_size, format, fmt_size)

#ifndef _MSWINDOWS_
   U_INTERNAL_ASSERT(U_Log_syslog(pthis))

   if (format == 0)
      {
      U_SYSCALL_VOID(openlog, "%S,%d,%d", ident, 0, 0);

      return;
      }

   uint32_t len;
   char buffer[4096];

   va_list argp;
   va_start(argp, fmt_size);

   len = u__vsnprintf(buffer, sizeof(buffer), format, fmt_size, argp);

   va_end(argp);

   U_SYSCALL_VOID(syslog, "%d,%S,%d,%p", priority, "%.*s", len, buffer);
#endif
}

// Decode a symbolic name to a numeric value

__pure U_NO_EXPORT int ULog::decode(const char* name, uint32_t len, bool bfacility)
{
   U_TRACE(0, "ULog::decode(%.*S,%u,%b)", len, name, len, bfacility)

#ifndef _MSWINDOWS_
   for (CODE* c = (bfacility ? facilitynames : prioritynames); c->c_name; ++c) if (u__strncasecmp(name, c->c_name, len) == 0) U_RETURN(c->c_val);
#endif

   U_RETURN(-1);
}

__pure int ULog::getPriorityForLogger(const char* s)
{
   U_TRACE(0, "ULog::getPriorityForLogger(%S)", s)

   int res;

#ifdef _MSWINDOWS_
   res = 0;
#else
   int fac, lev;
   const char* ptr;

   for (ptr = s; *ptr && *ptr != '.'; ++ptr) {}

   if (*ptr)
      {
      U_INTERNAL_ASSERT_EQUALS(*ptr, '.')

      fac = decode(s, ptr++ - s, true);
      }
   else
      {
      ptr = s;
      fac = LOG_USER;
      }

   lev = decode(s, ptr - s, false);

   res = ((lev & LOG_PRIMASK) | (fac & LOG_FACMASK));
#endif

   U_RETURN(res);
}

void ULog::closeLog()
{
   U_TRACE_NO_PARAM(1, "ULog::closeLog()")

   if (U_Log_syslog(this))
      {
#  ifndef _MSWINDOWS_
      U_SYSCALL_VOID_NO_PARAM(closelog);
#  endif

      return;
      }

   U_INTERNAL_DUMP("log_file_sz = %u", log_file_sz)

   if (log_file_sz)
      {
      U_INTERNAL_ASSERT_MINOR(ptr_log_data->file_ptr, UFile::st_size)

   // msync();

#  ifdef USE_LIBZ
      checkForLogRotateDataToWrite(); // check for previous data to write
#  endif

      U_INTERNAL_ASSERT_EQUALS(ptr_log_data->gzip_len, 0)

             UFile::munmap();
      (void) UFile::ftruncate(ptr_log_data->file_ptr);
             UFile::fsync();
      }

   UFile::close();

   if (log_gzip_sz == sizeof(log_data)) UFile::munmap(ptr_log_data, log_data_sz);
}

void ULog::close()
{
   U_TRACE_NO_PARAM(0, "ULog::close()")

   // NB: we need this check because all child try to close the log... (inherits from its parent)

   if (pthis)
      {
      U_INTERNAL_DUMP("pthis = %p", pthis)

      if (U_Log_start_stop_msg(pthis)) log(U_CONSTANT_TO_PARAM(U_FMT_START_STOP), "SHUTDOWN", sizeof(void*) * 8);

      pthis->closeLog();

      pthis = 0;
      }
}

#ifdef USE_LIBZ
UString ULog::getDirLogGz()
{
   U_TRACE_NO_PARAM(0, "ULog::getDirLogGz()")

   U_INTERNAL_ASSERT_POINTER(pthis)
   U_INTERNAL_ASSERT_POINTER(pthis->buf_path_compress)

   UString result = UStringExt::dirname(*(pthis->buf_path_compress));

   U_RETURN_STRING(result);
}

void ULog::checkForLogRotateDataToWrite()
{
   U_TRACE_NO_PARAM(0, "ULog::checkForLogRotateDataToWrite()")

   if (ptr_log_data->gzip_len)
      {
      // there are previous data to write

      char* ptr1 = buf_path_compress->c_pointer(index_path_compress);

      ptr1[u__snprintf(ptr1, 17, U_CONSTANT_TO_PARAM("%4D"))] = '.';

      (void) UFile::writeTo(*buf_path_compress, (char*)ptr_log_data+sizeof(log_data), ptr_log_data->gzip_len, O_RDWR | O_EXCL, false);

      ptr_log_data->gzip_len = 0;
      }
}
#endif

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* ULog::dump(bool _reset) const
{
   UFile::dump(false);

   *UObjectIO::os << '\n'
                  << "prefix_len                " << prefix_len  << '\n'
                  << "log_file_sz               " << log_file_sz << '\n'
                  << "lock     (ULock           " << (void*)lock << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
