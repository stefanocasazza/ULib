#!/bin/sh
#
# lg's caller-catcher centos rc script
#
# last modified: 2010/01/26
#
# WARNING:
#
#       heartbeat needs rc scripts, aka resource agents, to be LSB compatible:
#
#               *** start/stop actions ALWAYS exit 0 ***
#
#               implement status action and exit accordingly
#                       0 = running
#                       1 = dead (... but pid file exists)
#                       2 = dead (... but lock file exists)
#                       3 = not running/stopped/unused
#

# description: Start caller_catcher
# chkconfig: 2345 96 04

descr='caller catcher'
exepath=/usr/sbin/caller_catcherd
exeargs='--config-file /etc/caller-catcherd.conf'

pidfile=/var/run/caller_catcherd.pid

logfile=/tmp/caller_catcher.log
#outfile=/tmp/caller_catcher.out
#errfile=/tmp/caller_catcher.err


test -f /etc/ha.d/shellfuncs && {
        . /etc/ha.d/shellfuncs
        alias log="ha_log $descr: " ; } ||
        alias log="echo $descr: "

. /etc/init.d/functions

case "$1" in

    start)
        $0 status && exit 0

        log 'starting'

        daemon ${pidfile:+--pidfile=$pidfile} "$exepath $exeargs \
                >> ${outfile:-${logfile:-/dev/null}} \
                2>> ${errfile:-${logfile:-/dev/null}}"

        echo ;;

    stop)
        log 'stopping'

        killproc ${pidfile:+-p $pidfile} $exepath

        echo ;;

    status)
        log 'checking'

        status ${pidfile:+-p $pidfile} $exepath

        exit $? ;;

    restart)
        $0 stop ; $0 start

        ;;

    *)
        echo "usage: $0 (start|stop|status|restart)"

        exit 1 ;;
esac

exit 0
