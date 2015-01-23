## Makefile.am -- Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = ## dist-shar dist-zip

MAINTAINERCLEANFILES	= Makefile.in

DEFAULT_INCLUDES =  -I. -I$(top_srcdir)/include

PRG 	= test_trace test_memerror test_objectDB test_objectIO
TESTS = trace.test memerror.test objectDB.test objectIO.test simerr.test

test_trace_SOURCES 	 = test_trace.cpp
test_memerror_SOURCES = test_memerror.cpp
test_objectDB_SOURCES = test_objectDB.cpp
test_objectIO_SOURCES = test_objectIO.cpp

check_PROGRAMS  = $(PRG)
TESTS 			+= ../reset.color

EXTRA_DIST = error.sim ok $(TESTS)

LDADD = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

clean-local:
	-rm -rf out err core tmp/* .libs *.bb* *.da *.gcov trace.*.[0-9]* object.*.[0-9]* stack.*.[0-9]* *.log
