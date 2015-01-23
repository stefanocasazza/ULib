#!/bin/bash

# cert-install.sh

. ./.env

set_ENV $1 $2

if [ "$REQUEST_METHOD" = "GET" ]; then

 	if [ -s $REQ_CERT_PATH ]; then
	
		# call csp_client

		$CSPCLIENT -c $CSPCLIENT_CFG -- $CSP_OP $CA_NAME $REQ_CERT_PATH $POLICY >$CERT_FILE_PATH 2>$CERT_FILE_PATH.err

	else

		ERROR_MSG="Not found certificate request: $REQ_CERT_FILE"

	fi

 	if [ -s $CERT_FILE_PATH ]; then

		subject_dn=`openssl x509 -inform PEM -in $CERT_FILE_PATH -noout -subject | cut -b 10-`
		validity=`openssl   x509 -inform PEM -in $CERT_FILE_PATH -noout -enddate | cut -d= -f2`
		serial=`openssl	  x509 -inform PEM -in $CERT_FILE_PATH -noout -serial  | cut -d= -f2`
 
	   if [ -z "$BROWSER_MSIE" ]; then

			FORM_FILE=`cat $FORM_FILE_DIR/cert_install.tmpl`

			printf -v OUTPUT "$FORM_FILE" "$subject_dn" "$validity" "$serial" "$subject_dn" "$validity" $CERT_TOKEN

		else

			CERT=`openssl crl2pkcs7 -nocrl -inform PEM -certfile $CERT_FILE_PATH | grep -v ' ' | tr -d '\n'`

			FORM_FILE=`cat $FORM_FILE_DIR/MSIE_cert_install.tmpl`

			printf -v OUTPUT "$FORM_FILE" "$subject_dn" "$validity" $CERT "$serial" "$subject_dn" "$validity"

		fi

#		rm -f REQ_CERT_FILE

	else

 		rm -f  $CERT_FILE_PATH

		if [ -s $CERT_FILE_PATH.err ]; then

			ERROR_MSG=`cat $CERT_FILE_PATH.err`

		else

			rm -f  $CERT_FILE_PATH.err

		fi

		FORM_FILE=`cat $FORM_FILE_DIR/error.tmpl`

		printf -v OUTPUT "$FORM_FILE" "$ERROR_MSG" $CSP_OP $CERT_TOKEN

	fi

	write_OUTPUT "$OUTPUT" 0

fi

exit 1
