#!/bin/bash

DATE=`date '+%Y%m%d'` # 20140117

mv wifi-log		 wifi-log-$DATE
mv wifi-warning wifi-warning-$DATE

kill -HUP `cat /var/run/userver_tcp2.pid`

bzip2 wifi-log-$DATE
bzip2 wifi-warning-$DATE
