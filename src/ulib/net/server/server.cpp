// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    server.cpp
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/url.h>
#include <ulib/timer.h>
#include <ulib/db/rdb.h>
#include <ulib/net/udpsocket.h>
#include <ulib/utility/escape.h>
#include <ulib/orm/orm_driver.h>
#include <ulib/dynamic/dynamic.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/server.h>

#ifdef _MSWINDOWS_
#  include <ws2tcpip.h>
#else
#  include <pwd.h>
#  include <ulib/net/unixsocket.h>
#  ifdef HAVE_LIBNUMA
#     include <numa.h>
#  endif
#endif

#ifdef U_STATIC_HANDLER_RPC
#  include <ulib/net/server/plugin/mod_rpc.h>
#endif
#ifdef U_STATIC_HANDLER_SHIB
#  include <ulib/net/server/plugin/mod_shib.h>
#endif
#ifdef U_STATIC_HANDLER_ECHO
#  include <ulib/net/server/plugin/mod_echo.h>
#endif
#ifdef U_STATIC_HANDLER_STREAM
#  include <ulib/net/server/plugin/mod_stream.h>
#endif
#ifdef U_STATIC_HANDLER_NOCAT
#  include <ulib/net/server/plugin/mod_nocat.h>
#endif
#ifdef U_STATIC_HANDLER_SOCKET
#  include <ulib/net/server/plugin/mod_socket.h>
#endif
#ifdef U_STATIC_HANDLER_SCGI
#  include <ulib/net/server/plugin/mod_scgi.h>
#endif
#ifdef U_STATIC_HANDLER_FCGI
#  include <ulib/net/server/plugin/mod_fcgi.h>
#endif
#ifdef U_STATIC_HANDLER_GEOIP
#  include <ulib/net/server/plugin/mod_geoip.h>
#endif
#ifdef U_STATIC_HANDLER_PROXY
#  include <ulib/net/server/plugin/mod_proxy.h>
#endif
#ifdef U_STATIC_HANDLER_SOAP
#  include <ulib/net/server/plugin/mod_soap.h>
#endif
#ifdef U_STATIC_HANDLER_SSI
#  include <ulib/net/server/plugin/mod_ssi.h>
#endif
#ifdef U_STATIC_HANDLER_TSA
#  include <ulib/net/server/plugin/mod_tsa.h>
#endif
#ifdef U_STATIC_HANDLER_HTTP
#  include <ulib/net/server/plugin/mod_http.h>
#endif

int           UServer_Base::rkids;
int           UServer_Base::old_pid;
int           UServer_Base::timeoutMS;
int           UServer_Base::verify_mode;
int           UServer_Base::socket_flags;
int           UServer_Base::iAddressType;
int           UServer_Base::tcp_linger_set = -2;
int           UServer_Base::preforked_num_kids;
bool          UServer_Base::bssl;
bool          UServer_Base::bipc;
bool          UServer_Base::binsert;
bool          UServer_Base::flag_loop;
bool          UServer_Base::flag_sigterm;
bool          UServer_Base::public_address;
bool          UServer_Base::monitoring_process;
bool          UServer_Base::set_tcp_keep_alive;
bool          UServer_Base::set_realtime_priority;
bool          UServer_Base::update_date;
bool          UServer_Base::update_date1;
bool          UServer_Base::update_date2;
bool          UServer_Base::update_date3;
char          UServer_Base::mod_name[2][16];
ULog*         UServer_Base::log;
ULog*         UServer_Base::apache_like_log;
pid_t         UServer_Base::pid;
char*         UServer_Base::client_address;
ULock*        UServer_Base::lock_user1;
ULock*        UServer_Base::lock_user2;
time_t        UServer_Base::last_event;
uint32_t      UServer_Base::map_size;
uint32_t      UServer_Base::vplugin_size;
uint32_t      UServer_Base::nClientIndex;
uint32_t      UServer_Base::shared_data_add;
uint32_t      UServer_Base::client_address_len;
uint32_t      UServer_Base::document_root_size;
uint32_t      UServer_Base::num_client_threshold;
uint32_t      UServer_Base::min_size_for_sendfile;
uint32_t      UServer_Base::num_client_for_parallelization;
sigset_t      UServer_Base::mask;
UString*      UServer_Base::host;
UString*      UServer_Base::server;
UString*      UServer_Base::as_user;
UString*      UServer_Base::dh_file;
UString*      UServer_Base::ca_file;
UString*      UServer_Base::ca_path;
UString*      UServer_Base::key_file;
UString*      UServer_Base::password;
UString*      UServer_Base::cert_file;
UString*      UServer_Base::name_sock;
UString*      UServer_Base::IP_address;
UString*      UServer_Base::cenvironment;
UString*      UServer_Base::senvironment;
UString*      UServer_Base::document_root;
UString*      UServer_Base::str_preforked_num_kids;
USocket*      UServer_Base::socket;
USocket*      UServer_Base::csocket;
UProcess*     UServer_Base::proc;
UEventFd*     UServer_Base::handler_other;
UEventFd*     UServer_Base::handler_inotify;
UEventTime*   UServer_Base::ptime;
const char*   UServer_Base::document_root_ptr;
unsigned int  UServer_Base::port;
UFileConfig*  UServer_Base::cfg;
UServer_Base* UServer_Base::pthis;

UVector<UString>*                 UServer_Base::vplugin_name;
UVector<UString>*                 UServer_Base::vplugin_name_static;
UClientImage_Base*                UServer_Base::pClientImage;
UClientImage_Base*                UServer_Base::vClientImage;
UClientImage_Base*                UServer_Base::eClientImage;
UVector<UServerPlugIn*>*          UServer_Base::vplugin;
UServer_Base::shared_data*        UServer_Base::ptr_shared_data;
UVector<UServer_Base::file_LOG*>* UServer_Base::vlog;

#ifdef U_WELCOME_SUPPORT
UString* UServer_Base::msg_welcome;
#endif
#ifdef U_ACL_SUPPORT
UString*            UServer_Base::allow_IP;
UVector<UIPAllow*>* UServer_Base::vallow_IP;
#endif
#ifdef U_RFC1918_SUPPORT
bool                UServer_Base::enable_rfc1918_filter;
UString*            UServer_Base::allow_IP_prv;
UVector<UIPAllow*>* UServer_Base::vallow_IP_prv;
#endif

#ifdef DEBUG
#  ifdef USE_LIBEVENT
#     define U_WHICH "libevent" 
#  elif defined(HAVE_EPOLL_WAIT)
#     define U_WHICH "epoll" 
#  else
#     define U_WHICH "select" 
#  endif

uint32_t    UServer_Base::nread;
uint32_t    UServer_Base::max_depth;
uint32_t    UServer_Base::nread_again;
uint64_t    UServer_Base::stats_bytes;
uint32_t    UServer_Base::stats_connections;
uint32_t    UServer_Base::stats_simultaneous;
uint32_t    UServer_Base::wakeup_for_nothing;
UEventTime* UServer_Base::pstat;

UString UServer_Base::getStats()
{
   U_TRACE_NO_PARAM(0, "UTimeStat::getStats()")

   UString x(U_CAPACITY);

   x.snprintf("%3u connections (%5.2f/sec), %3u max simultaneous, %4u %s (%5.2f/sec) - %v/sec", UServer_Base::stats_connections,
               (float) UServer_Base::stats_connections / U_ONE_HOUR_IN_SECOND, UServer_Base::stats_simultaneous, UNotifier::nwatches, U_WHICH,
               (float) UNotifier::nwatches / U_ONE_HOUR_IN_SECOND, UStringExt::printSize(UServer_Base::stats_bytes).rep);

   U_RETURN_STRING(x);
}

class U_NO_EXPORT UTimeStat : public UEventTime {
public:

   // COSTRUTTORI

   UTimeStat() : UEventTime(U_ONE_HOUR_IN_SECOND, 0L)
      {
      U_TRACE_REGISTER_OBJECT(0, UTimeStat, "", 0)
      }

   virtual ~UTimeStat() U_DECL_FINAL
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTimeStat)
      }

   // define method VIRTUAL of class UEventTime

   virtual int handlerTime() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "UTimeStat::handlerTime()")

      if (UServer_Base::stats_bytes)
         {
         U_MESSAGE("%9D (pid %P): %v", UServer_Base::getStats().rep);

         UServer_Base::stats_bytes = 0;
         }

      UNotifier::nwatches              =
      UServer_Base::stats_connections  =
      UServer_Base::stats_simultaneous = 0;

      // ---------------
      // return value:
      // ---------------
      // -1 - normal
      //  0 - monitoring
      // ---------------

      U_RETURN(0);
      }

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool _reset) const { return UEventTime::dump(_reset); }
#endif

private:
   UTimeStat(const UTimeStat&) : UEventTime() {}
   UTimeStat& operator=(const UTimeStat&)     { return *this; }
};
#endif

class U_NO_EXPORT UTimeoutConnection : public UEventTime {
public:

   // COSTRUTTORI

   UTimeoutConnection() : UEventTime(UServer_Base::timeoutMS / 1000L, 0L)
      {
      U_TRACE_REGISTER_OBJECT(0, UTimeoutConnection, "", 0)
      }

   virtual ~UTimeoutConnection() U_DECL_FINAL
      {
      U_TRACE_UNREGISTER_OBJECT(0, UTimeoutConnection)
      }

   // define method VIRTUAL of class UEventTime

   virtual int handlerTime() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "UTimeoutConnection::handlerTime()")

      U_INTERNAL_DUMP("UNotifier::num_connection = %u UNotifier::min_connection = %u", UNotifier::num_connection, UNotifier::min_connection)

      U_INTERNAL_ASSERT_POINTER(UServer_Base::ptr_shared_data)
      U_INTERNAL_ASSERT_MAJOR(UNotifier::num_connection, UNotifier::min_connection)

      U_gettimeofday; // NB: optimization if it is enough a time resolution of one second...

      // there are idle connection... (timeout)

#  ifdef U_LOG_ENABLE
      if (UServer_Base::isLog())
         {
         UServer_Base::called_from_handlerTime = true;

#     ifdef DEBUG
         long delta = (u_now->tv_sec - UServer_Base::last_event) - UTimeVal::tv_sec;

         if (delta >=  1 ||
             delta <= -1)
            {
            U_SRV_LOG("handlerTime: server delta timeout exceed 1 sec: diff %ld sec", delta);
            }
#     endif
         }
#  endif

      UServer_Base::last_event = u_now->tv_sec;

      UNotifier::callForAllEntryDynamic(UServer_Base::handlerTimeoutConnection);

#  ifdef U_LOG_ENABLE
      if (U_CNT_PARALLELIZATION)
#  endif
      UServer_Base::removeZombies();

      // ---------------
      // return value:
      // ---------------
      // -1 - normal
      //  0 - monitoring
      // ---------------

      U_RETURN(-1);
      }

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool _reset) const { return UEventTime::dump(_reset); }
#endif

private:
   UTimeoutConnection(const UTimeoutConnection&) : UEventTime() {}
   UTimeoutConnection& operator=(const UTimeoutConnection&)     { return *this; }
};

/**
 * The throttle data lets you set maximum byte rates on URLs or URL groups. You can optionally set a minimum rate too.
 * The format of the throttle data is very simple, should consist of a pattern, whitespace, and a number. The pattern
 * is a simple shell-style filename pattern, using ?*, or multiple such patterns separated by |. The numbers are byte
 * rates, specified in units of kbytes per second. If you want to set a minimum rate as well, use number-number.
 *
 * For example assuming we have a bandwith of 150 kB/s
 *
 * a) *           20-100  => limit total web usage to 2/3, but never go below 20 kB/s
 * b) *.jpg|*.gif     50  => limit images to 1/3 of our bandwith
 * c) *.mpg           20  => and movies to even less
 *
 * Throttling is implemented by checking each incoming URL filename against all of the patterns in the throttle data.
 * The server accumulates statistics on how much bandwidth each pattern has accounted for recently (via a rolling average).
 * If a URL matches a pattern that has been exceeding its specified limit, then the data returned is actually slowed down,
 * with pauses between each block. If that's not possible or if the bandwidth has gotten way larger than the limit, then
 * the server returns a special code
 */

#ifdef U_THROTTLING_SUPPORT
#  define U_THROTTLE_TIME 2 // Time between updates of the throttle table's rolling averages

class U_NO_EXPORT UThrottling : public UDataStorage {
public:

   uint64_t bytes_since_avg;
   uint32_t krate, min_limit, max_limit, num_sending;

   // COSTRUTTORE

   UThrottling()
      {
      U_TRACE_REGISTER_OBJECT(0, UThrottling, "", 0)

      (void) memset(&bytes_since_avg, 0, sizeof(uint32_t) * 6);
      }

   ~UThrottling()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UThrottling)

      if (UServer_Base::db_throttling)
         {
                UServer_Base::db_throttling->close();
         delete UServer_Base::db_throttling;
         }
      }

   // define method VIRTUAL of class UDataStorage

   virtual char* toBuffer()
      {
      U_TRACE_NO_PARAM(0, "UThrottling::toBuffer()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      U_MEMCPY(u_buffer, &bytes_since_avg, u_buffer_len = sizeof(uint32_t) * 6);

      U_INTERNAL_ASSERT_MINOR(u_buffer_len, U_BUFFER_SIZE)

      U_RETURN(u_buffer);
      }

   virtual void fromData(const char* ptr, uint32_t len)
      {
      U_TRACE(0, "UThrottling::fromData(%.*S,%u)", len, ptr, len)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_POINTER(ptr)
      U_INTERNAL_ASSERT(len >= sizeof(uint32_t) * 6)

      U_MEMCPY(&bytes_since_avg, ptr, sizeof(uint32_t) * 6);
      }

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool breset) const { return ""; }
#endif

private:
   UThrottling(const UThrottling&) : UDataStorage() {}
   UThrottling& operator=(const UThrottling&)       { return *this; }
};

class U_NO_EXPORT UBandWidthThrottling : public UEventTime {
public:

   // COSTRUTTORI

   UBandWidthThrottling() : UEventTime(U_THROTTLE_TIME, 0L)
      {
      U_TRACE_REGISTER_OBJECT(0, UBandWidthThrottling, "", 0)
      }

   virtual ~UBandWidthThrottling() U_DECL_FINAL
      {
      U_TRACE_UNREGISTER_OBJECT(0, UBandWidthThrottling)
      }

   // SERVICES

   static int clearThrottling(UStringRep* key, UStringRep* data)
      {
      U_TRACE(0, "UBandWidthThrottling::clearThrottling(%p,%p)", key, data)

      if (key)
         {
         if (UServices::dosMatchWithOR(U_STRING_TO_PARAM(UServer_Base::pClientImage->uri), U_STRING_TO_PARAM(*key))) U_RETURN(4); // NB: call us later (after set record value from db)...

         U_RETURN(1);
         }

      U_INTERNAL_DUMP("krate = %u min_limit = %u max_limit = %u num_sending = %u bytes_since_avg = %llu", UServer_Base::throttling_rec->krate, UServer_Base::throttling_rec->min_limit,
                        UServer_Base::throttling_rec->max_limit, UServer_Base::throttling_rec->num_sending, UServer_Base::throttling_rec->bytes_since_avg)

      U_INTERNAL_ASSERT_MAJOR(UServer_Base::throttling_rec->num_sending, 0)

      UServer_Base::throttling_rec->num_sending--;

      (void) UServer_Base::db_throttling->putDataStorage();

      // NB: db can have different pattern matching the same url...

      U_RETURN(1);
      }

   static int checkThrottling(UStringRep* key, UStringRep* data)
      {
      U_TRACE(0, "UBandWidthThrottling::checkThrottling(%p,%p)", key, data)

      if (key)
         {
         if (UServices::dosMatchWithOR(U_HTTP_URI_TO_PARAM, U_STRING_TO_PARAM(*key))) U_RETURN(4); // NB: call us later (after set record value from db)...

         U_RETURN(1);
         }

      U_INTERNAL_DUMP("krate = %u min_limit = %u max_limit = %u num_sending = %u bytes_since_avg = %llu", UServer_Base::throttling_rec->krate, UServer_Base::throttling_rec->min_limit,
                        UServer_Base::throttling_rec->max_limit, UServer_Base::throttling_rec->num_sending, UServer_Base::throttling_rec->bytes_since_avg)

      if ( UServer_Base::throttling_rec->krate &&
          (UServer_Base::throttling_rec->krate > (UServer_Base::throttling_rec->max_limit * 2) || // if we're way over the limit, don't even start...
           UServer_Base::throttling_rec->krate <  UServer_Base::throttling_rec->min_limit))       // ...also don't start if we're under the minimum
         {
         UServer_Base::throttling_chk = false;

         U_RETURN(0); // stop
         }
      
      UServer_Base::throttling_rec->num_sending++;

      (void) UServer_Base::db_throttling->putDataStorage();

      uint32_t l = UServer_Base::throttling_rec->max_limit / UServer_Base::throttling_rec->num_sending;

      UServer_Base::pClientImage->max_limit = (UServer_Base::pClientImage->max_limit == U_NOT_FOUND ?       l
                                                                                                    : U_min(l, UServer_Base::pClientImage->max_limit));
      l = UServer_Base::throttling_rec->min_limit;

      UServer_Base::pClientImage->min_limit = (UServer_Base::pClientImage->min_limit == U_NOT_FOUND ?       l
                                                                                                    : U_max(l, UServer_Base::pClientImage->min_limit));

      U_INTERNAL_DUMP("UServer_Base::pClientImage->min_limit = %u UServer_Base::pClientImage->max_limit = %u",
                       UServer_Base::pClientImage->min_limit,     UServer_Base::pClientImage->max_limit)

      // NB: db can have different pattern matching the same url...

      U_RETURN(1);
      }

   static int updateThrottling(UStringRep* key, UStringRep* data)
      {
      U_TRACE(0, "UBandWidthThrottling::updateThrottling(%p,%p)", key, data)

      if (key) U_RETURN(4); // NB: call us later (after set record value from db)...

      U_INTERNAL_DUMP("krate = %u min_limit = %u max_limit = %u num_sending = %u bytes_since_avg = %llu", UServer_Base::throttling_rec->krate, UServer_Base::throttling_rec->min_limit,
                        UServer_Base::throttling_rec->max_limit, UServer_Base::throttling_rec->num_sending, UServer_Base::throttling_rec->bytes_since_avg)

      UServer_Base::throttling_rec->krate = ( 2 * UServer_Base::throttling_rec->krate + UServer_Base::throttling_rec->bytes_since_avg / (U_THROTTLE_TIME * 1024ULL)) / 3;
                                                                                        UServer_Base::throttling_rec->bytes_since_avg = 0;

      U_INTERNAL_DUMP("krate = %u", UServer_Base::throttling_rec->krate)

      (void) UServer_Base::db_throttling->putDataStorage();

#  ifdef U_LOG_ENABLE
      if (UServer_Base::isLog())
         {
         if (UServer_Base::throttling_rec->num_sending)
            {
            if (UServer_Base::throttling_rec->krate > UServer_Base::throttling_rec->max_limit)
               {
               ULog::log("throttle %V: krate %u %sexceeding limit %u; %u sending", UServer_Base::db_throttling->getKeyID().rep, UServer_Base::throttling_rec->krate,
                           (UServer_Base::throttling_rec->krate > UServer_Base::throttling_rec->max_limit * 2  ? "greatly " : ""),
                            UServer_Base::throttling_rec->max_limit, UServer_Base::throttling_rec->num_sending);
               }

            if (UServer_Base::throttling_rec->krate < UServer_Base::throttling_rec->min_limit)
               {
               ULog::log("throttle %V: krate %u lower than minimum %u; %u sending", UServer_Base::db_throttling->getKeyID().rep, UServer_Base::throttling_rec->krate,
                            UServer_Base::throttling_rec->min_limit, UServer_Base::throttling_rec->num_sending);
               }
            }
         }
#  endif

      U_RETURN(1);
      }

   static int updateSending(UStringRep* key, UStringRep* data)
      {
      U_TRACE(0, "UBandWidthThrottling::updateSending(%p,%p)", key, data)

      if (key)
         {
         if (UServices::dosMatchWithOR(U_HTTP_URI_TO_PARAM, U_STRING_TO_PARAM(*key))) U_RETURN(4); // NB: call us later (after set record value from db)...

         U_RETURN(1);
         }

      U_INTERNAL_DUMP("krate = %u min_limit = %u max_limit = %u num_sending = %u bytes_since_avg = %llu", UServer_Base::throttling_rec->krate, UServer_Base::throttling_rec->min_limit,
                        UServer_Base::throttling_rec->max_limit, UServer_Base::throttling_rec->num_sending, UServer_Base::throttling_rec->bytes_since_avg)

      UServer_Base::throttling_rec->bytes_since_avg += UServer_Base::pClientImage->bytes_sent;

      (void) UServer_Base::db_throttling->putDataStorage();

      // NB: db can have different pattern matching the same url...

      U_RETURN(1);
      }

   static int updateSendingRate(UStringRep* key, UStringRep* data)
      {
      U_TRACE(0, "UBandWidthThrottling::updateSendingRate(%p,%p)", key, data)

      if (key)
         {
         if (UServices::dosMatchWithOR(UServer_Base::pClientImage->uri, U_STRING_TO_PARAM(*key))) U_RETURN(4); // NB: call us later (after set record value from db)...

         U_RETURN(1);
         }

      U_INTERNAL_DUMP("krate = %u min_limit = %u max_limit = %u num_sending = %u bytes_since_avg = %llu", UServer_Base::throttling_rec->krate, UServer_Base::throttling_rec->min_limit,
                        UServer_Base::throttling_rec->max_limit, UServer_Base::throttling_rec->num_sending, UServer_Base::throttling_rec->bytes_since_avg)

      uint32_t l = UServer_Base::throttling_rec->max_limit / UServer_Base::throttling_rec->num_sending;

      UServer_Base::pClientImage->max_limit = (UServer_Base::pClientImage->max_limit == U_NOT_FOUND ?      l
                                                                                                    : U_min(l, UServer_Base::pClientImage->max_limit));

      U_RETURN(1);
      }

   static bool updateSendingRate(void* cimg)
      {
      U_TRACE(0, "UBandWidthThrottling::updateSendingRate(%p)", cimg)

      U_INTERNAL_ASSERT_POINTER(cimg)

      U_INTERNAL_DUMP("pthis = %p handler_other = %p handler_inotify = %p ", UServer_Base::pthis, UServer_Base::handler_other, UServer_Base::handler_inotify)

      if (cimg == UServer_Base::pthis         ||
          cimg == UServer_Base::handler_other ||
          cimg == UServer_Base::handler_inotify)
         {
         U_RETURN(false);
         }

      U_INTERNAL_ASSERT(((UClientImage_Base*)cimg)->socket->isOpen())

      UServer_Base::pClientImage = ((UClientImage_Base*)cimg);

      U_INTERNAL_DUMP("UServer_Base::pClientImage->min_limit = %u UServer_Base::pClientImage->max_limit = %u", UServer_Base::pClientImage->min_limit, UServer_Base::pClientImage->max_limit)

      UServer_Base::pClientImage->max_limit = U_NOT_FOUND;

      UServer_Base::db_throttling->callForAllEntry(updateSendingRate);

      U_RETURN(false);
      }

   // define method VIRTUAL of class UEventTime

   virtual int handlerTime() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "UBandWidthThrottling::handlerTime()")

      if (UServer_Base::db_throttling)
         {
         // Update the average sending rate for each throttle. This is only used when new connections start up

         UServer_Base::db_throttling->callForAllEntry(updateThrottling);

         // Now update the sending rate on all the currently-sending connections, redistributing it evenly

         U_INTERNAL_DUMP("UNotifier::num_connection = %d", UNotifier::num_connection)

         if (UNotifier::num_connection > UNotifier::min_connection) UNotifier::callForAllEntryDynamic(updateSendingRate);
         }

      // ---------------
      // return value:
      // ---------------
      // -1 - normal
      //  0 - monitoring
      // ---------------

      U_RETURN(0);
      }

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool _reset) const { return UEventTime::dump(_reset); }
#endif

private:
   UBandWidthThrottling(const UBandWidthThrottling&) : UEventTime() {}
   UBandWidthThrottling& operator=(const UBandWidthThrottling&)     { return *this; }
};

class U_NO_EXPORT UClientThrottling : public UEventTime {
public:

   // COSTRUTTORI

   UClientThrottling(UClientImage_Base* _pClientImage, long sec, long micro_sec) : UEventTime(sec, micro_sec)
      {
      U_TRACE_REGISTER_OBJECT(0, UClientThrottling, "%p,%ld,%ld", _pClientImage, sec, micro_sec)

      UNotifier::suspend(pClientImage = _pClientImage);
      }

   virtual ~UClientThrottling() U_DECL_FINAL
      {
      U_TRACE_UNREGISTER_OBJECT(0, UClientThrottling)
      }

   // define method VIRTUAL of class UEventTime

   virtual int handlerTime() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "UClientThrottling::handlerTime()")

      UNotifier::resume(pClientImage);

      delete this;

      // ---------------
      // return value:
      // ---------------
      // -1 - normal
      //  0 - monitoring
      // ---------------

      U_RETURN(-1);
      }

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool _reset) const { return UEventTime::dump(_reset); }
#endif

protected:
   UClientImage_Base* pClientImage;

private:
   UClientThrottling(const UClientThrottling&) : UEventTime() {}
   UClientThrottling& operator=(const UClientThrottling&)     { return *this; }
};

bool                              UServer_Base::throttling_chk;
UEventTime*                       UServer_Base::throttling_time;
UThrottling*                      UServer_Base::throttling_rec;
URDBObjectHandler<UDataStorage*>* UServer_Base::db_throttling;

void UServer_Base::initThrottlingClient()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::initThrottlingClient()")

   if (db_throttling)
      {
      pClientImage->uri.clear();

      pClientImage->min_limit  =
      pClientImage->max_limit  = U_NOT_FOUND;
      pClientImage->bytes_sent =
      pClientImage->started_at = 0;
      }
}

void UServer_Base::initThrottlingServer(const UString& x)
{
   U_TRACE(0, "UServer_Base::initThrottlingServer(%V)", x.rep)

   U_INTERNAL_ASSERT(x)
   U_INTERNAL_ASSERT_EQUALS(db_throttling, 0)

   if (bssl == false) // NB: we can't use throttling with SSL...
      {
      db_throttling = U_NEW(URDBObjectHandler<UDataStorage*>(U_STRING_FROM_CONSTANT("../db/BandWidthThrottling"), -1, (throttling_rec = U_NEW(UThrottling))));

      if (isPreForked()) db_throttling->setShared(U_LOCK_THROTTLING, U_SPINLOCK_THROTTLING);

      bool result = db_throttling->open(32 * 1024, false, true); // NB: we don't want truncate (we have only the journal)...

      U_SRV_LOG("%sdb initialization of BandWidthThrottling %s: size(%u)", (result ? "" : "WARNING: "), (result ? "success" : "failed"), db_throttling->size());

      if (result == false)
         {
         delete db_throttling;
                db_throttling = 0;
         }
      else
         {
         char* ptr;
         UVector<UString> vec(x);
         UString pattern, number;

         min_size_for_sendfile = 4096; // 4k

         db_throttling->reset(); // Initialize the db to contain no entries

         for (int32_t i = 0, n = vec.size(); i < n; i += 2)
            {
            pattern = vec[i];
             number = vec[i+1];

                                                                                      throttling_rec->max_limit = strtol(number.data(), &ptr, 10);
            if (ptr[0] == '-') throttling_rec->min_limit = throttling_rec->max_limit, throttling_rec->max_limit = strtol(        ptr+1,    0, 10);

            (void) db_throttling->insertDataStorage(pattern, RDB_INSERT);
            }

#     ifdef DEBUG
         db_throttling->resetKeyID(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#     endif
         }
      }
}

void UServer_Base::clearThrottling()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::clearThrottling()")

   U_INTERNAL_ASSERT(pClientImage)
   U_INTERNAL_ASSERT(pClientImage->uri)
   U_INTERNAL_ASSERT_POINTER(db_throttling)

   U_INTERNAL_DUMP("pClientImage->uri = %V", pClientImage->uri.rep)

   db_throttling->callForAllEntry(UBandWidthThrottling::clearThrottling);
}

bool UServer_Base::checkThrottling()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::checkThrottling()")

   if (db_throttling)
      {
      throttling_chk = true;

      pClientImage->max_limit =
      pClientImage->min_limit = U_NOT_FOUND;

      db_throttling->callForAllEntry(UBandWidthThrottling::checkThrottling);

      if (throttling_chk == false) U_RETURN(false);

      (void) pClientImage->uri.replace(U_HTTP_URI_TO_PARAM);
      }

   U_RETURN(true);
}

bool UServer_Base::checkThrottlingBeforeSend(bool bwrite)
{
   U_TRACE(0, "UServer_Base::checkThrottlingBeforeSend(%b)", bwrite)

   if (db_throttling)
      {
      U_ASSERT(pClientImage->uri.equal(U_HTTP_URI_TO_PARAM))

      U_INTERNAL_DUMP("pClientImage->max_limit = %u pClientImage->bytes_sent = %llu",
                       pClientImage->max_limit,     pClientImage->bytes_sent)

      if (pClientImage->max_limit != U_NOT_FOUND)
         {
         U_gettimeofday; // NB: optimization if it is enough a time resolution of one second...

         if (bwrite == false)
            {
            U_INTERNAL_ASSERT_EQUALS(pClientImage->started_at, 0)
            U_ASSERT(pClientImage->uri.equal(U_HTTP_URI_TO_PARAM))

            U_INTERNAL_DUMP("pClientImage->bytes_sent = %llu UClientImage_Base::ncount = %u", pClientImage->bytes_sent, UClientImage_Base::ncount)

            pClientImage->started_at = u_now->tv_sec;

            pClientImage->setPendingSendfile();

            db_throttling->callForAllEntry(UBandWidthThrottling::updateSending);

            U_RETURN(false);
            }

         // check if we're sending too fast

         uint32_t elapsed     = u_now->tv_sec - pClientImage->started_at,
                  kbytes_sent = pClientImage->bytes_sent / 1024ULL,
                  krate       = (elapsed > 1 ? kbytes_sent / elapsed : kbytes_sent);

         U_INTERNAL_DUMP("krate = %u", krate)

         if (krate > pClientImage->max_limit)
            {
            // How long should we wait to get back on schedule? If less than a second (integer math rounding), use 1/2 second

            time_t coast = kbytes_sent / pClientImage->max_limit - elapsed;

            UTimer::insert(U_NEW(UClientThrottling(pClientImage, coast, coast ? 0 : U_SECOND / 2L))); // set up the wakeup timer

            U_RETURN(false);
            }
         }
      }

   U_RETURN(true);
}
#endif

#ifdef ENABLE_THREAD
#include <ulib/thread.h>

class UClientThread : public UThread {
public:

   UClientThread() : UThread(PTHREAD_CREATE_DETACHED) {}

   virtual void run() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "UClientThread::run()")

      U_INTERNAL_ASSERT_EQUALS(UServer_Base::ptime, 0)

      while (UServer_Base::flag_loop) UNotifier::waitForEvent();
      }
};

#  ifndef _MSWINDOWS_
class UTimeThread : public UThread {
public:

   UTimeThread() : UThread(PTHREAD_CREATE_DETACHED) {}

   virtual void run() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "UTimeThread::run()")

      struct timespec ts;
      u_timeval.tv_sec = u_now->tv_sec;

      U_SRV_LOG("UTimeThread optimization for time resolution of one second activated (tid %u)", u_gettid());

      while (UServer_Base::flag_loop)
         {
         ts.tv_sec  = 1L;
         ts.tv_nsec = 0L;

         (void) U_SYSCALL(nanosleep, "%p,%p", &ts, 0);

#     if defined(U_LOG_ENABLE) && defined(USE_LIBZ)
         if (UServer_Base::log)                         UServer_Base::log->checkForLogRotateDataToWrite();
         if (UServer_Base::apache_like_log) UServer_Base::apache_like_log->checkForLogRotateDataToWrite();
#     endif

         U_INTERNAL_DUMP("u_timeval.tv_sec = %ld u_now->tv_sec = %ld", u_timeval.tv_sec, u_now->tv_sec)

         (void) U_SYSCALL(gettimeofday, "%p,%p", u_now, 0);

         if (u_timeval.tv_sec != u_now->tv_sec)
            {
            u_timeval.tv_sec = u_now->tv_sec;

            if (UServer_Base::update_date)
               {
#           if defined(U_LOG_ENABLE) && defined(USE_LIBZ)
               (void) U_SYSCALL(pthread_rwlock_wrlock, "%p", ULog::prwlock);
#           endif

               if ((u_timeval.tv_sec % U_ONE_HOUR_IN_SECOND) != 0)
                  {
                  if (UServer_Base::update_date1) UTimeDate::updateTime(ULog::ptr_shared_date->date1 + 12);
                  if (UServer_Base::update_date2) UTimeDate::updateTime(ULog::ptr_shared_date->date2 + 15);
                  if (UServer_Base::update_date3) UTimeDate::updateTime(ULog::ptr_shared_date->date3 + 26);
                  }
               else
                  {
                  if (UServer_Base::update_date1) (void) u_strftime2(ULog::ptr_shared_date->date1,     17, "%d/%m/%y %T",     u_timeval.tv_sec + u_now_adjust);
                  if (UServer_Base::update_date2) (void) u_strftime2(ULog::ptr_shared_date->date2,   26-6, "%d/%b/%Y:%T",     u_timeval.tv_sec + u_now_adjust); // %z  don't change...
                  if (UServer_Base::update_date3) (void) u_strftime2(ULog::ptr_shared_date->date3+6, 29-4, "%a, %d %b %Y %T", u_timeval.tv_sec);                // GMT can't change...
                  }

#           if defined(U_LOG_ENABLE) && defined(USE_LIBZ)
               (void) U_SYSCALL(pthread_rwlock_unlock, "%p", ULog::prwlock);
#           endif
               }
            }
         }
      }
};

#  if defined(USE_LIBSSL) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
#  include <ulib/net/tcpsocket.h>
#  include <ulib/net/client/client.h>

class UOCSPStapling : public UThread {
public:

   UOCSPStapling() : UThread(PTHREAD_CREATE_DETACHED) {}

   virtual void run() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "UOCSPStapling::run()")

      struct timespec ts;
      bool result = false;

      U_SRV_LOG("SSL: OCSP Stapling thread activated (tid %u)", u_gettid());

      errno = 0;
      ts.tv_nsec = 0L;

      while (UServer_Base::flag_loop)
         {
         if (errno != EINTR)
            {
            result = USSLSocket::doStapling();

            U_SRV_LOG("SSL: OCSP request for stapling to %V has %s", USSLSocket::staple.url->rep, (result ? "success" : "FAILED"));
            }

         ts.tv_sec = (result ? U_min(USSLSocket::staple.valid - u_now->tv_sec, 3600L) : 300L);

         (void) U_SYSCALL(nanosleep, "%p,%p", &ts, 0);
         }
      }
};

ULock*         UServer_Base::lock_ocsp_staple;
UOCSPStapling* UServer_Base::pthread_ocsp;
#endif
#endif
#endif

#if defined(LINUX) || defined(__LINUX__) || defined(__linux__)
static int sysctl_somaxconn, tcp_abort_on_overflow, sysctl_max_syn_backlog, tcp_fin_timeout;
#endif

UServer_Base::UServer_Base(UFileConfig* pcfg)
{
   U_TRACE_REGISTER_OBJECT(0, UServer_Base, "%p", pcfg)

   U_INTERNAL_ASSERT_EQUALS(pthis, 0)
   U_INTERNAL_ASSERT_EQUALS(cenvironment, 0)
   U_INTERNAL_ASSERT_EQUALS(senvironment, 0)

   port  = 80;
   pthis = this;

   as_user       = U_NEW(UString);
   dh_file       = U_NEW(UString);
   cert_file     = U_NEW(UString);
   key_file      = U_NEW(UString);
   password      = U_NEW(UString);
   ca_file       = U_NEW(UString);
   ca_path       = U_NEW(UString);
   name_sock     = U_NEW(UString);
   IP_address    = U_NEW(UString);
   document_root = U_NEW(UString);

   vlog          = U_NEW(UVector<file_LOG*>);
   cenvironment  = U_NEW(UString(U_CAPACITY));
   senvironment  = U_NEW(UString(U_CAPACITY));

   u_init_ulib_hostname();
   u_init_ulib_username();

   u_init_security();

   if (u_start_time     == 0 &&
       u_setStartTime() == false)
      {
      U_WARNING("System date not updated: %#5D", u_now->tv_sec);
      }

   if (pcfg)
      {
      U_INTERNAL_ASSERT_EQUALS(cfg, 0)

      cfg = pcfg;

      cfg->load();

      if (cfg->empty() == false) loadConfigParam();
      }

#ifdef ENABLE_IPV6
   if (UClientImage_Base::bIPv6) iAddressType = AF_INET6;
   else
#endif
                                 iAddressType = AF_INET;
}

UServer_Base::~UServer_Base()
{
   U_TRACE_UNREGISTER_OBJECT(0, UServer_Base)

   U_INTERNAL_ASSERT_POINTER(socket)
   U_INTERNAL_ASSERT_POINTER(vplugin)

#ifdef ENABLE_THREAD
# if !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   if (preforked_num_kids == -1)
      {
      U_INTERNAL_ASSERT_POINTER(UNotifier::pthread)

      delete UNotifier::pthread;
      }
# endif

# ifndef _MSWINDOWS_
   if (u_pthread_time)
      {
      delete (UTimeThread*)u_pthread_time;

      (void) pthread_rwlock_destroy(ULog::prwlock);
      }

#  if defined(USE_LIBSSL) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
   if (bssl)
      {
      if (pthread_ocsp) delete pthread_ocsp;

      USSLSocket::cleanupStapling();

      if (UServer_Base::lock_ocsp_staple) delete UServer_Base::lock_ocsp_staple;
      }
#  endif
# endif
#endif

# ifdef DEBUG
   delete pstat;
#endif

   if (ptime) delete ptime;

   UTimer::clear();

   UClientImage_Base::clear();

   delete socket;
   delete vplugin_name;
   delete vplugin;

   UOrmDriver::clear();

   U_INTERNAL_ASSERT_POINTER(cenvironment)
   U_INTERNAL_ASSERT_POINTER(senvironment)

   delete cenvironment;
   delete senvironment;

   if (host) delete host;

#ifdef U_THROTTLING_SUPPORT
   if (throttling_rec)  delete throttling_rec;
   if (throttling_time) delete throttling_time;
#endif

#ifdef U_WELCOME_SUPPORT
   if (msg_welcome) delete msg_welcome;
#endif

#ifdef U_ACL_SUPPORT
   if (vallow_IP)
      {
      delete  allow_IP;
      delete vallow_IP;
      }
#endif

#ifdef U_RFC1918_SUPPORT
   if (vallow_IP_prv)
      {
      delete  allow_IP_prv;
      delete vallow_IP_prv;
      }
#endif

#if defined(LINUX) || defined(__LINUX__) || defined(__linux__)
   if (as_user->empty() &&
       isChild() == false)
      {
      u_need_root(false);

      (void) UFile::setSysParam("/proc/sys/net/ipv4/tcp_fin_timeout", tcp_fin_timeout, true);

      if (USocket::iBackLog > SOMAXCONN)
         {
         (void) UFile::setSysParam("/proc/sys/net/core/somaxconn",           sysctl_somaxconn,       true);
         (void) UFile::setSysParam("/proc/sys/net/ipv4/tcp_max_syn_backlog", sysctl_max_syn_backlog, true);
         }

      if (USocket::iBackLog == 1) (void) UFile::setSysParam("/proc/sys/net/ipv4/tcp_abort_on_overflow", tcp_abort_on_overflow, true);
      }
#endif

   if (proc)       delete proc;
   if (server)     delete server;
   if (lock_user1) delete lock_user1;
   if (lock_user2) delete lock_user2;

   UFile::munmap(ptr_shared_data, map_size);

   delete as_user;
   delete dh_file;
   delete cert_file;
   delete key_file;
   delete password;
   delete ca_file;
   delete ca_path;
   delete name_sock;
   delete IP_address;
   delete document_root;

   UEventFd::fd = -1; // NB: to avoid to delete itself...

   UNotifier::num_connection = 0;

         UDynamic::clear();
        UNotifier::clear();
   UPlugIn<void*>::clear();
}

void UServer_Base::closeLog()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::closeLog()")

#ifdef U_LOG_ENABLE
   if (log &&
       log->isOpen())
      {
      ULog::close();

#  ifdef DEBUG
      delete log;
#  endif

      log = 0;
      }

   if (apache_like_log &&
       apache_like_log->isOpen())
      {
      apache_like_log->closeLog();

#  ifdef DEBUG 
      delete apache_like_log;
#  endif

      apache_like_log = 0;
      }
#endif

   if (vlog)
      {
      for (uint32_t i = 0, n = vlog->size(); i < n; ++i)
         {
         file_LOG* item = (*vlog)[i];

         if (item->LOG->isOpen()) item->LOG->close();

         delete item->LOG;
         }

      vlog->clear();

#  ifdef DEBUG 
      delete vlog;
#  endif

      vlog = 0;
      }
}

#ifdef U_WELCOME_SUPPORT
void UServer_Base::setMsgWelcome(const UString& msg)
{
   U_TRACE(0, "UServer_Base::setMsgWelcome(%V)", msg.rep)

   U_INTERNAL_ASSERT(msg)

   msg_welcome = U_NEW(UString(U_CAPACITY));

   UEscape::decode(msg, *msg_welcome);

   if (*msg_welcome) (void) msg_welcome->shrink();
   else
      {
      delete msg_welcome;
             msg_welcome = 0;
      }
}
#endif

bool UServer_Base::setDocumentRoot(const UString& dir)
{
   U_TRACE(0, "UServer_Base::setDocumentRoot(%V)", dir.rep)

   *document_root = dir;

   if (document_root->empty() ||
       document_root->equal(U_CONSTANT_TO_PARAM(".")))
      {
      (void) document_root->replace(u_cwd, (document_root_size = u_cwd_len));
      }
   else
      {
      U_INTERNAL_ASSERT(document_root->isNullTerminated())

      document_root_ptr = document_root->data();

      char c = *document_root_ptr;

      if (c == '~' ||
          c == '$')
         {
         *document_root = UStringExt::expandPath(*document_root, 0);

         document_root_ptr = document_root->data();

         if (document_root->empty())
            {
            U_WARNING("var DOCUMENT_ROOT %S expansion failed", document_root_ptr);

            U_RETURN(false);
            }
         }

      *document_root = UFile::getRealPath(document_root_ptr, false);

      document_root_size = document_root->size();
      }

   document_root_ptr = document_root->data();

   U_INTERNAL_DUMP("document_root(%u) = %.*S", document_root_size, document_root_size, document_root_ptr)

   if (UFile::chdir(document_root_ptr, false) == false)
      {
      U_WARNING("chdir to working directory (DOCUMENT_ROOT) %S failed", document_root_ptr);

      U_RETURN(false);
      }

   U_INTERNAL_ASSERT_POINTER(document_root)
   U_INTERNAL_ASSERT_EQUALS(document_root_size, u_cwd_len)
   U_INTERNAL_ASSERT_EQUALS(strncmp(document_root_ptr, u_cwd, u_cwd_len), 0)

   U_RETURN(true);
}

void UServer_Base::loadConfigParam()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::loadConfigParam()")

   U_INTERNAL_ASSERT_POINTER(cfg)

   // --------------------------------------------------------------------------------------------------------------------------------------
   // userver - configuration parameters
   // --------------------------------------------------------------------------------------------------------------------------------------
   // ENABLE_IPV6 flag indicating the use of ipv6
   // SERVER      host name or ip address for the listening socket
   // PORT        port number             for the listening socket
   // SOCKET_NAME file name               for the listening socket
   // IP_ADDRESS  ip address of host for the interface connected to the Internet (autodetected if not specified)
   //
   // ALLOWED_IP            list of comma separated client address for IP-based access control (IPADDR[/MASK])
   // ALLOWED_IP_PRIVATE    list of comma separated client private address for IP-based access control (IPADDR[/MASK]) for public server
   // ENABLE_RFC1918_FILTER reject request from private IP to public server address
   // MIN_SIZE_FOR_SENDFILE for major size it is better to use sendfile() to serve static content
   //
   // LISTEN_BACKLOG        max number of ready to be delivered connections to accept()
   // SET_REALTIME_PRIORITY flag indicating that the preforked processes will be scheduled under the real-time policies SCHED_FIFO
   //
   // CLIENT_THRESHOLD           min number of clients to active polling
   // CLIENT_FOR_PARALLELIZATION min number of clients to active parallelization
   //
   // PID_FILE      write pid on file indicated
   // WELCOME_MSG   message of welcome to send initially to client
   // RUN_AS_USER   downgrade the security to that of user account
   // DOCUMENT_ROOT The directory out of which you will serve your documents
   //
   // LOG_FILE      locations   for file log
   // LOG_FILE_SZ   memory size for file log
   // LOG_MSG_SIZE  limit length of print network message to LOG_MSG_SIZE chars (default 128)
   //
   // PLUGIN        list of plugins to load, a flexible way to add specific functionality to the server
   // PLUGIN_DIR    directory where there are plugins to load
   //
   // ORM_DRIVER     list of ORM drivers to load, a flexible way to add specific functionality to the ORM
   // ORM_DRIVER_DIR directory where there are ORM drivers to load
   //
   // REQ_TIMEOUT    timeout for request from client
   // TCP_KEEP_ALIVE Specifies to active the TCP keepalive implementation in the linux kernel
   // TCP_LINGER_SET Specifies how the TCP initiated the close
   // MAX_KEEP_ALIVE Specifies the maximum number of requests that can be served through a Keep-Alive (Persistent) session. (Value <= 0 will disable Keep-Alive)
   //
   // DH_FILE       DH param
   // CERT_FILE     server certificate
   // KEY_FILE      server private key
   // PASSWORD      password for server private key
   // CA_FILE       locations of trusted CA certificates used in the verification
   // CA_PATH       locations of trusted CA certificates used in the verification
   // VERIFY_MODE   mode of verification (SSL_VERIFY_NONE=0, SSL_VERIFY_PEER=1, SSL_VERIFY_FAIL_IF_NO_PEER_CERT=2, SSL_VERIFY_CLIENT_ONCE=4)
   // CIPHER_SUITE  cipher suite model (Intermediate=0, Modern=1, Old=2)
   //
   // PREFORK_CHILD number of child server processes created at startup: -1 - thread approach (experimental)
   //                                                                     0 - serialize, no forking
   //                                                                     1 - classic, forking after client accept
   //                                                                    >1 - pool of serialized processes plus monitoring process
   // --------------------------------------------------------------------------------------------------------------------------------------

#ifdef USE_LIBSSL
   U_INTERNAL_DUMP("bssl = %b", bssl)
#endif

#if !defined(HAVE_EPOLL_WAIT) && !defined(USE_LIBEVENT) && defined(DEBUG)
   U_INTERNAL_DUMP("SOMAXCONN = %d FD_SETSIZE = %d", SOMAXCONN, FD_SETSIZE)
#endif

   UString x  = cfg->at(U_CONSTANT_TO_PARAM("SERVER"));
   *name_sock = cfg->at(U_CONSTANT_TO_PARAM("SOCKET_NAME"));

   if (x) server = U_NEW(UString(x));

   *IP_address = cfg->at(U_CONSTANT_TO_PARAM("IP_ADDRESS"));

#ifdef ENABLE_IPV6
   UClientImage_Base::bIPv6 = cfg->readBoolean(U_CONSTANT_TO_PARAM("ENABLE_IPV6"));
#endif

   timeoutMS = cfg->readLong(U_CONSTANT_TO_PARAM("REQ_TIMEOUT"), -1);

   if (timeoutMS > 0) timeoutMS *= 1000;

   port = cfg->readLong(U_CONSTANT_TO_PARAM("PORT"), bssl ? 443 : 80);

   if ((port == 80 || port == 443) &&
       UServices::isSetuidRoot() == false)
      {
      port = 8080;

      U_WARNING("Sorry, it is required root privilege to listen on port 80 but I am not setuid root, I must try 8080");
      }

   set_tcp_keep_alive    = cfg->readBoolean(U_CONSTANT_TO_PARAM("TCP_KEEP_ALIVE"));
   set_realtime_priority = cfg->readBoolean(U_CONSTANT_TO_PARAM("SET_REALTIME_PRIORITY"), true);

   tcp_linger_set                 = cfg->readLong(U_CONSTANT_TO_PARAM("TCP_LINGER_SET"), -2);
   USocket::iBackLog              = cfg->readLong(U_CONSTANT_TO_PARAM("LISTEN_BACKLOG"), SOMAXCONN);
   UNotifier::max_connection      = cfg->readLong(U_CONSTANT_TO_PARAM("MAX_KEEP_ALIVE"));
   u_printf_string_max_length     = cfg->readLong(U_CONSTANT_TO_PARAM("LOG_MSG_SIZE"));

   num_client_threshold           = cfg->readLong(U_CONSTANT_TO_PARAM("CLIENT_THRESHOLD"));
   num_client_for_parallelization = cfg->readLong(U_CONSTANT_TO_PARAM("CLIENT_FOR_PARALLELIZATION"));

   x = cfg->at(U_CONSTANT_TO_PARAM("PREFORK_CHILD"));

   if (x) str_preforked_num_kids = U_NEW(UString(x));

#ifdef U_WELCOME_SUPPORT
   x = cfg->at(U_CONSTANT_TO_PARAM("WELCOME_MSG"));

   if (x) setMsgWelcome(x);
#endif

   min_size_for_sendfile = cfg->readLong(U_CONSTANT_TO_PARAM("MIN_SIZE_FOR_SENDFILE"));

   if (min_size_for_sendfile == 0) min_size_for_sendfile = 500 * 1024; // 500k: for major size we assume is better to use sendfile()

#ifdef USE_LIBSSL
   *password   = cfg->at(U_CONSTANT_TO_PARAM("PASSWORD"));
   *ca_file    = cfg->at(U_CONSTANT_TO_PARAM("CA_FILE"));
   *ca_path    = cfg->at(U_CONSTANT_TO_PARAM("CA_PATH"));
   *key_file   = cfg->at(U_CONSTANT_TO_PARAM("KEY_FILE"));
   *cert_file  = cfg->at(U_CONSTANT_TO_PARAM("CERT_FILE"));

   *dh_file    = cfg->at(U_CONSTANT_TO_PARAM("DH_FILE"));
   verify_mode = cfg->at(U_CONSTANT_TO_PARAM("VERIFY_MODE"));

   if (bssl) min_size_for_sendfile = U_NOT_FOUND; // NB: we can't use sendfile with SSL...
#endif

   U_INTERNAL_DUMP("min_size_for_sendfile = %u", min_size_for_sendfile)

   // Instructs server to accept connections from the IP address IPADDR. A CIDR mask length can be
   // supplied optionally after a trailing slash, e.g. 192.168.0.0/24, in which case addresses that
   // match in the most significant MASK bits will be allowed. If no options are specified, all clients
   // are allowed. Unauthorized connections are rejected by closing the TCP connection immediately. A
   // warning is logged on the server but nothing is sent to the client

#ifdef U_ACL_SUPPORT
   x = cfg->at(U_CONSTANT_TO_PARAM("ALLOWED_IP"));

   if (x)
      {
       allow_IP = U_NEW(UString(x));
      vallow_IP = U_NEW(UVector<UIPAllow*>);

      if (UIPAllow::parseMask(*allow_IP, *vallow_IP) == 0)
         {
         delete allow_IP;
                allow_IP = 0;

         delete vallow_IP;
                vallow_IP = 0;
         }
      }
#endif

#ifdef U_RFC1918_SUPPORT
   x = cfg->at(U_CONSTANT_TO_PARAM("ALLOWED_IP_PRIVATE"));

   if (x)
      {
       allow_IP_prv = U_NEW(UString(x));
      vallow_IP_prv = U_NEW(UVector<UIPAllow*>);

      if (UIPAllow::parseMask(*allow_IP_prv, *vallow_IP_prv) == 0)
         {
         delete allow_IP_prv;
                allow_IP_prv = 0;

         delete vallow_IP_prv;
                vallow_IP_prv = 0;
         }
      }

   enable_rfc1918_filter = cfg->readBoolean(U_CONSTANT_TO_PARAM("ENABLE_RFC1918_FILTER"));
#endif

   x = cfg->at(U_CONSTANT_TO_PARAM("PID_FILE"));

   if (x)
      {
      // write pid on file

      U_INTERNAL_ASSERT(x.isNullTerminated())

      old_pid = UFile::getSysParam(x.data());

      (void) UFile::writeTo(x, UString(u_pid_str, u_pid_str_len));
      }

   // If you want the webserver to run as a process of a defined user, you can do it.
   // For the change of user to work, it's necessary to execute the server with root privileges.
   // If it's started by a user that that doesn't have root privileges, this step will be omitted

#ifdef U_LOG_ENABLE
   bool bmsg = false;
#endif

   x = cfg->at(U_CONSTANT_TO_PARAM("RUN_AS_USER"));

   if (x)
      {
      if (UServices::isSetuidRoot())
         {
         U_INTERNAL_ASSERT(x.isNullTerminated())

         struct passwd* pw = (struct passwd*) U_SYSCALL(getpwnam, "%S", x.data());

         if (pw &&
             pw->pw_dir)
            {
            *as_user = x;

#        ifdef DEBUG
            UMemoryPool::obj_class = "";
            UMemoryPool::func_call = __PRETTY_FUNCTION__;
#        endif

            char* buffer = (char*)UMemoryPool::pop(U_SIZE_TO_STACK_INDEX(U_PATH_MAX));

            (void) snprintf(buffer, U_PATH_MAX, "HOME=%s", pw->pw_dir);

            (void) U_SYSCALL(putenv, "%S", buffer);

#        ifdef DEBUG
            UMemoryPool::obj_class = UMemoryPool::func_call = 0;
#        endif
            }
         }
#  ifdef U_LOG_ENABLE
      else bmsg = true;
#  endif
      }

   // DOCUMENT_ROOT: The directory out of which we will serve your documents

   if (setDocumentRoot(cfg->at(U_CONSTANT_TO_PARAM("DOCUMENT_ROOT"))) == false)
      {
      U_ERROR("setting DOCUMENT ROOT %V failed", document_root->rep);
      }

#ifdef U_LOG_ENABLE
   x = cfg->at(U_CONSTANT_TO_PARAM("LOG_FILE"));

   if (x)
      {
      // open log

      update_date  =
      update_date1 = true;

      log = U_NEW(ULog(x, cfg->readLong(U_CONSTANT_TO_PARAM("LOG_FILE_SZ"))));

      log->init(U_SERVER_LOG_PREFIX);

      U_SRV_LOG("Working directory (DOCUMENT_ROOT) changed to %.*S", u_cwd_len, u_cwd);

      if (bmsg) U_SRV_LOG("WARNING: the \"RUN_AS_USER\" directive makes sense only if the master process runs with super-user privileges, ignored");
      }
#endif

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   UString orm_driver_dir  = cfg->at(U_CONSTANT_TO_PARAM("ORM_DRIVER_DIR")),
           orm_driver_list = cfg->at(U_CONSTANT_TO_PARAM("ORM_DRIVER"));

   if (orm_driver_dir) UOrmDriver::setDriverDirectory(orm_driver_dir);

   // load ORM driver modules...

   if (orm_driver_list &&
       (UString::str_allocate(STR_ALLOCATE_ORM), UOrmDriver::loadDriver(orm_driver_list)) == false)
      {
      U_ERROR("ORM drivers load failed");
      }
#endif

   // load plugin modules and call server-wide hooks handlerConfig()...

   UString plugin_dir  = cfg->at(U_CONSTANT_TO_PARAM("PLUGIN_DIR")),
           plugin_list = cfg->at(U_CONSTANT_TO_PARAM("PLUGIN"));

   if (loadPlugins(plugin_dir, plugin_list) == U_PLUGIN_HANDLER_ERROR) U_ERROR("Plugins stage load failed");
}

U_NO_EXPORT void UServer_Base::loadStaticLinkedModules(const char* name)
{
   U_TRACE(0, "UServer_Base::loadStaticLinkedModules(%S)", name)

   U_INTERNAL_ASSERT_POINTER(vplugin_name)
   U_INTERNAL_ASSERT_MAJOR(vplugin_size, 0)

   UString x(name);

   if (vplugin_name->find(x) != U_NOT_FOUND) // NB: we load only the plugin that we want from configuration (PLUGIN var)...
      {
#  ifdef U_LOG_ENABLE
      const char* fmt = "WARNING: Link phase of static plugin %s failed\n";
#  endif

#if defined(U_STATIC_HANDLER_RPC)    || defined(U_STATIC_HANDLER_SHIB)   || defined(U_STATIC_HANDLER_ECHO)  || \
    defined(U_STATIC_HANDLER_STREAM) || defined(U_STATIC_HANDLER_SOCKET) || defined(U_STATIC_HANDLER_SCGI)  || \
    defined(U_STATIC_HANDLER_FCGI)   || defined(U_STATIC_HANDLER_GEOIP)  || defined(U_STATIC_HANDLER_PROXY) || \
    defined(U_STATIC_HANDLER_SOAP)   || defined(U_STATIC_HANDLER_SSI)    || defined(U_STATIC_HANDLER_TSA)   || \
    defined(U_STATIC_HANDLER_NOCAT)  || defined(U_STATIC_HANDLER_HTTP)
      const UServerPlugIn* _plugin = 0;
#  ifdef U_STATIC_HANDLER_RPC
      if (x.equal(U_CONSTANT_TO_PARAM("rpc")))    { _plugin = U_NEW(URpcPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_SHIB
      if (x.equal(U_CONSTANT_TO_PARAM("shib")))   { _plugin = U_NEW(UShibPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_ECHO
      if (x.equal(U_CONSTANT_TO_PARAM("echo")))   { _plugin = U_NEW(UEchoPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_STREAM
      if (x.equal(U_CONSTANT_TO_PARAM("stream"))) { _plugin = U_NEW(UStreamPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_SOCKET
      if (x.equal(U_CONSTANT_TO_PARAM("socket"))) { _plugin = U_NEW(UWebSocketPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_SCGI
      if (x.equal(U_CONSTANT_TO_PARAM("scgi")))   { _plugin = U_NEW(USCGIPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_FCGI
      if (x.equal(U_CONSTANT_TO_PARAM("fcgi")))   { _plugin = U_NEW(UFCGIPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_GEOIP
      if (x.equal(U_CONSTANT_TO_PARAM("geoip")))  { _plugin = U_NEW(UGeoIPPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_PROXY
      if (x.equal(U_CONSTANT_TO_PARAM("proxy")))  { _plugin = U_NEW(UProxyPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_SOAP
      if (x.equal(U_CONSTANT_TO_PARAM("soap")))   { _plugin = U_NEW(USoapPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_SSI
      if (x.equal(U_CONSTANT_TO_PARAM("ssi")))    { _plugin = U_NEW(USSIPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_TSA
      if (x.equal(U_CONSTANT_TO_PARAM("tsa")))    { _plugin = U_NEW(UTsaPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_NOCAT
      if (x.equal(U_CONSTANT_TO_PARAM("nocat")))  { _plugin = U_NEW(UNoCatPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_HTTP
      if (x.equal(U_CONSTANT_TO_PARAM("http")))   { _plugin = U_NEW(UHttpPlugIn); goto next; }
#  endif
next:
      if (_plugin)
         {
         vplugin->push_back(_plugin);
         vplugin_name_static->push_back(x);

#     ifdef U_LOG_ENABLE
         fmt = "Link phase of static plugin %s success";
#     endif
         }
#endif

#  ifdef U_LOG_ENABLE
      if (isLog()) ULog::log(fmt, name);
#  endif
      }
}

int UServer_Base::loadPlugins(UString& plugin_dir, const UString& plugin_list)
{
   U_TRACE(0, "UServer_Base::loadPlugins(%V,%V)", plugin_dir.rep, plugin_list.rep)

   if (plugin_dir)
      {
      if (IS_DIR_SEPARATOR(plugin_dir.first_char()) == false) // NB: we can't use relativ path because after we call chdir()...
         {
         U_INTERNAL_ASSERT(plugin_dir.isNullTerminated())

         plugin_dir = UFile::getRealPath(plugin_dir.data());
         }

      UDynamic::setPluginDirectory(plugin_dir);
      }

   vplugin             = U_NEW(UVector<UServerPlugIn*>(10U));
   vplugin_name        = U_NEW(UVector<UString>(10U));
   vplugin_name_static = U_NEW(UVector<UString>(20U));

   uint32_t i;
   UString item, _name;
   UServerPlugIn* _plugin;
   int result = U_PLUGIN_HANDLER_ERROR;

   if (plugin_list.empty())
      {
      vplugin_size = 1;

      vplugin_name->push_back(*UString::str_http);
      }
   else
      {
      UClientImage_Base::callerHandlerRequest = pluginsHandlerRequest;

      vplugin_size = vplugin_name->split(U_STRING_TO_PARAM(plugin_list)); // NB: we don't use split with substr() cause of dependency from config var PLUGIN...
      }

   /**
    * I do know that to include code in the middle of a function is hacky and dirty,
    * but this is the best solution that I could figure out. If you have some idea to
    * clean it up, please, don't hesitate and let me know
    */

#  include "plugin/loader.autoconf.cpp"

   for (i = 0; i < vplugin_size; ++i)
      {
      item = vplugin_name->at(i);

      uint32_t pos = vplugin_name_static->find(item);

      U_INTERNAL_DUMP("i = %u pos = %u item = %V", i, pos, item.rep)

      if (pos != U_NOT_FOUND) continue;

      _name.setBuffer(32U);

      _name.snprintf("server_plugin_%v", item.rep);

      _plugin = UPlugIn<UServerPlugIn*>::create(U_STRING_TO_PARAM(_name));

#  ifdef U_LOG_ENABLE
      if (isLog())
         {
         (void) u__snprintf(mod_name[0], sizeof(mod_name[0]), "[%v] ", item.rep);

         if (_plugin == 0) ULog::log("%sWARNING: Load phase of plugin %v failed", mod_name[0], item.rep);
         else              ULog::log("%sLoad phase of plugin %v success",         mod_name[0], item.rep);

         mod_name[0][0] = '\0';
         }
#  endif

      if (_plugin)
         {
         vplugin->insert(i, _plugin);
         vplugin_name_static->insert(i, item);
         }
      }

   U_INTERNAL_ASSERT_MAJOR(vplugin_size, 0)
   U_INTERNAL_ASSERT_EQUALS(vplugin->size(), vplugin_size)
   U_INTERNAL_ASSERT_EQUALS(vplugin_name->size(), vplugin_name_static->size())

   delete vplugin_name_static;

   UClientImage_Base::callerHandlerRead = pluginsHandlerREAD;

   if (cfg)
      {
      // NB: we load configuration in reverse order respect to the content of config var PLUGIN...

      i = vplugin_size;

      do {
         item = vplugin_name->at(--i);

         if (cfg->searchForObjectStream(U_STRING_TO_PARAM(item)))
            {
            cfg->table.clear();

            _plugin = vplugin->at(i);

#        ifdef U_LOG_ENABLE
            if (isLog()) (void) u__snprintf(mod_name[0], sizeof(mod_name[0]), "[%v] ", item.rep);
#        endif

            result = _plugin->handlerConfig(*cfg);

#        ifdef U_LOG_ENABLE
            if (isLog())
               {
               if ((result & (U_PLUGIN_HANDLER_ERROR | U_PLUGIN_HANDLER_PROCESSED)) != 0)
                  {
                  const char* fmt = ((result & U_PLUGIN_HANDLER_ERROR) == 0
                                       ? "%sConfiguration phase of plugin %v success"
                                       : "%sWARNING: Configuration phase of plugin %v failed");

                  ULog::log(fmt, mod_name[0], item.rep);
                  }

               mod_name[0][0] = '\0';
               }
#        endif

            cfg->reset();

            if ((result & U_PLUGIN_HANDLER_GO_ON) == 0) U_RETURN(result);
            }

         if (i == 0) U_RETURN(U_PLUGIN_HANDLER_FINISHED);
         }
      while (true);
      }

   U_RETURN(U_PLUGIN_HANDLER_FINISHED);
}

// manage plugin handler hooks...

#ifdef U_LOG_DISABLE
#  define U_PLUGIN_HANDLER(xxx)                                               \
int UServer_Base::pluginsHandler##xxx()                                       \
{                                                                             \
   U_TRACE_NO_PARAM(0, "UServer_Base::pluginsHandler"#xxx"()")                \
                                                                              \
   U_INTERNAL_ASSERT_POINTER(vplugin)                                         \
   U_INTERNAL_ASSERT_MAJOR(vplugin_size, 0)                                   \
                                                                              \
   int result;                                                                \
   uint32_t i = 0;                                                            \
                                                                              \
   do {                                                                       \
      result = vplugin->at(i)->handler##xxx();                                \
                                                                              \
      if ((result & U_PLUGIN_HANDLER_GO_ON) == 0) U_RETURN(result);           \
      }                                                                       \
   while (++i < vplugin_size);                                                \
                                                                              \
   U_RETURN(U_PLUGIN_HANDLER_FINISHED);                                       \
}
#else
#  define U_PLUGIN_HANDLER(xxx)                                               \
int UServer_Base::pluginsHandler##xxx()                                       \
{                                                                             \
   U_TRACE_NO_PARAM(0, "UServer_Base::pluginsHandler"#xxx"()")                \
                                                                              \
   U_INTERNAL_ASSERT_POINTER(vplugin)                                         \
   U_INTERNAL_ASSERT_MAJOR(vplugin_size, 0)                                   \
                                                                              \
   int result;                                                                \
   uint32_t i = 0;                                                            \
   const char* fmt;                                                           \
   UServerPlugIn* _plugin;                                                    \
                                                                              \
   do {                                                                       \
      _plugin = vplugin->at(i);                                               \
                                                                              \
      if (isLog() == false) result = _plugin->handler##xxx();                 \
      else                                                                    \
         {                                                                    \
         UString name = vplugin_name->at(i);                                  \
                                                                              \
         (void)u__snprintf(mod_name[0],sizeof(mod_name[0]),"[%v] ",name.rep); \
                                                                              \
         result = _plugin->handler##xxx();                                    \
                                                                              \
         if ((result & (U_PLUGIN_HANDLER_ERROR |                              \
                        U_PLUGIN_HANDLER_PROCESSED)) != 0)                    \
            {                                                                 \
            if ((result & U_PLUGIN_HANDLER_ERROR) != 0)                       \
               {                                                              \
               fmt = ((result & U_PLUGIN_HANDLER_FINISHED) != 0               \
                  ? 0                                                         \
                  : "%sWARNING: "#xxx" phase of plugin %v failed");           \
               }                                                              \
            else                                                              \
               {                                                              \
               fmt = (U_ClientImage_parallelization == 2 ||                   \
                      (result & U_PLUGIN_HANDLER_PROCESSED) == 0              \
                        ? 0                                                   \
                        : "%s"#xxx" phase of plugin %v success");             \
               }                                                              \
                                                                              \
            if (fmt) ULog::log(fmt, mod_name[0], name.rep);                   \
            }                                                                 \
                                                                              \
         mod_name[0][0] = '\0';                                               \
         }                                                                    \
                                                                              \
      if ((result & U_PLUGIN_HANDLER_GO_ON) == 0) U_RETURN(result);           \
      }                                                                       \
   while (++i < vplugin_size);                                                \
                                                                              \
   U_RETURN(U_PLUGIN_HANDLER_FINISHED);                                       \
}
#endif

// Connection-wide hooks
U_PLUGIN_HANDLER(Request)

// NB: we call the various handlerXXX() in reverse order respect to the content of config var PLUGIN...

#ifdef U_LOG_DISABLE
#  define U_PLUGIN_HANDLER_REVERSE(xxx)                                       \
int UServer_Base::pluginsHandler##xxx()                                       \
{                                                                             \
   U_TRACE_NO_PARAM(0, "UServer_Base::pluginsHandler"#xxx"()")                \
                                                                              \
   U_INTERNAL_ASSERT_POINTER(vplugin)                                         \
   U_INTERNAL_ASSERT_MAJOR(vplugin_size, 0)                                   \
                                                                              \
   int result;                                                                \
   uint32_t i = vplugin_size;                                                 \
                                                                              \
   do {                                                                       \
      result = vplugin->at(--i)->handler##xxx();                              \
                                                                              \
      if ((result & U_PLUGIN_HANDLER_GO_ON) == 0) U_RETURN(result);           \
                                                                              \
      if (i == 0) U_RETURN(U_PLUGIN_HANDLER_FINISHED);                        \
      }                                                                       \
   while (true);                                                              \
}
#else
#  define U_PLUGIN_HANDLER_REVERSE(xxx)                                       \
int UServer_Base::pluginsHandler##xxx()                                       \
{                                                                             \
   U_TRACE_NO_PARAM(0, "UServer_Base::pluginsHandler"#xxx"()")                \
                                                                              \
   U_INTERNAL_ASSERT_POINTER(vplugin)                                         \
   U_INTERNAL_ASSERT_MAJOR(vplugin_size, 0)                                   \
                                                                              \
   int result;                                                                \
   const char* fmt;                                                           \
   UServerPlugIn* _plugin;                                                    \
   uint32_t i = vplugin_size;                                                 \
                                                                              \
   do {                                                                       \
      _plugin = vplugin->at(--i);                                             \
                                                                              \
      if (isLog() == false) result = _plugin->handler##xxx();                 \
      else                                                                    \
         {                                                                    \
         UString name = vplugin_name->at(i);                                  \
                                                                              \
         (void)u__snprintf(mod_name[0],sizeof(mod_name[0]),"[%v] ",name.rep); \
                                                                              \
         result = _plugin->handler##xxx();                                    \
                                                                              \
         if ((result & (U_PLUGIN_HANDLER_ERROR |                              \
                        U_PLUGIN_HANDLER_PROCESSED)) != 0)                    \
            {                                                                 \
            if ((result & U_PLUGIN_HANDLER_ERROR) != 0)                       \
               {                                                              \
               fmt = ((result & U_PLUGIN_HANDLER_FINISHED) != 0               \
                  ? 0                                                         \
                  : "%sWARNING: "#xxx" phase of plugin %v failed");           \
               }                                                              \
            else                                                              \
               {                                                              \
               fmt = (U_ClientImage_parallelization == 2 ||                   \
                      (result & U_PLUGIN_HANDLER_PROCESSED) == 0              \
                        ? 0                                                   \
                        : "%s"#xxx" phase of plugin %v success");             \
               }                                                              \
                                                                              \
            if (fmt) ULog::log(fmt, mod_name[0], name.rep);                   \
            }                                                                 \
                                                                              \
         mod_name[0][0] = '\0';                                               \
         }                                                                    \
                                                                              \
      if ((result & U_PLUGIN_HANDLER_GO_ON) == 0) U_RETURN(result);           \
                                                                              \
      if (i == 0) U_RETURN(U_PLUGIN_HANDLER_FINISHED);                        \
      }                                                                       \
   while (true);                                                              \
}
#endif

// Server-wide hooks
U_PLUGIN_HANDLER_REVERSE(Init)   // NB: we call handlerInit()   in reverse order respect to the content of config var PLUGIN...
U_PLUGIN_HANDLER_REVERSE(Run)    // NB: we call handlerRun()    in reverse order respect to the content of config var PLUGIN...
U_PLUGIN_HANDLER_REVERSE(Fork)   // NB: we call handlerFork()   in reverse order respect to the content of config var PLUGIN...
U_PLUGIN_HANDLER_REVERSE(Stop)   // NB: we call handlerStop()   in reverse order respect to the content of config var PLUGIN...
// Connection-wide hooks
U_PLUGIN_HANDLER_REVERSE(READ)   // NB: we call handlerREAD()   in reverse order respect to the content of config var PLUGIN...
// SigHUP hook
U_PLUGIN_HANDLER_REVERSE(SigHUP) // NB: we call handlerSigHUP() in reverse order respect to the content of config var PLUGIN...

#undef U_PLUGIN_HANDLER
#undef U_PLUGIN_HANDLER_REVERSE

void UServer_Base::init()
{
   U_TRACE_NO_PARAM(1, "UServer_Base::init()")

   U_INTERNAL_ASSERT_POINTER(socket)

#ifdef USE_LIBSSL
   if (bssl)
      {
      U_ASSERT(((USSLSocket*)socket)->isSSL())

      if (cfg) ((USSLSocket*)socket)->ciphersuite_model = cfg->readLong(U_CONSTANT_TO_PARAM("CIPHER_SUITE"));

      // Load our certificate

      U_INTERNAL_ASSERT(  dh_file->isNullTerminated())
      U_INTERNAL_ASSERT(  ca_file->isNullTerminated())
      U_INTERNAL_ASSERT(  ca_path->isNullTerminated())
      U_INTERNAL_ASSERT( key_file->isNullTerminated())
      U_INTERNAL_ASSERT( password->isNullTerminated())
      U_INTERNAL_ASSERT(cert_file->isNullTerminated())

      if (((USSLSocket*)socket)->setContext( dh_file->data(), cert_file->data(), key_file->data(),
                                            password->data(),   ca_file->data(),  ca_path->data(), verify_mode) == false)
         {
         U_ERROR("SSL: server setContext() failed");
         }
      }
   else
#endif
   {
   if (bipc)
      {
      U_ASSERT(socket->isIPC())

#  ifdef _MSWINDOWS_
      U_ERROR("Sorry, I was compiled on Windows so I can't accept SOCKET_NAME");
#  else
      if (*name_sock) UUnixSocket::setPath(name_sock->data());

      if (UUnixSocket::path == 0) U_ERROR("UNIX domain socket is not bound to a file system pathname");
#  endif
      }
   }

   if (socket->setServer(port, server) == false)
      {
      UString x = (server ? *server : U_STRING_FROM_CONSTANT("*"));

      U_ERROR("Run as server with local address '%v:%u' failed", x.rep, port);
      }

   U_SRV_LOG("TCP SO_REUSEPORT status is: %susing", (USocket::tcp_reuseport ? "" : "NOT "));

   // get name host

   host = U_NEW(UString(server ? *server : USocketExt::getNodeName()));

   if (port != 80)
      {
      host->push_back(':');

      UStringExt::appendNumber32(*host, port);
      }

   U_SRV_LOG("HOST registered as: %v", host->rep);

   // get IP address host (default source)

   if (server              &&
       IP_address->empty() &&
       u_isIPAddr(UClientImage_Base::bIPv6, U_STRING_TO_PARAM(*server)))
      {
      *IP_address = *server;
      }

#ifdef _MSWINDOWS_
   if (IP_address->empty() ||
       socket->setHostName(*IP_address) == false)
      {
      U_ERROR("On windows we need a valid IP_ADDRESS value on configuration file");
      }
#else
   /**
    * This code does NOT make a connection or send any packets (to 8.8.8.8 which is google DNS).
    * Since UDP is a stateless protocol connect() merely makes a system call which figures out how to
    * route the packets based on the address and what interface (and therefore IP address) it should
    * bind to. Returns an array containing the family (AF_INET), local port, and local address (which
    * is what we want) of the socket
    */

   if (bipc == false)
      {
      UUDPSocket cClientSocket(UClientImage_Base::bIPv6);

      if (cClientSocket.connectServer(U_STRING_FROM_CONSTANT("8.8.8.8"), 1001))
         {
         socket->setLocal(cClientSocket.cLocalAddress);

         UString ip = UString(socket->getLocalInfo());

              if ( IP_address->empty()) *IP_address = ip;
         else if (*IP_address != ip)
            {
            U_SRV_LOG("WARNING: SERVER IP ADDRESS from configuration : %V differ from system interface: %V", IP_address->rep, ip.rep);
            }
         }

      struct in_addr ia;

      if (inet_aton(IP_address->c_str(), &ia) == 0)
         {
         U_WARNING("IP_ADDRESS conversion fail, we try using localhost");

         (void) inet_aton("localhost", &ia);
         }

      socket->setAddress(&ia);

      public_address = (socket->cLocalAddress.isPrivate() == false);

      U_SRV_LOG("SERVER IP ADDRESS registered as: %v (%s)", IP_address->rep, (public_address ? "public" : "private"));

      u_need_root(false);

#  if defined(LINUX) || defined(__LINUX__) || defined(__linux__)
      /**
       * timeout_timewait parameter: Determines the time that must elapse before TCP/IP can release a closed connection
       * and reuse its resources. This interval between closure and release is known as the TIME_WAIT state or twice the
       * maximum segment lifetime (2MSL) state. During this time, reopening the connection to the client and server cost
       * less than establishing a new connection. By reducing the value of this entry, TCP/IP can release closed connections
       * faster, providing more resources for new connections. Adjust this parameter if the running application requires rapid
       * release, the creation of new connections, and a low throughput due to many connections sitting in the TIME_WAIT state
       */

                                tcp_fin_timeout = UFile::getSysParam("/proc/sys/net/ipv4/tcp_fin_timeout");
      if (tcp_fin_timeout > 30) tcp_fin_timeout = UFile::setSysParam("/proc/sys/net/ipv4/tcp_fin_timeout", 30, true);

      /**
       * sysctl_somaxconn (SOMAXCONN: 128) specifies the maximum number of sockets in state SYN_RECV per listen socket queue.
       * At listen(2) time the backlog is adjusted to this limit if bigger then that.
       *
       * sysctl_max_syn_backlog on the other hand is dynamically adjusted, depending on the memory characteristic of the system.
       * Default is 256, 128 for small systems and up to 1024 for bigger systems.
       *
       * The system limits (somaxconn & tcp_max_syn_backlog) specify a _maximum_, the user cannot exceed this limit with listen(2).
       * The backlog argument for listen on the other  hand  specify a _minimum_
       */

      if (USocket::iBackLog == 1)
         {
         // sysctl_tcp_abort_on_overflow when its on, new connections are reset once the backlog is exhausted

         tcp_abort_on_overflow = UFile::setSysParam("/proc/sys/net/ipv4/tcp_abort_on_overflow", 1, true);
         }
      else if (USocket::iBackLog > SOMAXCONN)
         {
         // NB: take a look at `netstat -s | grep overflowed`

         sysctl_somaxconn = UFile::getSysParam("/proc/sys/net/core/somaxconn");

         if (sysctl_somaxconn < USocket::iBackLog)
            {
            int value = UFile::setSysParam("/proc/sys/net/core/somaxconn", USocket::iBackLog);

            if (value == USocket::iBackLog) sysctl_max_syn_backlog = UFile::setSysParam("/proc/sys/net/ipv4/tcp_max_syn_backlog", value * 8);
            else
               {
               U_WARNING("The TCP backlog (LISTEN_BACKLOG) setting of %u cannot be enforced because of OS error - "
                         "/proc/sys/net/core/somaxconn is set to the lower value of %d", USocket::iBackLog, sysctl_somaxconn);
               }
            }
         }

      U_INTERNAL_DUMP("sysctl_somaxconn = %d tcp_abort_on_overflow = %b sysctl_max_syn_backlog = %d",
                       sysctl_somaxconn,     tcp_abort_on_overflow,     sysctl_max_syn_backlog)
#  endif
      }
#endif

   if (str_preforked_num_kids)
      {
      preforked_num_kids = str_preforked_num_kids->strtol();

#  ifdef U_SERVER_CAPTIVE_PORTAL
      if (str_preforked_num_kids->c_char(0) == '0') monitoring_process = true;
#  endif

      delete str_preforked_num_kids;
             str_preforked_num_kids = 0;

#  if !defined(ENABLE_THREAD) || defined(USE_LIBEVENT) || !defined(U_SERVER_THREAD_APPROACH_SUPPORT)
      if (preforked_num_kids == -1)
         {
         U_WARNING("Sorry, I was compiled without server thread approach so I can't accept PREFORK_CHILD == -1");

         preforked_num_kids = 2;
         }
#  endif
      }
#ifndef _MSWINDOWS_
   else
      {
      preforked_num_kids = u_get_num_cpu();

      U_INTERNAL_DUMP("num_cpu = %d", preforked_num_kids)

      if (preforked_num_kids < 2) preforked_num_kids = 2;
      }
#endif

#ifndef U_CLASSIC_SUPPORT
   if (isClassic())
      {
      U_WARNING("Sorry, I was compiled without server classic model support so I can't accept PREFORK_CHILD == 1");

      preforked_num_kids = 2;
      }
#endif

#ifdef _MSWINDOWS_
   if (preforked_num_kids > 0)
      {
      U_WARNING("Sorry, I was compiled on Windows so I can't accept PREFORK_CHILD > 0");

      preforked_num_kids = 0;
      }
#endif

   if (preforked_num_kids > 1) monitoring_process = true;

   UTimer::init();

   UClientImage_Base::init();

   USocket::accept4_flags = SOCK_CLOEXEC | SOCK_NONBLOCK;

   U_INTERNAL_ASSERT_EQUALS(proc, 0)

   proc = U_NEW(UProcess);

   U_INTERNAL_ASSERT_POINTER(proc)

   proc->setProcessGroup();

#ifdef U_LOG_ENABLE
   uint32_t log_rotate_size = 0;

# ifdef USE_LIBZ
   if (isLog())
      {
      // The zlib documentation states that destination buffer size must be at least 0.1% larger than avail_in plus 12 bytes

      log_rotate_size =
      shared_data_add = log->UFile::st_size + (log->UFile::st_size / 10) + 12U;
      }
# endif

   U_INTERNAL_DUMP("log_rotate_size = %u", log_rotate_size)
#endif

   // init plugin modules, must run after the setting for shared log

   if (pluginsHandlerInit() != U_PLUGIN_HANDLER_FINISHED) U_ERROR("Plugins stage init failed");

   // manage shared data...

   U_INTERNAL_DUMP("shared_data_add = %u", shared_data_add)

   U_INTERNAL_ASSERT_EQUALS(ptr_shared_data, 0)

   map_size        = sizeof(shared_data) + shared_data_add;
   ptr_shared_data = (shared_data*) UFile::mmap(&map_size);

   U_INTERNAL_ASSERT_POINTER(ptr_shared_data)
   U_INTERNAL_ASSERT_DIFFERS(ptr_shared_data, MAP_FAILED)

#if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
   bool bpthread_time = (preforked_num_kids >= 4); // intuitive heuristic...
#else
   bool bpthread_time = false; 
#endif

   U_INTERNAL_DUMP("bpthread_time = %b", bpthread_time)

#ifdef U_LOG_ENABLE
   if (isLog() == false)
#endif
   ULog::initDate();

#if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
   if (bpthread_time)
      {
      U_INTERNAL_ASSERT_POINTER(ptr_shared_data)
      U_INTERNAL_ASSERT_EQUALS(ULog::ptr_shared_date, 0)

      *(u_now = &(ptr_shared_data->now_shared)) = u_timeval;

      ULog::ptr_shared_date = &(ptr_shared_data->log_date_shared);

      U_MEMCPY(ULog::ptr_shared_date, &ULog::date, sizeof(ULog::log_date));
      }
#endif

#if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
   // NB: we block SIGHUP and SIGTERM; the threads created will inherit a copy of the signal mask...
# ifdef sigemptyset
                    sigemptyset(&mask);
# else
   (void) U_SYSCALL(sigemptyset, "%p", &mask);
# endif

# ifdef sigaddset
                    sigaddset(&mask, SIGHUP);
                    sigaddset(&mask, SIGTERM);
# else
   (void) U_SYSCALL(sigaddset, "%p,%d", &mask, SIGHUP);
   (void) U_SYSCALL(sigaddset, "%p,%d", &mask, SIGTERM);
# endif

   (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_BLOCK, &mask, 0);
#endif

   flag_loop = true; // NB: UTimeThread loop depend on this setting...

#if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
   if (bpthread_time)
      {
      U_INTERNAL_ASSERT_EQUALS(ULog::prwlock, 0)
      U_INTERNAL_ASSERT_EQUALS(u_pthread_time, 0)

      U_NEW_ULIB_OBJECT(u_pthread_time, UTimeThread);

      U_INTERNAL_DUMP("u_pthread_time = %p", u_pthread_time)

      (void) UThread::initRwLock((ULog::prwlock = &(ptr_shared_data->rwlock)));

      ((UTimeThread*)u_pthread_time)->start(50);
      }
#endif

#ifdef U_LOG_ENABLE
   if (isLog())
      {
      // NB: if log is mapped must be always shared because the possibility of fork() by parallelization...

      if (log->isMemoryMapped()) log->setShared(&(ptr_shared_data->log_data_shared), log_rotate_size);

      U_SRV_LOG("Mapped %u bytes (%u KB) of shared memory for %d preforked process", sizeof(shared_data) + shared_data_add, map_size / 1024, preforked_num_kids);
      }
#endif

   last_event = u_now->tv_sec;

#ifdef U_LOG_ENABLE
   U_INTERNAL_ASSERT_EQUALS(U_TOT_CONNECTION, 0)
#endif

#ifdef DEBUG
   UTimer::insert(pstat = U_NEW(UTimeStat));
#endif
#ifdef U_THROTTLING_SUPPORT
   if (db_throttling) UTimer::insert(throttling_time = U_NEW(UBandWidthThrottling)); // set up the throttles timer
#endif

   // ---------------------------------------------------------------------------------------------------------
   // init notifier event manager
   // ---------------------------------------------------------------------------------------------------------

   socket_flags |= O_RDWR | O_CLOEXEC;

#if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   if (preforked_num_kids != -1)
#endif
      {
      // ---------------------------------------------------------------------------------------------------------
      // NB: in the classic model we don't need to be notified for request of connection (loop: accept-fork)
      //     and the forked child don't accept new client, but we need anyway the event manager because
      //     the forked child must feel the possibly timeout for request from the new client...
      // ---------------------------------------------------------------------------------------------------------

      if (timeoutMS > 0 ||
          isClassic() == false)
         {
         binsert = true; // NB: we ask to be notified for request of connection (=> accept)

         UNotifier::min_connection = 1;

#     ifndef USE_LIBEVENT
         if (timeoutMS > 0) ptime = U_NEW(UTimeoutConnection);
#     endif

         pthis->UEventFd::op_mask |= EPOLLET;

         U_INTERNAL_ASSERT_EQUALS(pthis->UEventFd::op_mask, EPOLLIN | EPOLLRDHUP | EPOLLET)

         /**
          * There may not always be a connection waiting after a SIGIO is delivered or select(2) or poll(2) return a readability
          * event because the connection might have been removed by an asynchronous network error or another thread before
          * accept() is called. If this happens then the call will block waiting for the next connection to arrive. To ensure
          * that accept() never blocks, the passed socket sockfd needs to have the O_NONBLOCK flag set (see socket(7))
          */

         socket_flags |= O_NONBLOCK;
         }

      if (handler_other)   UNotifier::min_connection++;
      if (handler_inotify) UNotifier::min_connection++;
      }

   UNotifier::max_connection = (UNotifier::max_connection ? UNotifier::max_connection : USocket::iBackLog) + (UNotifier::num_connection = UNotifier::min_connection);

   if (num_client_threshold == 0) num_client_threshold = U_NOT_FOUND;

   if (num_client_for_parallelization == 0) num_client_for_parallelization = UNotifier::max_connection / 2;

   U_INTERNAL_DUMP("UNotifier::max_connection = %u UNotifier::min_connection = %u num_client_for_parallelization = %u num_client_threshold = %u",
                    UNotifier::max_connection,     UNotifier::min_connection,     num_client_for_parallelization,     num_client_threshold)

   pthis->preallocate();

#if defined(USE_LIBSSL) && defined(ENABLE_THREAD) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB) && !defined(_MSWINDOWS_)
   if (bssl)
      {
      if (USSLSocket::setDataForStapling() == false)
         {
         U_WARNING("SSL: OCSP stapling ignored, some error occured...");
         }
      else
         {
         USSLSocket::staple.data = getPointerToDataShare(USSLSocket::staple.data);

         setLockOCSPStaple();

         U_INTERNAL_ASSERT_EQUALS(USSLSocket::staple.client, 0)

         USSLSocket::staple.client = U_NEW(UClient<UTCPSocket>(0));

         (void) USSLSocket::staple.client->setUrl(*USSLSocket::staple.url);

         U_INTERNAL_ASSERT_EQUALS(pthread_ocsp, 0)

         U_NEW_ULIB_OBJECT(pthread_ocsp, UOCSPStapling);

         U_INTERNAL_DUMP("pthread_ocsp = %p", pthread_ocsp)

         pthread_ocsp->start(0);
         }
      }
#endif

   if (pluginsHandlerRun() != U_PLUGIN_HANDLER_FINISHED) U_ERROR("Plugins stage run failed");

   if (u_start_time     == 0 &&
       u_setStartTime() == false)
      {
      U_ERROR("System date not updated");
      }

   if (cfg) cfg->clear();

   UInterrupt::syscall_restart                 = false;
   UInterrupt::exit_loop_wait_event_for_signal = true;

#if !defined(USE_LIBEVENT) && !defined(USE_RUBY)
   UInterrupt::insert(              SIGHUP, (sighandler_t)UServer_Base::handlerForSigHUP);  // async signal
   UInterrupt::insert(             SIGTERM, (sighandler_t)UServer_Base::handlerForSigTERM); // async signal
#else
   UInterrupt::setHandlerForSignal( SIGHUP, (sighandler_t)UServer_Base::handlerForSigHUP);  //  sync signal
   UInterrupt::setHandlerForSignal(SIGTERM, (sighandler_t)UServer_Base::handlerForSigTERM); //  sync signal
#endif

   if (old_pid > 0)
      {
#  ifdef U_LOG_ENABLE
      if (isLog()) ULog::log("Trying to kill another instance of userver that is running with pid %d", old_pid);
#  endif

      UProcess::kill(old_pid, SIGTERM); // SIGTERM is sent to every process in the process group of the calling process...
      }
}

bool UServer_Base::addLog(UFile* _log, int flags)
{
   U_TRACE(0, "UServer_Base::addLog(%p,%d)", _log, flags)

   U_INTERNAL_ASSERT_POINTER(vlog)

   if (_log->creat(flags, PERM_FILE))
      {
      file_LOG* item = U_NEW(file_LOG);

      item->LOG   = _log;
      item->flags = flags;

      vlog->push_back(item);

      U_RETURN(true);
      }

   U_RETURN(false);
}

void UServer_Base::reopenLog()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::reopenLog()")

   file_LOG* item;

   for (uint32_t i = 0, n = vlog->size(); i < n; ++i)
      {
      item = (*vlog)[i];

      item->LOG->reopen(item->flags);
      }
}

U_NO_EXPORT void UServer_Base::logMemUsage(const char* signame)
{
   U_TRACE(0, "UServer_Base::logMemUsage(%S)", signame)

   U_INTERNAL_ASSERT(isLog())

#ifdef U_LOG_ENABLE
   unsigned long vsz, rss;

   u_get_memusage(&vsz, &rss);

   ULog::log("%s (Interrupt): "
             "address space usage: %.2f MBytes - "
                       "rss usage: %.2f MBytes", signame,
             (double)vsz / (1024.0 * 1024.0),
             (double)rss / (1024.0 * 1024.0));
#endif
}

RETSIGTYPE UServer_Base::handlerForSigCHLD(int signo)
{
   U_TRACE(0, "[SIGCHLD] UServer_Base::handlerForSigCHLD(%d)", signo)

   U_INTERNAL_ASSERT_POINTER(proc)

   if (proc->parent()) proc->wait();
}

U_NO_EXPORT void UServer_Base::manageSigHUP()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::manageSigHUP()")

   U_INTERNAL_ASSERT_POINTER(proc)

   (void) proc->waitAll(1);

   if (pluginsHandlerSigHUP() != U_PLUGIN_HANDLER_FINISHED) U_WARNING("Plugins stage SigHUP failed...");
}

RETSIGTYPE UServer_Base::handlerForSigHUP(int signo)
{
   U_TRACE(0, "[SIGHUP] UServer_Base::handlerForSigHUP(%d)", signo)

   U_INTERNAL_ASSERT_POINTER(proc)
   U_INTERNAL_ASSERT_POINTER(pthis)
   U_INTERNAL_ASSERT(proc->parent())

#if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   if (preforked_num_kids == -1) return;
#endif

   // NB: for logrotate...

#ifdef U_LOG_ENABLE
   if (isLog())
      {
      logMemUsage("SIGHUP");

      log->reopen();
      }
#endif

   if (isOtherLog()) reopenLog();

   (void) U_SYSCALL(gettimeofday, "%p,%p", u_now, 0);

#if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
   if (u_pthread_time) ((UTimeThread*)u_pthread_time)->suspend();

# if defined(USE_LIBSSL) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
   if (pthread_ocsp) pthread_ocsp->suspend();
# endif
#endif

   pthis->handlerSignal(); // manage signal before we regenering the preforked pool of children...

   // NB: we can't use UInterrupt::erase() because it restore the old action (UInterrupt::init)...

   UInterrupt::setHandlerForSignal(SIGTERM, (sighandler_t)SIG_IGN);

   UProcess::kill(0, SIGTERM); // SIGTERM is sent to every process in the process group of the calling process...

#if defined(USE_LIBEVENT)
   UInterrupt::setHandlerForSignal(SIGTERM, (sighandler_t)UServer_Base::handlerForSigTERM); //  sync signal
#else
                UInterrupt::insert(SIGTERM, (sighandler_t)UServer_Base::handlerForSigTERM); // async signal
#endif

#if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
   if (u_pthread_time) ((UTimeThread*)u_pthread_time)->resume();

#  if defined(USE_LIBSSL) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
   if (pthread_ocsp) pthread_ocsp->resume();
#  endif
#endif

#ifdef U_LOG_ENABLE
   U_TOT_CONNECTION = 0;
#endif

   if (preforked_num_kids > 1) rkids = 0;
   else                        manageSigHUP();
}

RETSIGTYPE UServer_Base::handlerForSigTERM(int signo)
{
   U_TRACE(0, "[SIGTERM] UServer_Base::handlerForSigTERM(%d)", signo)

   flag_loop    = false;
   flag_sigterm = true;

   U_INTERNAL_ASSERT_POINTER(proc)

   if (proc->parent())
      {
#  if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
      if (u_pthread_time) ((UTimeThread*)u_pthread_time)->suspend();
#  endif

      // NB: we can't use UInterrupt::erase() because it restore the old action (UInterrupt::init)...

      UInterrupt::setHandlerForSignal(SIGTERM, (sighandler_t)SIG_IGN);

      UProcess::kill(0, SIGTERM); // SIGTERM is sent to every process in the process group of the calling process...
      }
   else
      {
#  ifdef USE_LIBEVENT
      (void) UDispatcher::exit(0);
#  elif !defined(USE_RUBY)
      UInterrupt::erase(SIGTERM); // async signal
#  endif

#  if defined(U_STDCPP_ENABLE) && defined(DEBUG)
      if (U_SYSCALL(getenv, "%S", "UMEMUSAGE"))
         {
         uint32_t len;
         char buffer[4096];
         unsigned long vsz, rss;

         u_get_memusage(&vsz, &rss);

         len = u__snprintf(buffer, sizeof(buffer),
                        "SIGTERM (Interrupt): "
                        "address space usage: %.2f MBytes - "
                                  "rss usage: %.2f MBytes\n"
                        "%v\nmax_nfd_ready = %u max_depth = %u again:read = (%u/%u - %u%%) wakeup_for_nothing = %u bepollet_threshold = (%u/10)\n",
                        getStats().rep,
                        (double)vsz / (1024.0 * 1024.0),
                        (double)rss / (1024.0 * 1024.0), UNotifier::max_nfd_ready, max_depth,
                        nread_again, nread, (nread_again * 100) / nread, wakeup_for_nothing, UNotifier::bepollet_threshold);

         ostrstream os(buffer + len, sizeof(buffer) - len);

         UMemoryPool::printInfo(os);

         len += os.pcount();

         U_INTERNAL_ASSERT_MINOR(len, sizeof(buffer))

         (void) UFile::writeToTmp(buffer, len, false, "%N.memusage.%P", 0);
         }
#  endif

      if (preforked_num_kids)
         {
#     if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
         if (preforked_num_kids == -1) UNotifier::pthread->suspend();
#       ifdef DEBUG
         u_trace_unlock();
#       endif
#     endif

#     ifdef U_LOG_ENABLE
         if (isLog()) logMemUsage("SIGTERM");
#     endif

         U_EXIT(0);
         }
      }
}

U_NO_EXPORT bool UServer_Base::clientImageHandlerRead()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::clientImageHandlerRead()")

   U_INTERNAL_ASSERT(csocket->isOpen())
   U_INTERNAL_ASSERT_EQUALS(csocket, pClientImage->socket)

   if (pClientImage->handlerRead() == U_NOTIFIER_DELETE)
      {
      if (csocket->isOpen())
         {
         csocket->iState = USocket::CONNECT;

         csocket->close();
         }

      pClientImage->UClientImage_Base::handlerDelete();

      U_RETURN(false);
      }

   U_RETURN(true);
}

#if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
#  define CSOCKET psocket
#  define CLIENT_INDEX lClientIndex
#  define CLIENT_ADDRESS lclient_address
#  define CLIENT_ADDRESS_LEN lclient_address_len
#  define CLIENT_IMAGE_HANDLER_READ (pClientImage = lClientIndex, csocket = psocket, \
                                     client_address = lclient_address, client_address_len = lclient_address_len, clientImageHandlerRead())
#else
#  define CSOCKET csocket
#  define CLIENT_INDEX pClientImage
#  define CLIENT_ADDRESS client_address
#  define CLIENT_ADDRESS_LEN client_address_len
#  define CLIENT_IMAGE_HANDLER_READ clientImageHandlerRead()
#endif

int UServer_Base::handlerRead() // This method is called to accept a new connection on the server socket (listening)
{
   U_TRACE_NO_PARAM(1, "UServer_Base::handlerRead()")

   U_INTERNAL_DUMP("nClientIndex = %u", nClientIndex)

   U_INTERNAL_ASSERT_POINTER(ptr_shared_data)
   U_INTERNAL_ASSERT_MINOR(nClientIndex, UNotifier::max_connection)

   // This loops until the accept() fails, trying to start new connections as fast as possible so we don't overrun the listen queue

   pClientImage = vClientImage + nClientIndex;

#if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   USocket* psocket;
   char* lclient_address;
   uint32_t lclient_address_len;
   UClientImage_Base* lClientIndex = pClientImage;
#endif
   int cround = 0;
#ifdef DEBUG
   CLIENT_ADDRESS_LEN = 0;
   uint32_t numc, nothing = 0;
#endif

loop:
   U_INTERNAL_ASSERT_MINOR(CLIENT_INDEX, eClientImage)
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, 1) // 1 => child of parallelization

   CSOCKET = CLIENT_INDEX->socket;

   U_INTERNAL_DUMP("----------------------------------------", 0)
   U_INTERNAL_DUMP("vClientImage[%d].last_event        = %#3D",  (CLIENT_INDEX - vClientImage), CLIENT_INDEX->last_event)
   U_INTERNAL_DUMP("vClientImage[%u].sfd               = %d",    (CLIENT_INDEX - vClientImage), CLIENT_INDEX->sfd)
   U_INTERNAL_DUMP("vClientImage[%u].UEventFd::fd      = %d",    (CLIENT_INDEX - vClientImage), CLIENT_INDEX->UEventFd::fd)
   U_INTERNAL_DUMP("vClientImage[%u].socket            = %p",    (CLIENT_INDEX - vClientImage), CSOCKET)
   U_INTERNAL_DUMP("vClientImage[%d].socket->flags     = %d %B", (CLIENT_INDEX - vClientImage), CSOCKET->flags, CSOCKET->flags)
   U_INTERNAL_DUMP("vClientImage[%d].socket->iSockDesc = %d",    (CLIENT_INDEX - vClientImage), CSOCKET->iSockDesc)
   U_INTERNAL_DUMP("----------------------------------------", 0)

   if (CSOCKET->isOpen()) // busy
      {
      if (cround >= 2) // polling mode
         {
         if (CLIENT_IMAGE_HANDLER_READ == false)
            {
            U_INTERNAL_DUMP("cround = %u UNotifier::num_connection = %u num_client_threshold = %u", cround, UNotifier::num_connection, num_client_threshold)

            if (UNotifier::num_connection < num_client_threshold)
               {
#           ifdef DEBUG
               U_WARNING("It has returned below the client threshold(%u): preallocation(%u) num_connection(%u)",
                           num_client_threshold, UNotifier::max_connection, UNotifier::num_connection - UNotifier::min_connection);
#           endif

               goto end;
               }
            }
#     ifdef DEBUG
         else if (UClientImage_Base::rbuffer->empty())
            {
            ++nothing;

            U_INTERNAL_DUMP("nothing = %u", nothing)
            }
#     endif
         }
      else if (ptime) // NB: we check if the connection is idle...
         {
         U_INTERNAL_ASSERT_POINTER(ptime)

         U_gettimeofday; // NB: optimization if it is enough a time resolution of one second...

         if ((u_now->tv_sec - CLIENT_INDEX->last_event) >= ptime->UTimeVal::tv_sec)
            {
#        ifdef U_LOG_ENABLE
            called_from_handlerTime = false;
#        endif
            
            if (handlerTimeoutConnection(CLIENT_INDEX))
               {
               UNotifier::erase((UEventFd*)CLIENT_INDEX);

               goto try_accept;
               }
            }
         }

try_next:
      if (++CLIENT_INDEX >= eClientImage)
         {
         U_INTERNAL_ASSERT_POINTER(vClientImage)

         CLIENT_INDEX = vClientImage;

         if (cround >= 2)
            {
#        ifdef DEBUG
            if (nothing > ((UNotifier::num_connection - UNotifier::min_connection) / 2))
               {
               U_WARNING("Polling mode suspect: cround = %u nothing = %u num_connection = %u", cround, nothing, UNotifier::num_connection - UNotifier::min_connection);
               }

            nothing = 0;
#        endif
            }
         else if (++cround >= 2)
            {
            U_INTERNAL_DUMP("cround = %u num_client_threshold = %u (UNotifier::num_connection * 2) / 3 = %u", cround, num_client_threshold, (UNotifier::num_connection * 2) / 3)

            num_client_threshold = (UNotifier::num_connection * 2) / 3;

#        ifdef DEBUG
            U_WARNING("Out of space on client image preallocation(%u): num_connection(%u)", UNotifier::max_connection, UNotifier::num_connection - UNotifier::min_connection);
#        endif
            }
         }

      goto loop;
      }

   if (cround >= 2) goto try_next; // polling mode

try_accept:
   U_INTERNAL_ASSERT(CSOCKET->isClosed())
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, 1) // 1 => child of parallelization

   if (socket->acceptClient(CSOCKET) == false)
      {
      U_INTERNAL_DUMP("flag_loop = %b CSOCKET->iState = %d", flag_loop, CSOCKET->iState)

#  ifdef DEBUG
      if (CLIENT_ADDRESS_LEN == 0 &&
          CSOCKET->iState == -EAGAIN)
         {
         ++wakeup_for_nothing;
         }
#  endif
#  ifdef U_LOG_ENABLE
      if (isLog()                   &&
          flag_loop                 && // NB: we check to avoid SIGTERM event...
          CSOCKET->iState != -EINTR && // NB: we check to avoid log spurious EINTR on accept() by any timer...
          CSOCKET->iState != -EAGAIN)
         {
         CSOCKET->setMsgError();

         if (u_buffer_len)
            {
            ULog::log("WARNING: accept new client failed %.*S", u_buffer_len, u_buffer);

            u_buffer_len = 0;
            }
         }
#  endif

#  ifdef U_EPOLLET_POSTPONE_STRATEGY
      if (CSOCKET->iState == -EAGAIN) U_ClientImage_state = U_PLUGIN_HANDLER_AGAIN;
#  endif

      goto end;
      }

   U_INTERNAL_ASSERT(CSOCKET->isConnected())

   CLIENT_ADDRESS     = UIPAddress::resolveStrAddress(iAddressType, CSOCKET->cRemoteAddress.pcAddress.p, CSOCKET->cRemoteAddress.pcStrAddress);
   CLIENT_ADDRESS_LEN = u__strlen(CLIENT_ADDRESS, __PRETTY_FUNCTION__);

   U_INTERNAL_DUMP("client_address = %.*S", CLIENT_ADDRESS_LEN, CLIENT_ADDRESS)

#if defined(_MSWINDOWS_) && !defined(USE_LIBEVENT)
   if (CSOCKET->iSockDesc >= FD_SETSIZE)
      {
      CSOCKET->abortive_close();

      U_SRV_LOG("WARNING: new client connected from %.*S, connection denied by FD_SETSIZE(%u)", CLIENT_ADDRESS_LEN, CLIENT_ADDRESS, FD_SETSIZE);

      goto end;
      }
#endif

#ifdef U_ACL_SUPPORT
   if (vallow_IP &&
       UIPAllow::isAllowed(CSOCKET->remoteIPAddress().getInAddr(), *vallow_IP) == false)
      {
      CSOCKET->abortive_close();

      /**
       * Instructs server to accept connections from the IP address IPADDR. A CIDR mask length can be supplied optionally after
       * a trailing slash, e.g. 192.168.0.0/24, in which case addresses that match in the most significant MASK bits will be allowed.
       * If no options are specified, all clients are allowed. Unauthorized connections are rejected by closing the TCP connection
       * immediately. A warning is logged on the server but nothing is sent to the client
       */

      U_SRV_LOG("WARNING: new client connected from %.*S, connection denied by Access Control List", CLIENT_ADDRESS_LEN, CLIENT_ADDRESS);

#  if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
      if (preforked_num_kids != -1)
#  endif
      {
      U_INTERNAL_ASSERT_DIFFERS(socket_flags & O_NONBLOCK, 0)

#  ifndef USE_LIBEVENT
      goto try_next;
#  endif
      }

      goto next;
      }
#endif

#ifdef U_RFC1918_SUPPORT
   if (public_address                         &&
       enable_rfc1918_filter                  &&
       CSOCKET->remoteIPAddress().isPrivate() &&
       (vallow_IP_prv == 0 ||
        UIPAllow::isAllowed(CSOCKET->remoteIPAddress().getInAddr(), *vallow_IP_prv) == false))
      {
      CSOCKET->abortive_close();

      U_SRV_LOG("WARNING: new client connected from %.*S, connection denied by RFC1918 filtering (reject request from private IP to public server address)",
                     CLIENT_ADDRESS_LEN, CLIENT_ADDRESS); 

#  if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
      if (preforked_num_kids != -1)
#  endif
      {
      U_INTERNAL_ASSERT_DIFFERS(socket_flags & O_NONBLOCK, 0)

#  ifndef USE_LIBEVENT
      goto try_next;
#  endif
      }

      goto next;
      }
#endif

#ifdef U_LOG_ENABLE
   U_TOT_CONNECTION++;

   U_INTERNAL_DUMP("U_TOT_CONNECTION = %u", U_TOT_CONNECTION)
#endif

   ++UNotifier::num_connection;

#ifdef DEBUG
   ++stats_connections;

   numc = UNotifier::num_connection -
          UNotifier::min_connection;

   if (max_depth          < numc)          max_depth = numc;
   if (stats_simultaneous < numc) stats_simultaneous = numc;

   U_INTERNAL_DUMP("numc = %u max_depth = %u stats_simultaneous = %u", numc, max_depth, stats_simultaneous)
#endif

   /**
    * PREFORK_CHILD number of child server processes created at startup:
    *
    * -1 - thread approach (experimental)
    *  0 - serialize, no forking
    *  1 - classic, forking after accept client
    * >1 - pool of process serialize plus monitoring process
    */

#ifdef U_CLASSIC_SUPPORT
   if (isClassic())
      {
      if (proc->fork() &&
          proc->parent())
         {
         int status;

         CSOCKET->close();

         U_SRV_LOG("Started new child (pid %d), up to %u children", proc->pid(), UNotifier::num_connection - UNotifier::min_connection);

retry:   pid = UProcess::waitpid(-1, &status, WNOHANG); // NB: to avoid too much zombie...

         if (pid > 0)
            {
            char buffer[128];

            --UNotifier::num_connection;

            U_SRV_LOG("Child (pid %d) exited with value %d (%s), down to %u children",
                              pid, status, UProcess::exitInfo(buffer, status), UNotifier::num_connection - UNotifier::min_connection);

            goto retry;
            }

#     ifdef U_LOG_ENABLE
         if (isLog()) ULog::log("Waiting for connection on port %u", port);
#     endif

         U_RETURN(U_NOTIFIER_OK);
         }

      if (proc->child()) UNotifier::init(false);
      }
#endif

#ifdef U_LOG_ENABLE
   if (isLog())
      {
#  ifdef USE_LIBSSL
      if (bssl) CLIENT_INDEX->logCertificate();
#  endif

      USocketExt::setRemoteInfo(CSOCKET, *CLIENT_INDEX->logbuf);

      U_INTERNAL_ASSERT(CLIENT_INDEX->logbuf->isNullTerminated())

      char buffer[32];
      uint32_t len = getNumConnection(buffer);

      ULog::log("New client connected from %v, %.*s clients currently connected", CLIENT_INDEX->logbuf->rep, len, buffer);

#  ifdef U_WELCOME_SUPPORT
      if (msg_welcome) ULog::log("Sending welcome message to %v", CLIENT_INDEX->logbuf->rep);
#  endif
      }
#endif

#ifdef U_WELCOME_SUPPORT
   if (msg_welcome &&
       USocketExt::write(CSOCKET, *msg_welcome, timeoutMS) == false)
      {
      CSOCKET->abortive_close();

      CLIENT_INDEX->UClientImage_Base::handlerDelete();

      goto next;
      }
#endif

#if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   if (preforked_num_kids == -1) lClientIndex->UEventFd::fd = psocket->iSockDesc;
   else
#endif
   {
   if (CLIENT_IMAGE_HANDLER_READ == false) goto next;
   }

   U_INTERNAL_ASSERT(CSOCKET->isOpen())
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, 1) // 1 => child of parallelization

#if defined(HAVE_EPOLL_CTL_BATCH) && !defined(USE_LIBEVENT)
   UNotifier::batch((UEventFd*)CLIENT_INDEX);
#else
   UNotifier::insert((UEventFd*)CLIENT_INDEX);
#endif

   if (++CLIENT_INDEX >= eClientImage) CLIENT_INDEX = vClientImage;

next:
   last_event = u_now->tv_sec;

#ifdef USE_LIBEVENT
   goto end;
#endif

#if defined(ENABLE_THREAD) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   if (preforked_num_kids != -1)
#endif
   {
   U_INTERNAL_ASSERT_DIFFERS(socket_flags & O_NONBLOCK, 0)

   U_INTERNAL_DUMP("cround = %u UNotifier::num_connection = %u num_client_threshold = %u", cround, UNotifier::num_connection, num_client_threshold)

   if (num_client_threshold > UNotifier::num_connection) cround = 0;
   else
      {
      cround = 2;

      CLIENT_INDEX = vClientImage;

#  ifdef DEBUG
      U_WARNING("It has passed the client threshold(%u): preallocation(%u) num_connection(%u)",
                  num_client_threshold, UNotifier::max_connection, UNotifier::num_connection - UNotifier::min_connection);
#  endif
      }

   goto loop;
   }

end:
#if defined(HAVE_EPOLL_CTL_BATCH) && !defined(USE_LIBEVENT)
   UNotifier::insertBatch();
#endif

   nClientIndex = CLIENT_INDEX - vClientImage;

   U_INTERNAL_DUMP("nClientIndex = %u", nClientIndex)

   U_INTERNAL_ASSERT_MINOR(nClientIndex, UNotifier::max_connection)

   U_RETURN(U_NOTIFIER_OK);
}

#undef CSOCKET
#undef CLIENT_INDEX
#undef CLIENT_ADDRESS
#undef CLIENT_ADDRESS_LEN
#undef CLIENT_IMAGE_HANDLER_READ

#ifdef U_LOG_ENABLE
bool UServer_Base::called_from_handlerTime;

uint32_t UServer_Base::getNumConnection(char* ptr)
{
   U_TRACE(0, "UServer_Base::getNumConnection(%p)", ptr)

   uint32_t len;

   if (preforked_num_kids <= 0) len = u_num2str32(ptr, UNotifier::num_connection - UNotifier::min_connection - 1);
   else
      {
      char* start = ptr;

      *ptr  = '(';
       ptr += 1+u_num2str32(ptr+1, UNotifier::num_connection - UNotifier::min_connection - 1);
      *ptr  = '/';
       ptr += 1+u_num2str32(ptr+1, U_TOT_CONNECTION - flag_loop); // NB: check for SIGTERM event...
      *ptr  = ')';

      len = ptr-start+1;
      }

   U_RETURN(len);
}
#endif

bool UServer_Base::handlerTimeoutConnection(void* cimg)
{
   U_TRACE(0, "UServer_Base::handlerTimeoutConnection(%p)", cimg)

   U_INTERNAL_ASSERT_POINTER(cimg)
   U_INTERNAL_ASSERT_POINTER(pthis)
   U_INTERNAL_ASSERT_POINTER(ptime)
   U_INTERNAL_ASSERT_DIFFERS(timeoutMS, -1)

   U_INTERNAL_DUMP("pthis = %p handler_other = %p handler_inotify = %p ", pthis, handler_other, handler_inotify)

   if (cimg == pthis         ||
       cimg == handler_other ||
       cimg == handler_inotify)
      {
      U_RETURN(false);
      }

   if (((UClientImage_Base*)cimg)->handlerTimeout() == U_NOTIFIER_DELETE)
      {
#  ifdef U_LOG_ENABLE
      if (isLog())
         {
         if (called_from_handlerTime)
            {
            ULog::log("%shandlerTime: client connected didn't send any request in %u secs (timeout), close connection %v",
                        UServer_Base::mod_name[0], ptime->UTimeVal::tv_sec, ((UClientImage_Base*)cimg)->logbuf->rep);
            }
         else
            {
            U_INTERNAL_ASSERT_EQUALS(cimg, pClientImage)

            ULog::log("%shandlerTimeoutConnection: client connected didn't send any request in %u secs (timeout), close connection %v",
                        UServer_Base::mod_name[0], last_event - ((UClientImage_Base*)cimg)->last_event, ((UClientImage_Base*)cimg)->logbuf->rep);
            }
         }
#  endif

      U_RETURN(true); // NB: return true mean that we want erase the item...
      }

   U_RETURN(false);
}

void UServer_Base::runLoop(const char* user)
{
   U_TRACE(0, "UServer_Base::runLoop(%S)", user)

   socket->reusePort(socket_flags);

#ifndef _MSWINDOWS
   if (bipc == false)
      {
      U_ASSERT_EQUALS(socket->isUDP(), false)

      /**
       * Let's say an application just issued a request to send a small block of data. Now, we could
       * either send the data immediately or wait for more data. Some interactive and client-server
       * applications will benefit greatly if we send the data right away. For example, when we are
       * sending a short request and awaiting a large response, the relative overhead is low compared
       * to the total amount of data transferred, and the response time could be much better if the
       * request is sent immediately. This is achieved by setting the TCP_NODELAY option on the socket,
       * which disables the Nagle algorithm.
       *
       * Another way to prevent delays caused by sending useless packets is to use the TCP_QUICKACK option.
       * This option is different from TCP_DEFER_ACCEPT, as it can be used not only to manage the process of
       * connection establishment, but it can be used also during the normal data transfer process. In addition,
       * it can be set on either side of the client-server connection. Delaying sending of the ACK packet could
       * be useful if it is known that the user data will be sent soon, and it is better to set the ACK flag on
       * that data packet to minimize overhead. When the sender is sure that data will be immediately be sent
       * (multiple packets), the TCP_QUICKACK option can be set to 0. The default value of this option is 1 for
       * sockets in the connected state, which will be reset by the kernel to 1 immediately after the first use.
       * (This is a one-time option)
       *
       * Linux (along with some other OSs) includes a TCP_DEFER_ACCEPT option in its TCP implementation.
       * Set on a server-side listening socket, it instructs the kernel not to wait for the final ACK packet
       * and not to initiate the process until the first packet of real data has arrived. After sending the SYN/ACK,
       * the server will then wait for a data packet from a client. Now, only three packets will be sent over the
       * network, and the connection establishment delay will be significantly reduced, which is typical for HTTP.
       * NB: Takes an integer value (seconds)
       */

#  if (defined(LINUX) || defined(__LINUX__) || defined(__linux__)) && !defined(U_SERVER_CAPTIVE_PORTAL)
                                 socket->setTcpFastOpen();
                                 socket->setTcpDeferAccept();
      if (bssl == false)         socket->setBufferSND(min_size_for_sendfile);
      if (set_tcp_keep_alive )   socket->setTcpKeepAlive(); 
#  endif
      if (tcp_linger_set > -2)   socket->setTcpLinger(tcp_linger_set);
      }
#endif

#ifndef _MSWINDOWS_
   if (user)
      {
      if (u_runAsUser(user, false) == false) U_ERROR("set user %S context failed", user);

      U_SRV_LOG("Server run with user %S permission", user);
      }
   else if (USocket::iBackLog != 1)
      {
      // We don't need these anymore.
      // Good security policy says we get rid of them

      u_never_need_root();
      u_never_need_group();
      }
#endif

   pthis->UEventFd::fd = socket->iSockDesc;

   UNotifier::init(false);

   U_INTERNAL_DUMP("UNotifier::min_connection = %d", UNotifier::min_connection)

   if (UNotifier::min_connection)
      {
      if (binsert)         UNotifier::insert(pthis);           // NB: we ask to be notified for request of connection (=> accept)
      if (handler_other)   UNotifier::insert(handler_other);   // NB: we ask to be notified for request from generic system
      if (handler_inotify) UNotifier::insert(handler_inotify); // NB: we ask to be notified for change of file system (=> inotify)
      }

#ifdef U_LOG_ENABLE
   if (isLog()) ULog::log("Waiting for connection on port %u", port);
#endif

#if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   if (preforked_num_kids == -1)
      {
      U_INTERNAL_ASSERT_EQUALS(UNotifier::pthread, 0)

      UNotifier::pthread = U_NEW(UClientThread);

#  ifdef _MSWINDOWS_
      InitializeCriticalSection(&UNotifier::mutex);
#  endif

      UNotifier::pthread->start(50);

      proc->_pid = UNotifier::pthread->id;

      U_ASSERT(proc->parent())
      }
#  ifdef DEBUG
   else
      {
      U_INTERNAL_ASSERT_EQUALS(UNotifier::pthread, 0)
      }
#  endif
#endif

#if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
   (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_UNBLOCK, &mask, 0);
#endif

   while (flag_loop)
      {
      U_INTERNAL_ASSERT_EQUALS(UInterrupt::event_signal_pending, 0)

      U_INTERNAL_DUMP("ptime = %p handler_other = %p handler_inotify = %p UNotifier::num_connection = %u UNotifier::min_connection = %u",
                       ptime,     handler_other,     handler_inotify,     UNotifier::num_connection,     UNotifier::min_connection)

#  if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
      if (preforked_num_kids != -1)
#  endif
      {
      if (UNotifier::min_connection) // NB: we need to notify someone for something...
         {
         if (ptime == 0) UNotifier::waitForEvent();
         else
            {
            bool some_client = (UNotifier::num_connection > UNotifier::min_connection);

            if (some_client) // NB: we must feel the possibly timeout for request from the client...
               {
               UTimer::insert(ptime);

               last_event = u_now->tv_sec;
               }

            UNotifier::waitForEvent();

            if (some_client &&
                UNotifier::nfd_ready > 0)
               {
               UTimer::erase(ptime);
               }
            }

         U_ASSERT_EQUALS(UNotifier::empty(), false)

         continue;
         }
      }

      // NB: we go directly on accept() and block on it...

      U_INTERNAL_ASSERT_EQUALS(socket_flags & O_NONBLOCK, 0)

#  if !defined(ENABLE_THREAD) || defined(USE_LIBEVENT) || !defined(U_SERVER_THREAD_APPROACH_SUPPORT)
      U_INTERNAL_ASSERT(UNotifier::min_connection == UNotifier::num_connection)
#  endif

      (void) pthis->UServer_Base::handlerRead();
      }
}

void UServer_Base::run()
{
   U_TRACE_NO_PARAM(1, "UServer_Base::run()")

   U_INTERNAL_ASSERT_POINTER(pthis)

   init();

   int status;
   const char* user = (as_user->empty() ? 0 : as_user->data());

   /**
    * PREFORK_CHILD number of child server processes created at startup:
    *
    * -1 - thread approach (experimental)
    *  0 - serialize, no forking
    *  1 - classic, forking after accept client
    * >1 - pool of process serialize plus monitoring process
    */

   if (monitoring_process)
      {
      /**
       * Main loop for the parent process with the new preforked implementation.
       * The parent is just responsible for keeping a pool of children and they accept connections themselves...
       */

      int nkids;
      cpu_set_t cpuset;
      pid_t pid_to_wait;
      bool baffinity = false;
      UTimeVal to_sleep(0L, 500L * 1000L);

#  if !defined(U_SERVER_CAPTIVE_PORTAL) && defined(HAVE_SCHED_GETAFFINITY)
      if (preforked_num_kids <= u_get_num_cpu() &&
          u_num_cpu > 1)
         {
         baffinity = true;

         U_SRV_LOG("cpu affinity is to be set; thread count (%d) <= cpu count (%d)", preforked_num_kids, u_num_cpu);
         }
#  endif

      U_INTERNAL_ASSERT_EQUALS(rkids, 0)

      if (preforked_num_kids <= 0) nkids = 1;
      else
         {
         pid_to_wait  = -1;
         nkids        = preforked_num_kids;
         }

      U_INTERNAL_DUMP("nkids = %d", nkids)

      while (flag_loop)
         {
         u_need_root(false);

         while (rkids < nkids)
            {
            if (proc->fork() &&
                proc->parent())
               {
               ++rkids;

               U_INTERNAL_DUMP("up to %u children, UNotifier::num_connection = %d", rkids, UNotifier::num_connection)

               pid = proc->pid();

               CPU_ZERO(&cpuset);

               if (baffinity) u_bind2cpu(&cpuset, pid, rkids); // Pin the process to a particular cpu...

               U_SRV_LOG("Started new child (pid %d), up to %u children, affinity mask: %x", pid, rkids, CPUSET_BITS(&cpuset)[0]);

               if (preforked_num_kids <= 0) pid_to_wait = pid;

               if (set_realtime_priority) u_switch_to_realtime_priority(pid);
               }

            if (proc->child())
               {
               U_INTERNAL_DUMP("child = %P UNotifier::num_connection = %d", UNotifier::num_connection)

#           ifdef HAVE_LIBNUMA
               if (U_SYSCALL_NO_PARAM(numa_max_node))
                  {
                  struct bitmask* bmask = (struct bitmask*) U_SYSCALL(numa_bitmask_alloc, "%u", 16);

                  (void) U_SYSCALL(numa_bitmask_setbit, "%p,%u", bmask, rkids % 2);

                  U_SYSCALL_VOID(numa_set_membind,  "%p", bmask);
                  U_SYSCALL_VOID(numa_bitmask_free, "%p", bmask);
                  }
#           endif

               /**
                * POSIX.1-1990 disallowed setting the action for SIGCHLD to SIG_IGN.
                * POSIX.1-2001 allows this possibility, so that ignoring SIGCHLD can be
                * used to prevent the creation of zombies (see wait(2)). Nevertheless, the
                * historical BSD and System V behaviors for ignoring SIGCHLD differ, so that
                * the only completely portable method of ensuring that terminated children do
                * not become zombies is to catch the SIGCHLD signal and perform a wait(2) or similar.
                *
                * NB: we cannot use SIGCHLD to avoid zombie for parallelization because in this way we
                *     interfere with waiting of cgi-bin processing that write on pipe and also to know
                *     exit status from script...
                *
                * UInterrupt::setHandlerForSignal(SIGCHLD, (sighandler_t)UServer_Base::handlerForSigCHLD);
                */

               // NB: we can't use UInterrupt::erase() because it restore the old action (UInterrupt::init)...
               UInterrupt::setHandlerForSignal(SIGHUP, (sighandler_t)SIG_IGN);

               if (pluginsHandlerFork() != U_PLUGIN_HANDLER_FINISHED) U_ERROR("Plugins stage fork failed");

               runLoop(user);

               return;
               }

            // Don't start them too quickly, or we might overwhelm a machine that's having trouble

            to_sleep.nanosleep();

#        ifdef U_SERVER_CAPTIVE_PORTAL
            if (proc->_pid == -1)
               {
               monitoring_process = false;

               goto no_monitoring_process;
               }
#        endif
            }

         // wait for any children to exit, and then start some more

#     if defined(ENABLE_THREAD) && !defined(_MSWINDOWS_)
         (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_UNBLOCK, &mask, 0);
#     endif

         u_dont_need_root();

         pid = UProcess::waitpid(pid_to_wait, &status, 0);

         U_INTERNAL_DUMP("rkids = %d", rkids)

         if (rkids == 0)  // NB: check for SIGHUP event...
            {
            manageSigHUP();

            continue;
            }

         if (pid > 0 &&
             flag_loop) // NB: check for SIGTERM event...
            {
            U_INTERNAL_ASSERT_MAJOR(rkids, 0)

            --rkids;

            baffinity = false;

            U_INTERNAL_DUMP("down to %u children", rkids)

            // Another little safety brake here: since children should not
            // exit too quickly, pausing before starting them should be harmless

            if (USemaphore::checkForDeadLock(to_sleep) == false) to_sleep.nanosleep();

#        ifdef U_LOG_ENABLE
            if (isLog())
               {
               char buffer[128];

               ULog::log("%sWARNING: child (pid %d) exited with value %d (%s), down to %u children", mod_name[0], pid, status, UProcess::exitInfo(buffer, status), rkids);
               }
#        endif
            }
         }

      U_INTERNAL_ASSERT(proc->parent())
      }
   else
      {
#  ifdef U_SERVER_CAPTIVE_PORTAL
no_monitoring_process:
      if (pluginsHandlerFork() != U_PLUGIN_HANDLER_FINISHED) U_ERROR("Plugins stage fork failed");
#  endif

      runLoop(user);
      }

   if (pluginsHandlerStop() != U_PLUGIN_HANDLER_FINISHED) U_WARNING("Plugins stage stop failed");

   status = proc->waitAll(2);

   if (status >= U_FAILED_SOME)
      {
      UProcess::kill(0, SIGKILL); // SIGKILL is sent to every process in the process group of the calling process...

      (void) proc->waitAll(2);
      }

#ifdef U_LOG_ENABLE
   if (isLog() &&
       flag_sigterm)
      {
      logMemUsage("SIGTERM");
      }
#endif

   if (proc->parent() ||
       preforked_num_kids <= 0)
      {
      closeLog();
      }

#ifdef DEBUG
   pthis->deallocate();
#endif
}

void UServer_Base::removeZombies()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::removeZombies()")

   U_INTERNAL_ASSERT_POINTER(ptr_shared_data)

#ifndef U_LOG_ENABLE
         (void) UProcess::removeZombies();
#else
   uint32_t n = UProcess::removeZombies();

   if (n) U_SRV_LOG("removed %u zombies - current parallelization (%d)", n, U_CNT_PARALLELIZATION);
#endif
}

// it creates a copy of itself, return true if parent...

pid_t UServer_Base::startNewChild()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::startNewChild()")

   UProcess p;

   if (p.fork() &&
       p.parent())
      {
      pid_t pid = p.pid();

#  ifndef U_LOG_ENABLE
            (void) UProcess::removeZombies();
#  else
      uint32_t n = UProcess::removeZombies();

      U_CNT_PARALLELIZATION++;

      U_SRV_LOG("Started new child (pid %d) for parallelization (%d) - removed %u zombies", pid, U_CNT_PARALLELIZATION, n);
#  endif

      U_RETURN(pid); // parent
      }

   if (p.child()) U_RETURN(0);

   U_RETURN(-1);
}

__noreturn void UServer_Base::endNewChild()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::endNewChild()")

#ifdef DEBUG
   UInterrupt::setHandlerForSignal(SIGHUP,  (sighandler_t)SIG_IGN);
   UInterrupt::setHandlerForSignal(SIGTERM, (sighandler_t)SIG_IGN);
#endif

#ifdef U_LOG_ENABLE
   if (LIKELY(U_CNT_PARALLELIZATION)) U_CNT_PARALLELIZATION--;

   U_INTERNAL_DUMP("cnt_parallelization = %d", U_CNT_PARALLELIZATION)

   U_SRV_LOG("child for parallelization ended (%d)", U_CNT_PARALLELIZATION);
#endif

   U_EXIT(0);
}

__pure bool UServer_Base::isParallelizationGoingToStart(uint32_t nclient)
{
   U_TRACE(0, "UServer_Base::isParallelizationGoingToStart(%u)", nclient)

   U_INTERNAL_ASSERT_POINTER(ptr_shared_data)

   U_INTERNAL_DUMP("U_ClientImage_pipeline = %b U_ClientImage_parallelization = %d UNotifier::num_connection - UNotifier::min_connection = %d",
                    U_ClientImage_pipeline,     U_ClientImage_parallelization,     UNotifier::num_connection - UNotifier::min_connection)

#ifndef U_SERVER_CAPTIVE_PORTAL
   if (U_ClientImage_parallelization != 1 && // 1 => child of parallelization
       (UNotifier::num_connection - UNotifier::min_connection) > nclient)
      {
      U_INTERNAL_DUMP("U_ClientImage_close = %b", U_ClientImage_close)

      U_RETURN(true);
      }
#endif

   U_RETURN(false);
}

bool UServer_Base::startParallelization(uint32_t nclient)
{
   U_TRACE(0, "UServer_Base::startParallelization(%u)", nclient)

   if (isParallelizationGoingToStart(nclient))
      {
      pid_t pid = startNewChild();

      if (pid > 0)
         {
         // NB: from now it is responsability of the child to services the request from the client on the same connection...

         U_ClientImage_close = true;
         U_ClientImage_parallelization = 2; // 2 => parent of parallelization

         UClientImage_Base::setRequestProcessed();

         if (U_ClientImage_pipeline) UClientImage_Base::resetPipeline();

         U_ASSERT(isParallelizationParent())

         U_RETURN(true);
         }

      if (pid == 0)
         {
         U_ClientImage_parallelization = 1; // 1 => child of parallelization

         U_ASSERT(isParallelizationChild())
         }
      }

   U_INTERNAL_DUMP("U_ClientImage_close = %b", U_ClientImage_close)

   U_RETURN(false);
}

void UServer_Base::logCommandMsgError(const char* cmd, bool balways)
{
   U_TRACE(0, "UServer_Base::logCommandMsgError(%S,%b)", cmd, balways)

#ifdef U_LOG_ENABLE
   if (isLog())
      {
      if (UCommand::setMsgError(cmd, !balways) || balways) ULog::log("%s%.*s", mod_name[0], u_buffer_len, u_buffer);

      errno        = 0;
      u_buffer_len = 0;
      }
#endif
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UServer_Base::dump(bool reset) const
{
   *UObjectIO::os << "port                      " << port                       << '\n'
                  << "map_size                  " << map_size                   << '\n'
                  << "flag_loop                 " << flag_loop                  << '\n'
                  << "timeoutMS                 " << timeoutMS                  << '\n'
                  << "last_event                " << last_event                 << '\n'
                  << "verify_mode               " << verify_mode                << '\n'
                  << "shared_data_add           " << shared_data_add            << '\n'
                  << "ptr_shared_data           " << (void*)ptr_shared_data     << '\n'
                  << "preforked_num_kids        " << preforked_num_kids         << '\n'
                  << "log           (ULog       " << (void*)log                 << ")\n"
                  << "socket        (USocket    " << (void*)socket              << ")\n"
                  << "host          (UString    " << (void*)host                << ")\n"
                  << "dh_file       (UString    " << (void*)dh_file             << ")\n"
                  << "ca_file       (UString    " << (void*)ca_file             << ")\n"
                  << "ca_path       (UString    " << (void*)ca_path             << ")\n"
#              ifdef U_ACL_SUPPORT
                  << "allow_IP      (UString    " << (void*)allow_IP            << ")\n"
#              endif
                  << "key_file      (UString    " << (void*)key_file            << ")\n"
                  << "password      (UString    " << (void*)password            << ")\n"
                  << "cert_file     (UString    " << (void*)cert_file           << ")\n"
                  << "name_sock     (UString    " << (void*)name_sock           << ")\n"
                  << "IP_address    (UString    " << (void*)IP_address          << ")\n"
#              ifdef U_WELCOME_SUPPORT
                  << "msg_welcome   (UString    " << (void*)msg_welcome         << ")\n"
#              endif
                  << "document_root (UString    " << (void*)document_root       << ")\n"
                  << "proc          (UProcess   " << (void*)proc                << ')';

   if (reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
