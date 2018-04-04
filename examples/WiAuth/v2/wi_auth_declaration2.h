// wi_auth_declaration2.h

#ifndef U_WI_AUTH_DECLARATION2_H
#define U_WI_AUTH_DECLARATION2_H 1

static UString* ip;
static UString* mac;

static UString* ap;
static UString* ap_label;
static UString* ap_address;
static UString* ap_hostname;
static UString* nodog_conf;
static UString* key_session;
static UString* db_anagrafica;
static UString* allowed_web_hosts;

static UFile* file_LOG;
static UFile* file_WARNING;

static UREDISClient_Base* rc;
static UHttpClient<UTCPSocket>* client;

static UHTTP::UFileCacheData*    deny_html;
static UHTTP::UFileCacheData* welcome_html;

static void* peer;
static UPing* sockp;
static bool ap_consume;
static UEventTime* wclean;
static UString* policySessionId;
static uint8_t policySessionNotify;
static uint64_t counter, device_counter;
static uint32_t addr, created, lastUpdate, lastReset;

#define U_TEST
#define U_CLEAN_INTERVAL (60 * 60) // 1h 
#define U_MAX_TIME_NO_TRAFFIC (15 * 60) // 15m

#ifdef U_TEST
#  define U_MAX_TRAFFIC_DAILY (50ULL * 1024ULL * 1024ULL) // 50M
#else
#  define U_MAX_TRAFFIC_DAILY (500ULL * 1024ULL * 1024ULL) // 500M
#endif

#define U_LOGGER(fmt,args...) ULog::log(file_WARNING->getFd(), U_CONSTANT_TO_PARAM("%v: " fmt), UClientImage_Base::request_uri->rep , ##args)

/**
enum UPolicy {
   U_NOTIFY        = 0x00,
   U_NO_NOTIFY     = 0x01,
   U_STRICT_NOTIFY = 0x02,
   U_HINERIT       = 0x03
};
*/

class WiAuthClean : public UEventTime {
public:

   WiAuthClean() : UEventTime(U_CLEAN_INTERVAL, 0L)
      {
      U_TRACE_REGISTER_OBJECT(0, WiAuthClean, "", U_NULLPTR)
      }

   virtual ~WiAuthClean() U_DECL_FINAL
      {
      U_TRACE_UNREGISTER_OBJECT(0, WiAuthClean)
      }

   // define method VIRTUAL of class UEventTime

   virtual int handlerTime() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "WiAuthClean::handlerTime()")

      pid_t pid = UServer_Base::startNewChild();

      if (pid > 0) U_RETURN(0); // parent

      // child

      uint8_t status;
      UString url(100U);

      /*
      for (uint32_t i = 0, n = vcaptive->size(); i < n; ++i)
         {
         url.snprintf(U_CONSTANT_TO_PARAM("http://%v:5280/ping"), vcaptive->at(i).rep);

         // NB: we need PREFORK_CHILD > 2

         if (client->connectServer(url) &&
             client->sendRequest())
            {
            status = 1;

            client->UClient_Base::close();
            }
         else
            {
            status = 0; // NB: nodog not respond

            if (sockp)
               {
               UIPAddress laddr;

               if (client->remoteIPAddress(laddr) &&
                   sockp->ping(laddr))
                  {
                  status = 2; // NB: nodog not respond but pingable => unreachable...
                  }
               }
            }

         client->reset();
         }
      */

      if (pid == 0) UServer_Base::endNewChild();

      U_RETURN(0);
      }

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool _reset) const { return UEventTime::dump(_reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(WiAuthClean)
};

static void usp_init_wi_auth2()
{
   U_TRACE_NO_PARAM(5, "::usp_init_wi_auth2()")

   // ANAGRAFICA

   U_NEW(UString, db_anagrafica, UString(UFile::contentOf(U_STRING_FROM_CONSTANT("../anagrafica.txt"))));

   if (db_anagrafica->empty())
      {
      U_ERROR("usp_init_wi_auth2(): ../anagrafica.txt empty");
      }

   // NODOG config template

   U_NEW(UString, nodog_conf, UString(UFile::contentOf(U_STRING_FROM_CONSTANT("../ap/nodog.conf.template"))));

   if (nodog_conf->empty())
      {
      U_ERROR("usp_init_wi_auth2(): ../ap/nodog.conf.template empty");
      }

   UString content = UFile::contentOf(U_STRING_FROM_CONSTANT("../DES3_KEY.txt"));

   if (content.empty())
      {
      U_ERROR("usp_init_wi_auth2(): ../DES3_KEY.txt empty");
      }

   UDES3::setPassword(content.c_strndup());

   welcome_html = UHTTP::getFileCachePointer(U_CONSTANT_TO_PARAM("welcome.html"));

   if (welcome_html == U_NULLPTR)
      {
      U_ERROR("usp_init_wi_auth2(): missing welcome.html from cache");
      }

   deny_html = UHTTP::getFileCachePointer(U_CONSTANT_TO_PARAM("deny.html"));

   if (deny_html == U_NULLPTR)
      {
      U_ERROR("usp_init_wi_auth2(): missing deny.html from cache");
      }

   // REDIS client

   U_NEW(UREDISClient<UTCPSocket>, rc, UREDISClient<UTCPSocket>);

   if (rc->connect() == false)
      {
      U_ERROR("usp_init_wi_auth2(): %V", rc->UClient_Base::getResponse().rep);
      }

   U_NEW(UString, allowed_web_hosts, UString);

   content = UFile::contentOf(U_STRING_FROM_CONSTANT("../etc/AllowedWebHosts.txt"));

   if (content)
      {
      UVector<UString> vec(content);

      if (vec.empty() == false) *allowed_web_hosts = vec.join() + ' ';
      }

   U_NEW(UString, ip, UString);
   U_NEW(UString, mac, UString);

   U_NEW(UString, ap, UString);
   U_NEW(UString, ap_label, UString);
   U_NEW(UString, ap_address, UString);
   U_NEW(UString, ap_hostname, UString);
   U_NEW(UString, key_session, UString);
   U_NEW(UString, policySessionId, UString);

   U_NEW(UFile, file_LOG,     UFile(U_STRING_FROM_CONSTANT("../log/wifi-log")));
   U_NEW(UFile, file_WARNING, UFile(U_STRING_FROM_CONSTANT("../log/wifi-warning")));

   UServer_Base::update_date  =
   UServer_Base::update_date1 = true;

   (void) UServer_Base::addLog(file_LOG);
   (void) UServer_Base::addLog(file_WARNING);

   // HTTP client

   U_NEW(UHttpClient<UTCPSocket>, client, UHttpClient<UTCPSocket>(U_NULLPTR));

   client->setFollowRedirects(true, false);
   client->getResponseHeader()->setIgnoreCase(false);

   U_ASSERT_EQUALS(client->isPasswordAuthentication(), false)

   UVector<UString> vnetmask;
   UTokenizer tok(*db_anagrafica, ",\n");
   UString lip, lnetmask, lbl, lnetmask1, lbl1;

   /**
    * 10.8.0.13,172.17.13.0/24,368
    * 10.8.0.13,172.16.13.0/24,368
    * 10.8.0.255,172.16.255.0/24,51
    * 151.11.47.3,172.23.0.0/20,47
    * 151.11.47.3,172.23.0.0/20,213
    * 192.168.44.55,172.16.55.0/24,55
    * 192.168.44.55,172.16.65.0/24,65
    */

   (void) tok.next(lip, (bool*)U_NULLPTR);

   U_INTERNAL_ASSERT(lip)

loop:
   (void) tok.next(lnetmask, (bool*)U_NULLPTR);
   (void) tok.next(lbl,      (bool*)U_NULLPTR);

   U_INTERNAL_ASSERT(lbl)
   U_INTERNAL_ASSERT(lnetmask)

   U_ASSERT(vnetmask.empty())

   vnetmask.push_back(lnetmask);

   U_INTERNAL_DUMP("lip = %V lbl = %V lnetmask = %V", lip.rep, lbl.rep, lnetmask.rep)

   *ip = lip;

   (void) UIPAddress::getBinaryForm(ip->c_str(), addr, true);

   (void) rc->hmset(U_CONSTANT_TO_PARAM("CAPTIVE:id:%u ip %v"), addr, ip->rep);

   (void) rc->zadd(U_CONSTANT_TO_PARAM("CAPTIVE:byId 0 id:%u"), addr);

   (void) rc->hmset(U_CONSTANT_TO_PARAM("AP:id:%v captiveId %u network %v"), lbl.rep, addr, lnetmask.rep);

   (void) rc->zadd(U_CONSTANT_TO_PARAM("AP:byCaptiveId %u id:%v;network:%v"), addr, lbl.rep, lnetmask.rep);

   while (tok.next(lip, (bool*)U_NULLPTR))
      {
      if (ip->equal(lip) == false)
         {
         if (vnetmask.size() > 1)
            {
            (void) rc->hmset(U_CONSTANT_TO_PARAM("AP:id:%v network %v"), lbl.rep, vnetmask.join(0, U_CONSTANT_TO_PARAM(",")).rep);
            }

         vnetmask.clear();

         goto loop;
         }

      (void) tok.next(lnetmask1, (bool*)U_NULLPTR);
      (void) tok.next(lbl1,      (bool*)U_NULLPTR);

      U_INTERNAL_ASSERT(lbl1)
      U_INTERNAL_ASSERT(lnetmask1)

      if (lbl1 == lbl)
         {
         U_ASSERT_DIFFERS(lnetmask1, lnetmask)

         vnetmask.push_back(lnetmask1);

         continue;
         }

      (void) rc->hmset(U_CONSTANT_TO_PARAM("AP:id:%v captiveId %u network %v"), lbl1.rep, addr, lnetmask1.rep);

      (void) rc->zadd(U_CONSTANT_TO_PARAM("AP:byCaptiveId %u id:%v;network:%v"), addr, lbl1.rep, lnetmask1.rep);

      vnetmask.clear();

      lbl      = lbl1;
      lnetmask = lnetmask1;
      }

   // CLEAN MONITORING

   U_NEW(UPing, sockp, UPing(5000, UClientImage_Base::bIPv6));

   if (sockp->initPing() == false)
      {
      delete sockp;
             sockp = U_NULLPTR;
      }

   U_NEW(WiAuthClean, wclean, WiAuthClean);

   UTimer::insert(wclean);
}

static void usp_end_wi_auth2()
{
   U_TRACE_NO_PARAM(5, "::usp_end_wi_auth2()")

#ifdef DEBUG
   delete ip;
   delete mac;

   delete ap;
   delete ap_label;
   delete ap_address;
   delete ap_hostname;
   delete nodog_conf;
   delete key_session;
   delete policySessionId;
   delete allowed_web_hosts;

   delete db_anagrafica;

   delete rc;
   delete client;
   delete wclean;
#endif
}

static bool setLabelAndNetmaskFromAnagrafica(UString& label, UString& netmask)
{
   U_TRACE(5, "::setLabelAndNetmaskFromAnagrafica(%p,%p,%p)", &label, &netmask)

   U_INTERNAL_DUMP("ap_address = %V", ap_address)

   U_INTERNAL_ASSERT(*ap_address)

   uint32_t pos = 0;

   /**
    * 159.213.248.233,172.25.0.0/22,213
    */

loop:
   pos = db_anagrafica->find(*ap_address, pos);

   U_INTERNAL_DUMP("pos = %u", pos)

   if (pos == U_NOT_FOUND) U_RETURN(false);

   pos += ap_address->size();

   if (db_anagrafica->c_char(pos) != ',') goto loop;

   if (pos != U_NOT_FOUND)
      {
      UVector<UString> vlabel, vnetmask;
      UString lip, lnetmask, lbl, lnetmask1, lbl1;
      UTokenizer tok(db_anagrafica->substr(pos), ",\n");

      /**
       * 159.213.248.233,172.25.0.0/22,213
       */

      (void) tok.next(lnetmask, (bool*)U_NULLPTR);
      (void) tok.next(lbl,      (bool*)U_NULLPTR);

      U_INTERNAL_ASSERT(lbl)
      U_INTERNAL_ASSERT(lnetmask)

        vlabel.push_back(lbl);
      vnetmask.push_back(lnetmask);

      U_INTERNAL_DUMP("tok.current() = %C", tok.current())

      /**
       * 151.11.47.3,172.23.0.0/20,47
       * 151.11.47.3,172.23.0.0/20,215
       * 151.11.47.3,172.23.0.0/20,216
       */

      while (tok.next(lip, (bool*)U_NULLPTR) &&
             ap_address->equal(lip))
         {
         (void) tok.next(lnetmask1, (bool*)U_NULLPTR);
         (void) tok.next(lbl1,      (bool*)U_NULLPTR);

         U_INTERNAL_ASSERT(lbl1)
         U_INTERNAL_ASSERT(lnetmask1)

                                      vlabel.push_back(lbl1);
         if (lnetmask != lnetmask1) vnetmask.push_back(lnetmask1);
         }

      /**
       * 10.8.0.13,172.17.13.0/24,368
       * 10.8.0.13,172.16.13.0/24,368
       */

      U_DUMP("vlabel.size() = %u vnetmask.size() = %u", vlabel.size(), vnetmask.size())

      if (vlabel.size() == vnetmask.size())
         {
           label =   vlabel.join();
         netmask = vnetmask.join();
         }
      else
         {
         U_ASSERT_EQUALS(vnetmask.size(), 1)

          label  =   vlabel[0];
         netmask = vnetmask[0];
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

static bool setAccessPoint()
{
   U_TRACE_NO_PARAM(5, "::setAccessPoint()")

   U_INTERNAL_ASSERT(ap)

      ap_label->clear();
    ap_address->clear();
   ap_hostname->clear();

   uint32_t pos1 = ap->find('@');

   if (pos1 == U_NOT_FOUND) U_RETURN(false);

   *ap_label = ap->substr(0U, pos1).copy();

   uint32_t pos2 = ap->find('/', pos1+1);

   if (pos2 == U_NOT_FOUND) *ap_hostname = ap->substr(pos1+1).copy();
   else
      {
      *ap_address = ap->substr(pos1+1, pos2-pos1-1).copy();

      if (*ap_address &&
          UIPAddress::getBinaryForm(ap_address->c_str(), addr, true) == false)
         {
         U_LOGGER("*** AP ADDRESS(%v) NOT VALID ***", ap_address->rep);

         U_RETURN(false);
         }

      *ap_hostname = ap->substr(pos2+1).copy();
      }

   if (u_isHostName(U_STRING_TO_PARAM(*ap_hostname)) == false)
      {
      U_LOGGER("*** AP HOSTNAME(%v) NOT VALID ***", ap_hostname->rep);

      U_RETURN(false);
      }

   U_INTERNAL_DUMP("ap_address = %V ap_hostname = %V ap_label = %V", ap_address->rep, ap_hostname->rep, ap_label->rep)

   U_RETURN(true);
}

static inline void setSessionkey()
{ key_session->setBuffer(200U); key_session->snprintf(U_CONSTANT_TO_PARAM("captiveId:%u;apId:%v;deviceId:%v;ip:%v"), addr, ap_label->rep, mac->rep, ip->rep); }

static void setSessionPolicy()
{
   U_TRACE_NO_PARAM(5, "::setSessionPolicy()")

   U_INTERNAL_DUMP("ap_label = %V mac = %V", ap_label->rep, mac->rep)

   (void) rc->hmget(U_CONSTANT_TO_PARAM("DEVICE:id:%v pId pNotify"), mac->rep);

   if ((*policySessionId = rc->getString())) rc->setUInt8(policySessionNotify, 1);
   else
      {
      *policySessionId = U_STRING_FROM_CONSTANT("DAILY");

      policySessionNotify = 0; // notify

      (void) rc->hmset(U_CONSTANT_TO_PARAM("DEVICE:id:%v id %v pId DAILY pNotify 0 pCounter 0 lastAccess %u pLastReset %u"), mac->rep, mac->rep, u_now->tv_sec, u_now->tv_sec);
      }

   U_INTERNAL_DUMP("policySessionId = %V policySessionNotify = %u", policySessionId->rep, policySessionNotify)

   U_INTERNAL_ASSERT(*policySessionId)

   // reset fields SESSION

   counter    = 0;
   created    =
   lastUpdate = u_now->tv_sec;

   (void) rc->hmget(U_CONSTANT_TO_PARAM("AP:id:%v consume notify"), ap_label->rep);

   uint8_t ap_notify;

   if (rc->getResult())
      {
      ap_consume = rc->x.strtob();

      rc->setUInt8(ap_notify, 1);
      }
   else
      {
      ap_consume = true;
      ap_notify  = 0; // notify

      (void) rc->hmset(U_CONSTANT_TO_PARAM("AP:id:%v consume 1 notify 3"), ap_label->rep);
      }

   U_INTERNAL_DUMP("ap_consume = %b ap_notify = %u", ap_consume, ap_notify)

   switch (ap_notify)
      {
      case 0: policySessionNotify = 0; break; // (notify)
      case 1: policySessionNotify = 1; break; // (no notify)
      case 2: policySessionNotify = 2; break; // (strict notify)
   // case 3:                          break; // (hinerit)
      }

   U_INTERNAL_DUMP("policySessionNotify = %u", policySessionNotify)
}

static bool getSession()
{
   U_TRACE_NO_PARAM(5, "::getSession()")

   (void) rc->hmget(U_CONSTANT_TO_PARAM("SESSION:%v created pId notify consume counter lastUpdate captiveId apId deviceId ip"), key_session->rep);

   if (rc->getResult())
      {
      created          = rc->x.strtoul();
      *policySessionId = rc->getString(1);

      rc->setUInt8(policySessionNotify, 2);

      ap_consume = rc->getBool(3);
      counter    = rc->getUInt64(4);
      lastUpdate = rc->getULong(5);

      U_INTERNAL_DUMP("policySessionId = %V ap_consume = %b counter = %llu created = %#2D lastUpdate = %#2D (%u <=> midnigth %u)",
                       policySessionId->rep, ap_consume, counter, created, lastUpdate, u_get_localtime(lastUpdate) / U_ONE_DAY_IN_SECOND, u_getLocalTime() / U_ONE_DAY_IN_SECOND)

      U_INTERNAL_ASSERT(*policySessionId)

      U_RETURN(true);
      }

   U_RETURN(false);
}

static UString getApInfo()
{
   U_TRACE_NO_PARAM(5, "::getApInfo()")

   UString x(1024U);

   x.snprintf(U_CONSTANT_TO_PARAM("%v@%v/%v"), ap_label->rep, ap_address->rep, ap_hostname->rep);

   U_RETURN_STRING(x);
}

static void writeToLOG(const char* op, uint32_t op_len, const UString& opt)
{
   U_TRACE(5, "::writeToLOG(%.*S,%u,%V)", op_len, op, op_len, opt.rep)

   U_INTERNAL_ASSERT_POINTER(op)
   U_INTERNAL_ASSERT_MAJOR(op_len, 0)

   // 11/02/18 04:14:21 op: PERMIT, mac: 20:ee:28:d0:38:93, ip: 172.22.155.28, ap: 61@151.11.47.8/CareggiConc-x86_64, policy: FLAT|no_consume|notify
   // 11/02/18 04:14:21 op: NOTIFIED, mac: 20:ee:28:d0:38:93, ip: 172.22.155.28, ap: 61@151.11.47.8/CareggiConc-x86_64, policy: FLAT|no_consume|notify
   // 11/02/18 04:14:21 op: RST_POLICY, mac: 20:ee:28:d0:38:93, ip: 172.22.155.28, ap: 61@151.11.47.8/CareggiConc-x86_64, policy: DAILY|consume|notify, traffic: 100
   // 11/02/18 04:14:21 op: DENY_NO_TRAFFIC, mac: 20:ee:28:d0:38:93, ip: 172.22.155.28, ap: 61@151.11.47.8/CareggiConc-x86_64, policy: FLAT|consume|notify, traffic: 100, elapsed: 10

   ULog::log(file_LOG->getFd(),
             U_CONSTANT_TO_PARAM("op: %.*s, mac: %v, ip: %v, ap: %v, policy: %v|%.*sconsume|%snotify%v"),
             op_len, op, mac->rep, ip->rep, getApInfo().rep, policySessionId->rep, (ap_consume ? 0 : 3), "no_", (policySessionNotify == 0 ? ""    :
                                                                                                                 policySessionNotify == 1 ? "no_" : "strict_"), opt.rep);
}

static void writeSessionToLOG(const char* op, uint32_t op_len)
{
   U_TRACE(5, "::writeSessionToLOG(%.*S,%u)", op_len, op, op_len)

   UString opt(200U);

   opt.snprintf(U_CONSTANT_TO_PARAM(", traffic: %llu, elapsed: %u"), counter/1024, (u_now->tv_sec-created)/60);

   writeToLOG(op, op_len, opt);
}

static void deleteSession()
{
   U_TRACE_NO_PARAM(5, "::deleteSession()")

   (void) rc->del(U_CONSTANT_TO_PARAM("SESSION:%v"), key_session->rep);
   (void) rc->zrem(U_CONSTANT_TO_PARAM("SESSION:byCaptiveIdAndApId deviceId:%v;ip:%v"), mac->rep, ip->rep);
   (void) rc->zrem(U_CONSTANT_TO_PARAM("SESSION:byLastUpdate %v"), key_session->rep);
}

static void resetDeviceDailyCounter()
{
   U_TRACE_NO_PARAM(5, "::resetDeviceDailyCounter()")

   U_ASSERT(policySessionId->equal(U_CONSTANT_TO_PARAM("DAILY")))

   (void) rc->hmget(U_CONSTANT_TO_PARAM("DEVICE:id:%v pCounter pLastReset"), mac->rep);

   device_counter = rc->getUInt64();
   lastReset      = rc->getULong(1);

   if ((u_get_localtime(lastReset) / U_ONE_DAY_IN_SECOND) < (u_getLocalTime() / U_ONE_DAY_IN_SECOND))
      {
      device_counter = 0;

      (void) rc->hmset(U_CONSTANT_TO_PARAM("DEVICE:id:%v pCounter 0 pLastReset %u"), mac->rep, u_now->tv_sec);
      }
}

static bool checkDevice()
{
   U_TRACE_NO_PARAM(5, "::checkDevice()")

   if (ap_consume &&
       policySessionId->equal(U_CONSTANT_TO_PARAM("DAILY")))
      {
      resetDeviceDailyCounter();

      if (device_counter >= U_MAX_TRAFFIC_DAILY) U_RETURN(false);
      }

   U_RETURN(true);
}

static bool sendRequestToNodog(const char* req, uint32_t req_len, const UString& data)
{
   U_TRACE(5, "::sendRequestToNodog(%.*S,%u,%V)", req_len, req, req_len, data.rep)

   UString url(100U);

   url.snprintf(U_CONSTANT_TO_PARAM("http://%v:5280/%.*s"), ap_address->rep, req_len, req);

   return client->sendPOSTRequestAsync(data, url);
}

static bool getDataFromPOST(bool bpeer)
{
   U_TRACE(5, "::getDataFromPOST(%b)", bpeer)

   // $1 -> ap (without localization => '@')
   // $2 -> mac
   // $3 -> ip
   // $4 -> peer

   UFlatBuffer fb, vec;

   fb.setRoot(*UClientImage_Base::body);
   fb.AsVector(vec);

   *ap = vec.AsVectorGet<UString>(0);

   if (*ap &&
       setAccessPoint())
      {
      *mac = vec.AsVectorGet<UString>(1);
      *ip  = vec.AsVectorGet<UString>(2);

      U_INTERNAL_ASSERT(u_isIPv4Addr(U_STRING_TO_PARAM(*ip)))

      setSessionPolicy();

      if (bpeer)
         {
         peer = (void*) vec.AsVectorGet<uint64_t>(3);

         if (checkDevice() == false) U_RETURN(false);
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}

static void GET_get_config()
{
   U_TRACE_NO_PARAM(5, "::GET_get_config()")

   // $1 -> ap (without localization => '@')
   // $2 -> key

   UString body;

   if (UHTTP::processForm() == 2*2)
      {
      UString key;

      UHTTP::getFormValue(key, U_CONSTANT_TO_PARAM("key"), 0, 3, 4);

      if (key)
         {
         UString buffer(U_CAPACITY);

         buffer.snprintf(U_CONSTANT_TO_PARAM("../ap/%v/nodog.conf"), key.rep);

         body = UFile::contentOf(buffer);

         if (body.empty())
            {
            body = *nodog_conf;

            UHTTP::getFormValue(*ap, U_CONSTANT_TO_PARAM("ap"), 0, 1, 4);

            UVector<UString> vec_subst;

            vec_subst.push_back(U_STRING_FROM_CONSTANT("<LAN>"));

                 if (UStringExt::endsWith(*ap, U_CONSTANT_TO_PARAM("_pico2"))) vec_subst.push_back(U_STRING_FROM_CONSTANT("ath0"));
            else if (UStringExt::endsWith(*ap, U_CONSTANT_TO_PARAM("_x86")))   vec_subst.push_back(U_STRING_FROM_CONSTANT("eth1"));
            else if (UStringExt::endsWith(*ap, U_CONSTANT_TO_PARAM("_rspro")))
               {
               if (UStringExt::startsWith(*ap, U_CONSTANT_TO_PARAM("wimo"))) vec_subst.push_back(U_STRING_FROM_CONSTANT("br-lan"));
               else                                                          vec_subst.push_back(U_STRING_FROM_CONSTANT("eth1"));
               }
            else
               {
               vec_subst.push_back(U_STRING_FROM_CONSTANT("wlan0")); // ..._picoM2, ..._locoM2, ..._bulletM2, ..._unifiAP
               }

            if (u_isIPv4Addr(U_STRING_TO_PARAM(key))) *ap_address = key;
            else                                (void) ap_address->assign(U_CLIENT_ADDRESS_TO_PARAM);

            buffer.snprintf(U_CONSTANT_TO_PARAM("../ap/%v/nodog.conf.local"), ap_address->rep);

            UVector<UString> vec(*ap_address, '.');
            UString lbl, netmask, local = UFile::contentOf(buffer), address_mask(32U);

            address_mask.snprintf(U_CONSTANT_TO_PARAM("%u.%v"), 16 + vec[2].strtoul(), vec[3].rep);

            if (setLabelAndNetmaskFromAnagrafica(lbl, netmask))
               {
               buffer.snprintf(U_CONSTANT_TO_PARAM("LOCAL_NETWORK_LABEL \"%v\""), lbl.rep);

               vec_subst.push_back(U_STRING_FROM_CONSTANT("LOCAL_NETWORK_LABEL ap"));
               vec_subst.push_back(buffer);

               if (local)
                  {
                  UString tmp(200U + local.size() + netmask.size() + lbl.size());

                  tmp.snprintf(U_STRING_TO_PARAM(local), netmask.rep, lbl.rep);

                  local = tmp;
                  }
               }

            vec_subst.push_back(U_STRING_FROM_CONSTANT("#include \"../ap/<AAA.BBB.CCC.DDD>/nodog.conf.local\""));
            vec_subst.push_back(local);
            vec_subst.push_back(U_STRING_FROM_CONSTANT("<AAA.BBB.CCC.DDD>"));
            vec_subst.push_back(*ap_address);
            vec_subst.push_back(U_STRING_FROM_CONSTANT("<CCC>.<DDD>"));
            vec_subst.push_back(address_mask);

            UFileConfig cfg(UStringExt::substitute(body, vec_subst), true);

            UHTTP::mime_index = U_know;

            if (cfg.processData(false)) body = cfg.getData();
            }
         }
      }

   UHTTP::setResponseBody(body);
}

static void GET_start_ap()
{
   U_TRACE_NO_PARAM(5, "::GET_start_ap()")

   /**
    * $1 -> ap (with localization => '@')
    * $2 -> public address to contact the access point
    * $3 -> pid (0 => start)
    * $4 -> uptime
    *
    * GET /start_ap?ap=1000@192.168.42.136/stefano&public=U2FsdGVkX1-UKZ7S9pzvcFnzkMVr_P428Pw7Cf_Ptm8%3A5280&pid=-1&uptime=1234567890
    */

   if (UHTTP::processForm() == 4*2)
      {
      ap->clear();

      UHTTP::getFormValue(*ap, U_CONSTANT_TO_PARAM("ap"), 0, 1, 8);

      if (*ap &&
          setAccessPoint())
         {
         uint32_t pos;
         UString address;

         UHTTP::getFormValue(address, U_CONSTANT_TO_PARAM("public"), 0, 3, 8);

         if (address)
            {
            pos = address.find(':');

            if (pos == U_NOT_FOUND ||
                address.substr(pos+1).strtoul() != 5280)
               {
               U_LOGGER("*** CAPTIVE ADDRESS_PUBLIC(%v) NOT VALID ***", address.rep);

               return;
               }

            UString IP_address_trust = address.substr(0U, pos).copy();

            if (IP_address_trust.isBase64Url() == false)
               {
               U_LOGGER("*** CAPTIVE ADDRESS_TRUST(%v) NOT BASE64 ***", IP_address_trust.rep);

               return;
               }

            address = UDES3::getSignedData(IP_address_trust);

            if (u_isIPv4Addr(U_STRING_TO_PARAM(address)) == false)
               {
               U_LOGGER("*** CAPTIVE ADDRESS_TRUST(%v) NOT VALID ***", IP_address_trust.rep);

               return;
               }

            if (ap_address->equal(address) == false)
               {
               U_LOGGER("*** CAPTIVE ADDRESS(%v) NOT EQUAL TO %V ***", ap_address->rep, address.rep);

               return;
               }
            }

         UString x(200U);

         x.snprintf(U_CONSTANT_TO_PARAM("CAPTIVE:id:%u"), addr);

         if (rc->exists(U_STRING_TO_PARAM(x)) == false)
            {
            U_LOGGER("*** %v NOT FOUND ***", x.rep);

            return;
            }

         UString pid, uptime;

         UHTTP::getFormValue(pid,    U_CONSTANT_TO_PARAM("pid"),    0, 5, 8);
         UHTTP::getFormValue(uptime, U_CONSTANT_TO_PARAM("uptime"), 0, 7, 8);

         U_LOGGER("%v %s", getApInfo().rep, pid.strtol() == -1 ? "started" : "*** NODOG CRASHED ***");

         (void) rc->hmset(U_CONSTANT_TO_PARAM("CAPTIVE:id:%u name %v status 1 uptime %v since %u lastUpdate %u"), addr, ap_hostname->rep, uptime.rep, u_now->tv_sec, u_now->tv_sec);

         x.snprintf(U_CONSTANT_TO_PARAM("%u"), addr);

         (void) rc->zrangebyscore(U_CONSTANT_TO_PARAM("SESSION:byCaptiveIdAndApId %v000000 %v999999 WITHSCORES"), x.rep, x.rep);

         uint32_t n = rc->vitem.size();

         if (n)
            {
            const char* ptr1;
            const char* ptr2;
            UVector<UString> vec(n);

            vec.copy(rc->vitem);

            key_session->setBuffer(200U);

            x.snprintf(U_CONSTANT_TO_PARAM("captiveId:%u;apId:"), addr);

            for (uint32_t i = 0; i < n; i += 2)
               {
               ptr1 =
               ptr2 = vec[i+1].pend()-6; // score (ex: 3232246838000064)

               while (*ptr2 == '0') { ++ptr2; }

               (void) ap_label->assign(ptr2, 6-(ptr2-ptr1));

               key_session->snprintf(U_CONSTANT_TO_PARAM("%v%v;%v"), x.rep, ap_label->rep, vec[i].rep);

               if (getSession())
                  {
                  U_ASSERT_EQUALS(addr,      rc->getULong(6))
                  U_ASSERT_EQUALS(*ap_label, rc->getString(7))

                  *mac = rc->getString(8);
                  *ip  = rc->getString(9);

                  U_INTERNAL_ASSERT(u_isIPv4Addr(U_STRING_TO_PARAM(*ip)))

                  writeSessionToLOG(U_CONSTANT_TO_PARAM("DENY_LOST"));

                  deleteSession();
                  }
               }
            }
         }
      }

   UHTTP::setResponseBody(*allowed_web_hosts);
}

static void GET_welcome()
{
   U_TRACE_NO_PARAM(5, "::GET_welcome()")

   // $1 -> url
   // $2 -> mac
   // $3 -> apid
   // $4 -> gateway

   bool ok = false;

   if (UHTTP::processForm() == 4*2)
      {
      UHTTP::getFormValue(*mac,      U_CONSTANT_TO_PARAM("mac"),  0, 3, 8);
      UHTTP::getFormValue(*ap_label, U_CONSTANT_TO_PARAM("apid"), 0, 5, 8);

      setSessionPolicy();

      ok = checkDevice();
      }

   UHTTP::setResponseFromFileCache(ok ? welcome_html : deny_html);

   U_http_info.nResponseCode = HTTP_NO_CONTENT; // NB: to escape management after usp exit...
}

static void POST_login()
{
   U_TRACE_NO_PARAM(5, "::POST_login()")

   // $1 -> ap (with localization => '@')
   // $2 -> mac
   // $3 -> ip
   // $4 -> peer

   peer = U_NULLPTR;

   bool ko = (getDataFromPOST(true) == false);

   if (*ap_address)
      {
      UFlatBuffer fb;
      char buffer[2] = { '1'-ko, '0'+policySessionNotify }; // deny|permit: ('0'|'1') policy: notify|no_notify|strict_notify ('0'|'1'|'2')

      writeToLOG(U_CONSTANT_TO_PARAM("PERMIT"), UString::getStringNull());

      (void) rc->hmset(U_CONSTANT_TO_PARAM("DEVICE:id:%v lastAccess %u"), mac->rep, u_now->tv_sec);
      (void) rc->zadd(U_CONSTANT_TO_PARAM("DEVICE:bylastAccess %u id:%v"), u_now->tv_sec, mac->rep);

      if (ko) writeSessionToLOG(U_CONSTANT_TO_PARAM("DENY_POLICY"));
      else
         {
         setSessionkey();

         (void) rc->hmset(U_CONSTANT_TO_PARAM("SESSION:%v captiveId %u apId %v deviceId %v ip %v created %u pId %v notify %c consume %c counter 0 lastUpdate %u"), key_session->rep,
                          addr, ap_label->rep, mac->rep, ip->rep, u_now->tv_sec, policySessionId->rep, buffer[1], '0'+ap_consume, u_now->tv_sec);

         (void) rc->zadd(U_CONSTANT_TO_PARAM("SESSION:byCaptiveIdAndApId %u%06u deviceId:%v;ip:%v"), addr, ap_label->strtoul(), mac->rep, ip->rep);
         (void) rc->zadd(U_CONSTANT_TO_PARAM("SESSION:byLastUpdate %u %v"), u_now->tv_sec, key_session->rep);
         }

      // $1 -> peer
      // $2 -> deny|permit: ('0'|'1') policy: notify|no_notify|strict_notify ('0'|'1'|'2')

#  ifdef HAVE_CXX11
      (void) fb.encodeVector([&]() {
         fb.UInt(U_PTR2INT(peer));
         fb.String(buffer, sizeof(buffer));
      });
#  else
      fb.StartBuild();
      (void) fb.StartVector();

      fb.UInt(U_PTR2INT(peer));
      fb.String(buffer, sizeof(buffer));

      fb.EndVector(0, false);
      (void) fb.EndBuild();
#  endif

      (void) sendRequestToNodog(U_CONSTANT_TO_PARAM("login_validate"), fb.getResult());
      }
}

static void POST_notify()
{
   U_TRACE_NO_PARAM(5, "::POST_notify()")

   // $1 -> ap (with localization => '@')
   // $2 -> mac
   // $3 -> ip

   if (getDataFromPOST(false))
      {
      writeToLOG(U_CONSTANT_TO_PARAM("NOTIFIED"), UString::getStringNull());
      }
}

static void POST_strict_notify()
{
   U_TRACE_NO_PARAM(5, "::POST_strict_notify()")

   // $1 -> ap (with localization => '@')
   // $2 -> mac
   // $3 -> ip

   if (getDataFromPOST(false))
      {
      UString x(200U);

      x.snprintf(U_CONSTANT_TO_PARAM("DEVICE:id:%v pNotify"), mac->rep);

      (void) rc->hmget(x);

      if (rc->getUInt8() == 2) // (strict notify) => (notify) 
         {
         (void) x.append(U_CONSTANT_TO_PARAM(" 0"));

         (void) rc->hmset(x);
         }

      writeToLOG(U_CONSTANT_TO_PARAM("STRICT_NOTIFIED"), UString::getStringNull());
      }
}

/*
$1 -> mac
$2 -> ip
$3 -> apId
$4 -> time
$5 -> traffic
$6 -> time_no_traffic

struct PostInfo {

   UString ip, mac, ap_label;
   uint32_t _ctime, ctraffic, time_no_traffic;

   void clear()
      {
      U_TRACE_NO_PARAM(5, "PostInfo::clear()")

            ip.clear();
           mac.clear();
      ap_label.clear();

      _ctime = ctraffic = time_no_traffic = 0;
      }

   void fromFlatBuffer(UFlatBuffer& fb)
      {
      U_TRACE(5, "PostInfo::fromFlatBuffer(%p)", &fb)

      fb.fromFlatBuffer(0, FLATBUFFER(ip,              UString));
      fb.fromFlatBuffer(1, FLATBUFFER(mac,             UString));
      fb.fromFlatBuffer(2, FLATBUFFER(ap_label,        UString));
      fb.fromFlatBuffer(3, FLATBUFFER(_ctime,          uint32_t));
      fb.fromFlatBuffer(4, FLATBUFFER(ctraffic,        uint32_t));
      fb.fromFlatBuffer(5, FLATBUFFER(time_no_traffic, uint32_t));
      }
};
*/

static void POST_info()
{
   U_TRACE_NO_PARAM(5, "::POST_info()")

   UFlatBuffer fb, vec;

   fb.setRoot(*UClientImage_Base::body);
   fb.AsVector(vec);

   *ap = vec.AsVectorGet<UString>(0);

   if (*ap &&
       setAccessPoint())
      {
      const char* op;
      UString x(200U);
      UVector<UString> vec_logout;
      uint32_t _ctime, ctraffic, time_no_traffic, ctime_no_traffic, op_len, midnigth = u_getLocalTime() / U_ONE_DAY_IN_SECOND;

      (void) rc->hmset(U_CONSTANT_TO_PARAM("CAPTIVE:id:%u status 1 lastUpdate %u"), addr, u_now->tv_sec);

      for (int32_t i = 1, n = (int32_t) vec.GetSize(); i < n; i += 6)
         {
         // -----------------------------------------------------------------------------------------------------------------------------------------
         // $1 -> mac
         // $2 -> ip
         // $3 -> apId
         // $4 -> time
         // $5 -> traffic
         // $6 -> time_no_traffic
         // -----------------------------------------------------------------------------------------------------------------------------------------

         *mac            = vec.AsVectorGet<UString>(i);
         *ip             = vec.AsVectorGet<UString>(i+1);
         *ap_label       = vec.AsVectorGet<UString>(i+2);
         _ctime          = vec.AsVectorGet<uint32_t>(i+3);
         ctraffic        = vec.AsVectorGet<uint32_t>(i+4);
         time_no_traffic = vec.AsVectorGet<uint32_t>(i+5);

         U_INTERNAL_DUMP("ap_label = %V mac = %V ip = %V _ctime = %u ctraffic = %u time_no_traffic = %u", ap_label->rep, mac->rep, ip->rep, _ctime, ctraffic, time_no_traffic)

         U_INTERNAL_ASSERT(u_isIPv4Addr(U_STRING_TO_PARAM(*ip)))

         setSessionkey();

         if (getSession() == false) goto del_login;

         U_INTERNAL_DUMP("ap_label = %V mac = %V ip = %V", rc->getString(7).rep, rc->getString(8).rep, rc->getString(9).rep)

         U_INTERNAL_ASSERT_EQUALS(addr, rc->getULong(6))
         U_ASSERT_EQUALS(*ap_label,     rc->getString(7))
         U_ASSERT_EQUALS(*mac,          rc->getString(8))
         U_ASSERT_EQUALS(*ip,           rc->getString(9))

         if (ctraffic == 0)
            {
            ctime_no_traffic = (u_now->tv_sec - lastUpdate);

            U_DEBUG("Peer IP %v MAC %v has made no traffic for %u secs", ip->rep, mac->rep, ctime_no_traffic);

            if (ctime_no_traffic >= U_MAX_TIME_NO_TRAFFIC) // (time_no_traffic >= U_MAX_TIME_NO_TRAFFIC)
               {
               op     =                 "DENY_NO_TRAFFIC";
               op_len = U_CONSTANT_SIZE("DENY_NO_TRAFFIC");

               created += ctime_no_traffic;

del_sess:      writeSessionToLOG(op, op_len);

               deleteSession();

del_login:     vec_logout.push_back(*ip);
               vec_logout.push_back(*mac);

               continue;
               }
            }

         if ((u_get_localtime(lastUpdate) / U_ONE_DAY_IN_SECOND) < midnigth)
            {
            if (policySessionId->equal(U_CONSTANT_TO_PARAM("DAILY"))) resetDeviceDailyCounter();

            if (counter)
               {
               x.snprintf(U_CONSTANT_TO_PARAM(", traffic: %llu"), counter/1024);

               writeToLOG(U_CONSTANT_TO_PARAM("RST_SESSION"), x);

               counter = 0;

               (void) rc->hmset(U_CONSTANT_TO_PARAM("SESSION:%v counter 0"), key_session->rep);
               }
            }

         if (ctraffic)
            {
            (void) rc->hmset(U_CONSTANT_TO_PARAM("SESSION:%v counter %llu lastUpdate %u"), key_session->rep, counter+ctraffic, u_now->tv_sec);

            if (ap_consume                                           &&
                policySessionId->equal(U_CONSTANT_TO_PARAM("DAILY")) &&
                rc->hincrby(U_CONSTANT_TO_PARAM("DEVICE:id:%v pCounter %llu"), mac->rep, ctraffic) >= U_MAX_TRAFFIC_DAILY)
               {
               op     =                 "DENY_POLICY";
               op_len = U_CONSTANT_SIZE("DENY_POLICY");

               goto del_sess;
               }
            }
         }

      if (vec_logout.empty() == false)
         {
         UFlatBufferSpace space;

         (void) sendRequestToNodog(U_CONSTANT_TO_PARAM("logout"), UDES3::signData(UFlatBuffer::fromVector(vec_logout)));
         }
      }
}
#endif
