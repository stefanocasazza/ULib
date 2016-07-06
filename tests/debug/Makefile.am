## Makefile.am -- Process this file with automake to produce Makefile.in

EXTRA_DIST = error.sim ok $(TESTS)

AUTOMAKE_OPTIONS = ## dist-shar dist-zip

MAINTAINERCLEANFILES	= Makefile.in

DEFAULT_INCLUDES =  -I. -I$(top_srcdir)/include

PRG 	= test_trace test_memerror
TESTS = trace.test memerror.test simerr.test

test_trace_SOURCES 	 = test_trace.cpp
test_memerror_SOURCES = test_memerror.cpp

if STDCPP
PRG 	+= test_objectDB test_objectIO
TESTS += objectDB.test objectIO.test
test_objectDB_SOURCES = test_objectDB.cpp
test_objectIO_SOURCES = test_objectIO.cpp
endif

check_PROGRAMS = $(PRG)
TESTS 		  += ../reset.color

LDADD = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

test:
	-make check

clean-local:
	-rm -rf out err core tmp/* .libs *.bb* *.da *.gc* trace.*.[0-9]* object.*.[0-9]* stack.*.[0-9]* *.log
