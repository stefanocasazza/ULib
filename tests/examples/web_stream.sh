#!/bin/sh

. ../.function

rm -f web_stream.log \
      out/userver_tcp.out err/userver_tcp.err \
		trace.*userver_tcp*.[0-9]*	object.*userver_tcp*.[0-9]* stack.*userver_tcp*.[0-9]*

#UTRACE="0 10M 0"
#UOBJDUMP="0 100k 10"
#USIMERR="error.sim"
 export UTRACE UOBJDUMP USIMERR

DIR_CMD="../../examples/userver"

#STRACE=$TRUSS
start_prg_background userver_tcp -c web_stream.cfg

#$SLEEP
#kill_prg userver_tcp TERM

mv err/userver_tcp.err err/web_stream.err
