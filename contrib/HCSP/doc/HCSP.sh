#!/bin/sh

cnt=0
IFS=$'\n'
LIST=`cat csp.lst`

for i in $LIST
do
	cat >HCSP.$cnt <<END
CRYPT_PROVIDER    = $i
PROVIDER_TYPE     = 1
CRYPT_CONTAINER   = default
CERTIFICATE_STORE = AuthRoot
CERTIFICATE_NAME  = Microsoft
END
let cnt=cnt+1
done
