#!/bin/sh
#
# lg's wifi-portal logrotate rc script
#
# last modified: 2010/04/22
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

# description: Start wifi-portal firenze logrotate
# chkconfig: 2345 95 05

descr='wifi-portal firenze logrotate'

conffile=/srv/wifi-portal-firenze/etc/wifi-portal-firenze.logrotate.conf


test -f /etc/ha.d/shellfuncs && {
	. /etc/ha.d/shellfuncs
	alias log="ha_log $descr: " ; } ||
	alias log="echo $descr: "

. /etc/init.d/functions

case "$1" in

    start)
	$0 status && exit 0

	log 'starting'

	ln -s $conffile /etc/logrotate.d/

	success ; echo ;;

    stop)
	log 'stopping'

	rm /etc/logrotate.d/`basename $conffile`

	success ; echo ;;

    status)
	log 'checking'

	test -h /etc/logrotate.d/`basename $conffile` && {
		log '..running' ; exit 0 ; } || {
		log '..stopped' ; exit 3 ; }

	exit $? ;;

    *)
	echo "usage: $0 (start|stop|status)"

	exit 1 ;;
esac

exit 0
