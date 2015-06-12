#!/bin/sh

. ../.function

DOC_ROOT=docroot

rm -f db/session.ssl* /tmp/ssl_session.txt /tmp/byterange* /tmp/*.memusage.* \
      $DOC_ROOT/webserver_ssl*.log* $DOC_ROOT/uploads/* /var/log/httpd/access_log \
      out/userver_ssl.out err/userver_ssl.err web_server_ssl.err \
                trace.*userver_ssl*.[0-9]*           object.*userver_ssl*.[0-9]*           stack.*userver_ssl*.[0-9]* mempool.*userver_ssl*.[0-9]* \
      $DOC_ROOT/trace.*userver_ssl*.[0-9]* $DOC_ROOT/object.*userver_ssl*.[0-9]* $DOC_ROOT/stack.*userver_ssl*.[0-9]* $DOC_ROOT/mempool.*userver_ssl*.[0-9]*

 UTRACE="0 50M 0"
#UOBJDUMP="0 100k 10"
#USIMERR="error.sim"
 export UTRACE UOBJDUMP USIMERR

cat <<EOF >inp/webserver.cfg
userver {
 PORT 443
 RUN_AS_USER apache
#MIN_SIZE_FOR_SENDFILE 2k
 LOG_FILE webserver_ssl.log
 LOG_FILE_SZ 1M
#LOG_FILE_SZ 20k
 LOG_MSG_SIZE -1
 PID_FILE /var/run/userver_tcp.pid
 PREFORK_CHILD 2
#REQ_TIMEOUT 300
#PLUGIN "ssi http"
#ORM_DRIVER "sqlite mysql"
 DOCUMENT_ROOT  docroot
 PLUGIN_DIR     ../../../src/ulib/net/server/plugin/.libs
 ORM_DRIVER_DIR ../../../src/ulib/orm/driver/.libs
 CERT_FILE ../../ulib/CA/server.crt
  KEY_FILE ../../ulib/CA/server.key
 PASSWORD caciucco
 CA_PATH ../../ulib/CA/CApath
 CA_FILE ../../ulib/CA/cacert.pem
 VERIFY_MODE 0
}
http {
#ALIAS [ / /index.php ]
#VIRTUAL_HOST yes
#ENABLE_INOTIFY yes
 LIMIT_REQUEST_BODY 1M 
 REQUEST_READ_TIMEOUT 30
#DIGEST_AUTHENTICATION yes
#CACHE_FILE_STORE nocat/webif.gz
#CACHE_FILE_MASK *.jpg|*.png|*.css|*.js|*.gif|inp/http/data/file1|*.*html|*.flv|*.svgz
}
EOF

DIR_CMD="../../examples/userver"

#STRACE=$TRUSS
start_prg_background userver_ssl -c inp/webserver.cfg 

#$SLEEP
#kill_prg userver_ssl TERM

mv err/userver_ssl.err err/web_server_ssl.err

echo "PID = `cat /var/run/userver_tcp.pid`"

#openssl s_client -debug -cert ../ulib/CA/username.crt -key ../ulib/CA/username.key -pass pass:caciucco -CApath ../ulib/CA/CApath -verify 0 -tls1_2 -cipher 'ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA:AES256-SHA:AES:CAMELLIA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA' -nextprotoneg h2-14 -no_ssl3 -no_ssl2 -connect localhost:443
