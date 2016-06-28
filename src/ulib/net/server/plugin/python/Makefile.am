## Makefile.am for embed the PYTHON interpreter in the plugin for uhttp

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include -I.. @PYTHONCPPFLAGS@

ulib_la = $(top_builddir)/src/ulib/lib@ULIB@.la

moduledir = @ULIB_MODULEDIR@
module_LTLIBRARIES = server_plugin_python.la
server_plugin_python_la_SOURCES = mod_python.cpp
server_plugin_python_la_LDFLAGS = $(PYTHONCONFIGLDFLAGS) -Wl,--start-group $(PYTHONCONFIGLIBS) -Wl,--end-group -module @MODULE_LIBTOOL_OPTIONS@
server_plugin_python_la_LIBADD  = $(ulib_la) @ULIB_LIBS@

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
