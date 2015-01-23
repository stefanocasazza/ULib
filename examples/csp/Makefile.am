## Makefile.am for examples/csp

EXTRA_DIST = *.spec*

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

if SSL
cspclient_rpc_LDADD   = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
cspclient_rpc_SOURCES = cspclient_rpc.cpp
cspclient_rpc_LDFLAGS = $(PRG_LDFLAGS)

noinst_PROGRAMS 		 = cspclient_rpc
if EXPAT
cspclient_LDADD   	= $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
cspclient_SOURCES 	= cspclient.cpp
cspclient_LDFLAGS 	= $(PRG_LDFLAGS)

noinst_PROGRAMS 	  += cspclient
endif
endif

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
