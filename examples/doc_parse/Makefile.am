## Makefile.am for examples/doc_parse

AUTOMAKE_OPTIONS = ## dist-shar dist-zip

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

EXTRA_DIST = *.spec* *.start

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

doc_parse_LDADD   = $(ulib_la)
doc_parse_SOURCES = parse.cpp
doc_parse_LDFLAGS = $(PRG_LDFLAGS)

noinst_PROGRAMS = doc_parse

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
