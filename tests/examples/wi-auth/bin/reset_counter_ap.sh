#/bin/bash

# reset_counter_ap.sh

PROGRAM=`basename $0 .sh`

exec /home/unirel/userver/bin/send_req_to_portal.sh $PROGRAM "reset_counter_ap"
