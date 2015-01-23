#!/bin/sh

echo "*** Compiling ***" | tee -a cdb_compile.log &&
make -i cdbdump cdbget cdbmake cdbstats cdbtest cdbnumrecs > cdb_compile.log 2>&1

if [ ! -x cdbdump ]
then
	echo "*** Environment CYGWIN ***" | tee -a cdb_compile.log

	./compile cygwin/setgroups.c
	ar r unix.a setgroups.o
	ranlib unix.a

	./load cdbdump  		 buffer.a unix.a byte.a
	./load cdbget 	 cdb.a buffer.a unix.a byte.a
	./load cdbstats cdb.a buffer.a unix.a byte.a
	./load cdbmake  cdb.a buffer.a unix.a byte.a alloc.a
fi

exit 0

### for testing

echo "*** Testing ***" | tee -a cdb_compile.log
./rts.sh > rts.log 2>&1

rm -f test.tmp

N=`diff rts.log rts.exp`

if [ "$N" = "" ]; then
 	echo "*** OK ***" | tee -a cdb_compile.log
 	exit 0
else
 	echo "*** KO ***" | tee -a cdb_compile.log
 	exit 1
fi

exit 0

### for install

if [ $# -eq 1 ] && [ -d $1 ]
then
 	cp cdbdump cdbget cdbmake cdbstats $1/bin
fi

### for ulib

# ./cdbmake ../tests/ulib/random.cdb test.tmp < ./random.txt
