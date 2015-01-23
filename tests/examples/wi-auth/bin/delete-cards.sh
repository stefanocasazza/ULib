#/bin/bash

IFS='$'
while read dn 
do
	dn=`echo -n "$dn" | cut -d':' -f2`
	echo $dn deleted
	ldapdelete -c -x -D "cn=admin,o=unwired-portal" -w programmer "$dn"
done < /tmp/dn-list.txt
