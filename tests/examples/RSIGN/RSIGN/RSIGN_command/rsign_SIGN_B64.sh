#!/bin/bash
 
# rsign_SIGN_B64.sh: Sign data with openssl
#
# ARGV[1] = DATA (base64 encoded)
# ARGV[2] = KEY  (PEM encoded)
#
# ENV[HOME]         = Base directory for op
# ENV[FILE_LOG]     = Log file for command
# ENV[MSG_LOG]      = Log separator
# ENV[OPENSSL]      = Openssl path
# ENV[ENGINE]       = Openssl Engine to use
# ENV[DEBUG]        = Enable debugging
# ENV[PASSWORD]     = Password for key decryption
# ENV[OPENSSL_CNF]  = Openssl configuration
# ENV[TMPDIR]       = Temporary directory

DATA=$1
KEY=$2

#echo $DATA	> 1
#echo $KEY	> 2

if [ -z "${DATA}" ]; then
	echo "DATA is empty" >&2
	exit 1
fi
if [ -z "${KEY}" ]; then
	echo "KEY is empty" >&2
	exit 1
fi

if [ -z "${HOME}" ]; then
	echo "HOME is not set" >&2
	exit 1
fi

cd ${HOME}

if [ ! -f RSIGN_command/.function ]; then
	echo "Unable to found RSIGN_command/.function" >&2
	exit 1
fi

. RSIGN_command/.function

chk_ENV `basename $0`

if [ -n "${OPENSSL_CNF}" ]; then
	T_CNF="-config ${OPENSSL_CNF}"
fi
if [ -n "${PASSWORD}" ]; then
	T_PASSWORD="-passin 'pass:${PASSWORD}'"
fi
if [ -z "${TMPDIR}" ]; then
	TMPDIR="/tmp"
fi

DEBUG_INFORMATION="ENV[TMPDIR]      = \"${TMPDIR}\"
ENV[OPENSSL_CNF]  = \"${OPENSSL_CNF}\"
ENV[PASSWORD]     = \"${PASSWORD}\"

ARGV[1] (DATA)   = \"${DATA}\"
ARGV[2] (KEY)    = \"${KEY}\"
"

FNAME1="${TMPDIR}/data.$$"
FNAME2="${TMPDIR}/key.$$"

# if something wrong exec this...

EXIT_CMD="rm -rf ${FNAME1} ${FNAME2}"

begin_CMD

run_CMD "echo \"\${DATA}\"" ${FNAME2}
run_CMD "${OPENSSL} base64 -d -in ${FNAME2} -out ${FNAME1}"
run_CMD "echo \"\${KEY}\""	 ${FNAME2}

if [ -f ${FNAME1} -a -f ${FNAME2} ]; then
	run_CMD "${OPENSSL} rsautl
				-sign
				${ENGINE}
				${T_CNF}
				${T_PASSWORD}
				-inkey ${FNAME2}
				-in ${FNAME1}"
fi

end_CMD
