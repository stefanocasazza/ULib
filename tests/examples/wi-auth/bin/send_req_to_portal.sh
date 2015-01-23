#!/bin/bash

# send_req_to_portal.sh

# set -x 

# load var

HOME=/home/unirel

for i in $HOME/wi-auth/etc/*/script.conf
do
	. $i
done

CLIENT_HTTP=`grep CLIENT_HTTP $HOME/wi-auth/etc/environment.conf | cut -d'=' -f2`
PORTAL_NAME=`grep PORTAL_NAME $HOME/wi-auth/etc/environment.conf | cut -d'=' -f2`
LOCAL_SYSLOG_SELECTOR=`grep LOCAL_SYSLOG_SELECTOR $HOME/wi-auth/etc/environment.conf | cut -d'=' -f2`

do_cmd() {

	logger -p $LOCAL_SYSLOG_SELECTOR "$PORTAL_NAME: $1 BEGIN"

	TMPFILE=/tmp/$1.$$

	#UTRACE="0 10M 0"
	#UOBJDUMP="0 100k 10"
	#USIMERR="error.sim"
	 export UTRACE UOBJDUMP USIMERR

 	eval $CLIENT_HTTP "http://wifi-aaa.comune.fi.it/$2" >$TMPFILE 2>/dev/null
#	eval $CLIENT_HTTP "http://auth.t-unwired.com/$2" >$TMPFILE 2>/dev/null

#	if [ $? -eq 0 ]; then
 		rm -f $TMPFILE
#	fi

	logger -p $LOCAL_SYSLOG_SELECTOR "$PORTAL_NAME: $1 END"

	if [ -n "$RSYNC_HOST" ]; then

		logger -p $LOCAL_SYSLOG_SELECTOR "$PORTAL_NAME: $1 START SYNCHRONIZING PORTAL STATUS INFO"

		/home/unirel/userver/bin/synchronize-status.sh &&
			{ logger -p $LOCAL_SYSLOG_SELECTOR "$PORTAL_NAME: $1 END SYNCHRONIZING PORTAL STATUS INFO: SUCCESS" ; true ; } ||
			  logger -p $LOCAL_SYSLOG_SELECTOR "$PORTAL_NAME: $1 END SYNCHRONIZING PORTAL STATUS INFO: FAILURE"
	fi
}

# TMPFILE=`basename $0`
# TMPFILE_OUT=/tmp/$1_$$.out
# TMPFILE_ERR=/tmp/$1_$$.err
# (
# echo "ENVIRONMENT:"
# echo "-----------------------------------------------------------"
# env
# echo "-----------------------------------------------------------"
# echo "STDERR:"
# echo "-----------------------------------------------------------"
# set -x
do_cmd "$@"
# set +x
# ) > $TMPFILE_OUT 2>>$TMPFILE_ERR
# echo "-----------------------------------------------------------" 2>>$TMPFILE_ERR >&2
# cat $TMPFILE_OUT
