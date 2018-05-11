#!/bin/sh

. ../.function

rm -f /tmp/web_socket.log \
      /tmp/UWebSocketPlugIn.err \
		out/web_socket.out err/userver_tcp.err \
		/tmp/trace.*userver_tcp*.[0-9]* /tmp/object.*userver_tcp*.[0-9]* /tmp/stack.*userver_tcp*.[0-9]*

 UTRACE="0 50M 0"
 UTRACE_SIGNAL="0 50M 0"
 UTRACE_FOLDER=/tmp
 TMPDIR=/tmp
#UOBJDUMP="0 10M 100"
#USIMERR="error.sim"
#UMEMUSAGE=yes
export UTRACE UOBJDUMP USIMERR UTRACE_SIGNAL UMEMUSAGE UTRACE_FOLDER TMPDIR

cat <<EOF >inp/webserver.cfg
userver {
 PORT 8787
 PID_FILE /tmp/web_socket.pid
 LOG_FILE /tmp/web_socket.log
 LOG_FILE_SZ 1M
 LOG_MSG_SIZE -1
 PLUGIN "socket http"
 DOCUMENT_ROOT websocket
 PLUGIN_DIR     ../../../src/ulib/net/server/plugin/.libs
 ORM_DRIVER_DIR ../../../src/ulib/orm/driver/.libs
}
EOF

(cd websocket; ln -sf ../../../src/ulib/net/server/plugin/usp/.libs/modsocket.so)

DIR_CMD="../../examples/userver"

check_for_netcat

#STRACE=$TRUSS
start_prg_background userver_tcp -c inp/webserver.cfg

wait_server_ready localhost 8787
$SLEEP
send_req $NCAT localhost 8787 inp/http/websocket.req web_socket 3 kill
$SLEEP
kill_prg userver_tcp TERM

mv err/userver_tcp.err /tmp/web_socket.err

#echo "PID = `cat /tmp/web_socket.pid`"
