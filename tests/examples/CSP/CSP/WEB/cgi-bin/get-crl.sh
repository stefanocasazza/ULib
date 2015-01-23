#!/bin/bash

# get_crl.sh

. ./.env

set_ENV $1-crl $1

if [ "$REQUEST_METHOD" = "GET" ]; then

 	$CSPCLIENT -c $CSPCLIENT_CFG -- 9 $CA_NAME >$CERT_FILE_PATH 2>/dev/null

	if [ -s $CERT_FILE_PATH ]; then

		if [ -z "$BROWSER_MSIE" ]; then
			echo -e "Content-Type: application/x-pkcs7-crl\r\n\r"
		else
			echo -e "Content-Type: application/pkix-crl\r\n\r"
		fi

#		cat $CERT_FILE_PATH
 		openssl crl -in $CERT_FILE_PATH -outform DER

		rm -f $CERT_FILE_PATH

		exit 0

	fi

fi

exit 1
