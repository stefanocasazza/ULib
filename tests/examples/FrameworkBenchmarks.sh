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
#UMEMUSAGE=yes
 export UTRACE UOBJDUMP USIMERR VALGRIND UMEMUSAGE

unset  ORM_DRIVER ORM_OPTION
export ORM_DRIVER ORM_OPTION UMEMPOOL
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# PLAINTEXT
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#UMEMPOOL="58,0,0,41,16401,-14,-15,11,25"
#sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET 0|g"		 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 16384|g"	 benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#Running 15s test @ http://localhost:8080/plaintext
#  4 threads and 256 connections
# Thread Stats   Avg      Stdev     Max   +/- Stdev
#   Latency     1.86ms    2.30ms  28.08ms   88.21%
#   Req/Sec   552.42k   200.56k    1.39M    68.22%
# Latency Distribution
#    50%    1.00ms
#    75%    2.46ms
#    90%    4.53ms
#    99%   11.03ms
# 30638208 requests in 14.99s, 3.68GB read
#Requests/sec: 2044562.05
#Transfer/sec:    251.53MB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# JSON
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#UMEMPOOL="58,0,0,41,273,-15,-14,-20,36"
#sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET 0|g"	 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 256|g"	 benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#Running 15s test @ http://localhost:8080/json
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency     0.90ms    2.52ms  36.05ms   93.49%
#    Req/Sec    89.35k    47.88k  229.78k    65.55%
#  Latency Distribution
#     50%  166.00us
#     75%  408.00us
#     90%    2.27ms
#     99%   11.68ms
#  4845404 requests in 15.00s, 688.52MB read
#Requests/sec: 323059.07
#Transfer/sec:     45.91MB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# DB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
 ORM_DRIVER="sqlite"
 ORM_OPTION="host=localhost dbname=../db/hello_world"
#ORM_DRIVER="mysql"
#ORM_OPTION="host=localhost user=benchmarkdbuser password=benchmarkdbpass character-set=utf8 dbname=hello_world"
#ORM_DRIVER="pgsql"
#ORM_OPTION="host=localhost user=benchmarkdbuser password=benchmarkdbpass dbname=hello_world"
 UMEMPOOL="545,0,0,49,275,-14,-13,-25,41"
 sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET -2|g"	benchmark/FrameworkBenchmarks/fbenchmark.cfg
 sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 256|g"	benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#Running 15s test @ http://localhost:8080/fortune
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency     0.97ms  444.01us   7.95ms   66.95%
#    Req/Sec    35.76k     4.13k   53.33k    62.05%
#  Latency Distribution
#     50%    0.93ms
#     75%    1.26ms
#     90%    1.56ms
#     99%    2.16ms
#  2023409 requests in 15.00s, 2.53GB read
#Requests/sec: 134905.88
#Transfer/sec:    172.91MB
#
#Running 15s test @ http://localhost:8080/db
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency   663.80us  318.91us  22.24ms   74.68%
#    Req/Sec    51.90k     4.80k   69.00k    76.78%
#  Latency Distribution
#     50%  631.00us
#     75%  839.00us
#     90%    1.04ms
#     99%    1.47ms
#  2917228 requests in 15.00s, 425.05MB read
#Requests/sec: 194484.25
#Transfer/sec:     28.34MB
#
#Running 15s test @ http://localhost:8080/query?queries=20
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency     2.44ms    1.20ms  32.83ms   68.96%
#    Req/Sec    14.40k     2.68k   24.89k    68.36%
#  Latency Distribution
#     50%    2.31ms
#     75%    3.16ms
#     90%    4.05ms
#     99%    5.66ms
#  817586 requests in 15.00s, 592.28MB read
#Requests/sec:  54503.98
#Transfer/sec:     39.48MB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#export REDIS_HOST=localhost
#UMEMPOOL="1261,0,0,49,274,-14,-15,-24,40"
#sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET -2|g"	benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 256|g"	benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#Running 15s test @ http://localhost:8080/rquery?queries=20
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency     6.26ms    1.50ms  12.84ms   58.54%
#    Req/Sec    10.58k   651.81    13.07k    69.19%
#  Latency Distribution
#     50%    6.47ms
#     75%    7.63ms
#     90%    8.26ms
#     99%    8.67ms
#  613843 requests in 15.00s, 444.69MB read
#Requests/sec:  40924.11
#Transfer/sec:     29.65MB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#export MONGODB_HOST=localhost
#UMEMPOOL="1057,0,0,49,274,-14,-15,-24,40"
#sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET -2|g"	benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 256|g"	benchmark/FrameworkBenchmarks/fbenchmark.cfg
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
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/rdb.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/mdb.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/update.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/rupdate.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/mupdate.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/query.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/rquery.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/mquery.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/fortune.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/rfortune.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/mfortune.so )
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

wait_server_ready localhost 8080

#run command on another computer
#ab -n 100000 -c10 http://stefano/servlet/benchmarking?name=stefano
#ab -n 100000 -c10 https://stefano/servlet/benchmarking?name=stefano

echo "PID = `cat benchmark/FrameworkBenchmarks/ULib/userver_tcp.pid`"

#$SLEEP
#kill_server userver_tcp

 mv err/userver_tcp.err err/FrameworkBenchmarks.err
#mv err/userver_ssl.err err/benchmarking.err

#grep -v 'EAGAIN\|EPIPE\|ENOTCONN\|ECONNRESET' err/benchmarking.err 

#gprof -b ../../examples/userver/userver_tcp gmon.out >profile.out 2>/dev/null
