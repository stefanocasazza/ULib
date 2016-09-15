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
      char spinlock_shared[1];
      // --------------> maybe unnamed array of char for gzip compression...
   } log_data;

   static ULog* pthis;
   static log_date date;
   static const char* prefix;
   static uint32_t prefix_len;
   static struct iovec iov_vec[5];
   static log_date* ptr_shared_date;
#if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
   static pthread_rwlock_t* prwlock;
#endif

    ULog(const UString& path, uint32_t size, const char* dir_log_gz = 0);
   ~ULog();

   void reopen()
      {
      U_TRACE_NO_PARAM(0, "ULog::reopen()")

      U_INTERNAL_ASSERT_EQUALS(U_Log_syslog(this), false)

      if (log_file_sz == 0) UFile::reopen(O_RDWR | O_APPEND);
      }

   void msync() // flushes changes made to memory mapped log file back to disk
      {
      U_TRACE_NO_PARAM(0, "ULog::msync()")

      U_INTERNAL_ASSERT_EQUALS(U_Log_syslog(this), false)

      U_INTERNAL_ASSERT_POINTER(ptr_log_data)

      UFile::msync(UFile::map + ptr_log_data->file_ptr, UFile::map + ptr_log_data->file_page, MS_SYNC);

      ptr_log_data->file_page = ptr_log_data->file_ptr;
      }

   void closeLog();
   void setShared(log_data* ptr, uint32_t size, bool breference = true);

   void      init(const char* prefix, uint32_t prefix_len); // server
   void setPrefix(const char* prefix, uint32_t prefix_len); // client

   // manage shared log

   static bool isMemoryMapped()
      {
      U_TRACE_NO_PARAM(0, "ULog::isMemoryMapped()")

      U_INTERNAL_ASSERT_POINTER(pthis)

      if (pthis->log_file_sz == 0) U_RETURN(false);

      U_RETURN(true);
      }

   // write with prefix

   static void write(const char* msg, uint32_t len);
   static void log(const char* format, uint32_t fmt_size, ...);   // (buffer write == 8196)

   // write without prefix

   static void log(int _fd, const char* format, uint32_t fmt_size, ...);   // (buffer write == 8196)

   // logger

   static int getPriorityForLogger(const char* s) __pure; // decode a symbolic name to a numeric value

   static void logger(const char* ident, int priority, const char* format, uint32_t fmt_size, ...); // (buffer write == 4096)

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   ULock* lock;
   log_data* ptr_log_data;
   uint32_t log_file_sz, log_gzip_sz;
   unsigned char flag[4];
#ifdef USE_LIBZ
   UString*   buf_path_compress;
   uint32_t index_path_compress;
   void checkForLogRotateDataToWrite();
#endif

   static uint32_t log_data_sz;
   static long tv_sec_old_1, tv_sec_old_2, tv_sec_old_3;

   void write(const struct iovec* iov, int n);

#ifdef USE_LIBZ
   static UString getDirLogGz();
#endif

   static void close();
   static void startup();
   static void initDate();
   static void updateDate1();
   static void updateDate2();
   static void updateDate3();
   static void logResponse(const UString& data, const char* name,                                                                  const char* format, uint32_t fmt_size, ...);
   static void log(const struct iovec* iov,     const char* name, const char* type, int ncount, const char* msg, uint32_t msg_len, const char* format, uint32_t fmt_size, ...);

private:
   static int decode(const char* name, uint32_t len, bool bfacility) __pure U_NO_EXPORT;

   U_DISALLOW_COPY_AND_ASSIGN(ULog)

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
};

#endif
