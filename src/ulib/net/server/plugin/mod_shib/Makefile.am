## Makefile.am for plugin userver for shibboleth

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include -I..

AM_CXXFLAGS = @CXXFLAGS@ -fexceptions -frtti

ulib_la = $(top_builddir)/src/ulib/lib@ULIB@.la

moduledir = @ULIB_MODULEDIR@
module_LTLIBRARIES = server_plugin_shib.la

server_plugin_shib_la_SOURCES = mod_shib.cpp
server_plugin_shib_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
server_plugin_shib_la_LIBADD  = -llog4cpp -lshib -lshib-target -lsaml -lxml-security-c -lxerces-c $(ulib_la) @ULIB_LIBS@

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
