#!/bin/sh

# userver centos rc script
#
# chkconfig: - 97 13
# description: Start userver the fabulous
# processname: userver
# config: /srv/wi-auth/userver/etc
# pidfile: /var/run/slapd.pid

#UTRACE="0 50M 1"
#UOBJDUMP="0 5M 100"
#USIMERR="error.accept"
#UTRACE_LEVEL=5
 export UTRACE UOBJDUMP USIMERR UTRACE_LEVEL

prog="userver"

exepath1=/home/unirel/userver/bin/userver_tcp
exeargs1='-c /home/unirel/userver/etc/firenze_tcp.cfg'
pidfile1=/var/run/userver-firenze_tcp.pid
outfile1=/tmp/userver-firenze_tcp.out
errfile1=/tmp/userver-firenze_tcp.err

exepath2=/home/unirel/userver/bin/userver_ssl
exeargs2='-c /home/unirel/userver/etc/firenze_ssl.cfg'
pidfile2=/var/run/userver-firenze_ssl.pid
outfile2=/tmp/userver-firenze_ssl.out
errfile2=/tmp/userver-firenze_ssl.err

export DIR_LOG_GZ=/var/log
export UMEMPOOL=11100,1369,-26,2131,2509,241,49,-9,94:268435456,274432

log_files=`ls /var/log/userver-*.log /var/log/uclient-*.log /var/log/wifi-portal 2>/dev/null` # /var/log/wifi-warning

# Source function library.
. /etc/rc.d/init.d/functions

case "$1" in
   start)
		$0 status && exit 0

		echo $"Starting $prog: "

		rm -f /tmp/*.err /tmp/[0-9][0-9]*.out /tmp/card_activation* \
		      /tmp/get_users_info* /tmp/send_req_to_portal* \
		      /tmp/nodog_check* /tmp/processCGIRequest.err

		for i in $log_files
		do
		   echo -n >$i
		done

 		daemon "$exepath1 $exeargs1 >$outfile1 2>$errfile1 &"
 		sleep 2
 		daemon "$exepath2 $exeargs2 >$outfile2 2>$errfile2 &"

		echo ;;

	stop)

		echo $"Stopping $prog: "

		if [ -f $pidfile2 ] ; then
			kill `cat $pidfile2`
		else
			killproc $exepath2
		fi

		sleep 2

		if [ -f $pidfile1 ] ; then
			kill `cat $pidfile1`
		else
			killproc $exepath1
		fi

		rm -f $pidfile1 $pidfile2

 		sleep 1

		pkill -9 `basename $exepath1` `basename $exepath2`

		echo ;;

	status)
		echo $"Checking $prog: "

		status -p $pidfile1 $exepath1
		status -p $pidfile2 $exepath2

		exit $? ;;

	restart)
		$0 stop ; sleep 2 ; $0 start
		;;

	*)
		echo "usage: $0 (start|stop|status|restart)"

		exit 1 ;;
esac

exit 0
