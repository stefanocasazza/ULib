// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_nodog.cpp - this is a plugin nodog for userver
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/date.h>
#include <ulib/file_config.h>
#include <ulib/utility/des3.h>
#include <ulib/utility/uhttp.h>
#include <ulib/net/udpsocket.h>
#include <ulib/net/ipt_ACCOUNT.h>
#include <ulib/utility/services.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/string_ext.h>
#include <ulib/serialize/flatbuffers.h>
#include <ulib/net/server/client_image.h>
#include <ulib/net/server/plugin/mod_nodog.h>

#ifdef USE_LIBTDB
#  include <ulib/db/tdb.h>
#endif

U_CREAT_FUNC(server_plugin_nodog, UNoDogPlugIn)

int      UNoDogPlugIn::fd_stderr;
bool     UNoDogPlugIn::bnetwork_interface;
bool     UNoDogPlugIn::mac_from_dhcp_data_file;
void*    UNoDogPlugIn::pdata;
uint32_t UNoDogPlugIn::T1;
uint32_t UNoDogPlugIn::T2;
uint32_t UNoDogPlugIn::check_expire;
UString* UNoDogPlugIn::label;
UString* UNoDogPlugIn::chrash;
UString* UNoDogPlugIn::fw_env;
UString* UNoDogPlugIn::fw_cmd;
UString* UNoDogPlugIn::extdev;
UString* UNoDogPlugIn::intdev;
UString* UNoDogPlugIn::mempool;
UString* UNoDogPlugIn::info_data;
UString* UNoDogPlugIn::arp_cache;
UString* UNoDogPlugIn::hostname;
UString* UNoDogPlugIn::localnet;
UString* UNoDogPlugIn::allowed_members;
UString* UNoDogPlugIn::IP_address_trust;

UString* UNoDogPlugIn::auth_host;
UString* UNoDogPlugIn::auth_info;
UString* UNoDogPlugIn::auth_login;
UString* UNoDogPlugIn::auth_notify;
UString* UNoDogPlugIn::auth_service;
UString* UNoDogPlugIn::auth_strict_notify;

UCommand*                  UNoDogPlugIn::fw;
UFlatBuffer*               UNoDogPlugIn::pfb;
UIptAccount*               UNoDogPlugIn::ipt;
UModNoDogPeer*             UNoDogPlugIn::peer;
UVector<UString>*          UNoDogPlugIn::varp_cache;
UVector<UString>*          UNoDogPlugIn::vInternalDevice;
UVector<UString>*          UNoDogPlugIn::vLocalNetworkLabel;
UVector<UString>*          UNoDogPlugIn::vLocalNetworkSpec;
UVector<UIPAllow*>*        UNoDogPlugIn::vLocalNetworkMask;
UHttpClient<UTCPSocket>*   UNoDogPlugIn::client;
UHashMap<UModNoDogPeer*>*  UNoDogPlugIn::peers;

UNoDogPlugIn::UNoDogPlugIn() : UEventTime(300L,0L)
{
   U_TRACE_REGISTER_OBJECT(0, UNoDogPlugIn, "")

   U_NEW(UCommand, fw, UCommand);

   U_NEW(UString, label, UString);
   U_NEW(UString, chrash, U_STRING_FROM_CONSTANT("/tmp/nodog.chrash"));
   U_NEW(UString, fw_cmd, UString);
   U_NEW(UString, fw_env, UString);
   U_NEW(UString, extdev, UString);
   U_NEW(UString, intdev, UString);
   U_NEW(UString, hostname, UString);
   U_NEW(UString, localnet, UString);
   U_NEW(UString, info_data, UString);
   U_NEW(UString, arp_cache, UString);
   U_NEW(UString, allowed_members, UString);
   U_NEW(UString, IP_address_trust, UString);

   U_NEW(UString, auth_host, UString);
   U_NEW(UString, auth_info, UString);
   U_NEW(UString, auth_login, UString);
   U_NEW(UString, auth_notify, UString);
   U_NEW(UString, auth_service, UString);
   U_NEW(UString, auth_strict_notify, UString);

   U_NEW(UVector<UString>, varp_cache, UVector<UString>);
   U_NEW(UVector<UString>, vInternalDevice, UVector<UString>);
   U_NEW(UVector<UString>, vLocalNetworkSpec, UVector<UString>);
   U_NEW(UVector<UString>, vLocalNetworkLabel, UVector<UString>);
   U_NEW(UVector<UIPAllow*>, vLocalNetworkMask, UVector<UIPAllow*>);

   U_NEW(UHttpClient<UTCPSocket>, client, UHttpClient<UTCPSocket>(U_NULLPTR));

   client->UClient_Base::setTimeOut(UServer_Base::timeoutMS);

   if (UString::str_without_label == U_NULLPTR) UString::str_allocate(STR_ALLOCATE_NOCAT);

#ifdef ENABLE_MEMPOOL
   U_NEW(UString, mempool, UString(UFile::contentOf(U_STRING_FROM_CONSTANT("/etc/nodog.mempool"))));
#endif
}

UNoDogPlugIn::~UNoDogPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, UNoDogPlugIn)

   delete fw;

   delete label;
   delete chrash;
   delete fw_cmd;
   delete fw_env;
   delete extdev;
   delete intdev;
   delete hostname;
   delete localnet;
   delete info_data;
   delete arp_cache;
   delete allowed_members;
   delete IP_address_trust;

   delete auth_host;
   delete auth_info;
   delete auth_login;
   delete auth_notify;
   delete auth_service;
   delete auth_strict_notify;

   delete varp_cache;
   delete vInternalDevice;
   delete vLocalNetworkSpec;
   delete vLocalNetworkLabel;
   delete vLocalNetworkMask;

   delete client;

#ifdef ENABLE_MEMPOOL
   delete mempool;
#endif

   if (ipt)   delete ipt;
   if (peers) delete peers;

#ifdef USE_LIBTDB
   if (pdata) delete (UTDB*)pdata;
#endif
}

int UModNoDogPeer::handlerTime()
{
   U_TRACE_NO_PARAM(0, "UModNoDogPeer::handlerTime()")

   UNoDogPlugIn::peer = this;

   if (U_peer_permit == false) UNoDogPlugIn::permit();

   U_RETURN(-1); // normal
}

U_NO_EXPORT void UNoDogPlugIn::getTraffic()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::getTraffic()")

#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
   UString ip(17U);
   const char* table;
   const unsigned char* bytep;
   struct ipt_acc_handle_ip* entry;

   for (uint32_t i = 0, n = vLocalNetworkMask->size(); i < n; ++i)
      {
      U_INTERNAL_ASSERT((*vLocalNetworkSpec)[i].isNullTerminated())

      table = (*vLocalNetworkSpec)[i].data();

      if (ipt->readEntries(table, true))
         {
         while ((entry = ipt->getNextEntry()))
            {
            bytep = (const unsigned char*) &(entry->ip);

#        ifdef HAVE_ARCH64
            ip.snprintf(U_CONSTANT_TO_PARAM("%u.%u.%u.%u"), bytep[3], bytep[2], bytep[1], bytep[0]);
#        else
            ip.snprintf(U_CONSTANT_TO_PARAM("%u.%u.%u.%u"), bytep[0], bytep[1], bytep[2], bytep[3]);
#        endif

            U_DEBUG("IP: %v SRC packets: %u bytes: %u DST packets: %u bytes: %u", ip.rep, entry->src_packets, entry->src_bytes, entry->dst_packets, entry->dst_bytes)

            peer = (*peers)[ip];

            U_INTERNAL_DUMP("peer = %p", peer)

            if (peer)
               {
               peer->ctraffic = (entry->src_bytes ? entry->src_bytes +
                                                    entry->dst_bytes : 0);

               U_INTERNAL_DUMP("peer->ctraffic = %u peer->mac = %V peer->ip = %V peer->label = %V", peer->ctraffic, peer->mac.rep, peer->ip.rep, peer->label.rep)
               }
            }
         }
      }
#endif
}

bool UNoDogPlugIn::getPeerInfo(UStringRep* key, void* value)
{
   U_TRACE(0, "UNoDogPlugIn::getPeerInfo(%V,%p)", key, value)

   peer = (UModNoDogPeer*)value;

   U_INTERNAL_DUMP("peer = %p", peer)

   U_INTERNAL_ASSERT_POINTER(peer)

   if (U_peer_allowed == false)
      {
      uint32_t _ctime = u_now->tv_sec - peer->_ctime;
                                        peer->_ctime = u_now->tv_sec;

      // -----------------------------------------------------------------------------------------------------------------------------------------
      // $1 -> mac
      // $2 -> ip
      // $3 -> ap
      // $4 -> time
      // $5 -> traffic
      // $6 -> time_no_traffic
      // -----------------------------------------------------------------------------------------------------------------------------------------

      pfb->String(peer->getMAC());
      pfb->String(peer->ip);
      pfb->String(peer->label);

      if (peer->ctraffic)
         {
         if (U_peer_permit == false)
            {
            U_WARNING("Peer IP %v MAC %v has made traffic(%u bytes) but it has status DENY", peer->ip.rep, peer->mac.rep, peer->ctraffic);
            }

         pfb->UInt(_ctime);

         pfb->UInt(peer->ctraffic);
                   peer->ctraffic = 0;

         pfb->UInt((peer->time_no_traffic = 0U));
         }
      else
         {
         pfb->UInt(0U);
         pfb->UInt(0U);
         pfb->UInt((peer->time_no_traffic += _ctime));

         U_SRV_LOG("Peer IP %v MAC %v has made no traffic for %u secs", peer->ip.rep, peer->mac.rep, peer->time_no_traffic);
         }
      }

   U_RETURN(true);
}

U_NO_EXPORT void UNoDogPlugIn::makeInfoData(UFlatBuffer* _pfb, void* param)
{
   U_TRACE(0, "UNoDogPlugIn::makeInfoData(%p,%p)", _pfb, param)

   (pfb = _pfb)->String(getApInfo(*label));

   if (peers->empty() == false)
      {
      getTraffic();

      peers->callForAllEntry(getPeerInfo);
      }
}

U_NO_EXPORT void UNoDogPlugIn::makeNotifyData(UFlatBuffer* _pfb, void* param)
{
   U_TRACE(0, "UNoDogPlugIn::makeNotifyData(%p,%p)", _pfb, param)

   U_INTERNAL_DUMP("peer->mac = %V peer->ip = %V peer->label = %V", peer->mac.rep, peer->ip.rep, peer->label.rep)

   U_INTERNAL_ASSERT(u_isIPv4Addr(U_STRING_TO_PARAM(peer->ip)))
   U_INTERNAL_ASSERT(u_isMacAddr(U_STRING_TO_PARAM(peer->mac)))

   _pfb->String(getApInfo(peer->label));
   _pfb->String(peer->getMAC());
   _pfb->String(peer->ip);
}

U_NO_EXPORT void UNoDogPlugIn::makeLoginData(UFlatBuffer* _pfb, void* param)
{
   U_TRACE(0, "UNoDogPlugIn::makeLoginData(%p,%p)", _pfb, param)

   makeNotifyData(_pfb, param);

   _pfb->UInt(U_PTR2INT(peer));
}

void UNoDogPlugIn::checkSystem()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::checkSystem()")

   U_SET_MODULE_NAME(nodog);

   U_SRV_LOG("Checking peers for info");

   UFlatBufferSpace space;

   *info_data = UFlatBuffer::toVector(makeInfoData);

   (void) client->sendPOSTRequestAsync(*info_data, *auth_info, true);

   info_data->clear();

   U_RESET_MODULE_NAME;
}

U_NO_EXPORT void UNoDogPlugIn::setMAC()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::setMAC()")

   if (peer->mac.empty() ||
       peer->mac == *UString::str_without_mac)
      {
      U_INTERNAL_ASSERT(peer->ifname.isNullTerminated())

      peer->mac = UServer_Base::csocket->getMacAddress(peer->ifname.data());

      if (peer->mac.empty())
         {
         UString arp_cache;
         UVector<UString> varp_cache;

         (void) USocketExt::getARPCache(arp_cache, varp_cache);

         for (uint32_t i = 0, n = varp_cache.size(); i < n; i += 3)
            {
            if (varp_cache[i].equal(peer->ip))
               {
               peer->mac = varp_cache[i+1].copy();

               U_ASSERT_EQUALS(peer->ifname, varp_cache[i+2])

               break;
               }
            }
         }

      U_INTERNAL_DUMP("peer->mac = %V", peer->mac.rep)

      U_INTERNAL_ASSERT(peer->mac)
      U_ASSERT_EQUALS(peer->mac, USocketExt::getMacAddress(peer->ip))
      }
}

U_NO_EXPORT void UNoDogPlugIn::setLabelAndMAC()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::setLabelAndMAC()")

   peer->label = ((uint32_t)U_peer_index_network >= vLocalNetworkLabel->size() ? *UString::str_without_label : (*vLocalNetworkLabel)[U_peer_index_network]);

   U_INTERNAL_DUMP("pdata = %p mac_from_dhcp_data_file = %b peer->label = %V", pdata, mac_from_dhcp_data_file, peer->label.rep)

   if (pdata == U_NULLPTR) setMAC();
#ifdef USE_LIBTDB
   else
      {
      typedef struct { uint8_t hwaddr[6], ip[4]; } __attribute__((packed)) tdbkey_t;

      tdbkey_t ks;
      uint32_t t1, t2, t3, t4, start, sz = 0;

      if (U_SYSCALL(sscanf, "%p,%S,%p,%p,%p,%p", peer->ip.data(), "%u.%u.%u.%u", &t1, &t2, &t3, &t4) == 4)
         {
         ks.ip[0] = t4;
         ks.ip[1] = t3;
         ks.ip[2] = t2;
         ks.ip[3] = t1;

            sz = 4;
         start = 6;
         }

      if (mac_from_dhcp_data_file == false)
         {
         uint32_t t5, t6;

         setMAC();

         if (U_SYSCALL(sscanf, "%p,%S,%p,%p,%p,%p,%p,%p", peer->mac.data(), "%x:%x:%x:%x:%x:%x", &t1, &t2, &t3, &t4, &t5, &t6) == 6)
            {
            ks.hwaddr[0] = t1;
            ks.hwaddr[1] = t2;
            ks.hwaddr[2] = t3;
            ks.hwaddr[3] = t4;
            ks.hwaddr[4] = t5;
            ks.hwaddr[5] = t6;

               sz = 10;
            start = 0;
            }
         }

      if (sz)
         {
         UString value = ((UTDB*)pdata)->at((const char*)&ks+start, sz);

         if (value)
            {
         // typedef struct { uint32_t area; uint8_t hwaddr[6]; uint32_t leasetime; time_t expiration; } __attribute__((packed)) tdbdata;

            UString area(32U);
            uint32_t id = *(uint32_t*)value.data();

            if (id == 16777215) (void) area.assign(U_CONSTANT_TO_PARAM("ffffff"));
            else                       area.setFromNumber32(id);

            if (peer->label != area) peer->label = area;

            if (mac_from_dhcp_data_file)
               {
               U_ASSERT_EQUALS(peer->mac, *UString::str_without_mac)

               UString mac(17U);
               const unsigned char* bytep = (const unsigned char*) value.c_pointer(sizeof(uint32_t));

               mac.snprintf(U_CONSTANT_TO_PARAM("%02x:%02x:%02x:%02x:%02x:%02x"), bytep[0], bytep[1], bytep[2], bytep[3], bytep[4], bytep[5]);

               peer->mac = mac;
               }

            U_SRV_LOG("get data from DHCP_DATA_FILE - key: %#.*S data: %#.10S peer->label = %V peer->mac = %V", sz, (const char*)&ks+start, value.data(), peer->label.rep, peer->mac.rep);
            }
         }
      }
#endif

   U_INTERNAL_DUMP("peer->label = %V peer->mac = %V", peer->label.rep, peer->mac.rep)
}

void UNoDogPlugIn::setNewPeer()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::setNewPeer()")

   U_INTERNAL_ASSERT_POINTER(peer)

   U_INTERNAL_DUMP("peer->UIPAddress::pcStrAddress = %S U_peer_index_network = %u",
                    peer->UIPAddress::pcStrAddress,     U_peer_index_network)

   if (U_peer_index_network == 0xFF)
      {
      U_ERROR("IP address for new peer %V not found in LocalNetworkMask %V", peer->ip.rep, localnet->rep);
      }

   UIPAllow* pallow = vLocalNetworkMask->at(U_peer_index_network);

   peer->gateway = (bnetwork_interface ? pallow->host : *UServer_Base::IP_address);

   U_INTERNAL_DUMP("peer->gateway = %V", peer->gateway.rep)

   U_INTERNAL_ASSERT(peer->gateway)

   if (pallow->device &&
       peer->ifname != pallow->device)
      {
      U_SRV_LOG("WARNING: different device (%v=>%v) for peer: IP %v MAC %v", peer->ifname.rep, pallow->device.rep, peer->ip.rep, peer->mac.rep);

      peer->ifname = pallow->device;
      }

   U_INTERNAL_DUMP("peer->ifname = %V", peer->ifname.rep)

   setLabelAndMAC();

   // NB: request(arp|deny|clear|reset|permit|openlist|initialize) mac ip class(Owner|Member|Public) UserDownloadRate UserUploadRate

   UString command(100U);

   command.snprintf(U_CONSTANT_TO_PARAM("/bin/sh %v deny %v %v Member 0 0"), fw_cmd->rep, (mac_from_dhcp_data_file ? *UString::str_without_mac : peer->mac).rep, peer->ip.rep);

   peer->fw.set(command, (char**)U_NULLPTR);
   peer->fw.setEnvironment(fw_env);

   peers->insert(peer->ip, peer);
}

U_NO_EXPORT void UNoDogPlugIn::eraseTimer()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::eraseTimer()")

   if (U_peer_timer_active)
      {
      U_peer_flag &= ~U_PEER_TIMER_ACTIVE;

      UTimer::erase(peer);
      }
}

U_NO_EXPORT void UNoDogPlugIn::sendNotify()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::sendNotify()")

   if (U_peer_notify_disable == false)
      {
      U_peer_flag |= U_PEER_NOTIFY_DISABLE;

      (void) client->sendPOSTRequestAsync(UFlatBuffer::toVector(makeNotifyData), *auth_notify, true);
      }
}

U_NO_EXPORT void UNoDogPlugIn::sendStrictNotify()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::sendStrictNotify()")

   if (U_peer_strict_notify_disable == false)
      {
      U_peer_flag |= U_PEER_STRICT_NOTIFY_DISABLE;

      (void) client->sendPOSTRequestAsync(UFlatBuffer::toVector(makeNotifyData), *auth_strict_notify, true);
      }
}

// Server-wide hooks

int UNoDogPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "UNoDogPlugIn::handlerConfig(%p)", &cfg)

   // -----------------------------------------------------------------------------------------------------------------------------------
   // nodog - plugin parameters
   // -----------------------------------------------------------------------------------------------------------------------------------
   // FW_CMD                shell script to manage the firewall
   // FW_ENV                environment for shell script to execute
   // DECRYPT_KEY           DES3 password stuff
   // ALLOWED_MEMBERS       file with list of allowed MAC/IP pairs or NETWORKS (default: /etc/nodog.allowed)
   // LOCAL_NETWORK_LABEL   access point localization tag to be used from portal
   // CHECK_EXPIRE_INTERVAL Number of seconds to send client info to portal
   // -----------------------------------------------------------------------------------------------------------------------------------

   if (cfg.loadTable())
      {
      UString x = cfg.at(U_CONSTANT_TO_PARAM("FW_ENV"));

      if (x.empty()) U_RETURN(U_PLUGIN_HANDLER_ERROR);

      *fw_cmd = cfg.at(U_CONSTANT_TO_PARAM("FW_CMD"));
      *fw_env = UStringExt::prepareForEnvironmentVar(x);

      T1           = cfg.readLong(U_CONSTANT_TO_PARAM("T1"), 20);
      T2           = cfg.readLong(U_CONSTANT_TO_PARAM("T2"), 60);
      check_expire = cfg.readLong(U_CONSTANT_TO_PARAM("CHECK_EXPIRE_INTERVAL"));

      U_INTERNAL_DUMP("check_expire = %u T1 = %u T = %u", check_expire, T1, T2)

      if (check_expire) UEventTime::setTimeToExpire(check_expire);

      *intdev = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("InternalDevice"),  fw_env);

      if (intdev->empty())
         {
         *intdev = UServer_Base::getNetworkDevice(extdev->data());

         U_SRV_LOG("Autodetected InternalDevice %V", intdev->rep);

         if (intdev->empty()) U_ERROR("No InternalDevice detected");
         }

      uint32_t num_radio = vInternalDevice->split(U_STRING_TO_PARAM(*intdev));

      U_INTERNAL_DUMP("num_radio = %u", num_radio)

      U_ASSERT_EQUALS(num_radio, 1)

      *localnet = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("LocalNetwork"), fw_env);

      if (localnet->empty())
         {
         *localnet = UServer_Base::getNetworkAddress(intdev->data());

         U_SRV_LOG("Autodetected LocalNetwork %V", localnet->rep);

         if (localnet->empty()) U_ERROR("No LocalNetwork detected");
         }

      (void) UIPAllow::parseMask(*localnet, *vLocalNetworkMask, vLocalNetworkSpec);

      *extdev = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("ExternalDevice"), fw_env);

      if (extdev->empty())
         {
         *extdev = UServer_Base::getNetworkDevice(U_NULLPTR);

         U_SRV_LOG("Autodetected ExternalDevice %V", extdev->rep);

         if (extdev->empty()) U_ERROR("No ExternalDevice detected");
         }

      *auth_host = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("AuthServiceAddr"), fw_env);

      x = cfg.at(U_CONSTANT_TO_PARAM("LOCAL_NETWORK_LABEL"));

      if (x) (void) vLocalNetworkLabel->split(U_STRING_TO_PARAM(x));

      U_ASSERT_EQUALS(vLocalNetworkLabel->size(), vLocalNetworkMask->size())

      *label = (vLocalNetworkLabel->empty() ? *UString::str_without_label : (*vLocalNetworkLabel)[0]);

      x = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("GatewayPort"), fw_env);

      UServer_Base::port = (x ? x.strtoul() : 5280);

      U_INTERNAL_DUMP("label = %V UServer_Base::port = %u", label->rep, UServer_Base::port)

      x = cfg.at(U_CONSTANT_TO_PARAM("ALLOWED_MEMBERS"));

      *allowed_members = UFile::contentOf(x ? x : *UString::str_allowed_members_default);

#  ifdef USE_LIBTDB
      x = cfg.at(U_CONSTANT_TO_PARAM("DHCP_DATA_FILE"));

      if (x)
         {
         U_NEW(UTDB, pdata, UTDB);

         U_INTERNAL_ASSERT(x.isNullTerminated())

         if (((UTDB*)pdata)->open(x.data()))
            {
            U_SRV_LOG("open DHCP_DATA_FILE %V", x.rep);

            x = cfg.at(U_CONSTANT_TO_PARAM("MAC_FROM_DHCP_DATA_FILE"));

            if (x) mac_from_dhcp_data_file = x.strtob();
            }
         else
            {
            U_SRV_LOG("WARNING: fail to open DHCP_DATA_FILE %V", x.rep);

            delete (UTDB*)pdata;
                          pdata = U_NULLPTR;
            }
         }
#  endif
      }

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED);
}

int UNoDogPlugIn::handlerInit()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::handlerInit()")

   if (   fw_cmd->empty() ||
       auth_host->empty())
      {
      U_RETURN(U_PLUGIN_HANDLER_ERROR);
      }

   // get IP address of AUTH host...

   UIPAddress auth_addr;
   Url auth_url(*auth_host);

   *auth_host    = auth_url.getHost().copy();
   *auth_service = auth_url.getService().copy();

   if (auth_addr.setHostName(*auth_host, UClientImage_Base::bIPv6)) (void) UServer_Base::auth_ip->replace(auth_addr.getAddressString());
   else
      {
      U_SRV_LOG("WARNING: unknown AUTH host %V", auth_host->rep);

      *UServer_Base::auth_ip = *auth_host;
      }

   *hostname = USocketExt::getNodeName();

   U_INTERNAL_DUMP("host = %v:%u hostname = %V ip = %V", UServer_Base::IP_address->rep, UServer_Base::port, hostname->rep, UServer_Base::IP_address->rep)

   UUDPSocket cClientSocket(UClientImage_Base::bIPv6);

   if (cClientSocket.connectServer(*UServer_Base::auth_ip, 1001))
      {
      const char* p = cClientSocket.getLocalInfo();

      UString ip = UString(p, u__strlen(p, __PRETTY_FUNCTION__));

      if (ip != *UServer_Base::IP_address)
         {
         U_FILE_WRITE_TO_TMP(ip, "IP_ADDRESS");

         U_SRV_LOG("WARNING: SERVER IP ADDRESS differ from IP address: %V to connect to AUTH: %V", ip.rep, UServer_Base::auth_ip->rep);
         }
      }

   U_SRV_LOG("AUTH ip registered: %v", UServer_Base::auth_ip->rep);

   fw_env->snprintf_add(U_CONSTANT_TO_PARAM("'AuthServiceIP=%v'\n"), UServer_Base::auth_ip->rep);

   (void) UFile::_unlink(chrash->data());

   bnetwork_interface = UIPAllow::getNetworkInterface(*vLocalNetworkMask);

   if (bnetwork_interface == false)
      {
      U_SRV_LOG("UIPAllow::getNetworkInterface() failed");
      }
   else
      {
      UIPAllow* pallow;

      for (uint32_t i = 0, n = vLocalNetworkMask->size(); i < n; ++i)
         {
         pallow = vLocalNetworkMask->at(i);

         U_SRV_LOG("vLocalNetworkMask->at(%u): host = %V device = %V", i, pallow->host.rep, pallow->device.rep);

         if (pallow->host.empty()) pallow->host = *UServer_Base::IP_address;
         }
      }

   *auth_info          = getUrlForSendMsgToPortal(U_CONSTANT_TO_PARAM("/info"));
   *auth_login         = getUrlForSendMsgToPortal(U_CONSTANT_TO_PARAM("/login"));
   *auth_notify        = getUrlForSendMsgToPortal(U_CONSTANT_TO_PARAM("/notify"));
   *auth_strict_notify = getUrlForSendMsgToPortal(U_CONSTANT_TO_PARAM("/strict_notify"));

#ifdef USE_LIBSSL
   UString content = UFile::contentOf(U_STRING_FROM_CONSTANT("../DES3_KEY.txt"));

   UDES3::setPassword(content.c_strndup());
#endif

   *IP_address_trust = UDES3::signData(U_CONSTANT_TO_PARAM("%v"), UServer_Base::IP_address->rep);

   // firewall cmd

   UString command(500U);

   command.snprintf(U_CONSTANT_TO_PARAM("/bin/sh %v initialize allowed_web_hosts"), fw_cmd->rep);

   fw->set(command, (char**)U_NULLPTR);
   fw->setEnvironment(fw_env);

   if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/server_plugin_nodog.err");

   // users monitoring

   U_INTERNAL_DUMP("check_expire = %u", check_expire)

   if (check_expire)
      {
      UTimer::insert(this);

      U_SRV_LOG("Monitoring set for every %d secs", UTimeVal::getSecond());
      }

   // users traffic

   U_NEW(UIptAccount, ipt, UIptAccount(UClientImage_Base::bIPv6));

   // users table

   U_NEW(UHashMap<UModNoDogPeer*>, peers, UHashMap<UModNoDogPeer*>);

   U_RETURN(U_PLUGIN_HANDLER_OK);
}

int UNoDogPlugIn::handlerFork()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::handlerFork()")

   /**
    * check if we want some preallocation for memory pool...
    *
    * U_WRITE_MEM_POOL_INFO_TO("mempool.%N.%P.handlerFork", 0)
    */

#ifdef ENABLE_MEMPOOL
   if (*mempool) UMemoryPool::allocateMemoryBlocks(mempool->data()); // NB: start from 1... (Ex: 20,30,0,1,1050,0,0,0,2)
#endif

   // send msg start to portal

   UString msg(300U), uptime(20U), allowed_web_hosts;

   uptime.setUpTime();

   msg.snprintf(U_CONSTANT_TO_PARAM("/start_ap?ap=%v&public=%v%%3A%u&pid=%d&uptime=%v"),
                getApInfo(*label).rep, IP_address_trust->rep, UServer_Base::port, UFile::getSysParam(chrash->data()), uptime.rep);

   (void) UFile::writeTo(*chrash, u_pid_str, u_pid_str_len);

   bool bqueue = false, we_need_response = true;

   U_INTERNAL_ASSERT_EQUALS(UClient_Base::queue_dir, U_NULLPTR)

#ifndef U_LOG_DISABLE
   const char* result = "FAILED";
#endif

loop:
   if (client->connectServer(getUrlForSendMsgToPortal(msg)) == false)
      {
      if (we_need_response)
         {
         U_SRV_LOG("send message to AUTH: %V FAILED", auth_host->rep);

         we_need_response = false;

         UClient_Base::queue_dir = UString::str_CLIENT_QUEUE_DIR;

         goto loop;
         }

      goto end;
      }

   if (client->sendRequest())
      {
      (void) allowed_web_hosts.replace(client->getContent());

#  ifndef U_LOG_DISABLE
      result = "success";
#  endif
      }
   else
      {
      bqueue = true;

#  ifdef U_LOG_DISABLE
      (void) client->putRequestOnQueue();
#  else
      if (   client->putRequestOnQueue()) result = "success";
#  endif
      }

end:
   U_INTERNAL_DUMP("we_need_response = %b UClient_Base::queue_dir = %p", we_need_response, UClient_Base::queue_dir)

   if (bqueue)
      {
      U_SRV_LOG("queue message %s to AUTH: %V", result, msg.rep);
      }

   UClient_Base::queue_dir = U_NULLPTR;

   client->reset(); // NB: url is referenced by UClient::url...

   // initialize the firewall: direct all port 80 traffic to us...

   U_INTERNAL_ASSERT(allowed_web_hosts.isNullTerminated())

   fw->setArgument(4, allowed_web_hosts.data());

   (void) fw->executeAndWait(U_NULLPTR, -1, fd_stderr);

#ifndef U_LOG_DISABLE
   UServer_Base::logCommandMsgError(fw->getCommand(), false);
#endif

   fw->delArgument();
   fw->setLastArgument("openlist");

   if (*allowed_members)
      {
      // 00:27:22:4f:69:f4 172.16.1.1 Member ### routed_ap-locoM2

      UVector<UString> vtmp;
      uint32_t n = vtmp.loadFromData(*allowed_members);

      UString ip,
              UserUploadRate,
              UserDownloadRate;
      uint32_t increment = ((n % 3) ? 5 : 3);

      for (uint32_t i = 0; i < n; i += increment)
         {
         ip = vtmp[i+1];

         uint32_t index_network = UIPAllow::find(ip, *vLocalNetworkMask);

         U_INTERNAL_DUMP("index_network = %u", index_network)

         if (index_network == U_NOT_FOUND)
            {
            U_SRV_LOG("WARNING: IP address for allowed_members %V not found in LocalNetworkMask %V", ip.rep, localnet->rep);

            continue;
            }

         U_INTERNAL_ASSERT_EQUALS(vtmp[i+2], "Member")

         if (increment == 5)
            {
            UserDownloadRate = vtmp[i+3];
            UserUploadRate   = vtmp[i+4];
            }

         U_NEW(UModNoDogPeer, peer, UModNoDogPeer);

         peer->ip  = ip;
         peer->mac = vtmp[i];

         peer->ip.copy(peer->UIPAddress::pcStrAddress);

         U_peer_flag |= U_PEER_ALLOWED;

         U_peer_index_network                   = index_network;
         U_ipaddress_StrAddressUnresolved(peer) = false;

         setNewPeer();

         permit();
         }
      }

   U_RETURN(U_PLUGIN_HANDLER_OK);
}

// Connection-wide hooks

int UNoDogPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(1, "UNoDogPlugIn::handlerRequest()")

   if (UHTTP::file->isRoot() ||
       UClientImage_Base::isRequestNotFound())
      {
      UString x, redirect;
      uint32_t index_network;

      U_http_info.nResponseCode = HTTP_NO_CONTENT;

      (void) UServer_Base::csocket->shutdown(SHUT_RD);

      U_INTERNAL_DUMP("UServer_Base::client_address = %.*S", U_CLIENT_ADDRESS_TO_TRACE)

      if (UServer_Base::auth_ip->equal(U_CLIENT_ADDRESS_TO_PARAM))
         {
         // ------------------------------------------
         // request from AUTH, which may be:
         // ------------------------------------------
         // 1) /login_validate - login user validation
         // 2) /logout         - logout specific users
         // 3) /ping           - check 
         // ------------------------------------------

         U_SRV_LOG("AUTH request: %.*S", U_HTTP_URI_TO_TRACE);

         if (U_HTTP_URI_STREQ("/login_validate"))
            {
            UFlatBuffer fb, vec;

            fb.setRoot(*UClientImage_Base::body);
            fb.AsVector(vec);

            // $1 -> peer
            // $2 -> deny|permit ('0'|'1') policy: notify|no_notify|strict_notify ('0'|'1'|'2')

            peer = (UModNoDogPeer*) vec.AsVectorGet<uint64_t>(0);
               x =                  vec.AsVectorGet<UString>(1);

            if (peer)
               {
               bool bdeny = (x.first_char() == '0'); // deny|permit ('0'|'1')
               char policy = x.c_char(1);            // policy: notify|no_notify|strict_notify ('0'|'1'|'2')

               U_INTERNAL_ASSERT(peer->mac)
               U_INTERNAL_ASSERT(peer->welcome)

               U_SRV_LOG("request to validate login for peer IP %v MAC %v: bdeny = %b policy = %C", peer->ip.rep, peer->mac.rep, bdeny, policy);

               if (bdeny)
                  {
                  eraseTimer();

                  if (U_peer_permit) deny();

                  delete peers->erase(peer->ip);

                  U_peer_flag |= U_PEER_ALLOW_DISABLE;

                  goto end;
                  }

               U_peer_policy = policy; // policy: notify|no_notify|strict_notify ('0'|'1'|'2')

               /*
               if (U_peer_policy == '0') // '0' (notify) (1<=T1<=3599) (1<=T2<=3599)
                  {
                  }
               */

               if (policy == '1') // '1' (no notify) (T1==T2==0)
                  {
                  eraseTimer();

                  if (U_peer_permit == false) permit();

                  goto end;
                  }

               if (policy == '2') // '2' (strict notify)
                  {
                  if (U_peer_permit == false)
                     {
                     eraseTimer();

                     U_peer_flag |= U_PEER_DELAY_DISABLE;
                     }
                  }
               }
            }
         else if (U_HTTP_URI_STREQ("/logout"))
            {
            UString data = UDES3::getSignedData(*UClientImage_Base::body);

            if (data.empty())
               {
               U_WARNING("AUTH request to logout users tampered");

               goto bad;
               }

            UVector<UString> vec;
            UString peer_ip, peer_mac;

            UFlatBuffer::toVector(data, vec);

            for (int32_t i = 0, n = (int32_t) vec.size(); i < n; i += 2)
               {
               // ---------
               // $1 -> ip
               // $2 -> mac
               // ---------

               peer_ip  = vec[i];
               peer_mac = vec[i+1];

               U_INTERNAL_DUMP("peer_ip = %V peer_mac = %V", peer_ip.rep, peer_mac.rep)

               if ((peer = (*peers)[peer_ip])) goto next;

               U_INTERNAL_ASSERT_DIFFERS(peer_mac, *UString::str_without_mac)

               if (peers->first())
                  {
                  do {
                     if (peer_mac == (peer = peers->elem())->getMAC()) goto next;
                     }
                  while (peers->next());
                  }

               continue;

               eraseTimer();

next:          if (U_peer_permit) deny();
               else
                  {
                  U_SRV_LOG("AUTH request to logout user with status DENY: IP %v MAC %v", peer->ip.rep, peer->mac.rep);
                  }

               delete peers->erase(peer->ip);
               }
            }
         else if (U_HTTP_URI_STREQ("/ping"))
            {
            }
         else
            {
bad:        UHTTP::setBadRequest();
            }

         goto end;
         }

      if ((peer = peers->at(U_CLIENT_ADDRESS_TO_PARAM)))
         {
         // -----------------
         // request from user
         // -----------------

         U_INTERNAL_ASSERT(peer->mac)
         U_INTERNAL_ASSERT(peer->welcome)
         U_ASSERT(peer->ip.equal(U_CLIENT_ADDRESS_TO_PARAM))

         if (U_peer_allow_disable == false &&
             U_HTTP_URI_MEMEQ("/nodog_peer_allow.sh"))
            {
            if (U_HTTP_QUERY_MEMEQ("url=") &&
                UHTTP::processForm() == 2*2)
               {
               /**
                * open firewall, respond with redirect to original request
                *
                * $1 -> url
                * $2 -> forced ('0'|'1')
                */

               UHTTP::getFormValue(redirect, U_CONSTANT_TO_PARAM("url"),    0, 1, 4);
               UHTTP::getFormValue(x,        U_CONSTANT_TO_PARAM("forced"), 0, 3, 4);

               if (x.first_char() == '1') // forced
                  {
                  sendStrictNotify();

                  goto next1;
                  }

               U_INTERNAL_DUMP("T2 = %u U_peer_policy = %C", T2, U_peer_policy)

               if (T2 < 3600 &&
                   U_peer_policy != '2') // (strict notify)
                  {
next1:            eraseTimer();

                  if (U_peer_permit == false) permit();

                  sendNotify();

                  UHTTP::setRedirectResponse(UHTTP::NO_BODY, U_STRING_TO_PARAM(redirect));
                  }
               }

            goto end;
            }

         if (U_peer_delay_disable == false &&
             U_HTTP_URI_STREQ("/nodog_peer_delay.sh"))
            {
            eraseTimer();

            U_INTERNAL_DUMP("T2 = %u", T2)

            if (T2 &&
                T2 < 3600)
               {
               peer->UEventTime::setTimeToExpire(T2);

               U_peer_flag |= U_PEER_TIMER_ACTIVE;

               UTimer::insert(peer);
               }

            sendNotify();

            goto end;
            }

         U_SRV_LOG("user request: %.*S", U_HTTP_URI_TO_TRACE);

         goto welcome;
         }

      index_network = UIPAllow::find(UServer_Base::client_address, *vLocalNetworkMask);

      U_INTERNAL_DUMP("index_network = %u", index_network)

      if (index_network == U_NOT_FOUND)
         {
         U_SRV_LOG("WARNING: IP address for new peer %.*S not found in LocalNetworkMask %V", U_CLIENT_ADDRESS_TO_TRACE, localnet->rep);

         goto end;
         }

      U_NEW(UModNoDogPeer, peer, UModNoDogPeer);

      U_peer_index_network = index_network;

      peer->ifname = (*vInternalDevice)[0];

      *((UIPAddress*)peer) = UServer_Base::csocket->remoteIPAddress();

      (void) peer->ip.replace(peer->UIPAddress::pcStrAddress, UServer_Base::client_address_len);

      U_INTERNAL_ASSERT_EQUALS(memcmp(peer->UIPAddress::pcStrAddress, U_CLIENT_ADDRESS_TO_PARAM), 0)

      setNewPeer();

      (void) client->sendPOSTRequestAsync(UFlatBuffer::toVector(makeLoginData), *auth_login, true);

      (void) redirect.reserve(8 + U_http_host_len + U_HTTP_URI_QUERY_LEN);

      redirect.snprintf(U_CONSTANT_TO_PARAM("http://%.*s/%.*s"), U_HTTP_HOST_TO_TRACE, U_HTTP_URI_QUERY_TO_TRACE);

      U_INTERNAL_DUMP("T1 = %u", T1)

      if (T1 == 0)
         {
         permit();

         UHTTP::setRedirectResponse(UHTTP::NO_BODY, U_STRING_TO_PARAM(redirect));
         }
      else
         {
         if (T1 < 3600)
            {
            peer->UEventTime::setTimeToExpire(T1);

            U_peer_flag |= U_PEER_TIMER_ACTIVE;

            UTimer::insert(peer);
            }

         x.setBuffer(redirect.size() * 3);

         Url::encode(redirect, x);

         peer->welcome.reserve(128U + auth_host->size() + x.size());

         peer->welcome.snprintf(U_CONSTANT_TO_PARAM("%v://%v/welcome?url=%v&mac=%v&apid=%v&gateway=%v%%3A%u"),
                                auth_service->rep, auth_host->rep, x.rep, peer->getMAC().rep, peer->label.rep, peer->gateway.rep, UServer_Base::port);

welcome: UHTTP::setRedirectResponse(UHTTP::NO_BODY, U_STRING_TO_PARAM(peer->welcome));
         }

end:  UClientImage_Base::setCloseConnection();

      if (U_http_info.nResponseCode == HTTP_NO_CONTENT) UHTTP::setResponse();

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED);
      }

   U_RETURN(U_PLUGIN_HANDLER_OK);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UModNoDogPeer::dump(bool _reset) const
{
   *UObjectIO::os << "next               " << (void*)next      <<  '\n'
                  << "_ctime             " << _ctime           <<  '\n'
                  << "ctraffic           " << ctraffic         <<  '\n'
                  << "ip       (UString  " << (void*)&ip       << ")\n"
                  << "mac      (UString  " << (void*)&mac      << ")\n"
                  << "label    (UString  " << (void*)&label    << ")\n"
                  << "ifname   (UString  " << (void*)&ifname   << ")\n"
                  << "gateway  (UString  " << (void*)&gateway  << ")\n"
                  << "welcome  (UString  " << (void*)&welcome  << ")\n"
                  << "fw       (UCommand " << (void*)&fw       << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}

const char* UNoDogPlugIn::dump(bool _reset) const
{
   *UObjectIO::os << "fd_stderr                                      " << fd_stderr                   <<  '\n'
                  << "label              (UString                  " << (void*)label                << ")\n"
                  << "extdev             (UString                  " << (void*)extdev               << ")\n"
                  << "fw_cmd             (UString                  " << (void*)fw_cmd               << ")\n"
                  << "fw_env             (UString                  " << (void*)fw_env               << ")\n"
                  << "intdev             (UString                  " << (void*)intdev               << ")\n"
                  << "hostname           (UString                  " << (void*)hostname             << ")\n"
                  << "localnet           (UString                  " << (void*)localnet             << ")\n"
                  << "auth_info          (UString                  " << (void*)auth_info           << ")\n"
                  << "auth_login         (UString                  " << (void*)auth_login           << ")\n"
                  << "allowed_members    (UString                  " << (void*)allowed_members      << ")\n"
                  << "fw                 (UCommand                 " << (void*)fw                   << ")\n"
                  << "ipt                (UIptAccount              " << (void*)ipt                  << ")\n"
                  << "vInternalDevice    (UVector<UString>         " << (void*)vInternalDevice      << ")\n"
                  << "vLocalNetworkLabel (UVector<UString>         " << (void*)vLocalNetworkLabel   << ")\n"
                  << "client             (UHttpClient<UTCPSocket>  " << (void*)client               << ")\n"
                  << "peers              (UHashMap<UModNoDogPeer*> " << (void*)peers                << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
