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
#    Latency     0.91ms  208.81us  11.38ms   80.51%
#    Req/Sec   606.98k    39.03k  699.20k    71.14%
#  Latency Distribution
#     50%    0.90ms
#     75%    1.01ms
#     90%    1.11ms
#     99%    1.76ms
#  34226368 requests in 14.94s, 4.11GB read
#Requests/sec: 2290284.44
#Transfer/sec:    281.76MB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# JSON
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#UMEMPOOL="237,0,0,49,273,-15,-14,-20,36"
#sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET 0|g"	 benchmark/FrameworkBenchmarks/fbenchmark.cfg
#sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 256|g"	 benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#Running 15s test @ http://localhost:8080/json
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency   353.08us  168.28us   8.74ms   84.43%
#    Req/Sec    93.33k     8.35k  167.78k    68.27%
#  Latency Distribution
#     50%  327.00us
#     75%  451.00us
#     90%  516.00us
#     99%  745.00us
#  5280348 requests in 15.00s, 750.32MB read
#Requests/sec: 352026.18
#Transfer/sec:     50.02MB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
# DB
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#ORM_DRIVER="sqlite"
#ORM_OPTION="host=localhost dbname=../db/hello_world"
#ORM_DRIVER="mysql"
#ORM_OPTION="host=localhost user=benchmarkdbuser password=benchmarkdbpass character-set=utf8 dbname=hello_world"
 ORM_DRIVER="pgsql"
 ORM_OPTION="host=localhost user=benchmarkdbuser password=benchmarkdbpass dbname=hello_world"
 UMEMPOOL="581,0,0,66,16416,-7,-20,-23,31"
 sed -i "s|TCP_LINGER_SET .*|TCP_LINGER_SET -2|g"	benchmark/FrameworkBenchmarks/fbenchmark.cfg
 sed -i "s|LISTEN_BACKLOG .*|LISTEN_BACKLOG 256|g"	benchmark/FrameworkBenchmarks/fbenchmark.cfg
# ----------------------------------------------------------------------------------------------------------------------------------------------------------
#Running 15s test @ http://localhost:8080/fortune
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency     5.97ms    2.42ms  16.39ms   94.85%
#    Req/Sec     8.63k     2.15k   11.82k    92.32%
#  Latency Distribution
#     50%    5.21ms
#     75%    6.09ms
#     90%    6.47ms
#     99%   16.39ms
#  520090 requests in 16.00s, 674.06MB read
#Requests/sec:  32502.85
#Transfer/sec:     42.13MB
#
#Running 15s test @ http://localhost:8080/db
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency     5.59ms    1.04ms  11.43ms   78.71%
#    Req/Sec     9.25k     1.44k   12.09k    90.49%
#  Latency Distribution
#     50%    5.69ms
#     75%    6.05ms
#     90%    6.39ms
#     99%   11.05ms
#  539816 requests in 16.01s, 78.25MB read
#Requests/sec:  33720.45
#Transfer/sec:      4.89MB
#
#Running 15s test @ http://localhost:8080/query?queries=20
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency    30.71ms    1.41ms  37.71ms   75.83%
#    Req/Sec     1.34k   355.28     2.00k    62.54%
#  Latency Distribution
#     50%   30.01ms
#     75%   31.05ms
#     90%   33.01ms
#     99%   34.28ms
#  59809 requests in 16.02s, 43.63MB read
#Requests/sec:   3734.19
#Transfer/sec:      2.72MB
#
#Running 15s test @ http://localhost:8080/update?queries=20
#  4 threads and 256 connections
#  Thread Stats   Avg      Stdev     Max   +/- Stdev
#    Latency   979.35ms  123.76ms   1.23s    74.15%
#    Req/Sec    37.42     25.76    70.00     52.78%
#  Latency Distribution
#     50%  954.28ms
#     75%    1.03s 
#     90%    1.23s 
#     99%    1.23s 
#  1263 requests in 16.01s, 0.92MB read
#Requests/sec:     78.89
#Transfer/sec:     58.70KB
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
