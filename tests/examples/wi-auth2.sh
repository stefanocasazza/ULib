#!/bin/sh

. ../.function

# set -x

## wi-auth2.sh

DOC_ROOT=wi-auth2/www

rm -f out/userver_tcp.out err/wi-auth2.err err/uclient.err \
                trace.*userver_*.[0-9]*           object.*userver_*.[0-9]*           stack.*userver_*.[0-9]*           mempool.*userver_*.[0-9]* \
      $DOC_ROOT/trace.*userver_*.[0-9]* $DOC_ROOT/object.*userver_*.[0-9]* $DOC_ROOT/stack.*userver_*.[0-9]* $DOC_ROOT/mempool.*userver_*.[0-9]*

#UTRACE="0 100M -1"
 UTRACE_FOLDER=/tmp
 TMPDIR=/tmp
#UOBJDUMP="0 10M 100"
#USIMERR="error.sim"
#UMEMUSAGE=yes
export UTRACE UOBJDUMP USIMERR UTRACE_SIGNAL UMEMUSAGE UTRACE_FOLDER TMPDIR

export REDIS_HOST=localhost

cat <<EOF >inp/webserver.cfg
userver {
 DOCUMENT_ROOT $DOC_ROOT
 LOG_FILE /tmp/wi-auth2.log
#LOG_FILE_SZ 1M
 LOG_MSG_SIZE -1
 PID_FILE /tmp/wi-auth2.pid
 PLUGIN_DIR ../../../../src/ulib/net/server/plugin/.libs
 PREFORK_CHILD 2
 REQ_TIMEOUT 30
}
http {
   CACHE_AVOID_MASK ap|immagini
 NOCACHE_FILE_MASK banner.html

 URI_PROTECTED_MASK /get_config
 USP_AUTOMATIC_ALIASING servlet/wi_auth2

 LIMIT_REQUEST_BODY 10M
 REQUEST_READ_TIMEOUT 30
}
EOF

DIR_CMD="../../examples/userver"

#STRACE=$TRUSS
start_prg_background userver_tcp -c inp/webserver.cfg

wait_server_ready localhost 80

sync
echo "PID = `cat /tmp/wi-auth2.pid`"

#$SLEEP
#kill_prg userver_tcp TERM

mv err/userver_tcp.err err/wi-auth2.err
