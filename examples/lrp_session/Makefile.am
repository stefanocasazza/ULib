## Makefile.am for examples/lrp_session

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

EXTRA_DIST = common1.cpp common2.cpp common3.cpp common4.cpp

lrp_session_LDADD   = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
lrp_session_SOURCES = session.cpp
lrp_session_LDFLAGS = $(PRG_LDFLAGS)

lrp_pusher_LDADD    = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
lrp_pusher_SOURCES  = pusher.cpp
lrp_pusher_LDFLAGS  = $(PRG_LDFLAGS)

noinst_PROGRAMS = lrp_session lrp_pusher

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
