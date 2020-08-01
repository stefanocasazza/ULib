#!/bin/sh

. ../.function

DOC_ROOT=benchmark/docroot

rm -f tmp/usp_compile.sh.err /tmp/*.hpack.* \
      /tmp/*userver_udp* \
      out/userver_*.out err/userver_*.err \
                trace.*userver_*.[0-9]*           object.*userver_*.[0-9]*           stack.*userver_*.[0-9]*           mempool.*userver_*.[0-9]* \
           /tmp/trace.*userver_*.[0-9]*      /tmp/object.*userver_*.[0-9]*      /tmp/stack.*userver_*.[0-9]*      /tmp/mempool.*userver_*.[0-9]* \
      $DOC_ROOT/trace.*userver_*.[0-9]* $DOC_ROOT/object.*userver_*.[0-9]* $DOC_ROOT/stack.*userver_*.[0-9]* $DOC_ROOT/mempool.*userver_*.[0-9]*

 UTRACE="0 20M 0"
 UTRACE_SIGNAL="0 50M 0"
 UTRACE_FOLDER=/tmp
#UOBJDUMP="0 10M 100"
#USIMERR="error.sim"
 UMEMUSAGE=yes
export UTRACE UOBJDUMP USIMERR UTRACE_SIGNAL UMEMUSAGE UTRACE_FOLDER

cat <<EOF >inp/webserver.cfg
userver {
 PORT 4433
 RUN_AS_USER nobody
 LOG_FILE /tmp/userver_udp.log
 LOG_FILE_SZ 10M
 PID_FILE /var/run/userver_udp.pid
 DOCUMENT_ROOT $DOC_ROOT
 PLUGIN "socket http"
 PLUGIN_DIR ../../../../src/ulib/net/server/plugin/.libs
 CERT_FILE cert.crt
  KEY_FILE cert.key
 PREFORK_CHILD 0
}
http3 {
# QUICHE_GREASE                              0 # Whether to send GREASE
# QUICHE_LOG_KEYS                            0 # Enables logging of secrets
# QUICHE_VERIFY_PEER                         0 # Whether to verify the peer's certificate
  QUICHE_CC_ALGORITHM                        0 # Sets the congestion control algorithm used
# QUICHE_MAX_ACK_DELAY                       ? # Sets the max_ack_delay transport parameter
  QUICHE_MAX_PACKET_SIZE                  1350 # Sets the max_packet_size transport parameter
  QUICHE_MAX_UDP_PAYLOAD_SIZE             1350 # Sets the max_packet_size transport parameter
  QUICHE_MAX_IDLE_TIMEOUT                 5000 # Sets the max_idle_timeout transport parameter
  QUICHE_INITIAL_MAX_DATA             10000000 # Sets the initial_max_data transport parameter
# QUICHE_ENABLE_EARLY_DATA                 yes # Enables sending or receiving early data
# QUICHE_ACK_DELAY_EXPONENT                  ? # Sets the ack_delay_exponent transport parameter
  QUICHE_INITIAL_MAX_STREAM_UNI            100 # Sets the initial_max_streams_uni transport parameter
  QUICHE_DISABLE_ACTIVE_MIGRATION         true # Sets the disable_active_migration transport parameter
  QUICHE_INITIAL_MAX_STREAMS_BIDI          100 # Sets the initial_max_streams_bidi transport parameter
  QUICHE_INITIAL_MAX_STREAM_DATA_UNI   1000000 # Sets the initial_max_stream_data_uni transport parameter

  QUICHE_INITIAL_MAX_STREAM_DATA_BIDI_LOCAL	1000000 # Sets the initial_max_stream_data_bidi_local transport parameter
  QUICHE_INITIAL_MAX_STREAM_DATA_BIDI_REMOTE 1000000 # Sets the initial_max_stream_data_bidi_remote transport parameter
 
# QUICHE_H3_MAX_HEADER_LIST_SIZE        16384 # Sets the SETTINGS_MAX_HEADER_LIST_SIZE setting
# QUICHE_H3_QPACK_BLOCKED_STREAMS           ? # Sets the SETTINGS_QPACK_BLOCKED_STREAMS setting
# QUICHE_H3_QPACK_MAX_TABLE_CAPACITY        ? # Sets the SETTINGS_QPACK_BLOCKED_STREAMS setting
}
EOF

DIR_CMD="../../examples/userver"

compile_usp

#STRACE=$TRUSS
start_prg_background userver_udp -c inp/webserver.cfg

wait_server_ready localhost 4433

sync
echo "PID = `cat /var/run/userver_udp.pid`"
exit 0
$SLEEP
curl -vvvv --http3 https://localhost:4433
$SLEEP
killall userver_udp
$SLEEP
pkill userver_udp 2>/dev/null

#$SLEEP
#sync
#grep ' quiche:' err/userver_udp.err >/tmp/out.txt
#gvimdiff /tmp/out.txt /mnt/data/storage/ulib/ULib-2.4.2/doc/quiche-0.4.0/examples/out.txt 

#nc -u -w 5 192.168.42.12 4433 < /tmp/audacious-temp-*
