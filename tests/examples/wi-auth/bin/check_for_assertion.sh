#/bin/bash

# check_for_assertion.sh

echo -----------------------------------------------------------------------------------------------------------------------------------
grep --colour -i -E -e 'assert|OVERLAP|SIGSEGV|allocate|ERROR|generic|handler|poll|simultaneous' /tmp/userver*.err
echo -----------------------------------------------------------------------------------------------------------------------------------
#grep --colour -i -E -e 'ERROR|ABORT|ASSERT' /var/log/userver-*.log /var/log/uclient-*.log | grep -v 'SSL EOF observed that violates the protocol'
#echo -----------------------------------------------------------------------------------------------------------------------------------
#grep --colour -i -E -e 'Bad Req| diff|detected|File cached' /var/log/userver-*.log | grep -v 'Detected strange initial WiFi'
#echo -----------------------------------------------------------------------------------------------------------------------------------
#zcat /var/log/userver-*.gz | grep --colour -i -E -e 'ERROR|ABORT|ASSERT|failed' | grep -v 'SSL EOF observed that violates the protocol'
#echo -----------------------------------------------------------------------------------------------------------------------------------
#zcat /var/log/wi-auth-logs-archives/*.gz | grep --colour -i -E -e 'ERROR|ABORT|ASSERT|SIGSEGV| diff|detected' | grep -v 'Detected strange initial WiFi'
#echo -----------------------------------------------------------------------------------------------------------------------------------
#echo '(gz) completed:' `zcat /var/log/userver-*.gz  2>/dev/null | grep ' completed' | wc -l` failed: `zcat /var/log/userver-*.gz  2>/dev/null | grep ' failed' | wc -l`
#echo '     completed:' ` cat /var/log/userver-*.log 2>/dev/null | grep ' completed' | wc -l` failed: ` cat /var/log/userver-*.log 2>/dev/null | grep ' failed' | wc -l`
#echo -----------------------------------------------------------------------------------------------------------------------------------
#for i in userver-firenze_*log.*.gz; do zcat $i | grep --colour -E -e ' failed| completed|partial response'; done 2>/dev/null
#echo -----------------------------------------------------------------------------------------------------------------------------------
#tail -f httpd/access_log | cgrep -e ^88.149.190.155
