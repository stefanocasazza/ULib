#!/bin/sh

. ../.function

# set -x

## nodog.sh

DOC_ROOT=wi-auth2/www

rm -f /tmp/uclient.log \
		out/uclient.out out/userver_tcp.out err/nodog.err err/uclient.err \
                trace.*userver_*.[0-9]*           object.*userver_*.[0-9]*           stack.*userver_*.[0-9]*           mempool.*userver_*.[0-9]* \
      $DOC_ROOT/trace.*userver_*.[0-9]* $DOC_ROOT/object.*userver_*.[0-9]* $DOC_ROOT/stack.*userver_*.[0-9]* $DOC_ROOT/mempool.*userver_*.[0-9]*

 UTRACE="0 100M -1"
 UTRACE_FOLDER=/tmp
 TMPDIR=/tmp
#UOBJDUMP="0 10M 100"
#USIMERR="error.sim"
#UMEMUSAGE=yes
export UTRACE UOBJDUMP USIMERR UTRACE_SIGNAL UMEMUSAGE UTRACE_FOLDER TMPDIR

PORTALE=192.168.42.206

DIR_CMD="../../examples/uclient"

cat <<EOF >inp/webclient.cfg
uclient {
#SERVER 10.30.1.131
#PORT 80
 RES_TIMEOUT 300
 LOG_FILE /tmp/uclient.log
 FOLLOW_REDIRECTS yes
 USER getconfig
 PASSWORD_AUTH 1011121314 
}
EOF

cat <<EOF >inp/webserver.cfg
userver {
 IP_ADDRESS 192.168.42.136
 DOCUMENT_ROOT $DOC_ROOT
 LOG_FILE /tmp/nodog.log
#LOG_FILE_SZ 1M
 LOG_MSG_SIZE -1
 PID_FILE /tmp/nodog.pid
 PLUGIN "nodog http"
 PLUGIN_DIR ../../../../src/ulib/net/server/plugin/.libs
 PREFORK_CHILD 0 
 REQ_TIMEOUT 5
 CGI_TIMEOUT 60
 TCP_LINGER_SET -1
 LISTEN_BACKLOG 128
}
http {
 CACHE_FILE_MASK _off_
 LIMIT_REQUEST_BODY 700K
 REQUEST_READ_TIMEOUT 30
}
nodog {
 FW_CMD ../firewall/nodog.fw
 DECRYPT_KEY vivalatopa
 CHECK_EXPIRE_INTERVAL 60
 FW_ENV "MasqueradeDevice=eth0 'AuthServiceAddr=http://localhost' FullPrivateNetwork=192.168.0.0/12 LocalNetwork=192.168.0.0/16 InternalDevice=usb0 'ExternalDevice=eth0 tun0 tun2'"
 LOCAL_NETWORK_LABEL 1000
#DHCP_DATA_FILE /tmp/kea-leases.tdb
}
EOF

#STRACE=$TRUSS
#start_prg uclient -c inp/webclient.cfg -o /tmp/nodog.conf.portal 'http://localhost/get_config?ap=firenzewificonc-dev-r47188_x86_64\&key=192.168.44.55'

DIR_CMD="../../examples/userver"

ulimit -s unlimited

#STRACE=$TRUSS
#VALGRIND='valgrind --leak-check=yes --track-origins=yes'
start_prg_background userver_tcp -c inp/webserver.cfg

wait_server_ready localhost 5280

sync
echo "PID = `cat /tmp/nodog.pid`"

#netcat -w 10 192.168.42.129 5280 < /mnt/storage/srv/realtime.req 

#$SLEEP
#kill_prg userver_tcp TERM

mv err/userver_tcp.err err/nodog.err
