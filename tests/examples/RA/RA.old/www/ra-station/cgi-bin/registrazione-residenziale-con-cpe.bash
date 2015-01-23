#!/bin/bash

# registrazione-residenziale-con-cpe.sh

. ./.env

if [ "$REQUEST_METHOD" = "GET" ]; then

	if [ $# -eq 0 ]; then

		view_form_input

	elif [ $# -eq 1 ]; then

		view_page_image $1 

	fi

elif [ "$REQUEST_METHOD" = "POST" ]; then

	if [ $# -eq 31 ]; then

		visualizza_contratto "$@"

	elif [ $# -eq 1 ]; then

		if [ "$1" = "Visualizza contratto" ]; then

			visualizza_contratto

		elif [ "$1" = "Visualizza RID" ]; then

			# codice fiscale
			visualizza_rid 6 MENSILE

		elif [ "$1" = "Reset scansione" ]; then

			reset_scansione

		elif [ "$1" = "Attiva scansione singola" ]; then

			rascan_image_singola

		elif [ "$1" = "Attiva scansione multipla" ]; then

			rascan_image_multipla

		elif [ "$1" = "Visualizza scansione" ]; then

			view_page_image 1

		elif [ "$1" = "Registra contratto" ]; then

			registrazione_contratto

		fi
	fi
fi

write_OUTPUT
