#!/bin/sh
#
# lg's wifi-portal crontab rc script
#
# last modified: 2010/01/26
#
# WARNING:
#
#	heartbeat needs rc scripts, aka resource agents, to be LSB compatible:
#
#		*** start/stop actions ALWAYS exit 0 ***
#
#		implement status action and exit accordingly
#			0 = running
#			1 = dead (... but pid file exists)
#			2 = dead (... but lock file exists)
#			3 = not running/stopped/unused
#

# description: Start wifi-portal firenze crontab
# chkconfig: 2345 95 05

descr='wifi-portal firenze crontab'

jobs='
	*/5 * * * * /srv/wifi-portal-firenze/bin/get_users_info.sh
	00 00 * * * /srv/wifi-portal-firenze/bin/reset_policy.sh
'


test -f /etc/ha.d/shellfuncs && {
	. /etc/ha.d/shellfuncs
	alias log="ha_log $descr: " ; } ||
	alias log="echo $descr: "

. /etc/init.d/functions

case "$1" in

    start)
	$0 status && exit 0

	log 'starting'

	echo "$jobs" | while read job ; do
		case $job in ''|\#*) continue ;; esac

		{ crontab -l | grep -v '^#' |
			grep -v "`echo "$job" | sed 's/\*/\\\*/g'`"
			echo "$job" ; } |
				crontab -
	done

	success ; echo ;;

    stop)
	log 'stopping'

	echo "$jobs" | while read job ; do
		case $job in ''|\#*) continue ;; esac

		crontab -l | grep -v '^#' |
			grep -v "`echo "$job" | sed 's/\*/\\\*/g'`" |
				crontab -
	done

	success ; echo ;;

    status)
	log 'checking'

	echo "$jobs" | { while read job ; do
		case $job in ''|\#*) continue ;; esac

		crontab -l | grep -v '^#' |
			grep -q "`echo "$job" | sed 's/\*/\\\*/g'`" &&
				running=true ||
				stopped=true
	done

	test $running && {
		test $stopped && {
			log '..unknown' ; exit 4 ; } || {
			log '..running' ; exit 0 ; } ; } || {
		log '..stopped' ; exit 3 ; }
	}

	exit $? ;;

    *)
	echo "usage: $0 (start|stop|status)"

	exit 1 ;;
esac

exit 0
