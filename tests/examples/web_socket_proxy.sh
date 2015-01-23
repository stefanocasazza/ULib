#!/bin/sh

. ../.function

rm -f out/userver_tcp.out err/userver_tcp.err \
		web_socket_proxy.log web_server_echo.log tmp/UWebSocketPlugIn.err \
		trace.*userver_tcp*.[0-9]*	object.*userver_tcp*.[0-9]* stack.*userver_tcp*.[0-9]*

 UTRACE="0 10M 0"
#UOBJDUMP="0 1M 10"
#USIMERR="error.sim"
 export UTRACE UOBJDUMP USIMERR

DIR_CMD="../../examples/userver"

#STRACE=$TRUSS
start_prg_background userver_tcp -c web_server_echo.cfg
start_prg_background userver_tcp -c web_socket_proxy.cfg

#$SLEEP
#kill_prg userver_tcp TERM

mv err/userver_tcp.err err/web_socket_proxy.err
