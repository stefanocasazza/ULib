#!/bin/sh

. ../.function

DOC_ROOT=benchmark/FrameworkBenchmarks/ULib/www

rm -f benchmark/FrameworkBenchmarks/benchmark.log* \
		/tmp/*.memusage.* /tmp/request* /tmp/response* \
		err/FrameworkBenchmarks.err out/userver_*.out err/userver_*.err \
					 trace.*userver_*.[0-9]*			  object.*userver_*.[0-9]*				 stack.*userver_*.[0-9]*			  mempool.*userver_*.[0-9]* \
      $DOC_ROOT/trace.*userver_*.[0-9]* $DOC_ROOT/object.*userver_*.[0-9]* $DOC_ROOT/stack.*userver_*.[0-9]* $DOC_ROOT/mempool.*userver_*.[0-9]*

#UTRACE="0 50M 0"
 UTRACE_SIGNAL="0 50M -1"
 UTRACE_FOLDER=/tmp
 TMPDIR=/tmp
#UOBJDUMP="0 10M 100"
#USIMERR="error.sim"
#UMEMUSAGE=yes
#VALGRIND="valgrind -v --trace-children=yes"
export UTRACE UOBJDUMP USIMERR UTRACE_SIGNAL UMEMUSAGE VALGRIND UTRACE_FOLDER TMPDIR

unset  ORM_DRIVER ORM_OPTION
export ORM_DRIVER ORM_OPTION UMEMPOOL
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# PLAINTEXT
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#UMEMPOOL="84,0,0,41,16401,-14,-15,11,25"
#sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET 0|g"	  benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 16384|g" benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#Running 15s test @ http://localhost:8080/plaintext
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency     0.91ms  353.79us  19.39ms   96.10%
#    Req/Sec   609.19k    62.71k  808.89k    77.44%
#  Latency Distribution
#     50%    0.88ms
#     75%    0.97ms
#     90%    1.06ms
#     99%    1.90ms
#  34449216 requests in 14.95s, 4.14GB read
#Requests/sec: 2303880.07
#Transfer/sec:    283.43MB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# JSON
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
 UMEMPOOL="237,0,0,49,273,-15,-14,-20,36"
 sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET 0|g"	 benchmark/FrameworkBenchmarks/fbenchmark.cfg
 sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 256|g"	 benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#Running 15s test @ http://localhost:8080/json
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency   416.42us  683.05us  22.19ms   98.26%
#    Req/Sec    89.37k    17.73k  170.55k    80.41%
#  Latency Distribution
#     50%  339.00us
#     75%  452.00us
#     90%  580.00us
#     99%    1.53ms
#  5037291 requests in 15.00s, 715.79MB read
#Requests/sec: 335816.85
#Transfer/sec:     47.72MB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# DB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#ORM_DRIVER="sqlite"
#ORM_OPTION="host=localhost dbname=../db/hello_world"
#ORM_DRIVER="mysql"
#ORM_OPTION="host=localhost user=benchmarkdbuser password=benchmarkdbpass character-set=utf8 dbname=hello_world"
#ORM_DRIVER="pgsql"
#ORM_OPTION="host=localhost user=benchmarkdbuser password=benchmarkdbpass dbname=hello_world"
#UMEMPOOL="581,0,0,66,16416,-7,-20,-23,31"
#sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET -2|g"	benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 256|g"	benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#Running 15s test @ http://localhost:8080/cached_worlds?queries=20
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency     1.10ms    3.00ms  33.28ms   93.31%
#    Req/Sec    91.87k    45.63k  204.00k    69.59%
#  Latency Distribution
#     50%  211.00us
#     75%  600.00us
#     90%    1.71ms
#     99%   18.38ms
#  5137994 requests in 15.00s, 3.65GB read
#Requests/sec: 342537.14
#Transfer/sec:    248.92MB
#Running 15s test @ http://localhost:8080/fortune
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency     4.19ms    1.06ms  22.93ms   78.97%
#    Req/Sec    13.88k     2.24k   19.07k    58.09%
#  Latency Distribution
#     50%    3.96ms
#     75%    4.61ms
#     90%    5.42ms
#     99%    8.84ms
#  807248 requests in 15.00s, 1.02GB read
#Requests/sec:  53817.00
#Transfer/sec:     69.75MB
#Running 15s test @ http://localhost:8080/db
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency     2.42ms    0.96ms  22.48ms   80.96%
#    Req/Sec    20.47k    10.97k   36.33k    53.29%
#  Latency Distribution
#     50%    2.25ms
#     75%    2.41ms
#     90%    4.00ms
#     99%    5.16ms
#  870010 requests in 16.03s, 126.95MB read
#Requests/sec:  54280.30
#Transfer/sec:      7.92MB
#Running 15s test @ http://localhost:8080/query?queries=20
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency    19.08ms    2.83ms  69.17ms   91.24%
#    Req/Sec     2.07k     1.12k    3.34k    42.54%
#  Latency Distribution
#     50%   18.37ms
#     75%   20.93ms
#     90%   21.66ms
#     99%   26.06ms
#  60493 requests in 16.02s, 43.96MB read
#Requests/sec:   3775.05
#Transfer/sec:      2.74MB
#Running 15s test @ http://localhost:8080/update?queries=20
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency   961.45ms  120.92ms   1.33s    72.70%
#    Req/Sec    41.03      9.16    54.00     58.06%
#  Latency Distribution
#     50%  954.03ms
#     75%    1.03s 
#     90%    1.11s 
#     99%    1.28s 
#  1232 requests in 16.03s, 0.90MB read
#Requests/sec:     76.85
#Transfer/sec:     57.32KB
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

#prepare_usp

mkdir -p $DOC_ROOT

# ln -sf ../../../docroot/ws; \
# ln -sf ../../../docroot/servlet; \
# ln -sf ../../../docroot/100.html; \
# ln -sf ../../../docroot/1000.html; \
# ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/rdb.so; \
# ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/mdb.so; \
# ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/rupdate.so; \
# ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/mupdate.so; \
# ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/rquery.so; \
# ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/mquery.so; \
# ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/rfortune.so; \
# ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/mfortune.so;

#								make json.la plaintext.la
#AM_LDFLAGS="-lWorld"   make db.la query.la update.la cached_worlds.la
#AM_LDFLAGS="-lFortune" make fortune.la

if [ "$TERM" != "cygwin" ]; then
	( cd $DOC_ROOT; rm -f *; \
	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/json.so; \
	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/plaintext.so; \
	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/cached_worlds.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/query.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/db.so; \
 	  ln -sf ../../../../../../src/ulib/net/server/plugin/usp/.libs/update.so; \
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
