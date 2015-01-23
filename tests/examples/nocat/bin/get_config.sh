#!/bin/sh

## ==================================================================================================================================================================
## DEBUG
## ==================================================================================================================================================================
## UTRACE=0 /usr/sbin/uclient -c /etc/uclient.conf -o /tmp/nodog.conf.portal -q 60 'https://wifi-aaa.comune.fi.it/get_config?ap=unifi-r29587_x86&key=159.213.248.230'
## UTRACE=0 /usr/sbin/uclient -c /etc/uclient.conf -o /tmp/nodog.conf.portal -q 60 'https://wifi-aaa.comune.fi.it/get_config?ap=lab5-r29587_rs&key=10.10.100.115'
## ==================================================================================================================================================================
## ssh -L10443:192.168.1.20:443 lab5
## ==================================================================================================================================================================

#IP_ADDRESS=`grep IP_ADDRESS /etc/nodog.conf | tr -s ' ' | cut -d' ' -f2`
#AUTH_PORTAL=`grep AuthServiceAddr /etc/nodog.conf | tr -d \\\\ | tr -d \\" | cut -d'=' -f2`

for url in `echo $AUTH_PORTAL`; do
   uclient -c /etc/uclient.conf "${url}/get_config?ap=`uname -n`&key=`cat /etc/nodog.cong.key`" >$1

	if [ $? -eq 0 ]; then
		return
	fi

	sleep 1
done
