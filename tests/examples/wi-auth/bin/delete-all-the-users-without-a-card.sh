#!/bin/bash

ldapsearch -x  -D  "cn=admin,o=unwired-portal" -w "programmer" -b "ou=users,o=unwired-portal" -LLL waUid | grep -v 'dn:' | grep -v '^$' | cut -d':' -f2 >/tmp/waUid.txt

while read waUid
do

	DN=`ldapsearch -x  -D  "cn=admin,o=unwired-portal" -w "programmer" -b "ou=cards,o=unwired-portal" -LLL waUsedBy=$waUid | grep 'dn:'`

	echo "DN: $DN"

	if [ -n "$DN" ]; then
		echo "User $waUid exists"
	else
		echo "User $waUid doesn't exists"
		ldapdelete -c -x -D "cn=admin,o=unwired-portal" -w programmer "waUid=$waUid,ou=users,o=unwired-portal"
		echo "User $waUid deleted"
		
	fi

	unset DN

done < /tmp/waUid.txt
