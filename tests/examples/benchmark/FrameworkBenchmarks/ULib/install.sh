#!/bin/bash

DIR=FrameworkBenchmarks

ULIB_VERSION=1.4.2
IROOT=/mnt/data/$DIR/installs
ULIB_ROOT=$IROOT/ULib
ULIB_DOCUMENT_ROOT=$ULIB_ROOT/ULIB_DOCUMENT_ROOT

rm -rf $ULIB_ROOT
mkdir -p $ULIB_ROOT

if [ ! -f "$ULIB_ROOT/benchmark.cfg" ]; then
  cat <<EOF >$ULIB_ROOT/benchmark.cfg
userver {
 PORT 8080
 PREFORK_CHILD 4
 TCP_LINGER_SET -1
 LISTEN_BACKLOG 256
 ORM_DRIVER "mysql pgsql sqlite"
 DOCUMENT_ROOT $ULIB_DOCUMENT_ROOT
}
EOF
fi

#wget -O ULib-${ULIB_VERSION}.tar.gz https://github.com/stefanocasazza/ULib/archive/v${ULIB_VERSION}.tar.gz
#tar  xf ULib-${ULIB_VERSION}.tar.gz
cd       ULib-${ULIB_VERSION}

#rm -f config.cache ../config.cache
cp ../config.cache .

USP_FLAGS="-DAS_cpoll_cppsp_DO" \
./configure -C --prefix=$ULIB_ROOT \
  --enable-debug --enable-HCRS --enable-HPRS --disable-CRPWS --disable-check-time --disable-HIS --disable-log --disable-GSDS --disable-alias --disable-HSTS \
  --disable-static --disable-examples \
  --without-libtdb --without-libzopfli \
  --with-mysql --with-pgsql --with-sqlite3 --with-mongodb \
  --without-ssl --without-pcre --without-expat \
  --without-libz --without-libuuid --without-magic --without-libares \
  --enable-static-orm-driver='mysql pgsql sqlite' --enable-static-server-plugin=http \
  	--with-mongodb --with-mongodb-includes="-I$IROOT/include/libbson-1.0 -I$IROOT/include/libmongoc-1.0" --with-mongodb-ldflags="-L$IROOT"
# --enable-debug --enable-HCRS --enable-HPRS --disable-CRPWS --disable-check-time --disable-HIS --disable-log --disable-GSDS --disable-alias --disable-HSTS \
#USP_LIBS="-ljson" \

cp config.cache ..

make -j4 install
cp -r tests/examples/benchmark/FrameworkBenchmarks/ULib/db $ULIB_ROOT

cd examples/userver
make clean
make install

cd ../../src/ulib/net/server/plugin/usp
make json.la plaintext.la db.la query.la update.la fortune.la rdb.la rquery.la rupdate.la rfortune.la mdb.la mquery.la mupdate.la mfortune.la

mkdir -p $ULIB_DOCUMENT_ROOT
cp .libs/json.so .libs/plaintext.so \
   .libs/db.so  .libs/query.so  .libs/update.so  .libs/fortune.so \
   .libs/rdb.so .libs/rquery.so .libs/rupdate.so .libs/rfortune.so \
   .libs/mdb.so .libs/mquery.so .libs/mupdate.so .libs/mfortune.so $ULIB_DOCUMENT_ROOT

echo "export ULIB_VERSION=${ULIB_VERSION}" > $IROOT/ulib.installed
echo "export ULIB_ROOT=${ULIB_ROOT}" >> $IROOT/ulib.installed
echo "export ULIB_DOCUMENT_ROOT=${ULIB_DOCUMENT_ROOT}" >> $IROOT/ulib.installed
echo -e "export PATH=\$ULIB_ROOT/bin:\$PATH" >> $IROOT/ulib.installed

#sudo /etc/init.d/ssh start
#sudo /etc/init.d/mysql start

#ORM_DRIVER="sqlite" ORM_OPTION="host=10.30.1.131 dbname=../db/%.*s"
#ORM_DRIVER="pgsql"  ORM_OPTION="host='10.30.1.131' user='benchmarkdbuser' password='benchmarkdbpass' client_encoding='UTF8' dbname='hello_world' sslmode='allow'"
#ORM_DRIVER="mysql"  ORM_OPTION="host=10.30.1.131 user=benchmarkdbuser password=benchmarkdbpass character-set=utf8 dbname=hello_world"
#/mnt/data/$DIR/installs/ULib/bin/userver_tcp -c /mnt/data/$DIR/installs/ULib/benchmark.cfg

#sudo /bin/bash -c "ulimit -r 99 && exec su stefano -p -c \"$IROOT/ULib/bin/userver_tcp -c $IROOT/ULib/benchmark.cfg\"" &
#sudo /bin/bash -c "ulimit -r 99 && exec su stefano -p -c \"strace -f $IROOT/ULib/bin/userver_tcp -c $IROOT/ULib/benchmark.cfg >/tmp/a 2>&1\"" &
