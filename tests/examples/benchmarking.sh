#!/bin/sh

. ../.function

DOC_ROOT=benchmark/docroot

rm -f err/benchmarking.err \
		benchmark/benchmarking.log* \
      out/userver_*.out err/userver_*.err \
					 trace.*userver_*.[0-9]*			  object.*userver_*.[0-9]*				 stack.*userver_*.[0-9]*			  mempool.*userver_*.[0-9]* \
      $DOC_ROOT/trace.*userver_*.[0-9]* $DOC_ROOT/object.*userver_*.[0-9]* $DOC_ROOT/stack.*userver_*.[0-9]* $DOC_ROOT/mempool.*userver_*.[0-9]*

#UTRACE="0 50M 1"
#UOBJDUMP="0 10M 100"
#USIMERR="error.sim"
#VALGRIND="valgrind -v --trace-children=yes"
UMEMPOOL=135,0,53,134,1057,-24,-18,-21,43
 export UTRACE UOBJDUMP USIMERR VALGRIND UMEMPOOL

ORM_DRIVER="sqlite"
ORM_OPTION="dbname=../db/hello_world"
 export ORM_DRIVER ORM_OPTION

DIR_CMD="../../examples/userver"

compile_usp

ulimit -n 100000
echo 0 > /proc/sys/net/ipv4/tcp_syncookies
# -----------------------------------------------------------------------------------------------------------------------------------------------------------
# cat /proc/sys/net/ipv4/tcp_mem
# ------------------------------
# 47556   63410   95112
# -------------------------------------------
# low threshold		  => 47556 * 4096 = 185M
# memory pressure		  => 63410 * 4096 = 247M
# Out of socket memory => 95112 * 4096 = 371M
# -------------------------------------------
# cat /proc/net/sockstat
# cat /proc/sys/net/ipv4/tcp_max_orphans => 131072
# -----------------------------------------------------------------------------------------------------------------------------------------------------------
# So in this case we have 21564 orphans. That doesn't seem very close to 65536... Yet, if you look once more at the code above that prints the warning,
# you'll see that there is this shift variable that has a value between 0 and 2, and that the check is testing (orphans << shift > sysctl_tcp_max_orphans).
# What this means is that in certain cases, the kernel decides to penalize some sockets more, and it does so by multiplying the number of orphans by 2x
# or 4x to artificially increase the "score" of the "bad socket" to penalize. The problem is that due to the way this is implemented, you can see a
# worrisome "Out of socket memory" error when in fact you're still 4x below the limit and you just had a couple "bad sockets" (which happens frequently
# when you have an Internet facing service). So unfortunately that means that you need to tune up the maximum number of orphan sockets even if you're
# 2x or 4x away from the threshold. What value is reasonable for you depends on your situation at hand. Observe how the count of orphans in
# /proc/net/sockstat is changing when your server is at peak traffic, multiply that value by 4, round it up a bit to have a nice value, and set it.
# You can set it by doing a echo of the new value in /proc/sys/net/ipv4/tcp_max_orphans, and don't forget to update the value of net.ipv4.tcp_max_orphans
# in /etc/sysctl.conf so that your change persists across reboots.
# -----------------------------------------------------------------------------------------------------------------------------------------------------------
# A server that uses SYN cookies doesn't have to drop connections when its SYN queue fills up.
# Instead it sends back a SYN+ACK, exactly as if the SYN queue had been larger.
# (Exceptions: the server must reject TCP options such as large windows, and it must use one of the
# eight MSS values that it can encode.) When the server receives an ACK, it checks that the secret
# function works for a recent value of t, and then rebuilds the SYN queue entry from the encoded MSS. 
# -----------------------------------------------------------------------------------------------------------------------------------------------------------
#echo   1024 > /proc/sys/net/core/somaxconn
#echo      1 > /proc/sys/net/ipv4/tcp_syncookies
#echo 524288 > /proc/sys/net/ipv4/tcp_max_orphans
#echo      2 > /proc/sys/net/ipv4/tcp_synack_retries # 5 -> 2 == 21 sec (Total time to keep half-open connections in the backlog queue)
#echo      0 > /proc/sys/kernel/printk_ratelimit # 5
#echo      0 > /proc/sys/kernel/printk_ratelimit_burst # 10
# -----------------------------------------------------------------------------------------------------------------------------------------------------------

#STRACE=$TRUSS
#VALGRIND="valgrind --tool=exp-dhat"
#MUDFLAP_OPTIONS="-ignore-reads  -backtrace=8"
 start_prg_background userver_tcp -c benchmark/benchmarking.cfg
#start_prg_background userver_ssl -c benchmark/benchmarking_ssl.cfg

#run command on another computer
#ab -n 100000 -c10 http://stefano/servlet/benchmarking?name=stefano
#ab -n 100000 -c10 https://stefano/servlet/benchmarking?name=stefano

#$SLEEP
#killall userver_tcp userver_ssl

 mv err/userver_tcp.err err/benchmarking.err
#mv err/userver_ssl.err err/benchmarking.err

echo "PID = `cat benchmark/userver_tcp.pid`"

# grep -v 'EAGAIN\|EPIPE\|ENOTCONN\|ECONNRESET' err/benchmarking.err 

# gprof -b ../../examples/userver/userver_tcp gmon.out >profile.out 2>/dev/null
