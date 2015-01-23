#!/bin/sh

. ../.function

rm -f uclient.log \
		out/uclient.out err/uclient.err \
		trace.*uclient*.[0-9]* object.*uclient*.[0-9]*

#./PIAZZE.sh
./nocat.sh

 UTRACE="0 10M 0"
 UOBJDUMP="0 100k 10"
#USIMERR="error.sim"
 export UTRACE UOBJDUMP USIMERR

DIR_CMD="../../examples/uclient"

#STRACE=$TRUSS
start_prg uclient -i -c uclient.cfg http://10.30.1.131:5280/check

killall userver_tcp userver_ssl
