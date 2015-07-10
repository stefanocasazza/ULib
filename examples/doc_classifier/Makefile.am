## Makefile.am for examples/doc_classifier

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

doc_classifier_LDADD   = $(ulib_la)
doc_classifier_SOURCES = main.cpp DocumentClassifier.cpp DocumentClassifier.h
doc_classifier_LDFLAGS = $(PRG_LDFLAGS)

noinst_PROGRAMS = doc_classifier

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out
