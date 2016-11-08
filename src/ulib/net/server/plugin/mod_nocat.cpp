// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_nocat.cpp - this is a plugin nocat for userver
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
#include <ulib/utility/base64.h>
#include <ulib/utility/escape.h>
#include <ulib/net/ipt_ACCOUNT.h>
#include <ulib/utility/hexdump.h>
#include <ulib/utility/services.h>
#include <ulib/utility/dir_walk.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/string_ext.h>
#include <ulib/net/server/client_image.h>
#include <ulib/net/server/plugin/mod_nocat.h>

#ifdef USE_LIBTDB
#  include <ulib/db/tdb.h>
#endif

U_CREAT_FUNC(server_plugin_nocat, UNoCatPlugIn)

int                        UNoCatPlugIn::fd_stderr;
int                        UNoCatPlugIn::check_type;
int                        UNoCatPlugIn::next_event_time;
bool                       UNoCatPlugIn::flag_check_system;
long                       UNoCatPlugIn::last_request_check;
long                       UNoCatPlugIn::last_request_firewall;
void*                      UNoCatPlugIn::pdata;
UPing**                    UNoCatPlugIn::sockp;
fd_set                     UNoCatPlugIn::addrmask;
fd_set*                    UNoCatPlugIn::paddrmask;
uint32_t                   UNoCatPlugIn::nfds;
uint32_t                   UNoCatPlugIn::num_radio;
uint32_t                   UNoCatPlugIn::check_expire;
uint32_t                   UNoCatPlugIn::time_available;
uint32_t                   UNoCatPlugIn::total_connections;
uint32_t                   UNoCatPlugIn::idx_peers_preallocate;
uint32_t                   UNoCatPlugIn::num_peers_preallocate;
uint64_t                   UNoCatPlugIn::traffic_available;
UString*                   UNoCatPlugIn::input;
UString*                   UNoCatPlugIn::label;
UString*                   UNoCatPlugIn::fw_env;
UString*                   UNoCatPlugIn::fw_cmd;
UString*                   UNoCatPlugIn::extdev;
UString*                   UNoCatPlugIn::intdev;
UString*                   UNoCatPlugIn::mempool;
UString*                   UNoCatPlugIn::hostname;
UString*                   UNoCatPlugIn::localnet;
UString*                   UNoCatPlugIn::location;
UString*                   UNoCatPlugIn::arp_cache;
UString*                   UNoCatPlugIn::auth_login;
UString*                   UNoCatPlugIn::decrypt_key;
UString*                   UNoCatPlugIn::status_content;
UString*                   UNoCatPlugIn::label_to_match;
UString*                   UNoCatPlugIn::allowed_members;
UString*                   UNoCatPlugIn::IP_address_trust;
UString*                   UNoCatPlugIn::peer_present_in_arp_cache;
UCommand*                  UNoCatPlugIn::fw;
UDirWalk*                  UNoCatPlugIn::dirwalk;
UIptAccount*               UNoCatPlugIn::ipt;
UModNoCatPeer*             UNoCatPlugIn::peer;
UModNoCatPeer*             UNoCatPlugIn::peers_delete;
UModNoCatPeer*             UNoCatPlugIn::peers_preallocate;
UVector<Url*>*             UNoCatPlugIn::vauth_url;
UVector<UString>*          UNoCatPlugIn::vauth;
UVector<UString>*          UNoCatPlugIn::vauth_ip;
UVector<UString>*          UNoCatPlugIn::openlist;
UVector<UString>*          UNoCatPlugIn::varp_cache;
UVector<UString>*          UNoCatPlugIn::vinfo_data;
UVector<UString>*          UNoCatPlugIn::vroaming_data;
UVector<UString>*          UNoCatPlugIn::vLoginValidate;
UVector<UString>*          UNoCatPlugIn::vInternalDevice;
UVector<UString>*          UNoCatPlugIn::vLocalNetworkLabel;
UVector<UIPAllow*>*        UNoCatPlugIn::vLocalNetworkMask;
UVector<UIPAddress*>**     UNoCatPlugIn::vaddr;
UHttpClient<UTCPSocket>*   UNoCatPlugIn::client;
UHashMap<UModNoCatPeer*>*  UNoCatPlugIn::peers;

#define U_NOCAT_STATUS \
   "<html>\n" \
   "<head>\n" \
   "<meta http-equiv=\"Cache Control\" content=\"max-age=0\">\n" \
   "<title>Access Point: %v</title>" \
   "</head>\n" \
   "<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n" \
   "<h1>Access Point: %v</h1>\n" \
   "<hr noshade=\"1\"/>\n" \
   "<table border=\"0\" cellpadding=\"5\" cellspacing=\"0\">\n" \
      "<tr><td>Current Time</td><td>%5D</td></tr>\n" \
      "<tr><td>Gateway Up Since</td><td>%#5D</td></tr>\n" \
      "<tr><td>GatewayVersion</td><td>" ULIB_VERSION "</td></tr>\n" \
      "<tr><td>ExternalDevice</td><td>%v</td></tr>\n" \
      "<tr><td>InternalDevice</td><td>%v</td></tr>\n" \
      "<tr><td>LocalNetwork</td><td>%v</td></tr>\n" \
      "<tr><td>GatewayPort</td><td>%u</td></tr>\n" \
      "<tr><td>AuthServiceAddr</td><td>%v</td></tr>\n" \
      "%v" \
      "</table>\n" \
      "<hr noshade=\"1\"/>\n" \
      "<table border=\"0\" cellpadding=\"5\" cellspacing=\"0\">\n" \
      "<tr><td>Users</td><td>%u</td></tr>\n" \
      "<tr><td>Users Connected</td><td>%u</td></tr>\n" \
      "<tr><td></td><td></td></tr>\n" \
      "<tr><td align=\"center\"><h2>Current Users</h2></td>\n" \
      "<table border=\"1\" cellpadding=\"5\">\n" \
         "<tr><th>User UID</th>\n" \
         "<th>IP address</th>\n" \
         "<th>Connection time</th>\n" \
         "<th>Elapsed connection time</th>\n" \
         "<th>Left connection time</th>\n" \
         "<th>Consumed traffic</th>\n" \
         "<th>Left traffic</th>\n" \
         "<th>MAC address</th>\n" \
         "<th>Status</th></tr>\n" \
          "%v" \
      "</table></td></tr>\n" \
   "</table>\n" \
   "<hr noshade=\"1\"/>\n" \
   "<img src=\"/images/auth_logo.png\" width=\"112\" height=\"35\">\n" \
   "<p style=\"text-align:right\">Powered by ULib</p>\n" \
   "</body>\n" \
   "</html>"

// define method VIRTUAL of class UEventTime

int UModNoCatPeer::handlerTime()
{
   U_TRACE_NO_PARAM(0, "UModNoCatPeer::handlerTime()")

   int disconnected = checkPeerInfo(true);

   if (disconnected)
      {
      UNoCatPlugIn::peer = this;

      UNoCatPlugIn::deny(disconnected, false);
      }

   U_RETURN(-1); // normal
}

int UNoCatPlugIn::handlerTime()
{
   U_TRACE_NO_PARAM(0, "UNoCatPlugIn::handlerTime()")

   U_INTERNAL_ASSERT_MAJOR(check_expire, 0)

   next_event_time = check_expire;

   checkSystem();

   U_INTERNAL_DUMP("check_expire = %u next_event_time = %u", check_expire, next_event_time)

   UEventTime::setTimeToExpire(next_event_time);

   U_RETURN(0); // monitoring
}

UNoCatPlugIn::UNoCatPlugIn()
{
   U_TRACE_REGISTER_OBJECT(0, UNoCatPlugIn, "")

   U_NEW(UCommand, fw, UCommand);

   U_NEW(UString, label, UString);
   U_NEW(UString, input, UString(U_CAPACITY));
   U_NEW(UString, fw_cmd, UString);
   U_NEW(UString, fw_env, UString);
   U_NEW(UString, extdev, UString);
   U_NEW(UString, intdev, UString);
   U_NEW(UString, mempool, UString(UFile::contentOf(U_STRING_FROM_CONSTANT("/etc/nodog.mempool"))));
   U_NEW(UString, hostname, UString);
   U_NEW(UString, localnet, UString);
   U_NEW(UString, location, UString(U_CAPACITY));
   U_NEW(UString, arp_cache, UString);
   U_NEW(UString, auth_login, UString);
   U_NEW(UString, decrypt_key, UString);
   U_NEW(UString, allowed_members, UString);
   U_NEW(UString, IP_address_trust, UString);
   U_NEW(UString, peer_present_in_arp_cache, UString);

   U_NEW(UVector<Url*>, vauth_url, UVector<Url*>(4U));
   U_NEW(UVector<UString>, vauth, UVector<UString>(4U));
   U_NEW(UVector<UString>, vauth_ip, UVector<UString>(4U));
   U_NEW(UVector<UString>, varp_cache, UVector<UString>);
   U_NEW(UVector<UString>, vinfo_data, UVector<UString>(4U));
   U_NEW(UVector<UString>, vroaming_data, UVector<UString>(4U));
   U_NEW(UVector<UString>, vLoginValidate, UVector<UString>);
   U_NEW(UVector<UString>, vInternalDevice, UVector<UString>(64U));
   U_NEW(UVector<UString>, vLocalNetworkLabel, UVector<UString>(64U));
   U_NEW(UVector<UIPAllow*>, vLocalNetworkMask, UVector<UIPAllow*>);
   U_NEW(UHttpClient<UTCPSocket>, client, UHttpClient<UTCPSocket>(0));

   client->UClient_Base::setTimeOut(UServer_Base::timeoutMS);

   UString::str_allocate(STR_ALLOCATE_NOCAT);
}

UNoCatPlugIn::~UNoCatPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, UNoCatPlugIn)

   delete fw;
   delete label;
   delete input;
   delete fw_cmd;
   delete fw_env;
   delete extdev;
   delete intdev;
   delete client;
   delete mempool;
   delete localnet;
   delete location;
   delete hostname;
   delete auth_login;
   delete decrypt_key;
   delete allowed_members;
   delete IP_address_trust;
   delete peer_present_in_arp_cache;

   delete vauth;
   delete vauth_ip;
   delete vauth_url;
   delete vinfo_data;
   delete vroaming_data;
   delete vLoginValidate;
   delete vInternalDevice;
   delete vLocalNetworkMask;
   delete vLocalNetworkLabel;

   delete varp_cache;
   delete  arp_cache;

   if (ipt)     delete ipt;
   if (peers)   delete peers;
   if (dirwalk) delete dirwalk;

   if (status_content)
      {
      delete openlist;
      delete status_content;
      }

   if (vaddr)
      {
      for (uint32_t i = 0; i < num_radio; ++i)
         {
         delete vaddr[i];
         delete sockp[i];
         }

      UMemoryPool::_free(sockp, num_radio, sizeof(UPing*));
      UMemoryPool::_free(vaddr, num_radio, sizeof(UVector<UIPAddress*>*));
      }

   if (peers_delete)
      {
      UModNoCatPeer* next;

      for (UModNoCatPeer* p = peers_delete; p; p = next)
         {
         next = p->next;

         delete[] p;
         }
      }

#ifdef USE_LIBTDB
   if (pdata) delete (UTDB*)pdata;
#endif
}

// status

bool UNoCatPlugIn::getPeerStatus(UStringRep* key, void* value)
{
   U_TRACE(0, "UNoCatPlugIn::getPeerStatus(%p,%p)", key, value)

   peer = (UModNoCatPeer*)value;

   if ( label_to_match &&
       *label_to_match != peer->label)
      {
      U_RETURN(true);
      }

   const char* color;
   const char* status;
   time_t how_much_connected;
   UString buffer(U_CAPACITY);
   int disconnected = peer->checkPeerInfo(key == 0);

   if (disconnected) deny(disconnected, check_expire);

   U_INTERNAL_DUMP("now    = %#5D", u_now->tv_sec)
   U_INTERNAL_DUMP("logout = %#5D", peer->logout)
   U_INTERNAL_DUMP("expire = %#5D", peer->expire)

   if (U_peer_status == UModNoCatPeer::PEER_PERMIT)
      {
      color  = "green";
      status = "PERMIT";

      how_much_connected = u_now->tv_sec - peer->connected;
      }
   else
      {
      color  = "red";
      status = "DENY";

      how_much_connected = peer->logout - peer->connected;
      }

   char c;
   const char* mac = peer->mac.data();
   uint64_t traffic_remain = peer->traffic_remain /1024;

   U_INTERNAL_DUMP("traffic_remain = %llu", traffic_remain)

   if (traffic_remain < 1024) c = 'K';
   else
      {
      c = 'M';

      traffic_remain /= 1024;
      }

   buffer.snprintf(U_CONSTANT_TO_PARAM("<tr>\n"
                     "<td>%v</td>\n"
                     "<td>%v</td>\n"
                     "<td>%#5D</td>\n"
                     "<td>%#2D</td>\n"
                     "<td>%#2D</td>\n"
                     "<td>%llu KBytes</td>\n"
                     "<td>%llu %cBytes</td>\n"
                     "<td><a href=\"http://standards.ieee.org/cgi-bin/ouisearch?%c%c%c%c%c%c\">%v</a></td>\n"
                     "<td style=\"color:%s\">%s</td>\n"
                   "</tr>\n"),
                   peer->user.rep,
                   peer->ip.rep,
                   peer->connected + u_now_adjust,
                   how_much_connected,
                   peer->time_remain,
                   peer->traffic_done / 1024,
                   traffic_remain, c,
                   mac[0], mac[1], mac[3], mac[4], mac[6], mac[7], peer->mac.rep,
                   color, status);

   (void) status_content->append(buffer);

   U_RETURN(true);
}

bool UNoCatPlugIn::getPeerListInfo(UStringRep* key, void* value)
{
   U_TRACE(0, "UNoCatPlugIn::getPeerListInfo(%p,%p)", key, value)

   peer = (UModNoCatPeer*) value;

   if (U_peer_allowed == false &&
       U_peer_status  == UModNoCatPeer::PEER_PERMIT)
      {
      U_INTERNAL_ASSERT(u_now->tv_sec >= peer->ctime)

      UString buffer(U_CAPACITY);

      buffer.snprintf(U_CONSTANT_TO_PARAM("%v %v %v %ld %llu\n"), peer->user.rep, peer->ip.rep, peer->mac.rep, u_now->tv_sec - peer->ctime, peer->traffic_done);

      (void) status_content->append(buffer);
      }

   U_RETURN(true);
}

void UNoCatPlugIn::setStatusContent(const UString& ap_label)
{
   U_TRACE(0, "UNoCatPlugIn::setStatusContent(%V)", ap_label.rep)

   status_content->setBuffer(U_CAPACITY);

   label_to_match = (ap_label.empty() ? 0 : (UString*)&ap_label);

   if (peer) getPeerStatus(0, peer);
   else
      {
      getTraffic();

      peers->callForAllEntry(getPeerStatus);

      U_INTERNAL_ASSERT(hostname->isNullTerminated())

      UString label_buffer(100U);

      if (label_to_match)
         {
         uint32_t index = vLocalNetworkLabel->find(ap_label);

         if (index < vLocalNetworkMask->size())
            {
            UString x = (*vLocalNetworkMask)[index]->spec;

            label_buffer.snprintf(U_CONSTANT_TO_PARAM("<tr><td>Label (Local Network Mask)</td><td>%v (%v)</td></tr>\n"), ap_label.rep, x.rep);
            }
         }

      UString buffer(1024U + sizeof(U_NOCAT_STATUS) + intdev->size() + localnet->size() + status_content->size() + label_buffer.size());

      buffer.snprintf(U_CONSTANT_TO_PARAM(U_NOCAT_STATUS), hostname->rep, hostname->rep, u_start_time,
                      extdev->rep,
                      intdev->rep,
                      localnet->rep,
                      UServer_Base::port,
                      auth_login->rep,
                      label_buffer.rep,
                      peers->size(),
                      total_connections,
                      status_content->rep);

      *status_content = buffer;
      }

   setHTTPResponse(*status_content, U_html);
}

void UNoCatPlugIn::getTraffic()
{
   U_TRACE_NO_PARAM(0, "UNoCatPlugIn::getTraffic()")

#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
   UString ip(17U);
   uint32_t traffic;
   UModNoCatPeer* _peer;
   const unsigned char* bytep;
   struct ipt_acc_handle_ip* entry;

   for (uint32_t i = 0, n = vLocalNetworkMask->size(); i < n; ++i)
      {
      U_INTERNAL_ASSERT((*vLocalNetworkMask)[i]->spec.isNullTerminated())

      const char* table = (*vLocalNetworkMask)[i]->spec.data();

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

            U_INTERNAL_DUMP("IP: %v SRC packets: %u bytes: %u DST packets: %u bytes: %u", ip.rep, entry->src_packets, entry->src_bytes, entry->dst_packets, entry->dst_bytes)

            _peer = (*peers)[ip];

            U_INTERNAL_DUMP("_peer = %p", _peer)

            if (_peer)
               {
               traffic = entry->src_bytes +
                         entry->dst_bytes;

               if (traffic)
                  {
                  _peer->ctraffic     += traffic;
                  _peer->traffic_done += traffic;

                  _peer->traffic_remain = (_peer->traffic_available > _peer->traffic_done ? (_peer->traffic_available - _peer->traffic_done) : 0);

                  U_INTERNAL_DUMP("traffic = %u traffic_done = %llu ctraffic = %u traffic_remain = %llu", traffic, _peer->traffic_done, _peer->ctraffic, _peer->traffic_remain)
                  }
               }
            }
         }
      }
#endif
}

void UNoCatPlugIn::uploadFileToPortal(UFile& file)
{
   U_TRACE(0, "UNoCatPlugIn::uploadFileToPortal(%.*S)", U_FILE_TO_TRACE(file))

   bool result = true;

   if (((void)file.stat(), file.getSize()) > (2 * 1024))
      {
      UString filename = file.getName();

      if (UStringExt::startsWith(filename, *hostname) == false)
         {
         UString tmp(300U);

         tmp.snprintf(U_CONSTANT_TO_PARAM("%v_%v"), hostname->rep, filename.rep);

         filename = tmp;
         }

      UClient_Base::queue_dir = UString::str_CLIENT_QUEUE_DIR;

      result = client->upload(getUrlForSendMsgToPortal(0, 0, 0), file, U_STRING_TO_PARAM(filename), 3);

      U_SRV_LOG("%s to queue log file: %.*S", (result ? "success" : "FAILED"), U_FILE_TO_TRACE(file));

      UClient_Base::queue_dir = 0;
      }

   if (result) (void) file._unlink();
}

void UNoCatPlugIn::sendMsgToPortal(uint32_t index_AUTH, const UString& msg, UString* poutput)
{
   U_TRACE(0, "UNoCatPlugIn::sendMsgToPortal(%u,%V,%p)", index_AUTH, msg.rep, poutput)

#ifndef U_LOG_DISABLE
   const char* result = "FAILED";
#endif
   bool bqueue = false, we_need_response = (poutput != 0);

   UString url = getUrlForSendMsgToPortal(index_AUTH, U_STRING_TO_PARAM(msg));

loop:
   if (we_need_response == false) UClient_Base::queue_dir = UString::str_CLIENT_QUEUE_DIR;

   if (client->connectServer(url) == false)
      {
      if (we_need_response)
         {
         U_SRV_LOG("sent message FAILED to AUTH(%u): %V", index_AUTH, msg.rep);

         we_need_response = false;

         goto loop;
         }

      goto end;
      }

   if (client->sendRequest())
      {
      if (poutput) *poutput = client->getContent();

#  ifndef U_LOG_DISABLE
      result = "success";
#  endif
      }
   else
      {
      if (poutput) poutput->clear();

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
      U_SRV_LOG("queue message %s to AUTH(%u): %V", result, index_AUTH, msg.rep);
      }

   UClient_Base::queue_dir = 0;

   client->reset(); // NB: url is referenced by UClient::url...
}

bool UNoCatPlugIn::checkPeerStatus(UStringRep* key, void* value)
{
   U_TRACE(0, "UNoCatPlugIn::checkPeerStatus(%V,%p)", key, value)

   U_INTERNAL_ASSERT_POINTER(openlist)

   peer         = (UModNoCatPeer*) value;
   uint32_t pos = (openlist->empty() ? U_NOT_FOUND
                                     : openlist->find(peer->ip));

   if (pos == U_NOT_FOUND)
      {
      // firewall close

      if (U_peer_status == UModNoCatPeer::PEER_PERMIT)
         {
         // status open

         if (U_peer_allowed)
            {
            U_SRV_LOG("WARNING: I should to deny user allowed: IP %v MAC %v - total_connections %u", peer->ip.rep, peer->mac.rep, total_connections);
            }
         else
            {
            U_SRV_LOG("WARNING: I try to deny user with status permit: IP %v MAC %v remain: %ld secs %llu bytes - total_connections %u",
                       peer->ip.rep, peer->mac.rep, peer->time_remain, peer->traffic_remain, total_connections);

            deny(U_LOGOUT_CHECK_FIREWALL, check_expire);
            }
         }
      }
   else
      {
      // firewall open

      if (U_peer_status == UModNoCatPeer::PEER_DENY)
         {
         // status deny

         U_SRV_LOG("WARNING: I try to deny user with status deny: IP %v MAC %v remain: %ld secs %llu bytes - total_connections %u - logout %#1D",
                    peer->ip.rep, peer->mac.rep, peer->time_remain, peer->traffic_remain, total_connections, peer->logout);

         executeCommand(UModNoCatPeer::PEER_DENY);
         }

      openlist->erase(pos);
      }

   U_RETURN(true);
}

uint32_t UNoCatPlugIn::checkFirewall(UString& output)
{
   U_TRACE(1, "UNoCatPlugIn::checkFirewall(%V)", output.rep)

   bool ok;
   uint32_t n = 0, counter = 0,
            timeoutMS = UCommand::timeoutMS;
                        UCommand::timeoutMS = -1;

loop:
   output.setEmpty();

   ok = fw->execute(0, &output, -1, fd_stderr);

#ifndef U_LOG_DISABLE
   UServer_Base::logCommandMsgError(fw->getCommand(), false);
#endif

   if (ok == false)
      {
      n = total_connections;

      if (UCommand::setMsgError(fw->getCommand(), true))
         {
         U_WARNING("%s%.*s", UServer_Base::mod_name[0], u_buffer_len, u_buffer);
         }

      errno        = 0;
      u_buffer_len = 0;
      }
   else
      {
      n = (output.empty() ? (openlist->clear(), 0)
                          :  openlist->split(output)); // NB: use substr(), so dependency from output...

      U_INTERNAL_DUMP("openlist->split() = %u total_connections = %u peers->size() = %u", n, total_connections, peers->size())

      if (n != total_connections    &&
          (output.isText() == false ||
          ++counter < 1))
         {
         goto loop;
         }
      }

   UCommand::timeoutMS = timeoutMS;

   U_RETURN(n);
}

bool UNoCatPlugIn::checkFirewall()
{
   U_TRACE_NO_PARAM(0, "UNoCatPlugIn::checkFirewall()")

   long firewall_interval = u_now->tv_sec - last_request_firewall;

   U_INTERNAL_DUMP("firewall_interval = %ld", firewall_interval)

   U_INTERNAL_ASSERT(u_now->tv_sec >= last_request_firewall)

   if (firewall_interval < 3)
      {
      next_event_time = 3 - firewall_interval;

      U_RETURN(false);
      }

   UString output(U_CAPACITY);
   uint32_t n = checkFirewall(output);

   U_SRV_LOG("checkFirewall(): total_connections %u firewall %u", total_connections, n);

   if (n != total_connections)
      {
      UCommand _fw;
      UString out, msg(300U), ip, mac;
      uint32_t n1, j, k = varp_cache->size();

      if (n)
         {
         out = openlist->join(' ');

      // (void) UFile::writeToTmp(U_STRING_TO_PARAM(out), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("nodog_chk.out"), 0);
         }

      n = total_connections;

      peers->callForAllEntry(checkPeerStatus);

      U_SRV_LOG("*** FIREWALL NOT ALIGNED: %V ***", out.rep);

      n1 = openlist->size();

      U_INTERNAL_DUMP("openlist->size() = %u total_connections = %u peers->size() = %u", n1, total_connections, peers->size())

      U_INTERNAL_ASSERT(n1 <= n)

      for (uint32_t i = 0; i < n1; ++i)
         {
         ip = (*openlist)[i];

         if ((*peers)[ip] ||
             u_isIPv4Addr(U_STRING_TO_PARAM(ip)) == false)
            {
            U_SRV_LOG("WARNING: anomalous peer: IP %S - total_connections %u", &ip, total_connections);
            }
         else  
            {
            for (j = 0; j < k; j += 3)
               {
               if ((*varp_cache)[j].equal(ip))
                  {
                  mac = (*varp_cache)[j+1];

                  break;
                  }
               }

            U_SRV_LOG("WARNING: I try to deny IP %V MAC %v - total_connections %u", ip.rep, mac.rep, total_connections);

            U_ASSERT_EQUALS(mac, USocketExt::getMacAddress(ip))

            // NB: request(arp|deny|clear|reset|permit|openlist|initialize) mac ip class(Owner|Member|Public) UserDownloadRate UserUploadRate

            setFireWallCommand(_fw, *fw_cmd, mac, ip);

            (void) _fw.executeAndWait(0, -1, fd_stderr);

#        ifndef U_LOG_DISABLE
            UServer_Base::logCommandMsgError(_fw.getCommand(), false);
#        endif
            }
         }

      if (n != total_connections)
         {
         // send msg to portal for resync

         msg.snprintf(U_CONSTANT_TO_PARAM("/error_ap?ap=%v@%v&public=%v%%3A%u"), label->rep, hostname->rep, IP_address_trust->rep, UServer_Base::port);

         sendMsgToAllPortal(msg);
         }
      }

   openlist->clear();

   output.clear();

   last_request_firewall = u_now->tv_sec;

   U_RETURN(true);
}

void UNoCatPlugIn::checkSystem()
{
   U_TRACE_NO_PARAM(1, "UNoCatPlugIn::checkSystem()")

   uint32_t i, n;
   long check_interval;

   U_INTERNAL_DUMP("flag_check_system = %b check_expire = %u", flag_check_system, check_expire)

   U_INTERNAL_ASSERT_MAJOR(check_expire, 0)
   U_INTERNAL_ASSERT_EQUALS(flag_check_system, false)

   flag_check_system = true;

   check_interval = u_now->tv_sec - last_request_check;

   U_INTERNAL_DUMP("check_interval = %ld", check_interval)

   U_INTERNAL_ASSERT(u_now->tv_sec >= last_request_check)

   if (check_interval < U_TIME_FOR_ARPING_ASYNC_COMPLETION) // NB: protection from DoS...
      {
      next_event_time = check_expire - check_interval;

      goto end;
      }

   (void) getARPCache();

   U_SET_MODULE_NAME(nocat);

   if ((check_type & U_CHECK_FIREWALL) != 0 &&
       checkFirewall() == false) // check firewall status
      {
      goto end;
      }

   U_INTERNAL_ASSERT_EQUALS(paddrmask, 0)

   if (isPingAsyncPending())
      {
      U_SRV_LOG("Pending arping in process (%u), waiting for completion...", nfds);

      paddrmask = UPing::checkForPingAsyncCompletion(nfds);

      if (paddrmask) goto result;

      next_event_time = U_TIME_FOR_ARPING_ASYNC_COMPLETION + 2;

      goto end;
      }

   for (nfds = i = 0; i < num_radio; ++i) vaddr[i]->clear();

   FD_ZERO(&addrmask);

   U_SRV_LOG("Checking peers for info");

   getTraffic();

   peers->callForAllEntry(checkPeerInfo);

   U_INTERNAL_DUMP("nfds = %u addrmask = %B", nfds, __FDS_BITS(&addrmask)[0])

   if (nfds)
      {
      if ((check_type & U_CHECK_ARP_CACHE) != 0)
         {
         paddrmask = UPing::checkARPCache(*varp_cache, vaddr, num_radio);
         }
#  ifdef HAVE_NETPACKET_PACKET_H
      else if ((check_type & U_CHECK_ARP_PING) != 0)
         {
         paddrmask = UPing::arping(sockp, vaddr, num_radio, true, *vInternalDevice);
         }
#  endif
      else
         {
         paddrmask = &addrmask;
         }
      }

   if (paddrmask)
      {
result:
      U_INTERNAL_ASSERT_MAJOR(nfds, 0)

      for (i = 0; i < nfds; ++i)
         {
         if (getPeer(i))
            {
            if (FD_ISSET(i, paddrmask)) addPeerInfo(0);
            else
               {
#           ifdef HAVE_NETPACKET_PACKET_H
               if ((check_type & U_CHECK_ARP_PING) != 0)
                  {
                  U_SRV_LOG("Peer IP %v MAC %v don't return ARP reply, I assume he is disconnected...", peer->ip.rep, peer->mac.rep);
                  }
#           endif

               deny(U_LOGOUT_NO_ARP_REPLY, check_expire);
               }
            }
         }

      nfds      = 0;
      paddrmask = 0;
      }

   for (i = 0, n = vauth_url->size(); i < n; ++i)
      {
      sendInfoData(i);

#  ifdef USE_LIBTDB
   // sendRoamingData(i);
#  endif
      }

   // check if there are some log file to upload

   if (dirwalk)
      {
      UVector<UString> vec(64);

      n = dirwalk->walk(vec, U_ALPHABETIC_SORT);

      if (n)
         {
         UFile file;

         for (i = 0; i < n; ++i)
            {
            file.setPath(vec[i]);

            uploadFileToPortal(file);
            }
         }
      }

end:
   U_RESET_MODULE_NAME;

   u_errno = errno = EINTR;

   flag_check_system = false;

   last_request_check = u_now->tv_sec;
}

void UNoCatPlugIn::deny(int disconnected, bool bcheck_expire)
{
   U_TRACE(0, "UNoCatPlugIn::deny(%d,%b)", disconnected, bcheck_expire)

   U_INTERNAL_ASSERT_POINTER(peer)

   if (U_peer_status == UModNoCatPeer::PEER_DENY)
      {
      U_SRV_LOG("WARNING: peer already deny: IP %v MAC %v", peer->ip.rep, peer->mac.rep);

      return;
      }

   if (U_peer_allowed)
      {
      U_SRV_LOG("WARNING: I should to deny user allowed: IP %v MAC %v", peer->ip.rep, peer->mac.rep);

      return;
      }

   addPeerInfo(disconnected);

   executeCommand(UModNoCatPeer::PEER_DENY);

   if (total_connections) --total_connections;

   U_SRV_LOG("Peer denied: IP %v MAC %v remain: %ld secs %llu bytes - total_connections %u", peer->ip.rep, peer->mac.rep, peer->time_remain, peer->traffic_remain, total_connections);

   U_INTERNAL_DUMP("check_expire = %u num_peers_preallocate = %u", check_expire, num_peers_preallocate)

   if (bcheck_expire) UTimer::erase(peer);
}

void UNoCatPlugIn::permit(const UString& UserDownloadRate, const UString& UserUploadRate)
{
   U_TRACE(0, "UNoCatPlugIn::permit(%S,%S)", &UserDownloadRate, &UserUploadRate)

   U_INTERNAL_ASSERT_POINTER(peer)
   U_INTERNAL_ASSERT_EQUALS(U_peer_status, UModNoCatPeer::PEER_DENY)

   if (UserDownloadRate) peer->fw.setArgument(7, UserDownloadRate.c_str());
   if (UserUploadRate)   peer->fw.setArgument(8, UserUploadRate.c_str());

   executeCommand(UModNoCatPeer::PEER_PERMIT);

   ++total_connections;

   U_SRV_LOG("Peer permitted: IP %v MAC %v - UserDownloadRate %v UserUploadRate %v remain: %ld secs %llu bytes - total_connections %u",
               peer->ip.rep, peer->mac.rep, UserDownloadRate.rep, UserUploadRate.rep, peer->time_remain, peer->traffic_remain, total_connections);

   if (check_expire)
      {
      peer->UEventTime::setTimeToExpire(peer->time_remain);

      UTimer::insert(peer);
      }
}

UString UNoCatPlugIn::getSignedData(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UNoCatPlugIn::getSignedData(%.*S,%u)", len, ptr, len)

   UString output;

// if (*decrypt_key)
      {
      output = UDES3::getSignedData(ptr, len);
      }
   /*
   else
      {
      UString buffer(U_CAPACITY), input(U_CAPACITY);

      Url::decode(ptr, len, buffer);

      input.snprintf(U_CONSTANT_TO_PARAM("-----BEGIN PGP MESSAGE-----\n\n"
                     "%v"
                     "\n-----END PGP MESSAGE-----"), buffer.rep));

      (void) pgp.execute(&input, &output, -1, fd_stderr);

#  ifndef U_LOG_DISABLE
      UServer_Base::logCommandMsgError(pgp.getCommand(), false);
#  endif
      }
   */

   U_RETURN_STRING(output);
}

bool UNoCatPlugIn::checkAuthMessage(const UString& msg)
{
   U_TRACE(0, "UNoCatPlugIn::checkAuthMessage(%V)", msg.rep)

   U_INTERNAL_ASSERT(msg)
   U_INTERNAL_ASSERT_POINTER(peer)

   uint32_t pos;
   time_t timeout;
   int64_t traffic;
   bool result = false;
   UHashMap<UString> args;
   UString action, redirect, user;

   args.loadFromData(msg);

   /**
    * Action    Permit
    * Mode      Login
    * Redirect  http://wi-auth/postlogin?uid=s.casazza&gateway=10.30.1.131:5280&redirect=http%3A//stefano%3A5280/pippo
    * Mac       00:e0:4c:d4:63:f5
    * Timeout   7200
    * Traffic   314572800
    * Token     10.30.1.105&1237907630&05608a4cbd42c9f72d2bd3a0e19ed23f
    * User      s.casazza
    * Policy    FLAT
    * NoTraffic 6
    *
    * UserDownloadRate 10240
    * UserUploadRate    1024
    */

   // check mac address

   if (peer->mac != args.at(U_CONSTANT_TO_PARAM("Mac")))
      {
      U_SRV_LOG("WARNING: different MAC in ticket from peer: IP %v MAC %v", peer->ip.rep, peer->mac.rep);

      goto end;
      }

   // check token

   if (peer->token != args.at(U_CONSTANT_TO_PARAM("Token")))
      {
      U_SRV_LOG("WARNING: tampered token from peer: IP %v MAC %v", peer->ip.rep, peer->mac.rep);

      peer->token = UStringExt::numberToString(u_random(u_now->tv_usec));

      goto end;
      }

   /**
    * get mode
    *
    * mode = args.at(U_CONSTANT_TO_PARAM("Mode")).copy();
    *
    * U_INTERNAL_DUMP("mode = %V", mode.rep))
    */

   // check user id

   user = args.at(U_CONSTANT_TO_PARAM("User"));

   pos = vLoginValidate->find(user);

   if (pos != U_NOT_FOUND)
      {
      U_SRV_LOG("Validation of user id %V in ticket from peer: IP %v MAC %v", user.rep, peer->ip.rep, peer->mac.rep);

      vLoginValidate->erase(pos);
      }

   peer->user = user.copy();

   // get redirect (destination)

   redirect = args.at(U_CONSTANT_TO_PARAM("Redirect"));

   if (redirect)
      {
      Url destination(redirect);
      UVector<UString> name_value;

      if (destination.getQuery(name_value) == 0)
         {
         U_SRV_LOG("WARNING: can't make sense of Redirect: %V in ticket from peer: IP %v MAC %v", U_URL_TO_TRACE(destination), peer->ip.rep, peer->mac.rep);

         goto end;
         }

      destination.eraseQuery();

      if (destination.setQuery(name_value) == false)
         {
         U_SRV_LOG("WARNING: error on setting Redirect: %V in ticket from peer: IP %v MAC %v", U_URL_TO_TRACE(destination), peer->ip.rep, peer->mac.rep);

         goto end;
         }

      if (destination.get() != redirect)
         {
         U_SRV_LOG("WARNING: error on check Redirect: %V in ticket from peer: IP %v MAC %v", U_URL_TO_TRACE(destination), peer->ip.rep, peer->mac.rep);

         goto end;
         }

      (void) location->replace(redirect);
      }

   // check for user policy FLAT

   if (args.at(U_CONSTANT_TO_PARAM("Policy")).equal(U_CONSTANT_TO_PARAM("FLAT"))) U_peer_policy_flat = true;

   // check for max time no traffic

   U_peer_max_time_no_traffic = args.at(U_CONSTANT_TO_PARAM("NoTraffic")).strtoul();

   if (U_peer_max_time_no_traffic == 0) U_peer_max_time_no_traffic = (check_expire / 60) * (U_peer_policy_flat ? 3 : 1);

   // get time available

   timeout = args.at(U_CONSTANT_TO_PARAM("Timeout")).strtoul();

   if (timeout ||
       peer->time_remain == 0)
      {
      if (timeout) peer->time_remain = timeout;
      else         peer->time_remain = (U_peer_policy_flat ? 24L : 2L) * 3600L; // default value...
      }

   U_INTERNAL_DUMP("time_remain = %ld", peer->time_remain)

   // get traffic available

   traffic = args.at(U_CONSTANT_TO_PARAM("Traffic")).strtoull();

   if (traffic > 0 ||
       peer->traffic_available == 0)
      {
      if (traffic > 0) peer->traffic_available = traffic;
      else             peer->traffic_available = (U_peer_policy_flat ? 4 * 1024ULL : 300ULL) * (1024ULL * 1024ULL); // default value...
      }

   U_INTERNAL_DUMP("traffic_available = %llu", peer->traffic_available)

   // get action

   action = args.at(U_CONSTANT_TO_PARAM("Action"));

   if (action.empty() ||
       action.equal(U_CONSTANT_TO_PARAM("Permit")))
      {
      result = true;

      if (U_peer_status == UModNoCatPeer::PEER_PERMIT)
         {
         U_SRV_LOG("WARNING: peer already permit: IP %v MAC %v", peer->ip.rep, peer->mac.rep);
         }
      else
         {
         // set time

         peer->ctraffic = peer->time_no_traffic = peer->logout = 0L;
         peer->expire   = (peer->ctime = peer->connected = u_now->tv_sec) + peer->time_remain;

         // set traffic

         peer->traffic_done   = 0ULL;
         peer->traffic_remain = peer->traffic_available;

         UString userDownloadRate = args.at(U_CONSTANT_TO_PARAM("UserDownloadRate")),
                 userUploadRate   = args.at(U_CONSTANT_TO_PARAM("UserUploadRate"));

         permit(userDownloadRate, userUploadRate);
         }
      }
   else
      {
      U_SRV_LOG("WARNING: can't make sense of Action: %V in ticket from peer: IP %v MAC %v", action.rep, peer->ip.rep, peer->mac.rep);
      }

end:
   args.clear();

   U_RETURN(result);
}

void UNoCatPlugIn::creatNewPeer()
{
   U_TRACE_NO_PARAM(0, "UNoCatPlugIn::creatNewPeer()")

   U_INTERNAL_DUMP("peer = %p peers_delete = %p idx_peers_preallocate = %u num_peers_preallocate = %u", peer, peers_delete, idx_peers_preallocate, num_peers_preallocate)

   U_INTERNAL_ASSERT_EQUALS(peer, 0)
   U_INTERNAL_ASSERT_POINTER(peers_preallocate)

   if ((idx_peers_preallocate+1) < num_peers_preallocate) peer = &(peers_preallocate[idx_peers_preallocate++]);
   else
      {
      UString msg(200U);

      msg.snprintf(U_CONSTANT_TO_PARAM("/error_ap?ap=%v@%v&public=%v%%3A%u&msg=%u"), label->rep, hostname->rep, IP_address_trust->rep, UServer_Base::port, num_peers_preallocate);

      sendMsgToPortal(0, msg, 0);

      UInterrupt::setHandlerForSegv(preallocatePeers, preallocatePeersFault);

      peer = &(peers_preallocate[0]);

      idx_peers_preallocate = 1;
      }

   U_INTERNAL_ASSERT_POINTER(peer)
}

bool UNoCatPlugIn::creatNewPeer(uint32_t index_AUTH)
{
   U_TRACE(0, "UNoCatPlugIn::creatNewPeer(%u)", index_AUTH)

   U_INTERNAL_ASSERT_EQUALS(peer, 0)

   uint32_t index_network = UIPAllow::find(UServer_Base::client_address, *vLocalNetworkMask);

   U_INTERNAL_DUMP("index_network = %u", index_network)

   if (index_network == U_NOT_FOUND)
      {
      U_SRV_LOG("WARNING: IP address for new peer %.*S not found in LocalNetworkMask %V", U_CLIENT_ADDRESS_TO_TRACE, localnet->rep);

      U_RETURN(false);
      }

   creatNewPeer();

   U_peer_allowed       = false;
   U_peer_index_AUTH    = index_AUTH;
   U_peer_index_network = index_network;

   peer->ifname        = (*vInternalDevice)[(index_network < num_radio ? index_network : 0)];
   U_peer_index_device =   vInternalDevice->find(peer->ifname);

   *((UIPAddress*)peer) = UServer_Base::csocket->remoteIPAddress();

   (void) peer->ip.assign(peer->UIPAddress::pcStrAddress);

   U_ASSERT(peer->ip.equal(U_CLIENT_ADDRESS_TO_PARAM))

   (void) peer->user.assign(U_CONSTANT_TO_PARAM("anonymous"));

   setNewPeer();

   U_RETURN(true);
}

bool UNoCatPlugIn::setPeerLabel()
{
   U_TRACE_NO_PARAM(0, "UNoCatPlugIn::setPeerLabel()")

#ifdef USE_LIBTDB
   typedef struct { uint8_t hwaddr[6], ip[4]; } __attribute__((packed)) tdbkey_t;

   tdbkey_t ks;
   uint32_t t1, t2, t3, t4, t5, t6;

   if (U_SYSCALL(sscanf, "%p,%S,%p,%p,%p,%p", peer->ip.data(), "%u.%u.%u.%u", &t1, &t2, &t3, &t4) == 4)
      {
      ks.ip[0] = t4;
      ks.ip[1] = t3;
      ks.ip[2] = t2;
      ks.ip[3] = t1;

      if (U_SYSCALL(sscanf, "%p,%S,%p,%p,%p,%p,%p,%p", peer->mac.data(), "%x:%x:%x:%x:%x:%x", &t1, &t2, &t3, &t4, &t5, &t6) == 6)
         {
         ks.hwaddr[0] = t1;
         ks.hwaddr[1] = t2;
         ks.hwaddr[2] = t3;
         ks.hwaddr[3] = t4;
         ks.hwaddr[4] = t5;
         ks.hwaddr[5] = t6;

         UString value = ((UTDB*)pdata)->at((const char*)&ks, 10);

         if (value)
            {
         // typedef struct { uint32_t area, leasetime; time_t expiration; } __attribute__((packed)) tdbdata;

            UString area(10U);
            uint32_t id = *(uint32_t*)value.data();

            if (id == 16777215) (void) area.assign(U_CONSTANT_TO_PARAM("ffffff"));
            else                       area.setFromNumber32(id);

            U_SRV_LOG("get data from DHCP_DATA_FILE - key: %#.*S data: %V", 10, &ks, area.rep);

            if (peer->label != area)
               {
               peer->label = area;

               U_RETURN(true);
               }
            }
         }
      }
#endif

   U_INTERNAL_DUMP("peer->label = %V", peer->label.rep)

   U_RETURN(false);
}

void UNoCatPlugIn::setNewPeer()
{
   U_TRACE_NO_PARAM(0, "UNoCatPlugIn::setNewPeer()")

   U_INTERNAL_ASSERT_POINTER(peer)

   U_INTERNAL_DUMP("peer->UIPAddress::pcStrAddress = %S U_peer_index_network = %u U_peer_index_AUTH = %u",
                    peer->UIPAddress::pcStrAddress,     U_peer_index_network,     U_peer_index_AUTH)

   if (U_peer_index_network == 0xFF)
      {
      U_ERROR("IP address for new peer %V not found in LocalNetworkMask %V", peer->ip.rep, localnet->rep);
      }

   if ((check_type & U_CHECK_MAC) != 0 && // not unifi (L2)
       peer->mac == *UString::str_without_mac)
      {
      U_INTERNAL_ASSERT(peer->ifname.isNullTerminated())

      UString mac = UServer_Base::csocket->getMacAddress(peer->ifname.data());

      if (mac) peer->mac = mac;
      else
         {
         (void) USocketExt::getARPCache(*arp_cache, *varp_cache);

         for (uint32_t i = 0, n = varp_cache->size(); i < n; i += 3)
            {
            if ((*varp_cache)[i].equal(peer->ip))
               {
               peer->mac = (*varp_cache)[i+1].copy();

               U_ASSERT_EQUALS(peer->ifname, (*varp_cache)[i+2])

               break;
               }
            }
         }

      U_INTERNAL_ASSERT(peer->mac)
      U_ASSERT_EQUALS(peer->mac, USocketExt::getMacAddress(peer->ip))
      }

   U_INTERNAL_DUMP("peer->mac", peer->mac.rep)

   UIPAllow* pallow = vLocalNetworkMask->at(U_peer_index_network);

   UString tmp(32), x = (pallow->isEmpty() == false || UIPAllow::getNetworkInterface(*vLocalNetworkMask) ? pallow->host : *UServer_Base::IP_address);

   tmp.snprintf(U_CONSTANT_TO_PARAM("%v:%d"), x.rep, UServer_Base::port);

   peer->gateway = tmp;

   U_INTERNAL_DUMP("peer->gateway = %V", peer->gateway.rep)

   if (pallow->device &&
       peer->ifname != pallow->device)
      {
      U_SRV_LOG("WARNING: device different (%v=>%v) for peer: IP %v MAC %v", peer->ifname.rep, pallow->device.rep, peer->ip.rep, peer->mac.rep);

      peer->ifname = pallow->device;

      U_peer_index_device = vInternalDevice->find(peer->ifname);
      }

   U_INTERNAL_DUMP("peer->ifname = %V U_peer_index_device = %u", peer->ifname.rep, U_peer_index_device)

   peer->label = ((uint32_t)U_peer_index_network >= vLocalNetworkLabel->size() ? *UString::str_without_label : (*vLocalNetworkLabel)[U_peer_index_network]);

#ifdef USE_LIBTDB
   if (pdata) (void) setPeerLabel();
#endif

   // NB: request(arp|deny|clear|reset|permit|openlist|initialize) mac ip class(Owner|Member|Public) UserDownloadRate UserUploadRate

   setFireWallCommand(peer->fw, *fw_cmd, peer->mac, peer->ip);

   peer->token = UStringExt::numberToString(u_random(u_now->tv_usec));

   peers->insert(peer->ip, peer);
}

void UNoCatPlugIn::checkOldPeer()
{
   U_TRACE_NO_PARAM(0, "UNoCatPlugIn::checkOldPeer()")

   U_INTERNAL_ASSERT_POINTER(peer)
   U_ASSERT(peer->ip.equal(U_CLIENT_ADDRESS_TO_PARAM))

   UString mac = peer->mac;

   if ((check_type & U_CHECK_MAC) != 0) // not unifi (L2)
      {
      mac = UServer_Base::csocket->getMacAddress(peer->ifname.data());

      if (mac.empty()) return;

      U_ASSERT_EQUALS(mac, USocketExt::getMacAddress(peer->ip))
      }

   if (mac != peer->mac)
      {
      // NB: we assume that the current peer is a different user that has acquired the same IP address from the DHCP...

      U_SRV_LOG("WARNING: different MAC %v for peer: IP %v MAC %v", mac.rep, peer->ip.rep, peer->mac.rep);

      U_INTERNAL_ASSERT(mac)
      U_INTERNAL_ASSERT(peer->mac)

      deny(U_LOGOUT_DIFFERENT_MAC_FOR_IP, check_expire);

      peer->mac = mac.copy();

      peer->fw.setArgument(4, peer->mac.c_str());

      (void) peer->user.assign(U_CONSTANT_TO_PARAM("anonymous"));

      /**
       * NB: tutto il traffico viene rediretto sulla 80 (CAPTIVE PORTAL) e quindi windows update, antivrus, etc...
       * questo introduce la possibilita' che durante la fase di autorizzazione il token generato per il ticket autorizzativo
       * non corrisponda piu' a quello inviato dal portale per l'autorizzazione...
       */

      peer->token = UStringExt::numberToString(u_random(u_now->tv_usec));
      }

#ifdef USE_LIBTDB
   if (pdata) (void) setPeerLabel();
#endif
}

void UNoCatPlugIn::addPeerInfo(int disconnected)
{
   U_TRACE(0, "UNoCatPlugIn::addPeerInfo(%d)", disconnected)

   U_INTERNAL_ASSERT_POINTER(peer)
   U_INTERNAL_ASSERT(peer->mac)
   U_INTERNAL_ASSERT(peer->user)
   U_INTERNAL_ASSERT(u_now->tv_sec >= peer->ctime)

   char* ptr;
   char buffer[64];
   UString info = (*vinfo_data)[U_peer_index_AUTH], str = UStringExt::substitute(peer->mac, ':', U_CONSTANT_TO_PARAM("%3A"));

   uint32_t sz = info.size();

   U_INTERNAL_DUMP("U_peer_index_AUTH = %u info = %V peer->ip = %V", U_peer_index_AUTH, info.rep, peer->ip.rep)

   // ------------------------------------------------------------------------------------------------------------------------------------------
   // $1 -> mac
   // $2 -> ip
   // $3 -> gateway
   // $4 -> ap (with localization => '@')
   // $5 -> => UID <=
   // $6 -> logout
   // $7 -> connected
   // $8 -> traffic
   // ------------------------------------------------------------------------------------------------------------------------------------------
   // /info?Mac=98%3A0c%3A82%3A76%3A3b%3A39&ip=172.16.1.8&gateway=172.16.1.254%3A5280&ap=ap%4010.8.0.1&User=1212&logout=-1&connected=3&traffic=0
   // ------------------------------------------------------------------------------------------------------------------------------------------

   (void) info.reserve(200U);

   info.snprintf_add(U_CONSTANT_TO_PARAM("%.*sMac=%v&ip=%v&"), (sz > 0), "&", str.rep, peer->ip.rep);

   info.snprintf_add(U_CONSTANT_TO_PARAM("gateway=%.*s&ap=%v%%40%v&User="),
                      u_url_encode((const unsigned char*)U_STRING_TO_PARAM(peer->gateway), (unsigned char*)buffer), buffer, peer->label.rep, IP_address_trust->rep);

   info.snprintf_add(U_CONSTANT_TO_PARAM("%.*s&logout="), u_url_encode((const unsigned char*)U_STRING_TO_PARAM(peer->user), (unsigned char*)buffer), buffer);

   ptr = (char*) info.pend();

   // NB: disconnected 0 => mean NOT logout (only info)
   //                 <0 => disconnected (logout implicito)

   if (disconnected == false) *ptr++ = '0'; // 0 => mean NOT logout (only info)
   else
      {
      *ptr++ = '-';
      *ptr++ = '0' + disconnected;

      if (peer->logout == 0) peer->logout = u_now->tv_sec;
#  ifdef DEBUG
      else // NB: user with no more time or no more traffic...
         {
         U_INTERNAL_ASSERT_EQUALS(peer->logout, peer->expire)
         }
#  endif
      }

   U_MEMCPY(ptr,"&connected=", U_CONSTANT_SIZE("&connected="));
            ptr +=             U_CONSTANT_SIZE("&connected=");

   ptr = u_num2str32(u_now->tv_sec - peer->ctime, ptr);
                                     peer->ctime = u_now->tv_sec;

   U_MEMCPY(ptr, "&traffic=", U_CONSTANT_SIZE("&traffic="));
            ptr +=            U_CONSTANT_SIZE("&traffic=");

   ptr = u_num2str32(peer->ctraffic, ptr);
                     peer->ctraffic = 0;

   info.size_adjust_force(ptr);

   U_INTERNAL_DUMP("info(%u) = %V", info.size(), info.rep)

   (void) vinfo_data->replace(U_peer_index_AUTH, info);
}

void UNoCatPlugIn::addPeerRoaming()
{
   U_TRACE(0, "UNoCatPlugIn::addPeerRoaming()")

#ifdef USE_LIBTDB
   U_INTERNAL_ASSERT_POINTER(peer)
   U_INTERNAL_ASSERT(peer->mac)
   U_INTERNAL_ASSERT(peer->user)

   char buffer[64];
   UString data = (*vroaming_data)[U_peer_index_AUTH], str = UStringExt::substitute(peer->mac, ':', U_CONSTANT_TO_PARAM("%3A"));

   uint32_t sz = data.size();

   U_INTERNAL_DUMP("U_peer_index_AUTH = %u data = %V peer->ip = %V", U_peer_index_AUTH, data.rep, peer->ip.rep)

   // -------------------------------------------------------------------------------------------------------------
   // $1 -> mac
   // $2 -> ip
   // $3 -> gateway
   // $4 -> ap (with localization => '@')
   // $5 -> => UID <=
   // -------------------------------------------------------------------------------------------------------------
   // /roaming?Mac=98%3A0c%3A82%3A76%3A3b%3A39&ip=172.16.1.8&gateway=172.16.1.254%3A5280&ap=ap%4010.8.0.1&User=1212
   // -------------------------------------------------------------------------------------------------------------

   (void) data.reserve(200U);

   data.snprintf_add(U_CONSTANT_TO_PARAM("%.*sMac=%v&ip=%v&"), (sz > 0), "&", str.rep, peer->ip.rep);

   data.snprintf_add(U_CONSTANT_TO_PARAM("gateway=%.*s&ap=%v%%40%v&User="),
                      u_url_encode((const unsigned char*)U_STRING_TO_PARAM(peer->gateway), (unsigned char*)buffer), buffer, peer->label.rep, IP_address_trust->rep);

   data.snprintf_add(U_CONSTANT_TO_PARAM("%.*s"), u_url_encode((const unsigned char*)U_STRING_TO_PARAM(peer->user), (unsigned char*)buffer), buffer);

   U_INTERNAL_DUMP("data(%u) = %V", data.size(), data.rep)

   (void) vroaming_data->replace(U_peer_index_AUTH, data);
#endif
}

int UModNoCatPeer::checkPeerInfo(bool btraffic)
{
   U_TRACE(0, "UModNoCatPeer::checkPeerInfo(%b)", btraffic)

   U_INTERNAL_ASSERT_POINTER(UNoCatPlugIn::peer)

   if (U_peer_allowed == false &&
       U_peer_status  == PEER_PERMIT)
      {
      if (btraffic) UNoCatPlugIn::getTraffic();

      // NB: we get the device for peer IP address from the ARP cache...

      UNoCatPlugIn::peer_present_in_arp_cache->clear();

      for (uint32_t i = 0, n = UNoCatPlugIn::varp_cache->size(); i < n; i += 3)
         {
         if ((*UNoCatPlugIn::varp_cache)[i].equal(ip) &&
             (*UNoCatPlugIn::varp_cache)[i+1] != *UString::str_without_mac)
            {
            *UNoCatPlugIn::peer_present_in_arp_cache = (*UNoCatPlugIn::varp_cache)[i+2].copy(); // dev

            U_ASSERT_EQUALS(*UNoCatPlugIn::peer_present_in_arp_cache, USocketExt::getNetworkInterfaceName(ip))

            break;
            }
         }

      U_INTERNAL_DUMP("UNoCatPlugIn::peer_present_in_arp_cache = %V peer->ifname = %V", UNoCatPlugIn::peer_present_in_arp_cache->rep, ifname.rep)

      if ( UNoCatPlugIn::peer_present_in_arp_cache->empty() &&
          (UNoCatPlugIn::check_type & UNoCatPlugIn::U_CHECK_ARP_CACHE) != 0)
         {
         U_SRV_LOG("Peer IP %v MAC %v not present in ARP cache, I assume he is disconnected", ip.rep, mac.rep);

         U_RETURN(U_LOGOUT_NO_ARP_CACHE);
         }

      if (ctraffic) time_no_traffic  = 0;
      else          time_no_traffic += (u_now->tv_sec - ctime);

      U_INTERNAL_DUMP("time_no_traffic = %ld", time_no_traffic)

      if (time_no_traffic)
         {
         long max_time_no_traffic = U_peer_max_time_no_traffic * 60L;

         U_SRV_LOG("Peer IP %v MAC %v has made no traffic for %ld secs - (max_time_no_traffic: %ld secs) (present in ARP cache: %s)",
                     ip.rep, mac.rep, time_no_traffic, max_time_no_traffic, UNoCatPlugIn::peer_present_in_arp_cache->empty() ? "NO" : "yes");

         if (time_no_traffic >= max_time_no_traffic)
            {
            U_INTERNAL_ASSERT(u_now->tv_sec >= ctime)

            U_RETURN(U_LOGOUT_NO_TRAFFIC);
            }
         }

      time_remain = (expire > u_now->tv_sec ? (expire - u_now->tv_sec) : 0);

      if (U_peer_policy_flat == false)
         {
         if (time_remain <= 15)
            {
            logout      = expire; // NB: user with no more time or no more traffic...
            time_remain = 0;

            U_SRV_LOG("Peer IP %v MAC %v has no more time - remain traffic for %llu bytes", ip.rep, mac.rep, traffic_remain);

            U_RETURN(U_LOGOUT_NO_MORE_TIME);
            }

         if (traffic_remain <= 1024ULL)
            {
            logout         = expire; // NB: user with no more time or no more traffic...
            traffic_remain = 0;

            U_SRV_LOG("Peer IP %v MAC %v has no more traffic - remain time for %lu sec", ip.rep, mac.rep, time_remain);

            U_RETURN(U_LOGOUT_NO_MORE_TRAFFIC);
            }
         }
      }

   U_RETURN(0);
}

bool UNoCatPlugIn::checkPeerInfo(UStringRep* key, void* value)
{
   U_TRACE(0, "UNoCatPlugIn::checkPeerInfo(%V,%p)", key, value)

   peer = (UModNoCatPeer*) value;

   int disconnected = peer->checkPeerInfo(false);

   if (disconnected)
      {
      deny(disconnected, check_expire);

      U_RETURN(true);
      }

   if (U_peer_status == UModNoCatPeer::PEER_PERMIT)
      {
      if (U_peer_allowed) U_RETURN(true);

      U_SRV_LOG("Peer IP %v MAC %v remain: %ld secs %llu bytes", peer->ip.rep, peer->mac.rep, peer->time_remain, peer->traffic_remain);

      uint32_t index_device = U_peer_index_device;

      if ((check_type & U_CHECK_ARP_CACHE) != 0)
         {
         U_INTERNAL_ASSERT(*peer_present_in_arp_cache)

         index_device = vInternalDevice->find(*peer_present_in_arp_cache);
         }

      U_INTERNAL_DUMP("index_device = %u U_peer_index_device = %u num_radio = %u", index_device, U_peer_index_device, num_radio)

      U_INTERNAL_ASSERT(index_device <= num_radio)

      if (index_device != U_NOT_FOUND)
         {
         FD_SET(nfds, &addrmask);

         ++nfds;

         vaddr[index_device]->push(peer);
         }

#  ifdef USE_LIBTDB
      if (pdata &&
          setPeerLabel())
         {
      // addPeerRoaming();
         }
#  endif
      }

   U_RETURN(true);
}

__pure bool UNoCatPlugIn::getPeer(uint32_t n)
{
   U_TRACE(0, "UNoCatPlugIn::getPeer(%u)", n)

   U_INTERNAL_ASSERT_MAJOR(nfds,0)

   int32_t index = -1;

   for (uint32_t k, i = 0; i < num_radio; ++i)
      {
      k = vaddr[i]->size();

      for (uint32_t j = 0; j < k; ++j)
         {
         if (++index == (int32_t)n)
            {
            peer = (UModNoCatPeer*) vaddr[i]->at(j);

            U_INTERNAL_DUMP("peer = %p", peer)

            U_RETURN(true);
            }
         }
      }

   U_RETURN(false);
}

void UNoCatPlugIn::sendData(uint32_t index_AUTH, const UString& data, const char* service, uint32_t service_len)
{
   U_TRACE(0, "UNoCatPlugIn::sendData(%u,%V,%.*S,%u)", index_AUTH, data.rep, service_len, service, service_len)

   U_INTERNAL_ASSERT(data)
   U_INTERNAL_ASSERT_EQUALS(UClient_Base::queue_dir, 0)

   // NB: we can't try to send immediately the info data on users to portal because the worst we have a hole
   //     of 10 seconds and the portal can have need to ask us something (to logout some user, the list of peer permitted, ...)

   char buffer[4096];
   const char* log_msg = 0;
   uint32_t sz = data.size();
   UString body = data, url = getUrlForSendMsgToPortal(index_AUTH, service, service_len);

#ifdef USE_LIBZ
   if (sz > U_MIN_SIZE_FOR_DEFLATE) body = UStringExt::deflate(data, 1);
#endif

#ifndef U_LOG_DISABLE
   if (UServer_Base::isLog())
      {
      UString str = UStringExt::substitute(data.data(), U_min(sz,200), '%', U_CONSTANT_TO_PARAM("%%")); // NB: we need this because we have a message with url encoded char...

      (void) u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("[nocat] Sent %.*s %%s (%u bytes) after %%d attempts to AUTH(%u): %V"), service_len, service, sz, index_AUTH, str.rep);

      log_msg = buffer;
      }
#endif

   (void) client->sendPOSTRequestAsync(body, url, true, log_msg);
}

void UNoCatPlugIn::sendInfoData(uint32_t index_AUTH)
{
   U_TRACE(0, "UNoCatPlugIn::sendInfoData(%u)", index_AUTH)

   UString info = (*vinfo_data)[index_AUTH];

   if (info)
      {
      sendData(index_AUTH, info, U_CONSTANT_TO_PARAM("/info"));

      vinfo_data->getStringRep(index_AUTH)->size_adjust_force(0U);
      }
}

void UNoCatPlugIn::sendRoamingData(uint32_t index_AUTH)
{
   U_TRACE(0, "UNoCatPlugIn::sendRoamingData(%u)", index_AUTH)

#ifdef USE_LIBTDB
   UString data = (*vroaming_data)[index_AUTH];

   if (data)
      {
      sendData(index_AUTH, data, U_CONSTANT_TO_PARAM("/roaming"));

      vroaming_data->getStringRep(index_AUTH)->size_adjust_force(0U);
      }
#endif
}

void UNoCatPlugIn::notifyAuthOfUsersInfo(uint32_t index_AUTH)
{
   U_TRACE(0, "UNoCatPlugIn::notifyAuthOfUsersInfo(%u)", index_AUTH)

   sendInfoData(index_AUTH);

   // NB: if there is arping pending AUTH must recall us after ~15sec for completion...

   U_http_info.nResponseCode = (isPingAsyncPending() ? HTTP_NO_CONTENT : HTTP_NOT_MODIFIED);

   UClientImage_Base::setCloseConnection();

   UHTTP::setResponse();
}

void UNoCatPlugIn::setHTTPResponse(const UString& content, int mime_index)
{
   U_TRACE(0, "UNoCatPlugIn::setHTTPResponse(%V%d)", content.rep, mime_index)

   if (content.empty())
      {
      U_http_info.nResponseCode = HTTP_NO_CONTENT;

      UHTTP::setResponse();
      }
   else
      {
      U_http_info.endHeader       = 0;
      U_http_info.nResponseCode   = HTTP_OK;
      *UClientImage_Base::wbuffer = content;

      UHTTP::setDynamicResponse();
      }
}

bool UNoCatPlugIn::getPeerFromMAC(const UString& mac)
{
   U_TRACE(0, "UNoCatPlugIn::getPeerFromMAC(%V)", mac.rep)

   if (peers->first())
      {
      do {
         peer = peers->elem();

         if (mac == peer->mac)
            {
            U_INTERNAL_DUMP("peer = %p", peer)

            U_RETURN(true);
            }
         }
      while (peers->next());
      }

   peer = 0;

   U_RETURN(false);
}

UString UNoCatPlugIn::getIPAddress(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UNoCatPlugIn::getIPAddress(%.*S,%u)", len, ptr, len)

   UString x(U_INET_ADDRSTRLEN);

   for (uint32_t i = 0; i < len; ++i)
      {
      char c = ptr[i];

      if (u__isipv4(c) == false) break;

      x._append(c);
      }

   x._append();

   U_RETURN_STRING(x);
}

__pure uint32_t UNoCatPlugIn::getIndexAUTH(const char* ip_address)
{
   U_TRACE(0, "UNoCatPlugIn::getIndexAUTH(%S)", ip_address)

   uint32_t index_AUTH, sz_AUTH = vauth_ip->size();

   if (sz_AUTH == 1) index_AUTH = 0;
   else
      {
      // NB: we are multi portal, we must find which portal we indicate to redirect the client...

      index_AUTH = UIPAllow::find(ip_address, *vLocalNetworkMask);

      if (index_AUTH >= sz_AUTH) index_AUTH = 0;
      }

   U_RETURN(index_AUTH);
}

// Server-wide hooks

int UNoCatPlugIn::handlerConfig(UFileConfig& cfg)
{
   U_TRACE(0, "UNoCatPlugIn::handlerConfig(%p)", &cfg)

   // nocat - plugin parameters
   // -----------------------------------------------------------------------------------------------------------------------------------
   // FW_ENV                environment for shell script to execute
   // FW_CMD                shell script to manage the firewall
   // DECRYPT_KEY           DES3 password stuff
   // ALLOWED_MEMBERS       file with list of allowed MAC/IP pairs or NETWORKS (default: /etc/nodog.allowed)
   // LOCAL_NETWORK_LABEL   access point localization tag to be used from portal
   // LOGIN_TIMEOUT         Number of seconds after a client last login/renewal to terminate their connection
   // CHECK_TYPE            mode of verification (U_CHECK_ARP_CACHE=1, U_CHECK_ARP_PING=2, U_CHECK_MAC=4, U_CHECK_FIREWALL=8)
   // CHECK_EXPIRE_INTERVAL Number of seconds to check if some client has terminate their connection then send info to portal
   // NUM_PEERS_PREALLOCATE Size of memory block to preallocate for table of users
   // -----------------------------------------------------------------------------------------------------------------------------------

   if (cfg.loadTable())
      {
      *fw_env = cfg.at(U_CONSTANT_TO_PARAM("FW_ENV"));

      if (fw_env->empty()) U_RETURN(U_PLUGIN_HANDLER_ERROR);

      int port = 0;
      UString lnetlbl, tmp;

      *fw_cmd      = cfg.at(U_CONSTANT_TO_PARAM("FW_CMD"));
      lnetlbl      = cfg.at(U_CONSTANT_TO_PARAM("LOCAL_NETWORK_LABEL"));
   // *decrypt_cmd = cfg.at(U_CONSTANT_TO_PARAM("DECRYPT_CMD"));
      *decrypt_key = cfg.at(U_CONSTANT_TO_PARAM("DECRYPT_KEY"));

      check_type   = cfg.readLong(U_CONSTANT_TO_PARAM("CHECK_TYPE"));
      check_expire = cfg.readLong(U_CONSTANT_TO_PARAM("CHECK_EXPIRE_INTERVAL"));

      tmp = cfg.at(U_CONSTANT_TO_PARAM("NUM_PEERS_PREALLOCATE"));

      num_peers_preallocate = (tmp ? tmp.strtoul() : 512);

      tmp = cfg.at(U_CONSTANT_TO_PARAM("LOGIN_TIMEOUT"));

      if (tmp)
         {
         char* ptr;

                               time_available = ::strtol(tmp.data(), &ptr, 10);
         if (ptr[0] == ':') traffic_available = ::strtoll(ptr+1,         0, 10);

         if (   time_available > U_ONE_DAY_IN_SECOND) time_available = U_ONE_DAY_IN_SECOND;
         if (traffic_available == 0)               traffic_available = 4ULL * 1024ULL * 1024ULL * 1024ULL; // 4G

         U_INTERNAL_DUMP("check_expire = %u time_available = %u traffic_available = %llu", check_expire, time_available, traffic_available)

         if (check_expire == 0 ||
             check_expire > time_available) 
            {
            check_expire = time_available;
            }
         }

      if (check_expire) UEventTime::setTimeToExpire(check_expire);

      U_INTERNAL_DUMP("check_expire = %u time_available = %u check_type = %B", check_expire, time_available, check_type)

      *fw_env = UStringExt::prepareForEnvironmentVar(*fw_env);

      tmp = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("InternalDevice"), fw_env);

      if (tmp) *intdev = tmp;

      tmp = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("ExternalDevice"), fw_env);

      if (tmp) *extdev = tmp;

      tmp = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("LocalNetwork"), fw_env);

      if (tmp) *localnet = tmp;

      tmp = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("AuthServiceAddr"), fw_env);

      if (tmp) *auth_login = tmp;

      tmp = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("GatewayPort"), fw_env);

      if (tmp) port = tmp.strtoul();

      (void) vauth->split(U_STRING_TO_PARAM(*auth_login));

      if (extdev->empty())
         {
         *extdev = UServer_Base::getNetworkDevice(0);

         if (extdev->empty()) U_ERROR("No ExternalDevice detected");

         U_SRV_LOG("Autodetected ExternalDevice %S", extdev->data());
         }

      if (intdev->empty())
         {
         *intdev = UServer_Base::getNetworkDevice(extdev->data());

         if (intdev->empty()) U_ERROR("No InternalDevice detected");

         U_SRV_LOG("Autodetected InternalDevice %S", intdev->data());
         }

      num_radio = vInternalDevice->split(U_STRING_TO_PARAM(*intdev));

      U_INTERNAL_DUMP("num_radio = %u", num_radio)

      if (localnet->empty())
         {
         *localnet = UServer_Base::getNetworkAddress(intdev->data());

         if (localnet->empty()) U_ERROR("No LocalNetwork detected");

         U_SRV_LOG("Autodetected LocalNetwork %S", localnet->data());
         }

      (void) UIPAllow::parseMask(*localnet, *vLocalNetworkMask);

      (void) vLocalNetworkLabel->split(U_STRING_TO_PARAM(lnetlbl));

      U_ASSERT_EQUALS(vLocalNetworkLabel->size(), vLocalNetworkMask->size())

      U_INTERNAL_DUMP("port = %d", port)

      UServer_Base::port = (port ? port : 5280);

      *allowed_members = cfg.at(U_CONSTANT_TO_PARAM("ALLOWED_MEMBERS"));
      *allowed_members = UFile::contentOf(allowed_members->empty() ? *UString::str_allowed_members_default : *allowed_members);

#  ifdef USE_LIBTDB
      tmp = cfg.at(U_CONSTANT_TO_PARAM("DHCP_DATA_FILE"));

      if (tmp)
         {
         U_NEW(UTDB, pdata, UTDB);

         U_INTERNAL_ASSERT(tmp.isNullTerminated())

         if (((UTDB*)pdata)->open(tmp.data()))
            {
            U_SRV_LOG("open DHCP_DATA_FILE %V", tmp.rep);
            }
         else
            {
            U_SRV_LOG("WARNING: fail to open DHCP_DATA_FILE %V", tmp.rep);

            delete (UTDB*)pdata;
                          pdata = 0;
            }
         }
#  endif
      }

   *label = (vLocalNetworkLabel->empty() ? *UString::str_without_label : (*vLocalNetworkLabel)[0]);

   U_INTERNAL_DUMP("label = %V", label->rep)

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

int UNoCatPlugIn::handlerInit()
{
   U_TRACE_NO_PARAM(0, "UNoCatPlugIn::handlerInit()")

   if (fw_cmd->empty()) U_RETURN(U_PLUGIN_HANDLER_ERROR);

// U_PRINT_SIZEOF(UModNoCatPeer);

   Url* url;
   UIPAddress addr;
   UString auth_ip, ip, info(U_CAPACITY), data(U_CAPACITY);

   // NB: get IP address of AUTH hosts...

   for (uint32_t i = 0, n = vauth->size(); i < n; ++i)
      {
      ip = (*vauth)[i];

      U_NEW(Url, url, Url(ip));

      vauth_url->push(url);

      auth_ip = url->getHost();

      if (addr.setHostName(auth_ip.copy(), UClientImage_Base::bIPv6) == false)
         {
         U_SRV_LOG("WARNING: unknown AUTH host %V", auth_ip.rep);
         }
      else
         {
         (void) auth_ip.replace(addr.getAddressString());

         U_SRV_LOG("AUTH host registered: %v", auth_ip.rep);

         vauth_ip->push(auth_ip);
         }

         vinfo_data->push(info);
      vroaming_data->push(data);
      }

   U_ASSERT_EQUALS(vauth->size(),      vauth_ip->size())
   U_ASSERT_EQUALS(vauth->size(),    vinfo_data->size())
   U_ASSERT_EQUALS(vauth->size(), vroaming_data->size())
 
   *hostname = USocketExt::getNodeName();

   U_INTERNAL_DUMP("host = %v:%d hostname = %V ip = %V", UServer_Base::IP_address->rep, UServer_Base::port, hostname->rep, UServer_Base::IP_address->rep)

   UUDPSocket cClientSocket(UClientImage_Base::bIPv6);

   auth_ip = (*vauth_ip)[0];

   if (cClientSocket.connectServer(auth_ip, 1001))
      {
      const char* p = cClientSocket.getLocalInfo();

      ip = UString(p, u__strlen(p, __PRETTY_FUNCTION__));

      if (ip != *UServer_Base::IP_address)
         {
         (void) UFile::writeToTmp(U_STRING_TO_PARAM(ip), O_RDWR | O_TRUNC, U_CONSTANT_TO_PARAM("IP_ADDRESS"), 0);

         U_SRV_LOG("WARNING: SERVER IP ADDRESS differ from IP address: %V to connect to AUTH: %V", ip.rep, auth_ip.rep);
         }
      }

   auth_ip = vauth_ip->join(' ');

   fw_env->snprintf_add(U_CONSTANT_TO_PARAM("'AuthServiceIP=%v'\n"), auth_ip.rep);

   U_INTERNAL_ASSERT_EQUALS(UPing::addrmask, 0)

   UPing::addrmask = (fd_set*) UServer_Base::getOffsetToDataShare(sizeof(fd_set) + sizeof(uint32_t));

   // crypto cmd

// if (*decrypt_cmd) pgp.set(decrypt_cmd, (char**)0);
// if (*decrypt_key)
      {
      U_INTERNAL_ASSERT(decrypt_key->isNullTerminated())

      UDES3::setPassword(decrypt_key->data());

      *IP_address_trust = UDES3::signData(U_CONSTANT_TO_PARAM("%v"), UServer_Base::IP_address->rep);
      }

   // firewall cmd

   UString command(500U);

   command.snprintf(U_CONSTANT_TO_PARAM("/bin/sh %v initialize allowed_web_hosts"), fw_cmd->rep);

   fw->set(command, (char**)0);
   fw->setEnvironment(fw_env);

   if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/server_plugin_nocat.err");

   // users monitoring

   if (check_expire)
      {
      UTimer::insert(this);

      U_SRV_LOG("Monitoring set for every %d secs", UTimeVal::getSecond());
      }

   // users traffic

   U_NEW(UIptAccount, ipt, UIptAccount(UClientImage_Base::bIPv6));

   // users table
 
   U_INTERNAL_DUMP("num_peers_preallocate = %u", num_peers_preallocate)

   U_NEW(UHashMap<UModNoCatPeer*>, peers, UHashMap<UModNoCatPeer*>(num_peers_preallocate < 49157U ? U_GET_NEXT_PRIME_NUMBER(num_peers_preallocate * 8) : 49157U));

   U_NEW(UString, status_content, UString(U_CAPACITY));
   U_NEW(UVector<UString>, openlist, UVector<UString>);

   U_INTERNAL_ASSERT_EQUALS(peers_preallocate, 0)

   UInterrupt::setHandlerForSegv(preallocatePeers, preallocatePeersFault);

   U_INTERNAL_DUMP("num_peers_preallocate = %u peers_preallocate = %p", num_peers_preallocate, peers_preallocate)

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

bool UNoCatPlugIn::preallocatePeersFault()
{
   U_TRACE_NO_PARAM(0, "UNoCatPlugIn::preallocatePeersFault()")

   // send msg to portal

   UString msg(200U);

   unsigned long vsz, rss;

   u_get_memusage(&vsz, &rss);

   double _vsz = (double)vsz / (1024.0 * 1024.0),
          _rss = (double)rss / (1024.0 * 1024.0);

   msg.snprintf(U_CONSTANT_TO_PARAM("/error_ap?ap=%v@%v&public=%v%%3A%u&msg="
                           "address+space+usage%%3A%2f+MBytes+-+"
                                     "rss+usage%%3A%2f+MBytes"), label->rep, hostname->rep, IP_address_trust->rep, UServer_Base::port, _vsz, _rss);

   sendMsgToPortal(0, msg, 0);

   U_INTERNAL_ASSERT_EQUALS(peers_preallocate, 0)

   if (num_peers_preallocate > 512)
      {
      num_peers_preallocate = 512;

      U_RETURN(true);
      }

   U_RETURN(false);
}

int UNoCatPlugIn::handlerFork()
{
   U_TRACE_NO_PARAM(0, "UNoCatPlugIn::handlerFork()")

   uint32_t i, n;

   // check if we want some preallocation for memory pool...

// U_WRITE_MEM_POOL_INFO_TO("mempool.%N.%P.handlerFork", 0)

   if (*mempool) UMemoryPool::allocateMemoryBlocks(mempool->data()); // NB: start from 1... (Ex: 20,30,0,1,1050,0,0,0,2)

   // manage internal device...

   U_INTERNAL_DUMP("num_radio = %u", num_radio)

   sockp = (UPing**)                UMemoryPool::_malloc(num_radio, sizeof(UPing*));
   vaddr = (UVector<UIPAddress*>**) UMemoryPool::_malloc(num_radio, sizeof(UVector<UIPAddress*>*));

   UPing::addrmask = (fd_set*) UServer_Base::getPointerToDataShare(UPing::addrmask);

   for (i = 0; i < num_radio; ++i)
      {
      U_NEW(UVector<UIPAddress*>, vaddr[i], UVector<UIPAddress*>);
      U_NEW(UPing, sockp[i], UPing(3000, UClientImage_Base::bIPv6));

      if (((check_type & U_CHECK_ARP_PING) != 0))
         {
         sockp[i]->setLocal(UServer_Base::socket->localIPAddress());

#     ifdef HAVE_NETPACKET_PACKET_H
         U_INTERNAL_ASSERT((*vInternalDevice)[i].isNullTerminated())

         sockp[i]->initArpPing((*vInternalDevice)[i].data());
#     endif
         }
      }

   // send start to portal

   U_INTERNAL_ASSERT_EQUALS(UClient_Base::queue_dir, 0)

   UString msg(300U), output(U_CAPACITY), allowed_web_hosts(U_CAPACITY);

   msg.snprintf(U_CONSTANT_TO_PARAM("/start_ap?ap=%v@%v&public=%v%%3A%u&pid=%u"), label->rep, hostname->rep, IP_address_trust->rep, UServer_Base::port, U_SRV_CNT_USR9);
                                                                                                                                                        U_SRV_CNT_USR9 = u_pid;

   for (i = 0, n = vauth_url->size(); i < n; ++i)
      {
      sendMsgToPortal(i, msg, &output);

      (void) allowed_web_hosts.append(output);
      }

   // initialize the firewall: direct all port 80 traffic to us...

   total_connections = 0;

   fw->setArgument(4, allowed_web_hosts.c_str());

   (void) fw->executeAndWait(0, -1, fd_stderr);

#ifndef U_LOG_DISABLE
   UServer_Base::logCommandMsgError(fw->getCommand(), false);
#endif

   fw->delArgument();
   fw->setLastArgument("openlist");

   if (*allowed_members)
      {
      // 00:27:22:4f:69:f4 172.16.1.1 Member ### routed_ap-locoM2

      UVector<UString> vtmp;

      n = vtmp.loadFromData(*allowed_members);

      UString ip,
              UserUploadRate,
              UserDownloadRate;
      uint32_t increment = ((n % 3) ? 5 : 3);

      for (i = 0; i < n; i += increment)
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

         creatNewPeer();

         peer->ip  = ip;
         peer->mac = vtmp[i];

         peer->ip.copy(peer->UIPAddress::pcStrAddress);

         (void) peer->user.assign(U_CONSTANT_TO_PARAM("allowed"));

         U_peer_allowed                         = true;
         U_peer_index_AUTH                      = getIndexAUTH(peer->UIPAddress::pcStrAddress);
         U_peer_index_network                   = index_network;
         U_ipaddress_StrAddressUnresolved(peer) = false;

         setNewPeer();

         permit(UserDownloadRate, UserUploadRate);
         }
      }

   U_INTERNAL_ASSERT_EQUALS(dirwalk, 0)

#if !defined(U_LOG_DISABLE) && defined(USE_LIBZ)
   if (UServer_Base::isLog()) U_NEW(UDirWalk, dirwalk, UDirWalk(ULog::getDirLogGz(), U_CONSTANT_TO_PARAM("*.gz")));
#endif

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

// Connection-wide hooks

int UNoCatPlugIn::handlerRequest()
{
   U_TRACE_NO_PARAM(1, "UNoCatPlugIn::handlerRequest()")

   if (UHTTP::file->isRoot() ||
       UClientImage_Base::isRequestNotFound())
      {
      Url url;
      int mode = UHTTP::NETWORK_AUTHENTICATION_REQUIRED;
      UString host(U_HTTP_HOST_TO_PARAM), buffer(U_CAPACITY);

      U_INTERNAL_DUMP("host = %V UServer_Base::client_address = %.*S", host.rep, U_CLIENT_ADDRESS_TO_TRACE)

      peer = 0;

      // NB: check for request from AUTH, which may be:
      // -------------------------------------------------------------
      // 1) /checkFirewall  - check firewall and report info
      // 2) /check          - check system and report info
      // 3) /uptime         - report uptime info 
      // 4) /users          - report list ip of peers permitted
      // 5) /status         - report status user
      // 6) /logout         - logout specific user
      // 7) /checkForUsersF - check users ffffff and report change
      // -------------------------------------------------------------

      U_INTERNAL_DUMP("vauth_ip = %p vauth_ip = %S", vauth_ip, UObject2String(*vauth_ip))

      uint32_t index_AUTH = vauth_ip->find(U_CLIENT_ADDRESS_TO_PARAM);

      if (index_AUTH != U_NOT_FOUND)
         {
         (void) UServer_Base::csocket->shutdown(SHUT_RD);

         U_SRV_LOG("AUTH(%u) request: %.*S", index_AUTH, U_HTTP_URI_TO_TRACE);

         if (U_HTTP_URI_STREQ("/check"))
            {
            if (flag_check_system == false) checkSystem();

            notifyAuthOfUsersInfo(index_AUTH);
            }
         else if (U_HTTP_URI_STREQ("/uptime"))
            {
            status_content->setBuffer(U_CAPACITY);

            status_content->setFromNumber32(u_get_uptime());

            setHTTPResponse(*status_content, U_unknow);
            }
         else if (U_HTTP_URI_STREQ("/users"))
            {
            // NB: request from AUTH to get list info on peers permitted

            status_content->setBuffer(U_CAPACITY);

            peers->callForAllEntry(getPeerListInfo);

            setHTTPResponse(*status_content, U_html);

            U_SRV_LOG("AUTH request to get list info on peers permitted");
            }
         else if (U_HTTP_URI_STREQ("/status"))
            {
            UString ap_label;

            if (U_HTTP_QUERY_MEMEQ("ip="))
               {
               // NB: request from AUTH to get status user

               uint32_t len    = U_http_info.query_len - U_CONSTANT_SIZE("ip=");
               const char* ptr = U_http_info.query     + U_CONSTANT_SIZE("ip=");

               UString peer_ip = getIPAddress(ptr, len);

               U_SRV_LOG("AUTH request to get status for user: IP %v", peer_ip.rep);

               peer = (*peers)[peer_ip];

               U_INTERNAL_DUMP("peer = %p", peer)

               if (peer == 0)
                  {
                  UHTTP::setBadRequest();

                  goto end;
                  }
               }
            else if (U_HTTP_QUERY_MEMEQ("label="))
               {
               // NB: request from AUTH to get status users...

               uint32_t len    = U_http_info.query_len - U_CONSTANT_SIZE("label=");
               const char* ptr = U_http_info.query     + U_CONSTANT_SIZE("label=");

               (void) ap_label.assign(ptr, len);

               U_SRV_LOG("AUTH request to get status for users: ap %V", ap_label.rep);
               }

            // status

            (void) getARPCache();

            if (total_connections > peers->size()) (void) checkFirewall(); // anomalous

            setStatusContent(ap_label); // NB: peer == 0 -> request from AUTH to get status access point...
            }
         else if (U_HTTP_URI_STREQ("/checkFirewall"))
            {
            if (flag_check_system == false)
               {
               (void) getARPCache();

               (void) checkFirewall();
               }

            notifyAuthOfUsersInfo(index_AUTH);
            }
         else if (U_HTTP_URI_STREQ("/logout") &&
                  U_http_info.query_len)
            {
            // NB: request from AUTH to logout user (ip=192.168.301.223&mac=00:e0:4c:d4:63:f5)

            UString data = getSignedData(U_HTTP_QUERY_TO_PARAM);

            if (data.empty())
               {
               U_SRV_LOG("WARNING: AUTH request to logout user tampered");

               goto set_redirection_url;
               }

            uint32_t len    = data.size() - U_CONSTANT_SIZE("ip=");
            const char* ptr = data.data() + U_CONSTANT_SIZE("ip=");

            UString peer_ip = getIPAddress(ptr, len);

            U_SRV_LOG("AUTH request to logout user: IP %v", peer_ip.rep);

            peer = (*peers)[peer_ip];

            if (peer == 0)
               {
               uint32_t pos = data.find_first_of('&', 3);

               if (pos != U_NOT_FOUND &&
                   strncmp(data.c_pointer(pos+1), U_CONSTANT_TO_PARAM("mac=")) == 0)
                  {
                  UString mac = data.substr(pos + 5, U_CONSTANT_SIZE("00:00:00:00:00:00"));

                  U_SRV_LOG("AUTH request to logout user: MAC %v", mac.rep);

                  if (mac != *UString::str_without_mac &&
                      getPeerFromMAC(mac))
                     {
                     goto next;
                     }
                  }

               UHTTP::setBadRequest();

               goto end;
               }

next:       (void) getARPCache();

            if (U_peer_status != UModNoCatPeer::PEER_PERMIT)
               {
               U_SRV_LOG("AUTH request to logout user with status DENY: IP %v", peer_ip.rep);

               setStatusContent(UString::getStringNull());
               }
            else
               {
               int disconnected = peer->checkPeerInfo(true);

               if (disconnected == 0) disconnected = U_LOGOUT_REQUEST_FROM_AUTH;

               deny(disconnected, check_expire);

               notifyAuthOfUsersInfo(index_AUTH);
               }
            }
#     ifdef USE_LIBTDB
         else if (U_HTTP_URI_STREQ("/checkForUsersF") &&
                  U_http_info.query_len)
            {
            status_content->setBuffer(U_CAPACITY);

            if (pdata)
               {
               UVector<UString> vusersF;

               for (uint32_t i = 0, n = vusersF.split(U_HTTP_QUERY_TO_PARAM, ','); i < n; ++i)
                  {
                  peer = (*peers)[vusersF[i]]; // ip address

                  if (peer)
                     {
                     (void) setPeerLabel();

                     if (peer->label.equal(U_CONSTANT_TO_PARAM("ffffff")) == false)
                        {
                        (void) status_content->append(peer->user);
                        (void) status_content->append(0, ',');
                        (void) status_content->append(peer->label);
                        (void) status_content->append(0, ',');
                        }
                     }
                  }
               }

            setHTTPResponse(*status_content, U_txt);
            }
#     endif
         else
            {
            UHTTP::setBadRequest();
            }

         goto end;
         }

      // ---------------------------
      // NB: check request from user
      // ---------------------------

      index_AUTH = getIndexAUTH(UServer_Base::client_address);

      url  = *((*vauth_url)[index_AUTH]);
      peer = peers->at(U_CLIENT_ADDRESS_TO_PARAM);

      U_SRV_LOG("Start CHECK_REQUEST_FROM_USER phase of plugin nocat: index_AUTH = %u peer = %p", index_AUTH, peer);

      // NB: check for strange initial WiFi behaviour of the iPhone...

      if (U_HTTP_URI_STREQ("/library/test/success.html") &&
          host.equal(U_CONSTANT_TO_PARAM("www.apple.com")))
         {
         // When the iPhone automatically assesses the WiFi connection they appear to
         // issue a HTTP GET request to http://www.apple.com/library/test/success.html.
         // This is issued by a User Agent 'CaptiveNetworkSupport' and it does not attempt
         // to use any proxy that may be configured on the iPhone. This attempt will obviously
         // result in a 404 failure and the WLAN text. After this the iPhone attempts to connect
         // to www.apple.com again, this time through Safari to some-kind of login page, again
         // resulting in the WLAN text. The WiFi link on the iPhone is then discarded. It was clear
         // that initial connection needed to succeed. When that connection was attempted a single
         // page of HTML is returned which has the word 'Success' in it. Once this behaviour had
         // been characterised casual Internet searching found that several people had noted this
         // behaviour as well, and speculated upon its meaning. What is clear is that Apple fail
         // to document it. Since the iPhone needs a positive response to its strange little query
         // it was decided to give it one

#     ifdef DEBUG
         U_SRV_LOG("Detected strange initial WiFi request (iPhone) from peer: IP %.*s", U_CLIENT_ADDRESS_TO_TRACE);
#     endif

         /*
         if (u_find(U_HTTP_USER_AGENT_TO_PARAM, U_CONSTANT_TO_PARAM("CaptiveNetworkSupport")) != 0)
            {
            setHTTPResponse(UString::str_IPHONE_SUCCESS, U_html);

            goto end;
            }
         */
            
         (void) buffer.assign(U_CONSTANT_TO_PARAM("IPHONE"));

         goto set_redirect_to_AUTH;
         }

      if (U_http_info.user_agent_len &&
          UServices::dosMatchWithOR(U_HTTP_USER_AGENT_TO_PARAM, U_CONSTANT_TO_PARAM("*BDNC*|*TMUFE*"), FNM_IGNORECASE))
         {
#     ifdef DEBUG
         U_SRV_LOG("Detected User-Agent: (TMUFE|BDNC) (AntiVirus) from peer: IP %.*s", U_CLIENT_ADDRESS_TO_TRACE);
#     endif

         UHTTP::setForbidden();

         goto end;
         }

      // NB: other kind of message (from user), which may be:
      // ---------------------------------------------------------------
      // a) /cpe            - specific request, force redirect via https 
      // b) /test           - force redirect even without a firewall
      // e) /ticket         - authorization ticket with info
      // h) /login_validate - before authorization ticket with info
      // ---------------------------------------------------------------

      if (U_HTTP_URI_STREQ("/cpe") ||
          U_HTTP_URI_STREQ("/test"))
         {
         if (U_HTTP_URI_STREQ("/cpe"))
            {
            url.setPath(U_CONSTANT_TO_PARAM("/cpe"));
            url.setService(U_CONSTANT_TO_PARAM("https"));
            }

         (void) buffer.assign(U_CONSTANT_TO_PARAM("http://www.google.com"));

         goto set_redirect_to_AUTH;
         }

      if (U_HTTP_URI_STREQ("/login_validate") &&
          U_http_info.query_len)
         {
         // user has pushed the login button

         uint32_t len;
         const char* ptr;
         UString _uid(100U), data = getSignedData(U_HTTP_QUERY_TO_PARAM);

         if (data.empty())
            {
            U_SRV_LOG("WARNING: tampered request to validate login from peer: IP %.*s", U_CLIENT_ADDRESS_TO_TRACE);

            goto set_redirection_url;
            }

         len = data.size() - U_CONSTANT_SIZE("uid=");
         ptr = data.data() + U_CONSTANT_SIZE("uid=");

         (void) buffer.assign(ptr, len);

         len = buffer.find('&');

         if (len == U_NOT_FOUND) (void) _uid.replace(buffer);
         else                    (void) _uid.replace(ptr, len);

         U_SRV_LOG("request to validate login for uid %V from peer: IP %.*s", _uid.rep, U_CLIENT_ADDRESS_TO_TRACE);

         if (vLoginValidate->find(_uid) == U_NOT_FOUND) vLoginValidate->push(_uid);

         (void) buffer.assign(U_HTTP_QUERY_TO_PARAM); // NB: we resend the same data to portal... (as redirect field)

         url.setPath(U_CONSTANT_TO_PARAM("/login_validate"));

         mode = UHTTP::NO_BODY;

         goto set_redirect_to_AUTH;
         }

      if (U_HTTP_URI_STREQ("/ticket") &&
          U_HTTP_QUERY_MEMEQ("ticket="))
         {
         // user with a ticket

         uint32_t len    = U_http_info.query_len - U_CONSTANT_SIZE("ticket=");
         const char* ptr = U_http_info.query     + U_CONSTANT_SIZE("ticket=");

         UString data = getSignedData(ptr, len);

         if (data.empty())
            {
            U_SRV_LOG("WARNING: invalid ticket from peer: IP %.*s", U_CLIENT_ADDRESS_TO_TRACE);

            goto set_redirection_url;
            }


#     ifndef U_LOG_DISABLE
         if (UServer_Base::isLog())
            {
            UString printable(data.size() * 4);

            UEscape::encode(data, printable);

            ULog::log(U_CONSTANT_TO_PARAM("%sauth message: %v"), UServer_Base::mod_name[0], printable.rep);
            }
#     endif

         if (peer == 0 ||
             checkAuthMessage(data) == false)
            {
            goto set_redirection_url;
            }

         // OK: go to the destination (with Location: ...)

         mode = 0;

         goto redirect;
         }

      if (peer                  &&
          U_http_version == '1' &&
          host == peer->gateway)
         {
         U_SRV_LOG("WARNING: missing ticket from peer: IP %.*s", U_CLIENT_ADDRESS_TO_TRACE);
         }

set_redirection_url:
      (void) buffer.reserve(7 + U_http_host_len + U_http_info.uri_len);

      buffer.snprintf(U_CONSTANT_TO_PARAM("http://%v%.*s"), host.rep, U_HTTP_URI_TO_TRACE);

set_redirect_to_AUTH:
           if (peer) checkOldPeer();
      else if (creatNewPeer(index_AUTH) == false) goto end;

      if (mode == UHTTP::NO_BODY          && // login_validate
          (check_type & U_CHECK_MAC) != 0 && // not unifi (L2)
          (peer->mac == *UString::str_without_mac || peer->mac.empty()))
         {
         mode = UHTTP::REFRESH | UHTTP::NO_BODY;

         (void) location->reserve(7 + U_http_host_len + U_http_info.uri_len + 1 + U_http_info.query_len);

         location->snprintf(U_CONSTANT_TO_PARAM("http://%v/login_validate?%.*s"), host.rep, U_HTTP_QUERY_TO_TRACE);

         U_SRV_LOG("WARNING: missing MAC from peer: IP %.*s - redirect to %V", U_CLIENT_ADDRESS_TO_TRACE, location->rep);
         }
      else
         {
         U_INTERNAL_ASSERT_POINTER(peer)
         U_INTERNAL_ASSERT(peer->mac)

         Url* auth = (*vauth_url)[index_AUTH];
         uint32_t sz = (buffer.size() * 3);
         UString str = UStringExt::substitute(peer->mac, ':', U_CONSTANT_TO_PARAM("%3A"));

         (void) location->reserve(sz + 4096U);

         location->snprintf(U_CONSTANT_TO_PARAM("%.*s%s?mac=%v&ip="), U_URL_TO_TRACE(url), (url.isPath() ? "" : "/login"), str.rep);

         U_INTERNAL_ASSERT_EQUALS(u_isUrlEncodeNeeded(U_STRING_TO_PARAM(peer->ip)), false)

         location->snprintf_add(U_CONSTANT_TO_PARAM("%v&redirect="), peer->ip.rep);

         if (strncmp(buffer.data(), U_URL_TO_PARAM(*auth)) == 0) (void) location->append(U_CONSTANT_TO_PARAM("http%3A//www.google.com&gateway="));
         else
            {
            // NB: redirect can be signed data for example (base64)...

            if (u_isUrlEncodeNeeded(U_STRING_TO_PARAM(buffer)) == false) location->snprintf_add(U_CONSTANT_TO_PARAM("%v&gateway="), buffer.rep);
            else
               {
               UString value_encoded(sz);

               Url::encode(buffer, value_encoded);

               location->snprintf_add(U_CONSTANT_TO_PARAM("%v&gateway="), value_encoded.rep);
               }
            }

         str = UStringExt::substitute(peer->gateway, ':', U_CONSTANT_TO_PARAM("%3A"));

         location->snprintf_add(U_CONSTANT_TO_PARAM("%v&timeout=%u&token="), str.rep, time_available);

         U_INTERNAL_ASSERT(peer->token)

         location->snprintf_add(U_CONSTANT_TO_PARAM("%v&ap=%v@%v&ts=%ld"), peer->token.rep, peer->label.rep, IP_address_trust->rep, u_now->tv_sec);
         }

redirect:
      UHTTP::ext->clear();

      UHTTP::setRedirectResponse(mode, U_STRING_TO_PARAM(*location));

end:  UClientImage_Base::setCloseConnection();

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UModNoCatPeer::dump(bool _reset) const
{
   *UObjectIO::os << "next                  " << (void*)next       <<  '\n'
                  << "expire                " << expire            <<  '\n'
                  << "logout                " << logout            <<  '\n'
                  << "ctraffic              " << ctraffic          <<  '\n'
                  << "connected             " << connected         <<  '\n'
                  << "time_remain           " << time_remain       <<  '\n'
                  << "traffic_done          " << traffic_done      <<  '\n'
                  << "traffic_remain        " << traffic_remain    <<  '\n'
                  << "time_no_traffic       " << time_no_traffic   <<  '\n'
                  << "traffic_available     " << traffic_available <<  '\n'
                  << "ip        (UString    " << (void*)&ip        << ")\n"
                  << "mac       (UString    " << (void*)&mac       << ")\n"
                  << "token     (UString    " << (void*)&token     << ")\n"
                  << "label     (UString    " << (void*)&label     << ")\n"
                  << "ifname    (UString    " << (void*)&ifname    << ")\n"
                  << "gateway   (UString    " << (void*)&gateway   << ")\n"
                  << "fw        (UCommand   " << (void*)&fw        << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}

const char* UNoCatPlugIn::dump(bool _reset) const
{
   *UObjectIO::os << "nfds                                           " << nfds                        <<  '\n'
                  << "pdata                                          " << pdata                       <<  '\n' 
                  << "vaddr                                          " << (void*)vaddr                <<  '\n' 
                  << "sockp                                          " << (void*)sockp                <<  '\n'
                  << "paddrmask                                      " << (void*)paddrmask            <<  '\n'
                  << "fd_stderr                                      " << fd_stderr                   <<  '\n'
                  << "num_radio                                      " << num_radio                   <<  '\n'
                  << "time_available                                 " << time_available              <<  '\n'
                  << "next_event_time                                " << next_event_time             <<  '\n'
                  << "total_connections                              " << total_connections           <<  '\n'
                  << "traffic_available                              " << traffic_available           <<  '\n'
                  << "flag_check_system                              " << flag_check_system           <<  '\n'
                  << "last_request_check                             " << last_request_check          <<  '\n'
                  << "last_request_firewall                          " << last_request_firewall       <<  '\n'
                  << "label                (UString                  " << (void*)label                << ")\n"
                  << "input                (UString                  " << (void*)input                << ")\n"
                  << "extdev               (UString                  " << (void*)extdev               << ")\n"
                  << "fw_cmd               (UString                  " << (void*)fw_cmd               << ")\n"
                  << "fw_env               (UString                  " << (void*)fw_env               << ")\n"
                  << "intdev               (UString                  " << (void*)intdev               << ")\n"
                  << "mempool              (UString                  " << (void*)mempool              << ")\n"
                  << "hostname             (UString                  " << (void*)hostname             << ")\n"
                  << "localnet             (UString                  " << (void*)localnet             << ")\n"
                  << "location             (UString                  " << (void*)location             << ")\n"
                  << "auth_login           (UString                  " << (void*)auth_login           << ")\n"
                  << "decrypt_key          (UString                  " << (void*)decrypt_key          << ")\n"
                  << "status_content       (UString                  " << (void*)status_content       << ")\n"
                  << "allowed_members      (UString                  " << (void*)allowed_members      << ")\n"
                  << "fw                   (UCommand                 " << (void*)fw                   << ")\n"
                  << "ipt                  (UIptAccount              " << (void*)ipt                  << ")\n"
                  << "vauth_url            (UVector<Url*>            " << (void*)vauth_url            << ")\n"
                  << "vauth                (UVector<UString>         " << (void*)vauth                << ")\n"
                  << "vauth_ip             (UVector<UString>         " << (void*)vauth_ip             << ")\n"
                  << "vInternalDevice      (UVector<UString>         " << (void*)vInternalDevice      << ")\n"
                  << "vLocalNetworkLabel   (UVector<UString>         " << (void*)vLocalNetworkLabel   << ")\n"
                  << "client               (UHttpClient<UTCPSocket>  " << (void*)client               << ")\n"
                  << "peers                (UHashMap<UModNoCatPeer*> " << (void*)peers                << ')';

   if (_reset)
      {
      UObjectIO::output();

      return UObjectIO::buffer_output;
      }

   return 0;
}
#endif
