#!/bin/sh

do_cmd() {

	while read input
	do
		echo $input 1>&2
		echo $input
	done
}

DBG_FILE_OUT=/tmp/main_$$.out
DBG_FILE_ERR=/tmp/main_$$.err
(
echo "ENVIRONMENT:"
echo "-----------------------------------------------------------"
env
echo "-----------------------------------------------------------"
echo "STDERR:"
echo "-----------------------------------------------------------"
set -x
do_cmd "$@"
set +x
) > $DBG_FILE_OUT 2>>$DBG_FILE_ERR
echo "-----------------------------------------------------------" 2>>$DBG_FILE_ERR >&2
cat $DBG_FILE_OUT
