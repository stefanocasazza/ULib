#!/bin/bash
 
# csp_REVOKE_CERT.sh: Revoking a certificate
#
# ARGV[1] = CA NAME
# ARGV[2] = SERIAL NUMBER
#
# ENV[HOME]         = Base directory for CA
# ENV[FILE_LOG]     = Log file for command
# ENV[MSG_LOG]      = Log separator
# ENV[OPENSSL]      = Openssl path
# ENV[ENGINE]       = Openssl Engine to use
# ENV[DEBUG]        = Enable debugging

CANAME=$1
SNUMBER=$2

#echo $CANAME	> 1
#echo $SNUMBER > 2

if [ -z "${CANAME}" ]; then
	echo "CA name is empty" >&2
	exit 1
fi
if [ -z "${SNUMBER}" ]; then
	echo "ERROR: empty serial number" >&2
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

ARGV[1] (CANAME)  = \"${CANAME}\"
ARGV[2] (SNUMBER) = \"${SNUMBER}\"
"

begin_CMD

FNAME="${CANAME}/newcerts/${SNUMBER}.pem"

if [ -f ${FNAME} ]; then
	run_CMD "${OPENSSL} ca
				-config ${CANAME}/openssl.cnf
				-name CA_${CANAME}
				-revoke ${FNAME}"
else
	echo "ERROR: Certificate with serial ${SNUMBER} doesn't exists" >>${FILE_LOG}
	ESITO=1
fi

end_CMD
