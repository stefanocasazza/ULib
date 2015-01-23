#!/bin/bash

# unregistered-user.sh

. ./.env

set_ENV $0

if [ "$REQUEST_METHOD" = "GET" ]; then

	FORM_FILE=`cat $FORM_FILE_DIR/unregistered_user.tmpl`

	printf -v OUTPUT "$FORM_FILE"

	write_OUTPUT "$OUTPUT" 0

fi

exit 1
