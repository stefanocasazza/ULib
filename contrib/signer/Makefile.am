## Makefile.am for contrib/signer

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

EXTRA_DIST = window.h RECTWrapper.h

signer_SOURCES = signer.cpp window.cpp
signer_LDADD   = $(top_builddir)/src/ulib/lib@ULIB@.la -luser32 -lkernel32 -lcomctl32 -lole32 -lwsock32 -lnetapi32 -ladvapi32 -luuid
signer_LDFLAGS = $(PRG_LDFLAGS) -mwindows ## Create GUI application

noinst_PROGRAMS = signer

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
