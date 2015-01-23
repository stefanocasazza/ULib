#!/bin/sh

. ../.function

start_msg IR_WEB

DOC_ROOT=IR/WEB

mkdir -p ../db
rm -f IR_WEB.log* tmp/usp_compile.sh.err \
		../db/session.ssl* /tmp/ssl_session.txt /tmp/byterange* /tmp/*.memusage.* \
		web_server*.log* uploads/* /var/log/httpd/access_log out/userver_tcp.out err/userver_tcp.err web_server.err \
					 trace.*userver_*.[0-9]*			  object.*userver_*.[0-9]*				 stack.*userver_*.[0-9]*			  mempool.*userver_*.[0-9]* \
		$DOC_ROOT/trace.*userver_*.[0-9]* $DOC_ROOT/object.*userver_*.[0-9]* $DOC_ROOT/stack.*userver_*.[0-9]* $DOC_ROOT/mempool.*userver_*.[0-9]*

 UTRACE="0 50M 0"
#UOBJDUMP="0 100k 10"
#USIMERR="error.sim"
 export UTRACE UOBJDUMP USIMERR

DIR_CMD="../../examples/userver"

compile_usp

if [ "$TERM" != "cygwin" ]; then
   ( cd ../../examples/IR; make ir_web.la || exit 1; ) >& /dev/null
   ( cd $DOC_ROOT; ln -sf ../doc; mkdir -p servlet; cd servlet; rm -f *.so; ln -sf ../../../../../examples/IR/.libs/ir_web.so; )
fi

#STRACE=$TRUSS
start_prg_background userver_tcp -c IR_WEB.cfg

#$SLEEP
#kill_prg userver_tcp TERM

mv err/userver_tcp.err err/IR_WEB.err

echo "PID = `cat /var/run/userver_tcp.pid`"
