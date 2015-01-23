## Makefile.am for examples/XAdES

EXTRA_DIST = *.spec*

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

if CURL
feed_LDADD      = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
feed_SOURCES    = feed.cpp
feed_LDFLAGS    = $(PRG_LDFLAGS)
endif

sign_LDADD      = libutility.la $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
sign_SOURCES    = sign.cpp
sign_LDFLAGS    = $(PRG_LDFLAGS)

archive_LDADD   = libutility.la $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
archive_SOURCES = archive.cpp
archive_LDFLAGS = $(PRG_LDFLAGS)

client_LDADD    = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
client_SOURCES  = client.cpp
client_LDFLAGS  = $(PRG_LDFLAGS)

verify_LDADD    = libverify.la libutility.la $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
verify_SOURCES  = verify.cpp
verify_LDFLAGS  = $(PRG_LDFLAGS)

libverify_la_SOURCES  = context.cpp transform.cpp xpath.cpp
libutility_la_SOURCES = utility.cpp

## for convenience library
noinst_LTLIBRARIES = libutility.la libverify.la

bin_PROGRAMS = sign archive client verify feed

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
