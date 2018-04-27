// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    log.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_LOG_H
#define ULIB_LOG_H 1

#include <ulib/file.h>
#include <ulib/utility/lock.h>
#include <ulib/utility/string_ext.h>

class ULib;
class UHTTP;
class UHTTP2;
class Application;
class UTimeThread;
class UProxyPlugIn;
class UNoCatPlugIn;
class UServer_Base;
class UClient_Base;
class UHttpClient_Base;
class UClientImage_Base;

// Logging is the process of recording application actions and state to a secondary interface

#define U_Log_syslog(obj)         (obj)->ULog::flag[0]
#define U_Log_start_stop_msg(obj) (obj)->ULog::flag[1]

class U_EXPORT ULog : public UFile {
public:

   typedef struct log_date {
      char date1[17+1];             // 18/06/12 18:45:56
      char date2[26+1];             // 04/Jun/2012:18:18:37 +0200
      char date3[6+29+2+12+2+19+1]; // Date: Wed, 20 Jun 2012 11:43:17 GMT\r\nServer: ULib\r\nConnection: close\r\n
   } log_date;

   typedef struct log_data {
      uint32_t file_ptr;
      uint32_t file_page;
      uint32_t gzip_len;
      sem_t lock_shared;
      // --------------> maybe unnamed array of char for gzip compression...
   } log_data;

   static log_date date;
   static const char* prefix;
   static uint32_t prefix_len;
   static struct iovec iov_vec[5];
   static log_date* ptr_shared_date;
#if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
   static pthread_rwlock_t* prwlock;
#endif

   ULog(const UString& path, uint32_t size);

   ~ULog()
      {
      U_TRACE_DTOR(0, ULog)

#  ifdef USE_LIBZ
      if (buf_path_compress) U_DELETE(buf_path_compress)
#  endif

      if (UFile::isMapped()) UFile::munmap();
      }

   void reopen()
      {
      U_TRACE_NO_PARAM(0, "ULog::reopen()")

      U_INTERNAL_ASSERT_EQUALS(U_Log_syslog(this), false)

      if (log_file_sz == 0) UFile::reopen(O_RDWR | O_APPEND);
      }

   void msync() // flushes changes made to memory mapped log file back to disk
      {
      U_TRACE_NO_PARAM(0, "ULog::msync()")

      U_ASSERT(isMemoryMapped())
      U_INTERNAL_ASSERT_EQUALS(U_Log_syslog(this), false)

      UFile::msync(UFile::map + ptr_log_data->file_ptr, UFile::map + ptr_log_data->file_page, MS_SYNC);

      ptr_log_data->file_page = ptr_log_data->file_ptr;
      }

   void init(const char* _prefix, uint32_t _prefix_len) // server
      {
      U_TRACE(0, "ULog::init(%.*S,%u)", _prefix_len, _prefix, _prefix_len)

      U_INTERNAL_ASSERT_EQUALS(U_Log_syslog(this), false)

      prefix                     = _prefix;
      prefix_len                 = _prefix_len;
      U_Log_start_stop_msg(this) = true;

      startup();
      }

   void setPrefix(const char* _prefix, uint32_t _prefix_len) // client
      {
      U_TRACE(0, "ULog::setPrefix(%.*S,%u)", _prefix_len, _prefix, _prefix_len)

      if (U_Log_syslog(this) == false)
         {
         prefix                     = _prefix;
         prefix_len                 = _prefix_len;
         U_Log_start_stop_msg(this) = true;

         startup();
         }
      }

   void closeLog();

   // manage shared log

   bool isMemoryMapped()
      {
      U_TRACE_NO_PARAM(0, "ULog::isMemoryMapped()")

      if (log_file_sz == 0) U_RETURN(false);

      U_RETURN(true);
      }

   uint32_t getSizeLogRotateData()
      {
      U_TRACE_NO_PARAM(0, "ULog::getSizeLogRotateData()")

      U_INTERNAL_DUMP("log_file_sz = %u", log_file_sz)

      U_ASSERT(isMemoryMapped())

      uint32_t x = log_file_sz + (log_file_sz / 10) + 12U; // The zlib documentation states that destination buffer size must be at least 0.1% larger than avail_in plus 12 bytes

      U_RETURN(x);
      }

#ifdef USE_LIBZ
   void setShared(log_data* ptr);
   void checkForLogRotateDataToWrite();
   void setLogRotate(const char* dir_log_gz = U_NULLPTR);
#endif

   UString getDirLogGz()
      {
      U_TRACE_NO_PARAM(0, "ULog::getDirLogGz()")

      U_ASSERT(isMemoryMapped())

      UString result;

#  ifdef USE_LIBZ
      U_INTERNAL_ASSERT_POINTER(buf_path_compress)

      result = UStringExt::dirname(*buf_path_compress);
#  endif

      U_RETURN_STRING(result);
      }

   // LOCK

   void   lock() { if (_lock.sem) _lock.lock(); }
   void unlock() { if (_lock.sem) _lock.unlock(); }

   // write with prefix

   void write(const char* msg, uint32_t len);

   void log(const char* fmt, uint32_t fmt_size, ...)
      {
      U_TRACE(0, "ULog::log(%.*S,%u)", fmt_size, fmt, fmt_size)

      uint32_t len;
      char buffer[8196];

      va_list argp;
      va_start(argp, fmt_size);

      len = u__vsnprintf(buffer, U_CONSTANT_SIZE(buffer), fmt, fmt_size, argp);

      va_end(argp);

      write(buffer, len);
      }

   // write without prefix

   static void log(int lfd, const char* format, uint32_t fmt_size, ...); // (buffer write == 8196)

   // logger

   void logger(const char* ident, int priority, const char* format, uint32_t fmt_size, ...); // (buffer write == 4096)

   static int getPriorityForLogger(const char* s) __pure; // decode a symbolic name to a numeric value

#ifdef DEBUG
   static ULog* first; // active list 
# ifdef U_STDCPP_ENABLE
   const char* dump(bool reset) const;
# endif
#endif

protected:
   ULock _lock;
   log_data* ptr_log_data;
   uint32_t log_file_sz,
            log_gzip_sz;
   unsigned char flag[4];

#ifdef DEBUG
   ULog* next;
   static void close();
#endif

   void startup();
   void closeLogInternal();
   void write(const struct iovec* iov, int n);
   void logResponse(const UString& data,  const char* format, uint32_t fmt_size, ...);
   void log(const struct iovec* iov, const char* type, int ncount, const char* msg, uint32_t msg_len, const char* format, uint32_t fmt_size, ...);

#ifdef USE_LIBZ
   UString*   buf_path_compress;
   uint32_t index_path_compress;
#endif

   static long tv_sec_old_1, tv_sec_old_2, tv_sec_old_3;

   static void initDate();
   static void updateDate1();
   static void updateDate2();
   static void updateDate3(char* ptr_date);

private:
   static int decode(const char* name, uint32_t len, bool bfacility) __pure U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(ULog)

   friend class ULib;
   friend class UHTTP;
   friend class UHTTP2;
   friend class Application;
   friend class UTimeThread;
   friend class UProxyPlugIn;
   friend class UNoCatPlugIn;
   friend class UServer_Base;
   friend class UClient_Base;
   friend class UHttpClient_Base;
   friend class UClientImage_Base;

   // friend int main(int, char**, char**);
};

#endif
