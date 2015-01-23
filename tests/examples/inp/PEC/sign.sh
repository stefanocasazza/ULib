#!/bin/bash

for FILE in `find $1 -name '*.log'`
do
   openssl smime -in $FILE -sign -signer /mnt/develop/pongo/environment/working_on/project_universita-telematica/certs/segreteria.crt.pem \
											-inkey  /mnt/develop/pongo/environment/working_on/project_universita-telematica/certs/segreteria.key \
											-nodetach -binary -outform DER -passin pass:segreteria > $FILE.p7m
done
