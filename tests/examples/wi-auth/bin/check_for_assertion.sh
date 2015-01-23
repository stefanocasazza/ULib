#/bin/bash

# check_for_assertion.sh

sync
echo -----------------------------------------------------------------------------------------------------------------------------------
grep --colour -i -E -e 'assert|OVERLAP|SIGSEGV|allocate' /tmp/userver-*.err
echo -----------------------------------------------------------------------------------------------------------------------------------
grep --colour -i -E -e 'ERROR|ABORT|ASSERT' /var/log/userver-*.log /var/log/uclient-*.log | grep -v 'SSL EOF observed that violates the protocol'
echo -----------------------------------------------------------------------------------------------------------------------------------
grep --colour -i -E -e 'Bad Req| diff|detected|File cached' /var/log/userver-*.log
echo -----------------------------------------------------------------------------------------------------------------------------------
#zcat /var/log/userver-*.gz | grep --colour -i -E -e 'ERROR|ABORT|ASSERT' | grep -v 'SSL EOF observed that violates the protocol'
echo -----------------------------------------------------------------------------------------------------------------------------------
zcat /var/log/wi-auth-logs-archives/*.gz | grep --colour -i -E -e 'ERROR|ABORT|ASSERT|SIGSEGV| diff|detected'
echo -----------------------------------------------------------------------------------------------------------------------------------
