## Makefile.am for examples/client_server

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

client_LDADD   = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
client_SOURCES = client.cpp
client_LDFLAGS = $(PRG_LDFLAGS)

server_SOURCES = server.cpp
server_LDADD   = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
server_LDFLAGS = $(PRG_LDFLAGS)

noinst_PROGRAMS = client server

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
