## Makefile.am for examples/workflow

AUTOMAKE_OPTIONS = ## dist-shar dist-zip

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

EXTRA_DIST = action.h *.spec*

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

error_LDADD   		 = libaction.la $(ulib_la) @ULIB_LIBS@
error_SOURCES 		 = error.cpp
error_LDFLAGS 		 = $(PRG_LDFLAGS)

validation_LDADD   = libaction.la $(ulib_la) @ULIB_LIBS@
validation_SOURCES = validation.cpp
validation_LDFLAGS = $(PRG_LDFLAGS)

production_LDADD   = libaction.la $(ulib_la) @ULIB_LIBS@
production_SOURCES = production.cpp
production_LDFLAGS = $(PRG_LDFLAGS)

libaction_la_SOURCES = action.cpp

## for convenience library
noinst_LTLIBRARIES = libaction.la

noinst_PROGRAMS = error validation production

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
