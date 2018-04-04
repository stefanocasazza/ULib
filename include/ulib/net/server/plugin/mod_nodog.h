// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    mod_nodog.h
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef U_MOD_NODOG_H
#define U_MOD_NODOG_H 1

#include <ulib/url.h>
#include <ulib/timer.h>
#include <ulib/command.h>
#include <ulib/net/tcpsocket.h>
#include <ulib/net/client/http.h>
#include <ulib/net/server/server_plugin.h>

class UIptAccount;
class UFlatBuffer;
class UNoDogPlugIn;

class UModNoDogPeer : public UEventTime, UIPAddress {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UModNoDogPeer() : UEventTime(30L,0L), mac(*UString::str_without_mac)
      {
      U_TRACE_REGISTER_OBJECT(0, UModNoDogPeer, "", 0)

      next            = U_NULLPTR;
      _ctime          = u_now->tv_sec;
      ctraffic        = 
      time_no_traffic =
      flag.u          = 0;
      }

   virtual ~UModNoDogPeer()
      {
      U_TRACE_UNREGISTER_OBJECT(0, UModNoDogPeer)
      }

   // define method VIRTUAL of class UEventTime

   virtual int handlerTime() U_DECL_FINAL;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UCommand fw;
   UModNoDogPeer* next;
   UString ip, mac, ifname, label, gateway, welcome;
   long _ctime;
   uint32_t ctraffic, time_no_traffic;
   uucflag flag;

   UString getMAC()
      {
      U_TRACE_NO_PARAM(0, "UModNoDogPeer::getMAC()")

      UString x(13U);

      unsigned char* src = (unsigned char*)mac.data();
      unsigned char* dst = (unsigned char*)  x.data();

   // "%2u:%2u:%2u:%2u:%2u:%2u"

      dst[ 0] = src[ 0];
      dst[ 1] = src[ 1];
      dst[ 2] = src[ 3]; 
      dst[ 3] = src[ 4];
      dst[ 4] = src[ 6]; 
      dst[ 5] = src[ 7];
      dst[ 6] = src[ 9]; 
      dst[ 7] = src[10];
      dst[ 8] = src[12]; 
      dst[ 9] = src[13];
      dst[10] = src[15]; 
      dst[11] = src[16];

      x.size_adjust(12);

      U_RETURN_STRING(x);
      }

private:
   U_DISALLOW_ASSIGN(UModNoDogPeer)

   friend class UNoDogPlugIn;
};

enum UPeerType {
   U_PEER_PERMIT                = 0x01,
   U_PEER_ALLOWED               = 0x02,
   U_PEER_ALLOW_DISABLE         = 0x04,
   U_PEER_DELAY_DISABLE         = 0x08,
   U_PEER_NOTIFY_DISABLE        = 0x10,
   U_PEER_STRICT_NOTIFY_DISABLE = 0x20,
   U_PEER_TIMER_ACTIVE          = 0x40
};

#define U_peer_index_network (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[0]
#define U_peer_policy        (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[1]
#define U_peer_user          (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[2]

#define U_peer_index_network (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[0]
#define U_peer_policy        (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[1]
#define U_peer_user          (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[2]
#define U_peer_flag          (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[3]

#define U_peer_permit                ((U_peer_flag & U_PEER_PERMIT)                != 0)
#define U_peer_allowed               ((U_peer_flag & U_PEER_ALLOWED)               != 0)
#define U_peer_timer_active          ((U_peer_flag & U_PEER_TIMER_ACTIVE)          != 0)
#define U_peer_allow_disable         ((U_peer_flag & U_PEER_ALLOW_DISABLE)         != 0)
#define U_peer_delay_disable         ((U_peer_flag & U_PEER_DELAY_DISABLE)         != 0)
#define U_peer_notify_disable        ((U_peer_flag & U_PEER_NOTIFY_DISABLE)        != 0)
#define U_peer_strict_notify_disable ((U_peer_flag & U_PEER_STRICT_NOTIFY_DISABLE) != 0)

// override the default...
template <> inline void u_destroy(const UIPAddress** ptr, uint32_t n) { U_TRACE(0,"u_destroy<UIPAddress*>(%p,%u)", ptr, n) }

class U_EXPORT UNoDogPlugIn : public UServerPlugIn, UEventTime {
public:

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

            UNoDogPlugIn();
   virtual ~UNoDogPlugIn();

   // define method VIRTUAL of class UServerPlugIn

   // Server-wide hooks

   virtual int handlerConfig(UFileConfig& cfg) U_DECL_FINAL;
   virtual int handlerInit() U_DECL_FINAL;
   virtual int handlerFork() U_DECL_FINAL;

   // Connection-wide hooks

   virtual int handlerRequest() U_DECL_FINAL;

   // define method VIRTUAL of class UEventTime

   virtual int handlerTime() U_DECL_FINAL
      {
      U_TRACE_NO_PARAM(0, "UNoDogPlugIn::handlerTime()")

      checkSystem();

      U_RETURN(0); // monitoring
      }

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   static UString* label;
   static UString* chrash;
   static UString* fw_cmd;
   static UString* fw_env;
   static UString* extdev;
   static UString* intdev;
   static UString* hostname;
   static UString* localnet;
   static UString* info_data;
   static UString* arp_cache;
   static UString* allowed_members;
   static UString* IP_address_trust;

   static UString* auth_host;
   static UString* auth_info;
   static UString* auth_login;
   static UString* auth_notify;
   static UString* auth_service;
   static UString* auth_strict_notify;

   static void* pdata;
   static UCommand* fw;
   static int fd_stderr;
   static UString* mempool;
   static UFlatBuffer* pfb;
   static UIptAccount* ipt;
   static uint32_t check_expire, T1, T2;
   static UHttpClient<UTCPSocket>* client;
   static bool mac_from_dhcp_data_file, bnetwork_interface;

   static UModNoDogPeer* peer;
   static UHashMap<UModNoDogPeer*>* peers;

   static UVector<UString>* varp_cache;
   static UVector<UString>* vInternalDevice;
   static UVector<UString>* vLocalNetworkSpec;
   static UVector<UString>* vLocalNetworkLabel;
   static UVector<UIPAllow*>* vLocalNetworkMask;

   static void setNewPeer();
   static void checkSystem();
   static bool preallocatePeersFault();
   static bool getPeerInfo(UStringRep* key, void* value);

   static void executeCommand(const char* type)
      {
      U_TRACE(0, "UNoDogPlugIn::executeCommand(%S)", type)

      U_INTERNAL_ASSERT_POINTER(peer)

      peer->fw.setArgument(3, type);

      (void) peer->fw.executeAndWait(U_NULLPTR, -1, fd_stderr);

#  ifndef U_LOG_DISABLE
      UServer_Base::logCommandMsgError(peer->fw.getCommand(), false);
#  endif
      }

   static void deny()
      {
      U_TRACE_NO_PARAM(0, "UNoDogPlugIn::deny()")

      U_INTERNAL_ASSERT_POINTER(peer)
      U_INTERNAL_ASSERT(U_peer_permit)

      if (U_peer_allowed)
         {
         U_SRV_LOG("WARNING: I should to deny user allowed: IP %v MAC %v", peer->ip.rep, peer->mac.rep);

         return;
         }

      executeCommand("deny");

      U_SRV_LOG("Peer denied: IP %v MAC %v", peer->ip.rep, peer->mac.rep);
      }

   static void permit()
      {
      U_TRACE_NO_PARAM(0, "UNoDogPlugIn::permit()")

      U_INTERNAL_ASSERT_POINTER(peer)
      U_INTERNAL_ASSERT_EQUALS(U_peer_permit, false)

      executeCommand("permit");

      peer->_ctime   = u_now->tv_sec;

      U_peer_flag |= U_PEER_PERMIT;

      U_SRV_LOG("Peer permitted: IP %v MAC %v", peer->ip.rep, peer->mac.rep);
      }

   static UString getUrlForSendMsgToPortal(const char* service, uint32_t service_len)
      {
      U_TRACE(0, "UNoDogPlugIn::getUrlForSendMsgToPortal(%.*S,%u)", service_len, service, service_len)

      UString url(100U + service_len);

      url.snprintf(U_CONSTANT_TO_PARAM("%v://%v%.*s"), auth_service->rep, auth_host->rep, service_len, service);

      U_RETURN_STRING(url);
      }

   static UString getUrlForSendMsgToPortal(const UString& service) { return getUrlForSendMsgToPortal(U_STRING_TO_PARAM(service)); }

   static UString getApInfo(const UString& lbl)
      {
      U_TRACE(0, "UNoDogPlugIn::getApInfo(%V)", lbl.rep)

      UString x(1024U);

      x.snprintf(U_CONSTANT_TO_PARAM("%v@%v/%v"), lbl.rep, UServer_Base::IP_address->rep, hostname->rep);

      U_RETURN_STRING(x);
      }

private:
   U_DISALLOW_COPY_AND_ASSIGN(UNoDogPlugIn)

   static void setMAC() U_NO_EXPORT;
   static void erasePeer() U_NO_EXPORT;
   static void sendLogin() U_NO_EXPORT;
   static void getTraffic() U_NO_EXPORT;
   static void sendNotify() U_NO_EXPORT;
   static void eraseTimer() U_NO_EXPORT;
   static void setLabelAndMAC() U_NO_EXPORT;
   static void sendStrictNotify() U_NO_EXPORT;

   static void printPeers(const char* fmt, uint32_t len) U_NO_EXPORT;
   static void makeInfoData(UFlatBuffer* pfb, void* param) U_NO_EXPORT;
   static void makeLoginData(UFlatBuffer* pfb, void* param) U_NO_EXPORT;
   static void makeNotifyData(UFlatBuffer* pfb, void* param) U_NO_EXPORT;

   friend class UModNoDogPeer;
};
#endif
