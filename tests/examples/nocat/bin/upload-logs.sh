#!/bin/sh

LOG_DIR=/tmp
AP_NAME=`uname -n`
DOC_ROOT=`grep '^[ \t]*DOCUMENT_ROOT[ \t]' /etc/nodog.conf | tr -s ' ' | cut -d' ' -f3`
LOG_FILES=`ls $DOC_ROOT/*.gz $LOG_DIR/*.gz 2>/dev/null`
AUTH_PORTAL=`grep AuthServiceAddr /etc/nodog.conf | tr -d \\\\ | tr -d \\" | cut -d'=' -f2`

upload_to_authserver() {

   for url in $AUTH_PORTAL; do
		uclient -c /etc/uclient.conf -u $1 "${url}/uploader"
   done
}

for file in $LOG_FILES
do
	LOG_RENAMED=$LOG_DIR/${AP_NAME}_`basename $file`

	mv $file $LOG_RENAMED

	upload_to_authserver $LOG_RENAMED

	if [ $? -eq 0 ]; then
		rm $LOG_RENAMED
	fi
done
