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

#define U_LOCAL_CONN_ID_LEN 16

class UHTTP;
class UHttpPlugIn;
class UClientImage_Base;

/**
 * HTTP3 connection Information
 *
 * This class contains data about an HTTP3 connection
 */

// override the default...
template <> inline void u_destroy(  const UClientImage_Base*  ptr)             { U_TRACE(0,"u_destroy<UClientImage_Base*>(%p)",      ptr) }
template <> inline void u_destroy(  const UClientImage_Base** ptr, uint32_t n) { U_TRACE(0,"u_destroy<UClientImage_Base*>(%p,%u)",   ptr, n) }
template <> inline void u_construct(const UClientImage_Base** ptr, bool b)     { U_TRACE(0,"u_construct<UClientImage_Base*>(%p,%b)", ptr, b) }

class U_EXPORT UHTTP3 {
public:

   // QUIC packet type
   enum Type {
      Initial            = 0x001,
      Retry              = 0x002,
      Handshake          = 0x003,
      ZeroRTT            = 0x004,
      VersionNegotiation = 0x005,
      Short              = 0x006
   };

   typedef struct conn_io {
      quiche_conn* conn;
      quiche_h3_conn* http3;
   } conn_io;

   static bool handlerRead();
   static bool handlerAccept();
   static int loadConfigParam();

protected:
   static conn_io conn;
   static size_t conn_id_len;
   static quiche_config* qconfig;
   static quiche_h3_config* http3_config;
   static struct sockaddr_storage peer_addr;
   static uint8_t conn_id[QUICHE_MAX_CONN_ID_LEN];
   static uint32_t quiche_max_packet_size, peer_addr_len;

   static UHashMap<UClientImage_Base*>* peers;

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

      if (peers) U_DELETE(peers)

      if (qconfig)      U_SYSCALL_VOID(quiche_config_free, "%p", qconfig);
      if (http3_config) U_SYSCALL_VOID(quiche_h3_config_free, "%p", http3_config);
      }
   
   static int for_each_header(uint8_t* name, size_t name_len, uint8_t* value, size_t value_len, void* argp)
      {
      U_TRACE(0, "UHTTP3::for_each_header(%.*S,%u,%.*S,%u,%p)", name_len, name, name_len, value_len, value, value_len, argp)

      U_DEBUG("got HTTP header: %.*S(%u)=%.*S(%u)", name_len, name, name_len, value_len, value, value_len);

      U_RETURN(0);
      }

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
   friend class UClientImage_Base;
};
#endif
