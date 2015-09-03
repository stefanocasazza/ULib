## Makefile.am for examples/hello

EXTRA_DIST = hello.sh

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

hello_LDADD   = $(ulib_la)
hello_SOURCES = hello.cpp

noinst_PROGRAMS = hello

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out
