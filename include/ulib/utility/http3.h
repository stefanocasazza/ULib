// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    http3.h - HTTP/3 utility
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#ifndef ULIB_HTTP3_H
#define ULIB_HTTP3_H 1

#include <ulib/db/rdb.h>
#include <ulib/notifier.h>

#include <quiche.h>

class UHTTP;
class UHttpPlugIn;
class UClientImage_Base;

/**
 * HTTP3 connection Information
 *
 * This class contains data about an HTTP3 connection
 */

#define U_LOCAL_CONN_ID_LEN 16

class U_EXPORT UHttp3ConnIo : public UDataStorage {
public:

   UHttp3ConnIo()
      {
      U_TRACE_CTOR(0, UHttp3ConnIo, "")
      }

   virtual ~UHttp3ConnIo() U_DECL_FINAL
      {
      U_TRACE_DTOR(0, UHttp3ConnIo)
      }

   // define method VIRTUAL of class UDataStorage

   virtual char* toBuffer() U_DECL_FINAL;
   virtual void  fromData(const char* ptr, uint32_t len) U_DECL_FINAL;

   // SERVICES

#if defined(DEBUG) && defined(U_STDCPP_ENABLE)
   const char* dump(bool reset) const { return UDataStorage::dump(reset); }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHttp3ConnIo)

   friend class UHTTP3;
};

// override the default...
template <> inline void u_destroy(  const UClientImage_Base*  ptr)             { U_TRACE(0,"u_destroy<UClientImage_Base*>(%p)",      ptr) }
template <> inline void u_destroy(  const UClientImage_Base** ptr, uint32_t n) { U_TRACE(0,"u_destroy<UClientImage_Base*>(%p,%u)",   ptr, n) }
template <> inline void u_construct(const UClientImage_Base** ptr, bool b)     { U_TRACE(0,"u_construct<UClientImage_Base*>(%p,%b)", ptr, b) }

class U_EXPORT UHTTP3 {
public:
      
   typedef struct connio {
      uint8_t cid[U_LOCAL_CONN_ID_LEN];
      quiche_conn* conn;
      quiche_h3_conn* http3;
#  ifdef ENABLE_IPV6
      struct sockaddr_in6 peer_addr;
#  else
      struct sockaddr_in  peer_addr;
#  endif
      socklen_t peer_addr_len;
   } connio;

   static int loadConfigParam();
   static void handlerRequest();

protected:
   static quiche_config*    qconfig;
   static quiche_h3_config* http3_config;
   static uint32_t quiche_max_packet_size;

   static UHttp3ConnIo* data_conn_io;
   static UHashMap<UClientImage_Base*>* peers;
   static URDBObjectHandler<UDataStorage*>* db;

   // SERVICES

   static void ctor()
      {
      U_TRACE_NO_PARAM(0, "UHTTP3::ctor()")

      U_INTERNAL_ASSERT_EQUALS(peers, U_NULLPTR)

      U_NEW(UHashMap<UClientImage_Base*>, peers, UHashMap<UClientImage_Base*>(UNotifier::max_connection));
      }

   static void dtor()
      {
      U_TRACE_NO_PARAM(0, "UHTTP3::dtor()")

      if (db) clearDb();
      if (peers) U_DELETE(peers)

      if (qconfig)      U_SYSCALL_VOID(quiche_config_free, "%p", qconfig);
      if (http3_config) U_SYSCALL_VOID(quiche_h3_config_free, "%p", http3_config);
      }
   
   static void  initDb();
   static void clearDb();

#ifdef DEBUG
   static void quiche_debug_log(const char* line, void* argp)
      {
      U_TRACE(0, "UHTTP3::quiche_debug_log(%S,%p)", line, argp)

      U_DEBUG("quiche_debug_log: %s\n", line);
      }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UHTTP3)

   friend class UHTTP;
   friend class Application;
   friend class UHttpPlugIn;
   friend class UHttp3ConnIo;
   friend class UClientImage_Base;
};
#endif
