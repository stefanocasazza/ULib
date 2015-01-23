#!/bin/sh

cn=`awk -F= '/[Ss]ubject[:=]/ {                      
     gsub(" ", "_") ; print "vpn " $NF }' \
     /etc/openvpn/client.crt 2> /dev/null`     
            
test "$cn" || cn=`awk -F= '/[Ss]ubject[:=]/ {    
       gsub(" ", "_") ; print "wiauth " $NF }' \
    /etc/wiauthclt/user.crt.pem 2> /dev/null`

echo "cn=$cn"
