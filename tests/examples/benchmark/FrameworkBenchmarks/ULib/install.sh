#!/bin/bash

DIR=FrameworkBenchmarks

ULIB_VERSION=1.4.2
IROOT=/mnt/data/$DIR/installs
ULIB_ROOT=$IROOT/ULib
ULIB_DOCUMENT_ROOT=$ULIB_ROOT/ULIB_DOCUMENT_ROOT

rm -rf   $ULIB_ROOT
mkdir -p $ULIB_ROOT
mkdir -p $ULIB_DOCUMENT_ROOT

cat <<EOF >$ULIB_ROOT/benchmark.cfg
userver {
PORT 8080
PREFORK_CHILD 4 
TCP_LINGER_SET 0 
LISTEN_BACKLOG 16384
CLIENT_FOR_PARALLELIZATION 100
ORM_DRIVER "mysql pgsql sqlite"
DOCUMENT_ROOT $ULIB_DOCUMENT_ROOT
#LOG_FILE $ULIB_ROOT/web_server.log
#LOG_FILE_SZ 10M
#LOG_MSG_SIZE -1
}
EOF

build_userver()
{
	DIR=`pwd`

	LIBS=-lgcov \
	CPPFLAGS="-fprofile-generate" \
	USP_FLAGS="-DAS_cpoll_cppsp_DO" \
	./configure --prefix=$ULIB_ROOT \
   --disable-static --disable-examples \
   --with-mysql --with-pgsql --with-sqlite3 \
   --without-ssl --without-pcre --without-expat \
   --without-libz --without-libuuid --without-magic --without-libares \
   --enable-static-orm-driver='mysql pgsql sqlite' --enable-static-server-plugin=http \
	--with-mongodb --with-mongodb-includes="-I$IROOT/include/libbson-1.0 -I$IROOT/include/libmongoc-1.0" --with-mongodb-ldflags="-L$IROOT"
#  --enable-debug \
#	USP_LIBS="-ljson" \
#	cp $TROOT/src/* src/ulib/net/server/plugin/usp
#	cp -f config.cache ..

	make -j "$(nproc)"
	cd examples/userver
	make -j "$(nproc)"

	# Never use setcap inside of TRAVIS
	[ "$TRAVIS" != "true" ] || { \
	if [ `ulimit -r` -eq 99 ]; then
		sudo setcap cap_sys_nice,cap_sys_resource,cap_net_bind_service,cap_net_raw+eip .libs/userver_tcp
	fi
	}

	# Compile usp pages (no more REDIS)
	cd ../../src/ulib/net/server/plugin/usp
	make -j "$(nproc)" json.la plaintext.la db.la query.la update.la fortune.la \
                      mdb.la mquery.la mupdate.la mfortune.la
#                     rdb.la rquery.la rupdate.la rfortune.la

	# Check that compilation worked
	if [ ! -e .libs/db.so ]; then
		exit 1
	fi

	USP_DIR=`pwd`

	cd $ULIB_DOCUMENT_ROOT

   ln -sf $USP_DIR/.libs/plaintext.so
   ln -sf $USP_DIR/.libs/json.so
   ln -sf $USP_DIR/.libs/db.so
#  ln -sf $USP_DIR/.libs/rdb.so
#  ln -sf $USP_DIR/.libs/mdb.so
   ln -sf $USP_DIR/.libs/update.so
#  ln -sf $USP_DIR/.libs/rupdate.so
#  ln -sf $USP_DIR/.libs/mupdate.so
   ln -sf $USP_DIR/.libs/query.so
#  ln -sf $USP_DIR/.libs/rquery.so
#  ln -sf $USP_DIR/.libs/mquery.so
   ln -sf $USP_DIR/.libs/fortune.so
#  ln -sf $USP_DIR/.libs/rfortune.so
#  ln -sf $USP_DIR/.libs/mfortune.so

	cd $DIR
}

install_userver()
{
 	cp src/ulib/.libs/*.gcda							  src/ulib
 	cp src/ulib/net/server/plugin/usp/.libs/*.gcda src/ulib/net/server/plugin/usp

	make clean

 	mkdir	src/ulib/.libs src/ulib/net/server/plugin/usp/.libs

 	cp src/ulib/*.gcda							  src/ulib/.libs
 	cp src/ulib/net/server/plugin/usp/*.gcda src/ulib/net/server/plugin/usp/.libs

	CPPFLAGS="-fprofile-use" \
	USP_FLAGS="-DAS_cpoll_cppsp_DO" \
	./configure --prefix=$ULIB_ROOT \
   --disable-static --disable-examples \
   --with-mysql --with-pgsql --with-sqlite3 \
   --without-ssl --without-pcre --without-expat \
   --without-libz --without-libuuid --without-magic --without-libares \
   --enable-static-orm-driver='mysql pgsql sqlite' --enable-static-server-plugin=http \
	--with-mongodb --with-mongodb-includes="-I$IROOT/include/libbson-1.0 -I$IROOT/include/libmongoc-1.0" --with-mongodb-ldflags="-L$IROOT"

	make -j "$(nproc)" install
	cd examples/userver
	make -j "$(nproc)" install

	# Compile usp pages (no more REDIS)
	cd ../../src/ulib/net/server/plugin/usp
	make -j "$(nproc)" json.la plaintext.la db.la query.la update.la fortune.la \
                      mdb.la mquery.la mupdate.la mfortune.la
#                     rdb.la rquery.la rupdate.la rfortune.la

	# Check that compilation worked
	if [ ! -e .libs/db.so ]; then
		exit 1
	fi

	rm -rf $ULIB_DOCUMENT_ROOT/*
	cp .libs/json.so .libs/plaintext.so \
		.libs/db.so   .libs/query.so  .libs/update.so  .libs/fortune.so \
		.libs/mdb.so  .libs/mquery.so .libs/mupdate.so .libs/mfortune.so $ULIB_DOCUMENT_ROOT
#		.libs/rdb.so  .libs/rquery.so .libs/rupdate.so .libs/rfortune.so \

	# Never use setcap inside of TRAVIS 
	[ "$TRAVIS" != "true" ] || { \
	if [ `ulimit -r` -eq 99 ]; then
		sudo setcap cap_sys_nice,cap_sys_resource,cap_net_bind_service,cap_net_raw+eip $IROOT/ULib/bin/userver_tcp
	fi
	}
}

ULIB_PROFILE_URL="http://127.0.0.1:8080"

run_curl()
{
	for i in $(seq 1 10); do
		curl "${ULIB_PROFILE_URL}/$1" > /dev/null 2>&1
	done
}

generate_profile_data()
{
	cd examples/userver

	export ORM_DRIVER="sqlite"
	export ORM_OPTION="host=localhost user=benchmarkdbuser password=benchmarkdbpass character-set=utf8 dbname=${IROOT}/ULib/db/%.*s"
	export UMEMPOOL="545,0,0,49,275,-14,-13,-25,41"

	./userver_tcp -c $ULIB_ROOT/benchmark.cfg &

	local ULIB_PROFILE_PID=$!

	while ! curl ${ULIB_PROFILE_URL} > /dev/null 2>&1; do sleep 1; done

	run_curl plaintext 
	run_curl json 
 	run_curl db
 	run_curl query?queries=20
 	run_curl fortune
#	run_curl update?queries=20

	sleep 1
	kill -s TERM $ULIB_PROFILE_PID 2>/dev/null
	sleep 1
	kill -s TERM $ULIB_PROFILE_PID 2>/dev/null
	wait			 $ULIB_PROFILE_PID 2>/dev/null

	cd ../..
}

rm -rf ULib-1.4.2
tar xf /mnt/storage/ULib-1.4.2.tar.gz
cd ULib-$ULIB_VERSION
#cp -f ../config.cache .

cp -r tests/examples/benchmark/FrameworkBenchmarks/ULib/db $ULIB_ROOT

build_userver
generate_profile_data
install_userver

cat <<EOF >$IROOT/ulib.installed
export ULIB_ROOT=${ULIB_ROOT}
export ULIB_VERSION=${ULIB_VERSION}
export ULIB_DOCUMENT_ROOT=${ULIB_DOCUMENT_ROOT}
export PATH=${ULIB_ROOT}/bin:${PATH}
EOF
