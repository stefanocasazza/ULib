#!/bin/sh

. ../.function

#DOC_ROOT=/srv/wifi-portal-siena/www
#DOC_ROOT=/srv/wifi-portal-firenze/www

rm -f nocat.log uclient.log /tmp/firewall.err /tmp/nodog.log /tmp/nocat.err /tmp/nodog.pid \
		out/uclient.out err/uclient.err out/userver_tcp.out err/userver_tcp.err \
		trace.*uclient*.[0-9]* object.*uclient*.[0-9]* stack.*uclient*.[0-9]* mempool*uclient*.[0-9]* \
		trace.*userver_tcp*.[0-9]* object.*userver_tcp*.[0-9]* stack.*userver_tcp*.[0-9]* mempool*userver_tcp*.[0-9]* \
 		nocat/trace.*userver_tcp*.[0-9]* nocat/object.*userver_tcp*.[0-9]* nocat/stack.*userver_tcp*.[0-9]* nocat/mempool*userver_tcp*.[0-9]* \
 		/usr/lib/nodog*/trace.*userver_tcp*.[0-9]* /usr/lib/nodog*/object.*userver_tcp*.[0-9]* /usr/lib/nodog*/stack.*userver_tcp*.[0-9]* /usr/lib/nodog*/mempool*userver_tcp*.[0-9]*
#		$DOC_ROOT/trace.*userver_tcp*.[0-9]* $DOC_ROOT/object.*userver_tcp*.[0-9]* $DOC_ROOT/stack.*userver_tcp*.[0-9]*

 UTRACE="0 100M 0"
#UOBJDUMP="0 5M 10"
#USIMERR="error.sim"
 export UTRACE UOBJDUMP USIMERR

DIR_CMD="../../examples/uclient"

QUERY="start_ap?ap=stefano\&public=10.30.1.131:5280"

#STRACE=$TRUSS
#start_prg uclient -i -c uclient.cfg "http://www.auth-firenze.com/$QUERY"

DIR_CMD="../../examples/userver"

#STRACE=$TRUSS
export DIR_LOG_GZ=/tmp
 start_prg_background userver_tcp -c nocat.cfg
#start_prg_background userver_tcp -c nodog.conf.portal
#start_prg_background userver_tcp -c nocat/etc/nodog.conf

#$SLEEP
#kill_prg userver_tcp TERM

mv err/userver_tcp.err err/nocat.err
