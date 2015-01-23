#!/bin/bash
 
# csp_LIST_CA.sh: Getting CA list
#
# ENV[HOME]         = Base directory for CA
# ENV[FILE_LOG]     = Log file for command
# ENV[MSG_LOG]      = Log separator
# ENV[OPENSSL]      = Openssl path
# ENV[ENGINE]       = Openssl Engine to use
# ENV[DEBUG]        = Enable debugging

if [ -z "${HOME}" ]; then
   echo "HOME is not set" >&2
   exit 1
fi

cd ${HOME}

if [ ! -f ../LCSP_command/.function ]; then
   echo "Unable to found ../LCSP_command/.function" >&2
   exit 1
fi

. ../LCSP_command/.function

chk_ENV `basename $0`

begin_CMD

echo_CMD 'for CA in `ls */openssl.cnf`; do dirname $CA; done'

(
for CA in `ls */openssl.cnf`; do
	dirname $CA
done
) 2>>${FILE_LOG}

end_CMD
