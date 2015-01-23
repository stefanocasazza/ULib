#/bin/bash

# get_users_info_for_ap.sh

PROGRAM=`basename $0 .sh`

# unifi-r29587_x86 151.11.47.120
exec /home/unirel/userver/bin/send_req_to_portal.sh $PROGRAM "get_users_info?ap=$1\&public=$2"
