#!/bin/sh

. ../.function

DOC_ROOT=benchmark/docroot

rm -f tmp/usp_compile.sh.err /tmp/*.hpack.* \
      /tmp/*userver_udp* \
      out/userver_*.out err/userver_*.err \
                trace.*userver_*.[0-9]*           object.*userver_*.[0-9]*           stack.*userver_*.[0-9]*           mempool.*userver_*.[0-9]* \
      $DOC_ROOT/trace.*userver_*.[0-9]* $DOC_ROOT/object.*userver_*.[0-9]* $DOC_ROOT/stack.*userver_*.[0-9]* $DOC_ROOT/mempool.*userver_*.[0-9]*

 UTRACE="0 50M 0"
 UTRACE_SIGNAL="0 50M 0"
 UTRACE_FOLDER=/tmp
#UOBJDUMP="0 10M 100"
#USIMERR="error.sim"
 UMEMUSAGE=yes
export UTRACE UOBJDUMP USIMERR UTRACE_SIGNAL UMEMUSAGE UTRACE_FOLDER

cat <<EOF >inp/webserver.cfg
userver {
 PORT 8080
 RUN_AS_USER nobody
 LOG_FILE /tmp/userver_udp.log
 LOG_FILE_SZ 10M
 PID_FILE /var/run/userver_udp.pid
 PREFORK_CHILD 0
}
EOF

DIR_CMD="../../examples/userver"

#STRACE=$TRUSS
start_prg_background userver_udp -c inp/webserver.cfg

$SLEEP
sync
echo "PID = `cat /var/run/userver_udp.pid`"

#$SLEEP
#$SLEEP
#killall userver_udp

#nc -u -w 5 192.168.42.12 8080 < /tmp/audacious-temp-*
