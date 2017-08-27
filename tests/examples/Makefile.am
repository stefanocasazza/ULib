## Makefile.am -- Process this file with automake to produce Makefile.in

MAINTAINERCLEANFILES	= Makefile.in

DEFAULT_INCLUDES =  -I. -I$(top_srcdir) -I$(top_srcdir)/include -I$(top_srcdir)/examples/http_header/include

EXTRA_DIST = inp ok CA CSP LCSP TSA RSIGN XAdES nocat wi-auth WAGSM RA IR/WEB IR/benchmark IR/doc_dir *.cfg .htpasswd .htdigest python \
				 *.properties *.test *.sh error_msg workflow doc_parse robots.txt alias.txt throttling.txt css js benchmark websocket docroot php.sh test_http_parser.h

## DEFS  = -DU_TEST @DEFS@

TESTS = client_server.test test_manager.test IR.test web_server.test web_server_multiclient.test web_socket.test ## workflow.test

if DEBUG
PRG = bench_http_parser test_http_parser
bench_http_parser_SOURCES = bench_http_parser.cpp
test_http_parser_SOURCES = test_http_parser.cpp ctest_http_parser.c
endif

if SSL
TESTS += tsa_http.test tsa_https.test csp_rpc.test rsign_rpc.test tsa_rpc.test uclient.test
if EXPAT
TESTS += csp.test tsa_ssoap.test rsign.test
endif
## MINGW don't have unix socket...
if !MINGW
TESTS += lcsp_rpc.test
if EXPAT
TESTS += lcsp.test
endif
endif
endif

if SSL
TESTS += web_server_ssl.test
## PRG += test_http_header
## test_http_header_SOURCES = test_http_header.cpp
## HTTP_LIB = $(top_builddir)/examples/http_header/libhttp.la
## TESTS += http_header.test
if LIBZ
TESTS += PEC_report_rejected.test PEC_report_messaggi.test PEC_report_virus.test PEC_report_anomalie.test PEC_check_namefile.test
if ZIP
TESTS += doc_parse.test doc_classifier.test
endif
endif
##if LIBXML2
##TESTS += XAdES.test
##endif
endif

if EXPAT
TESTS += xml2txt.test
endif

## if LDAP
## TESTS += form_completion.test
## if SSL
## if SSH
## TESTS += lrp_session.test lrp_pusher.test
## endif
## endif
## endif

## if USE_PARSER
## TESTS += parser.test
## endif

#if LINUX
#TESTS += download_accelerator.test
#endif

check_PROGRAMS  = $(PRG)
TESTS 			+= ../reset.color

LDADD = @ULIBS@ $(HTTP_LIB) $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

## web_server_multiclient.test form_completion.test http_header.test lrp_pusher.test lrp_session.test workflow.test 
test: all-am
	$(MAKE) $(AM_MAKEFLAGS) $(check_PROGRAMS)
	../make_test.sh client_server.test test_manager.test IR.test web_server.test web_socket.test tsa_http.test tsa_https.test rsign_rpc.test tsa_rpc.test uclient.test tsa_ssoap.test rsign.test web_server_ssl.test PEC_report_rejected.test PEC_report_messaggi.test PEC_report_virus.test PEC_report_anomalie.test PEC_check_namefile.test doc_parse.test xml2txt.test ../reset.color

clean-local:
	-rm -rf out err core .libs *.bb* *.da *.gc* IR/db* TSA/gSOAP/tsa_client \
			  *.log* \
			*/*.log* \
			  trace.*.[0-9]*   stack.*.[0-9]*   object.*.[0-9]*   mempool.*.[0-9]* \
			*/trace.*.[0-9]* */stack.*.[0-9]* */object.*.[0-9]* */mempool.*.[0-9]* \
			websocket/cgi-bin \
			websocket/uploads \
			benchmark/docroot/*.log* \
			benchmark/docroot/trace.*.[0-9]* \
			benchmark/docroot/stack.*.[0-9]* \
			benchmark/docroot/object.*.[0-9]* \
			benchmark/docroot/mempool.*.[0-9]* \
			benchmark/FrameworkBenchmarks/ULib/www/trace.*.[0-9]* \
			benchmark/FrameworkBenchmarks/ULib/www/stack.*.[0-9]* \
			benchmark/FrameworkBenchmarks/ULib/www/object.*.[0-9]* \
			benchmark/FrameworkBenchmarks/ULib/www/mempool.*.[0-9]*
