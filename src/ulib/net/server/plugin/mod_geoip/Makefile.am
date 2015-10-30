## Makefile.am for plugin userver for GeoIP

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include -I..

ulib_la = $(top_builddir)/src/ulib/lib@ULIB@.la

moduledir = @ULIB_MODULEDIR@
module_LTLIBRARIES = server_plugin_geoip.la

server_plugin_geoip_la_SOURCES = mod_geoip.cpp
server_plugin_geoip_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
server_plugin_geoip_la_LIBADD  = -lGeoIP $(ulib_la) @ULIB_LIBS@

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
