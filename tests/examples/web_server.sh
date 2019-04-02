#!/bin/sh

. ../.function

(cd benchmark; rm -f  db; creat_link FrameworkBenchmarks/ULib/db db)

#DOC_ROOT=SESSION
#DOC_ROOT=docroot
#DOC_ROOT=sse_example
#DOC_ROOT=ruby/blog
 DOC_ROOT=JONATHAN/docroot
#DOC_ROOT=benchmark/docroot
#DOC_ROOT=ShivShankarDayal/docroot

rm -f tmp/usp_compile.sh.err /tmp/*.hpack.* \
      $DOC_ROOT/web_server.log* \
      out/userver_*.out err/userver_*.err \
                trace.*userver_*.[0-9]*           object.*userver_*.[0-9]*           stack.*userver_*.[0-9]*           mempool.*userver_*.[0-9]* \
      $DOC_ROOT/trace.*userver_*.[0-9]* $DOC_ROOT/object.*userver_*.[0-9]* $DOC_ROOT/stack.*userver_*.[0-9]* $DOC_ROOT/mempool.*userver_*.[0-9]*

 UTRACE="0 50M 0"
#UTRACE_SIGNAL="0 50M -1"
 UTRACE_FOLDER=/tmp
 TMPDIR=/tmp
#UOBJDUMP="0 10M 100"
#USIMERR="error.sim"
#UMEMUSAGE=yes
export UTRACE UOBJDUMP USIMERR UTRACE_SIGNAL UMEMUSAGE UTRACE_FOLDER TMPDIR

SOCK1=tmp/fcgi.socket

start_test() {

   CMD=test_fcgi

   PIDS=`ps x | grep $CMD | grep -v grep | awk '{ print $1 }'`

   if [ -z "$PIDS" ]; then
#     rm -f $SOCK1
      ../../src/ulib/net/server/plugin/fcgi/$CMD $SOCK1 2>/tmp/$CMD.err &
      chmod 777 $SOCK1
   fi
}

#start_test
#/usr/bin/spawn-fcgi -p 8080 -f /usr/bin/php-cgi -C 5 -P /var/run/spawn-fcgi.pid

# =================================================================
# HTTP2
# =================================================================
# ./h2a -c server.crt -k server.key -p 8000 -H 127.0.0.1 -P 443
#
# Once h2a starts, you can access http://localhost:8000 from the
# HTTP client such as Firefox and you will be able to check the
# HTTP/2 traffic
#
# ./web_server.sh
#
# /opt/go/bin/h2a    -p 80 -H 127.0.0.1 -P 8080 -d -D >& h2a.out &
# /opt/go/bin/h2spec -p 80                            >& h2spec.out
# =================================================================

cat <<EOF >inp/webserver.cfg
userver {
 PORT 8080
 RUN_AS_USER nobody
 MIN_SIZE_FOR_SENDFILE 2k
 LOG_FILE web_server.log
 LOG_FILE_SZ 10M
#LOG_FILE_SZ 20k
 LOG_MSG_SIZE -1
 PID_FILE /var/run/userver_tcp.pid
#LOAD_BALANCE_CLUSTER 10.30.0.1,10.30.0.2
#LOAD_BALANCE_DEVICE_NETWORK enp0s20u1
#LOAD_BALANCE_LOADAVG_THRESHOLD 4.0
 PREFORK_CHILD 0
#PREFORK_CHILD 2
#CRASH_COUNT 1
#CRASH_EMAIL_NOTIFY mail.unirel.com:stefano.casazza@unirel.com
#DOS_SITE_COUNT 1
#DOS_WHITE_LIST 127.0.0.1
#DOS_LOGFILE /tmp/dos_blacklist.txt
#REQ_TIMEOUT 300
#PLUGIN "ssi http"
#ORM_DRIVER "sqlite mysql"
 ORM_DRIVER sqlite

#DOCUMENT_ROOT .
#PLUGIN_DIR            ../../src/ulib/net/server/plugin/.libs
#ORM_DRIVER_DIR        ../../src/ulib/orm/driver/.libs
#DOCUMENT_ROOT docroot
#PLUGIN_DIR        ../../../src/ulib/net/server/plugin/.libs
#ORM_DRIVER_DIR    ../../../src/ulib/orm/driver/.libs
#DOCUMENT_ROOT benchmark/docroot
 PLUGIN_DIR     ../../../../src/ulib/net/server/plugin/.libs
 ORM_DRIVER_DIR ../../../../src/ulib/orm/driver/.libs

 DOCUMENT_ROOT $DOC_ROOT
}
http {
#ALIAS "[ / /100.html ]"
#VIRTUAL_HOST yes
#ENABLE_INOTIFY yes
 LIMIT_REQUEST_BODY 3M
#REQUEST_READ_TIMEOUT 30
#APACHE_LIKE_LOG /var/log/httpd/access_log
#LOG_FILE_SZ 10M
#DIGEST_AUTHENTICATION yes
#URI_PROTECTED_MASK /tutor/*|/learner/*|/HOD/*
#CACHE_FILE_STORE nocat/webif.gz
#CACHE_FILE_MASK inp/http/data/file1|*.flv|*.svgz
#URI_REQUEST_STRICT_TRANSPORT_SECURITY_MASK *
}
EOF

export ORM_DRIVER="sqlite"
export ELASTICSEARCH_HOST="localhost"
export UMEMPOOL="750,0,123,251,305,53,-6,-26,52"
#export ORM_OPTION="host=localhost dbname=../db/fortune"
#export ORM_OPTION="host=localhost dbname=../db/hello_world"
 export ORM_OPTION="host=localhost user=dbuser password=dbpass character-set=utf8 dbname=../db/concise-ile"

DIR_CMD="../../examples/userver"

prepare_usp

#STRACE=$TRUSS
start_prg_background userver_tcp -c inp/webserver.cfg
                                  # /srv/userver_orm.cfg
                                  # RA/RA.cfg
                                  # deployment.properties

wait_server_ready localhost 8080

sync
echo "PID = `cat /var/run/userver_tcp.pid`"

# HTTP pseudo-streaming for FLV video

#curl -I -s -D -        'http://localhost:8080/test.flv'              -o /dev/null
#curl -I -s -D -        'http://localhost:8080/test.flv'              -o /tmp/test.flv
#curl    -s -v -r0-499  'http://localhost:8080/test.flv'              -o /tmp/test.flv
#curl    -s -D          'http://localhost:8080/test.flv?start=669000' -o /tmp/test.flv

#sleep 6
#kill_server userver_tcp

mv err/userver_tcp.err err/web_server.err

#check_for_netcat
#send_req localhost 8080 inp/http/get_geoip.req web_server 3
#openssl s_client -debug -cert ../ulib/CA/username.crt -key ../ulib/CA/username.key -pass pass:caciucco -CApath ../ulib/CA/CApath -verify 0 -connect localhost:8080
