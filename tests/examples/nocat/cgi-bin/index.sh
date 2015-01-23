#!/bin/sh

# index.sh

# set -x

IMAGE=bannerWorldBF3.jpg
INTERNET=www.worldbachfest.it
AP_ADDRESS=`grep AuthServiceAddr /etc/nodog.conf | tr -d \\\\ | grep -v '#' | tr -d \\" | cut -d'/' -f3`
EVENTI=`echo $AP_ADDRESS`/cgi-bin/index.sh

allow_internet() {

	/usr/lib/nodog/firewall/nodog.fw permit $1 $2 Member

	echo -e "Refresh: 1; url=http://${INTERNET}\r\n\r\n<html><body>OK</body></html>"

	logger "Access allowed for MAC: $1 with IP: $2"
}

do_cmd() {

	if [ $# -eq 7 -a \
	  "$REQUEST_METHOD" = "GET" ]; then

		# $1 -> mac
		# $2 -> ip
		# $3 -> redirect
		# $4 -> gateway
		# $5 -> timeout
		# $6 -> token
		# $7 -> ap

		if [ "$REQUEST_URI" = "/login" ]; then

			OUTPUT=`cat /etc/nodog_index.tmpl 2>/dev/null`
			OUTPUT=`printf "$OUTPUT" "$EVENTI?$QUERY_STRING" "$IMAGE" "$EVENTI?$QUERY_STRING" 2>/dev/null`

			echo -e "Content-Type: text/html; charset=utf-8\r\n\r"
			echo -n -E "$OUTPUT"

			return

	 	elif [ "$REQUEST_URI" = "/cgi-bin/index.sh" ]; then

		 	allow_internet $1 $2

			return
		fi
	fi

	echo -e "Status: 400\r\n\r\n" \
        "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n" \
        "<html><head>\r\n" \
        "<title>400 Bad Request</title>\r\n" \
        "</head><body>\r\n" \
        "<h1>Bad Request</h1>\r\n" \
        "<p>Your browser sent a request that this server could not understand.<br />\r\n" \
        "</p>\r\n" \
        "<hr>\r\n" \
        "<address>ULib Server</address>\r\n" \
        "</body></html>\r"
}

do_cmd "$@"
