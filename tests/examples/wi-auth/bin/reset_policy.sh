#/bin/bash

# reset_policy.sh

# echo '00 00 * * * /home/unirel/userver/bin/reset_policy.sh' | crontab

# wget supports only Basic auth as the only auth type over HTTP proxy
 wget -q --no-check-certificate --user=admin --password=nessun4 https://10.30.1.111/admin_login_nodog -O /var/log/wi-auth-status-access/`date -u -d "now" +"%Y%m%d%H%M%SZ"`.html
#curl -s -k --digest -u admin:nessun4									 https://10.30.1.111/admin_login_nodog -o /var/log/wi-auth-status-access/`date -u -d "now" +"%Y%m%d%H%M%SZ"`.html

PROGRAM=`basename $0 .sh`

/home/unirel/userver/bin/send_req_to_portal.sh $PROGRAM "reset_policy"

#sleep 1
#wget -q --no-check-certificate --user=admin --password=nessun4 https://10.30.1.111/admin_login_nodog -O /var/log/wi-auth-status-access/`date -u -d "now" +"%Y%m%d%H%M%SZ"`.html
