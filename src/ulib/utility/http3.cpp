// ============================================================================
//
// = LIBRARY
//    ULib - c++ library
//
// = FILENAME
//    http3.cpp - HTTP/3 utility 
//
// = AUTHOR
//    Stefano Casazza + Victor Stewart
//
// ============================================================================

#include <ulib/utility/uhttp.h>
#include <ulib/utility/http3.h>

uint32_t           UHTTP3::headers_len;
quiche_h3_conn*    UHTTP3::http3;
quiche_h3_header*  UHTTP3::headers;
quiche_h3_config*  UHTTP3::http3_config;
UHashMap<UString>* UHTTP3::itable; // headers request

int UHTTP3::loadConfigParam()
{
   U_TRACE_NO_PARAM(0, "UHTTP3::loadConfigParam()")

   if (UQuic::loadConfigParam() == U_PLUGIN_HANDLER_OK)
      {
      // Stores configuration shared between multiple connections
      http3_config = (quiche_h3_config*) U_SYSCALL_NO_PARAM(quiche_h3_config_new); // Creates an HTTP/3 config object with default settings values

      if (http3_config == U_NULLPTR)
         {
         U_ERROR("Failed to create HTTP3 config");
         }

      // Configures the list of supported application protocols
      (void) U_SYSCALL(quiche_config_set_application_protos, "%p,%p,%u", UQuic::qconfig, (uint8_t*)U_CONSTANT_TO_PARAM(QUICHE_H3_APPLICATION_PROTOCOL));

      if (UServer_Base::pcfg->searchForObjectStream(U_CONSTANT_TO_PARAM("http3")))
         {
         UServer_Base::pcfg->table.clear();

         if (UServer_Base::pcfg->loadTable())
            {
            // --------------------------------------------------------------------------------------------------------------------------------------
            // userver_udp - http3 configuration parameters
            // --------------------------------------------------------------------------------------------------------------------------------------
            // QUICHE_H3_MAX_HEADER_LIST_SIZE             Sets the `SETTINGS_MAX_HEADER_LIST_SIZE` setting
            // QUICHE_H3_QPACK_BLOCKED_STREAMS            Sets the `SETTINGS_QPACK_BLOCKED_STREAMS` setting
            // QUICHE_H3_QPACK_MAX_TABLE_CAPACITY         Sets the `SETTINGS_QPACK_BLOCKED_STREAMS` setting
            // --------------------------------------------------------------------------------------------------------------------------------------

            /*
            long param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_H3_MAX_HEADER_LIST_SIZE"), 16384);

            U_SYSCALL_VOID(quiche_h3_config_set_max_header_list_size, "%p,%lu", http3_config, param0);
            */

            /** TODO

            param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_H3_QPACK_BLOCKED_STREAMS"), ???);

            U_SYSCALL_VOID(quiche_h3_config_set_qpack_blocked_streams, "%p,%lu", http3_config, param0);

            param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_H3_QPACK_MAX_TABLE_CAPACITY"), ???);

            U_SYSCALL_VOID(quiche_h3_config_set_qpack_max_table_capacity, "%p,%lu", http3_config, param0);
            */

            UServer_Base::pcfg->reset();
            }
         }
      }

   U_RETURN(U_PLUGIN_HANDLER_OK);
}

int UHTTP3::for_each_header(uint8_t* name, size_t name_len, uint8_t* value, size_t value_len, void* argp)
{
   U_TRACE(0, "UHTTP3::for_each_header(%.*S,%u,%.*S,%u,%p)", name_len, name, name_len, value_len, value, value_len, argp)

   /*
    * quiche: got HTTP header: ":method"(7)="GET"(3)
    * quiche: got HTTP header: ":scheme"(7)="https"(5)
    * quiche: got HTTP header: ":authority"(10)="127.0.0.1"(9)
    * quiche: got HTTP header: ":path"(5)="/"(1)
    * quiche: got HTTP header: "user-agent"(10)="quiche"(6)
    */

   U_INTERNAL_DUMP("quiche: got HTTP header: %.*S(%u)=%.*S(%u)", name_len, name, name_len, value_len, value, value_len)

   U_RETURN(0);
}

void UHTTP3::handlerRequest(quiche_conn* lconn, quiche_h3_conn* lh3)
{
   U_TRACE(0, "UHTTP3::handlerRequest(%p,%p)", lconn, lh3)

   U_INTERNAL_ASSERT_POINTER(lh3)
   U_INTERNAL_ASSERT_POINTER(lconn)

   int rc;
   int64_t s;
   quiche_h3_event* ev;

   while (true)
      {
      // Processes HTTP/3 data received from the peer
      s = U_SYSCALL(quiche_h3_conn_poll, "%p,%p,%p", lh3, lconn, &ev);

      if (s < 0)
         {
         if (UQuic::handlerRead(lconn, true) &&
             UQuic::processesPackets(lconn))
            {
            continue;
            }

         break;
         }

      U_INTERNAL_DUMP("quiche_h3_event_type(ev) = %d", quiche_h3_event_type(ev))

      switch (quiche_h3_event_type(ev))
         {
         case QUICHE_H3_EVENT_HEADERS:
            {
            if (itable->empty() == false)
               {
               U_DUMP_CONTAINER(*itable)

               itable->clear();
               }

            // Iterates over the headers in the event. The `cb` callback will be called for each header in `ev`. `cb` should check the validity of
            // pseudo-headers and headers. If `cb` returns any value other than `0`, processing will be interrupted and the value is returned to the caller

            rc = U_SYSCALL(quiche_h3_event_for_each_header, "%p,%p,%p", ev, for_each_header, 0);

            if (rc != 0)
               {
               U_DEBUG("UHTTP3::handlerRequest(): failed to process headers: %d", rc)
               }

            U_INTERNAL_DUMP("Host            = %.*S", U_HTTP_HOST_TO_TRACE)
            U_INTERNAL_DUMP("Range           = %.*S", U_HTTP_RANGE_TO_TRACE)
            U_INTERNAL_DUMP("Accept          = %.*S", U_HTTP_ACCEPT_TO_TRACE)
            U_INTERNAL_DUMP("Cookie          = %.*S", U_HTTP_COOKIE_TO_TRACE)
            U_INTERNAL_DUMP("Referer         = %.*S", U_HTTP_REFERER_TO_TRACE)
            U_INTERNAL_DUMP("User-Agent      = %.*S", U_HTTP_USER_AGENT_TO_TRACE)
            U_INTERNAL_DUMP("Content-Type    = %.*S", U_HTTP_CTYPE_TO_TRACE)
            U_INTERNAL_DUMP("Accept-Language = %.*S", U_HTTP_ACCEPT_LANGUAGE_TO_TRACE)

            break;
            }

         case QUICHE_H3_EVENT_DATA:
            {
            U_DEBUG("quiche: got HTTP data")

            break;
            }

         case QUICHE_H3_EVENT_FINISHED:
            {
            /* Sends an HTTP/3 response on the specified stream
            (void) U_SYSCALL(quiche_h3_send_response, "%p,%p,%lu,%p,%p,%b", lh3, lconn, s, headers, headers_len, false);

            // Sends an HTTP/3 body chunk on the given stream.
            (void) U_SYSCALL(quiche_h3_send_body, "%p,%p,%lu,%p,%p", lh3, lconn, s, (uint8_t*)U_STRING_TO_PARAM(*UHTTP::body), true);
            */

            break;
            }
         }

      U_SYSCALL_VOID(quiche_h3_event_free, "%p", ev);
      }
}

bool UHTTP3::handlerNewConnection()
{
   U_TRACE_NO_PARAM(0, "UHTTP3::handlerNewConnection()")

   http3 = U_NULLPTR;

   if (UQuic::handlerNewConnection())
      {
      // Creates a new HTTP/3 connection using the provided QUIC connection
      http3 = (quiche_h3_conn*) U_SYSCALL(quiche_h3_conn_new_with_transport, "%p,%p", UQuic::conn, http3_config);

      if (http3 == U_NULLPTR)
         {
         U_DEBUG("UHTTP3::handlerNewConnection(): failed to create HTTP/3 connection")

         U_RETURN(false);
         }

      U_RETURN(true);
      }

   U_RETURN(false);
}
