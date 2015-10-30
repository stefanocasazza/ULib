## Makefile.am for embed the RUBY interpreter in the plugin for uhttp

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include -I.. @RUBYCONFIGINCLUDES@ @RUBYCPPFLAGS@

ulib_la = $(top_builddir)/src/ulib/lib@ULIB@.la

moduledir = @ULIB_MODULEDIR@
module_LTLIBRARIES = server_plugin_ruby.la
server_plugin_ruby_la_SOURCES = mod_ruby.cpp
server_plugin_ruby_la_LDFLAGS = $(RUBYCONFIGLDFLAGS) -Wl,--start-group $(RUBYCONFIGLIBS) -Wl,--end-group -module @MODULE_LIBTOOL_OPTIONS@
server_plugin_ruby_la_LIBADD  = $(ulib_la) @ULIB_LIBS@

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
