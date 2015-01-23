#!/bin/bash

# get_ca.sh

. ./.env

set_ENV $1-cert $1

if [ "$REQUEST_METHOD" = "GET" ]; then

 	$CSPCLIENT -c $CSPCLIENT_CFG -- 10 $CA_NAME >$CERT_FILE_PATH 2>/dev/null

	if [ -s $CERT_FILE_PATH ]; then

#		application/x-x509-ca-cert
#		--------------------------
#		The certificate being downloaded represents a Certificate Authority. When it is downloaded the user will be shown a sequence
#		of dialogs that will guide them through the process of accepting the Certificate Authority and deciding if they wish to trust
#		sites certified by the CA. If a certificate chain is being imported then the first certificate in the chain must be the CA
#		certificate, and any subsequent certificates will be added as untrusted CA certificates to the local database.

	   if [ -z "$BROWSER_MSIE" ]; then
			echo -e "Content-Type: application/x-x509-ca-cert\r\n\r"
		else
			echo -e "Content-Type: application/pkix-cert\r\n\r"
		fi

# 		cat $CERT_FILE_PATH
  		openssl x509 -in $CERT_FILE_PATH -outform DER

		rm -f $CERT_FILE_PATH

		exit 0

	fi

fi

exit 1
