#/bin/bash

PROGRAM=`basename $0 .sh`

#/home/unirel/userver/bin/send_req_to_portal.sh $PROGRAM "get_ap_uptime?ap=01@unifi-r29587_x86\&public=151.11.47.5"
/home/unirel/userver/bin/send_req_to_portal.sh $PROGRAM "get_ap_uptime?ap=01@unifi-r29587_x86\&public=159.213.248.230"
/home/unirel/userver/bin/send_req_to_portal.sh $PROGRAM "get_ap_uptime?ap=01@cascineConc-r29587_x86\&public=159.213.248.232"
