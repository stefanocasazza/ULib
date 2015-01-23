#!/bin/sh

# postlogin.sh

if [ "$REQUEST_METHOD" = "GET" -a $# -eq 1 ]; then

	cat <<END
Status: 200
Connection: close
Content-Length: 100
X-Real-IP: 10.30.1.131
Set-Cookie: TODO[ "$1" 24 ]
Set-Cookie: TestCookie=pippo
Content-Type: text/html; charset=iso-8859-1

UID          = $1
HTTP_COOKIE  = $HTTP_COOKIE
ULIB_SESSION = $ULIB_SESSION
END

	exit 0

fi

exit 1
