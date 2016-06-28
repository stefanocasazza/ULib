## Makefile.am for plugin userver

EXTRA_DIST = *.cpp Makefile.* \
				 mod_shib/*.cpp mod_shib/Makefile.* \
				 mod_geoip/*.cpp mod_geoip/Makefile.* \
				 page_speed/*.cpp page_speed/Makefile.* \
				 v8/*.cpp v8/Makefile.* \
				 php/*.cpp php/Makefile.* \
				 ruby/*.cpp ruby/Makefile.* \
				 python/*.cpp python/Makefile.*

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

ulib_la = $(top_builddir)/src/ulib/lib@ULIB@.la

moduledir = @ULIB_MODULEDIR@
module_LTLIBRARIES =

if !STATIC_HANDLER_RPC
module_LTLIBRARIES += server_plugin_rpc.la
server_plugin_rpc_la_SOURCES = mod_rpc.cpp
server_plugin_rpc_la_LIBADD  = $(ulib_la) @ULIB_LIBS@
server_plugin_rpc_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif

if !STATIC_HANDLER_STREAM
module_LTLIBRARIES += server_plugin_stream.la
server_plugin_stream_la_SOURCES = mod_stream.cpp
server_plugin_stream_la_LIBADD  = $(ulib_la) @ULIB_LIBS@
server_plugin_stream_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif

if !STATIC_HANDLER_NOCAT
module_LTLIBRARIES += server_plugin_nocat.la
server_plugin_nocat_la_SOURCES = mod_nocat.cpp
server_plugin_nocat_la_LIBADD  = $(ulib_la) @ULIB_LIBS@
server_plugin_nocat_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif

if !STATIC_HANDLER_SOCKET
module_LTLIBRARIES += server_plugin_socket.la
server_plugin_socket_la_SOURCES = mod_socket.cpp
server_plugin_socket_la_LIBADD  = $(ulib_la) @ULIB_LIBS@
server_plugin_socket_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif

if !STATIC_HANDLER_SCGI
module_LTLIBRARIES += server_plugin_scgi.la
server_plugin_scgi_la_SOURCES = mod_scgi.cpp
server_plugin_scgi_la_LIBADD  = $(ulib_la) @ULIB_LIBS@
server_plugin_scgi_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif

if !STATIC_HANDLER_FCGI
module_LTLIBRARIES += server_plugin_fcgi.la
server_plugin_fcgi_la_SOURCES = mod_fcgi.cpp
server_plugin_fcgi_la_LIBADD  = $(ulib_la) @ULIB_LIBS@
server_plugin_fcgi_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif

if PCRE
if !STATIC_HANDLER_PROXY
module_LTLIBRARIES += server_plugin_proxy.la
server_plugin_proxy_la_SOURCES = mod_proxy.cpp
server_plugin_proxy_la_LIBADD  = $(pkg_cv_LIBNETFILTER_CONNTRACK_LIBS) $(ulib_la) @ULIB_LIBS@
server_plugin_proxy_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif
endif

if EXPAT
if !STATIC_HANDLER_SOAP
module_LTLIBRARIES += server_plugin_soap.la
server_plugin_soap_la_SOURCES = mod_soap.cpp
server_plugin_soap_la_LIBADD  = $(ulib_la) @ULIB_LIBS@
server_plugin_soap_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif
endif

if !STATIC_HANDLER_SSI
module_LTLIBRARIES += server_plugin_ssi.la
server_plugin_ssi_la_SOURCES = mod_ssi.cpp
server_plugin_ssi_la_LIBADD  = $(ulib_la) @ULIB_LIBS@
server_plugin_ssi_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif

if !STATIC_HANDLER_TSA
module_LTLIBRARIES += server_plugin_tsa.la
server_plugin_tsa_la_SOURCES = mod_tsa.cpp
server_plugin_tsa_la_LIBADD  = $(ulib_la) @ULIB_LIBS@
server_plugin_tsa_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif

if !STATIC_HANDLER_HTTP
module_LTLIBRARIES += server_plugin_http.la
server_plugin_http_la_SOURCES = mod_http.cpp
server_plugin_http_la_LIBADD  = $(ulib_la) @ULIB_LIBS@
server_plugin_http_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif

if !STATIC_HANDLER_ECHO
module_LTLIBRARIES += server_plugin_echo.la
server_plugin_echo_la_SOURCES = mod_echo.cpp
server_plugin_echo_la_LIBADD  = $(ulib_la) @ULIB_LIBS@
server_plugin_echo_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif

SUBDIRS = usp

if PAGE_SPEED
SUBDIRS += page_speed
endif

if V8
SUBDIRS += v8
endif

if PHP
SUBDIRS += php
endif

if RUBY
SUBDIRS += ruby
endif

if PYTHON
SUBDIRS += python
endif

if GEOIP
if !STATIC_HANDLER_GEOIP
SUBDIRS += mod_geoip
endif
endif

if SSL
if PCRE
if MOD_SHIB
if !STATIC_HANDLER_SHIB
SUBDIRS += mod_shib
endif
endif
endif
endif

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
