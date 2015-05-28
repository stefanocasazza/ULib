#!/bin/sh

. ../.function

DOC_ROOT=benchmark/FrameworkBenchmarks/ULib/www

rm -f benchmark/FrameworkBenchmarks/benchmark.log* \
		/tmp/*.memusage.* /tmp/request* /tmp/response* \
		err/FrameworkBenchmarks.err out/userver_*.out err/userver_*.err \
					 trace.*userver_*.[0-9]*			  object.*userver_*.[0-9]*				 stack.*userver_*.[0-9]*			  mempool.*userver_*.[0-9]* \
      $DOC_ROOT/trace.*userver_*.[0-9]* $DOC_ROOT/object.*userver_*.[0-9]* $DOC_ROOT/stack.*userver_*.[0-9]* $DOC_ROOT/mempool.*userver_*.[0-9]*

#UTRACE="0 100M 0"
#UOBJDUMP="0 10M 100"
#USIMERR="error.sim"
#VALGRIND="valgrind -v --trace-children=yes"
 UMEMUSAGE=yes
 export UTRACE UOBJDUMP USIMERR VALGRIND UMEMUSAGE

unset  ORM_DRIVER ORM_OPTION
export ORM_DRIVER ORM_OPTION UMEMPOOL
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# PLAINTEXT
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#UMEMPOOL="982,0,0,36,9846,-24,-23,1727,1151"
#sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET 0|g"									 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 16384|g"								 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|CLIENT_THRESHOLD .*|CLIENT_THRESHOLD 4000|g"							 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|CLIENT_FOR_PARALLELIZATION .*|CLIENT_FOR_PARALLELIZATION 8000|g" benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# Running 15s test @ http://laptop:8080/plaintext
#   4 threads and 16384 connections
#   Thread Stats   Avg      Stdev     Max   +/- Stdev
#     Latency    13.30s     2.61s   14.43s    87.33%
#     Req/Sec     3.06k     0.94k    5.07k    62.22%
#   Latency Distribution
#      50%   14.38s 
#      75%   14.43s 
#      90%   14.43s 
#      99%   14.43s 
#   174245 requests in 15.00s, 23.93MB read
#   Socket errors: connect 0, read 143, write 0, timeout 5565
# Requests/sec: 11613.20
# Transfer/sec: 1.60MB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# JSON
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
 UMEMPOOL="56,0,0,40,150,-24,-13,-20,0"
 sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET 0|g"									 benchmark/FrameworkBenchmarks/fbenchmark.cfg
 sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 256|g"									 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|CLIENT_THRESHOLD .*|CLIENT_THRESHOLD 50|g"								 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|CLIENT_FOR_PARALLELIZATION .*|CLIENT_FOR_PARALLELIZATION 100|g"  benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# Running 15s test @ http://laptop:8080/json
#   4 threads and 256 connections
#   Thread Stats   Avg      Stdev     Max   +/- Stdev
#     Latency    18.26ms    3.84ms 234.44ms   91.85%
#     Req/Sec     3.48k   490.60     4.95k    67.32%
#   Latency Distribution
#      50%   18.56ms
#      75%   19.81ms
#      90%   21.00ms
#      99%   23.05ms
#   205824 requests in 15.01s, 32.19MB read
# Requests/sec: 13716.68
# Transfer/sec: 2.15MB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# DB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#ORM_DRIVER="sqlite"
#ORM_OPTION="host=localhost dbname=../db/hello_world"
#ORM_DRIVER="mysql"
#ORM_OPTION="host=localhost user=benchmarkdbuser password=benchmarkdbpass character-set=utf8 dbname=hello_world"
#ORM_DRIVER="pgsql"
#ORM_OPTION="host=localhost user=benchmarkdbuser password=benchmarkdbpass dbname=hello_world"
#UMEMPOOL="146,0,0,90,150,-22,-12,-20,0"
#sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET -2|g"									 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 256|g"									 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|CLIENT_THRESHOLD .*|CLIENT_THRESHOLD 80|g"								 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|CLIENT_FOR_PARALLELIZATION .*|CLIENT_FOR_PARALLELIZATION 100|g"  benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# Running 15s test @ http://laptop:8080/fortune
#   4 threads and 256 connections
#   Thread Stats   Avg      Stdev     Max   +/- Stdev
#     Latency    53.07ms   28.32ms 155.09ms   65.25%
#     Req/Sec     1.22k   164.62     1.57k    72.42%
#   Latency Distribution
#      50%   51.15ms
#      75%   72.30ms
#      90%   88.69ms
#      99%  134.90ms
#   72743 requests in 15.00s, 94.28MB read
# Requests/sec: 4849.05
# Transfer/sec: 6.28MB
#
# Running 15s test @ http://laptop:8080/db
#   4 threads and 256 connections
#   Thread Stats   Avg      Stdev     Max   +/- Stdev
#     Latency    18.44ms    3.19ms  29.77ms   87.44%
#     Req/Sec     3.45k   439.79     4.98k    78.05%
#   Latency Distribution
#      50%   18.80ms
#      75%   20.13ms
#      90%   21.27ms
#      99%   23.43ms
#   203711 requests in 15.00s, 32.60MB read
# Requests/sec: 13583.13
# Transfer/sec: 2.17MB
#
# Running 15s test @ http://laptop:8080/query?queries=20
#   4 threads and 256 connections
#   Thread Stats   Avg      Stdev     Max   +/- Stdev
#     Latency    31.59ms    5.02ms  52.48ms   68.50%
#     Req/Sec     2.03k   167.87     2.45k    64.85%
#   Latency Distribution
#      50%   31.69ms
#      75%   34.52ms
#      90%   38.22ms
#      99%   43.36ms
#   121069 requests in 15.00s, 89.44MB read
# Requests/sec: 8072.57
# Transfer/sec: 5.96MB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
DIR_CMD="../../examples/userver"

prepare_usp

mkdir -p $DOC_ROOT

# ln -sf ../../../docroot/ws; \
# ln -sf ../../../docroot/servlet; \
# ln -sf ../../../docroot/100.html; \
# ln -sf ../../../docroot/1000.html; \

if [ "$TERM" != "cygwin" ]; then
	( cd $DOC_ROOT; rm -f *; \
	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/json.so; \
	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/plaintext.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/db.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/update.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/query.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/fortune.so )
fi

# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#ulimit -n 100000
#echo 0 > /proc/sys/net/ipv4/tcp_syncookies
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
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
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# So in this case we have 21564 orphans. That doesn't seem very close to 65536... Yet, if you look once more at the code above that prints the warning,
# you'll see that there is this shift variable that has a value between 0 and 2, and that the check is testing if
# (orphans << shift > sysctl_tcp_max_orphans). What this means is that in certain cases, the kernel decides to penalize some sockets more, and it does
# so by multiplying the number of orphans by 2x or 4x to artificially increase the "score" of the "bad socket" to penalize. The problem is that due to
# the way this is implemented, you can see a worrisome "Out of socket memory" error when in fact you're still 4x below the limit and you just had a
# couple "bad sockets" (which happens frequently when you have an Internet facing service). So unfortunately that means that you need to tune up the
# maximum number of orphan sockets even if you're 2x or 4x away from the threshold. What value is reasonable for you depends on your situation at hand.
# Observe how the count of orphans in /proc/net/sockstat is changing when your server is at peak traffic, multiply that value by 4, round it up a bit
# to have a nice value, and set it. You can set it by doing a echo of the new value in /proc/sys/net/ipv4/tcp_max_orphans, and don't forget to update
# the value of net.ipv4.tcp_max_orphans in /etc/sysctl.conf so that your change persists across reboots.
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# A server that uses SYN cookies doesn't have to drop connections when its SYN queue fills up.
# Instead it sends back a SYN+ACK, exactly as if the SYN queue had been larger.
# (Exceptions: the server must reject TCP options such as large windows, and it must use one of the
# eight MSS values that it can encode.) When the server receives an ACK, it checks that the secret
# function works for a recent value of t, and then rebuilds the SYN queue entry from the encoded MSS. 
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#echo   1024 > /proc/sys/net/core/somaxconn
#echo      1 > /proc/sys/net/ipv4/tcp_syncookies
#echo 524288 > /proc/sys/net/ipv4/tcp_max_orphans
#echo      2 > /proc/sys/net/ipv4/tcp_synack_retries # 5 -> 2 == 21 sec (Total time to keep half-open connections in the backlog queue)
#echo      0 > /proc/sys/kernel/printk_ratelimit # 5
#echo      0 > /proc/sys/kernel/printk_ratelimit_burst # 10
# --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#STRACE=$TRUSS
#VALGRIND="valgrind --tool=exp-dhat"
#MUDFLAP_OPTIONS="-ignore-reads  -backtrace=8"
echo 16384 > /proc/sys/net/core/somaxconn

start_prg_background userver_tcp -c benchmark/FrameworkBenchmarks/fbenchmark.cfg

#run command on another computer
#ab -n 100000 -c10 http://stefano/servlet/benchmarking?name=stefano
#ab -n 100000 -c10 https://stefano/servlet/benchmarking?name=stefano

#$SLEEP
#killall userver_tcp userver_ssl

 mv err/userver_tcp.err err/FrameworkBenchmarks.err
#mv err/userver_ssl.err err/benchmarking.err

echo "PID = `cat benchmark/FrameworkBenchmarks/ULib/userver_tcp.pid`"

# grep -v 'EAGAIN\|EPIPE\|ENOTCONN\|ECONNRESET' err/benchmarking.err 

# gprof -b ../../examples/userver/userver_tcp gmon.out >profile.out 2>/dev/null
