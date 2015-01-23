#!/bin/sh

. ../.function

rm -f hello_world.log* err/hello_world.err \
      out/userver_tcp.out err/userver_tcp.err \
      out/userver_ssl.out err/userver_ssl.err \
		trace.*userver_tcp*.[0-9]* object.*userver_tcp*.[0-9]* stack.*userver_tcp*.[0-9]* \
		trace.*userver_ssl*.[0-9]* object.*userver_ssl*.[0-9]* stack.*userver_ssl*

#UTRACE="0 30M 0"
#UOBJDUMP="0 100k 10"
#USIMERR="error.sim"
 export UTRACE UOBJDUMP USIMERR

DIR_CMD="../../examples/userver"

compile_usp

# A server that uses SYN cookies doesn't have to drop connections when its SYN queue fills up.
# Instead it sends back a SYN+ACK, exactly as if the SYN queue had been larger.
# (Exceptions: the server must reject TCP options such as large windows, and it must use one of the
# eight MSS values that it can encode.) When the server receives an ACK, it checks that the secret
# function works for a recent value of t, and then rebuilds the SYN queue entry from the encoded MSS. 

#ulimit -n 100000
#echo 1024 > /proc/sys/net/core/somaxconn
#echo    1 > /proc/sys/net/ipv4/tcp_syncookies
#echo    2 > /proc/sys/net/ipv4/tcp_synack_retries # 5 -> 2 == 21 sec (Total time to keep half-open connections in the backlog queue)
#echo    0 > /proc/sys/kernel/printk_ratelimit # 5
#echo    0 > /proc/sys/kernel/printk_ratelimit_burst # 10

#STRACE=$TRUSS
#VALGRIND="valgrind --tool=exp-dhat"
#MUDFLAP_OPTIONS="-ignore-reads  -backtrace=8"
 start_prg_background userver_tcp -c benchmark/benchmarking.cfg
#start_prg_background userver_ssl -c benchmark/benchmarking_ssl.cfg

# run command on another computer
# ab -f SSL3 -n 100000 -c10 -t 1 https://stefano/usp/hello_world.usp

#$SLEEP
#killall userver_tcp userver_ssl

 mv err/userver_tcp.err err/hello_world.err
#mv err/userver_ssl.err err/hello_world.err

# gprof -b ../../examples/userver/userver_tcp gmon.out >profile.out 2>/dev/null
