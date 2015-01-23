#!/bin/bash
 
# csp_GET_CA.sh: Getting list CA certificates
#
# ARGV[1] = CA NAME
# ARGV[2] = output compress
#
# ENV[HOME]         = Base directory for CA
# ENV[FILE_LOG]     = Log file for command
# ENV[MSG_LOG]      = Log separator
# ENV[OPENSSL]      = Openssl path
# ENV[ENGINE]       = Openssl Engine to use
# ENV[DEBUG]        = Enable debugging

CANAME=$1
COMPRESS=$2

#echo $CANAME	 > 1
#echo $COMPRESS > 2

if [ -z "${CANAME}" ]; then
	echo "CA name is empty" >&2
	exit 1
fi

if [ -z "${HOME}" ]; then
   echo "HOME is not set" >&2
   exit 1
fi

cd ${HOME}

if [ ! -f ${CANAME}/cacert.pem ]; then
   echo "ERROR: CA ${CANAME} doesn't exists" >&2
   exit 1
fi

if [ ! -f ../LCSP_command/.function ]; then
   echo "Unable to found ../LCSP_command/.function" >&2
   exit 1
fi

. ../LCSP_command/.function

chk_ENV `basename $0`

DEBUG_INFORMATION="
ARGV[1] (CANAME)   = \"${CANAME}\"
ARGV[2] (COMPRESS) = \"${COMPRESS}\"
"

begin_CMD

(
if [ -n "${COMPRESS}" -a "${COMPRESS}" = "1" ]; then
	echo_CMD '(for CERT in `ls ${CANAME}/newcerts/*.pem`; do openssl x509 -in ${CERT} -outform PEM; done) | gzip -c >$$.tmp; openssl base64 -in $$.tmp -e'

	( for CERT in `ls ${CANAME}/newcerts/*.pem`; do
		${OPENSSL} x509 \
		-in ${CERT} \
		-outform PEM
	  done ) | gzip -c >$$.tmp; ${OPENSSL} base64 -in $$.tmp -e
else
	echo_CMD 'for CERT in `ls ${CANAME}/newcerts/*.pem`; do openssl x509 -in ${CERT} -outform PEM; done'

	for CERT in `ls ${CANAME}/newcerts/*.pem`; do
		${OPENSSL} x509 \
		-in ${CERT} \
		-outform PEM
	done
fi
) 2>>${FILE_LOG}

rm -f $$.tmp

end_CMD
