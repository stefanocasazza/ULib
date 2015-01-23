## Makefile.am for examples/lcsp

EXTRA_DIST = *.spec* *.start

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

if SSL
lcspclient_rpc_LDADD   = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
lcspclient_rpc_SOURCES = lcspclient_rpc.cpp
lcspclient_rpc_LDFLAGS = $(PRG_LDFLAGS)

noinst_PROGRAMS 		  = lcspclient_rpc
if EXPAT
lcspclient_LDADD   	  = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
lcspclient_SOURCES 	  = lcspclient.cpp
lcspclient_LDFLAGS 	  = $(PRG_LDFLAGS)

noinst_PROGRAMS 		  += lcspclient
endif
endif

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
