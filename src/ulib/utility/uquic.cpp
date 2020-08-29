// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    uquic.cpp - Quic utility
//
// = AUTHOR
//    Stefano Casazza + Victor Stewart
//
// ============================================================================

#include <ulib/utility/http3.h>

size_t                  UQuic::scid_len;
size_t                  UQuic::token_len;
size_t                  UQuic::conn_id_len;
uint8_t                 UQuic::pkt_type;
uint8_t                 UQuic::token[U_MAX_TOKEN_LEN];
uint8_t                 UQuic::scid[QUICHE_MAX_CONN_ID_LEN];
uint8_t                 UQuic::conn_id[QUICHE_MAX_CONN_ID_LEN];
uint32_t                UQuic::pkt_version;
uint32_t                UQuic::peer_addr_len;
quiche_conn*            UQuic::conn;
quiche_config*          UQuic::qconfig;

struct sockaddr_storage       UQuic::peer_addr;
UHashMap<UClientImage_Base*>* UQuic::peers;

int UQuic::loadConfigParam()
{
   U_TRACE_NO_PARAM(0, "UQuic::loadConfigParam()")

   U_INTERNAL_ASSERT(UServer_Base::budp)
   U_INTERNAL_ASSERT_POINTER(UServer_Base::pcfg)

#ifndef USE_LIBQUICHE
   U_ERROR("Sorry, I was compiled without libquiche so there isn't HTTP/3 support");
#endif

   if (UServer_Base::key_file->empty() ||
       UServer_Base::cert_file->empty())
      {
      U_ERROR("You need to specify in configuration file the property KEY_FILE and CERT_FILE");
      }

#ifdef DEBUG
// (void) U_SYSCALL(quiche_enable_debug_logging, "%p,%p", quiche_debug_log, U_NULLPTR);
#endif

   // Stores configuration shared between multiple connections
   qconfig = (quiche_config*) U_SYSCALL(quiche_config_new, "%u", QUICHE_PROTOCOL_VERSION);

   if (qconfig == U_NULLPTR)
      {
      U_ERROR("Failed to create quiche config");
      }

   // Configures the given certificate chain
   (void) U_SYSCALL(quiche_config_load_cert_chain_from_pem_file, "%p,%S", qconfig, UServer_Base::cert_file->data());

   // Configures the given private key
   (void) U_SYSCALL(quiche_config_load_priv_key_from_pem_file, "%p,%S", qconfig, UServer_Base::key_file->data());

   if (UServer_Base::pcfg->searchForObjectStream(U_CONSTANT_TO_PARAM("quic")))
      {
      UServer_Base::pcfg->table.clear();

      if (UServer_Base::pcfg->loadTable())
         {
         // --------------------------------------------------------------------------------------------------------------------------------------
         // userver_udp - quic configuration parameters
         // --------------------------------------------------------------------------------------------------------------------------------------
         // QUICHE_GREASE                              Whether to send GREASE 
         // QUICHE_LOG_KEYS                            Enables logging of secrets 
         // QUICHE_VERIFY_PEER                         Whether to verify the peer's certificate 
         // QUICHE_CC_ALGORITHM                        Sets the congestion control algorithm used
         // QUICHE_MAX_ACK_DELAY                       Sets the `max_ack_delay` transport parameter
         // QUICHE_MAX_PACKET_SIZE                     Sets the max_packet_size transport parameter
         // QUICHE_MAX_IDLE_TIMEOUT                    Sets the `max_idle_timeout` transport parameter 
         // QUICHE_INITIAL_MAX_DATA                    Sets the `initial_max_data` transport parameter 
         // QUICHE_ENABLE_EARLY_DATA                   Enables sending or receiving early data 
         // QUICHE_ACK_DELAY_EXPONENT                  Sets the `ack_delay_exponent` transport parameter 
         // QUICHE_MAX_UDP_PAYLOAD_SIZE                Sets the `max_udp_payload_size` transport parameter 
         // QUICHE_INITIAL_MAX_STREAM_UNI              Sets the `initial_max_streams_uni` transport parameter  
         // QUICHE_DISABLE_ACTIVE_MIGRATION            Sets the `disable_active_migration` transport parameter 
         // QUICHE_INITIAL_MAX_STREAMS_BIDI            Sets the `initial_max_streams_bidi` transport parameter 
         // QUICHE_INITIAL_MAX_STREAM_DATA_UNI         Sets the `initial_max_stream_data_uni` transport parameter    
         // QUICHE_INITIAL_MAX_STREAM_DATA_BIDI_LOCAL  Sets the `initial_max_stream_data_bidi_local` transport parameter 
         // QUICHE_INITIAL_MAX_STREAM_DATA_BIDI_REMOTE Sets the `initial_max_stream_data_bidi_remote` transport parameter 
         // --------------------------------------------------------------------------------------------------------------------------------------

         long param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_MAX_IDLE_TIMEOUT"), 180000);
         bool param1 = UServer_Base::pcfg->readBoolean(U_CONSTANT_TO_PARAM("QUICHE_DISABLE_ACTIVE_MIGRATION"), true);

         U_SYSCALL_VOID(quiche_config_set_max_idle_timeout,         "%p,%lu", qconfig, param0);
         U_SYSCALL_VOID(quiche_config_set_disable_active_migration, "%p,%b",  qconfig, param1);

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_CC_ALGORITHM"), 0); // QUICHE_CC_RENO = 0

         U_SYSCALL_VOID(quiche_config_set_cc_algorithm, "%p,%lu", qconfig, (quiche_cc_algorithm)param0);

#     ifdef LIBQUICHE_AT_LEAST_0_5
         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_MAX_UDP_PAYLOAD_SIZE"), U_MAX_DATAGRAM_SIZE);

         U_SYSCALL_VOID(quiche_config_set_max_udp_payload_size, "%p,%lu", qconfig, param0);
#     else
         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_MAX_PACKET_SIZE"), U_MAX_DATAGRAM_SIZE);

         U_SYSCALL_VOID(quiche_config_set_max_packet_size, "%p,%lu", qconfig, param0);
#     endif

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_INITIAL_MAX_DATA"), 10485760);

         U_SYSCALL_VOID(quiche_config_set_initial_max_data, "%p,%lu", qconfig, param0);

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_INITIAL_MAX_STREAM_UNI"), 3); // For HTTP/3 we only need 3 unidirectional streams

         U_SYSCALL_VOID(quiche_config_set_initial_max_streams_uni, "%p,%lu", qconfig, param0);

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_INITIAL_MAX_STREAMS_BIDI"), 128);

         U_SYSCALL_VOID(quiche_config_set_initial_max_streams_bidi, "%p,%lu", qconfig, param0);

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_INITIAL_MAX_STREAM_DATA_UNI"), 1048576);

         U_SYSCALL_VOID(quiche_config_set_initial_max_stream_data_uni, "%p,%lu", qconfig, param0);

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_INITIAL_MAX_STREAM_DATA_BIDI_LOCAL"), 1000000);

         U_SYSCALL_VOID(quiche_config_set_initial_max_stream_data_bidi_local, "%p,%lu", qconfig, param0);

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_INITIAL_MAX_STREAM_DATA_BIDI_REMOTE"), 1048576);

         U_SYSCALL_VOID(quiche_config_set_initial_max_stream_data_bidi_remote, "%p,%lu", qconfig, param0);

         /*
         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_MAX_ACK_DELAY"), 25);

         U_SYSCALL_VOID(quiche_config_set_max_ack_delay, "%p,%lu", qconfig, param0);

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_ACK_DELAY_EXPONENT"), 3);

         U_SYSCALL_VOID(quiche_config_set_ack_delay_exponent, "%p,%lu", qconfig, param0);
         */

         if (UServer_Base::pcfg->readBoolean(U_CONSTANT_TO_PARAM("QUICHE_ENABLE_EARLY_DATA"), false)) U_SYSCALL_VOID(quiche_config_enable_early_data, "%p", qconfig);

         /** TODO

         void quiche_config_log_keys(quiche_config *config);
         void quiche_config_grease(quiche_config *config, bool v);
         void quiche_config_verify_peer(quiche_config *config, bool v);
         */

         UServer_Base::pcfg->reset();
         }
      }

   U_RETURN(U_PLUGIN_HANDLER_OK);
}

bool UQuic::parseHeader()
{
   U_TRACE_NO_PARAM(0, "UQuic::parseHeader()")

   // Parse the QUIC packet's header

      scid_len =
   conn_id_len = QUICHE_MAX_CONN_ID_LEN;
     token_len = U_MAX_TOKEN_LEN;

   // Extracts version, type, source / destination connection ID and address verification token from the packet in buffer
   int rc = U_SYSCALL(quiche_header_info, "%p,%u,%u,%p,%p,%p,%p,%p,%p,%p,%p", (const uint8_t*)U_STRING_TO_PARAM(*UServer_Base::rbuffer), U_LOCAL_CONN_ID_LEN,
                                                                          &pkt_version, &pkt_type, scid, &scid_len, conn_id, &conn_id_len, token, &token_len);

   if (rc < 0)
      {
      U_DEBUG("UQuic::handlerRead(): failed to parse header: %d", rc)

      U_RETURN(false);
      }

   U_INTERNAL_DUMP("scid(%u) = %#.*S token(%u) = %#.*S conn_id(%u) = %#.*S pkt_version = %p pkt_type = %u",
                    scid_len, scid_len, scid, token_len, token_len, token, conn_id_len, conn_id_len, conn_id, pkt_version, pkt_type)

   U_RETURN(true);
}

bool UQuic::handlerRead(quiche_conn* lconn, bool bwait_for_data)
{
   U_TRACE(0, "UQuic::handlerRead(%p,%b)", lconn, bwait_for_data)

   U_ASSERT_EQUALS(UClientImage_Base::rbuffer->capacity(), UServer_Base::rbuffer_size)

   int iBytesRead;
   char* ptr = UServer_Base::rbuffer->data();

   // Read incoming UDP packets from the socket and feed them to quiche, until there are no more packets to read

loop:
   peer_addr_len = sizeof(peer_addr);

   (void) U_SYSCALL(memset, "%p,%u,%u", &peer_addr, 0, U_SIZE_SOCKADDR);

   iBytesRead = U_SYSCALL(recvfrom, "%d,%p,%u,%u,%p,%p", UServer_Base::fds[0], ptr, UServer_Base::rbuffer_size, (bwait_for_data ? 0 : MSG_DONTWAIT),
                                                         (struct sockaddr*)&peer_addr, &peer_addr_len);

   if (iBytesRead <= 0)
      {
      if (errno == EAGAIN)
         {
         U_DEBUG("quiche: recvfrom would block")

         U_INTERNAL_ASSERT_EQUALS(bwait_for_data, false)

         if (lconn) // reported no read packets, we will then proceed with the send loop
            {
            uint8_t out[65536];
            ssize_t written, sent;
            uint32_t start = 0, remain = sizeof(out);

            while (true)
               {
               written = U_SYSCALL(quiche_conn_send, "%p,%p,%u", lconn, &out[start], remain);

               if (written == QUICHE_ERR_DONE)
                  {
                  U_DEBUG("quiche: done writing")

                  break;
                  }

               if (written < 0)
                  {
                  U_DEBUG("UQuic::handlerRead(): failed to create packet: %d", written)

                  break;
                  }

               start  += written;
               remain -= written;

               U_INTERNAL_ASSERT_MINOR(start, sizeof(out))

               U_DEBUG("quiche: sent %u bytes", written)
               }

            sent = U_SYSCALL(sendto, "%d,%p,%u,%u,%p,%d", UServer_Base::fds[0], out, start, 0, (struct sockaddr*)&USocket::peer_addr, USocket::peer_addr_len);

            if (sent != start)
               {
               U_DEBUG("UQuic::handlerRead(): failed to send")

               U_RETURN(false);
               }

            if (U_SYSCALL(quiche_conn_is_closed, "%p", lconn))
               {
               quiche_stats stats;

               U_SYSCALL_VOID(quiche_conn_stats, "%p,%p", lconn, &stats);

               U_DEBUG("quiche: connection closed, recv=%u sent=%u lost=%u rtt=%u cwnd=%u", stats.recv, stats.sent, stats.lost, stats.rtt, stats.cwnd)

               U_SYSCALL_VOID(quiche_conn_free, "%p", lconn);

               U_RETURN(false);
               }
            }

         bwait_for_data = true;

         goto loop;
         }

      U_WARNING("recvfrom on fd %d got %d%R", UServer_Base::fds[0], iBytesRead, 0); // NB: the last argument (0) is necessary...

      if (errno == EINTR) UInterrupt::checkForEventSignalPending();

      U_RETURN(false);
      }

   U_INTERNAL_DUMP("peer_addr_len = %u BytesRead(%u) = %#.*S", peer_addr_len, iBytesRead, iBytesRead, ptr)

   U_INTERNAL_ASSERT_MAJOR(peer_addr_len, 0)

   UServer_Base::rbuffer->size_adjust_force(iBytesRead);

   if (memcmp(&peer_addr, &USocket::peer_addr, peer_addr_len) != 0)
      {
      // TODO

      U_WARNING("recvfrom() different address");

      /* Lookup a connection based on the packet's connection ID. If there is no connection matching, create a new one

      if (lookup())
         {
         }
      */
      }

   if (lconn || parseHeader()) U_RETURN(true);

   U_RETURN(false);
}

bool UQuic::handlerNewConnection()
{
   U_TRACE_NO_PARAM(0, "UQuic::handlerNewConnection()")

   U_INTERNAL_ASSERT_POINTER(peers)
   U_INTERNAL_ASSERT(UServer_Base::budp)

   const char* ptr;
   ssize_t written, sent;
   const char* pkt = "vneg";
   uint8_t out[U_MAX_DATAGRAM_SIZE];

   conn = U_NULLPTR;

loop:
   if (pkt_type != Initial)
      {
      U_DEBUG("UQuic::handlerNewConnection(): packet is NOT initial: %u", pkt_type)

      U_RETURN(false);
      }

   // Client Initial packets must be at least 1200 bytes
   if (UServer_Base::rbuffer->size() < QUICHE_MIN_CLIENT_INITIAL_LEN)
      {
      U_DEBUG("UQuic::handlerNewConnection(): quic initial packet is too short: %u", UServer_Base::rbuffer->size())

      U_RETURN(false);
      }

   // Returns true if the given protocol version is supported
   if (U_SYSCALL(quiche_version_is_supported, "%u", pkt_version) == false)
      {
      U_DEBUG("quiche: version negotiation")

      // Writes a version negotiation packet
      written = U_SYSCALL(quiche_negotiate_version, "%p,%u,%p,%u,%p,%u", scid, scid_len, conn_id, conn_id_len, out, sizeof(out));

pkt:  if (written < 0)
         {
         U_DEBUG("UQuic::handlerNewConnection(): failed to create %S packet: %d", pkt, written)

         U_RETURN(false);
         }

      sent = U_SYSCALL(sendto, "%u,%p,%u,%u,%p,%d", UServer_Base::fds[0], out, written, 0, (struct sockaddr*)&USocket::peer_addr, USocket::peer_addr_len);

      if (sent == written)
         {
         U_DEBUG("quiche: sent %u bytes", sent)
         }
      else
         {
         U_DEBUG("UQuic::handlerNewConnection(): failed to send")

         U_RETURN(false);
         }

      if (handlerRead()) goto loop;

      U_RETURN(false);
      }

   if (token_len == 0)
      {
      // Generate a stateless retry token. The token includes the static string "quiche" followed
      // by the IP address of the client and by the original destination connection ID generated by the client

      U_DEBUG("quiche: stateless retry")

      U_INTERNAL_DUMP("USocket::peer_addr_len = %u", USocket::peer_addr_len)

      U_INTERNAL_ASSERT_MAJOR(conn_id_len, 0)
      U_INTERNAL_ASSERT_MAJOR(USocket::peer_addr_len, 0)

      // 6 -> U_CONSTANT_SIZE("quiche")
      U_MEMCPY(token, "quiche", 6);
      U_MEMCPY(token + 6, &USocket::peer_addr, USocket::peer_addr_len);
      U_MEMCPY(token + 6 + USocket::peer_addr_len, conn_id, conn_id_len);

      token_len = 6 + USocket::peer_addr_len + conn_id_len;

      U_INTERNAL_DUMP("scid(%u) = %.*S conn_id(%u) = %.*S token(%u) = %.*S pkt_version = %p",
                      scid_len, scid_len, scid, conn_id_len, conn_id_len, conn_id, token_len, token_len, token, pkt_version)

      // Writes a retry packet
#  ifdef LIBQUICHE_AT_LEAST_0_5
      written = U_SYSCALL(quiche_retry, "%p,%u,%p,%u,%p,%u,%p,%u,%u,%p,%u", scid, scid_len, conn_id, conn_id_len, conn_id, conn_id_len, token, token_len, pkt_version, out, sizeof(out));
#  else
      written = U_SYSCALL(quiche_retry, "%p,%u,%p,%u,%p,%u,%p,%u,   %p,%u", scid, scid_len, conn_id, conn_id_len, conn_id, conn_id_len, token, token_len,              out, sizeof(out));
#  endif

      pkt = "retry";

      goto pkt;
      }

   // Validates a stateless retry token. This checks that the ticket includes the `"quiche"` static string, and that the client IP address matches the address stored in the ticket
   if (token_len < 6 ||
       memcmp(token, U_CONSTANT_TO_PARAM("quiche")))
      {
      U_DEBUG("UQuic::handlerNewConnection(): invalid address validation token")

      U_RETURN(false);
      }

   token_len -= 6;

   ptr = (const char*)&token[0] + 6;

   if (token_len < USocket::peer_addr_len ||
       memcmp(ptr, &USocket::peer_addr, USocket::peer_addr_len))
      {
      U_DEBUG("UQuic::handlerNewConnection(): invalid address validation token")

      U_RETURN(false);
      }

   ptr       += USocket::peer_addr_len;
   token_len -= USocket::peer_addr_len;

   if (conn_id_len != token_len) // The token was not valid, meaning the retry failed, so drop the packet
      {
      U_DEBUG("UQuic::handlerNewConnection(): invalid address validation token")

      U_RETURN(false);
      }

   // Reuse the source connection ID we sent in the Retry packet, instead of changing it again. Creates a new server-side connection
   conn = (quiche_conn*) U_SYSCALL(quiche_accept, "%p,%u,%p,%u,%p", conn_id, conn_id_len, (const uint8_t*)ptr, token_len, qconfig);

   if (conn == U_NULLPTR)
      {
      U_DEBUG("UQuic::handlerNewConnection(): failed to create connection")

      U_RETURN(false);
      }

   U_DEBUG("quiche: new connection")

   while (processesPackets(conn))
      {
      if (U_SYSCALL(quiche_conn_is_established, "%p", conn))
         {
         U_DEBUG("UQuic::handlerNewConnection(): connection handshake is complete")

         U_RETURN(true);
         }

      if (handlerRead(conn) == false) U_RETURN(false);
      }

   U_RETURN(false);
}
