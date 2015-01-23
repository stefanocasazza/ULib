#!/bin/bash

# set -x
# env >/tmp/main.bash.env

# load common function
. ./.functions

if [ "$REQUEST_METHOD" = "GET" ]; then

	if [ $# -eq 1 ]; then

		view_page_image $1

	elif [ $# -eq 0 ]; then

	   if   [ "$REQUEST_URI" = "/" ]; then

			view_menu

	   elif [ "$REQUEST_URI" = "/card-generation" ]; then

			card_generation

	   elif [ "$REQUEST_URI" = "/stampa-contratto-business" -o \
				 "$REQUEST_URI" = "/stampa-contratto-residenziale-con-cpe" ]; then

			visualizza_contratto

	   elif [ "$REQUEST_URI" = "/stampa-rid-business" -o \
				 "$REQUEST_URI" = "/stampa-rid-residenziale-con-cpe" ]; then

	 		visualizza_rid

		else

			view_form_input

		fi
	fi

elif [ "$REQUEST_METHOD" = "POST" ]; then

	if [ $# -ge 25 ]; then

		visualizza_contratto "$@"

	elif [ $# -eq 4 -a "$REQUEST_URI" = "/card-generation" ]; then

		send_MAIL_for_card_generation "$1" "$2" "$3"

	elif [ $# -eq 1 ]; then

		if   [ "$1" = "Visualizza contratto" ]; then

			visualizza_contratto

		elif [ "$1" = "Visualizza RID" ]; then

			visualizza_rid

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

write_SSI
