## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2001 Unirel s.r.l.

AUTOMAKE_OPTIONS = ## dist-shar dist-zip

EXTRA_DIST = random.cdb plugin inp ok *.test *.cpp CA private server_rpc.cfg \
				 file_config.cf file_config.gperf file_config.gperf.sh file_config.key test_bison.h

MAINTAINERCLEANFILES	= Makefile.in

DEFAULT_INCLUDES =  -I. -I$(top_builddir)/include

LDADD = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

## DEFS = -DU_TEST @DEFS@
## TESTS_ENVIRONMENT = srcdir=$(srcdir)
## AM_LFLAGS = -olex.yy.c -S$(shell pwd)/../../include/ulib/flex/flex.skl
## NB: il comando per generare il file .cc dal file .ll viene lanciato in una subdirectory:
## sh -x /mnt/source/ulib/autoconf/ylwrap test_flexer.ll lex.yy.c test_flexer.cc -- \
## 		flex -olex.yy.c -S../../include/ulib/flex/flex.skl

AM_LFLAGS = -olex.yy.c
AM_YFLAGS = -d -v # -Sbison.skl

PRG = test_timeval test_timer test_notifier test_string \
		test_file test_cdb test_rdb test_file_config test_log \
		test_vector test_options test_application test_tree test_compress test_cache test_date \
		test_services test_base64 test_header test_entity \
		test_ipaddress test_socket test_smtp test_pop3 test_imap test_ftp test_http test_rdb_client \
		test_tokenizer test_query_parser test_multipart test_command test_dialog test_rdb_server test_json test_server

TST = timeval.test timer.test notifier.test string.test \
		file.test cdb.test rdb.test file_config.test log.test \
		vector.test options.test application.test tree.test compress.test cache.test date.test \
		services.test base64.test header.test entity.test \
		ipaddress.test socket.test smtp.test pop3.test imap.test ftp.test http.test \
		tokenizer.test query_parser.test multipart.test rdb_client_server.test command.test json.test server.test server_rpc.test
## 	dialog.test

test_ipaddress_SOURCES = test_ipaddress.cpp
test_socket_SOURCES = test_socket.cpp
test_ftp_SOURCES = test_ftp.cpp
test_smtp_SOURCES = test_smtp.cpp
test_pop3_SOURCES = test_pop3.cpp
test_imap_SOURCES = test_imap.cpp
test_http_SOURCES = test_http.cpp
test_timeval_SOURCES = test_timeval.cpp
test_timer_SOURCES = test_timer.cpp
test_notifier_SOURCES = test_notifier.cpp
test_string_SOURCES = test_string.cpp
test_file_SOURCES = test_file.cpp
test_cdb_SOURCES = test_cdb.cpp
test_rdb_SOURCES = test_rdb.cpp
test_file_config_SOURCES = test_file_config.cpp
test_log_SOURCES = test_log.cpp
test_vector_SOURCES = test_vector.cpp
test_options_SOURCES = test_options.cpp
test_application_SOURCES = test_application.cpp
test_tree_SOURCES = test_tree.cpp
test_compress_SOURCES = test_compress.cpp
test_cache_SOURCES = test_cache.cpp
test_date_SOURCES = test_date.cpp
test_services_SOURCES = test_services.cpp
test_base64_SOURCES = test_base64.cpp
test_header_SOURCES = test_header.cpp
test_entity_SOURCES = test_entity.cpp
test_rdb_client_SOURCES = test_rdb_client.cpp
test_tokenizer_SOURCES = test_tokenizer.cpp
test_query_parser_SOURCES = test_query_parser.cpp
test_multipart_SOURCES = test_multipart.cpp
test_command_SOURCES = test_command.cpp
test_dialog_SOURCES = test_dialog.cpp
test_rdb_server_SOURCES = test_rdb_server.cpp
test_json_SOURCES = test_json.cpp
test_server_SOURCES = test_server.cpp

if PTHREAD
PRG += test_thread
TST += thread.test
test_thread_SOURCES = test_thread.cpp
endif

if ZIP
PRG += test_zip
TST += zip.test
test_zip_SOURCES = test_zip.cpp
endif

if USE_PARSER
PRG += test_flexer test_bison
TST +=  flexer.test bison.test
test_flexer_SOURCES =  test_flexer.ll
test_bison_SOURCES =  test_bison_flexer.ll test_bison.yy
endif

if PCRE
PRG += test_pcre
TST += pcre.test
test_pcre_SOURCES = test_pcre.cpp
endif

if SSL
PRG += test_des3 test_digest test_certificate test_crl test_pkcs10 test_ssl_client test_ssl_server test_https test_pkcs7 test_url
TST += des3.test digest.test certificate.test crl.test pkcs10.test ssl_client_server.test https.test pkcs7.test url.test
test_des3_SOURCES = test_des3.cpp
test_digest_SOURCES = test_digest.cpp
test_certificate_SOURCES = test_certificate.cpp
test_crl_SOURCES = test_crl.cpp
test_pkcs10_SOURCES = test_pkcs10.cpp
test_pkcs7_SOURCES = test_pkcs7.cpp
test_ssl_client_SOURCES = test_ssl_client.cpp
test_ssl_server_SOURCES = test_ssl_server.cpp
test_https_SOURCES = test_https.cpp
test_url_SOURCES = test_url.cpp
if SSL_TS
PRG += test_timestamp
TST += timestamp.test
test_timestamp_SOURCES = test_timestamp.cpp
endif
endif

if SSH
PRG += test_ssh_client
TST += ssh_client.test
test_ssh_client_SOURCES = test_ssh_client.cpp
endif

if LDAP
PRG += test_ldap
TST += ldap.test
test_ldap_SOURCES = test_ldap.cpp
endif

if CURL
PRG += test_curl
TST += curl.test
test_curl_SOURCES = test_curl.cpp
endif

if MAGIC
PRG += test_magic
TST += magic.test
test_magic_SOURCES = test_magic.cpp
endif

if EXPAT
PRG += test_expat test_soap_server test_soap_client
TST += expat.test soap_server.test soap_client.test
test_expat_SOURCES = test_expat.cpp
test_soap_client_SOURCES = test_soap_client.cpp
test_soap_server_SOURCES = test_soap_server.cpp
endif

if MEMORY_POOL
PRG += test_memory_pool
TST += memory_pool.test
test_memory_pool_SOURCES = test_memory_pool.cpp
endif

if DBI
PRG += test_dbi
TST += dbi.test
test_dbi_SOURCES = test_dbi.cpp
endif

if HAVE_SQLITE3
PRG += test_orm
TST += orm.test
test_orm_SOURCES = test_orm.cpp
endif

if LIBEVENT
PRG += test_event
TST += event.test
test_event_SOURCES = test_event.cpp
endif

if !MINGW
PRG += test_process test_interrupt test_unixsocket_client test_unixsocket_server test_arping
TST += process.test interrupt.test unixsocket.test
test_arping_SOURCES = test_arping.cpp
test_process_SOURCES = test_process.cpp
test_interrupt_SOURCES = test_interrupt.cpp
test_unixsocket_client_SOURCES = test_unixsocket_client.cpp
test_unixsocket_server_SOURCES = test_unixsocket_server.cpp
endif

if PLUGIN
if STDCPP
PRG += test_plugin
TST += plugin.test
test_plugin_SOURCES  = test_plugin.cpp

product1_la_SOURCES  = plugin/product1.cpp
product1_la_LIBADD   = $(LDADD)
product1_la_LDFLAGS  = -module -rpath /nowhere $(MODULE_LIBTOOL_OPTIONS)

product2_la_SOURCES  = plugin/product2.cpp
product2_la_LIBADD   = $(LDADD)
product2_la_LDFLAGS  = -module -rpath /nowhere $(MODULE_LIBTOOL_OPTIONS)

noinst_moduledir = $(libexecdir)/ulib

## for convenience module
noinst_LTLIBRARIES = product1.la product2.la
endif
endif

check_PROGRAMS = $(PRG)
TESTS = $(TST) ../reset.color

clean-local:
	-rm -rf out err core .libs *.bb* *.da *.gcov *.log test_log.log* tmp/* \
			trace.*.[0-9]*  object.*.[0-9]* stack.*.[0-9]* mempool.*.[0-9]*
