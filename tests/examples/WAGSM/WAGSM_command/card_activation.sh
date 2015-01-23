#!/bin/bash

# card_activation.sh: activate a card and send back login/pwd via SMS
#
# ARGV[1] = SMS from client
#
# ENV[HOME]				= Base directory
# ENV[FILE_LOG]		= Log file for command
# ENV[MSG_LOG]			= Log separator
# ENV[DEBUG]			= Enable debugging
# ENV[LDAP_HOST]     = LDAP Host
# ENV[LDAP_PASSWORD] = file contenente password for LDAP binding
# ENV[MAIL_TO]			= Email Address for workflow
# ENV[MAIL_FROM]		= Email Address for workflow

SMS="$1"

if [ -z "${SMS}" ]; then
	echo "SMS is empty" >&2
	exit 1
fi

if [ -z "${HOME}" ]; then
   echo "HOME is not set" >&2
   exit 1
fi

cd ${HOME}

if [ ! -f WAGSM_command/.function ]; then
   echo "Unable to found WAGSM_command/.function" >&2
   exit 1
fi

. WAGSM_command/.function

chk_ENV `basename $0`

# if something wrong exec this...

EXIT_CMD="ESITO=0; echo 'Sorry, service not available'"

# start

DEBUG_INFORMATION="ENV[LDAP_HOST]    = \"${LDAP_HOST}\"
ENV[LDAP_PASSWORD]= \"${LDAP_PASSWORD}\"
ENV[MAIL_TO]      = \"${MAIL_TO}\"
ENV[MAIL_FROM]    = \"${MAIL_FROM}\"

ARGV[1] (SMS)     = \"${SMS}\"
"

begin_CMD

# extract information

run_CMD "PIN=`echo \"${SMS}\" | tail -n 1`"
run_CMD "PHONE_NUMBER=`echo ${SMS} | grep 'From:' | cut -f2 -d' '`"

# Search card by pin

run_CMD_with_output CARD "ldapsearch -x -h $LDAP_HOST -p 389 -D uid=unwired-portal-agent,ou=managers,o=unwired-portal -b ou=cards,o=unwired-portal -y $LDAP_PASSWORD -LLL waPin=${PIN}"

if [ -z "$CARD" ]
then
	EXIT_CMD="echo 'Pin errato'"

	end_CMD 0
fi

# Search card by pin and verify that is not already assigned to a user

run_CMD_with_output CARD "ldapsearch -x -h $LDAP_HOST -p 389 -D uid=unwired-portal-agent,ou=managers,o=unwired-portal -b ou=cards,o=unwired-portal -y $LDAP_PASSWORD -LLL (&(waPin=${PIN})(!(waUsedBy=*)))"

if [ -z "$CARD" ]
then
	EXIT_CMD="echo 'Carta in uso!'"

	end_CMD 0
fi

CARD_ID=${CARD:10:36}

# Search user by phone number

run_CMD_with_output USER "ldapsearch -x -h $LDAP_HOST -p 389 -D uid=unwired-portal-agent,ou=managers,o=unwired-portal -b ou=users,o=unwired-portal -y $LDAP_PASSWORD -LLL waCell=${PHONE_NUMBER}"

# generate LOGIN/PASSWORD

run_CMD_with_output UUID uuidgen

LOGIN=${UUID:0:8}
PASSWORD=${UUID:28:8}

if [ -n "$USER" ]
then
	USER_ID=${USER:10:36}
else
#	If user does not exists, generate USER_ID before and then create user

	USER_ID=$UUID

	LDIF="dn: waUid=${USER_ID},ou=users,o=unwired-portal
objectClass: top
objectClass: waUser
waUid: ${USER_ID}
waCell: ${PHONE_NUMBER}
waActive: TRUE"

	run_CMD_with_input "${LDIF}" "ldapadd -x -c -h $LDAP_HOST -p 389 -D cn=admin,o=unwired-portal -y $LDAP_PASSWORD"
fi

# Update card with USER_ID and a new generated LOGIN/PASSWORD

LDIF="dn: waCid=${CARD_ID},ou=cards,o=unwired-portal
waUsedBy: ${USER_ID}
waLogin: ${LOGIN}
waPassword: ${PASSWORD}"

run_CMD_with_input "${LDIF}" "ldapmodify -x -c -h $LDAP_HOST -p 389 -D cn=admin,o=unwired-portal -y $LDAP_PASSWORD"

# send output for SMS to client

unset EXIT_CMD

echo "Login: ${LOGIN}"
echo "Password: ${PASSWORD}"

# sending a mail for workflow

run_CMD "GSMBOX_ID=`echo ${SMS} | grep 'From_SMSC:'   | cut -f2 -d' '`"
run_CMD "waCardId=`echo \"${CARD}\"	  | cut -f5 -d':' | cut -f2 -d' '`"
run_CMD "waValidity=`echo \"${CARD}\" | cut -f8 -d':' | cut -f2 -d' '`"

run_CMD_with_output DATE "date -Iseconds"

MAIL="To: ${MAIL_TO}
From: ${MAIL_FROM}
MIME-Version: 1.0
Subject: cardact
Content-Type: multipart/mixed; boundary=\"_-_as978sd78fd912y_-_\"

--_-_as978sd78fd912y_-_
Content-Type: text/plain;
Content-Disposition: attachment; filename=\"fields.txt\"

waGsmBoxId=$GSMBOX_ID
waCell=$PHONE_NUMBER
waPin=$PIN
waSmsReceptionDate=$DATE
waCid=${CARD_ID}
waCardId=${waCardId}
waValidity=${waValidity}
waUsedBy=${USER_ID}
waLogin=${LOGIN}
waPassword=${PASSWORD}
--_-_as978sd78fd912y_-_
Content-Type: text/plain;
Content-Disposition: attachment; filename=\"sms.txt\"
Content-Transfer-Encoding: binary;

${SMS}
--_-_as978sd78fd912y_-_--"

run_CMD_with_input "${MAIL}" "sendmail -t"

end_CMD 0
