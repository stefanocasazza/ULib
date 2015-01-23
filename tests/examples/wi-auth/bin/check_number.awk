BASENAME=`basename $1 '.reg'`
CALLER_ID=`echo -n $BASENAME | \
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
	} else {
		if (match(phoneNumber,"^+[0-9]+$")) {
			phoneNumber=substr(phoneNumber,4);
		}
	}

	if (length(phoneNumber) > 10) {
		prefix=substr(phoneNumber,1,3)
		first=substr(phoneNumber,4,3)

		if (match(first,prefix) == 1) {
			print substr(phoneNumber,4)": ""Not repeat the prefix number please"; exit (1);
		}
	}

	print phoneNumber;

	}' >/dev/null 2>/dev/null`

if [ $? -gt 0 ]; then
	result=`ldapsearch -LLL -b ou=cards,o=unwired-portal -x -D cn=admin,o=unwired-portal -w programmer -H ldap://10.30.1.131 waLogin=$1`

	if [ -z "$result" ]; then
		rm -f $1
	fi
fi
