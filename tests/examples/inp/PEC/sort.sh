#!/bin/bash

for FILE in `find $1 -name '*.log.p7m'`
do
	DIRECTORY=`dirname $FILE`
	FILENAME_VERIFIED=`basename $FILE .p7m`
	openssl smime -in $FILE -verify -noverify -inform DER | sort -t':' -k4 > $DIRECTORY/$FILENAME_VERIFIED 
done
