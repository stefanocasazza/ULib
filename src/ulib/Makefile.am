## Makefile.am -- Process this file with automake to produce Makefile.in

MAINTAINERCLEANFILES = Makefile.in Makefile

EXTRA_DIST = base/* container/* db/* debug/* dynamic/* flex/* internal/* ldap/* mime/* net/* pcre/* \
			 	 replace/* ssl/* ssh/* ui/* utility/* xml/expat/* xml/libxml2/* zip/* lemon/*.cpp \
				 orm/driver/*.cpp orm/driver/Makefile.* \
             net/server/plugin/*.cpp net/server/plugin/Makefile.* \
				 *.cpp all_c.c ULib.pc.in ULib.rc.in utility/*_store.bin

DEFAULT_INCLUDES = -I. -I$(top_builddir)/include

pkgconfigdir 	= $(libdir)/pkgconfig
pkgconfig_DATA = ULib.pc

## DEFS = -DDEBUG_DEBUG @DEFS@
## base/lzo/minilzo.c
## base/mkqsort.c base/rsort.c

SRC_C = base/base.c base/base_error.c base/hash.c base/utility.c base/miniz/miniz.c base/xxhash/xxhash.c \
		  base/coder/cbase64.c base/coder/cquoted_printable.c base/coder/curl_coder.c base/coder/cxml_coder.c base/coder/cescape.c base/coder/chexdump.c

SRC_CPP = internal/common.cpp internal/error.cpp \
			 ui/dialog.cpp db/cdb.cpp db/rdb.cpp \
			 dynamic/dynamic.cpp dynamic/plugin.cpp \
			 mime/header.cpp mime/entity.cpp mime/multipart.cpp \
			 container/vector.cpp container/hash_map.cpp container/tree.cpp \
			 utility/interrupt.cpp utility/services.cpp utility/semaphore.cpp utility/base64.cpp \
			 utility/lock.cpp utility/string_ext.cpp utility/socket_ext.cpp utility/uhttp.cpp \
			 utility/data_session.cpp utility/ring_buffer.cpp utility/websocket.cpp utility/dir_walk.cpp \
			 lemon/expression.cpp \
			 orm/orm.cpp orm/orm_driver.cpp \
			 net/ipaddress.cpp net/socket.cpp net/ping.cpp \
			 net/server/server.cpp net/server/client_image.cpp \
			 net/client/client_rdb.cpp net/server/client_image_rdb.cpp net/server/server_rdb.cpp net/server/plugin/mod_proxy_service.cpp \
 			 net/rpc/rpc.cpp net/rpc/rpc_envelope.cpp net/rpc/rpc_fault.cpp net/rpc/rpc_method.cpp net/rpc/rpc_encoder.cpp \
 			 net/rpc/rpc_object.cpp net/rpc/rpc_gen_method.cpp net/rpc/rpc_parser.cpp net/rpc/rpc_client.cpp \
			 net/client/smtp.cpp net/client/ftp.cpp net/client/pop3.cpp net/client/imap.cpp \
			 net/client/http.cpp net/client/client.cpp net/client/redis.cpp net/client/elasticsearch.cpp \
			 net/ipt_ACCOUNT.cpp \
			 json/value.cpp \
			 query/query_parser.cpp  event/event_time.cpp \
			 timeval.cpp timer.cpp notifier.cpp string.cpp file.cpp process.cpp file_config.cpp log.cpp \
			 options.cpp application.cpp cache.cpp date.cpp url.cpp tokenizer.cpp command.cpp

if LIBTDB
SRC_CPP += db/tdb.cpp
endif

if MONGODB
SRC_CPP += net/client/mongodb.cpp
endif

if HTTP2
SRC_CPP += utility/http2.cpp
endif

if STDCPP
SRC_CPP += internal/objectIO.cpp 
endif

if PTHREAD
SRC_CPP += thread.cpp
endif

if DEBUG
SRC_C   += base/base_trace.c
SRC_CPP += debug/debug_common.cpp debug/trace.cpp debug/error_memory.cpp debug/error_simulation.cpp
if STDCPP
SRC_CPP += debug/objectDB.cpp
endif
endif

if !GETOPT_LONG
SRC_C += replace/getopt_long.c
endif

if !MREMAP
SRC_C += replace/mremap.c
endif

if !NANOSLEEP
SRC_C += replace/nanosleep.c
endif

if !SENDFILE
SRC_C += replace/sendfile.c
endif

if !STRNDUP
SRC_C += replace/strndup.c
endif

if !SEMAPHORE
SRC_C += replace/sem.c
else
if !SEM_TIMEDWAIT
SRC_C += replace/sem_timedwait.c
endif
endif

if !STRPTIME
SRC_C += replace/strptime.c
endif

if !MKDTEMP
SRC_C += replace/mkdtemp.c
endif

if !MEMMEM
SRC_C += replace/memmem.c
endif

if !MEMRCHR
SRC_C += replace/memrchr.c
endif

if !GMTIME_R
SRC_C += replace/gmtime.c
endif

if !TIMEGM
SRC_C += replace/timegm.c
endif

if !FNMATCH
SRC_C += replace/fnmatch.c
endif

if !FALLOCATE
SRC_C += replace/fallocate.c
endif

if !FALLOCATE64
SRC_C += replace/fallocate64.c
endif

if !PREAD
SRC_C += replace/pread.c
endif

if !ASSERT
SRC_C += replace/assert.c
endif

if APEXMEM
SRC_C += base/apex/apex_memmove.c
endif

if MEMORY_POOL
SRC_CPP += internal/memory_pool.cpp
endif

if !SCHED_GETCPU
SRC_C += replace/sched_getcpu.c
endif

if ZIP
SRC_C   += base/zip/inflate.c base/zip/dostime.c base/zip/pushback.c base/zip/ziptool.c
SRC_CPP += zip/zip.cpp
endif

if LIBZ
SRC_C += base/coder/cgzio.c
endif

if USE_PARSER
SRC_CPP += flex/flexer.cpp flex/bison.cpp
endif

if PCRE
SRC_CPP += pcre/pcre.cpp
endif

if SSL
SRC_C   += base/ssl/cdes3.c base/ssl/dgst.c
SRC_CPP += ssl/certificate.cpp ssl/pkcs7.cpp ssl/crl.cpp ssl/pkcs10.cpp net/client/twilio.cpp \
		     ssl/mime/mime_pkcs7.cpp ssl/net/sslsocket.cpp ssl/net/ssl_session.cpp utility/des3.cpp
if SSL_TS
SRC_CPP += ssl/timestamp.cpp
endif
endif

if SSH
SRC_CPP += ssh/net/sshsocket.cpp
endif

if LDAP
SRC_CPP += ldap/ldap.cpp
endif

if CURL
SRC_CPP += curl/curl.cpp
endif

if EXPAT
SRC_CPP += xml/expat/attribute.cpp xml/expat/element.cpp xml/expat/xml_parser.cpp xml/expat/xml2txt.cpp \
		     xml/soap/soap_fault.cpp xml/soap/soap_gen_method.cpp xml/soap/soap_parser.cpp xml/soap/soap_encoder.cpp xml/soap/soap_client.cpp
endif

if LIBXML2
SRC_CPP += xml/libxml2/node.cpp xml/libxml2/document.cpp xml/libxml2/schema.cpp
endif

if MAGIC
SRC_CPP += magic/magic.cpp
endif

if DBI
SRC_CPP += dbi/dbi.cpp
endif

if LIBEVENT
SRC_CPP += libevent/event.cpp
endif

if MINGW
SRC_C   += base/win32/mingw32.c
else
SRC_CPP += net/unixsocket.cpp
endif

# Handler static plugin

if STATIC_HANDLER_RPC
SRC_CPP += net/server/plugin/mod_rpc.cpp
endif

if MOD_SHIB
if STATIC_HANDLER_SHIB
SRC_CPP += net/server/plugin/mod_shib/mod_shib.cpp
endif
endif

if STATIC_HANDLER_STREAM
SRC_CPP += net/server/plugin/mod_stream.cpp
endif

if STATIC_HANDLER_NOCAT
SRC_CPP += net/server/plugin/mod_nocat.cpp
endif

if STATIC_HANDLER_SOCKET
SRC_CPP += net/server/plugin/mod_socket.cpp
endif

if STATIC_HANDLER_SCGI
SRC_CPP += net/server/plugin/mod_scgi.cpp
endif

if STATIC_HANDLER_FCGI
SRC_CPP += net/server/plugin/mod_fcgi.cpp
endif

if GEOIP
if STATIC_HANDLER_GEOIP
SRC_CPP += net/server/plugin/mod_geoip/mod_geoip.cpp
endif
endif

if PCRE
if STATIC_HANDLER_PROXY
SRC_CPP += net/server/plugin/mod_proxy.cpp
endif
endif

if EXPAT
if STATIC_HANDLER_SOAP
SRC_CPP += net/server/plugin/mod_soap.cpp
endif
endif

if STATIC_HANDLER_SSI
SRC_CPP += net/server/plugin/mod_ssi.cpp
endif

if STATIC_HANDLER_TSA
SRC_CPP += net/server/plugin/mod_tsa.cpp
endif

if STATIC_HANDLER_HTTP
SRC_CPP += net/server/plugin/mod_http.cpp
endif

if STATIC_HANDLER_ECHO
SRC_CPP += net/server/plugin/mod_echo.cpp
endif

# Handler static orm driver

#lib@ULIB@_la_CXXFLAGS =
lib@ULIB@_la_LDFLAGS = @ULIB_LIBS@ -version-info 1:0:0 -release @ULIB_VERSION@

if STATIC_ORM_DRIVER_SQLITE
SRC_CPP += orm/driver/orm_driver_sqlite.cpp
endif

if STATIC_ORM_DRIVER_MYSQL
SRC_CPP += orm/driver/orm_driver_mysql.cpp
endif

if STATIC_ORM_DRIVER_PGSQL
SRC_CPP += orm/driver/orm_driver_pgsql.cpp
endif

# Handler static http servlet

if STATIC_ONLY
if STATIC_SERVLET
SRC_CPP += net/server/plugin/usp/loader.autoconf.inc 
endif
##if STATIC_SERVLET_WI_AUTH
##SRC_CPP += ../../examples/WiAuth/wi_auth.cpp
##endif
endif

if FINAL
lib@ULIB@_la_SOURCES = all_c.c base/xxhash/xxhash.c all_cpp.cpp
else
lib@ULIB@_la_SOURCES = $(SRC_C) $(SRC_CPP)
endif

## for convenience library
## noinst_LTLIBRARIES = lib@ULIB@.la

lib_LTLIBRARIES = lib@ULIB@.la

dist-hook:
	find $(distdir) -type f -name \*.c   -exec /utility/expand_tab '{}' ';'
	find $(distdir) -type f -name \*.cpp -exec /utility/expand_tab '{}' ';'
	find $(distdir) -type f -name \*.usp -exec /utility/expand_tab '{}' ';'

clean-local:
	@cd $(top_builddir)/src/ulib/net/server/plugin/usp; $(MAKE) $(AM_MAKEFLAGS) clean 
	-rm -rf core .libs *.bb* *.da *.la gmon.out *.gc* */*.gc* */*/*.gc* */*/*/*.gc* */*/*/*/*.gc*

## ----------------------------------------------------------------------------------------------------
## usp section
## ----------------------------------------------------------------------------------------------------
if !CROSS_COMPILING
SUFFIXES = .cpp .usp .lo .la

net/server/plugin/usp/usp_translator$(EXEEXT):
	cd net/server/plugin/usp; $(MAKE) $(AM_MAKEFLAGS) ./usp_translator$(EXEEXT)

.usp.cpp: net/server/plugin/usp/usp_translator$(EXEEXT)
	@$(WINELOADER) net/server/plugin/usp/usp_translator$(EXEEXT) $<

../../examples/WiAuth/wi_auth.cpp: ../../examples/WiAuth/wi_auth.usp net/server/plugin/usp/usp_translator$(EXEEXT)

if STATIC_ONLY
if STATIC_SERVLET
@STATIC_SERVLET_MAKE@
endif
endif

.lo.la:
	$(CXXLINK) -rpath @ULIB_MODULEDIR@ -module $(MODULE_LIBTOOL_OPTIONS) $< lib@ULIB@.la @ULIB_LIBS@

## all: net/server/plugin/usp/usp_translator$(EXEEXT)
endif
## ----------------------------------------------------------------------------------------------------

# ---------- Win32 stuff ----------
##if MINGW
##CXX_LD = $(CXX) $(LDFLAGS) -Wl,--export-all-symbols -Wl,--dll -Wl,--enable-auto-import
##
##.libs/lib@ULIB@.so: .libs/lib@ULIB@.a
##	$(DLLTOOL) --export-all-symbols -e .libs/exports.o -l .libs/lib@ULIB@.lib -z .libs/lib@ULIB@.def $<
##	$(CXX_LD) -shared .libs/exports.o .libs/all_c.o base/xxhash/.libs/xxhash.o .libs/all_cpp.o $< -o $@ @ULIB_LIBS@
##
##all: .libs/lib@ULIB@.so
##endif

if MINGW
lib@ULIB@_la_LDFLAGS 	 += -export-symbols $(srcdir)/@ULIB@.def -no-undefined -Wl,ULib-win32-res.o
lib@ULIB@_la_DEPENDENCIES = ULib-win32-res.o
endif

ULib-win32-res.o: ULib.rc
	$(WINDRES) $< $@
	$(DLLTOOL) -export-all-symbols -z $(srcdir)/@ULIB@.def $(srcdir)/.libs/all_c.o $(srcdir)/base/xxhash/.libs/xxhash.o $(srcdir)/.libs/all_cpp.o
	$(SED) -i 's/EXPORTS/;/' $(srcdir)/@ULIB@.def

if MINGW
install-def-file:
	$(INSTALL) $(srcdir)/@ULIB@.def $(DESTDIR)$(libdir)
uninstall-def-file:
	-rm $(DESTDIR)$(libdir)/@ULIB@.def
else
install-def-file:
uninstall-def-file:
endif

if MS_LIB_AVAILABLE
noinst_DATA = @ULIB@-@ULIB_VERSION@.lib

install-ms-lib:
	$(INSTALL) @ULIB@-@ULIB_VERSION@.lib $(DESTDIR)$(libdir)

uninstall-ms-lib:
	-rm $(DESTDIR)$(libdir)/@ULIB@-@ULIB_VERSION@.lib
else
install-ms-lib:
uninstall-ms-lib:
endif

ULib-@ULIB_VERSION@.lib: libulib-@ULIB_VERSION@.la $(srcdir)/@ULIB@.def
	lib -name:libulib-@ULIB_VERSION@-0.dll -def:$(srcdir)/@ULIB@.def -out:$@

install-data-local: install-ms-lib install-def-file

uninstall-local: uninstall-ms-lib uninstall-def-file
