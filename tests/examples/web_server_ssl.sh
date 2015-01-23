#!/bin/sh

. ../.function

rm -f ../db/session.ssl* /tmp/ssl_session.txt \
      web_server_ssl?.log* uploads/* out/userver_ssl.out err/userver_ssl.err \
		trace.*userver_ssl*.[0-9]* object.*userver_ssl*.[0-9]* stack.*userver_ssl*.[0-9]*

#UTRACE="0 10M 0"
#UOBJDUMP="0 100k 10"
#USIMERR="error.sim"
 export UTRACE UOBJDUMP USIMERR

DIR_CMD="../../examples/userver"

#STRACE=$TRUSS
start_prg_background userver_ssl -c web_server_ssl2.cfg

#$SLEEP
#kill_prg userver_ssl TERM

mv err/userver_ssl.err err/web_server_ssl.err

#openssl s_client -debug -cert ../ulib/CA/username.crt -key ../ulib/CA/username.key -pass pass:caciucco -CApath ../ulib/CA/CApath -verify 0 -connect 10.30.1.131:443
