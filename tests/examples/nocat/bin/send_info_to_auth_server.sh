#!/bin/sh

AP_NAME=`uname -n`
ip_address=/tmp/IP_ADDRESS
AUTH_PORTAL=`grep AuthServiceAddr /etc/nodog.conf | tr -d \\\\ | tr -d \\" | cut -d'=' -f2`
tmp=`grep IP_ADDRESS /etc/nodog.conf | tr -s ' ' | cut -d' ' -f2`
public=`echo $tmp`

get_address() {

	if [ -s /tmp/nodog.log ]; then
		sync
	fi

	public=`grep 'SERVER IP ADDRESS' /tmp/nodog.log | cut -d' ' -f10`

	if [ -z "$public" ]; then
		get_address
	fi
}

if [ -z "$public" ]; then
	if [ -f $ip_address ]; then
		public=`cat $ip_address`
	else
		get_address
	fi
fi

send_info_to_authserver() {

   for url in `echo $AUTH_PORTAL`; do
      uclient -c /etc/uclient.conf "${url}/start_ap?ap=$AP_NAME&public=$public:5280" &
		sleep 1
   done
}

send_info_to_authserver
