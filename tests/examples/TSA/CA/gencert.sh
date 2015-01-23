#!/bin/sh

touch index.txt
mkdir -p private

if [ ! -f serial ]; then
	echo 01 > serial
fi

openssl_ca="../../bin/openssl   ca -config openssl.cnf -days 4000"
openssl_req="../../bin/openssl req -config openssl.cnf -days 4000"

# Generate DH params
#$openssl dhparam -out dh1024.pem 1024

# Generate CA key/cert
$openssl_req -x509 -newkey rsa:1024 -keyout private/cakey.pem -out cacert.pem

# Generate server key/cert
$openssl_req -new -nodes -keyout server.key -out server.csr

# Generate user key/cert
# $openssl_req -newkey rsa:1024 -keyout username.key -out username.csr

# Sign keys
$openssl_ca -in   server.csr -out   server.crt
# $openssl_ca -in username.csr -out username.crt

# emit CRL
# $openssl_ca -gencrl -out crl.pem # -revoke compromised_cert.pem

rm -f 0[0-9].pem *.csr *.old

mkdir -p CApath
cd CApath
ln -sf ../server.crt server.pem
#ln -sf ../username.crt username.pem
ln -sf ../cacert.pem
#ln -sf ../crl.pem
c_rehash .
