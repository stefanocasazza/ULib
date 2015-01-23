# Don't edit Makefile! Use conf-* for configuration.

SHELL=/bin/sh

default: it

alloc.a: makelib alloc.o
	./makelib alloc.a alloc.o

alloc.o: compile alloc.c alloc.h error.h
	./compile alloc.c

auto-str: load auto-str.o buffer.a unix.a byte.a
	./load auto-str buffer.a unix.a byte.a 

auto-str.o: compile auto-str.c buffer.h readwrite.h exit.h
	./compile auto-str.c

auto_home.c: auto-str conf-home
	./auto-str auto_home `head -1 conf-home` > auto_home.c

auto_home.o: compile auto_home.c
	./compile auto_home.c

buffer.a: makelib strerr_sys.o buffer.o buffer_put.o buffer_2.o strerr_die.o buffer_get.o buffer_copy.o buffer_1.o buffer_0.o buffer_1s.o
	./makelib buffer.a strerr_sys.o buffer.o buffer_put.o buffer_2.o strerr_die.o buffer_get.o buffer_copy.o buffer_1.o buffer_0.o buffer_1s.o

buffer.o: compile buffer.c buffer.h
	./compile buffer.c

buffer_0.o: compile buffer_0.c readwrite.h buffer.h
	./compile buffer_0.c

buffer_1.o: compile buffer_1.c readwrite.h buffer.h
	./compile buffer_1.c

buffer_1s.o: compile buffer_1s.c readwrite.h buffer.h
	./compile buffer_1s.c

buffer_2.o: compile buffer_2.c readwrite.h buffer.h
	./compile buffer_2.c

buffer_copy.o: compile buffer_copy.c buffer.h
	./compile buffer_copy.c

buffer_get.o: compile buffer_get.c buffer.h byte.h error.h
	./compile buffer_get.c

buffer_put.o: compile buffer_put.c buffer.h str.h byte.h error.h
	./compile buffer_put.c

byte.a: makelib byte_copy.o byte_cr.o str_len.o fmt_ulong.o uint32_unpack.o byte_diff.o uint32_pack.o scan_ulong.o
	./makelib byte.a byte_copy.o byte_cr.o str_len.o fmt_ulong.o uint32_unpack.o byte_diff.o uint32_pack.o scan_ulong.o

byte_copy.o: compile byte_copy.c byte.h
	./compile byte_copy.c

byte_cr.o: compile byte_cr.c byte.h
	./compile byte_cr.c

byte_diff.o: compile byte_diff.c byte.h
	./compile byte_diff.c

cdb.a: makelib cdb.o cdb_hash.o cdb_make.o cdb_traverse.o
	./makelib cdb.a cdb.o cdb_hash.o cdb_make.o cdb_traverse.o

cdb.o: compile cdb.c readwrite.h error.h seek.h byte.h cdb.h uint32.h
	./compile cdb.c

cdb_traverse.o: compile cdb_traverse.c cdb.h uint32.h seek.h
	./compile cdb_traverse.c

cdb_hash.o: compile cdb_hash.c cdb.h uint32.h
	./compile cdb_hash.c

cdb_make.o: compile cdb_make.c readwrite.h seek.h error.h alloc.h cdb.h uint32.h cdb_make.h buffer.h uint32.h
	./compile cdb_make.c

cdbdump: load cdbdump.o buffer.a unix.a byte.a
	./load cdbdump buffer.a unix.a byte.a 

cdbdump.o: compile cdbdump.c uint32.h fmt.h buffer.h strerr.h
	./compile cdbdump.c

cdbget: load cdbget.o cdb.a buffer.a unix.a byte.a
	./load cdbget cdb.a buffer.a unix.a byte.a 

cdbget.o: compile cdbget.c exit.h scan.h str.h buffer.h strerr.h cdb.h uint32.h
	./compile cdbget.c

cdbmake: load cdbmake.o cdb.a alloc.a buffer.a unix.a byte.a
	./load cdbmake cdb.a alloc.a buffer.a unix.a byte.a 

cdbmake-12: warn-auto.sh cdbmake-12.sh conf-home
	cat warn-auto.sh cdbmake-12.sh \
	| sed s}HOME}"`head -1 conf-home`"}g \
	> cdbmake-12
	chmod 755 cdbmake-12

cdbmake-sv: warn-auto.sh cdbmake-sv.sh conf-home
	cat warn-auto.sh cdbmake-sv.sh \
	| sed s}HOME}"`head -1 conf-home`"}g \
	> cdbmake-sv
	chmod 755 cdbmake-sv

cdbmake.o: compile cdbmake.c error.h open.h strerr.h cdb_make.h buffer.h uint32.h cdb.h uint32.h
	./compile cdbmake.c

cdbstats: load cdbstats.o cdb.a buffer.a unix.a byte.a
	./load cdbstats cdb.a buffer.a unix.a byte.a 

cdbstats.o: compile cdbstats.c uint32.h fmt.h buffer.h strerr.h seek.h cdb.h uint32.h
	./compile cdbstats.c

cdbtest: load cdbtest.o cdb.a buffer.a unix.a byte.a
	./load cdbtest cdb.a buffer.a unix.a byte.a 

cdbtest.o: compile cdbtest.c uint32.h fmt.h buffer.h strerr.h seek.h cdb.h uint32.h
	./compile cdbtest.c

cdbnumrecs: load cdbnumrecs.o cdb.a buffer.a unix.a byte.a
	./load cdbnumrecs cdb.a buffer.a unix.a byte.a 

cdbnumrecs.o: compile cdbnumrecs.c uint32.h fmt.h buffer.h strerr.h seek.h cdb.h uint32.h
	./compile cdbnumrecs.c

check: it instcheck
	./instcheck

compile: warn-auto.sh conf-cc
	( cat warn-auto.sh; \
	echo exec "`head -1 conf-cc`" '-c $${1+"$$@"}' \
	) > compile
	chmod 755 compile

error.o: compile error.c error.h
	./compile error.c

error_str.o: compile error_str.c error.h
	./compile error_str.c

fmt_ulong.o: compile fmt_ulong.c fmt.h
	./compile fmt_ulong.c

hier.o: compile hier.c auto_home.h
	./compile hier.c

####### softlimit
alloc_re.o: alloc.h alloc_re.c byte.h compile
	./compile alloc_re.c

env.o: compile env.c env.h str.h
	./compile env.c

str_diff.o: compile str.h str_diff.c
	./compile str_diff.c

str_chr.o: compile str.h str_chr.c
	./compile str_chr.c

str_start.o: compile str.h str_start.c
	./compile str_start.c

stralloc_catb.o: byte.h compile gen_alloc.h stralloc.h stralloc_catb.c
	./compile stralloc_catb.c

stralloc_cats.o: byte.h compile gen_alloc.h str.h stralloc.h stralloc_cats.c
	./compile stralloc_cats.c

stralloc_eady.o: alloc.h compile gen_alloc.h gen_allocdefs.h stralloc.h stralloc_eady.c
	./compile stralloc_eady.c

stralloc_opyb.o: byte.h compile gen_alloc.h stralloc.h stralloc_opyb.c
	./compile stralloc_opyb.c

stralloc_pend.o: alloc.h compile gen_alloc.h gen_allocdefs.h stralloc.h stralloc_pend.c
	./compile stralloc_pend.c

pathexec_run.o: compile env.h error.h gen_alloc.h pathexec.h pathexec_run.c str.h stralloc.h
	./compile pathexec_run.c

subgetopt.o: compile subgetopt.c subgetopt.h
	./compile subgetopt.c

sgetopt.o: buffer.h compile sgetopt.c sgetopt.h subgetopt.h
	./compile sgetopt.c

softlimit.o: compile pathexec.h scan.h sgetopt.h softlimit.c str.h strerr.h subgetopt.h
	./compile softlimit.c

softlimit: byte.a load softlimit.o env.o str_diff.o str_chr.o str_start.o \
				stralloc_catb.o stralloc_cats.o stralloc_eady.o stralloc_opyb.o stralloc_pend.o \
				pathexec_run.o subgetopt.o sgetopt.o unix.a alloc.a alloc_re.o
	./load softlimit env.o str_diff.o str_chr.o str_start.o \
				stralloc_catb.o stralloc_cats.o stralloc_eady.o stralloc_opyb.o stralloc_pend.o \
				pathexec_run.o subgetopt.o sgetopt.o buffer.a unix.a byte.a alloc.a alloc_re.o
#######

install: load install.o hier.o auto_home.o buffer.a unix.a byte.a
	./load install hier.o auto_home.o buffer.a unix.a byte.a 

install.o: compile install.c buffer.h strerr.h error.h open.h readwrite.h exit.h
	./compile install.c

instcheck: load instcheck.o hier.o auto_home.o buffer.a unix.a byte.a
	./load instcheck hier.o auto_home.o buffer.a unix.a byte.a 

instcheck.o: compile instcheck.c strerr.h error.h readwrite.h exit.h
	./compile instcheck.c

it: prog install instcheck

load: warn-auto.sh conf-ld
	( cat warn-auto.sh; \
	echo 'main="$$1"; shift'; \
	echo exec "`head -1 conf-ld`" \
	'-o "$$main" "$$main".o $${1+"$$@"}' \
	) > load
	chmod 755 load

makelib: warn-auto.sh systype
	( cat warn-auto.sh; \
	echo 'main="$$1"; shift'; \
	echo 'rm -f "$$main"'; \
	echo 'ar cr "$$main" $${1+"$$@"}'; \
	case "`cat systype`" in \
	sunos-5.*) ;; \
	unix_sv*) ;; \
	irix64-*) ;; \
	irix-*) ;; \
	dgux-*) ;; \
	hp-ux-*) ;; \
	sco*) ;; \
	*) echo 'ranlib "$$main"' ;; \
	esac \
	) > makelib
	chmod 755 makelib

open_read.o: compile open_read.c open.h
	./compile open_read.c

open_trunc.o: compile open_trunc.c open.h
	./compile open_trunc.c

prog: cdbget cdbmake cdbdump cdbstats cdbtest cdbmake-12 cdbmake-sv rts testzero traverse cdbnext

rts: warn-auto.sh rts.sh conf-home
	cat warn-auto.sh rts.sh \
	| sed s}HOME}"`head -1 conf-home`"}g \
	> rts
	chmod 755 rts

scan_ulong.o: compile scan_ulong.c scan.h
	./compile scan_ulong.c

seek_cur.o: compile seek_cur.c seek.h
	./compile seek_cur.c

seek_set.o: compile seek_set.c seek.h
	./compile seek_set.c

setup: it install
	./install

str_len.o: compile str_len.c str.h
	./compile str_len.c

strerr_die.o: compile strerr_die.c buffer.h exit.h strerr.h
	./compile strerr_die.c

strerr_sys.o: compile strerr_sys.c error.h strerr.h
	./compile strerr_sys.c

systype: find-systype.sh conf-cc conf-ld trycpp.c x86cpuid.c
	( cat warn-auto.sh; \
	echo CC=\'`head -1 conf-cc`\'; \
	echo LD=\'`head -1 conf-ld`\'; \
	cat find-systype.sh; \
	) | sh > systype

testzero: load testzero.o cdb.a alloc.a buffer.a unix.a byte.a
	./load testzero cdb.a alloc.a buffer.a unix.a byte.a 

testzero.o: compile testzero.c uint32.h scan.h strerr.h cdb_make.h buffer.h uint32.h
	./compile testzero.c

uint32.h: tryulong32.c compile load uint32.h1 uint32.h2
	( ( ./compile tryulong32.c && ./load tryulong32 && \
	./tryulong32 ) >/dev/null 2>&1 \
	&& cat uint32.h2 || cat uint32.h1 ) > uint32.h
	rm -f tryulong32.o tryulong32

uint32_pack.o: compile uint32_pack.c uint32.h
	./compile uint32_pack.c

uint32_unpack.o: compile uint32_unpack.c uint32.h
	./compile uint32_unpack.c

unix.a: makelib error.o open_read.o open_trunc.o error_str.o seek_set.o seek_cur.o
	./makelib unix.a error.o open_read.o open_trunc.o error_str.o seek_set.o seek_cur.o

traverse: load traverse.o cdb.a uint32_unpack.o byte_copy.o seek_set.o error.o byte_diff.o fmt_ulong.o
	./load traverse cdb.a uint32_unpack.o byte_copy.o seek_set.o error.o byte_diff.o fmt_ulong.o

traverse.o: compile traverse.c cdb.h uint32.h fmt.h
	./compile traverse.c

cdbnext: load cdbnext.o cdb.a buffer.a byte.a error.o error_str.o seek_set.o
	./load cdbnext cdb.a buffer.a byte.a error.o error_str.o seek_set.o

cdbnext.o: compile cdbnext.c exit.h scan.h str.h buffer.h strerr.h cdb.h uint32.h
	./compile cdbnext.c

clean:
	rm -rf `cat TARGETS` *.o softlimit rts-tmp
