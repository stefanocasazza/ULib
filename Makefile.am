## Makefile.am -- Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies ## dist-shar dist-zip
ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = cdb configure.help TODO LICENSE* README* *.spec* ulib.html \
	 			 ULib.m4 rpm.sh rpmpkgreq.lst rpmpkgreq.lst.suse openwrt \
	 			 doc/Doxyfile doc/readme.txt shtool *.awk .travis.yml autogen.sh nativejson-benchmark \
				 fuzz/http1-corpus fuzz/http2-corpus fuzz/build_libFuzzer.sh fuzz/Makefile.in fuzz/Makefile.am fuzz/uclient_fuzzer.cpp fuzz/uclient.cfg

MAINTAINERCLEANFILES = configure aclocal.m4 libtool Makefile.in Makefile INSTALL

moduledir = @ULIB_MODULEDIR@

SUBDIRS  = include src/ulib
if PLUGIN
SUBDIRS += src/ulib/net/server/plugin src/ulib/orm/driver
endif
if ENABLE_EXAMPLES
SUBDIRS += examples
endif
SUBDIRS += tests

DIST_SUBDIRS = include src/ulib src/ulib/net/server/plugin src/ulib/orm/driver m4 doc examples tests contrib

## bin_SCRIPTS = ulib-config

aclocaldir = $(datadir)/aclocal
aclocal_DATA = ULib.m4

.phony: doxy todo

##README.html: README.md
##@Markdown.pl README.md > README.html

test:
	@cd $(top_builddir)/tests; $(MAKE) $(AM_MAKEFLAGS) test 

doc: $(top_builddir)/doc/Doxyfile
	@cd $(top_builddir)/doc; -rm -rf doc; $(MAKE) $(AM_MAKEFLAGS) doxy 

todo:
	@grep --color=tty --exclude="Makefile*" --exclude-dir=.git -nrEI "/[*] TODO|/[*] FIXME|// TODO|// FIXME" .

cppcheck:
	cppcheck . --enable=all --force 2>&1 | sed 's/^/warning: /g' 1>&2;

coverage:
	@lcov --output-file coverage.info --directory . --capture
	@lcov --output-file coverage.info --remove coverage.info '/usr/*' 'src/ulib/expression.*'
	 lcov --list   	  coverage.info

coverage_clean:
	@lcov --directory . -z

coverage_html:
	@genhtml --ignore-errors source -o html coverage.info

install-data-local:
	@cd $(top_builddir)/src/ulib/net/server/plugin/usp; $(MAKE) $(AM_MAKEFLAGS) usp_compile.sh
	@${INSTALL} -m 777 $(top_builddir)/libtool $(DESTDIR)${bindir}/usp_libtool.sh; \
	 ${INSTALL} -m 777 $(top_builddir)/src/ulib/net/server/plugin/usp/usp_compile.sh $(DESTDIR)${bindir}

dist-hook: cleandist

cleandist:
	@rm -rf \
		$(distdir)/*.spec \
		$(distdir)/config.cache \
		$(distdir)/cdb/*.o \
		$(distdir)/cdb/*.a \
		$(distdir)/cdb/rts* \
		$(distdir)/cdb/softlimit \
		$(distdir)/examples/*/*.spec \
		$(distdir)/tests/ulib/tmp/* \
		$(distdir)/tests/ulib/private/* \
		$(distdir)/tests/ulib/CApath/*.*0 \
		$(distdir)/tests/ulib/ok/json.ok \
		$(distdir)/tests/ulib/ok/timer.ok \
		$(distdir)/tests/ulib/ok/socket.ok \
		$(distdir)/tests/ulib/ok/interrupt.ok \
		$(distdir)/tests/ulib/ok/memory_pool.ok \
		$(distdir)/src/ulib/*/*.lo \
		$(distdir)/src/ulib/*/*/*.lo \
		$(distdir)/src/ulib/*/*/*/*.lo \
		$(distdir)/src/ulib/*/*/*/*/*.lo \
		$(distdir)/src/ulib/*/.dirstamp \
		$(distdir)/src/ulib/*/*/.dirstamp \
		$(distdir)/src/ulib/*/*/*/.dirstamp \
		$(distdir)/src/ulib/*/*/*/*/.dirstamp \
		$(distdir)/src/ulib/*/.deps \
		$(distdir)/src/ulib/*/*/.deps \
		$(distdir)/src/ulib/*/*/*/.deps \
		$(distdir)/src/ulib/*/*/*/*/.deps \
  		$(distdir)/src/ulib/wi_auth.cpp \
  		$(distdir)/src/ulib/orm/driver/*.o \
  		$(distdir)/src/ulib/orm/driver/*.l? \
  		$(distdir)/src/ulib/orm/driver/.deps \
  		$(distdir)/src/ulib/orm/driver/.libs \
  		$(distdir)/src/ulib/orm/driver/Makefile \
  		$(distdir)/src/ulib/net/server/plugin/*.o \
  		$(distdir)/src/ulib/net/server/plugin/*.l? \
  		$(distdir)/src/ulib/net/server/plugin/.deps \
  		$(distdir)/src/ulib/net/server/plugin/.libs \
  		$(distdir)/src/ulib/net/server/plugin/Makefile \
  		$(distdir)/src/ulib/net/server/plugin/*/*.o \
  		$(distdir)/src/ulib/net/server/plugin/*/*.l? \
  		$(distdir)/src/ulib/net/server/plugin/*/.deps \
  		$(distdir)/src/ulib/net/server/plugin/*/.libs \
  		$(distdir)/src/ulib/net/server/plugin/*/Makefile \
  		$(distdir)/src/ulib/net/server/plugin/usp/usp_compile.sh \
  		$(distdir)/src/ulib/net/server/plugin/usp/usp_translator \
		$(distdir)/tests/ulib/test_bison.cc \
		$(distdir)/tests/ulib/plugin/*.lo \
		$(distdir)/tests/ulib/plugin/.dirstamp \
		$(distdir)/tests/ulib/plugin/*/.dirstamp \
		$(distdir)/tests/examples/RSIGN/RSIGN/log \
		$(distdir)/tests/examples/WAGSM/log \
		$(distdir)/tests/examples/CApath/*.*0 \
		$(distdir)/tests/examples/nocat/.ht* \
		$(distdir)/tests/examples/nocat/image \
		$(distdir)/tests/examples/nocat/webif* \
		$(distdir)/tests/examples/python/*.pyc \
		$(distdir)/tests/examples/TSA/log \
		$(distdir)/tests/examples/TSA/TSA \
		$(distdir)/tests/examples/TSA/.rnd \
		$(distdir)/tests/examples/TSA/response/* \
		$(distdir)/tests/examples/websocket/cgi-bin \
		$(distdir)/tests/examples/websocket/uploads \
		$(distdir)/tests/examples/CSP/CSP/DB_CA/CA* \
		$(distdir)/tests/examples/CSP/CSP/DB_CA/log \
		$(distdir)/tests/examples/CSP/CSP/DB_CA/.rnd \
		$(distdir)/tests/examples/IR/db* \
		$(distdir)/tests/examples/IR/WEB/doc \
		$(distdir)/tests/examples/IR/WEB/servlet/*.so \
		$(distdir)/tests/examples/IR/benchmark/out \
		$(distdir)/tests/examples/IR/benchmark/log \
		$(distdir)/tests/examples/IR/benchmark/err \
		$(distdir)/tests/examples/IR/benchmark/index.cfg \
		$(distdir)/tests/examples/inp/*.cfg \
		$(distdir)/tests/examples/inp/http/and \
		$(distdir)/tests/examples/benchmark/db \
		$(distdir)/tests/examples/docroot/ok \
		$(distdir)/tests/examples/docroot/tmp \
		$(distdir)/tests/examples/docroot/uploads \
		$(distdir)/tests/examples/docroot/vuoto.txt \
		$(distdir)/tests/examples/docroot/www.sito1.com \
		$(distdir)/tests/examples/ok/client_server.ok \
		$(distdir)/tests/examples/ok/web_server_multiclient.ok \
		$(distdir)/tests/examples/benchmark/userver_tcp.pid \
		$(distdir)/tests/examples/benchmark/docroot/servlet/*.so \
		$(distdir)/tests/examples/benchmark/docroot/servlet/*.usp \
		$(distdir)/tests/examples/benchmark/libraries/libcaptcha.o \
		$(distdir)/tests/examples/docroot/benchmark/docroot/servlet \
		$(distdir)/tests/examples/benchmark/FrameworkBenchmarks/ULib/www/*.so \
		$(distdir)/tests/examples/benchmark/FrameworkBenchmarks/ULib/db/*.jnl \
		$(distdir)/tests/examples/benchmark/FrameworkBenchmarks/ULib/db/session.* \
		$(distdir)/tests/examples/benchmark/FrameworkBenchmarks/ULib/userver_tcp.pid
