#!/bin/bash

# set -x

# internal var
USER=""
OUTPUT=""
TMPFILE=""
SSI_HEAD=""
SSI_BODY=""
TITLE_TXT=""
EXIT_VALUE=0
CONNECTION_CLOSE=""
HTTP_RESPONSE_BODY=""
HTTP_RESPONSE_HEADER=""

# external var
# REGEX_HISTORICAL_LOGS=""
# UNCOMPRESS_COMMAND_HISTORICAL_LOGS=""

. $DIR_ROOT/etc/$VIRTUAL_HOST/script.conf
# common func
. $DIR_ROOT/etc/common.sh

#-----------------------------------
# START STATISTICS FUNCTION
#-----------------------------------
export_statistics_login_as_csv() {

	COMMAND=cat

	if [ -n "$1" ]; then
		FILE_LOG="$HISTORICAL_LOG_DIR/$1"
		COMMAND=$UNCOMPRESS_COMMAND_HISTORICAL_LOGS
	fi

	TMPFILE=/tmp/statistics_login_$$.csv

	$COMMAND $FILE_LOG | \
	awk '
	/LOGIN/ { a=$8; gsub(",","",a) ; login[a $1]+=1 ; if (!date[$1]) date[$1]+=1 ; if (!ap[a]) ap[a]+=1 }

	END {
		n=asorti(date, sorted_date);

		printf "\"\","; 
		
		for (i = 1; i <= n; i++) {
			printf "\"%s\",", sorted_date[i] 
		}; 
		
		printf "\n"
		
		for (j in ap) { 

			printf "\"%s\",", j
			
			for (i = 1; i <= n; i++) {
				printf "%.0f,", login[j sorted_date[i]] 
			}

			printf "\n"
		}
	}
	' > $TMPFILE 2>/dev/null

	HTTP_RESPONSE_BODY="<html><body>OK</body></html>"
	HTTP_RESPONSE_HEADER="X-Sendfile: $TMPFILE\r\n"
}

export_statistics_registration_as_csv() {

	TMPFILE=/tmp/statistics_registration_$$.csv

	ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_CARD_BASEDN $LDAP_CARD_PARAM" "waNotAfter=*" waNotAfter

	awk '
	/^waNotAfter/ {
		year=substr(	$2, 0,4); 
		month=substr(	$2, 5,2); 
		day=substr(		$2, 7,2); 
		hour=substr(	$2, 9,2); 
		minutes=substr($2,11,2); 
		seconds=substr($2,13,2); 

		expire_date=year" "month" "day" "hour" "minutes" "seconds;

		# print expire_date;

		expire_date_in_seconds = mktime(expire_date);

		# print expire_date_in_seconds;

		validity=180*60*60*24;

		date=expire_date_in_seconds-validity;

		# print date;

		date_formatted = strftime("%Y/%m/%d", date);

		# print date_formatted;

		registrations[date_formatted]+=1;
		total+=1;
	}

	END {
		n=asorti(registrations, sorted_registrations);

		printf "\"%s\"", "Data";
		printf ",";
		printf "\"%s\"", "Registrazioni";
		printf "\n";

		for (i = 1; i <= n; i++) {

			printf "\"%s\"", sorted_registrations[i];
			printf ",";
			printf "%.0f", registrations[sorted_registrations[i]];
			printf "\n";
		}

		printf "\"%s\"", "Totale";
		printf ",";
		printf "%.0f", total;
		printf "\n";
	}
	' $TMPFILE.out > $TMPFILE 2>/dev/null

	HTTP_RESPONSE_BODY="<html><body>OK</body></html>"
	HTTP_RESPONSE_HEADER="X-Sendfile: $TMPFILE\r\n"
}

historical_statistics_login() {

	TABLE_TAG_START="<table class=\"centered\" border=\"1\">"
	TABLE_TAG_END="</table>"
	TR_TAG_START="<tr>"
	TR_TAG_END="</tr>"
	TD_HEADER_TAG_START="<td class=\"header_smaller\">"
	TD_DATA_TAG_START="<td class=\"data_smaller\" align=\"right\">"
	TD_TAG_END="</td>"
	TH_TAG_START="<th class=\"header_smaller\">"
	TH_TAG_END="</th>"
	URL_TAG_START="<a href=\"admin_view_statistics_login?file=%s\">%s</a>"

	TABLE="$TABLE_TAG_START\n\t$TH_TAG_START\n\t\tARCHIVI\t$TH_TAG_END\n"

	for file in `ls -rt $HISTORICAL_LOG_DIR/$REGEX_HISTORICAL_LOGS`
	do
		filename=`basename $file 2>/dev/null`
		TAG=`printf "$URL_TAG_START" $filename $filename`

		TABLE="$TABLE\t$TR_TAG_START\n\t\t$TD_DATA_TAG_START\n\t\t\t$TAG\n\t\t$TD_TAG_END\n\t$TR_TAG_END\n"
	done

	TABLE=`echo -e "$TABLE$TABLE_TAG_END"`

	TITLE_TXT="Storico"

	print_page "Storico" "$TABLE"
}

view_statistics_login() {

	REQUEST_URI=view_statistics
	HREF_TAG="admin_export_statistics_login_as_csv"

	if [ -n "$1" ]; then
		HREF_TAG="$HREF_TAG?file=$1"
		FILE_LOG=$HISTORICAL_LOG_DIR/$1
		COMMAND=$UNCOMPRESS_COMMAND_HISTORICAL_LOGS
		TITLE_TXT="Numero LOGIN per Access Point: file $1"
	else
		COMMAND=cat
		TITLE_TXT="Numero LOGIN per Access Point"
	fi

	export TABLE_TAG_START="<table class=\"centered\" border=\"1\">"
	export TABLE_TAG_END="</table>"
	export TR_TAG_START="<tr>"
	export TR_TAG_END="</tr>"
	export TD_HEADER_TAG_START="<td class=\"header_smaller\">"
	export TD_HEADER_ALIGNED_TAG_START="<td class=\"header_smaller\" align=\"right\">"
	export TD_DATA_TAG_START="<td class=\"data_smaller\" align=\"right\">"
	export TD_TAG_END="</td>"
	export TH_TAG_START="<th class=\"header_smaller\">"
	export TH_TAG_END="</th>"

	TABLE=`$COMMAND $FILE_LOG | awk '
	BEGIN {

		trTagStart=ENVIRON["TR_TAG_START"];
		trTagEnd=ENVIRON["TR_TAG_END"];

		tdTagHeaderStart=ENVIRON["TD_HEADER_TAG_START"];
		tdTagHeaderAlignedStart=ENVIRON["TD_HEADER_ALIGNED_TAG_START"];

		tdTagDataStart=ENVIRON["TD_DATA_TAG_START"];

		tdTagEnd=ENVIRON["TD_TAG_END"];

		thTagStart=ENVIRON["TH_TAG_START"];
		thTagEnd=ENVIRON["TH_TAG_END"];

		printf "%s\n", ENVIRON["TABLE_TAG_START"];
	}

	/LOGIN/ { a2=$8; gsub(",","",a2)			###				  label@hostname
				 a1=a2; sub("[^@]*@","",a1)   ### hostname
				 a=a1 " " a2						### hostname " " label@hostname
				 login[a $1]+=1 ; login[a]+=1 ; login[$1]+=1 ; if (!date[$1]) date[$1]+=1 ; if (!ap[a]) ap[a]+=1 }

	END {
		n=asorti(date, sorted_date);

		printf "\t%s\n\t\t%s%s%s\n", trTagStart, thTagStart, thTagEnd, "" ; 

		for (i = 1; i <= n; i++) {
			printf "\t\t%s%s%s\n", thTagStart, sorted_date[i], thTagEnd 
		} 

		printf "\t\t%s%s%s\n", thTagStart, "Totale x AP", thTagEnd 
		printf "\t%s\n", trTagEnd

		m=asorti(ap, sorted_ap);

		for (j = 1; j <= m; j++) {

			a=sorted_ap[j]; sub("[^ ]* ", "", a) ### label@hostname

			printf "\t%s\n\t\t%s%s%s\n", trTagStart, tdTagHeaderStart, a, tdTagEnd

			for (i = 1; i <= n; i++) {
				printf "\t\t%s%.0f%s\n", tdTagDataStart, login[sorted_ap[j] sorted_date[i]], tdTagEnd
			}

			printf "\t\t%s%.0f%s\n", tdTagHeaderAlignedStart, login[sorted_ap[j]], tdTagEnd
			printf "\t%s\n", trTagEnd
		}

		printf "\t%s\n\t\t%sTotale x data%s\n", trTagStart, tdTagHeaderStart, tdTagEnd

		for (i = 1; i <= n; i++) {
			printf "\t\t%s%.0f%s\n", tdTagHeaderAlignedStart, login[sorted_date[i]], tdTagEnd
			totale+=login[sorted_date[i]]
		}

		printf "\t\t%s%.0f%s\n", tdTagHeaderAlignedStart, totale, tdTagEnd
		printf "\t%s\n", trTagEnd
		printf "%s\n", ENVIRON["TABLE_TAG_END"];
	}
	'`

	print_page "$TITLE_TXT" "$TABLE" "<a class=\"back\" href=\"$HREF_TAG\">Esporta in formato CSV</a>"
}

view_statistics_registration() {

	export TABLE_TAG_START="<table class=\"centered\" border=\"1\">"
	export TABLE_TAG_END="</table>"
	export TR_TAG_START="<tr>"
	export TR_TAG_END="</tr>"
	export TD_HEADER_TAG_START="<td class=\"header_smaller\">"
	export TD_DATA_TAG_START="<td class=\"data_smaller\" align=\"right\">"
	export TD_TAG_END="</td>"
	export TH_TAG_START="<th class=\"header_smaller\">"
	export TH_TAG_END="</th>"

	ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_CARD_BASEDN $LDAP_CARD_PARAM" "waUsedBy=*" "waUsedBy modifyTimestamp"

	TABLE=`awk '
	BEGIN {

		trTagStart=ENVIRON["TR_TAG_START"];
		trTagEnd=ENVIRON["TR_TAG_END"];

		tdTagHeaderStart=ENVIRON["TD_HEADER_TAG_START"];
		tdTagDataStart=ENVIRON["TD_DATA_TAG_START"];

		tdTagEnd=ENVIRON["TD_TAG_END"];

		thTagStart=ENVIRON["TH_TAG_START"];
		thTagEnd=ENVIRON["TH_TAG_END"];

		printf "%s\n", ENVIRON["TABLE_TAG_START"];
	}

	/^modifyTimestamp/ { 
		year=substr($2, 0 , 4); 

		month=substr($2, 5 , 2); 

		day=substr($2, 7 , 2); 

		hour=substr($2, 9 , 2); 

		minutes=substr($2, 11 , 2); 

		seconds=substr($2, 13 , 2); 

		expire_date=year" "month" "day" "hour" "minutes" "seconds;

		#print expire_date;

		expire_date_in_seconds = mktime(expire_date);

		#print expire_date_in_seconds;

		validity=ENVIRON["REG_VALIDITY"];

		date=expire_date_in_seconds-validity;

		#print date;

		date_formatted = strftime("%Y/%m/%d", date);

		#print date_formatted;

		registrations[date_formatted]+=1;

		total+=1;
	}

	END {
		n=asorti(registrations, sorted_registrations);

		printf "\t%s\n", trTagStart;
		printf "\t\t%s\n", thTagStart;
		printf "\t\t\t%s\n", "Data";
		printf "\t\t%s\n", thTagEnd;
		printf "\t\t%s\n", thTagStart;
		printf "\t\t\t%s\n", "Registrazioni";
		printf "\t\t%s\n", thTagEnd;
		printf "\t%s\n", trTagEnd;

		for (i = 1; i <= n; i++) {
			printf "\t%s\n", trTagStart;

			printf "\t\t%s\n", tdTagDataStart;
			printf "\t\t\t%s\n", sorted_registrations[i];
			printf "\t\t%s\n", tdTagEnd;

			printf "\t\t%s\n", tdTagDataStart;
			printf "\t\t\t%.0f\n", registrations[sorted_registrations[i]];
			printf "\t\t%s\n", tdTagEnd;

			printf "\t%s\n", trTagEnd;
		}

		printf "\t%s\n", trTagStart;

		printf "\t\t%s\n", tdTagDataStart;
		printf "\t\t\t%s\n", "Totale";
		printf "\t\t%s\n", tdTagEnd;

		printf "\t\t%s\n", tdTagDataStart;
		printf "\t\t\t%.0f\n", total;
		printf "\t\t%s\n", tdTagEnd;

		printf "\t%s\n", trTagEnd;

		printf "%s\n", ENVIRON["TABLE_TAG_END"];
	}
	' $TMPFILE.out`

	REQUEST_URI=view_statistics
	TITLE_TXT="Numero Registrazioni per data"

	print_page "$TITLE_TXT" "$TABLE" "<a class=\"back\" href=\"admin_export_statistics_registration_as_csv\">Esporta in formato CSV</a>"
}

printlog() {

	TMPFILE=/tmp/printlog.$$

	# 2009/11/07 13:14:35 op: PASS_AUTH, uid: 3397363258, ap: dev, ip: 10.30.1.105, mac: 00:e0:4c:d4:63:f5, timeout: 86400, traffic: 300

	awk '
	{
	for (f = 1 ; f <= NF ; f++)
		{
		row[NR,f] = $f

		l = length($f) ; if (l > max[f]) max[f] = l
		}

	if (NF > maxNF) maxNF = NF
	}

	END {
		for (r = (NR > 200 ? NR : 201) - 200 ; r <= NR ; r++)
			for (f = 1 ; f <= maxNF ; f++)
				printf "%-*s%s",
								 (max[f] > 999 ? 999 : max[f]),
					(length(row[r,f]) > 999 ? substr(row[r,f], 1, 999 -3) "..." : row[r,f]),
					(f < maxNF ? " " : "\n")
	}
	' $FILE_LOG > $TMPFILE 2>/dev/null

	if [ ! -s "$TMPFILE" ]; then
		echo "EMPTY" > $TMPFILE
	fi

	HTTP_RESPONSE_BODY="<html><body>OK</body></html>"
	HTTP_RESPONSE_HEADER="X-Sendfile: $TMPFILE\r\n"
}
#-----------------------------------
# END STATISTICS FUNCTION
#-----------------------------------

#-----------------------------------
# START FUNCTION
#-----------------------------------
execute_recovery() {

	# $1 -> uid

	ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_USER_BASEDN $LDAP_USER_PARAM" "waUid=$1" dn:

	if [ ! -s $TMPFILE.out ]; then
		message_page "Recovery utente: utente non registrato" "Recovery utente: $1 non registrato!"
	fi

	USER_DN=`cut -f2 -d':' $TMPFILE.out 2>/dev/null`

	ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_CARD_BASEDN $LDAP_CARD_PARAM" "waPin=$1" dn:

	if [ ! -s $TMPFILE.out ]; then
		message_page "Errore recovery utente" "Errore recovery utente: $1 (card)"
	fi

	CARD_DN=`cut -f2 -d':' $TMPFILE.out 2>/dev/null`

	ask_to_LDAP ldapdelete "$LDAP_CARD_PARAM" "$CARD_DN"

	if [ $EXIT_VALUE -ne 0 ]; then
		message_page "Errore" "Errore recovery: fallita cancellazione utente $1 (ldap branch card)"
	fi

	ask_to_LDAP ldapdelete "$LDAP_USER_PARAM" "$USER_DN"

	if [ $EXIT_VALUE -ne 0 ]; then
		message_page "Errore" "Errore recovery: fallita cancellazione utente $1 (ldap branch user)"
	fi

	$CLIENT_HTTP "http://$VIRTUAL_NAME/recovery?user=$1" >/dev/null 2>>/tmp/CLIENT_HTTP.err

#	BACK_TAG="<a href=\"admin\">TORNA AL MENU</a>"

	message_page "Esito recovery" "Recovery completato!"
}

view_user() {

	# $1 -> uid

	ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_USER_BASEDN $LDAP_USER_PARAM" "waUid=$1"

	if [ ! -s $TMPFILE.out ]; then
		message_page "Visualizzazione dati utente: utente non registrato" "Visualizzazione dati utente: $1 non registrato!"
	fi

#  ask_to_LDAP ldapsearch "-LLL -b $WIAUTH_CARD_BASEDN $LDAP_CARD_PARAM" "waUsedBy=$1"
#
#  if [ ! -s $TMPFILE.out ]; then
#     message_page "Visualizzazione dati utente: utente non attivato" "Visualizzazione dati utente: $1 non attivato!"
#  fi

	rm -f $TMPFILE.out $TMPFILE.err

	TMPFILE=/tmp/view_user.$$

	$CLIENT_HTTP "http://$VIRTUAL_NAME/view_user?uid=$1&outfile=$TMPFILE.out" >/dev/null 2>>/tmp/CLIENT_HTTP.err

	if [ $? -eq 0 ]; then
		TITLE_TXT="Visualizzazione dati registrazione utente"

		print_page "`cat $TMPFILE.out`"
	fi
}

do_cmd() {

	if [ "$REQUEST_METHOD" = "GET" ]; then

		case "$REQUEST_URI" in
			/admin_printlog)										 printlog										;;
			/admin_view_statistics_login)						 view_statistics_login "$@"				;;
			/admin_historical_statistics_login)				 historical_statistics_login				;;
			/admin_view_statistics_registration)			 view_statistics_registration				;;
			/admin_export_statistics_login_as_csv)			 export_statistics_login_as_csv "$@"	;;
			/admin_export_statistics_registration_as_csv) export_statistics_registration_as_csv ;;

         /admin)
            HTTP_RESPONSE_BODY="<html><body>OK</body></html>"
            HTTP_RESPONSE_HEADER="Refresh: 0; url=https://$HTTP_HOST/admin.html\r\n"
         ;;
			/admin_view_user)
				REQUEST_URI=get_user_id
				TITLE_TXT="Visualizzazione dati utente"
				SSI_HEAD="<link type=\"text/css\" href=\"css/livevalidation.css\" rel=\"stylesheet\">
							  <script type=\"text/javascript\" src=\"js/livevalidation_standalone.compressed.js\"></script>"

				print_page "Visualizzazione dati utente" "admin_view_user"
			;;
			/admin_recovery)
				REQUEST_URI=get_user_id
				TITLE_TXT="Recovery utente"
				SSI_HEAD="<link type=\"text/css\" href=\"css/livevalidation.css\" rel=\"stylesheet\">
							  <script type=\"text/javascript\" src=\"js/livevalidation_standalone.compressed.js\"></script>"

				print_page "Procedura di Recovery" "admin_recovery"
			;;
			/admin_status_network)
				TMPFILE=/tmp/status_network.$$

				# export UTRACE="0 5M"
				$CLIENT_HTTP "http://$VIRTUAL_NAME/status_network?outfile=$TMPFILE.out" >/dev/null 2>>/tmp/CLIENT_HTTP.err

				if [ $? -eq 0 ]; then
					TITLE_TXT="$TITLE_DEFAULT: stato rete"
					SSI_HEAD="<meta http-equiv=\"refresh\" content=\"30\">"

					print_page "`date`" "`cat $TMPFILE.out`"
				fi
			;;
			/admin_status_nodog)
				TMPFILE=/tmp/status_nodog.$$

				$CLIENT_HTTP "http://$VIRTUAL_NAME/status_nodog?outfile=$TMPFILE.out" >/dev/null 2>>/tmp/CLIENT_HTTP.err

				if [ $? -eq 0 ]; then
					TITLE_TXT="$TITLE_DEFAULT: stato access point"
					SSI_HEAD="<meta http-equiv=\"refresh\" content=\"30\">"

					print_page "`date`" "`cat $TMPFILE.out`"
				fi
			;;

			*) print_page ;;
		esac

	elif [ "$REQUEST_METHOD" = "POST" ]; then

		case "$REQUEST_URI" in
			/admin_view_user)			 view_user			"$@" ;;
			/admin_execute_recovery) execute_recovery "$@" ;;

			/admin_recovery)
				REQUEST_URI=confirm_page
				TITLE_TXT="Conferma recovery"

				get_user_nome_cognome "$1" # $1 -> uid

				print_page "$TITLE_TXT" "$USER" $1 "admin_execute_recovery" $1
			;;
		esac
	fi

	write_SSI
}
#-----------------------------------
# END FUNCTION
#-----------------------------------
DEBUG=0

main "$@"
