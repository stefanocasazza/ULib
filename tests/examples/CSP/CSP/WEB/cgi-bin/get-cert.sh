#!/bin/bash

# get_cert.sh

. ./.env

set_ENV $1

if [ "$REQUEST_METHOD" = "GET" ]; then

	if [ -s $CERT_FILE_PATH ]; then

		if [ -s $CERT_FILE_PATH.err ]; then
			rm -f $CERT_FILE_PATH.err
		fi

#		application/x-x509-user-cert
#		----------------------------
#		The certificate being downloaded is a user certificate belonging to the user operating the Communicator.
#		If the private key associated with the certificate does not exist in the user's local key database, then an error dialog
#		is generated and the certificate is not imported. If a certificate chain is being imported then the first certificate in
#		the chain must be the user certificate, and any subsequent certificates will be added as untrusted CA certificates to the
#		local database.

		if [ -z "$BROWSER_MSIE" ]; then
			echo -e "Content-Type: application/x-x509-user-cert\r\n\r"
		else
			echo -e "Content-Type: application/x-x509-user-cert\r\n\r"
		fi

		cat $CERT_FILE_PATH

#		rm -f $CERT_FILE_PATH

		exit 0

	fi

fi

exit 1
