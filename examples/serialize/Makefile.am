## Makefile.am for examples/serialize

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

serialize_LDADD   = $(ulib_la)
serialize_SOURCES = serialize.cpp

noinst_PROGRAMS = serialize

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out
