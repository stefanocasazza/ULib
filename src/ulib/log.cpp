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

#include <ulib/date.h>
#include <ulib/net/server/server.h>

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
#endif

#define U_MARK_END "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" // 24
#define U_FMT_START_STOP "*** %s %N (%ubit, pid %P) [%U@%H] ***"

long              ULog::tv_sec_old_1;
long              ULog::tv_sec_old_2;
long              ULog::tv_sec_old_3;
uint32_t          ULog::prefix_len;
const char*       ULog::prefix;
struct iovec      ULog::iov_vec[5];
ULog::log_date    ULog::date;
ULog::log_date*   ULog::ptr_shared_date;
#if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
pthread_rwlock_t* ULog::prwlock;
#endif

ULog::ULog(const UString& path, uint32_t _size) : UFile(path, U_NULLPTR)
{
   U_TRACE_CTOR(0, ULog, "%V,%u", path.rep, _size)

#ifdef DEBUG
   U_INTERNAL_DUMP("first = %p next = %p this = %p", first, next, this)

   next  = first;
           first = this;

   U_INTERNAL_DUMP("first = %p next = %p this = %p", first, next, this)

   U_INTERNAL_ASSERT_DIFFERS(first, next)
#endif

   log_file_sz  =
   log_gzip_sz  = 0;
   ptr_log_data = U_NULLPTR;

   U_Log_syslog(this)         =
   U_Log_start_stop_msg(this) = false;

#ifdef USE_LIBZ
     buf_path_compress = U_NULLPTR;
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
      }

   /**
    * typedef struct log_data {
    *  uint32_t file_ptr;
    *  uint32_t file_page;
    *  uint32_t gzip_len;
    *  sem_t lock_shared;
    *  // --------------> maybe unnamed array of char for gzip compression...
    * } log_data;
    */

   ptr_log_data = U_MALLOC_TYPE(log_data);

   ptr_log_data->file_ptr = 0;
   ptr_log_data->gzip_len = 0;

   if (_size)
      {
      uint32_t file_size = UFile::size();

      bool bsize = (file_size != _size);

      if ((bsize && UFile::ftruncate(_size) == false) ||
          UFile::memmap(PROT_READ | PROT_WRITE) == false)
         {
         U_ERROR("Cannot init log file %.*S", U_FILE_TO_TRACE(*this));
         }

      U_INTERNAL_DUMP("bsize = %b", bsize)

      if (bsize) ptr_log_data->file_ptr = file_size; // append mode
      else
         {
         // NB: we can have a previous crash (so the file is not truncate) or there are an other process that already use this file...

         char* ptr = (char*) u_find(UFile::map, file_size, U_CONSTANT_TO_PARAM(U_MARK_END));

         if (ptr == U_NULLPTR) ptr = (char*) u_find(UFile::map, file_size, "\0\0\0\0\0\0\0\0", 8);

         if (ptr) ptr_log_data->file_ptr = ptr - UFile::map;

         U_INTERNAL_ASSERT_MINOR(ptr_log_data->file_ptr, UFile::st_size)
         }

      log_file_sz = UFile::st_size;
      }

   U_INTERNAL_DUMP("UFile::map = %p ptr_log_data->file_ptr = %u log_file_sz = %u", UFile::map, ptr_log_data->file_ptr, log_file_sz)

   U_INTERNAL_ASSERT(ptr_log_data->file_ptr <= UFile::st_size)

   ptr_log_data->file_page = ptr_log_data->file_ptr;
}

void ULog::initDate()
{
   U_TRACE_NO_PARAM(1, "ULog::initDate()")

   iov_vec[0].iov_len  = 17;
   iov_vec[1].iov_len  =
   iov_vec[4].iov_len  = 1;
   iov_vec[0].iov_base = (caddr_t)date.date1;
   iov_vec[1].iov_base = (caddr_t)" ";
   iov_vec[4].iov_base = (caddr_t)U_LF;

   u_gettimenow();

   tv_sec_old_1 =
   tv_sec_old_2 =
   tv_sec_old_3 = u_now->tv_sec;

   (void) u_strftime2(date.date1, 17,   U_CONSTANT_TO_PARAM("%d/%m/%y %T"),    u_get_localtime(tv_sec_old_1));
   (void) u_strftime2(date.date2, 26,   U_CONSTANT_TO_PARAM("%d/%b/%Y:%T %z"), u_get_localtime(tv_sec_old_2));
   (void) u_strftime2(date.date3, 6+29, U_CONSTANT_TO_PARAM("Date: %a, %d %b %Y %T GMT"),      tv_sec_old_3);
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

void ULog::updateDate1()
{
   U_TRACE_NO_PARAM(1, "ULog::updateDate1()")

   /**
    * 18/06/12 18:45:56
    * 012345678901234567890123456789
    */

#if defined(U_LINUX) && !defined(U_SERVER_CAPTIVE_PORTAL) && defined(ENABLE_THREAD)
   if (u_pthread_time)
      {
      if (tv_sec_old_1 != u_now->tv_sec)
         {
         (void) U_SYSCALL(pthread_rwlock_rdlock, "%p", prwlock);

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

         (void) U_SYSCALL(pthread_rwlock_unlock, "%p", prwlock);
         }
      }
   else
#endif
   {
   U_INTERNAL_ASSERT_EQUALS(u_pthread_time, U_NULLPTR)

   u_gettimenow();

   if (tv_sec_old_1 != u_now->tv_sec)
      {
      long tv_sec = u_now->tv_sec;

      U_INTERNAL_DUMP("tv_sec_old_1 = %lu u_now->tv_sec = %lu", tv_sec_old_1, tv_sec)

      if ((tv_sec - tv_sec_old_1) != 1 ||
          (tv_sec % U_ONE_HOUR_IN_SECOND) == 0)
         {
         (void) u_strftime2(date.date1, 17, U_CONSTANT_TO_PARAM("%d/%m/%y %T"), u_get_localtime(tv_sec_old_1 = tv_sec));
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

#if defined(U_LINUX) && !defined(U_SERVER_CAPTIVE_PORTAL) && defined(ENABLE_THREAD)
   if (u_pthread_time)
      {
      if (tv_sec_old_2 != u_now->tv_sec)
         {
         (void) U_SYSCALL(pthread_rwlock_rdlock, "%p", prwlock);

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

         (void) U_SYSCALL(pthread_rwlock_unlock, "%p", prwlock);
         }
      }
   else
#endif
   {
   U_INTERNAL_ASSERT_EQUALS(u_pthread_time, U_NULLPTR)

   u_gettimenow();

   if (tv_sec_old_2 != u_now->tv_sec)
      {
      long tv_sec = u_now->tv_sec;

      U_INTERNAL_DUMP("tv_sec_old_2 = %lu u_now->tv_sec = %lu", tv_sec_old_2, tv_sec)

      if ((tv_sec - tv_sec_old_2) != 1 ||
          (tv_sec % U_ONE_HOUR_IN_SECOND) == 0)
         {
         (void) u_strftime2(date.date2, 26-6, U_CONSTANT_TO_PARAM("%d/%b/%Y:%T"), u_get_localtime(tv_sec_old_2 = tv_sec));
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

void ULog::updateDate3(char* ptr_date)
{
   U_TRACE(1, "ULog::updateDate3(%p)", ptr_date)

   /**
    * Date: Wed, 20 Jun 2012 11:43:17 GMT\r\nServer: ULib\r\n
    *       0123456789012345678901234567890123
    * 0123456789012345678901234567890123456789
    */

#if defined(U_LINUX) && !defined(U_SERVER_CAPTIVE_PORTAL) && defined(ENABLE_THREAD)
   if (u_pthread_time)
      {
      if (tv_sec_old_3 != u_now->tv_sec)
         {
         (void) U_SYSCALL(pthread_rwlock_rdlock, "%p", prwlock);

         long tv_sec = u_now->tv_sec;

         U_INTERNAL_DUMP("tv_sec_old_3 = %lu u_now->tv_sec = %lu", tv_sec_old_3, tv_sec)

         /*
         U_INTERNAL_ASSERT_DIFFERS(u_get_unalignedp64(            date.date3+6+U_CONSTANT_SIZE("Wed, 20 Jun 2012 ")),
                                   u_get_unalignedp64(ptr_shared_date->date3+6+U_CONSTANT_SIZE("Wed, 20 Jun 2012 ")))
         */

         if ((tv_sec - tv_sec_old_3) != 1 ||
             (tv_sec % U_ONE_HOUR_IN_SECOND) == 0)
            {
            tv_sec_old_3 = tv_sec;

                          U_MEMCPY(date.date3+6, ptr_shared_date->date3+6, 29-4);
            if (ptr_date) U_MEMCPY(  ptr_date+6, ptr_shared_date->date3+6, 29-4);
            }
         else
            {
            ++tv_sec_old_3;

            u_put_unalignedp16(date.date3+26,  U_MULTICHAR_CONSTANT16(ptr_shared_date->date3[26],ptr_shared_date->date3[27]));
            u_put_unalignedp16(date.date3+26+3,U_MULTICHAR_CONSTANT16(ptr_shared_date->date3[29],ptr_shared_date->date3[30]));

            if (ptr_date)
               {
               u_put_unalignedp16(ptr_date+26,  U_MULTICHAR_CONSTANT16(ptr_shared_date->date3[26],ptr_shared_date->date3[27]));
               u_put_unalignedp16(ptr_date+26+3,U_MULTICHAR_CONSTANT16(ptr_shared_date->date3[29],ptr_shared_date->date3[30]));
               }
            }

         U_INTERNAL_ASSERT_EQUALS(tv_sec, tv_sec_old_3)

         (void) U_SYSCALL(pthread_rwlock_unlock, "%p", prwlock);
         }
      }
   else
#endif
   {
   U_INTERNAL_ASSERT_EQUALS(u_pthread_time, U_NULLPTR)

   u_gettimenow();

   if (tv_sec_old_3 != u_now->tv_sec)
      {
      long tv_sec = u_now->tv_sec;

      U_INTERNAL_DUMP("tv_sec_old_3 = %lu u_now->tv_sec = %lu", tv_sec_old_3, tv_sec)

      if ((tv_sec - tv_sec_old_3) != 1 ||
          (tv_sec % U_ONE_HOUR_IN_SECOND) == 0)
         {
         (void) u_strftime2(date.date3+6, 29-4, U_CONSTANT_TO_PARAM("%a, %d %b %Y %T"), (tv_sec_old_3 = tv_sec)); // GMT can't change...

         if (ptr_date) U_MEMCPY(ptr_date+6, date.date3+6, 29-4);
         }
      else
         {
         ++tv_sec_old_3;

                       UTimeDate::updateTime(date.date3+26);
         if (ptr_date) UTimeDate::updateTime(  ptr_date+26);
         }

      U_INTERNAL_ASSERT_EQUALS(tv_sec, tv_sec_old_3)
      }
   }

#ifdef DEBUG
   U_INTERNAL_DUMP("date.date3+6 = %.29S", date.date3+6)

   if (ptr_date) U_INTERNAL_DUMP("ptr_date+6 = %.29S", ptr_date+6)
#endif
}

void ULog::write(const struct iovec* iov, int n)
{
   U_TRACE(1+256, "ULog::write(%p,%d)", iov, n)

   U_INTERNAL_ASSERT_EQUALS(U_Log_syslog(this), false)

   if (log_file_sz == 0)
      {
      (void) UFile::writev(iov, n);

      return;
      }

   int len;
   const char* ptr;
   uint32_t file_ptr;

   U_INTERNAL_DUMP("UFile::map = %p ptr_log_data->file_ptr = %u log_file_sz = %u", UFile::map, ptr_log_data->file_ptr, log_file_sz)

   lock();

   file_ptr = ptr_log_data->file_ptr;

   for (int i = 0; i < n; ++i)
      {
      if ((len = iov[i].iov_len))
         {
         ptr = (const char*)iov[i].iov_base;

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
#           ifndef U_COVERITY_FALSE_POSITIVE // FORWARD_NULL
               checkForLogRotateDataToWrite(); // check if there are previous data to write
#           endif

               ptr_log_data->gzip_len = u_gz_deflate(UFile::map, file_ptr, (char*)ptr_log_data+sizeof(log_data), Z_DEFAULT_COMPRESSION);

               U_INTERNAL_DUMP("u_gz_deflate(%u) = %u", file_ptr, ptr_log_data->gzip_len)
               }
            else if (buf_path_compress)
               {
               U_INTERNAL_ASSERT_EQUALS(ptr_log_data->gzip_len, 0)

               UString data_to_write = UStringExt::deflate(UFile::map, file_ptr, 0);

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
      }

   U_INTERNAL_DUMP("ptr_log_data->file_ptr = %u", file_ptr)

   if ((log_file_sz - file_ptr) > U_CONSTANT_SIZE(U_MARK_END))
      {
      char* p = UFile::map + file_ptr;

      u_put_unalignedp64(p,    U_MULTICHAR_CONSTANT64('\n','\n','\n','\n','\n','\n','\n','\n'));
      u_put_unalignedp64(p+8,  U_MULTICHAR_CONSTANT64('\n','\n','\n','\n','\n','\n','\n','\n'));
      u_put_unalignedp64(p+16, U_MULTICHAR_CONSTANT64('\n','\n','\n','\n','\n','\n','\n','\n'));
      }

   ptr_log_data->file_ptr = file_ptr;

   unlock();
}

void ULog::write(const char* msg, uint32_t len)
{
   U_TRACE(0+256, "ULog::write(%.*S,%u)", len, msg, len)

   if (U_Log_syslog(this))
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
      static char buffer[32];

      iov_vec[2].iov_base = (caddr_t)buffer;
      iov_vec[2].iov_len  = u__snprintf(buffer, U_CONSTANT_SIZE(buffer), prefix, prefix_len, 0);
      }

   iov_vec[3].iov_len  = len;
   iov_vec[3].iov_base = (caddr_t) msg;

   updateDate1();

   write(iov_vec, 5);

   iov_vec[2].iov_len = 0;
}

void ULog::log(int lfd, const char* fmt, uint32_t fmt_size, ...)
{
   U_TRACE(1, "ULog::log(%d,%.*S,%u)", lfd, fmt_size, fmt, fmt_size)

   U_INTERNAL_ASSERT_DIFFERS(lfd, -1)

   uint32_t len;
   char buffer[8196];

   va_list argp;
   va_start(argp, fmt_size);

   len = u__vsnprintf(buffer, U_CONSTANT_SIZE(buffer), fmt, fmt_size, argp);

   va_end(argp);

   if (UServer_Base::isLog() &&
       lfd == UServer_Base::log->getFd())
      {
      UServer_Base::log->write(buffer, len);
      }
   else
      {
      U_INTERNAL_ASSERT_EQUALS(iov_vec[0].iov_len, 17)
      U_INTERNAL_ASSERT_EQUALS(iov_vec[1].iov_len,  1)
      U_INTERNAL_ASSERT_EQUALS(iov_vec[4].iov_len,  1)

      iov_vec[3].iov_len  = len;
      iov_vec[3].iov_base = (caddr_t)buffer;

      updateDate1();

      (void) U_SYSCALL(writev, "%d,%p,%d", lfd, iov_vec, 5);
      }
}

void ULog::log(UString& lbuffer, const char* fmt, uint32_t fmt_size, ...)
{
   U_TRACE(0, "ULog::log(%V,%.*S,%u)", lbuffer.rep, fmt_size, fmt, fmt_size)

   uint32_t len;
   char buffer[8196];

   va_list argp;
   va_start(argp, fmt_size);

   len = u__vsnprintf(buffer, U_CONSTANT_SIZE(buffer), fmt, fmt_size, argp);

   va_end(argp);

   U_INTERNAL_ASSERT_EQUALS(iov_vec[0].iov_len, 17)
   U_INTERNAL_ASSERT_EQUALS(iov_vec[1].iov_len,  1)
   U_INTERNAL_ASSERT_EQUALS(iov_vec[4].iov_len,  1)

   iov_vec[3].iov_len  = len;
   iov_vec[3].iov_base = (caddr_t)buffer;

   updateDate1();

   for (int i = 0; i < 5; ++i)
      {
      if (iov_vec[i].iov_len) (void) lbuffer.append((const char*)iov_vec[i].iov_base, iov_vec[i].iov_len);
      }
}

void ULog::log(const struct iovec* iov, const char* type, int ncount, const char* msg, uint32_t msg_len, const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0, "ULog::log(%p,%S,%d,%.*S,%u,%.*S,%u)", iov, type, ncount, msg_len, msg, msg_len, fmt_size, format, fmt_size)

   U_INTERNAL_ASSERT_MAJOR(ncount, 0)

   char buffer1[8192], buffer2[8192];
   const char* ptr = (const char*)iov[2].iov_base;
   uint32_t len, u_printf_string_max_length_save = u_printf_string_max_length,
            sz = iov[2].iov_len, sz1 = iov[0].iov_len + iov[1].iov_len, sz_header = sz1 + sz;

   U_INTERNAL_DUMP("u_printf_string_max_length = %d iov[0].len = %d iov[1].len = %d iov[2].len = %d iov[3].len = %d",
                    u_printf_string_max_length,     iov[0].iov_len, iov[1].iov_len, iov[2].iov_len, iov[3].iov_len)

   if (u_printf_string_max_length == -1)
      {
      uint32_t endHeader = (sz ? u_findEndHeader1(ptr, sz) : U_NOT_FOUND);

      U_INTERNAL_DUMP("endHeader = %u sz_header = %u", endHeader, sz_header)

      if (endHeader == U_NOT_FOUND) u_printf_string_max_length = U_min(sz_header, sizeof(buffer1));
      else                          u_printf_string_max_length = sz1 + endHeader;

      if (u_printf_string_max_length < 2000) u_printf_string_max_length = 2000;
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

   len  = (UServer_Base::mod_name[0][0] ? u__snprintf(buffer2, U_CONSTANT_SIZE(buffer2), U_CONSTANT_TO_PARAM("%s"), UServer_Base::mod_name) : 0);
   len += u__snprintf(buffer2+len, U_CONSTANT_SIZE(buffer2)-len, U_CONSTANT_TO_PARAM("send %s (%u bytes) %.*s%#.*S"),type,ncount,msg_len,msg,(sz <= (uint32_t)ncount ? sz : ncount),ptr);

   va_list argp;
   va_start(argp, fmt_size);

   len += u__vsnprintf(buffer2+len, U_CONSTANT_SIZE(buffer2)-len, format, fmt_size, argp);

   va_end(argp);

   write(buffer2, len);

   u_printf_string_max_length = u_printf_string_max_length_save;
}

void ULog::logResponse(const UString& data, const char* format, uint32_t fmt_size, ...) // Ex: log->logResponse(response, U_CONSTANT_TO_PARAM(" from %v"), host_port.rep);
{
   U_TRACE(0, "ULog::logResponse(%V,%.*S,%u)", data.rep, fmt_size, format, fmt_size)

   U_INTERNAL_ASSERT(data)

   char buffer[8192];
   const char* ptr  = data.data();
   uint32_t len, sz = data.size(), u_printf_string_max_length_save = u_printf_string_max_length;

   U_INTERNAL_DUMP("u_printf_string_max_length = %d", u_printf_string_max_length)

   if (u_printf_string_max_length == -1)
      {
      u_printf_string_max_length = u_findEndHeader1(ptr, sz);

      if (u_printf_string_max_length < 2000) u_printf_string_max_length = 2000;
      }

   U_INTERNAL_DUMP("u_printf_string_max_length = %d UServer_Base::mod_name = %S", u_printf_string_max_length, UServer_Base::mod_name)

   len = u__snprintf(buffer, U_CONSTANT_SIZE(buffer), U_CONSTANT_TO_PARAM("%sreceived response (%u bytes) %#.*S"),
                     UServer_Base::mod_name[0][0] ? (const char*)UServer_Base::mod_name : "", sz, sz, ptr);

   U_INTERNAL_DUMP("len = %u", len)

   if (fmt_size)
      {
      va_list argp;
      va_start(argp, fmt_size);

      len += u__vsnprintf(buffer+len, U_CONSTANT_SIZE(buffer)-len, format, fmt_size, argp);

      va_end(argp);
      }

   write(buffer, len);

   u_printf_string_max_length = u_printf_string_max_length_save;
}

void ULog::logger(const char* ident, int priority, const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(1, "ULog::logger(%S,%d,%.*S,%u)", ident, priority, fmt_size, format, fmt_size)

#ifndef _MSWINDOWS_
   U_INTERNAL_ASSERT(U_Log_syslog(this))

   if (format == U_NULLPTR)
      {
      U_SYSCALL_VOID(openlog, "%S,%d,%d", ident, 0, 0);

      return;
      }

   uint32_t len;
   char buffer[4096];

   va_list argp;
   va_start(argp, fmt_size);

   len = u__vsnprintf(buffer, U_CONSTANT_SIZE(buffer), format, fmt_size, argp);

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

void ULog::closeLogInternal()
{
   U_TRACE_NO_PARAM(1, "ULog::closeLogInternal()")

   if (U_Log_start_stop_msg(this)) log(U_CONSTANT_TO_PARAM(U_FMT_START_STOP), "SHUTDOWN", sizeof(void*) * 8);

   if (U_Log_syslog(this))
      {
#  ifndef _MSWINDOWS_
      U_SYSCALL_VOID_NO_PARAM(closelog);
#  endif
      }
   else
      {
      U_INTERNAL_DUMP("log_file_sz = %u", log_file_sz)

      U_INTERNAL_ASSERT_POINTER(ptr_log_data)

      if (log_file_sz)
         {
         U_INTERNAL_ASSERT_MINOR(ptr_log_data->file_ptr, UFile::st_size)

      // msync();

#     ifdef USE_LIBZ
         checkForLogRotateDataToWrite(); // check for previous data to write
#     endif

         U_INTERNAL_ASSERT_EQUALS(ptr_log_data->gzip_len, 0)

                UFile::munmap();
         (void) UFile::ftruncate(ptr_log_data->file_ptr);
             // UFile::fsync();
         }

      UFile::close();
      }
}

void ULog::closeLog()
{
   U_TRACE_NO_PARAM(0, "ULog::closeLog()")

#ifdef DEBUG
   U_INTERNAL_DUMP("first = %p next = %p this = %p", first, next, this)

   ULog* item;

   for (ULog** ptr = &first; (item = *ptr); ptr = &(*ptr)->next)
      {
      if (item == this)
         {
         U_INTERNAL_DUMP("*ptr = %p item->next = %p", *ptr, item->next)

         *ptr = item->next; // remove it from its active list
#endif

         closeLogInternal();

#ifdef DEBUG
         break;
         }
      }

   U_INTERNAL_DUMP("first = %p next = %p this = %p", first, next, this)
#endif
}

#ifdef USE_LIBZ
void ULog::setLogRotate(const char* dir_log_gz)
{
   U_TRACE(0, "ULog::setLogRotate(%S)", dir_log_gz)

   U_INTERNAL_ASSERT_POINTER(ptr_log_data)
   U_INTERNAL_ASSERT_EQUALS(U_Log_syslog(this), false)
   U_INTERNAL_ASSERT_EQUALS(buf_path_compress, U_NULLPTR)

   char suffix[32];
   uint32_t len_suffix = u__snprintf(suffix, sizeof(suffix), U_CONSTANT_TO_PARAM(".%4D.gz"));

   U_NEW_STRING(buf_path_compress, UString(MAX_FILENAME_LEN));

   char* p = buf_path_compress->data();

   if (dir_log_gz == U_NULLPTR)
      {
#  ifndef U_COVERITY_FALSE_POSITIVE // Uninitialized pointer read (UNINIT)
      (void) UFile::setPathFromFile(*this, p, suffix, len_suffix);
#  endif

      buf_path_compress->size_adjust();

      index_path_compress = (buf_path_compress->size() - len_suffix + 1);
      }
   else
      {
      UString name = UFile::getName();
      uint32_t len = u__strlen(dir_log_gz, __PRETTY_FUNCTION__), sz = name.size();

      U_MEMCPY(p, dir_log_gz, len);

       p  += len;
      *p++ = '/';

      buf_path_compress->size_adjust(len + 1 + sz + len_suffix);

      U_MEMCPY(p, name.data(), sz);
               p +=            sz;
      U_MEMCPY(p, suffix, len_suffix);

      index_path_compress = buf_path_compress->distance(p) + 1;
      }
}

void ULog::setShared(log_data* ptr)
{
   U_TRACE(0, "ULog::setShared(%p)", ptr)

   U_INTERNAL_ASSERT_POINTER(ptr_log_data)
   U_INTERNAL_ASSERT_EQUALS(U_Log_syslog(this), false)

   log_gzip_sz = getSizeLogRotateData();

   if (ptr == U_NULLPTR)
      {
      char somename[256];

      // For portable use, a shared memory object should be identified by a name of the form /somename; that is,
      // a null-terminated string of up to NAME_MAX (i.e., 255) characters consisting of an initial slash,
      // followed by one or more characters, none of which are slashes

      UString basename = UFile::getName();

      (void) u__snprintf(somename, sizeof(somename), U_CONSTANT_TO_PARAM("/%v"), basename.rep);

      ptr = (log_data*) UFile::shm_open(somename, sizeof(log_data) + log_gzip_sz);
      }

   ptr->file_ptr  = ptr_log_data->file_ptr;
   ptr->file_page = ptr_log_data->file_page;

   U_FREE_TYPE(ptr_log_data, log_data);

   (ptr_log_data = ptr)->gzip_len = 0;

   _lock.init(&(ptr_log_data->lock_shared));

   U_INTERNAL_DUMP("ptr_log_data->file_ptr = %u UFile::st_size = %u log_gzip_sz = %u", ptr_log_data->file_ptr, UFile::st_size, log_gzip_sz)

   U_INTERNAL_ASSERT(ptr_log_data->file_ptr <= UFile::st_size)
}

void ULog::checkForLogRotateDataToWrite()
{
   U_TRACE_NO_PARAM(0, "ULog::checkForLogRotateDataToWrite()")

   uint32_t gzip_len = ptr_log_data->gzip_len;

   if (gzip_len) // there are previous data to write
      {
      ptr_log_data->gzip_len = 0;

      char* ptr = buf_path_compress->c_pointer(index_path_compress);

      ptr[u__snprintf(ptr, 17, U_CONSTANT_TO_PARAM("%4D"))] = '.';

      (void) UFile::writeTo(*buf_path_compress, (char*)ptr_log_data+sizeof(log_data), gzip_len, O_RDWR | O_EXCL, false);
      }
}
#endif

// DEBUG

#ifdef DEBUG
ULog* ULog::first;

void ULog::close()
{
   U_TRACE_NO_PARAM(0, "ULog::close()")

   ULog* next;
   ULog* item;

   if (first)
      {
      next = first;
             first = U_NULLPTR;

      do {
         item = next;
                next = item->next;

         U_INTERNAL_DUMP("item = %p next = %p", item, next)

         if (item->fd != -1) item->closeLogInternal();
         }
      while (next);
      }
}

# ifdef U_STDCPP_ENABLE
const char* ULog::dump(bool _reset) const
{
   UFile::dump(false);

   *UObjectIO::os << '\n'
                  << "prefix_len                " << prefix_len    << '\n'
                  << "log_file_sz               " << log_file_sz   << '\n'
                  << "log_gzip_sz               " << log_gzip_sz   << '\n'
                  << "_lock     (ULock          " << (void*)&_lock << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
# endif
#endif
