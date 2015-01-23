#/bin/bash

# get_users_info.sh

# echo '*/5 * * * * /home/unirel/userver/bin/get_users_info.sh' | crontab

PROGRAM=`basename $0 .sh`

exec /home/unirel/userver/bin/send_req_to_portal.sh $PROGRAM "get_users_info"
