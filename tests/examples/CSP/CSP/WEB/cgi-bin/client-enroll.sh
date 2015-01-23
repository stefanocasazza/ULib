#!/bin/bash

# client-enroll.sh

. ./.env

set_ENV $0

#	echo "browsertype  = $1"
#	echo "challenge    = $2"
#	echo "commonName	 = $3"
#	echo "emailAddress = $4"

if	[ $# -eq 0 -a "$REQUEST_METHOD" = "GET" ]; then

	EXPIRE="$DEFAULT_DAYS days"
	EXPIRE_DATE=`date -d "$EXPIRE"`

	FORM_FILE=`cat $FORM_FILE_DIR/client-enroll.tmpl`

	printf -v OUTPUT "$FORM_FILE" $DEFAULT_DAYS "$EXPIRE_DATE"

elif	[ $# -eq 4 ]; then

	if [ "$1" = "Microsoft Internet Explorer" ]; then

		FORM_FILE=`cat $FORM_FILE_DIR/MSIE.tmpl`

		printf -v OUTPUT "$FORM_FILE" "$3" "$4" "$3" "$4" "$2" "$3" "$3" "$4" "$4"

	else

		FORM_FILE=`cat $FORM_FILE_DIR/SPKAC.tmpl`

		printf -v OUTPUT "$FORM_FILE"	"$1" "$1" "$2" "$3" "$3" "$4" "$4" "$2"

	fi

elif [ $# -eq 6 -a "$REQUEST_METHOD" = "POST" ]; then

#	echo "CA_name = $5"
#	echo "SPKAC   = $6"

	set_ENV "`date +%s`$$"

	printf "commonName = %s\nemailAddress = %s\nSPKAC = %s" "$3" "$4" "$6" > $REQ_CERT_PATH

	FORM_FILE=`cat $FORM_FILE_DIR/cert_req_stored.tmpl`

	printf -v OUTPUT "$FORM_FILE" "$1" "$3" "$4" ${#6}

elif [ $# -eq 7 -a "$REQUEST_METHOD" = "POST" ]; then

#	echo "CA_name = $5"
#	echo "KeySize = $6"
#	echo "PKCS10  = $7"

	set_ENV "`date +%s`$$"

	echo "-----BEGIN CERTIFICATE REQUEST-----"  > $REQ_CERT_PATH
	echo "$7" | fold -w 64							 >> $REQ_CERT_PATH
	echo "-----END CERTIFICATE REQUEST-----"	 >> $REQ_CERT_PATH

	FORM_FILE=`cat $FORM_FILE_DIR/cert_req_stored.tmpl`

	printf -v OUTPUT "$FORM_FILE" "$1" "$3" "$4" ${#7}

fi

write_OUTPUT "$OUTPUT" 1

if [ $CERT_TOKEN -ne 0 ]; then

	FORM_FILE=`cat $FORM_FILE_DIR/mail.tmpl`

	printf "$FORM_FILE" "$4" req-confirmation@register.test.trust-engine.com \
							  "$1" "$2" "$3" "$4" http://10.30.1.131:4433/CSP/WEB/cgi-bin/cert-install.sh?req=$CERT_TOKEN | \
							  mailx -s "Richiesta di emissione certificato accettata" $4 2>/dev/null

	exit 0

fi

exit 1
