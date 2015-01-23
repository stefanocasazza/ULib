#-----------------------------------
# START FUNCTION
#-----------------------------------
print_page() {

	if [ -r $DIR_TEMPLATE/$REQUEST_URI.tmpl ]; then

		SSI_BODY=`cat $DIR_TEMPLATE/$REQUEST_URI.tmpl 2>/dev/null`

		if [ $# -ne 0 ]; then
			SSI_BODY=`printf "$SSI_BODY" "$@" 2>/dev/null`
		fi
	fi

	EXIT_VALUE=0
}

message_page() {

	TITLE_TXT="$1"
	REQUEST_URI=message_page

	shift

	print_page "$@"

	write_SSI
}

# NB: check if /etc/openldap/ldap.conf contains TLS_REQCERT never
# --------------------------------------------------------------------------------------------------------------------
# ldapsearch -v -H 'ldaps://94.138.39.149:636' -b ou=users,o=unwired-portal -D cn=admin,o=unwired-portal -w programmer

ask_to_LDAP() {

	# $1 -> cmd
	# $2 -> param
	# $3 -> option
	# $4 -> filter_option

	TMPFILE=/tmp/ask_to_LDAP.$$

	if [ "$1" = ldapsearch ]; then
		ldapsearch $2 "$3" $4 >$TMPFILE.out 2>$TMPFILE.err
	else
				  $1 $2 <<END   >$TMPFILE.out 2>$TMPFILE.err
$3
END
	fi

	EXIT_VALUE=$?

	if [ $EXIT_VALUE -eq 0 ]; then

		rm -f $TMPFILE.err

	elif [ -s $TMPFILE.err ]; then

		rm -f $TMPFILE.out

		grep '(-1)' < $TMPFILE.err # Can't contact LDAP server (-1)

#		if [ $? -eq 0 ]; then
#			anomalia 8
#		fi
	fi
}

load_policy() {

	if [ -n "$1" -a -s "$1" ]; then
		# ------------------------------------------------------------------------
		# GET POLICY FROM FILE (UUID_TO_LOG POLICY USER_MAX_TIME USER_MAX_TRAFFIC)
		# ------------------------------------------------------------------------
		POLICY=""

		read UUID_TO_LOG POLICY USER_MAX_TIME USER_MAX_TRAFFIC < $1 2>/dev/null
	fi

	POLICY_FILE=$DIR_POLICY/$POLICY

 	mkdir -p $DIR_CNT/$POLICY

	unset POLICY_RESET
	unset BONUS_FOR_EXIT

	source $DIR_POLICY/$POLICY
	# --------------------------------------------------------------------
	# TIME POLICY
	# --------------------------------------------------------------------
	if [ -z "$MAX_TIME" ]; then
		MAX_TIME=0
	fi

	if [ -n "$USER_MAX_TIME" ]; then
		if [ $USER_MAX_TIME -ne $MAX_TIME ]; then
			MAX_TIME=$USER_MAX_TIME
		fi
	fi
	# --------------------------------------------------------------------
	# TRAFFIC POLICY
	# --------------------------------------------------------------------
	if [ -z "$MAX_TRAFFIC" ]; then
		MAX_TRAFFIC=0
	fi

	if [ -n "$USER_MAX_TRAFFIC" ]; then
		if [ $USER_MAX_TRAFFIC -ne $MAX_TRAFFIC ]; then
			MAX_TRAFFIC=$USER_MAX_TRAFFIC
		fi
	fi
	# --------------------------------------------------------------------
}

load_value_session() {

	if [ -s $TMP_FORM_FILE ]; then
		i=1
		while read LINE
		do
			eval v$i=\"$LINE\"

			let "i = i + 1"
		done < $TMP_FORM_FILE
	fi
}

get_user_nome_cognome() {

	# $1 -> uuid

	TMP_FORM_FILE=$DIR_REG/$1.reg

	v1=""
	v2=""

	load_value_session

	if [ -n "$v1$v2" ]; then
		USER="$v1 $v2"
	else
		USER=$UUID
	fi
}

uscita() {

	rm -f $TMPFILE.out $TMPFILE.err

	if [ $EXIT_VALUE -eq 0 ]; then
		exit 0
	fi

	exit 400 # Bad Request
}

write_SSI() {

	if [ -n "$CONNECTION_CLOSE" ]; then
		HTTP_RESPONSE_HEADER="Connection: close\r\n$HTTP_RESPONSE_HEADER"
	fi

	if [ -n  "$HTTP_RESPONSE_HEADER" ]; then
		echo \"HTTP_RESPONSE_HEADER=$HTTP_RESPONSE_HEADER\"
	fi

	if [ -n "$HTTP_RESPONSE_BODY" ]; then
		echo  \"HTTP_RESPONSE_BODY=$HTTP_RESPONSE_BODY\"
	else
		if [ -z "$SSI_BODY" ]; then
			EXIT_VALUE=400
			uscita
		fi

		if [ -z "$TITLE_TXT" ]; then
			TITLE_TXT=$TITLE_DEFAULT
		fi

		echo -e "'TITLE_TXT=$TITLE_TXT'\nBODY_STYLE=$BODY_STYLE"
		
		if [ -n "$SSI_HEAD" ]; then
			echo "'SSI_HEAD=$SSI_HEAD'"
		fi

		echo -n -e "$SSI_BODY" > $SSI_FILE_BODY
	fi

	uscita
}

main() {

	if [ $DEBUG -eq 0 ]; then
		do_cmd "$@"
	else
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
	fi
}
#-----------------------------------
# END FUNCTION
#-----------------------------------
