#!/bin/sh

# login.sh

if [ "$REQUEST_METHOD" = "POST" -a $# -eq 2 ]; then

	echo "Connection: close"

	# -----------------------------------------------------------------------------------------------------------------------------------
	# REQ: Set-Cookie: TODO[ data expire path domain secure HttpOnly ]
	# -----------------------------------------------------------------------------------------------------------------------------------
	# string -- key_id or data to put into the cookie -- must
	# int    -- lifetime of the cookie in HOURS       -- must (0 -> valid until browser exit)
	# string -- path where the cookie can be used     --  opt
	# string -- domain which can read the cookie      --  opt
	# bool   -- secure mode                           --  opt
	# bool   -- only allow HTTP usage                 --  opt
	# -----------------------------------------------------------------------------------------------------------------------------------
	# RET: Set-Cookie: ulib.s<counter>=data&expire&HMAC-MD5(data&expire); expires=expire(GMT); path=path; domain=domain; secure; HttpOnly
	# -----------------------------------------------------------------------------------------------------------------------------------
 
	echo "Set-Cookie: TODO[ \"$1\" 24 ]"
	echo

	exit 0
fi

exit 1
