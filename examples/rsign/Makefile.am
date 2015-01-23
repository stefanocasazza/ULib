## Makefile.am for examples/rsign

EXTRA_DIST = *.spec*

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

if SSL
rsignclient_rpc_LDADD   = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
rsignclient_rpc_SOURCES = rsignclient_rpc.cpp
rsignclient_rpc_LDFLAGS = $(PRG_LDFLAGS)

noinst_PROGRAMS 			= rsignclient_rpc
if EXPAT
rsignclient_LDADD   		= $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
rsignclient_SOURCES 		= rsignclient.cpp
rsignclient_LDFLAGS 		= $(PRG_LDFLAGS)

noinst_PROGRAMS 		  += rsignclient
endif
endif

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
