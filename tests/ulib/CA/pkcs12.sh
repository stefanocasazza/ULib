#!/bin/sh

# export server.crt as PKCS#12 file, mycert.pfx
openssl pkcs12 -export -out mycert.pfx -in server.crt -inkey server_nopass.key -name "My Certificate"
