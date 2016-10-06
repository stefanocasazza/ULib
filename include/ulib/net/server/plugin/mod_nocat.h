// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_nocat.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_NOCAT_H
#define U_MOD_NOCAT_H 1

#include <ulib/url.h>
#include <ulib/timer.h>
#include <ulib/command.h>
#include <ulib/net/ping.h>
#include <ulib/net/tcpsocket.h>
#include <ulib/net/client/http.h>
#include <ulib/net/server/server_plugin.h>

enum LogoutType {
   U_LOGOUT_NO_TRAFFIC           = 1,
   U_LOGOUT_NO_ARP_CACHE         = 2,
   U_LOGOUT_NO_ARP_REPLY         = 3,
   U_LOGOUT_NO_MORE_TIME         = 4,
   U_LOGOUT_NO_MORE_TRAFFIC      = 5,
   U_LOGOUT_CHECK_FIREWALL       = 6, 
   U_LOGOUT_REQUEST_FROM_AUTH    = 7,
   U_LOGOUT_DIFFERENT_MAC_FOR_IP = 8 
};

class UDirWalk;
class UIptAccount;
class UNoCatPlugIn;

// sizeof(UModNoCatPeer) 32bit == 216

class UModNoCatPeer : public UEventTime, UIPAddress {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   void* operator new(                 size_t sz) { U_TRACE(0, "UModNoCatPeer::operator new(%u)",   sz) return U_SYSCALL(malloc, "%u", sz); }
   void* operator new[](               size_t sz) { U_TRACE(0, "UModNoCatPeer::operator new[](%u)", sz) return U_SYSCALL(malloc, "%u", sz); }
   void  operator delete(  void* _ptr, size_t sz) { U_TRACE(0, "UModNoCatPeer::operator delete(%p,%u)",   _ptr, sz) U_SYSCALL_VOID(free, "%p", _ptr); }
   void  operator delete[](void* _ptr, size_t sz) { U_TRACE(0, "UModNoCatPeer::operator delete[](%p,%u)", _ptr, sz) U_SYSCALL_VOID(free, "%p", _ptr); } 

   void init()
      {
      U_TRACE_NO_PARAM(0, "UModNoCatPeer::init()")

      next = 0;

      ctime        = connected = expire = u_now->tv_sec;
      ctraffic     = time_no_traffic    = time_remain = logout = 0L;
      traffic_done = traffic_available  = traffic_remain = 0ULL;

      (void) U_SYSCALL(memset, "%p,%d,%u", flag, 0, sizeof(flag));
      }

   enum Status { PEER_DENY, PEER_PERMIT };

   UModNoCatPeer() : UEventTime(0L,1L), mac(*UString::str_without_mac)
      {
      U_TRACE_REGISTER_OBJECT(0, UModNoCatPeer, "", 0)

      init();
      }

   virtual ~UModNoCatPeer()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UModNoCatPeer)
      }

   // define method VIRTUAL of class UEventTime

   virtual int handlerTime() U_DECL_FINAL;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UCommand fw;
   UModNoCatPeer* next;
   UString ip, mac, token, user, ifname, label, gateway;
   uint64_t traffic_done, traffic_available, traffic_remain;
   long connected, expire, logout, ctime, time_no_traffic, time_remain;
   unsigned char flag[8];
   uint32_t ctraffic;

   int checkPeerInfo(bool btraffic);

private:
   U_DISALLOW_ASSIGN(UModNoCatPeer)

   friend class UNoCatPlugIn;
};

#define U_peer_status              (UNoCatPlugIn::peer)->UModNoCatPeer::flag[0]
#define U_peer_allowed             (UNoCatPlugIn::peer)->UModNoCatPeer::flag[1]
#define U_peer_index_AUTH          (UNoCatPlugIn::peer)->UModNoCatPeer::flag[2]
#define U_peer_policy_flat         (UNoCatPlugIn::peer)->UModNoCatPeer::flag[3]
#define U_peer_index_device        (UNoCatPlugIn::peer)->UModNoCatPeer::flag[4]
#define U_peer_index_network       (UNoCatPlugIn::peer)->UModNoCatPeer::flag[5]
#define U_peer_max_time_no_traffic (UNoCatPlugIn::peer)->UModNoCatPeer::flag[6]
#define U_peer_unused1             (UNoCatPlugIn::peer)->UModNoCatPeer::flag[7]

// override the default...
template <> inline void u_destroy(const UIPAddress** ptr, uint32_t n) { U_TRACE(0,"u_destroy<UIPAddress*>(%p,%u)", ptr, n) }

class U_EXPORT UNoCatPlugIn : public UServerPlugIn, UEventTime {
public:

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   enum CheckType {
      U_CHECK_ARP_CACHE = 0x001,
      U_CHECK_ARP_PING  = 0x002,
      U_CHECK_MAC       = 0x004,
      U_CHECK_FIREWALL  = 0x010
   };

            UNoCatPlugIn();
   virtual ~UNoCatPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg) U_DECL_FINAL;
   virtual int handlerInit() U_DECL_FINAL;
   virtual int handlerFork() U_DECL_FINAL;

   // Connection-wide hooks

   virtual int handlerRequest() U_DECL_FINAL;

   // define method VIRTUAL of class UEventTime

   virtual int handlerTime() U_DECL_FINAL;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   static UString* input;
   static UString* label;
   static UString* fw_env;
   static UString* fw_cmd;
   static UString* extdev;
   static UString* intdev;
   static UString* mempool;
   static UString* hostname;
   static UString* localnet;
   static UString* location;
   static UString* arp_cache;
   static UString* auth_login;
   static UString* decrypt_key;
   static UString* label_to_match;
   static UString* status_content;
   static UString* allowed_members;
   static UString* IP_address_trust;
   static UString* peer_present_in_arp_cache;

   static UVector<Url*>* vauth_url;
   static UVector<UString>* vauth;
   static UVector<UString>* vauth_ip;
   static UVector<UString>* openlist;
   static UVector<UString>* varp_cache;
   static UVector<UString>* vinfo_data;
   static UVector<UIPAddress*>** vaddr;
   static UVector<UString>* vroaming_data;
   static UHttpClient<UTCPSocket>* client;
   static UHashMap<UModNoCatPeer*>* peers;
   static UVector<UString>* vLoginValidate;
   static UVector<UString>* vInternalDevice;
   static UVector<UString>* vLocalNetworkLabel;
   static UVector<UIPAllow*>* vLocalNetworkMask;

   static void* pdata;
   static UCommand* fw;
   static UPing** sockp;
   static fd_set addrmask;
   static fd_set* paddrmask;
   static UIptAccount* ipt;
   static UDirWalk* dirwalk;
   static UModNoCatPeer* peer;
   static bool flag_check_system;
   static uint64_t traffic_available;
   static int fd_stderr, check_type, next_event_time;
   static long last_request_firewall, last_request_check;
   static uint32_t total_connections, nfds, num_radio, idx_peers_preallocate, num_peers_preallocate, time_available, check_expire;

   static UModNoCatPeer* peers_delete; // delete list 
   static UModNoCatPeer* peers_preallocate;

   static void getTraffic();
   static void setNewPeer();
   static void checkSystem();
   static void checkOldPeer();
   static void creatNewPeer();
   static bool setPeerLabel();
   static bool checkFirewall();
   static void addPeerRoaming();
   static bool preallocatePeersFault();
   static bool getPeer(uint32_t i) __pure;
   static void addPeerInfo(int disconnected);
   static void uploadFileToPortal(UFile& file);
   static bool creatNewPeer(uint32_t index_AUTH);
   static void sendInfoData(uint32_t index_AUTH);
   static bool getPeerFromMAC(const UString& mac);
   static void sendRoamingData(uint32_t index_AUTH);
   static bool checkAuthMessage(const UString& msg);
   static void setStatusContent(const UString& label);
   static void deny(int disconnected, bool bcheck_expire);
   static void notifyAuthOfUsersInfo(uint32_t index_AUTH);
   static bool getPeerStatus(UStringRep* key, void* value);
   static bool checkPeerInfo(UStringRep* key, void* value);
   static bool checkPeerStatus(UStringRep* key, void* value);
   static bool getPeerListInfo(UStringRep* key, void* value);
   static void setHTTPResponse(const UString& content, int mime_index);
   static void permit(const UString& UserDownloadRate, const UString& UserUploadRate);
   static void sendMsgToPortal(uint32_t index_AUTH, const UString& msg, UString* poutput);
   static void sendData(uint32_t index_AUTH, const UString& data, const char* service, uint32_t service_len);

   static uint32_t checkFirewall(UString& output);
   static uint32_t getIndexAUTH(const char* ip_address) __pure;
   static UString  getIPAddress(const char* ptr, uint32_t len);
   static UString  getSignedData(const char* ptr, uint32_t len);

   static void preallocatePeers()
      {
      U_TRACE_NO_PARAM(0+256, "UNoCatPlugIn::preallocatePeers()")

      peers_preallocate = new UModNoCatPeer[num_peers_preallocate];

      // put the new preallocated peers on the delete list

      peers_preallocate->next = peers_delete;
                                peers_delete = peers_preallocate;
      }

   static bool getARPCache()
      {
      U_TRACE_NO_PARAM(0+256, "UNoCatPlugIn::getARPCache()")

      return USocketExt::getARPCache(*arp_cache, *varp_cache);
      }

   static void sendMsgToAllPortal(const UString& msg)
      {
      U_TRACE(0, "UNoCatPlugIn::sendMsgToAllPortal(%V)", msg.rep)

      for (uint32_t i = 0, n = vauth_url->size(); i < n; ++i) sendMsgToPortal(i, msg, 0);
      }

   static bool isPingAsyncPending()
      {
      U_TRACE_NO_PARAM(0, "UNoCatPlugIn::isPingAsyncPending()")

      U_INTERNAL_DUMP("check_type = %B nfds = %u paddrmask = %p", check_type, nfds, paddrmask)

      bool result = (((check_type & U_CHECK_ARP_PING) != 0) && nfds && paddrmask == 0);

      U_RETURN(result);
      }

   static void executeCommand(int type)
      {
      U_TRACE(0, "UNoCatPlugIn::executeCommand(%d)", type)

      U_INTERNAL_ASSERT_POINTER(peer)

      peer->fw.setArgument(3, (type == UModNoCatPeer::PEER_PERMIT ? "permit" : "deny"));

      (void) peer->fw.executeAndWait(0, -1, fd_stderr);

#  ifndef U_LOG_DISABLE
      UServer_Base::logCommandMsgError(peer->fw.getCommand(), false);
#  endif

      U_peer_status = type;
      }

   static UString getUrlForSendMsgToPortal(uint32_t index_AUTH, const char* service, uint32_t service_len)
      {
      U_TRACE(0, "UNoCatPlugIn::getUrlForSendMsgToPortal(%u,%.*S,%u)", index_AUTH, service_len, service, service_len)

      Url* auth = (*vauth_url)[index_AUTH];
      UString auth_host    = auth->getHost(),
              auth_service = auth->getService(),
              url(200U + auth_host.size() + auth_service.size() + service_len);

      url.snprintf(U_CONSTANT_TO_PARAM("%v://%v%.*s"), auth_service.rep, auth_host.rep, service_len, service);

      U_RETURN_STRING(url);
      }

   static void setFireWallCommand(UCommand& cmd, const UString& script, const UString& mac, const UString& ip)
      {
      U_TRACE(0, "UNoCatPlugIn::setFireWallCommand(%p,%V,%V,%V)", &cmd, script.rep, mac.rep, ip.rep)

      // NB: request(arp|deny|clear|reset|permit|openlist|initialize) mac ip class(Owner|Member|Public) UserDownloadRate UserUploadRate

      UString command(100U);

      command.snprintf(U_CONSTANT_TO_PARAM("/bin/sh %v deny %v %v Member 0 0"), script.rep, mac.rep, ip.rep);

      cmd.set(command, (char**)0);
      cmd.setEnvironment(fw_env);
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UNoCatPlugIn)

   friend class UModNoCatPeer;
};

#endif
