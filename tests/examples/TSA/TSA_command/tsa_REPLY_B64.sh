#!/bin/sh
 
# tsa_REPLY_B64.sh: reply to a TSA request
#
# ARGV[1] = TSA REQUEST
# ARGV[2] = TOKEN
# ARGV[3] = SEC
# ARGV[3] = POLICY
#
# ENV[HOME]         = Base directory for CA
# ENV[FILE_LOG]     = Log file for command
# ENV[MSG_LOG]      = Log separator
# ENV[OPENSSL]      = Openssl path
# ENV[ENGINE]       = Openssl Engine to use
# ENV[DEBUG]        = Enable debugging
# ENV[TMPDIR]       = Temporary directory
# ENV[OPENSSL_CNF]  = Openssl configuration
# ENV[PASSWORD]     = Password for key decryption
# ENV[TSA_CERT]     = TSA certificate
# ENV[TSA_CACERT]   = TSA CA chain certificate
# ENV[TSA_KEY]      = TSA private key

REQUEST=$1
TOKEN=$2
SECTION=$3
POLICY=$4

if [ -z "${REQUEST}" ]; then
	echo "ERROR: empty TSA request" >&2
	exit 1
fi
if [ -z "${TSA_CERT}" ]; then
	echo "ERROR: empty TSA certificate" >&2
	exit 1
fi
if [ -z "${TSA_CACERT}" ]; then
	echo "ERROR: empty TSA CA certificate" >&2
	exit 1
fi
if [ -z "${TSA_KEY}" ]; then
	echo "ERROR: empty TSA key" >&2
	exit 1
fi

if [ -z "${HOME}" ]; then
	echo "HOME is not set" >&2
	exit 1
fi

cd ${HOME}

if [ ! -f TSA_command/.function ]; then
   echo "Unable to found TSA_command/.function" >&2
   exit 1
fi

. TSA_command/.function

chk_ENV `basename $0`

if [ -n "${OPENSSL_CNF}" ]; then
	T_CNF="-config ${OPENSSL_CNF}"
fi
if [ -n "${PASSWORD}" ]; then
	T_PASSWORD="-passin 'pass:${PASSWORD}'"
fi
if [ -n "${TOKEN}" ]; then
	T_TOKEN="-token_out"
	T_TOKEN_OUT="-token_in"
fi
if [ -n "${SECTION}" ]; then
	T_SECTION="-section ${SECTION}"
fi
if [ -n "${POLICY}" ]; then
	T_POLICY="-policy ${POLICY}"
fi
if [ -z "${TMPDIR}" ]; then
	TMPDIR="/tmp"
fi

DEBUG_INFORMATION="
ENV[TMPDIR]       = \"${TMPDIR}\"
ENV[OPENSSL_CNF]  = \"${OPENSSL_CNF}\"
ENV[PASSWORD]     = \"${PASSWORD}\"
ENV[TSA_CERT]     = \"${TSA_CERT}\"
ENV[TSA_KEY]      = \"${TSA_KEY}\"
ENV[TSA_CACERT]   = \"${TSA_CACERT}\"

ARGV[1] (REQUEST) = \"${REQUEST}\"
ARGV[2] (TOKEN)   = \"${TOKEN}\"
ARGV[3] (SECTION) = \"${SECTION}\"
ARGV[4] (POLICY)  = \"${POLICY}\"
"

FNAME="${TMPDIR}/tsa.$$"
FNOUT="${TMPDIR}/tsa.`date +%s`$$"

# if something wrong exec this...

EXIT_CMD="rm -rf ${FNAME} ${FNOUT}"

begin_CMD

run_CMD "echo \"\${REQUEST}\"" ${FNAME}
run_CMD "${OPENSSL} base64 -d -in ${FNAME} -out ${FNOUT}"

if [ -f ${FNOUT} ]; then
   run_CMD "${OPENSSL} ts -query -in ${FNOUT} -out ${FNAME} -text"

	grep "Certificate required: yes" < ${FNAME} >/dev/null

	if test $?; then
		T_CHAIN="-chain ${TSA_CACERT}"
	fi

	run_CMD "${OPENSSL} ts
				-reply
				${T_CNF}
				-signer ${TSA_CERT}
				${T_PASSWORD}
				-inkey "${TSA_KEY}"
				-queryfile ${FNOUT}
				${T_SECTION}
				${T_POLICY}
				${T_TOKEN}
				${T_CHAIN}"
fi

end_CMD
