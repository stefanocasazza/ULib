#!/bin/sh

. ../.function

rm -f JONATHAN/log/* /tmp/trace.*userver_tcp*.[0-9]* /tmp/object.*userver_tcp*.[0-9]* /tmp/stack.*userver_tcp*.[0-9]*

#UTRACE="0 50M 0"
 UTRACE_SIGNAL="0 5M -1"
#UOBJDUMP="0 1M 300"
 UTRACE_FOLDER="/tmp"
#USIMERR="error.sim"
 export UTRACE UTRACE_SIGNAL UOBJDUMP USIMERR UTRACE_FOLDER

#export UMEMPOOL="581,0,0,59,16409,-7,-20,-23,31"

export ORM_DRIVER="sqlite"
export ORM_OPTION="host=localhost user=dbuser password=dbpass character-set=utf8 dbname=../db/concise-ile"

DIR_CMD="../../examples/userver"

compile_usp

#STRACE=$TRUSS
start_prg_background userver_tcp -c JONATHAN/etc/userver.cfg

wait_server_ready localhost 8080

sync
echo "PID = `cat JONATHAN/log/userver.pid`"

mv err/userver_tcp.err err/jon.err
