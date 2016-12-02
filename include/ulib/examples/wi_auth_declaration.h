// wi_auth_declaration.h

#ifndef U_WI_AUTH_DECLARATION_H
#define U_WI_AUTH_DECLARATION_H 1

#include <ulib/net/server/plugin/mod_ssi.h>
#include <ulib/net/server/plugin/mod_nocat.h>
#include <ulib/net/server/plugin/mod_proxy_service.h>

static UString* ap;
static UString* ip;
static UString* ts;
static UString* uid;
static UString* mac;
static UString* realm;
static UString* pbody;
static UString* redir;
static UString* token;
static UString* label;
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
static UString* ap_address;
static UString* logout_url;
static UString* empty_list;
static UString* nodog_conf;
static UString* str_ffffff;
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
static UString* status_nodog_and_user_body_template;

static UString* buffer_srv;
static UString* buffer_data;
static UString* db_anagrafica;
static UString* db_filter_tavarnelle;

static UString* help_url;
static UString* wallet_url;
static UString* fmt_auth_cmd;
static UString* url_banner_ap;
static UString* url_banner_comune;

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

static UPing* sockp;
static UFile* file_LOG;
static UFile* file_INFO;
static UFile* file_WARNING;
static UFile* file_RECOVERY;
static UFile* file_UTILIZZO;

static UVector<UString>* vuid;
static UHashMap<UString>* table;
static UVector<UIPAllow*>* pvallow;
static UVector<UIPAllow*>* vallow_IP_user;
static UVector<UIPAllow*>* vallow_IP_request;
static UHttpClient<UTCPSocket>* client;
static UHashMap<UVectorUString>* table1;

static int today;
static UTimeDate* date;
static UString*  yearName;
static UString* monthName;
static UString*  weekName;

static uint32_t    time_available_daily,   time_available_flat;
static uint64_t traffic_available_daily, traffic_available_flat;

static bool     user_exist, brenew, isIP, isMAC, ap_address_trust, db_user_filter_tavarnelle, admin_status_nodog_and_user_as_csv, status_nodog_and_user_resync;
static uint32_t index_access_point, num_users, num_users_connected, num_users_connected_on_nodog, num_users_delete, num_ap_delete, num_ap, num_ap_noconsume, num_ap_up,
                num_ap_down, num_ap_open, num_ap_unreachable;

static const char* ptr1;
static const char* ptr2;
static const char* ptr3;
static const char* ptr4;

static uint32_t totale1, totale2, totale3;
static uint64_t totale4;

class WiAuthUser;
class WiAuthNodog;
class WiAuthDataStorage;
class WiAuthVirtualAccessPoint;

static WiAuthUser*              user_rec;
static WiAuthNodog*            nodog_rec;
static WiAuthDataStorage*       data_rec;
static WiAuthVirtualAccessPoint* vap_rec;

static URDBObjectHandler<UDataStorage*>* db_ap;
static URDBObjectHandler<UDataStorage*>* db_user;
static URDBObjectHandler<UDataStorage*>* db_nodog;

#define IP_UNIFI                "159.213.248.230"
#define IP_CASCINE              "159.213.248.232"
#define IP_UNIFI_TMP            "151.11.47.5"
#define NAMED_PIPE              "/tmp/wi_auth_db.op"
#define FIRENZECARD_REDIR       "http://wxfi.comune.fi.it/?ap=%v"
#define LOGIN_VALIDATE_REDIR    "http://www.google.com/login_validate?%v"
#define LOGIN_VALIDATE_REDIR_FI "http://151.11.47.4/login_validate?%v"

//#define IP_CASCINE              "151.11.47.3"
//#define FIRENZECARD_REDIR       "http://159.213.248.2/wxfi/?ap=%s"
//#define LOGIN_VALIDATE_REDIR_FI "http://151.11.45.77/cgi-bin/login_validata.cgi?%v"

#define U_LOGGER(fmt,args...) ULog::log(file_WARNING->getFd(), U_CONSTANT_TO_PARAM("%v: " fmt), UClientImage_Base::request_uri->rep , ##args)

class WiAuthDataStorage : public UDataStorage {
public:

   uint64_t       traffico_generato_giornaliero_globale;
   uint32_t         utenti_connessi_giornaliero_globale,
            tempo_permanenza_utenti_giornaliero_globale;

   // COSTRUTTORE

   WiAuthDataStorage() : UDataStorage(*UString::str_storage_keyid)
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

   // SERVICES

   void reset()
      {
      U_TRACE_NO_PARAM(5, "WiAuthDataStorage::reset()")

      U_CHECK_MEMORY

              utenti_connessi_giornaliero_globale =
      tempo_permanenza_utenti_giornaliero_globale = 0U;
            traffico_generato_giornaliero_globale = 0ULL;
      }

   // define method VIRTUAL of class UDataStorage

   virtual char* toBuffer()
      {
      U_TRACE_NO_PARAM(5, "WiAuthDataStorage::toBuffer()")

      U_CHECK_MEMORY

      buffer_data->snprintf(U_CONSTANT_TO_PARAM("%u %u %llu"),
                             utenti_connessi_giornaliero_globale,
                     tempo_permanenza_utenti_giornaliero_globale,
                           traffico_generato_giornaliero_globale);

      buffer_len = buffer_data->size();

      U_RETURN(buffer_data->data());
      }

#if defined(U_STDCPP_ENABLE)
   virtual void fromStream(istream& is)
      {
      U_TRACE(5, "WiAuthDataStorage::fromStream(%p)", &is)

      U_CHECK_MEMORY

      is >>         utenti_connessi_giornaliero_globale 
         >> tempo_permanenza_utenti_giornaliero_globale
         >>       traffico_generato_giornaliero_globale;
      }

# ifdef DEBUG
   const char* dump(bool breset) const { return ""; }
# endif
#endif
};

class WiAuthVirtualAccessPoint : public UDataStorage {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UString nodog;
   uint32_t _index_access_point, _num_users_connected;

   // COSTRUTTORE

   WiAuthVirtualAccessPoint()
      {
      U_TRACE_REGISTER_OBJECT(5, WiAuthVirtualAccessPoint, "", 0)

      _index_access_point  =
      _num_users_connected = 0;
      }

   ~WiAuthVirtualAccessPoint()
      {
      U_TRACE_UNREGISTER_OBJECT(5, WiAuthVirtualAccessPoint)

      db_ap->close();

      delete db_ap;
      }

   // SERVICES

   static UString getKey()
      {
      U_TRACE_NO_PARAM(5, "WiAuthVirtualAccessPoint::getKey()")

      U_INTERNAL_ASSERT(*ap_label)
      U_INTERNAL_ASSERT(*ap_address)

      UString key(32U);

      key.snprintf(U_CONSTANT_TO_PARAM("%v@%v"), ap_label->rep, ap_address->rep);

      U_RETURN_STRING(key);
      }

   static bool getRecord()
      {
      U_TRACE_NO_PARAM(5, "WiAuthVirtualAccessPoint::getRecord()")

      return db_ap->getDataStorage(getKey());
      }

   static void setRecord()
      {
      U_TRACE(5, "WiAuthVirtualAccessPoint::setRecord()")

      U_INTERNAL_ASSERT(*ap_label)
      U_INTERNAL_ASSERT(*ap_address)

      vap_rec->nodog                = *ap_address;
      vap_rec->_index_access_point  = index_access_point;

      (void) db_ap->insertDataStorage(getKey());
      }

   static void setIndexAccessPoint()
      {
      U_TRACE_NO_PARAM(5, "WiAuthVirtualAccessPoint::setIndexAccessPoint()")

      if (getRecord() == false)
         {
         U_LOGGER("*** AP(%v@%v) NOT FOUND ***", ap_label->rep, ap_address->rep);

         index_access_point = U_NOT_FOUND;
         }
      else
         {
         index_access_point = vap_rec->_index_access_point;

         U_ASSERT_EQUALS(*ap_address, vap_rec->nodog)
         }
      }

   // define method VIRTUAL of class UDataStorage

   virtual void clear()
      {
      U_TRACE_NO_PARAM(5, "WiAuthVirtualAccessPoint::clear()")

      U_CHECK_MEMORY

      nodog.clear();

      _index_access_point  =
      _num_users_connected = 0;
      }

   virtual char* toBuffer()
      {
      U_TRACE_NO_PARAM(5, "WiAuthVirtualAccessPoint::toBuffer()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(nodog)

      buffer_data->snprintf(U_CONSTANT_TO_PARAM("%u %v %u"), _index_access_point, nodog.rep, _num_users_connected);

      buffer_len = buffer_data->size();

      U_RETURN(buffer_data->data());
      }

#if defined(U_STDCPP_ENABLE)
   virtual void fromStream(istream& is)
      {
      U_TRACE(5, "WiAuthVirtualAccessPoint::fromStream(%p)", &is)

      U_CHECK_MEMORY

      is >> _index_access_point;

      is.get(); // skip ' '

      nodog.get(is);

      is >> _num_users_connected;

      U_INTERNAL_ASSERT(nodog)
      }

# ifdef DEBUG
   const char* dump(bool breset) const { return ""; }
# endif
#endif

   // DB QUERY FUNCTION

   static int checkForLabel(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthVirtualAccessPoint::checkForLabel(%V,%V)", key, data)

      U_INTERNAL_ASSERT_POINTER(data)

      const char* ptr = data->data();

      if (u__isspace(ptr[u_buffer_len]) &&
          memcmp(ptr, u_buffer, u_buffer_len) == 0)
         {
         U_INTERNAL_ASSERT_POINTER(key)

         ptr1 =
         ptr2 = key->data();

         while (*++ptr2 != '@') {}

         (void) label->assign(ptr1, ptr2-ptr1);

         U_RETURN(0); // stop
         }

      U_RETURN(1);
      }

private:
   WiAuthVirtualAccessPoint& operator=(const WiAuthVirtualAccessPoint&) { return *this; }
};

class WiAuthAccessPoint {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   uint64_t traffic_done;
   UString _label, mac_mask, group_account_mask;
   uint32_t num_login, _num_users_connected, num_auth_domain_ALL, num_auth_domain_FICARD;
   bool noconsume;

   // COSTRUTTORE

   WiAuthAccessPoint()
      {
      U_TRACE_REGISTER_OBJECT(5, WiAuthAccessPoint, "", 0)

      reset();

      noconsume = false;
      }

   WiAuthAccessPoint(const UString& lbl, bool _noconsume) : _label(lbl), noconsume(_noconsume)
      {
      U_TRACE_REGISTER_OBJECT(5, WiAuthAccessPoint, "%V,%b", lbl.rep, _noconsume)

      U_INTERNAL_ASSERT(_label)

      reset();
      }

   WiAuthAccessPoint(const WiAuthAccessPoint& _ap) : _label(_ap._label), mac_mask(_ap.mac_mask), group_account_mask(_ap.group_account_mask)
      {
      U_TRACE_REGISTER_OBJECT(5, WiAuthAccessPoint, "%p", &_ap)

      U_MEMORY_TEST_COPY(_ap)

      ((UString&)_ap._label).clear();
      ((UString&)_ap.mac_mask).clear();
      ((UString&)_ap.group_account_mask).clear();

      traffic_done           = _ap.traffic_done;
      num_login              = _ap.num_login;
      _num_users_connected   = _ap._num_users_connected;
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
      U_TRACE_NO_PARAM(5, "WiAuthAccessPoint::reset()")

      traffic_done = 0ULL;

                  num_login  =
      num_auth_domain_ALL    =
      num_auth_domain_FICARD = 0U;
      }

   void toBuffer()
      {
      U_TRACE_NO_PARAM(5, "WiAuthAccessPoint::toBuffer()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(_label)

      char buffer[1024];

      (void) buffer_data->append(buffer, u__snprintf(buffer, sizeof(buffer),
                        U_CONSTANT_TO_PARAM(" %c%u %v \"%v\" \"%v\""
                        " %u %u %u %llu"),
                        noconsume ? '-' : '+', _num_users_connected, _label.rep, group_account_mask.rep, mac_mask.rep,
                        num_login, num_auth_domain_ALL, num_auth_domain_FICARD, traffic_done));
      }

   bool isNumberOfUserConnectedNotAligned()
      {
      U_TRACE_NO_PARAM(5, "WiAuthAccessPoint::isNumberOfUserConnectedNotAligned()")

      U_INTERNAL_ASSERT_EQUALS(index_access_point, vap_rec->_index_access_point)

      int32_t df  = (_num_users_connected - vap_rec->_num_users_connected); 
      bool result = (df > 0);

      if (result == false)
         {
         result = (df > 25) ||
                  ((uint32_t)df > (_num_users_connected * 2));
         }

      if (result)
         {
         U_LOGGER("*** ON AP(%v@%v) THE NUMBER OF USER CONNECTED IS NOT ALIGNED (%u=>%u) ***", _label.rep, ap_address->rep, _num_users_connected, vap_rec->_num_users_connected);

         _num_users_connected = vap_rec->_num_users_connected;
         }

      vap_rec->_num_users_connected = 0;

      WiAuthVirtualAccessPoint::setRecord();

      U_RETURN(result);
      }

   // STREAMS

#if defined(U_STDCPP_ENABLE)
   void fromStream(istream& is)
      {
      U_TRACE(5, "WiAuthAccessPoint::fromStream(%p)", &is)

      U_CHECK_MEMORY

      streambuf* sb = is.rdbuf();

      int c = sb->sbumpc();

      U_INTERNAL_DUMP("c = %C", c)

      U_INTERNAL_ASSERT(c == '+' || c == '-')

      noconsume = (c == '-');

      is >> _num_users_connected;

      is.get(); // skip ' '

      _label.get(is);

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

# ifdef DEBUG
   const char* dump(bool breset) const { return ""; }
# endif
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
      last_info =
          since =
          start = u_now->tv_sec;
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
      U_TRACE_NO_PARAM(5, "WiAuthNodog::clear()")

      U_CHECK_MEMORY

      sz = 0;

              hostname.clear();
      vec_access_point.clear();
      }

   virtual char* toBuffer()
      {
      U_TRACE_NO_PARAM(5, "WiAuthNodog::toBuffer()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(hostname)
      U_ASSERT_EQUALS(sz, vec_access_point.size())

      buffer_data->snprintf(U_CONSTANT_TO_PARAM("%d %ld %ld %u %v %ld ["),
                            status,
                            last_info,
                            since,
                            port,
                            hostname.rep,
                            start);

      for (uint32_t i = 0; i < sz; ++i) vec_access_point[i]->toBuffer();

      (void) buffer_data->append(U_CONSTANT_TO_PARAM(" ]"));

      buffer_len = buffer_data->size();

      U_RETURN(buffer_data->data());
      }

#if defined(U_STDCPP_ENABLE)
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

# ifdef DEBUG
   const char* dump(bool breset) const { return ""; }
# endif
#endif

   // SERVICES

   bool findLabel()
      {
      U_TRACE_NO_PARAM(5, "WiAuthNodog::findLabel()")

      U_CHECK_MEMORY

      U_ASSERT_EQUALS(sz, vec_access_point.size())

      if (ap_label->empty()) U_RETURN(false);

      U_INTERNAL_DUMP("ap_label = %V", ap_label->rep)

      if (WiAuthVirtualAccessPoint::getRecord())
         {
         index_access_point = vap_rec->_index_access_point;

         if (index_access_point < vec_access_point.size() &&
             *ap_label == vec_access_point[index_access_point]->_label)
            {
            U_RETURN(true);
            }
         }

      for (index_access_point = 0; index_access_point < sz; ++index_access_point)
         {
         if (*ap_label == vec_access_point[index_access_point]->_label)
            {
            U_INTERNAL_ASSERT(*ap_address)

            WiAuthVirtualAccessPoint::setRecord();

            U_LOGGER("*** WiAuthVirtualAccessPoint::setRecord(): AP(%v@%v) INSERT ON DB WiAuthVirtualAccessPoint ***", ap_label->rep, ap_address->rep);

            U_RETURN(true);
            }
         }

      index_access_point = U_NOT_FOUND;

      U_LOGGER("*** LABEL(%v) NOT EXISTENT ON AP(%v:%v) ***", ap_label->rep, ap_address->rep, hostname.rep);

      U_RETURN(false);
      }

   bool findLabel(uint32_t n)
      {
      U_TRACE(5, "WiAuthNodog::findLabel(%u)", n)

      U_CHECK_MEMORY

      U_ASSERT_EQUALS(sz, vec_access_point.size())

      U_INTERNAL_DUMP("ap_label = %V", ap_label->rep)

      U_INTERNAL_ASSERT(*ap_label)

      if (n < sz &&
          *ap_label == vec_access_point[n]->_label)
         {
         index_access_point = n;

         U_RETURN(true);
         }

      return findLabel();
      }

   UString getLabelAP(uint32_t n)
      {
      U_TRACE(5, "WiAuthNodog::getLabelAP(%u)", n)

      U_CHECK_MEMORY

      U_ASSERT_EQUALS(sz, vec_access_point.size())

      if (n < sz) U_RETURN_STRING(vec_access_point[n]->_label);

      return UString::getStringNull();
      }

   void resetCounter()
      {
      U_TRACE_NO_PARAM(5, "WiAuthNodog::resetCounter()")

      U_CHECK_MEMORY

      U_ASSERT(status <= 0)
      U_ASSERT_EQUALS(sz, vec_access_point.size())

      for (uint32_t i = 0; i < sz; ++i)
         {
         UString key = vec_access_point[i]->_label + '@' + *ap_address;

         // NB: we may be a different process from what it has updated so that we need to read the record...

         if (db_ap->getDataStorage(key))
            {
            vap_rec->_num_users_connected = 0;

            (void) db_ap->putDataStorage(key);
            }
         }
      }

   static bool checkMAC()
      {
      U_TRACE_NO_PARAM(5, "WiAuthNodog::checkMAC()")

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

               U_LOGGER("*** FORCED NO CONSUME ON AP(%v@%v) LABEL(%v) MAC_MASK(%v) ***", ap_address->rep, ap_label->rep, ap_address->rep, ap_rec->mac_mask.rep);

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
      U_INTERNAL_ASSERT_RANGE(0,_status,3)

      // NB: we may be a different process from what it has updated so that we need to read the record...

      if (db_nodog->getDataStorage(*ap_address))
         {
         U_INTERNAL_DUMP("status = %d", status)

         if (_status != status)
            {
            int old_status = status;

            since = u_now->tv_sec;

                 if (_status == 0) status = _status;
            else if (_status == 3) // 3 => startup
               {
               start  = since;
               status = 0;

               for (uint32_t i = 0; i < sz; ++i) vec_access_point[i]->_num_users_connected = 0;
               }
            else if (  status >   0 ||
                     --status <= -3)
               {
               status = _status;
               }

            U_LOGGER("*** WiAuthNodog::setStatus(%d) AP(%v) CHANGE STATE (%d=>%d) ***", _status, ap_address->rep, old_status, status);
            }

         last_info = u_now->tv_sec;

         (void) db_nodog->putDataStorage();
         }
      }

   UString sendRequestToNodog(const char* fmt, uint32_t fmt_size, ...) // NB: request => http://%s:%u/...", *ap_address, 5280...
      {
      U_TRACE(5, "WiAuthNodog::sendRequestToNodog(%.*S,%u)", fmt_size, fmt, fmt_size)

      U_INTERNAL_ASSERT(*ap_address)

      int _status = 0;
      UString result, buffer(U_CAPACITY);

      va_list argp;
      va_start(argp, fmt_size);

      buffer.vsnprintf(fmt, fmt_size, argp);

      va_end(argp);

      url_nodog->snprintf(U_CONSTANT_TO_PARAM("http://%v:5280/%v"), ap_address->rep, buffer.rep);

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

   static void editRecord(bool breboot, bool bconsume, UString& mac_mask, UString& group_account_mask)
      {
      U_TRACE(5, "WiAuthNodog::editRecord(%b,%b,%V,%V)", breboot, bconsume, mac_mask.rep, group_account_mask.rep)

      U_INTERNAL_ASSERT(*ap_address)

      // NB: we may be a different process from what it has updated so that we need to read the record...

      if (db_nodog->getDataStorage(*ap_address))
         {
         if (nodog_rec->findLabel())
            {
            WiAuthAccessPoint* ap_rec = nodog_rec->vec_access_point[index_access_point];

         // ap_rec->noconsume          = (bconsume == false);
            ap_rec->mac_mask           = mac_mask;
            ap_rec->group_account_mask = group_account_mask;

            (void) db_nodog->putDataStorage();
            }

         if (breboot) // NB: REBOOT access point tramite webif...
            {
            U_INTERNAL_ASSERT(UServer_Base::bssl)

            UString url(U_CAPACITY);

            url.snprintf(U_CONSTANT_TO_PARAM("http://%v/cgi-bin/webif/reboot.sh"), ap_address->rep);

            if (client->sendPost(url, *pbody)) nodog_rec->setStatus(1); // unreachable
            }
         }
      }

   void addAccessPoint(bool noconsume)
      {
      U_TRACE(5, "WiAuthNodog::addAccessPoint(%b)", noconsume)

      U_CHECK_MEMORY

      U_INTERNAL_DUMP("sz = %u ap_label = %V", sz, ap_label->rep)

      U_ASSERT_EQUALS(sz, vec_access_point.size())

      U_INTERNAL_ASSERT(*ap_label)
      U_INTERNAL_ASSERT_DIFFERS(ap_label->c_char(0), '0')

      WiAuthAccessPoint* p;

      U_NEW(WiAuthAccessPoint, p, WiAuthAccessPoint(*ap_label, noconsume));

      vec_access_point.push_back(p);

      ++sz;

      U_LOGGER("*** LABEL(%v) ADDED ON AP(%v:%v) - noconsume(%b) ***", ap_label->rep, ap_address->rep, ap_hostname->rep, noconsume);

      WiAuthVirtualAccessPoint::setRecord();

      U_LOGGER("*** WiAuthVirtualAccessPoint::setRecord(): AP(%v@%v) INSERT ON DB WiAuthVirtualAccessPoint ***", ap_label->rep, ap_address->rep);
      }

   static uint32_t getPositionFromAnagrafica()
      {
      U_TRACE_NO_PARAM(5, "WiAuthNodog::getPositionFromAnagrafica()")

      U_INTERNAL_ASSERT(*ap_address)

      uint32_t pos = 0;

      /**
       * 159.213.248.233,172.25.0.0/22,213,off
       */

loop: pos = db_anagrafica->find(*ap_address, pos);

      U_INTERNAL_DUMP("pos = %u", pos)

      if (pos == U_NOT_FOUND) U_RETURN(U_NOT_FOUND);

      pos += ap_address->size();

      if (db_anagrafica->c_char(pos) != ',') goto loop;

      U_RETURN(pos);
      }

   static bool setLabelAndNetmaskFromAnagrafica(UVector<UString>& vlabel, UVector<UString>& vnetmask, UVector<UString>* vconsume)
      {
      U_TRACE(5, "WiAuthNodog::setLabelAndNetmaskFromAnagrafica(%p,%p,%p)", &vlabel, &vnetmask, vconsume)

      uint32_t pos = getPositionFromAnagrafica();

      if (pos != U_NOT_FOUND)
         {
         UString netmask, netmask1, lbl;
         UTokenizer tok(db_anagrafica->substr(pos), ",\n");

         /**
          * 159.213.248.233,172.25.0.0/22,213,off
          */

         (void) tok.next(netmask, (bool*)0);
         (void) tok.next(    lbl, (bool*)0);

         U_INTERNAL_ASSERT(lbl)
         U_INTERNAL_ASSERT(netmask)

           vlabel.push_back(lbl);
         vnetmask.push_back(netmask);

         U_INTERNAL_DUMP("tok.current() = %C", tok.current())

         if (tok.current() == 'o') // on/off
            {
            if (vconsume == 0) (void) tok.skipToken();
            else
               {
               UString consume;

               (void) tok.next(consume, (bool*)0);

               vconsume->push_back(consume);
               }
            }

         while (tok.next(*ip, (bool*)0) &&
                ap_address->equal(*ip))
            {
            (void) tok.next(netmask1, (bool*)0);
            (void) tok.next(     lbl, (bool*)0);

            U_INTERNAL_ASSERT(lbl)
            U_INTERNAL_ASSERT(netmask1)

                                       vlabel.push_back(lbl);
            if (netmask != netmask1) vnetmask.push_back(netmask1);
            }

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static bool setLabelAndNetmaskFromAnagrafica(UString& lbl, UString& netmask)
      {
      U_TRACE(5, "WiAuthNodog::setLabelAndNetmaskFromAnagrafica(%p,%p)", &lbl, &netmask)

      UVector<UString> vlabel, vnetmask;

      if (setLabelAndNetmaskFromAnagrafica(vlabel, vnetmask, 0))
         {
         U_DUMP("vlabel.size() = %u vnetmask.size() = %u", vlabel.size(), vnetmask.size())

         if (vlabel.size() == vnetmask.size())
            {
                lbl =   vlabel.join(' ');
            netmask = vnetmask.join(' ');
            }
         else
            {
            U_ASSERT_EQUALS(vnetmask.size(), 1)

                lbl =   vlabel[0];
            netmask = vnetmask[0];
            }

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static int setLabelFromAnagrafica(UVector<UString>& vlabel, UVector<UString>* vconsume)
      {
      U_TRACE(5, "WiAuthNodog::setLabelFromAnagrafica(%p,%p)", &vlabel, vconsume)

      UVector<UString> vnetmask;

      if (setLabelAndNetmaskFromAnagrafica(vlabel, vnetmask, vconsume))
         {
         U_DUMP("vlabel.size() = %u vnetmask.size() = %u", vlabel.size(), vnetmask.size())

         if (vlabel.size() == vnetmask.size())
            {
            if (vlabel.size() == 2 &&
                vlabel[0] == vlabel[1])
               {
               vlabel.erase(1);
               }

            U_RETURN(1);
            }

         // concentratore cisco con opzione 82 on

         U_ASSERT_EQUALS(vnetmask.size(), 1)

         U_RETURN(2);
         }

      U_RETURN(0);
      }

   static bool getNoConsumeFromAnagrafica()
      {
      U_TRACE_NO_PARAM(5, "WiAuthNodog::getNoConsumeFromAnagrafica()")

      uint32_t pos = getPositionFromAnagrafica();

      if (pos != U_NOT_FOUND)
         {
         UTokenizer tok(db_anagrafica->substr(pos), ",\n");

         /**
          * 159.213.248.233,172.25.0.0/22,213,off
          */

         (void) tok.skipToken();
         (void) tok.skipToken();

         U_INTERNAL_DUMP("tok.current() = %C", tok.current())

         if (tok.current() == 'o') // on/off
            {
            UString consume;

            (void) tok.next(consume, (bool*)0);

            if (consume.equal(U_CONSTANT_TO_PARAM("off"))) U_RETURN(true);
            }
         }

      U_RETURN(false);
      }

   bool setRecord(int _port, bool binsert)
      {
      U_TRACE(5, "WiAuthNodog::setRecord(%d,%b)", _port, binsert)

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(*ap_address)

      int op = -1;

      if (ap_label->empty()) (void) ap_label->assign(U_CONSTANT_TO_PARAM("ap"));

      if (db_nodog->getDataStorage(*ap_address))
         {
         if (findLabel() == false)
            {
            if (binsert == false ||
                ap_address_trust == false)
               {
               U_RETURN(false);
               }

            op = RDB_REPLACE;

            addAccessPoint(getNoConsumeFromAnagrafica());
            }

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

         if (op == RDB_REPLACE) (void) db_nodog->putDataStorage(*ap_address);

         U_RETURN(true);
         }

      if (binsert          == false ||
          ap_address_trust == false)
         {
         U_RETURN(false);
         }

      sz        = 0;
      port      = _port;
      status    = 0;
      hostname  = (*ap_hostname ? *ap_hostname : U_STRING_FROM_CONSTANT("hostname_empty"));
      start     =
      since     =
      last_info = u_now->tv_sec;

      vec_access_point.clear();

      UVector<UString> vlabel, vconsume;

      if (setLabelFromAnagrafica(vlabel, &vconsume) == 0) addAccessPoint(false);
      else
         {
         for (uint32_t i = 0, n = vlabel.size(); i < n; ++i)
            {
            *ap_label = vlabel[i];

            addAccessPoint(vconsume[i].equal(U_CONSTANT_TO_PARAM("off")));
            }
         }

      (void) db_nodog->insertDataStorage(*ap_address);

      U_RETURN(true);
      }

   // DB QUERY FUNCTION

   static void countAP(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthNodog::countAP(%p,%p)", key, data)

      U_INTERNAL_ASSERT_POINTER(key)

      uint32_t sz = nodog_rec->sz;

      U_INTERNAL_ASSERT_MAJOR(sz, 0)

      num_ap += sz;

      WiAuthAccessPoint* ap_rec;

      for (uint32_t i = 0; i < sz; ++i)
         {
         ap_rec = nodog_rec->vec_access_point[i];

         if (ap_rec->noconsume) ++num_ap_noconsume;

         num_users_connected += ap_rec->_num_users_connected;
         }
      }

   static void resetPolicy(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthNodog::resetPolicy(%p,%p)", key, data)

      U_INTERNAL_ASSERT_POINTER(key)

      uint32_t sz = nodog_rec->sz;

      U_INTERNAL_ASSERT_MAJOR(sz, 0)

      WiAuthAccessPoint* ap_rec;

      for (uint32_t i = 0 ; i < sz; ++i)
         {
         ap_rec = nodog_rec->vec_access_point[i];

         ap_rec->reset();
         }

      (void) db_nodog->putDataStorage();
      }

   static void getNameAccessPoint(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthNodog::getNameAccessPoint(%p,%p)", key, data)

      U_INTERNAL_ASSERT_POINTER(key)

      ap_address->_assign(key);

      UVector<UString> vlabel;

      int ret = setLabelFromAnagrafica(vlabel, 0);

      if (ret)
         {
         UString riga(U_CAPACITY);
         WiAuthAccessPoint* ap_rec;
         uint32_t i, n = vlabel.size(), bffffff = 0;

         if (ret == 2) // concentratore cisco con opzione 82 on
            {
            *ap_label = *str_ffffff;

            if (nodog_rec->findLabel()) bffffff = index_access_point;
            }

         for (i = 0; i < n; ++i)
            {
            *ap_label = vlabel[i];

            if (nodog_rec->findLabel(i))
               {
another:       ap_rec = nodog_rec->vec_access_point[index_access_point];

               // -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
               // $1 -> ap_label
               // $2 -> ap_address
               // $3 -> ap_hostname
               // $4 -> ap_mac_mask
               // $5 -> ap_group_account
               // $6 -> ap_up
               // $7 -> ap_consume
               // $8 -> submit
               // -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
               // "ap_label=1&ap_address=10.10.100.115&ap_hostname=lab5-r29587_locoM2&ap_mac_mask=&ap_group_account=&ap_consume=on&submit=Registrazione"
               // -------------------------------------------------------------------------------------------------------------------------------------------------------------------------

               riga.snprintf(U_CONSTANT_TO_PARAM("%v,%v,%v,%s,%s,on,%s\n"),
                             ap_label->rep,
                             ap_address->rep,
                             nodog_rec->hostname.rep,
                             (ap_rec->mac_mask ? ap_rec->mac_mask.data() : "\"\""),
                             (ap_rec->group_account_mask ? ap_rec->group_account_mask.data() : "\"\""),
                             (ap_rec->noconsume == false ? "on" : "off"));

               (void) output->append(riga);
               }
            }

         if (bffffff)
            {
            *ap_label = *str_ffffff;

            index_access_point = bffffff;
                                 bffffff = 0;

            goto another;
            }
         }
      }

   static void checkAccessPointFromAnagrafica(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthNodog::checkAccessPointFromAnagrafica(%p,%p)", key, data)

      U_INTERNAL_ASSERT_POINTER(key)

      ap_address->_assign(key);

      UVector<UString> vlabel, vconsume;

      int ret = setLabelFromAnagrafica(vlabel, &vconsume);

      if (ret)
         {
         WiAuthAccessPoint* ap_rec;
         uint32_t i, n = vlabel.size(), bffffff = 0;

         if (ret == 2) // concentratore cisco con opzione 82 on
            {
            *ap_label = *str_ffffff;

            if (nodog_rec->findLabel()) bffffff = index_access_point;
            }

         for (i = 0; i < n; ++i)
            {
            *ap_label = vlabel[i];

            if (nodog_rec->findLabel(i))
               {
another:       ap_rec = nodog_rec->vec_access_point[index_access_point];

               if (ap_rec->noconsume != vconsume[i].equal(U_CONSTANT_TO_PARAM("off")))
                  {
                  ap_rec->noconsume = !ap_rec->noconsume;

                  U_LOGGER("*** AP(%v:%v) - noconsume(%b) ***", ap_label->rep, ap_address->rep, ap_rec->noconsume);

                  (void) db_nodog->putDataStorage();
                  }
               }
            }

         if (bffffff)
            {
            *ap_label = *str_ffffff;

            index_access_point = bffffff;
                                 bffffff = 0;

            goto another;
            }
         }
      }

   static int setNameAccessPoint(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthNodog::setNameAccessPoint(%p,%p)", key, data)

      if (key) U_RETURN(4); // NB: call us later (after set record value from db)...

      if (*ap_address == db_nodog->getKeyID())
         {
         *ap_hostname = nodog_rec->hostname;

         U_RETURN(0); // stop
         }

      U_RETURN(1);
      }

   static int checkAccessPoint(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthNodog::checkAccessPoint(%p,%p)", key, data)

      if (key)
         {
         if (key == (void*)-1)
            {
            *ap_address = db_nodog->getKeyID();

            // NB: request => http://%s:5280/...", *ap_address...

            (void) nodog_rec->sendRequestToNodog(U_CONSTANT_TO_PARAM("check"), 0);

            U_RETURN(1);
            }

         U_RETURN(4); // NB: call us later (after set record value from db)...
         }

      if ((u_now->tv_sec - nodog_rec->last_info) > (16L * 60L)) U_RETURN(3); // NB: call us later (without lock on db)...

      U_RETURN(1);
      }

   static void setStatusNodog(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthNodog::setStatusNodog(%p,%p)", key, data)

      U_INTERNAL_ASSERT_POINTER(key)

      U_INTERNAL_DUMP("num_ap = %u num_ap_up = %u num_ap_down = %u num_ap_unreachable = %u num_ap_open = %u num_ap_noconsume = %u",
                       num_ap,     num_ap_up,     num_ap_down,     num_ap_unreachable,     num_ap_open,     num_ap_noconsume)

      uint32_t sz = nodog_rec->sz;

      U_INTERNAL_ASSERT_MAJOR(sz, 0)

      num_ap += sz;

      int status = nodog_rec->status;

      if (status <= 0)
         {
         ptr1 = "green";
         ptr2 = "yes";

         num_ap_up += sz;
         }
      else
         {
         ptr1 = (status == 2 ? (num_ap_down        += sz, "red")
                             : (num_ap_unreachable += sz, "orange"));
         ptr2 = "NO";
         }

      ap_address->_assign(key);

      UVector<UString> vlabel;

      int ret = setLabelFromAnagrafica(vlabel, 0);

      if (ret)
         {
         WiAuthAccessPoint* ap_rec;
         uint32_t i, n = vlabel.size(), bffffff = 0;
         UString riga(U_CAPACITY), mac_mask, group_account_mask;

         if (ret == 2) // concentratore cisco con opzione 82 on
            {
            *ap_label = *str_ffffff;

            if (nodog_rec->findLabel()) bffffff = index_access_point;
            }

         for (i = 0; i < n; ++i)
            {
            *ap_label = vlabel[i];

            if (nodog_rec->findLabel(i) == false)
               {
               ptr3 = "green";
               ptr4 = "yes";

                         mac_mask.clear();
               group_account_mask.clear();
               }
            else
               {
another:       ap_rec = nodog_rec->vec_access_point[index_access_point];

                         mac_mask = ap_rec->mac_mask;
               group_account_mask = ap_rec->group_account_mask;

               if (ap_rec->noconsume == false)
                  {
                  ptr3 = "green";
                  ptr4 = "yes";
                  }
               else
                  {
                  ptr3 = "orange";
                  ptr4 = "no";

                  ++num_ap_noconsume;

                  if (ap_rec->mac_mask.equal(U_CONSTANT_TO_PARAM("*"))) ++num_ap_open;
                  }
               }

            if (i)
               {
               riga.snprintf(U_STRING_TO_PARAM(*status_nodog_ap_template),
                             ap_label->rep,
                             ptr3, ptr4,
                             mac_mask.rep,
                             group_account_mask.rep,
                             buffer_srv->rep, ap_label->rep, nodog_rec->hostname.rep, ap_address->rep, nodog_rec->port);
               }
            else
               {
               uint32_t n1 = n + (bffffff != 0);
               long uptime = (status <= 0 ? u_now->tv_sec - nodog_rec->start : 0);

               riga.snprintf(U_STRING_TO_PARAM(*status_nodog_template),
                             ap_label->rep,
                             n1, virtual_name->rep, ap_label->rep, nodog_rec->hostname.rep, ap_address->rep, nodog_rec->port, ap_address->rep,
                             n1, nodog_rec->hostname.rep,
                             n1, uptime,
                             n1, ptr1, ptr2,
                             n1, u_now->tv_sec - nodog_rec->since,
                             n1, nodog_rec->last_info + u_now_adjust,
                             ptr3, ptr4,
                             mac_mask.rep,
                             group_account_mask.rep,
                             buffer_srv->rep, ap_label->rep, nodog_rec->hostname.rep, ap_address->rep, nodog_rec->port);
               }

            (void) output->append(riga);
            }

         if (bffffff)
            {
            *ap_label = *str_ffffff;

            index_access_point = bffffff;
                                 bffffff = 0;

            goto another;
            }
         }

      U_INTERNAL_DUMP("num_ap = %u num_ap_up = %u num_ap_down = %u num_ap_unreachable = %u num_ap_open = %u num_ap_noconsume = %u",
                       num_ap,     num_ap_up,     num_ap_down,     num_ap_unreachable,     num_ap_open,     num_ap_noconsume)
      }

   static void setStatusNodogAndUser(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthNodog::setStatusNodogAndUser(%p,%p)", key, data)

      U_INTERNAL_ASSERT_POINTER(key)
      U_INTERNAL_ASSERT_EQUALS(db_user_filter_tavarnelle, false)

      int status = nodog_rec->status;

      if (admin_status_nodog_and_user_as_csv == false)
         {
         uint32_t sz = nodog_rec->sz;

         U_INTERNAL_ASSERT_MAJOR(sz, 0)

         num_ap += sz;

         if (status <= 0)
            {
            ptr1 = "green";
            ptr2 = "yes";

            num_ap_up += sz;
            }
         else
            {
            ptr1 = (status == 2 ? (num_ap_down        += sz, "red")
                                : (num_ap_unreachable += sz, "orange"));
            ptr2 = "NO";
            }
         }

      ap_address->_assign(key);

      UVector<UString> vlabel;

      int ret = setLabelFromAnagrafica(vlabel, 0);

      if (ret)
         {
         UString riga(U_CAPACITY);
         WiAuthAccessPoint* ap_rec;
         uint32_t i, n = vlabel.size(), bffffff = 0;

         if (ret == 2) // concentratore cisco con opzione 82 on
            {
            *ap_label = *str_ffffff;

            if (nodog_rec->findLabel()) bffffff = index_access_point;
            }

         for (i = 0; i < n; ++i)
            {
            *ap_label = vlabel[i];

            num_users_connected_on_nodog = 0;

            if (status <= 0)
               {
               if (nodog_rec->findLabel(i))
                  {
another:          ap_rec = nodog_rec->vec_access_point[index_access_point];

                  num_users_connected += (num_users_connected_on_nodog = ap_rec->_num_users_connected);
                  }
               }

            if (i)
               {
               if (admin_status_nodog_and_user_as_csv)
                  {
                  riga.snprintf(U_CONSTANT_TO_PARAM("\"%v@%v:%u/%v\",%u,\n"), ap_label->rep, ap_address->rep, nodog_rec->port, nodog_rec->hostname.rep, num_users_connected_on_nodog);
                  }
               else
                  {
                  riga.snprintf(U_CONSTANT_TO_PARAM("<tr>"
                                "  <td class=\"padded\">%v</td>"
                                "  <td class=\"padded\">%u</td>"
                                "</tr>"),
                                ap_label->rep,
                                num_users_connected_on_nodog);
                  }
               }
            else
               {
               if (admin_status_nodog_and_user_as_csv)
                  {
                  riga.snprintf(U_CONSTANT_TO_PARAM("\"%v@%v:%u/%v\",%u,\n"), ap_label->rep, ap_address->rep, nodog_rec->port, nodog_rec->hostname.rep, num_users_connected_on_nodog);
                  }
               else
                  {
                  uint32_t n1 = n + (bffffff != 0);

                  riga.snprintf(U_STRING_TO_PARAM(*status_nodog_and_user_body_template),
                                ap_label->rep,
                                n1, virtual_name->rep, ap_label->rep, nodog_rec->hostname.rep, ap_address->rep, nodog_rec->port, ap_address->rep,
                                n1, nodog_rec->hostname.rep,
                                n1, ptr1, ptr2,
                                num_users_connected_on_nodog);
                  }
               }

            (void) output->append(riga);
            }

         if (bffffff)
            {
            *ap_label = *str_ffffff;

            index_access_point = bffffff;
                                 bffffff = 0;

            goto another;
            }
         }
      }

   static void setLoginNodogTotal(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthNodog::setLoginNodogTotal(%p,%p)", key, data)

      U_INTERNAL_ASSERT_POINTER(key)
      U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

      WiAuthAccessPoint* ap_rec;

      for (uint32_t i = 0; i < nodog_rec->sz; ++i)
         {
         ap_rec = nodog_rec->vec_access_point[i];

         totale1 += ap_rec->num_login;
         totale2 += ap_rec->num_auth_domain_ALL;
         totale3 += ap_rec->num_auth_domain_FICARD;
         totale4 += ap_rec->traffic_done;
         }
      }

   static void setLoginNodog(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthNodog::setLoginNodog(%p,%p)", key, data)

      U_INTERNAL_ASSERT_POINTER(key)

      double rate1, rate2;
      UString riga(U_CAPACITY);
      WiAuthAccessPoint* ap_rec;

      num_ap += nodog_rec->sz;

      ap_address->_assign(key);

      for (uint32_t i = 0; i < nodog_rec->sz; ++i)
         {
         ap_rec = nodog_rec->vec_access_point[i];

         num_users_connected += ap_rec->_num_users_connected;

         rate1 = (ap_rec->num_login    ? (double)ap_rec->num_login    * (double)100. / (double)totale1 : .0);
         rate2 = (ap_rec->traffic_done ? (double)ap_rec->traffic_done * (double)100. / (double)totale4 : .0);

         riga.snprintf(U_STRING_TO_PARAM(*login_nodog_template),
                       ap_rec->_label.rep, ap_address->rep, nodog_rec->port, nodog_rec->hostname.rep,
                       ap_rec->num_login, rate1, 
                       ap_rec->num_auth_domain_ALL,
                       ap_rec->num_auth_domain_FICARD,
                       (uint32_t)(ap_rec->traffic_done / (1024ULL * 1024ULL)), rate2);

         (void) output->append(riga);
         }
      }

   static int checkStatusNodog(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthNodog::checkStatusNodog(%p,%p)", key, data)

      if (key)
         {
         if (key == (void*)-1)
            {
            ++num_ap_delete;

            U_LOGGER("*** AP TO REMOVE: AP(%v) ***", nodog_rec->hostname.rep);

            U_RETURN(2); // delete
            }

         U_RETURN(4); // NB: call us later (after set record value from db)...
         }

      U_INTERNAL_DUMP("nodog_rec->sz = %u UNotifier::last_event = %ld nodog_rec->since = %ld nodog_rec->hostname = %V",
                       nodog_rec->sz,     UNotifier::last_event,      nodog_rec->since,      nodog_rec->hostname.rep)

      U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

      if (nodog_rec->status == 2 &&
          nodog_rec->since <= (UNotifier::last_event - (2 * 30 * 24 * 60 * 60))) // 2 month
         {
         U_RETURN(3); // NB: call us later (without lock on db)...
         }

      if (status_nodog_and_user_resync &&
          nodog_rec->status <= 0)
         {
         *ap_address = db_nodog->getKeyID();

         nodog_rec->resetCounter(); 
         }

      U_RETURN(1);
      }

   static int                dbNodogFilter(UStringRep* key, UStringRep* data);
   static int             getAccessPointUP(UStringRep* key, UStringRep* data);
   static void resyncNumberOfUserConnected(UStringRep* key, UStringRep* data);
};

static void sendRedirect()
{
   U_TRACE_NO_PARAM(5, "::sendRedirect()")

   U_INTERNAL_DUMP("redir = %V", redir->rep)

   U_INTERNAL_ASSERT(*redir)

   if (strncmp(redir->c_pointer(U_CONSTANT_SIZE("http://")), U_STRING_TO_PARAM(*virtual_name))) USSIPlugIn::setAlternativeRedirect("%v", redir->rep);
   else                                                                                         USSIPlugIn::setAlternativeRedirect("http://www.google.com", 0);
}

static void setCookie1()
{
   U_TRACE_NO_PARAM(5, "::setCookie1()")

   U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

   UString param(200U);

   // -----------------------------------------------------------------------------------------------------------------------------------
   // param: "[ data expire path domain secure HttpOnly ]"
   // -----------------------------------------------------------------------------------------------------------------------------------
   // string -- key_id or data to put in cookie    -- must
   // int    -- lifetime of the cookie in HOURS    -- must (0 -> valid until browser exit)
   // string -- path where the cookie can be used  -- opt
   // string -- domain which can read the cookie   -- opt
   // bool   -- secure mode                        -- opt
   // bool   -- only allow HTTP usage              -- opt
   // -----------------------------------------------------------------------------------------------------------------------------------
   // RET: Set-Cookie: ulib.s<counter>=data&expire&HMAC-MD5(data&expire); expires=expire(GMT); path=path; domain=domain; secure; HttpOnly
   // -----------------------------------------------------------------------------------------------------------------------------------

   param.snprintf(U_CONSTANT_TO_PARAM("[ %v %u / %v ]"), UHTTP::getKeyIdDataSession(*mac).rep, 24 * 30, virtual_name->rep);

   UHTTP::setCookie(param);
}

static void setCookie2(const char* hexdump)
{
   U_TRACE(5, "::setCookie2(%S)", hexdump)

   UString cookie(200U);

   if (hexdump == 0) cookie.snprintf(U_CONSTANT_TO_PARAM("WCID=; expires=%#8D"), u_now->tv_sec - U_ONE_DAY_IN_SECOND);
   else
      {
      cookie.snprintf(U_CONSTANT_TO_PARAM("WCID=%s; expires=%#8D"), hexdump, u_now->tv_sec + 30L * U_ONE_DAY_IN_SECOND);

      UHTTP::set_cookie_option->snprintf(U_CONSTANT_TO_PARAM("; path=/login_request; domain=%v; secure"), virtual_name->rep);
      }

   UHTTP::addSetCookie(cookie);
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

static bool getCookie1()
{
   U_TRACE_NO_PARAM(5, "::getCookie1()")

   U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

   if (WiAuthNodog::checkMAC())
      {
      UString cookie = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("ULIB_SESSION"), UClientImage_Base::environment);

      if (cookie &&
          cookie == *mac)
         {
         *auth_domain = *cookie_auth + "MAC";

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

static bool getCookie2(UString* prealm, UString* pid)
{
   U_TRACE(5, "::getCookie2(%p,%p)", prealm, pid)

   U_INTERNAL_ASSERT(UServer_Base::bssl)

   UString cookie = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("HTTP_COOKIE"), UClientImage_Base::environment);

   if (cookie)
      {
      uint32_t pos = U_STRING_FIND(cookie, 0, "WCID=");

      if ( pos != U_NOT_FOUND &&
          (pos += U_CONSTANT_SIZE("WCID="), pos < cookie.size()))
         {
         uint32_t pos1   = cookie.findWhiteSpace();
         const char* ptr = (pos1 == U_NOT_FOUND ? cookie.pend() : cookie.c_pointer(pos1));

         if (ptr[-1] == ';') --ptr;

         UString value = cookie.substr(pos, ptr - cookie.c_pointer(pos));

         if (pid) *pid = value.copy();

         if (askToLDAP(0, 0, 0, "ldapsearch -LLL -b %v %v (&(objectClass=waSession)(&(waCookieId=%v)))", wiauth_session_basedn->rep, ldap_session_param->rep, value.rep) != 1)
            {
            setCookie2(0);

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
               // U_LOGGER("*** COOKIE REALM DIFFER(%v=>%v) - UID(%v) IP(%v) MAC(%v) AP(%v@%v) ***", prealm->rep, x.rep, uid->rep, ip->rep, mac->rep, ap_label->rep, ap_address->rep);

                  *prealm = UStringExt::trim(x);
                  }
               }

            U_RETURN(true);
            }
         }
      }

   U_RETURN(false);
}

static void loginWithProblem()
{
   U_TRACE_NO_PARAM(5, "::loginWithProblem()")

   if (UServer_Base::bssl) setCookie2(0);

   if (*uid)
      {
      if (uid->isPrintable() == false) (void) uid->assign(U_CONSTANT_TO_PARAM("not printable"));

      U_LOGGER("*** FAILURE: UID(%v) IP(%v) MAC(%v) AP(%v@%v) POLICY(%v) ***", uid->rep, ip->rep, mac->rep, ap_label->rep, ap_address->rep, policy->rep);
      }

   USSIPlugIn::setMessagePageWithVar(*message_page_template, "Login",
                                     U_CONSTANT_TO_PARAM("Problema in fase di autenticazione. "
                                     "Si prega di riprovare, se il problema persiste contattare: %v"), telefono->rep);
}

static void checkForRedirect()
{
   U_TRACE_NO_PARAM(5, "::checkForRedirect()")

   if (*redir) sendRedirect();
   else        loginWithProblem();
}

static bool checkTimeRequest()
{
   U_TRACE_NO_PARAM(5, "::checkTimeRequest()")

   long timestamp;
   bool ko = (*ts && (u_now->tv_sec - (timestamp = ts->strtoul())) > (5L * 60L));

   if (ko)
      {
      U_LOGGER("*** IP(%v) MAC(%v) AP(%v) request expired: %#5D ***", ip->rep, mac->rep, ap->rep, timestamp);
      }
   else
      {
      ko = ts->empty();

      if (ko)
         {
         U_LOGGER("*** IP(%v) MAC(%v) AP(%v) request without timestamp ***", ip->rep, mac->rep, ap->rep);
         }
      }

   if (ko)
      {
      checkForRedirect();

      U_RETURN(false);
      }

   U_RETURN(true);
}

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
      U_TRACE_NO_PARAM(5, "WiAuthUser::clear()")

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
      U_TRACE_NO_PARAM(5, "WiAuthUser::toBuffer()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT(_mac)
      U_INTERNAL_ASSERT(nodog)

      // 172.16.1.172 0 1391085937 1391085921 0 0 878 7200 0 314572800 0 1 3291889980 0 0 MAC_AUTH_all 00:14:a5:6e:9c:cb DAILY 10.10.100.115 "anonymous"

      buffer_data->snprintf(U_CONSTANT_TO_PARAM("%v %u "
                            "%ld %ld "
                            "%u %llu "
                            "%u %u %llu %llu "
                            "%u %u %u %u %u "
                            "%v %v %v %v \"%v\""),
                            _ip.rep, connected,
                            last_modified, login_time,
                            _time_done, _traffic_done,
                            _time_consumed, _time_available, _traffic_consumed, _traffic_available,
                            _index_access_point, consume, agent, DownloadRate, UploadRate,
                            _auth_domain.rep, _mac.rep, _policy.rep, nodog.rep, _user.rep);

      buffer_len = buffer_data->size();

      U_RETURN(buffer_data->data());
      }

#if defined(U_STDCPP_ENABLE)
   virtual void fromStream(istream& is)
      {
      U_TRACE(5, "WiAuthUser::fromStream(%p)", &is)

      U_CHECK_MEMORY

      _ip.get(is);

      is.get(); // skip ' '

      is >> connected
         >> last_modified >> login_time
         >>    _time_done
         >> _traffic_done
         >>    _time_consumed >>    _time_available
         >> _traffic_consumed >> _traffic_available
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

      U_INTERNAL_ASSERT(_mac)
      }

# ifdef DEBUG
   const char* dump(bool breset) const { return ""; }
# endif
#endif

   bool setNodogReference()
      {
      U_TRACE_NO_PARAM(5, "WiAuthUser::setNodogReference()")

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

   bool isUserF()
      {
      U_TRACE_NO_PARAM(5, "WiAuthUser::isUserF()")

      U_CHECK_MEMORY

      if (connected           &&
          setNodogReference() &&
          nodog_rec->vec_access_point[_index_access_point]->_label.equal(U_CONSTANT_TO_PARAM("ffffff")))
         {
         U_RETURN(true);
         }

      U_RETURN(false);
      }

   UString getAP()
      {
      U_TRACE_NO_PARAM(5, "WiAuthUser::getAP()")

      U_CHECK_MEMORY

      *label = nodog_rec->getLabelAP(_index_access_point);

      if (label->empty()) (void) label->assign(U_CONSTANT_TO_PARAM("ap"));

      UString x(U_CAPACITY);

      x.snprintf(U_CONSTANT_TO_PARAM("%v@%v:%u/%v"), label->rep, nodog.rep, nodog_rec->port, nodog_rec->hostname.rep);

      U_RETURN_STRING(x);
      }

   void setLabelAP()
      {
      U_TRACE_NO_PARAM(5, "WiAuthUser::setLabelAP()")

      U_CHECK_MEMORY

      U_INTERNAL_ASSERT_EQUALS(u_buffer_len, 0)

      label->clear();

      U_INTERNAL_DUMP("user_rec->nodog = %V user_rec->_index_access_point = %u", user_rec->nodog.rep, user_rec->_index_access_point)

      u_buffer_len = u__snprintf(u_buffer, U_BUFFER_SIZE,
                                 U_CONSTANT_TO_PARAM("%u %v"), user_rec->_index_access_point, user_rec->nodog.rep);

      db_ap->callForAllEntry(WiAuthVirtualAccessPoint::checkForLabel);

      u_buffer_len = 0;
      }

   void setConnected(bool bconnected)
      {
      U_TRACE(5, "WiAuthUser::setConnected(%b)", bconnected)

      U_CHECK_MEMORY

      if ((connected = bconnected)) login_time = u_now->tv_sec;
      }

   void getDone()
      {
      U_TRACE_NO_PARAM(5, "WiAuthUser::getDone()")

      U_CHECK_MEMORY

         time_done->setFromNumber32(   _time_done / 60U);
      traffic_done->setFromNumber64(_traffic_done / (1024ULL * 1024ULL));
      }

   void getConsumed()
      {
      U_TRACE_NO_PARAM(5, "WiAuthUser::getConsumed()")

      U_CHECK_MEMORY

         time_consumed->setFromNumber32(   _time_consumed / 60);
      traffic_consumed->setFromNumber64(_traffic_consumed / (1024ULL * 1024ULL));
      }

   void getCounter()
      {
      U_TRACE_NO_PARAM(5, "WiAuthUser::getCounter()")

      U_CHECK_MEMORY

         time_counter->setFromNumber32s(((int32_t)   _time_available - (int32_t)   _time_consumed) /  60);
      traffic_counter->setFromNumber64s(((int64_t)_traffic_available - (int64_t)_traffic_consumed) / (1024LL * 1024LL));
      }

   UString getPolicy()
      {
      U_TRACE_NO_PARAM(5, "WiAuthUser::getPolicy()")

      UString x = (consume || _policy != *policy_daily ? _policy : *policy_flat);

      U_RETURN_STRING(x);
      }

   void writeTo(int fd, const char* op)
      {
      U_TRACE(5, "WiAuthUser::writeTo(%d,%S)", fd, op)

      U_INTERNAL_ASSERT_POINTER(op)

      getDone();

      /**
       * Example
       * ---------------------------------------------------------------------------------------------------------------------------------------------------
       * 2012/08/08 14:56:00 op: PASS_AUTH, uid: 33437934, ap: 00@10.8.1.2, ip: 172.16.1.172, mac: 00:14:a5:6e:9c:cb, time: 233, traffic: 342, policy: DAILY
       * ---------------------------------------------------------------------------------------------------------------------------------------------------
       */

      U_INTERNAL_ASSERT(_mac)
      U_INTERNAL_ASSERT(*time_done)
      U_INTERNAL_ASSERT(*traffic_done)

      if (brenew == false)
         {
         ULog::log(fd, U_CONSTANT_TO_PARAM("op: %s, uid: %v, ap: %v, ip: %v, mac: %v, time: %v, traffic: %v, policy: %v"),
                                            op, uid->rep, getAP().rep, _ip.rep, _mac.rep, time_done->rep, traffic_done->rep, getPolicy().rep);
         }
      else
         {
         UString x(U_CAPACITY);

         db_nodog->callForAllEntry(WiAuthNodog::setNameAccessPoint);

         x.snprintf(U_CONSTANT_TO_PARAM("%v@%v:%u/%v"), ap_label->rep, ap_address->rep, nodog_rec->port, ap_hostname->rep);

         ULog::log(fd, U_CONSTANT_TO_PARAM("op: %s, uid: %v, ap: %v, ip: %v, mac: %v, time: %v, traffic: %v, policy: %v"),
                                            op, uid->rep, x.rep, ip->rep, _mac.rep, time_done->rep, traffic_done->rep, getPolicy().rep);
         }
      }

   void writeToLOG(const char* op) { writeTo(file_LOG->getFd(), op); }

   static bool checkMAC()
      {
      U_TRACE_NO_PARAM(5, "WiAuthUser::checkMAC()")

      if (mac->empty())
         {
         USSIPlugIn::setMessagePage(*message_page_template, "Errore", "Errore Autorizzazione - MAC vuoto");

         U_RETURN(false);
         }

      if (*mac != *UString::str_without_mac)
         {
         *uid         = *mac;
         *auth_domain = *mac_auth + "_" + *realm;
         }
      else
         {
         if (ip->empty())
            {
            USSIPlugIn::setMessagePage(*message_page_template, "Errore", "Errore Autorizzazione - IP vuoto");

            U_RETURN(false);
            }

         *uid         = *ip;
         *auth_domain = *ip_auth + "_" + *realm;
         }

      U_RETURN(true);
      }

   static const char* getWriteToLog(const UString& logout)
      {
      U_TRACE(5, "WiAuthUser::getWriteToLog(%V)", logout.rep)

      U_ASSERT_EQUALS(logout.size(), 2)

      const char* write_to_log = 0;

      switch (logout.c_char(1) - '0')
         {
         case U_LOGOUT_NO_TRAFFIC:           write_to_log = "EXIT_NO_TRAFFIC";           break; // 1
         case U_LOGOUT_NO_ARP_CACHE:         write_to_log = "EXIT_NO_ARP_CACHE";         break; // 2
         case U_LOGOUT_NO_ARP_REPLY:         write_to_log = "EXIT_NO_ARP_REPLY";         break; // 3 
         case U_LOGOUT_NO_MORE_TIME:         write_to_log = "EXIT_NO_MORE_TIME";         break; // 4
         case U_LOGOUT_NO_MORE_TRAFFIC:      write_to_log = "EXIT_NO_MORE_TRAFFIC";      break; // 5
         case U_LOGOUT_CHECK_FIREWALL:       write_to_log = "EXIT_CHECK_FIREWALL";       break; // 6
         case U_LOGOUT_REQUEST_FROM_AUTH:    write_to_log = "EXIT_REQUEST_FROM_AUTH";    break; // 7
         case U_LOGOUT_DIFFERENT_MAC_FOR_IP: write_to_log = "EXIT_DIFFERENT_MAC_FOR_IP"; break; // 8

         default: U_ERROR("Unexpected value for logout: %S", logout.rep);
         }

      return write_to_log;
      }

   const char* updateCounter(const UString& logout, long time_connected, uint64_t traffic, bool& ask_logout)
      {
      U_TRACE(5, "WiAuthUser::updateCounter(%V,%ld,%llu,%b)", logout.rep, time_connected, traffic, ask_logout)

      const char* write_to_log = 0;

      uint32_t    time_done_save =    _time_done;
      uint64_t traffic_done_save = _traffic_done;

         _time_done = time_connected;
      _traffic_done = traffic;

   // writeTo(file_INFO->getFd(), "INFO");

         _time_done +=    time_done_save;
      _traffic_done += traffic_done_save;

      if (consume)
         {
            _time_consumed += time_connected;
         _traffic_consumed += traffic;
         }

      char c = (logout ? logout.first_char() : '0');

      if (c == '0') // NB: logout == 0 mean NOT logout (only info)...
         {
         if (traffic == 0 &&
             time_connected > (30L * 60L)) // 30m (MAX_TIME_NO_TRAFFIC for FLAT policy)
            {
            ask_logout = true;

            /*
            U_LOGGER("*** updateCounter() UID(%v) IP(%v) MAC(%v) AP(%v@%v) connected=%b EXCEED MAX_TIME_NO_TRAFFIC(%ld secs) ***",
                                          uid->rep, _ip.rep, _mac.rep, ap_label->rep, ap_address->rep, connected, time_connected);
            */
            }
         else if (consume)
            {
            if (_time_consumed > _time_available)
               {
               ask_logout = true;

               /*
               long time_diff = _time_consumed - _time_available;

               U_LOGGER("*** updateCounter() UID(%v) IP(%v) MAC(%v) AP(%v@%v) EXCEED TIME_AVAILABLE (%ld sec) ***",
                                             uid->rep, _ip.rep, _mac.rep, ap_label->rep, ap_address->rep, time_diff);
               */
               }

            if (_traffic_consumed > _traffic_available)
               {
               ask_logout = true;

               /*
               uint64_t traffic_diff = _traffic_consumed - _traffic_available;

               U_LOGGER("*** updateCounter() UID(%v) IP(%v) MAC(%v) AP(%v@%v) connected=%b EXCEED TRAFFIC_AVAILABLE (%llu bytes) ***",
                                             uid->rep, _ip.rep, _mac.rep, ap_label->rep, ap_address->rep, connected, traffic_diff);
               */
               }
            }
         }
      else
         {
         connected = brenew;

         write_to_log = getWriteToLog(logout);

         U_INTERNAL_DUMP("_auth_domain = %V", _auth_domain.rep)

         if (_auth_domain == *account_auth)
            {
            /*
            U_LOGGER("*** updateCounter() UID(%v) IP(%v) MAC(%v) AP(%v@%v) connected=%b GROUP ACCOUNT %s (%ld secs) ***",
                                          uid->rep, _ip.rep, _mac.rep, ap_label->rep, ap_address->rep, connected, write_to_log, time_connected);
            */
            
            // --------------------------------------------------------------------------------------------------
            // NB: we have an exit for a group account and we check if there are other user on the
            //     access point that has this group account, in this case we consider the user still connected...
            // --------------------------------------------------------------------------------------------------

            if (nodog_rec->vec_access_point[_index_access_point]->group_account_mask)
               {
               U_INTERNAL_ASSERT_EQUALS(*ap_address, nodog)

               UString result = nodog_rec->sendRequestToNodog(U_CONSTANT_TO_PARAM("users"), 0);

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
         }
next:
      last_modified = u_now->tv_sec;

      U_RETURN(write_to_log);
      }

   static bool checkTypeUID() // NB: return if uid is a mac or an ip address...
      {
      U_TRACE_NO_PARAM(5, "WiAuthUser::checkTypeUID()")

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

   static bool checkUserID()
      {
      U_TRACE_NO_PARAM(5, "WiAuthUser::checkUserID()")

      if (checkTypeUID()) // NB: checkTypeUID() return if uid is a mac or an ip address...
         {
         bool ko = false;

         if (isMAC &&
             *mac != *uid)
            {
            U_LOGGER("*** MAC MISMATCH: UID(%v) IP(%v) MAC(%v) AP(%v@%v) REDIR(%v) ***", uid->rep, ip->rep, mac->rep, ap_label->rep, ap_address->rep, redir->rep);

            UString x;

            if (*mac) x = UStringExt::trim(*mac);

            if (x) *uid = x;

            ko = (checkMAC() == false);
            }
         else if (isIP &&
                  *ip != *uid)
            {
            ko = true;

            U_LOGGER("*** IP MISMATCH: UID(%v) IP(%v) MAC(%v) AP(%v@%v) REDIR(%v) ***", uid->rep, ip->rep, mac->rep, ap_label->rep, ap_address->rep, redir->rep);

            if (*ip)
               {
               UString x = UStringExt::trim(*ip);

               if (x) *uid = x;
               }
            }

         if (ko)
            {
            checkForRedirect();

            U_RETURN(false);
            }

         if (checkTimeRequest() == false) U_RETURN(false);
         }

      U_RETURN(true);
      }

   static UString _getUserName()
      {
      U_TRACE_NO_PARAM(5, "WiAuthUser::_getUserName()")

      UString user, content;

      if (checkTypeUID() == false) // NB: checkTypeUID() return if uid is a mac or an ip address...
         {
         UString pathname(U_CAPACITY);

         pathname.snprintf(U_CONSTANT_TO_PARAM("%v/%v.reg"), dir_reg->rep, uid->rep);

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
            U_LOGGER("*** USER EMPTY: UID(%v) ***", uid->rep);
            }
         }

      U_RETURN_STRING(user);
      }

   static UString getUserName()
      {
      U_TRACE_NO_PARAM(5, "WiAuthUser::getUserName()")

      UString user, x = _getUserName();

      if (user_exist &&
          user_rec->_user != x)
         {
         U_LOGGER("*** USER(%v=>%v) WITH DIFFERENT VALUE ***", user_rec->_user.rep, x.rep);

         user_rec->_user = x;
         }

      user = x;

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

         if (UFileConfig::loadProperties(*table, content))
            {
            key_time            = "MAX_TIME";
            key_traffic         = "MAX_TRAFFIC";
            key_time_no_traffic = "MAX_TIME_NO_TRAFFIC";
            }
         }

      if (key_time)
         {
                                     *time_available   = (*table)[key_time];
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
      U_TRACE_NO_PARAM(5, "WiAuthUser::setRecord()")

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

         connected = brenew;
         }
      else
         {
         op    = RDB_INSERT;
         _user = _getUserName();

         _time_done        =
         _time_consumed    = 0L;
         _traffic_done     =
         _traffic_consumed = 0ULL;

         _policy = (bflat ? *policy_daily : *policy); // NB: if the system policy is flat we assume default...

         connected = false;
         }

      nodog               = *ap_address;
      _index_access_point = index_access_point;

      U_INTERNAL_DUMP("db_user->isRecordFound() = %b _index_access_point = %u nodog = %V", db_user->isRecordFound(), _index_access_point, nodog.rep)

      U_INTERNAL_ASSERT(*mac)

      _ip           = *ip;
      _mac          = *mac;
      agent         = 0; // UHTTP::getUserAgent();
      login_time    = 0L;
      last_modified = u_now->tv_sec;

        UploadRate  =   user_UploadRate->strtoul();
      DownloadRate  = user_DownloadRate->strtoul();

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

      uint32_t _time_available_tmp    =    time_available->strtoul();
      uint64_t _traffic_available_tmp = traffic_available->strtoull();

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

   static const char* getPointerToConnection(UStringRep* data) __pure
      {
      U_TRACE(5, "WiAuthUser::getPointerToConnection(%p)", data)

      U_INTERNAL_ASSERT_POINTER(data)

// ip           c modified   login time_consume traff_consume time_done time_avail traff_done traff_avail x m agent     Dr Ur domain
// 172.16.1.172 0 1391085937 1391085921  0      0             878       7200            0     314572800   0 1 3291889980 0 0  MAC_AUTH_all 00:14:a5:6e:9c:cb DAILY 10.10.100.115 "anonymous"

      const char* ptr = data->c_pointer(7); // "1.1.1.1"

      do { ++ptr; } while (u__isspace(*ptr) == false);

      U_INTERNAL_ASSERT(u__isspace(ptr[0]))

      return ptr;
      }

   static bool isConnected(UStringRep* data) __pure
      {
      U_TRACE(5, "WiAuthUser::isConnected(%p)", data)

      const char* ptr = getPointerToConnection(data);

      if (ptr[1] == '1') U_RETURN(true);

      U_RETURN(false);
      }

   static const char* getIndexAccessPoint(const char* ptr, uint32_t* pindex_access_point)
      {
      U_TRACE(5, "WiAuthUser::getIndexAccessPoint(%p,%p)", ptr, pindex_access_point)

      U_INTERNAL_ASSERT_POINTER(ptr)
      U_INTERNAL_ASSERT(u__isblank(*ptr))

// ip           c modified   login time_consume traff_consume time_done time_avail traff_done traff_avail x m agent     Dr Ur domain
// 172.16.1.172 0 1391085937 1391085921  0      0             878       7200            0     314572800   0 1 3291889980 0 0  MAC_AUTH_all 00:14:a5:6e:9c:cb DAILY 10.10.100.115 "anonymous"

      ptr += 3;

      U_INTERNAL_DUMP("char (last_modified field) = %C", *ptr)

      U_INTERNAL_ASSERT(u__isdigit(*ptr))

      while (u__isdigit(*++ptr)) {}

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      ++ptr;

      U_INTERNAL_DUMP("char (login_time field) = %C", *ptr)

      U_INTERNAL_ASSERT(u__isdigit(*ptr))

      while (u__isdigit(*++ptr)) {}

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      ++ptr;

      U_INTERNAL_DUMP("char (_time_done field) = %C", *ptr)

      U_INTERNAL_ASSERT(u__isdigit(*ptr))

      while (u__isdigit(*++ptr)) {}

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      ++ptr;

      U_INTERNAL_DUMP("char (_traffic_done field) = %C", *ptr)

      U_INTERNAL_ASSERT(u__isdigit(*ptr))

      while (u__isdigit(*++ptr)) {}

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      ++ptr;

      U_INTERNAL_DUMP("char (_time_consumed field) = %C", *ptr)

      U_INTERNAL_ASSERT(u__isdigit(*ptr))

      while (u__isdigit(*++ptr)) {}

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      ++ptr;

      U_INTERNAL_DUMP("char (_time_available field) = %C", *ptr)

      U_INTERNAL_ASSERT(u__isdigit(*ptr))

      while (u__isdigit(*++ptr)) {}

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      ++ptr;

      U_INTERNAL_DUMP("char (_traffic_consumed field) = %C", *ptr)

      U_INTERNAL_ASSERT(u__isdigit(*ptr))

      while (u__isdigit(*++ptr)) {}

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      ++ptr;

      U_INTERNAL_DUMP("char (_traffic_available field) = %C", *ptr)

      U_INTERNAL_ASSERT(u__isdigit(*ptr))

      while (u__isdigit(*++ptr)) {}

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      ++ptr;

      U_INTERNAL_DUMP("char (index_access_point field) = %C", *ptr)

      U_INTERNAL_ASSERT(u__isdigit(*ptr))

      if (pindex_access_point == 0)
         {
         if (u__isblank(ptr[1])) ++ptr; 
         else
            {
            ++ptr;

            while (u__isdigit(*++ptr)) {}
            }
         }
      else
         {
         uint32_t _index_access_point;

         if (u__isblank(ptr[1])) _index_access_point = *ptr++ - '0'; 
         else
            {
            const char* start = ptr++;

            while (u__isdigit(*++ptr)) {}

            _index_access_point = u_strtoul(start, ptr);
            }

         *pindex_access_point = _index_access_point;

         U_INTERNAL_DUMP("_index_access_point = %u", _index_access_point)
         }

      return ptr;
      }

   static const char* getNodogReference(const char* ptr, uint32_t* psize)
      {
      U_TRACE(5, "WiAuthUser::getNodogReference(%p,%p)", ptr, psize)

      U_INTERNAL_ASSERT_POINTER(ptr)
      U_INTERNAL_ASSERT(u__isblank(*ptr))

      ptr += U_CONSTANT_SIZE("0 0"); // consume

      U_INTERNAL_DUMP("char (agent field) = %C", *ptr)

      U_INTERNAL_ASSERT(u__isdigit(*ptr))

      while (u__isdigit(*++ptr)) {}

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      ptr += U_CONSTANT_SIZE(" 0 0 M"); // DownloadRate, UploadRate

      U_INTERNAL_DUMP("char (_auth_domain field) = %C", *ptr)

      if (u__isalpha(*ptr) == false)
         {
         U_LOGGER("*** WiAuthUser::getNodogReference() THE RECORD IS NOT ALIGNED (%C instead of alpha) %20S ***", *ptr, ptr-10);
         }

      U_INTERNAL_ASSERT(u__isalpha(*ptr))

      while (u__isblank(*++ptr) == false) {}

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      ++ptr;

      U_INTERNAL_DUMP("char (_mac field) = %C", *ptr)

      U_INTERNAL_ASSERT(u__isxdigit(*ptr))

      while (u__isblank(*++ptr) == false) {}

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      ++ptr;

      U_INTERNAL_DUMP("char (_policy field) = %C", *ptr)

      U_INTERNAL_ASSERT(u__isupper(*ptr))

      while (u__isblank(*++ptr) == false) {}

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      const char* start = ++ptr;

      U_INTERNAL_DUMP("char (nodog field) = %C", *ptr)

      while (u__isipv4(*++ptr)) {}

      U_INTERNAL_ASSERT(u__isblank(*ptr))

      *psize = ptr - start;

      U_INTERNAL_DUMP("nodog ref = %.*S", *psize, start)

      U_INTERNAL_ASSERT(u_isIPv4Addr(start, *psize))

      return start;
      }

   static bool setNodog(UStringRep* data)
      {
      U_TRACE(5, "WiAuthUser::setNodog(%p)", data)

      const char* ptr = getPointerToConnection(data);

      if (ptr[1] == '0') U_RETURN(false);

      ptr = getIndexAccessPoint(ptr, 0);

      uint32_t sz;

      ptr = getNodogReference(ptr, &sz);

      (void) ap_address->assign(ptr, sz);

      U_RETURN(true);
      }

   // DB QUERY FUNCTION

   static int countUserConnectedOnNodog(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthUser::countUserConnectedOnNodog(%p,%p)", key, data)

      U_INTERNAL_ASSERT_EQUALS(db_user_filter_tavarnelle, false)

      const char* ptr = getPointerToConnection(data);

      if (ptr[1] == '0') U_RETURN(1);

      uint32_t _index_access_point;

      ptr = getIndexAccessPoint(ptr, &_index_access_point);

      if (_index_access_point != index_access_point) U_RETURN(1);

      uint32_t sz;

      ptr = getNodogReference(ptr, &sz);

      if (sz == ap_address->size() &&
          memcmp(ptr, ap_address->data(), sz) == 0)
         {
         ++num_users_connected_on_nodog;
         }

      U_RETURN(1);
      }

   static int checkIfUserExist(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthUser::checkIfUserExist(%V,%V)", key, data)

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

   static int quitUserConnected(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthUser::quitUserConnected(%p,%p)", key, data)

      if (key)
         {
         if (isConnected(data)) U_RETURN(4); // NB: call us later (after set record value from db)...

         U_RETURN(1);
         }

      U_INTERNAL_ASSERT(user_rec->connected)

      if (user_rec->nodog == *ap_address)
         {
         *uid = db_user->getKeyID();

         user_rec->setConnected(false);

         U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

         (void) db_user->putDataStorage(*uid);

         if (user_rec->setNodogReference()) user_rec->writeToLOG("QUIT");
         }

      U_RETURN(1);
      }

   static void checkForUserPolicy(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthUser::checkForUserPolicy(%p,%p)", key, data)

      U_INTERNAL_ASSERT_POINTER(key)

      bool bflat = (user_rec->_policy == *policy_flat);

      if (bflat ||
          user_rec->_policy == *policy_daily)
         {
         uid->_assign(key);

         if (user_rec->connected &&
             user_rec->setNodogReference())
            {
            user_rec->writeToLOG("RST_POLICY");
            }

         user_rec->_time_done        =
         user_rec->_time_consumed    = 0;
         user_rec->_traffic_done     =
         user_rec->_traffic_consumed = 0;

         if (bflat)
            {
            user_rec->_time_available    =    time_available_flat;
            user_rec->_traffic_available = traffic_available_flat; 
            }
         else
            {
            user_rec->_time_available    =    time_available_daily;
            user_rec->_traffic_available = traffic_available_daily; 
            }

         U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

         (void) db_user->putDataStorage(*uid);
         }
      }

   static int getStatusUser(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthUser::getStatusUser(%p,%p)", key, data)

      if (key)
         {
         if (isConnected(data)) U_RETURN(4); // NB: call us later (after set record value from db)...

         U_RETURN(1);
         }

      U_INTERNAL_DUMP("user_rec->last_modified = %ld UNotifier::last_event = %ld user_rec->connected = %b", user_rec->last_modified, UNotifier::last_event, user_rec->connected)

      U_INTERNAL_ASSERT(user_rec->connected)

      *uid = db_user->getKeyID();

      if (user_rec->setNodogReference())
         {
         ++num_users_connected_on_nodog;

         UStringRep* p3;
         UStringRep* p4;

         if (user_rec->consume)
            {
            ptr1 = "green";
            ptr2 = "yes";

            user_rec->getConsumed();

            p3 =    time_consumed->rep;
            p4 = traffic_consumed->rep;
            }
         else
            {
            ptr1 = "orange";
            ptr2 = "no";

            user_rec->getDone();

            p3 =    time_done->rep;
            p4 = traffic_done->rep;
            }

         UString riga(U_CAPACITY),
                 user = getUserName(),
                 x = user_rec->getPolicy(),
                 y = user_rec->getAP();

         riga.snprintf(U_STRING_TO_PARAM(*status_network_template),
                       user.rep,
                       uid->rep,
                       user_rec->_auth_domain.rep,
                       user_rec->_ip.rep,
                       user_rec->_mac.rep,
                       user_rec->login_time + u_now_adjust,
                       x.rep,
                       ptr1, ptr2,
                       p3, p4,
                       virtual_name->rep,
                       label->rep, nodog_rec->hostname.rep, user_rec->nodog.rep, nodog_rec->port, y.rep);

         (void) output->append(riga);
         }

      U_RETURN(1);
      }

   static int setNumberOfUserConnected(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthUser::setNumberOfUserConnected(%p,%p)", key, data)

      if (key)
         {
         if (isConnected(data)) U_RETURN(4); // NB: call us later (after set record value from db)...

         U_RETURN(1);
         }

      U_INTERNAL_ASSERT(user_rec->connected)

      *uid = db_user->getKeyID();

      if (user_rec->setNodogReference())
         {
         UString _key(32U);

         _key.snprintf(U_CONSTANT_TO_PARAM("%v@%v"), nodog_rec->getLabelAP(user_rec->_index_access_point).rep, user_rec->nodog.rep);

         // NB: we may be a different process from what it has updated so that we need to read the record...

         if (db_ap->getDataStorage(_key))
            {
            vap_rec->_num_users_connected++;

            (void) db_ap->putDataStorage(_key);
            }
         }

      U_RETURN(1);
      }

   static int checkStatusUser(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthUser::checkStatusUser(%p,%p)", key, data)

      if (key)
         {
         if (key == (void*)-1)
            {
            ++num_users_delete;

            *uid = db_user->getKeyID();

            U_LOGGER("*** USER TO DELETE: UID(%v) IP(%v) MAC(%v) AP(%v) POLICY(%v) ***", uid->rep, user_rec->_ip.rep, user_rec->_mac.rep, user_rec->nodog.rep, user_rec->_policy.rep);

            U_RETURN(2); // delete
            }

         if (isConnected(data) == false) U_RETURN(4); // NB: call us later (after set record value from db)...
         }
      else
         {
         U_INTERNAL_DUMP("user_rec->last_modified = %ld UNotifier::last_event = %ld user_rec->connected = %b", user_rec->last_modified, UNotifier::last_event, user_rec->connected)

         U_INTERNAL_ASSERT_EQUALS(user_rec->connected, false)

         if (user_rec->last_modified <= (UNotifier::last_event - (30 * 24 * 60 * 60))) // 1 month
            {
            U_RETURN(3); // NB: call us later (without lock on db)...
            }
         }

      U_RETURN(1);
      }

   static int setUsersF(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthUser::setUsersF(%p,%p)", key, data)

      if (key)
         {
         if (key == (void*)-1)
            {
            U_INTERNAL_ASSERT(user_rec->connected)

            *uid = db_user->getKeyID();

            if (user_rec->setNodogReference()) table1->push(user_rec->nodog, *uid);
            }
         else if (isConnected(data)) U_RETURN(4); // NB: call us later (after set record value from db)...

         U_RETURN(1);
         }

      *uid = db_user->getKeyID();

      if (user_rec->isUserF() &&
          nodog_rec->status <= 0)
         {
         U_RETURN(3); // NB: call us later (without lock on db)...
         }

      U_RETURN(1);
      }

   static bool checkUsersF(UStringRep* key, void* data)
      {
      U_TRACE(5, "WiAuthUser::checkUsersF(%p,%p)", key, data)

      WiAuthAccessPoint* ap_rec;
      UVector<UString>* _vuid = (UVector<UString>*)data;

      ap_address->_assign(key);

      for (uint32_t i = 0, n = _vuid->size(); i < n; ++i)
         {
         *uid = _vuid->pop();

         if (db_user->getDataStorage(*uid) &&
             user_rec->setNodogReference())
            {
            vuid->push_back(user_rec->_ip);

            U_INTERNAL_ASSERT(user_rec->connected)
            U_ASSERT_EQUALS(user_rec->nodog, *ap_address)

            ap_rec = nodog_rec->vec_access_point[user_rec->_index_access_point];

            U_INTERNAL_ASSERT_MAJOR(ap_rec->_num_users_connected, 0)
            U_ASSERT_EQUALS(ap_rec->_label, U_STRING_FROM_CONSTANT("ffffff"))

            U_LOGGER("*** USER TO CHECK: UID(%v) IP(%v) MAC(%v) AP(%v@%v) POLICY(%v) ***",
                        uid->rep, user_rec->_ip.rep, user_rec->_mac.rep, ap_rec->_label.rep, ap_address->rep, user_rec->_policy.rep);
            }
         }

      if (vuid->empty() == false)
         {
         UString result = nodog_rec->sendRequestToNodog(U_CONSTANT_TO_PARAM("checkForUsersF?%v"), vuid->join(',').rep);

         if (result &&
             U_IS_HTTP_ERROR(U_http_info.nResponseCode) == false)
            {
#        ifdef USE_LIBZ
            if (UStringExt::isGzip(result)) result = UStringExt::gunzip(result);
#        endif

            UVector<UString> vec(result, ',');

            for (int32_t i = 0, n = vec.size(); i < n; i += 2)
               {
               *uid      = vec[i];
               *ap_label = vec[i+1];

               U_ASSERT_DIFFERS(*ap_label, U_STRING_FROM_CONSTANT("ffffff"))

               if (db_user->getDataStorage(*uid) &&
                   user_rec->setNodogReference())
                  {
                  U_INTERNAL_ASSERT(user_rec->connected)
                  U_ASSERT_EQUALS(user_rec->nodog, *ap_address)

                  ap_rec = nodog_rec->vec_access_point[user_rec->_index_access_point];

                  U_INTERNAL_ASSERT_MAJOR(ap_rec->_num_users_connected, 0)
                  U_ASSERT_EQUALS(ap_rec->_label, U_STRING_FROM_CONSTANT("ffffff"))

                  ap_rec->_num_users_connected--;

                  if (nodog_rec->findLabel() == false) nodog_rec->addAccessPoint(WiAuthNodog::getNoConsumeFromAnagrafica());

                  ap_rec = nodog_rec->vec_access_point[(user_rec->_index_access_point = index_access_point)];

                  ap_rec->_num_users_connected++;

                  (void) db_user->putDataStorage(*uid);
                  (void) db_nodog->putDataStorage(*ap_address);

                  user_rec->writeToLOG("RENEW");
                  }
               }
            }

         vuid->clear();
         }

      U_RETURN(true);
      }

   static int checkStatusUserOnNodog(UStringRep* key, UStringRep* data)
      {
      U_TRACE(5, "WiAuthUser::checkStatusUserOnNodog(%p,%p)", key, data)

      if (key) U_RETURN(4); // NB: call us later (after set record value from db)...

      if (user_rec->nodog == *ap_address)
         {
         bool blogout = false;

         if (pvallow)
            {
            if (UIPAllow::isAllowed(user_rec->_ip.data(), *pvallow))
               {
               *uid = db_user->getKeyID();

               if (user_rec->connected == false &&
                   vuid->find(*uid) != U_NOT_FOUND)
                  {
                  U_LOGGER("*** USER NOT ALIGNED: UID(%v) IP(%v) MAC(%v) AP(%v) POLICY(%v) ***",
                              uid->rep, user_rec->_ip.rep, user_rec->_mac.rep, user_rec->nodog.rep, user_rec->_policy.rep);

                  if (user_rec->last_modified < UNotifier::last_event) blogout = true;
                  }
               }
            }
         else if (user_rec->connected &&
                  user_rec->last_modified <= (UNotifier::last_event - (2 * 60 * 60))) // 2h
            {
            blogout = true;

            *uid = db_user->getKeyID();

            U_LOGGER("*** USER TO QUIT: UID(%v) IP(%v) MAC(%v) AP(%v) POLICY(%v) ***", uid->rep, user_rec->_ip.rep, user_rec->_mac.rep, user_rec->nodog.rep, user_rec->_policy.rep);
            }

         if (blogout)
            {
            user_rec->setConnected(true);

            U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

            (void) db_user->putDataStorage(*uid);

            if (user_rec->setNodogReference()) user_rec->writeToLOG(status_nodog_and_user_resync ? "RESYNC" : "QUIT");
            }
         }

      U_RETURN(1);
      }

   static int dbUserFilter(UStringRep* key, UStringRep* data);
};

// DB QUERY FUNCTION

int WiAuthNodog::getAccessPointUP(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "WiAuthNodog::getAccessPointUP(%p,%p)", key, data)

   if (key)
      {
      if (key == (void*)-1)
         {
         *ap_address = db_nodog->getKeyID();

         if ((vPF)data != (vPF)WiAuthUser::checkStatusUserOnNodog) ((vPF)data)();
         else
            {
            U_INTERNAL_ASSERT_EQUALS(pvallow, 0)

            WiAuthUser::checkStatusUserOnNodog(0, 0);
            }

         U_RETURN(1);
         }

      U_RETURN(4); // NB: call us later (after set record value from db)...
      }

   U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

   if (nodog_rec->status <= 0) U_RETURN(3); // NB: call us later (without lock on db)...

   U_RETURN(1);
}

void WiAuthNodog::resyncNumberOfUserConnected(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "WiAuthNodog::resyncNumberOfUserConnected(%p,%p)", key, data)

   U_INTERNAL_ASSERT_POINTER(key)
   U_INTERNAL_ASSERT_EQUALS(db_user_filter_tavarnelle, false)

   if (nodog_rec->status <= 0)
      {
      ap_address->_assign(key);

      UVector<UString> vlabel;

      int ret = setLabelFromAnagrafica(vlabel, 0);

      if (ret)
         {
         bool bchange = false;
         WiAuthAccessPoint* ap_rec;
         uint32_t i, n = vlabel.size(), bffffff = 0;

         if (ret == 2) // concentratore cisco con opzione 82 on
            {
            *ap_label = *str_ffffff;

            if (nodog_rec->findLabel()) bffffff = index_access_point;
            }

         for (i = 0; i < n; ++i)
            {
            *ap_label = vlabel[i];

            if (nodog_rec->findLabel())
               {
another:       ap_rec = nodog_rec->vec_access_point[index_access_point];

               if (ap_rec->isNumberOfUserConnectedNotAligned()) bchange = true;
               }
            }

         if (bffffff)
            {
            *ap_label = *str_ffffff;

            index_access_point = bffffff;
                                 bffffff = 0;

            (void) WiAuthVirtualAccessPoint::getRecord();

            goto another;
            }

         if (bchange) (void) db_nodog->putDataStorage();
         }
      }
}

// SERVICES

static void resync()
{
   U_TRACE_NO_PARAM(5, "::resync()")

   num_users_delete = 0;

   db_user->callForAllEntryWithVector(WiAuthUser::checkStatusUser);

   U_LOGGER("*** WE HAVE DELETED %u USERS ***", num_users_delete);

   status_nodog_and_user_resync = true;

   db_nodog->callForAllEntry(WiAuthNodog::getAccessPointUP, (vPF)WiAuthUser::checkStatusUserOnNodog);

   num_ap_delete = 0;

   db_nodog->callForAllEntryWithVector(WiAuthNodog::checkStatusNodog);

   status_nodog_and_user_resync = false;

   U_LOGGER("*** WE HAVE REMOVED %u ACCESS POINT ***", num_ap_delete);

   db_user->callForAllEntry(WiAuthUser::setNumberOfUserConnected);

   db_nodog->callForAllEntryWithSetEntry(WiAuthNodog::resyncNumberOfUserConnected);
}

static void db_open()
{
   U_TRACE(5, "::db_open()")

   bool no_ssl = (UServer_Base::bssl == false);

   bool result1 = db_nodog->open(10 * 1024 * 1024, false, false, no_ssl); // 10M
   bool result2 =  db_user->open(10 * 1024 * 1024, false, false, no_ssl); // 10M
   bool result3 =    db_ap->open(10 * 1024 * 1024, false, false, no_ssl); // 10M

   num_ap              =
   num_users           =
   num_ap_noconsume    =
   num_users_connected = 0;

   db_nodog->callForAllEntryWithSetEntry(WiAuthNodog::countAP);

   U_SRV_LOG("%sdb initialization of wi-auth users WiAuthUser.cdb %s: size(%u) num_users_connected(%u)",
               (result2 ? "" : "WARNING: "), (result2 ? "success" : "failed"), db_user->size(), num_users_connected);

   U_SRV_LOG("%sdb initialization of wi-auth access point WiAuthAccessPoint.cdb %s: size(%u) num_ap(%u) noconsume(%u)",
               (result1 ? "" : "WARNING: "), (result1 ? "success" : "failed"), db_nodog->size(), num_ap, num_ap_noconsume);

   U_SRV_LOG("%sdb initialization of wi-auth virtual access point WiAuthVirtualAccessPoint.cdb %s: size(%u)",
               (result3 ? "" : "WARNING: "), (result2 ? "success" : "failed"), db_ap->size());
}

static void usp_init_wi_auth()
{
   U_TRACE_NO_PARAM(5, "::usp_init_wi_auth()")

   U_NEW(UString, ap_ref,   UString(100U));
   U_NEW(UString, ap_label, UString);

#ifndef U_ALIAS
   U_ERROR("Sorry, I can't run the USP wi_auth because alias URI support is missing, please recompile ULib");
#endif

   U_NEW(UPing, sockp, UPing(5000, UClientImage_Base::bIPv6));

   if (sockp->initPing() == false)
      {
      delete sockp;
             sockp = 0;
      }

   U_NEW(UString, ip, UString);
   U_NEW(UString, ap, UString);
   U_NEW(UString, ts, UString);
   U_NEW(UString, uid, UString);
   U_NEW(UString, mac, UString);
   U_NEW(UString, realm, UString);
   U_NEW(UString, label, UString);
   U_NEW(UString, yearName, UString(U_STRING_FROM_CONSTANT("year")));
   U_NEW(UString, monthName, UString(U_STRING_FROM_CONSTANT("month")));
   U_NEW(UString, weekName, UString(U_STRING_FROM_CONSTANT("week")));
   U_NEW(UString, pbody, U_STRING_FROM_CONSTANT("reboot=+Yes%2C+really+reboot+now+"));
   U_NEW(UString, redir, UString);
   U_NEW(UString, token, UString);
   U_NEW(UString, output, UString);
   U_NEW(UString, policy, UString);
   U_NEW(UString, gateway, UString);
   U_NEW(UString, ip_auth, U_STRING_FROM_CONSTANT("IP_AUTH"));
   U_NEW(UString, redirect, UString);
   U_NEW(UString, mac_auth, U_STRING_FROM_CONSTANT("MAC_AUTH"));
   U_NEW(UString, ip_server, UString(UServer_Base::getIPAddress()));
   U_NEW(UString, empty_str, U_STRING_FROM_CONSTANT("\"\""));
   U_NEW(UString, url_nodog, UString(U_CAPACITY));
   U_NEW(UString, buffer_srv, UString(U_CAPACITY));
   U_NEW(UString, buffer_data, UString(U_CAPACITY));
   U_NEW(UString, cert_auth, U_STRING_FROM_CONSTANT("CERT_AUTH"));
   U_NEW(UString, str_ffffff, U_STRING_FROM_CONSTANT("ffffff"));
   U_NEW(UString, nodog_conf, UString(UFile::contentOf(U_STRING_FROM_CONSTANT("ap/nodog.conf.template"))));
   U_NEW(UString, logout_url, UString(200U));
   U_NEW(UString, ap_address, UString);
   U_NEW(UString, empty_list, U_STRING_FROM_CONSTANT("()"));
   U_NEW(UString, auth_domain, UString);
   U_NEW(UString, ap_hostname, UString);
   U_NEW(UString, cookie_auth, U_STRING_FROM_CONSTANT("COOKIE_AUTH_"));
   U_NEW(UString, account_auth, U_STRING_FROM_CONSTANT("ACCOUNT_AUTH"));
   U_NEW(UString, user_UploadRate, UString(10U));
   U_NEW(UString, user_DownloadRate, UString(10U));
   U_NEW(UString, allowed_web_hosts, UString);
   U_NEW(UString, dir_server_address, UString(200U));
   U_NEW(UString, max_time_no_traffic, UString);

   U_NEW(UString, time_done, UString(20U));
   U_NEW(UString, time_counter, UString(20U));
   U_NEW(UString, time_consumed, UString(20U));
   U_NEW(UString, traffic_done, UString(20U));
   U_NEW(UString, traffic_counter, UString(20U));
   U_NEW(UString, traffic_consumed, UString(20U));

   U_NEW(UTimeDate, date, UTimeDate);

   U_INTERNAL_ASSERT_POINTER(USSIPlugIn::environment)

   U_NEW(UString, environment, UString(*USSIPlugIn::environment));
   U_NEW(UString, virtual_name, UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("VIRTUAL_NAME"),  environment)));

   (void) environment->append(U_CONSTANT_TO_PARAM("VIRTUAL_HOST="));
   (void) environment->append(*virtual_name);

   UString dir_root = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("DIR_ROOT"), environment);

   U_NEW(UString, dir_reg, UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("DIR_REG"), environment)));
   U_NEW(UString, title_default, UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("TITLE_DEFAULT"), environment)));
   U_NEW(UString, historical_log_dir, UString(UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("HISTORICAL_LOG_DIR"), environment)));

   UHTTP::setUploadDir(*historical_log_dir);

   dir_server_address->snprintf(U_CONSTANT_TO_PARAM("%v/client"), dir_root.rep);

   UString tmp1 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("LDAP_CARD_PARAM"),    environment),
           tmp2 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("WIAUTH_CARD_BASEDN"), environment);

   U_NEW(UString, ldap_card_param, UString(UStringExt::expandEnvironmentVar(tmp1, environment)));
   U_NEW(UString, wiauth_card_basedn, UString(UStringExt::expandEnvironmentVar(tmp2, environment)));

   tmp1 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("LDAP_USER_PARAM"),    environment),
   tmp2 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("WIAUTH_USER_BASEDN"), environment);

   U_NEW(UString, ldap_user_param, UString(UStringExt::expandEnvironmentVar(tmp1, environment)));
   U_NEW(UString, wiauth_user_basedn, UString(UStringExt::expandEnvironmentVar(tmp2, environment)));

   tmp1 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("LDAP_SESSION_PARAM"),    environment),
   tmp2 = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("WIAUTH_SESSION_BASEDN"), environment);

   U_NEW(UString, ldap_session_param, UString(UStringExt::expandEnvironmentVar(tmp1, environment)));
   U_NEW(UString, wiauth_session_basedn, UString(UStringExt::expandEnvironmentVar(tmp2, environment)));

   UString content = UFile::contentOf(U_STRING_FROM_CONSTANT("$DIR_ROOT/etc/AllowedWebHosts.txt"), O_RDONLY, false, environment);

   if (content)
      {
      UVector<UString> vec(content);

      if (vec.empty() == false) *allowed_web_hosts = vec.join(' ') + ' ';
      }

   U_NEW(UCache,        cache, UCache);
   U_NEW(UCache,  admin_cache, UCache);
   U_NEW(UCache, policy_cache, UCache);

   UString x(U_CAPACITY);

   x.snprintf(U_CONSTANT_TO_PARAM("$DIR_ROOT/etc/%v/cache.tmpl"), virtual_name->rep);

   (void) cache->open(x, U_STRING_FROM_CONSTANT("$DIR_TEMPLATE"), environment, true);

   x.snprintf(U_CONSTANT_TO_PARAM("$DIR_ROOT/etc/%v/policy_cache.tmpl"), virtual_name->rep);

   (void) policy_cache->open(x, U_STRING_FROM_CONSTANT("$DIR_POLICY"), environment, true);

   x.snprintf(U_CONSTANT_TO_PARAM("$DIR_ROOT/etc/%v/admin_cache.tmpl"), ip_server->rep);

   (void) admin_cache->open(x, U_STRING_FROM_CONSTANT("$DIR_ADMIN_TEMPLATE"), environment, true);

   U_NEW(UString, message_page_template, UString(cache->getContent(U_CONSTANT_TO_PARAM("message_page.tmpl"))));
   U_NEW(UString, login_nodog_template, UString(admin_cache->getContent(U_CONSTANT_TO_PARAM("login_nodog_body.tmpl"))));
   U_NEW(UString, status_network_template, UString(admin_cache->getContent(U_CONSTANT_TO_PARAM("status_network_body.tmpl"))));
   U_NEW(UString, status_nodog_template, UString(admin_cache->getContent(U_CONSTANT_TO_PARAM("status_nodog_body.tmpl"))));
   U_NEW(UString, status_nodog_ap_template, UString(admin_cache->getContent(U_CONSTANT_TO_PARAM("status_nodog_body_ap.tmpl"))));
   U_NEW(UString, status_nodog_and_user_body_template, UString(admin_cache->getContent(U_CONSTANT_TO_PARAM("status_nodog_and_user_body.tmpl"))));

   x.snprintf(U_CONSTANT_TO_PARAM("$DIR_ROOT/etc/%v/script.conf"), virtual_name->rep);

   content = UFile::contentOf(x, O_RDONLY, false, environment);

   U_NEW(UHashMap<UString>, table,  UHashMap<UString>);
   U_NEW(UHashMap<UString>, table1, UHashMap<UVectorUString>);

   if (UFileConfig::loadProperties(*table, content))
      {
      U_NEW(UString, telefono,         UString((*table)["TELEFONO"]));
      U_NEW(UString, fmt_auth_cmd,     UString((*table)["FMT_AUTH_CMD"]));
      U_NEW(UString, redirect_default, UString((*table)["REDIRECT_DEFAULT"]));

      U_NEW(UString, url_banner_ap,     UString(UStringExt::expandPath((*table)["URL_BANNER_AP"],     environment)));
      U_NEW(UString, url_banner_comune, UString(UStringExt::expandPath((*table)["URL_BANNER_COMUNE"], environment)));

      U_NEW(UString,          help_url, UString(UStringExt::expandEnvironmentVar((*table)["HELP_URL"],          environment)));
      U_NEW(UString,         login_url, UString(UStringExt::expandEnvironmentVar((*table)["LOGIN_URL"],         environment)));
      U_NEW(UString,        wallet_url, UString(UStringExt::expandEnvironmentVar((*table)["WALLET_URL"],        environment)));
      U_NEW(UString,      password_url, UString(UStringExt::expandEnvironmentVar((*table)["PASSWORD_URL"],      environment)));
      U_NEW(UString, registrazione_url, UString(UStringExt::expandEnvironmentVar((*table)["REGISTRAZIONE_URL"], environment)));

#  ifdef USE_LIBSSL
      U_NEW(UString, des3_key, UString((*table)["DES3_KEY"]));

      UDES3::setPassword(des3_key->c_str());
#  endif

      logout_url->snprintf(U_CONSTANT_TO_PARAM("%v/logout"), login_url->rep);
      }

   table->clear();

   // POLICY

   U_NEW(UString, time_available, UString);
   U_NEW(UString, traffic_available, UString);

   U_NEW(UString, policy_flat, U_STRING_FROM_CONSTANT("FLAT"));
   U_NEW(UString, policy_daily, U_STRING_FROM_CONSTANT("DAILY"));
   U_NEW(UString, policy_traffic, U_STRING_FROM_CONSTANT("TRAFFIC"));

   WiAuthUser::loadPolicy(*policy_daily); // NB: time_available e traffic_available sono valorizzati da loadPolicy()...

      time_available_daily =    time_available->strtoul();
   traffic_available_daily = traffic_available->strtoull();

   WiAuthUser::loadPolicy(*policy_flat); // NB: time_available e traffic_available sono valorizzati da loadPolicy()...

      time_available_flat =    time_available->strtoul();
   traffic_available_flat = traffic_available->strtoull();

   // HTTP client

   U_NEW(UHttpClient<UTCPSocket>, client, UHttpClient<UTCPSocket>(0));

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

   U_NEW(UFile, file_LOG, UFile(U_STRING_FROM_CONSTANT("$FILE_LOG"), environment));

   UString dir = UStringExt::dirname(file_LOG->getPath());

   U_NEW(UFile, file_INFO,     UFile(dir + U_STRING_FROM_CONSTANT("/wifi-info")));
   U_NEW(UFile, file_WARNING,  UFile(dir + U_STRING_FROM_CONSTANT("/wifi-warning")));
   U_NEW(UFile, file_RECOVERY, UFile(dir + U_STRING_FROM_CONSTANT("/wifi-recovery")));
   U_NEW(UFile, file_UTILIZZO, UFile(dir + U_STRING_FROM_CONSTANT("/wifi-utilizzo")));

   UServer_Base::update_date  =
   UServer_Base::update_date1 = true;

   (void) UServer_Base::addLog(file_LOG);
   (void) UServer_Base::addLog(file_INFO);
   (void) UServer_Base::addLog(file_WARNING);
   (void) UServer_Base::addLog(file_RECOVERY, O_APPEND | O_RDWR);
   (void) UServer_Base::addLog(file_UTILIZZO, O_APPEND | O_RDWR);

   U_NEW(UVector<UString>, vuid, UVector<UString>);
   U_NEW(UVector<UIPAllow*>, vallow_IP_user, UVector<UIPAllow*>);
   U_NEW(UVector<UIPAllow*>, vallow_IP_request, UVector<UIPAllow*>);

   (void) x.assign(U_CONSTANT_TO_PARAM("172.0.0.0/8, " IP_UNIFI ", " IP_UNIFI_TMP ", " IP_CASCINE)); // NB: unifi and cascine has MasqueradeDevice...

   (void) UIPAllow::parseMask(x, *vallow_IP_request);

   // RECORD - DB

   U_NEW(WiAuthDataStorage, data_rec, WiAuthDataStorage);

   if (UHTTP::data_session == 0) U_NEW(UDataSession, UHTTP::data_session, UDataSession);

   if (UHTTP::db_session == 0) UHTTP::initSession();

   UHTTP::db_session->setPointerToDataStorage(data_rec);

   if (UHTTP::db_session->getDataStorage() == false)
      {
      data_rec->reset();

      (void) UHTTP::db_session->putDataStorage();

      U_LOGGER("*** DATA STORAGE EMPTY ***");
      }

   U_NEW(WiAuthUser,              user_rec, WiAuthUser);
   U_NEW(WiAuthNodog,            nodog_rec, WiAuthNodog);
   U_NEW(WiAuthVirtualAccessPoint, vap_rec, WiAuthVirtualAccessPoint);

   U_NEW(URDBObjectHandler<UDataStorage*>, db_user,  URDBObjectHandler<UDataStorage*>(U_STRING_FROM_CONSTANT("../db/WiAuthUser.cdb"),               -1,  user_rec));
   U_NEW(URDBObjectHandler<UDataStorage*>, db_nodog, URDBObjectHandler<UDataStorage*>(U_STRING_FROM_CONSTANT("../db/WiAuthAccessPoint.cdb"),        -1, nodog_rec));
   U_NEW(URDBObjectHandler<UDataStorage*>, db_ap,    URDBObjectHandler<UDataStorage*>(U_STRING_FROM_CONSTANT("../db/WiAuthVirtualAccessPoint.cdb"), -1,   vap_rec));

   // POSIX shared memory object (interprocess - can be used by unrelated processes (userver_tcp and userver_ssl)

      db_ap->setShared(0, 0);
    db_user->setShared(0, 0);
   db_nodog->setShared(0, 0);

   (void) UFile::mkfifo(NAMED_PIPE, PERM_FILE);

   db_open();

   // TAVARNELLE

   U_NEW(UString, db_filter_tavarnelle, UString(UFile::contentOf(U_STRING_FROM_CONSTANT("../tavarnelle.rule"))));

   if (*db_filter_tavarnelle) *db_filter_tavarnelle = UStringExt::trim(*db_filter_tavarnelle);

   // ANAGRAFICA

   content = UFile::contentOf(U_STRING_FROM_CONSTANT("../anagrafica.txt"));

   if (content) U_NEW(UString, db_anagrafica, UString(content));

   U_INTERNAL_ASSERT_POINTER(db_anagrafica)

   (void) memcpy(UServices::key, U_CONSTANT_TO_PARAM("1234567890123456")); // for ULib session cookies... 
}

static void usp_sighup_wi_auth()
{
   U_TRACE_NO_PARAM(5, "::usp_sighup_wi_auth()")

   if (db_nodog)
      {
      int fd;

         db_ap->close();
       db_user->close();
      db_nodog->close();

      if (UServer_Base::bssl)
         {
         fd = UFile::open(NAMED_PIPE, O_WRONLY, PERM_FILE);

         if (fd != -1) UFile::close(fd);
         }
      else
         {
         fd = UFile::open(NAMED_PIPE, O_RDONLY, PERM_FILE);

         if (fd != -1) UFile::close(fd);
         }

      db_open();
      }
}

static void usp_end_wi_auth()
{
   U_TRACE_NO_PARAM(5, "::usp_end_wi_auth()")

   if (db_nodog)
      {
#  ifdef DEBUG
      delete ip;
      delete ap;
      delete ts;
      delete uid;
      delete mac;
      delete realm;
      delete pbody;
      delete redir;
      delete token;
      delete label;
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
      delete empty_str;
      delete url_nodog;
      delete cert_auth;
      delete ap_address;
      delete str_ffffff;
      delete nodog_conf;
      delete empty_list;
      delete buffer_srv;
      delete buffer_data;
      delete cookie_auth;
      delete ap_hostname;
      delete auth_domain;
      delete environment;
      delete policy_flat;
      delete policy_daily;
      delete account_auth;
      delete virtual_name;
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
      delete status_nodog_and_user_body_template;

      delete time_done;
      delete time_counter;
      delete time_consumed;
      delete time_available;
      delete traffic_done;
      delete traffic_counter;
      delete traffic_consumed;
      delete traffic_available;

      delete date;
      delete yearName;
      delete monthName;
      delete weekName;

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
         }

      delete vuid;
      delete table;
      delete table1;
      delete cache;
      delete client;
      delete vallow_IP_user;
      delete vallow_IP_request;
      delete admin_cache;
      delete policy_cache;
#  endif

      delete   vap_rec;
      delete  data_rec;
      delete  user_rec;
      delete nodog_rec;
      delete db_filter_tavarnelle;
      }

   (void) UFile::_unlink(NAMED_PIPE);
}

static bool checkIfUserConnected()
{
   U_TRACE_NO_PARAM(5, "::checkIfUserConnected()")

   U_INTERNAL_DUMP("uid = %V", uid->rep)

   user_exist = false;

   if (uid->empty()) db_user->callForAllEntry(WiAuthUser::checkIfUserExist);
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

static bool runAuthCmd(const char* password, const char* prealm)
{
   U_TRACE(5, "::runAuthCmd(%S,%S)", password, prealm)

   U_INTERNAL_ASSERT(*fmt_auth_cmd)

   static int fd_stderr;

   UString cmd(U_CAPACITY);

   if (uid->size() > 32) goto error;

   cmd.snprintf(U_STRING_TO_PARAM(*fmt_auth_cmd), uid->c_str(), password, prealm);

   if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/auth_cmd.err");

   *output = UCommand::outputCommand(cmd, 0, -1, fd_stderr);

   UServer_Base::logCommandMsgError(cmd.data(), true);

   if (UCommand::exit_value ||
       output->empty())
      {
      U_LOGGER("*** AUTH_CMD failed: EXIT_VALUE=%d RESPONSE=%V - realm=%s uid=%v pass=%s ***", UCommand::exit_value, output->rep, prealm, uid->rep, password);

error:
      char msg[4096];
      const char* title;

      (void) u__snprintf(msg, sizeof(msg),
               U_CONSTANT_TO_PARAM("%s<br>"
               "<a href=\"javascript:history.go(-1)\">Indietro</a>"),
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

#ifndef USE_LIBSSL
   UString data(100U);
   data.snprintf(U_CONSTANT_TO_PARAM("ip=%v&mac=%v"), _ip.rep, _mac.rep);
#else
   UString data = UDES3::signData(U_CONSTANT_TO_PARAM("ip=%v&mac=%v"), _ip.rep, _mac.rep);
#endif

   UString result = nodog_rec->sendRequestToNodog(U_CONSTANT_TO_PARAM("logout?%v"), data.rep);

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
                            U_CONSTANT_TO_PARAM("%v: *** LOGOUT %%s AFTER %%d ATTEMPTS: UID(%v) IP(%v) MAC(%v) AP(%v) ***"),
                            UClientImage_Base::request_uri->rep, uid->rep, _ip.rep, _mac.rep, ap_address->rep);

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
   U_TRACE_NO_PARAM(5, "::setAccessPointAddress()")

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

static bool setAccessPoint(bool localization, const UString& hostname, const UString& address, bool binsert)
{
   U_TRACE(5, "::setAccessPoint(%b,%V,%V,%b)", localization, hostname.rep, address.rep, binsert)

   uint32_t pos;
   int ap_port = 5280;

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
          ap_port    = address.substr(pos+1).strtoul();
         }

      if ((ap_address_trust = ap_address->isBase64Url())) *ap_address = UDES3::getSignedData(*ap_address);

      if (u_isIPv4Addr(U_STRING_TO_PARAM(*ap_address)) == false)
         {
         U_LOGGER("*** AP ADDRESS(%v) NOT VALID ***", ap_address->rep);

         U_RETURN(false);
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
   if (nodog_rec->setRecord(ap_port, binsert)) U_RETURN(true);

   U_RETURN(false);
}

static bool setAccessPoint(bool localization, bool binsert)
{
   U_TRACE(5, "::setAccessPoint(%b,%b)", localization, binsert)

   // $1 -> ap
   // $2 -> public address to contact the access point
   // $3 -> pid (0 => start)

            ap->clear();
      ap_label->clear();
    ap_address->clear();
   ap_hostname->clear();

   uint32_t end = UHTTP::processForm();

   if (end)
      {
      UString hostname, address;

      UHTTP::getFormValue(hostname, U_CONSTANT_TO_PARAM("ap"),     0, 1, end);
      UHTTP::getFormValue(address,  U_CONSTANT_TO_PARAM("public"), 0, 3, end);

      return setAccessPoint(localization, hostname, address, binsert);
      }

   U_RETURN(false);
}

static void setAccessPointReference(const char* s, uint32_t n)
{
   U_TRACE(5, "::setAccessPointReference(%.*S,%u)", n, s, n)

   ap_ref->setBuffer(100U);

   if (ap_label->empty()) (void) ap_label->assign(U_CONSTANT_TO_PARAM("ap"));

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

   ap_ref->snprintf(U_CONSTANT_TO_PARAM("X%04dR%v"), certid, ap_label->rep);

   U_INTERNAL_DUMP("ap_ref = %V", ap_ref->rep)
}

static bool checkLoginRequest(uint32_t n, uint32_t end, int tolerance, bool bempty, bool binsert)
{
   U_TRACE(5, "::checkLoginRequest(%u,%u,%d,%b,%b)", n, end, tolerance, bempty, binsert)

            ap->clear();
            ip->clear();
            ts->clear();
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

      if (*mac &&
          nodog_rec->setRecord(5280, binsert))
         {
         UHTTP::getFormValue(*ts, U_CONSTANT_TO_PARAM("ts"), 0, 15, n);

         U_RETURN(true);
         }
      }

   U_RETURN(false);
}

static void getLoginRequest(UString& timeout)
{
   U_TRACE(5, "::getLoginRequest(%p)", &timeout)

   uint32_t n = UHTTP::processForm();

   if (checkLoginRequest(n, 18, 18, false, false))
      {
      UHTTP::getFormValue(timeout, U_CONSTANT_TO_PARAM("timeout"), 0,  9, n);
      UHTTP::getFormValue(*token,  U_CONSTANT_TO_PARAM("token"),   0, 11, n);
      UHTTP::getFormValue(*ap,     U_CONSTANT_TO_PARAM("ap"),      0, 13, n);
      }

   UHTTP::getFormValue(*realm, U_CONSTANT_TO_PARAM("realm"),    0, 15, n);
   UHTTP::getFormValue(*redir, U_CONSTANT_TO_PARAM("redir_to"), 0, 17, n);

   if (realm->empty()) *realm = U_STRING_FROM_CONSTANT("all");
}

static bool checkLoginValidate(bool all)
{
   U_TRACE(5, "::checkLoginValidate(%b)", all)

   U_INTERNAL_DUMP("redirect = %V", redirect->rep)

   if (redirect->empty())
      {
      U_LOGGER("*** checkLoginValidate(%b) FAILED: redirect empty ***", 0);

      U_RETURN(false);
      }

   uint32_t sz = redirect->size();

   if (sz <= 32)
      {
      *uid    = *redirect;
      *redir  = *redirect_default;
      *policy = *policy_daily;
      }
   else
      {
      policy->clear();

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

      for (const char* end = str.pend(); ptr3 < end; ++ptr3)
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
         UHTTP::getFormValue(*ap,    U_CONSTANT_TO_PARAM("ap"),    0, 13, 14);

         if (setAccessPointAddress() == false) U_RETURN(false);
         }
      }

   redirect->clear();

   U_RETURN(true);
}

static void sendLoginValidate()
{
   U_TRACE_NO_PARAM(5, "::sendLoginValidate()")

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
      U_CONSTANT_TO_PARAM("uid=%v&policy=%v&auth_domain=%v&max_time=%v&max_traffic=%v&UserDownloadRate=%v&UserUploadRate=%v&redir_to=%v"),
      uid->rep, policy->rep, auth_domain->rep, time_available->rep, traffic_available->rep, user_DownloadRate->rep, user_UploadRate->rep, redir->rep);

   // -------------------------------------------------------------------------------------------------------------------
   // NB: in questo modo l'utente ripassa dal firewall e NoDog lo rimanda da noi (login_validate) con i dati rinnovati
   // -------------------------------------------------------------------------------------------------------------------
   // Nel caso che l'utente sia già in internet ho predisposto un servizio di redirect sulla tnet concentratore che
   // redirige sul portale autorizzativo al servizio 'fake_login_validate' che semplicemente estrae la url di redirect
   // contenuta nella URL di richiesta e fa semplicemente la redirect sulla stessa
   // -------------------------------------------------------------------------------------------------------------------

   USSIPlugIn::setAlternativeRedirect(LOGIN_VALIDATE_REDIR_FI, signed_data.rep);
}

// GET

static void GET_fake_login_validate()
{
   U_TRACE_NO_PARAM(5, "::GET_fake_login_validate()")

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

static void GET_admin()
{
   U_TRACE_NO_PARAM(5, "::GET_admin()")

   USSIPlugIn::setAlternativeRedirect("https://%v/admin.html", ip_server->rep);
}

static void GET_admin_continuing_status_ap()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_continuing_status_ap()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
}

static void GET_admin_current_status_ap()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_current_status_ap()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
}

static void setBufferForAdminEditAP()
{
   U_TRACE_NO_PARAM(5, "::setBufferForAdminEditAP()")

   if (db_user_filter_tavarnelle) (void) buffer_srv->assign(U_CONSTANT_TO_PARAM("tavarnelle.shtml?admin_edit_ap=0&"));
   else                           (void) buffer_srv->assign(U_CONSTANT_TO_PARAM(                 "admin_edit_ap=?"));
}

static void GET_admin_edit_ap()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_edit_ap()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      // $1 -> ap (with localization => '@')
      // $2 -> public address to contact the access point

      if (setAccessPoint(true, false))
         {
         WiAuthAccessPoint* ap_rec = nodog_rec->vec_access_point[index_access_point];

         setBufferForAdminEditAP();

         USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_edit_ap.tmpl")), 0, false,
                                           title_default->data(), 0, 0,
                                           buffer_srv->rep,
                                           ap_rec->_label.rep,
                                           ap_address->rep,
                                           nodog_rec->hostname.rep,
                                           ap_rec->mac_mask.rep,
                                           ap_rec->group_account_mask.rep,
                                           ap_rec->noconsume ? "" : "checked");
         }
      else
         {
         USSIPlugIn::setBadRequest();

         /*
         if (UHTTP::form_name_value->empty() == false) USSIPlugIn::setBadRequest();
         else
            {
            UString tmp1 = U_STRING_FROM_CONSTANT("ap"),
                    tmp2 = U_STRING_FROM_CONSTANT("10.8.0.xxx"),
                    tmp3 = U_STRING_FROM_CONSTANT("aaa-r29587_bbb");

            USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_edit_ap.tmpl")), 0, false,
                                              title_default->data(), 0, 0,
                                              buffer_srv->rep,
                                              tmp1.rep,
                                              tmp2.rep,
                                              tmp3.rep,
                                              UStringRep::string_rep_null,
                                              UStringRep::string_rep_null,
                                              "checked");
            }
         */
         }
      }
}

static void GET_admin_export_view_using_historical_as_csv()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_export_view_using_historical_as_csv()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      UString body = file_UTILIZZO->_getContent();

      USSIPlugIn::setAlternativeResponse(body);
      }
}

static int login_nodog_compare(const void* p, const void* q)
{
   U_TRACE(5, "::login_nodog_compare(%p,%p)", p, q)

   uint32_t i, _totale1 = 0, _totale2 = 0;

   (void) ap_address->assign(U_STRING_TO_PARAM(*(UStringRep*)p));

   (void) db_nodog->getDataStorage(*ap_address);

   U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

   for (i = 0; i < nodog_rec->sz; ++i) _totale1 += nodog_rec->vec_access_point[i]->num_login;

   (void) ap_address->assign(U_STRING_TO_PARAM(*(UStringRep*)q));

   (void) db_nodog->getDataStorage(*ap_address);

   U_INTERNAL_ASSERT_MAJOR(nodog_rec->sz, 0)

   for (i = 0; i < nodog_rec->sz; ++i) _totale2 += nodog_rec->vec_access_point[i]->num_login;

   U_RETURN(_totale2 < _totale1);
}

static void GET_admin_login_nodog()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_login_nodog()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      totale4 = 0;
      totale1 = totale2 = totale3 = 0;
      num_ap = num_users_connected = 0;

      output->setBuffer(U_CAPACITY);

      db_nodog->callForAllEntryWithSetEntry(WiAuthNodog::setLoginNodogTotal);
      db_nodog->callForAllEntryWithSetEntry(WiAuthNodog::setLoginNodog, 0, login_nodog_compare);

      USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_login_nodog.tmpl")), output->size(), false,
                            "stato accesso utenti", 0, 0,
                            num_ap, num_users_connected,
                            "FIRENZE CARD",
                            output->rep,
                            totale1, totale2, totale3, (double)totale4 / (1024. * 1024. * 1024.));
      }
}

static UString printMonth(int month, int year)
{
   U_TRACE(5, "::printMonth(%d,%d)", month, year)

   char buffer[4096];
   UTimeDate curr = *date;
   UString result(U_CAPACITY);
   bool btoday = false, bmonth = false, byear = (curr.getYear() == year);

   U_INTERNAL_DUMP("byear = %b", byear)

   curr.setMondayPrevMonth(month);

   (void) result.assign(U_CONSTANT_TO_PARAM("<table><tr><th>Mon</th><th>Tue</th><th>Wed</th><th>Thu</th><th>Fri</th><th>Sat</th><th>Sun</th></tr>\n"));

   for (int i = 0; i < 42; ++curr)
      {
      if (byear)
         {
         bmonth = (curr.getMonth()  == month);
         btoday = (curr.getJulian() == today);

         U_INTERNAL_DUMP("bmonth = %b btoday = %b", bmonth, btoday)
         }

      if ((i % 7) == 0) (void) result.append(U_CONSTANT_TO_PARAM("<tr>\n"));

      (void) result.append(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("<td%.*s><a href=\"%vt=%u&i=86400\">%.*s%u%.*s</a></td>\n"),
                              (bmonth          ? 0 : U_CONSTANT_SIZE(" class=\"inactive\""))," class=\"inactive\"", buffer_srv->rep, curr.getSecond(),
                              (btoday == false ? 0 : U_CONSTANT_SIZE("<strong>")),"<strong>", curr.getDay(),
                              (btoday == false ? 0 : U_CONSTANT_SIZE("</strong>")),"</strong>"));

      if ((++i % 7) == 0) (void) result.append(U_CONSTANT_TO_PARAM("</tr>\n"));
      }

   (void) result.append(U_CONSTANT_TO_PARAM("</table>\n"));

   U_RETURN_STRING(result);
}

static void GET_calendar()
{
   U_TRACE_NO_PARAM(5, "::GET_calendar()")

   // $1 -> type (week, month, year)
   // $2 -> year
   // $3 -> week, month

   uint32_t end = UHTTP::processForm();

   if (end < 4)
      {
      USSIPlugIn::setBadRequest();

      return;
      }

   UTimeDate last;
   const char* name_month = 0;
   int i, year, month, week, day, lastyear = 0;
   UString type, pyear, param3, nextURL(100U), prevURL(100U);

   UHTTP::getFormValue(type,   1);
   UHTTP::getFormValue(pyear,  3);
   UHTTP::getFormValue(param3, 5);

   bool bweek  = (                          type ==  *weekName),
        bmonth = (bweek           ? false : type == *monthName),
        byear  = (bweek || bmonth ? false : true);

   date->setCurrentDate();

   if (bweek)
      {
      if (pyear &&
          param3)
         {
         date->setYearAndWeek(pyear.strtoul(), param3.strtoul());
         }

      name_month = date->getMonthName();

      (last = *date).addDays(6);

      lastyear = last.getYear();

      UTimeDate next, prev;

      (next = *date).addDays( 7);
      (prev = *date).addDays(-7);

      nextURL.snprintf(U_CONSTANT_TO_PARAM("?type=week&year=%u&week=%u"), next.getYear(), next.getWeekOfYear());
      prevURL.snprintf(U_CONSTANT_TO_PARAM("?type=week&year=%u&week=%u"), prev.getYear(), prev.getWeekOfYear());
      }
   else if (bmonth)
      {
      today = date->getJulian();

      if (pyear &&
          param3)
         {
         date->set(1, param3.strtoul(), pyear.strtoul());
         }

      name_month = date->getMonthName();

      UTimeDate next, prev;

      (next = *date).addMonths( 1);
      (prev = *date).addMonths(-1);

      nextURL.snprintf(U_CONSTANT_TO_PARAM("?type=month&year=%u&month=%u"), next.getYear(), next.getMonth());
      prevURL.snprintf(U_CONSTANT_TO_PARAM("?type=month&year=%u&month=%u"), prev.getYear(), prev.getMonth());
      }
   else
      {
      today = date->getJulian();

      if (pyear) date->setYear(pyear.strtoul());

      UTimeDate next, prev;

      (next = *date).addYears( 1);
      (prev = *date).addYears(-1);

      nextURL.snprintf(U_CONSTANT_TO_PARAM("?type=year&year=%u"), next.getYear());
      prevURL.snprintf(U_CONSTANT_TO_PARAM("?type=year&year=%u"), prev.getYear());
      }

   day   = date->getDay();
   month = date->getMonth();
   year  = date->getYear();
   week  = date->getWeekOfYear();

   output->setBuffer(U_CAPACITY);

   output->snprintf(U_CONSTANT_TO_PARAM(
   "<!DOCTYPE html>"
   "<html lang=\"en\">"
     "<head>"
     "<title>Calendario</title>"
     "<meta charset=\"utf-8\" />"
     "<link rel=\"stylesheet\" href=\"css/calendar.css\" type=\"text/css\" />"
   "</head>"
   "<body>"
   "<div><center><h1>SELEZIONA UNA DATA DAL CALENDARIO</h1></center></div>"
   "<header>"
    "<nav>"
      "<ul>"
        "<li><a %.*s href=\"?type=week&year=%u&week=%u\" >Week</a></li>"
        "<li><a %.*s href=\"?type=month&year=%u&month=%u\" >Month</a></li>"
        "<li><a %.*s href=\"?type=year&year=%u\" >Year</a></li>"
      "</ul>"
    "</nav>"
   "</header>"
   "<section class=\"%v\">"
   "<h1>"
    "<a class=\"arrow\" href=\"%v\">&larr; </a>"),
   (bweek  ? U_CONSTANT_SIZE(" class=\"active\"") : 0), " class=\"active\"", year, week,
   (bmonth ? U_CONSTANT_SIZE(" class=\"active\"") : 0), " class=\"active\"", year, month,
   (byear  ? U_CONSTANT_SIZE(" class=\"active\"") : 0), " class=\"active\"", year,
   type.rep,
   prevURL.rep);

   if (byear)
      {
      UStringExt::appendNumber32(*output, year);
      }
   else if (bmonth)
      {
      output->snprintf_add(U_CONSTANT_TO_PARAM("%s <a href=\"?type=year&year=%u\">%u</a> "), name_month, year, year);
      }
   else
      {
      output->snprintf_add(U_CONSTANT_TO_PARAM("%02u <a href=\"?type=month&year=%u&month=%u\">%s</a> <a href=\"?type=year&year=%u\">%u</a> \xE2\x80\x93 "
                           "%02u <a href=\"?type=month&year=%u&month=%u\">%s</a> <a href=\"?type=year&year=%u\">%u</a>"),
                                    day,      year,           month,          name_month,     year,     year,
                           last.getDay(), lastyear, last.getMonth(), last.getMonthName(), lastyear, lastyear);
      }

   output->snprintf_add(U_CONSTANT_TO_PARAM(
                        "<a class=\"arrow\" href=\"%v\"> &rarr;</a>"
                        "</h1>"),
                        nextURL.rep);

   if (db_user_filter_tavarnelle) (void) buffer_srv->assign(U_CONSTANT_TO_PARAM("tavarnelle.shtml?admin_login_nodog_historical_view_data=0&"));
   else                           (void) buffer_srv->assign(U_CONSTANT_TO_PARAM(                 "admin_login_nodog_historical_view_data?"));

   UString buffer(U_CAPACITY);

   if (byear)
      {
      (void) output->append(U_CONSTANT_TO_PARAM("<ul>\n"));

      date->setMonth((i = 1));

      while (true)
         {
         buffer.snprintf(U_CONSTANT_TO_PARAM("<li><h2><a href=\"?type=month&year=%u&month=%u\">%s</a></h2>"), year, i, u_month_name[i-1]);

         (void) output->append(buffer);
         (void) output->append(printMonth(i, year));
         (void) output->append(U_CONSTANT_TO_PARAM("</li>"));

         if (++i > 12) break;

                 date->addMonths(1);
         month = date->getMonth();
         }

      (void) output->append(U_CONSTANT_TO_PARAM("</ul>\n"));
      }
   else if (bmonth)
      {
      (void) output->append(printMonth(month, year));
      }
   else
      {
      UTimeDate curr = *date;

      (void) output->append(U_CONSTANT_TO_PARAM("<table><tr>\n"));

      for (i = 0; i < 7; ++i,++curr) buffer.snprintf_add(U_CONSTANT_TO_PARAM("<th>%s, %u</th>\n"), curr.getDayName(), curr.getDay());

      (void) output->append(buffer);
      (void) output->append(U_CONSTANT_TO_PARAM("</tr><tr>\n"));

      buffer.setBuffer(U_CAPACITY);

      curr = *date;

      for (i = 0; i < 7; ++i,++curr)
         {
         uint32_t sec = curr.getSecond();

         buffer.snprintf(U_CONSTANT_TO_PARAM(
         "<td><ul>\n"
         "<li><a href=\"%vt=%u&i=3600\">00:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">01:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">02:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">03:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">04:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">05:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">06:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">07:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">08:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">09:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">10:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">11:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">12:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">13:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">14:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">15:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">16:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">17:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">18:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">19:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">20:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">21:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">22:00</a></li>"
         "<li><a href=\"%vt=%u&i=3600\">23:00</a></li>"
         "</ul></td>\n"),
         buffer_srv->rep, sec,
         buffer_srv->rep, sec+3600,
         buffer_srv->rep, sec+7200,
         buffer_srv->rep, sec+11200,
         buffer_srv->rep, sec+14800,
         buffer_srv->rep, sec+18400,
         buffer_srv->rep, sec+22000,
         buffer_srv->rep, sec+25600,
         buffer_srv->rep, sec+29200,
         buffer_srv->rep, sec+32800,
         buffer_srv->rep, sec+36400,
         buffer_srv->rep, sec+40000,
         buffer_srv->rep, sec+43600,
         buffer_srv->rep, sec+47200,
         buffer_srv->rep, sec+50800,
         buffer_srv->rep, sec+54400,
         buffer_srv->rep, sec+58000,
         buffer_srv->rep, sec+61600,
         buffer_srv->rep, sec+65200,
         buffer_srv->rep, sec+68800,
         buffer_srv->rep, sec+72400,
         buffer_srv->rep, sec+76000,
         buffer_srv->rep, sec+79600,
         buffer_srv->rep, sec+83200);

         (void) output->append(buffer);
         }

      (void) output->append(U_CONSTANT_TO_PARAM("</tr></table>\n"));
      }

   (void) output->append(U_CONSTANT_TO_PARAM("</section>\n</body></html>"));

   USSIPlugIn::setAlternativeResponse(*output);
}

static void GET_admin_login_nodog_historical()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_login_nodog_historical()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      date->setCurrentDate();

      USSIPlugIn::setAlternativeRedirect("calendar?type=year&year=%u", date->getYear());
      }
}

static void GET_admin_login_nodog_historical_view_data()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_login_nodog_historical_view_data()")

   // $1 -> time
   // $2 -> interval

   uint32_t end = UHTTP::processForm();

   if (end < 4)
      {
      USSIPlugIn::setBadRequest();

      return;
      }

   UString time, interval;

   if (db_user_filter_tavarnelle)
      {
      UHTTP::getFormValue(time,     3);
      UHTTP::getFormValue(interval, 5);

      (void) buffer_srv->assign(U_CONSTANT_TO_PARAM("/var/log/wi-auth-status-access/tavarnelle/20160101230001Z.html"));
      }
   else
      {
      UHTTP::getFormValue(time,     1);
      UHTTP::getFormValue(interval, 3);

      (void) buffer_srv->assign(U_CONSTANT_TO_PARAM("/var/log/wi-auth-status-access/20160101230001Z.html"));
      }

   time_t sec = time.strtoul();

   U_INTERNAL_DUMP("buffer_srv(%u) = %V", buffer_srv->size(), buffer_srv->rep)

   char* ptr = buffer_srv->pend() - U_CONSTANT_SIZE("230001Z.html");

   (void) u_strftime2(ptr - U_CONSTANT_SIZE("20160101"), U_CONSTANT_SIZE("20160101"), U_CONSTANT_TO_PARAM("%Y%m%d"), sec);

   static const char* vpath[] = { "220001", "230001", "220002", "230002", "220003", "230003" };

   for (unsigned int i = 0; i < U_NUM_ELEMENTS(vpath); ++i)
      {
      U_MEMCPY(ptr, vpath[i], U_CONSTANT_SIZE("230001"));

      if (UFile::access(buffer_srv->data(), R_OK))
         {
         UString body = UFile::contentOf(*buffer_srv);

         USSIPlugIn::setAlternativeResponse(body);

         return;
         }
      }

   output->setBuffer(U_CAPACITY);

   output->snprintf(U_CONSTANT_TO_PARAM("%#9D: NO DATA available"), sec);

   USSIPlugIn::setAlternativeResponse(*output);
}

static void GET_admin_status_network()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_status_network()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      output->setBuffer(U_CAPACITY);

      user_exist = true;
      num_ap = num_users_connected = num_users_connected_on_nodog = 0;

      db_nodog->callForAllEntryWithSetEntry(WiAuthNodog::countAP);
       db_user->callForAllEntry(WiAuthUser::getStatusUser);

      if (num_users_connected != num_users_connected_on_nodog)
         {
         U_LOGGER("*** THE NUMBER OF USER CONNECTED IS NOT ALIGNED (%u=>%u) ***", num_users_connected, num_users_connected_on_nodog);
         }

      USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_status_network.tmpl")), output->size(), false,
                                        "stato rete", 0, 0,
                                        num_ap, num_users_connected_on_nodog, output->rep);
      }
}

static void GET_admin_status_nodog()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_status_nodog()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      num_ap             =
      num_ap_up          =
      num_ap_down        =
      num_ap_open        =
      num_ap_noconsume   =
      num_ap_unreachable = 0;

      U_INTERNAL_DUMP("num_ap = %u num_ap_up = %u num_ap_down = %u num_ap_unreachable = %u num_ap_open = %u num_ap_noconsume = %u",
                       num_ap,     num_ap_up,     num_ap_down,     num_ap_unreachable,     num_ap_open,     num_ap_noconsume)

      setBufferForAdminEditAP();

      output->setBuffer(U_CAPACITY);

      db_nodog->callForAllEntryWithSetEntry(WiAuthNodog::setStatusNodog, 0, UStringExt::qscompver);

      U_INTERNAL_DUMP("num_ap = %u num_ap_up = %u num_ap_down = %u num_ap_unreachable = %u num_ap_open = %u num_ap_noconsume = %u",
                       num_ap,     num_ap_up,     num_ap_down,     num_ap_unreachable,     num_ap_open,     num_ap_noconsume)

      uint32_t sz = output->size();

      USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_status_nodog.tmpl")), sz, false,
                            "stato access point", 0, 0,
                            num_ap, num_ap_up, num_ap_down, num_ap_unreachable, num_ap_open, num_ap_noconsume,
                            output->rep);
      }
}

static void GET_admin_view_user()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_view_user()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("get_user_id.tmpl")), 0, false,
                            "Visualizzazione dati utente",
                            "<link type=\"text/css\" href=\"css/livevalidation.css\" rel=\"stylesheet\" />"
                            "<script type=\"application/javascript\" src=\"js/livevalidation_standalone.compressed.js\"></script>", 0,
                            "Visualizzazione dati utente", "admin_view_user");
      }
}

static void GET_admin_view_using()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_view_using()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      num_ap              =
      num_ap_noconsume    =
      num_users_connected = 0;

      db_nodog->callForAllEntryWithSetEntry(WiAuthNodog::countAP);

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
   U_TRACE_NO_PARAM(5, "::GET_admin_view_using_historical()")

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

            _totale1 += tmp1.strtoul();
            _totale2 += tmp2.strtoul();
            _totale3 += tmp3.strtoul();

            riga.snprintf(U_CONSTANT_TO_PARAM(
                     "<tr><td class=\"data_smaller\" align=\"right\">%v</td>\n"
                     "<td class=\"data_smaller\" align=\"right\">%v</td>\n"
                     "<td class=\"data_smaller\" align=\"right\">%v</td>\n"
                     "<td class=\"data_smaller\" align=\"right\">%v</td></tr>\n"),
                     tmp0.rep, tmp1.rep, tmp2.rep, tmp3.rep);

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

static void GET_check_for_usersF()
{
   U_TRACE_NO_PARAM(5, "::GET_check_for_usersF()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      db_nodog->callForAllEntryWithSetEntry(WiAuthNodog::checkAccessPointFromAnagrafica);

      U_ASSERT(  vuid->empty())
      U_ASSERT(table1->empty())

      db_user->callForAllEntryWithVector(WiAuthUser::setUsersF);

      if (table1->empty() == false)
         {
         table1->callForAllEntry(WiAuthUser::checkUsersF);

         table1->clear();
         }

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_get_ap_check_firewall()
{
   U_TRACE_NO_PARAM(5, "::GET_get_ap_check_firewall()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      // $1 -> ap (without localization => '@')
      // $2 -> public address to contact the access point

      if (setAccessPoint(false, false)) (void) nodog_rec->sendRequestToNodog(U_CONSTANT_TO_PARAM("checkFirewall"), 0);

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_get_ap_check_zombies()
{
   U_TRACE_NO_PARAM(5, "::GET_get_ap_check_zombies()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      // $1 -> ap (without localization => '@')
      // $2 -> public address to contact the access point

      if (setAccessPoint(false, false)) (void) nodog_rec->sendRequestToNodog(U_CONSTANT_TO_PARAM("checkZombies"), 0);

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_get_ap_name()
{
   U_TRACE_NO_PARAM(5, "::GET_get_ap_name()")

   output->setBuffer(U_CAPACITY);

   db_nodog->callForAllEntryWithSetEntry(WiAuthNodog::getNameAccessPoint);

   USSIPlugIn::setAlternativeResponse(*output);
}

static void GET_get_ap_uptime()
{
   U_TRACE_NO_PARAM(5, "::GET_get_ap_uptime()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      // $1 -> ap (without localization => '@')
      // $2 -> public address to contact the access point

      if (setAccessPoint(false, false))
         {
         // NB: request => http://%s:%u/...", *ap_address, nodog_rec->port...

         UString result = nodog_rec->sendRequestToNodog(U_CONSTANT_TO_PARAM("uptime"), 0);

         if (result &&
             U_IS_HTTP_ERROR(U_http_info.nResponseCode) == false)
            {
            // NB: we may be a different process from what it has updated so that we need to read the record...

            if (db_nodog->getDataStorage(*ap_address))
               {
               U_INTERNAL_DUMP("nodog_rec->start = %ld result = %V", nodog_rec->start, result.rep)

               nodog_rec->start = u_now->tv_sec - result.strtoul();

               (void) db_nodog->putDataStorage();

               U_INTERNAL_DUMP("uptime = %#2D", u_now->tv_sec - nodog_rec->start)
               }
            }
         }

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_get_config()
{
   U_TRACE_NO_PARAM(5, "::GET_get_config()")

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
         UString buffer(U_CAPACITY);

         buffer.snprintf(U_CONSTANT_TO_PARAM("%w/ap/%v/nodog.conf"), key.rep);

         _body = UFile::contentOf(buffer);

         if (_body.empty())
            {
            _body = *nodog_conf;

            UHTTP::getFormValue(*ap, U_CONSTANT_TO_PARAM("ap"), 0, 1, end);

            if (*ap)
               {
               const char* lan =                 "???";
                  uint32_t len = U_CONSTANT_SIZE("???");

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

               _body = UStringExt::substitute(_body, U_CONSTANT_TO_PARAM("<LAN>"), lan, len);
               }

            if (u_isIPv4Addr(U_STRING_TO_PARAM(key))) *ap_address = key;
            else                                      (void) ap_address->assign(U_CLIENT_ADDRESS_TO_PARAM);

            buffer.snprintf(U_CONSTANT_TO_PARAM("%w/ap/%v/nodog.conf.local"), ap_address->rep);

            UString lbl, netmask, local = UFile::contentOf(buffer);

            if (WiAuthNodog::setLabelAndNetmaskFromAnagrafica(lbl, netmask))
               {
               buffer.snprintf(U_CONSTANT_TO_PARAM("LOCAL_NETWORK_LABEL \"%v\""), lbl.rep);

               _body = UStringExt::substitute(_body, U_CONSTANT_TO_PARAM("LOCAL_NETWORK_LABEL ap"), U_STRING_TO_PARAM(buffer));
               _body = UStringExt::substitute(_body, U_CONSTANT_TO_PARAM("172.<CCC>.<DDD>.0/24"),   U_STRING_TO_PARAM(netmask));

               if (local)
                  {
                  UString tmp(200U + local.size() + netmask.size() + lbl.size());

                  tmp.snprintf(U_STRING_TO_PARAM(local), netmask.rep, lbl.rep);

                  local = tmp;
                  }
               }
            else
               {
               UVector<UString> vec(*ap_address, '.');

               buffer.snprintf(U_CONSTANT_TO_PARAM("%u.%v"), 16 + vec[2].strtoul(), vec[3].rep);

               _body = UStringExt::substitute(_body, U_CONSTANT_TO_PARAM("<CCC>.<DDD>"), U_STRING_TO_PARAM(buffer));
               }

            _body = UStringExt::substitute(_body, U_CONSTANT_TO_PARAM("#include \"ap/<AAA.BBB.CCC.DDD>/nodog.conf.local\""), U_STRING_TO_PARAM(local));
            _body = UStringExt::substitute(_body, U_CONSTANT_TO_PARAM(              "<AAA.BBB.CCC.DDD>"),                    U_STRING_TO_PARAM(*ap_address));

            UFileConfig cfg(_body, true);

            UHTTP::mime_index = U_know;

            if (cfg.processData(false))
               {
               _body = cfg.getData();

#           ifdef USE_LIBZ
               if (U_http_is_accept_gzip &&
                   _body.size() > U_MIN_SIZE_FOR_DEFLATE)
                  {
                  _body = UStringExt::deflate(_body, 1);
                  }
#           endif
               }
            }
         }
      }

   USSIPlugIn::setAlternativeResponse(_body);
}

static void GET_get_users_info()
{
   U_TRACE_NO_PARAM(5, "::GET_get_users_info()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      if (UServer_Base::startParallelization()) return; // parent

      // child
      // ------------------------------------------------
      // $1 -> ap (without localization => '@')
      // $2 -> public address to contact the access point
      // ------------------------------------------------

      if (setAccessPoint(false, false))
         {
         UTimeVal to_sleep(U_TIME_FOR_ARPING_ASYNC_COMPLETION + 2);
loop:
         (void) nodog_rec->sendRequestToNodog(U_CONSTANT_TO_PARAM("check"), 0);

         if (U_http_info.nResponseCode == HTTP_NO_CONTENT)
            {
            to_sleep.nanosleep();

            goto loop;
            }
         }
      else
         {
         db_nodog->callForAllEntryWithVector(WiAuthNodog::checkAccessPoint);

         if ((U_SRV_CNT_USR4++ % 10) == 0) resync();
         }

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_help_wifi()
{
   U_TRACE_NO_PARAM(5, "::GET_help_wifi()")

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("help_wifi.tmpl")), 0, false,
                                     title_default->data(), 0, 0,
                                     0);
}

static void GET_logged()
{
   U_TRACE_NO_PARAM(5, "::GET_logged()")

   USSIPlugIn::setAlternativeRedirect("http://www.google.com", 0);
}

static void GET_logged_login_request()
{
   U_TRACE_NO_PARAM(5, "::GET_logged_login_request()")

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("logged_login_request.tmpl")), 0, false,
                                     title_default->data(), 0, 0,
                                     0);
}

static void GET_login() // MAIN PAGE (se il portatile non mostra la login page controllare in /etc/hosts se e' valorizzato wifi-..)
{
   U_TRACE_NO_PARAM(5, "::GET_login()")

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
   // $8 -> ts (timestamp)
   // -----------------------------------------------------------------------------------------------------------------------------------------------
   // GET /login?mac=00%3A14%3AA5%3A6E%3A9C%3ACB&ip=192.168.226.2&redirect=http%3A%2F%2Fgoogle&gateway=192.168.226.1%3A5280&timeout=0&token=x&ap=lab2
   // -----------------------------------------------------------------------------------------------------------------------------------------------

   bool login_validate = false;

   if (checkLoginRequest(0, 14, 2, true, false))
      {
      if (checkTimeRequest() == false) return;

      auth_domain->clear();

      if (getCookie1())
         {
         *uid = *mac;

         login_validate = true;

         if (auth_domain->empty()) *auth_domain = *mac_auth;
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

      if (u_isUrlEncodeNeeded(U_STRING_TO_PARAM(*redirect)) == false) *redir = *redirect;
      else
         {
         UString value_encoded((redirect->size() + 1024U) * 3U);

         Url::encode(*redirect, value_encoded);

         *redir = value_encoded;
         }

      redirect->clear();

      U_INTERNAL_DUMP("redir = %V", redir->rep)
      }

   if (login_validate)
      {
      user_UploadRate->replace('0');
      user_DownloadRate->replace('0');

      sendLoginValidate(); // NB: in questo modo l'utente ripassa dal firewall e NoDog lo rimanda da noi (login_validate) con i dati rinnovati...

      return;
      }

   uint32_t sz;
   char format[8 * 1024U];

   UString request1(8 * 1024U);
   UString request2(8 * 1024U);
   UString request3(8 * 1024U);

   uint32_t fmt_size = u__snprintf(format, sizeof(format), U_CONSTANT_TO_PARAM("http%%s://%v/%%s?%%.*s%srealm=%%s&redir_to=%%v"), virtual_name->rep, U_http_info.query_len ? "&" : "");

   request3.snprintf(format, fmt_size, "s", "login_request", U_HTTP_QUERY_TO_TRACE, "all", redir->rep);

   U_INTERNAL_DUMP("request3 = %V", request3.rep)

   sz = request3.size();

   ptr2 =                        (ptr1 = "s", "login_request");
   ptr3 = (mac->empty() ? ptr2 : (ptr1 =  "", "login_request_by_MAC"));

   request1.snprintf(format, fmt_size, ptr1, ptr3, U_HTTP_QUERY_TO_TRACE, "all", redir->rep);

   sz += request1.size();

   U_INTERNAL_DUMP("request1 = %V", request1.rep)

   request2.snprintf(format, fmt_size, "s", "login_request", U_HTTP_QUERY_TO_TRACE, "firenzecard", redir->rep);

   sz += request2.size();

   U_INTERNAL_DUMP("request2 = %S ap_address = %V", request2.rep, ap_address->rep)

   UString ap_ref_comune(100U);

   if (*ap_address &&
       u_isIPv4Addr(U_STRING_TO_PARAM(*ap_address)))
      {
      setAccessPointReference(U_STRING_TO_PARAM(*ap_address));

      ap_ref_comune.snprintf(U_CONSTANT_TO_PARAM("/%v"), ap_ref->rep);
      }

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("login.tmpl")), sz, false,
                                     title_default->data(), 0, 0,
                                     url_banner_ap->rep,
                                     ap_ref_comune.rep,
                                     request1.rep,
                                     help_url->rep,
                                     request2.rep,
                                     request3.rep,
                                     url_banner_comune->rep,
                                     ap_ref_comune.rep);
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

   UString id, timeout;

   getLoginRequest(timeout);

   if (idp == false &&
       getCookie2(realm, &id))
      {
      (void) checkIfUserConnected();

      if (user_exist)
         {
         user_UploadRate->setFromNumber32(user_rec->UploadRate);
         user_DownloadRate->setFromNumber32(user_rec->DownloadRate);
         }
      else
         {
      // ==========================================================================================================================================================
      // NB: this can happen if the user is authenticated but failed to redirect with login_validate by nodog...
      // ==========================================================================================================================================================
      // U_LOGGER("*** COOKIE(%V) FOUND BUT UID(%v) NOT EXIST: IP(%v) MAC(%v) AP(%v@%v) ***", id.rep, uid->rep, ip->rep, mac->rep, ap_label->rep, ap_address->rep);
      // ==========================================================================================================================================================

         user_UploadRate->replace('0');
         user_DownloadRate->replace('0');
         }

      table->clear();

      *auth_domain = *cookie_auth + *realm;

      if (realm->equal(U_CONSTANT_TO_PARAM("firenzecard")))
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
         if (realm->equal(U_CONSTANT_TO_PARAM("all")) == false ||
             askToLDAP(0,0,0,"ldapsearch -LLL -b %v %v waLogin=%v", wiauth_card_basedn->rep, ldap_card_param->rep, uid->rep) == -1)
            {
            *policy = (user_exist ? user_rec->_policy : *policy_daily);
            }
         else
            {
            *policy = (*table)["waPolicy"];

            if (policy->empty()) *policy = *policy_daily;
            }

         U_INTERNAL_ASSERT(*policy)
         }

      sendLoginValidate(); // NB: in questo modo l'utente ripassa dal firewall e NoDog lo rimanda da noi (login_validate) con i dati rinnovati...

      return;
      }

   UString tmp1, tmp2; // = U_STRING_FROM_CONSTANT("<a href=\"password\">Non riesci a ricordare la password?</a>");

   if (realm->equal(U_CONSTANT_TO_PARAM("firenzecard")) == false)
      {
      tmp1 = U_STRING_FROM_CONSTANT("<p><input type=\"checkbox\" name=\"PersistentCookie\" "
                                    "id=\"PersistentCookie\" value=\"yes\" checked=\"checked\"><strong>Resta connesso</strong></p>");

      USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("login_request.tmpl")), 0, false,
                                        title_default->data(), 0, 0,
                                        login_url->rep, mac->rep, ip->rep, redirect->rep, gateway->rep, timeout.rep, token->rep, ap->rep, realm->rep, redir->rep,
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
   U_TRACE_NO_PARAM(5, "::GET_login_request()")

   GET_LoginRequest(false);
}

static void GET_login_request_IdP()
{
   U_TRACE_NO_PARAM(5, "::GET_login_request_IdP()")

   GET_LoginRequest(true);
}

static void GET_login_request_by_MAC()
{
   U_TRACE_NO_PARAM(5, "::GET_login_request_by_MAC()")

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

   UString timeout;

   getLoginRequest(timeout);

   if (WiAuthUser::checkMAC())
      {
      *policy = *policy_daily;

      user_UploadRate->replace('0');
      user_DownloadRate->replace('0');

      if (*uid == *mac) setCookie1();

      sendLoginValidate(); // NB: in questo modo l'utente ripassa dal firewall e NoDog lo rimanda da noi (login_validate) con i dati rinnovati...
      }
}

static void GET_login_validate()
{
   U_TRACE_NO_PARAM(5, "::GET_login_validate()")

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
   // $8 -> ts (timestamp)
   // ---------------------------------------------------------------------------------------------------

   if (checkLoginRequest(0, 14, 2, false, true) == false ||
       checkLoginValidate(true)                 == false)
      {
      loginWithProblem();

      return;
      }

   if (WiAuthUser::checkUserID() == false) return;

   brenew = false;
   UVector<UString> vec;

   if (checkIfUserConnected())
      {
      if (isIP == false)
         {
         // Check if change of connection context for user id (RENEW: mobility between access point)...

         if (*ip         != user_rec->_ip  ||
             *mac        != user_rec->_mac ||
             *ap_address != user_rec->nodog)
            {
            if (user_rec->_auth_domain == *account_auth)
               {
               USSIPlugIn::setMessagePage(*message_page_template, "Account busy", "Hai usato una credenziale che non e' disponibile in questo momento!");

               return;
               }

            brenew = true;

            user_rec->setLabelAP();

            U_LOGGER("*** RENEW: UID(%v) IP(%v=>%v) MAC(%v=>%v) ADDRESS(%v@%v=>%v@%v) AUTH_DOMAIN(%v) ***", uid->rep,
                        user_rec->_ip.rep, ip->rep,
                        user_rec->_mac.rep, mac->rep,
                        label->rep, user_rec->nodog.rep, ap_label->rep, ap_address->rep,
                        user_rec->_auth_domain.rep);

            vec.push_back(user_rec->nodog);
            vec.push_back(user_rec->_ip);
            vec.push_back(user_rec->_mac);
            }
         }
      }

   WiAuthVirtualAccessPoint::setIndexAccessPoint();

   if (index_access_point == U_NOT_FOUND) index_access_point = 0; 

   user_rec->setRecord();

   UString x, signed_data(500U + U_http_info.query_len);

   if (user_rec->consume == false)
      {
         time_counter->snprintf(U_CONSTANT_TO_PARAM("%v"),    time_available->rep);
      traffic_counter->snprintf(U_CONSTANT_TO_PARAM("%v"), traffic_available->rep);
      }
   else
      {
      if (user_rec->_time_consumed >= user_rec->_time_available)
         {
         char msg[4096];

         (void) u__snprintf(msg, sizeof(msg),
                  U_CONSTANT_TO_PARAM("Hai consumato il tempo disponibile del servizio!<br>"
                  "ma puoi continuare comunque a utilizzare illimitatamente i servizi esposti sulla <a href=\"%v\">rete cittadina</a>"),
                  wallet_url->rep);

         USSIPlugIn::setMessagePage(*message_page_template, "Tempo consumato", msg);

         goto end;
         }

      if (user_rec->_traffic_consumed >= user_rec->_traffic_available)
         {
         char msg[4096];

         (void) u__snprintf(msg, sizeof(msg),
                  U_CONSTANT_TO_PARAM("Hai consumato il traffico disponibile del servizio!<br>"
                  "ma puoi continuare comunque a utilizzare illimitatamente i servizi esposti sulla <a href=\"%v\">rete cittadina</a>"),
                  wallet_url->rep);

         USSIPlugIn::setMessagePage(*message_page_template, "Traffico consumato", msg);

         goto end;
         }

         time_counter->setFromNumber32s(user_rec->_time_available    - user_rec->_time_consumed);
      traffic_counter->setFromNumber64s(user_rec->_traffic_available - user_rec->_traffic_consumed);
      }

   // redirect back to the gateway appending a signed ticket that will signal NoDog to unlock the firewall...

   WiAuthUser::loadPolicy(x = user_rec->getPolicy());

#ifdef USE_LIBSSL
   signed_data = UDES3::signData(
#else
   signed_data.snprintf(
#endif
      U_CONSTANT_TO_PARAM("\nMac %v\n"
      "Timeout %v\n"
      "Traffic %v\n"
      "Token %v\n"
      "User %v\n"
      "Policy %v\n"
      "NoTraffic %v\n"
      "UserUploadRate %v\n"
      "UserDownloadRate %v\n"
      "Redirect http://%v/postlogin?%.*s\n"),
      mac->rep, time_counter->rep, traffic_counter->rep, token->rep, uid->rep, x.rep,
      max_time_no_traffic->rep, user_DownloadRate->rep, user_UploadRate->rep, virtual_name->rep, U_HTTP_QUERY_TO_TRACE);

   char buffer[128];

   (void) u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%s%v"), (brenew ? "RENEW_" : ""), auth_domain->rep); 

   user_rec->writeToLOG(buffer); // NB: writeToLOG() change time_counter and traffic_counter strings...

   if (gateway->c_char(0) == ':') (void) gateway->insert(0, *ap_address);

   USSIPlugIn::setAlternativeRedirect("http://%v/ticket?ticket=%v", gateway->rep, signed_data.rep);

end:
   brenew = false;

   if (vec.empty() == false) askNodogToLogoutUser(vec, false);
}

static void GET_logout_page()
{
   U_TRACE_NO_PARAM(5, "::GET_logout_page()")

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("logout_page.tmpl")), 0, false,
                                     title_default->data(), 0, 0,
                                     logout_url->rep);
}

static void GET_password()
{
   U_TRACE_NO_PARAM(5, "::GET_password()")

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("password.tmpl")), 0, false,
                         "Modifica password utente",
                         "<link type=\"text/css\" href=\"css/livevalidation.css\" rel=\"stylesheet\" />"
                         "<script type=\"application/javascript\" src=\"js/livevalidation_standalone.compressed.js\"></script>", 0,
                         password_url->rep);
}

static void GET_postlogin()
{
   U_TRACE_NO_PARAM(5, "::GET_postlogin()")

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
   // $8 -> ts (timestamp)
   // -----------------------------------------------------------------------------------------------------------------------------------------------------------------------
   // GET /postlogin?mac=00%3A00%3A00%3A00%3A00%3A00&ip=172.22.134.79&redirect=U2FsdGVkX1_...&gateway=151.11.47.5%3A5280&timeout=0&token=3&ap=90@U2FsdGVkX1_...&ts=1475223893
   // -----------------------------------------------------------------------------------------------------------------------------------------------------------------------

   if (n != 14 &&
       n != 16)
      {
   // U_LOGGER("*** NUM ARGS(%u) DIFFERENT FROM EXPECTED(14) ***", n);

error:
      loginWithProblem();

      return;
      }

   if (checkLoginRequest(n, 14, 2, false, false) == false ||
       checkLoginValidate(false)                 == false)
      {
      goto error;
      }

   U_INTERNAL_DUMP("redir = %V", redir->rep)

   U_INTERNAL_ASSERT(*redir)
   U_ASSERT_MINOR(redir->size(), 2048)

   if (WiAuthUser::checkUserID() == false) return;

   if (checkIfUserConnected())
      {
      if (user_rec->_auth_domain == *account_auth)
         {
         U_LOGGER("*** GROUP ACCOUNT: UID(%v) IP(%v) MAC(%v) AP(%v@%v) REDIR(%v) ***", uid->rep, ip->rep, mac->rep, ap_label->rep, ap_address->rep, redir->rep);
         }
      }
   else if (user_exist == false)
      {
      U_LOGGER("*** POSTLOGIN ERROR(user NOT exist): UID(%v) IP(%v) MAC(%v) AP(%v@%v) REDIR(%v) ***", uid->rep, ip->rep, mac->rep, ap_label->rep, ap_address->rep, redir->rep);

      goto error;
      }

   // Sanity check for user id (the param of url come from login_validate)...

   if (*ip         != user_rec->_ip  ||
       *mac        != user_rec->_mac ||
       *ap_address != user_rec->nodog)
      {
      *label = nodog_rec->getLabelAP(user_rec->_index_access_point);

      U_LOGGER("*** POSTLOGIN ERROR(sanity check failed): UID(%v) IP(%v=>%v) MAC(%v=>%v) ADDRESS(%v=>%v@%v) AUTH_DOMAIN(%v) REDIR(%v) ***", uid->rep,
                     user_rec->_ip.rep, ip->rep,
                     user_rec->_mac.rep, mac->rep,
                     ap->rep, label->rep, user_rec->nodog.rep,
                     user_rec->_auth_domain.rep, redir->rep);

      goto error;
      }

   if (user_rec->setNodogReference() == false) goto error;

   // NB: send as response the message of waiting to redirect to original site...

   bool ball         = false,
        bfirenzecard = false;

   if (U_STRING_FIND(user_rec->_auth_domain, 0, "firenzecard") == U_NOT_FOUND)
      {
      if (U_STRING_FIND(user_rec->_auth_domain, 0, "all") != U_NOT_FOUND) ball = true;
      }
   else
      {
      bfirenzecard = true;

      setAccessPointReference(U_STRING_TO_PARAM(*ap_address));

      redir->setBuffer(U_CAPACITY);

      redir->snprintf(U_CONSTANT_TO_PARAM(FIRENZECARD_REDIR), ap_ref->rep);
      }

   // NB: we may be a different process from what it has updated so that we need to read the record...

   (void) db_nodog->getDataStorage(*ap_address);

   WiAuthAccessPoint* ap_rec = nodog_rec->vec_access_point[user_rec->_index_access_point];

                          ap_rec->num_login++;
                          ap_rec->_num_users_connected++;
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

   if (u_isUrlEncodeNeeded(U_STRING_TO_PARAM(*uid))) Url::encode(*uid, uid_encoded);
   else                                              uid_encoded = *uid;

   U_INTERNAL_DUMP("uid_encoded = %V", uid_encoded.rep)

   UString buffer(100U + uid_encoded.size() + redir->size());

   buffer.snprintf(U_CONSTANT_TO_PARAM("onload=\"doOnLoad('postlogin?uid=%v','%v')\""), uid_encoded.rep, redir->rep); 

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("postlogin.tmpl")), 0, false,
                                     title_default->data(), "<script type=\"application/javascript\" src=\"js/logout_popup.js\"></script>", buffer.data(),
                                     uid->rep, redir->rep, redir->rep);
}

static void GET_recovery()
{
   U_TRACE_NO_PARAM(5, "::GET_recovery()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      // ----------------------------
      // $1 -> uid
      // ----------------------------

      UHTTP::getFormValue(*uid, U_CONSTANT_TO_PARAM("user"), 0, 1, UHTTP::processForm());

      if (checkIfUserConnected() &&
          user_rec->setNodogReference())
         {
         *ap_address = user_rec->nodog;

         (void) askNodogToLogoutUser(user_rec->_ip, user_rec->_mac, true);
         }

      UString user = WiAuthUser::_getUserName();

      ULog::log(file_RECOVERY->getFd(), U_CONSTANT_TO_PARAM("%v \"%v\""), uid->rep, user.rep);

      U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

      int result = db_user->remove(*uid);

      if (result) U_SRV_LOG("WARNING: remove of user %v on db WiAuthUser failed with error %d", uid->rep, result);

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_registrazione()
{
   U_TRACE_NO_PARAM(5, "::GET_registrazione()")

   // $1 -> ap (with localization => '@')

   UString tutela_dati = cache->getContent(U_CONSTANT_TO_PARAM("tutela_dati.txt"));

   U_INTERNAL_ASSERT(tutela_dati.isNullTerminated())

   USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("registrazione.tmpl")), 0, false,
                         "Registrazione utente",
                         "<link type=\"text/css\" href=\"css/livevalidation.css\" rel=\"stylesheet\" />"
                         "<script type=\"application/javascript\" src=\"js/livevalidation_standalone.compressed.js\"></script>", 0,
                         registrazione_url->rep, tutela_dati.rep);
}

static void GET_reset_counter_ap()
{
   U_TRACE_NO_PARAM(5, "::GET_reset_counter_ap()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      db_nodog->callForAllEntryWithSetEntry(WiAuthNodog::resetPolicy);

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_reset_policy()
{
   U_TRACE_NO_PARAM(5, "::GET_reset_policy()")

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

      file_UTILIZZO->printf(U_CONSTANT_TO_PARAM("\"%#6D\",\"%u\",\"%u\",\"%u\","),
                            u_now->tv_sec - (60L * 60L),
                            data_rec->utenti_connessi_giornaliero_globale,
                            data_rec->tempo_permanenza_utenti_giornaliero_globale / divisor,
                            (uint32_t)(data_rec->traffico_generato_giornaliero_globale / (1024ULL * 1024ULL * 1024ULL)));

      data_rec->reset();

      (void) UHTTP::db_session->putDataStorage();

      db_nodog->callForAllEntryWithSetEntry(WiAuthNodog::resetPolicy);

      // reset policy

      db_user->callForAllEntryWithSetEntry(WiAuthUser::checkForUserPolicy);

      USSIPlugIn::setAlternativeResponse();
      }
}

static void GET_start_ap()
{
   U_TRACE_NO_PARAM(5, "::GET_start_ap()")

   // $1 -> ap (with localization => '@')
   // $2 -> public address to contact the access point
   // $3 -> pid (0 => start)

   if (setAccessPoint(true, true))
      {
      UString pid(100U);

      UHTTP::getFormValue(pid, U_CONSTANT_TO_PARAM("pid"), 0, 5, UHTTP::form_name_value->size());

      nodog_rec->setStatus(3); // startup

      U_LOGGER("%v:%v %s", ap_address->rep, ap_hostname->rep, (pid.strtoul() == 0 ? "started" : "*** NODOG CRASHED ***"));

      db_user->callForAllEntry(WiAuthUser::quitUserConnected);
      }

   USSIPlugIn::setAlternativeResponse(*allowed_web_hosts);
}

static void GET_stato_utente()
{
   U_TRACE_NO_PARAM(5, "::GET_stato_utente()")

   UString result;

   uid->clear();

   if (checkIfUserConnected() &&
       user_rec->setNodogReference())
      {
      *ap_address = user_rec->nodog;

      result = nodog_rec->sendRequestToNodog(U_CONSTANT_TO_PARAM("status?ip=%.*s"), U_CLIENT_ADDRESS_TO_TRACE);

      if (result.empty() ||
          U_IS_HTTP_ERROR(U_http_info.nResponseCode))
         {
         user_rec->setConnected(false);

         user_rec->writeToLOG("QUIT");

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
      user_rec->setConnected(true);

      user_rec->writeToLOG("RESYNC");

      U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

      (void) db_user->putDataStorage(*uid);
      }

#ifdef USE_LIBZ
   if (UStringExt::isGzip(result)) result = UStringExt::gunzip(result);
#endif

   USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("stato_utente.tmpl")), 0, false,
                                     "Stato utente", 0, 0,
                                     WiAuthUser::getUserName().rep, uid->rep, user_rec->getAP().rep, result.rep);
}

static void GET_webif_ap()
{
   U_TRACE_NO_PARAM(5, "::GET_webif_ap()")

   // $1 -> ap (with localization => '@')
   // $2 -> public address to contact the access point

   if (virtual_name->equal(U_HTTP_VHOST_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      if (setAccessPoint(true, false) &&
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

// request managed with the main.bash script...

static void GET_admin_export_statistics_login_as_csv()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_export_statistics_login_as_csv()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
}

static void GET_admin_export_statistics_registration_as_csv()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_export_statistics_registration_as_csv()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
}

static void GET_admin_historical_statistics_login()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_historical_statistics_login()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
}

static void GET_admin_printlog()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_printlog()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
}

static void GET_admin_recovery()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_recovery()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
}

static void GET_admin_view_statistics_login()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_view_statistics_login()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      if (db_user_filter_tavarnelle)
         {
         USSIPlugIn::setMessagePage(admin_cache->getContent(U_CONSTANT_TO_PARAM("message_page.tmpl")), "Servizio non disponibile (non ancora implementato). Riprovare piu' tardi");
         }
      }
}

static void GET_admin_view_statistics_registration()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_view_statistics_registration()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
}

static void GET_gen_activation()
{
   U_TRACE_NO_PARAM(5, "::GET_gen_activation()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
}

static void GET_polling_attivazione()
{
   U_TRACE_NO_PARAM(5, "::GET_polling_attivazione()")
}

static void GET_polling_password()
{
   U_TRACE_NO_PARAM(5, "::GET_polling_password()")
}

// POST

static void POST_admin_execute_recovery()
{
   U_TRACE_NO_PARAM(5, "::POST_admin_execute_recovery()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
}

static void POST_admin_recovery()
{
   U_TRACE_NO_PARAM(5, "::POST_admin_recovery()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
}

static void POST_password()
{
   U_TRACE_NO_PARAM(5, "::POST_password()")
}

static void POST_registrazione()
{
   U_TRACE_NO_PARAM(5, "::POST_registrazione()")
}

static void POST_admin_edit_ap()
{
   U_TRACE_NO_PARAM(5, "::POST_admin_edit_ap()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      // -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      // $1 -> ap_label
      // $2 -> ap_address
      // $3 -> ap_hostname
      // $4 -> ap_mac_mask
      // $5 -> ap_group_account
      // $6 -> ap_up
      // $7 -> ap_consume
      // $8 -> submit
      // -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      // 10.1.55.83 - - [28/Nov/2016:18:19:36 +0100] "POST /admin_edit_ap? HTTP/1.1" 200 - "https://10.30.1.111/admin_edit_ap?ap=1@lab5-r29587_locoM2&public=10.10.100.115%3A5280"
      // -------------------------------------------------------------------------------------------------------------------------------------------------------------------------
      // "ap_label=1&ap_address=10.10.100.115&ap_hostname=lab5-r29587_locoM2&ap_mac_mask=&ap_group_account=&ap_consume=on&submit=Registrazione"
      // -------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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
            WiAuthNodog::editRecord(reboot.equal(U_CONSTANT_TO_PARAM("on")), consume.equal(U_CONSTANT_TO_PARAM("on")), mac_mask, group_account_mask);
            }
         }

      if (db_user_filter_tavarnelle) USSIPlugIn::setAlternativeRedirect("tavarnelle.shtml?admin_status_nodog", 0);
      else                           USSIPlugIn::setAlternativeRedirect(                 "admin_status_nodog", 0);
      }
}

static void POST_admin_view_user()
{
   U_TRACE_NO_PARAM(5, "::POST_admin_view_user()")

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

            not_after.snprintf(U_CONSTANT_TO_PARAM("%.2s/%.2s/%.4s - %.2s:%.2s"), ptr+6, ptr+4, ptr, ptr+8, ptr+10);
            }

         if (WA_USEDBY)
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
         UString user = WiAuthUser::getUserName(); // NB: must be after checkIfUserConnected()...

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
 
   if (checkLoginRequest(0, 26, 4, true, false) == false)
      {
      loginWithProblem();

      return;
      }

   UHTTP::getFormValue(*realm, U_CONSTANT_TO_PARAM("realm"), 0, 15, 24);

   U_INTERNAL_DUMP("realm = %V", realm->rep)

   if (realm->empty())
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

   if (realm->equal(U_CONSTANT_TO_PARAM("all")) == false)
      {
      if (fmt_auth_cmd->empty() ||
          runAuthCmd(password.c_str(), realm->c_str()) == false)
         {
         return;
         }

      *policy = *(realm->equal(U_CONSTANT_TO_PARAM("firenzecard"))
                     ? policy_traffic
                     : policy_daily);

      UString x    = UStringExt::trim(*output);
      uint32_t pos = x.findWhiteSpace();

      if (pos == U_NOT_FOUND) pos = x.size();

      auth_domain->setBuffer(80U);
      auth_domain->snprintf(U_CONSTANT_TO_PARAM("AUTH_%.*s"), pos, x.data());

      user_UploadRate->replace('0');
      user_DownloadRate->replace('0');
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
             runAuthCmd(password.c_str(), realm->c_str()) == false)
            {
            return;
            }

         *policy = *policy_daily;

         UString x    = UStringExt::trim(*output);
         uint32_t pos = x.findWhiteSpace();

         if (pos != U_NOT_FOUND) (void) realm->assign(x.data(), pos);
            {
            pos    = x.size();
            *realm = x;
            }

         auth_domain->setBuffer(80U);
         auth_domain->snprintf(U_CONSTANT_TO_PARAM("AUTH_%v"), realm->rep);

         user_UploadRate->replace('0');
         user_DownloadRate->replace('0');
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
                     U_CONSTANT_TO_PARAM("Credenziali errate!<br>"
                     "<a href=\"javascript:history.go(-1)\">Indietro</a>"), 0);

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
                                         : UTimeDate::strftime(U_CONSTANT_TO_PARAM("%Y%m%d%H%M%SZ"), u_now->tv_sec + (VALIDITY.strtoul() * U_ONE_DAY_IN_SECOND)));

            UString input(U_CAPACITY);

            input.snprintf(U_CONSTANT_TO_PARAM("dn: %v\n"
                           "changetype: modify\n"
                           "add: waNotAfter\n"
                           "waNotAfter: %v\n"
                           "-"),
                           DN.rep, NOT_AFTER.rep);

            if (askToLDAP(&input, "Errore", "LDAP error", "ldapmodify -c %v", ldap_card_param->rep) <= 0) return;
            }

         *policy = (*table)["waPolicy"]; // waPolicy: DAILY

         user_UploadRate->replace('0');
         user_DownloadRate->replace('0');
         }
      }

   if (idp == false                                   &&
       *ip                                            &&
        ip->equal(U_CLIENT_ADDRESS_TO_PARAM) == false &&
       UClientImage_Base::isAllowed(*vallow_IP_request) == false) // 172.0.0.0/8, ...
      {
      U_LOGGER("*** PARAM IP(%v) FROM AP(%v@%v) IS DIFFERENT FROM CLIENT ADDRESS(%.*s) - REALM(%v) UID(%v) ***",
                     ip->rep, ap_label->rep, ap_address->rep, U_CLIENT_ADDRESS_TO_TRACE, realm->rep, uid->rep);
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

         if (getCookie2(0, 0) == false ||
             *uid != uid_save)
            {
            UString input(U_CAPACITY);
            unsigned char key[16], hexdump[33];

            UServices::generateKey(key, hexdump);

            input.snprintf(U_CONSTANT_TO_PARAM("dn: waCookieId=%s, o=sessions\n"
                           "waCookieId: %s\n"
                           "objectClass: waSession\n"
                           "waFederatedUserId: %v@%v\n"), // Ex: 3343793489@all
                           hexdump, hexdump, uid->rep, realm->rep);

            if (askToLDAP(&input, "Errore", "LDAP error", "ldapadd -c %v", ldap_session_param->rep) == 1) setCookie2((const char*)hexdump);
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
   U_TRACE_NO_PARAM(5, "::POST_login_request()")

   if (UClientImage_Base::request_uri->equal(U_CONSTANT_TO_PARAM("/login_request/logout")) == false) POST_LoginRequest(false);
   else
      {
      // manage logout request

      bool bpopup;
      UVector<UString> vec;
      uint32_t end = UHTTP::processForm();
      UString buffer(U_CAPACITY), delete_cookie, uid_cookie = (uid->clear(), (void)getCookie2(0, 0), *uid),
              uid_session = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("ULIB_SESSION"), UClientImage_Base::environment);

      // ----------------------------
      // $1 -> uid
      // $2 -> submit
      // $3 -> PersistentCookie
      // ----------------------------

      UHTTP::getFormValue(*uid,          U_CONSTANT_TO_PARAM("uid"), 0, 1, end);
      UHTTP::getFormValue(delete_cookie, U_CONSTANT_TO_PARAM("PersistentCookie"));

      if (*uid)
         {
         bpopup = true;

         if (uid_cookie)
            {
            if (uid->equal(uid_cookie) == false) U_LOGGER("*** UID COOKIE DIFFER(%v=>%v) ***", uid->rep, uid_cookie.rep);
            }
         else if (uid_session)
            {
            if (uid->equal(uid_session) == false) U_LOGGER("*** UID SESSION DIFFER(%v=>%v) ***", uid->rep, uid_session.rep);
            }
         }
      else
         {
         bpopup = false;

              if (uid_cookie)  *uid = uid_cookie;
         else if (uid_session) *uid = uid_session;
         }

      if (checkIfUserConnected() &&
          user_rec->setNodogReference())
         {
         if (user_rec->_auth_domain != *account_auth)
            {
            vec.push_back(user_rec->nodog);
            vec.push_back(user_rec->_ip);
            vec.push_back(user_rec->_mac);

            user_rec->writeToLOG("LOGOUT");
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

      if (delete_cookie.equal(U_CONSTANT_TO_PARAM("yes")))
         {
              if (uid_cookie) setCookie2(0);
         else if (uid_session)
            {
            UHTTP::removeCookieSession();

            U_SRV_LOG("Delete session ulib.s%u keyid=%V", UHTTP::sid_counter_cur, uid_session.rep);
            }
         }

      user_rec->getCounter();

      if (bpopup)
         {
         ptr1 = "<script type=\"application/javascript\" src=\"js/logout_popup.js\"></script>";
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

      if (user_rec->consume)
         {
         if (   time_counter->first_char() == '-')    time_counter->setFromNumber32(0);
         if (traffic_counter->first_char() == '-') traffic_counter->setFromNumber64(0LL);

         buffer.snprintf(U_CONSTANT_TO_PARAM("<table class=\"centered\" border=\"1\">"
                            "<tr><th class=\"header\" colspan=\"2\" align=\"left\">Utente&nbsp;&nbsp;&nbsp;%v</th></tr>"
                            "<tr><td class=\"header\">Tempo residuo (min)</td><td class=\"data_italic\">%v</td></tr>"
                            "<tr><td class=\"header\">Traffico residuo (MB)</td><td class=\"data_italic\">%v</td></tr>"
                         "</table>"), uid->rep, time_counter->rep, traffic_counter->rep);
         }

      USSIPlugIn::setAlternativeInclude(cache->getContent(U_CONSTANT_TO_PARAM("ringraziamenti.tmpl")), 0, true,
                                        title_default->data(), ptr1, 0,
                                        buffer.rep, ptr2);

      if (vec.empty() == false) askNodogToLogoutUser(vec, false);
      }
}

static void POST_login_request_IdP()
{
   U_TRACE_NO_PARAM(5, "::POST_login_request_IdP()")

   POST_LoginRequest(true);
}

static void POST_uploader()
{
   U_TRACE_NO_PARAM(5, "::POST_uploader()")

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

            dest.snprintf(U_CONSTANT_TO_PARAM("%v/%v"), historical_log_dir->rep, basename.rep);

            (void) UFile::writeTo(dest, content);
            }
         }
      }

   USSIPlugIn::setAlternativeResponse();
}

// STATUS ACCESS POINT

static bool status_ap_no_label;

static void GET_status_ap()
{
   U_TRACE_NO_PARAM(5, "::GET_status_ap()")

   // $1 -> ap (with localization => '@')
   // $2 -> public address to contact the access point

   if (virtual_name->equal(U_HTTP_VHOST_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      UString result;

      if (setAccessPoint(true, false)) result = (status_ap_no_label ? nodog_rec->sendRequestToNodog(U_CONSTANT_TO_PARAM("status"), 0)
                                                                    : nodog_rec->sendRequestToNodog(U_CONSTANT_TO_PARAM("status?label=%v"), ap_label->rep));

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

static void GET_status_ap_no_label()
{
   U_TRACE_NO_PARAM(5, "::GET_status_ap_no_label()")

   status_ap_no_label = true;

   GET_status_ap();

   status_ap_no_label = false;
}

// INFO

static void POST_info()
{
   U_TRACE_NO_PARAM(5, "::POST_info()")

   U_INTERNAL_ASSERT_EQUALS(UServer_Base::bssl, false)

#ifdef USE_LIBZ
   if (UStringExt::isGzip(*UClientImage_Base::body)) *UClientImage_Base::body = UStringExt::gunzip(*UClientImage_Base::body);
#endif

   uint32_t end;
   uint64_t _traffic;
   long time_connected;
   UVector<UString> vec;
   const char* write_to_log;
   WiAuthAccessPoint* ap_rec;
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

      bcheckIfUserConnected  = checkIfUserConnected();
      bsetAccessPointAddress = (*ap && setAccessPointAddress());
      bsetNodogReference     = (user_exist && bsetAccessPointAddress ? user_rec->setNodogReference() : false);
      blogout                = (logout && logout.first_char() != '0'); // NB: str logout == "0" mean NOT logout (only info)...

      if (bsetNodogReference     == false ||
          bcheckIfUserConnected  == false ||
          bsetAccessPointAddress == false)
         {
         U_LOGGER("*** INFO(%b,%b,%b,%b): UID(%v) IP(%v) MAC(%v) AP(%v@%v) LOGOUT(%v) user_exist(%b) ***",
                     bcheckIfUserConnected, bsetAccessPointAddress, bsetNodogReference, blogout, uid->rep, ip->rep, mac->rep, ap_label->rep, ap_address->rep, logout.rep, user_exist);

         if (bsetAccessPointAddress)
            {
            if (blogout == false)
               {
               ask_logout = true;

               goto next;
               }

            if (bsetNodogReference) user_rec->writeToLOG(WiAuthUser::getWriteToLog(logout));
            }

         continue;
         }

      // Check if change of connection context for user id (RENEW: mobility between access point)...

      brenew = false;

      if (*ip         != user_rec->_ip  ||
          *mac        != user_rec->_mac ||
          *ap_address != user_rec->nodog)
         {
         brenew = true;

         *label = nodog_rec->getLabelAP(user_rec->_index_access_point);

         U_LOGGER("*** INFO DIFFERENCE: UID(%v) IP(%v=>%v) MAC(%v=>%v) AP(%v@%v=>%v@%v) AUTH_DOMAIN(%v) LOGOUT(%v) ***", uid->rep,
                  ip->rep,  user_rec->_ip.rep,
                  mac->rep, user_rec->_mac.rep,
                  ap_label->rep, ap_address->rep, label->rep, user_rec->nodog.rep,
                  user_rec->_auth_domain.rep, logout.rep);

         if (blogout == false &&
             user_rec->_auth_domain != *account_auth)
            {
            vec.push_back(*ap_address);
            vec.push_back(*ip);
            vec.push_back(*mac);
            }
         }

next:       _traffic =   traffic.strtoull();
      time_connected = connected.strtoul();

      if (bsetNodogReference == false) write_to_log = 0;
      else
         {
         write_to_log = user_rec->updateCounter(logout, time_connected, _traffic, ask_logout);

         (void) db_user->putDataStorage(*uid);

         (void) db_nodog->getDataStorage(*ap_address); // NB: we may be a different process from what it has updated so that we need to read the record...

         nodog_rec->last_info = u_now->tv_sec;

         if (nodog_rec->status != 0)
            {
            U_LOGGER("*** AP(%v) CHANGE STATE (%d => 0) ***", ap_address->rep, nodog_rec->status);

            nodog_rec->status = 0;
            }

         WiAuthVirtualAccessPoint::setIndexAccessPoint();

         if (index_access_point == U_NOT_FOUND) index_access_point = 0; 

         ap_rec = nodog_rec->vec_access_point[index_access_point];

         ap_rec->traffic_done += _traffic;

         if (blogout &&
             bcheckIfUserConnected)
            {
            if (ap_rec->_num_users_connected) ap_rec->_num_users_connected--;
            else
               {
               num_users_connected_on_nodog = 0;

               db_user->callForAllEntry(WiAuthUser::countUserConnectedOnNodog);

               if (num_users_connected_on_nodog)
                  {
                  U_LOGGER("*** ON AP(%v@%v) THE NUMBER OF USER CONNECTED IS NOT ALIGNED (0=>%u) ***", ap_rec->_label.rep, ap_address->rep, num_users_connected_on_nodog);
                  }

               ap_rec->_num_users_connected = num_users_connected_on_nodog;
               }
            }

         (void) db_nodog->putDataStorage();
         }

             UHTTP::db_session->setPointerToDataStorage(data_rec);
      (void) UHTTP::db_session->getDataStorage();

      data_rec->traffico_generato_giornaliero_globale       += _traffic;
      data_rec->tempo_permanenza_utenti_giornaliero_globale += time_connected;

      (void) UHTTP::db_session->putDataStorage();

      if (write_to_log &&
          user_rec->setNodogReference())
         {
         user_rec->writeToLOG(write_to_log);
         }

      if (ask_logout)
         {
         ask_logout = false;

         if (user_rec->nodog)
            {
            vec.push_back(user_rec->nodog);
            vec.push_back(user_rec->_ip);
            vec.push_back(user_rec->_mac);
            }
         }
      }

   brenew = false;

   USSIPlugIn::setAlternativeResponse();

   if (vec.empty() == false) askNodogToLogoutUser(vec, false);
}

static void POST_roaming()
{
   U_TRACE_NO_PARAM(5, "::POST_roaming()")

#ifdef USE_LIBZ
   if (UStringExt::isGzip(*UClientImage_Base::body)) *UClientImage_Base::body = UStringExt::gunzip(*UClientImage_Base::body);
#endif

   uint32_t end;
   UVector<UString> vec;
   bool bcheckIfUserConnected, bsetAccessPointAddress, bsetNodogReference;

   for (int32_t i = 0, n = UHTTP::processForm(); i < n; i += 10)
      {
      // --------------------------------------------------------------------------------------------------------------
      // $1 -> mac
      // $2 -> ip
      // $3 -> gateway
      // $4 -> ap (with localization => '@')
      // $5 -> => UID <=
      // -------------------------------------------------------------------------------------------------------------
      // /roaming?Mac=98%3A0c%3A82%3A76%3A3b%3A39&ip=172.16.1.8&gateway=172.16.1.254%3A5280&ap=ap%4010.8.0.1&User=1212
      // -------------------------------------------------------------------------------------------------------------

      end = i+10;

      if (end > (uint32_t)n) end = n;

      UHTTP::getFormValue(*mac, U_CONSTANT_TO_PARAM("Mac"),   i, i+1,  end);
      UHTTP::getFormValue(*ip,  U_CONSTANT_TO_PARAM("ip"),    i, i+3,  end);
      UHTTP::getFormValue(*ap,  U_CONSTANT_TO_PARAM("ap"),    i, i+7,  end);
      UHTTP::getFormValue(*uid, U_CONSTANT_TO_PARAM("User"),  i, i+9,  end);

         ap_label->clear();
       ap_address->clear();
      ap_hostname->clear();

      bcheckIfUserConnected  = checkIfUserConnected(),
      bsetAccessPointAddress = (*ap && setAccessPointAddress()),
      bsetNodogReference     = (user_exist && bsetAccessPointAddress ? user_rec->setNodogReference() : false);

      if (bsetNodogReference     == false ||
          bcheckIfUserConnected  == false ||
          bsetAccessPointAddress == false)
         {
         U_LOGGER("*** ROAMING(%b,%b,%b): UID(%v) IP(%v) MAC(%v) AP(%v@%v=>%v) ***",
                     bcheckIfUserConnected, bsetAccessPointAddress, bsetNodogReference, uid->rep, ip->rep, mac->rep, ap_label->rep, ap_address->rep, ap->rep);

         if (bsetAccessPointAddress)
            {
            /*
            ask_logout = true;

            goto next;
            */
            }

         continue;
         }

      WiAuthVirtualAccessPoint::setIndexAccessPoint();

      if (index_access_point == U_NOT_FOUND)
         {
         /*
         ask_logout = true;

         goto next;
         */
         }

      U_LOGGER("*** UID(%v) IP(%v) MAC(%v) AP(%v@%v=>%v) ***", uid->rep, ip->rep, mac->rep, ap_label->rep, ap_address->rep, ap->rep);
      }

   USSIPlugIn::setAlternativeResponse();

   if (vec.empty() == false) askNodogToLogoutUser(vec, false);
}

static void GET_admin_status_nodog_and_user()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_status_nodog_and_user()")

   if (UServer_Base::bssl == false) USSIPlugIn::setBadRequest();
   else
      {
      num_ap              =
      num_ap_up           =
      num_ap_down         =
      num_ap_unreachable  =
      num_users_connected = 0;

      output->setBuffer(U_CAPACITY);

      db_nodog->callForAllEntryWithSetEntry(WiAuthNodog::setStatusNodogAndUser, 0, UStringExt::qscompver);

      if (admin_status_nodog_and_user_as_csv) USSIPlugIn::setAlternativeResponse(*output);
      else
         {
         uint32_t sz = output->size();

         USSIPlugIn::setAlternativeInclude(admin_cache->getContent(U_CONSTANT_TO_PARAM("admin_status_nodog_and_user.tmpl")), sz, false,
                               "stato access point and user access", 0, 0,
                               num_users_connected, num_ap, num_ap_up, num_ap_down, num_ap_unreachable, output->rep);
         }
      }
}

static void GET_admin_status_nodog_and_user_as_csv()
{
   U_TRACE_NO_PARAM(5, "::GET_admin_status_nodog_and_user_as_csv()")

   admin_status_nodog_and_user_as_csv = true;

   GET_admin_status_nodog_and_user();

   admin_status_nodog_and_user_as_csv = false;
}

// TAVARNELLE

__pure int WiAuthNodog::dbNodogFilter(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "WiAuthNodog::dbNodogFilter(%p,%p)", key, data)

   U_INTERNAL_ASSERT_POINTER(key)

   return UServices::dosMatchExtWithOR(U_STRING_TO_PARAM(*key), U_STRING_TO_PARAM(*db_filter_tavarnelle), 0);
}

int WiAuthUser::dbUserFilter(UStringRep* key, UStringRep* data)
{
   U_TRACE(5, "WiAuthUser::dbUserFilter(%p,%p)", key, data)

   U_INTERNAL_ASSERT_POINTER(key)

   if (WiAuthUser::setNodog(data)) return UServices::dosMatchExtWithOR(U_STRING_TO_PARAM(*ap_address), U_STRING_TO_PARAM(*db_filter_tavarnelle), 0);

   U_RETURN(0);
}

static void POST_tavarnelle()
{
   U_TRACE_NO_PARAM(5, "::POST_tavarnelle()")

   U_INTERNAL_DUMP("query = %.*S", U_HTTP_QUERY_TO_TRACE)

   if (U_http_info.query_len == 0) USSIPlugIn::setBadRequest();
   else
      {
      U_INTERNAL_ASSERT(*db_filter_tavarnelle)

      db_user_filter_tavarnelle = true;

       db_user->setFilterToFunctionToCall(WiAuthUser::dbUserFilter);
      db_nodog->setFilterToFunctionToCall(WiAuthNodog::dbNodogFilter);

      if (U_HTTP_QUERY_MEMEQ("admin_edit_ap")) // STATUS ACCESS POINT (Edit)
         {
         POST_admin_edit_ap();
         }
      else
         {
         USSIPlugIn::setMessagePage(admin_cache->getContent(U_CONSTANT_TO_PARAM("message_page.tmpl")), "Servizio non disponibile (non ancora implementato). Riprovare piu' tardi");
         }

      db_user_filter_tavarnelle = false;

       db_user->resetFilterToFunctionToCall();
      db_nodog->resetFilterToFunctionToCall();
      }
}

static void GET_tavarnelle()
{
   U_TRACE_NO_PARAM(5, "::GET_tavarnelle()")

   U_INTERNAL_DUMP("query = %.*S", U_HTTP_QUERY_TO_TRACE)

   if (U_http_info.query_len == 0) USSIPlugIn::setBadRequest();
   else
      {
      U_INTERNAL_ASSERT(*db_filter_tavarnelle)

      db_user_filter_tavarnelle = true;

       db_user->setFilterToFunctionToCall(WiAuthUser::dbUserFilter);
      db_nodog->setFilterToFunctionToCall(WiAuthNodog::dbNodogFilter);

      if (U_HTTP_QUERY_STREQ("admin_status_network")) // STATUS NETWORK
         {
         GET_admin_status_network();
         }
      else if (U_HTTP_QUERY_STREQ("admin_status_nodog")) // STATUS ACCESS POINT
         {
         GET_admin_status_nodog();
         }
      else if (U_HTTP_QUERY_MEMEQ("admin_edit_ap")) // STATUS ACCESS POINT (Edit)
         {
         GET_admin_edit_ap();
         }
      else if (U_HTTP_QUERY_STREQ("admin_login_nodog")) // STATUS USER ACCESS
         {
         GET_admin_login_nodog();
         }
      else if (U_HTTP_QUERY_STREQ("admin_login_nodog_historical")) // STATUS USER ACCESS (storico) 
         {
         GET_admin_login_nodog_historical();
         }
      else if (U_HTTP_QUERY_MEMEQ("admin_login_nodog_historical_view_data")) // STATUS USER ACCESS (storico) 
         {
         GET_admin_login_nodog_historical_view_data();
         }
      else if (U_HTTP_QUERY_STREQ("admin_view_statistics_login")) // STATISTICHE: Accessi
         {
         GET_admin_view_statistics_login();
         }
      else if (U_HTTP_QUERY_STREQ("admin_view_statistics_login")) // STATISTICHE: Accessi
         {
         GET_admin_view_statistics_login();
         }
      else if (U_HTTP_QUERY_STREQ("admin_current_status_ap")) // SITUAZIONE CORRENTE
         {
         GET_admin_current_status_ap();
         }
      else if (U_HTTP_QUERY_STREQ("admin_continuing_status_ap")) // MONITORAGGIO CONTINUATIVO
         {
         GET_admin_continuing_status_ap();
         }
      else
         {
         USSIPlugIn::setMessagePage(admin_cache->getContent(U_CONSTANT_TO_PARAM("message_page.tmpl")), "Servizio non disponibile (non ancora implementato). Riprovare piu' tardi");
         }

      db_user_filter_tavarnelle = false;

       db_user->resetFilterToFunctionToCall();
      db_nodog->resetFilterToFunctionToCall();
      }
}

// RESYNC

static void GET_resync()
{
   U_TRACE_NO_PARAM(5, "::GET_resync()")

   if (ip_server->equal(U_CLIENT_ADDRESS_TO_PARAM) == false) USSIPlugIn::setBadRequest();
   else
      {
      if (UServer_Base::startParallelization()) return; // parent

      resync();

      USSIPlugIn::setAlternativeResponse();
      }
}

// ERROR

static void GET_error_ap()
{
   U_TRACE_NO_PARAM(5, "::GET_error_ap()")

   // $1 -> ap (without localization => '@')
   // $2 -> public address to contact the access point
   // $3 -> msg: out of memory condition

   if (setAccessPoint(false, false))
      {
      uint32_t end = UHTTP::form_name_value->size();

      if (end == 6)
         {
         UString msg(200U);

         UHTTP::getFormValue(msg, U_CONSTANT_TO_PARAM("msg"), 0, 5, end);

         if (msg.c_char(0) == 'a') U_LOGGER("*** ON AP(%v:%v) NODOG IS OUT OF MEMORY *** %v",      ap_address->rep, ap_hostname->rep, msg.rep);
         else                      U_LOGGER("*** ON AP(%v:%v) PREALLOCATION(%v) IS EXHAUSTED ***", ap_address->rep, ap_hostname->rep, msg.rep);
         }
      else
         {
         U_LOGGER("*** ON AP(%v:%v) THE FIREWALL IS NOT ALIGNED ***", ap_address->rep, ap_hostname->rep);

         UString result = nodog_rec->sendRequestToNodog(U_CONSTANT_TO_PARAM("users"), 0);

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
                  U_NEW(UIPAllow, elem, UIPAllow);

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

            db_user->callForAllEntry(WiAuthUser::checkStatusUserOnNodog);

                      vuid->clear();
            vallow_IP_user->clear();
            }
         }
      }

   USSIPlugIn::setAlternativeResponse();
}

#endif
