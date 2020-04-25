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

#define U_MAX_DATAGRAM_SIZE 1350

#define U_MAX_TOKEN_LEN \
    sizeof("quiche")-1 + \
    sizeof(struct sockaddr_storage) + \
    QUICHE_MAX_CONN_ID_LEN

uint32_t          UHTTP3::quiche_max_packet_size = 1350;
quiche_config*    UHTTP3::qconfig;
quiche_h3_config* UHTTP3::http3_config;

UHttp3ConnIo*                     UHTTP3::data_conn_io;
UHashMap<UClientImage_Base*>*     UHTTP3::peers;
URDBObjectHandler<UDataStorage*>* UHTTP3::db;

// define method VIRTUAL of class UDataStorage

char* UHttp3ConnIo::toBuffer()
{
   U_TRACE_NO_PARAM(0, "UHttp3ConnIo::toBuffer()")

   unsigned char* p = (unsigned char*)u_buffer;

// u_buffer_len = U_SYSCALL(i2d_SSL_SESSION, "%p,%p", sess, &p);

   U_INTERNAL_ASSERT_MAJOR(u_buffer_len, 0)

   buffer_len = u_buffer_len;

   U_RETURN(u_buffer);
}

void UHttp3ConnIo::fromData(const char* ptr, uint32_t len)
{
   U_TRACE(0, "UHttp3ConnIo::fromData(%.*S,%u)", len, ptr, len)

   U_INTERNAL_ASSERT_POINTER(ptr)
}

/*
int USSLSession::newEntry(SSL* ssl, SSL_SESSION* _sess)
{
   U_TRACE(0, "USSLSession::newEntry(%p,%p)", ssl, _sess)

   U_INTERNAL_ASSERT_POINTER(UHTTP3::db)

   unsigned int idlen;
   const unsigned char* id;

   UHTTP3::db->insertDataStorage((const char*)id, idlen);

   U_RETURN(0);
}

void* UHttp3ConnIo::getEntry(unsigned char* id, int len)
{
   U_TRACE(0, "UHttp3ConnIo::getEntry(%.*S,%d)", len, id, len)

   U_INTERNAL_ASSERT_POINTER(UHTTP3::db)

   UHTTP3::db->getDataStorage((const char*)id, (uint32_t)len);

   U_RETURN_POINTER(sess, SSL_SESSION);
}

void UHttp3ConnIo::removeEntry(SSL_SESSION* _sess)
{
   U_TRACE(0, "UHttp3ConnIo::removeEntry(%p)", _sess)

   U_INTERNAL_ASSERT_POINTER(UHTTP3::db)

   int result;

   unsigned int idlen;
   const unsigned char* id;

   result = UHTTP3::db->remove((const char*)id, (uint32_t)idlen);

   // -2: The entry was already marked deleted in the hash-tree

   if (result &&
       result != -2)
      {
      U_WARNING("Remove of HTTP3 connection Information on db failed with error %d", result);
      }
}
*/

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

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_H3_MAX_HEADER_LIST_SIZE"), 16384);

         U_SYSCALL_VOID(quiche_h3_config_set_max_header_list_size, "%p,%lu", http3_config, param0);

         if (UServer_Base::pcfg->readBoolean(U_CONSTANT_TO_PARAM("QUICHE_ENABLE_EARLY_DATA"), true)) U_SYSCALL_VOID(quiche_config_enable_early_data, "%p", qconfig);

         /** TODO

         void quiche_config_log_keys(quiche_config *config);
         void quiche_config_grease(quiche_config *config, bool v);
         void quiche_config_verify_peer(quiche_config *config, bool v);
         void quiche_config_set_max_ack_delay(quiche_config *config, uint64_t v);
         void quiche_config_set_ack_delay_exponent(quiche_config *config, uint64_t v);

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

void UHTTP3::initDb()
{
   U_TRACE_NO_PARAM(0, "UHTTP3::initDb()")

   U_INTERNAL_ASSERT_EQUALS(db, U_NULLPTR)
   U_INTERNAL_ASSERT_EQUALS(data_conn_io, U_NULLPTR)

   U_NEW(UHttp3ConnIo, data_conn_io, UHttp3ConnIo);
   U_NEW(URDBObjectHandler<UDataStorage*>, db, URDBObjectHandler<UDataStorage*>(U_STRING_FROM_CONSTANT("../db/http3.db"), -1, data_conn_io));

   if (db->open(8 * U_1M, false, true, true, U_SRV_LOCK_DB_HTTP3)) // NB: we don't want truncate (we have only the journal)...
      {
      U_SRV_LOG("db HTTP3 initialization success");

      db->reset(); // Initialize the cache to contain no entries
      }
   else
      {
      U_SRV_LOG("WARNING: db HTTP3 initialization failed");

      U_DELETE(db)

      db = U_NULLPTR;
      }
}

void UHTTP3::clearDb()
{
   U_TRACE_NO_PARAM(0, "UHTTP3::clearDb()")

   U_INTERNAL_ASSERT_POINTER(db)

   db->close();

   U_DELETE(data_conn_io)

   U_DELETE(db)

   db = U_NULLPTR;
}

void UHTTP3::handlerRequest()
{
   U_TRACE_NO_PARAM(0, "UHTTP3::handlerRequest()")

   U_INTERNAL_ASSERT(UServer_Base::budp)

   int rc;
   ssize_t done;
   uint8_t pkt_type;
   UClientImage_Base* peer;
   char* data = UClientImage_Base::rbuffer->data();
   size_t scid_len, dcid_len, odcid_len, token_len;
   uint32_t pkt_version, sz = UClientImage_Base::rbuffer->size();
   uint8_t token[U_MAX_TOKEN_LEN], scid[QUICHE_MAX_CONN_ID_LEN], dcid[QUICHE_MAX_CONN_ID_LEN], odcid[QUICHE_MAX_CONN_ID_LEN];

loop:
    scid_len = QUICHE_MAX_CONN_ID_LEN;
    dcid_len = QUICHE_MAX_CONN_ID_LEN;
   odcid_len = QUICHE_MAX_CONN_ID_LEN;
   token_len = U_MAX_TOKEN_LEN;

   // Extracts version, type, source / destination connection ID and address verification token from the packet in buffer

   rc = U_SYSCALL(quiche_header_info, "%p,%u,%u,%p,%p,%p,%p,%p,%p,%p,%p", (const uint8_t*)data, sz, U_LOCAL_CONN_ID_LEN,
                                                                          &pkt_version, &pkt_type, scid, &scid_len, dcid, &dcid_len, token, &token_len);

   if (rc < 0)
      {
      U_WARNING("UHTTP3::handlerRequest(): failed to parse header: %d", rc);

      goto end;
      }

   U_INTERNAL_ASSERT_POINTER(peers)

   peer = (peers->empty() ? U_NULLPTR : peers->at((const char*)dcid, dcid_len));

   if (peer == U_NULLPTR)
      {
      const char* pkt = "veg";
      uint8_t out[U_MAX_DATAGRAM_SIZE];
      ssize_t written, sent, iBytesRead;
      int fd = UServer_Base::csocket->getFd();

      if (U_SYSCALL(quiche_version_is_supported, "%u", pkt_version) == false)
         {
         U_WARNING("UHTTP3::handlerRequest(): version negotiation");

         // Writes a version negotiation packet
         written = U_SYSCALL(quiche_negotiate_version, "%p,%u,%p,%u,%p,%u", scid, scid_len, dcid, dcid_len, out, sizeof(out));

retry:   if (written < 0)
            {
            U_WARNING("UHTTP3::handlerRequest(): failed to create %s packet: %d", pkt, written);

            goto end;
            }

         sent = U_SYSCALL(sendto, "%d,%p,%u,%u,%p,%d", fd, out, written, 0, (struct sockaddr*)&USocket::peer_addr, USocket::peer_addr_len);

         if (sent != written)
            {
            U_WARNING("UHTTP3::handlerRequest(): failed to send");

            goto end;
            }

         U_WARNING("UHTTP3::handlerRequest(): sent %u bytes", sent);

         iBytesRead = U_SYSCALL(recvfrom, "%d,%p,%u,%u,%p,%p", fd, data, 65535, 0, (struct sockaddr*)&USocket::peer_addr, &USocket::peer_addr_len);

         if (iBytesRead <= 0)
            {
            U_WARNING("UHTTP3::handlerRequest(): failed to receive");

            goto end;
            }

         if (sz != iBytesRead) UClientImage_Base::rbuffer->size_adjust_force(sz = iBytesRead);

         goto loop;
         }

      if (token_len == 0)
         {
         U_DEBUG("UHTTP3::handlerRequest(): stateless retry");

         (void) memcpy(token, U_CONSTANT_TO_PARAM("quiche"));
         (void) memcpy(token + U_CONSTANT_SIZE("quiche"), &USocket::peer_addr, USocket::peer_addr_len);
         (void) memcpy(token + U_CONSTANT_SIZE("quiche") + USocket::peer_addr_len, dcid, dcid_len);

         token_len = U_CONSTANT_SIZE("quiche") + USocket::peer_addr_len + dcid_len;

         // Writes a retry packet
         written = U_SYSCALL(quiche_retry, "%p,%u,%p,%u,%p,%u,%p,%u,%p,%u", scid, scid_len, dcid, dcid_len, dcid, dcid_len, token, token_len, out, sizeof(out));

         pkt = "retry";

         goto retry;
         }

      if (token_len <   U_CONSTANT_SIZE("quiche") ||
          memcmp(token, U_CONSTANT_TO_PARAM("quiche")))
         {
         U_WARNING("UHTTP3::handlerRequest(): invalid address validation token");

         goto end;
         }

      token_len -= U_CONSTANT_SIZE("quiche");

      const char* ptr = (const char*)&token[0] + U_CONSTANT_SIZE("quiche");

      if (token_len < USocket::peer_addr_len ||
          memcmp(ptr, &USocket::peer_addr, USocket::peer_addr_len))
         {
         U_WARNING("UHTTP3::handlerRequest(): invalid address validation token");

         goto end;
         }

      ptr       += USocket::peer_addr_len;
      token_len -= USocket::peer_addr_len;

      if (odcid_len < token_len)
         {
         U_WARNING("UHTTP3::handlerRequest(): invalid address validation token");

         goto end;
         }

      u__memcpy(odcid, ptr, odcid_len = token_len, __PRETTY_FUNCTION__);

      U_INTERNAL_DUMP("UServer_Base::nClientIndex = %u", UServer_Base::nClientIndex)

      U_INTERNAL_ASSERT_MINOR(UServer_Base::nClientIndex, UNotifier::max_connection)

      UServer_Base::pClientImage = UServer_Base::vClientImage + UServer_Base::nClientIndex;

loop1:
      U_INTERNAL_ASSERT_MINOR(UServer_Base::pClientImage, UServer_Base::eClientImage)
      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD)

      U_INTERNAL_DUMP("\n----------------------------------------\n"
                      "vClientImage[%u].last_event        = %#3D\n"
                      "vClientImage[%u].sfd               = %d\n"
                      "vClientImage[%u].http3connio->cid  = %#.16S\n"
                      "vClientImage[%u].http3connio->conn = %p"
                      "\n----------------------------------------\n",
                      (UServer_Base::pClientImage - UServer_Base::vClientImage), UServer_Base::pClientImage->last_event,
                      (UServer_Base::pClientImage - UServer_Base::vClientImage), UServer_Base::pClientImage->sfd,
                      (UServer_Base::pClientImage - UServer_Base::vClientImage), UServer_Base::pClientImage->http3connio.cid,
                      (UServer_Base::pClientImage - UServer_Base::vClientImage), UServer_Base::pClientImage->http3connio.conn)

      if (UServer_Base::pClientImage->http3connio.conn) // busy
         {
         /*
         if (ptime) // NB: we check if the connection is idle...
            {
            U_gettimeofday // NB: optimization if it is enough a time resolution of one second...

            if ((u_now->tv_sec - UServer_Base::pClientImage->last_event) >= ptime->UTimeVal::tv_sec)
               {
               if (handlerTimeoutConnection(UServer_Base::pClientImage))
                  {
                  UNotifier::handlerDelete((UEventFd*)UServer_Base::pClientImage);

                  goto try_accept;
                  }
               }
            }

try_next:
         */
         if (++UServer_Base::pClientImage >= UServer_Base::eClientImage)
            {
            U_INTERNAL_ASSERT_POINTER(UServer_Base::vClientImage)

            UServer_Base::pClientImage = UServer_Base::vClientImage;
            }

         goto loop1;
         }

try_accept:
      U_INTERNAL_ASSERT_DIFFERS(U_ClientImage_parallelization, U_PARALLELIZATION_CHILD)

      UServices::generateKey(UServer_Base::pClientImage->http3connio.cid, U_NULLPTR);

      // Creates a new server-side connection
      UServer_Base::pClientImage->http3connio.conn = (quiche_conn*) U_SYSCALL(quiche_accept, "%p,%u,%p,%u,%p", UServer_Base::pClientImage->http3connio.cid, U_LOCAL_CONN_ID_LEN,
                                                                                                               odcid, odcid_len, qconfig);

      if (UServer_Base::pClientImage->http3connio.conn == U_NULLPTR)
         {
         U_WARNING("UHTTP3::handlerRequest(): failed to create connection");

         goto end;
         }

      peers->insert((const char*)UServer_Base::pClientImage->http3connio.cid, U_LOCAL_CONN_ID_LEN, UServer_Base::pClientImage);

      u__memcpy(&UServer_Base::pClientImage->http3connio.peer_addr, &USocket::peer_addr, UServer_Base::pClientImage->http3connio.peer_addr_len = USocket::peer_addr_len, __PRETTY_FUNCTION__);
      }

   // Processes QUIC packets received from the peer
   done = U_SYSCALL(quiche_conn_recv, "%p,%p,%u", UServer_Base::pClientImage->http3connio.conn, (uint8_t*)data, sz);

   if (done == QUICHE_ERR_DONE)
      {
      U_DEBUG("UHTTP3::handlerRequest(): done reading");

      goto iteration;
      }

   if (done < 0)
      {
      U_WARNING("UHTTP3::handlerRequest(): failed to process packet: %d", done);

      goto end;
      }

   U_DEBUG("UHTTP3::handlerRequest(): recv %d bytes", done);

   if (U_SYSCALL(quiche_conn_is_established, "%p", UServer_Base::pClientImage->http3connio.conn)) // Returns true if the connection handshake is complete
      {
      quiche_h3_event* ev;

      if (UServer_Base::pClientImage->http3connio.http3 == U_NULLPTR)
         {
         UServer_Base::pClientImage->http3connio.http3 = (quiche_h3_conn*) U_SYSCALL(quiche_h3_conn_new_with_transport, "%p,%p", UServer_Base::pClientImage->http3connio.conn, http3_config);

         if (UServer_Base::pClientImage->http3connio.http3 == U_NULLPTR)
            {
            U_WARNING("UHTTP3::handlerRequest(): failed to create HTTP/3 connection");

            goto end;
            }
         }

   /*
      while (1)
      {
      int64_t s = quiche_h3_conn_poll(conn_io->http3, conn_io->conn, &ev);

      if (s < 0) {
      break;
      }

      switch (quiche_h3_event_type(ev))
      {
      case QUICHE_H3_EVENT_HEADERS: {
      int rc = quiche_h3_event_for_each_header(ev, for_each_header, NULL);

      if (rc != 0) {
      fprintf(stderr, "failed to process headers\n");
      }

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

      break;
      }

      case QUICHE_H3_EVENT_DATA: {
      fprintf(stderr, "got HTTP data\n");
      break;
      }

      case QUICHE_H3_EVENT_FINISHED:
      break;
      }

      quiche_h3_event_free(ev);
      }
   */
      }

   // TODO

iteration:

end:
   UClientImage_Base::bnoheader = true;

   UClientImage_Base::setRequestProcessed();
}
