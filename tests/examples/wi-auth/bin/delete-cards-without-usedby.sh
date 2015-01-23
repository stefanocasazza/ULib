#/bin/bash

if [ -f /tmp/dn-list.txt ]; then
	rm -f /tmp/dn-list.txt
fi

ldapsearch -x -D "cn=admin,o=unwired-portal" -w "programmer" -b "ou=cards,o=unwired-portal" '(&(objectClass=waCard)(!(waUsedBy='*')))' | grep 'dn:' >/tmp/dn-list.txt

IFS='$'
while read dn 
do
	dn=`echo -n "$dn" | cut -d':' -f2`
	echo $dn deleted
	ldapdelete -c -x -D "cn=admin,o=unwired-portal" -w programmer "$dn"
done < /tmp/dn-list.txt
