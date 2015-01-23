#!/bin/bash

# csp_CA.sh: Creating a new CA
#
# ARGV[1] = CA NAME
# ARGV[2] = CA certificate life in days
# ARGV[3] = CA openssl.cnf
#
# ENV[HOME]         = Base directory for CA
# ENV[FILE_LOG]     = Log file for command
# ENV[MSG_LOG]      = Log separator
# ENV[OPENSSL]      = Openssl path
# ENV[ENGINE]       = Openssl Engine to use
# ENV[CNF_TEMPLATE] = Template for openssl.cnf
# ENV[CA_EXT]       = Section for CA extensions
# ENV[START_SERIAL] = Start CA serial number
# ENV[DEBUG]        = Enable debugging

CANAME=$1
CACERT_DAYS=$2
CNF=$3

#echo  $CANAME			> 1
#echo  $CACERT_DAYS  > 2
#echo "$CNF"			> 3

if [ -z "${CANAME}" ]; then
	echo "CA name is empty" >&2
	exit 1
fi
if [ -z "${CACERT_DAYS}" ]; then
	echo "CACERT_DAYS is empty" >&2
	exit 1
fi

if [ -z "${HOME}" ]; then
   echo "HOME is not set" >&2
   exit 1
fi

cd ${HOME}

if [ -f "${CANAME}/serial" ]; then
	echo "CA ${CANAME} already exist" >&2
	exit 1
fi

if [ -z "${CNF}" ]; then
	if [ -z "${CNF_TEMPLATE}" ]; then
		CNF="`cat ./openssl.cnf.tmpl`"
		test $? || exit $?
	else
		CNF="`cat \"${CNF_TEMPLATE}\"`"
	fi
	if [ -z "${CNF}" ]; then
		echo "CNF is empty" >&2
		exit 1
	fi
fi

if [ ! -f ../LCSP_command/.function ]; then
   echo "Unable to found ../LCSP_command/.function" >&2
   exit 1
fi

. ../LCSP_command/.function

chk_ENV `basename $0`

# if something wrong exec this...

EXIT_CMD="rm -rf ${CANAME}"

# start

if [ -z "${CA_EXT}" ]; then
   CA_EXT="CA_x509_extensions"
fi
if [ -z "${START_SERIAL}" ]; then
   START_SERIAL="01"
fi

DEBUG_INFORMATION="ENV[CNF_TEMPLATE] = \"${CNF_TEMPLATE}\"
ENV[CA_EXT]       = \"${CA_EXT}\"
ENV[START_SERIAL] = \"${START_SERIAL}\"

ARGV[1] (CANAME)  = \"${CANAME}\"
ARGV[2] (DAYS)    = \"${CACERT_DAYS}\"
ARGV[3]=-----OPENSSL.CNF BEGIN-----
${CNF}
-----OPENSSL.CNF END-----
"

begin_CMD

# create the directory hierarchy

run_CMD "mkdir -p ${CANAME}/crl
						${CANAME}/certs
						${CANAME}/newcerts
						${CANAME}/private"

run_CMD "echo ${START_SERIAL}" ${CANAME}/serial
run_CMD "touch ${CANAME}/index.txt"
run_CMD "touch ${CANAME}/private/cakey.pem"
run_CMD "chmod u=rw,go= ${CANAME}/private/cakey.pem"

run_CMD "printf \"\${CNF}\" ${CANAME} ${CANAME} ${CANAME}"  ${CANAME}/openssl.cnf

# maybe reuse the private key...

if [ -f "/srv/keys/priv/${CANAME}.pem.key" ]
then
	run_CMD "cp /srv/keys/priv/${CANAME}.pem.key ${CANAME}/private/cakey.pem"
else
	KEY_SIZE=`cat "${CANAME}/openssl.cnf" | awk '/default_bits/{gsub(".*=[ \t]*",""); print; exit}'`

	run_CMD "${OPENSSL} genrsa
				${ENGINE}
				-out ${CANAME}/private/cakey.pem
				$KEY_SIZE"
fi

# generate the certificate request

run_CMD "${OPENSSL} req
			${ENGINE}
			-config ${CANAME}/openssl.cnf
			-batch
			-new
			-key ${CANAME}/private/cakey.pem
			-out ${CANAME}/cacert.req"

# check for openssl version

${OPENSSL} version -v | awk '/0\.9\.[5678]/{exit 1}'

if [ $? -eq 1 ]; then
	run_CMD "${OPENSSL} req
				${ENGINE}
				-config ${CANAME}/openssl.cnf
				-days ${CACERT_DAYS}
				-batch
				-x509
				-set_serial 0
				-key ${CANAME}/private/cakey.pem
				-in ${CANAME}/cacert.req
				-extensions ${CA_EXT}
				-out ${CANAME}/cacert.pem"
else
	run_CMD "${OPENSSL} ca
				${ENGINE}
				-config ${CANAME}/openssl.cnf
				-days ${CACERT_DAYS}
				-batch
				-selfsign
				-create_serial
				-name CA_${CANAME}
				-keyfile ${CANAME}/private/cakey.pem
				-extensions ${CA_EXT}
				-out ${CANAME}/cacert.pem
				-infiles ${CANAME}/cacert.req"
fi

unset EXIT_CMD

end_CMD
