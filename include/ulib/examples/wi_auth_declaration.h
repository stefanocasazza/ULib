// wi_auth_declaration.h

#ifndef U_WI_AUTH_DECLARATION_H
#define U_WI_AUTH_DECLARATION_H 1

#include <ulib/date.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/plugin/mod_ssi.h>
#include <ulib/net/server/plugin/mod_proxy_service.h>

#ifdef USE_LIBSSL
#  include <ulib/utility/des3.h>
#  include <ulib/ssl/certificate.h>
#  include <ulib/ssl/net/sslsocket.h>
#endif

static UString* ap;
static UString* ip;
static UString* uid;
static UString* mac;
static UString* pbody;
static UString* redir;
static UString* token;
static UString* ap_ref;
static UString* policy;
static UString* output;
static UString* gateway;
static UString* dir_reg;
static UString* ip_auth;
static UString* des3_key;
static UString* telefono;
static UString* ap_label;
static UString* mac_auth;
static UString* redirect;
static UString* url_nodog;
static UString* cert_auth;
static UString* ip_server;
static UString* login_url;
static UString* empty_str;
static UString* ap_ref_ap;
static UString* ap_address;
static UString* logout_url;
static UString* empty_list;
static UString* nodog_conf;
static UString* ap_hostname;
static UString* cookie_auth;
static UString* environment;
static UString* auth_domain;
static UString* policy_flat;
static UString* policy_daily;
static UString* virtual_name;
static UString* account_auth;
static UString* password_url;
static UString* title_default;
static UString* ap_ref_comune;
static UString* policy_traffic;
static UString* ldap_card_param;
static UString* ldap_user_param;
static UString* redirect_default;
static UString* registrazione_url;
static UString* allowed_web_hosts;
static UString* historical_log_dir;
static UString* dir_server_address;
static UString* ldap_session_param;
static UString* wiauth_card_basedn;
static UString* wiauth_user_basedn;
static UString* max_time_no_traffic;
static UString* login_nodog_template;
static UString* message_page_template;
static UString* status_nodog_template;
static UString* wiauth_session_basedn;
static UString* status_network_template;
static UString* status_nodog_ap_template;

static UString* help_url;
static UString* wallet_url;
static UString* fmt_auth_cmd;
static UString* url_banner_ap;
static UString* url_banner_comune;
static UString* url_banner_ap_path;
static UString* url_banner_comune_path;

static UString* time_done;
static UString* time_counter;
static UString* time_consumed;
static UString* time_available;
static UString* traffic_done;
static UString* traffic_counter;
static UString* traffic_consumed;
static UString* traffic_available;

static UString* user_UploadRate;
static UString* user_DownloadRate;

static UCache*        cache;
static UCache*  admin_cache;
static UCache* policy_cache;

static UFile* file_LOG;
static UFile* file_WARNING;
static UFile* file_RECOVERY;
static UFile* file_UTILIZZO;
static UFile* url_banner_ap_default;
static UFile* url_banner_comune_default;

static UVector<UString>*        vuid;
static UHashMap<UString>*       table;
static UVector<UIPAllow*>*      pvallow;
static UVector<UIPAllow*>*      vallow_IP_user;
static UVector<UIPAllow*>*      vallow_IP_request;
static UHttpClient<UTCPSocket>* client;

static bool     user_exist;
static bool     isIP, isMAC, ap_address_trust;
static UPing*   sockp;
static uint32_t index_access_point,
                num_users, num_users_connected, num_users_delete, num_ap_delete,
                num_ap, num_ap_noconsume, num_ap_up, num_ap_down, num_ap_open, num_ap_unreachable;

static const char* ptr1;
static const char* ptr2;
static const char* ptr3;
static const char* ptr4;

static uint32_t totale1, totale2, totale3;
static uint64_t totale4;

class WiAuthUser;
class WiAuthNodog;
class WiAuthDataStorage;

static WiAuthUser*        user_rec;
static WiAuthNodog*      nodog_rec;
static WiAuthDataStorage* data_rec;

static URDBObjectHandler<UDataStorage*>* db_user;
static URDBObjectHandler<UDataStorage*>* db_nodog;

#define IP_UNIFI                "151.11.47.5"
#define IP_UNIFI_TMP            "159.213.248.230"
#define IP_CASCINE              "159.213.248.232"
#define NAMED_PIPE              "/tmp/wi_auth_db.op"
#define FIRENZECARD_REDIR       "http://wxfi.comune.fi.it/?ap=%v"
#define LOGIN_VALIDATE_REDIR    "http://www.google.com/login_validate?%v"
#define LOGIN_VALIDATE_REDIR_FI "http://151.11.47.4/login_validate?%v"

//#define IP_CASCINE              "151.11.47.3"
//#define FIRENZECARD_REDIR       "http://159.213.248.2/wxfi/?ap=%s"
//#define LOGIN_VALIDATE_REDIR_FI "http://151.11.45.77/cgi-bin/login_validata.cgi?%v"

#define U_LOGGER(fmt,args...) ULog::log(file_WARNING->getFd(), "%v: " fmt, UClientImage_Base::request_uri->rep , ##args)

class WiAuthDataStorage : public UDataStorage {
public:

   uint64_t       traffico_generato_giornaliero_globale;
   uint32_t         utenti_connessi_giornaliero_globale,
            tempo_permanenza_utenti_giornaliero_globale;

   // COSTRUTTORE

   WiAuthDataStorage() : UDataStorage(*UHTTP::str_storage_keyid)
      {
      U_TRACE_REGISTER_OBJECT(5, WiAuthDataStorage, "", 0)

      utenti_connessi_giornaliero_globale         =
      tempo_permanenza_utenti_giornaliero_globale = 0;
      traffico_generato_giornaliero_globale       = 0;
      }

   ~WiAuthDataStorage()
      {
      U_TRACE_UNREGISTER_OBJECT(5, WiAuthDataStorage)

      UHTTP::db_session->setPointerToDataStorage(0);
      }

   // define method VIRTUAL of class UDataStorage

   virtual char* toBuffer()
      {
      U_TRACE(5, "WiAuthDataStorage::toBuffer()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE,
                                 "%u %u %llu",
                             utenti_connessi_giornaliero_globale,
                     tempo_permanenza_utenti_giornaliero_globale,
                           traffico_generato_giornaliero_globale);

      U_RETURN(u_buffer);
      }

   virtual void fromStream(istream& is)
      {
      U_TRACE(5, "WiAuthDataStorage::fromStream(%p)", &is)

      U_CHECK_MEMORY

      is >>         utenti_connessi_giornaliero_globale 
         >> tempo_permanenza_utenti_giornaliero_globale
         >>       traffico_generato_giornaliero_globale;
      }

   // SERVICES

   void reset()
      {
      U_TRACE(5, "WiAuthDataStorage::reset()")

      U_CHECK_MEMORY

              utenti_connessi_giornaliero_globale =
      tempo_permanenza_utenti_giornaliero_globale = 0U;
            traffico_generato_giornaliero_globale = 0ULL;
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool breset) const { return ""; }
#endif
};

class WiAuthAccessPoint {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   uint64_t traffic_done;
   UString label, mac_mask, group_account_mask;
   uint32_t num_login, num_auth_domain_ALL, num_auth_domain_FICARD;
   bool noconsume;

   // COSTRUTTORE

   WiAuthAccessPoint()
      {
      U_TRACE_REGISTER_OBJECT(5, WiAuthAccessPoint, "", 0)

      reset();

      noconsume = false;
      }

   WiAuthAccessPoint(const UString& lbl) : label(lbl)
      {
      U_TRACE_REGISTER_OBJECT(5, WiAuthAccessPoint, "%V", lbl.rep)

      reset();

      noconsume = false;

      if (label.empty()) (void) label.assign(U_CONSTANT_TO_PARAM("ap"));
      }

   WiAuthAccessPoint(const WiAuthAccessPoint& _ap) : label(_ap.label), mac_mask(_ap.mac_mask), group_account_mask(_ap.group_account_mask)
      {
      U_TRACE_REGISTER_OBJECT(5, WiAuthAccessPoint, "%p", &_ap)

      U_MEMORY_TEST_COPY(_ap)

      ((UString&)_ap.label).clear();
      ((UString&)_ap.mac_mask).clear();
      ((UString&)_ap.group_account_mask).clear();

      traffic_done           = _ap.traffic_done;
      num_login              = _ap.num_login;
      num_auth_domain_ALL    = _ap.num_auth_domain_ALL;
      num_auth_domain_FICARD = _ap.num_auth_domain_FICARD;
      noconsume              = _ap.noconsume;
      }

   ~WiAuthAccessPoint()
      {
      U_TRACE_UNREGISTER_OBJECT(5, WiAuthAccessPoint)
      }

   // SERVICES

   void reset()
      {
      U_TRACE(5, "WiAuthAccessPoint::reset()")

      traffic_done = 0ULL;
      num_login = num_auth_domain_ALL = num_auth_domain_FICARD = 0U;
      }

   void toBuffer()
      {
      U_TRACE(5, "WiAuthAccessPoint::toBuffer()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(label)

      u_buffer_len += u__snprintf(u_buffer + u_buffer_len, U_BUFFER_SIZE,
                        " %u %v \"%v\" \"%v\""
                        " %u %u %u %llu",
                        noconsume, label.rep, group_account_mask.rep, mac_mask.rep,
                        num_login, num_auth_domain_ALL, num_auth_domain_FICARD, traffic_done);
      }

   // STREAMS

#if defined(U_STDCPP_ENABLE)
   void fromStream(istream& is)
      {
      U_TRACE(5, "WiAuthAccessPoint::fromStream(%p)", &is)

      U_CHECK_MEMORY

      is >> noconsume;

      is.get(); // skip ' '

      label.get(is);

      is.get(); // skip ' '

      group_account_mask.get(is);

      is.get(); // skip ' '

      mac_mask.get(is);

      is >> num_login
         >> num_auth_domain_ALL
         >> num_auth_domain_FICARD
         >> traffic_done;
      }

   friend istream& operator>>(istream& is, WiAuthAccessPoint& _ap) { _ap.fromStream(is); return is; }

#  ifdef DEBUG
   const char* dump(bool breset) const { return ""; }
#  endif
#endif

private:
   WiAuthAccessPoint& operator=(const WiAuthAccessPoint&) { return *this; }
};

class WiAuthNodog : public UDataStorage {
public:

   uint32_t sz;
   int port, status;
   UString hostname;
   long last_info, since, start;
   UVector<WiAuthAccessPoint*> vec_access_point;

   // COSTRUTTORE

   WiAuthNodog()
      {
      U_TRACE_REGISTER_OBJECT(5, WiAuthNodog, "", 0)

      sz        = 0;
      port      = 5280;
      status    = 1;
      last_info = since = start = u_now->tv_sec;
      }

   ~WiAuthNodog()
      {
      U_TRACE_UNREGISTER_OBJECT(5, WiAuthNodog)

      db_nodog->close();

      delete db_nodog;
      }

   // define method VIRTUAL of class UDataStorage

   virtual void clear()
      {
      U_TRACE(5, "WiAuthNodog::clear()")

      U_CHECK_MEMORY

      sz = 0;

              hostname.clear();
      vec_access_point.clear();
      }

   virtual char* toBuffer()
      {
      U_TRACE(5, "WiAuthNodog::toBuffer()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(hostname)
      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)
      U_ASSERT_EQUALS(sz, vec_access_point.size())

      u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE,
                        "%d %ld %ld %u %v %ld [",
                        status,
                        last_info,
                        since,
                        port,
                        hostname.rep,
                        start);

      for (uint32_t i = 0; i < sz; ++i) vec_access_point[i]->toBuffer();

      u_buffer[u_buffer_len++] = ' ';
      u_buffer[u_buffer_len++] = ']';
      u_buffer[u_buffer_len]   = '\0';

      U_INTERNAL_ASSERT_MINOR(u_buffer_len, U_BUFFER_SIZE)

      U_RETURN(u_buffer);
      }

   virtual void fromStream(istream& is)
      {
      U_TRACE(5, "WiAuthNodog::fromStream(%p)", &is)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_EQUALS(sz, 0)
      U_ASSERT(vec_access_point.empty())

      is >> status
         >> last_info
         >> since
         >> port;

      is.get(); // skip ' '

      hostname.get(is);

      is.get(); // skip ' '

      is >> start;

      is.get(); // skip ' '

      is >> vec_access_point;

      sz = vec_access_point.size();

      U_INTERNAL_ASSERT_MAJOR(sz, 0)
      }

   // SERVICES

   bool findLabel()
      {
      U_TRACE(5, "WiAuthNodog::findLabel()")

      U_CHECK_MEMORY

      U_ASSERT_EQUALS(sz, vec_access_point.size())

      if (ap_label->empty()) U_RETURN(false);

      U_INTERNAL_DUMP("ap_label = %V", ap_label->rep)

      for (index_access_point = 0; index_access_point < sz; ++index_access_point)
         {
         if (*ap_label == vec_access_point[index_access_point]->label) U_RETURN(true);
         }

      index_access_point = U_NOT_FOUND;

      U_LOGGER("*** LABEL(%v) NOT EXISTENT ON AP(%v) ***", ap_label->rep, hostname.rep);

      U_RETURN(false);
      }

   UString getLabelAP(uint32_t n)
      {
      U_TRACE(5, "WiAuthNodog::getLabelAP(%u)", n)

      U_CHECK_MEMORY

      U_ASSERT_EQUALS(sz, vec_access_point.size())

      if (n < sz) U_RETURN_STRING(vec_access_point[n]->label);

      return UString::getStringNull();
      }

   static bool checkMAC()
      {
      U_TRACE(5, "WiAuthNodog::checkMAC()")

      if (*mac                                  &&
          *ap_address                           &&
          db_nodog->getDataStorage(*ap_address) &&
          nodog_rec->findLabel())
         {
         WiAuthAccessPoint* ap_rec = nodog_rec->vec_access_point[index_access_point];

         U_INTERNAL_DUMP("mac = %V mac_mask = %V", mac->rep, ap_rec->mac_mask.rep)

         if (ap_rec->mac_mask &&
             UServices::dosMatchWithOR(*mac, ap_rec->mac_mask, FNM_IGNORECASE))
            {
            if (ap_rec->noconsume == false &&
                ap_rec->mac_mask.equal(U_CONSTANT_TO_PARAM("*")))
               {
               ap_rec->noconsume = true;

               U_LOGGER("*** FORCED NO CONSUME ON AP(%v) LABEL(%v) MAC_MASK(%v) ***", ap_address->rep, ap_label->rep, ap_rec->mac_mask.rep);

               *policy = *policy_daily; // NB: become flat on connection base because noconsume...
               }
            else
               {
               *policy = *policy_flat;
               }

            U_RETURN(true);
            }
         }

      U_RETURN(false);
      }

   void setStatus(int _status)
      {
      U_TRACE(5, "WiAuthNodog::setStatus(%d)", _status)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(*ap_address)
   // U_INTERNAL_ASSERT_RANGE(-2,status,2)
      U_INTERNAL_ASSERT_RANGE(0,_status,3)

      // NB: we may be a different process from what it has updated so that we need to read the record...

      if (db_nodog->getDataStorage(*ap_address))
         {
         U_INTERNAL_DUMP("status = %d", status)

         if (_status != status)
            {
            since = u_now->tv_sec;

                 if (_status == 0) status = _status;
            else if (_status == 3) // 3 => start
               {
               start  = since;
               status = 0;
               }
            else if (  status >   0 ||
                     --status <= -3)
               {
               status = _status;
               }

            U_LOGGER("*** WiAuthNodog::setStatus(%d) AP(%v) CHANGE STATE (%d) ***", _status, ap_address->rep, status);
            }

         last_info = u_now->tv_sec;

         (void) db_nodog->putDataStorage();
         }
      }

   UString sendRequestToNodog(const char* fmt, ...) // NB: request => http://%s:%u/...", *ap_address, 5280...
      {
      U_TRACE(5, "WiAuthNodog::sendRequestToNodog(%S)", fmt)

      U_INTERNAL_ASSERT(*ap_address)

      int _status = 0;
      UString result, buffer(U_CAPACITY);

      va_list argp;
      va_start(argp, fmt);

      buffer.vsnprintf(fmt, argp);

      va_end(argp);

      url_nodog->snprintf("http://%v:5280/%v", ap_address->rep, buffer.rep);

      // NB: we need PREFORK_CHILD > 2

      if (client->connectServer(*url_nodog) &&
          client->sendRequest())
         {
         result = client->getContent();

         client->UClient_Base::close();
         }
      else
         {
         _status = 2; // NB: nodog not respond

         if (sockp)
            {
            UIPAddress addr;

            if (client->remoteIPAddress(addr) &&
                            sockp->ping(addr))
               {
               _status = 1; // NB: nodog not respond but pingable => unreachable...
               }
            }
         }

      client->reset();

      setStatus(_status);

      U_RETURN_STRING(result);
      }

   void editRecord(bool reboot, bool bnoconsume, UString& mac_mask, UString& group_account_mask)
      {
      U_TRACE(5, "WiAuthNodog::editRecord(%b,%b,%V,%V)", reboot, bnoconsume, mac_mask.rep, group_account_mask.rep)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(*ap_address)

      // NB: we may be a different process from what it has updated so that we need to read the record...

      if (db_nodog->getDataStorage(*ap_address) &&
          findLabel())
         {
         WiAuthAccessPoint* ap_rec = nodog_rec->vec_access_point[index_access_point];

         ap_rec->noconsume          = bnoconsume;
         ap_rec->mac_mask           = mac_mask;
         ap_rec->group_account_mask = group_account_mask;

         (void) db_nodog->putDataStorage();
         }

      // NB: REBOOT access point tramite webif...

      if (reboot)
         {
         U_INTERNAL_ASSERT(UServer_Base::bssl)

         UString url(U_CAPACITY);

         url.snprintf("http://%v/cgi-bin/webif/reboot.sh", ap_address->rep);

         if (client->sendPost(url, *pbody)) setStatus(1); // unreachable
         }
      }

   bool setRecord(int _port)
      {
      U_TRACE(5, "WiAuthNodog::setRecord(%d)", _port)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(*ap_address)

      int op = -1;

      if (ap_label->empty()) (void) ap_label->assign(U_CONSTANT_TO_PARAM("ap"));

      if (db_nodog->getDataStorage(*ap_address))
         {
              if ( ap_hostname->empty()) *ap_hostname = hostname;
         else if (*ap_hostname != hostname)
            {
            U_LOGGER("*** AP HOSTNAME (%v) NOT EQUAL (%v) ON NODOG RECORD: ADDRESS(%v) ***", ap_hostname->rep, hostname.rep, ap_address->rep);

            if (UClientImage_Base::request_uri->equal(U_CONSTANT_TO_PARAM("/start_ap")))
               {
               op = RDB_REPLACE;

               hostname = *ap_hostname;
               }
            }

         if (findLabel() == false)
            {
            if (ap_address_trust == false ||
                sz > 1024)
               {
               U_RETURN(false);
               }

            op = RDB_REPLACE;

            index_access_point = sz++;

            vec_access_point.push_back(U_NEW(WiAuthAccessPoint(*ap_label)));
            }
         }
      else
         {
         if (ap_address_trust == false) U_RETURN(false);

         op        = RDB_INSERT;
         port      = _port;
         status    = 0;
         hostname  = (*ap_hostname ? *ap_hostname : U_STRING_FROM_CONSTANT("hostname_empty"));
         last_info = since = start = u_now->tv_sec;

         sz                 = 1;
         index_access_point = 0;

         vec_access_point.clear();
         vec_access_point.push_back(U_NEW(WiAuthAccessPoint(*ap_label)));
         }

      if (op != -1)
         {
         if (op == RDB_REPLACE) (void) db_nodog->putDataStorage(   *ap_address);
         else                   (void) db_nodog->insertDataStorage(*ap_address);
         }

      U_RETURN(true);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool breset) const { return ""; }
#endif
};

class WiAuthUser : public UDataStorage {
public:

   long login_time, last_modified;
   uint64_t _traffic_done, _traffic_available, _traffic_consumed;
   UString _ip, _auth_domain, _mac, _policy, nodog, _user;
   uint32_t _time_done, _time_available, _time_consumed, _index_access_point, agent, DownloadRate, UploadRate;
   bool connected, consume;

   // COSTRUTTORE

   WiAuthUser()
      {
      U_TRACE_REGISTER_OBJECT(5, WiAuthUser, "", 0)

      login_time    = last_modified = 0L;
      _traffic_done = _traffic_available = _traffic_consumed = 0ULL;
      _time_done    = _time_available  = _time_consumed = _index_access_point = agent = DownloadRate = UploadRate = 0;
      connected     = false;
      consume       = true;
      }

   ~WiAuthUser()
      {
      U_TRACE_UNREGISTER_OBJECT(5, WiAuthUser)

      db_user->close();

      delete db_user;
      }

   // define method VIRTUAL of class UDataStorage

   virtual void clear()
      {
      U_TRACE(5, "WiAuthUser::clear()")

      U_CHECK_MEMORY

               _ip.clear();
              _mac.clear();
             _user.clear();
             nodog.clear();
           _policy.clear();
      _auth_domain.clear();
      }

   virtual char* toBuffer()
      {
      U_TRACE(5, "WiAuthUser::toBuffer()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(nodog)
      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      // 172.16.1.172 0 1391085937 1391085921 0 0 878 7200 0 314572800 0 1 3291889980 0 0 MAC_AUTH_all 00:14:a5:6e:9c:cb DAILY 10.10.100.115 "anonymous"

      u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE,
                          "%v %u "
                          "%ld %ld "
                          "%u %llu "
                          "%u %u %llu %llu "
                          "%u %u %u %u %u "
                          "%v %v %v %v \"%v\"",
                          _ip.rep, connected,
                          last_modified, login_time,
                          _time_consumed, _traffic_consumed,
                          _time_done, _time_available, _traffic_done, _traffic_available,
                          _index_access_point, consume, agent, DownloadRate, UploadRate,
                          _auth_domain.rep, _mac.rep, _policy.rep, nodog.rep, _user.rep);

      U_RETURN(u_buffer);
      }

   virtual void fromStream(istream& is)
      {
      U_TRACE(5, "WiAuthUser::fromStream(%p)", &is)

      U_CHECK_MEMORY

      _ip.get(is);

      is.get(); // skip ' '

      is >> connected
         >> last_modified >> login_time
         >>    _time_consumed
         >> _traffic_consumed
         >>    _time_done >>    _time_available
         >> _traffic_done >> _traffic_available
         >> _index_access_point >> consume >> agent >> DownloadRate >> UploadRate;

      is.get(); // skip ' '

      _auth_domain.get(is);

      is.get(); // skip ' '

      _mac.get(is);

      is.get(); // skip ' '

      _policy.get(is);

      is.get(); // skip ' '

      nodog.get(is);

      is.get(); // skip ' '

      _user.get(is);
      }

   UString getLabelAP() { return nodog_rec->getLabelAP(_index_access_point); }

   UString getAP(UString& label)
      {
      U_TRACE(5, "WiAuthUser::getAP(%V)", label.rep)

      U_CHECK_MEMORY

      label = getLabelAP();

      if (label.empty()) label = U_STRING_FROM_CONSTANT("ap");

      UString x(100U);

      x.snprintf("%v@%v:%u/%v", label.rep, nodog.rep, nodog_rec->port, nodog_rec->hostname.rep);

      U_RETURN_STRING(x);
      }

   static bool isConnected(UStringRep* data)
      {
      U_TRACE(5, "WiAuthUser::isConnected(%p)", data)

      U_INTERNAL_ASSERT_POINTER(data)

      // ip           c modified    login time_consume traff_consume  time_done time_avail traff_done  traff_avail x m agent     Dr Ur domain
      // 172.16.1.172 0 1355492583    0       2572         4378722          0          0      4378722    314572800 0 1 2161242255 0 0 PASS_AUTH

      const char* ptr = data->c_pointer(7); // "1.1.1.1"

      do { ++ptr; } while (u__isspace(*ptr) == false);

      U_INTERNAL_ASSERT(u__isspace(ptr[0]))

      bool result = (ptr[1] == '1');

      U_RETURN(result);
      }

   void setConnected(bool bconnected)
      {
      U_TRACE(5, "WiAuthUser::setConnected(%b)", bconnected)

      U_CHECK_MEMORY

      if (bconnected)
         {
      // if (connected == false) addConnection();

         connected  = true;
         login_time = u_now->tv_sec;
         }
      else
         {
      // if (connected) delConnection();

         connected = false;
         }
      }

   const char* updateCounter(const UString& logout, long time_connected, uint64_t traffic, bool& ask_logout)
      {
      U_TRACE(5, "WiAuthUser::updateCounter(%V,%ld,%llu,%b)", logout.rep, time_connected, traffic, ask_logout)

      _time_done    += time_connected;
      _traffic_done += traffic;

      if (consume)
         {
         if (_time_done > _time_available)
            {
            ask_logout = true;

            /*
            long time_diff = _time_done - _time_available;

            U_LOGGER("*** updateCounter() UID(%v) IP(%v) MAC(%v) AP(%v) EXCEED TIME_AVAILABLE (%ld sec) ***", uid->rep, _ip.rep, _mac.rep, ap->rep, time_diff);
            */
            }

         if (_traffic_done > _traffic_available)
            {
            ask_logout = true;

            /*
            uint64_t traffic_diff = _traffic_done - _traffic_available;

            U_LOGGER("*** updateCounter() UID(%v) IP(%v) MAC(%v) AP(%v) connected=%b EXCEED TRAFFIC_AVAILABLE (%llu bytes) ***",
                                          uid->rep, _ip.rep, _mac.rep, ap->rep, connected, traffic_diff);
            */
            }
         }

      const char* write_to_log;

      char c = (logout ? logout.first_char() : '0');

      if (c == '0') // NB: _logout == 0 mean NOT logout (only info)...
         {
         write_to_log = 0;

         if (traffic == 0 &&
             time_connected > (30 * 60)) // 30m
            {
            ask_logout = true;

            /*
            U_LOGGER("*** updateCounter() UID(%v) IP(%v) MAC(%v) AP(%v) connected=%b EXCEED MAX_TIME_NO_TRAFFIC(%ld secs) ***",
                                          uid->rep, _ip.rep, _mac.rep, ap->rep, connected, time_connected);
            */
            }
         }
      else
         {
         write_to_log = (c == '-' || traffic == 0 ? "EXIT" : "LOGOUT"); // LOGOUT (-1... => implicito)

         U_INTERNAL_DUMP("_auth_domain = %V", _auth_domain.rep)

         if (_auth_domain == *account_auth)
            {
            /*
            U_LOGGER("*** updateCounter() UID(%v) IP(%v) MAC(%v) AP(%v) connected=%b GROUP ACCOUNT %s (%ld secs) ***",
                                          uid->rep, _ip.rep, _mac.rep, ap->rep, connected, write_to_log, time_connected);
            */
            
            // --------------------------------------------------------------------------------------------------
            // NB: we have an exit for a group account and we check if there are other user on the
            //     access point that has this group account, in this case we consider the user still connected...
            // --------------------------------------------------------------------------------------------------

            if (nodog_rec->vec_access_point[_index_access_point]->group_account_mask)
               {
               U_INTERNAL_ASSERT_EQUALS(*ap_address, nodog)

               UString result = nodog_rec->sendRequestToNodog("users", 0);

               if (result &&
                   U_IS_HTTP_ERROR(U_http_info.nResponseCode) == false)
                  {
#              ifdef USE_LIBZ
                  if (UStringExt::isGzip(result)) result = UStringExt::gunzip(result);
#              endif

                  UVector<UString> vec(result);

                  for (uint32_t i = 0, n = vec.size(); i < n; i += 5)
                     {
                     if (UServices::dosMatchWithOR(vec[i], nodog_rec->vec_access_point[_index_access_point]->group_account_mask)) goto next;
                     }
                  }
               }
            }

         long logout_time = (c == '-' ? logout.substr(1) : logout).strtol();

         if (logout_time == 1 ||
             logout_time >= last_modified)
            {
            connected = false;
            }
         /*
         else
            {
            U_LOGGER("*** updateCounter() UID(%v) IP(%v) MAC(%v) AP(%v) connected=%b LOGOUT TIME EXPIRED (%#8D < %#8D) ***",
                                          uid->rep, _ip.rep, _mac.rep, ap->rep, connected, logout_time, last_modified);
            }
         */
         }
next:
      last_modified = u_now->tv_sec;

      U_RETURN(write_to_log);
      }

   void getDone()
      {
      U_TRACE(5, "WiAuthUser::getDone()")

      U_CHECK_MEMORY

         time_done->setFromNumber32(_time_done / 60);
      traffic_done->setFromNumber64(_traffic_done / (1024ULL * 1024ULL));
      }

   void getCounter()
      {
      U_TRACE(5, "WiAuthUser::getCounter()")

      U_CHECK_MEMORY

         time_counter->setFromNumber32s(((int32_t)   _time_available - (int32_t)   _time_done) /   60);
      traffic_counter->setFromNumber64s(((int64_t)_traffic_available - (int64_t)_traffic_done) / (1024LL * 1024LL));
      }

   void getConsumed()
      {
      U_TRACE(5, "WiAuthUser::getConsume()")

      U_CHECK_MEMORY

         time_consumed->setFromNumber32((   _time_consumed +    _time_done) / 60U);
      traffic_consumed->setFromNumber64((_traffic_consumed + _traffic_done) / (1024ULL * 1024ULL));
      }

   UString getPolicy()
      {
      U_TRACE(5, "WiAuthUser::getPolicy()")

      UString x = (consume || _policy != *policy_daily ? _policy : *policy_flat);

      U_RETURN_STRING(x);
      }

   bool setNodogReference()
      {
      U_TRACE(5, "WiAuthUser::setNodogReference()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(*uid)

      if (nodog.empty())
         {
         U_LOGGER("*** UID(%v) IS NOT BINDED WITH AP ***", uid->rep);

         U_RETURN(false);
         }

      if (db_nodog->getDataStorage(nodog) == false)
         {
         U_LOGGER("*** UID(%v) BINDED WITH AP(%v) THAT IT IS NOT REGISTERED ***", uid->rep, nodog.rep);

         U_RETURN(false);
         }

      if (_index_access_point >= nodog_rec->sz)
         {
         U_LOGGER("*** UID(%v) BINDED WITH AP(%v) HAS AN INDEX(%u) THAT IT IS INVALID ***", uid->rep, nodog.rep, _index_access_point);

         U_RETURN(false);
         }

      U_RETURN(true);
      }

   static bool checkUserID()
      {
      U_TRACE(5, "WiAuthUser::checkUserID()")

      U_INTERNAL_ASSERT(*uid)
      U_ASSERT_EQUALS(*uid, UStringExt::trim(*uid))

      uint32_t sz     = uid->size();
      const char* ptr = uid->data();

      isIP  = u_isIPv4Addr(ptr, sz);
      isMAC = u_isMacAddr( ptr, sz);

      U_INTERNAL_DUMP("isIP = %b isMAC = %b uid = %V mac = %V ip = %V", isIP, isMAC, uid->rep, mac->rep, ip->rep)

      if (isMAC ||
          isIP)
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static UString get_UserName()
      {
      U_TRACE(5, "WiAuthUser::get_UserName()")

      UString user, content;

      if (checkUserID() == false)
         {
         UString pathname(U_CAPACITY);

         pathname.snprintf("%v/%v.reg", dir_reg->rep, uid->rep);

         content = UFile::contentOf(pathname);
         }

      if (content.empty()) user = U_STRING_FROM_CONSTANT("anonymous");
      else
         {
         UVector<UString> vec(content);

         if (vec.size() > 2)
            {
            user = UStringExt::trim(vec[0]);

            user.push_back(' ');

            (void) user.append(UStringExt::trim(vec[1]));
            }

         if (user.empty())
            {
            U_LOGGER("*** getUserName() USER EMPTY: UID(%v) ***", uid->rep);
            }
         }

      U_RETURN_STRING(user);
      }

   static void loadPolicy(const UString& name)
      {
      U_TRACE(5, "WiAuthUser::loadPolicy(%V)", name.rep)

      U_INTERNAL_ASSERT(name)

      const char* key_time            = 0;
      const char* key_traffic         = 0;
      const char* key_time_no_traffic = 0;

      if (table->empty() == false)
         {
         key_time    = "waTime";
         key_traffic = "waTraffic";
         }
      else
         {
         table->clear();

         UString content = policy_cache->getContent(U_STRING_TO_PARAM(name));

         if (content.empty()) content = policy_cache->getContent(U_STRING_TO_PARAM(*policy_daily));

         U_INTERNAL_ASSERT(content)

         if (UFileConfig::loadProperties(*table, content.data(), content.end()))
            {
            key_time            = "MAX_TIME";
            key_traffic         = "MAX_TRAFFIC";
            key_time_no_traffic = "MAX_TIME_NO_TRAFFIC";
            }
         }

      if (key_time)
         {
                                  *time_available      = (*table)[key_time];
                                  *traffic_available   = (*table)[key_traffic];
         if (key_time_no_traffic) *max_time_no_traffic = (*table)[key_time_no_traffic];
         }
      else
         {
         (void)      time_available->assign(U_CONSTANT_TO_PARAM("7200"));
         (void)   traffic_available->assign(U_CONSTANT_TO_PARAM("314572800"));
         (void) max_time_no_traffic->assign(U_CONSTANT_TO_PARAM("10"));
         }

      table->clear();
      }

   void setRecord() // NB: it is called only in the context of login validation...
      {
      U_TRACE(5, "WiAuthUser::setRecord()")

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("index_access_point = %u nodog_rec->sz = %u", index_access_point, nodog_rec->sz)

      U_INTERNAL_ASSERT(*uid)
      U_INTERNAL_ASSERT(*ap_address)
      U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)
      U_ASSERT_EQUALS(*uid, UStringExt::trim(*uid))
      U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)
      U_INTERNAL_ASSERT_MINOR(index_access_point, nodog_rec->sz)

      // NB: flat policy to user is assigned only manually (generally only for uid with cellular number)...

      int op;
      bool bflat = (*policy == *policy_flat);

      if (db_user->getDataStorage(*uid))
         {
         op = RDB_REPLACE;

         // NB: we check for errors...

         if (_policy.empty()) _policy = *policy_daily;
         else
            {
            if ((isMAC || isIP) &&
                (_policy == *policy_flat))
               {
               _policy = *policy_daily;
               }
            }

         if (bflat == false &&
             _policy != *policy)
            {
            if ( _policy == *policy_flat) bflat = true; // NB: if the user has flat policy we force noconsume...
            else _policy  = *policy;                    //     ...otherwise we assume the system policy...
            }
         }
      else
         {
         op    = RDB_INSERT;
         _user = get_UserName();

         _time_done        =
         _time_consumed    = 0L;
         _traffic_done     =
         _traffic_consumed = 0ULL;

         _policy = (bflat ? *policy_daily : *policy); // NB: if the system policy is flat we assume default...
         }

      nodog               = *ap_address;
      _index_access_point = index_access_point;

      U_INTERNAL_DUMP("db_user->isRecordFound() = %b _index_access_point = %u nodog = %V", db_user->isRecordFound(), _index_access_point, nodog.rep)

      _ip           = *ip;
      _mac          = *mac;
   // agent         = UHTTP::getUserAgent();
      login_time    = 0L;
      connected     = false;
      last_modified = u_now->tv_sec;

        UploadRate  = user_UploadRate->strtol();
      DownloadRate  = user_DownloadRate->strtol();

      WiAuthAccessPoint* ap_rec = nodog_rec->vec_access_point[index_access_point];

      if (ap_rec->group_account_mask &&
          UServices::dosMatchWithOR(*uid, ap_rec->group_account_mask))
         {
         consume      = false;
         _auth_domain = *account_auth;
         }
      else
         {
         // NB: if the user/system policy is flat we force noconsume...

         consume      = (bflat ? false : (ap_rec->noconsume == false));
         _auth_domain = *auth_domain;
         }

      loadPolicy(_policy); // NB: time_available e traffic_available sono valorizzati da loadPolicy()...

      uint32_t _time_available_tmp    =    time_available->strtol();
      uint64_t _traffic_available_tmp = traffic_available->strtoll();

      if (db_user->isRecordFound()                     &&
             _time_available !=    _time_available_tmp &&
          _traffic_available != _traffic_available_tmp)
         {
         U_LOGGER("*** UID(%v) ON AP(%v) HAS RESOURCE DIFFERENT FROM POLICY(%v:%v) (TIME:%u=>%u) (TRAFFIC:%llu=>%llu) - isMAC(%b) isIP(%b) ***",
                       uid->rep, nodog.rep, policy->rep, _policy.rep, _time_available, _time_available_tmp, _traffic_available, _traffic_available_tmp, isMAC, isIP);
         }

         _time_available   =  _time_available_tmp;
      _traffic_available = _traffic_available_tmp;

      if (op == RDB_REPLACE) (void) db_user->putDataStorage(   *uid);
      else                   (void) db_user->insertDataStorage(*uid);
      }

   void writeToLOG(const char* op)
      {
      U_TRACE(5, "WiAuthUser::writeToLOG(%S)", op)

      U_INTERNAL_ASSERT_POINTER(op)

      UString label,
              x = getPolicy(),
              y = getAP(label);

      getCounter();

      /* Example
      ------------------------------------------------------------------------------------------------------------------------------------------------------ 
      2012/08/08 14:56:00 op: PASS_AUTH, uid: 33437934, ap: 00@10.8.1.2, ip: 172.16.1.172, mac: 00:14:a5:6e:9c:cb, timeout: 233, traffic: 342, policy: DAILY
      ------------------------------------------------------------------------------------------------------------------------------------------------------ 
      */

      ULog::log(file_LOG->getFd(), "op: %s, uid: %v, ap: %v, ip: %v, mac: %v, timeout: %v, traffic: %v, policy: %v",
                                    op,    uid->rep, y.rep, _ip.rep, _mac.rep, time_counter->rep, traffic_counter->rep, x.rep);
      }

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool breset) const { return ""; }
#endif
};

static int countAP(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::countAP(%p,%p)", key, data)

   if (key) U_RETURN(4); // NB: call us later (after set record value from db)...

   U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

   num_ap += nodog_rec->sz;

   U_RETURN(1);
}

static int getNumAccessPointNoConsume(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::getNumAccessPointNoConsume(%p,%p)", key, data)

   if (key) U_RETURN(4); // NB: call us later (after set record value from db)...

   U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

   num_ap += nodog_rec->sz;

   for (uint32_t i = 0; i < nodog_rec->sz; ++i)
      {
      if (nodog_rec->vec_access_point[i]->noconsume) ++num_ap_noconsume;
      }

   U_RETURN(1);
}

static int countUsersConnected(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::countUsersConnected(%p,%p)", key, data)

   if (key) U_RETURN(4); // NB: call us later (after set record value from db)...

   U_INTERNAL_DUMP("user_rec->last_modified = %ld start_op = %ld user_rec->connected = %b", user_rec->last_modified, start_op, user_rec->connected)

                            ++num_users;
   if (user_rec->connected) ++num_users_connected;

   U_RETURN(1);
}

static void db_open()
{
   U_TRACE(5, "::db_open()")

   bool no_ssl = (UServer_Base::bssl == false);

   bool result1 = db_nodog->open(       32 * 1024, false, false, no_ssl); // 32k
   bool result2 =  db_user->open(10 * 1024 * 1024, false, false, no_ssl); // 10M

   num_ap              =
   num_users           =
   num_ap_noconsume    =
   num_users_connected = 0;

    db_user->callForAllEntry(countUsersConnected);
   db_nodog->callForAllEntry(getNumAccessPointNoConsume);

   U_SRV_LOG("%sdb initialization of wi-auth users WiAuthUser.cdb %s: size(%u) num_users(%u) connected(%u)",
               (result2 ? "" : "WARNING: "), (result2 ? "success" : "failed"), db_user->size(), num_users, num_users_connected);

   U_SRV_LOG("%sdb initialization of wi-auth access point WiAuthAccessPoint.cdb %s: size(%u) num_ap(%u) noconsume(%u)",
               (result1 ? "" : "WARNING: "), (result1 ? "success" : "failed"), db_nodog->size(), num_ap, num_ap_noconsume);

   /*
   if (no_ssl)
      {
      UString x = db_nodog->print();

      (void) UFile::writeToTmp(U_STRING_TO_PARAM(x), false, "WiAuthAccessPoint.init", 0);

      x = db_user->print();

      (void) UFile::writeToTmp(U_STRING_TO_PARAM(x), false, "WiAuthUser.init", 0);

      (void) UFile::_unlink("/tmp/WiAuthUser.end");
      (void) UFile::_unlink("/tmp/WiAuthAccessPoint.end");
      }
   */
}

static void setAccessPointReference(const char* s, uint32_t n)
{
   U_TRACE(5, "::setAccessPointReference(%.*S,%u)", n, s, n)

   // Ex: ap@10.8.1.2 => X0256Rap

   char* ptr;
   unsigned char c = *s;
   uint32_t count = 0, certid = 0;

   while (n--)
      {
   // U_INTERNAL_DUMP("c = %C n = %u count = %u certid = %u", c, n, count, certid)

      if (c == '.') ++count;
      else
         {
         U_INTERNAL_ASSERT(u__isdigit(c))

         if (count == 2)
            {
            certid = 254 * strtol(s, &ptr, 10);

            c = *(s = ptr);

            continue;
            }

         if (count == 3)
            {
            certid += strtol(s, 0, 10);

            break;
            }
         }

      c = *(++s);
      }

   if (ap_label->empty()) (void) ap_label->assign(U_CONSTANT_TO_PARAM("ap"));

   ap_ref->setBuffer(100U);

   ap_ref->snprintf("X%04dR%v", certid, ap_label->rep);

   U_INTERNAL_DUMP("ap_ref = %V", ap_ref->rep)
}

static void usp_init_wi_auth()
{
   U_TRACE(5, "::usp_init_wi_auth()")

   ap_ref   = U_NEW(UString(100U));
   ap_label = U_NEW(UString);

#ifndef U_ALIAS
   U_ERROR("Sorry, I can't run wi_auth(USP) because alias URI support is missing, please recompile ULib");
#endif

#ifdef DEBUG
   if ((setAccessPointReference(U_CONSTANT_TO_PARAM("10.8.1.2")),      ap_ref->equal(U_CONSTANT_TO_PARAM("X0256Rap")))  == false ||
       (setAccessPointReference(U_CONSTANT_TO_PARAM("10.10.100.115")), ap_ref->equal(U_CONSTANT_TO_PARAM("X25515Rap"))) == false)
      {
      U_ERROR("setAccessPointReference() failed");
      }
#endif

   sockp = U_NEW(UPing(5000, UClientImage_Base::bIPv6));

   if (sockp->initPing() == false)
      {
      delete sockp;
             sockp = 0;
      }

   ip                  = U_NEW(UString);
   ap                  = U_NEW(UString);
   uid                 = U_NEW(UString);
   mac                 = U_NEW(UString);
   pbody               = U_NEW(U_STRING_FROM_CONSTANT("reboot=+Yes%2C+really+reboot+now+"));
   redir               = U_NEW(UString);
   token               = U_NEW(UString);
   output              = U_NEW(UString);
   policy              = U_NEW(UString);
   gateway             = U_NEW(UString);
   ip_auth             = U_NEW(U_STRING_FROM_CONSTANT("IP_AUTH"));
   redirect            = U_NEW(UString);
   mac_auth            = U_NEW(U_STRING_FROM_CONSTANT("MAC_AUTH"));
   ip_server           = U_NEW(UString(UServer_Base::getIPAddress()));
   empty_str           = U_NEW(U_STRING_FROM_CONSTANT("\"\""));
   url_nodog           = U_NEW(UString(U_CAPACITY));
   cert_auth           = U_NEW(U_STRING_FROM_CONSTANT("CERT_AUTH"));
   ap_ref_ap           = U_NEW(UString(100U));
   nodog_conf          = U_NEW(UString(UFile::contentOf("ap/nodog.conf.template")));
   logout_url          = U_NEW(UString(200U));
   ap_address          = U_NEW(UString);
   empty_list          = U_NEW(U_STRING_FROM_CONSTANT("()"));
   auth_domain         = U_NEW(UString);
   ap_hostname         = U_NEW(UString);
   cookie_auth         = U_NEW(U_STRING_FROM_CONSTANT("COOKIE_AUTH_"));
   account_auth        = U_NEW(U_STRING_FROM_CONSTANT("ACCOUNT_AUTH"));
   ap_ref_comune       = U_NEW(UString(100U));
   user_UploadRate     = U_NEW(UString(10U));
   user_DownloadRate   = U_NEW(UString(10U));
   allowed_web_hosts   = U_NEW(UString);
   dir_server_address  = U_NEW(UString(200U));
   max_time_no_traffic = U_NEW(UString);

   policy_flat         = U_NEW(U_STRING_FROM_CONSTANT("FLAT"));
   policy_daily        = U_NEW(U_STRING_FROM_CONSTANT("DAILY"));
   policy_traffic      = U_NEW(U_STRING_FROM_CONSTANT("TRAFFIC"));

   time_done           = U_NEW(UString(20U));
   time_counter        = U_NEW(UString(20U));
   time_consumed       = U_NEW(UString(20U));
   time_available      = U_NEW(UString);
   traffic_done        = U_NEW(UString(20U));
   traffic_counter     = U_NEW(UString(20U));
   traffic_consumed    = U_NEW(UString(20U));
   traffic_available   = U_NEW(UString);

   U_INTERNAL_ASSERT_POINTER(USSIPlugIn::environment)

   environment  = U_NEW(UString(*USSIPlugIn::environment));
   virtual_name = U_NEW(UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("VIRTUAL_NAME"), environment)));

   (void) environment->append(U_CONSTANT_TO_PARAM("VIRTUAL_HOST="));
   (void) environment->append(*virtual_name);

   UString dir_root;

   dir_reg            = U_NEW(UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("DIR_REG"),            environment)));
   dir_root           =               UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("DIR_ROOT"),           environment);
   title_default      = U_NEW(UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("TITLE_DEFAULT"),      environment)));
   historical_log_dir = U_NEW(UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("HISTORICAL_LOG_DIR"), environment)));

   dir_server_address->snprintf("%v/client", dir_root.rep);

   UString tmp1 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("LDAP_CARD_PARAM"),    environment),
           tmp2 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("WIAUTH_CARD_BASEDN"), environment);

     ldap_card_param  = U_NEW(UString(UStringExt::expandEnvironmentVar(tmp1, environment)));
   wiauth_card_basedn = U_NEW(UString(UStringExt::expandEnvironmentVar(tmp2, environment)));

   tmp1 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("LDAP_USER_PARAM"),    environment),
   tmp2 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("WIAUTH_USER_BASEDN"), environment);

     ldap_user_param  = U_NEW(UString(UStringExt::expandEnvironmentVar(tmp1, environment)));
   wiauth_user_basedn = U_NEW(UString(UStringExt::expandEnvironmentVar(tmp2, environment)));

   tmp1 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("LDAP_SESSION_PARAM"),    environment),
   tmp2 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("WIAUTH_SESSION_BASEDN"), environment);

     ldap_session_param  = U_NEW(UString(UStringExt::expandEnvironmentVar(tmp1, environment)));
   wiauth_session_basedn = U_NEW(UString(UStringExt::expandEnvironmentVar(tmp2, environment)));

   UString content = UFile::contentOf("$DIR_ROOT/etc/AllowedWebHosts.txt", O_RDONLY, false, environment);

   if (content)
      {
      UVector<UString> vec(content);

      if (vec.empty() == false) *allowed_web_hosts = vec.join(" ") + ' ';
      }

          cache = U_NEW(UCache);
    admin_cache = U_NEW(UCache);
   policy_cache = U_NEW(UCache);

   UString x(U_CAPACITY);

   x.snprintf("$DIR_ROOT/etc/%v/cache.tmpl", virtual_name->rep);

   (void) cache->open(x, U_STRING_FROM_CONSTANT("$DIR_TEMPLATE"), environment, true);

   x.snprintf("$DIR_ROOT/etc/%v/policy_cache.tmpl", virtual_name->rep);

   (void) policy_cache->open(x, U_STRING_FROM_CONSTANT("$DIR_POLICY"), environment, true);

   x.snprintf("$DIR_ROOT/etc/%v/admin_cache.tmpl", ip_server->rep);

   (void) admin_cache->open(x, U_STRING_FROM_CONSTANT("$DIR_ADMIN_TEMPLATE"), environment, true);

   message_page_template = U_NEW(UString(cache->getContent(U_CONSTANT_TO_PARAM("message_page.tmpl"))));

   login_nodog_template     = U_NEW(UString(admin_cache->getContent(U_CONSTANT_TO_PARAM("login_nodog_body.tmpl"))));
   status_network_template  = U_NEW(UString(admin_cache->getContent(U_CONSTANT_TO_PARAM("status_network_body.tmpl"))));
   status_nodog_template    = U_NEW(UString(admin_cache->getContent(U_CONSTANT_TO_PARAM("status_nodog_body.tmpl"))));
   status_nodog_ap_template = U_NEW(UString(admin_cache->getContent(U_CONSTANT_TO_PARAM("status_nodog_body_ap.tmpl"))));

   x.snprintf("$DIR_ROOT/etc/%v/script.conf", virtual_name->rep);

   content = UFile::contentOf(x.data(), O_RDONLY, false, environment);

   table = U_NEW(UHashMap<UString>);

   if (UFileConfig::loadProperties(*table, content.data(), content.end()))
      {
      telefono          = U_NEW(UString((*table)["TELEFONO"]));
      fmt_auth_cmd      = U_NEW(UString((*table)["FMT_AUTH_CMD"]));
      redirect_default  = U_NEW(UString((*table)["REDIRECT_DEFAULT"]));

               help_url = U_NEW(UString(UStringExt::expandEnvironmentVar((*table)["HELP_URL"],          environment)));
              login_url = U_NEW(UString(UStringExt::expandEnvironmentVar((*table)["LOGIN_URL"],         environment)));
             wallet_url = U_NEW(UString(UStringExt::expandEnvironmentVar((*table)["WALLET_URL"],        environment)));
           password_url = U_NEW(UString(UStringExt::expandEnvironmentVar((*table)["PASSWORD_URL"],      environment)));
      registrazione_url = U_NEW(UString(UStringExt::expandEnvironmentVar((*table)["REGISTRAZIONE_URL"], environment)));

      logout_url->snprintf("%v/logout", login_url->rep);

      url_banner_ap = U_NEW(UString(UStringExt::expandPath((*table)["URL_BANNER_AP"], environment)));

      x.snprintf("$DIR_WEB/%v%v", virtual_name->rep, url_banner_ap->rep);

      url_banner_ap_path = U_NEW(UString(UStringExt::expandPath(x, environment)));

      x.snprintf("%v/default", url_banner_ap_path->rep);

      url_banner_ap_default = U_NEW(UFile(x.copy(), environment));

      url_banner_comune = U_NEW(UString(UStringExt::expandPath((*table)["URL_BANNER_COMUNE"], environment)));

      x.snprintf("$DIR_WEB/%v%v", virtual_name->rep, url_banner_comune->rep);

      url_banner_comune_path = U_NEW(UString(UStringExt::expandPath(x, environment)));

      x.snprintf("%v/default", url_banner_comune_path->rep);

      url_banner_comune_default = U_NEW(UFile(x, environment));

      if (url_banner_ap_default->stat() == false)
         {
         delete url_banner_ap_default;
                url_banner_ap_default = 0;
         }

      if (url_banner_comune_default->stat() == false)
         {
         delete url_banner_comune_default;
                url_banner_comune_default = 0;
         }

#  ifdef USE_LIBSSL
      des3_key = U_NEW(UString((*table)["DES3_KEY"]));

      UDES3::setPassword(des3_key->c_str());
#  endif
      }

   table->clear();

   client = U_NEW(UHttpClient<UTCPSocket>(0));

   client->setFollowRedirects(true, false);

   // NB: REBOOT access point tramite webif...

#  ifdef USE_LIBSSL
      if (UServer_Base::bssl)
         {
         U_ASSERT_EQUALS(client->isPasswordAuthentication(), false)

         if (UServer_Base::cfg->searchForObjectStream(U_CONSTANT_TO_PARAM("proxy")))
            {
            UServer_Base::cfg->table.clear();

            if (UModProxyService::loadConfig(*UServer_Base::cfg))
               {
               UServer_Base::cfg->reset();

               U_http_method_type = HTTP_GET;

               UModProxyService* service = UModProxyService::findService(U_STRING_TO_PARAM(*virtual_name),
                                                                         U_CONSTANT_TO_PARAM("/cgi-bin/webif/status.sh"));

               if (service) client->setRequestPasswordAuthentication(service->getUser(), service->getPassword());
               }
            }
         }
#  endif

   file_LOG = U_NEW(UFile(U_STRING_FROM_CONSTANT("$FILE_LOG"), environment));

   UString dir = UStringExt::dirname(file_LOG->getPath());

   file_WARNING  = U_NEW(UFile(dir + U_STRING_FROM_CONSTANT("/wifi-warning")));
   file_RECOVERY = U_NEW(UFile(dir + U_STRING_FROM_CONSTANT("/wifi-recovery")));
   file_UTILIZZO = U_NEW(UFile(dir + U_STRING_FROM_CONSTANT("/wifi-utilizzo")));

   UServer_Base::update_date  =
   UServer_Base::update_date1 = true;

   (void) UServer_Base::addLog(file_LOG);
   (void) UServer_Base::addLog(file_WARNING);
   (void) UServer_Base::addLog(file_RECOVERY, O_APPEND | O_RDWR);
   (void) UServer_Base::addLog(file_UTILIZZO, O_APPEND | O_RDWR);

   vuid              = U_NEW(UVector<UString>);
   vallow_IP_user    = U_NEW(UVector<UIPAllow*>);
   vallow_IP_request = U_NEW(UVector<UIPAllow*>);

   (void) x.assign(U_CONSTANT_TO_PARAM("172.0.0.0/8, " IP_UNIFI ", " IP_UNIFI_TMP ", " IP_CASCINE)); // NB: unifi and cascine has MasqueradeDevice...

   (void) UIPAllow::parseMask(x, *vallow_IP_request);

   UHTTP::set_cookie_option->snprintf("; path=/login_request; domain=%v; secure", virtual_name->rep);

   // RECORD - DB

   data_rec = U_NEW(WiAuthDataStorage);

   if (UHTTP::db_session == 0) UHTTP::initSession();

   UHTTP::db_session->setPointerToDataStorage(data_rec);

   if (UHTTP::db_session->getDataStorage() == false)
      {
      data_rec->reset();

      (void) UHTTP::db_session->putDataStorage();

      U_LOGGER("*** DATA STORAGE EMPTY ***");
      }

    db_user = U_NEW(URDBObjectHandler<UDataStorage*>(U_STRING_FROM_CONSTANT("../db/WiAuthUser.cdb"),        -1, ( user_rec = U_NEW(WiAuthUser))));
   db_nodog = U_NEW(URDBObjectHandler<UDataStorage*>(U_STRING_FROM_CONSTANT("../db/WiAuthAccessPoint.cdb"), -1, (nodog_rec = U_NEW(WiAuthNodog))));

    db_user->setShared(U_LOCK_USER1, U_SPINLOCK_USER1);
   db_nodog->setShared(           0,                0); // POSIX shared memory object (interprocess - can be used by unrelated processes)

   (void) UFile::mkfifo(NAMED_PIPE, PERM_FILE);

   db_open();
}

static void usp_sighup_wi_auth()
{
   U_TRACE(5, "::usp_sighup_wi_auth()")

   if (db_nodog)
      {
      int fd;

       db_user->close();
      db_nodog->close();

      if (UServer_Base::bssl)
         {
         fd = UFile::open(NAMED_PIPE, O_WRONLY, PERM_FILE);

         if (fd != -1)
            {
         // (void) UFile::write(fd, U_CONSTANT_TO_PARAM("ready"));

            UFile::close(fd);
            }
         }
      else
         {
      // char buffer[1024];

         fd = UFile::open(NAMED_PIPE, O_RDONLY, PERM_FILE);

         if (fd != -1)
            {
         // (void) UFile::read(fd, buffer, sizeof(buffer));

            UFile::close(fd);
            }
         }

      db_open();
      }
}

static void usp_end_wi_auth()
{
   U_TRACE(5, "::usp_end_wi_auth()")

   if (db_nodog)
      {
#  ifdef DEBUG
      delete ip;
      delete ap;
      delete uid;
      delete mac;
      delete pbody;
      delete redir;
      delete token;
      delete policy;
      delete output;
      delete ap_ref;
      delete dir_reg;
      delete gateway;
      delete ip_auth;
      delete ap_label;
      delete redirect;
      delete mac_auth;
      delete ip_server;
      delete ap_ref_ap;
      delete empty_str;
      delete url_nodog;
      delete cert_auth;
      delete ap_address;
      delete nodog_conf;
      delete empty_list;
      delete cookie_auth;
      delete ap_hostname;
      delete auth_domain;
      delete environment;
      delete policy_flat;
      delete policy_daily;
      delete account_auth;
      delete virtual_name;
      delete ap_ref_comune;
      delete title_default;
      delete policy_traffic;
      delete ldap_user_param;
      delete ldap_card_param;
      delete allowed_web_hosts;
      delete ldap_session_param;
      delete dir_server_address;
      delete wiauth_card_basedn;
      delete wiauth_user_basedn;
      delete historical_log_dir;
      delete max_time_no_traffic;
      delete login_nodog_template;
      delete wiauth_session_basedn;
      delete message_page_template;
      delete status_nodog_template;
      delete status_network_template;
      delete status_nodog_ap_template;

      delete time_done;
      delete time_counter;
      delete time_consumed;
      delete time_available;
      delete traffic_done;
      delete traffic_counter;
      delete traffic_consumed;
      delete traffic_available;

      delete user_UploadRate;
      delete user_DownloadRate;

      if (sockp) delete sockp;

      if (help_url)
         {
         delete des3_key;
         delete telefono;
         delete help_url;
         delete login_url;
         delete logout_url;
         delete wallet_url;
         delete password_url;
         delete fmt_auth_cmd;
         delete url_banner_ap;
         delete redirect_default;
         delete url_banner_comune;
         delete registrazione_url;
         delete url_banner_ap_path;
         delete url_banner_comune_path;

         if (url_banner_ap_default)     delete url_banner_ap_default;
         if (url_banner_comune_default) delete url_banner_comune_default;
         }

      delete vuid;
      delete table;
      delete cache;
      delete client;
      delete vallow_IP_user;
      delete vallow_IP_request;
      delete admin_cache;
      delete policy_cache;
#  endif

      delete data_rec;
      delete user_rec;
      delete nodog_rec;
      }

   (void) UFile::_unlink(NAMED_PIPE);
}

static int checkIfUserExist(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::checkIfUserExist(%p,%p)", key, data)

   if (key)
      {
      const char* ptr = data->data();

      if (u__isspace(ptr[UServer_Base::client_address_len]) &&
              memcmp(ptr, U_CLIENT_ADDRESS_TO_PARAM) == 0)
         {
         U_RETURN(4); // NB: call us later (after set record value from db)...
         }

      U_RETURN(1);
      }

   U_ASSERT(user_rec->_ip.equal(U_CLIENT_ADDRESS_TO_PARAM))

   user_exist = true;

   // NB: db can have different users with the same ip...

   if (user_rec->connected) U_RETURN(0); // stop

   U_RETURN(1);
}

static bool checkIfUserConnected()
{
   U_TRACE(5, "::checkIfUserConnected()")

   U_INTERNAL_DUMP("uid = %V", uid->rep)

   user_exist = false;

   if (uid->empty()) db_user->callForAllEntry(checkIfUserExist);
   else
      {
      if (uid->findWhiteSpace() != U_NOT_FOUND) *uid = UStringExt::trim(*uid);

      user_exist = db_user->getDataStorage(*uid);
      }

   U_INTERNAL_DUMP("user_exist = %b", user_exist)

   if (user_exist)
      {
      U_INTERNAL_ASSERT(u__isdigit(db_user->recval.c_char(0)))

      // NB: db can have different users for the same ip...

      if (user_rec->connected)
         {
         *uid = db_user->getKeyID();

         U_INTERNAL_ASSERT(*uid)
         U_INTERNAL_ASSERT(user_rec->nodog)

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

static bool checkIfUserConnectedOnAP(const UString& label)
{
   U_TRACE(5, "::checkIfUserConnectedOnAP(%V)", label.rep)

   U_INTERNAL_ASSERT(user_exist)
   U_INTERNAL_ASSERT(user_rec->connected)

   if (label           == *ap_label &&
       user_rec->nodog == *ap_address)
      {
      U_RETURN(true);
      }

   U_RETURN(false);
}

static UString getUserName()
{
   U_TRACE(5, "::getUserName()")

   UString user, x = WiAuthUser::get_UserName();

   if (user_exist &&
       user_rec->_user != x)
      {
      U_LOGGER("*** getUserName() USER(%v=>%v) WITH DIFFERENT VALUE ***", user_rec->_user.rep, x.rep);

      user_rec->_user = x;
      }

   user = x;

   U_RETURN_STRING(user);
}

static int quitUserConnected(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::quitUserConnected(%p,%p)", key, data)

   if (key)
      {
      if (WiAuthUser::isConnected(data)) U_RETURN(4); // NB: call us later (after set record value from db)...

      U_RETURN(1);
      }

   U_INTERNAL_ASSERT(user_rec->connected)

   if (user_rec->nodog == *ap_address)
      {
      *uid = db_user->getKeyID();

      user_rec->setConnected(false);

      U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

      (void) db_user->putDataStorage(*uid);

      user_rec->writeToLOG("QUIT");
      }

   U_RETURN(1);
}

static int checkIfUserHasPolicyDaily(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::checkIfUserHasPolicyDaily(%p,%p)", key, data)

   if (key) U_RETURN(4); // NB: call us later (after set record value from db)...

   if (user_rec->_policy == *policy_daily)
      {
      *uid = db_user->getKeyID();

      if (user_rec->connected)
         {
         uint64_t traffic = user_rec->_traffic_done / (1024ULL * 1024ULL);

         if (traffic)
            {
            U_LOGGER("*** checkIfUserHasPolicyDaily() UID(%v) IP(%v) MAC(%v) AP(%v) traffic: %llu ***", uid->rep, user_rec->_ip.rep, user_rec->_mac.rep, user_rec->nodog.rep, traffic);
            }
         }

      user_rec->_time_consumed    += user_rec->_time_done;
                                     user_rec->_time_done = 0;
      user_rec->_traffic_consumed += user_rec->_traffic_done;
                                     user_rec->_traffic_done = 0;

      user_rec->_time_available   =     time_available->strtol();
      user_rec->_traffic_available = traffic_available->strtoll();

      U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

      (void) db_user->putDataStorage(*uid);
      }

   U_RETURN(1);
}

static int resetCounterAP(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::resetCounterAP(%p,%p)", key, data)

   if (key) U_RETURN(4); // NB: call us later (after set record value from db)...

   U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

   for (uint32_t i = 0 ; i < nodog_rec->sz; ++i)
      {
      nodog_rec->vec_access_point[i]->reset();
      }

   (void) db_nodog->putDataStorage();

   U_RETURN(1);
}

static int getStatusUser(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::getStatusUser(%p,%p)", key, data)

   if (key)
      {
      if (WiAuthUser::isConnected(data)) U_RETURN(4); // NB: call us later (after set record value from db)...

      U_RETURN(1);
      }

   U_INTERNAL_DUMP("user_rec->last_modified = %ld start_op = %ld user_rec->connected = %b", user_rec->last_modified, start_op, user_rec->connected)

   U_INTERNAL_ASSERT(user_rec->connected)

   *uid = db_user->getKeyID();

   if (user_rec->setNodogReference())
      {
      ++num_users_connected;

      user_rec->getDone();

      if (user_rec->consume)
         {
         ptr1 = "green";
         ptr2 = "yes";
         }
      else
         {
         ptr1 = "orange";
         ptr2 = "no";
         }

      UString label,
              riga(U_CAPACITY),
              user = getUserName(),
              x = user_rec->getPolicy(),
              y = user_rec->getAP(label);

      riga.snprintf(status_network_template->data(),
                    user.rep,
                    uid->rep,
                    user_rec->_auth_domain.rep,
                    user_rec->_ip.rep,
                    user_rec->_mac.rep,
                    user_rec->login_time + u_now_adjust,
                    x.rep,
                    ptr1, ptr2,
                    time_done->rep, traffic_done->rep,
                    virtual_name->rep,
                    label.rep, nodog_rec->hostname.rep, user_rec->nodog.rep, nodog_rec->port, y.rep);

      (void) output->append(riga);
      }

   U_RETURN(1);
}

static int setStatusNodog(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::setStatusNodog(%p,%p)", key, data)

   if (key) U_RETURN(4); // NB: call us later (after set record value from db)...

   U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

   num_ap += nodog_rec->sz;

   if (nodog_rec->status <= 0)
      {
      ptr1 = "green";
      ptr2 = "yes";

      num_ap_up += nodog_rec->sz;
      }
   else
      {
      ptr1 = (nodog_rec->status == 2 ? (num_ap_down        += nodog_rec->sz, "red")
                                     : (num_ap_unreachable += nodog_rec->sz, "orange"));
      ptr2 = "NO";
      }

   WiAuthAccessPoint* ap_rec = nodog_rec->vec_access_point[0];

   if (ap_rec->noconsume)
      {
      ptr3 = "orange";
      ptr4 = "no";

      ++num_ap_noconsume;

      if (ap_rec->mac_mask.equal(U_CONSTANT_TO_PARAM("*"))) ++num_ap_open;
      }
   else
      {
      ptr3 = "green";
      ptr4 = "yes";
      }

   UString riga(U_CAPACITY);

   long uptime = (nodog_rec->status <= 0 ? u_now->tv_sec - nodog_rec->start : 0);

   *ap_address = db_nodog->getKeyID();

   riga.snprintf(status_nodog_template->data(),
                 ap_rec->label.rep,
                 nodog_rec->sz, virtual_name->rep, ap_rec->label.rep, nodog_rec->hostname.rep, ap_address->rep, nodog_rec->port, ap_address->rep,
                 nodog_rec->sz, nodog_rec->hostname.rep,
                 nodog_rec->sz, uptime,
                 nodog_rec->sz, ptr1, ptr2,
                 nodog_rec->sz, u_now->tv_sec - nodog_rec->since,
                 nodog_rec->sz, nodog_rec->last_info + u_now_adjust,
                 ptr3, ptr4,
                 ap_rec->mac_mask.rep,
                 ap_rec->group_account_mask.rep,
                 virtual_name->rep, ap_rec->label.rep, nodog_rec->hostname.rep, ap_address->rep, nodog_rec->port);

   (void) output->append(riga);

   for (uint32_t i = 1; i < nodog_rec->sz; ++i)
      {
      ap_rec = nodog_rec->vec_access_point[i];

      if (ap_rec->noconsume)
         {
         ptr3 = "orange";
         ptr4 = "no";

         ++num_ap_noconsume;

         if (ap_rec->mac_mask.equal(U_CONSTANT_TO_PARAM("*"))) ++num_ap_open;
         }
      else
         {
         ptr3 = "green";
         ptr4 = "yes";
         }

      riga.snprintf(status_nodog_ap_template->data(),
                    ap_rec->label.rep,
                    ptr3, ptr4,
                    ap_rec->mac_mask.rep,
                    ap_rec->group_account_mask.rep,
                    virtual_name->rep, ap_rec->label.rep, nodog_rec->hostname.rep, ap_address->rep, nodog_rec->port);

      (void) output->append(riga);
      }

   U_RETURN(1);
}

static int login_nodog_compare(const void* p, const void* q)
{
   U_TRACE(5, "::login_nodog_compare(%p,%p)", p, q)

   uint32_t i, _totale1 = 0, _totale2 = 0;

   (void) ap_address->assign(U_STRING_TO_PARAM(**(UStringRep**)p));

   (void) db_nodog->getDataStorage(*ap_address);

   U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

   for (i = 0; i < nodog_rec->sz; ++i)
      {
      _totale1 += nodog_rec->vec_access_point[i]->num_login;
      }

   (void) ap_address->assign(U_STRING_TO_PARAM(**(UStringRep**)q));

   (void) db_nodog->getDataStorage(*ap_address);

   U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

   for (i = 0; i < nodog_rec->sz; ++i)
      {
      _totale2 += nodog_rec->vec_access_point[i]->num_login;
      }

   int result = (_totale1 > _totale2 ? -1 :
                 _totale1 < _totale2 ?  1 : 0);

   U_RETURN(result);
}

static int setLoginNodogTotal(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::setLoginNodogTotal(%p,%p)", key, data)

   if (key) U_RETURN(4); // NB: call us later (after set record value from db)...

   WiAuthAccessPoint* ap_rec;

   U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

   for (uint32_t i = 0; i < nodog_rec->sz; ++i)
      {
      ap_rec = nodog_rec->vec_access_point[i];

      totale1 += ap_rec->num_login;
      totale2 += ap_rec->num_auth_domain_ALL;
      totale3 += ap_rec->num_auth_domain_FICARD;
      totale4 += ap_rec->traffic_done;
      }

   U_RETURN(1);
}

static int setLoginNodog(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::setLoginNodog(%p,%p)", key, data)

   if (key) U_RETURN(4); // NB: call us later (after set record value from db)...

   double rate1, rate2;
   UString riga(U_CAPACITY);
   WiAuthAccessPoint* ap_rec;

   num_ap += nodog_rec->sz;

   *ap_address = db_nodog->getKeyID();

   for (uint32_t i = 0; i < nodog_rec->sz; ++i)
      {
      ap_rec = nodog_rec->vec_access_point[i];

      rate1 = (ap_rec->num_login    ? (double)ap_rec->num_login    * (double)100. / (double)totale1 : .0);
      rate2 = (ap_rec->traffic_done ? (double)ap_rec->traffic_done * (double)100. / (double)totale4 : .0);

      riga.snprintf(login_nodog_template->data(),
                    ap_rec->label.rep, ap_address->rep, nodog_rec->port, nodog_rec->hostname.rep,
                    ap_rec->num_login, rate1, 
                    ap_rec->num_auth_domain_ALL,
                    ap_rec->num_auth_domain_FICARD,
                    (uint32_t)(ap_rec->traffic_done / (1024ULL * 1024ULL)), rate2);

      (void) output->append(riga);
      }

   U_RETURN(1);
}

static void get_ap_uptime()
{
   U_TRACE(5, "::get_ap_uptime()")

   // NB: request => http://%s:%u/...", *ap_address, nodog_rec->port...

   UString result = nodog_rec->sendRequestToNodog("uptime", 0);

   if (result &&
       U_IS_HTTP_ERROR(U_http_info.nResponseCode) == false)
      {
      // NB: we may be a different process from what it has updated so that we need to read the record...

      if (db_nodog->getDataStorage(*ap_address))
         {
         U_INTERNAL_DUMP("nodog_rec->start = %ld result = %V", nodog_rec->start, result.rep)

         nodog_rec->start = u_now->tv_sec - result.copy().strtol();

         (void) db_nodog->putDataStorage();

         U_INTERNAL_DUMP("uptime = %#2D", u_now->tv_sec - nodog_rec->start)
         }
      }
}

static int checkAccessPoint(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::checkAccessPoint(%p,%p)", key, data)

   if (key)
      {
      if (key == (void*)-1)
         {
         *ap_address = db_nodog->getKeyID();

         // NB: request => http://%s:5280/...", *ap_address...

         (void) nodog_rec->sendRequestToNodog("check", 0);

         U_RETURN(1);
         }

      U_RETURN(4); // NB: call us later (after set record value from db)...
      }

   if ((u_now->tv_sec - nodog_rec->last_info) > (16L * 60L)) U_RETURN(3); // NB: call us later (without lock on db)...

   U_RETURN(1);
}

static int checkStatusUser(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::checkStatusUser(%p,%p)", key, data)

   if (key)
      {
      if (key == (void*)-1)
         {
         ++num_users_delete;

         U_RETURN(2); // delete
         }

      U_RETURN(4); // NB: call us later (after set record value from db)...
      }

   U_INTERNAL_DUMP("user_rec->last_modified = %ld start_op = %ld user_rec->connected = %b", user_rec->last_modified, start_op, user_rec->connected)

   if (user_rec->last_modified <= (start_op - (4 * 60 * 60)))
      {
      *uid = db_user->getKeyID();

      if (user_rec->connected)
         {
         U_LOGGER("*** USER TO QUIT: UID(%v) IP(%v) MAC(%v) AP(%v) POLICY(%v) ***", uid->rep, user_rec->_ip.rep, user_rec->_mac.rep, user_rec->nodog.rep, user_rec->_policy.rep);

         user_rec->setConnected(false);

         U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

         (void) db_user->putDataStorage(*uid);

         if (user_rec->setNodogReference()) user_rec->writeToLOG("RESYNC");
         }
      else
         {
         if (user_rec->_policy != *policy_flat &&
             user_rec->last_modified <= (start_op - (30 * 24 * 60 * 60))) // 1 month
            {
            U_LOGGER("*** USER TO DELETE: UID(%v) IP(%v) MAC(%v) AP(%v) POLICY(%v) ***", uid->rep, user_rec->_ip.rep, user_rec->_mac.rep, user_rec->nodog.rep, user_rec->_policy.rep);

            U_RETURN(3); // NB: call us later (without lock on db)...
            }
         }
      }

   U_RETURN(1);
}

static int checkStatusNodog(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::checkStatusNodog(%p,%p)", key, data)

   if (key)
      {
      if (key == (void*)-1)
         {
         ++num_ap_delete;

         U_RETURN(2); // delete
         }

      U_RETURN(4); // NB: call us later (after set record value from db)...
      }

   U_INTERNAL_DUMP("nodog_rec->sz = %u start_op = %ld nodog_rec->since = %ld nodog_rec->hostname = %V", nodog_rec->sz, start_op, nodog_rec->since, nodog_rec->hostname.rep)

   U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

   if (nodog_rec->status == 2 &&
       nodog_rec->since <= (start_op - (3 * 30 * 24 * 60 * 60))) // 3 month
      {
      U_LOGGER("*** AP TO REMOVE: AP(%v) ***", nodog_rec->hostname.rep);

      U_RETURN(3); // NB: call us later (without lock on db)...
      }

   U_RETURN(1);
}

static int checkStatusUserOnNodog(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::checkStatusUserOnNodog(%p,%p)", key, data)

   if (key) U_RETURN(4); // NB: call us later (after set record value from db)...

   if (user_rec->nodog != *ap_address)
      {
      if (user_rec->connected) ++num_users_connected;
      }
   else
      {
      *uid = db_user->getKeyID();

      if (pvallow &&
          UIPAllow::isAllowed(user_rec->_ip.data(), *pvallow))
         {
         ++num_users_connected;

         if (user_rec->connected == false &&
             vuid->find(*uid) != U_NOT_FOUND)
            {
            U_LOGGER("*** USER NOT ALIGNED: UID(%v) IP(%v) MAC(%v) AP(%v) POLICY(%v) ***", uid->rep, user_rec->_ip.rep, user_rec->_mac.rep, user_rec->nodog.rep, user_rec->_policy.rep);

            if (user_rec->last_modified < start_op)
               {
               user_rec->setConnected(true);

               U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

               (void) db_user->putDataStorage(*uid);

               user_rec->writeToLOG("RESYNC");
               }
            }
         }
      else if (user_rec->connected)
         {
         U_LOGGER("*** USER TO QUIT: UID(%v) IP(%v) MAC(%v) AP(%v) POLICY(%v) ***", uid->rep, user_rec->_ip.rep, user_rec->_mac.rep, user_rec->nodog.rep, user_rec->_policy.rep);

         if (user_rec->last_modified < start_op)
            {
            user_rec->setConnected(false);

            U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

            (void) db_user->putDataStorage(*uid);

            user_rec->writeToLOG("QUIT");
            }
         }
      }

   U_RETURN(1);
}

static int getAccessPointUP(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::getAccessPointUP(%p,%p)", key, data)

   if (key)
      {
      if (key == (void*)-1)
         {
         *ap_address = db_nodog->getKeyID();

         if ((vPF)data != (vPF)checkStatusUserOnNodog) ((vPF)data)();
         else
            {
            U_INTERNAL_ASSERT_EQUALS(pvallow, 0)

            checkStatusUserOnNodog(0, 0);
            }

         U_RETURN(1);
         }

      U_RETURN(4); // NB: call us later (after set record value from db)...
      }

   U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

   if (nodog_rec->status <= 0) U_RETURN(3); // NB: call us later (without lock on db)...

   U_RETURN(1);
}

static int getNameAccessPoint(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "::getNameAccessPoint(%p,%p)", key, data)

   if (key) U_RETURN(4); // NB: call us later (after set record value from db)...

   UString riga(U_CAPACITY);

   *ap_address = db_nodog->getKeyID();

   riga.snprintf("%v %v\n", nodog_rec->hostname.rep, ap_address->rep);

   (void) output->append(riga);

   U_RETURN(1);
}

static void setCookie(const char* hexdump)
{
   U_TRACE(5, "::setCookie(%p)", hexdump)

   U_INTERNAL_ASSERT(UServer_Base::bssl)

   UString cookie(200U);

   long expire = u_now->tv_sec + (hexdump ? 30L : -1L) * U_ONE_DAY_IN_SECOND;

   cookie.snprintf("WCID=%s; expires=%#8D", (hexdump ? hexdump : ""), expire);

   UHTTP::addSetCookie(cookie);
}

static void loginWithProblem()
{
   U_TRACE(5, "::loginWithProblem()")

   if (UServer_Base::bssl) setCookie(0);

   if (*uid)
      {
      if (uid->isPrintable() == false) (void) uid->assign(U_CONSTANT_TO_PARAM("not printable"));

      U_LOGGER("*** FAILURE: UID(%v) IP(%v) MAC(%v) AP(%v) POLICY(%v) ***", uid->rep, ip->rep, mac->rep, ap->rep, policy->rep);
      }

   USSIPlugIn::setMessagePageWithVar(*message_page_template, "Login",
                                     "Problema in fase di autenticazione. "
                                     "Si prega di riprovare, se il problema persiste contattare: %v", telefono->rep);
}

static int askToLDAP(UString* pinput, const char* title_txt, const char* message, const char* fmt, ...)
{
   U_TRACE(5, "::askToLDAP(%p,%S,%S,%S)",  pinput, title_txt, message, fmt)

   /*
   ldapsearch -LLL -b ou=cards,o=unwired-portal -x -D cn=admin,o=unwired-portal -w programmer -H ldap://127.0.0.1 waLogin=3386453924
   ---------------------------------------------------------------------------------------------------------------------------------
   dn: waCid=6bc07bf3-a09f-4815-8029-db68f32f4189,ou=cards,o=unwired-portal
   objectClass: top
   objectClass: waCard
   waCid: 6bc07bf3-a09f-4815-8029-db68f32f4189
   waPin: 3386453924
   waCardId: db68f32f4189
   waLogin: 3386453924
   waPassword: {MD5}ciwjVccK0u68vqupEXFukQ==
   waRevoked: FALSE
   waValidity: 0
   waPolicy: DAILY
   waTime: 7200
   waTraffic: 314572800
   ---------------------------------------------------------------------------------------------------------------------------------
   */

   va_list argp;
   va_start(argp, fmt);

   int result = UServices::askToLDAP(pinput, table, fmt, argp);

   va_end(argp);

   if (result <= 0)
      {
      if (result == -1) // Can't contact LDAP server (-1)
         {
         U_LOGGER("*** LDAP NON DISPONIBILE (anomalia 008) ***", 0);

         title_txt = "Servizio LDAP non disponibile";
         message   = "Servizio LDAP non disponibile (anomalia 008)";
         }

      if (title_txt && message) USSIPlugIn::setMessagePage(*message_page_template, title_txt, message);
      }

   U_RETURN(result);
}

static bool runAuthCmd(const char* password, const char* realm)
{
   U_TRACE(5, "::runAuthCmd(%S,%S)", password, realm)

   U_INTERNAL_ASSERT(*fmt_auth_cmd)

   static int fd_stderr;

   UString cmd(U_CAPACITY);

   if (uid->size() > 32) goto error;

   cmd.snprintf(fmt_auth_cmd->data(), uid->c_str(), password, realm);

   if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/auth_cmd.err");

   *output = UCommand::outputCommand(cmd, 0, -1, fd_stderr);

   UServer_Base::logCommandMsgError(cmd.data(), true);

   if (UCommand::exit_value ||
       output->empty())
      {
   // U_LOGGER("*** AUTH_CMD failed: EXIT_VALUE=%d RESPONSE=%V - realm=%s uid=%s pass=%s ***", UCommand::exit_value, output->rep, realm, _uid, password);

error:
      char msg[4096];
      const char* title;

      (void) u__snprintf(msg, sizeof(msg),
               "%s<br>"
               "<a href=\"javascript:history.go(-1)\">Indietro</a>",
               UCommand::exit_value == 1 ? (title = "Utente e/o Password errato/i", "Credenziali errate!")
                                         : (title = "Errore", "Richiesta autorizzazione ha avuto esito errato"));

      USSIPlugIn::setMessagePage(*message_page_template, title, msg);

      U_RETURN(false);
      }

   U_RETURN(true);
}

// NB: logout => http://%s:5280/logout?ip=%s&mac=%s", *ap_address, user_rec->_ip, user_rec->_mac...

static bool askNodogToLogoutUser(const UString& _ip, const UString& _mac, bool bcheck)
{
   U_TRACE(5, "::askNodogToLogoutUser(%V,%V,%b)", _ip.rep, _mac.rep, bcheck)

   if (user_rec)
      {
      U_INTERNAL_DUMP("user_rec->_auth_domain = %V", user_rec->_auth_domain.rep)

      if (user_rec->_auth_domain == *account_auth)
         {
         U_LOGGER("*** GROUP ACCOUNT LOGOUT: UID(%v) IP(%v) MAC(%v) AP(%v) ***", uid->rep, _ip.rep, _mac.rep, ap_address->rep);

         U_RETURN(false);
         }
      }

#ifdef USE_LIBSSL
   UString data = UDES3::signData("ip=%v&mac=%v", _ip.rep, _mac.rep);
#else
   UString data(100U);

   data.snprintf("ip=%v&mac=%v", _ip.rep, _mac.rep);
#endif

   UString result = nodog_rec->sendRequestToNodog("logout?%v", data.rep);

   // ----------------------------------------------------------------------------------------------------------------------------------
   // NB: we can have 4 possibility:
   // ----------------------------------------------------------------------------------------------------------------------------------
   // 1) response Bad Request (404) => nodog don't know about this user
   // 2) response Ok          (200) => content is user status that must be DENY
   // 4) response Moved Temp  (302) => (response from redirect to portal with request GET /info?... but portal NOT responding)
   // 3) response No Content  (204) => (response from redirect to portal with request GET /info?... and portal     responding)
   // ----------------------------------------------------------------------------------------------------------------------------------

   if (U_IS_HTTP_ERROR(U_http_info.nResponseCode))
      {
      // 1) response Bad Request (404) => nodog don't know about this user

      U_LOGGER("*** LOGOUT FAILED: (nodog don't know about this user - U_http_info.nResponseCode = %d) UID(%v) IP(%v) MAC(%v) AP(%v) ***",
                  U_http_info.nResponseCode, uid->rep, _ip.rep, _mac.rep, ap_address->rep);

      U_RETURN(false);
      }

   if (result &&
       U_http_info.nResponseCode == HTTP_OK)
      {
      // 2) response Ok (200) => content is user status that must be DENY

      U_INTERNAL_ASSERT(U_IS_HTTP_SUCCESS(U_http_info.nResponseCode))

#  ifdef USE_LIBZ
      if (UStringExt::isGzip(result)) result = UStringExt::gunzip(result);
#  endif

      if (U_STRING_FIND(result, 0, "DENY") == U_NOT_FOUND)
         {
         U_LOGGER("*** USER STATUS NOT DENY: UID(%v) IP(%v) MAC(%v) AP(%v) ***", uid->rep, _ip.rep, _mac.rep, ap_address->rep);

         U_RETURN(false);
         }
      }
   else
      {
      if (U_http_info.nResponseCode == 0 ||
          U_http_info.nResponseCode == HTTP_MOVED_TEMP)
         {
         // 4) (302) => (response from redirect to portal with request GET /info?... but portal NOT responding

         char log_msg[4096];

         (void) u__snprintf(log_msg, sizeof(log_msg),
                            "%v: *** LOGOUT %%s AFTER %%d ATTEMPTS: UID(%v) IP(%v) MAC(%v) AP(%v) ***", UClientImage_Base::request_uri->rep, uid->rep, _ip.rep, _mac.rep, ap_address->rep);

         (void) client->sendGETRequestAsync(*url_nodog, false, log_msg, file_WARNING->getFd());

         U_RETURN(false);
         }

      // 3) response No Content (204) => (response from redirect to portal with request GET /info?... and portal responding)

      // NB: we must had serviced a info request from nodog by another process istance (PREFORK_CHILD > 2)...

      if (bcheck                        &&
          db_user->getDataStorage(*uid) &&
          user_rec->connected)
         {
         U_LOGGER("*** USER STILL CONNECTED: UID(%v) IP(%v) MAC(%v) AP(%v) ***", uid->rep, _ip.rep, _mac.rep, ap_address->rep);

         U_RETURN(false);
         }

      U_INTERNAL_DUMP("U_http_info.nResponseCode = %d", U_http_info.nResponseCode)
      }

// U_LOGGER("*** LOGOUT SUCCESS: UID(%v) IP(%v) MAC(%v) AP(%v) ***", uid->rep, _ip.rep, _mac.rep, ap_address->rep);

   U_RETURN(true);
}

static void askNodogToLogoutUser(UVector<UString>& vec, bool bcheck)
{
   U_TRACE(5, "::askNodogToLogoutUser(%p,%b)", &vec, bcheck)

   pid_t pid = UServer_Base::startNewChild();

   if (pid > 0) return; // parent

   // child

   if (bcheck == false) user_rec = 0;

   for (uint32_t i = 0, n = vec.size(); i < n; i += 3)
      {
      *ap_address = vec[i];

      (void) askNodogToLogoutUser(vec[i+1], vec[i+2], bcheck);
      }

   if (pid == 0) UServer_Base::endNewChild();
}

static bool setAccessPointAddress()
{
   U_TRACE(5, "::setAccessPointAddress()")

   U_INTERNAL_ASSERT(ap)

   uint32_t pos = ap->find('@');

   if (pos != U_NOT_FOUND)
      {
      *ap_label   = ap->substr(0U, pos).copy();
      *ap_address = ap->substr(pos + 1).copy();

      if (*ap_address)
         {
         if ((ap_address_trust = ap_address->isBase64Url())) *ap_address = UDES3::getSignedData(*ap_address);

         if (u_isIPv4Addr(U_STRING_TO_PARAM(*ap_address)) &&
                   db_nodog->getDataStorage(*ap_address))
            {
            U_RETURN(true);
            }
         }
      }

   U_RETURN(false);
}

static void setAccessPointLocalization()
{
   U_TRACE(5, "::setAccessPointLocalization()")

   U_INTERNAL_ASSERT_POINTER(ap_ref)
   U_INTERNAL_ASSERT_POINTER(ap_ref_ap)
   U_INTERNAL_ASSERT_POINTER(ap_ref_comune)
   U_INTERNAL_ASSERT_POINTER(url_banner_ap)
   U_INTERNAL_ASSERT_POINTER(url_banner_comune)
   U_INTERNAL_ASSERT_POINTER(url_banner_ap_path)
   U_INTERNAL_ASSERT_POINTER(url_banner_comune_path)

   if (ap_address->equal(U_CONSTANT_TO_PARAM(IP_UNIFI)) ||
       ap_address->equal(U_CONSTANT_TO_PARAM(IP_UNIFI)))
      {
      if (ap_label->equal(U_CONSTANT_TO_PARAM("05"))) // careggi
         {
         ap_ref_ap->snprintf(    "/Xcareggi", 0);
         ap_ref_comune->snprintf("/Xcareggi", 0);
         }
      else
         {
         ap_ref_ap->snprintf(    "/Xunifi", 0);
         ap_ref_comune->snprintf("/Xunifi", 0);
         }

      return;
      }

   if (ap_address->equal(U_CONSTANT_TO_PARAM(IP_CASCINE)))
      {
      ap_ref_ap->snprintf(    "/Xcascine", 0);
      ap_ref_comune->snprintf("/Xcascine", 0);

      return;
      }

   ap_ref_ap->setEmpty();
   ap_ref_comune->setEmpty();

   if ((ap_address->empty()                                    ||
        u_isIPv4Addr(U_STRING_TO_PARAM(*ap_address)) == false) ||
       (url_banner_ap_default     == 0                         &&
        url_banner_comune_default == 0))
      {
      return;
      }

   char* ptr;
   uint32_t len;
   char buffer[4096];

   setAccessPointReference(U_STRING_TO_PARAM(*ap_address));

   if (url_banner_ap_default)
      {
      U_ASSERT(url_banner_ap_default->dir())

      // "wifi-aaa.comune.fi.it/banner/luoghi/X0054R13/full/banner.html"
      // "wifi-aaa.comune.fi.it/banner/luoghi/X0054R13/mobile/banner.html"

      ptr = buffer + (len = u__snprintf(buffer, sizeof(buffer), "%v%v/%v/", virtual_name->rep, url_banner_ap->rep, ap_ref->rep));

      if (UHTTP::getFileInCache(buffer, len + u__snprintf(ptr, sizeof(buffer)-len, "%s",   "full/banner.html")) ||
          UHTTP::getFileInCache(buffer, len + u__snprintf(ptr, sizeof(buffer)-len, "%s", "mobile/banner.html")))
         {
         ap_ref_ap->snprintf("/%v", ap_ref->rep);
         }

      /*
      banner.snprintf("%v/%v", url_banner_ap_path->rep, ap_ref->rep);

      ptr = banner.data();

           if (UFile::access(ptr)) ap_ref_ap->snprintf("/%v", ap_ref->rep);
      else if (UFile::_mkdir(ptr))
         {
         (void) banner.append(U_CONSTANT_TO_PARAM("/default"));

         ptr = banner.data();

         if (UFile::access(ptr) == false) (void) url_banner_ap_default->symlink(ptr);
         }
      */
      }

   if (url_banner_comune_default)
      {
      U_ASSERT(url_banner_comune_default->dir())

      // "wifi-aaa.comune.fi.it/banner/eventi/X0054R13/full/banner.html"
      // "wifi-aaa.comune.fi.it/banner/eventi/X0054R13/mobile/banner.html"

      ptr = buffer + (len = u__snprintf(buffer, sizeof(buffer), "%v%v/%v/", virtual_name->rep, url_banner_comune->rep, ap_ref->rep));

      if (UHTTP::getFileInCache(buffer, len + u__snprintf(ptr, sizeof(buffer)-len, "%s",   "full/banner.html")) ||
          UHTTP::getFileInCache(buffer, len + u__snprintf(ptr, sizeof(buffer)-len, "%s", "mobile/banner.html")))
         {
         ap_ref_comune->snprintf("/%v", ap_ref->rep);
         }

      /*
      banner.snprintf("%v/%v", url_banner_comune_path->rep, ap_ref->rep);

      ptr = banner.data();

           if (UFile::access(ptr)) ap_ref_comune->snprintf("/%v", ap_ref->rep);
      else if (UFile::_mkdir(ptr))
         {
         (void) banner.append(U_CONSTANT_TO_PARAM("/default"));

         ptr = banner.data();

         if (UFile::access(ptr) == false) (void) url_banner_comune_default->symlink(ptr);
         }
      */
      }
}

static bool setAccessPoint(bool localization)
{
   U_TRACE(5, "::setAccessPoint(%b)", localization)

   // $1 -> ap
   // $2 -> public address to contact the access point
   // $3 -> pid (0 => start)

            ap->clear();
      ap_label->clear();
    ap_address->clear();
   ap_hostname->clear();

   int ap_port  = 5280;
   uint32_t end = UHTTP::processForm();

   if (end)
      {
      uint32_t pos;
      UString hostname, address;

      UHTTP::getFormValue(hostname, U_CONSTANT_TO_PARAM("ap"),     0, 1, end);
      UHTTP::getFormValue(address,  U_CONSTANT_TO_PARAM("public"), 0, 3, end);

      if (hostname)
         {
         pos = hostname.find('@');

         if (pos == U_NOT_FOUND) *ap_hostname = hostname;
         else
            {
            *ap_label    = hostname.substr(0U, pos).copy();
            *ap_hostname = hostname.substr(pos + 1).copy();
            }

         // NB: bad case generated by main.bash...

         if (u_isIPv4Addr(U_STRING_TO_PARAM(*ap_hostname)))
            {
            *ap_address = *ap_hostname;
                           ap_hostname->clear();

            goto next;
            }

         if (u_isHostName(U_STRING_TO_PARAM(*ap_hostname)) == false)
            {
            U_LOGGER("*** AP HOSTNAME(%v) NOT VALID ***", ap_hostname->rep);

            U_RETURN(false);
            }
         }

      if (address)
         {
         pos = address.find(':');

         if (pos == U_NOT_FOUND) *ap_address = address;
         else
            {
            *ap_address = address.substr(0U, pos).copy();
             ap_port    = address.substr(pos+1).strtol();
            }

         if ((ap_address_trust = ap_address->isBase64Url())) *ap_address = UDES3::getSignedData(*ap_address);

         if (u_isIPv4Addr(U_STRING_TO_PARAM(*ap_address)) == false)
            {
            U_LOGGER("*** AP ADDRESS(%v) NOT VALID ***", ap_address->rep);

            U_RETURN(false);
            }
         }
      }

   U_INTERNAL_DUMP("ap_address = %V ap_hostname = %V ap_label = %V", ap_address->rep, ap_hostname->rep, ap_label->rep)

   if (ap_address->empty()    ||
       (localization          &&
        (   ap_label->empty() ||
         ap_hostname->empty())))
      {
      U_RETURN(false);
      }

next:
   if (nodog_rec->setRecord(ap_port)) U_RETURN(true);

   U_RETURN(false);
}

static bool checkLoginRequest(uint32_t n, uint32_t end, int tolerance, bool bempty)
{
   U_TRACE(5, "::checkLoginRequest(%u,%u,%d,%b)", n, end, tolerance, bempty)

            ap->clear();
            ip->clear();
           uid->clear();
           mac->clear();
         redir->clear();
       gateway->clear();
      redirect->clear();
      ap_label->clear();
    ap_address->clear();
   ap_hostname->clear();

   if (n == 0) n = UHTTP::processForm();

   U_INTERNAL_DUMP("n = %u end = %u diff = %d", n, end, n - end)

   if (n != end)
      {
      int diff = (n - end);

      if (diff != tolerance)
         {
         int a = end - tolerance,
             b = end + tolerance;

         if ((int)n < a ||
             (int)n > b)
            {
         // if (n) U_LOGGER("*** NUM FORM ELEMENT(%u) DIFFERENT FROM EXPECTED(%u:%d) ***", n, end, tolerance);

            U_RETURN(false);
            }
         }
      }

   // ----------------------------------------------------------------------------------------
   // NB: *** the params CAN be empty ***
   // ----------------------------------------------------------------------------------------
   // $1 -> mac
   // $2 -> ip
   // $3 -> redirect
   // $4 -> gateway
   // $5 -> timeout
   // $6 -> token
   // $7 -> ap (with localization => '@')
   // ----------------------------------------------------------------------------------------

   UHTTP::getFormValue(*ap, U_CONSTANT_TO_PARAM("ap"), 0, 13, n);

   if (ap->empty()) U_RETURN(bempty);

   if (setAccessPointAddress())
      {
      UHTTP::getFormValue(*mac,      U_CONSTANT_TO_PARAM("mac"),      0, 1, n);
      UHTTP::getFormValue(*ip,       U_CONSTANT_TO_PARAM("ip"),       0, 3, n);
      UHTTP::getFormValue(*redirect, U_CONSTANT_TO_PARAM("redirect"), 0, 5, n);
      UHTTP::getFormValue(*gateway,  U_CONSTANT_TO_PARAM("gateway"),  0, 7, n);

      U_RETURN(true);
      }

   U_RETURN(false);
}

static void getLoginRequest(UString& timeout, UString& realm)
{
   U_TRACE(5, "::getLoginRequest(%p,%p)", &timeout, &realm)

   uint32_t n = UHTTP::processForm();

   if (checkLoginRequest(n, 18, 18, false))
      {
      UHTTP::getFormValue(timeout, U_CONSTANT_TO_PARAM("timeout"), 0,  9, n);
      UHTTP::getFormValue(*token,  U_CONSTANT_TO_PARAM("token"),   0, 11, n);
      UHTTP::getFormValue(*ap,     U_CONSTANT_TO_PARAM("ap"),      0, 13, n);
      }

   UHTTP::getFormValue(realm,  U_CONSTANT_TO_PARAM("realm"),    0, 15, n);
   UHTTP::getFormValue(*redir, U_CONSTANT_TO_PARAM("redir_to"), 0, 17, n);

   if (realm.empty())
      {
      /*
      USSIPlugIn::setMessagePage(*message_page_template, "Errore", "Errore Autorizzazione - dominio vuoto");

      return;
      */

      realm = U_STRING_FROM_CONSTANT("all");
      }
}

static void checkUserID()
{
   U_TRACE(5, "::checkUserID()")

   if (WiAuthUser::checkUserID())
      {
      if (isMAC &&
          *mac != *uid)
         {
         U_LOGGER("*** MAC MISMATCH: UID(%v) IP(%v) MAC(%v) AP(%v) REDIR(%v) ***", uid->rep, ip->rep, mac->rep, ap->rep, redir->rep);

      // U_ASSERT(UStringExt::startsWith(*auth_domain, *mac_auth))

         UString x;

         if (*mac) x = UStringExt::trim(*mac);

         if (x) *uid = x;
         }
      else if (isIP &&
               *ip != *uid)
         {
         U_LOGGER("*** IP MISMATCH: UID(%v) IP(%v) MAC(%v) AP(%v) REDIR(%v) ***", uid->rep, ip->rep, mac->rep, ap->rep, redir->rep);

      // U_ASSERT(UStringExt::startsWith(*auth_domain, *ip_auth))

         UString x;

         if (*ip) x = UStringExt::trim(*ip);

         if (x) *uid = x;
         }
      }
}

static bool checkLoginValidate(bool all)
{
   U_TRACE(5, "::checkLoginValidate(%b)", all)

   policy->clear();

   U_INTERNAL_DUMP("redirect = %V", redirect->rep)

   U_INTERNAL_ASSERT(*redirect)

   uint32_t sz = redirect->size();

   if (sz <= 32)
      {
      *uid    = *redirect;
      *redir  = *redirect_default;
      *policy = *policy_daily;
      }
   else
      {
      UString str = UDES3::getSignedData(*redirect);

      // ========================
      // 1 => uid
      // 2 => policy
      // 3 => auth_domain
      // 4 => max_time
      // 5 => max_traffic
      // 6 => UserDownloadRate
      // 7 => UserUploadRate
      // 8 => redir_to
      // ========================

      ptr3 = ptr2 = (ptr1 = str.data()) + U_CONSTANT_SIZE("uid=");

      for (const char* end = str.end(); ptr3 < end; ++ptr3)
         {
         if (*ptr3 == '&') break;
         }

      sz = ptr3 - ptr2;

      (void) uid->replace(ptr2, sz);

      if (*ptr3 != '&'                         ||
          uid->findWhiteSpace() != U_NOT_FOUND ||
          u_get_unalignedp32(ptr1) != U_MULTICHAR_CONSTANT32('u','i','d','='))
         {
         U_LOGGER("*** checkLoginValidate(%b) FAILED: DATA(%v) ***", all, str.rep);

         U_RETURN(false);
         }

      UVector<UString> name_value(14);

      sz = UStringExt::getNameValueFromData(str.substr(U_CONSTANT_SIZE("uid=")+sz), name_value, U_CONSTANT_TO_PARAM("&"));

      if (sz < 12)
         {
      // U_LOGGER("*** NAME=VALUE DECODE FAILED: sz=%u VECTOR(%S) ***", sz, UObject2String(name_value));

         U_RETURN(false);
         }

      (void) policy->replace(           name_value[ 1]);
      (void)  redir->replace(sz >= 14 ? name_value[13] : *redirect_default);

      if (all)
         {
         (void)       auth_domain->replace(name_value[ 3]);
         (void)    time_available->replace(name_value[ 5]);
         (void) traffic_available->replace(name_value[ 7]);
         (void) user_DownloadRate->replace(name_value[ 9]);
         (void)   user_UploadRate->replace(name_value[11]);

         UHTTP::getFormValue(*token, U_CONSTANT_TO_PARAM("token"), 0, 11, 14);
         }
      }

   redirect->clear();

   U_RETURN(true);
}

static bool getCookie(UString* prealm, UString* pid)
{
   U_TRACE(5, "::getCookie(%p,%p)", prealm, pid)

   if (UServer_Base::bssl)
      {
      UString cookie = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("HTTP_COOKIE"), UClientImage_Base::environment);

      if (cookie)
         {
         uint32_t pos = U_STRING_FIND(cookie, 0, "WCID=");

         if ( pos != U_NOT_FOUND &&
             (pos += U_CONSTANT_SIZE("WCID="), pos < cookie.size()))
            {
            uint32_t pos1   = cookie.findWhiteSpace();
            const char* ptr = (pos1 == U_NOT_FOUND ? cookie.end() : cookie.c_pointer(pos1));

            if (ptr[-1] == ';') --ptr;

            UString value = cookie.substr(pos, ptr - cookie.c_pointer(pos));

            if (pid) *pid = value.copy();

            if (askToLDAP(0, 0, 0, "ldapsearch -LLL -b %v %v (&(objectClass=waSession)(&(waCookieId=%v)))", wiauth_session_basedn->rep, ldap_session_param->rep, value.rep) != 1)
               {
               setCookie(0);

               U_RETURN(false);
               }

            value = (*table)["waFederatedUserId"]; // Ex: 3343793489@all

            if (value)
               {
               pos = value.find('@');

               if (pos == U_NOT_FOUND) *uid = value;
               else
                  {
                  UString x;

                  *uid = value.substr(0U, pos).copy();
                  x    = value.substr(pos + 1).copy();

                  if (prealm &&
                      prealm->equal(x) == false)
                     {
                  // U_LOGGER("*** COOKIE REALM DIFFER(%v=>%v) - UID(%v) IP(%v) MAC(%v) AP(%v) ***", prealm->rep, x.rep, uid->rep, ip->rep, mac->rep, ap->rep);

                     *prealm = UStringExt::trim(x);
                     }
                  }

               U_RETURN(true);
               }
            }
         }
      }

   U_RETURN(false);
}

static void sendRedirect()
{
   U_TRACE(5, "::sendRedirect()")

   U_INTERNAL_DUMP("redir = %V", redir->rep)

   U_INTERNAL_ASSERT(*redir)

   if (strncmp(redir->c_pointer(U_CONSTANT_SIZE("http://")), U_STRING_TO_PARAM(*virtual_name))) USSIPlugIn::setAlternativeRedirect("%v", redir->rep);
   else                                                                                         USSIPlugIn::setAlternativeRedirect("http://www.google.com", 0);
}

static void sendLoginValidate()
{
   U_TRACE(5, "::sendLoginValidate()")

   if (policy->empty()) *policy = *policy_daily;

   WiAuthUser::loadPolicy(*policy); // NB: time_available e traffic_available sono valorizzati da loadPolicy()...

   if (redir->empty() ||
       redir->size() > 2048)
      {
      *redir = *redirect_default;
      }

   // ========================
   // 1 => uid
   // 2 => policy
   // 3 => auth_domain
   // 4 => max_time
   // 5 => max_traffic
   // 6 => UserDownloadRate
   // 7 => UserUploadRate
   // 8 => redir_to
   // ========================

#ifdef USE_LIBSSL
   UString signed_data = UDES3::signData(
#else
   UString signed_data(500U + redir->size());

   signed_data.snprintf(
#endif
      "uid=%v&policy=%v&auth_domain=%v&max_time=%v&max_traffic=%v&UserDownloadRate=%v&UserUploadRate=%v&redir_to=%v",
       uid->rep, policy->rep, auth_domain->rep, time_available->rep, traffic_available->rep, user_DownloadRate->rep, user_UploadRate->rep, redir->rep);

   // -------------------------------------------------------------------------------------------------------------------
   // NB: in questo modo l'utente ripassa dal firewall e NoDog lo rimanda da noi (login_validate) con i dati rinnovati...
   // -------------------------------------------------------------------------------------------------------------------
   // Ho predisposto un servizio di redirect sulla tnet concentratore che redirige sul portale autorizzativo.
   // Il portale autorizzativo dovrà implementare il servizio 'fake_login_validate' che semplicemente estrae la
   // url di redirect contenuta nella URL di richiesta e fa semplicemente la redirect sulla stessa
   // -------------------------------------------------------------------------------------------------------------------

   USSIPlugIn::setAlternativeRedirect(UStringExt::startsWith(*title_default, U_CONSTANT_TO_PARAM("Firenze ")) ? LOGIN_VALIDATE_REDIR_FI : LOGIN_VALIDATE_REDIR, signed_data.rep);
}

// -------------------------------------------------------------------------------------------------
// NB: se il portatile non mostra la login page controllare in /etc/hosts se e' valorizzato wifi-...
// -------------------------------------------------------------------------------------------------

static void GET_admin()
{
   U_TRACE(5, "::GET_admin()")

   USSIPlugIn::setAlternativeRedirect("https://%v/admin.html", ip_server->rep);
}

static void GET_admin_edit_ap()
{
   U_TRACE(5, "::GET_admin_edit_ap()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      // $1 -> ap (with localization => '@')
      // $2 -> public address to contact the access point

      if (setAccessPoint(true))
         {
         WiAuthAccessPoint* ap_rec = nodog_rec->vec_access_point[index_access_point];

         USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_edit_ap.tmpl")), 0, false,
                                           title_default->data(), 0, 0,
                                           ap_rec->label.rep,       "readonly",
                                           ap_address->rep,         "readonly",
                                           nodog_rec->hostname.rep, "readonly",
                                           ap_rec->mac_mask.rep,
                                           ap_rec->group_account_mask.rep,
                                           ap_rec->noconsume ? "" : "checked");
         }
      else
         {
         if (UHTTP::form_name_value->empty() == false) USSIPlugIn::setBadRequest();
         else
            {
            UString tmp1 = U_STRING_FROM_CONSTANT("ap"),
                    tmp2 = U_STRING_FROM_CONSTANT("10.8.0.xxx"),
                    tmp3 = U_STRING_FROM_CONSTANT("aaa-r29587_bbb");

            USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_edit_ap.tmpl")), 0, false,
                                              title_default->data(), 0, 0,
                                              tmp1.rep, "",
                                              tmp2.rep, "",
                                              tmp3.rep, "",
                                              UStringRep::string_rep_null,
                                              UStringRep::string_rep_null,
                                              "checked");
            }
         }
      }
}

static void GET_admin_export_statistics_login_as_csv()
{
   U_TRACE(5, "::GET_admin_export_statistics_login_as_csv()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void GET_admin_export_statistics_registration_as_csv()
{
   U_TRACE(5, "::GET_admin_export_statistics_registration_as_csv()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void GET_admin_export_view_using_historical_as_csv()
{
   U_TRACE(5, "::GET_admin_export_view_using_historical_as_csv()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      UString body = file_UTILIZZO->_getContent();

      USSIPlugIn::setAlternativeResponse(body);
      }
}

static void GET_admin_historical_statistics_login()
{
   U_TRACE(5, "::GET_admin_historical_statistics_login()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void GET_admin_login_nodog()
{
   U_TRACE(5, "::GET_admin_login_nodog()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      totale4 = 0;
      totale1 = totale2 = totale3 = 0;
      num_ap = num_users_connected = 0;

      output->setBuffer(U_CAPACITY);

       db_user->callForAllEntry(countUsersConnected);
      db_nodog->callForAllEntry(setLoginNodogTotal);
      db_nodog->callForAllEntry(setLoginNodog, 0, login_nodog_compare);

      USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_login_nodog.tmpl")), output->size(), false,
                            "stato accesso utenti", 0, 0,
                            num_ap, num_users_connected,
                            (UStringExt::startsWith(*title_default, U_CONSTANT_TO_PARAM("Firenze ")) ? "FIRENZE CARD" : "OTHER"),
                            output->rep,
                            totale1, totale2, totale3, (uint32_t)(totale4 / (1024ULL * 1024ULL * 1024ULL)));
      }
}

static void GET_admin_printlog()
{
   U_TRACE(5, "::GET_admin_printlog()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void GET_admin_recovery()
{
   U_TRACE(5, "::GET_admin_recovery()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void GET_admin_status_network()
{
   U_TRACE(5, "::GET_admin_status_network()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      output->setBuffer(U_CAPACITY);

      user_exist = true;
      num_ap = num_users_connected = 0;

       db_user->callForAllEntry(getStatusUser);
      db_nodog->callForAllEntry(countAP);

      USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_status_network.tmpl")), output->size(), false,
                                        "stato rete", 0, 0,
                                        num_ap, num_users_connected, output->rep);
      }
}

static void GET_admin_status_nodog()
{
   U_TRACE(5, "::GET_admin_status_nodog()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      num_ap             =
      num_ap_up          =
      num_ap_down        =
      num_ap_open        =
      num_ap_noconsume   =
      num_ap_unreachable = 0;

      output->setBuffer(U_CAPACITY);

      db_nodog->callForAllEntry(setStatusNodog, 0, UStringExt::qscompver);

      uint32_t sz = output->size();

      USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_status_nodog.tmpl")), sz, false,
                            "stato access point", 0, 0,
                            num_ap, num_ap_up, num_ap_down, num_ap_unreachable, num_ap_open, num_ap_noconsume,
                            virtual_name->rep, output->rep);
      }
}

static void GET_admin_view_statistics_login()
{
   U_TRACE(5, "::GET_admin_view_statistics_login()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void GET_admin_view_statistics_registration()
{
   U_TRACE(5, "::GET_admin_view_statistics_registration()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void GET_admin_view_user()
{
   U_TRACE(5, "::GET_admin_view_user()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("get_user_id.tmpl")), 0, false,
                            "Visualizzazione dati utente",
                            "<link type=\"text/css\" href=\"css/livevalidation.css\" rel=\"stylesheet\" />"
                            "<script type=\"text/javascript\" src=\"js/livevalidation_standalone.compressed.js\"></script>", 0,
                            "Visualizzazione dati utente", "admin_view_user");
      }
}

static void GET_admin_view_using()
{
   U_TRACE(5, "::GET_admin_view_using()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      num_ap              =
      num_ap_noconsume    =
      num_users_connected = 0;

       db_user->callForAllEntry(countUsersConnected);
      db_nodog->callForAllEntry(getNumAccessPointNoConsume);

      // NB: we may be a different process from what it has updated so that we need to read the record...

             UHTTP::db_session->setPointerToDataStorage(data_rec);
      (void) UHTTP::db_session->getDataStorage();

      uint32_t divisor = (data_rec->utenti_connessi_giornaliero_globale ? (data_rec->utenti_connessi_giornaliero_globale * 60U)
                                                                        : 1); // to avoid Signal SIGFPE (8, Floating point exception)

      USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_view_using.tmpl")), 0, false,
                            "Visualizzazione dati utilizzo",
                            "<meta http-equiv=\"refresh\" content=\"30\" />", 0,
                            num_ap, num_users_connected,
                            data_rec->utenti_connessi_giornaliero_globale,
                            data_rec->tempo_permanenza_utenti_giornaliero_globale / divisor,
                            num_ap_noconsume, (uint32_t)(data_rec->traffico_generato_giornaliero_globale / (1024ULL * 1024ULL * 1024ULL)));
      }
}

static void GET_admin_view_using_historical()
{
   U_TRACE(5, "::GET_admin_view_using_historical()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      UString content = file_UTILIZZO->_getContent();
      uint32_t n = 1, _totale1 = 0, _totale2 = 0, _totale3 = 0;

      output->setBuffer(U_CAPACITY);

      if (content)
         {
         UVector<UString> vec1(content, '\n'), vec2(4);
         UString tmp0, tmp1, tmp2, tmp3, riga(U_CAPACITY);
         const char* form = "<tr><td class=\"data_smaller\" align=\"right\">%v</td>\n"
                                "<td class=\"data_smaller\" align=\"right\">%v</td>\n"
                                "<td class=\"data_smaller\" align=\"right\">%v</td>\n"
                                "<td class=\"data_smaller\" align=\"right\">%v</td></tr>\n";

         /**
          * .....................
          * "2013/09/11","2843","78","103",
          * "2013/09/11","3157","76","111",
          * .....................
          */

         n = vec1.size();

         for (uint32_t i = 0; i < n; ++i)
            {
            (void) vec2.split(vec1[i], ',');

            tmp0 = vec2[0];
            tmp1 = vec2[1];
            tmp2 = vec2[2];
            tmp3 = vec2[3];

            tmp0.rep->unQuote();
            tmp1.rep->unQuote();
            tmp2.rep->unQuote();
            tmp3.rep->unQuote();

            _totale1 += tmp1.strtol();
            _totale2 += tmp2.strtol();
            _totale3 += tmp3.strtol();

            riga.snprintf(form, tmp0.rep, tmp1.rep, tmp2.rep, tmp3.rep);

            (void) output->append(riga);

            vec2.clear();
            }
         }

      USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_view_using_historical.tmpl")), output->size(), false,
                            "Visualizzazione dati utilizzo per data",
                            0, 0,
                            output->rep, _totale1, _totale2 / n, _totale3);
      }
}

static void GET_error_ap()
{
   U_TRACE(5, "::GET_error_ap()")

   // $1 -> ap (without localization => '@')
   // $2 -> public address to contact the access point
   // $3 -> msg: out of memory condition

   if (setAccessPoint(false))
      {
      uint32_t end = UHTTP::form_name_value->size();

      if (end == 6)
         {
         UString msg(200U);

         UHTTP::getFormValue(msg, U_CONSTANT_TO_PARAM("msg"), 0, 5, end);

         U_LOGGER("*** ON AP(%v:%v) NODOG IS OUT OF MEMORY *** %v", ap_address->rep, ap_hostname->rep, msg.rep);
         }
      else
         {
         U_LOGGER("*** ON AP(%v:%v) THE FIREWALL IS NOT ALIGNED ***", ap_address->rep, ap_hostname->rep);

         UString result = nodog_rec->sendRequestToNodog("users", 0);

         if (U_IS_HTTP_ERROR(U_http_info.nResponseCode) == false)
            {
            if (result.empty()) pvallow = 0;
            else
               {
               U_ASSERT(          vuid->empty())
               U_ASSERT(vallow_IP_user->empty())

#           ifdef USE_LIBZ
               if (UStringExt::isGzip(result)) result = UStringExt::gunzip(result);
#           endif

               UIPAllow* elem;
               UVector<UString> vec(result);

               for (uint32_t i = 0, n = vec.size(); i < n; i += 5)
                  {
                  elem = U_NEW(UIPAllow);

                  if (elem->parseMask(vec[i+1]) == false)
                     {
                     delete elem;

                     continue;
                     }

                  vuid->push_back(vec[i]);

                  vallow_IP_user->push_back(elem);
                  }

               pvallow = vallow_IP_user;
               }

            num_users_connected = 0;

            db_user->callForAllEntry(checkStatusUserOnNodog);

                      vuid->clear();
            vallow_IP_user->clear();
            }
         }
      }

   USSIPlugIn::setAlternativeResponse();
}

static void GET_fake_login_validate()
{
   U_TRACE(5, "::GET_fake_login_validate()")

   // ========================
   // => uid
   // => policy
   // => auth_domain
   // => max_time
   // => max_traffic
   // => UserDownloadRate
   // => UserUploadRate
   // => redir_to
   // ========================

   if (u_clientimage_info.http_info.query_len == 0 ||
       ((void) redirect->assign(U_HTTP_QUERY_TO_PARAM), checkLoginValidate(false)) == false)
      {
      loginWithProblem();

      return;
      }

   U_LOGGER("*** ALREADY LOGGED IN: UID(%v) IP(%.*s) REDIR(%v) ***", uid->rep, U_CLIENT_ADDRESS_TO_TRACE, redir->rep);

// USSIPlugIn::setMessagePage(*message_page_template, "Login", "Sei già loggato! (fake_login_validate)");

   sendRedirect();
}

static void GET_gen_activation()
{
   U_TRACE(5, "::GET_gen_activation()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void GET_get_ap_check_firewall()
{
   U_TRACE(5, "::GET_get_ap_check_firewall()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      // $1 -> ap (without localization => '@')
      // $2 -> public address to contact the access point

      if (setAccessPoint(false)) (void) nodog_rec->sendRequestToNodog("checkFirewall", 0);

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_get_ap_check_zombies()
{
   U_TRACE(5, "::GET_get_ap_check_zombies()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      // $1 -> ap (without localization => '@')
      // $2 -> public address to contact the access point

      if (setAccessPoint(false)) (void) nodog_rec->sendRequestToNodog("checkZombies", 0);

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_get_ap_name()
{
   U_TRACE(5, "::GET_get_ap_name()")

   output->setBuffer(U_CAPACITY);

   db_nodog->callForAllEntry(getNameAccessPoint);

   USSIPlugIn::setAlternativeResponse(*output);
}

static void GET_get_ap_uptime()
{
   U_TRACE(5, "::GET_get_ap_uptime()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      // $1 -> ap (without localization => '@')
      // $2 -> public address to contact the access point

      if (setAccessPoint(false))                       get_ap_uptime();
   // else db_nodog->callForAllEntry(getAccessPointUP, get_ap_uptime);

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_get_config()
{
   U_TRACE(5, "::GET_get_config()")

// if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
// else
      {
      // $1 -> ap (without localization => '@')
      // $2 -> key

      UString _body;
      uint32_t end = UHTTP::processForm();

      if (end)
         {
         UString key;

         UHTTP::getFormValue(key, U_CONSTANT_TO_PARAM("key"), 0, 3, end);

         if (key)
            {
            UString pathname(U_CAPACITY);

            pathname.snprintf("%w/ap/%v/nodog.conf", key.rep);

            _body = UFile::contentOf(pathname);

            if (_body.empty())
               {
               _body = *nodog_conf;

               if (u_isIPv4Addr(U_STRING_TO_PARAM(key))) *ip = key;
               else                                      (void) ip->assign(U_CLIENT_ADDRESS_TO_PARAM);

               UVector<UString> vec(*ip, '.');

               key = vec[3]; // 10.8.0.54

               pathname.snprintf("%w/ap/%v/nodog.conf.local", ip->rep);

               UString local = UFile::contentOf(pathname);

               const char* lan =                 "???";
               uint32_t    len = U_CONSTANT_SIZE("???");

               UHTTP::getFormValue(*ap, U_CONSTANT_TO_PARAM("ap"), 0, 1, end);

               if (*ap)
                  {
                  if (UStringExt::endsWith(*ap, U_CONSTANT_TO_PARAM("_pico2")))
                     {
                     lan =                 "ath0";
                     len = U_CONSTANT_SIZE("ath0");
                     }
                  else if (UStringExt::endsWith(*ap, U_CONSTANT_TO_PARAM("_x86")))
                     {
                     lan =                 "eth1";
                     len = U_CONSTANT_SIZE("eth1");
                     }
                  else if (UStringExt::endsWith(*ap, U_CONSTANT_TO_PARAM("_rspro")))
                     {
                     if (UStringExt::startsWith(*ap, U_CONSTANT_TO_PARAM("wimo")))
                        {
                        lan =                 "br-lan";
                        len = U_CONSTANT_SIZE("br-lan");
                        }
                     else
                        {
                        lan =                 "eth1";
                        len = U_CONSTANT_SIZE("eth1");
                        }
                     }
                  else
                     {
                     lan =                 "wlan0"; // ..._picoM2, ..._locoM2, ..._bulletM2, ..._unifiAP
                     len = U_CONSTANT_SIZE("wlan0");
                     }
                  }

               _body = UStringExt::substitute(_body, U_CONSTANT_TO_PARAM("<LAN>"),                                              lan, len);
               _body = UStringExt::substitute(_body, U_CONSTANT_TO_PARAM("<DDD>"),                                              U_STRING_TO_PARAM(key));
               _body = UStringExt::substitute(_body, U_CONSTANT_TO_PARAM("#include \"ap/<AAA.BBB.CCC.DDD>/nodog.conf.local\""), U_STRING_TO_PARAM(local));
               _body = UStringExt::substitute(_body, U_CONSTANT_TO_PARAM("<AAA.BBB.CCC.DDD>"),                                  U_STRING_TO_PARAM(*ip));

               UFileConfig cfg(_body, true);

               UHTTP::mime_index = U_know;

               if (cfg.processData())
                  {
                  _body = cfg.getData();

#              ifdef USE_LIBZ
                  if (U_http_is_accept_gzip &&
                      _body.size() > U_MIN_SIZE_FOR_DEFLATE)
                     {
                     _body = UStringExt::deflate(_body, 1);
                     }
#              endif
                  }
               }
            }
         }

      USSIPlugIn::setAlternativeResponse(_body);
      }
}

static void GET_get_users_info()
{
   U_TRACE(5, "::GET_get_users_info()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      if (UServer_Base::startParallelization()) return; // parent

      // ------------------------------------------------
      // child
      // ------------------------------------------------
      // $1 -> ap (without localization => '@')
      // $2 -> public address to contact the access point
      // ------------------------------------------------

      if (setAccessPoint(false))
         {
         UTimeVal to_sleep(U_TIME_FOR_ARPING_ASYNC_COMPLETION + 2);
loop:
         (void) nodog_rec->sendRequestToNodog("check", 0);

         if (U_http_info.nResponseCode == HTTP_NO_CONTENT)
            {
            to_sleep.nanosleep();

            goto loop;
            }
         }
      else
         {
         db_nodog->callForAllEntry(checkAccessPoint, (vPF)U_NOT_FOUND);
         }

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_help_wifi()
{
   U_TRACE(5, "::GET_help_wifi()")

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("help_wifi.tmpl")), 0, false,
                                     title_default->data(), 0, 0,
                                     0);
}

static void GET_info()
{
   U_TRACE(5, "::GET_info()")

   uint32_t end;
   uint64_t _traffic;
   long time_connected;
   UVector<UString> vec;
   const char* write_to_log;
   UString logout, connected, traffic;
   bool blogout, ask_logout = false, bcheckIfUserConnected, bsetAccessPointAddress, bsetNodogReference;

   for (int32_t i = 0, n = UHTTP::processForm(); i < n; i += 16)
      {
      // -----------------------------------------------------------------------------------------------------------------------------------------
      // $1 -> mac
      // $2 -> ip
      // $3 -> gateway
      // $4 -> ap (with localization => '@')
      // $5 -> => UID <=
      // $6 -> logout
      // $7 -> connected
      // $8 -> traffic
      // -----------------------------------------------------------------------------------------------------------------------------------------
      // /info?Mac=98%3A0c%3A82%3A76%3A3b%3A39&ip=172.16.1.8&gateway=172.16.1.254%3A5280&ap=ap%4010.8.0.1&User=1212&logout=1&connected=3&traffic=0
      // -----------------------------------------------------------------------------------------------------------------------------------------

      end = i+16;

      if (end > (uint32_t)n) end = n;

      UHTTP::getFormValue(*mac,      U_CONSTANT_TO_PARAM("Mac"),       i, i+1,  end);
      UHTTP::getFormValue(*ip,       U_CONSTANT_TO_PARAM("ip"),        i, i+3,  end);
      UHTTP::getFormValue(*ap,       U_CONSTANT_TO_PARAM("ap"),        i, i+7,  end);
      UHTTP::getFormValue(*uid,      U_CONSTANT_TO_PARAM("User"),      i, i+9,  end);
      UHTTP::getFormValue(logout,    U_CONSTANT_TO_PARAM("logout"),    i, i+11, end);
      UHTTP::getFormValue(connected, U_CONSTANT_TO_PARAM("connected"), i, i+13, end);
      UHTTP::getFormValue(traffic,   U_CONSTANT_TO_PARAM("traffic"),   i, i+15, end);

         ap_label->clear();
       ap_address->clear();
      ap_hostname->clear();

      blogout                = (logout && logout.first_char() != '0'); // NB: str logout == "0" mean NOT logout (only info)...
      bcheckIfUserConnected  = checkIfUserConnected(),
      bsetAccessPointAddress = (*ap && setAccessPointAddress()),
      bsetNodogReference     = (user_exist && bsetAccessPointAddress ? user_rec->setNodogReference() : false);

      if (bsetNodogReference     == false ||
          bcheckIfUserConnected  == false ||
          bsetAccessPointAddress == false)
         {
         U_LOGGER("*** INFO(%b,%b,%b,%b): UID(%v) IP(%v) MAC(%v) AP(%v) LOGOUT(%v) ***",
                     bcheckIfUserConnected, bsetAccessPointAddress, bsetNodogReference, blogout, uid->rep, ip->rep, mac->rep, ap->rep, logout.rep);

         if (bsetAccessPointAddress &&
             nodog_rec->findLabel())
            {
            if (blogout == false) ask_logout = true;

            goto next;
            }

         continue;
         }

      // Check if change of connection context for user id (mobility between access point)...

      if (*ip         != user_rec->_ip  ||
          *mac        != user_rec->_mac ||
          *ap_address != user_rec->nodog)
         {
         if (blogout == false &&
             user_rec->_auth_domain != *account_auth)
            {
            UString label = user_rec->getLabelAP();

            U_LOGGER("*** INFO DIFFERENCE: UID(%v) IP(%v=>%v) MAC(%v=>%v) AP(%v=>%v@%v) AUTH_DOMAIN(%v) LOGOUT(%v) ***", uid->rep,
                     ip->rep,  user_rec->_ip.rep,
                     mac->rep, user_rec->_mac.rep,
                     ap->rep, label.rep, user_rec->nodog.rep,
                     user_rec->_auth_domain.rep, logout.rep);

            vec.push_back(*ap_address);
            vec.push_back(*ip);
            vec.push_back(*mac);
            }
         }

      ask_logout = false;
next:
      _traffic       = traffic.strtoll();
      time_connected = connected.strtol();

      if (bsetNodogReference == false) write_to_log = 0;
      else
         {
         write_to_log = user_rec->updateCounter(logout, time_connected, _traffic, ask_logout);

         U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

         (void) db_user->putDataStorage(*uid);

         index_access_point = user_rec->_index_access_point;

         // NB: we may be a different process from what it has updated so that we need to read the record...

         (void) db_nodog->getDataStorage(*ap_address);

         // ------------------------------------------------------------------------------------------------------------------
         // INFO DIFFERENCE: UID(7c:11:be:86:5e:04) IP(172.16.180.184=>172.31.11.82) MAC(7c:11:be:86:5e:04=>7c:11:be:86:5e:04)
         //                  ADDRESS(ap@10.8.0.180=>11@10.8.0.54) AUTH_DOMAIN(MAC_AUTH_all) LOGOUT(0)
         //              *** UID(7c:11:be:86:5e:04) BINDED WITH AP(10.8.0.54) HAS AN INDEX(10) THAT IT IS INVALID (>= 1) ***
         // ------------------------------------------------------------------------------------------------------------------

         if (index_access_point >= nodog_rec->vec_access_point.size())
            {
            if (*ap_address == user_rec->nodog)
               {
               U_LOGGER("*** UID(%v) BINDED WITH AP(%v.*s) HAS AN INDEX(%u) THAT IT IS INVALID (>= %u) ***",
                             uid->rep, user_rec->nodog.rep, index_access_point, nodog_rec->vec_access_point.size());
               }

            index_access_point = 0;
            }

         nodog_rec->last_info = u_now->tv_sec;

         if (nodog_rec->status != 0)
            {
            U_LOGGER("*** AP(%v) CHANGE STATE (%d => 0) ***", ap_address->rep, nodog_rec->status);

            nodog_rec->status = 0;
            }

         nodog_rec->vec_access_point[index_access_point]->traffic_done += _traffic;

         (void) db_nodog->putDataStorage();
         }

             UHTTP::db_session->setPointerToDataStorage(data_rec);
      (void) UHTTP::db_session->getDataStorage();

      data_rec->traffico_generato_giornaliero_globale       += _traffic;
      data_rec->tempo_permanenza_utenti_giornaliero_globale += time_connected;

      (void) UHTTP::db_session->putDataStorage();

      if (write_to_log) user_rec->writeToLOG(write_to_log);

      if (ask_logout &&
          user_rec->nodog)
         {
         vec.push_back(user_rec->nodog);
         vec.push_back(user_rec->_ip);
         vec.push_back(user_rec->_mac);
         }
      }

   USSIPlugIn::setAlternativeResponse();

   if (vec.empty() == false) askNodogToLogoutUser(vec, false);
}

static void POST_info()
{
   U_TRACE(5, "::POST_info()")

#ifdef USE_LIBZ
   if (UStringExt::isGzip(*UClientImage_Base::body)) *UClientImage_Base::body = UStringExt::gunzip(*UClientImage_Base::body);
#endif

   GET_info();
}

static void GET_logged()
{
   U_TRACE(5, "::GET_logged()")

   USSIPlugIn::setAlternativeRedirect("http://www.google.com", 0);
}

static void GET_logged_login_request()
{
   U_TRACE(5, "::GET_logged_login_request()")

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("logged_login_request.tmpl")), 0, false,
                                     title_default->data(), 0, 0,
                                     0);
}

static void GET_login() // MAIN PAGE
{
   U_TRACE(5, "::GET_login()")

   // -----------------------------------------------------------------------------------------------------------------------------------------------
   // NB: *** the params CAN be empty ***
   // -----------------------------------------------------------------------------------------------------------------------------------------------
   // $1 -> mac
   // $2 -> ip
   // $3 -> redirect
   // $4 -> gateway
   // $5 -> timeout
   // $6 -> token
   // $7 -> ap (with localization => '@')
   // -----------------------------------------------------------------------------------------------------------------------------------------------
   // GET /login?mac=00%3A14%3AA5%3A6E%3A9C%3ACB&ip=192.168.226.2&redirect=http%3A%2F%2Fgoogle&gateway=192.168.226.1%3A5280&timeout=0&token=x&ap=lab2
   // -----------------------------------------------------------------------------------------------------------------------------------------------

   bool login_validate = false;

   if (checkLoginRequest(0, 14, 0, true))
      {
      if (WiAuthNodog::checkMAC())
         {
         *uid         = *mac;
         *auth_domain = *mac_auth;

         login_validate = true;
         }
#  ifdef USE_LIBSSL
      else if (UServer_Base::bssl)
         {
         X509* x509 = ((USSLSocket*)UServer_Base::csocket)->getPeerCertificate();

         if (x509)
            {
            long serial    = UCertificate::getSerialNumber(x509);
            UString issuer = UCertificate::getIssuer(x509);

            if (askToLDAP(0,0,0,"ldapsearch -LLL -b %v %v (&(objectClass=waUser)(&(waIssuer=%v)(waSerial=%ld)(waActive=TRUE)))",
                                 wiauth_user_basedn->rep, ldap_user_param->rep, issuer.rep, serial) == 1)
               {
               *uid = (*table)["waUid"];

               table->clear();

               *policy      = *policy_flat;
               *auth_domain = *cert_auth;

               login_validate = true;
               }
            }
         }
#  endif
      }

   if (*redirect)
      {
      // NB: we can have problems without encoding if redirect have query params...

      UString value_encoded((redirect->size() + 1024U) * 3U);

      Url::encode(*redirect, value_encoded);

      *redir = value_encoded;

      redirect->clear();

      U_INTERNAL_DUMP("redir = %V", redir->rep)
      }

   if (login_validate)
      {
      user_DownloadRate->replace('0');
      user_UploadRate->replace('0');

      sendLoginValidate(); // NB: in questo modo l'utente ripassa dal firewall e NoDog lo rimanda da noi (login_validate) con i dati rinnovati...

      return;
      }

   uint32_t sz;
   char format[8 * 1024U];
   UString request3(8 * 1024U);

   (void) u__snprintf(format, sizeof(format), "http%%s://%v/%%s?%%.*s%srealm=%%s&redir_to=%%v", virtual_name->rep, U_http_info.query_len ? "&" : "");

   request3.snprintf(format, "s", "login_request", U_HTTP_QUERY_TO_TRACE, "all", redir->rep);

   U_INTERNAL_DUMP("request3 = %V", request3.rep)

   sz = request3.size();

   if (UStringExt::startsWith(*title_default, U_CONSTANT_TO_PARAM("VillaBorbone ")) == false)
      {
      UString request1(8 * 1024U);

      ptr2 =                        (ptr1 = "s", "login_request");
      ptr3 = (mac->empty() ? ptr2 : (ptr1 =  "", "login_request_by_MAC"));

      request1.snprintf(format, ptr1, ptr3,  U_HTTP_QUERY_TO_TRACE, "all", redir->rep);

      sz += request1.size();

      setAccessPointLocalization();

      U_INTERNAL_DUMP("request1 = %V", request1.rep)

      if (UStringExt::startsWith(*title_default, U_CONSTANT_TO_PARAM("Firenze ")))
         {
         UString request2(8 * 1024U);

         request2.snprintf(format, "s", "login_request", U_HTTP_QUERY_TO_TRACE, "firenzecard", redir->rep);

         sz += request2.size();

         U_INTERNAL_DUMP("request2 = %S", request2.rep)

         USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("login.tmpl")), sz, false,
                                           title_default->data(), 0, 0,
                                           url_banner_ap->rep,
                                               ap_ref_ap->rep,
                                           request1.rep,
                                           help_url->rep,
                                           request2.rep,
                                           request3.rep,
                                           url_banner_comune->rep,
                                               ap_ref_comune->rep);
         }
      else
         {
         USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("login.tmpl")), sz, false,
                                           title_default->data(), 0, 0,
                                           url_banner_ap->data(),
                                               ap_ref_ap->data(),
                                           request1.data(),
                                           url_banner_comune->data(),
                                               ap_ref_comune->data());
         }
      }
   else
      {
      USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("login.tmpl")), sz, false,
                                        title_default->data(), 0, 0,
                                        request3.data(),
                                        ap->c_str(),
                                        wallet_url->data(),
                                        help_url->data());
      }
}

static void GET_LoginRequest(bool idp)
{
   U_TRACE(5, "::GET_LoginRequest(%b)", idp)

   // -----------------------------------------------------------------------------
   // *** the params CAN be empty ***
   // -----------------------------------------------------------------------------
   // $1 -> mac
   // $2 -> ip
   // $3 -> redirect
   // $4 -> gateway
   // $5 -> timeout
   // $6 -> token
   // $7 -> ap (with localization => '@')
   // $8 -> realm
   // $9 -> redir_to
   // -----------------------------------------------------------------------------

   UString id, timeout, realm;

   getLoginRequest(timeout, realm);

   if (idp == false &&
       getCookie(&realm, &id))
      {
      (void) checkIfUserConnected();

      if (user_exist)
         {
         user_DownloadRate->setFromNumber32(user_rec->DownloadRate);
         user_UploadRate->setFromNumber32(user_rec->UploadRate);
         }
      else
         {
         // ====================================================================================================================================
         // NB: this can happen if the user is authenticated but failed to redirect with login_validate by nodog...
         // ====================================================================================================================================
         // U_LOGGER("*** COOKIE(%V) FOUND BUT UID(%v) NOT EXIST: IP(%v) MAC(%v) AP(%v) ***", id.rep, uid->rep, ip->rep, mac->rep, ap->rep);
         // ====================================================================================================================================

         user_DownloadRate->replace('0');
         user_UploadRate->replace('0');
         }

      table->clear();

      *auth_domain = *cookie_auth + realm;

      if (realm.equal(U_CONSTANT_TO_PARAM("firenzecard")))
         {
         *policy = *policy_traffic;

         if (user_exist &&
             user_rec->_policy != *policy_traffic)
            {
            user_rec->_policy = *policy_traffic;

            U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

            (void) db_user->putDataStorage(*uid);
            }
         }
      else
         {
         if (realm.equal(U_CONSTANT_TO_PARAM("all")) == false ||
             askToLDAP(0,0,0,"ldapsearch -LLL -b %v %v waLogin=%v", wiauth_card_basedn->rep, ldap_card_param->rep, uid->rep) == -1)
            {
            *policy = (user_exist ? user_rec->_policy : *policy_daily);
            }
         else
            {
            *policy = (*table)["waPolicy"];
            }

         U_INTERNAL_ASSERT(*policy)
         }

      sendLoginValidate(); // NB: in questo modo l'utente ripassa dal firewall e NoDog lo rimanda da noi (login_validate) con i dati rinnovati...

      return;
      }

   UString tmp1, tmp2 = U_STRING_FROM_CONSTANT("<a href=\"password\">Non riesci a ricordare la password?</a>");

   if (realm.equal(U_CONSTANT_TO_PARAM("firenzecard")) == false)
      {
      tmp1 = U_STRING_FROM_CONSTANT("<p><input type=\"checkbox\" name=\"PersistentCookie\" "
                                    "id=\"PersistentCookie\" value=\"yes\" checked=\"checked\"><strong>Resta connesso</strong></p>");

      USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("login_request.tmpl")), 0, false,
                                        title_default->data(), 0, 0,
                                        login_url->rep, mac->rep, ip->rep, redirect->rep, gateway->rep, timeout.rep, token->rep, ap->rep, realm.rep, redir->rep,
                                        tmp1.rep, tmp2.rep);
      }
   else
      {
      if (idp)
         {
         tmp1 = U_STRING_FROM_CONSTANT("_IdP");

         tmp2.clear();
         }

      USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("login_request_firenzecard.tmpl")), 0, false,
                                        title_default->data(), 0, 0,
                                        login_url->rep, tmp1.rep, mac->rep, ip->rep, redirect->rep, gateway->rep, timeout.rep, token->rep, ap->rep, redir->rep, tmp2.rep);
      }
}

static void GET_login_request()
{
   U_TRACE(5, "::GET_login_request()")

   GET_LoginRequest(false);
}

static void GET_login_request_IdP()
{
   U_TRACE(5, "::GET_login_request_IdP()")

   GET_LoginRequest(true);
}

static void GET_login_request_by_MAC()
{
   U_TRACE(5, "::GET_login_request_by_MAC()")

   // -----------------------------------------------------------------------------
   // *** the params CAN be empty ***
   // -----------------------------------------------------------------------------
   // $1 -> mac
   // $2 -> ip
   // $3 -> redirect
   // $4 -> gateway
   // $5 -> timeout
   // $6 -> token
   // $7 -> ap (with localization => '@')
   // $8 -> realm
   // $9 -> redir_to
   // -----------------------------------------------------------------------------

   UString timeout, realm;

   getLoginRequest(timeout, realm);

   if (mac->empty())
      {
      USSIPlugIn::setMessagePage(*message_page_template, "Errore", "Errore Autorizzazione - MAC vuoto");

      return;
      }

   if (*mac == *UString::str_without_mac)
      {
      if (ip->empty())
         {
         USSIPlugIn::setMessagePage(*message_page_template, "Errore", "Errore Autorizzazione - IP vuoto");

         return;
         }

      *uid         = *ip;
      *auth_domain = *ip_auth + "_" + realm;
      }
   else
      {
      *uid         = *mac;
      *auth_domain = *mac_auth + "_" + realm;
      }

   *policy = *policy_daily;

   user_DownloadRate->replace('0');
   user_UploadRate->replace('0');

   sendLoginValidate(); // NB: in questo modo l'utente ripassa dal firewall e NoDog lo rimanda da noi (login_validate) con i dati rinnovati...
}

static void GET_login_validate()
{
   U_TRACE(5, "::GET_login_validate()")

   if (UServer_Base::bssl)
      {
      USSIPlugIn::setBadRequest();

      return;
      }

   // ---------------------------------------------------------------------------------------------------
   // NB: come back from the gateway (NoDog) after the POST of login_request, the params CANNOT be empty
   // ---------------------------------------------------------------------------------------------------
   // $1 -> mac
   // $2 -> ip
   // $3 -> redirect
   // ========================
   // => uid
   // => policy
   // => auth_domain
   // => max_time
   // => max_traffic
   // => UserDownloadRate 
   // => UserUploadRate 
   // => redir_to
   // ========================
   // $4 -> gateway
   // $5 -> timeout
   // $6 -> token
   // $7 -> ap (with localization => '@')
   // ---------------------------------------------------------------------------------------------------

   if (checkLoginRequest(0, 14, 0, false) == false ||
       checkLoginValidate(true)           == false ||
       nodog_rec->findLabel()             == false)
      {
      loginWithProblem();

      return;
      }

   UVector<UString> vec;
   UString x, signed_data(500U + U_http_info.query_len);

   checkUserID();

   if (checkIfUserConnected() &&
       isIP == false)
      {
      UString label = user_rec->getLabelAP();

      if (checkIfUserConnectedOnAP(label) &&
          user_rec->_auth_domain == *account_auth)
         {
         U_LOGGER("*** GROUP ACCOUNT: UID(%v) IP(%v) MAC(%v) AP(%v) REDIR(%v) ***", uid->rep, ip->rep, mac->rep, ap->rep, redir->rep);

         goto next;
         }

      // Check if change of connection context for user id (RENEW)

      if (*ip         == user_rec->_ip  &&
          *mac        == user_rec->_mac &&
          *ap_address == user_rec->nodog)
         {
         UString result = nodog_rec->sendRequestToNodog("users", 0);

         if (result &&
             U_IS_HTTP_ERROR(U_http_info.nResponseCode) == false)
            {
#        ifdef USE_LIBZ
            if (UStringExt::isGzip(result)) result = UStringExt::gunzip(result);
#        endif

            UVector<UString> vec1(result);

            for (uint32_t i = 0, n = vec1.size(); i < n; i += 5)
               {
               if (vec1[i] == *uid)
                  {
                  /*
                  U_LOGGER("*** ALREADY LOGGED IN: UID(%v) IP(%v) MAC(%v) AP(%v) REDIR(%v) ***", uid->rep, ip->rep, mac->rep, ap->rep, redir->rep);

                  USSIPlugIn::setMessagePage(*message_page_template, "Login", "Sei già loggato! (login_validate)");
                  */

                  sendRedirect();

                  return;
                  }
               }
            }
         }
      else
         {
         if (user_rec->_auth_domain == *account_auth)
            {
            USSIPlugIn::setMessagePage(*message_page_template, "Account busy", "Hai usato una credenziale che non e' disponibile in questo momento!");

            return;
            }

         U_LOGGER("*** RENEW: UID(%v) IP(%v=>%v) MAC(%v=>%v) ADDRESS(%v=>%v@%v) AUTH_DOMAIN(%v) ***", uid->rep,
                     user_rec->_ip.rep, ip->rep,
                     user_rec->_mac.rep, mac->rep,
                     label.rep, user_rec->nodog.rep, ap->rep,
                     user_rec->_auth_domain.rep);

         vec.push_back(user_rec->nodog);
         vec.push_back(user_rec->_ip);
         vec.push_back(user_rec->_mac);
         }
      }

   user_rec->setRecord();

   if (user_rec->consume == false)
      {
         time_counter->snprintf("%v", time_available->rep);
      traffic_counter->snprintf("%v", traffic_available->rep);
      }
   else
      {
      if (user_rec->_time_done >= user_rec->_time_available)
         {
         char msg[4096];

         (void) u__snprintf(msg, sizeof(msg),
                  "Hai consumato il tempo disponibile del servizio!<br>"
                  "ma puoi continuare comunque a utilizzare illimitatamente i servizi esposti sulla <a href=\"%v\">rete cittadina</a>",
                  wallet_url->rep);

         USSIPlugIn::setMessagePage(*message_page_template, "Tempo consumato", msg);

         goto end;
         }

      if (user_rec->_traffic_done >= user_rec->_traffic_available)
         {
         char msg[4096];

         (void) u__snprintf(msg, sizeof(msg),
                  "Hai consumato il traffico disponibile del servizio!<br>"
                  "ma puoi continuare comunque a utilizzare illimitatamente i servizi esposti sulla <a href=\"%v\">rete cittadina</a>",
                  wallet_url->rep);

         USSIPlugIn::setMessagePage(*message_page_template, "Traffico consumato", msg);

         goto end;
         }

         time_counter->setFromNumber32s(user_rec->_time_available    - user_rec->_time_done);
      traffic_counter->setFromNumber64s(user_rec->_traffic_available - user_rec->_traffic_done);
      }

   // redirect back to the gateway appending a signed ticket that will signal NoDog to unlock the firewall...
next:
   x = user_rec->getPolicy();

#ifdef USE_LIBSSL
   signed_data = UDES3::signData("\n"
#else
   signed_data.snprintf("\n"
#endif
   // "Action Permit\n"
   // "Mode Login\n"
      "Mac %v\n"
      "Timeout %v\n"
      "Traffic %v\n"
      "Token %v\n"
      "User %v\n"
      "Policy %v\n"
      "NoTraffic %v\n"
      "UserUploadRate %v\n"
      "UserDownloadRate %v\n"
      "Redirect http://%v/postlogin?%.*s\n",
      mac->rep, time_counter->rep, traffic_counter->rep, token->rep, uid->rep, x.rep,
      max_time_no_traffic->rep, user_DownloadRate->rep, user_UploadRate->rep, virtual_name->rep, U_HTTP_QUERY_TO_TRACE);

   if (gateway->c_char(0) == ':')
      {
      UString ap_port = *gateway;

      *gateway = *ap_address + ap_port;
      }

   user_rec->writeToLOG(auth_domain->c_str()); // NB: writeToLOG() change time_counter and traffic_counter...

   USSIPlugIn::setAlternativeRedirect("http://%v/ticket?ticket=%v", gateway->rep, signed_data.rep);

end:
   if (vec.empty() == false) askNodogToLogoutUser(vec, true);
}

static void GET_logout_page()
{
   U_TRACE(5, "::GET_logout_page()")

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("logout_page.tmpl")), 0, false,
                                     title_default->data(), 0, 0,
                                     logout_url->rep);
}

static void GET_password()
{
   U_TRACE(5, "::GET_password()")

   /*
   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("servizio_non_disponibile.tmpl")), 0, false,
                                     title_default->data(), 0, 0,
                                     0);
   */

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("password.tmpl")), 0, false,
                         "Modifica password utente",
                         "<link type=\"text/css\" href=\"css/livevalidation.css\" rel=\"stylesheet\" />"
                         "<script type=\"text/javascript\" src=\"js/livevalidation_standalone.compressed.js\"></script>", 0,
                         password_url->rep);
}

static void GET_polling_attivazione()
{
   U_TRACE(5, "::GET_polling_attivazione()")

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void GET_polling_password()
{
   U_TRACE(5, "::GET_polling_password()")

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void GET_postlogin()
{
   U_TRACE(5, "::GET_postlogin()")

   if (UServer_Base::bssl)
      {
      USSIPlugIn::setAlternativeRedirect("http://%v/postlogin?%.*s", virtual_name->rep, U_HTTP_QUERY_TO_TRACE);

      return;
      }

   uint32_t n = UHTTP::processForm();

   U_INTERNAL_DUMP("n = %u", n)

   if (n == 2)
      {
      // ----------------------------
      // $1 -> uid
      // ----------------------------

      UHTTP::getFormValue(*uid, U_CONSTANT_TO_PARAM("uid"), 0, 1, 2);

      if (uid->empty()) USSIPlugIn::setBadRequest();
      else
         {
         *uid = UStringExt::trim(*uid);

         USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("logout_popup.tmpl")), 0, false,
                                           "Logout popup", 0, 0,
                                           logout_url->rep, uid->rep, uid->rep);
         }

      return;
      }

   // -----------------------------------------------------------------------------------
   // NB: come back from the gateway (NoDog) after the ticket, the params CANNOT be empty
   // -----------------------------------------------------------------------------------
   // $1 -> mac
   // $2 -> ip
   // $3 -> redirect
   // ========================
   // => uid
   // => policy
   // => auth_domain
   // => max_time
   // => max_traffic
   // => UserDownloadRate
   // => UserUploadRate
   // => redir_to
   // ========================
   // $4 -> gateway
   // $5 -> timeout
   // $6 -> token
   // $7 -> ap (with localization => '@')
   // -----------------------------------------------------------------------------------

   if (n != 14)
      {
   // U_LOGGER("*** NUM ARGS(%u) DIFFERENT FROM EXPECTED(14) ***", n);

error:
      loginWithProblem();

      return;
      }

   if (checkLoginRequest(n, 14, 0, false) == false ||
       checkLoginValidate(false)          == false)
      {
      goto error;
      }

   U_INTERNAL_DUMP("redir = %V", redir->rep)

   U_INTERNAL_ASSERT(*redir)
   U_ASSERT_MINOR(redir->size(), 2048)

   checkUserID();

   if (checkIfUserConnected())
      {
      if (user_rec->_auth_domain == *account_auth)
         {
         U_LOGGER("*** GROUP ACCOUNT: UID(%v) IP(%v) MAC(%v) AP(%v) REDIR(%v) ***", uid->rep, ip->rep, mac->rep, ap->rep, redir->rep);
         }
      }
   else if (user_exist == false)
      {
      U_LOGGER("*** POSTLOGIN ERROR: UID(%v) IP(%v) MAC(%v) ADDRESS(%v) REDIR(%v) ***", uid->rep, ip->rep, mac->rep, ap->rep, redir->rep);

      goto error;
      }

   // Sanity check for user id (the param of url come from login_validate)...

   if (*ip         != user_rec->_ip  ||
       *mac        != user_rec->_mac ||
       *ap_address != user_rec->nodog)
      {
      UString label;

      if (user_rec->connected) label = user_rec->getLabelAP();

      U_LOGGER("*** POSTLOGIN ERROR: UID(%v) IP(%v=>%v) MAC(%v=>%v) ADDRESS(%v=>%v@%v) AUTH_DOMAIN(%v) REDIR(%v) ***", uid->rep,
                     user_rec->_ip.rep, ip->rep,
                     user_rec->_mac.rep, mac->rep,
                     label.rep, user_rec->nodog.rep, ap->rep,
                     user_rec->_auth_domain.rep, redir->rep);

      goto error;
      }

   if (user_rec->setNodogReference() == false) goto error;

   // NB: send as response the message of waiting to redirect to original site...

   bool ball         = false,
        bfirenzecard = false;

   if (user_rec->_auth_domain.find("firenzecard") != U_NOT_FOUND)
      {
      bfirenzecard = true;

      setAccessPointReference(U_STRING_TO_PARAM(*ap_address));

      redir->setBuffer(U_CAPACITY);

      redir->snprintf(FIRENZECARD_REDIR, ap_ref->rep);
      }
   else
      {
      if (user_rec->_auth_domain.find("all") != U_NOT_FOUND) ball = true;
      }

   // NB: we may be a different process from what it has updated so that we need to read the record...

   (void) db_nodog->getDataStorage(*ap_address);

   WiAuthAccessPoint* ap_rec = nodog_rec->vec_access_point[user_rec->_index_access_point];

                          ap_rec->num_login++;
        if (ball)         ap_rec->num_auth_domain_ALL++;
   else if (bfirenzecard) ap_rec->num_auth_domain_FICARD++;

   (void) db_nodog->putDataStorage();

   user_rec->setConnected(true);

   U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

   (void) db_user->putDataStorage(*uid);

   // NB: we may be a different process from what it has updated so that we need to read the record...

          UHTTP::db_session->setPointerToDataStorage(data_rec);
   (void) UHTTP::db_session->getDataStorage();

   data_rec->utenti_connessi_giornaliero_globale++;

   (void) UHTTP::db_session->putDataStorage();

   user_rec->writeToLOG("LOGIN");

   // NB: we can have problems without encoding if uid is the mac address...

   UString uid_encoded((uid->size() + 24U) * 3U);

   Url::encode(*uid, uid_encoded);

   U_INTERNAL_DUMP("uid_encoded = %V", uid_encoded.rep)

   UString buffer(100U + uid_encoded.size() + redir->size());

   buffer.snprintf("onload=\"doOnLoad('postlogin?uid=%v','%v')\"", uid_encoded.rep, redir->rep); 

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("postlogin.tmpl")), 0, false,
                                     title_default->data(), "<script type=\"text/javascript\" src=\"js/logout_popup.js\"></script>", buffer.data(),
                                     uid->rep, redir->rep, redir->rep);
}

static void GET_recovery()
{
   U_TRACE(5, "::GET_recovery()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      // ----------------------------
      // $1 -> uid
      // ----------------------------

      UHTTP::getFormValue(*uid, U_STRING_TO_PARAM(*UString::str_user), 0, 1, UHTTP::processForm());

      if (checkIfUserConnected() &&
          user_rec->setNodogReference())
         {
         *ap_address = user_rec->nodog;

         (void) askNodogToLogoutUser(user_rec->_ip, user_rec->_mac, true);
         }

      UString user = WiAuthUser::get_UserName();

      ULog::log(file_RECOVERY->getFd(), "%v \"%v\"", uid->rep, user.rep);

      U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

      int result = db_user->remove(*uid);

      if (result) U_SRV_LOG("WARNING: remove of user %v on db WiAuthUser failed with error %d", uid->rep, result);

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_registrazione()
{
   U_TRACE(5, "::GET_registrazione()")

   // $1 -> ap (with localization => '@')

   UString tutela_dati = cache->getContent(U_CONSTANT_TO_PARAM("tutela_dati.txt"));

   U_INTERNAL_ASSERT(tutela_dati.isNullTerminated())

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("registrazione.tmpl")), 0, false,
                         "Registrazione utente",
                         "<link type=\"text/css\" href=\"css/livevalidation.css\" rel=\"stylesheet\" />"
                         "<script type=\"text/javascript\" src=\"js/livevalidation_standalone.compressed.js\"></script>", 0,
                         registrazione_url->rep, tutela_dati.rep);
}

static void GET_reset_counter_ap()
{
   U_TRACE(5, "::GET_reset_counter_ap()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      db_nodog->callForAllEntry(resetCounterAP);

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_reset_policy()
{
   U_TRACE(5, "::GET_reset_policy()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      // NB: we may be a different process from what it has updated so that we need to read the record...

             UHTTP::db_session->setPointerToDataStorage(data_rec);
      (void) UHTTP::db_session->getDataStorage();

      // ............
      // "2013/09/08","2630","67","57244",
      // "2013/09/09","2630","67","57244",
      // ............

      uint32_t divisor = (data_rec->utenti_connessi_giornaliero_globale
                              ? (data_rec->utenti_connessi_giornaliero_globale * 60U)
                              : 1); // to avoid Signal SIGFPE (8, Floating point exception)

      u__printf(file_UTILIZZO->getFd(),
                "\"%#6D\",\"%u\",\"%u\",\"%u\",",
                u_now->tv_sec - (60L * 60L),
                data_rec->utenti_connessi_giornaliero_globale,
                data_rec->tempo_permanenza_utenti_giornaliero_globale / divisor,
                (uint32_t)(data_rec->traffico_generato_giornaliero_globale / (1024ULL * 1024ULL * 1024ULL)));

      data_rec->reset();

      (void) UHTTP::db_session->putDataStorage();

      db_nodog->callForAllEntry(resetCounterAP);

      // reset policy

      WiAuthUser::loadPolicy(*policy_daily);

      db_user->callForAllEntry(checkIfUserHasPolicyDaily);

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_resync()
{
   U_TRACE(5, "::GET_resync()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      num_users_delete = 0;

      db_user->callForAllEntry(checkStatusUser, (vPF)U_NOT_FOUND);

      U_LOGGER("*** WE HAVE DELETED %u USERS ***", num_users_delete);

      db_nodog->callForAllEntry(getAccessPointUP, (vPF)checkStatusUserOnNodog);

      num_ap_delete = 0;

      db_nodog->callForAllEntry(checkStatusNodog, (vPF)U_NOT_FOUND);

      U_LOGGER("*** WE HAVE REMOVED %u ACCESS POINT ***", num_ap_delete);

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_start_ap()
{
   U_TRACE(5, "::GET_start_ap()")

   // $1 -> ap (with localization => '@')
   // $2 -> public address to contact the access point
   // $3 -> pid (0 => start)

   if (setAccessPoint(true))
      {
      UString pid(100U);

      UHTTP::getFormValue(pid, U_CONSTANT_TO_PARAM("pid"), 0, 5, UHTTP::form_name_value->size());

      bool bstart = (pid.strtol() == 0);

      nodog_rec->setStatus(bstart ? 3 : 0); // startup

      U_LOGGER("%v:%v %s", ap_address->rep, ap_hostname->rep, bstart ? "started" : "*** NODOG CRASHED ***");

      db_user->callForAllEntry(quitUserConnected);
      }

   USSIPlugIn::setAlternativeResponse(*allowed_web_hosts);
}

static void GET_stato_utente()
{
   U_TRACE(5, "::GET_stato_utente()")

   UString label, x, user, result;

   uid->clear();

   if (checkIfUserConnected() &&
       user_rec->setNodogReference())
      {
      *ap_address = user_rec->nodog;

      result = nodog_rec->sendRequestToNodog("status?ip=%.*s", U_CLIENT_ADDRESS_TO_TRACE);

      if (result.empty() ||
          U_IS_HTTP_ERROR(U_http_info.nResponseCode))
         {
         user_rec->writeToLOG("QUIT");

         user_rec->setConnected(false);

         U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

         (void) db_user->putDataStorage(*uid);
error:
         USSIPlugIn::setMessagePage(*message_page_template, "Utente non connesso", "");

         return;
         }
      }
   else
      {
      if (user_exist == false ||
          user_rec->_auth_domain != *account_auth)
         {
         goto error;
         }
      }

   if (user_rec->connected == false)
      {
      user_rec->writeToLOG("RESYNC");

      user_rec->setConnected(true);

      U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

      (void) db_user->putDataStorage(*uid);
      }

   x    = user_rec->getAP(label);
   user = getUserName();

#ifdef USE_LIBZ
   if (UStringExt::isGzip(result)) result = UStringExt::gunzip(result);
#endif

   USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("stato_utente.tmpl")), 0, false,
                                     "Stato utente", 0, 0,
                                     user.rep, uid->rep, x.rep, result.rep);
}

static void GET_status_ap()
{
   U_TRACE(5, "::GET_status_ap()")

   // $1 -> ap (with localization => '@')
   // $2 -> public address to contact the access point

   if (virtual_name->equal(U_HTTP_VHOST_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      UString result;

      if (setAccessPoint(true)) result = nodog_rec->sendRequestToNodog("status?label=%v", ap_label->rep);

      if (result.empty() ||
          U_IS_HTTP_ERROR(U_http_info.nResponseCode))
         {
         USSIPlugIn::setMessagePage(*message_page_template, "Servizio non disponibile",
                                                            "Servizio non disponibile (access point non contattabile). "
                                                            "Riprovare piu' tardi");
         }
      else
         {
         USSIPlugIn::setAlternativeResponse(result);
         }
      }
}

/*
static void GET_unifi()
{
   U_TRACE(5, "::GET_unifi()")

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("unifi_page.tmpl")), 0, false,
                                     title_default->data(), 0, 0,
                                     url_banner_ap->data(), help_url->data(), wallet_url->data(),
                                     "unifi", "/unifi_login_request", url_banner_comune->data());
}

static void GET_unifi_login_request()
{
   U_TRACE(5, "::GET_unifi_login_request()")

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("unifi_login_request.tmpl")), 0, false,
                                     title_default->data(), 0, 0,
                                     0);
}
*/

static void GET_webif_ap()
{
   U_TRACE(5, "::GET_webif_ap()")

   // $1 -> ap (with localization => '@')
   // $2 -> public address to contact the access point

   if (virtual_name->equal(U_HTTP_VHOST_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      if (setAccessPoint(true) &&
          nodog_rec->status <= 1)
         {
         if (UModProxyService::setServerAddress(*dir_server_address, U_STRING_TO_PARAM(*ap_address)))
            {
            USSIPlugIn::setAlternativeRedirect("http://%v/cgi-bin/webif/status-basic.sh?cat=Status", virtual_name->rep);
            }
         else
            {
            USSIPlugIn::setMessagePage(*message_page_template, "Errore", "Errore interno - contattare amministratore");
            }
         }
      else
         {
         USSIPlugIn::setMessagePage(*message_page_template, "Servizio non disponibile",
                                                            "Servizio non disponibile (access point non contattabile). "
                                                            "Riprovare piu' tardi");
         }
      }
}

static void POST_admin_edit_ap()
{
   U_TRACE(5, "::POST_admin_edit_ap()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      // $1 -> ap_label
      // $2 -> ap_address
      // $3 -> ap_hostname
      // $4 -> ap_mac_mask
      // $5 -> ap_group_account
      // $6 -> ap_up
      // $7 -> ap_consume
      // $8 -> submit

      uint32_t end = UHTTP::processForm();

      if (end)
         {
         UString reboot, consume, mac_mask, group_account_mask;

         UHTTP::getFormValue(*ap_label,          U_CONSTANT_TO_PARAM("ap_label"),         0, 1, end);
         UHTTP::getFormValue(*ap_address,        U_CONSTANT_TO_PARAM("ap_address"),       0, 3, end);
         UHTTP::getFormValue(*ap_hostname,       U_CONSTANT_TO_PARAM("ap_hostname"),      0, 5, end);
         UHTTP::getFormValue(mac_mask,           U_CONSTANT_TO_PARAM("ap_mac_mask"),      0, 7, end);
         UHTTP::getFormValue(group_account_mask, U_CONSTANT_TO_PARAM("ap_group_account"), 0, 9, end);

         UHTTP::getFormValue(reboot,  U_CONSTANT_TO_PARAM("ap_up"));
         UHTTP::getFormValue(consume, U_CONSTANT_TO_PARAM("ap_consume"));

         U_INTERNAL_DUMP("ap_address = %V ap_hostname = %V ap_label = %V", ap_address->rep, ap_hostname->rep, ap_label->rep)

         if (u_isIPv4Addr(U_STRING_TO_PARAM(*ap_address)) == false)
            {
            U_LOGGER("*** ADDRESS AP(%v) NOT VALID ***", ap_address->rep);

            USSIPlugIn::setBadRequest();
            }
         else if (u_isHostName(U_STRING_TO_PARAM(*ap_hostname)) == false)
            {
            U_LOGGER("*** AP HOSTNAME(%v) NOT VALID ***", ap_hostname->rep);

            USSIPlugIn::setBadRequest();
            }
         else if (ap_label->empty())
            {
            U_LOGGER("*** AP LABEL EMPTY ***", 0);

            USSIPlugIn::setBadRequest();
            }
         else 
            {
            nodog_rec->editRecord(reboot, consume.empty(), mac_mask, group_account_mask);
            }
         }

      USSIPlugIn::setAlternativeRedirect("https://%v/admin_status_nodog", ip_server->rep);
      }
}

static void POST_admin_view_user()
{
   U_TRACE(5, "::POST_admin_view_user()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      // $1 -> uid

      uint32_t end = UHTTP::processForm();

      if (end)
         {
         UHTTP::getFormValue(*uid, U_CONSTANT_TO_PARAM("uid"), 0, 1, end);

         if (askToLDAP(0, "utente non registrato", "utente non registrato", "ldapsearch -LLL -b %v %v waLogin=%v", wiauth_card_basedn->rep, ldap_card_param->rep, uid->rep) <= 0)
            {
            return;
            }

         UString WA_POLICY   = (*table)["waPolicy"],   // waPolicy: DAILY
                 WA_USEDBY   = (*table)["waUsedBy"],   // waUsedBy: 3343793489
                 WA_REVOKED  = (*table)["waRevoked"],  // waRevoked: FALSE
                 WA_VALIDITY = (*table)["waValidity"], // waValidity: 0
                 WA_PASSWORD = (*table)["waPassword"], // waPassword: {MD5}M7Bt9PlxMhHxcVd2HCVGcg==
                 WA_NOTAFTER = (*table)["waNotAfter"]; // waNotAfter: 20371231235959Z

         table->clear();

         UString not_after, tmp1, tmp2, tmp3 = U_STRING_FROM_CONSTANT("yes"), tmp4 = U_STRING_FROM_CONSTANT("no"), tmp5 = U_STRING_FROM_CONSTANT("Non disponibile");

         if (WA_NOTAFTER.empty()) not_after = tmp5;
         else
            {
            not_after.setBuffer(100U);

            const char* ptr = WA_NOTAFTER.data();

            not_after.snprintf("%.2s/%.2s/%.4s - %.2s:%.2s", ptr+6, ptr+4, ptr, ptr+8, ptr+10);
            }

         if (WA_USEDBY.empty() == false)
            {
            tmp1 = U_STRING_FROM_CONSTANT("green");
            tmp2 = tmp3;
            }
         else
            {
            tmp1 = U_STRING_FROM_CONSTANT("red");
            tmp2 = U_STRING_FROM_CONSTANT("NO");
            }

         UStringRep* p3;
         UStringRep* p4;
         UStringRep* p5;
         UStringRep* p6;
         UStringRep* p7 = tmp4.rep;

         long last_modified;
         bool connected = checkIfUserConnected();
         UString user = getUserName(); // NB: must be after checkIfUserConnected()...

         if (user_exist)
            {
            last_modified = user_rec->last_modified + u_now_adjust;

            user_rec->getCounter();

            p3 =    time_counter->rep;
            p4 = traffic_counter->rep;

            user_rec->getConsumed();

            p5 =    time_consumed->rep;
            p6 = traffic_consumed->rep;

            if (connected) p7 = tmp3.rep;

            if (WA_POLICY.empty()) WA_POLICY = user_rec->_policy;
            }
         else
            {
            last_modified = 0;

            p3 = p4 = p5 = p6 = tmp5.rep;
            }

         USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_view_user.tmpl")), 0, false,
                                           "Visualizzazione dati utente", 0, 0,
                                           user.rep, uid->rep,
                                           p3, p4,
                                           WA_PASSWORD.rep,
                                           not_after.rep,
                                           WA_VALIDITY.rep,
                                           WA_REVOKED.rep,
                                           WA_POLICY.rep,
                                           tmp1.rep, tmp2.rep,
                                           UStringExt::substr_count(file_RECOVERY->_getContent(), *uid),
                                           p7, p5, p6, last_modified);
         }
      }
}

static void POST_admin_execute_recovery()
{
   U_TRACE(5, "::POST_admin_execute_recovery()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void POST_admin_recovery()
{
   U_TRACE(5, "::POST_admin_recovery()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void POST_LoginRequest(bool idp)
{
   U_TRACE(5, "::POST_LoginRequest(%b)", idp)

   // ---------------------------------------------------------------------------------------------------
   // *** this params CAN be empty ***
   // ---------------------------------------------------------------------------------------------------
   // $1  -> mac
   // $2  -> ip
   // $3  -> redirect
   // $4  -> gateway
   // $5  -> timeout
   // $6  -> token
   // $7  -> ap (with localization => '@')
   // ---------------------------------------------------------------------------------------------------
   // *** this params CANNOT be empty ***
   // ---------------------------------------------------------------------------------------------------
   // $8  -> realm
   // $9  -> redir_to
   // $10 -> uid
   // $11 -> password
   // ---------------------------------------------------------------------------------------------------
   // $12 -> submit.x 
   // $13 -> submit.y - if it came from main.bash...
   // $14 -> submit   - if it came from some mobile device...
   // ---------------------------------------------------------------------------------------------------
 
   if (checkLoginRequest(0, 26, 4, true) == false)
      {
      loginWithProblem();

      return;
      }

   UString realm;

   UHTTP::getFormValue(realm, U_CONSTANT_TO_PARAM("realm"), 0, 15, 24);

   U_INTERNAL_DUMP("realm = %V", realm.rep)

   if (realm.empty())
      {
      USSIPlugIn::setMessagePage(*message_page_template, "Errore", "Errore Autorizzazione - dominio vuoto");

      return;
      }

   UString password;

   UHTTP::getFormValue(*uid,     U_CONSTANT_TO_PARAM("uid"),  0, 19, 24);
   UHTTP::getFormValue(password, U_CONSTANT_TO_PARAM("pass"), 0, 21, 24);

   if (    uid->empty() ||
       password.empty())
      {
      USSIPlugIn::setMessagePage(*message_page_template, "Impostare utente e/o password", "Impostare utente e/o password"); 

      return;
      }

   UHTTP::getFormValue(*redir, U_CONSTANT_TO_PARAM("redir_to"), 0, 17, 24);

   if (realm.equal(U_CONSTANT_TO_PARAM("all")) == false)
      {
      if (fmt_auth_cmd->empty() ||
          runAuthCmd(password.c_str(), realm.c_str()) == false)
         {
         return;
         }

      *policy = *(realm.equal(U_CONSTANT_TO_PARAM("firenzecard"))
                     ? policy_traffic
                     : policy_daily);

      UString x    = UStringExt::trim(*output);
      uint32_t pos = x.findWhiteSpace();

      if (pos == U_NOT_FOUND) pos = x.size();

      auth_domain->setBuffer(80U);
      auth_domain->snprintf("AUTH_%.*s", pos, x.data());

      user_DownloadRate->replace('0');
      user_UploadRate->replace('0');
      }
   else
      {
      if (askToLDAP(0, 0, 0, "ldapsearch -LLL -b %v %v waLogin=%v", wiauth_card_basedn->rep, ldap_card_param->rep, uid->rep) == -1)
         {
         return;
         }

      UString password_on_ldap = (*table)["waPassword"]; // waPassword: {MD5}ciwjVccK0u68vqupEXFukQ==

      if (strncmp(password_on_ldap.data(), U_CONSTANT_TO_PARAM("{MD5}")) != 0)
         {
         // NB: realm is 'all' and we not have a MD5 password so we check credential by AUTH command...

         if (fmt_auth_cmd->empty() ||
             runAuthCmd(password.c_str(), realm.c_str()) == false)
            {
            return;
            }

         *policy = *policy_daily;

         UString x    = UStringExt::trim(*output);
         uint32_t pos = x.findWhiteSpace();

         if (pos != U_NOT_FOUND) (void) realm.assign(x.data(), pos);
            {
            pos   = x.size();
            realm = x;
            }

         auth_domain->setBuffer(80U);
         auth_domain->snprintf("AUTH_%v", realm.rep);

         user_DownloadRate->replace('0');
         user_UploadRate->replace('0');
         }
      else
         {
         UString passwd(33U);

         // Check 1: Wrong user and/or password

         UServices::generateDigest(U_HASH_MD5, 0, (unsigned char*)U_STRING_TO_PARAM(password), passwd, true);

         if (strncmp(password_on_ldap.c_pointer(U_CONSTANT_SIZE("{MD5}")), U_STRING_TO_PARAM(passwd)))
            {
            char msg[4096];

            (void) u__snprintf(msg, sizeof(msg),
                     "Credenziali errate!<br>"
                     "<a href=\"javascript:history.go(-1)\">Indietro</a>", 0);

            USSIPlugIn::setMessagePage(*message_page_template, "Utente e/o Password errato/i", msg);

            return;
            }

         // Check 2: Activation required

         if ((*table)["waUsedBy"].empty()) // waUsedBy: 3343793489
            {
            USSIPlugIn::setMessagePage(*message_page_template, "Attivazione non effettuata", "Per utilizzare il servizio e' richiesta l'attivazione");

            return;
            }

         // Check 3: Card revoked

         if ((*table)["waRevoked"] != "FALSE") // waRevoked: FALSE
            {
            USSIPlugIn::setMessagePage(*message_page_template, "Carta revocata", "La tua carta e' revocata!");

            return;
            }

         UString NOT_AFTER = (*table)["waNotAfter"]; // waNotAfter: 20371231235959Z

         if (NOT_AFTER)
            {
            // Check 4: Expired validity

            if (UTimeDate::getSecondFromTime(NOT_AFTER.data(), true, "%4u%2u%2u%2u%2u%2uZ") <= u_now->tv_sec)
               {
               USSIPlugIn::setMessagePage(*message_page_template, "Validita' scaduta", "La tua validita' e' scaduta!");

               return;
               }

            *auth_domain = U_STRING_FROM_CONSTANT("PASS_AUTH");
            }
         else
            {
            *auth_domain = U_STRING_FROM_CONSTANT("FIRST_PASS_AUTH");

            // Update card with a new generated waNotAfter

            UString DN       = (*table)["dn"],         // dn: waCid=80e415bc-4be0-4385-85ee-970aa1f52ef6,ou=cards,o=unwired-portal
                    VALIDITY = (*table)["waValidity"]; // waValidity: 0

            NOT_AFTER = (VALIDITY == "0" ? U_STRING_FROM_CONSTANT("20371231235959Z")
                                         : UTimeDate::strftime("%Y%m%d%H%M%SZ", u_now->tv_sec + (VALIDITY.strtol() * U_ONE_DAY_IN_SECOND)));

            UString input(U_CAPACITY);

            input.snprintf("dn: %v\n"
                           "changetype: modify\n"
                           "add: waNotAfter\n"
                           "waNotAfter: %v\n"
                           "-",
                           DN.rep, NOT_AFTER.rep);

            if (askToLDAP(&input, "Errore", "LDAP error", "ldapmodify -c %v", ldap_card_param->rep) <= 0) return;
            }

         *policy = (*table)["waPolicy"]; // waPolicy: DAILY

         user_DownloadRate->replace('0');
         user_UploadRate->replace('0');
         }
      }

   if (idp == false                                   &&
       *ip                                            &&
        ip->equal(U_CLIENT_ADDRESS_TO_PARAM) == false &&
       UClientImage_Base::isAllowed(*vallow_IP_request) == false) // 172.0.0.0/8, ...
      {
      U_LOGGER("*** PARAM IP(%v) FROM AP(%v) IS DIFFERENT FROM CLIENT ADDRESS(%.*s) - REALM(%v) UID(%v) ***", ip->rep, ap->rep, U_CLIENT_ADDRESS_TO_TRACE, realm.rep, uid->rep);
      }

   if (UServer_Base::bssl)
      {
      bool bset = true;

      if (idp == false)
         {
         UString persistent_cookie;

         UHTTP::getFormValue(persistent_cookie, U_CONSTANT_TO_PARAM("PersistentCookie"));

         bset = persistent_cookie.equal(U_CONSTANT_TO_PARAM("yes"));
         }

      if (bset)
         {
         UString uid_save = *uid;

         if (getCookie(0,0) == false ||
             *uid != uid_save)
            {
            UString input(U_CAPACITY);
            unsigned char key[16], hexdump[33];

            UServices::generateKey(key, hexdump);

            input.snprintf("dn: waCookieId=%s, o=sessions\n"
                           "waCookieId: %s\n"
                           "objectClass: waSession\n"
                           "waFederatedUserId: %v@%v\n", // Ex: 3343793489@all
                           hexdump, hexdump, uid->rep, realm.rep);

            if (askToLDAP(&input, "Errore", "LDAP error", "ldapadd -c %v", ldap_session_param->rep) == 1) setCookie((const char*)hexdump);
            }

         *uid = uid_save;
         }

      if (idp)
         {
         sendRedirect();

         return;
         }
      }

   sendLoginValidate(); // NB: in questo modo l'utente ripassa dal firewall e NoDog lo rimanda da noi (login_validate) con i dati rinnovati...
}

static void POST_login_request()
{
   U_TRACE(5, "::POST_login_request()")

   if (UClientImage_Base::request_uri->equal(U_CONSTANT_TO_PARAM("/login_request/logout")) == false) POST_LoginRequest(false);
   else
      {
      // manage logout request

      UVector<UString> vec;
      bool bpopup, bcheck = false;
      uint32_t end = UHTTP::processForm();
      UString uid_cookie = (uid->clear(), (void)getCookie(0,0), *uid);

      // ----------------------------
      // $1 -> uid
      // $2 -> submit
      // $3 -> PersistentCookie
      // ----------------------------

      UHTTP::getFormValue(*uid, U_CONSTANT_TO_PARAM("uid"), 0, 1, end);

      if (*uid)
         {
         bpopup = true;

         if (uid_cookie)
            {
            if (uid->equal(uid_cookie) == false)
               {
               U_LOGGER("*** UID DIFFER(%v=>%v) ***", uid->rep, uid_cookie.rep);
               }
            }
         }
      else
         {
         bpopup = false;

         *uid = uid_cookie;
         }

      if (checkIfUserConnected() &&
          user_rec->setNodogReference())
         {
         if (user_rec->_auth_domain != *account_auth)
            {
            bcheck = true;

            vec.push_back(user_rec->nodog);
            vec.push_back(user_rec->_ip);
            vec.push_back(user_rec->_mac);
            }
         }
      else
         {
         if (user_exist == false)
            {
            USSIPlugIn::setMessagePage(*message_page_template, "Utente non connesso", "");

            return;
            }

         if (user_rec->_auth_domain != *account_auth)
            {
            vec.push_back(user_rec->nodog);
            vec.push_back(U_CLIENT_ADDRESS_TO_PARAM);
            vec.push_back(*UString::str_without_mac);
            }
         }

      if (uid_cookie)
         {
         UString delete_cookie;

         UHTTP::getFormValue(delete_cookie, U_CONSTANT_TO_PARAM("PersistentCookie"));

         if (delete_cookie.equal(U_CONSTANT_TO_PARAM("yes"))) setCookie(0);
         }

      user_rec->getCounter();

      if (bpopup)
         {
         ptr1 = "<script type=\"text/javascript\" src=\"js/logout_popup.js\"></script>";
         ptr2 = "<br>\n"
                "<form method=\"post\" action=\"\">\n"
                "<input type=\"button\" value=\"Close\" onclick=\"CloseItOnClick()\" style=\"display:block;height:3em;width:10em;margin:2em auto;\">\n"
                "</form>";
         }
      else
         {
         ptr1 = "";
         ptr2 = "";
         }

      USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("ringraziamenti.tmpl")), 0, true,
                                        title_default->data(), ptr1, 0,
                                        uid->rep,
                                        time_counter->rep, traffic_counter->rep, ptr2);

      if (vec.empty() == false) askNodogToLogoutUser(vec, bcheck);
      }
}

static void POST_login_request_IdP()
{
   U_TRACE(5, "::POST_login_request_IdP()")

   POST_LoginRequest(true);
}

static void POST_password()
{
   U_TRACE(5, "::POST_password()")

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   // $1 -> cellulare_prefisso
   // $2 -> telefono_cellulare
   // $3 -> password
   // $4 -> password_conferma
   // $5 -> submit

   uint32_t end = UHTTP::processForm();

   if (end)
      {
      UString cellulare_prefisso, telefono_cellulare, password, password_conferma;

      UHTTP::getFormValue(cellulare_prefisso,  1);
      UHTTP::getFormValue(telefono_cellulare,  3);
      UHTTP::getFormValue(password,            5);
      UHTTP::getFormValue(password_conferma,   7);

   if (password != password_conferma) USSIPlugIn::setMessagePage(*message_page_template, "Conferma Password errata", "Conferma Password errata");
   else
      {
      ....

      USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("post_password.tmpl")), 0, true,
                                        "Modofica password effettuata", 0, 0,
                                        caller_id->data(), password.c_str(), "polling_password", caller_id->data(), password.c_str());
      }
   }
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void POST_registrazione()
{
   U_TRACE(5, "::POST_registrazione()")

   /*
   ---------------------------------------------------------------------------------------------------------------
   NB: we can manage this request with the main.bash script...
   ---------------------------------------------------------------------------------------------------------------
   // $1  -> nome
   // $2  -> cognome
   // $3  -> luogo_di_nascita
   // $4  -> data_di_nascita
   // $5  -> email
   // $6  -> cellulare_prefisso
   // $7  -> telefono_cellulare
   // $8  -> password
   // $9  -> password_conferma
   // $10 -> submit

   uint32_t end = UHTTP::processForm();

   if (end)
      {
      UString nome, cognome, luogo_di_nascita, data_di_nascita, email,
              cellulare_prefisso, telefono_cellulare, password, password_conferma;

      UHTTP::getFormValue(nome,                 1);
      UHTTP::getFormValue(cognome,              3);
      UHTTP::getFormValue(luogo_di_nascita,     5);
      UHTTP::getFormValue(data_di_nascita,      7);
      UHTTP::getFormValue(email,                9);
      UHTTP::getFormValue(cellulare_prefisso,  11);
      UHTTP::getFormValue(telefono_cellulare,  13);
      UHTTP::getFormValue(password,            15);
      UHTTP::getFormValue(password_conferma,   17);

      if (password != password_conferma) USSIPlugIn::setMessagePage(*message_page_template, "Conferma Password errata", "Conferma Password errata");
      else
         {
         ....

         USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("post_registrazione.tmpl")), 0, true,
                                           "Registrazione effettuata", 0, 0,
                                           caller_id->data(), password.c_str(), "polling_attivazione", caller_id->data(), password.c_str());
         }
      }
   ---------------------------------------------------------------------------------------------------------------
   */
}

static void POST_uploader()
{
   U_TRACE(5, "::POST_uploader()")

   // $1 -> path file uploaded

   if (UHTTP::processForm())
      {
      UString tmpfile(100U);

      UHTTP::getFormValue(tmpfile, 1);

      if (tmpfile)
         {
         UString content = UFile::contentOf(tmpfile);

         U_INTERNAL_ASSERT(tmpfile.isNullTerminated())

         (void) UFile::_unlink(tmpfile.data());

         if (content.size() > (2 * 1024))
            {
            UString dest(U_CAPACITY), basename = UStringExt::basename(tmpfile);

            dest.snprintf("%v/%v", historical_log_dir->rep, basename.rep);

            (void) UFile::writeTo(dest, content);
            }
         }
      }

   USSIPlugIn::setAlternativeResponse();
}
#endif
