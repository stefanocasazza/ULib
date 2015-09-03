#!/bin/sh

# set -x

rm -f trace.*hello_*.[0-9]* object.*hello_*.[0-9]* stack.*hello_*.[0-9]* mempool.*hello_*.[0-9]*

 UTRACE="0 100M 0" # -1 to disable trace limitation
#UTRACE_SIGNAL="0 50M 0"
#UOBJDUMP="0 10M 100"
#USIMERR="error.sim"
export UTRACE UOBJDUMP USIMERR UTRACE_SIGNAL

./hello -c
./hello -V
./hello     -n pippo
./hello --name pluto
./hello paperino
./hello
