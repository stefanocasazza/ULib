#!/bin/sh

# Running PHP scripts in ULib

ARG=$1
if [ -z "$ARG" ]; then
	ARG=1
fi

mkdir -p log out err www/cgi-bin

rm -f log/php${ARG}.log \
		out/*.out err/*.err \
		/tmp/processCGIRequest.err /tmp/server_plugin_fcgi.err \
			 trace.*.[0-9]*	  object.*.[0-9]*		 stack.*.[0-9]*	  mempool.*.[0-9]* \
      www/trace.*.[0-9]* www/object.*.[0-9]* www/stack.*.[0-9]* www/mempool.*.[0-9]*

if [ ! -s "www/index.php" ]; then
	echo -n "<?php phpinfo(); ?>" > www/index.php
fi
if [ ! -s "www/cgi-bin/index.php" ]; then
	echo -n "<?php phpinfo(); ?>" > www/cgi-bin/index.php
fi

if [ "$ARG" = "1" ]; then
#1) run PHP scripts using ULib CGI support
	cat <<EOF >userver.cfg
userver {
 PORT 8080
 LOG_FILE_SZ 1M
 LOG_MSG_SIZE -1
 DOCUMENT_ROOT www
 LOG_FILE ../log/php1.log
}
EOF
#2) run PHP requests proxed by FastCGI protocol 
elif [ "$ARG" = "2a" ]; then
#a) TCP socket (IP and port) approach
	cat <<EOF >userver.cfg
userver {
 PORT 8080
 LOG_FILE_SZ 1M
 LOG_MSG_SIZE -1
 DOCUMENT_ROOT www
 LOG_FILE ../log/php2a.log
 PLUGIN "fcgi http"
}
fcgi {
 FCGI_URI_MASK *.php
 SERVER 127.0.0.1
 PORT 9000
 LOG_FILE ../log/php2a.log
}
EOF
elif [ "$ARG" = "2b" ]; then
#b) unix domain socket (UDS) approach
	cat <<EOF >userver.cfg
userver {
 PORT 8080
 LOG_FILE_SZ 1M
 LOG_MSG_SIZE -1
 DOCUMENT_ROOT www
 LOG_FILE ../log/php2b.log
 PLUGIN "fcgi http"
}
fcgi {
 FCGI_URI_MASK *.php
 SOCKET_NAME /tmp/fcgi.socket
 LOG_FILE ../log/php2b.log
}
EOF
elif [ "$ARG" = "3" ]; then
#3) run PHP scripts using ULib php embedded support (--with-php-embedded).
	cat <<EOF >userver.cfg
userver {
 PORT 8080
 LOG_FILE_SZ 1M
 LOG_MSG_SIZE -1
 DOCUMENT_ROOT www
 LOG_FILE ../log/php3.log
}
EOF
fi

#UTRACE="0 50M 0"
#UTRACE_SIGNAL="0 50M 0"
#UOBJDUMP="0 10M 100"
#USIMERR="error.sim"
export UTRACE UTRACE_SIGNAL UOBJDUMP USIMERR

#STRACE="strace -t -f -s 100"
PID=`( eval "$STRACE userver_tcp -c userver.cfg >>out/php${ARG}.out 2>>err/php${ARG}.err &"; echo $! )`
echo "PID = "$PID
