#!/bin/bash

# doc_parse.sh

. ./.env

set_ENV $0

if [ "$REQUEST_METHOD" = "GET" ]; then

	if [ $# -eq 1 ]; then

		view_doc $1

	elif [ $# -eq 2 ]; then

		view_doc_tag $1 $2

	fi

fi

write_OUTPUT "$OUTPUT"

exit 1
