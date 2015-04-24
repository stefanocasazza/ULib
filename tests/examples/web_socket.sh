#!/bin/sh

. ../.function

rm -f web_socket.log \
		/tmp/UWebSocketPlugIn.err \
      out/userver_tcp.out err/userver_tcp.err \
		trace.*userver_tcp*.[0-9]*	object.*userver_tcp*.[0-9]* stack.*userver_tcp*.[0-9]*

 UTRACE="0 50M 0"
#UOBJDUMP="0 1M 10"
#USIMERR="error.sim"
 export UTRACE UOBJDUMP USIMERR

cat <<EOF >web_socket_sh.cfg
userver {
 PORT 8787
 MAX_KEEP_ALIVE 6
 RUN_AS_USER apache
 PID_FILE docroot/web_socket_sh.pid
 LOG_FILE web_socket.log
 LOG_FILE_SZ 1M
 LOG_MSG_SIZE -1
 PLUGIN "socket http"
 DOCUMENT_ROOT docroot
 PLUGIN_DIR     ../../../src/ulib/net/server/plugin/.libs
 ORM_DRIVER_DIR ../../../src/ulib/orm/driver/.libs
 PREFORK_CHILD 1
}
socket {
 COMMAND ../my_websocket.sh
}
EOF

DIR_CMD="../../examples/userver"

#STRACE=$TRUSS
start_prg_background userver_tcp -c web_socket_sh.cfg

#$SLEEP
#kill_prg userver_tcp TERM

mv err/userver_tcp.err err/web_socket.err

echo "PID = `cat docroot/web_socket_sh.pid`"
