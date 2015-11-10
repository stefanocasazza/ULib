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
#UMEMPOOL="52,0,0,39,8205,8205,-11,-20,22"
#UMEMPOOL="982,0,0,36,9846,-24,-23,1727,1151"
#sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET 0|g"									 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 16384|g"								 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|CLIENT_THRESHOLD .*|CLIENT_THRESHOLD 4000|g"							 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|CLIENT_FOR_PARALLELIZATION .*|CLIENT_FOR_PARALLELIZATION 8000|g" benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#Running 15s test @ http://localhost:8080/plaintext
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency     1.07ms  292.99us  17.09ms   83.28%
#    Req/Sec   509.25k    28.12k  568.89k    75.06%
#  Latency Distribution
#     50%    1.07ms
#     75%    1.22ms
#     90%    1.34ms
#     99%    1.53ms
#  29127120 requests in 15.00s, 3.91GB read
#Requests/sec: 1941819.65
#Transfer/sec:    266.67MB
#
# Running 15s test @ http://laptop:8080/plaintext
#   4 threads and 256 connections
#   Thread Stats   Avg      Stdev     Max   +/- Stdev
#     Latency   220.76ms   49.25ms 937.77ms   86.40%
#     Req/Sec     4.68k   355.58     5.69k    71.20%
#   Latency Distribution
#      50%  220.85ms
#      75%  235.07ms
#      90%  254.43ms
#      99%  353.92ms
#   279265 requests in 15.02s, 38.35MB read
# Requests/sec:  18588.96
# Transfer/sec:      2.55MB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# JSON
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
 UMEMPOOL="56,0,0,40,150,-24,-13,-20,0"
 sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET 0|g"									 benchmark/FrameworkBenchmarks/fbenchmark.cfg
 sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 256|g"									 benchmark/FrameworkBenchmarks/fbenchmark.cfg
 sed -i "s|CLIENT_THRESHOLD .*|CLIENT_THRESHOLD 50|g"								 benchmark/FrameworkBenchmarks/fbenchmark.cfg
 sed -i "s|CLIENT_FOR_PARALLELIZATION .*|CLIENT_FOR_PARALLELIZATION 100|g"  benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#Running 15s test @ http://localhost:8080/json
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency   454.29us  279.11us  21.29ms   95.78%
#    Req/Sec    74.50k     9.20k  134.33k    82.76%
#  Latency Distribution
#     50%  441.00us
#     75%  561.00us
#     90%  649.00us
#     99%    0.90ms
#  4203920 requests in 15.00s, 657.50MB read
#Requests/sec: 280264.58
#Transfer/sec:     43.83MB
#
#Running 15s test @ http://laptop:8080/json
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency    18.95ms   24.43ms 239.50ms   98.69%
#    Req/Sec     3.37k   647.94     5.19k    65.72%
#  Latency Distribution
#     50%   16.50ms
#     75%   17.92ms
#     90%   19.30ms
#     99%  225.25ms
#  199539 requests in 15.00s, 31.21MB read
#Requests/sec:  13299.02
#Transfer/sec:      2.08MB
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
#export REDIS_HOST=localhost
#UMEMPOOL="146,0,0,90,150,-22,-12,-20,0"
#sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET -2|g"									 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 256|g"									 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|CLIENT_THRESHOLD .*|CLIENT_THRESHOLD 80|g"								 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|CLIENT_FOR_PARALLELIZATION .*|CLIENT_FOR_PARALLELIZATION 100|g"  benchmark/FrameworkBenchmarks/fbenchmark.cfg
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
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/rquery.so; \
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
#pkill userver_tcp userver_ssl

 mv err/userver_tcp.err err/FrameworkBenchmarks.err
#mv err/userver_ssl.err err/benchmarking.err

echo "PID = `cat benchmark/FrameworkBenchmarks/ULib/userver_tcp.pid`"

# grep -v 'EAGAIN\|EPIPE\|ENOTCONN\|ECONNRESET' err/benchmarking.err 

# gprof -b ../../examples/userver/userver_tcp gmon.out >profile.out 2>/dev/null
