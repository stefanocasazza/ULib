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

#include <ulib/base/hash.h>

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

in_addr_t                  UNoDogPlugIn::addr;
UCommand*                  UNoDogPlugIn::fw;
UIPAllow*                  UNoDogPlugIn::pallow;
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
   U_TRACE_CTOR(0, UNoDogPlugIn, "")

   U_NEW(UCommand, fw, UCommand);

   U_NEW_STRING(label, UString);
   U_NEW_STRING(chrash, UString(U_CONSTANT_TO_PARAM("/tmp/nodog.chrash")));
   U_NEW_STRING(fw_cmd, UString);
   U_NEW_STRING(fw_env, UString);
   U_NEW_STRING(extdev, UString);
   U_NEW_STRING(intdev, UString);
   U_NEW_STRING(hostname, UString);
   U_NEW_STRING(localnet, UString);
   U_NEW_STRING(info_data, UString);
   U_NEW_STRING(arp_cache, UString);
   U_NEW_STRING(allowed_members, UString);
   U_NEW_STRING(IP_address_trust, UString);

   U_NEW_STRING(auth_host, UString);
   U_NEW_STRING(auth_info, UString);
   U_NEW_STRING(auth_login, UString);
   U_NEW_STRING(auth_notify, UString);
   U_NEW_STRING(auth_service, UString);
   U_NEW_STRING(auth_strict_notify, UString);

   U_NEW(UVector<UString>, varp_cache, UVector<UString>);
   U_NEW(UVector<UString>, vInternalDevice, UVector<UString>);
   U_NEW(UVector<UString>, vLocalNetworkSpec, UVector<UString>);
   U_NEW(UVector<UString>, vLocalNetworkLabel, UVector<UString>);
   U_NEW(UVector<UIPAllow*>, vLocalNetworkMask, UVector<UIPAllow*>);

   U_NEW(UHttpClient<UTCPSocket>, client, UHttpClient<UTCPSocket>(U_NULLPTR));

   client->UClient_Base::setTimeOut(UServer_Base::timeoutMS);

   if (UString::str_without_label == U_NULLPTR) UString::str_allocate(STR_ALLOCATE_NOCAT);
}

UNoDogPlugIn::~UNoDogPlugIn()
{
   U_TRACE_DTOR(0, UNoDogPlugIn)

   U_DELETE(fw)

   U_DELETE(label)
   U_DELETE(chrash)
   U_DELETE(fw_cmd)
   U_DELETE(fw_env)
   U_DELETE(extdev)
   U_DELETE(intdev)
   U_DELETE(hostname)
   U_DELETE(localnet)
   U_DELETE(info_data)
   U_DELETE(arp_cache)
   U_DELETE(allowed_members)
   U_DELETE(IP_address_trust)

   U_DELETE(auth_host)
   U_DELETE(auth_info)
   U_DELETE(auth_login)
   U_DELETE(auth_notify)
   U_DELETE(auth_service)
   U_DELETE(auth_strict_notify)

   U_DELETE(varp_cache)
   U_DELETE(vInternalDevice)
   U_DELETE(vLocalNetworkSpec)
   U_DELETE(vLocalNetworkLabel)
   U_DELETE(vLocalNetworkMask)

   U_DELETE(client)

   if (ipt)   U_DELETE(ipt)
   if (peers) U_DELETE(peers)

#ifdef USE_LIBTDB
   if (pdata) U_DELETE((UTDB*)pdata)
#endif
}

int UModNoDogPeer::handlerTime()
{
   U_TRACE_NO_PARAM(0, "UModNoDogPeer::handlerTime()")

   UNoDogPlugIn::peer = this;

   if (U_peer_permit == false) UNoDogPlugIn::permit();

   U_RETURN(-1); // normal
}

U_NO_EXPORT void UNoDogPlugIn::makeInfoData(UFlatBuffer* pfb, void* param)
{
   U_TRACE(0, "UNoDogPlugIn::makeInfoData(%p,%p)", pfb, param)

   char buffer[256];

   pfb->String(buffer, getApInfo(buffer, sizeof(buffer), *label));

   if (peers->first())
      {
      uint32_t _ctime = peers->getIndexNode();

#  ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
      const char* table;
      struct ipt_acc_handle_ip* entry;

      for (uint32_t i = 0, n = vLocalNetworkMask->size(); i < n; ++i)
         {
         U_INTERNAL_ASSERT((*vLocalNetworkSpec)[i].isNullTerminated())

         table = (*vLocalNetworkSpec)[i].data();

         if (ipt->readEntries(table, true))
            {
            while ((entry = ipt->getNextEntry()))
               {
               /*
               const unsigned char* bytep = (const unsigned char*) &(entry->ip);

#           ifdef HAVE_ARCH64
               uint32_t ip_len = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%u.%u.%u.%u"), bytep[3], bytep[2], bytep[1], bytep[0]);
#           else
               uint32_t ip_len = u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%u.%u.%u.%u"), bytep[0], bytep[1], bytep[2], bytep[3]);
#           endif

               U_SRV_LOG("IP: %.*s SRC packets: %u bytes: %u DST packets: %u bytes: %u", ip_len, buffer, entry->src_packets, entry->src_bytes, entry->dst_packets, entry->dst_bytes);

               peer = peers->at(buffer, ip_len);
               */

#           ifdef HAVE_ARCH64
               entry->ip = htonl(entry->ip);
#           endif

               peer = peers->at((const char*)&(entry->ip), sizeof(uint32_t));

               U_SRV_LOG("IP: %v SRC packets: %u bytes: %u DST packets: %u bytes: %u peer = %p",
                          UIPAddress::toString(entry->ip).rep, entry->src_packets, entry->src_bytes, entry->dst_packets, entry->dst_bytes, peer);

               if (peer)
                  {
                  U_INTERNAL_DUMP("peer = %p", peer)

                  peer->ctraffic = (entry->src_bytes ? entry->src_bytes +
                                                       entry->dst_bytes : 0);

                  U_INTERNAL_DUMP("peer->ctraffic = %u peer->mac = %V peer->ip = %V peer->label = %V", peer->ctraffic, peer->mac.rep, peer->ip.rep, peer->label.rep)
                  }
               }
            }
         }
#  endif

      peers->setNodePointer(_ctime);

      do {
         peer = peers->elem();

         if (U_peer_allowed == false)
            {
            U_INTERNAL_ASSERT(u_isIPv4Addr(U_STRING_TO_PARAM(peer->ip)))

            // -----------------------------------------------------------------------------------------------------------------------------------------
            // $1 -> mac
            // $2 -> ip
            // $3 -> ap
            // $4 -> traffic
            // ---------------------
            // $5 -> time
            // $6 -> time_no_traffic
            // -----------------------------------------------------------------------------------------------------------------------------------------

            peer->getMAC(buffer);

            pfb->String(buffer, 12);
            pfb->UInt(ntohl(peer->addr));
            pfb->String(peer->label);

            _ctime = u_now->tv_sec - peer->_ctime;
                                     peer->_ctime = u_now->tv_sec;

            if (peer->ctraffic)
               {
               if (U_peer_permit == false)
                  {
                  U_SRV_LOG("WARNING: Peer IP %v MAC %v has made traffic(%u bytes) but it has status DENY", peer->ip.rep, peer->mac.rep, peer->ctraffic);
                  }

               pfb->UInt(peer->ctraffic);
                         peer->ctraffic = 0;

               peer->time_no_traffic = 0U;

               /*
               pfb->UInt(_ctime);
               pfb->UInt(0U);
               */
               }
            else
               {
               pfb->UInt(0U);

               peer->time_no_traffic += _ctime;

               /*
               pfb->UInt(0U);
               pfb->UInt(peer->time_no_traffic);
               */

               U_SRV_LOG("Peer IP %v MAC %v has made no traffic for %u secs", peer->ip.rep, peer->mac.rep, peer->time_no_traffic);
               }
            }
         }
      while (peers->next());
      }
}

U_NO_EXPORT void UNoDogPlugIn::makeNotifyData(UFlatBuffer* pfb, void* param)
{
   U_TRACE(0, "UNoDogPlugIn::makeNotifyData(%p,%p)", pfb, param)

   U_INTERNAL_DUMP("peer->mac = %V peer->ip = %V peer->label = %V", peer->mac.rep, peer->ip.rep, peer->label.rep)

   char buffer[256];

   pfb->String(buffer, getApInfo(buffer, sizeof(buffer), peer->label));

   peer->getMAC(buffer);

   pfb->String(buffer, 12);
   pfb->String(peer->ip);
}

U_NO_EXPORT void UNoDogPlugIn::makeLoginData(UFlatBuffer* pfb, void* param)
{
   U_TRACE(0, "UNoDogPlugIn::makeLoginData(%p,%p)", pfb, param)

   makeNotifyData(pfb, param);

   pfb->UInt(U_PTR2INT(peer));
}

int UNoDogPlugIn::handlerTime()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::handlerTime()")

   U_SET_MODULE_NAME(nodog);

   U_SRV_LOG("Checking peers for info");

#ifdef HAVE_ARCH64
   UFlatBufferSpaceLarge space;
#else
   UFlatBufferSpaceShort space;
#endif

   U_INTERNAL_ASSERT(info_data->isNull())

   UFlatBuffer::toVector(*info_data, makeInfoData);

   U_SRV_LOG("info_data(%u) = %#V", info_data->size(), info_data->rep);

   (void) client->sendPOSTRequestAsync(*info_data, *auth_info, true);

   info_data->clear();

   U_RESET_MODULE_NAME;

   U_RETURN(0); // monitoring
}

void UNoDogPlugIn::executeCommand(const char* type, uint32_t len)
{
   U_TRACE(0, "UNoDogPlugIn::executeCommand(%.*S,%u)", len, type, len)

   U_INTERNAL_ASSERT_POINTER(peer)

   char buffer[256];

   // NB: request(arp|deny|clear|reset|permit|openlist|initialize) mac ip class(Owner|Member|Public) UserDownloadRate UserUploadRate

   UCommand cmd(buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("/bin/sh %v %.*s %v %v Member 0 0"),
                                    fw_cmd->rep, len, type, (U_peer_mac_from_dhcp_data_file ? *UString::str_without_mac : peer->mac).rep, peer->ip.rep), fw_env);

   (void) cmd.executeAndWait(U_NULLPTR, -1, fd_stderr);

   U_SRV_LOG_CMD_MSG_ERR(cmd, false);
}

U_NO_EXPORT void UNoDogPlugIn::setMAC()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::setMAC()")

   if (peer->mac.empty() ||
       peer->mac == *UString::str_without_mac)
      {
      UString ifname = (*vInternalDevice)[0];

      if (pallow->device &&
          pallow->device != ifname)
         {
         U_SRV_LOG("WARNING: different device (%v => %v) for peer: IP %v MAC %v", ifname.rep, pallow->device.rep, peer->ip.rep, peer->mac.rep);

         ifname = pallow->device;
         }

      U_INTERNAL_DUMP("ifname = %V", ifname.rep)

      U_INTERNAL_ASSERT(ifname.isNullTerminated())

      peer->mac = UServer_Base::csocket->getMacAddress(ifname.data());

      if (peer->mac.empty())
         {
         (void) USocketExt::getARPCache(*arp_cache, *varp_cache);

         for (uint32_t i = 0, n = varp_cache->size(); i < n; i += 3)
            {
            if ((*varp_cache)[i].equal(peer->ip))
               {
               peer->mac = (*varp_cache)[i+1].copy();

               U_ASSERT_EQUALS(ifname, (*varp_cache)[i+2])

               break;
               }
            }
         }

      U_INTERNAL_DUMP("peer->mac = %V", peer->mac.rep)

      U_INTERNAL_ASSERT(peer->mac)
      U_INTERNAL_ASSERT(u_isMacAddr(U_STRING_TO_PARAM(peer->mac)))
      U_ASSERT_EQUALS(peer->mac, USocketExt::getMacAddress(peer->ip))
      }
}

U_NO_EXPORT void UNoDogPlugIn::setLabelAndMAC()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::setLabelAndMAC()")

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

   U_INTERNAL_DUMP("U_peer_index_network = %u", U_peer_index_network)

   if (U_peer_index_network == 0xFF)
      {
      U_ERROR("IP address for new peer %V not found in LocalNetworkMask %V", peer->ip.rep, localnet->rep);
      }

   peer->label = ((uint32_t)U_peer_index_network >= vLocalNetworkLabel->size() ? *UString::str_without_label : (*vLocalNetworkLabel)[U_peer_index_network]);

   setLabelAndMAC();

   if (mac_from_dhcp_data_file) U_peer_flag |= U_PEER_MAC_FROM_DHCP_DATA_FILE;

   peers->insert((const char*)&(peer->addr), sizeof(uint32_t), peer); // peers->insert(peer->ip, peer);
}

U_NO_EXPORT bool UNoDogPlugIn::getPeer()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::getPeer()")

   if (peers->empty() == false)
      {
      peer = peers->at((const char*)&addr, sizeof(uint32_t)); // peer = peers->at(U_CLIENT_ADDRESS_TO_PARAM);

      U_INTERNAL_DUMP("peer = %p", peer)

      if (peer) U_RETURN(true);
      }

   U_RETURN(false);
}

U_NO_EXPORT void UNoDogPlugIn::erasePeer()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::erasePeer()")

   U_INTERNAL_ASSERT_POINTER(peer)

   U_DELETE(peers->erase((const char*)&(peer->addr), sizeof(uint32_t))) // U_DELETE(peers->erase(peer->ip))
}

/*
U_NO_EXPORT void UNoDogPlugIn::printPeers(const char* msg, uint32_t len)
{
   U_TRACE(0, "UNoDogPlugIn::printPeers(%.*S,%u)", len, msg, len)

#if defined(DEBUG)  && !defined(U_LOG_DISABLE)
   if (UServer_Base::isLog())
      {
      typedef UHashMap<UModNoDogPeer*> uhashpeer;

#  ifdef HAVE_ARCH64
      char buffer_output[16U * 1024U];
#  else
      char buffer_output[ 1U * 1024U];
#  endif

      uint32_t buffer_output_len = UObject2String<uhashpeer>(*peers, buffer_output, sizeof(buffer_output));

      UServer_Base::log->log(U_CONSTANT_TO_PARAM("[nodog] %.*S peers = %.*S"), len, msg, buffer_output_len, buffer_output);
      }
#endif
}
*/

U_NO_EXPORT void UNoDogPlugIn::eraseTimer()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::eraseTimer()")

   if (U_peer_timer_active)
      {
      U_peer_flag &= ~U_PEER_TIMER_ACTIVE;

      UTimer::erase(peer);
      }
}

U_NO_EXPORT void UNoDogPlugIn::sendLogin()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::sendLogin()")

   UFlatBufferSpaceUser space;

   (void) client->sendPOSTRequestAsync(UFlatBuffer::toVector(makeLoginData), *auth_login, true);
}

U_NO_EXPORT void UNoDogPlugIn::sendNotify()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::sendNotify()")

   if (U_peer_notify_disable == false)
      {
      UFlatBufferSpaceUser space;

      U_peer_flag |= U_PEER_NOTIFY_DISABLE;

      (void) client->sendPOSTRequestAsync(UFlatBuffer::toVector(makeNotifyData), *auth_notify, true);
      }
}

U_NO_EXPORT void UNoDogPlugIn::sendStrictNotify()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::sendStrictNotify()")

   if (U_peer_strict_notify_disable == false)
      {
      UFlatBufferSpaceUser space;

      U_peer_flag |= U_PEER_STRICT_NOTIFY_DISABLE;

      (void) client->sendPOSTRequestAsync(UFlatBuffer::toVector(makeNotifyData), *auth_strict_notify, true);
      }
}

U_NO_EXPORT bool UNoDogPlugIn::checkOldPeer()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::checkOldPeer()")

   U_INTERNAL_ASSERT_POINTER(peer)
   U_ASSERT(peer->ip.equal(U_CLIENT_ADDRESS_TO_PARAM))

   UString mac    = peer->mac,
           llabel = peer->label;

   peer->mac = *UString::str_without_mac;

   setLabelAndMAC();

   if (   mac != peer->mac ||
       llabel != peer->label)
      {
      // NB: we assume that the current peer is a different user that has acquired the same IP address from the DHCP...

      U_SRV_LOG("WARNING: different user for peer (IP %v): (MAC %v LABEL %v) => (MAC %v LABEL %v)", peer->ip.rep, mac.rep, llabel.rep, peer->mac.rep, peer->label.rep);

      U_INTERNAL_ASSERT(mac)
      U_INTERNAL_ASSERT(llabel)
      U_INTERNAL_ASSERT(peer->mac)
      U_INTERNAL_ASSERT(peer->label)

      if (U_peer_permit)
         {
         UString x;

         if (U_peer_mac_from_dhcp_data_file == false)
            {
            x = peer->mac;
                peer->mac = mac;
            }

         deny();

         if (U_peer_mac_from_dhcp_data_file == false) peer->mac = x;
         }

      U_RETURN(false);
      }

   U_RETURN(true);
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
         U_NEW_WITHOUT_CHECK_MEMORY(UTDB, pdata, UTDB);

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

            U_DELETE((UTDB*)pdata)

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

   UString command(300U);

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

#ifndef HAVE_ARCH64
   U_NEW(UHashMap<UModNoDogPeer*>, peers, UHashMap<UModNoDogPeer*>( 256, UHashMap<void*>::setIndexIntHash));
#else
   U_NEW(UHashMap<UModNoDogPeer*>, peers, UHashMap<UModNoDogPeer*>(8192, UHashMap<void*>::setIndexIntHash));
#endif

   U_RETURN(U_PLUGIN_HANDLER_OK);
}

int UNoDogPlugIn::handlerFork()
{
   U_TRACE_NO_PARAM(0, "UNoDogPlugIn::handlerFork()")

   // send msg start to portal

   char buffer[256];
   bool bqueue = false, we_need_response = true;
   UString msg(200U), uptime(20U), allowed_web_hosts;

   uptime.setUpTime();

   msg.snprintf(U_CONSTANT_TO_PARAM("/start_ap?ap=%.*s&public=%v%%3A%u&pid=%d&uptime=%v"),
                getApInfo(buffer, sizeof(buffer), *label), buffer, IP_address_trust->rep, UServer_Base::port, UFile::getSysParam(chrash->data()), uptime.rep);

   (void) UFile::writeTo(*chrash, u_pid_str, u_pid_str_len);

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

   U_SRV_LOG_CMD_MSG_ERR(*fw, false);

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
      uint32_t index_network, increment = ((n % 3) ? 5 : 3);

      for (uint32_t i = 0; i < n; i += increment)
         {
         ip = vtmp[i+1];

         index_network = UIPAllow::find(ip, *vLocalNetworkMask);

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

         peer->ip  = ip.copy();
         peer->mac = vtmp[i];

         U_peer_flag |= U_PEER_ALLOWED;

         U_peer_index_network = index_network;

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
      UString x;
      uint32_t sz, index_network;
      char buffer[U_BUFFER_SIZE-512];

      U_http_info.nResponseCode = HTTP_NO_CONTENT;

      U_INTERNAL_DUMP("UServer_Base::client_address = %.*S", U_CLIENT_ADDRESS_TO_TRACE)

      U_SRV_LOG("Start REQUEST phase of plugin nodog: %.*S client_address = %.*S", U_HTTP_URI_TO_TRACE, U_CLIENT_ADDRESS_TO_TRACE);

      (void) UServer_Base::csocket->shutdown(SHUT_RD);

      if (UServer_Base::auth_ip->equal(U_CLIENT_ADDRESS_TO_PARAM))
         {
         // ------------------------------------------
         // request from AUTH, which may be:
         // ------------------------------------------
         // 1) /login_validate - login user validation
         // 2) /logout         - logout specific users
         // 3) /ping           - check 
         // ------------------------------------------

         U_SRV_LOG("Start REQUEST_FROM_AUTH phase of plugin nodog");

         if (U_HTTP_URI_STREQ("/login_validate"))
            {
            bool bdeny;
            char policy;
            UFlatBuffer fb, vec;

            fb.setRoot(*UClientImage_Base::body);
            fb.AsVector(vec);

            // $1 -> peer
            // $2 -> deny|permit ('0'|'1') policy: notify|no_notify|strict_notify ('0'|'1'|'2')

#        if defined(HAVE_ARCH64) && defined(U_LINUX)
            peer = (UModNoDogPeer*)  vec.AsVectorGet<uint64_t>(0);
#        else
            peer = (UModNoDogPeer*) (vec.AsVectorGet<uint64_t>(0) & 0x00000000ffffffffLL);
#        endif

            U_SRV_LOG("request to validate login: peer = %p", peer);

            U_INTERNAL_ASSERT_POINTER(peer)

            if (peers->findElement(peer) == false) goto bad;

            x = vec.AsVectorGet<UString>(1);

             bdeny = (x.first_char() == '0'); // deny|permit ('0'|'1')
            policy =  x.c_char(1);            // policy: notify|no_notify|strict_notify ('0'|'1'|'2')

            U_SRV_LOG("request to validate login for peer IP %v MAC %v: bdeny = %b policy = %C", peer->ip.rep, peer->mac.rep, bdeny, policy);

            U_INTERNAL_ASSERT(peer->mac)
            U_INTERNAL_ASSERT(u_isMacAddr(U_STRING_TO_PARAM(peer->mac)))

            if (bdeny)
               {
               eraseTimer();

               if (U_peer_permit) deny();

               erasePeer();

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
         else if (U_HTTP_URI_STREQ("/logout"))
            {
#        ifndef HAVE_ARCH64
            uint32_t vec[256],
#        else
            uint32_t vec[8192],
#        endif
            n = UFlatBuffer::toVectorInt(*UClientImage_Base::body, vec), ip_peer;

            U_SRV_LOG("AUTH request to logout %u users", n);

            for (uint32_t i = 0; i < n; ++i)
               {
               // --------
               // $1 -> ip
               // --------

               ip_peer = htonl(vec[i]);

               peer = peers->at((const char*)&ip_peer, sizeof(uint32_t)); // (*peers)[UIPAddress::toString(vec[i])];

               if (peer == U_NULLPTR)
                  {
                  U_SRV_LOG("AUTH request to logout user failed: IP %V", UIPAddress::toString(ip_peer).rep);

                  continue;
                  }

               U_INTERNAL_ASSERT(peer->mac)
               U_INTERNAL_ASSERT(u_isMacAddr(U_STRING_TO_PARAM(peer->mac)))

               U_SRV_LOG("AUTH request to logout user(%u): IP %v MAC %v", i, peer->ip.rep, peer->mac.rep);

               if (U_peer_permit) deny();
               else
                  {
                  eraseTimer();

                  U_SRV_LOG("AUTH request to logout user with status DENY", 0);
                  }

               erasePeer();
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

   // U_SRV_LOG("Start REQUEST_FROM_USER phase of plugin nodog");

   // printPeers(U_CONSTANT_TO_PARAM("user request"));

      addr = UServer_Base::getClientAddress();

      if (getPeer())
         {
         // ---------------------
         // request from OLD user
         // ---------------------

         U_SRV_LOG("Start REQUEST_FROM_OLD_USER phase of plugin nodog: peer = %p", peer);

         U_INTERNAL_ASSERT(peer->mac)
         U_ASSERT(peer->ip.equal(U_CLIENT_ADDRESS_TO_PARAM))
         U_INTERNAL_ASSERT(u_isMacAddr(U_STRING_TO_PARAM(peer->mac)))

         if (U_HTTP_URI_MEMEQ("/nodog_peer_allow.sh"))
            {
            if (U_HTTP_QUERY_MEMEQ("url=") &&
                U_peer_allow_disable == false)
               {
               /**
                * open firewall, respond with redirect to original request
                *
                * $1 -> url
                * $2 -> forced ('0'|'1')
                */

               UString redirect;
               bool forced = false;
               uint32_t n = UHTTP::processForm();

               UHTTP::getFormValue(redirect, U_CONSTANT_TO_PARAM("url"), 0, 1, n);

               if (n == 2*2)
                  {
                  UHTTP::getFormValue(x, U_CONSTANT_TO_PARAM("forced"), 0, 3, 4);

                  if (x.first_char() == '1') forced = true;
                  }

               if (forced)
                  {
                  sendStrictNotify();

                  goto next1;
                  }

               U_INTERNAL_DUMP("T2 = %u U_peer_policy = %C", T2, U_peer_policy)

               if (T2 < 3600 &&
                   U_peer_policy != '2') // (strict notify)
                  {
                  sendNotify();

next1:            eraseTimer();

                  if (U_peer_permit == false) permit();

                  UHTTP::setRedirectResponse(UHTTP::NO_BODY, U_STRING_TO_PARAM(redirect));
                  }
               }

            goto end;
            }

         if (U_HTTP_URI_MEMEQ("/nodog_peer_delay.sh"))
            {
            if (U_peer_delay_disable == false)
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
               }

            goto end;
            }

         sz = setRedirect(buffer, sizeof(buffer));

         if (checkOldPeer() == false) goto log;

         goto welcome;
         }

      // ---------------------
      // request from NEW user
      // ---------------------

      index_network = UIPAllow::find(UServer_Base::client_address, *vLocalNetworkMask);

      U_INTERNAL_DUMP("index_network = %u", index_network)

      if (index_network == U_NOT_FOUND)
         {
         U_SRV_LOG("WARNING: IP address for new peer %.*S not found in LocalNetworkMask %V", U_CLIENT_ADDRESS_TO_TRACE, localnet->rep);

         goto end;
         }

      U_NEW(UModNoDogPeer, peer, UModNoDogPeer);

      U_SRV_LOG("Start REQUEST_FROM_NEW_USER phase of plugin nodog: index_network = %u peer = %p", index_network, peer);

      pallow = vLocalNetworkMask->at((U_peer_index_network = index_network));

      U_INTERNAL_ASSERT_POINTER(pallow)

             peer->addr = addr;
      (void) peer->ip.replace(U_CLIENT_ADDRESS_TO_PARAM);

      U_ASSERT_EQUALS(peer->ip, UIPAddress::toString(peer->addr))

      setNewPeer();

      sz = setRedirect(buffer, sizeof(buffer));

log:  sendLogin();

      U_INTERNAL_DUMP("T1 = %u", T1)

      if (T1 == 0)
         {
         permit();

         UHTTP::setRedirectResponse(UHTTP::NO_BODY, buffer, sz);
         }
      else
         {
         if (T1 < 3600)
            {
            peer->UEventTime::setTimeToExpire(T1);

            U_peer_flag |= U_PEER_TIMER_ACTIVE;

            UTimer::insert(peer);
            }

welcome: x = UString::getUBuffer();

         x.snprintf(U_CONSTANT_TO_PARAM("%v://%v/welcome?url="), auth_service->rep, auth_host->rep);

         Url::encode_add(buffer, sz, x);

         peer->getMAC(buffer);

         x.snprintf_add(U_CONSTANT_TO_PARAM("&mac=%.12s&apid=%v&gateway=%v%%3A%u"),
                        buffer, peer->label.rep, (bnetwork_interface ? pallow->host : *UServer_Base::IP_address).rep, UServer_Base::port);

         UHTTP::setRedirectResponse(UHTTP::NO_BODY, U_STRING_TO_PARAM(x));
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
   *UObjectIO::os << "addr              " << addr            <<  '\n'
                  << "_ctime            " << _ctime          <<  '\n'
                  << "ctraffic          " << ctraffic        <<  '\n'
                  << "time_no_traffic   " << time_no_traffic <<  '\n'
                  << "ip       (UString " << (void*)&ip      << ")\n"
                  << "mac      (UString " << (void*)&mac     << ")\n"
                  << "label    (UString " << (void*)&label   << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}

const char* UNoDogPlugIn::dump(bool _reset) const
{
   *UObjectIO::os << "fd_stderr                                    " << fd_stderr                 <<  '\n'
                  << "label              (UString                  " << (void*)label              << ")\n"
                  << "extdev             (UString                  " << (void*)extdev             << ")\n"
                  << "fw_cmd             (UString                  " << (void*)fw_cmd             << ")\n"
                  << "fw_env             (UString                  " << (void*)fw_env             << ")\n"
                  << "intdev             (UString                  " << (void*)intdev             << ")\n"
                  << "hostname           (UString                  " << (void*)hostname           << ")\n"
                  << "localnet           (UString                  " << (void*)localnet           << ")\n"
                  << "auth_info          (UString                  " << (void*)auth_info          << ")\n"
                  << "auth_login         (UString                  " << (void*)auth_login         << ")\n"
                  << "allowed_members    (UString                  " << (void*)allowed_members    << ")\n"
                  << "fw                 (UCommand                 " << (void*)fw                 << ")\n"
                  << "ipt                (UIptAccount              " << (void*)ipt                << ")\n"
                  << "vInternalDevice    (UVector<UString>         " << (void*)vInternalDevice    << ")\n"
                  << "vLocalNetworkLabel (UVector<UString>         " << (void*)vLocalNetworkLabel << ")\n"
                  << "client             (UHttpClient<UTCPSocket>  " << (void*)client             << ")\n"
                  << "peers              (UHashMap<UModNoDogPeer*> " << (void*)peers              << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return U_NULLPTR;
}
#endif
