#!/bin/sh

. ../.function

#DOC_ROOT=ruby/blog
 DOC_ROOT=benchmark/docroot

rm -f tmp/usp_compile.sh.err /tmp/*.hpack.* \
		$DOC_ROOT/web_server.log* \
      out/userver_*.out err/userver_*.err \
					 trace.*userver_*.[0-9]*			  object.*userver_*.[0-9]*				 stack.*userver_*.[0-9]*			  mempool.*userver_*.[0-9]* \
      $DOC_ROOT/trace.*userver_*.[0-9]* $DOC_ROOT/object.*userver_*.[0-9]* $DOC_ROOT/stack.*userver_*.[0-9]* $DOC_ROOT/mempool.*userver_*.[0-9]*

 UTRACE="0 50M 0"
#UTRACE_SIGNAL="0 50M -1"
#UOBJDUMP="0 10M 100"
#USIMERR="error.sim"
export UTRACE UOBJDUMP USIMERR UTRACE_SIGNAL

SOCK1=tmp/fcgi.socket

start_test() {

	CMD=test_fcgi

	PIDS=`ps x | grep $CMD | grep -v grep | awk '{ print $1 }'`

	if [ -z "$PIDS" ]; then
#		rm -f	$SOCK1
		../../src/ulib/net/server/plugin/fcgi/$CMD $SOCK1 2>/tmp/$CMD.err &
		chmod 777 $SOCK1
	fi
}

#start_test
#/usr/bin/spawn-fcgi -p 8080 -f /usr/bin/php-cgi -C 5 -P /var/run/spawn-fcgi.pid

cat <<EOF >inp/webserver.cfg
userver {
 PORT 8080
 RUN_AS_USER nobody
#MIN_SIZE_FOR_SENDFILE 2k
 LOG_FILE web_server.log
 LOG_FILE_SZ 1M
#LOG_FILE_SZ 20k
 LOG_MSG_SIZE -1
 PID_FILE /var/run/userver_tcp.pid
 PREFORK_CHILD 2
#REQ_TIMEOUT 300
#PLUGIN "ssi http"
#ORM_DRIVER "sqlite mysql"
 DOCUMENT_ROOT  benchmark/docroot
 PLUGIN_DIR     ../../../../src/ulib/net/server/plugin/.libs
 ORM_DRIVER_DIR ../../../../src/ulib/orm/driver/.libs
#DOCUMENT_ROOT  .
#PLUGIN_DIR     ../../src/ulib/net/server/plugin/.libs
#ORM_DRIVER_DIR ../../src/ulib/orm/driver/.libs
#DOCUMENT_ROOT  php
#PLUGIN_DIR     ../../../src/ulib/net/server/plugin/.libs
#ORM_DRIVER_DIR ../../../src/ulib/orm/driver/.libs
#DOCUMENT_ROOT  ruby/blog/public
#PLUGIN_DIR     ../../../../../src/ulib/net/server/plugin/.libs
#ORM_DRIVER_DIR ../../../../../src/ulib/orm/driver/.libs
}
http {
#ALIAS "[ / /index.php ]"
#VIRTUAL_HOST yes
#ENABLE_INOTIFY yes
 LIMIT_REQUEST_BODY 3M
 REQUEST_READ_TIMEOUT 30
#DIGEST_AUTHENTICATION yes
#CACHE_FILE_STORE nocat/webif.gz
#CACHE_FILE_MASK *.jpg|*.png|*.css|*.js|*.gif|inp/http/data/file1|*.html|*.flv|*.svgz
}
EOF

export ORM_DRIVER="sqlite"
export ELASTICSEARCH_HOST="localhost"
export UMEMPOOL="136,0,60,100,250,-22,-17,-23,60"
export ORM_OPTION="host=localhost dbname=../db/hello_world"

DIR_CMD="../../examples/userver"

compile_usp

#STRACE=$TRUSS
start_prg_background userver_tcp -c inp/webserver.cfg
												# RA/RA.cfg
												# deployment.properties

# HTTP pseudo-streaming for FLV video

#curl -I -s -D -			'http://10.30.1.131/test.flv'					 -o /dev/null
#curl -I -s -D -			'http://10.30.1.131/test.flv'					 -o /tmp/test.flv
#curl    -s -v -r0-499	'http://10.30.1.131/test.flv'					 -o /tmp/test.flv
#curl    -s -D				'http://10.30.1.131/test.flv?start=669000' -o /tmp/test.flv

#sleep 6
#kill_prg userver_tcp TERM

mv err/userver_tcp.err err/web_server.err

echo "PID = `cat /var/run/userver_tcp.pid`"

# check_for_netcat
# $NCAT 10.30.1.131 80 <inp/http/get_geoip.req >>out/web_server.out

#openssl s_client -debug -cert ../ulib/CA/username.crt -key ../ulib/CA/username.key -pass pass:caciucco -CApath ../ulib/CA/CApath -verify 0 -connect 10.30.1.131:80
