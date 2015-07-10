## Makefile.am for examples/rsign

EXTRA_DIST = *.spec*

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

if SSL
rsignclient_rpc_LDADD   = $(ulib_la)
rsignclient_rpc_SOURCES = rsignclient_rpc.cpp
rsignclient_rpc_LDFLAGS = $(PRG_LDFLAGS)

noinst_PROGRAMS 			= rsignclient_rpc
if EXPAT
rsignclient_LDADD   		= $(ulib_la)
rsignclient_SOURCES 		= rsignclient.cpp
rsignclient_LDFLAGS 		= $(PRG_LDFLAGS)

noinst_PROGRAMS 		  += rsignclient
endif
endif

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out
