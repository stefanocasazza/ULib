#!/bin/bash
 
# csp_ZERO_CERTS.sh: Removing all certificate
#
# ARGV[1] = CA NAME
#
# ENV[HOME]         = Base directory for CA
# ENV[FILE_LOG]     = Log file for command
# ENV[MSG_LOG]      = Log separator
# ENV[OPENSSL]      = Openssl path
# ENV[ENGINE]       = Openssl Engine to use
# ENV[DEBUG]        = Enable debugging

CANAME=$1

#echo $CANAME > 1

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
ARGV[1] (CANAME)  = \"${CANAME}\"
"

proc_CMD "rm -f ${CANAME}/newcerts/*.pem"
