#!/bin/bash

# seek.sh

. ./.env

set_ENV $0

if [ "$REQUEST_METHOD" = "GET" ]; then

	if [ $# -eq 0 ]; then

		view_form_without_help

	elif [ $# -eq 1 ]; then

		if [ "$1" = "help" ]; then

			view_form_with_help

		else

			view_page_result $1

		fi
	fi

elif [ "$REQUEST_METHOD" = "POST" -a $# -eq 2 ]; then

	execute_query "$1" $2

fi

write_OUTPUT "$OUTPUT"

exit 1
