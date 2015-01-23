#/bin/bash

# get_ap_check_zombies_for_ap.sh

PROGRAM=`basename $0 .sh`

# unifi-r29587_x86 159.213.248.230
exec /home/unirel/userver/bin/send_req_to_portal.sh $PROGRAM "get_ap_check_zombies?ap=$1\&public=$2"
