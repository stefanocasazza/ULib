#!/bin/sh

SERVER="ldap://10.30.1.131"
ROOTDN="cn=admin,o=sessions"
ROOTPW="programmer"
BASEDN="o=sessions"
LASTDATE="1 day ago"

DATE=`date -u -d "$LASTDATE" +"%Y%m%d%H%M%SZ"`
TFILE=`mktemp`

if [ -n "$DATE" -a -n "$TFILE" ]; then
	ldapsearch -LLL -x \
		-H "$SERVER" \
		-D "$ROOTDN" \
		-w "$ROOTPW" \
		-b "$BASEDN" \
		"(&(objectClass=waSession)(createTimestamp<=$DATE))"\
		dn | awk '/dn: /{print $0;print "changeType: delete\n"}' > "$TFILE"

	if [ -s "$TFILE" ]; then
		ldapmodify -x \
			-H "$SERVER" \
			-D "$ROOTDN" \
			-w "$ROOTPW" \
			-f "$TFILE"
	fi
fi

rm "$TFILE"
