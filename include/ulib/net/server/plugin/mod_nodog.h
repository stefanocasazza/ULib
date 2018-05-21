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

class UModNoDogPeer : public UEventTime {
public:

   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator

   void* operator new(size_t sz)
      {
      U_TRACE(0, "UModNoDogPeer::operator new(%u)", sz)
      
      char* p = (char*) U_SYSCALL(malloc, "%u", sz);
      
      U_INTERNAL_ASSERT_POINTER_MSG(p, "cannot allocate memory, exiting...")

      return p;
      }

   void operator delete(void* _ptr, size_t sz)
      {
      U_TRACE(0, "UModNoDogPeer::operator delete(%p,%u)", _ptr, sz)
      
      U_SYSCALL_VOID(free, "%p", _ptr);
      }

   UModNoDogPeer() : UEventTime(30L,0L), mac(*UString::str_without_mac)
      {
      U_TRACE_CTOR(0, UModNoDogPeer, "", 0)

      _ctime          = u_now->tv_sec;
      ctraffic        = 
      time_no_traffic =
      flag.u          = 0;
      }

   virtual ~UModNoDogPeer()
      {
      U_TRACE_DTOR(0, UModNoDogPeer)
      }

   // define method VIRTUAL of class UEventTime

   virtual int handlerTime() U_DECL_FINAL;

   // DEBUG

#if defined(U_STDCPP_ENABLE) && defined(DEBUG)
   const char* dump(bool reset) const;
#endif

protected:
   UString ip, mac, label;
   long _ctime;
   uint32_t addr, ctraffic, time_no_traffic;
   uucflag flag;

private:
   U_DISALLOW_ASSIGN(UModNoDogPeer)

   friend class UNoDogPlugIn;
};

enum UPeerType {
   U_PEER_PERMIT                  = 0x01,
   U_PEER_ALLOWED                 = 0x02,
   U_PEER_ALLOW_DISABLE           = 0x04,
   U_PEER_DELAY_DISABLE           = 0x08,
   U_PEER_NOTIFY_DISABLE          = 0x10,
   U_PEER_STRICT_NOTIFY_DISABLE   = 0x20,
   U_PEER_TIMER_ACTIVE            = 0x40,
   U_PEER_MAC_FROM_DHCP_DATA_FILE = 0x80
};

#define U_peer_index_network (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[0]
#define U_peer_policy        (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[1]
#define U_peer_user          (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[2]

#define U_peer_index_network (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[0]
#define U_peer_policy        (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[1]
#define U_peer_user          (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[2]
#define U_peer_flag          (UNoDogPlugIn::peer)->UModNoDogPeer::flag.c[3]

#define U_peer_permit                  ((U_peer_flag & U_PEER_PERMIT)                  != 0)
#define U_peer_allowed                 ((U_peer_flag & U_PEER_ALLOWED)                 != 0)
#define U_peer_timer_active            ((U_peer_flag & U_PEER_TIMER_ACTIVE)            != 0)
#define U_peer_allow_disable           ((U_peer_flag & U_PEER_ALLOW_DISABLE)           != 0)
#define U_peer_delay_disable           ((U_peer_flag & U_PEER_DELAY_DISABLE)           != 0)
#define U_peer_notify_disable          ((U_peer_flag & U_PEER_NOTIFY_DISABLE)          != 0)
#define U_peer_strict_notify_disable   ((U_peer_flag & U_PEER_STRICT_NOTIFY_DISABLE)   != 0)
#define U_peer_mac_from_dhcp_data_file ((U_peer_flag & U_PEER_MAC_FROM_DHCP_DATA_FILE) != 0)

// override the default...
template <> inline void u_destroy(const UModNoDogPeer** ptr, uint32_t n) { U_TRACE(0,"u_destroy<UModNoDogPeer*>(%p,%u)", ptr, n) }

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

   virtual int handlerTime() U_DECL_FINAL;

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
   static UString* mac_old;
   static UString* label_old;
   static UString* hostname;
   static UString* localnet;
   static UString* info_data;
   static UString* arp_cache;
   static UString* allowed_members;
   static UString* IP_address_trust;

   static UString* auth_host;
   static UString* auth_deny;
   static UString* auth_info;
   static UString* auth_login;
   static UString* auth_notify;
   static UString* auth_service;
   static UString* auth_strict_notify;

   static void* pdata;
   static UCommand* fw;
   static int fd_stderr;
   static in_addr_t addr;
   static UIptAccount* ipt;
   static uint32_t check_expire, T1, T2;
   static UHttpClient<UTCPSocket>* client;
   static bool mac_from_dhcp_data_file, bnetwork_interface, bdifferent;

   static UIPAllow* pallow;
   static UModNoDogPeer* peer;
   static UHashMap<UModNoDogPeer*>* peers;

   static UVector<UString>* varp_cache;
   static UVector<UString>* vInternalDevice;
   static UVector<UString>* vLocalNetworkSpec;
   static UVector<UString>* vLocalNetworkLabel;
   static UVector<UIPAllow*>* vLocalNetworkMask;

   static bool setNewPeer();
   static bool preallocatePeersFault();
   static void executeCommand(const char* type, uint32_t len, const UString& mac);

   static void deny(const UString& mac)
      {
      U_TRACE(0, "UNoDogPlugIn::deny(%V)", mac.rep)

      U_INTERNAL_ASSERT_POINTER(peer)
      U_INTERNAL_ASSERT(U_peer_permit)

      if (U_peer_allowed)
         {
         U_SRV_LOG("WARNING: I should to deny user allowed: IP %v MAC %v", peer->ip.rep, mac.rep);

         return;
         }

      executeCommand(U_CONSTANT_TO_PARAM("deny"), mac);

      U_SRV_LOG("Peer denied: IP %v MAC %v", peer->ip.rep, mac.rep);
      }

   static void permit(const UString& mac)
      {
      U_TRACE(0, "UNoDogPlugIn::permit(%V)", mac.rep)

      U_INTERNAL_ASSERT_POINTER(peer)
      U_INTERNAL_ASSERT_EQUALS(U_peer_permit, false)

      executeCommand(U_CONSTANT_TO_PARAM("permit"), mac);

      peer->_ctime = u_now->tv_sec;

      U_peer_flag |= U_PEER_PERMIT;

      U_SRV_LOG("Peer permitted: IP %v MAC %v", peer->ip.rep, mac.rep);
      }

   static UString getUrlForSendMsgToPortal(const char* service, uint32_t service_len)
      {
      U_TRACE(0, "UNoDogPlugIn::getUrlForSendMsgToPortal(%.*S,%u)", service_len, service, service_len)

      char buffer[256];

      UString url((void*)buffer, u__snprintf(buffer, sizeof(buffer), U_CONSTANT_TO_PARAM("%v://%v%.*s"), auth_service->rep, auth_host->rep, service_len, service));

      U_RETURN_STRING(url);
      }

   static UString getUrlForSendMsgToPortal(const UString& service) { return getUrlForSendMsgToPortal(U_STRING_TO_PARAM(service)); }

   static uint32_t getApInfo(char* buffer, uint32_t bufsize, const UString& lbl)
      {
      U_TRACE(0, "UNoDogPlugIn::getApInfo(%p,%u,%V)", buffer, bufsize, lbl.rep)

      return u__snprintf(buffer, bufsize, U_CONSTANT_TO_PARAM("%v@%v/%v"), lbl.rep, UServer_Base::IP_address->rep, hostname->rep);
      }

   static uint32_t checkUrl(char* buffer, uint32_t buffer_len, uint32_t sz, const char* user, uint32_t user_len);

private:
   U_DISALLOW_COPY_AND_ASSIGN(UNoDogPlugIn)

   static bool   getPeer() U_NO_EXPORT;
   static void erasePeer() U_NO_EXPORT;

   static bool setMAC() U_NO_EXPORT;
   static void sendLogin() U_NO_EXPORT;
   static void sendNotify() U_NO_EXPORT;
   static void eraseTimer() U_NO_EXPORT;
   static bool setLabelAndMAC() U_NO_EXPORT;
   static void sendStrictNotify() U_NO_EXPORT;

// static void printPeers(const char* fmt, uint32_t len) U_NO_EXPORT;
   static void makeInfoData(UFlatBuffer* pfb, void* param) U_NO_EXPORT;
   static void makeLoginData(UFlatBuffer* pfb, void* param) U_NO_EXPORT;
   static void makeNotifyData(UFlatBuffer* pfb, void* param) U_NO_EXPORT;

   friend class UModNoDogPeer;
};
#endif
