## Makefile.am for V8 JavaScript Engine plugin for uhttp

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include -I..

ulib_la = $(top_builddir)/src/ulib/lib@ULIB@.la

moduledir = @ULIB_MODULEDIR@
module_LTLIBRARIES = server_plugin_v8.la
server_plugin_v8_la_SOURCES = mod_v8.cpp
server_plugin_v8_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
server_plugin_v8_la_LIBADD  = -lv8 $(ulib_la) @ULIB_LIBS@

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
