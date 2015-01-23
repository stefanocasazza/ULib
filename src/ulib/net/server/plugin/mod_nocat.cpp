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
#include <ulib/utility/services.h>
#include <ulib/utility/dir_walk.h>
#include <ulib/net/server/server.h>
#include <ulib/utility/string_ext.h>
#include <ulib/net/server/client_image.h>
#include <ulib/net/server/plugin/mod_nocat.h>

#include <errno.h>

U_CREAT_FUNC(server_plugin_nocat, UNoCatPlugIn)

int                        UNoCatPlugIn::fd_stderr;
int                        UNoCatPlugIn::check_type;
int                        UNoCatPlugIn::next_event_time;
bool                       UNoCatPlugIn::flag_check_system;
long                       UNoCatPlugIn::last_request_check;
long                       UNoCatPlugIn::last_request_firewall;
UPing**                    UNoCatPlugIn::sockp;
fd_set                     UNoCatPlugIn::addrmask;
fd_set*                    UNoCatPlugIn::paddrmask;
uint32_t                   UNoCatPlugIn::nfds;
uint32_t                   UNoCatPlugIn::num_radio;
uint32_t                   UNoCatPlugIn::check_expire;
uint32_t                   UNoCatPlugIn::time_available;
uint32_t                   UNoCatPlugIn::total_connections;
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
UString*                   UNoCatPlugIn::login_timeout;
UString*                   UNoCatPlugIn::status_content;
UString*                   UNoCatPlugIn::label_to_match;
UString*                   UNoCatPlugIn::allowed_members;
UString*                   UNoCatPlugIn::peer_present_in_arp_cache;
UCommand*                  UNoCatPlugIn::fw;
UDirWalk*                  UNoCatPlugIn::dirwalk;
UIptAccount*               UNoCatPlugIn::ipt;
UModNoCatPeer*             UNoCatPlugIn::peer;
UModNoCatPeer*             UNoCatPlugIn::peers_preallocate;
UVector<Url*>*             UNoCatPlugIn::vauth_url;
UVector<Url*>*             UNoCatPlugIn::vinfo_url;
UVector<UString>*          UNoCatPlugIn::openlist;
UVector<UString>*          UNoCatPlugIn::vauth;
UVector<UString>*          UNoCatPlugIn::vauth_ip;
UVector<UString>*          UNoCatPlugIn::varp_cache;
UVector<UString>*          UNoCatPlugIn::vLoginValidate;
UVector<UString>*          UNoCatPlugIn::vInternalDevice;
UVector<UString>*          UNoCatPlugIn::vLocalNetworkLabel;
UVector<UIPAllow*>*        UNoCatPlugIn::vLocalNetworkMask;
UVector<UIPAddress*>**     UNoCatPlugIn::vaddr;
UHttpClient<UTCPSocket>*   UNoCatPlugIn::client;
UHashMap<UModNoCatPeer*>*  UNoCatPlugIn::peers;

const UString*    UNoCatPlugIn::str_without_label;
const UString*    UNoCatPlugIn::str_allowed_members_default;
//const UString*  UNoCatPlugIn::str_IPHONE_SUCCESS;

#define U_NOCAT_STATUS \
"<html>\n" \
"<head>\n" \
"<meta http-equiv=\"Cache Control\" content=\"max-age=0\">\n" \
"<title>Access Point: %s</title>" \
"</head>\n" \
"<body bgcolor=\"#FFFFFF\" text=\"#000000\">\n" \
"<h1>Access Point: %s</h1>\n" \
"<hr noshade=\"1\"/>\n" \
"<table border=\"0\" cellpadding=\"5\" cellspacing=\"0\">\n" \
   "<tr><td>Current Time</td><td>%5D</td></tr>\n" \
   "<tr><td>Gateway Up Since</td><td>%#5D</td></tr>\n" \
   "<tr><td>GatewayVersion</td><td>" ULIB_VERSION "</td></tr>\n" \
   "<tr><td>ExternalDevice</td><td>%.*s</td></tr>\n" \
   "<tr><td>InternalDevice</td><td>%.*s</td></tr>\n" \
   "<tr><td>LocalNetwork</td><td>%.*s</td></tr>\n" \
   "<tr><td>GatewayPort</td><td>%u</td></tr>\n" \
   "<tr><td>AuthServiceAddr</td><td>%.*s</td></tr>\n" \
   "%s" \
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
       "%.*s" \
   "</table></td></tr>\n" \
"</table>\n" \
"<hr noshade=\"1\"/>\n" \
"<img src=%s width=\"112\" height=\"35\">\n" \
"<p style=\"text-align:right\">Powered by ULib</p>\n" \
"</body>\n" \
"</html>"

// define method VIRTUAL of class UEventTime

int UModNoCatPeer::handlerTime()
{
   U_TRACE(0, "UModNoCatPeer::handlerTime()")

   if (checkPeerInfo(true) == false)
      {
      UNoCatPlugIn::peer = this;

      UString* tmp =
      UNoCatPlugIn::login_timeout;
      UNoCatPlugIn::login_timeout = 0;

      UNoCatPlugIn::deny(false);

      UNoCatPlugIn::login_timeout = tmp;
      }

   // return value:
   // ---------------
   // -1 - normal
   //  0 - monitoring
   // ---------------

   U_RETURN(-1);
}

void UNoCatPlugIn::str_allocate()
{
   U_TRACE(0, "UNoCatPlugIn::str_allocate()")

   U_INTERNAL_ASSERT_EQUALS(str_without_label, 0)
   U_INTERNAL_ASSERT_EQUALS(str_allowed_members_default, 0)
// U_INTERNAL_ASSERT_EQUALS(str_IPHONE_SUCCESS, 0)

   static ustringrep stringrep_storage[] = {
      { U_STRINGREP_FROM_CONSTANT("without_label") },
      { U_STRINGREP_FROM_CONSTANT("/etc/nodog.allowed") },
      /*
      { U_STRINGREP_FROM_CONSTANT("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n"
                                  "<HTML>\n"
                                  "<HEAD>\n"
                                  "   <TITLE>Success</TITLE>\n"
                                  "</HEAD>\n"
                                  "<BODY>\n"
                                  "Success\n"
                                  "</BODY>\n"
                                  "</HTML>") }
      */
   };

   U_NEW_ULIB_OBJECT(str_without_label,           U_STRING_FROM_STRINGREP_STORAGE(0));
   U_NEW_ULIB_OBJECT(str_allowed_members_default, U_STRING_FROM_STRINGREP_STORAGE(1));
// U_NEW_ULIB_OBJECT(str_IPHONE_SUCCESS,          U_STRING_FROM_STRINGREP_STORAGE(2));
}

UNoCatPlugIn::UNoCatPlugIn()
{
   U_TRACE_REGISTER_OBJECT(0, UNoCatPlugIn, "")

   fw                  = U_NEW(UCommand);

   label               = U_NEW(UString);
   input               = U_NEW(UString(U_CAPACITY));
   fw_cmd              = U_NEW(UString);
   fw_env              = U_NEW(UString);
   extdev              = U_NEW(UString);
   intdev              = U_NEW(UString);
   mempool             = U_NEW(UString(UFile::contentOf("/etc/nodog.mempool")));
   hostname            = U_NEW(UString);
   localnet            = U_NEW(UString);
   location            = U_NEW(UString(U_CAPACITY));
   arp_cache           = U_NEW(UString);
   auth_login          = U_NEW(UString);
   decrypt_key         = U_NEW(UString);
   allowed_members     = U_NEW(UString);

   vauth               = U_NEW(UVector<UString>(4U));
   vauth_ip            = U_NEW(UVector<UString>(4U));
   vauth_url           = U_NEW(UVector<Url*>(4U));
   vinfo_url           = U_NEW(UVector<Url*>(4U));
   varp_cache          = U_NEW(UVector<UString>);
   vLoginValidate      = U_NEW(UVector<UString>);
   vInternalDevice     = U_NEW(UVector<UString>(64U));
   vLocalNetworkMask   = U_NEW(UVector<UIPAllow*>);
   vLocalNetworkLabel  = U_NEW(UVector<UString>(64U));

   client = U_NEW(UHttpClient<UTCPSocket>(0));

   client->UClient_Base::setTimeOut(UServer_Base::timeoutMS);

   peer_present_in_arp_cache = U_NEW(UString);

   str_allocate();
}

UNoCatPlugIn::~UNoCatPlugIn()
{
   U_TRACE_UNREGISTER_OBJECT(0, UNoCatPlugIn)

   if (login_timeout ||
       UTimeVal::notZero())
      {
      UTimer::stop();
      UTimer::clear(true);
      }

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
   delete peer_present_in_arp_cache;

   delete vauth;
   delete vauth_ip;
   delete vauth_url;
   delete vinfo_url;
   delete vLoginValidate;
   delete vInternalDevice;
   delete vLocalNetworkMask;
   delete vLocalNetworkLabel;

   delete varp_cache;
   delete  arp_cache;

   if (ipt)   delete ipt;
   if (peers) delete peers;

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

   if (dirwalk) delete dirwalk;
}

// status

bool UNoCatPlugIn::getPeerStatus(UStringRep* key, void* value)
{
   U_TRACE(0, "UNoCatPlugIn::getPeerStatus(%p,%p)", key, value)

   peer = (UModNoCatPeer*) value;

   if ( label_to_match &&
       *label_to_match != peer->label)
      {
      U_RETURN(true);
      }

   const char* color;
   const char* status;
   time_t how_much_connected;
   UString buffer(U_CAPACITY);

   (void) peer->checkPeerInfo((key == 0));

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

   buffer.snprintf("<tr>\n"
                     "<td>%.*s</td>\n"
                     "<td>%.*s</td>\n"
                     "<td>%#5D</td>\n"
                     "<td>%#2D</td>\n"
                     "<td>%#2D</td>\n"
                     "<td>%llu KBytes</td>\n"
                     "<td>%llu %cBytes</td>\n"
                     "<td><a href=\"http://standards.ieee.org/cgi-bin/ouisearch?%c%c%c%c%c%c\">%.*s</a></td>\n"
                     "<td style=\"color:%s\">%s</td>\n"
                   "</tr>\n",
                   U_STRING_TO_TRACE(peer->user),
                   U_STRING_TO_TRACE(peer->ip),
                   peer->connected + u_now_adjust,
                   how_much_connected,
                   peer->time_remain,
                   peer->traffic_done / 1024,
                   traffic_remain, c,
                   mac[0], mac[1], mac[3], mac[4], mac[6], mac[7], peer->mac.size(), mac,
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

      buffer.snprintf("%.*s %.*s %.*s %ld %llu\n", U_STRING_TO_TRACE(peer->user), U_STRING_TO_TRACE(peer->ip),
                                                   U_STRING_TO_TRACE(peer->mac), u_now->tv_sec - peer->ctime, peer->traffic_done);

      (void) status_content->append(buffer);
      }

   U_RETURN(true);
}

void UNoCatPlugIn::setStatusContent(const UString& ap_label)
{
   U_TRACE(0, "UNoCatPlugIn::setStatusContent(%.*S)", U_STRING_TO_TRACE(ap_label))

   status_content->setBuffer(U_CAPACITY);

   label_to_match = (ap_label.empty() ? 0 : (UString*)&ap_label);

   if (peer) getPeerStatus(0, peer);
   else
      {
      getTraffic();

      peers->callForAllEntry(getPeerStatus);

      U_INTERNAL_ASSERT(hostname->isNullTerminated())

      const char* name = hostname->data();

      UString label_buffer(100U);

      if (label_to_match)
         {
         uint32_t index = vLocalNetworkLabel->find(ap_label);

         if (index < vLocalNetworkMask->size())
            {
            UString x = (*vLocalNetworkMask)[index]->spec;

            label_buffer.snprintf("<tr><td>Label (Local Network Mask)</td><td>%.*s (%.*s)</td></tr>\n", U_STRING_TO_TRACE(ap_label), U_STRING_TO_TRACE(x));
            }
         }

      UString buffer(1024U + sizeof(U_NOCAT_STATUS) + intdev->size() + localnet->size() + status_content->size() + label_buffer.size());

      buffer.snprintf(U_NOCAT_STATUS, name, name, u_start_time,
                      U_STRING_TO_TRACE(*extdev),
                      U_STRING_TO_TRACE(*intdev),
                      U_STRING_TO_TRACE(*localnet),
                      UServer_Base::port,
                      U_STRING_TO_TRACE(*auth_login),
                      label_buffer.data(),
                      peers->size(),
                      total_connections,
                      U_STRING_TO_TRACE(*status_content), "/images/auth_logo.png");

      *status_content = buffer;
      }

   setHTTPResponse(*status_content);
}

void UNoCatPlugIn::getTraffic()
{
   U_TRACE(0, "UNoCatPlugIn::getTraffic()")

#ifdef HAVE_LINUX_NETFILTER_IPV4_IPT_ACCOUNT_H
   union uuaddr {
      uint32_t       a;
      struct in_addr addr;
   };

   union uuaddr u;
   const char* ip;
   uint32_t traffic;
   const char* table;
   UModNoCatPeer* _peer;
   struct ipt_acc_handle_ip* entry;

   for (uint32_t i = 0, n = vLocalNetworkMask->size(); i < n; ++i)
      {
      U_INTERNAL_ASSERT((*vLocalNetworkMask)[i]->spec.isNullTerminated())

      table = (*vLocalNetworkMask)[i]->spec.data();

      if (ipt->readEntries(table, false))
         {
         while ((entry = ipt->getNextEntry()))
            {
            u.a = entry->ip;
            ip  = inet_ntoa(u.addr);

            U_INTERNAL_DUMP("IP: %s SRC packets: %u bytes: %u DST packets: %u bytes: %u",
                             ip, entry->src_packets, entry->src_bytes, entry->dst_packets, entry->dst_bytes)

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

                  U_INTERNAL_DUMP("traffic = %u traffic_done = %llu ctraffic = %u traffic_remain = %llu",
                                   traffic, _peer->traffic_done, _peer->ctraffic, _peer->traffic_remain)
                  }
               }
            }
         }
      }
#endif
}

void UNoCatPlugIn::setFireWallCommand(UCommand& cmd, const UString& script, const UString& mac,const UString& ip)
{
   U_TRACE(0, "UNoCatPlugIn::setFireWallCommand(%p,%.*S,%.*S,%.*S)", &cmd, U_STRING_TO_TRACE(script), U_STRING_TO_TRACE(mac), U_STRING_TO_TRACE(ip))

   // NB: request(arp|deny|clear|reset|permit|openlist|initialize) mac ip class(Owner|Member|Public) UserDownloadRate UserUploadRate

   UString command(100U);

   command.snprintf("/bin/sh %.*s deny %.*s %.*s Member 0 0", U_STRING_TO_TRACE(script), U_STRING_TO_TRACE(mac), U_STRING_TO_TRACE(ip));

   cmd.set(command, (char**)0);
   cmd.setEnvironment(fw_env);
}

UString UNoCatPlugIn::getUrlForSendMsgToPortal(uint32_t index_AUTH, const char* msg, uint32_t msg_len)
{
   U_TRACE(0, "UNoCatPlugIn::getUrlForSendMsgToPortal(%u,%.*S,%u)", index_AUTH, msg_len, msg, msg_len)

   Url* auth = (*vinfo_url)[index_AUTH];
   UString auth_host    = auth->getHost(),
           auth_service = auth->getService(),
           url(200U + auth_host.size() + auth_service.size() + msg_len);

   url.snprintf("%.*s://%.*s%.*s", U_STRING_TO_TRACE(auth_service), U_STRING_TO_TRACE(auth_host), msg_len, msg);

   U_RETURN_STRING(url);
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

         tmp.snprintf("%.*s_%.*s", U_STRING_TO_TRACE(*hostname), U_STRING_TO_TRACE(filename));

         filename = tmp;
         }

      UClient_Base::queue_dir = UString::str_CLIENT_QUEUE_DIR;

      result = client->upload(getUrlForSendMsgToPortal(0, U_CONSTANT_TO_PARAM("/uploader")), file, U_STRING_TO_PARAM(filename));

      U_SRV_LOG("%s to queue log file: %.*S", (result ? "success" : "FAILED"), U_FILE_TO_TRACE(file));

      UClient_Base::queue_dir = 0;
      }

   if (result) (void) file._unlink();
}

void UNoCatPlugIn::sendMsgToPortal(uint32_t index_AUTH, const UString& msg, UString* poutput, bool basync)
{
   U_TRACE(0, "UNoCatPlugIn::sendMsgToPortal(%u,%.*S,%p,%b)", index_AUTH, U_STRING_TO_TRACE(msg), poutput, basync)

   UString url = getUrlForSendMsgToPortal(index_AUTH, U_STRING_TO_PARAM(msg));

   if (basync)
      {
      U_INTERNAL_ASSERT_EQUALS(poutput, 0)
      U_INTERNAL_ASSERT_EQUALS(UClient_Base::queue_dir, 0)

#  ifdef U_LOG_ENABLE
      if (UServer_Base::isLog())
         {
         char log_msg[4096];

         UString str = UStringExt::substitute(msg, '%', U_CONSTANT_TO_PARAM("%%")); // NB: we need this because we have many url encoded char...

         (void) u__snprintf(log_msg, sizeof(log_msg), "[nocat] Sent info %%s after %%d attempts to AUTH(%u): %.*S", index_AUTH, U_STRING_TO_TRACE(str));

         (void) client->sendRequestAsync(url, false, log_msg);
         }
      else
#  endif
      (void) client->sendRequestAsync(url);

      return;
      }

#ifdef U_LOG_ENABLE
   const char* result    = "FAILED";
#endif
   bool bqueue = false, we_need_response = (poutput != 0);

loop:
   if (we_need_response == false) UClient_Base::queue_dir = UString::str_CLIENT_QUEUE_DIR;

   if (client->connectServer(url) == false)
      {
      if (we_need_response)
         {
         U_SRV_LOG("sent message FAILED to AUTH(%u): %.*S", index_AUTH, U_STRING_TO_TRACE(msg));

         we_need_response = false;

         goto loop;
         }

      goto end;
      }

   if (client->sendRequest())
      {
      if (poutput) *poutput = client->getContent();

#  ifdef U_LOG_ENABLE
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
      U_SRV_LOG("queue message %s to AUTH(%u): %.*S", result, index_AUTH, U_STRING_TO_TRACE(msg));
      }

   UClient_Base::queue_dir = 0;

   client->reset(); // NB: url is referenced by UClient::url...
}

int UNoCatPlugIn::handlerTime()
{
   U_TRACE(0, "UNoCatPlugIn::handlerTime()")

   U_INTERNAL_DUMP("flag_check_system = %b", flag_check_system)

   if (flag_check_system == false)
      {
      U_INTERNAL_DUMP("check_expire = %u", check_expire)

      U_INTERNAL_ASSERT_MAJOR(check_expire, 0)

      next_event_time = check_expire;

      U_INTERNAL_DUMP("U_http_method_type = %u", U_http_method_type)

      if (U_http_method_type == 0) checkSystem();
      else                         UServer_Base::setCallerHandlerReset(); // NB: we put the check after the http request processing...

      U_INTERNAL_ASSERT_MAJOR(next_event_time, 0)

      UTimeVal::setSecond(next_event_time);
      }

   // return value:
   // ---------------
   // -1 - normal
   //  0 - monitoring
   // ---------------

   U_RETURN(0);
}

bool UNoCatPlugIn::checkPeerStatus(UStringRep* key, void* value)
{
   U_TRACE(0, "UNoCatPlugIn::checkPeerStatus(%.*S,%p)", U_STRING_TO_TRACE(*key), value)

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
            U_SRV_LOG("WARNING: I should to deny user allowed: IP %.*s MAC %.*s - total_connections %u",
                       U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac), total_connections);
            }
         else
            {
            U_SRV_LOG("WARNING: I try to deny user with status permit: IP %.*s MAC %.*s remain: %ld secs %llu bytes - total_connections %u",
                       U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac), peer->time_remain, peer->traffic_remain, total_connections);

            executeCommand(UModNoCatPeer::PEER_DENY);

            --total_connections;
            }
         }
      /*
      else
         {
         U_SRV_LOG("User deny: IP %.*s MAC %.*s", U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));
         }
      */
      }
   else
      {
      // firewall open

      if (U_peer_status == UModNoCatPeer::PEER_DENY)
         {
         // status deny

         U_SRV_LOG("WARNING: I try to deny user with status deny: IP %.*s MAC %.*s remain: %ld secs %llu bytes - total_connections %u - logout %#1D",
                    U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac), peer->time_remain, peer->traffic_remain, total_connections, peer->logout);

         executeCommand(UModNoCatPeer::PEER_DENY);
         }
      /*
      else
         {
         U_SRV_LOG("User %s permit: IP %.*s MAC %.*s", (U_peer_allowed ? "ALLOWED" : ""), U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));
         }
      */

      openlist->erase(pos);
      }

   U_RETURN(true);
}

uint32_t UNoCatPlugIn::checkFirewall(UString& output)
{
   U_TRACE(1, "UNoCatPlugIn::checkFirewall(%.*S)", U_STRING_TO_TRACE(output))

   bool ok;
   uint32_t n = 0, counter = 0,
            timeoutMS = UCommand::timeoutMS;
                        UCommand::timeoutMS = -1;

loop:
   output.setEmpty();

   ok = fw->execute(0, &output, -1, fd_stderr);

#ifdef U_LOG_ENABLE
   UServer_Base::logCommandMsgError(fw->getCommand(), false);
#endif

   if (ok == false) n = total_connections;
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
   U_TRACE(0, "UNoCatPlugIn::checkFirewall()")

   UString output(U_CAPACITY);
   long firewall_interval = u_now->tv_sec - last_request_firewall;

   U_INTERNAL_DUMP("firewall_interval = %ld", firewall_interval)

   U_INTERNAL_ASSERT(u_now->tv_sec >= last_request_firewall)

   if (firewall_interval < 3)
      {
      next_event_time = 3 - firewall_interval;

      U_RETURN(false);
      }

   uint32_t i, n = checkFirewall(output);

   U_SRV_LOG("checkFirewall(): total_connections %u firewall %u", total_connections, n);

   if (n != total_connections)
      {
      UCommand _fw;
      UString out, msg(300U), ip, mac;
      uint32_t n1, j, k = varp_cache->size();

      if (n)
         {
         out = openlist->join(U_CONSTANT_TO_PARAM(" "));

      // (void) UFile::writeToTmp(U_STRING_TO_PARAM(out), false, "nodog_chk.out", 0);
         }

      n = total_connections;

      peers->callForAllEntry(checkPeerStatus);

      U_SRV_LOG("*** FIREWALL NOT ALIGNED: %.*S ***", U_STRING_TO_TRACE(out));

      n1 = openlist->size();

      U_INTERNAL_DUMP("openlist->size() = %u total_connections = %u peers->size() = %u", n1, total_connections, peers->size())

      U_INTERNAL_ASSERT(n1 <= n)

      for (i = 0; i < n1; ++i)
         {
         ip = (*openlist)[i];

         if ((*peers)[ip] ||
             u_isIPv4Addr(U_STRING_TO_PARAM(ip)) == false)
            {
            U_SRV_LOG("WARNING: anomalous peer: IP %S - total_connections %u", U_STRING_TO_TRACE(ip), total_connections);
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

            U_SRV_LOG("WARNING: I try to deny IP %.*S MAC %.*s - total_connections %u", U_STRING_TO_TRACE(ip), U_STRING_TO_TRACE(mac), total_connections);

            U_ASSERT_EQUALS(mac, USocketExt::getMacAddress(ip))

            // NB: request(arp|deny|clear|reset|permit|openlist|initialize) mac ip class(Owner|Member|Public) UserDownloadRate UserUploadRate

            setFireWallCommand(_fw, *fw_cmd, mac, ip);

            (void) _fw.executeAndWait(0, -1, fd_stderr);

#        ifdef U_LOG_ENABLE
            UServer_Base::logCommandMsgError(_fw.getCommand(), false);
#        endif
            }
         }

      if (n != total_connections)
         {
         // send msg to portal for resync

         msg.snprintf("/error_ap?ap=%.*s@%.*s&public=%.*s:%u",
                        U_STRING_TO_TRACE(*label), U_STRING_TO_TRACE(*hostname), U_STRING_TO_TRACE(*UServer_Base::IP_address), UServer_Base::port);

         sendMsgToPortal(msg);
         }
      }

   openlist->clear();

   output.clear();

   U_RETURN(true);
}

void UNoCatPlugIn::checkSystem()
{
   U_TRACE(1, "UNoCatPlugIn::checkSystem()")

   UString info;
   Url* info_url;
   uint32_t i, n;
   long check_interval;

   flag_check_system = true;

   check_interval = u_now->tv_sec - last_request_check;

   U_INTERNAL_DUMP("check_interval = %ld", check_interval)

   U_INTERNAL_ASSERT(u_now->tv_sec >= last_request_check)

   if (check_interval < U_TIME_FOR_ARPING_ASYNC_COMPLETION) // NB: protection from DoS...
      {
      next_event_time = check_expire - check_interval;

      goto end;
      }

   getARPCache();

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
      U_INTERNAL_ASSERT_MAJOR(nfds,0)

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
                  U_SRV_LOG("Peer IP %.*s MAC %.*s don't return ARP reply, I assume he is disconnected...",
                                 U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));
                  }
#           endif

               deny(true);
               }
            }
         }

      nfds      = 0;
      paddrmask = 0;
      }

   for (i = 0, n = vinfo_url->size(); i < n; ++i)
      {
      info_url = (*vinfo_url)[i];

      if (info_url->isQuery())
         {
         info = info_url->getPathAndQuery();

         U_INTERNAL_ASSERT(info)

         // NB: we can't try to send immediately the info data on users to portal because the worst we have a hole
         //     of 10 seconds and the portal can have need to ask us something (to logout some user, the list of peer permitted, ...)

         sendMsgToPortal(i, info, 0, true);

#     ifdef DEBUG
         info.clear(); // NB: to avoid DEAD OF SOURCE STRING WITH CHILD ALIVE...
#     endif

         *info_url = *((*vauth_url)[i]);
         }
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

void UNoCatPlugIn::executeCommand(int type)
{
   U_TRACE(0, "UNoCatPlugIn::executeCommand(%d)", type)

   U_INTERNAL_ASSERT_POINTER(peer)

   peer->fw.setArgument(3, (type == UModNoCatPeer::PEER_PERMIT ? "permit" : "deny"));

   if (peer->fw.executeAndWait(0, -1, fd_stderr)) U_peer_status = type;

#ifdef U_LOG_ENABLE
   UServer_Base::logCommandMsgError(peer->fw.getCommand(), false);
#endif

   last_request_firewall = u_now->tv_sec;
}

void UNoCatPlugIn::deny(bool disconnected)
{
   U_TRACE(0, "UNoCatPlugIn::deny(%b)", disconnected)

   U_INTERNAL_ASSERT_POINTER(peer)

   if (U_peer_status == UModNoCatPeer::PEER_DENY)
      {
      U_SRV_LOG("WARNING: peer already deny: IP %.*s MAC %.*s", U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));

      return;
      }

   if (U_peer_allowed)
      {
      U_SRV_LOG("WARNING: I should to deny user allowed: IP %.*s MAC %.*s", U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));

      return;
      }

   if (peer->logout == 0) peer->logout = u_now->tv_sec; // NB: request of logout or user disconnected...
#ifdef DEBUG
   else // NB: user with no more time or no more traffic...
      {
      U_INTERNAL_ASSERT_EQUALS(peer->logout, peer->expire)
      }
#endif

   addPeerInfo(disconnected ? -1 : peer->logout); // -1 => disconnected (logout implicito)

   executeCommand(UModNoCatPeer::PEER_DENY);

   --total_connections;

   U_SRV_LOG("Peer denied: IP %.*s MAC %.*s remain: %ld secs %llu bytes - total_connections %u",
               U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac), peer->time_remain, peer->traffic_remain, total_connections);

   if (login_timeout)
      {
      UTimer::erase(peer, false, true);

      peer->UEventTime::reset();
      }
}

void UNoCatPlugIn::permit(const UString& UserDownloadRate, const UString& UserUploadRate)
{
   U_TRACE(0, "UNoCatPlugIn::permit(%S,%S)", U_STRING_TO_TRACE(UserDownloadRate), U_STRING_TO_TRACE(UserUploadRate))

   U_INTERNAL_ASSERT_POINTER(peer)
   U_INTERNAL_ASSERT_EQUALS(U_peer_status, UModNoCatPeer::PEER_DENY)

   if (UserDownloadRate) peer->fw.setArgument(7, UserDownloadRate.c_str());
   if (UserUploadRate)   peer->fw.setArgument(8, UserUploadRate.c_str());

   executeCommand(UModNoCatPeer::PEER_PERMIT);

   if (login_timeout)
      {
      peer->UTimeVal::setSecond(peer->time_remain);

      UTimer::insert(peer, true);
      }

   ++total_connections;

   U_SRV_LOG("Peer permitted: IP %.*s MAC %.*s - UserDownloadRate %s UserUploadRate %s remain: %ld secs %llu bytes - total_connections %u",
               U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac),
               UserDownloadRate.data(), UserUploadRate.data(),
               peer->time_remain, peer->traffic_remain, total_connections);
}

void UNoCatPlugIn::setRedirectLocation(const UString& redirect, const Url& auth)
{
   U_TRACE(0, "UNoCatPlugIn::setRedirectLocation(%.*S,%.*S)", U_STRING_TO_TRACE(redirect), U_URL_TO_TRACE(auth))

   U_INTERNAL_ASSERT_POINTER(peer)

   uint32_t sz = redirect.size() + 1024U;

   UString value_encoded(sz * 3U);

   Url::encode(peer->mac, value_encoded);

   (void) location->reserve(sz + 1024U);

   location->snprintf("%.*s%s?mac=%.*s&ip=", U_URL_TO_TRACE(auth), (auth.isPath() ? "" : "/login"), U_STRING_TO_TRACE(value_encoded));

   if (u_isUrlEncodeNeeded(U_STRING_TO_PARAM(peer->ip)) == false) location->snprintf_add("%.*s&redirect=", U_STRING_TO_TRACE(peer->ip));
   else
      {
      Url::encode(peer->ip, value_encoded);

      location->snprintf_add("%.*s&redirect=", U_STRING_TO_TRACE(value_encoded));
      }

   // NB: redirect can be signed data for example (base64)...

   if (u_isUrlEncodeNeeded(U_STRING_TO_PARAM(redirect)) == false) location->snprintf_add("%.*s&gateway=", U_STRING_TO_TRACE(redirect));
   else
      {
      Url::encode(redirect, value_encoded);

      location->snprintf_add("%.*s&gateway=", U_STRING_TO_TRACE(value_encoded));
      }

   Url::encode(peer->gateway, value_encoded);

   location->snprintf_add("%.*s&timeout=%u&token=", U_STRING_TO_TRACE(value_encoded), time_available);

   U_INTERNAL_ASSERT(peer->token)

   location->snprintf_add("%.*s&ap=%.*s@%.*s", U_STRING_TO_TRACE(peer->token), U_STRING_TO_TRACE(peer->label), U_STRING_TO_TRACE(*UServer_Base::IP_address));
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

      input.snprintf("-----BEGIN PGP MESSAGE-----\n\n"
                     "%.*s"
                     "\n-----END PGP MESSAGE-----", U_STRING_TO_TRACE(buffer));

      (void) pgp.execute(&input, &output, -1, fd_stderr);

#  ifdef U_LOG_ENABLE
      UServer_Base::logCommandMsgError(pgp.getCommand(), false);
#  endif
      }
   */

   U_RETURN_STRING(output);
}

bool UNoCatPlugIn::checkAuthMessage(const UString& msg)
{
   U_TRACE(0, "UNoCatPlugIn::checkAuthMessage(%.*S)", U_STRING_TO_TRACE(msg))

   U_INTERNAL_ASSERT(msg)
   U_INTERNAL_ASSERT_POINTER(peer)

   uint32_t pos;
   time_t timeout;
   Url destination;
   int64_t traffic;
   bool result = false;
   UHashMap<UString> args;
   UString action, redirect;

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
      U_SRV_LOG("WARNING: different MAC in ticket from peer: IP %.*s MAC %.*s", U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));

      goto end;
      }

   // check token

   if (peer->token != args.at(U_CONSTANT_TO_PARAM("Token")))
      {
      U_SRV_LOG("WARNING: tampered token from peer: IP %.*s MAC %.*s", U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));

      /* ---------------------------------------------------------------------------------------------------------------------
      // NB: tutto il traffico viene rediretto sulla 80 (CAPTIVE PORTAL) e quindi windows update, antivrus, etc...
      // questo introduce la possibilita' che durante la fase di autorizzazione il token generato per il ticket autorizzativo
      // non corrisponda piu' a quello inviato dal portale per l'autorizzazione...
      // ---------------------------------------------------------------------------------------------------------------------
      */
      peer->token.snprintf("%u", u_random(u_now->tv_usec));

      goto end;
      }

   /* get mode

   mode = args.at(U_CONSTANT_TO_PARAM("Mode")).copy();

   U_INTERNAL_DUMP("mode = %.*S", U_STRING_TO_TRACE(mode))
   */

   // check user id

   peer->user = args.at(U_CONSTANT_TO_PARAM("User")).copy();

   pos = vLoginValidate->find(peer->user);

   if (pos != U_NOT_FOUND)
      {
      U_SRV_LOG("Validation of user id %.*S in ticket from peer: IP %.*s MAC %.*s",
                  U_STRING_TO_TRACE(peer->user), U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));

      vLoginValidate->erase(pos);
      }

   // get redirect (destination)

   redirect = args.at(U_CONSTANT_TO_PARAM("Redirect")).copy();

   if (redirect)
      {
      UVector<UString> name_value;

      destination.set(redirect);

      if (destination.getQuery(name_value) == 0)
         {
         U_SRV_LOG("WARNING: can't make sense of Redirect: %.*S in ticket from peer: IP %.*s MAC %.*s",
                     U_URL_TO_TRACE(destination), U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));

         goto end;
         }

      destination.eraseQuery();

      if (destination.setQuery(name_value) == false)
         {
         U_SRV_LOG("WARNING: error on setting Redirect: %.*S in ticket from peer: IP %.*s MAC %.*s",
                        U_URL_TO_TRACE(destination), U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));

         goto end;
         }

      (void) location->replace(destination.get());
      }

   // check for user policy FLAT

   if (args.at(U_CONSTANT_TO_PARAM("Policy")).equal(U_CONSTANT_TO_PARAM("FLAT"))) U_peer_policy_flat = true;

   // check for max time no traffic

   U_peer_max_time_no_traffic = args.at(U_CONSTANT_TO_PARAM("NoTraffic")).strtol();

   if (U_peer_max_time_no_traffic == 0) U_peer_max_time_no_traffic = (check_expire / 60) * (U_peer_policy_flat ? 3 : 1);

   // get time available

   timeout = args.at(U_CONSTANT_TO_PARAM("Timeout")).strtol();

   if (timeout ||
       peer->time_remain == 0)
      {
      if (timeout) peer->time_remain = timeout;
      else         peer->time_remain = (U_peer_policy_flat ? 24L : 2L) * 3600L; // default value...
      }

   U_INTERNAL_DUMP("time_remain = %ld", peer->time_remain)

   // get traffic available

   traffic = args.at(U_CONSTANT_TO_PARAM("Traffic")).strtoll();

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
      if (U_peer_status == UModNoCatPeer::PEER_PERMIT)
         {
         U_SRV_LOG("WARNING: peer already permit: IP %.*s MAC %.*s", U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));
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

      result = true;
      }
   else if (action.equal(U_CONSTANT_TO_PARAM("Deny")))
      {
      (void) peer->checkPeerInfo(true);

      deny(false);

      result = true;
      }
   else
      {
      U_SRV_LOG("WARNING: can't make sense of Action: %.*S in ticket from peer: IP %.*s MAC %.*s",
                  U_STRING_TO_TRACE(action), U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));
      }

end:
   args.clear();
   args.deallocate();

   U_RETURN(result);
}

void UNoCatPlugIn::creatNewPeer()
{
   U_TRACE(0, "UNoCatPlugIn::creatNewPeer()")

   U_INTERNAL_ASSERT_EQUALS(peer, 0)

   U_INTERNAL_DUMP("num_peers_preallocate = %u", num_peers_preallocate)

   if (num_peers_preallocate)
      {
      U_INTERNAL_ASSERT_POINTER(peers_preallocate)

      peer = peers_preallocate + --num_peers_preallocate;
      }
   else
      {
   // U_WRITE_MEM_POOL_INFO_TO("mempool.%N.%P.creatNewPeer", 0)

      peer = U_NEW(UModNoCatPeer);

      if (peer == 0)
         {
         unsigned long vsz, rss;

         u_get_memusage(&vsz, &rss);

         double _vsz = (double)vsz / (1024.0 * 1024.0),
                _rss = (double)rss / (1024.0 * 1024.0);

         U_ERROR("cannot allocate %u bytes (%u KB) of memory - "
                     "address space usage: %.2f MBytes - "
                               "rss usage: %.2f MBytes\n",
                     sizeof(UModNoCatPeer),
                     sizeof(UModNoCatPeer) / 1024, _vsz, _rss);
         }
      }
}

bool UNoCatPlugIn::creatNewPeer(uint32_t index_AUTH)
{
   U_TRACE(0, "UNoCatPlugIn::creatNewPeer(%u)", index_AUTH)

   U_INTERNAL_ASSERT_EQUALS(peer, 0)

   uint32_t index_network = UIPAllow::find(UServer_Base::client_address, *vLocalNetworkMask);

   U_INTERNAL_DUMP("index_network = %u", index_network)

   if (index_network == U_NOT_FOUND)
      {
      U_SRV_LOG("WARNING: IP address for new peer %.*S not found in LocalNetworkMask %.*S", U_CLIENT_ADDRESS_TO_TRACE, U_STRING_TO_TRACE(*localnet));

      U_RETURN(false);
      }

   creatNewPeer();

   U_peer_allowed       = false;
   U_peer_index_AUTH    = index_AUTH;
   U_peer_index_network = index_network;

   *((UIPAddress*)peer) = UClientImage_Base::psocket->remoteIPAddress();

   (void) peer->ip.assign(peer->UIPAddress::pcStrAddress);

   U_ASSERT(peer->ip.equal(U_CLIENT_ADDRESS_TO_PARAM))

   (void) peer->user.assign(U_CONSTANT_TO_PARAM("anonymous"));

   if (USocketExt::getARPCache(*arp_cache, *varp_cache) ||
       (check_type & U_CHECK_MAC) != 0)
      {
      for (uint32_t i = 0, n = varp_cache->size(); i < n; i += 3)
         {
         if ((*varp_cache)[i].equal(peer->ip))
            {
            peer->mac    = (*varp_cache)[i+1].copy();
            peer->ifname = (*varp_cache)[i+2].copy();

            U_peer_index_device = vInternalDevice->find(peer->ifname);

            U_INTERNAL_ASSERT(peer->mac)

            U_ASSERT_EQUALS(peer->mac, USocketExt::getMacAddress(peer->ip))

            break;
            }
         }
      }

   setNewPeer();

   U_RETURN(true);
}

void UNoCatPlugIn::setNewPeer()
{
   U_TRACE(0, "UNoCatPlugIn::setNewPeer()")

   U_INTERNAL_ASSERT_POINTER(peer)

   U_INTERNAL_DUMP("peer->UIPAddress::pcStrAddress = %S U_peer_index_network = %u U_peer_index_AUTH = %u",
                    peer->UIPAddress::pcStrAddress,     U_peer_index_network,     U_peer_index_AUTH)

   if (U_peer_index_network == 0xFF)
      {
      U_ERROR("IP address for new peer %.*S not found in LocalNetworkMask %.*S", U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(*localnet));
      }

   peer->label = ((uint32_t)U_peer_index_network >= vLocalNetworkLabel->size()
                              ? *str_without_label
                              : (*vLocalNetworkLabel)[U_peer_index_network]);

   U_INTERNAL_DUMP("peer->label = %.*S", U_STRING_TO_TRACE(peer->label))

   UIPAllow* pallow = vLocalNetworkMask->at(U_peer_index_network);

   UString x = (pallow->isEmpty() == false || UIPAllow::getNetworkInterface(*vLocalNetworkMask) ? pallow->host : *UServer_Base::IP_address);

   peer->gateway.snprintf("%.*s:%d", U_STRING_TO_TRACE(x), UServer_Base::port);

   U_INTERNAL_DUMP("peer->gateway = %.*S", U_STRING_TO_TRACE(peer->gateway))

   if (pallow->device &&
       peer->ifname != pallow->device)
      {
      U_SRV_LOG("WARNING: device different (%.*s=>%.*s) for peer: IP %.*s MAC %.*s",
                     U_STRING_TO_TRACE(peer->ifname), U_STRING_TO_TRACE(pallow->device),
                     U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));

      peer->ifname = pallow->device;

      U_peer_index_device = vInternalDevice->find(peer->ifname);
      }

   U_INTERNAL_DUMP("peer->ifname = %.*S U_peer_index_device = %u", U_STRING_TO_TRACE(peer->ifname), U_peer_index_device)

   // NB: request(arp|deny|clear|reset|permit|openlist|initialize) mac ip class(Owner|Member|Public) UserDownloadRate UserUploadRate

   setFireWallCommand(peer->fw, *fw_cmd, peer->mac, peer->ip);

   /* ---------------------------------------------------------------------------------------------------------------------
   // NB: tutto il traffico viene rediretto sulla 80 (CAPTIVE PORTAL) e quindi windows update, antivrus, etc...
   // questo introduce la possibilita' che durante la fase di autorizzazione il token generato per il ticket autorizzativo
   // non corrisponda piu' a quello inviato dal portale per l'autorizzazione...
   // ---------------------------------------------------------------------------------------------------------------------
   */
   peer->token.snprintf("%u", u_random(u_now->tv_usec));

   peers->insert(peer->ip, peer);
}

void UNoCatPlugIn::checkOldPeer()
{
   U_TRACE(0, "UNoCatPlugIn::checkOldPeer()")

   U_INTERNAL_ASSERT_POINTER(peer)
   U_ASSERT(peer->ip.equal(U_CLIENT_ADDRESS_TO_PARAM))

   // NB: we get the MAC for peer IP address from the ARP cache...

   UString mac = peer->mac;

   getARPCache();

   for (uint32_t i = 0, n = varp_cache->size(); i < n; i += 3)
      {
      if ((*varp_cache)[i].equal(peer->ip))
         {
         mac = (*varp_cache)[i+1];

         U_INTERNAL_ASSERT(mac)

         U_ASSERT_EQUALS(mac, USocketExt::getMacAddress(peer->ip))

         break;
         }
      }

   if (mac != peer->mac)
      {
      // NB: we assume that the current peer is a different user that has acquired the same IP address from the DHCP...

      U_SRV_LOG("WARNING: different MAC (%.*s) from arp cache for peer: IP %.*s MAC %.*s",
                     U_STRING_TO_TRACE(mac), U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac));

      (void) peer->checkPeerInfo(true);

      deny(true);

      peer->mac = mac.copy();

      peer->fw.setArgument(4, peer->mac.data());

      // ---------------------------------------------------------------------------------------------------------------------
      // NB: tutto il traffico viene rediretto sulla 80 (CAPTIVE PORTAL) e quindi windows update, antivrus, etc...
      // questo introduce la possibilita' che durante la fase di autorizzazione il token generato per il ticket autorizzativo
      // non corrisponda piu' a quello inviato dal portale per l'autorizzazione...
      // ---------------------------------------------------------------------------------------------------------------------
      peer->token.snprintf("%u", u_random(u_now->tv_usec));
      }
}

void UNoCatPlugIn::addPeerInfo(time_t logout)
{
   U_TRACE(0, "UNoCatPlugIn::addPeerInfo(%T)", logout)

   U_INTERNAL_ASSERT_POINTER(peer)

   // $1 -> mac
   // $2 -> ip
   // $3 -> gateway
   // $4 -> ap
   // $5 -> uid
   // $6 -> logout
   // $7 -> connected
   // $8 -> traffic

   Url* info_url = (*vinfo_url)[U_peer_index_AUTH];

   info_url->setPath(U_CONSTANT_TO_PARAM("/info"));

   U_INTERNAL_DUMP("U_peer_index_AUTH = %u info_url = %p", U_peer_index_AUTH, info_url)

   U_INTERNAL_ASSERT(peer->user)

   UString buffer(U_CAPACITY);

   buffer.snprintf("%.*s@%.*s", U_STRING_TO_TRACE(peer->label), U_STRING_TO_TRACE(*UServer_Base::IP_address));

   info_url->addQuery(U_CONSTANT_TO_PARAM("Mac"),     U_STRING_TO_PARAM(peer->mac));
   info_url->addQuery(U_CONSTANT_TO_PARAM("ip"),      U_STRING_TO_PARAM(peer->ip));
   info_url->addQuery(U_CONSTANT_TO_PARAM("gateway"), U_STRING_TO_PARAM(peer->gateway));
   info_url->addQuery(U_CONSTANT_TO_PARAM("ap"),      U_STRING_TO_PARAM(buffer));
   info_url->addQuery(U_CONSTANT_TO_PARAM("User"),    U_STRING_TO_PARAM(peer->user));

   // NB: 0 => mean NOT logout (only info)
   //    -1 => disconnected (logout implicito)

   bool logout_implicito = (logout == -1);

   if (logout_implicito) logout = u_now->tv_sec;

   buffer.setFromNumber32(logout);

   if (logout_implicito) (void) buffer.insert(0, '-');

   info_url->addQuery(U_CONSTANT_TO_PARAM("logout"), U_STRING_TO_PARAM(buffer));

   U_INTERNAL_ASSERT(u_now->tv_sec >= peer->ctime)

   buffer.setFromNumber32(u_now->tv_sec - peer->ctime);
                                          peer->ctime = u_now->tv_sec;

   info_url->addQuery(U_CONSTANT_TO_PARAM("connected"), U_STRING_TO_PARAM(buffer));

   if (peer->ctraffic == 0) info_url->addQuery(U_CONSTANT_TO_PARAM("traffic"), U_CONSTANT_TO_PARAM("0"));
   else
      {
      buffer.snprintf("%u", peer->ctraffic);
                            peer->ctraffic = peer->time_no_traffic = 0;

      info_url->addQuery(U_CONSTANT_TO_PARAM("traffic"), U_STRING_TO_PARAM(buffer));
      }
}

bool UModNoCatPeer::checkPeerInfo(bool btraffic)
{
   U_TRACE(0, "UModNoCatPeer::checkPeerInfo(%b)", btraffic)

   U_INTERNAL_ASSERT_POINTER(UNoCatPlugIn::peer)

   if (U_peer_allowed == false &&
       U_peer_status  == PEER_PERMIT)
      {
      bool result = true;

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

      U_INTERNAL_DUMP("UNoCatPlugIn::peer_present_in_arp_cache = %.*S peer->ifname = %.*S",
                       U_STRING_TO_TRACE(*UNoCatPlugIn::peer_present_in_arp_cache), U_STRING_TO_TRACE(ifname))

      if (ctraffic == 0) time_no_traffic += (u_now->tv_sec - ctime);

      U_INTERNAL_DUMP("time_no_traffic = %ld", time_no_traffic)

      if (time_no_traffic)
         {
         long max_time_no_traffic = U_peer_max_time_no_traffic * 60L;

         U_SRV_LOG("Peer IP %.*s MAC %.*s has made no traffic for %ld secs - (max_time_no_traffic: %ld secs) (present in ARP cache: %s)",
                     U_STRING_TO_TRACE(ip), U_STRING_TO_TRACE(mac), time_no_traffic, max_time_no_traffic,
                     UNoCatPlugIn::peer_present_in_arp_cache->empty() ? "NO" : "yes");

         if (time_no_traffic >= max_time_no_traffic ||
             UNoCatPlugIn::peer_present_in_arp_cache->empty())
            {
            U_INTERNAL_ASSERT(u_now->tv_sec >= ctime)

            result = false;
            }
         }

      time_remain = (expire > u_now->tv_sec ? (expire - u_now->tv_sec) : 0);

      if (U_peer_policy_flat == false)
         {
         if (time_remain <= 15)
            {
            result      = false;
            logout      = expire; // NB: user with no more time or no more traffic...
            time_remain = 0;

            U_SRV_LOG("Peer IP %.*s MAC %.*s has no more time - remain traffic for %llu bytes", U_STRING_TO_TRACE(ip), U_STRING_TO_TRACE(mac), traffic_remain);
            }

         if (traffic_remain <= 1024ULL)
            {
            result         = false;
            logout         = expire; // NB: user with no more time or no more traffic...
            traffic_remain = 0;

            U_SRV_LOG("Peer IP %.*s MAC %.*s has no more traffic - remain time for %lu sec", U_STRING_TO_TRACE(ip), U_STRING_TO_TRACE(mac), time_remain);
            }
         }

      if ( UNoCatPlugIn::peer_present_in_arp_cache->empty() &&
          (UNoCatPlugIn::check_type & UNoCatPlugIn::U_CHECK_ARP_CACHE) != 0)
         {
         result = false;

         U_SRV_LOG("Peer IP %.*s MAC %.*s not present in ARP cache, I assume he is disconnected", U_STRING_TO_TRACE(ip), U_STRING_TO_TRACE(mac));
         }

      U_RETURN(result);
      }

   U_RETURN(true);
}

bool UNoCatPlugIn::checkPeerInfo(UStringRep* key, void* value)
{
   U_TRACE(0, "UNoCatPlugIn::checkPeerInfo(%.*S,%p)", U_STRING_TO_TRACE(*key), value)

   peer = (UModNoCatPeer*) value;

   if (peer->checkPeerInfo(false) == false)
      {
      deny(peer_present_in_arp_cache->empty());

      U_RETURN(true);
      }

   if (U_peer_status == UModNoCatPeer::PEER_PERMIT)
      {
      if (U_peer_allowed)
         {
      // addPeerInfo(0);

         U_RETURN(true);
         }

      U_SRV_LOG("Peer IP %.*s MAC %.*s remain: %ld secs %llu bytes",
                  U_STRING_TO_TRACE(peer->ip), U_STRING_TO_TRACE(peer->mac), peer->time_remain, peer->traffic_remain);

      uint32_t index_device = U_peer_index_device;

      if ((check_type & U_CHECK_ARP_CACHE) != 0)
         {
         U_INTERNAL_ASSERT(*peer_present_in_arp_cache)

         index_device = vInternalDevice->find(*peer_present_in_arp_cache);
         }

      U_INTERNAL_DUMP("index_device = %u U_peer_index_device = %u", index_device, U_peer_index_device)

      U_INTERNAL_ASSERT(index_device <= num_radio)

      if (index_device != U_NOT_FOUND)
         {
         FD_SET(nfds, &addrmask);

         ++nfds;

         vaddr[index_device]->push(peer);
         }
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

void UNoCatPlugIn::notifyAuthOfUsersInfo(uint32_t index_AUTH)
{
   U_TRACE(0, "UNoCatPlugIn::notifyAuthOfUsersInfo(%u)", index_AUTH)

   Url* info_url = (*vinfo_url)[index_AUTH];

   U_INTERNAL_DUMP("index_AUTH = %u info_url = %p", index_AUTH, info_url)

   // NB: if there are some data to transmit we need redirect...

   if (info_url->isQuery())
      {
      // NB: we need to send a body for portal discrimination...

      UHTTP::setRedirectResponse(0, UString::getStringNull(), U_URL_TO_PARAM(*info_url));

      // NB: we assume that the redirect always have success...

      *info_url = *((*vauth_url)[index_AUTH]);
      }
   else
      {
      // NB: if there is arping pending AUTH must recall us after ~15sec for completion...

      u_http_info.nResponseCode = (isPingAsyncPending() ? HTTP_NO_CONTENT : HTTP_NOT_MODIFIED);

      UClientImage_Base::setCloseConnection();

      UHTTP::setResponse(0, 0);
      }
}

void UNoCatPlugIn::setHTTPResponse(const UString& content)
{
   U_TRACE(0, "UNoCatPlugIn::setHTTPResponse(%.*S)", U_STRING_TO_TRACE(content))

   if (content.empty())
      {
      u_http_info.nResponseCode = HTTP_NO_CONTENT;

      UHTTP::setResponse(0, 0);
      }
   else
      {
      u_http_info.endHeader       = 0;
      u_http_info.nResponseCode   = HTTP_OK;
      *UClientImage_Base::wbuffer = content;

      UHTTP::mime_index = (u_isHTML(content.data()) ? U_html : U_unknow);

      UHTTP::setCgiResponse();
      }
}

bool UNoCatPlugIn::getPeerFromMAC(const UString& mac)
{
   U_TRACE(0, "UNoCatPlugIn::getPeerFromMAC(%.*S)", U_STRING_TO_TRACE(mac))

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

   char c;
   uint32_t i;
   UString x(U_INET_ADDRSTRLEN);

   for (i = 0; i < len; ++i)
      {
      c = ptr[i];

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

   // -----------------------------------------------------------------------------------------------------------------------------------
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

      tmp          = cfg.at(U_CONSTANT_TO_PARAM("LOGIN_TIMEOUT"));
      *fw_cmd      = cfg.at(U_CONSTANT_TO_PARAM("FW_CMD"));
      lnetlbl      = cfg.at(U_CONSTANT_TO_PARAM("LOCAL_NETWORK_LABEL"));
   // *decrypt_cmd = cfg.at(U_CONSTANT_TO_PARAM("DECRYPT_CMD"));
      *decrypt_key = cfg.at(U_CONSTANT_TO_PARAM("DECRYPT_KEY"));

      check_type            = cfg.readLong(U_CONSTANT_TO_PARAM("CHECK_TYPE"));
      check_expire          = cfg.readLong(U_CONSTANT_TO_PARAM("CHECK_EXPIRE_INTERVAL"));
      num_peers_preallocate = cfg.readLong(U_CONSTANT_TO_PARAM("NUM_PEERS_PREALLOCATE"), 512);

      if (tmp)
         {
         char* ptr;

         login_timeout = U_NEW(UString(tmp));

         time_available = strtol(login_timeout->data(), &ptr, 0);

         if (time_available > U_ONE_DAY_IN_SECOND) time_available = U_ONE_DAY_IN_SECOND; // check for safe timeout...

         U_INTERNAL_DUMP("ptr[0] = %C", ptr[0])

         if (ptr[0] == ':') traffic_available = strtoll(ptr+1, 0, 0);

         if (traffic_available == 0) traffic_available = 4ULL * 1024ULL * 1024ULL * 1024ULL; // 4G

         U_INTERNAL_DUMP("time_available = %u traffic_available = %llu", time_available, traffic_available)
         }

      if (check_expire) UTimeVal::setSecond(check_expire);

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

      if (tmp) port = tmp.strtol();

      /*
      =========================================================================================
      NON piace a Ghivizzani
      =========================================================================================
      UString pathconf = UStringExt::getEnvironmentVar(U_CONSTANT_TO_PARAM("FW_CONF"), fw_env);

      if (pathconf)
         {
         // UploadRate=10240
         // DownloadRate=10240
         // UserUploadRate=1024
         // UserDownloadRate=10240
         // EnableBandwidthLimits=1
         // 
         // DosPrevention=0
         // DosTimeInterval=60
         // DosAllowTries=5

         UString data      = UFile::contentOf(pathconf);
         const char* end   = data.end();
         const char* start = data.data();

         cfg.clear();

         if (UFileConfig::loadProperties(cfg.table, start, end) == false) goto err;
         }
      =========================================================================================
      */

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
      *allowed_members = UFile::contentOf(allowed_members->empty() ? *str_allowed_members_default : *allowed_members);
      }

   *label = (vLocalNetworkLabel->empty() ? *str_without_label : (*vLocalNetworkLabel)[0]);

   U_INTERNAL_DUMP("label = %.*S", U_STRING_TO_TRACE(*label))

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

int UNoCatPlugIn::handlerInit()
{
   U_TRACE(0, "UNoCatPlugIn::handlerInit()")

   if (fw_cmd->empty()) U_RETURN(U_PLUGIN_HANDLER_ERROR);

// U_PRINT_SIZEOF(UModNoCatPeer);

   Url* url;
   UIPAddress addr;
   UString auth_ip, ip;

   // NB: get IP address of AUTH hosts...

   for (uint32_t i = 0, n = vauth->size(); i < n; ++i)
      {
      ip = (*vauth)[i];
      url = U_NEW(Url(ip));

      vauth_url->push(url);

      auth_ip = url->getHost();

      if (addr.setHostName(auth_ip.copy(), UClientImage_Base::bIPv6) == false)
         {
         U_SRV_LOG("WARNING: unknown AUTH host %.*S", U_STRING_TO_TRACE(auth_ip));
         }
      else
         {
         (void) auth_ip.replace(addr.getAddressString());

         U_SRV_LOG("AUTH host registered: %.*s", U_STRING_TO_TRACE(auth_ip));

         vauth_ip->push(auth_ip);
         }

      url = U_NEW(Url(ip));

      vinfo_url->push(url);
      }

   U_ASSERT_EQUALS(vauth->size(), vauth_ip->size())
 
   *hostname = USocketExt::getNodeName();

   U_INTERNAL_DUMP("host = %.*s:%d hostname = %.*S ip = %.*S", U_STRING_TO_TRACE(*UServer_Base::IP_address),
                        UServer_Base::port, U_STRING_TO_TRACE(*hostname), U_STRING_TO_TRACE(*UServer_Base::IP_address))

   UUDPSocket cClientSocket(UClientImage_Base::bIPv6);

   auth_ip = (*vauth_ip)[0];

   if (cClientSocket.connectServer(auth_ip, 1001))
      {
      ip = UString(cClientSocket.getLocalInfo());

      if (ip != *UServer_Base::IP_address)
         {
         (void) UFile::writeToTmp(U_STRING_TO_PARAM(ip), false, "IP_ADDRESS", 0);

         U_SRV_LOG("WARNING: SERVER IP ADDRESS differ from IP address: %.*S to connect to AUTH: %.*S", U_STRING_TO_TRACE(ip), U_STRING_TO_TRACE(auth_ip));
         }
      }

   auth_ip = vauth_ip->join(U_CONSTANT_TO_PARAM(" "));

   fw_env->snprintf_add("'AuthServiceIP=%.*s'\n", U_STRING_TO_TRACE(auth_ip));

   U_INTERNAL_ASSERT_EQUALS(UPing::addrmask, 0)

   UPing::addrmask = (fd_set*) UServer_Base::getOffsetToDataShare(sizeof(fd_set) + sizeof(uint32_t));

   // crypto cmd

// if (*decrypt_cmd) pgp.set(decrypt_cmd, (char**)0);
// if (*decrypt_key)
      {
      U_INTERNAL_ASSERT(decrypt_key->isNullTerminated())
      
      UDES3::setPassword(decrypt_key->data());
      }

   // firewall cmd

   UString command(500U);

   command.snprintf("/bin/sh %.*s initialize allowed_web_hosts", U_STRING_TO_TRACE(*fw_cmd));

   fw->set(command, (char**)0);
   fw->setEnvironment(fw_env);

   if (fd_stderr == 0) fd_stderr = UServices::getDevNull("/tmp/server_plugin_nocat.err");

   // users table

   peers = U_NEW(UHashMap<UModNoCatPeer*>(U_GET_NEXT_PRIME_NUMBER(1024)));

   openlist       = U_NEW(UVector<UString>);
   status_content = U_NEW(UString(U_CAPACITY));
 
   // users traffic

   ipt = U_NEW(UIptAccount(UClientImage_Base::bIPv6));

   U_INTERNAL_DUMP("num_peers_preallocate = %u", num_peers_preallocate)

   U_INTERNAL_ASSERT_EQUALS(peers_preallocate, 0)

   UInterrupt::setHandlerForSegv(preallocatePeers, preallocatePeersFault);

   U_INTERNAL_DUMP("num_peers_preallocate = %u peers_preallocate = %p", num_peers_preallocate, peers_preallocate)

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

bool UNoCatPlugIn::preallocatePeersFault()
{
   U_TRACE(0, "UNoCatPlugIn::preallocatePeersFault()")

   // send msg to portal

   UString msg(200U);

   unsigned long vsz, rss;

   u_get_memusage(&vsz, &rss);

   double _vsz = (double)vsz / (1024.0 * 1024.0),
          _rss = (double)rss / (1024.0 * 1024.0);

   msg.snprintf("/error_ap?ap=%.*s@%.*s&public=%.*s:%u&msg="
                           "address+space+usage:+%.2f+MBytes+-+"
                                     "rss+usage:+%.2f+MBytes",
                  U_STRING_TO_TRACE(*label), U_STRING_TO_TRACE(*hostname),
                  U_STRING_TO_TRACE(*UServer_Base::IP_address), UServer_Base::port, _vsz, _rss);

   sendMsgToPortal(0, msg, 0, false);

   U_INTERNAL_ASSERT_EQUALS(peers_preallocate, 0)

   if (num_peers_preallocate > 256)
      {
      num_peers_preallocate = 256;

      U_RETURN(true);
      }

   U_RETURN(false);
}

int UNoCatPlugIn::handlerFork()
{
   U_TRACE(0, "UNoCatPlugIn::handlerFork()")

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
      vaddr[i] = U_NEW(UVector<UIPAddress*>);
      sockp[i] = U_NEW(UPing(3000, UClientImage_Base::bIPv6));

      if (((check_type & U_CHECK_ARP_PING) != 0))
         {
         sockp[i]->setLocal(UServer_Base::socket->localIPAddress());

#     ifdef HAVE_NETPACKET_PACKET_H
         U_INTERNAL_ASSERT((*vInternalDevice)[i].isNullTerminated())

         sockp[i]->initArpPing((*vInternalDevice)[i].data());
#     endif
         }
      }

   // manage internal timer...

   if (login_timeout ||
       UTimeVal::notZero())
      {
      UTimer::init(true); // async...

      U_SRV_LOG("Initialization of timer success");

      if (UTimeVal::notZero())
         {
         UTimer::insert(this, true);

         U_SRV_LOG("Monitoring set for every %d secs", UTimeVal::getSecond());
         }
      }

   // send start to portal

   U_INTERNAL_ASSERT_EQUALS(UClient_Base::queue_dir, 0)

   UString msg(300U), output(U_CAPACITY), allowed_web_hosts(U_CAPACITY);

   msg.snprintf("/start_ap?ap=%.*s@%.*s&public=%.*s:%u&pid=%u",
                     U_STRING_TO_TRACE(*label), U_STRING_TO_TRACE(*hostname),
                     U_STRING_TO_TRACE(*UServer_Base::IP_address), UServer_Base::port, UServer_Base::pid);

   for (i = 0, n = (*vinfo_url).size(); i < n; ++i)
      {
      sendMsgToPortal(i, msg, &output, false);

      (void) allowed_web_hosts.append(output);
      }

   // initialize the firewall: direct all port 80 traffic to us...

   fw->setArgument(4, allowed_web_hosts.data());

   (void) fw->executeAndWait(0, -1, fd_stderr);

#ifdef U_LOG_ENABLE
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
      uint32_t index_network, increment = ((n % 3) ? 5 : 3);

      for (i = 0; i < n; i += increment)
         {
         ip            = vtmp[i+1];
         index_network = UIPAllow::find(ip, *vLocalNetworkMask);

         U_INTERNAL_DUMP("index_network = %u", index_network)

         if (index_network == U_NOT_FOUND)
            {
            U_SRV_LOG("WARNING: IP address for allowed_members %.*S not found in LocalNetworkMask %.*S",
                           U_STRING_TO_TRACE(ip), U_STRING_TO_TRACE(*localnet));

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

#if defined(U_LOG_ENABLE) && defined(USE_LIBZ)
   if (UServer_Base::isLog()) dirwalk = U_NEW(UDirWalk(ULog::getDirLogGz(), U_CONSTANT_TO_PARAM("*.gz")));
#endif

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

// Connection-wide hooks

int UNoCatPlugIn::handlerRequest()
{
   U_TRACE(1, "UNoCatPlugIn::handlerRequest()")

   if (UHTTP::file->isRoot() ||
       UClientImage_Base::isRequestNotFound())
      {
      Url url;
      int mode = UHTTP::NETWORK_AUTHENTICATION_REQUIRED;
      UString host(U_HTTP_HOST_TO_PARAM), buffer(U_CAPACITY);

      U_INTERNAL_DUMP("host = %.*S UServer_Base::client_address = %.*S", U_STRING_TO_TRACE(host), U_CLIENT_ADDRESS_TO_TRACE)

      peer = 0;

      // -----------------------------------------------------
      // NB: check for request from AUTH, which may be:
      // -----------------------------------------------------
      // 1) /checkFirewall - check firewall and report info
      // 2) /check         - check system and report info
      // 3) /uptime        - report uptime info 
      // 4) /status        - report status user
      // 5) /logout        - logout specific user
      // 6) /users         - report list ip of peers permitted
      // -----------------------------------------------------

      U_INTERNAL_DUMP("vauth_ip = %S", UObject2String(*vauth_ip))

      uint32_t index_AUTH = vauth_ip->find(U_CLIENT_ADDRESS_TO_PARAM);

      if (index_AUTH != U_NOT_FOUND)
         {
         U_SRV_LOG("AUTH(%u) request: %.*S", index_AUTH, U_HTTP_URI_TO_TRACE);

         if (U_HTTP_URI_STREQ("/checkFirewall"))
            {
            if (flag_check_system == false)
               {
               getARPCache();

               checkFirewall();
               }

            notifyAuthOfUsersInfo(index_AUTH);

            U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
            }

         if (U_HTTP_URI_STREQ("/check"))
            {
            if (flag_check_system == false) checkSystem();

            notifyAuthOfUsersInfo(index_AUTH);

            U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
            }

         if (U_HTTP_URI_STREQ("/uptime"))
            {
            status_content->setBuffer(U_CAPACITY);

            status_content->snprintf("%u", u_get_uptime());

            setHTTPResponse(*status_content);

            U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
            }

         if (U_HTTP_URI_STREQ("/status"))
            {
            UString ap_label;

            if (U_HTTP_QUERY_MEMEQ("ip="))
               {
               // NB: request from AUTH to get status user

               uint32_t len    = u_http_info.query_len - U_CONSTANT_SIZE("ip=");
               const char* ptr = u_http_info.query     + U_CONSTANT_SIZE("ip=");

               UString peer_ip = getIPAddress(ptr, len);

               U_SRV_LOG("AUTH request to get status for user: IP %.*s", U_STRING_TO_TRACE(peer_ip));

               peer = (*peers)[peer_ip];

               U_INTERNAL_DUMP("peer = %p", peer)

               if (peer == 0)
                  {
                  UHTTP::setBadRequest();

                  U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
                  }
               }
            else if (U_HTTP_QUERY_MEMEQ("label="))
               {
               // NB: request from AUTH to get status users...

               uint32_t len    = u_http_info.query_len - U_CONSTANT_SIZE("label=");
               const char* ptr = u_http_info.query     + U_CONSTANT_SIZE("label=");

               (void) ap_label.assign(ptr, len);

               U_SRV_LOG("AUTH request to get status for users: ap %.*S", U_STRING_TO_TRACE(ap_label));
               }

            // status

            getARPCache();

            setStatusContent(ap_label); // NB: peer == 0 -> request from AUTH to get status access point...

            U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
            }

         if (U_HTTP_URI_STREQ("/logout") &&
             u_http_info.query_len)
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

            U_SRV_LOG("AUTH request to logout user: IP %.*s", U_STRING_TO_TRACE(peer_ip));

            peer = (*peers)[peer_ip];

            if (peer == 0)
               {
               uint32_t pos = data.find_first_of('&', 3);

               if (pos != U_NOT_FOUND &&
                   strncmp(data.c_pointer(pos+1), U_CONSTANT_TO_PARAM("mac=")) == 0)
                  {
                  UString mac = data.substr(pos + 5, U_CONSTANT_SIZE("00:00:00:00:00:00"));

                  U_SRV_LOG("AUTH request to logout user: MAC %.*s", U_STRING_TO_TRACE(mac));

                  if (mac != *UString::str_without_mac &&
                      getPeerFromMAC(mac))
                     {
                     goto next;
                     }
                  }
               }

            if (peer == 0) UHTTP::setBadRequest();
            else
               {
next:          getARPCache();

               if (U_peer_status != UModNoCatPeer::PEER_PERMIT)
                  {
                  setStatusContent(UString::getStringNull()); // NB: peer == 0 -> request from AUTH to get status access point...
                  }
               else
                  {
                  (void) peer->checkPeerInfo(true);

                  deny(false);

                  notifyAuthOfUsersInfo(index_AUTH);
                  }
               }

            U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
            }

         if (U_HTTP_URI_STREQ("/users"))
            {
            // NB: request from AUTH to get list info on peers permitted

            status_content->setBuffer(U_CAPACITY);

            peers->callForAllEntry(getPeerListInfo);

            setHTTPResponse(*status_content);

            U_SRV_LOG("AUTH request to get list info on peers permitted");

            U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
            }

         UHTTP::setBadRequest();

         U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
         }

      // ---------------------------
      // NB: check request from user
      // ---------------------------

      index_AUTH = getIndexAUTH(UServer_Base::client_address);
      url        = *((*vauth_url)[index_AUTH]);
      peer       = peers->at(U_CLIENT_ADDRESS_TO_PARAM);

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
         // it was decided to give it one.

#     ifdef DEBUG
         U_SRV_LOG("Detected strange initial WiFi request (iPhone) from peer: IP %.*s", U_CLIENT_ADDRESS_TO_TRACE);
#     endif

         /*
         if (u_find(U_HTTP_USER_AGENT_TO_PARAM, U_CONSTANT_TO_PARAM("CaptiveNetworkSupport")) != 0)
            {
            setHTTPResponse(str_IPHONE_SUCCESS);

            U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
            }
         */
            
         (void) buffer.assign(U_CONSTANT_TO_PARAM("IPHONE"));

         goto set_redirect_to_AUTH;
         }

      if (U_HTTP_USER_AGENT_STREQ("TMUFE"))
         {
#     ifdef DEBUG
         U_SRV_LOG("Detected User-Agent: TMUFE (AntiVirus) from peer: IP %.*s", U_CLIENT_ADDRESS_TO_TRACE);
#     endif

         UHTTP::setForbidden();

         U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
         }

      // ---------------------------------------------------------------
      // NB: other kind of message (from user), which may be:
      // ---------------------------------------------------------------
      // a) /cpe            - specific request, force redirect via https 
      // b) /test           - force redirect even without a firewall
      // e) /ticket         - authorization ticket with info
      // h) /login_validate - before authorization ticket with info
      // ---------------------------------------------------------------

      if (U_HTTP_URI_STREQ("/cpe"))
         {
         (void) buffer.assign(U_CONSTANT_TO_PARAM("http://www.google.com"));

         url.setPath(U_CONSTANT_TO_PARAM("/cpe"));
         url.setService(U_CONSTANT_TO_PARAM("https"));

         goto set_redirect_to_AUTH;
         }

      if (U_HTTP_URI_STREQ("/test"))
         {
         (void) buffer.assign(U_CONSTANT_TO_PARAM("http://www.google.com"));

         goto set_redirect_to_AUTH;
         }

      if (U_HTTP_URI_STREQ("/login_validate") &&
          u_http_info.query_len)
         {
         // user has pushed the login button

         UString data = getSignedData(U_HTTP_QUERY_TO_PARAM);

         if (data.empty())
            {
            U_SRV_LOG("WARNING: tampered request to validate login from peer: IP %.*s", U_CLIENT_ADDRESS_TO_TRACE);

            goto set_redirection_url;
            }

         UString _uid;
         uint32_t len    = data.size() - U_CONSTANT_SIZE("uid=");
         const char* ptr = data.data() + U_CONSTANT_SIZE("uid=");

         (void) buffer.assign(ptr, len);

         len = buffer.find('&');

         if (len == U_NOT_FOUND) (void) _uid.replace(buffer);
         else                    (void) _uid.assign(ptr, len);

         U_SRV_LOG("request to validate login for uid %.*S from peer: IP %.*s", U_STRING_TO_TRACE(_uid), U_CLIENT_ADDRESS_TO_TRACE);

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

         uint32_t len    = u_http_info.query_len - U_CONSTANT_SIZE("ticket=");
         const char* ptr = u_http_info.query     + U_CONSTANT_SIZE("ticket=");

         UString data = getSignedData(ptr, len);

         if (data.empty())
            {
            U_SRV_LOG("WARNING: invalid ticket from peer: IP %.*s", U_CLIENT_ADDRESS_TO_TRACE);

            goto set_redirection_url;
            }

#     ifdef U_LOG_ENABLE
         if (UServer_Base::isLog())
            {
            UString printable(data.size() * 4);

            UEscape::encode(data, printable, false);

            ULog::log("%sauth message: %.*s", UServer_Base::mod_name[0], U_STRING_TO_TRACE(printable));
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
          host           == peer->gateway)
         {
         U_SRV_LOG("WARNING: missing ticket from peer: IP %.*s", U_CLIENT_ADDRESS_TO_TRACE);
         }

set_redirection_url:
      (void) buffer.reserve(7 + U_http_host_len + u_http_info.uri_len);

      buffer.snprintf("http://%.*s%.*s", U_STRING_TO_TRACE(host), U_HTTP_URI_TO_TRACE);

set_redirect_to_AUTH:
           if (peer) checkOldPeer();
      else if (creatNewPeer(index_AUTH) == false)
         {
         UClientImage_Base::resetAndClose();

         U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
         }

      if (mode == UHTTP::NO_BODY          && // login_validate
          (check_type & U_CHECK_MAC) != 0 &&
          peer->mac == *UString::str_without_mac)
         {
         mode = UHTTP::REFRESH | UHTTP::NO_BODY;

         (void) location->reserve(7 + U_http_host_len + u_http_info.uri_len + 1 + u_http_info.query_len);

         location->snprintf("http://%.*s/login_validate?%.*s", U_STRING_TO_TRACE(host), U_HTTP_QUERY_TO_TRACE);

         U_SRV_LOG("WARNING: missing MAC from peer: IP %.*s - redirect to %.*S", U_CLIENT_ADDRESS_TO_TRACE,  U_STRING_TO_TRACE(*location));
         }
      else
         {
         setRedirectLocation(buffer, url);
         }

redirect:
      UHTTP::setRedirectResponse(mode, UString::getStringNull(), U_STRING_TO_PARAM(*location));

      U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
      }

   U_RETURN(U_PLUGIN_HANDLER_GO_ON);
}

int UNoCatPlugIn::handlerReset()
{
   U_TRACE(0, "UNoCatPlugIn::handlerReset()")

   U_INTERNAL_DUMP("U_http_method_type = %u", U_http_method_type)

   checkSystem();

   U_http_method_type = 0;

   UServer_Base::resetCallerHandlerReset();

   U_RETURN(U_PLUGIN_HANDLER_PROCESSED | U_PLUGIN_HANDLER_GO_ON);
}

// DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
const char* UModNoCatPeer::dump(bool _reset) const
{
   *UObjectIO::os << "expire                " << expire            <<  '\n'
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
                  << "login_timeout        (UString                  " << (void*)login_timeout        << ")\n"
                  << "status_content       (UString                  " << (void*)status_content       << ")\n"
                  << "allowed_members      (UString                  " << (void*)allowed_members      << ")\n"
                  << "fw                   (UCommand                 " << (void*)fw                   << ")\n"
                  << "ipt                  (UIptAccount              " << (void*)ipt                  << ")\n"
                  << "vauth_url            (UVector<Url*>            " << (void*)vauth_url            << ")\n"
                  << "vinfo_url            (UVector<Url*>            " << (void*)vinfo_url            << ")\n"
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
