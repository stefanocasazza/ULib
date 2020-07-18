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

#include <ulib/utility/uhttp.h>
#include <ulib/utility/http3.h>

bool              UHTTP3::bwait_for_data;
size_t            UHTTP3::scid_len;
size_t            UHTTP3::token_len;
size_t            UHTTP3::conn_id_len;
ssize_t           UHTTP3::sent;
ssize_t           UHTTP3::written;
uint8_t           UHTTP3::pkt_type;
uint8_t           UHTTP3::token[U_MAX_TOKEN_LEN];
uint8_t           UHTTP3::scid[QUICHE_MAX_CONN_ID_LEN];
uint8_t           UHTTP3::conn_id[QUICHE_MAX_CONN_ID_LEN];
uint32_t          UHTTP3::pkt_version;
uint32_t          UHTTP3::headers_len;
uint32_t          UHTTP3::peer_addr_len;
quiche_config*    UHTTP3::qconfig;
UHTTP3::conn_io   UHTTP3::conn;
quiche_h3_header* UHTTP3::headers;
quiche_h3_config* UHTTP3::http3_config;

struct sockaddr_storage       UHTTP3::peer_addr;
UHashMap<UClientImage_Base*>* UHTTP3::peers;

/** example
quiche: version negotiation
quiche: sent 43 bytes
quiche: recvfrom would block
quiche: stateless retry
quiche: sent 93 bytes
quiche: recvfrom would block
quiche: new connection
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Initial version=ff00001b dcid=671f151ec208965185946a0b70c42ce7 scid=c4f6a47cff6c39e26023d7f48e3b8555 token=71756963686502008e437f0000010000000000000000671f151ec208965185946a0b70c42ce7 len=1120 pn=2
quiche: 671f151ec208965185946a0b70c42ce7 rx frm CRYPTO off=0 len=512
quiche::tls: checking peer ALPN Ok("h3-27") against Ok("h3-27")
quiche::tls: 671f151ec208965185946a0b70c42ce7 write message lvl=Initial len=122
quiche::tls: 671f151ec208965185946a0b70c42ce7 set write secret lvl=Handshake
quiche::tls: 671f151ec208965185946a0b70c42ce7 write message lvl=Handshake len=1320
quiche::tls: 671f151ec208965185946a0b70c42ce7 set write secret lvl=OneRTT
quiche::tls: 671f151ec208965185946a0b70c42ce7 set read secret lvl=Handshake
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=587
quiche: recv 1200 bytes
quiche: recvfrom would block
quiche: 671f151ec208965185946a0b70c42ce7 tx pkt Initial version=ff00001b dcid=c4f6a47cff6c39e26023d7f48e3b8555 scid=671f151ec208965185946a0b70c42ce7 len=148 pn=0
quiche: 671f151ec208965185946a0b70c42ce7 tx frm ACK delay=421 blocks=[2..2]
quiche: 671f151ec208965185946a0b70c42ce7 tx frm CRYPTO off=0 len=122
quiche::recovery: 671f151ec208965185946a0b70c42ce7 timer=999.973416ms latest_rtt=0ns srtt=None min_rtt=0ns rttvar=0ns loss_time=[None, None, None] loss_probes=[0, 0, 0] cwnd=14520 ssthresh=18446744073709551615 bytes_in_flight=191 app_limited=true congestion_recovery_start_time=None delivered=0 delivered_time=30.902µs recent_delivered_packet_sent_time=31.248µs app_limited_at_pkt=0  hystart=window_end=None last_round_min_rtt=None current_round_min_rtt=None rtt_sample_count=0 lss_start_time=None  
quiche: sent 191 bytes
quiche: 671f151ec208965185946a0b70c42ce7 tx pkt Handshake version=ff00001b dcid=c4f6a47cff6c39e26023d7f48e3b8555 scid=671f151ec208965185946a0b70c42ce7 len=1154 pn=0
quiche: 671f151ec208965185946a0b70c42ce7 tx frm CRYPTO off=0 len=1134
quiche::recovery: 671f151ec208965185946a0b70c42ce7 timer=999.941479ms latest_rtt=0ns srtt=None min_rtt=0ns rttvar=0ns loss_time=[None, None, None] loss_probes=[0, 0, 0] cwnd=14520 ssthresh=18446744073709551615 bytes_in_flight=1387 app_limited=true congestion_recovery_start_time=None delivered=0 delivered_time=61.369µs recent_delivered_packet_sent_time=61.679µs app_limited_at_pkt=0  hystart=window_end=None last_round_min_rtt=None current_round_min_rtt=None rtt_sample_count=0 lss_start_time=None  
quiche: sent 1196 bytes
quiche: 671f151ec208965185946a0b70c42ce7 tx pkt Handshake version=ff00001b dcid=c4f6a47cff6c39e26023d7f48e3b8555 scid=671f151ec208965185946a0b70c42ce7 len=207 pn=1
quiche: 671f151ec208965185946a0b70c42ce7 tx frm CRYPTO off=1134 len=186
quiche::recovery: 671f151ec208965185946a0b70c42ce7 timer=999.916966ms latest_rtt=0ns srtt=None min_rtt=0ns rttvar=0ns loss_time=[None, None, None] loss_probes=[0, 0, 0] cwnd=14520 ssthresh=18446744073709551615 bytes_in_flight=1636 app_limited=true congestion_recovery_start_time=None delivered=0 delivered_time=85.64µs recent_delivered_packet_sent_time=85.955µs app_limited_at_pkt=0  hystart=window_end=None last_round_min_rtt=None current_round_min_rtt=None rtt_sample_count=0 lss_start_time=None  
quiche: sent 249 bytes
::quiche_conn_send(0x55e1c3a0ec00,0x7ffc47bf4e24,63900) = -1
quiche: done writing
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Initial version=ff00001b dcid=671f151ec208965185946a0b70c42ce7 scid=c4f6a47cff6c39e26023d7f48e3b8555 token=71756963686502008e437f0000010000000000000000671f151ec208965185946a0b70c42ce7 len=1200 pn=3
quiche: 671f151ec208965185946a0b70c42ce7 rx frm ACK delay=99 blocks=[0..0]
quiche::recovery: 671f151ec208965185946a0b70c42ce7 packet newly acked 0
quiche::recovery: 671f151ec208965185946a0b70c42ce7 timer=26.92279ms latest_rtt=977.22µs srtt=Some(977.22µs) min_rtt=977.22µs rttvar=488.61µs loss_time=[None, None, None] loss_probes=[0, 0, 0] cwnd=14520 ssthresh=18446744073709551615 bytes_in_flight=1445 app_limited=true congestion_recovery_start_time=None delivered=191 delivered_time=36.115µs recent_delivered_packet_sent_time=1.013735ms app_limited_at_pkt=0  hystart=window_end=None last_round_min_rtt=None current_round_min_rtt=None rtt_sample_count=0 lss_start_time=None  
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=1177
quiche: recv 1280 bytes
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Handshake version=ff00001b dcid=671f151ec208965185946a0b70c42ce7 scid=c4f6a47cff6c39e26023d7f48e3b8555 len=61 pn=0
quiche: 671f151ec208965185946a0b70c42ce7 rx frm ACK delay=61 blocks=[0..1]
quiche::recovery: 671f151ec208965185946a0b70c42ce7 packet newly acked 1
quiche::recovery: 671f151ec208965185946a0b70c42ce7 packet newly acked 0
quiche::recovery: 671f151ec208965185946a0b70c42ce7 timer=none latest_rtt=970.978µs srtt=Some(976.439µs) min_rtt=970.978µs rttvar=368.017µs loss_time=[None, None, None] loss_probes=[0, 0, 0] cwnd=14520 ssthresh=18446744073709551615 bytes_in_flight=0 app_limited=true congestion_recovery_start_time=None delivered=1636 delivered_time=29.267µs recent_delivered_packet_sent_time=1.070225ms app_limited_at_pkt=0  hystart=window_end=None last_round_min_rtt=None current_round_min_rtt=None rtt_sample_count=0 lss_start_time=None  
quiche: 671f151ec208965185946a0b70c42ce7 rx frm CRYPTO off=0 len=36
quiche::tls: 671f151ec208965185946a0b70c42ce7 set read secret lvl=OneRTT
quiche::tls: 671f151ec208965185946a0b70c42ce7 write message lvl=OneRTT len=396
quiche: 671f151ec208965185946a0b70c42ce7 connection established: proto=Ok("h3-27") cipher=Some(ChaCha20_Poly1305) curve=Some("X25519") sigalg=None resumed=false TransportParams { original_connection_id: None, max_idle_timeout: 5000, stateless_reset_token: None, max_packet_size: 1350, initial_max_data: 10000000, initial_max_stream_data_bidi_local: 1000000, initial_max_stream_data_bidi_remote: 1000000, initial_max_stream_data_uni: 1000000, initial_max_streams_bidi: 100, initial_max_streams_uni: 100, ack_delay_exponent: 3, max_ack_delay: 25, disable_active_migration: true, active_conn_id_limit: 0 }
quiche: 671f151ec208965185946a0b70c42ce7 dropped epoch 0 state
quiche: recv 101 bytes
quiche::h3: 671f151ec208965185946a0b70c42ce7 open GREASE stream 15
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Short dcid=671f151ec208965185946a0b70c42ce7 key_phase=false len=40 pn=0
quiche: 671f151ec208965185946a0b70c42ce7 rx frm STREAM id=2 off=0 len=19 fin=false
quiche: recv 57 bytes
quiche::h3: 671f151ec208965185946a0b70c42ce7 stream id 2 is readable
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 1 bytes on stream 2
quiche::h3: 671f151ec208965185946a0b70c42ce7 open peer's control stream 2
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 1 bytes on stream 2
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 1 bytes on stream 2
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 16 bytes on stream 2
quiche::h3: 671f151ec208965185946a0b70c42ce7 rx frm SETTINGS max_headers=None, qpack_max_table=None, qpack_blocked=None  stream=2
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Short dcid=671f151ec208965185946a0b70c42ce7 key_phase=false len=22 pn=1
quiche: 671f151ec208965185946a0b70c42ce7 rx frm STREAM id=6 off=0 len=1 fin=false
quiche: recv 39 bytes
quiche::h3: 671f151ec208965185946a0b70c42ce7 stream id 6 is readable
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 1 bytes on stream 6
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Short dcid=671f151ec208965185946a0b70c42ce7 key_phase=false len=22 pn=2
quiche: 671f151ec208965185946a0b70c42ce7 rx frm STREAM id=10 off=0 len=1 fin=false
quiche: recv 39 bytes
quiche::h3: 671f151ec208965185946a0b70c42ce7 stream id 10 is readable
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 1 bytes on stream 10
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Short dcid=671f151ec208965185946a0b70c42ce7 key_phase=false len=47 pn=3
quiche: 671f151ec208965185946a0b70c42ce7 rx frm STREAM id=14 off=0 len=26 fin=false
quiche: recv 64 bytes
quiche::h3: 671f151ec208965185946a0b70c42ce7 stream id 14 is readable
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 1 bytes on stream 14
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 7 bytes on stream 14
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Short dcid=671f151ec208965185946a0b70c42ce7 key_phase=false len=81 pn=4
quiche: 671f151ec208965185946a0b70c42ce7 rx frm STREAM id=0 off=0 len=60 fin=true
quiche: recv 98 bytes
quiche::h3: 671f151ec208965185946a0b70c42ce7 stream id 0 is readable
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 1 bytes on stream 0
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 7 bytes on stream 0
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 1 bytes on stream 0
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 0 bytes on stream 0
quiche::h3: 671f151ec208965185946a0b70c42ce7 rx frm UNKNOWN stream=0
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 1 bytes on stream 0
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 7 bytes on stream 0
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 1 bytes on stream 0
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 18 bytes on stream 0
quiche::h3: 671f151ec208965185946a0b70c42ce7 rx frm UNKNOWN stream=0
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 1 bytes on stream 0
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 1 bytes on stream 0
quiche::h3::stream: 671f151ec208965185946a0b70c42ce7 read 22 bytes on stream 0
quiche::h3: 671f151ec208965185946a0b70c42ce7 rx frm HEADERS len=22 stream=0
quiche::h3::qpack::decoder: Header count=0 base=0
quiche::h3::qpack::decoder: Indexed index=17 static=true
quiche::h3::qpack::decoder: Indexed index=23 static=true
quiche::h3::qpack::decoder: Literal name_idx=0 static=true value="127.0.0.1"
quiche::h3::qpack::decoder: Indexed index=1 static=true
quiche::h3::qpack::decoder: Literal name_idx=95 static=true value="quiche"
quiche: got HTTP header: ":method"(7)="GET"(3)
quiche: got HTTP header: ":scheme"(7)="https"(5)
quiche: got HTTP header: ":authority"(10)="127.0.0.1"(9)
quiche: got HTTP header: ":path"(5)="/"(1)
quiche: got HTTP header: "user-agent"(10)="quiche"(6)
quiche::h3: 671f151ec208965185946a0b70c42ce7 tx frm GREASE stream=0
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Handshake version=ff00001b dcid=671f151ec208965185946a0b70c42ce7 scid=c4f6a47cff6c39e26023d7f48e3b8555 len=21 pn=1
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 61 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Handshake version=ff00001b dcid=671f151ec208965185946a0b70c42ce7 scid=c4f6a47cff6c39e26023d7f48e3b8555 len=21 pn=2
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 61 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Short dcid=671f151ec208965185946a0b70c42ce7 key_phase=false len=21 pn=5
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 38 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Short dcid=671f151ec208965185946a0b70c42ce7 key_phase=false len=21 pn=6
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 38 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Handshake version=ff00001b dcid=671f151ec208965185946a0b70c42ce7 scid=c4f6a47cff6c39e26023d7f48e3b8555 len=21 pn=3
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 61 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Handshake version=ff00001b dcid=671f151ec208965185946a0b70c42ce7 scid=c4f6a47cff6c39e26023d7f48e3b8555 len=21 pn=4
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 61 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Short dcid=671f151ec208965185946a0b70c42ce7 key_phase=false len=21 pn=7
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 38 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Short dcid=671f151ec208965185946a0b70c42ce7 key_phase=false len=21 pn=8
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 38 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Handshake version=ff00001b dcid=671f151ec208965185946a0b70c42ce7 scid=c4f6a47cff6c39e26023d7f48e3b8555 len=21 pn=5
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 61 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Handshake version=ff00001b dcid=671f151ec208965185946a0b70c42ce7 scid=c4f6a47cff6c39e26023d7f48e3b8555 len=21 pn=6
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 61 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Short dcid=671f151ec208965185946a0b70c42ce7 key_phase=false len=21 pn=9
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 38 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Short dcid=671f151ec208965185946a0b70c42ce7 key_phase=false len=21 pn=10
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 38 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Handshake version=ff00001b dcid=671f151ec208965185946a0b70c42ce7 scid=c4f6a47cff6c39e26023d7f48e3b8555 len=21 pn=7
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 61 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
quiche: 671f151ec208965185946a0b70c42ce7 rx pkt Handshake version=ff00001b dcid=671f151ec208965185946a0b70c42ce7 scid=c4f6a47cff6c39e26023d7f48e3b8555 len=21 pn=8
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PING
quiche: 671f151ec208965185946a0b70c42ce7 rx frm PADDING len=3
quiche: recv 61 bytes
::quiche_h3_conn_poll(0x55e1c3a6fc10,0x55e1c3a0ec00,0x7ffc47c05858) = -1
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
         //
         // QUICHE_H3_MAX_HEADER_LIST_SIZE             Sets the `SETTINGS_MAX_HEADER_LIST_SIZE` setting
         // QUICHE_H3_QPACK_BLOCKED_STREAMS            Sets the `SETTINGS_QPACK_BLOCKED_STREAMS` setting
         // QUICHE_H3_QPACK_MAX_TABLE_CAPACITY         Sets the `SETTINGS_QPACK_BLOCKED_STREAMS` setting
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

         param0 = UServer_Base::pcfg->readLong(U_CONSTANT_TO_PARAM("QUICHE_H3_MAX_HEADER_LIST_SIZE"), 16384);

         U_SYSCALL_VOID(quiche_h3_config_set_max_header_list_size, "%p,%lu", http3_config, param0);
         */

         if (UServer_Base::pcfg->readBoolean(U_CONSTANT_TO_PARAM("QUICHE_ENABLE_EARLY_DATA"), false)) U_SYSCALL_VOID(quiche_config_enable_early_data, "%p", qconfig);

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

bool UHTTP3::parseHeader(const char* data, uint32_t iBytesRead)
{
   U_TRACE(0, "UHTTP3::parseHeader(%.*S,%u)", iBytesRead, data, iBytesRead)

   // Parse the QUIC packet's header

      scid_len =
   conn_id_len = QUICHE_MAX_CONN_ID_LEN;
     token_len = U_MAX_TOKEN_LEN;

   // Extracts version, type, source / destination connection ID and address verification token from the packet in buffer
   int rc = U_SYSCALL(quiche_header_info, "%p,%u,%u,%p,%p,%p,%p,%p,%p,%p,%p", (const uint8_t*)data, iBytesRead, U_LOCAL_CONN_ID_LEN,
                                                                              &pkt_version, &pkt_type, scid, &scid_len, conn_id, &conn_id_len, token, &token_len);

   if (rc < 0)
      {
      U_DEBUG("UHTTP3::parseHeader(): failed to parse header: %d", rc)

      U_RETURN(false);
      }

   U_INTERNAL_DUMP("scid(%u) = %#.*S token(%u) = %#.*S conn_id(%u) = %#.*S pkt_version = %p pkt_type = %u",
                   scid_len, scid_len, scid, token_len, token_len, token, conn_id_len, conn_id_len, conn_id, pkt_version, pkt_type)

   U_RETURN(true);
}

bool UHTTP3::flush_egress()
{
   U_TRACE_NO_PARAM(0, "UHTTP3::flush_egress()")

   uint8_t out[65536];
   uint32_t start = 0, remain = sizeof(out);

   while (true)
      {
      written = U_SYSCALL(quiche_conn_send, "%p,%p,%u", conn.conn, &out[start], remain);

      if (written == QUICHE_ERR_DONE)
         {
         U_DEBUG("quiche: done writing")

         break;
         }

      if (written < 0)
         {
         U_DEBUG("UHTTP3::flush_egress(): failed to create packet: %d", written)

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
      U_DEBUG("UHTTP3::flush_egress(): failed to send")

      U_RETURN(false);
      }

   if (U_SYSCALL(quiche_conn_is_closed, "%p", conn.conn))
      {
      quiche_stats stats;

      U_SYSCALL_VOID(quiche_conn_stats, "%p,%p", conn.conn, &stats);

      U_DEBUG("quiche: connection closed, recv=%u sent=%u lost=%u rtt=%u cwnd=%u", stats.recv, stats.sent, stats.lost, stats.rtt, stats.cwnd)

      U_SYSCALL_VOID(quiche_conn_free, "%p", conn.conn);
                                             conn.conn = U_NULLPTR;

      U_RETURN(false);
      }

   U_RETURN(true);
}

bool UHTTP3::handlerRead()
{
   U_TRACE_NO_PARAM(0, "UHTTP3::handlerRead()")

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

         // reported no read packets, we will then proceed with the send loop.

         if (conn.conn &&
             flush_egress() == false)
            {
            U_RETURN(false);
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

   if (memcmp(&peer_addr, &USocket::peer_addr, peer_addr_len) != 0)
      {
      // TODO

      U_WARNING("recvfrom() different address");

      U_RETURN(false);
      }

   if (parseHeader(ptr, iBytesRead))
      {
      UServer_Base::rbuffer->size_adjust_force(iBytesRead);

      // Lookup a connection based on the packet's connection ID. If there is no connection matching, create a new one

      if (lookup())
         {
         // TODO

         U_RETURN(false);
         }

      bwait_for_data = false;

      U_RETURN(true);
      }

   U_RETURN(false);
}

bool UHTTP3::handlerNewConnection()
{
   U_TRACE_NO_PARAM(0, "UHTTP3::handlerNewConnection()")

   U_INTERNAL_ASSERT_POINTER(peers)
   U_INTERNAL_ASSERT(UServer_Base::budp)
 
   int rc;
   int64_t s;
   ssize_t done;
   const char* ptr;
   quiche_h3_event* ev;
   const char* pkt = "vneg";
   uint8_t out[U_MAX_DATAGRAM_SIZE];

loop1:
   if (pkt_type != Initial)
      {
      U_DEBUG("UHTTP3::handlerNewConnection(): packet is NOT initial: %u", pkt_type)

      U_RETURN(false);
      }

   // Client Initial packets must be at least 1200 bytes
   if (UServer_Base::rbuffer->size() < QUICHE_MIN_CLIENT_INITIAL_LEN)
      {
      U_DEBUG("UHTTP3::handlerNewConnection(): quic initial packet is too short: %u", UServer_Base::rbuffer->size())

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
         U_DEBUG("UHTTP3::handlerNewConnection(): failed to create %S packet: %d", pkt, written)

         U_RETURN(false);
         }

      sent = U_SYSCALL(sendto, "%u,%p,%u,%u,%p,%d", UServer_Base::fds[0], out, written, 0, (struct sockaddr*)&USocket::peer_addr, USocket::peer_addr_len);

      if (sent == written)
         {
         U_DEBUG("quiche: sent %u bytes", sent)
         }
      else
         {
         U_DEBUG("UHTTP3::handlerNewConnection(): failed to send")

         U_RETURN(false);
         }

      if (handlerRead()) goto loop1;

      U_RETURN(false);
      }

   if (token_len == 0)
      {
      // Generate a stateless retry token. The token includes the static string "quiche" followed
      // by the IP address of the client and by the original destination connection ID generated by the client

      U_DEBUG("quiche: stateless retry")

      U_INTERNAL_DUMP("peer_addr_len = %u", peer_addr_len)

      U_INTERNAL_ASSERT_MAJOR(conn_id_len, 0)
      U_INTERNAL_ASSERT_MAJOR(peer_addr_len, 0)

      // 6 -> U_CONSTANT_SIZE("quiche")
      U_MEMCPY(token, "quiche", 6);
      U_MEMCPY(token + 6, &peer_addr, peer_addr_len);
      U_MEMCPY(token + 6 + peer_addr_len, conn_id, conn_id_len);

      token_len = 6 + peer_addr_len + conn_id_len;

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
      U_DEBUG("UHTTP3::handlerNewConnection(): invalid address validation token")

      U_RETURN(false);
      }

   token_len -= 6;

   ptr = (const char*)&token[0] + 6;

   if (token_len < peer_addr_len ||
       memcmp(ptr, &peer_addr, peer_addr_len))
      {
      U_DEBUG("UHTTP3::handlerNewConnection(): invalid address validation token")

      U_RETURN(false);
      }

   ptr       += peer_addr_len;
   token_len -= peer_addr_len;

   if (conn_id_len != token_len) // The token was not valid, meaning the retry failed, so drop the packet
      {
      U_DEBUG("UHTTP3::handlerNewConnection(): invalid address validation token")

      U_RETURN(false);
      }

   // Reuse the source connection ID we sent in the Retry packet, instead of changing it again. Creates a new server-side connection
   conn.conn = (quiche_conn*) U_SYSCALL(quiche_accept, "%p,%u,%p,%u,%p", conn_id, conn_id_len, (const uint8_t*)ptr, token_len, qconfig);

   if (conn.conn == U_NULLPTR)
      {
      U_DEBUG("UHTTP3::handlerNewConnection(): failed to create connection")

      U_RETURN(false);
      }

   U_DEBUG("quiche: new connection")

loop2:
   U_INTERNAL_ASSERT_POINTER(conn.conn)

   // Processes QUIC packets received from the peer
   done = U_SYSCALL(quiche_conn_recv, "%p,%p,%u", conn.conn, (uint8_t*)U_STRING_TO_PARAM(*UServer_Base::rbuffer));

   if (done < 0)
      {
      U_DEBUG("UHTTP3::handlerNewConnection(): failed to process packet: %d", done)

      U_RETURN(false);
      }

   U_DEBUG("quiche: recv %d bytes", done)

   if (U_SYSCALL(quiche_conn_is_established, "%p", conn.conn))
      {
      // the connection handshake is complete

      if (conn.http3 == U_NULLPTR)
         {
         // Creates a new HTTP/3 connection using the provided QUIC connection
         conn.http3 = (quiche_h3_conn*) U_SYSCALL(quiche_h3_conn_new_with_transport, "%p,%p", conn.conn, http3_config);

         if (conn.http3 == U_NULLPTR)
            {
            U_DEBUG("UHTTP3::handlerNewConnection(): failed to create HTTP/3 connection")

            U_RETURN(false);
            }
         }

      while (true)
         {
         // Processes HTTP/3 data received from the peer
         s = U_SYSCALL(quiche_h3_conn_poll, "%p,%p,%p", conn.http3, conn.conn, &ev);

         if (s < 0)
            {
            bwait_for_data = true;

            break;
            }

         switch (quiche_h3_event_type(ev))
            {
            case QUICHE_H3_EVENT_HEADERS:
               {
               // Iterates over the headers in the event. The `cb` callback will be called for each header in `ev`. `cb` should check the validity of
               // pseudo-headers and headers. If `cb` returns any value other than `0`, processing will be interrupted and the value is returned to the caller
               rc = U_SYSCALL(quiche_h3_event_for_each_header, "%p,%p,%p", ev, for_each_header, 0);

               if (rc != 0)
                  {
                  U_DEBUG("UHTTP3::handlerNewConnection(): failed to process headers: %d", rc)
                  }

               // Sends an HTTP/3 response on the specified stream
               (void) U_SYSCALL(quiche_h3_send_response, "%p,%p,%lu,%p,%p,%b", conn.http3, conn.conn, s, headers, headers_len, false);

               // Sends an HTTP/3 body chunk on the given stream.
               (void) U_SYSCALL(quiche_h3_send_body, "%p,%p,%lu,%p,%p", conn.http3, conn.conn, s, (uint8_t*)U_STRING_TO_PARAM(*UHTTP::body), true);

               break;
               }

            case QUICHE_H3_EVENT_DATA:
               {
               U_DEBUG("quiche: got HTTP data")

               break;
               }

            case QUICHE_H3_EVENT_FINISHED:
            break;
            }

         U_SYSCALL_VOID(quiche_h3_event_free, "%p", ev);
         }
      }

   if (handlerRead()) goto loop2;

   U_RETURN(false);

   /*
   {
   peers->insert((const char*)conn_id, conn_id_len, (const UClientImage_Base*)&conn);

   if (U_SYSCALL(quiche_conn_is_in_early_data, "%p", conn.conn) || // the connection has a pending handshake that has progressed
   {
   U_DEBUG("UHTTP3::handlerRead(): connection handshake is complete")

   U_INTERNAL_ASSERT_EQUALS(conn.http3, U_NULLPTR)
   }
   }
   */
}
