// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    http3.cpp - HTTP/3 utility 
//
// = AUTHOR
//    Stefano Casazza
//
// ============================================================================

#include <ulib/utility/http3.h>
#include <ulib/net/server/server.h>

#define U_MAX_TOKEN_LEN \
    sizeof("quiche")-1 + \
    sizeof(struct sockaddr_storage) + \
    QUICHE_MAX_CONN_ID_LEN

#define U_MAX_DATAGRAM_SIZE 1350

size_t            UHTTP3::conn_id_len;
uint8_t           UHTTP3::conn_id[QUICHE_MAX_CONN_ID_LEN];
uint32_t          UHTTP3::peer_addr_len;
uint32_t          UHTTP3::quiche_max_packet_size = U_MAX_DATAGRAM_SIZE;
quiche_config*    UHTTP3::qconfig;
UHTTP3::conn_io   UHTTP3::conn;
quiche_h3_config* UHTTP3::http3_config;

struct sockaddr_storage       UHTTP3::peer_addr;
UHashMap<UClientImage_Base*>* UHTTP3::peers;

int UHTTP3::loadConfigParam()
{
   U_TRACE_NO_PARAM(0, "UHTTP3::loadConfigParam()")

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
   (void) U_SYSCALL(quiche_enable_debug_logging, "%p,%p", quiche_debug_log, U_NULLPTR);
#endif

   // Stores configuration shared between multiple connections
   qconfig      = (quiche_config*)    U_SYSCALL(quiche_config_new, "%u", QUICHE_PROTOCOL_VERSION);
   http3_config = (quiche_h3_config*) U_SYSCALL_NO_PARAM(quiche_h3_config_new); // Creates an HTTP/3 config object with default settings values

   if (qconfig      == U_NULLPTR ||
       http3_config == U_NULLPTR)
      {
      U_ERROR("Failed to create quiche/HTTP3 config");
      }

   // Configures the given certificate chain
   (void) U_SYSCALL(quiche_config_load_cert_chain_from_pem_file, "%p,%S", qconfig, UServer_Base::cert_file->data());

   // Configures the given private key
   (void) U_SYSCALL(quiche_config_load_priv_key_from_pem_file, "%p,%S", qconfig, UServer_Base::key_file->data());

   // Configures the list of supported application protocols
   (void) U_SYSCALL(quiche_config_set_application_protos, "%p,%p,%u", qconfig, (uint8_t*)U_CONSTANT_TO_PARAM(QUICHE_H3_APPLICATION_PROTOCOL));

   if (UServer_Base::pcfg->searchForObjectStream(U_CONSTANT_TO_PARAM("http3")))
      {
      UServer_Base::pcfg->table.clear();

      if (UServer_Base::pcfg->loadTable())
         {
         // --------------------------------------------------------------------------------------------------------------------------------------
         // userver_udp - http3 configuration parameters
         // --------------------------------------------------------------------------------------------------------------------------------------
         // QUICHE_GREASE                              Whether to send GREASE 
         // QUICHE_LOG_KEYS                            Enables logging of secrets 
         // QUICHE_VERIFY_PEER                         Whether to verify the peer's certificate 
         // QUICHE_CC_ALGORITHM                        Sets the congestion control algorithm used
         // QUICHE_MAX_ACK_DELAY                       Sets the `max_ack_delay` transport parameter
         // QUICHE_MAX_PACKET_SIZE                     Sets the `max_packet_size` transport parameter 
         // QUICHE_MAX_IDLE_TIMEOUT                    Sets the `max_idle_timeout` transport parameter 
         // QUICHE_INITIAL_MAX_DATA                    Sets the `initial_max_data` transport parameter 
         // QUICHE_ENABLE_EARLY_DATA                   Enables sending or receiving early data 
         // QUICHE_ACK_DELAY_EXPONENT                  Sets the `ack_delay_exponent` transport parameter 
         // QUICHE_INITIAL_MAX_STREAM_UNI              Sets the `initial_max_streams_uni` transport parameter  
         // QUICHE_DISABLE_ACTIVE_MIGRATION            Sets the `disable_active_migration` transport parameter 
         // QUICHE_INITIAL_MAX_STREAMS_BIDI            Sets the `initial_max_streams_bidi` transport parameter 
         // QUICHE_INITIAL_MAX_STREAM_DATA_UNI         Sets the `initial_max_stream_data_uni` transport parameter    
         // QUICHE_INITIAL_MAX_STREAM_DATA_BIDI_LOCAL  Sets the `initial_max_stream_data_bidi_local` transport parameter 
         // QUICHE_INITIAL_MAX_STREAM_DATA_BIDI_REMOTE Sets the `initial_max_stream_data_bidi_remote` transport parameter 
         //
         // QUICHE_H3_MAX_HEADER_LIST_SIZE             Sets the `SETTINGS_MAX_HEADER_LIST_SIZE` setting
         // QUICHE_H3_QPACK_BLOCKED_STREAMS            Sets the `SETTINGS_QPACK_BLOCKED_STREAMS` setting
         // QUICHE_H3_QPACK_MAX_TABLE_CAPACITY         Sets the `SETTINGS_QPACK_BLOCKED_STREAMS` setting
         // --------------------------------------------------------------------------------------------------------------------------------------

         quiche_max_packet_size = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_MAX_PACKET_SIZE"), 1350);

         U_SYSCALL_VOID(quiche_config_set_max_packet_size, "%p,%lu", qconfig, quiche_max_packet_size);

         long param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_MAX_IDLE_TIMEOUT"), 180000);
         bool param1 = UServer_Base::pcfg->readBoolean(U_CONSTANT_TO_PARAM("QUICHE_DISABLE_ACTIVE_MIGRATION"), true);

         U_SYSCALL_VOID(quiche_config_set_max_idle_timeout,         "%p,%lu", qconfig, param0);
         U_SYSCALL_VOID(quiche_config_set_disable_active_migration, "%p,%b",  qconfig, param1);

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_CC_ALGORITHM"), 0); // QUICHE_CC_RENO = 0

         U_SYSCALL_VOID(quiche_config_set_cc_algorithm, "%p,%lu", qconfig, (quiche_cc_algorithm)param0);

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

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_MAX_ACK_DELAY"), 25);

         U_SYSCALL_VOID(quiche_config_set_max_ack_delay, "%p,%lu", qconfig, param0);

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_ACK_DELAY_EXPONENT"), 3);

         U_SYSCALL_VOID(quiche_config_set_ack_delay_exponent, "%p,%lu", qconfig, param0);

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_H3_MAX_HEADER_LIST_SIZE"), 16384);

         U_SYSCALL_VOID(quiche_h3_config_set_max_header_list_size, "%p,%lu", http3_config, param0);

         if (UServer_Base::pcfg->readBoolean(U_CONSTANT_TO_PARAM("QUICHE_ENABLE_EARLY_DATA"), true)) U_SYSCALL_VOID(quiche_config_enable_early_data, "%p", qconfig);

         /** TODO

         void quiche_config_log_keys(quiche_config *config);
         void quiche_config_grease(quiche_config *config, bool v);
         void quiche_config_verify_peer(quiche_config *config, bool v);

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_H3_QPACK_BLOCKED_STREAMS"), ???);

         U_SYSCALL_VOID(quiche_h3_config_set_qpack_blocked_streams, "%p,%lu", http3_config, param0);

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_H3_QPACK_MAX_TABLE_CAPACITY"), ???);

         U_SYSCALL_VOID(quiche_h3_config_set_qpack_max_table_capacity, "%p,%lu", http3_config, param0);
         */

         UServer_Base::pcfg->reset();
         }
      }

   U_RETURN(U_PLUGIN_HANDLER_OK);
}

bool UHTTP3::handlerRead()
{
   U_TRACE_NO_PARAM(0, "UHTTP3::handlerRead()")

   U_INTERNAL_ASSERT_POINTER(peers)
   U_INTERNAL_ASSERT(UServer_Base::budp)

   int rc;
   void* pconn;
   uint8_t pkt_type;
   uint32_t pkt_version;
   size_t scid_len, token_len;
   ssize_t done, written, sent;
   char* data = UClientImage_Base::rbuffer->data();
   int iBytesRead, fd = UServer_Base::socket->getFd();
   uint8_t token[U_MAX_TOKEN_LEN], scid[QUICHE_MAX_CONN_ID_LEN], out[U_MAX_DATAGRAM_SIZE];

   while (true)
      {
      USocket::resetPeerAddr();

      iBytesRead = U_SYSCALL(recvfrom, "%d,%p,%u,%u,%p,%p", fd, data, 65535, 0, (struct sockaddr*)&peer_addr, &peer_addr_len);

      if (iBytesRead <= 0)
         {
         if (errno != EAGAIN)
            {
            U_DEBUG("UHTTP3::handlerRead(): failed to receive");

            U_RETURN(false);
            }

         break;
         }

      U_INTERNAL_DUMP("BytesRead(%u) = %#.*S", iBytesRead, iBytesRead, data)

      u__memcpy(&USocket::peer_addr, &peer_addr, USocket::peer_addr_len = peer_addr_len, __PRETTY_FUNCTION__);

         scid_len =
      conn_id_len = QUICHE_MAX_CONN_ID_LEN;
        token_len = U_MAX_TOKEN_LEN;

      // Extracts version, type, source / destination connection ID and address verification token from the packet in buffer
      rc = U_SYSCALL(quiche_header_info, "%p,%u,%u,%p,%p,%p,%p,%p,%p,%p,%p", (const uint8_t*)data, iBytesRead, U_LOCAL_CONN_ID_LEN,
                                                                             &pkt_version, &pkt_type, scid, &scid_len, conn_id, &conn_id_len, token, &token_len);

      if (rc < 0)
         {
         U_DEBUG("UHTTP3::handlerRead(): failed to parse header: %d", rc);

         U_RETURN(false);
         }

      U_INTERNAL_DUMP("\n"
                      "   scid(%u) = %#.*S\n"
                      "  token(%u) = %#.*S\n"
                      "conn_id(%u) = %#.*S\n"
                      "pkt_version = %d pkt_type = %d",
                      scid_len, scid_len, scid, token_len, token_len, token, conn_id_len, conn_id_len, conn_id, pkt_version, pkt_type)

      // Lookup a connection based on the packet's connection ID. If there is no connection matching, create a new one
      pconn = (peers->empty() ? U_NULLPTR : peers->at((const char*)conn_id, conn_id_len));

      U_INTERNAL_DUMP("pconn = %p", pconn)

      if (pconn == U_NULLPTR)
         {
         const char* pkt = "vneg";

         if (pkt_type != Initial)
            {
            U_DEBUG("UHTTP3::handlerRead(): packet is NOT initial: %d", pkt_type);

            continue;
            }

         // Client Initial packets must be at least 1200 bytes
         if (iBytesRead < QUICHE_MIN_CLIENT_INITIAL_LEN)
            {
            U_DEBUG("UHTTP3::handlerRead(): quic initial packet is too short: %d", iBytesRead);

            continue;
            }

         // Returns true if the given protocol version is supported
         if (U_SYSCALL(quiche_version_is_supported, "%u", pkt_version) == false)
            {
            U_DEBUG("UHTTP3::handlerRead(): version negotiation");

            // Writes a version negotiation packet
            written = U_SYSCALL(quiche_negotiate_version, "%p,%u,%p,%u,%p,%u", scid, scid_len, conn_id, conn_id_len, out, sizeof(out));

pkt:        if (written < 0)
               {
               U_DEBUG("UHTTP3::handlerRead(): failed to create %s packet: %d", pkt, written);

               U_RETURN(false);
               }

            sent = U_SYSCALL(sendto, "%d,%p,%u,%u,%p,%d", fd, out, written, 0, (struct sockaddr*)&peer_addr, peer_addr_len);

            if (sent == written)
               {
               U_DEBUG("UHTTP3::handlerRead(): sent %u bytes", sent);
               }
            else
               {
               U_DEBUG("UHTTP3::handlerRead(): failed to send");
               }

            U_RETURN(false);
            }

         if (token_len == 0)
            {
            uint8_t odcid[QUICHE_MAX_CONN_ID_LEN];

            // Generate a stateless retry token. The token includes the static string `"quiche"` followed
            // by the IP address of the client and by the original destination connection ID generated by the client

            U_DEBUG("UHTTP3::handlerRead(): stateless retry");

            (void) memcpy(token, U_CONSTANT_TO_PARAM("quiche"));
            (void) memcpy(token + U_CONSTANT_SIZE("quiche"), &peer_addr, peer_addr_len);
            (void) memcpy(token + U_CONSTANT_SIZE("quiche") + peer_addr_len, conn_id, conn_id_len);

            token_len = U_CONSTANT_SIZE("quiche") + peer_addr_len + conn_id_len;

            u__memcpy(odcid, conn_id, conn_id_len, __PRETTY_FUNCTION__);

            // Writes a retry packet
            written = U_SYSCALL(quiche_retry, "%p,%u,%p,%u,%p,%u,%p,%u,%p,%u", scid, scid_len, conn_id, conn_id_len, odcid, conn_id_len, token, token_len, out, sizeof(out));

            pkt = "retry";

            goto pkt;
            }

         // Validates a stateless retry token. This checks that the ticket includes the `"quiche"` static string, and that the client IP address matches the address stored in the ticket
         if (token_len <   U_CONSTANT_SIZE("quiche") ||
             memcmp(token, U_CONSTANT_TO_PARAM("quiche")))
            {
            U_DEBUG("UHTTP3::handlerRead(): invalid address validation token");

            U_RETURN(false);
            }

         token_len -= U_CONSTANT_SIZE("quiche");

         const char* ptr = (const char*)&token[0] + U_CONSTANT_SIZE("quiche");

         if (token_len < peer_addr_len ||
             memcmp(ptr, &peer_addr, peer_addr_len))
            {
            U_DEBUG("UHTTP3::handlerRead(): invalid address validation token");

            U_RETURN(false);
            }

         ptr       += peer_addr_len;
         token_len -= peer_addr_len;

         if (conn_id_len != token_len) // The token was not valid, meaning the retry failed, so drop the packet
            {
            U_DEBUG("UHTTP3::handlerRead(): invalid address validation token");

            U_RETURN(false);
            }

         U_INTERNAL_ASSERT_EQUALS(conn.conn, U_NULLPTR)

         // Reuse the source connection ID we sent in the Retry packet, instead of changing it again. Creates a new server-side connection
         conn.conn = (quiche_conn*) U_SYSCALL(quiche_accept, "%p,%u,%p,%u,%p", conn_id, conn_id_len, (const uint8_t*)ptr, token_len, qconfig);

         if (conn.conn == U_NULLPTR)
            {
            U_DEBUG("UHTTP3::handlerRead(): failed to create connection");

            U_RETURN(false);
            }

         peers->insert((const char*)conn_id, conn_id_len, (const UClientImage_Base*)&conn);
         }

      // Processes QUIC packets received from the peer
      done = U_SYSCALL(quiche_conn_recv, "%p,%p,%u", conn.conn, (uint8_t*)data, iBytesRead);

      if (done == QUICHE_ERR_DONE)
         {
         U_DEBUG("UHTTP3::handlerRead(): done reading");

         U_RETURN(true);
         }

      if (done < 0)
         {
         U_DEBUG("UHTTP3::handlerRead(): failed to process packet: %d", done);

         U_RETURN(false);
         }

      U_DEBUG("UHTTP3::handlerRead(): recv %d bytes", done);

      if (U_SYSCALL(quiche_conn_is_in_early_data, "%p", conn.conn) || // the connection has a pending handshake that has progressed
          U_SYSCALL(quiche_conn_is_established,   "%p", conn.conn))    // the connection handshake is complete
         {
         U_DEBUG("UHTTP3::handlerRead(): connection handshake is complete");

         U_INTERNAL_ASSERT_EQUALS(conn.http3, U_NULLPTR)

         // Creates a new HTTP/3 connection using the provided QUIC connection
         conn.http3 = (quiche_h3_conn*) U_SYSCALL(quiche_h3_conn_new_with_transport, "%p,%p", conn.conn, http3_config);

         if (conn.http3 == U_NULLPTR)
            {
            U_DEBUG("UHTTP3::handlerAccept(): failed to create HTTP/3 connection");

            U_RETURN(false);
            }

         quiche_h3_event* ev;

         while (true)
            {
            // Processes HTTP/3 data received from the peer
            done = U_SYSCALL(quiche_h3_conn_poll, "%p,%p,%p", conn.http3, conn.conn, &ev);

            if (done < 0) break;

            switch (quiche_h3_event_type(ev))
               {
               case QUICHE_H3_EVENT_HEADERS:
                  {
                  // Iterates over the headers in the event. The `cb` callback will be called for each header in `ev`. `cb` should check the validity of
                  // pseudo-headers and headers. If `cb` returns any value other than `0`, processing will be interrupted and the value is returned to the caller
                  rc = U_SYSCALL(quiche_h3_event_for_each_header, "%p,%p,%p", ev, for_each_header, NULL);

                  if (rc != 0)
                     {
                     U_DEBUG("UHTTP3::handlerRead(): failed to process headers");
                     }

                  /*
                  quiche_h3_header headers[] = {
                     {
                     .name = (const uint8_t *) ":status",
                     .name_len = sizeof(":status") - 1,

                     .value = (const uint8_t *) "200",
                     .value_len = sizeof("200") - 1,
                     },

                     {
                     .name = (const uint8_t *) "server",
                     .name_len = sizeof("server") - 1,

                     .value = (const uint8_t *) "quiche",
                     .value_len = sizeof("quiche") - 1,
                     },

                     {
                     .name = (const uint8_t *) "content-length",
                     .name_len = sizeof("content-length") - 1,

                     .value = (const uint8_t *) "5",
                     .value_len = sizeof("5") - 1,
                     },
                  };

                  quiche_h3_send_response(conn_io->http3, conn_io->conn, s, headers, 3, true);

                  quiche_h3_send_body(conn_io->http3, conn_io->conn, s, (uint8_t *) "byez\n", 5, true);
                  */
                  break;
                  }

               case QUICHE_H3_EVENT_DATA:
                  {
                  U_DEBUG("UHTTP3::handlerRead(): got HTTP data");

                  break;
                  }

               case QUICHE_H3_EVENT_FINISHED:
               break;
               }

            U_SYSCALL_VOID(quiche_h3_event_free, "%p", ev);
            }
         }
      }

   while (true)
      {
      written = U_SYSCALL(quiche_conn_send, "%p,%p,%u", conn.conn, out, sizeof(out));

      if (written == QUICHE_ERR_DONE)
         {
         U_DEBUG("UHTTP3::handlerRead(): done writing");

         U_RETURN(false);
         }

      if (written < 0)
         {
         U_DEBUG("UHTTP3::handlerRead(): failed to create packet: %d", written);

         U_RETURN(false);
         }

      sent = U_SYSCALL(sendto, "%d,%p,%u,%u,%p,%d", fd, out, written, 0, (struct sockaddr*)&USocket::peer_addr, USocket::peer_addr_len);

      if (sent != written)
         {
         U_DEBUG("UHTTP3::handlerRead(): failed to send");

         U_RETURN(false);
         }

      U_DEBUG("UHTTP3::handlerRead(): sent %u bytes", sent);
      }

// UClientImage_Base::rbuffer->size_adjust(iBytesRead);

   U_RETURN(true);
}

bool UHTTP3::handlerAccept()
{
   U_TRACE_NO_PARAM(0, "UHTTP3::handlerAccept()")

   U_INTERNAL_ASSERT_POINTER(conn.conn)
   U_INTERNAL_ASSERT_POINTER(conn.http3)
   U_INTERNAL_ASSERT(UServer_Base::budp)
   U_INTERNAL_ASSERT_POINTER(UServer_Base::pClientImage)

   UServer_Base::pClientImage->conn  = conn.conn;
   UServer_Base::pClientImage->http3 = conn.http3;

   conn.conn  = U_NULLPTR;
   conn.http3 = U_NULLPTR;

   UServer_Base::pClientImage->socket->setRemoteAddressAndPort();

   peers->replace((const char*)conn_id, conn_id_len, UServer_Base::pClientImage);

   U_RETURN(true);
}
