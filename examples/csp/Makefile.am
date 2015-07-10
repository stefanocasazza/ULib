## Makefile.am for examples/csp

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

EXTRA_DIST = *.spec*

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

if SSL
cspclient_rpc_LDADD   = $(ulib_la)
cspclient_rpc_SOURCES = cspclient_rpc.cpp
cspclient_rpc_LDFLAGS = $(PRG_LDFLAGS)

noinst_PROGRAMS 		 = cspclient_rpc
if EXPAT
cspclient_LDADD   	= $(ulib_la)
cspclient_SOURCES 	= cspclient.cpp
cspclient_LDFLAGS 	= $(PRG_LDFLAGS)

noinst_PROGRAMS 	  += cspclient
endif
endif

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out
