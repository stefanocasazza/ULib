#!/bin/bash

# set -x

# internal var
OP=""
AP=""
IP=""
MAC=""
UUID=""
USER=""
WA_CID=""
WA_UID=""
OUTPUT=""
TMPFILE=""
GATEWAY=""
MAX_TIME=""
FILE_CTX=""
FILE_CNT=""
FILE_UID=""
REQ_FILE=""
SSI_HEAD=""
SSI_BODY=""
TITLE_TXT=""
EXIT_VALUE=0
CALLER_ID=""
BODY_STYLE=""
CONSUME_ON=""
MAX_TRAFFIC=""
SIGNED_DATA=""
UUID_TO_LOG=""
AUTH_DOMAIN=""
POLICY_FILE=""
POLICY_RESET=""
TMP_FORM_FILE=""
USER_MAX_TIME=""
BONUS_FOR_EXIT=""
UUID_TO_APPEND=""
CONNECTION_CLOSE=""
USER_MAX_TRAFFIC=""
ACCESS_POINT_NAME=""
FILE_RESPONSE_HTML=""
HTTP_RESPONSE_BODY=""
REMAINING_TIME_MIN=""
REMAINING_TRAFFIC_MB=""
HTTP_RESPONSE_HEADER=""

# external var
# SERVICE=""
# TELEFONO=""
# HELP_URL=""
# LOGIN_URL=""
# WALLET_URL=""
# LOGOUT_HTML=""
# LOGOUT_NOTE=""
# MSG_ANOMALIA=""
# FMT_AUTH_CMD=""
# URL_BANNER_AP=""
# REDIRECT_DEFAULT=""
# URL_BANNER_COMUNE=""
# REGISTRAZIONE_URL=""
# GET_USER_INFO_INTERVAL=0

. $DIR_ROOT/etc/$VIRTUAL_HOST/script.conf
# common func
. $DIR_ROOT/etc/common.sh

#-----------------------------------
# START FUNCTION
#-----------------------------------
write_ENV() {

	return
	(
	echo "ENVIRONMENT:"
	echo "-----------------------------------------------------------"
	env | sort
	echo "-----------------------------------------------------------"
	) > /tmp/main_$$.env
}

write_LOG() {

	logger -p $LOCAL_SYSLOG_SELECTOR "$PORTAL_NAME: $REQUEST_URI: $1"
}

write_FILE() {

	# $1 -> data
	# $2 -> filename
	# $3 -> option

	local filename=`basename $2 2>/dev/null`

	if [ -d $2 -o "${filename:0:1}" = "." ]; then
		write_ENV
		write_LOG "write_FILE() failure (anomalia 002) on data=$1 filename=$2 option=$3"
	else
		echo $3 "$1" > $2

		if [ $? -ne 0 ]; then
			anomalia 2 "$2"
		fi
	fi
}

append_to_FILE() {

	# $1 -> data
	# $2 -> filename

	local filename=`basename $2 2>/dev/null`

	if [ -d $2 -o "${filename:0:1}" = "." ]; then
		write_ENV
		write_LOG "append_to_FILE() failure (anomalia 003) on data=$1 filename=$2"
	else
		echo "$1" >> $2

		if [ $? -ne 0 ]; then
			anomalia 3 "$2"
		fi
	fi
}

anomalia() {

	unset BACK_TAG
	# ------------------------------------------------------------
	# 10 load_policy (policy not set or policy without file)
	#  9 login_request (req without ctx)
	#  8 ask_to_LDAP
	#  7 info_notified_from_nodog (RENEW)
	#  6 info_notified_from_nodog (missing ctx)
	#  5 send_request_to_nodog (curl | uclient)
	#  4 send_request_to_nodog (missing ap)
	# ------------------------------------------------------------
	# CRITICAL:
	# ------------------------------------------------------------
	#  3 append_to_FILE
	#  2 write_FILE
	# ------------------------------------------------------------
	# $1 -> exit value
	# -----------------------------------------------
	EXIT_VALUE=$1

	case "$1" in
	10)
		write_ENV
		write_LOG "load_policy() failure (anomalia 010) POLICY=$POLICY"

		message_page "$SERVICE" "$SERVICE (anomalia 010). Contattare l'assistenza: $TELEFONO"
	;;
	9)
		write_ENV
		write_LOG "login_request() failure (anomalia 009) IP=$IP MAC=$MAC"

		MSG=`printf "$MSG_ANOMALIA" 009`
		BACK_TAG="<a class=\"back\" href=\"$REDIRECT_DEFAULT\">RIPROVA</a>"

		message_page "$SERVICE" "$MSG"
	;;
	8)
		write_LOG "Servizio LDAP non disponibile (anomalia 008)"

		if [ "$HTTPS" = "on" ]; then
			MSG="Servizio LDAP non disponibile (anomalia 008). Contattare l'assistenza: $TELEFONO"
		else
			MSG=`printf "$MSG_ANOMALIA" 008`
			BACK_TAG="<a class=\"back\" href=\"$REDIRECT_DEFAULT\">RIPROVA</a>"
		fi

		message_page "Servizio LDAP non disponibile" "$MSG"
	;;
	7)
		write_ENV
		write_LOG "info_notified_from_nodog() failure (anomalia 007) IP=$IP MAC=$MAC"

		ask_nodog_to_logout_user $IP $MAC

		return
	;;
	6)
		write_LOG "info_notified_from_nodog() failure (anomalia 006) IP=$IP MAC=$MAC file_ctx=$FILE_CTX"

		ask_nodog_to_logout_user $IP $MAC

		return
	;;
	5)
		write_LOG "info_notified_from_nodog() failure (anomalia 005) gateway=$GATEWAY"

		message_page "$SERVICE" "$SERVICE (anomalia 005). Contattare l'assistenza: $TELEFONO"
	;;
	4)
		write_ENV
		write_LOG "info_notified_from_nodog() failure (anomalia 004) gateway=$GATEWAY"

		message_page "$SERVICE" "$SERVICE (anomalia 004). Contattare l'assistenza: $TELEFONO"
	;;
	*)
	  #chmod 777 $DIR_WEB/$VIRTUAL_HOST/ANOMALIA
		write_LOG "info_notified_from_nodog() failure (anomalia 00$1) on file=$2"

		message_page "$SERVICE" "$SERVICE per anomalia interna. Contattare l'assistenza: $TELEFONO"
	;;
	esac

	uscita
}

is_group_ACCOUNT() {

	# $1 -> uid
	# $2 -> password (opzionale)

	local uid=$1
	local password=$2

	# List of privileged ACCOUNT

	local file=$DIR_ROOT/etc/$VIRTUAL_HOST/.GROUP_ACCOUNT

	test -s $file && awk -F : '$1 == "'"$uid"'" && (! length("'"$password"'") || $2 == "'"$password"'") {found = 1} END {exit found ? 0 : 1}' $file
}

get_user_context_connection() {

   # $1 -> uid
   # $2 -> mac

   if [ -n "$1" ]; then
      FILE_CTX=$DIR_CTX/$1.ctx
   else
      FILE_CTX=`grep -l $REMOTE_ADDR $DIR_CTX/*.ctx 2>/dev/null`
   fi

   # data connection context saved on file
   # -------------------------------------
   # ap uid gateway mac ip auth_domain

   if [ -n "$FILE_CTX" -a -s "$FILE_CTX" ]; then
      read           AP UUID GATEWAY MAC IP AUTH_DOMAIN < $FILE_CTX 2>/dev/null
   else
      unset FILE_CTX AP UUID GATEWAY MAC IP AUTH_DOMAIN
   fi
}

check_if_user_is_connected() {

	# $1 -> mac
	# $2 -> ip
	# $3 -> gateway
	# $4 -> ap
	# $5 -> uid

	get_user_context_connection "$5" "$1"

	if [ -n "$FILE_CTX" ]; then

		if [ "$MAC"		 != "$1" -o \
			  "$IP"		 != "$2" -o \
			  "$GATEWAY" != "$3" -o \
			  "$AP"		 != "$4" ]; then

			OP=RENEW
		fi
	fi
}

check_if_user_connected_to_AP_NO_CONSUME() {

	# List of Access Point with NO CONSUME

	FILE=$DIR_ROOT/etc/.AP_NO_CONSUME

	if [ -s $FILE ]; then

		AP_WITH_NO_CONSUME=`egrep "$1" $FILE 2>/dev/null`

		if [ -n "$AP_WITH_NO_CONSUME" ]; then

			unset CONSUME_ON

			return
		fi
	fi

	CONSUME_ON=true
}

main_page() {

	# -----------------------------------------------------------------------------------------------------------------------------------------------
	# GET /login?mac=00%3A14%3AA5%3A6E%3A9C%3ACB&ip=192.168.226.2&redirect=http%3A%2F%2Fgoogle&gateway=192.168.226.1%3A5280&timeout=0&token=x&ap=lab2
	# -----------------------------------------------------------------------------------------------------------------------------------------------
	# $1 -> mac
	# $2 -> ip
	# $3 -> redirect
	# $4 -> gateway
	# $5 -> timeout
	# $6 -> token
	# $7 -> ap
	# -----------------------------------------------------------------------------
	# 00:e0:4c:d4:63:f5 10.30.1.105 http://google 10.30.1.131:5280 stefano 0 x lab2
	# -----------------------------------------------------------------------------

	get_user_context_connection "" "$1"

	if [ -n "$GATEWAY" ]; then

		# check if he is still connected...
		# -----------------------------------------------------------------------------
		# NB: we need PREFORK_CHILD > 2
		# -----------------------------------------------------------------------------
		send_request_to_nodog "status?ip=$REMOTE_ADDR"

		echo "$OUTPUT" | grep PERMIT >/dev/null 2>&1

		if [ $? -ne 0 ]; then

			unset UUID

			# NB: FILE_CTX is set by get_user_context_connection...

			rm -f $FILE_CTX
		fi
	fi

	if [ -z "$UUID" ]; then
		# ------------------------------------------------------------------------------------------------------------------------------------------------
		PARAM="$7 $4 $1 $2 $3 $5 $6"
		# ------------------------------------------------------------------------------------------------------------------------------------------------
		# 1 -> ap
		# 2 -> gateway
		# 3 -> mac
		# 4 -> ip
		# 5 -> redirect
		# 6 -> timeout
		# 7 -> token
		# 8 -> uuid
		# ------------------------------------------------------------------------------------------------------------------------------------------------
		# 10.30.1.131:5280 00:e0:4c:d4:63:f5 10.30.1.105 http://www.google.com 0 10.30.1.105&1257603166&2a2436611f452f8eebddce4992e88f8d 055340773
		# ------------------------------------------------------------------------------------------------------------------------------------------------
		if [ "$PARAM" != "      " ]; then
			save_connection_request "$PARAM" # save nodog data on file
		fi

		UUID_TO_APPEND=1

		user_has_valid_MAC "$@"

		if [ -n "$SSL_CLIENT_CERT_SERIAL" ]; then
			user_has_valid_cert "$@"
		fi
	fi

	print_page "$URL_BANNER_AP" "$HELP_URL" "$WALLET_URL" "$7" "/login_request?$QUERY_STRING" "$URL_BANNER_COMUNE"
}

unifi_page() {

	REQUEST_URI=unifi_page

	print_page "$URL_BANNER_AP" "$HELP_URL" "$WALLET_URL" unifi "/unifi_login_request" "$URL_BANNER_COMUNE"
}

logged_page() {

	get_user_context_connection "" ""

	if [ -n "$AP" ]; then
		print_page "$URL_BANNER_AP" "$HELP_URL" "$WALLET_URL" $AP "/logged_login_request" "$URL_BANNER_COMUNE"
	else
		HTTP_RESPONSE_BODY="<html><body>OK</body></html>"
		HTTP_RESPONSE_HEADER="Refresh: 0; url=http://www.google.com\r\n"
	fi
}

send_ticket_to_nodog() {

	# ------------------------
	# $1 -> mac
	# $2 -> ip
	# $3 -> redirect
	# $4 -> gateway
	# $5 -> timeout
	# $6 -> token
	# $7 -> ap
	# $8 -> uid
	# ------------------------

	# ------------------------------------------------------------------------------------
	# SAVE REAL UID AND POLICY ON FILE (UUID_TO_LOG POLICY USER_MAX_TIME USER_MAX_TRAFFIC)
	# ------------------------------------------------------------------------------------
	FILE_UID=$DIR_REQ/$8.uid

	if [ -z "$WA_UID" ]; then
		WA_UID=$8
	fi

	write_FILE "$WA_UID $POLICY $MAX_TIME $MAX_TRAFFIC" $FILE_UID
	# --------------------------------------------------------------------

	if [ "$OP" != "ACCOUNT_AUTH" ]; then
		# -------------------------------------------------------------
		# CHECK FOR CHANGE OF CONNECTION CONTEXT FOR SAME USER ID
		# -------------------------------------------------------------
		check_if_user_is_connected "$1" "$2" "$4" "$7" "$8"

		if [ "$OP" = "RENEW" ]; then
			ask_nodog_to_logout_user $IP $MAC
		fi
	fi

	check_if_user_connected_to_AP_NO_CONSUME "$7"

	FILE_CNT=$DIR_CNT/$POLICY/$WA_UID
	# --------------------------------------------------------------------
	# TIME POLICY
	# --------------------------------------------------------------------
	if [ -z "$MAX_TIME" ]; then
		MAX_TIME=0
	fi

	if [ $MAX_TIME -gt 0 ]; then
		# --------------------------------------------------------------------
		# WE CHECK FOR THE TIME REMAIN FOR CONNECTION (SECS) SAVED ON FILE
		# --------------------------------------------------------------------
		REMAIN=$FILE_CNT.timeout

		if [ -s "$REMAIN" ]; then

			read TIMEOUT < $REMAIN 2>/dev/null

			if [ $TIMEOUT -eq 0 -a "$CONSUME_ON" = "true" ]; then
				message_page "Tempo consumato" "Hai consumato il tempo disponibile del servizio!"
			fi
		else
			TIMEOUT=$MAX_TIME
			# ---------------------------------------------------------
			# we save the time remain for connection (secs) on file
			# ---------------------------------------------------------
			write_FILE $TIMEOUT $FILE_CNT.timeout
			# ---------------------------------------------------------
		fi
		# --------------------------------------------------------------------
	fi
	# --------------------------------------------------------------------

	# --------------------------------------------------------------------
	# TRAFFIC POLICY
	# --------------------------------------------------------------------
	if [ -z "$MAX_TRAFFIC" ]; then
		MAX_TRAFFIC=0
	fi

	if [ $MAX_TRAFFIC -gt 0 ]; then
		# --------------------------------------------------------------------
		# WE CHECK FOR THE TRAFFIC REMAIN FOR CONNECTION (BYTES) SAVED ON FILE
		# --------------------------------------------------------------------
		REMAIN=$FILE_CNT.traffic

		if [ -s "$REMAIN" ]; then

			read TRAFFIC < $REMAIN 2>/dev/null

			if [ $TRAFFIC -eq 0 -a "$CONSUME_ON" = "true" ]; then
				message_page "Traffico consumato" "Hai consumato il traffico disponibile del servizio!"
			fi
		else
			TRAFFIC=$MAX_TRAFFIC
			# ---------------------------------------------------------
			# we save the remain traffic for connection (bytes) on file
			# ---------------------------------------------------------
			write_FILE $TRAFFIC $FILE_CNT.traffic
			# ---------------------------------------------------------
		fi
		# --------------------------------------------------------------------
	fi
	# --------------------------------------------------------------------

	if [ -z "$REDIRECT_DEFAULT" ]; then
		REDIRECT_DEFAULT=$3
	fi

	if [ "$POLICY" = "FLAT" -o -z "$CONSUME_ON" ]; then
		TIMEOUT=86400
		TRAFFIC=4294967296
	fi

	sign_data "
Action   Permit
Mode	   Login
Redirect	http://$HTTP_HOST/postlogin?uid=$8&gateway=$4&redirect=$REDIRECT_DEFAULT&ap=$7&ip=$2&mac=$1&timeout=$TIMEOUT&traffic=$TRAFFIC&auth_domain=$OP
Mac		$1
Timeout	$TIMEOUT
Traffic	$TRAFFIC
Token		$6
User		$8"

	write_to_LOG "$8" "$7" "$2" "$1" "$TIMEOUT" "$TRAFFIC"

	test -n "$UUID_TO_APPEND" && {
		if [ "$OP" != "ACCOUNT_AUTH" -o ! -s $DIR_CTX/$UUID.ctx ]; then
			append_to_FILE " $8" $REQ_FILE # NB: si aggiunge UUID alla request...
		fi
	}

	HTTP_RESPONSE_BODY="<html><body>OK</body></html>"
	HTTP_RESPONSE_HEADER="Refresh: 0; url=http://$4/ticket?ticket=$SIGNED_DATA\r\n"
}

user_has_valid_MAC() {

	# ------------------------
	# 1 -> mac
	# 2 -> ip
	# 3 -> redirect
	# 4 -> gateway
	# 5 -> timeout
	# 6 -> token
	# 7 -> ap
	# ------------------------
	# 00:e0:4c:d4:63:f5 10.30.1.105 http://www.google.com 10.30.1.131:5280 stefano 86400 lOosGl9h1aHxo lab2.wpp54
	# ------------------------

	# List of allowed MAC

	FILE=$DIR_ROOT/etc/$VIRTUAL_HOST/.MAC_WHITE_LIST

	if [ -s $FILE ]; then

		while read MAC
		do
			if [ "$MAC" = "$1" ]; then

				# ap is calling for auth, redirect back to the gateway appending a signed ticket that will signal ap to unlock the firewall...

				OP=MAC_AUTH
				POLICY=FLAT

				load_policy

				send_ticket_to_nodog "$@" "$MAC" 

				write_SSI
			fi
		done < $FILE
	fi
}

user_has_valid_cert() {

 	ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_USER_BASEDN $LDAP_USER_PARAM" \
			"(&(objectClass=waUser)(&(waIssuer=$SSL_CLIENT_I_DN)(waSerial=$SSL_CLIENT_CERT_SERIAL)(waActive=TRUE)))"

	if [ -s $TMPFILE.out ]; then

		USER=`cat $TMPFILE.out | grep 'waUid: ' | cut -f2 -d' ' 2>/dev/null`

		if [ -z "$USER" ]; then
			USER=unknow
		fi

		# NoDog is calling for auth, redirect back to the gateway appending a signed ticket that will signal NoDog to unlock the firewall...

		OP=CERT_AUTH
		POLICY=FLAT

		load_policy

		send_ticket_to_nodog "$@" "$USER"

		write_SSI
	fi
}

_date() { date '+%Y/%m/%d %H:%M:%S' ; }

write_to_LOG() {

	# $1 -> uid
	# $2 -> ap
	# $3 -> ip
	# $4 -> mac
	# $5 -> timeout
	# $6 -> traffic

	SPACE=`echo $1 | egrep " "`

	if [ -n "$SPACE" ]; then
		write_ENV

		return
	fi

	# --------------------------------------------------------------------
	# GET REAL UID FROM FILE (UUID_TO_LOG POLICY MAX_TIME MAX_TRAFFIC)
	# --------------------------------------------------------------------
	FILE_UID=$DIR_REQ/$1.uid

	if [ -s "$FILE_UID" ]; then
		read UUID_TO_LOG POLICY USER_MAX_TIME USER_MAX_TRAFFIC < $FILE_UID 2>/dev/null
	fi

	if [ -z "$UUID_TO_LOG" ]; then
		UUID_TO_LOG=$1
	fi
	# --------------------------------------------------------------------

	RIGA="`_date` op: $OP, uid: $UUID_TO_LOG, ap: $2, ip: $3, mac: $4, timeout: $5, traffic: $6 policy: $POLICY"

	append_to_FILE "$RIGA" $FILE_LOG

	sync

	logger -p $REMOTE_SYSLOG_SELECTOR "$PORTAL_NAME: $RIGA"
}

set_ap() {

	# $1 -> ap

	GATEWAY=""
	ACCESS_POINT_NAME=${1##*@}

	ACCESS_POINT=`egrep "$ACCESS_POINT_NAME" $ACCESS_POINT_LIST.down 2>/dev/null`

	if [ -z "$ACCESS_POINT" ]; then

		ACCESS_POINT=`egrep "$ACCESS_POINT_NAME" $ACCESS_POINT_LIST.up 2>/dev/null`

		if [ -n "$ACCESS_POINT" ]; then
			GATEWAY=`echo -n $ACCESS_POINT | cut -d' ' -f2 2>/dev/null`
		fi
	fi
}

send_request_to_nodog() {

	# $1 -> request
	# $2 -> filename to save output
	# $3 -> option

	if [ -n "$2" ]; then
		rm -f "$2"
	fi

	set_ap $AP

	if [ -n "$GATEWAY" ]; then
		# -----------------------------------------------------------------------------
		# we send request to nodog
		# -----------------------------------------------------------------------------
		# NB: we need PREFORK_CHILD > 2
		# -----------------------------------------------------------------------------
		# UTRACE="0 10M 0"
		# UOBJDUMP="0 100k 10"
		# USIMERR="error.sim"
		# export UTRACE UOBJDUMP USIMERR
		# -----------------------------------------------------------------------------

		OUTPUT=`$CLIENT_HTTP $3 http://$GATEWAY/$1 2>>/tmp/CLIENT_HTTP.err`

		if [ $? -ne 0 ]; then

			# si aggiunge access point alla lista di quelli non contattabili...

			ACCESS_POINT=`egrep "$ACCESS_POINT_NAME" $ACCESS_POINT_LIST.down 2>/dev/null`

			if [ -z "$ACCESS_POINT" ]; then
				append_to_FILE "$ACCESS_POINT_NAME $GATEWAY" $ACCESS_POINT_LIST.down
			fi

			anomalia 5
		fi

		if [ -n "$OUTPUT" -a -n "$2" ]; then

			write_FILE "$OUTPUT" $2

			unset OUTPUT
		fi
	fi
}

save_connection_request() {

	# $1 -> data

	# ---------------------------------------------------------------------------------
	# SAVE REQUEST CONTEXT DATA ON FILE (AP GATEWAY MAC IP REDIRECT TIMEOUT TOKEN UUID)
	# ---------------------------------------------------------------------------------
	REQ_FILE=$DIR_REQ/$SESSION_ID.req

	write_FILE "$1" $DIR_REQ/$SESSION_ID.req "-n"
	# ---------------------------------------------------------------------------------
}

save_connection_context() {

	# $1 -> ap
	# $2 -> uid
	# $3 -> gateway
	# $4 -> mac
	# $5 -> ip
	# $6 -> auth_domain

	# -------------------------------------------------------------------------
	# SAVE CONNECTION CONTEXT DATA ON FILE (AP UUID GATEWAY MAC IP AUTH_DOMAIN)
	# -------------------------------------------------------------------------
	FILE_CTX=$DIR_CTX/$2.ctx

	write_FILE "$1 $2 $3 $4 $5 $6" $FILE_CTX
	# -------------------------------------------------------------------------
}

login_with_problem() {

	write_LOG "failure REQ_FILE=$REQ_FILE IP=$IP MAC=$MAC"

	BACK_TAG="<a class=\"back\" href=\"$REDIRECT_DEFAULT\">RIPROVA</a>"

	message_page "Login" "Problema in fase di autenticazione. Si prega di riprovare, se il problema persiste contattare: $TELEFONO"
}

sign_data() {

	SIGNED_DATA=`echo -n -E "$1" | openssl des3 -pass pass:vivalatopa -a -e | tr -d '\n'`
}

ask_nodog_to_logout_user() {

	# we request to logout this user with the old ip from the associated gateway...
	# -----------------------------------------------------------------------------
	# NB: we need PREFORK_CHILD > 2
	# -----------------------------------------------------------------------------
	sign_data "ip=$1&mac=$2"

	send_request_to_nodog "logout?$SIGNED_DATA"
}

info_notified_from_nodog() {

	local LOGOUT=0
	local	TRAFFIC=0
	local	TIMEOUT=0
	local ASK_LOGOUT=0
	local LOGOUT_IMPLICITO=0

	# $1 -> mac
	# $2 -> ip
	# $3 -> gateway
	# $4 -> ap
	# $5 -> uid
	# $6 -> logout
	# $7 -> connected
	# $8 -> traffic

	append_to_FILE "`_date` op: INFO uid: $5, ap: $4, ip: $2, mac: $1, logout: $6, connected: $7, traffic: $8" $FILE_LOG.info

	if [ -z "$5" ]; then
		return
	fi

	OP=INFO

	# NB: FILE_CTX e' settato da get_user_context_connection() che e' chiamato da check_if_user_is_connected()...

	check_if_user_is_connected "$1" "$2" "$3" "$4" "$5"

	if [ -z "$FILE_CTX" -o "$OP" = "RENEW" ]; then

		IP=$2
		AP=$4
		MAC=$1
		GATEWAY=$3

		if [ "$OP" = "RENEW" ]; then
			anomalia 7
		else
			save_connection_context "$4" "$5" "$3" "$1" "$2" "$OP" # save connection context data on file (ap uuid gateway mac ip) to avoid another anomalia...

			# NB: succede che arrivino 2 info su stesso utente di cui la prima e' un logout...
			anomalia 6
		fi
	else
		load_policy $DIR_REQ/$5.uid # GET POLICY FROM FILE (UUID_TO_LOG POLICY USER_MAX_TIME USER_MAX_TRAFFIC)

		FILE_CNT=$DIR_CNT/$POLICY/$5

		check_if_user_connected_to_AP_NO_CONSUME "$4"

		if [ $8 -eq 0 -a $6 -le 0 ]; then # no traffic and no logout => logout implicito

			if [ $LOGOUT_IMPLICITO -eq 1 ]; then

				ASK_LOGOUT=1

				ask_nodog_to_logout_user $IP $MAC

				if [ -n "$GET_USER_INFO_INTERVAL" ]; then
					# --------------------------------------------------------------------
					# WE CHECK FOR THE TIME REMAIN FOR CONNECTION (SECS) SAVED ON FILE
					# --------------------------------------------------------------------
					if [ -s $FILE_CNT.timeout ]; then

						read TIMEOUT < $FILE_CNT.timeout 2>/dev/null

						if [ "$CONSUME_ON" = "true" ]; then
							let "TIMEOUT = TIMEOUT + $GET_USER_INFO_INTERVAL"

							if [ $TIMEOUT -gt $MAX_TIME ]; then
								let "TIMEOUT = TIMEOUT - $GET_USER_INFO_INTERVAL"
							fi
						fi
						# ---------------------------------------------------------
						# we save the time remain for connection (secs) on file
						# ---------------------------------------------------------
						write_FILE $TIMEOUT $FILE_CNT.timeout
					fi
				fi
			fi
		else
			if [ "$CONSUME_ON" = "true" ]; then
				# --------------------------------------------------------------------
				# TRAFFIC POLICY
				# --------------------------------------------------------------------
				if [ -z "$MAX_TRAFFIC" ]; then
					MAX_TRAFFIC=0
				fi

				if [ $MAX_TRAFFIC -gt 0 ]; then
					# --------------------------------------------------------------------
					# WE CHECK FOR THE TRAFFIC REMAIN FOR CONNECTION (BYTES) SAVED ON FILE
					# --------------------------------------------------------------------
					if [ -s $FILE_CNT.traffic ]; then
						read TRAFFIC < $FILE_CNT.traffic 2>/dev/null

						let "TRAFFIC = TRAFFIC - $8"

						if [ $TRAFFIC -lt 0 ]; then
							TRAFFIC=0
						fi
					fi
					# ---------------------------------------------------------
					# we save the remain traffic for connection (bytes) on file
					# ---------------------------------------------------------
					write_FILE $TRAFFIC $FILE_CNT.traffic

					if [ $TRAFFIC -eq 0 -a $ASK_LOGOUT -eq 0 ]; then

						ASK_LOGOUT=1

						ask_nodog_to_logout_user $IP $MAC
					fi
					# ---------------------------------------------------------
				fi
				# ---------------------------------------------------------

				# --------------------------------------------------------------------
				# TIME POLICY
				# --------------------------------------------------------------------
				if [ -z "$MAX_TIME" ]; then
					MAX_TIME=0
				fi

				if [ $MAX_TIME -gt 0 ]; then
					# --------------------------------------------------------------------
					# WE CHECK FOR THE TIME REMAIN FOR CONNECTION (SECS) SAVED ON FILE
					# --------------------------------------------------------------------
					if [ -s $FILE_CNT.timeout ]; then

						read TIMEOUT < $FILE_CNT.timeout 2>/dev/null

						let "TIMEOUT = TIMEOUT - $7"

						if [ -n "$BONUS_FOR_EXIT" -a $6 -eq -1 ]; then # disconneted (logout implicito)
							let "TIMEOUT = TIMEOUT + $BONUS_FOR_EXIT"
						fi

						if [ $TIMEOUT -lt 0 ]; then
							TIMEOUT=0
						elif [ $TIMEOUT -gt $MAX_TIME ]; then
							let "TIMEOUT = TIMEOUT - $BONUS_FOR_EXIT"
						fi
					fi
					# ---------------------------------------------------------
					# we save the time remain for connection (secs) on file
					# ---------------------------------------------------------
					write_FILE $TIMEOUT $FILE_CNT.timeout

					if [ $TIMEOUT -eq 0 -a $ASK_LOGOUT -eq 0 ]; then

						ASK_LOGOUT=1

						ask_nodog_to_logout_user $IP $MAC
					fi
					# ---------------------------------------------------------
				fi
				# ---------------------------------------------------------
			fi
		fi

		if [ $6 -ne 0 ]; then # logout

			LOGOUT=1

			if [ $6 -eq -1 ]; then # -1 => disconnected (logout implicito)
				OP=EXIT
			fi
		fi
	fi

	HTTP_RESPONSE_HEADER="Connection: close\r\n"

	if [ $LOGOUT -eq 0 ]; then
		HTTP_RESPONSE_BODY="OK"
	else
		OP=LOGOUT
		HTTP_RESPONSE_BODY="LOGOUT"

		write_to_LOG "$5" "$4" "$2" "$1" "$TIMEOUT" "$TRAFFIC"

		rm -f $FILE_CTX $DIR_REQ/$2.req $DIR_REQ/$5.uid # we remove the data saved on file (connection context data and NoDog data)
	fi

	if [ $# -gt 8 ]; then

		shift 8

		info_notified_from_nodog "$@"
	fi
}

ask_nodog_to_check_for_users_info() {

	# we request nodog to check for users logout or disconnect...
	# -----------------------------------------------------------------------------
	# NB: we need PREFORK_CHILD > 2
	# -----------------------------------------------------------------------------
	send_request_to_nodog "check" $TMPFILE "-i"

	if [ -s "$TMPFILE" ]; then

		read HTTP_VERSION HTTP_STATUS HTTP_DESCR < $TMPFILE 2>/dev/null

		if [ "$HTTP_STATUS" = "204" ]; then # 204 - HTTP_NO_CONTENT

			sleep 17

			ask_nodog_to_check_for_users_info
		fi

		rm -f "$TMPFILE"
	fi
}

read_connection_request() {

	# ------------------------------------------------------------------------------------------------------------------------------------------------
	# nodog data saved on file
	# ------------------------------------------------------------------------------------------------------------------------------------------------
	# stefano 10.30.1.131:5280 00:e0:4c:d4:63:f5 10.30.1.105 http://www.google.com 0 10.30.1.105&1257603166&2a2436611f452f8eebddce4992e88f8d 055340773
	# ------------------------------------------------------------------------------------------------------------------------------------------------

	unset	  AP GATEWAY MAC IP REDIRECT TIMEOUT TOKEN UUID
	if [ -s $REQ_FILE ]; then
		read AP GATEWAY MAC IP REDIRECT TIMEOUT TOKEN UUID < $REQ_FILE 2>/dev/null
	fi
}

logout_user() {

	REQ_FILE=$DIR_REQ/$SESSION_ID.req

	read_connection_request

	if [ -z $AP ]; then

		write_LOG "failure REQ_FILE=$REQ_FILE"

		unset BACK_TAG

		message_page "ID di sessione mancante" "Utente non connesso (session id: $SESSION_ID)"
	fi

	if [ -z "$UUID" -o ! -s $DIR_CTX/$UUID.ctx ]; then

		unset BACK_TAG

		message_page "Utente non connesso" "Utente non connesso"
	fi

	# ------------------------------------------------------------------------
	# GET POLICY FROM FILE (UUID_TO_LOG POLICY USER_MAX_TIME USER_MAX_TRAFFIC)
	# ------------------------------------------------------------------------
	load_policy $DIR_REQ/$UUID.uid

	ask_nodog_to_logout_user $IP $MAC
}

read_counter() {

   # $1 -> uuid

   if [ -s "$DIR_CNT/$POLICY/$1.timeout" ]; then

      REMAINING_TIME=`cat $DIR_CNT/$POLICY/$1.timeout`

      # expressing the time in minutes
      REMAINING_TIME_MIN=`expr $REMAINING_TIME / 60`
   else
      REMAINING_TIME_MIN="Non disponibile"
   fi

   if [ -s "$DIR_CNT/$POLICY/$1.traffic" ]; then

      REMAINING_TRAFFIC=`cat $DIR_CNT/$POLICY/$1.traffic`

      # expressing the traffic in MB 1024*1024=1048576
      REMAINING_TRAFFIC_MB=`expr $REMAINING_TRAFFIC / 1048576`
   else
      REMAINING_TRAFFIC_MB="Non disponibile"
   fi
}

logout_request() {

	logout_user

	read_counter $UUID

	REQUEST_URI=ringraziamenti

	print_page $UUID "$REMAINING_TIME_MIN" "$REMAINING_TRAFFIC_MB"
}

recovery_user() {

	# $1 -> uid

	get_user_context_connection "$1" ""

	if [ -n "$AP" ]; then
		ask_nodog_to_logout_user $IP $MAC
	fi

	rm -f $DIR_CTX/$1.ctx $DIR_REG/$1.reg $DIR_CNT/$POLICY/$1.*

	write_LOG "User <$1> recovered"

	HTTP_RESPONSE_BODY="OK"
	HTTP_RESPONSE_HEADER="Connection: close\r\n"
}

get_timeout_secs() {

	DSTART=`printf "%4s-%2s-%2s %2s:%2s:%2s" ${1:0:4} ${1:4:2} ${1:6:2} ${1:8:2} ${1:10:2} ${1:12:2} 2>/dev/null`
	  DEND=`printf "%4s-%2s-%2s %2s:%2s:%2s" ${2:0:4} ${2:4:2} ${2:6:2} ${2:8:2} ${2:10:2} ${2:12:2} 2>/dev/null`

	START=`date --date="$DSTART" +%s 2>/dev/null`
	  END=`date --date="$DEND"   +%s 2>/dev/null`

	let "TIMEOUT = $END - $START"
}

login_request() {

	if [ "$REQUEST_METHOD" = "GET" ]; then

		# GET
		# ------------
		# $1 -> mac
		# $2 -> ip
		# $3 -> redirect
		# $4 -> gateway
		# $5 -> timeout
		# $6 -> token
		# $7 -> ap

      print_page "$LOGIN_URL" \
                 "$1" "$2" "$3" "$4" "$5" "$6" "$7"

		return
	fi

	# POST
	# ------------
	# $8  -> realm (10_piazze, paas, ...)
	# $9  -> uid
	# $10 -> password
	# $11 -> bottone

	if [ "$8" != "10_piazze" -a "$8" != "auth_service" ]; then

		unset BACK_TAG

		message_page "Errore" "Errore Autorizzazione - dominio sconosciuto: $8"
	fi

	if [ -z "$9" -o \
		  -z "${10}" ]; then

		unset BACK_TAG

		message_page "Impostare utente e/o password" "Impostare utente e/o password"
	fi

	# Check 1: Wrong user and/or password

	ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_CARD_BASEDN $LDAP_CARD_PARAM" "waLogin=$9"

	if [ -s $TMPFILE.out ]; then
		PASSWORD=`awk '/^waPassword/{print $2}' $TMPFILE.out 2>/dev/null`
	fi

	if [ -n "$PASSWORD" -a "${PASSWORD:0:5}" = "{MD5}" ]; then

		  MD5SUM=`/usr/sbin/slappasswd -h {MD5} -s "${10}"`

		  if [ "$PASSWORD" != "$MD5SUM" ]; then

				unset BACK_TAG

				message_page "Utente e/o Password errato/i" "Credenziali errate!"
		  fi

		OP=PASS_AUTH
	else

		if [ "$8" != "auth_service" ]; then

			unset BACK_TAG

			message_page "Utente e/o Password errato/i" "Credenziali errate!"
		fi

		# $9  -> uid
		# $10 -> password

		AUTH_CMD=`printf "$FMT_AUTH_CMD" "$9" "${10}" 2>/dev/null`

		RESPONSE=`$AUTH_CMD 2>/dev/null`
		EXIT_VALUE=$?

		if [ $EXIT_VALUE -eq 1 ]; then

			unset BACK_TAG

			message_page "Utente e/o Password errato/i" "Credenziali errate!"
		fi

		if [ $EXIT_VALUE -ne 0 -o -z "$RESPONSE" ]; then

			write_LOG "AUTH_CMD failure EXIT_VALUE=$EXIT_VALUE RESPONSE=$RESPONSE"

			unset BACK_TAG

			message_page "Errore" "Esito comando richiesta autorizzazione: EXIT_VALUE=$EXIT_VALUE RESPONSE=$RESPONSE"
		fi

		POLICY=DAILY

		OP=AUTH_$RESPONSE
	fi

	if [ "$OP" = "PASS_AUTH" ]; then
		# --------------------------------------------------------------------
		# GET USER FOR THIS CARD
		# --------------------------------------------------------------------
		WA_UID=`grep 'waUsedBy: ' $TMPFILE.out | cut -f2 -d' ' 2>/dev/null`

		# Check 2: Activation required

		if [ -z "$WA_UID" ]; then

			unset BACK_TAG

			message_page "Attivazione non effettuata" "Per utilizzare il servizio e' richiesta l'attivazione"
		fi
		# --------------------------------------------------------------------

		# Check 3: Card revoked

		REVOKED=`grep 'waRevoked: ' $TMPFILE.out | cut -f2 -d' ' 2>/dev/null`

		if [ "$REVOKED" != "FALSE" ]; then

			unset BACK_TAG

			message_page "Carta revocata" "La tua carta e' revocata!"
		fi

		# --------------------------------------------------------------------
		# GET POLICY FOR THIS CARD
		# --------------------------------------------------------------------
		WA_POLICY=`grep 'waPolicy: ' $TMPFILE.out | cut -f2 -d' ' 2>/dev/null`

		if [ -n "$WA_POLICY" ]; then
			POLICY=$WA_POLICY
		fi
	fi

	load_policy

	if [ "$OP" = "PASS_AUTH" ]; then

		# Check 4: Not After

		NOT_AFTER=`grep 'waNotAfter: ' $TMPFILE.out | cut -f2 -d' ' 2>/dev/null`

		if [ -n "$NOT_AFTER" ]; then

			# Check 5: Expired validity

			GEN_TIME=`date +%Y%m%d%H%M%SZ` # GeneralizedTime YYYYmmddHH[MM[SS]][(./,)d...](Z|(+/-)HH[MM])

			get_timeout_secs "$GEN_TIME" "$NOT_AFTER"

			if [ $TIMEOUT -lt 0 ]; then

				unset BACK_TAG

				message_page "Validita' scaduta" "La tua validita' e' scaduta!"
			fi

		else

			OP=FIRST_PASS_AUTH

			# --------------------------------------------------------------------
			# waTime - valore di *** CONSUMO ***
			# --------------------------------------------------------------------
			if [ "$POLICY" != "FLAT" ]; then

				WA_TIME=`grep 'waTime: ' $TMPFILE.out | cut -f2 -d' ' 2>/dev/null`

				if [ -n "$WA_TIME" ]; then
					MAX_TIME=$WA_TIME
				fi
			fi
			# --------------------------------------------------------------------
			# waTraffic - valore di *** CONSUMO ***
			# --------------------------------------------------------------------
			if [ "$POLICY" != "FLAT" ]; then

				WA_TRAFFIC=`grep 'waTraffic: ' $TMPFILE.out | cut -f2 -d' ' 2>/dev/null`

				if [ -n "$WA_TRAFFIC" ]; then
					MAX_TRAFFIC=$WA_TRAFFIC
				fi
			fi
			# --------------------------------------------------------------------

			# Update card with a new generated waNotAfter

			DN=`grep 'dn: '					$TMPFILE.out | cut -f2 -d' ' 2>/dev/null`
			VALIDITY=`grep 'waValidity: ' $TMPFILE.out | cut -f2 -d' ' 2>/dev/null`

			if [ $VALIDITY -eq 0 ]; then
				NOT_AFTER=20371231235959Z
			else
				let "TIMEOUT = $VALIDITY * 86400"

				NOT_AFTER=`date --date="+$VALIDITY days" +%Y%m%d%H%M%SZ 2>/dev/null`
			fi

			ask_to_LDAP ldapmodify "-c $LDAP_CARD_PARAM" "
dn: $DN
changetype: modify
add: waNotAfter
waNotAfter: $NOT_AFTER
-
"
		fi
	fi

	is_group_ACCOUNT "$9" "${10}"

	if [ $? -eq 0 ]; then
		POLICY=FLAT

		OP=ACCOUNT_AUTH
	fi

	LOGIN_VALIDATE=0

	REQ_FILE=$DIR_REQ/$SESSION_ID.req # nodog data saved on file

	read_connection_request

	if [ "$7" != "$AP" ]; then
		LOGIN_VALIDATE=1
	elif [ "$4" != "$GATEWAY" ]; then
		LOGIN_VALIDATE=1
	elif [ "$1" != "$MAC" ]; then
		LOGIN_VALIDATE=1
	elif [ "$2" != "$IP" ]; then
		LOGIN_VALIDATE=1
	elif [ "$5" != "$TIMEOUT" ]; then
		LOGIN_VALIDATE=1
	elif [ "$3" != "$REDIRECT" ]; then
		LOGIN_VALIDATE=1
	elif [ "$6" != "$TOKEN" ]; then
		LOGIN_VALIDATE=1
	fi

	if [ $LOGIN_VALIDATE -eq 0 ]; then
		# ------------------------------------------------------------------------------------------------------------------------------------------------
		# redirect back to the gateway appending a signed ticket that will signal NoDog to unlock the firewall...
		# ------------------------------------------------------------------------------------------------------------------------------------------------
		# $1 -> mac
		# $2 -> ip
		# $3 -> redirect
		# $4 -> gateway
		# $5 -> timeout
		# $6 -> token
		# $7 -> ap
		# $8 -> uid
		# ------------------------------------------------------------------------------------------------------------------------------------------------
		UUID_TO_APPEND=1

		send_ticket_to_nodog "$MAC" $IP "$REDIRECT" $GATEWAY $TIMEOUT "$TOKEN" $AP "$9"
	else
		# ------------------------------------------------------------------------------------
		# SAVE DATA ON FILE
		# ------------------------------------------------------------------------------------
		FILE_UID=$DIR_REQ/$9.uid

		write_FILE "$OP $POLICY $MAX_TIME $MAX_TRAFFIC" $FILE_UID
		# --------------------------------------------------------------------

		sign_data "uid=$9"

		HTTP_RESPONSE_BODY="<html><body>OK</body></html>"
		HTTP_RESPONSE_HEADER="Refresh: 0; url=http://www.google.com/login_validate?$SIGNED_DATA\r\n"
	fi

	write_SSI
}

login_validate() {

	# 1 -> mac
	# 2 -> ip
	# 3 -> uid
	# 4 -> gateway
	# 5 -> timeout
	# 6 -> token
	# 7 -> ap

	# ----------------------
	# GET DATA FROM FILE
	# ----------------------
	FILE_UID=$DIR_REQ/$3.uid

	if [ -s "$FILE_UID" ]; then
		read OP POLICY MAX_TIME MAX_TRAFFIC < $FILE_UID 2>/dev/null
	else
		login_with_problem
	fi

	if [ "$OP" != "ACCOUNT_AUTH" ]; then

		FILE_CTX=$DIR_CTX/$3

		if [ -s $FILE_CTX ]; then

			unset BACK_TAG

			message_page "Login" "Sei già loggato! (login_request)"
		fi
	fi

	load_policy

	PARAM="$7 $4 $1 $2 http://www.google.com $5 $6 $3"
	# ------------------------------------------------------------------------------------------------------------------------------------------------
	# SAVE REQUEST CONTEXT DATA ON FILE (AP GATEWAY MAC IP REDIRECT TIMEOUT TOKEN UUID)
	# ------------------------------------------------------------------------------------------------------------------------------------------------
	# 1 -> ap
	# 2 -> gateway
	# 3 -> mac
	# 4 -> ip
	# 5 -> redirect
	# 6 -> timeout
	# 7 -> token
	# 8 -> uuid
	# ------------------------------------------------------------------------------------------------------------------------------------------------
	# 10.30.1.131:5280 00:e0:4c:d4:63:f5 10.30.1.105 http://www.google.com 0 10.30.1.105&1257603166&2a2436611f452f8eebddce4992e88f8d 055340773
	# ------------------------------------------------------------------------------------------------------------------------------------------------
	save_connection_request "$PARAM" # save nodog data on file

	# ------------------------------------------------------------------------------------------------------------------------------------------------
	# redirect back to the gateway appending a signed ticket that will signal NoDog to unlock the firewall...
	# ------------------------------------------------------------------------------------------------------------------------------------------------
	# $1 -> mac
	# $2 -> ip
	# $3 -> redirect
	# $4 -> gateway
	# $5 -> timeout
	# $6 -> token
	# $7 -> ap
	# $8 -> uid
	# ------------------------------------------------------------------------------------------------------------------------------------------------

	send_ticket_to_nodog "$1" "$2" "http://www.google.com" "$4" "$5" "$6" "$7" "$3"

	write_SSI
}

postlogin() {

	if [ $# -eq 9 ]; then

		unset BACK_TAG

		# $1 -> uid
		# $2 -> gateway
		# $3 -> redirect
		# $4 -> ap
		# $5 -> ip
		# $6 -> mac
		# $7 -> timeout
		# $8 -> traffic
		# $9 -> auth_domain

		FILE_CTX=$DIR_CTX/$1.ctx

		test -s "$FILE_CTX" && ! is_group_ACCOUNT "$1" "" && {
			message_page "PostLogin" "Sei già loggato! (postlogin)"
		}

		REQ_FILE=$DIR_REQ/$SESSION_ID.req

		read_connection_request

		if [ -z "$UUID" ]; then
			login_with_problem
		fi

		OP=LOGIN

		write_to_LOG "$1" "$4" "$5" "$6" "$7" "$8"

		# --------------------------------------------------------------------
		# SAVE CONNECTION CONTEXT DATA ON FILE (AP UUID GATEWAY MAC IP)
		# --------------------------------------------------------------------
		save_connection_context "$4" "$1" "$2" "$6" "$5" "$9"
		# --------------------------------------------------------------------

		CONNECTION_CLOSE=1

 		SSI_HEAD="<script type=\"text/javascript\" src=\"js/logout_popup.js\"></script>"
		BODY_STYLE=`printf "onload=\"doOnLoad('postlogin?uid=%s&gateway=%s','%s')\"" "$1" "$2" "$3" 2>/dev/null`

		print_page "$1" "$3" "$3"

	elif [ $# -eq 2 ]; then

		# $1 -> uid
		# $2 -> gateway

		TITLE_TXT="Logout popup"
		REQUEST_URI=logout_popup

		CONNECTION_CLOSE=1

		print_page "$1" "$1"
	fi
}

logout_notified_from_popup() {

	logout_user

	CONNECTION_CLOSE=1

	SSI_HEAD="<script type=\"text/javascript\" src=\"js/logout_popup.js\"></script>"
	BODY_STYLE='onload="CloseItAfterSomeTime()"'

	REQUEST_URI=logout_notify

	print_page "$1"
}

save_value_session() {

	cat <<END >$TMP_FORM_FILE
$1
$2
$3
$4
$5
$6
$7
$8
$9
${10}
${11}
${12}
${13}
${14}
${15}
${16}
${17}
END
#${18}
#${19}
#${20}
#${21}
#${22}
#${23}
#${24}
}

check_phone_number() {

	# -----------------------------------------
	# $1 -> CALLER_ID
	# -----------------------------------------

	# Check for italian prefix

	CALLER_ID=`echo -n "$1" | \
		awk 'BEGIN {phoneNumber=""}
		{

		if (match($1,"^+") == 0 && match($1,"^00") == 0) {
			phoneNumber="+39"$1	
		} else {
			phoneNumber=$1
		}

		if (match(phoneNumber,"^[0-9]+$") == 0 && match(phoneNumber,"^+[0-9]+$") == 0) {
			print phoneNumber": ""Invalid phone number"; exit (1);
		}

		if (match(phoneNumber,"^+39") == 0 && match(phoneNumber,"^0039") == 0) {
			print phoneNumber": ""Not italian number"; exit (1);
		}

		if (match(phoneNumber,"^[0-9]+$")) {
			phoneNumber=substr(phoneNumber,5);
		}

		if (match(phoneNumber,"^+[0-9]+$")) {
			phoneNumber=substr(phoneNumber,4);
		}

		if (length(phoneNumber) > 10) {
			prefix=substr(phoneNumber,1,3)
			first=substr(phoneNumber,4,3)

			if (match(first,prefix) == 1) {
				print substr(phoneNumber,4)": ""Not repeat the prefix number please"; exit (1);
			}
		}

		print phoneNumber;

		}' 2>/dev/null`

	EXIT_VALUE=$?
}

gen_activation() {

   # -----------------------------------------
   # $1 -> CALLER_ID
   # -----------------------------------------

   check_phone_number "$1"

   if [ $EXIT_VALUE -ne 0 ]; then

      write_LOG "$CALLER_ID not valid!"

      uscita
   fi

   # Search card by pin

   ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_CARD_BASEDN $LDAP_CARD_PARAM" "waPin=$CALLER_ID"

   if [ ! -s $TMPFILE.out ]; then

      write_LOG "Utente $CALLER_ID non registrato!"

		EXIT_VALUE=400

      uscita
   fi

	WA_CID=`awk '/^waCid/{print $2}'	$TMPFILE.out 2>/dev/null`

	# check for what type of operation...

	TAG=""
	PWDFILE=$DIR_REG/$CALLER_ID.pwd
	PASSWORD=`cat $PWDFILE 2>/dev/null`

   if [ -n "$PASSWORD" ]; then

		TAG="(MOD)"

		# Update card with a new PASSWORD

		ask_to_LDAP ldapmodify "-c $LDAP_CARD_PARAM" "
dn: waCid=$WA_CID,$WIAUTH_CARD_BASEDN
changetype: modify
replace: waPassword
waPassword: $PASSWORD
-
"
	else
		# Verify the card is already activated

		WA_USEDBY=`awk '/^waUsedBy/{print $2}' $TMPFILE.out 2>/dev/null`

		if [ -n "$WA_USEDBY" ]; then

			write_LOG "Utente $CALLER_ID già attivato!"

			EXIT_VALUE=400

			uscita
		fi

		# Update card

		ask_to_LDAP ldapmodify "-c $LDAP_CARD_PARAM" "
dn: waCid=$WA_CID,$WIAUTH_CARD_BASEDN
changetype: modify
add: waUsedBy
waUsedBy: $CALLER_ID
-
"
	fi

	if [ $EXIT_VALUE -ne 0 ]; then

		write_LOG "Update card failed!"

		uscita
	fi

   if [ -n "$PASSWORD" ]; then
		rm -f $PWDFILE
	fi

	write_LOG "Login    <$CALLER_ID>"

	ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_CARD_BASEDN $LDAP_CARD_PARAM" "waLogin=$CALLER_ID"

	PASSWORD=`awk '/^waPassword/{print $2}' $TMPFILE.out 2>/dev/null`

	write_LOG "Password <$PASSWORD> $TAG"

	HTTP_RESPONSE_BODY="OK"
	HTTP_RESPONSE_HEADER="Connection: close\r\n"
}

polling_password() {

	# $1 WA_CELL
	# $2 password

	INFO=""
	LOGIN_FORM=""
	TITLE="VERIFICA MODIFICA PASSWORD: ATTENDERE..."
	SSI_HEAD="<meta http-equiv=\"refresh\" content=\"3\">"
	CREDENTIALS_TAG="<p class=\"bigger\">&nbsp;</p><p class=\"bigger\">&nbsp;</p>"

	# ldapsearch -LLL -b ou=cards,o=unwired-portal -x -D cn=admin,o=unwired-portal -w programmer -H ldap://10.30.1.131

	ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_CARD_BASEDN $LDAP_CARD_PARAM" "waUsedBy=$1" waPassword

	if [ -s $TMPFILE.out ]; then
		PASSWORD=`awk '/^waPassword/{print $2}' $TMPFILE.out 2>/dev/null`
	fi

	if [ -n "$PASSWORD" -a "${PASSWORD:0:5}" = "{MD5}" ]; then

		  MD5SUM=`/usr/sbin/slappasswd -h {MD5} -s "${2}"`

		  if [ "$PASSWORD" = "$MD5SUM" ]; then
				ATTIVATO=yes
		  fi
	fi

	if [ "$ATTIVATO" = "yes" ]; then

		INFO="
<p>Ti suggeriamo di prendere nota: </p>
<ul>
	<li>delle credenziali che ti serviranno ogni volta che vorrai accedere al servizio</li>
	<li><!--#echo var=\"$LOGOUT_NOTE\"--></li>
	<br/>
	<!--#echo var=\"$LOGOUT_HTML\"-->
</ul>
<br/>
<p class=\"bigger\">Ora puoi accedere al servizio cliccando il bottone</p>
"
	# $1	-> mac
	# $2  -> ip
	# $3	-> redirect
	# $4	-> gateway
	# $5	-> timeout
	# $6	-> token
	# $7	-> ap
	# $8  -> realm (all, firenzecard, ...)
	# $9  -> uid
	# $10 -> password
	# $11 -> bottone

		LOGIN_FORM="
<form action=\"/login_request\" method=\"post\">
<input type=\"hidden\" name=\"mac\" value=\"\">
<input type=\"hidden\" name=\"ip\" value=\"\">
<input type=\"hidden\" name=\"redirect\" value=\"\">
<input type=\"hidden\" name=\"gateway\" value=\"\">
<input type=\"hidden\" name=\"timeout\" value=\"\">
<input type=\"hidden\" name=\"token\" value=\"\">
<input type=\"hidden\" name=\"ap\" value=\"\">
<input type=\"hidden\" name=\"realm\" value=\"all\" />
<input type=\"hidden\" name=\"uid\" value=\"$1\">
<input type=\"hidden\" name=\"pass\" value=\"$2\">
<input type=\"image\" src=\"images/accedi.png\" name=\"submit\" value=\"Entra\" />
</form>"

		SSI_HEAD=""
		TITLE="LE TUE CREDENZIALI SONO:"
		CREDENTIALS_TAG="<p class=\"bigger\">Utente: $1</p><p class=\"bigger\">Password: $2</p>"
	fi

	TITLE_TXT="Verifica modifica password"

	print_page "$TITLE" "$CREDENTIALS_TAG" "$INFO" "$LOGIN_FORM"
}

password_request() {

	# $1 -> cellulare_prefisso
	# $2 -> telefono_cellulare
	# $3 -> password
	# $4 -> password_conferma
	# $5 -> submit

	if [ "$3" != "$4" ]; then
		message_page "Conferma Password errata" "Conferma Password errata"
	fi

	RESULT=`echo $3 | egrep "^.{6,255}"`

	# if the result string is empty, one of the conditions has failed

	if [ -z "$RESULT" ]; then
		message_page "Password non conforme" "Password non conforme: deve essere di almeno 6 caratteri"
	fi

	check_phone_number "${1}${2}" # numero cellulare

	if [ $EXIT_VALUE -ne 0 ]; then
		message_page "$CALLER_ID - check fallito" "$CALLER_ID - check fallito"
	fi

	REQUEST_URI=post_password
	PWDFILE=$DIR_REG/$CALLER_ID.pwd
	TITLE_TXT="Modifica password effettuata"

	/usr/sbin/slappasswd -h {MD5} -s "${3}" > $PWDFILE

	if [ ! -s $PWDFILE ]; then
		message_page "$CALLER_ID - creazione file fallita" "$CALLER_ID - creazione file fallita"
	fi

	print_page "$TELEFONO_REGISTRAZIONE" $CALLER_ID "$3" "polling_password" $CALLER_ID "$3"
}

registrazione_request() {

	if [ "$REQUEST_METHOD" = "GET" ]; then

		# $1 -> ap

		CONNECTION_CLOSE=1
		TITLE_TXT="Registrazione utente"
		SSI_HEAD="<link type=\"text/css\" href=\"css/livevalidation.css\" rel=\"stylesheet\">
					  <script type=\"text/javascript\" src=\"js/livevalidation_standalone.compressed.js\"></script>"

		print_page $REGISTRAZIONE_URL "`cat $DIR_TEMPLATE/tutela_dati.txt`"

		return
	fi

	if [ -n "$generate_PASSWORD" ]; then

		PASSWORD=`apg -a 1 -M n -n 1 -m 6 -x 6` # generate PASSWORD

		check_phone_number "${15}${16}" # numero cellulare
	else

		# $1  -> nome
		# $2  -> cognome
		# $3  -> luogo_di_nascita
		# $4  -> data_di_nascita
		# $5  -> email
		# $6  -> cellulare_prefisso
		# $7  -> telefono_cellulare
		# $8  -> password
		# $9  -> password_conferma
		# $10 -> submit

		if [ "$8" != "$9" ]; then
			message_page "Conferma Password errata" "Conferma Password errata"
		fi

		RESULT=`echo $8 | egrep "^.{6,255}"`

		# if the result string is empty, one of the conditions has failed

		if [ -z "$RESULT" ]; then
			message_page "Password non conforme" "Password non conforme: deve essere di almeno 6 caratteri"
		fi

		PASSWORD=`/usr/sbin/slappasswd -h {MD5} -s "$8"`

		check_phone_number "${6}${7}" # numero cellulare
	fi

	if [ $EXIT_VALUE -ne 0 ]; then
		message_page "$CALLER_ID - check fallito" "$CALLER_ID - check fallito"
	fi

	TMP_FORM_FILE=$DIR_REG/$CALLER_ID.reg

	ask_to_LDAP ldapadd "$LDAP_USER_PARAM" "
dn: waUid=$CALLER_ID,$WIAUTH_USER_BASEDN
objectClass: top
objectClass: waUser
waActive: TRUE
waUid: $CALLER_ID
waCell: $CALLER_ID
"

	if [ $EXIT_VALUE -eq 68 ]; then
		message_page "Utente già registrato" "Utente già registrato"
	fi

	UUID=`uuidgen 2>/dev/null`

	WA_CARDID=${UUID:24:12}

	# Update card with a new LOGIN/PASSWORD

	load_policy

	ask_to_LDAP ldapadd "$LDAP_CARD_PARAM" "
dn: waCid=$UUID,$WIAUTH_CARD_BASEDN
objectClass: top
objectClass: waCard
waCid: $UUID
waPin: $CALLER_ID
waCardId: $WA_CARDID
waLogin: $CALLER_ID
waPassword: $PASSWORD
waRevoked: FALSE
waValidity: $REG_VALIDITY
waPolicy: $POLICY
waTime: $MAX_TIME
waTraffic: $MAX_TRAFFIC
"

	if [ $EXIT_VALUE -eq 68 ]; then
		message_page "Utente già registrato" "Utente già registrato (ldap branch card)"
	fi

	save_value_session "$@"

	rm -rf $DIR_REG/$CALLER_ID.pwd

	REQUEST_URI=post_registrazione
	TITLE_TXT="Registrazione effettuata"

	print_page "$TELEFONO_REGISTRAZIONE" $CALLER_ID "$8" "polling_attivazione" $CALLER_ID "$8"
}

stato_utente() {

	# $1 -> mac

	get_user_context_connection "" "$1"

	if [ -z "$GATEWAY" ]; then
		message_page "Utente non connesso" "Utente non connesso"
	else
		get_user_nome_cognome $UUID

		# we request the status of the indicated user...
		# -----------------------------------------------------------------------------
		# NB: we need PREFORK_CHILD > 2
		# -----------------------------------------------------------------------------
		send_request_to_nodog "status?ip=$REMOTE_ADDR"

		TITLE_TXT="Stato utente"

		FMT=`cat $DIR_TEMPLATE/stato_utente.tmpl 2>/dev/null`
		DATE=`date`

		SSI_BODY=`printf "$FMT" "$USER" $UUID $AP "$OUTPUT" 2>/dev/null`
	fi
}

update_ap_list() {

	# $1 -> ap
	# $2 -> public address to contact the access point

	ACCESS_POINT_NAME=${1##*@}

	ACCESS_POINT=`egrep "$ACCESS_POINT_NAME" $ACCESS_POINT_LIST.up 2>/dev/null`

	if [ -z "$ACCESS_POINT" ]; then

		# si controlla che non ci sia un access point con lo stesso ip...
		ACCESS_POINT=`egrep " $2" $ACCESS_POINT_LIST.up 2>/dev/null`

		if [ -z "$ACCESS_POINT" ]; then
			# si aggiunge access point alla lista di quelli contattabili...
			append_to_FILE "$ACCESS_POINT_NAME $2" $ACCESS_POINT_LIST.up
		fi
	fi

	ACCESS_POINT=`egrep "$ACCESS_POINT_NAME" $ACCESS_POINT_LIST.down 2>/dev/null`

	if [ -n "$ACCESS_POINT" ]; then

		LIST=`egrep -v "$ACCESS_POINT_NAME" $ACCESS_POINT_LIST.down 2>/dev/null`

		if [ -n "$LIST" ]; then

			# si toglie access point dalla lista di quelli non contattabili...

			write_FILE "$LIST" $ACCESS_POINT_LIST.down
		else
			rm -f					 $ACCESS_POINT_LIST.down
		fi
	fi
}

quit_user_logged() {

	# $1 -> ap

	# ------------------------------------------------------------------------------------------------------------------------------------------------
	# *.req (AP GATEWAY MAC IP REDIRECT TIMEOUT TOKEN UUID)
	# ------------------------------------------------------------------------------------------------------------------------------------------------
	# stefano 10.30.1.131:5280 00:e0:4c:d4:63:f5 10.30.1.105 http://www.google.com 0 10.30.1.105&1257603166&2a2436611f452f8eebddce4992e88f8d 055340773
	# ------------------------------------------------------------------------------------------------------------------------------------------------
	# *.cxt (AP UUID GATEWAY MAC IP)
	# ------------------------------------------------------------------------------------------------------------------------------------------------
	# stefano 055340773 10.30.1.131:5280 00:e0:4c:d4:63:f5 10.30.1.105
	# ------------------------------------------------------------------------------------------------------------------------------------------------

	LIST=`egrep -l "$1 " $DIR_REQ/*.req $DIR_CTX/*.ctx 2>/dev/null`

	if [ -n "$LIST" ]; then

		OP=QUIT
		LIST_SAFE=""

		for FILE in $LIST
		do
			unset GATEWAY

			SUFFIX="${FILE##*.}"

			if [ "$SUFFIX" = "req" ]; then
				read AP GATEWAY MAC IP REDIRECT TIMEOUT TOKEN UUID < $FILE 2>/dev/null
			elif [ "$SUFFIX" = "ctx" ]; then
				read AP UUID GATEWAY MAC IP AUTH_DOMAIN < $FILE 2>/dev/null
			fi

			ACCESS_POINT_NAME=${AP##*@}

			if [ "$ACCESS_POINT_NAME" = "$1" ]; then

				LIST_SAFE="$FILE $LIST_SAFE"

				if [ "$SUFFIX" = "ctx" ]; then
					write_to_LOG "$UUID" "$AP" "$IP" "$MAC" "$TIMEOUT" "$TRAFFIC"
				fi
			fi
		done

		rm -f $LIST_SAFE
	fi
}

start_ap() {

	# for safety...
	mkdir -p $DIR_POLICY $DIR_AP $DIR_CTX $DIR_CNT $DIR_REQ $DIR_CLIENT $DIR_REG $DIR_TEMPLATE $HISTORICAL_LOG_DIR

	# $1 -> ap
	# $2 -> public address to contact the access point
	# $3 -> pid (0 => start)

	quit_user_logged "$1"

	update_ap_list "$1" "$2"

	INFO=start

	if [ "$3" != "0" ]; then
		INFO=CRASH
	fi

	write_LOG "$1 $INFO"

	HTTP_RESPONSE_BODY=`grep -svE '^[ \t]*(#|$)' $DIR_ROOT/etc/AllowedWebHosts.txt | sed 's/#.*//' | tr '\n' ' '`
	HTTP_RESPONSE_HEADER="Connection: close\r\n"
}

error_ap() {

	# $1 -> ap
	# $2 -> public address to contact the access point

	update_ap_list "$1" "$2"

	write_LOG "$1 FIREWALL NOT ALIGNED"

	HTTP_RESPONSE_BODY=OK
	HTTP_RESPONSE_HEADER="Connection: close\r\n"
}

get_users_info() {

	# $1 -> ap
	# $2 -> gateway

	if [ -n "$1" -a -n "$2" ]; then

		AP=$1
		GATEWAY=$2
		TMPFILE=/tmp/nodog_check.$$

		ask_nodog_to_check_for_users_info &
	else
		# ------------------------------------------------------------------------------------------------------------------------------------------------
		# stefano 10.30.1.131:5280 00:e0:4c:d4:63:f5 10.30.1.105 http://www.google.com 0 10.30.1.105&1257603166&2a2436611f452f8eebddce4992e88f8d 055340773
		# ------------------------------------------------------------------------------------------------------------------------------------------------
		# cat $DIR_REQ/*.req 2>/dev/null | cut -f 1-2 -d' ' | uniq >/tmp/ACCESS_POINT.lst 2>/dev/null
		# ------------------------------------------------------------------------------------------------------------------------------------------------

		# NB: wc se non legge da stdin stampa anche il nome del file...

		NUM_ACCESS_POINT=`wc -l < $ACCESS_POINT_LIST.up 2>/dev/null`

		if [ -n "$NUM_ACCESS_POINT" -a \
					$NUM_ACCESS_POINT -gt 0 ]; then

			local i=0

			while read AP GATEWAY
			do
				let "i = i + 1"

				# we request nodog to check for users logout or disconnect...

				TMPFILE=/tmp/nodog_check.$i

				ask_nodog_to_check_for_users_info &

				sleep 1

			done < $ACCESS_POINT_LIST.up
		fi
	fi

	HTTP_RESPONSE_BODY="OK"
	HTTP_RESPONSE_HEADER="Connection: close\r\n"
}

status_ap() {

	# $1 -> ap

	AP=$1
	TMPFILE=/tmp/$1.html

	send_request_to_nodog "status" $TMPFILE

	if [ -s "$TMPFILE" ]; then
		HTTP_RESPONSE_BODY="<html><body>OK</body></html>"
		HTTP_RESPONSE_HEADER="X-Sendfile: $TMPFILE\r\n"

#		send_request_to_nodog "users" /tmp/$1.users
	else
		message_page "$SERVICE" "$SERVICE (access point non contattabile). Riprovare piu' tardi"
	fi
}

view_user() {

	# $1 -> uid
	# $2 -> outfile

	ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_USER_BASEDN $LDAP_USER_PARAM" "waUid=$1"

	WA_ACTIVE=`awk '/^waActive/{print $2}' $TMPFILE.out 2>/dev/null`
	WA_UID=`	  awk	'/^waUid/{print $2}'		$TMPFILE.out 2>/dev/null`
	WA_CELL=`  awk	'/^waCell/{print $2}'	$TMPFILE.out 2>/dev/null`

	ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_CARD_BASEDN $LDAP_CARD_PARAM" "waUsedBy=$1"

	WA_CID=`		 awk	'/^waCid/{print $2}'			$TMPFILE.out 2>/dev/null`
	WA_PIN=`		 awk	'/^waPin/{print $2}'			$TMPFILE.out 2>/dev/null`
	WA_CARDID=`	 awk	'/^waCardId/{print $2}'		$TMPFILE.out 2>/dev/null`
	WA_REVOKED=` awk	'/^waRevoked/{print $2}'	$TMPFILE.out 2>/dev/null`
	WA_VALIDITY=`awk	'/^waValidity/{print $2}'	$TMPFILE.out 2>/dev/null`
	WA_LOGIN=`	 awk	'/^waLogin/{print $2}'		$TMPFILE.out 2>/dev/null`
	WA_PASSWORD=`awk	'/^waPassword/{print $2}'	$TMPFILE.out 2>/dev/null`
	WA_USEDBY=`	 awk	'/^waUsedBy/{print $2}'		$TMPFILE.out 2>/dev/null`
	WA_NOTAFTER=`awk	'/^waNotAfter/{print $2}'	$TMPFILE.out 2>/dev/null`

	YEAR=`	echo ${WA_NOTAFTER:0:4}`
	MONTH=`	echo ${WA_NOTAFTER:4:2}`
	DAY=`		echo ${WA_NOTAFTER:6:2}`
	HOUR=`	echo ${WA_NOTAFTER:8:2}`
	MINUTES=`echo ${WA_NOTAFTER:10:2}`

	# --------------------------------------------------------------------
	# GET POLICY FOR THIS CARD
	# --------------------------------------------------------------------
	WA_POLICY=`grep 'waPolicy: ' $TMPFILE.out | cut -f2 -d' ' 2>/dev/null`

	if [ -n "$WA_POLICY" ]; then
		POLICY=$WA_POLICY
	fi

	load_policy

	read_counter "$1"

	if [ -z "$WA_NOTAFTER" ]; then
		WA_NOTAFTER="Non disponibile"
	else
		WA_NOTAFTER="$DAY/$MONTH/$YEAR - $HOUR:$MINUTES"
	fi

	REVOKED=NO

	if [ "$WA_REVOKED" = "TRUE" ]; then
		REVOKED=SI
	fi

	get_user_nome_cognome $1

	printf "`cat $DIR_TEMPLATE/view_user.tmpl`" "$USER" $1 \
															  "$REMAINING_TIME_MIN" "$REMAINING_TRAFFIC_MB" \
															  $WA_PASSWORD "$WA_NOTAFTER" $WA_VALIDITY $REVOKED $POLICY > $2

	HTTP_RESPONSE_BODY="OK"
	HTTP_RESPONSE_HEADER="Connection: close\r\n"
}

status_network() {

	TMPFILE=/tmp/wi-auth-stat.$$

	# stefano 055340773 10.30.1.131:5280 00:e0:4c:d4:63:f5 10.30.1.105

	cat $DIR_CTX/*.ctx 2>/dev/null | sort > $TMPFILE 2>/dev/null

	# NB: wc se non legge da stdin stampa anche il nome del file...

	NUM_USERS=`			wc -l < $TMPFILE				   2>/dev/null`
	NUM_ACCESS_POINT=`wc -l < $ACCESS_POINT_LIST.up 2>/dev/null`

	# $1 -> outfile

	printf "`cat $DIR_TEMPLATE/status_network_head.tmpl`" $NUM_ACCESS_POINT $NUM_USERS > $1

	if [ $NUM_USERS -gt 0 ]; then

		BODY=`cat $DIR_TEMPLATE/status_network_body.tmpl 2>/dev/null`

		while read AP UUID GATEWAY MAC IP AUTH_DOMAIN
		do
			# --------------------------------------------------------------------
			# GET POLICY FROM FILE (UUID_TO_LOG POLICY MAX_TIME MAX_TRAFFIC)
			# --------------------------------------------------------------------
			FILE_UID=$DIR_REQ/$UUID.uid

			if [ -s "$FILE_UID" ]; then
				read UUID_TO_LOG POLICY USER_MAX_TIME USER_MAX_TRAFFIC < $FILE_UID 2>/dev/null
			fi

			read_counter $UUID

			check_if_user_connected_to_AP_NO_CONSUME $AP

			if [ "$CONSUME_ON" = "true" ]; then
				COLOR="green"
				CONSUME="yes"
			else
				COLOR="orange"
				CONSUME="no"
			fi

			if [ -s $DIR_CTX/$UUID.ctx ]; then
				LOGIN_TIME=`date -r $DIR_CTX/$UUID.ctx 2>/dev/null`

				get_user_nome_cognome $UUID

				RIGA=`printf "$BODY" "$USER" $UUID "$AUTH_DOMAIN" $IP $MAC \
											"$LOGIN_TIME" $POLICY \
											"$REMAINING_TIME_MIN" "$REMAINING_TRAFFIC_MB" \
											$COLOR $CONSUME \
											"http://$VIRTUAL_NAME/status_ap?ap=$AP&public=$GATEWAY" $AP \
											2>/dev/null`

				OUTPUT=`echo "$OUTPUT"; echo "$RIGA" 2>/dev/null`
			fi
		done < $TMPFILE
	fi

	rm -f $TMPFILE

	echo "$OUTPUT" >> $1

	HTTP_RESPONSE_BODY="OK"
	HTTP_RESPONSE_HEADER="Connection: close\r\n"
}

uploader() {

	# $1 -> path file uploaded

	mv $1 $HISTORICAL_LOG_DIR

	HTTP_RESPONSE_BODY="OK"
	HTTP_RESPONSE_HEADER="Connection: close\r\n"
}

reset_policy() {

	for POLICY_FILEPATH in `ls $DIR_POLICY/* 2>/dev/null`
	do
		POLICY=`basename $POLICY_FILEPATH 2>/dev/null`

		load_policy

 		if [ -n "$POLICY_RESET" ]; then
 			rm	-rf   $DIR_CNT/$POLICY 2>/dev/null
 			mkdir -p $DIR_CNT/$POLICY
 		fi
	done

	# cleaning
	find $DIR_CTX	  -type f -mtime +2 -exec rm -f {} \; 2>/dev/null
	find $DIR_REQ	  -type f -mtime +2 -exec rm -f {} \; 2>/dev/null
	find $DIR_CLIENT -type f -mtime +1 -exec rm -f {} \; 2>/dev/null

	HTTP_RESPONSE_BODY="OK"
	HTTP_RESPONSE_HEADER="Connection: close\r\n"
}

polling_attivazione() {

	# $1 WA_CELL
	# $2 password

	INFO=""
	LOGIN_FORM=""
	TITLE="VERIFICA ATTIVAZIONE: ATTENDERE..."
	SSI_HEAD="<meta http-equiv=\"refresh\" content=\"3\">"
	CREDENTIALS_TAG="<p class=\"bigger\">&nbsp;</p><p class=\"bigger\">&nbsp;</p>"

	# ldapsearch -LLL -b ou=cards,o=unwired-portal -x -D cn=admin,o=unwired-portal -w programmer -H ldap://10.30.1.131

	if [ -z "$generate_PASSWORD" ]; then

		ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_CARD_BASEDN $LDAP_CARD_PARAM" "waLogin=$1"

		WA_PASSWORD=$2

		ATTIVATO=`awk '/^waUsedBy/{print $2}' $TMPFILE.out 2>/dev/null`
	else

		ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_CARD_BASEDN $LDAP_CARD_PARAM" "waUsedBy=$1" waPassword

		WA_PASSWORD=`awk '/^waPassword/{print $2}' $TMPFILE.out 2>/dev/null`

		ATTIVATO="$WA_PASSWORD"
	fi

	if [ -n "$ATTIVATO" ]; then

		INFO="
<p>Ti suggeriamo di prendere nota: </p>
<ul>
	<li>delle credenziali che ti serviranno ogni volta che vorrai accedere al servizio</li>
	<li><!--#echo var=\"$LOGOUT_NOTE\"--></li>
	<br/>
	<!--#echo var=\"$LOGOUT_HTML\"-->
</ul>
<br/>
<p class=\"bigger\">Ora puoi accedere al servizio cliccando il bottone</p>
"
	# $1	-> mac
	# $2  -> ip
	# $3	-> redirect
	# $4	-> gateway
	# $5	-> timeout
	# $6	-> token
	# $7	-> ap
	# $8  -> realm (all, firenzecard, ...)
	# $9  -> uid
	# $10 -> password
	# $11 -> bottone

		LOGIN_FORM="
<form action=\"/login_request\" method=\"post\">
<input type=\"hidden\" name=\"mac\" value=\"\">
<input type=\"hidden\" name=\"ip\" value=\"\">
<input type=\"hidden\" name=\"redirect\" value=\"\">
<input type=\"hidden\" name=\"gateway\" value=\"\">
<input type=\"hidden\" name=\"timeout\" value=\"\">
<input type=\"hidden\" name=\"token\" value=\"\">
<input type=\"hidden\" name=\"ap\" value=\"\">
<input type=\"hidden\" name=\"realm\" value=\"all\" />
<input type=\"hidden\" name=\"uid\" value=\"$1\">
<input type=\"hidden\" name=\"pass\" value=\"$WA_PASSWORD\">
<input type=\"image\" src=\"images/accedi.png\" name=\"submit\" value=\"Entra\" />
</form>
"

		SSI_HEAD=""
		TITLE="LE TUE CREDENZIALI SONO:"
		CREDENTIALS_TAG="<p class=\"bigger\">Utente: $1</p><p class=\"bigger\">Password: $WA_PASSWORD</p>"
	fi

	TITLE_TXT="Verifica attivazione"

	print_page "$TITLE" "$CREDENTIALS_TAG" "$INFO" "$LOGIN_FORM"
}

do_cmd() {

	# check if we are operative...

	if [ -x $DIR_WEB/$VIRTUAL_HOST/ANOMALIA ]; then

		unset BACK_TAG

		write_LOG "Sistema non disponibile"

		message_page "$SERVICE" "$SERVICE per anomalia interna. Contattare l'assistenza: $TELEFONO"
	fi

	if [ "$REQUEST_METHOD" = "GET" ]; then

		case "$REQUEST_URI" in
			/info)						info_notified_from_nodog	"$@"	;;
			/start_ap)					start_ap							"$@"	;;
			/error_ap)					error_ap							"$@"	;;
			/postlogin)					postlogin						"$@"	;;
			/stato_utente)				stato_utente					"$@"	;;
			/polling_password)		polling_password				"$@"	;;
			/polling_attivazione)	polling_attivazione			"$@"	;;
			/registrazione)			registrazione_request		"$@"	;;
			/login_request)			login_request					"$@"	;;
			/login_validate)			login_validate					"$@"	;;
			/logout)						logout_request							;;
			/login)						main_page						"$@"	;;
			/unifi)						unifi_page								;;
			/logged)						logged_page								;;

		 # /unifi_login_request|/logout_page) print_page ;;

			/admin)
				HTTP_RESPONSE_BODY="<html><body>OK</body></html>"
				HTTP_RESPONSE_HEADER="Refresh: 0; url=https://$SERVER_ADDR/admin.html\r\n"
			;;

			/status_ap)
				if [ "$VIRTUAL_HOST" = "$VIRTUAL_NAME" ]; then
					status_ap "$@"
				fi
			;;

			/webif_ap)
				if [ "$VIRTUAL_HOST" = "$VIRTUAL_NAME" ]; then

					set_ap $1

					if [ -n "$GATEWAY" ]; then

						echo -n ${GATEWAY%:*} > $DIR_ROOT/client/$REQUEST_ID.srv

						HTTP_RESPONSE_BODY="<html><body>OK</body></html>"
						HTTP_RESPONSE_HEADER="Refresh: 0; url=http://$VIRTUAL_NAME/cgi-bin/webif/status-basic.sh?cat=Status\r\n"
					fi
				fi
			;;

			/gen_activation)
				if [ "$REMOTE_ADDR" = "$SERVER_ADDR" ]; then
					gen_activation "$@"
				fi
			;;
			/get_users_info)
				if [ "$REMOTE_ADDR" = "$SERVER_ADDR" ]; then
					get_users_info "$@"
				fi
			;;
			/reset_policy)
				if [ "$REMOTE_ADDR" = "$SERVER_ADDR" ]; then
					reset_policy
				fi
			;;
			/recovery)
				if [ "$REMOTE_ADDR" = "$SERVER_ADDR" ]; then
					recovery_user "$@"
				fi
			;;
			/view_user)
				if [ "$REMOTE_ADDR" = "$SERVER_ADDR" ]; then
					view_user "$@"
				fi
			;;
			/status_network)
				if [ "$REMOTE_ADDR" = "$SERVER_ADDR" ]; then
					status_network "$@"
				fi
			;;

			*) print_page ;;
		esac

	elif [ "$REQUEST_METHOD" = "POST" ]; then

		case "$REQUEST_URI" in
			/logout)				logout_notified_from_popup "$@" ;;
			/password)			password_request				"$@" ;;
			/uploader)			uploader							"$@" ;;
			/login_request)	login_request					"$@" ;;
			/registrazione)	registrazione_request		"$@" ;;
		esac
	fi

	write_SSI
}
#-----------------------------------
# END FUNCTION
#-----------------------------------
DEBUG=0

main "$@"
