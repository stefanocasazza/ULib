// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    uquic.h - Quic utility 
//
// = AUTHOR
//    Stefano Casazza + Victor Stewart
//
// ============================================================================

#ifndef U_QUIC_H
#define U_QUIC_H 1

#include <ulib/db/rdb.h>
#include <ulib/net/server/server.h>

#define U_MAX_TOKEN_LEN \
    sizeof("quiche")-1 + \
    sizeof(struct sockaddr_storage) + \
    QUICHE_MAX_CONN_ID_LEN

#define U_LOCAL_CONN_ID_LEN 16
#define U_MAX_DATAGRAM_SIZE 1350

// override the default...
template <> inline void u_destroy(  const UClientImage_Base*  ptr)             { U_TRACE(0,"u_destroy<UClientImage_Base*>(%p)",      ptr) }
template <> inline void u_destroy(  const UClientImage_Base** ptr, uint32_t n) { U_TRACE(0,"u_destroy<UClientImage_Base*>(%p,%u)",   ptr, n) }
template <> inline void u_construct(const UClientImage_Base** ptr, bool b)     { U_TRACE(0,"u_construct<UClientImage_Base*>(%p,%b)", ptr, b) }

class UHTTP3;

class U_EXPORT UQuic {
public:

   // QUIC packet type
   enum Type {
      Initial            = 1,
      Retry              = 2,
      Handshake          = 3,
      ZeroRTT            = 4,
      Short              = 5,
      VersionNegotiation = 6
   };

   static int loadConfigParam();

protected:
   static uint8_t pkt_type;
   static quiche_conn* conn;
   static quiche_config* qconfig;
   static struct sockaddr_storage peer_addr;
   static UHashMap<UClientImage_Base*>* peers;
   static uint32_t pkt_version, peer_addr_len;
   static size_t conn_id_len, scid_len, token_len;
   static uint8_t token[U_MAX_TOKEN_LEN], scid[QUICHE_MAX_CONN_ID_LEN], conn_id[QUICHE_MAX_CONN_ID_LEN];

   // SERVICES

   static void ctor()
      {
      U_TRACE_NO_PARAM(0, "UQuic::ctor()")

      U_INTERNAL_ASSERT_EQUALS(peers, U_NULLPTR)

      U_NEW(UHashMap<UClientImage_Base*>, peers, UHashMap<UClientImage_Base*>(UNotifier::max_connection));
      }

   static void dtor()
      {
      U_TRACE_NO_PARAM(0, "UQuic::dtor()")

      if (peers) U_DELETE(peers)

      if (qconfig) U_SYSCALL_VOID(quiche_config_free, "%p", qconfig);
      }

   static bool parseHeader();
   static bool handlerNewConnection();
   static void handlerRequest(quiche_conn* lconn, quiche_h3_conn* lh3);
   static bool handlerRead(quiche_conn* lconn = U_NULLPTR, bool bwait_for_data = false);

   // Lookup a connection based on the packet's connection ID

   static bool lookup()
      {
      U_TRACE_NO_PARAM(0, "UQuic::lookup()")

      if (peers->empty() == false &&
          ((UServer_Base::pClientImage = peers->at((const char*)conn_id, conn_id_len))))
         {
         UServer_Base::nClientIndex = UServer_Base::pClientImage - UServer_Base::vClientImage;

         U_INTERNAL_DUMP("UServer_Base::nClientIndex = %u", UServer_Base::nClientIndex)

         U_INTERNAL_ASSERT_MINOR(UServer_Base::nClientIndex, UNotifier::max_connection)

         U_RETURN(true);
         }

      U_RETURN(false);
      }

   static void insert()
      {
      U_TRACE_NO_PARAM(0, "UQuic::insert()")

      peers->insert((const char*)conn_id, conn_id_len, UServer_Base::pClientImage);
      }

   static bool processesPackets(quiche_conn* lconn)
      {
      U_TRACE(0, "UQuic::processesPackets(%p)", lconn)

      U_INTERNAL_ASSERT_POINTER(lconn)

      // Processes QUIC packets received from the peer
      ssize_t done = U_SYSCALL(quiche_conn_recv, "%p,%p,%u", lconn, (uint8_t*)U_STRING_TO_PARAM(*UServer_Base::rbuffer));

      if (done < 0)
         {
         U_DEBUG("UQuic::processesPackets(): failed to process packet: %d", done)

         U_RETURN(false);
         }

      U_DEBUG("quiche: recv %d bytes", done)

      U_RETURN(true);
      }

#ifdef DEBUG
   static void quiche_debug_log(const char* line, void* argp)
      {
      U_DEBUG("%s\n", line)
      }
#endif

private:
   U_DISALLOW_COPY_AND_ASSIGN(UQuic)

   friend class UHTTP;
   friend class UHTTP3;
   friend class Application;
   friend class UHttpPlugIn;
   friend class UServer_Base;
   friend class UClientImage_Base;
};
#endif
