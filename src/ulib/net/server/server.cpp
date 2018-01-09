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
#include <ulib/net/client/http.h>
#include <ulib/net/client/smtp.h>
#include <ulib/dynamic/dynamic.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/server.h>

#ifdef _MSWINDOWS_
#  include <ws2tcpip.h>
#else
#  include <pwd.h>
#  include <sys/prctl.h>
#  include <ulib/net/unixsocket.h>
#  ifdef HAVE_SCHED_GETCPU
#     include <sched.h>
#  endif
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
int           UServer_Base::timeoutMS;
int           UServer_Base::verify_mode;
int           UServer_Base::socket_flags;
int           UServer_Base::iAddressType;
int           UServer_Base::tcp_linger_set = -2;
int           UServer_Base::preforked_num_kids;
bool          UServer_Base::bssl;
bool          UServer_Base::bipc;
bool          UServer_Base::budp;
bool          UServer_Base::binsert;
bool          UServer_Base::flag_loop;
bool          UServer_Base::public_address;
bool          UServer_Base::monitoring_process;
bool          UServer_Base::set_tcp_keep_alive;
bool          UServer_Base::set_realtime_priority;
bool          UServer_Base::update_date;
bool          UServer_Base::update_date1;
bool          UServer_Base::update_date2;
bool          UServer_Base::update_date3;
bool          UServer_Base::called_from_handlerTime;
long          UServer_Base::last_time_email_crash;
char          UServer_Base::mod_name[2][16];
ULog*         UServer_Base::log;
ULog*         UServer_Base::apache_like_log;
char*         UServer_Base::client_address;
ULock*        UServer_Base::lock_user1;
ULock*        UServer_Base::lock_user2;
uint32_t      UServer_Base::vplugin_size;
uint32_t      UServer_Base::nClientIndex;
uint32_t      UServer_Base::crash_count;
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
UString*      UServer_Base::crashEmailAddress;
USocket*      UServer_Base::socket;
USocket*      UServer_Base::csocket;
UProcess*     UServer_Base::proc;
UEventFd*     UServer_Base::handler_other;
UEventFd*     UServer_Base::handler_inotify;
UEventTime*   UServer_Base::ptime;
const char*   UServer_Base::document_root_ptr;
unsigned int  UServer_Base::port;
USmtpClient*  UServer_Base::emailClient;
UFileConfig*  UServer_Base::cfg;
UServer_Base* UServer_Base::pthis;

uint32_t                   UServer_Base::map_size;
uint32_t                   UServer_Base::shared_data_add;
UServer_Base::shared_data* UServer_Base::ptr_shared_data;

uint32_t                   UServer_Base::shm_size;
uint32_t                   UServer_Base::shm_data_add;
UServer_Base::shm_data*    UServer_Base::ptr_shm_data;

UVector<UString>*                 UServer_Base::vplugin_name;
UVector<UString>*                 UServer_Base::vplugin_name_static;
UClientImage_Base*                UServer_Base::vClientImage;
UClientImage_Base*                UServer_Base::pClientImage;
UClientImage_Base*                UServer_Base::eClientImage;
UVector<UServerPlugIn*>*          UServer_Base::vplugin;
UVector<UServerPlugIn*>*          UServer_Base::vplugin_static;
UVector<UServer_Base::file_LOG*>* UServer_Base::vlog;

#ifdef USERVER_UDP
vPFi UServer_Base::runDynamicPage_udp;
#endif
#ifdef U_WELCOME_SUPPORT
UString* UServer_Base::msg_welcome;
#endif
#ifdef USE_LOAD_BALANCE
UString*            UServer_Base::ifname;
unsigned char       UServer_Base::loadavg_threshold = 45; // => 4.5
UVector<UIPAllow*>* UServer_Base::vallow_cluster;
#endif
#ifdef U_ACL_SUPPORT
UVector<UIPAllow*>* UServer_Base::vallow_IP;
#endif
#ifdef U_RFC1918_SUPPORT
bool                UServer_Base::enable_rfc1918_filter;
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
#  ifndef U_LOG_DISABLE
long     UServer_Base::last_event;
#  endif
uint32_t UServer_Base::nread;
uint32_t UServer_Base::max_depth;
uint32_t UServer_Base::nread_again;
uint64_t UServer_Base::stats_bytes;
uint32_t UServer_Base::stats_connections;
uint32_t UServer_Base::stats_simultaneous;
uint32_t UServer_Base::wakeup_for_nothing;

UString UServer_Base::getStats()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::getStats()")

   UString x(U_CAPACITY);

   x.snprintf(U_CONSTANT_TO_PARAM("%4u connections (%5.2f/sec), %3u max simultaneous, %4u %s (%5.2f/sec) - %v/sec"), UServer_Base::stats_connections,
               (float) UServer_Base::stats_connections / U_ONE_HOUR_IN_SECOND, UServer_Base::stats_simultaneous, UNotifier::nwatches, U_WHICH,
               (float) UNotifier::nwatches / U_ONE_HOUR_IN_SECOND, UStringExt::printSize(UServer_Base::stats_bytes).rep);

   U_RETURN_STRING(x);
}

class U_NO_EXPORT UTimeStat : public UEventTime {
public:

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
         U_DEBUG("%v", UServer_Base::getStats().rep)

         UServer_Base::stats_bytes = 0;
         }

      UNotifier::nwatches              =
      UServer_Base::stats_connections  =
      UServer_Base::stats_simultaneous = 0;

      U_RETURN(0); // monitoring
      }

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool _reset) const { return UEventTime::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UTimeStat)
};
#endif

class U_NO_EXPORT UTimeoutConnection : public UEventTime {
public:

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

      // there are idle connection... (timeout)

#  if !defined(U_LOG_DISABLE) || (!defined(USE_LIBEVENT) && defined(HAVE_EPOLL_WAIT) && defined(DEBUG))
      UServer_Base::called_from_handlerTime = true;
#  endif

#  ifndef U_LOG_DISABLE
      if (UServer_Base::isLog())
         {
#     ifdef DEBUG
         long delta = (u_now->tv_sec - UServer_Base::last_event) - UTimeVal::tv_sec;

         if (delta >=  1 ||
             delta <= -1)
            {
            U_SRV_LOG("handlerTime: server delta timeout exceed 1 sec: diff %ld sec", delta);
            }

         UServer_Base::last_event = u_now->tv_sec;
#     endif
         }
#  endif

      U_INTERNAL_DUMP("UNotifier::num_connection = %u UNotifier::min_connection = %u", UNotifier::num_connection, UNotifier::min_connection)

      if (UNotifier::num_connection > UNotifier::min_connection) UNotifier::callForAllEntryDynamic(UServer_Base::handlerTimeoutConnection);

#  ifndef U_LOG_DISABLE
      if (U_SRV_CNT_PARALLELIZATION)
#  endif
      UServer_Base::removeZombies();

      U_RETURN(0); // monitoring
      }

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool _reset) const { return UEventTime::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UTimeoutConnection)
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
bool                              UServer_Base::throttling_chk;
UString*                          UServer_Base::throttling_mask;
UServer_Base::uthrottling*        UServer_Base::throttling_rec;
URDBObjectHandler<UDataStorage*>* UServer_Base::db_throttling;

#define U_THROTTLE_TIME 2 // Time between updates of the throttle table's rolling averages

class U_NO_EXPORT UBandWidthThrottling : public UEventTime {
public:

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
         if (UServices::dosMatchWithOR(U_STRING_TO_PARAM(UServer_Base::pClientImage->uri), U_STRING_TO_PARAM(*key))) U_RETURN(4); // NB: call us later (after set record value from db)

         U_RETURN(1);
         }

      U_INTERNAL_DUMP("krate = %u min_limit = %u max_limit = %u num_sending = %u bytes_since_avg = %llu", UServer_Base::throttling_rec->krate, UServer_Base::throttling_rec->min_limit,
                        UServer_Base::throttling_rec->max_limit, UServer_Base::throttling_rec->num_sending, UServer_Base::throttling_rec->bytes_since_avg)

      U_INTERNAL_ASSERT_MAJOR(UServer_Base::throttling_rec->num_sending, 0)

      UServer_Base::throttling_rec->num_sending--;

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

#  ifndef U_LOG_DISABLE
      if (UServer_Base::isLog())
         {
         if (UServer_Base::throttling_rec->num_sending)
            {
            if (UServer_Base::throttling_rec->krate > UServer_Base::throttling_rec->max_limit)
               {
               UServer_Base::log->log(U_CONSTANT_TO_PARAM("throttle %V: krate %u %sexceeding limit %u; %u sending"), UServer_Base::db_throttling->getKeyID().rep,
                                      UServer_Base::throttling_rec->krate,
                                     (UServer_Base::throttling_rec->krate > UServer_Base::throttling_rec->max_limit * 2  ? "greatly " : ""),
                                      UServer_Base::throttling_rec->max_limit, UServer_Base::throttling_rec->num_sending);
               }

            if (UServer_Base::throttling_rec->krate < UServer_Base::throttling_rec->min_limit)
               {
               UServer_Base::log->log(U_CONSTANT_TO_PARAM("throttle %V: krate %u lower than minimum %u; %u sending"), UServer_Base::db_throttling->getKeyID().rep,
                                      UServer_Base::throttling_rec->krate,
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

      U_INTERNAL_ASSERT_MAJOR(UServer_Base::throttling_rec->num_sending, 0)

      uint32_t l = UServer_Base::throttling_rec->max_limit / UServer_Base::throttling_rec->num_sending;

      UServer_Base::pClientImage->max_limit = (UServer_Base::pClientImage->max_limit == U_NOT_FOUND
                                                   ?       l
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

      U_INTERNAL_DUMP("UServer_Base::pClientImage->min_limit = %u UServer_Base::pClientImage->max_limit = %u",
                       UServer_Base::pClientImage->min_limit,     UServer_Base::pClientImage->max_limit)

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

         U_INTERNAL_DUMP("UNotifier::num_connection = %u UNotifier::min_connection = %u", UNotifier::num_connection, UNotifier::min_connection)

         if (UNotifier::num_connection > UNotifier::min_connection) UNotifier::callForAllEntryDynamic(updateSendingRate);
         }

      U_RETURN(0); // monitoring
      }

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool _reset) const { return UEventTime::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UBandWidthThrottling)
};

class U_NO_EXPORT UClientThrottling : public UEventTime {
public:

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

      U_RETURN(-1); // normal
      }

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool _reset) const { return UEventTime::dump(_reset); }
#endif

protected:
   UClientImage_Base* pClientImage;

private:
   U_DISALLOW_COPY_AND_ASSIGN(UClientThrottling)
};

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

void UServer_Base::initThrottlingServer()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::initThrottlingServer()")

   U_INTERNAL_ASSERT(*throttling_mask)
   U_INTERNAL_ASSERT_EQUALS(db_throttling, U_NULLPTR)

   if (bssl)
      {
      U_WARNING("Sorry, we can't use bandwidth throttling with SSL"); // NB: we need to use sendfile()...
      }
   else
      {
      U_NEW(URDBObjectHandler<UDataStorage*>, db_throttling, URDBObjectHandler<UDataStorage*>(U_STRING_FROM_CONSTANT("../db/BandWidthThrottling"), -1, &throttling_rec, true));

      if (db_throttling->open(32 * 1024, false, true, true, U_SRV_LOCK_THROTTLING)) // NB: we don't want truncate (we have only the journal)...
         {
         char* ptr;
         UString pattern, number;
         UVector<UString> vec(*throttling_mask);

         U_SRV_LOG("db BandWidthThrottling initialization success: size(%u)", db_throttling->size());

         db_throttling->reset(); // Initialize the db to contain no entries

         UServer_Base::uthrottling rec = { 0, 0, 0, 0, 0 };

         for (int32_t i = 0, n = vec.size(); i < n; i += 2)
            {
            pattern = vec[i];
             number = vec[i+1];

                                                              rec.max_limit = ::strtol(number.data(),      &ptr, 10);
            if (ptr[0] == '-') rec.min_limit = rec.max_limit, rec.max_limit = ::strtol(        ptr+1, U_NULLPTR, 10);

            (void) db_throttling->insertDataStorage(&rec, sizeof(uthrottling), U_STRING_TO_PARAM(pattern), RDB_INSERT);
            }

         min_size_for_sendfile = 4096; // 4k
         }
      else
         {
         U_SRV_LOG("WARNING: db BandWidthThrottling initialization failed");

         delete db_throttling;
                db_throttling = U_NULLPTR;
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
         U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

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

         U_INTERNAL_DUMP("krate = %u elapsed = %u", krate, elapsed)

         if (krate > pClientImage->max_limit)
            {
            // how long should we wait to get back on schedule? If less than a second (integer math rounding), use 1/2 second

            uint32_t div = pClientImage->max_limit - elapsed;

            if (div == 0) div = 1;

            time_t coast = kbytes_sent / div;

            // set up the wakeup timer

            UClientThrottling* pc;

            U_NEW(UClientThrottling, pc, UClientThrottling(pClientImage, coast, coast ? 0 : U_SECOND / 2L));

            UTimer::insert(pc);

            U_RETURN(false);
            }
         }
      }

   U_RETURN(true);
}
#endif

/**
 * This code provide evasive action in the event of an HTTP DoS or DDoS attack or brute force attack. It is also designed to be a
 * detection tool, and can be easily configured to talk to ipchains, firewalls, routers, and etcetera. Detection is performed by
 * creating an internal dynamic hash table of IP Addresses and URIs, and denying any single IP address from any of the following:
 *
 * - Requesting the same page more than a few times per second
 * - Making more than 50 concurrent requests per second
 * - Making any requests while temporarily blacklisted (on a blocking list)
 *
 * This method has worked well in both single-server script attacks as well as distributed attacks, but just like other evasive tools,
 * is only as useful to the point of bandwidth and processor consumption (e.g. the amount of bandwidth and processor required to
 * receive/process/respond to invalid requests), which is why it's a good idea to integrate this with your firewalls and routers.
 *
 * This code has a built-in cleanup mechanism and scaling capabilities. Because of this, legitimate requests are rarely ever compromised,
 * only legitimate attacks. Even a user repeatedly clicking on 'reload' should not be affected unless they do it maliciously,
 *
 * A web hit request comes in. The following steps take place:
 *
 * - The IP address of the requestor is looked up on the temporary blacklist.
 * - The IP address of the requestor is hashed into a "key". A lookup is performed in the listener's internal hash table to determine if
 *   the same host has requested more than 50 objects within the past second.
 * - The IP address of the requestor and the URI are both hashed into a "key". A lookup is performed in the listener's internal hash table
 *   to determine if the same host has requested this page more than 3 within the past 1 second.
 *
 * If any of the above are true, a RESET is sent. This conserves bandwidth and system resources in the event of a DoS attack. Additionally,
 * a system command and/or an email notification can also be triggered to block all the originating addresses of a DDoS attack.
 *
 * Once a single RESET incident occurs, evasive code now blocks the entire IP address for a period of 10 seconds (configurable). If the host
 * requests a page within this period, it is forced to wait even longer. Since this is triggered from requesting the same URL multiple times
 * per second, this again does not affect legitimate users.
 *
 * The blacklist can/should be configured to talk to your network's firewalls and/or routers to push the attack out to the front lines, but
 * this is not required. This tool is *excellent* at fending off request-based DoS attacks or scripted attacks, and brute force attacks.
 * When integrated with firewalls or IP filters, evasive code can stand up to even large attacks. Its features will prevent you from wasting
 * bandwidth or having a few thousand CGI scripts running as a result of an attack.
 *
 * If you do not have an infrastructure capable of fending off any other types of DoS attacks, chances are this tool will only help you to the
 * point of your total bandwidth or server capacity for sending RESET's. Without a solid infrastructure and address filtering tool in place, a
 * heavy distributed DoS will most likely still take you offline
 */

#ifdef U_EVASIVE_SUPPORT
bool                              UServer_Base::bwhitelist;
UFile*                            UServer_Base::dos_LOG;
uint32_t                          UServer_Base::page_count;
uint32_t                          UServer_Base::site_count;
uint32_t                          UServer_Base::site_interval;
uint32_t                          UServer_Base::page_interval;
uint32_t                          UServer_Base::blocking_period;
UString*                          UServer_Base::dosEmailAddress;
UString*                          UServer_Base::systemCommand;
UVector<UIPAllow*>*               UServer_Base::vwhitelist_IP;
UServer_Base::uevasive*           UServer_Base::evasive_rec;
URDBObjectHandler<UDataStorage*>* UServer_Base::db_evasive;

void UServer_Base::initEvasive()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::initEvasive()")

   U_INTERNAL_ASSERT_EQUALS(db_evasive, U_NULLPTR)

   U_NEW(URDBObjectHandler<UDataStorage*>, db_evasive, URDBObjectHandler<UDataStorage*>(U_STRING_FROM_CONSTANT("../db/Evasive"), -1, &evasive_rec, true));

   // POSIX shared memory object: interprocess - can be used by unrelated processes (userver_tcp and userver_ssl)

   if (db_evasive->open(128 * U_1M, false, true, true, U_SHM_LOCK_EVASIVE)) // NB: we don't want truncate (we have only the journal)...
      {
      U_SRV_LOG("db Evasive initialization success: size(%u)", db_evasive->size());

      URDB::initRecordLock();

      db_evasive->reset(); // Initialize the db to contain no entries

      if (dos_LOG) (void) UServer_Base::addLog(dos_LOG);
      }
   else
      {
      U_SRV_LOG("WARNING: db Evasive initialization failed");

      delete db_evasive;
             db_evasive = U_NULLPTR;
      }
}

bool UServer_Base::checkHold(in_addr_t client)
{
   U_TRACE(0, "UServer_Base::checkHold(%u)", client)

   U_INTERNAL_ASSERT_POINTER(db_evasive)

   bool result = false;

   if ((bwhitelist = (vwhitelist_IP && UIPAllow::isAllowed(client, *vwhitelist_IP))) == false) // Check whitelist
      {
      U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

      // First see if the IP itself is on "hold"

      if (db_evasive->getDataStorage(client))
         {
         db_evasive->lockRecord();

         if ((u_now->tv_sec - evasive_rec->timestamp) < blocking_period)
            {
            result = true;

            evasive_rec->timestamp = u_now->tv_sec; // Make it wait longer in blacklist land
            }

         db_evasive->unlockRecord();
         }
      }

   U_RETURN(result);
}

bool UServer_Base::checkHitStats(const char* key, uint32_t key_len, uint32_t interval, uint32_t count)
{
   U_TRACE(0, "UServer_Base::checkHitStats(%.*S,%u,%u,%u)", key_len, key, key_len, interval, count)

   U_INTERNAL_ASSERT_POINTER(db_evasive)
   U_INTERNAL_ASSERT_EQUALS(bwhitelist, false)

   bool binterval;

   if (db_evasive->getDataStorage(key, key_len) == false)
      {
      UServer_Base::uevasive rec = { (uint32_t)u_now->tv_sec, 0 };

      (void) db_evasive->insertDataStorage(&rec, sizeof(uevasive), key, key_len, RDB_INSERT);

      U_RETURN(false);
      }

   db_evasive->lockRecord();

   binterval = ((u_now->tv_sec - evasive_rec->timestamp) >= interval);

   // If site/URI is being hit too much, add to "hold" list

   if (binterval ||
       evasive_rec->count < count)
      {
      if (binterval) evasive_rec->count = 0; // Reset our hit count list as necessary

      evasive_rec->count++;
      evasive_rec->timestamp = u_now->tv_sec;

      db_evasive->unlockRecord();

      U_RETURN(false);
      }

   U_INTERNAL_ASSERT_EQUALS(binterval, false)
   U_INTERNAL_ASSERT(evasive_rec->count >= count)

   evasive_rec->count     = 0;
   evasive_rec->timestamp = u_now->tv_sec;

   db_evasive->unlockRecord();

   in_addr_t client = UServer_Base::csocket->remoteIPAddress().getInAddr();

   if (db_evasive->getDataStorage(client)) evasive_rec->timestamp = u_now->tv_sec; // Make it wait longer in blacklist land
   else
      {
      UServer_Base::uevasive rec = { (uint32_t)u_now->tv_sec, 0 };

      (void) db_evasive->insertDataStorage(&rec, sizeof(uevasive), client, RDB_INSERT);
      }

   /*
   U_INTERNAL_DUMP("u_now->tv_sec = %ld evasive_rec->timestamp = %u evasive_rec->count = %u", u_now->tv_sec, evasive_rec->timestamp, evasive_rec->count)

   U_INTERNAL_ASSERT_EQUALS(evasive_rec->timestamp, (uint32_t)u_now->tv_sec)
   */

   char msg[4096];
   uint32_t msg_len;
   bool bmail = false;

   if (count == site_count)
      {
      msg_len = u__snprintf(msg, sizeof(msg), U_CONSTANT_TO_PARAM("(pid %P) it has requested more than %u objects within the past %u seconds"), site_count, site_interval);

      if (dosEmailAddress)
         {
         if ((u_now->tv_sec - U_SHM_LAST_TIME_EMAIL_DOS) > U_ONE_DAY_IN_SECOND)
            {
            bmail                     = true;
            U_SHM_LAST_TIME_EMAIL_DOS = u_now->tv_sec;
            }
         }
      }
   else
      {
      uint32_t sz;
      const char* ptr = UClientImage_Base::getRequestUri(sz);

      msg_len = u__snprintf(msg, sizeof(msg), U_CONSTANT_TO_PARAM("(pid %P) it has requested the same page %.*S more than %u within the past %u seconds"),
                            sz, ptr, page_count, page_interval);

      UClientImage_Base::abortive_close();
      }

   U_DEBUG("blacklisting address %.*S, possible DoS attack: %.*s", U_CLIENT_ADDRESS_TO_TRACE, msg_len, msg);

   U_SRV_LOG("WARNING: blacklisting address %.*S, possible DoS attack: %.*s", U_CLIENT_ADDRESS_TO_TRACE, msg_len, msg);

   if (dos_LOG) ULog::log(dos_LOG->getFd(), U_CONSTANT_TO_PARAM("blacklisting %.*S: %.*s"), U_CLIENT_ADDRESS_TO_TRACE, msg_len, msg);

   if (bmail ||
       systemCommand)
      {
      // Perform system functions and/or email notification

      pid_t pid = UServer_Base::startNewChild();

      if (pid > 0) U_RETURN(true); // parent

      // child

      if (systemCommand) manageCommand(U_STRING_TO_PARAM(*systemCommand), U_CLIENT_ADDRESS_TO_TRACE, msg_len, msg);

      if (bmail)
         {
         U_INTERNAL_ASSERT_POINTER(emailClient)

         UString body(100U);

         body.snprintf(U_CONSTANT_TO_PARAM("blacklisting address %.*S, possible DoS attack: %.*s"), U_CLIENT_ADDRESS_TO_TRACE, msg_len, msg);

         emailClient->sendEmail(*dosEmailAddress, U_STRING_FROM_CONSTANT("possible DoS attack"), body);
         }

      if (pid == 0) UServer_Base::endNewChild();
      }

   U_RETURN(true);
}

bool UServer_Base::checkHitSiteStats()
{
   U_TRACE_NO_PARAM(0+256, "UServer_Base::checkHitSiteStats()")

   U_INTERNAL_ASSERT_POINTER(db_evasive)

   if (bwhitelist == false)
      {
      char key[6] = { '_', 0, 0, 0, 0, '_' };

      u_put_unalignedp32(key+1, UServer_Base::csocket->remoteIPAddress().getInAddr());

      U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

      if (checkHitStats(key, sizeof(key), site_interval, site_count)) U_RETURN(true);
      }

   U_RETURN(false);
}

bool UServer_Base::checkHitUriStats()
{
   U_TRACE_NO_PARAM(0+256, "UServer_Base::checkHitUriStats()")

   U_INTERNAL_ASSERT_POINTER(db_evasive)

   if (bwhitelist == false)
      {
      uint32_t sz;
      const char* ptr = UClientImage_Base::getRequestUri(sz);

      if (sz != U_CONSTANT_SIZE("/favicon.ico")                                                &&
          u_get_unalignedp64(ptr)   != U_MULTICHAR_CONSTANT64('/','f','a','v','i','c','o','n') &&
          u_get_unalignedp32(ptr+8) != U_MULTICHAR_CONSTANT32('.','i','c','o'))
         {
         char key[260];
         uint32_t key_sz = (sz < 256 ? sz : 256),
                  addr = UServer_Base::csocket->remoteIPAddress().getInAddr();

         union uukey {
            char*     k;
            uint32_t* u;
         };

         union uukey ukey = { &key[0] };

         u_put_unalignedp32(ukey.u, addr);

         U_MEMCPY(key+4, ptr, key_sz);

         if (checkHitStats(key, key_sz, page_interval, page_count)) U_RETURN(true);
         }
      }

   U_RETURN(false);
}
#endif

#ifdef ENABLE_THREAD
#  include <ulib/thread.h>

class UClientThread : public UThread {
public:

   UClientThread() : UThread(PTHREAD_CREATE_DETACHED) {}

   virtual void run() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "UClientThread::run()")

      U_INTERNAL_ASSERT_EQUALS(UServer_Base::ptime, U_NULLPTR)

      while (UServer_Base::flag_loop) UNotifier::waitForEvent();
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UClientThread)
};

#  ifdef U_LINUX
class UTimeThread : public UThread {
public:

   UTimeThread() : UThread(PTHREAD_CREATE_DETACHED) {}

   virtual void run() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "UTimeThread::run()")

      U_SRV_LOG("UTimeThread optimization for time resolution of one second activated (tid %u)", u_gettid());

#   ifdef USE_LOAD_BALANCE
      int fd_sock = -1;
      uint32_t addr = 0;
      uusockaddr srv_addr, cli_addr;
      UUDPSocket udp_sock(UClientImage_Base::bIPv6);

#   if defined(USERVER_UDP) || defined(USERVER_IPC)
      if (UServer_Base::budp == false &&
          UServer_Base::bipc == false)
#   endif
      {
      if (UServer_Base::ifname == U_NULLPTR) U_NEW(UString, UServer_Base::ifname, UString(UServer_Base::getNetworkDevice(U_NULLPTR)));

      if (*UServer_Base::ifname)
         {
         udp_sock._socket();
         udp_sock.setReuseAddress();

         fd_sock = udp_sock.getFd();

         srv_addr.psaIP4Addr.sin_port        = htons(UServer_Base::port);
         srv_addr.psaIP4Addr.sin_family      = PF_INET;
         srv_addr.psaIP4Addr.sin_addr.s_addr = htonl(INADDR_ANY);

         if (U_SYSCALL(bind, "%d,%p,%d", fd_sock, &(srv_addr.psaGeneric), sizeof(uusockaddr))) U_ERROR("bind on udp socket failed");

         if (UIPAddress::setBroadcastAddress(srv_addr, *UServer_Base::ifname) == false)
            {
            U_ERROR("set broadcast address on interface %V failed", UServer_Base::ifname->rep);
            }

         if (udp_sock.setSockOpt(SOL_SOCKET, SO_BROADCAST, (const int[]){ 1 }) == false) U_ERROR("setting SO_BROADCAST on udp socket failed");

         udp_sock.setNonBlocking();

         addr = UServer_Base::socket->cLocalAddress.get_addr();

         U_DUMP("addr = %V", UIPAddress::toString(addr).rep)

         U_SRV_LOG("Load balance activated (by udp socket: %u): loadavg_threshold = %u brodacast address = (%v:%v)",
                     fd_sock, UServer_Base::loadavg_threshold, UServer_Base::ifname->rep, UIPAddress::toString(srv_addr.psaIP4Addr.sin_addr.s_addr).rep);
         }
      }
#    endif

      u_gettimenow();

      struct timespec ts = { 0L, 0L };
      uint32_t lnow, sec = u_now->tv_sec;

      while (UServer_Base::flag_loop)
         {
         ts.tv_nsec = 10000L + (U_SECOND - u_now->tv_usec) * 1000L;

         U_INTERNAL_DUMP("u_now->tv_sec = %ld u_now->tv_usec = %ld ts.tv_nsec = %ld", u_now->tv_sec, u_now->tv_usec, ts.tv_nsec)

         if (U_SYSCALL(nanosleep, "%p,%p", &ts, U_NULLPTR) == -1)
            {
            U_INTERNAL_ASSERT_EQUALS(errno, EINTR)

            U_INTERNAL_DUMP("UServer_Base::flag_loop = %b", UServer_Base::flag_loop)

            u_gettimenow();

            continue;
            }

#       if !defined(U_SERVER_CAPTIVE_PORTAL) && !defined(U_LOG_DISABLE) && defined(USE_LIBZ)
         if (UServer_Base::log)                         UServer_Base::log->checkForLogRotateDataToWrite();
         if (UServer_Base::apache_like_log) UServer_Base::apache_like_log->checkForLogRotateDataToWrite();
#       endif

#       ifdef USE_LOAD_BALANCE
         if (fd_sock > 0)
            {
            U_SRV_MY_LOAD         = u_get_loadavg();
            U_SRV_MIN_LOAD_REMOTE = 255;

            U_INTERNAL_DUMP("U_SRV_MY_LOAD = %u", U_SRV_MY_LOAD)

            int iBytesTransferred = U_SYSCALL(sendto, "%d,%p,%u,%u,%p,%d", fd_sock, &U_SRV_MY_LOAD, sizeof(char), MSG_DONTROUTE,
                                                                           (sockaddr*)&(srv_addr.psaGeneric), sizeof(srv_addr));

            if (iBytesTransferred == sizeof(char))
               {
               unsigned char datagram[4096];
               socklen_t slDummy = sizeof(cli_addr);

               uint8_t  min_loadavg_remote    = 255;
               uint32_t min_loadavg_remote_ip = 0;

               while ((iBytesTransferred = U_SYSCALL(recvfrom,"%d,%p,%u,%u,%p,%p", fd_sock, datagram, sizeof(datagram), MSG_DONTWAIT,
                                                                           (sockaddr*)&(cli_addr.psaGeneric),&slDummy)) > 0)
                  {
                  U_DUMP("Received datagram from (%V:%u) = (%u,%#.*S)",
                           UIPAddress::toString(cli_addr.psaIP4Addr.sin_addr.s_addr).rep,
                                          ntohs(cli_addr.psaIP4Addr.sin_port), iBytesTransferred, iBytesTransferred, datagram)

                  if (iBytesTransferred == 1)
                     {
                     if ((      cli_addr.psaIP4Addr.sin_addr.s_addr == addr          &&
                          ntohs(cli_addr.psaIP4Addr.sin_port) == UServer_Base::port) ||
                         (UServer_Base::vallow_cluster                               &&
                          UIPAllow::isAllowed(cli_addr.psaIP4Addr.sin_addr.s_addr, *UServer_Base::vallow_cluster) == false))
                        {
                        continue;
                        }

                     if (datagram[0] < min_loadavg_remote)
                        {
                        min_loadavg_remote = datagram[0];

                        u_put_unalignedp32(&min_loadavg_remote_ip, srv_addr.psaIP4Addr.sin_addr.s_addr);
                        }
                     }
                  }

               U_SRV_MIN_LOAD_REMOTE = min_loadavg_remote;

               u_put_unalignedp32(&U_SRV_MIN_LOAD_REMOTE_IP, min_loadavg_remote_ip);

               U_DUMP("U_SRV_MIN_LOAD_REMOTE = %u U_SRV_MIN_LOAD_REMOTE_IP = %s", U_SRV_MIN_LOAD_REMOTE, UIPAddress::getAddressString(U_SRV_MIN_LOAD_REMOTE_IP))
               }
            }
#       endif

         u_gettimenow();

         U_INTERNAL_DUMP("u_now->tv_sec = %ld u_now->tv_usec = %ld", u_now->tv_sec, u_now->tv_usec)

         sec = u_now->tv_sec;

#       ifndef U_SERVER_CAPTIVE_PORTAL
         if (daylight &&
             (sec % U_ONE_HOUR_IN_SECOND) == 0)
            {
            (void) UTimeDate::checkForDaylightSavingTime(sec);
            }
#       endif

         if (UServer_Base::update_date)
            {
#        ifndef U_SERVER_CAPTIVE_PORTAL
            (void) U_SYSCALL(pthread_rwlock_wrlock, "%p", ULog::prwlock);
#        endif

            if ((sec % U_ONE_HOUR_IN_SECOND) != 0)
               {
               if (UServer_Base::update_date1) UTimeDate::updateTime(ULog::ptr_shared_date->date1 + 12);
               if (UServer_Base::update_date2) UTimeDate::updateTime(ULog::ptr_shared_date->date2 + 15);
               if (UServer_Base::update_date3) UTimeDate::updateTime(ULog::ptr_shared_date->date3 + 26);
               }
            else
               {
               lnow = u_get_localtime(sec);

               if (UServer_Base::update_date1) (void) u_strftime2(ULog::ptr_shared_date->date1,     17, U_CONSTANT_TO_PARAM("%d/%m/%y %T"),     lnow);
               if (UServer_Base::update_date2) (void) u_strftime2(ULog::ptr_shared_date->date2,   26-6, U_CONSTANT_TO_PARAM("%d/%b/%Y:%T"),     lnow);
               if (UServer_Base::update_date3) (void) u_strftime2(ULog::ptr_shared_date->date3+6, 29-4, U_CONSTANT_TO_PARAM("%a, %d %b %Y %T"), sec);
               }

#        ifndef U_SERVER_CAPTIVE_PORTAL
            (void) U_SYSCALL(pthread_rwlock_unlock, "%p", ULog::prwlock);
#        endif
            }
         }
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UTimeThread)
};
#  endif

#  if defined(USE_LIBSSL) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB) && !defined(_MSWINDOWS_)
#     include <ulib/net/tcpsocket.h>
#     include <ulib/net/client/client.h>

class UOCSPStapling : public UThread {
public:

   UOCSPStapling() : UThread(PTHREAD_CREATE_DETACHED) {}

   virtual void run() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "UOCSPStapling::run()")

      U_SRV_LOG("SSL: OCSP Stapling thread activated (tid %u)", u_gettid());

      struct timespec ts = { 0L, 50000L }, rem = { 0L, 0L };

      while (UServer_Base::flag_loop)
         {
         if (U_SYSCALL(nanosleep, "%p,%p", &ts, &rem) == -1)
            {
            U_INTERNAL_ASSERT_EQUALS(errno, EINTR)

            U_INTERNAL_DUMP("UServer_Base::flag_loop = %b rem.tv_sec = %ld", UServer_Base::flag_loop, rem.tv_sec)

            ts.tv_sec = rem.tv_sec;

            continue;
            }

         ts.tv_sec = USSLSocket::doStapling();
         }
      }

   static bool init()
      {
      U_TRACE_NO_PARAM(0, "UOCSPStapling::init()")

      if (USSLSocket::setDataForStapling() == false) U_RETURN(false);

      USSLSocket::staple.data = UServer_Base::getPointerToDataShare(USSLSocket::staple.data);

      UServer_Base::setLockOCSPStaple();

      U_INTERNAL_ASSERT_EQUALS(UServer_Base::pthread_ocsp, U_NULLPTR)

      U_NEW_ULIB_OBJECT(UOCSPStapling, UServer_Base::pthread_ocsp, UOCSPStapling);

      U_INTERNAL_DUMP("UServer_Base::pthread_ocsp = %p", UServer_Base::pthread_ocsp)

      // NB: we must run before fork...

      UServer_Base::pthread_ocsp->start(0);

      U_RETURN(true);
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UOCSPStapling)
};

ULock*   UServer_Base::lock_ocsp_staple;
UThread* UServer_Base::pthread_ocsp;
#  endif

#  ifdef U_SSE_ENABLE // SERVER SENT EVENTS (SSE)
int           UServer_Base::sse_event_fd;
int           UServer_Base::sse_socketpair[2];
char          UServer_Base::iovbuf[1] = { 0 };
char          UServer_Base::sse_fifo_name[256];
ULock*        UServer_Base::lock_sse;
uint32_t      UServer_Base::sse_fifo_pos;
UThread*      UServer_Base::pthread_sse;
UString*      UServer_Base::sse_id;
UString*      UServer_Base::sse_event;
UString*      UServer_Base::str_asterisk;
struct iovec  UServer_Base::iov[1] = { { iovbuf, 1 } };
struct msghdr UServer_Base::msg = { 0, 0, iov, 1, &cmsg, sizeof(struct ucmsghdr), 0 };

UVector<USSEClient*>*         UServer_Base::sse_vclient;
struct UServer_Base::ucmsghdr UServer_Base::cmsg = { sizeof(struct ucmsghdr), SOL_SOCKET, SCM_RIGHTS, 0 };

#  ifdef USE_LIBSSL
ULock* UServer_Base::lock_sse_ssl;
#  endif

class USSEThread;

class U_NO_EXPORT USSEClient {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   USSEClient(const UString& id, const UString& subscribe, int _fd, bool process) : sub(subscribe), uniq_id(id), fd(_fd), bprocess(process)
      {
      U_TRACE_REGISTER_OBJECT(0, USSEClient, "%V,%V,%d,%b", id.rep, subscribe.rep, _fd, process)

      U_INTERNAL_ASSERT_DIFFERS(fd, -1)
      }

   ~USSEClient()
      {
      U_TRACE_UNREGISTER_OBJECT(0, USSEClient)

      UFile::close(fd);

      if (bprocess)
         {
         UServer_Base::setFIFOForSSE(uniq_id);

         (void) UFile::_unlink(UServer_Base::sse_fifo_name);
         }
      }

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool _reset) const { return ""; }
#endif

protected:
   UString sub,     // Token of subscribe (optional). Can be set by parameter "subscribe"
           uniq_id; //   unique client id (optional). Can be set by parameter "id"
   int fd;
   bool bprocess;

   bool sendMsg(const UString& message, UString* pevent)
      {
      U_TRACE(0, "USSEClient::sendMsg(%V,%p)", message.rep, pevent)

      if (bprocess)
         {
         if (U_SYSCALL(write, "%u,%S,%u", fd, U_STRING_TO_PARAM(message)) <= 0) U_RETURN(false);
         }
      else
         {
         U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

         UString tmp = UServer_Base::printSSE(U_SRV_SSE_CNT1, message, pevent);

         if (U_SYSCALL(write, "%u,%S,%u", fd, U_STRING_TO_PARAM(tmp)) <= 0) U_RETURN(false);
         }

      U_RETURN(true);
      }

private:
   friend class USSEThread;
};

class USSEThread : public UThread {
public:

   USSEThread() : UThread(PTHREAD_CREATE_DETACHED) {}

   virtual void run() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "USSEThread::run()")

      int fd;
      const char* ptr;
      USSEClient* client;
      bool ball, bprocess;
      UVector<UString> vec;
      UVector<UString> vmessage;
      uint32_t pos, mr, sz, last_event_id, i, n, k, start = 0, end = 0;
      UString input(U_CAPACITY), output(U_CAPACITY), row, _id, token, rID, message, tmp;

      while (U_SRV_FLAG_SIGTERM == false)
         {
         if (UNotifier::waitForRead(UServer_Base::sse_event_fd) == 1 &&
             UServices::read(UServer_Base::sse_event_fd, input))
            {
            /**
             * NEW <id> <token> <last_event_id> <bprocess>
             * DEL <id>
             *
             * LIST - List clients to /tmp/SSE_list.txt
             *
             * MSG <id> <message>      - Send message to the stream <id>
             * <token>=<message>       - Send message to the subscribers of <token>
             * <token>-<rId>=<message> - Send message to the subscribers of <token> (* => all) except <rId>
             */

            for (k = 0, sz = vec.split(input); k < sz; ++k)
               {
               row = vec[k];

               pos = row.find('=');

               if (pos > 32)
                  {
                  ptr = row.data();

                  if (u_get_unalignedp32(ptr) == U_MULTICHAR_CONSTANT32('N','E','W',' ')) // NEW <id> <token> <last_event_id> <bprocess>
                     {
                               _id = vec[k+1];
                             token = vec[k+2];
                     last_event_id = vec[k+3].strtoul();
                          bprocess = vec[k+4].strtoul();

                     U_INTERNAL_DUMP("_id = %V token = %V bprocess = %b last_event_id = %u U_SRV_SSE_CNT1 = %u", _id.rep, token.rep, bprocess, last_event_id, U_SRV_SSE_CNT1)

                     if ((ball = token.equal(*UServer_Base::str_asterisk))) token = *UServer_Base::str_asterisk;
                     else                                                   token.duplicate();

                     if (last_event_id &&
                         last_event_id < U_SRV_SSE_CNT1)
                        {
                        UVector<UString> vec1;

                        vmessage.getFromLast(last_event_id, start, end, vec1);

                        for (i = 0, n = vec1.size(); i < n; ++i)
                           {
                           message = vec1[i];

                           pos = message.find('=');

                           U_INTERNAL_DUMP("vmessage[%u] = %V pos = %u", i % vmessage.capacity(), message.rep, pos)

                           if (ball ||
                               token.equal(message.data(), pos))
                              {
                              (void) output.append(UServer_Base::printSSE(i+1, message.substr(pos+1), (ball ? U_NULLPTR : &token)));
                              }
                           }

                        output.push('\n');

                        (void) U_SYSCALL(write, "%u,%S,%u", UServer_Base::sse_event_fd, U_STRING_TO_PARAM(output));

                        output.setEmpty();
                        }

                     if (bprocess == false) fd = (U_SYSCALL(recvmsg, "%u,%p,%u", UServer_Base::sse_socketpair[0], &UServer_Base::msg, 0) == 1 ? UServer_Base::cmsg.cmsg_data : -1);
                     else
                        {
                        UServer_Base::setFIFOForSSE(_id);

                        (void) UFile::mkfifo(UServer_Base::sse_fifo_name);

                        fd = UFile::open(UServer_Base::sse_fifo_name, O_WRONLY, 0);

                        if (fd == -1) U_ERROR("Error on opening SSE FIFO: %S", UServer_Base::sse_fifo_name);
                        }

                     U_NEW(USSEClient, client, USSEClient(_id.copy(), token, fd, bprocess));

                     UServer_Base::sse_vclient->push(client);

                     k += 4;
                     }
                  else if (u_get_unalignedp32(ptr) == U_MULTICHAR_CONSTANT32('D','E','L',' ')) // DEL <id>
                     {
                     _id = vec[k+1];

                     U_INTERNAL_DUMP("_id = %V", _id.rep)

                     for (i = 0, n = UServer_Base::sse_vclient->size(); i < n; ++i)
                        {
                        client = UServer_Base::sse_vclient->at(i);

                        U_INTERNAL_DUMP("sse_vclient[%u]->uniq_id = %V", i, client->uniq_id.rep)

                        if (client->uniq_id == _id)
                           {
                           --n;

                           delete UServer_Base::sse_vclient->remove(i--);

                           break;
                           }
                        }

                     k += 1;
                     }
#              ifdef DEBUG
                  else if (u_get_unalignedp32(ptr) == U_MULTICHAR_CONSTANT32('L','I','S','T'))
                     {
                     UString row1(U_CAPACITY);

                     for (i = 0, n = UServer_Base::sse_vclient->size(); i < n; ++i)
                        {
                        client = UServer_Base::sse_vclient->at(i);

                        row1.snprintf(U_CONSTANT_TO_PARAM("uniq_id: %V sub: %V fd: %u proc: %b\n"), client->uniq_id.rep, client->sub.rep, client->fd, client->bprocess);

                        (void) output.append(row1);
                        }

                     (void) UFile::writeToTmp(U_STRING_TO_PARAM(output), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("SSE_client.txt"), 0);

                     output = vmessage.join('\n');

                     (void) UFile::writeToTmp(U_STRING_TO_PARAM(output), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("SSE_message.txt"), 0);

                     output.setEmpty();

                     ++k;
                     }
#              endif
                  else if (u_get_unalignedp32(ptr) == U_MULTICHAR_CONSTANT32('M','S','G',' ')) // MSG <id> <message>
                     {
                         _id = vec[k+1];
                     message = vec[k+2];

                     U_INTERNAL_DUMP("_id = %V message = %V U_SRV_SSE_CNT1 = %u", _id.rep, message.rep, U_SRV_SSE_CNT1)

                     for (i = 0, n = UServer_Base::sse_vclient->size(); i < n; ++i)
                        {
                        client = UServer_Base::sse_vclient->at(i);

                        U_INTERNAL_DUMP("sse_vclient[%u]->uniq_id = %V", i, client->uniq_id.rep)

                        if (client->uniq_id == _id)
                           {
                           U_SRV_LOG("[sse] send message(%u) to %V: %V", U_SRV_SSE_CNT1, _id.rep, message.rep);

                           if (client->sendMsg(message, U_NULLPTR) == false) delete UServer_Base::sse_vclient->remove(i);

                           break;
                           }
                        }

                     k += 2;
                     }
                  else
                     {
                     U_WARNING("Wrong formatted message from SSE FIFO, ignored: row = %V", row.rep);

                     ++k;
                     }
                  }
               else
                  {
                  mr = row.find('-', 0, pos);

                  if (mr == U_NOT_FOUND) token = row.substr(0U, pos);
                  else
                     {
                     token = row.substr(0U, mr);
                       rID = row.substr(mr+1, pos-mr-1);
                     }

                  message = row.substr(pos+1);

                  U_SRV_SSE_CNT1++;

                  U_INTERNAL_DUMP("token = %V rID = %V message = %V U_SRV_SSE_CNT1 = %u", token.rep, rID.rep, message.rep, U_SRV_SSE_CNT1)

                  U_SRV_LOG("[sse] send message(%u) to subscribers of %V%.*s%.*s: %V", U_SRV_SSE_CNT1, token.rep,
                              rID ? U_CONSTANT_SIZE(" except SSE_") : 0, " except SSE_", rID ? rID.size() : 0, rID.data(), message.rep);

                  tmp = ((ball = token.equal(*UServer_Base::str_asterisk)) ? "*="+message : token+'='+message);

                  vmessage.insertWithBound(tmp, start, end);

                  for (i = 0, n = UServer_Base::sse_vclient->size(); i < n; ++i)
                     {
                     client = UServer_Base::sse_vclient->at(i);

                     if (rID != client->uniq_id &&
                         (ball                  ||
                          client->sub.equal(token)))
                        {
                        if (client->sendMsg(message, (ball ? U_NULLPTR : &token)) == false)
                           {
                           --n;

                           delete UServer_Base::sse_vclient->remove(i--);
                           }
                        }
                     }
                  }
               }

            UServer_Base::unlockSSE();

                vec.clear();
                rID.clear();
                tmp.clear();
              token.clear();
            message.clear();
            input.setEmpty();
            }
         }
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(USSEThread)
};
#  endif
#endif

#ifdef U_LINUX
static long sysctl_somaxconn, tcp_abort_on_overflow, sysctl_max_syn_backlog, tcp_fin_timeout;
#endif

UServer_Base::UServer_Base(UFileConfig* pcfg)
{
   U_TRACE_REGISTER_OBJECT(0, UServer_Base, "%p", pcfg)

   U_INTERNAL_ASSERT_EQUALS(pthis, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(cenvironment, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(senvironment, U_NULLPTR)

   port  = 80;
   pthis = this;

   U_NEW(UString, as_user,       UString);
   U_NEW(UString, dh_file,       UString);
   U_NEW(UString, cert_file,     UString);
   U_NEW(UString, key_file,      UString);
   U_NEW(UString, password,      UString);
   U_NEW(UString, ca_file,       UString);
   U_NEW(UString, ca_path,       UString);
   U_NEW(UString, name_sock,     UString);
   U_NEW(UString, IP_address,    UString);
   U_NEW(UString, cenvironment,  UString(U_CAPACITY));
   U_NEW(UString, senvironment,  UString(U_CAPACITY));
   U_NEW(UString, document_root, UString);

   U_NEW(UVector<file_LOG*>, vlog, UVector<file_LOG*>);

   u_init_ulib_hostname();
   u_init_ulib_username();

   u_init_security();

   if (u_start_time     == 0 &&
       u_setStartTime() == false)
      {
      U_WARNING("System date not updated: %#5D", u_now->tv_sec);
      }

   /**
    * TMPDIR is the canonical Unix environment variable which points to user scratch space. Most Unix utilities will honor the setting of this
    * variable and use its value to denote the scratch area for temporary files instead of the common default of /tmp. Other forms sometimes
    * accepted are TEMP, TEMPDIR, and TMP but these are used more commonly by non-POSIX Operating systems. If defined, otherwise it will be
    * /tmp. By default, /tmp on Fedora 18 will be on a tmpfs. Storage of large temporary files should be done in /var/tmp. This will reduce the
    * I/O generated on disks, increase SSD lifetime, save power, and improve performance of the /tmp filesystem 
    */

   const char* tmpdir = (const char*) getenv("TMPDIR");

   u_tmpdir = (tmpdir ? tmpdir : "/var/tmp");

   if (pcfg)
      {
      U_INTERNAL_ASSERT_EQUALS(cfg, U_NULLPTR)

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

#ifdef ENABLE_THREAD
# if !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   if (preforked_num_kids == -1)
      {
      U_INTERNAL_ASSERT_POINTER(UNotifier::pthread)

      delete UNotifier::pthread;
      }
# endif

# ifdef U_LINUX
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

      if (lock_ocsp_staple) delete lock_ocsp_staple;
      }
#  endif

#  ifdef U_SSE_ENABLE // SERVER SENT EVENTS (SSE)
   if (lock_sse) delete lock_sse;
#  ifdef USE_LIBSSL
   if (lock_sse_ssl) delete lock_sse_ssl;
#  endif
   if (pthread_sse) delete (USSEThread*)pthread_sse;
#  endif
# endif
#endif

   UTimer::clear();

   UClientImage_Base::clear();

   delete socket;

   if (vplugin)
      {
      delete vplugin_name;
      delete vplugin;
      }

   UOrmDriver::clear();

   U_INTERNAL_ASSERT_POINTER(cenvironment)
   U_INTERNAL_ASSERT_POINTER(senvironment)

   delete cenvironment;
   delete senvironment;

   if (host)              delete host;
   if (emailClient)       delete emailClient;
   if (crashEmailAddress) delete crashEmailAddress;

#ifdef USE_LOAD_BALANCE
   if (ifname)         delete ifname;
   if (vallow_cluster) delete vallow_cluster;
#endif

#ifdef U_THROTTLING_SUPPORT
   if (db_throttling)
      {
             db_throttling->close();
      delete db_throttling;
      }

   if (throttling_mask) delete throttling_mask;
#endif

#ifdef U_EVASIVE_SUPPORT
   if (db_evasive)
      {
             db_evasive->close();
      delete db_evasive;
      }

   if (vwhitelist_IP)   delete vwhitelist_IP;
   if (dosEmailAddress) delete dosEmailAddress;
#endif

#ifdef U_WELCOME_SUPPORT
   if (msg_welcome) delete msg_welcome;
#endif

#ifdef U_ACL_SUPPORT
   if (vallow_IP) delete vallow_IP;
#endif

#ifdef U_RFC1918_SUPPORT
   if (vallow_IP_prv) delete vallow_IP_prv;
#endif

#ifdef U_LINUX
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

   if (ptr_shared_data) UFile::munmap(ptr_shared_data, map_size);

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

   UDynamic::clear();

   UEventFd::fd = -1; // NB: to avoid to delete itself...

   UNotifier::num_connection = 0;

   UNotifier::clear();
}

void UServer_Base::closeLog()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::closeLog()")

#ifndef U_LOG_DISABLE
   if (log &&
       log->isOpen())
      {
      log->closeLog();

#  ifdef DEBUG
      delete log;
#  endif

      log = U_NULLPTR;
      }

   if (apache_like_log &&
       apache_like_log->isOpen())
      {
      apache_like_log->closeLog();

#  ifdef DEBUG 
      delete apache_like_log;
#  endif

      apache_like_log = U_NULLPTR;
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

      vlog = U_NULLPTR;
      }
}

#ifdef U_WELCOME_SUPPORT
void UServer_Base::setMsgWelcome(const UString& msg)
{
   U_TRACE(0, "UServer_Base::setMsgWelcome(%V)", msg.rep)

   U_INTERNAL_ASSERT(msg)

   U_NEW(UString, msg_welcome, UString(U_CAPACITY));

   UEscape::decode(msg, *msg_welcome);

   if (*msg_welcome) (void) msg_welcome->shrink();
   else
      {
      delete msg_welcome;
             msg_welcome = U_NULLPTR;
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
         *document_root = UStringExt::expandPath(*document_root, U_NULLPTR);

         document_root_ptr = document_root->data();

         if (document_root->empty())
            {
            U_WARNING("Var DOCUMENT_ROOT %S expansion failed", document_root_ptr);

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
      U_WARNING("Chdir to working directory (DOCUMENT_ROOT) %S failed", document_root_ptr);

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
   // LOAD_BALANCE_CLUSTER           list of comma separated IP address (IPADDR[/MASK]) to define the load balance cluster
   // LOAD_BALANCE_DEVICE_NETWORK    network interface name of cluster of physical server
   // LOAD_BALANCE_LOADAVG_THRESHOLD system load threshold to proxies the request on other userver on the network cluster ([0-9].[0-9])
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
   //
   // CRASH_COUNT         this is the threshold for the number of crash of child server processes
   // CRASH_EMAIL_NOTIFY  the email address to send a message whenever the number of crash > CRASH_COUNT
   // --------------------------------------------------------------------------------------------------------------------------------------
   // This directive are for evasive action in the event of an HTTP DoS or DDoS attack or brute force attack
   // --------------------------------------------------------------------------------------------------------------------------------------
   // DOS_PAGE_COUNT      this is the threshold for the number of requests for the same page (or URI) per page interval
   // DOS_PAGE_INTERVAL   the interval for the page count threshold; defaults to 1 second intervals
   // DOS_SITE_COUNT      this is the threshold for the total number of requests for any object by the same client per site interval
   // DOS_SITE_INTERVAL   the interval for the site count threshold; defaults to 1 second intervals
   // DOS_BLOCKING_PERIOD the blocking period is the amount of time (in seconds) that a client will be blocked for if they are added to the blocking list (defaults to 10)
   // DOS_WHITE_LIST      list of comma separated IP addresses of trusted clients can be whitelisted to insure they are never denied (IPADDR[/MASK])
   // DOS_EMAIL_NOTIFY    the email address to send a message whenever an IP address becomes blacklisted
   // DOS_SYSTEM_COMMAND  the system command specified will be executed whenever an IP address becomes blacklisted. Use %.*s to denote the IP address of the blacklisted IP 
   // DOS_LOGFILE         the file to write DOS event
   // --------------------------------------------------------------------------------------------------------------------------------------

   U_INTERNAL_DUMP("bssl = %b budp = %b bipc = %b", bssl, budp, bipc)

#ifdef USERVER_IPC
   if (bipc)
      {
#  ifdef _MSWINDOWS_
      U_ERROR("Sorry, I was compiled on Windows so there isn't UNIX domain sockets");
#  endif

      *name_sock = cfg->at(U_CONSTANT_TO_PARAM("SOCKET_NAME"));

      if (name_sock->empty()) U_ERROR("Sorry, I cannot run without SOCKET_NAME value");
      }
#endif

   UString x  = cfg->at(U_CONSTANT_TO_PARAM("SERVER"));

   if (x) U_NEW(UString, server, UString(x));

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
      unsigned int _port = (port == 80 ? 8080 : 4433);

      U_WARNING("Sorry, it is required root privilege to listen on port %u but I am not setuid root, I must try %u", port, _port);

      port = _port;
      }

   U_INTERNAL_DUMP("SOMAXCONN = %d FD_SETSIZE = %d", SOMAXCONN, FD_SETSIZE)

   set_tcp_keep_alive    = cfg->readBoolean(U_CONSTANT_TO_PARAM("TCP_KEEP_ALIVE"));
   set_realtime_priority = cfg->readBoolean(U_CONSTANT_TO_PARAM("SET_REALTIME_PRIORITY"), true);

   crash_count                    = cfg->readLong(U_CONSTANT_TO_PARAM("CRASH_COUNT"), 5);
   tcp_linger_set                 = cfg->readLong(U_CONSTANT_TO_PARAM("TCP_LINGER_SET"), -2);
   USocket::iBackLog              = cfg->readLong(U_CONSTANT_TO_PARAM("LISTEN_BACKLOG"), SOMAXCONN);
   min_size_for_sendfile          = cfg->readLong(U_CONSTANT_TO_PARAM("MIN_SIZE_FOR_SENDFILE"), 500 * 1024); // 500k: for major size we assume is better to use sendfile()
   num_client_threshold           = cfg->readLong(U_CONSTANT_TO_PARAM("CLIENT_THRESHOLD"));
   UNotifier::max_connection      = cfg->readLong(U_CONSTANT_TO_PARAM("MAX_KEEP_ALIVE"));
   u_printf_string_max_length     = cfg->readLong(U_CONSTANT_TO_PARAM("LOG_MSG_SIZE"));
   num_client_for_parallelization = cfg->readLong(U_CONSTANT_TO_PARAM("CLIENT_FOR_PARALLELIZATION"));

#ifdef USERVER_UDP
   if (budp &&
       u_printf_string_max_length == -1)
      {
      u_printf_string_max_length = 128;
      }
#endif

   x = cfg->at(U_CONSTANT_TO_PARAM("CRASH_EMAIL_NOTIFY"));

   if (x)
      {
      U_INTERNAL_ASSERT_EQUALS(crashEmailAddress, U_NULLPTR)

      U_NEW(UString, crashEmailAddress, UString(x));

      if (emailClient == U_NULLPTR)
         {
         U_NEW(USmtpClient, emailClient, USmtpClient(UClientImage_Base::bIPv6));
         }
      }

#ifdef U_WELCOME_SUPPORT
   x = cfg->at(U_CONSTANT_TO_PARAM("WELCOME_MSG"));

   if (x) setMsgWelcome(x);
#endif

   x = cfg->at(U_CONSTANT_TO_PARAM("PREFORK_CHILD"));

   if (x)
      {
      preforked_num_kids = x.strtol();

#  if defined(U_SERVER_CAPTIVE_PORTAL) && !defined(ENABLE_THREAD)
      if (x.c_char(0) == '0') monitoring_process = true;
#  endif

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

#ifdef USE_LIBSSL
   if (bssl)
      {
      *dh_file   = cfg->at(U_CONSTANT_TO_PARAM("DH_FILE"));
      *ca_file   = cfg->at(U_CONSTANT_TO_PARAM("CA_FILE"));
      *ca_path   = cfg->at(U_CONSTANT_TO_PARAM("CA_PATH"));
      *key_file  = cfg->at(U_CONSTANT_TO_PARAM("KEY_FILE"));
      *password  = cfg->at(U_CONSTANT_TO_PARAM("PASSWORD"));
      *cert_file = cfg->at(U_CONSTANT_TO_PARAM("CERT_FILE"));

      verify_mode = cfg->readLong(U_CONSTANT_TO_PARAM("VERIFY_MODE"));

      min_size_for_sendfile = U_NOT_FOUND; // NB: we can't use sendfile with SSL...
      }
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
      U_INTERNAL_ASSERT_EQUALS(vallow_IP, U_NULLPTR)

      U_NEW(UVector<UIPAllow*>, vallow_IP, UVector<UIPAllow*>);

      if (UIPAllow::parseMask(x, *vallow_IP) == 0)
         {
         delete vallow_IP;
                vallow_IP = U_NULLPTR;
         }
      }
#endif

#ifdef U_RFC1918_SUPPORT
   x = cfg->at(U_CONSTANT_TO_PARAM("ALLOWED_IP_PRIVATE"));

   if (x)
      {
      U_INTERNAL_ASSERT_EQUALS(vallow_IP_prv, U_NULLPTR)

      U_NEW(UVector<UIPAllow*>, vallow_IP_prv, UVector<UIPAllow*>);

      if (UIPAllow::parseMask(x, *vallow_IP_prv) == 0)
         {
         delete vallow_IP_prv;
                vallow_IP_prv = U_NULLPTR;
         }
      }

   enable_rfc1918_filter = cfg->readBoolean(U_CONSTANT_TO_PARAM("ENABLE_RFC1918_FILTER"));
#endif

   // If you want the webserver to run as a process of a defined user, you can do it.
   // For the change of user to work, it's necessary to execute the server with root privileges.
   // If it's started by a user that that doesn't have root privileges, this step will be omitted

#ifndef U_LOG_DISABLE
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
            UMemoryPool::obj_class = UMemoryPool::func_call = U_NULLPTR;
#        endif
            }
         }
#  ifndef U_LOG_DISABLE
      else bmsg = true;
#  endif
      }

   // DOCUMENT_ROOT: The directory out of which we will serve your documents

#ifdef USERVER_UDP
   if (budp == false)
#endif
   {
   if (setDocumentRoot(cfg->at(U_CONSTANT_TO_PARAM("DOCUMENT_ROOT"))) == false) U_ERROR("Setting DOCUMENT ROOT to %V failed", document_root->rep);
   }

#ifndef U_LOG_DISABLE
   x = cfg->at(U_CONSTANT_TO_PARAM("LOG_FILE"));

   if (x)
      {
      // open log

      update_date  =
      update_date1 = true;

      U_NEW(ULog, log, ULog(x, cfg->readLong(U_CONSTANT_TO_PARAM("LOG_FILE_SZ"))));

      log->init(U_CONSTANT_TO_PARAM(U_SERVER_LOG_PREFIX));

#  ifdef USERVER_UDP
      if (budp == false)
#  endif
      {
      U_SRV_LOG("Working directory (DOCUMENT_ROOT) changed to %.*S", u_cwd_len, u_cwd);
      }

      if (bmsg) U_SRV_LOG("WARNING: the \"RUN_AS_USER\" directive makes sense only if the master process runs with super-user privileges, ignored");
      }
#endif

   x = cfg->at(U_CONSTANT_TO_PARAM("PID_FILE"));

   if (x)
      {
      // write pid on file

      U_INTERNAL_ASSERT(x.isNullTerminated())

      int old_pid = (int) UFile::getSysParam(x.data());

      if (old_pid > 0)
         {
         U_SRV_LOG("Trying to kill another instance of userver that is running with pid %d", old_pid);

         U_INTERNAL_ASSERT_DIFFERS(old_pid, u_pid)

         UProcess::kill(old_pid, SIGTERM); // SIGTERM is sent to every process in the process group of the calling process...
         }

      (void) UFile::writeTo(x, u_pid_str, u_pid_str_len);

      U_DEBUG("We have %s the PID_FILE %V with content: %P", (old_pid > 0 ? "updated" : "created"), x.rep);
      }

#ifdef USE_LOAD_BALANCE
# ifdef USERVER_UDP
   if (budp == false)
# endif
   {
   x = cfg->at(U_CONSTANT_TO_PARAM("LOAD_BALANCE_DEVICE_NETWORK"));

   if (x)
      {
      U_INTERNAL_ASSERT_EQUALS(ifname, U_NULLPTR)

      U_NEW(UString, ifname, UString(x));
      }

   x = cfg->at(U_CONSTANT_TO_PARAM("LOAD_BALANCE_LOADAVG_THRESHOLD"));

   if (x) loadavg_threshold = u_loadavg(x.data()); // 0.19 => 2, 4.56 => 46, ...

   U_INTERNAL_DUMP("loadavg_threshold = %u", loadavg_threshold)

   x = cfg->at(U_CONSTANT_TO_PARAM("LOAD_BALANCE_CLUSTER"));

   if (x)
      {
      U_INTERNAL_ASSERT_EQUALS(vallow_cluster, U_NULLPTR)

      U_NEW(UVector<UIPAllow*>, vallow_cluster, UVector<UIPAllow*>);

      if (UIPAllow::parseMask(x, *vallow_cluster) == 0)
         {
         delete vallow_cluster;
                vallow_cluster = U_NULLPTR;
         }
      }
   }
#endif

#ifdef U_EVASIVE_SUPPORT
# ifdef USERVER_UDP
   if (budp == false)
# endif
   {
   /**
    * This is the threshold for the number of requests for the same page (or URI) per page interval.
    * Once the threshold for that interval has been exceeded (defaults to 2), the IP address of the client will be added to the blocking list
    */

   page_count = cfg->readLong(U_CONSTANT_TO_PARAM("DOS_PAGE_COUNT"), 2);

   /**
    * The interval for the page count threshold; defaults to 1 second intervals
    */

   page_interval = cfg->readLong(U_CONSTANT_TO_PARAM("DOS_PAGE_INTERVAL"), 1);

   /**
    * This is the threshold for the total number of requests for any object by the same client per site interval.
    * Once the threshold for that interval has been exceeded (defaults to 50), the IP address of the client will be added to the blocking list
    */

   site_count = cfg->readLong(U_CONSTANT_TO_PARAM("DOS_SITE_COUNT"), 50);

   /**
    * The interval for the site count threshold; defaults to 1 second intervals
    */

   site_interval = cfg->readLong(U_CONSTANT_TO_PARAM("DOS_SITE_INTERVAL"), 1);

   /**
    * The blocking period is the amount of time (in seconds) that a client will be blocked for if they are added to the blocking list (defaults to 10).
    * During this time, all subsequent requests from the client will result in a abortive close and the timer being reset (e.g. another 10 seconds).
    * Since the timer is reset for every subsequent request, it is not necessary to have a long blocking period; in the event of a DoS attack, this
    * timer will keep getting reset
    */

   blocking_period = cfg->readLong(U_CONSTANT_TO_PARAM("DOS_BLOCKING_PERIOD"), 10);

   /**
    * IP addresses of trusted clients can be whitelisted to insure they are never denied. The purpose of whitelisting is to protect software, scripts, local
    * searchbots, or other automated tools from being denied for requesting large amounts of data from the server. Whitelisting should *not* be used to add
    * customer lists or anything of the sort, as this will open the server to abuse. This module is very difficult to trigger without performing some type of
    * malicious attack, and for that reason it is more appropriate to allow the module to decide on its own whether or not an individual customer should be blocked
    */

   x = cfg->at(U_CONSTANT_TO_PARAM("DOS_WHITE_LIST"));

   if (x)
      {
      U_INTERNAL_ASSERT_EQUALS(vwhitelist_IP, U_NULLPTR)

      U_NEW(UVector<UIPAllow*>, vwhitelist_IP, UVector<UIPAllow*>);

      if (UIPAllow::parseMask(x, *vwhitelist_IP) == 0)
         {
         delete vwhitelist_IP;
                vwhitelist_IP = U_NULLPTR;
         }
      }

   /**
    * If this value is set, an email will be sent to the address specified whenever an IP address becomes blacklisted
    */

   x = cfg->at(U_CONSTANT_TO_PARAM("DOS_EMAIL_NOTIFY"));

   if (x)
      {
      U_INTERNAL_ASSERT_EQUALS(dosEmailAddress, U_NULLPTR)

      U_NEW(UString, dosEmailAddress, UString(x));

      if (emailClient == U_NULLPTR)
         {
         U_NEW(USmtpClient, emailClient, USmtpClient(UClientImage_Base::bIPv6));
         }
      }

   /**
    * If this value is set, the system command specified will be executed whenever an IP address becomes blacklisted.
    * This is designed to enable system calls to ip filter or other tools. Use %.*s to denote the IP address of the blacklisted IP
    */

   x = cfg->at(U_CONSTANT_TO_PARAM("DOS_SYSTEM_COMMAND"));

   if (x)
      {
      U_INTERNAL_ASSERT_EQUALS(systemCommand, U_NULLPTR)

      U_NEW(UString, systemCommand, UString(x));
      }

   x = cfg->at(U_CONSTANT_TO_PARAM("DOS_LOGFILE"));

   if (x)
      {
      U_INTERNAL_ASSERT_EQUALS(dos_LOG, U_NULLPTR)

      U_NEW(UFile, dos_LOG, UFile(x));
      }
   }
#endif

   // load ORM driver modules...

#if defined(USE_SQLITE) || defined(USE_MYSQL) || defined(USE_PGSQL)
   UString orm_driver_dir  = cfg->at(U_CONSTANT_TO_PARAM("ORM_DRIVER_DIR")),
           orm_driver_list = cfg->at(U_CONSTANT_TO_PARAM("ORM_DRIVER"));

   if (UOrmDriver::loadDriver(orm_driver_dir, orm_driver_list) == false) U_ERROR("ORM drivers load failed");
#endif

   // load plugin modules and call server-wide hooks handlerConfig()...

#ifdef USERVER_UDP
   if (budp == false)
#endif
   {
   UString plugin_dir  = cfg->at(U_CONSTANT_TO_PARAM("PLUGIN_DIR")),
           plugin_list = cfg->at(U_CONSTANT_TO_PARAM("PLUGIN"));

   if (loadPlugins(plugin_dir, plugin_list) != U_PLUGIN_HANDLER_FINISHED) U_ERROR("Plugins stage load failed");
   }
}

U_NO_EXPORT void UServer_Base::loadStaticLinkedModules(const char* name)
{
   U_TRACE(0, "UServer_Base::loadStaticLinkedModules(%S)", name)

   U_INTERNAL_ASSERT_POINTER(vplugin_name)
   U_INTERNAL_ASSERT_MAJOR(vplugin_size, 0)

   uint32_t name_len;
   UString x(name, (name_len = u__strlen(name, __PRETTY_FUNCTION__)));

   if (vplugin_name->find(x) != U_NOT_FOUND) // NB: we load only the plugin that we want from configuration (PLUGIN var)...
      {
#if defined(U_STATIC_HANDLER_RPC)    || defined(U_STATIC_HANDLER_SHIB)   || defined(U_STATIC_HANDLER_ECHO)  || \
    defined(U_STATIC_HANDLER_STREAM) || defined(U_STATIC_HANDLER_SOCKET) || defined(U_STATIC_HANDLER_SCGI)  || \
    defined(U_STATIC_HANDLER_FCGI)   || defined(U_STATIC_HANDLER_GEOIP)  || defined(U_STATIC_HANDLER_PROXY) || \
    defined(U_STATIC_HANDLER_SOAP)   || defined(U_STATIC_HANDLER_SSI)    || defined(U_STATIC_HANDLER_TSA)   || \
    defined(U_STATIC_HANDLER_NOCAT)  || defined(U_STATIC_HANDLER_HTTP)
      const UServerPlugIn* _plugin = U_NULLPTR;
#  ifdef U_STATIC_HANDLER_RPC
      if (x.equal(U_CONSTANT_TO_PARAM("rpc")))    { U_NEW(URpcPlugIn, _plugin, URpcPlugIn);  goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_SHIB
      if (x.equal(U_CONSTANT_TO_PARAM("shib")))   { U_NEW(UShibPlugIn, _plugin, UShibPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_ECHO
      if (x.equal(U_CONSTANT_TO_PARAM("echo")))   { U_NEW(UEchoPlugIn, _plugin, UEchoPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_STREAM
      if (x.equal(U_CONSTANT_TO_PARAM("stream"))) { U_NEW(UStreamPlugIn, _plugin, UStreamPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_SOCKET
      if (x.equal(U_CONSTANT_TO_PARAM("socket"))) { U_NEW(UWebSocketPlugIn, _plugin, UWebSocketPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_SCGI
      if (x.equal(U_CONSTANT_TO_PARAM("scgi")))   { U_NEW(USCGIPlugIn, _plugin, USCGIPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_FCGI
      if (x.equal(U_CONSTANT_TO_PARAM("fcgi")))   { U_NEW(UFCGIPlugIn, _plugin, UFCGIPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_GEOIP
      if (x.equal(U_CONSTANT_TO_PARAM("geoip")))  { U_NEW(UGeoIPPlugIn, _plugin, UGeoIPPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_PROXY
      if (x.equal(U_CONSTANT_TO_PARAM("proxy")))  { U_NEW(UProxyPlugIn, _plugin, UProxyPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_SOAP
      if (x.equal(U_CONSTANT_TO_PARAM("soap")))   { U_NEW(USoapPlugIn, _plugin, USoapPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_SSI
      if (x.equal(U_CONSTANT_TO_PARAM("ssi")))    { U_NEW(USSIPlugIn, _plugin, USSIPlugIn);  goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_TSA
      if (x.equal(U_CONSTANT_TO_PARAM("tsa")))    { U_NEW(UTsaPlugIn, _plugin, UTsaPlugIn);  goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_NOCAT
      if (x.equal(U_CONSTANT_TO_PARAM("nocat")))  { U_NEW(UNoCatPlugIn, _plugin, UNoCatPlugIn); goto next; }
#  endif
#  ifdef U_STATIC_HANDLER_HTTP
      if (x.equal(U_CONSTANT_TO_PARAM("http")))   { U_NEW(UHttpPlugIn, _plugin, UHttpPlugIn); goto next; }
#  endif
next: if (_plugin)
         {
         vplugin_name_static->push_back(x);

         vplugin_static->push_back(_plugin);

         U_SRV_LOG("Link phase of static plugin %V success", x.rep);
         }
      else
         {
         U_SRV_LOG("WARNING: Link phase of static plugin %V failed", x.rep);
         }
#endif
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

   U_NEW(UVector<UString>, vplugin_name,        UVector<UString>(10U));
   U_NEW(UVector<UString>, vplugin_name_static, UVector<UString>(20U));

   U_NEW(UVector<UServerPlugIn*>, vplugin,        UVector<UServerPlugIn*>(10U));
   U_NEW(UVector<UServerPlugIn*>, vplugin_static, UVector<UServerPlugIn*>(10U));

   int result;
   uint32_t i, pos;
   UString item, _name;
   UServerPlugIn* _plugin;

   if (plugin_list) vplugin_size = vplugin_name->split(U_STRING_TO_PARAM(plugin_list)); // NB: we don't use split with substr() cause of dependency from config var PLUGIN...
   else
      {
      vplugin_size = 1;

      vplugin_name->push_back(*UString::str_http);
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

      pos = vplugin_name_static->find(item);

      U_INTERNAL_DUMP("i = %u pos = %u item = %V", i, pos, item.rep)

      if (pos != U_NOT_FOUND)
         {
         vplugin_name_static->erase(pos);

         _plugin = vplugin_static->remove(pos);

         vplugin->push(_plugin);
         }
      else
         {
         _name.setBuffer(32U);

         _name.snprintf(U_CONSTANT_TO_PARAM("server_plugin_%v"), item.rep);

         _plugin = UPlugIn<UServerPlugIn*>::create(U_STRING_TO_PARAM(_name));

         if (_plugin)
            {
            vplugin->push(_plugin);

            U_SRV_LOG("Load phase of plugin %V success", item.rep);
            }
         else
            {
            U_SRV_LOG("WARNING: Load phase of plugin %V failed", item.rep);
            }
         }
      }

   U_INTERNAL_ASSERT_MAJOR(vplugin_size, 0)
   U_INTERNAL_ASSERT_EQUALS(vplugin->size(), vplugin_size)

   delete vplugin_static;
   delete vplugin_name_static;

   if (cfg)
      {
      // NB: we load configuration in reverse order respect to the content of config var PLUGIN...

      i = vplugin_size;

      do {
         item = vplugin_name->at(--i);

         if (cfg->searchForObjectStream(U_STRING_TO_PARAM(item)))
            {
            cfg->table.clear();

            result = vplugin->at(i)->handlerConfig(*cfg);

            cfg->reset();

            if (result)
               {
               if (result == U_PLUGIN_HANDLER_ERROR)
                  {
                  U_SRV_LOG("WARNING: Configuration phase of plugin %V failed", item.rep);

                  U_RETURN(U_PLUGIN_HANDLER_ERROR);
                  }

               U_SRV_LOG("Configuration phase of plugin %V success", item.rep);
               }
            }
         }
      while (i > 0);
      }

   U_RETURN(U_PLUGIN_HANDLER_FINISHED);
}

#ifdef U_LOG_DISABLE
void UServer_Base::pluginsHandlerRequest()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::pluginsHandlerRequest()")

   U_INTERNAL_ASSERT_POINTER(vplugin)
   U_INTERNAL_ASSERT_MAJOR(vplugin_size, 0)

   int result;

   for (uint32_t i = 0; i < vplugin_size; ++i)
      {
      if ((result = vplugin->at(i)->handlerRequest()))
         {
         if (result == U_PLUGIN_HANDLER_ERROR)
            {
            U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;

            return;
            }

         U_INTERNAL_ASSERT_EQUALS(result, U_PLUGIN_HANDLER_PROCESSED)

         if (UClientImage_Base::isRequestAlreadyProcessed() ||
             U_ClientImage_parallelization == U_PARALLELIZATION_PARENT)
            {
            return;
            }
         }
      }
}
#else
void UServer_Base::pluginsHandlerRequest()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::pluginsHandlerRequest()")

   U_INTERNAL_ASSERT_POINTER(vplugin)
   U_INTERNAL_ASSERT_MAJOR(vplugin_size, 0)

   int result;
   UString name;
   UServerPlugIn* _plugin;

   for (uint32_t i = 0; i < vplugin_size; ++i)
      {
      _plugin = vplugin->at(i);

      if (isLog() == false)
         {
         if ((result = _plugin->handlerRequest()))
            {
            if (result == U_PLUGIN_HANDLER_ERROR)
               {
               U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;

               return;
               }

            U_INTERNAL_ASSERT_EQUALS(result, U_PLUGIN_HANDLER_PROCESSED)

            if (UClientImage_Base::isRequestAlreadyProcessed() ||
                U_ClientImage_parallelization == U_PARALLELIZATION_PARENT)
               {
               return;
               }
            }

         continue;
         }

      name = vplugin_name->at(i);

      (void) u__snprintf(mod_name[0], sizeof(mod_name[0]), U_CONSTANT_TO_PARAM("[%v] "), name.rep);

      result = _plugin->handlerRequest();

      mod_name[0][0] = '\0';

      if (result)
         {
         if (result == U_PLUGIN_HANDLER_ERROR)
            {
            U_ClientImage_state = U_PLUGIN_HANDLER_ERROR;

            log->log(U_CONSTANT_TO_PARAM("WARNING: Request phase of plugin %V failed"), name.rep);

            return;
            }

         U_INTERNAL_ASSERT_EQUALS(result, U_PLUGIN_HANDLER_PROCESSED)

         if (U_ClientImage_parallelization == U_PARALLELIZATION_PARENT) return;

         log->log(U_CONSTANT_TO_PARAM("Request phase of plugin %V success"), name.rep);

         if (UClientImage_Base::isRequestAlreadyProcessed()) return;
         }
      }
}
#endif

// NB: we call the various handlerXXX() in reverse order respect to the content of config var PLUGIN...

#ifdef U_LOG_DISABLE
#  define U_PLUGIN_HANDLER_REVERSE(xxx)                                            \
int UServer_Base::pluginsHandler##xxx()                                            \
{                                                                                  \
   U_TRACE_NO_PARAM(0, "UServer_Base::pluginsHandler"#xxx"()")                     \
                                                                                   \
   U_INTERNAL_ASSERT_POINTER(vplugin)                                              \
   U_INTERNAL_ASSERT_MAJOR(vplugin_size, 0)                                        \
                                                                                   \
   uint32_t i = vplugin_size;                                                      \
                                                                                   \
   do {                                                                            \
      if (vplugin->at(--i)->handler##xxx() == U_PLUGIN_HANDLER_ERROR)              \
         {                                                                         \
         U_RETURN(U_PLUGIN_HANDLER_ERROR);                                         \
         }                                                                         \
      }                                                                            \
   while (i > 0);                                                                  \
                                                                                   \
   U_RETURN(U_PLUGIN_HANDLER_FINISHED);                                            \
}
#else
#  define U_PLUGIN_HANDLER_REVERSE(xxx)                                            \
int UServer_Base::pluginsHandler##xxx()                                            \
{                                                                                  \
   U_TRACE_NO_PARAM(0, "UServer_Base::pluginsHandler"#xxx"()")                     \
                                                                                   \
   U_INTERNAL_ASSERT_POINTER(vplugin)                                              \
   U_INTERNAL_ASSERT_MAJOR(vplugin_size, 0)                                        \
                                                                                   \
   int result;                                                                     \
   UString name;                                                                   \
   UServerPlugIn* _plugin;                                                         \
   uint32_t i = vplugin_size;                                                      \
                                                                                   \
   do {                                                                            \
      _plugin = vplugin->at(--i);                                                  \
                                                                                   \
      if (isLog() == false)                                                        \
         {                                                                         \
         if ((result = _plugin->handler##xxx()))                                   \
            {                                                                      \
            if (result == U_PLUGIN_HANDLER_ERROR) U_RETURN(U_PLUGIN_HANDLER_ERROR);\
                                                                                   \
            U_INTERNAL_ASSERT_EQUALS(result, U_PLUGIN_HANDLER_PROCESSED)           \
            }                                                                      \
                                                                                   \
         continue;                                                                 \
         }                                                                         \
                                                                                   \
      name = vplugin_name->at(i);                                                  \
                                                                                   \
      (void) u__snprintf(mod_name[0], sizeof(mod_name[0]),                         \
                         U_CONSTANT_TO_PARAM("[%v] "), name.rep);                  \
                                                                                   \
      result = _plugin->handler##xxx();                                            \
                                                                                   \
      mod_name[0][0] = '\0';                                                       \
                                                                                   \
      if (result)                                                                  \
         {                                                                         \
         if (result == U_PLUGIN_HANDLER_ERROR)                                     \
            {                                                                      \
            log->log(U_CONSTANT_TO_PARAM("WARNING: "#xxx" phase of plugin "        \
                                         "%V failed"), name.rep);                  \
                                                                                   \
            U_RETURN(U_PLUGIN_HANDLER_ERROR);                                      \
            }                                                                      \
                                                                                   \
         U_INTERNAL_ASSERT_EQUALS(result, U_PLUGIN_HANDLER_PROCESSED)              \
                                                                                   \
         if (U_ClientImage_parallelization != U_PARALLELIZATION_PARENT)            \
            {                                                                      \
            log->log(U_CONSTANT_TO_PARAM(#xxx" phase of plugin %V success"),       \
                     name.rep);                                                    \
            }                                                                      \
         }                                                                         \
      }                                                                            \
   while (i > 0);                                                                  \
                                                                                   \
   U_RETURN(U_PLUGIN_HANDLER_FINISHED);                                            \
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

   U_INTERNAL_DUMP("bssl = %b budp = %b bipc = %b", bssl, budp, bipc)

#ifdef USERVER_UDP
   if (budp)
      {
      U_ASSERT(socket->isUDP())

      if (socket->setServer(port, U_NULLPTR) == false)
         {
         U_ERROR("Run as server UDP with port '%u' failed", port);
         }

      goto next;
      }
#endif

#ifdef USE_LIBSSL
   if (bssl)
      {
      U_ASSERT(((USSLSocket*)socket)->isSSL())

      U_INTERNAL_ASSERT(  dh_file->isNullTerminated())
      U_INTERNAL_ASSERT(  ca_file->isNullTerminated())
      U_INTERNAL_ASSERT(  ca_path->isNullTerminated())
      U_INTERNAL_ASSERT( key_file->isNullTerminated())
      U_INTERNAL_ASSERT( password->isNullTerminated())
      U_INTERNAL_ASSERT(cert_file->isNullTerminated())

      // Load our certificate

      if (((USSLSocket*)socket)->setContext( dh_file->data(), cert_file->data(), key_file->data(),
                                            password->data(),   ca_file->data(),  ca_path->data(), verify_mode) == false)
         {
         U_ERROR("SSL: server setContext() failed");
         }
      }
#endif

#ifdef USERVER_IPC
   if (bipc)
      {
      U_ASSERT(socket->isIPC())

      UUnixSocket::setPath(name_sock->data());

      if (UUnixSocket::path == U_NULLPTR) U_ERROR("UNIX domain socket is not bound to a file system pathname");
      }
#endif

   if (socket->setServer(port, server) == false)
      {
      UString x = (server ? *server : U_STRING_FROM_CONSTANT("*"));

      U_ERROR("Run as server with local address '%v:%u' failed", x.rep, port);
      }

#ifdef USERVER_UDP
next:
#endif

   U_SRV_LOG("SO_REUSEPORT status is: %susing", (USocket::breuseport ? "" : "NOT "));

   // get name host

   U_NEW(UString, host, UString(server ? *server : USocketExt::getNodeName()));

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
#endif

   /**
    * This code does NOT make a connection or send any packets (to 8.8.8.8 which is google DNS).
    * Since UDP is a stateless protocol connect() merely makes a system call which figures out how to
    * route the packets based on the address and what interface (and therefore IP address) it should
    * bind to. Returns an array containing the family (AF_INET), local port, and local address (which
    * is what we want) of the socket
    */

#ifdef USERVER_IPC
   if (bipc == false)
#endif
   {
   UUDPSocket cClientSocket(UClientImage_Base::bIPv6);

   if (cClientSocket.connectServer(U_STRING_FROM_CONSTANT("8.8.8.8"), 1001))
      {
      socket->setLocal(cClientSocket.cLocalAddress);

      const char* p = socket->getLocalInfo();

      UString ip(p, u__strlen(p, __PRETTY_FUNCTION__));

           if ( IP_address->empty()) *IP_address = ip;
      else if (*IP_address != ip)
         {
         U_SRV_LOG("WARNING: SERVER IP ADDRESS from configuration (%V) differ from system interface (%V)", IP_address->rep, ip.rep);
         }
      }

   if (IP_address->empty())
      {
      (void) IP_address->assign(U_CONSTANT_TO_PARAM("127.0.0.1"));

      socket->cLocalAddress.setLocalHost(UClientImage_Base::bIPv6);

      U_WARNING("getting IP_ADDRESS from system interface fail, we try using localhost");
      }
   else
      {
      struct in_addr ia;

      if (inet_aton(IP_address->c_str(), &ia) == 0) U_ERROR("IP_ADDRESS conversion fail: %V", IP_address->rep);

      socket->setAddress(&ia);

      public_address = (socket->cLocalAddress.isPrivate() == false);
      }
   }

   U_SRV_LOG("SERVER IP ADDRESS registered as: %v (%s)", IP_address->rep, (public_address ? "public" : "private"));

#ifdef U_LINUX
   u_need_root(false);

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
#endif

   UTimer::init(UTimer::NOSIGNAL);

   UClientImage_Base::init();

   USocket::accept4_flags = SOCK_CLOEXEC | SOCK_NONBLOCK;

   U_INTERNAL_ASSERT_EQUALS(proc, U_NULLPTR)

   U_NEW(UProcess, proc, UProcess);

   U_INTERNAL_ASSERT_POINTER(proc)

   proc->setProcessGroup();

#if !defined(U_LOG_DISABLE) && defined(USE_LIBZ)
   if (isLog() &&
       log->isMemoryMapped())
      {
      U_INTERNAL_ASSERT_EQUALS(shared_data_add, 0)

      shared_data_add = log->getSizeLogRotateData();
      }
# endif

   // init plugin modules, must run after the setting for shared log

#ifdef USERVER_UDP
   if (budp == false)
#endif
   {
   if (pluginsHandlerInit() != U_PLUGIN_HANDLER_FINISHED) U_ERROR("Plugins stage init failed");
   }

   // manage shared data...

   U_INTERNAL_DUMP("shared_data_add = %u shm_data_add = %u", shared_data_add, shm_data_add)

   U_INTERNAL_ASSERT_EQUALS(ptr_shm_data, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(ptr_shared_data, U_NULLPTR)

   U_DEBUG("sizeof(shared_data) = %u sizeof(shm_data) = %u shared_data_add = %u shm_data_add = %u", sizeof(shared_data), sizeof(shm_data), shared_data_add, shm_data_add);

   // For portable use, a shared memory object should be identified by a name of the form /somename; that is, a null-terminated string of
   // up to NAME_MAX (i.e., 255) characters consisting of an initial slash, followed by one or more characters, none of which are slashes

#ifndef U_SERVER_CAPTIVE_PORTAL
   shm_size     = sizeof(shm_data) + shm_data_add;
   ptr_shm_data = (shm_data*) UFile::shm_open("/userver", shm_size);

   U_INTERNAL_ASSERT_POINTER(ptr_shm_data)
#endif

#ifndef U_LOG_DISABLE
   if (isLog() == false)
#endif
   ULog::initDate();

   flag_loop = true; // NB: UTimeThread loop depend on this setting...

#if defined(U_LINUX) && defined(ENABLE_THREAD)
   map_size        = sizeof(shared_data) + shared_data_add;
   ptr_shared_data = (shared_data*) UFile::mmap(&map_size);

   U_INTERNAL_ASSERT_POINTER(ptr_shared_data)
   U_INTERNAL_ASSERT_DIFFERS(ptr_shared_data, MAP_FAILED)
   U_INTERNAL_ASSERT_EQUALS(ULog::ptr_shared_date, U_NULLPTR)

   bool _daylight    = *u_pdaylight;
   int now_adjust    = *u_pnow_adjust; /* GMT based time */
   struct timeval tv = *u_now;

   *(u_now         = &(ptr_shared_data->now_shared))        = tv;
   *(u_pdaylight   = &(ptr_shared_data->daylight_shared))   = _daylight;
   *(u_pnow_adjust = &(ptr_shared_data->now_adjust_shared)) = now_adjust;

   ULog::ptr_shared_date = &(ptr_shared_data->log_date_shared);

   U_MEMCPY(ULog::ptr_shared_date, &ULog::date, sizeof(ULog::log_date));

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

   (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_BLOCK, &mask, U_NULLPTR);

   U_INTERNAL_ASSERT_EQUALS(ULog::prwlock,  U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(u_pthread_time, U_NULLPTR)

# ifdef USERVER_UDP
   if (budp == false ||
       UServer_Base::update_date)
# endif
   {
   U_NEW_ULIB_OBJECT(UTimeThread, u_pthread_time, UTimeThread);

   (void) UThread::initRwLock((ULog::prwlock = &(ptr_shared_data->rwlock)));

   ((UTimeThread*)u_pthread_time)->start(50);
   }
#endif

#if !defined(U_LOG_DISABLE) && defined(USE_LIBZ)
   if (isLog() &&
       log->isMemoryMapped())
      {
      log->setLogRotate();

      log->setShared(&(ptr_shared_data->log_data_shared)); // NB: if log is memory mapped must be shared because the possibility of fork() by parallelization

      U_SRV_LOG("Mapped %u bytes (%u KB) of shared memory for %d preforked process", sizeof(shared_data) + shared_data_add, map_size / 1024, preforked_num_kids);
      }

   if (apache_like_log &&
       apache_like_log->isMemoryMapped())
      {
      apache_like_log->setLogRotate();

      apache_like_log->setShared(&(ptr_shm_data->log_data_shared)); // NB: if apache like log is memory mapped must be shared because the possibility of condivision with userver_ssl

      U_SRV_LOG("Mapped %u bytes (%u KB) of shared memory for apache like log", apache_like_log->getSizeLogRotateData(), apache_like_log->getSizeLogRotateData() / 1024);
      }
#endif

   U_INTERNAL_ASSERT_EQUALS(U_SRV_TOT_CONNECTION, 0)

#ifdef DEBUG
   UEventTime* pstat;

   U_NEW(UTimeStat, pstat, UTimeStat);

   UTimer::insert(pstat);
#endif

   (void) UFile::_mkdir("../db");

#ifdef U_THROTTLING_SUPPORT
   if (db_throttling)
      {
      // set up the throttles timer

      UEventTime* throttling_time;

      U_NEW(UBandWidthThrottling, throttling_time, UBandWidthThrottling);

      UTimer::insert(throttling_time);
      }
#endif

   socket_flags |= O_RDWR | O_CLOEXEC;

#ifdef USERVER_UDP
   if (budp)
      {
      UNotifier::max_connection = 1;
      UNotifier::num_connection =
      UNotifier::min_connection = 0;
      }
   else
#endif
   {
   // ---------------------------------------------------------------------------------------------------------
   // init notifier event manager
   // ---------------------------------------------------------------------------------------------------------

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

#  ifndef USE_LIBEVENT
      if (timeoutMS > 0) U_NEW(UTimeoutConnection, ptime, UTimeoutConnection);
#  endif

      pthis->UEventFd::op_mask |=  EPOLLET;
      pthis->UEventFd::op_mask &= ~EPOLLRDHUP;

      U_INTERNAL_ASSERT_EQUALS(pthis->UEventFd::op_mask, EPOLLIN | EPOLLET)

      /**
       * There may not always be a connection waiting after a SIGIO is delivered or select(2) or poll(2) return a readability
       * event because the connection might have been removed by an asynchronous network error or another thread before
       * accept() is called. If this happens then the call will block waiting for the next connection to arrive. To ensure
       * that accept() never blocks, the passed socket sockfd needs to have the O_NONBLOCK flag set (see socket(7))
       */

      socket_flags |= O_NONBLOCK;
      }

   if (handler_other)
      {
      UNotifier::min_connection++;

      handler_other->UEventFd::op_mask &= ~EPOLLRDHUP;
      }

   if (handler_inotify)
      {
      UNotifier::min_connection++;

      handler_inotify->UEventFd::op_mask &= ~EPOLLRDHUP;
      }
   }

   UNotifier::max_connection = (UNotifier::max_connection ? UNotifier::max_connection : USocket::iBackLog) + (UNotifier::num_connection = UNotifier::min_connection);

   if (num_client_threshold == 0) num_client_threshold = U_NOT_FOUND;

   if (num_client_for_parallelization == 0) num_client_for_parallelization = UNotifier::max_connection / 2;
   }

   U_INTERNAL_DUMP("UNotifier::max_connection = %u UNotifier::min_connection = %u num_client_for_parallelization = %u num_client_threshold = %u",
                    UNotifier::max_connection,     UNotifier::min_connection,     num_client_for_parallelization,     num_client_threshold)

   pthis->preallocate();

#if defined(USE_LIBSSL) && defined(ENABLE_THREAD) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB) && !defined(_MSWINDOWS_)
   if (bssl &&
       UOCSPStapling::init() == false)
      {
      U_WARNING("SSL: OCSP stapling ignored, some error occured...");
      }
#endif

#ifdef USERVER_UDP
   if (budp == false)
#endif
   {
   if (pluginsHandlerRun() != U_PLUGIN_HANDLER_FINISHED) U_ERROR("Plugins stage run failed");
   }

   if (u_start_time     == 0 &&
       u_setStartTime() == false)
      {
      U_ERROR("System date not updated");
      }

#ifdef U_THROTTLING_SUPPORT
# ifdef USERVER_UDP
   if (budp == false)
# endif
   if (throttling_mask) initThrottlingServer();
#endif

#ifdef U_EVASIVE_SUPPORT
# ifdef USERVER_UDP
   if (budp == false)
# endif
   initEvasive();
#endif

#ifdef U_SSE_ENABLE // SERVER SENT EVENTS (SSE)
# ifdef USERVER_UDP
   if (budp == false)
# endif
   {
   sse_fifo_pos = u__snprintf(sse_fifo_name, 256, U_CONSTANT_TO_PARAM("%s/SSE_%s_EVENT"), u_tmpdir, bssl ? "SSL" : "TCP") - U_CONSTANT_SIZE("EVENT");

   (void) UFile::mkfifo(sse_fifo_name, PERM_FILE);

   sse_event_fd = UFile::open(sse_fifo_name, O_RDWR, PERM_FILE);

   if (sse_event_fd == -1) U_ERROR("Error on opening SSE FIFO: %S", sse_fifo_name);

# ifndef DEBUG
   (void) UFile::_unlink(sse_fifo_name);
# endif

   setLockSSE();

   U_INTERNAL_ASSERT_EQUALS(sse_id, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(sse_vclient, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(pthread_sse, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(str_asterisk, U_NULLPTR)

   U_NEW(UString, sse_id, UString);
   U_NEW(USSEThread, pthread_sse, USSEThread);
   U_NEW(UString, str_asterisk, U_STRING_FROM_CONSTANT("*"));
   U_NEW(UVector<USSEClient*>, sse_vclient, UVector<USSEClient*>);

   pthread_sse->start(0);

   (void) U_SYSCALL(socketpair, "%d,%d,%d,%p", AF_UNIX, SOCK_STREAM, 0, sse_socketpair);

   U_INTERNAL_DUMP("sse_socketpair[0] = %u sse_socketpair[1] = %u", sse_socketpair[0], sse_socketpair[1])

   U_SRV_LOG("SSE thread activated: %s(%d)", sse_fifo_name, sse_event_fd);
   }
#endif

   if (cfg) cfg->clear();

   UInterrupt::syscall_restart                 = false;
   UInterrupt::exit_loop_wait_event_for_signal = true;

#if !defined(USE_LIBEVENT) && !defined(USE_RUBY)
   UInterrupt::insert(               SIGHUP, (sighandler_t)UServer_Base::handlerForSigHUP);   // async signal
   UInterrupt::insert(              SIGTERM, (sighandler_t)UServer_Base::handlerForSigTERM);  // async signal
#else
   UInterrupt::setHandlerForSignal(  SIGHUP, (sighandler_t)UServer_Base::handlerForSigHUP);   //  sync signal
   UInterrupt::setHandlerForSignal( SIGTERM, (sighandler_t)UServer_Base::handlerForSigTERM);  //  sync signal
#endif
}

bool UServer_Base::addLog(UFile* _log, int flags)
{
   U_TRACE(0, "UServer_Base::addLog(%p,%d)", _log, flags)

   U_INTERNAL_ASSERT_POINTER(vlog)

   if (_log->creat(flags, PERM_FILE))
      {
      file_LOG* item;

      U_NEW(file_LOG, item, file_LOG);

      item->LOG   = _log;
      item->flags = flags;

      vlog->push_back(item);

      U_RETURN(true);
      }

   U_RETURN(false);
}

void UServer_Base::sendSignalToAllChildren(int signo, sighandler_t handler)
{
   U_TRACE(0, "UServer_Base::sendSignalToAllChildren(%d,%p)", signo, handler)

   U_INTERNAL_ASSERT_POINTER(proc)
   U_INTERNAL_ASSERT_POINTER(pthis)
   U_INTERNAL_ASSERT(proc->parent())

#if defined(U_LINUX) && defined(ENABLE_THREAD)
   if (u_pthread_time) ((UTimeThread*)u_pthread_time)->suspend();

# if defined(USE_LIBSSL) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
   if (pthread_ocsp) pthread_ocsp->suspend();
# endif

# ifdef U_SSE_ENABLE // SERVER SENT EVENTS (SSE)
   if (pthread_sse) pthread_sse->suspend();
# endif
#endif

   // NB: we can't use UInterrupt::erase() because it restore the old action (UInterrupt::init)...

   UInterrupt::setHandlerForSignal(signo, (sighandler_t)SIG_IGN);

   pthis->handlerSignal(signo); // manage signal before we send it to the preforked pool of children...

   UProcess::kill(0, signo); // signo is sent to every process in the process group of the calling process...

#ifndef USE_LIBEVENT
                UInterrupt::insert(signo, handler); // async signal
#else
   UInterrupt::setHandlerForSignal(signo, handler); //  sync signal
#endif

#if defined(U_LINUX) && defined(ENABLE_THREAD)
   if (u_pthread_time) ((UTimeThread*)u_pthread_time)->resume();

# if defined(USE_LIBSSL) && !defined(OPENSSL_NO_OCSP) && defined(SSL_CTRL_SET_TLSEXT_STATUS_REQ_CB)
   if (pthread_ocsp) pthread_ocsp->resume();
# endif

# ifdef U_SSE_ENABLE // SERVER SENT EVENTS (SSE)
   if (pthread_sse) pthread_sse->resume();
# endif
#endif
}

RETSIGTYPE UServer_Base::handlerForSigHUP(int signo)
{
   U_TRACE(0, "[SIGHUP] UServer_Base::handlerForSigHUP(%d)", signo)

#if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   if (preforked_num_kids == -1) return;
#endif

#ifndef U_LOG_DISABLE
   if (isLog()) // NB: for logrotate...
      {
      logMemUsage("SIGHUP");

      log->reopen();
      }
#endif

   if (isOtherLog()) reopenLog();

   u_gettimenow();

   sendSignalToAllChildren(SIGTERM, (sighandler_t)UServer_Base::handlerForSigTERM);

   if (preforked_num_kids > 1) rkids = 0;
   else                        manageSigHUP();
}

/*
RETSIGTYPE UServer_Base::handlerForSigCHLD(int signo)
{
   U_TRACE(0, "[SIGCHLD] UServer_Base::handlerForSigCHLD(%d)", signo)

   U_INTERNAL_ASSERT_POINTER(proc)

   if (proc->parent()) proc->wait();
}
*/

U_NO_EXPORT void UServer_Base::manageCommand(const char* format, uint32_t fmt_size, ...)
{
   U_TRACE(0, "UServer_Base::manageCommand(%.*S,%u)", fmt_size, format, fmt_size)

   U_INTERNAL_ASSERT_POINTER(format)

   static int fd_stderr;

   if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/command.err");

   UString cmd(U_CAPACITY);

   va_list argp;
   va_start(argp, fmt_size);

   cmd.vsnprintf(format, fmt_size, argp);

   va_end(argp);

   UString output = UCommand::outputCommand(cmd, U_NULLPTR, -1, fd_stderr);

   logCommandMsgError(cmd.data(), true);

   if (UCommand::exit_value) U_WARNING("command failed: EXIT_VALUE=%d OUTPUT=%V", UCommand::exit_value, output.rep);

   (void) proc->waitAll();
}

RETSIGTYPE UServer_Base::handlerForSigTERM(int signo)
{
   U_TRACE(0, "[SIGTERM] UServer_Base::handlerForSigTERM(%d)", signo)

   flag_loop = false;

   U_SRV_FLAG_SIGTERM      =
   UNotifier::flag_sigterm = true;

   U_INTERNAL_ASSERT_POINTER(proc)

   if (proc->parent())
      {
#  if defined(U_LINUX) && defined(ENABLE_THREAD)
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

      if (preforked_num_kids)
         {
#     if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
         if (preforked_num_kids == -1) UNotifier::pthread->suspend();
#       ifdef DEBUG
         u_trace_unlock();
#       endif
#     endif

#     ifndef U_LOG_DISABLE
         if (isLog()) logMemUsage("SIGTERM");
#     endif

         U_EXIT(0);
         }
      }
}

U_NO_EXPORT bool UServer_Base::clientImageHandlerRead()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::clientImageHandlerRead()")

   U_INTERNAL_ASSERT_POINTER(csocket)
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

#ifndef U_LOG_DISABLE
void UServer_Base::logNewClient(USocket* psocket, UClientImage_Base* lClientImage)
{
   U_TRACE(0, "UServer_Base::logNewClient(%p,%p)", psocket, lClientImage)

   if (isLog())
      {
#  ifdef USE_LIBSSL
      if (bssl) lClientImage->logCertificate();
#  endif

      USocketExt::setRemoteInfo(psocket, *(lClientImage->logbuf));

      U_INTERNAL_ASSERT(lClientImage->logbuf->isNullTerminated())

      char buffer[32];
      uint32_t len = setNumConnection(buffer);

      log->log(U_CONSTANT_TO_PARAM("New client connected from %v, %.*s clients currently connected"), lClientImage->logbuf->rep, len, buffer);

#  ifdef U_WELCOME_SUPPORT
      if (msg_welcome) log->log(U_CONSTANT_TO_PARAM("Sending welcome message to %v"), lClientImage->logbuf->rep);
#  endif
      }
}
#endif

#if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
#  define CSOCKET psocket
#  define CLIENT_IMAGE lClientIndex
#  define CLIENT_ADDRESS lclient_address
#  define CLIENT_ADDRESS_LEN lclient_address_len
#  define CLIENT_IMAGE_HANDLER_READ (pClientImage = lClientIndex, csocket = psocket, \
                                     client_address = lclient_address, client_address_len = lclient_address_len, clientImageHandlerRead())
#else
#  define CSOCKET csocket
#  define CLIENT_IMAGE pClientImage
#  define CLIENT_ADDRESS client_address
#  define CLIENT_ADDRESS_LEN client_address_len
#  define CLIENT_IMAGE_HANDLER_READ clientImageHandlerRead()
#endif

// This method is called to accept a new connection on the server socket (listening).
// Let all the process race to call accept() on the socket; since the latter is
// non-blocking, that will effectively act as load balancing

int UServer_Base::handlerRead()
{
   U_TRACE_NO_PARAM(1, "UServer_Base::handlerRead()")

   U_INTERNAL_DUMP("nClientIndex = %u", nClientIndex)

   U_INTERNAL_ASSERT_MINOR(nClientIndex, UNotifier::max_connection)

   // This loops until the accept() fails, trying to start new connections as fast as possible so we don't overrun the listen queue

   pClientImage = vClientImage + nClientIndex;

#if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   USocket* psocket;
   uint32_t lclient_address_len = 0;
   char* lclient_address = U_NULLPTR;
   UClientImage_Base* lClientIndex = pClientImage;
#endif
   int cround = 0;
#ifdef DEBUG
   CLIENT_ADDRESS_LEN = 0;
   uint32_t numc, nothing = 0;
#endif

loop:
   U_INTERNAL_ASSERT_MINOR(CLIENT_IMAGE, eClientImage)
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD)

   CSOCKET = CLIENT_IMAGE->socket;

   U_INTERNAL_DUMP("\n----------------------------------------\n"
                   "vClientImage[%u].last_event        = %#3D\n"
                   "vClientImage[%u].sfd               = %d\n"
                   "vClientImage[%u].UEventFd::fd      = %d\n"
                   "vClientImage[%u].socket            = %p\n"
                   "vClientImage[%u].socket->flags     = %u %B\n"
                   "vClientImage[%u].socket->iSockDesc = %d"
                   "\n----------------------------------------\n",
                   (CLIENT_IMAGE - vClientImage), CLIENT_IMAGE->last_event,
                   (CLIENT_IMAGE - vClientImage), CLIENT_IMAGE->sfd,
                   (CLIENT_IMAGE - vClientImage), CLIENT_IMAGE->UEventFd::fd,
                   (CLIENT_IMAGE - vClientImage), CSOCKET,
                   (CLIENT_IMAGE - vClientImage), CSOCKET->flags, CSOCKET->flags,
                   (CLIENT_IMAGE - vClientImage), CSOCKET->iSockDesc)

   if (CSOCKET->isOpen()) // busy
      {
      if (cround >= 2) // polling mode
         {
         if (CLIENT_IMAGE_HANDLER_READ == false)
            {
            U_INTERNAL_DUMP("cround = %u UNotifier::num_connection = %u num_client_threshold = %u", cround, UNotifier::num_connection, num_client_threshold)

            if (UNotifier::num_connection < num_client_threshold)
               {
               U_DEBUG("It has returned below the client threshold(%u): preallocation(%u) num_connection(%u)",
                           num_client_threshold, UNotifier::max_connection, UNotifier::num_connection - UNotifier::min_connection)

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
         U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

         if ((u_now->tv_sec - CLIENT_IMAGE->last_event) >= ptime->UTimeVal::tv_sec)
            {
#        if !defined(U_LOG_DISABLE) || (!defined(USE_LIBEVENT) && defined(HAVE_EPOLL_WAIT) && defined(DEBUG))
            called_from_handlerTime = false;
#        endif

            if (handlerTimeoutConnection(CLIENT_IMAGE))
               {
               UNotifier::handlerDelete((UEventFd*)CLIENT_IMAGE);

               goto try_accept;
               }
            }
         }

try_next:
      if (++CLIENT_IMAGE >= eClientImage)
         {
         U_INTERNAL_ASSERT_POINTER(vClientImage)

         CLIENT_IMAGE = vClientImage;

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

            U_DEBUG("Out of space on client image preallocation(%u): num_connection(%u)", UNotifier::max_connection, UNotifier::num_connection - UNotifier::min_connection)
            }
         }

      goto loop;
      }

   if (cround >= 2) goto try_next; // polling mode

try_accept:
   U_INTERNAL_ASSERT(CSOCKET->isClosed())
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD)

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

#  ifndef U_LOG_DISABLE
      if (isLog()                   &&
          flag_loop                 && // NB: we check to avoid SIGTERM event...
          CSOCKET->iState != -EINTR && // NB: we check to avoid log spurious EINTR on accept() by any timer...
          CSOCKET->iState != -EAGAIN)
         {
         CSOCKET->setMsgError();

         if (u_buffer_len)
            {
            log->log(U_CONSTANT_TO_PARAM("WARNING: accept new client failed %.*S"), u_buffer_len, u_buffer);

            u_buffer_len = 0;
            }
         }
#  endif

#  if defined(U_EPOLLET_POSTPONE_STRATEGY)
      if (CSOCKET->iState == -EAGAIN) U_ClientImage_state = U_PLUGIN_HANDLER_AGAIN;
#  endif

      goto end;
      }

   setClientAddress(CSOCKET, CLIENT_ADDRESS, CLIENT_ADDRESS_LEN);

#ifdef U_EVASIVE_SUPPORT
   if (checkHold(CSOCKET->remoteIPAddress().getInAddr()))
      {
      CSOCKET->abortive_close();

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

#if defined(_MSWINDOWS_) && !defined(USE_LIBEVENT)
   if (CSOCKET->iSockDesc >= FD_SETSIZE)
      {
      CSOCKET->abortive_close();

      U_SRV_LOG("WARNING: new client connected from %.*S, connection denied by FD_SETSIZE(%u)", CLIENT_ADDRESS_LEN, CLIENT_ADDRESS, FD_SETSIZE);

      goto end;
      }
#endif

#ifdef U_ACL_SUPPORT
   if (vallow_IP && UIPAllow::isAllowed(CSOCKET->remoteIPAddress().getInAddr(), *vallow_IP) == false)
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
       (vallow_IP_prv == U_NULLPTR            ||
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

#ifndef U_LOG_DISABLE
   ULock::atomicIncrement(U_SRV_TOT_CONNECTION);

   U_INTERNAL_DUMP("U_SRV_TOT_CONNECTION = %u", U_SRV_TOT_CONNECTION)
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
         int pid, status;

         CSOCKET->close();

         U_SRV_LOG("Started new child (pid %d), up to %u children", proc->pid(), UNotifier::num_connection - UNotifier::min_connection);

retry:   pid = UProcess::waitpid(-1, &status, WNOHANG); // NB: to avoid too much zombie...

         if (pid > 0)
            {
            --UNotifier::num_connection;

#        ifndef U_LOG_DISABLE
            if (isLog())
               {
               char buffer[128];

               log->log(U_CONSTANT_TO_PARAM("Child (pid %d) exited with value %d (%s), down to %u children"),
                        pid, status, UProcess::exitInfo(buffer, status), UNotifier::num_connection - UNotifier::min_connection);
               }
#        endif

            goto retry;
            }

         U_SRV_LOG("Waiting for connection on port %u", port);

         U_RETURN(U_NOTIFIER_OK);
         }

      if (proc->child())
         {
         // ---------------------------------------------------------------------------------------------------------
         // NB: the forked child don't accept new client, but we need anyway the event manager because
         //     the forked child must feel the possibly timeout for request from the new client...
         // ---------------------------------------------------------------------------------------------------------

         uint32_t num_connection_save = UNotifier::num_connection;
                                        UNotifier::num_connection = 0;

         UNotifier::init();

         UNotifier::num_connection = num_connection_save;
         }
      }
#endif

#ifndef U_LOG_DISABLE
   logNewClient(CSOCKET, CLIENT_IMAGE);
#endif

#ifdef U_WELCOME_SUPPORT
   if (msg_welcome &&
       USocketExt::write(CSOCKET, *msg_welcome, timeoutMS) == false)
      {
      CSOCKET->abortive_close();

      CLIENT_IMAGE->UClientImage_Base::handlerDelete();

      goto next;
      }
#endif

#if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   if (preforked_num_kids == -1) lClientIndex->UEventFd::fd = psocket->iSockDesc;
   else
#endif
   {
#if defined(DEBUG) && (!defined(U_SERVER_CAPTIVE_PORTAL) || defined(ENABLE_THREAD))
   uint32_t len = 0;
   int cpu = U_SYSCALL_NO_PARAM(sched_getcpu), scpu = -1;

# ifdef SO_INCOMING_CPU
   if (USocket::bincoming_cpu)
      {
      len = sizeof(socklen_t);

      (void) CSOCKET->getSockOpt(SOL_SOCKET, SO_INCOMING_CPU, (void*)&scpu, len);

      len = (USocket::incoming_cpu == scpu ? 0 : U_CONSTANT_SIZE(" [DIFFER]"));
      }
# endif

   U_INTERNAL_DUMP("USocket::incoming_cpu = %d USocket::bincoming_cpu = %b sched cpu = %d socket cpu = %d", USocket::incoming_cpu, USocket::bincoming_cpu, cpu, scpu)

   if (len) U_DEBUG("UServer_Base::handlerRead(): CPU: %d sched(%d) socket(%d)%.*s", USocket::incoming_cpu, cpu, scpu, len, " [DIFFER]")
#endif

   if (CLIENT_IMAGE_HANDLER_READ == false) goto next;
   }

   U_INTERNAL_ASSERT(CSOCKET->isOpen())
   U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD)

#if defined(HAVE_EPOLL_CTL_BATCH) && !defined(USE_LIBEVENT)
   UNotifier::batch((UEventFd*)CLIENT_IMAGE);
#else
   UNotifier::insert((UEventFd*)CLIENT_IMAGE);
#endif

   if (++CLIENT_IMAGE >= eClientImage) CLIENT_IMAGE = vClientImage;

next:
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

      CLIENT_IMAGE = vClientImage;

      U_DEBUG("It has passed the client threshold(%u): preallocation(%u) num_connection(%u)",
                  num_client_threshold, UNotifier::max_connection, UNotifier::num_connection - UNotifier::min_connection)
      }

   goto loop;
   }

end:
#if defined(HAVE_EPOLL_CTL_BATCH) && !defined(USE_LIBEVENT)
   UNotifier::insertBatch();
#endif

   nClientIndex = CLIENT_IMAGE - vClientImage;

   U_INTERNAL_DUMP("nClientIndex = %u", nClientIndex)

   U_INTERNAL_ASSERT_MINOR(nClientIndex, UNotifier::max_connection)

   U_RETURN(U_NOTIFIER_OK);
}

#undef CSOCKET
#undef CLIENT_IMAGE
#undef CLIENT_ADDRESS
#undef CLIENT_ADDRESS_LEN
#undef CLIENT_IMAGE_HANDLER_READ

#ifndef U_LOG_DISABLE
uint32_t UServer_Base::setNumConnection(char* ptr)
{
   U_TRACE(0, "UServer_Base::setNumConnection(%p)", ptr)

# ifdef USERVER_UDP
   *ptr = '0';

   U_RETURN(1);
# else
   uint32_t len, sz = UNotifier::num_connection - UNotifier::min_connection - 1;

   if (preforked_num_kids <= 0) len = u_num2str32(sz, ptr) - ptr;
   else
      {
      char* start = ptr;

      *ptr = '(';
       ptr = u_num2str32(sz, ptr+1);
      *ptr = '/';

      len = U_SRV_TOT_CONNECTION;

      U_INTERNAL_DUMP("len = %d", len)

           if ((int32_t)len < 0) U_SRV_TOT_CONNECTION = len = 0;
      else if (len >= 1 &&
               flag_loop) // NB: check for SIGTERM event...
         {
         --len;
         }

       ptr = u_num2str32(len, ptr+1);
      *ptr = ')';

      len = ptr-start+1;
      }

   U_RETURN(len);
# endif
}
#endif

bool UServer_Base::handlerTimeoutConnection(void* cimg)
{
   U_TRACE(0, "UServer_Base::handlerTimeoutConnection(%p)", cimg)

   U_INTERNAL_ASSERT_POINTER(cimg)
   U_INTERNAL_ASSERT_POINTER(pthis)
   U_INTERNAL_ASSERT_POINTER(ptime)
   U_INTERNAL_ASSERT_DIFFERS(timeoutMS, -1)

   U_INTERNAL_DUMP("pthis = %p handler_other = %p handler_inotify = %p", pthis, handler_other, handler_inotify)

   if (cimg == pthis         ||
       cimg == handler_other ||
       cimg == handler_inotify)
      {
      U_RETURN(false);
      }

   if (((UClientImage_Base*)cimg)->handlerTimeout() == U_NOTIFIER_DELETE)
      {
#  ifndef U_LOG_DISABLE
      if (isLog())
         {
         if (called_from_handlerTime)
            {
            log->log(U_CONSTANT_TO_PARAM("handlerTime: client connected didn't send any request in %u secs (timeout), close connection %v"),
                     ptime->UTimeVal::tv_sec, ((UClientImage_Base*)cimg)->logbuf->rep);
            }
         else
            {
            log->log(U_CONSTANT_TO_PARAM("handlerTimeoutConnection: client connected didn't send any request in %u secs, close connection %v"),
                     UNotifier::last_event - ((UClientImage_Base*)cimg)->last_event, ((UClientImage_Base*)cimg)->logbuf->rep);
            }
         }
#  endif
#  if !defined(USE_LIBEVENT) && defined(HAVE_EPOLL_WAIT) && defined(DEBUG)
      if (called_from_handlerTime)
         {
         U_DEBUG("handlerTime: client connected didn't send any request in %u secs (timeout %u sec) - "
                 "UEventFd::fd = %d socket->iSockDesc = %d UNotifier::num_connection = %d UNotifier::min_connection = %d",
                 UNotifier::last_event - ((UClientImage_Base*)cimg)->last_event, ptime->UTimeVal::tv_sec,
                                         ((UClientImage_Base*)cimg)->UEventFd::fd,
                                         ((UClientImage_Base*)cimg)->socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection)
         }
      else
         {
         U_DEBUG("handlerTimeoutConnection: client connected didn't send any request in %u secs - "
                 "UEventFd::fd = %d socket->iSockDesc = %d UNotifier::num_connection = %d UNotifier::min_connection = %d",
                 UNotifier::last_event - ((UClientImage_Base*)cimg)->last_event,
                                         ((UClientImage_Base*)cimg)->UEventFd::fd,
                                         ((UClientImage_Base*)cimg)->socket->iSockDesc, UNotifier::num_connection, UNotifier::min_connection)
         }
#  endif

      U_RETURN(true); // NB: return true mean that we want to erase the item...
      }

   U_RETURN(false);
}

void UServer_Base::runLoop(const char* user)
{
   U_TRACE(0, "UServer_Base::runLoop(%S)", user)

#ifdef USERVER_UDP
   if (budp == false)
#endif
   {
   if (pluginsHandlerFork() != U_PLUGIN_HANDLER_FINISHED) U_ERROR("Plugins stage fork failed");
   }

   socket->reusePort(socket_flags);

#ifdef U_LINUX
# if defined(USERVER_UDP) || defined(USERVER_IPC)
   if (budp == false &&
       bipc == false)
# endif
   {
   /**
    * Let's say an application just issued a request to send a small block of data. Now, we could
    * either send the data immediately or wait for more data. Some interactive and client-server
    * applications will benefit greatly if we send the data right away. For example, when we are
    * sending a short request and awaiting a large response, the relative overhead is low compared
    * to the total amount of data transferred, and the response time could be much better if the
    * request is sent immediately. This is achieved by setting the TCP_NODELAY option on the socket,
    * which disables the Nagle algorithm.
    *
    * Linux (along with some other OSs) includes a TCP_DEFER_ACCEPT option in its TCP implementation.
    * Set on a server-side listening socket, it instructs the kernel not to wait for the final ACK packet
    * and not to initiate the process until the first packet of real data has arrived. After sending the SYN/ACK,
    * the server will then wait for a data packet from a client. Now, only three packets will be sent over the
    * network, and the connection establishment delay will be significantly reduced, which is typical for HTTP.
    * NB: Takes an integer value (seconds)
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
    */

   if (tcp_linger_set > -2) socket->setTcpLinger(tcp_linger_set);

# if !defined(U_SERVER_CAPTIVE_PORTAL) || defined(ENABLE_THREAD)
                           socket->setTcpNoDelay();
                           socket->setTcpFastOpen();
                           socket->setTcpDeferAccept();
   if (bssl == false)      socket->setBufferSND(500 * 1024); // 500k: for major size we assume is better to use sendfile()
   if (set_tcp_keep_alive) socket->setTcpKeepAlive();
# endif
   }

   if (user)
      {
      if (u_runAsUser(user, false) == false) U_ERROR("Set user %S context failed", user);

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

#ifdef USERVER_UDP
   if (budp == false)
#endif
   {
   UNotifier::init();

   U_INTERNAL_DUMP("UNotifier::min_connection = %d", UNotifier::min_connection)

   if (UNotifier::min_connection)
      {
      if (binsert)         UNotifier::insert(pthis,           EPOLLEXCLUSIVE | EPOLLROUNDROBIN); // NB: we ask to be notified for request of connection (=> accept)
      if (handler_other)   UNotifier::insert(handler_other,   EPOLLEXCLUSIVE | EPOLLROUNDROBIN); // NB: we ask to be notified for request from generic system
      if (handler_inotify) UNotifier::insert(handler_inotify, EPOLLEXCLUSIVE | EPOLLROUNDROBIN); // NB: we ask to be notified for change of file system (=> inotify)
      }

#if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   U_INTERNAL_ASSERT_EQUALS(UNotifier::pthread, U_NULLPTR)

   if (preforked_num_kids == -1)
      {
      U_NEW(UClientThread, UNotifier::pthread, UClientThread);

#  ifdef _MSWINDOWS_
      InitializeCriticalSection(&UNotifier::mutex);
#  endif

      UNotifier::pthread->start(50);

      proc->_pid = UNotifier::pthread->id;

      U_ASSERT(proc->parent())
      }
#endif
   }

   U_SRV_LOG("Waiting for connection on port %u", port);

#if defined(U_LINUX) && defined(ENABLE_THREAD)
   (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_UNBLOCK, &mask, U_NULLPTR);
#endif

   if (ptime)
      {
      UTimer::insert(ptime);

#  if !defined(U_LOG_DISABLE) && defined(DEBUG)
      last_event = u_now->tv_sec;
#  endif
      }

#ifdef USERVER_UDP
   if (budp)
      {
      struct stat st;
      char buffer[U_PATH_MAX];
      HINSTANCE handle = U_NULLPTR;
      uint32_t len = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM(U_LIBEXECDIR "/usp/udp.%s"), U_LIB_SUFFIX);

      if (U_SYSCALL(stat, "%S,%p", buffer, &st))
         {
         U_WARNING("I can't found the usp page: %.*S", len, buffer);
         }
      else
         {
         if ((handle = UDynamic::dload(buffer)) == U_NULLPTR ||
             (runDynamicPage_udp = (vPFi)UDynamic::lookup(handle, "runDynamicPage_udp")) == U_NULLPTR)
            {
            U_WARNING("load failed of usp page: %.*S", len, buffer);
            }
         else
            {
            runDynamicPage_udp(U_DPAGE_INIT);
            runDynamicPage_udp(U_DPAGE_FORK);
            }
         }

      csocket                    =
      pClientImage->socket       = socket;
      pClientImage->UEventFd::fd = socket->iSockDesc;

      UClientImage_Base::callerHandlerRead = UServer_Base::handlerUDP;

      U_INTERNAL_DUMP("handler_other = %p handler_inotify = %p UNotifier::num_connection = %u UNotifier::min_connection = %u",
                       handler_other,     handler_inotify,     UNotifier::num_connection,     UNotifier::min_connection)

      // NB: we can go directly on recvFrom() and block on it...

      U_INTERNAL_ASSERT_EQUALS(socket_flags & O_NONBLOCK, 0)

      while (flag_loop)
         {
         if (pClientImage->handlerRead() == U_NOTIFIER_DELETE) break;

#     ifndef U_LOG_DISABLE
         if (isLog())
            {
            pClientImage->logbuf->setEmpty();

            log->log(U_CONSTANT_TO_PARAM("Waiting for connection on port %u"), port);
            }
#     endif
         }

      if (runDynamicPage_udp)
         {
         runDynamicPage_udp(U_DPAGE_DESTROY);

         UDynamic::dclose(handle);
         }
      }
   else
#endif
   {
   while (flag_loop)
      {
      U_INTERNAL_DUMP("handler_other = %p handler_inotify = %p UNotifier::num_connection = %u UNotifier::min_connection = %u",
                       handler_other,     handler_inotify,     UNotifier::num_connection,     UNotifier::min_connection)

#  if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
      if (preforked_num_kids != -1)
#  endif
      {
      if (UNotifier::min_connection) // NB: we need to notify something to someone...
         {
         UNotifier::waitForEvent();

         if (ptime &&
             UNotifier::nfd_ready > 0)
            {
#        if !defined(U_LOG_DISABLE) && defined(DEBUG)
            last_event = u_now->tv_sec;

#          ifndef _MSWINDOWS_
            if (monitoring_process &&
                U_SYSCALL_NO_PARAM(getppid) == 1)
               {
               U_ERROR("the monitoring process has crashed, exiting...");
               }
#          endif
#        endif

            UTimer::updateTimeToExpire(ptime);
            }

         U_ASSERT_EQUALS(UNotifier::empty(), false)

         continue;
         }
      }

      // NB: we can go directly on accept() and block on it...

      U_INTERNAL_ASSERT_EQUALS(socket_flags & O_NONBLOCK, 0)

#  if !defined(ENABLE_THREAD) || defined(USE_LIBEVENT) || !defined(U_SERVER_THREAD_APPROACH_SUPPORT)
      U_INTERNAL_ASSERT(UNotifier::min_connection == UNotifier::num_connection)
#  endif

      (void) pthis->UServer_Base::handlerRead();
      }
   }
}

void UServer_Base::run()
{
   U_TRACE_NO_PARAM(1, "UServer_Base::run()")

   init();

   int status;
   uint32_t crash_counter = 0;
   bool baffinity = false, bmail;
   UTimeVal to_sleep(0L, 500L * 1000L);
   const char* user = (as_user->empty() ? U_NULLPTR : as_user->data());

   UHttpClient_Base::server_context_flag = true;

   /**
    * PREFORK_CHILD number of child server processes created at startup:
    *
    * -1 - thread approach (experimental)
    *  0 - serialize, no forking
    *  1 - classic, forking after accept client
    * >1 - pool of process serialize plus monitoring process
    */

   U_INTERNAL_DUMP("monitoring_process = %b", monitoring_process)

   if (monitoring_process == false)
      {
      runLoop(user);

      goto stop;
      }

   /**
    * Main loop for the parent process with the new preforked implementation.
    * The parent is just responsible for keeping a pool of children and they accept connections themselves...
    */

   int nkids;
   cpu_set_t cpuset;
   pid_t pid, pid_to_wait;

#if defined(HAVE_SCHED_GETAFFINITY) && (!defined(U_SERVER_CAPTIVE_PORTAL) || defined(ENABLE_THREAD))
   if (u_get_num_cpu() > 1 &&
       (preforked_num_kids % u_num_cpu) == 0)
      {
      baffinity = true;

      U_SRV_LOG("cpu affinity is to be set; thread count (%u) multiple of cpu count (%u)", preforked_num_kids, u_num_cpu);
      }
#endif

   U_INTERNAL_ASSERT_EQUALS(rkids, 0)

   nkids = (preforked_num_kids <= 0 ? 1 : (pid_to_wait = -1, preforked_num_kids));

   U_INTERNAL_DUMP("nkids = %u", nkids)

   while (flag_loop)
      {
      u_need_root(false);

      while (rkids < nkids)
         {
         if (proc->fork() &&
             proc->parent())
            {
            ++rkids;

            if (preforked_num_kids <= 0) pid_to_wait = proc->_pid;

            U_SRV_LOG("Started new child (pid %d), up to %u children", proc->_pid, rkids);

            U_INTERNAL_DUMP("up to %u children, UNotifier::num_connection = %d", rkids, UNotifier::num_connection)
            }

         if (proc->child())
            {
#        ifdef U_SSE_ENABLE // SERVER SENT EVENTS (SSE)
            UFile::close(sse_socketpair[0]);
#        endif

            U_INTERNAL_DUMP("child = %P UNotifier::num_connection = %d", UNotifier::num_connection)

#        if defined(HAVE_SCHED_GETAFFINITY) && (!defined(U_SERVER_CAPTIVE_PORTAL) || defined(ENABLE_THREAD))
            if (baffinity)
               {
               CPU_ZERO(&cpuset);

               u_bind2cpu(&cpuset, rkids % u_num_cpu); // Pin the process to a particular cpu...

#           ifdef SO_INCOMING_CPU
               USocket::incoming_cpu = rkids % u_num_cpu;
#           endif

#           ifndef U_LOG_DISABLE
               if (isLog())
                  {
                  uint32_t sz;
                  char buffer[64];
                  int cpu = U_SYSCALL_NO_PARAM(sched_getcpu);

#              ifdef HAVE_SCHED_GETCPU
                  if (USocket::incoming_cpu != cpu &&
                      USocket::incoming_cpu != -1)
                     {
                     sz = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM(" (EXPECTED CPU %d)"), USocket::incoming_cpu);
                     }
                  else
#              endif
                  sz = 0;

                  log->log(U_CONSTANT_TO_PARAM("New child started, affinity mask: %x, cpu: %d%.*s"),  CPUSET_BITS(&cpuset)[0], cpu, sz, buffer);
                  }
#           endif
               }

#          ifdef HAVE_LIBNUMA
            if (U_SYSCALL_NO_PARAM(numa_max_node))
               {
               struct bitmask* bmask = (struct bitmask*) U_SYSCALL(numa_bitmask_alloc, "%u", 16);

               (void) U_SYSCALL(numa_bitmask_setbit, "%p,%u", bmask, rkids % 2);

               U_SYSCALL_VOID(numa_set_membind,  "%p", bmask);
               U_SYSCALL_VOID(numa_bitmask_free, "%p", bmask);
               }
#          endif

            if (set_realtime_priority) u_switch_to_realtime_priority();
#        endif

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

            UInterrupt::setHandlerForSignal(SIGHUP, (sighandler_t)SIG_IGN); // NB: we can't use UInterrupt::erase() because it restore the old action (UInterrupt::init)...

#        ifdef U_LINUX
            (void) U_SYSCALL(prctl, "%d,%lu", PR_SET_PDEATHSIG, SIGTERM);
#        endif

            runLoop(user);

            return;
            }

         // Don't start them too quickly, or we might overwhelm a machine that's having trouble

         to_sleep.nanosleep();

#     if defined(U_SERVER_CAPTIVE_PORTAL) && !defined(ENABLE_THREAD)
         if (proc->_pid == -1) // If the child don't start (not enough memory) we disable the monitoring process...
            {
            monitoring_process = false;

            runLoop(user);

            goto stop;
            }
#     endif
         }

      // wait for any children to exit, and then start some more

#  if defined(U_LINUX) && defined(ENABLE_THREAD)
      (void) U_SYSCALL(pthread_sigmask, "%d,%p,%p", SIG_UNBLOCK, &mask, U_NULLPTR);
#  endif

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

#     ifndef U_LOG_DISABLE
         if (isLog())
            {
            char buffer[128];

            log->log(U_CONSTANT_TO_PARAM("WARNING: child (pid %d) exited with value %d (%s), down to %u children"),
                     pid, status, UProcess::exitInfo(buffer, status), rkids);
            }
#     endif

         U_DEBUG("child (pid %d) exited with value %d, down to %u children - crash_counter = %u", pid, status, rkids, crash_counter+1);

         bmail = false;

         if (crashEmailAddress &&
             ++crash_counter > crash_count)
            {
            if ((u_now->tv_sec - last_time_email_crash) > U_ONE_DAY_IN_SECOND)
               {
               bmail                 = true;
               last_time_email_crash = u_now->tv_sec;
               }
            }

         if (bmail)
            {
            U_INTERNAL_ASSERT_POINTER(emailClient)

            pid_t lpid = startNewChild();

            if (lpid > 0) continue; // parent

            UString body(200U);

            body.snprintf(U_CONSTANT_TO_PARAM("%N (on %V): number of crashes %u has exceeded the threshold %u since %#5D"), IP_address->rep, crash_counter, crash_count, u_start_time);

            emailClient->sendEmail(*crashEmailAddress, U_STRING_FROM_CONSTANT("number of crashes has exceeded the threshold"), body);

            if (lpid == 0) endNewChild();
            }
         }
      }

   U_INTERNAL_ASSERT(proc->parent())

stop:
#ifdef USERVER_UDP
   if (budp == false)
#endif
   {
   if (pluginsHandlerStop() != U_PLUGIN_HANDLER_FINISHED) U_WARNING("Plugins stage stop failed");
   }

   manageWaitAll();

#ifndef U_LOG_DISABLE
   if (isLog() &&
       UNotifier::flag_sigterm)
      {
      logMemUsage("SIGTERM");
      }
#endif

   if (proc->parent() ||
       preforked_num_kids <= 0)
      {
      to_sleep.nanosleep();

      closeLog();
      }

#ifdef U_SSE_ENABLE // SERVER SENT EVENTS (SSE)
   if (pthread_sse) sse_vclient->clear();
#endif

#ifdef DEBUG
   pthis->deallocate();
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

#  ifdef U_LOG_DISABLE
            (void) UProcess::removeZombies();
#  else
      uint32_t n = UProcess::removeZombies();

      ULock::atomicIncrement(U_SRV_TOT_CONNECTION);

      U_SRV_LOG("Started new child (pid %d) for parallelization (%d) - removed %u zombies", pid, U_SRV_TOT_CONNECTION, n);
#  endif

      U_RETURN(pid); // parent
      }

   if (p.child()) U_RETURN(0);

   U_RETURN(-1);
}

__noreturn void UServer_Base::endNewChild()
{
   U_TRACE_NO_PARAM(0, "UServer_Base::endNewChild()")

#ifndef U_LOG_DISABLE
   ULock::atomicDecrement(U_SRV_TOT_CONNECTION);

   U_INTERNAL_DUMP("cnt_parallelization = %u U_SRV_FLAG_SIGTERM = %b", U_SRV_TOT_CONNECTION, U_SRV_FLAG_SIGTERM)

   if (U_SRV_FLAG_SIGTERM == false) U_SRV_LOG("child for parallelization ended (%u)", U_SRV_TOT_CONNECTION);
#endif

   U_EXIT(0);
}

bool UServer_Base::startParallelization(uint32_t nclient)
{
   U_TRACE(0, "UServer_Base::startParallelization(%u)", nclient)

#if defined(ENABLE_THREAD) && !defined(USE_LIBEVENT) && defined(U_SERVER_THREAD_APPROACH_SUPPORT)
   if (preforked_num_kids != -1)
#endif
   {
   if (isParallelizationGoingToStart(nclient))
      {
      pid_t pid = startNewChild();

      if (pid > 0)
         {
         // NB: from now it is responsability of the child to services the request from the client on the same connection...

         csocket->close();

         UClientImage_Base::resetPipelineAndSetCloseConnection();

         U_ClientImage_parallelization = U_PARALLELIZATION_PARENT;

         U_RETURN(true);
         }

      if (pid == 0) U_ClientImage_parallelization = U_PARALLELIZATION_CHILD;
      }
   }

   U_RETURN(false);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UServer_Base::dump(bool reset) const
{
   *UObjectIO::os << "port                      " << port                       << '\n'
                  << "map_size                  " << map_size                   << '\n'
                  << "flag_loop                 " << flag_loop                  << '\n'
                  << "timeoutMS                 " << timeoutMS                  << '\n'
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

   return U_NULLPTR;
}
#endif
