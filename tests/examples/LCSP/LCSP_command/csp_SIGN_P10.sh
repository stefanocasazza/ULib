#!/bin/bash
 
# csp_SIGN_P10.sh: Creating a new certificate
#
# ARGV[1] = CA NAME
# ARGV[2] = REQUEST
# ARGV[3] = POLICY
#
# ENV[HOME]         = Base directory for CA
# ENV[FILE_LOG]     = Log file for command
# ENV[MSG_LOG]      = Log separator
# ENV[OPENSSL]      = Openssl path
# ENV[ENGINE]       = Openssl Engine to use
# ENV[DEBUG]        = Enable debugging
# ENV[TMPDIR]       = Temporary directory

CANAME=$1
REQUEST=$2
POLICY=$3

#echo $CANAME	> 1
#echo $REQUEST > 2
#echo $POLICY	> 3

if [ -z "${CANAME}" ]; then
	echo "CA name is empty" >&2
	exit 1
fi
if [ -z "${REQUEST}" ]; then
   echo "REQUEST is empty" >&2
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

if [ -n "${POLICY}" ]; then
	POLICY="-policy ${POLICY}"
fi
if [ -z "${TMPDIR}" ]; then
	TMPDIR="/tmp"
fi

DEBUG_INFORMATION="ENV[TMPDIR]      = \"${TMPDIR}\"

ARGV[1] (CANAME)  = \"${CANAME}\"
ARGV[2] (REQUEST) = \"${REQUEST}\"
ARGV[3] (POLICY)  = \"${POLICY}\"
"

FNAME="${TMPDIR}/casign`date +%s`$$"

# if something wrong exec this...

EXIT_CMD="rm -rf ${FNAME}"

begin_CMD

run_CMD "echo \"\${REQUEST}\"" ${FNAME}

if [ -f ${FNAME} ]; then
	run_CMD "${OPENSSL} ca
				-config ${CANAME}/openssl.cnf
				-batch
				-notext
				${ENGINE}
				${POLICY}
				-name CA_${CANAME}
				-in ${FNAME}"
fi

end_CMD
