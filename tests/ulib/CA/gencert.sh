#!/bin/sh

#if [ $# -eq 0 ]; then
#  echo $"Usage: `basename $0` days [...]"
#  exit 0
#fi

answers() {
        echo IT
        echo Italy
        echo Sesto Fiorentino
        echo Unirel spa
        echo $1
        echo localhost.localdomain
        echo root@localhost.localdomain
	echo # A challenge password []:
	echo # An optional company name []:
}

export OPENSSL=openssl

openssl_ca="$OPENSSL   ca -config ./openssl.cnf -days 4000"
openssl_req="$OPENSSL req -config ./openssl.cnf -days 4000"

touch index.txt
mkdir -p private newcerts

umask 077

if [ ! -f serial ]; then
  echo 01 > serial
fi

# TSA
# echo 01 > tsaserial

# Generate CA key/cert
answers CA       | $openssl_req -x509 -newkey rsa:1024 -keyout private/cakey.pem -out cacert.pem
# Generate server key/cert
answers server   | $openssl_req       -newkey rsa:1024 -keyout server.key        -out server.csr
# Generate user key/cert
answers username | $openssl_req       -newkey rsa:1024 -keyout username.key      -out username.csr

# Sign keys
$openssl_ca -in   server.csr -out   server.crt
$openssl_ca -in username.csr -out username.crt

# emit CRL
echo 01 > crlnumber

$openssl_ca -gencrl -out crl.pem # -revoke compromised_cert.pem

rm -f 0[0-9].pem *.csr *.old

mkdir -p CApath
cd CApath
cp ../server.crt server.pem
cp ../username.crt username.pem
cp ../cacert.pem .
cp ../crl.pem .
c_rehash .
rm -f *.pem
