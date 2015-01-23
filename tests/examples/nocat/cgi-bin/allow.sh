#!/bin/sh

# allow.sh

# set -x

if [ $# -eq 2 -a \
	  "$HTTP_HOST" = "10.30.1.131:5280" -a \
	  "$REQUEST_METHOD" = "GET" ]; then

	# $1 -> link
	# $2 -> mac

	/usr/lib/nodog/firewall/nodog.fw permit $2 $REMOTE_ADDR Member

	echo -e "Refresh: 1; url=http://$1\r\n\r\n<html><body>OK</body></html>"

	logger "Access allowed for MAC: $2 with IP: $REMOTE_ADDR"
fi
