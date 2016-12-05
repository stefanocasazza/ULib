#/bin/bash

# resync.sh

# echo '00 04 * * * /home/unirel/userver/bin/reset_policy.sh' | crontab

PROGRAM=`basename $0 .sh`

exec /home/unirel/userver/bin/send_req_to_portal.sh $PROGRAM "resync"
