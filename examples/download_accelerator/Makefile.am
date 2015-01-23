## Makefile.am for examples/download_accelerator

AUTOMAKE_OPTIONS = ## dist-shar dist-zip

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

download_accelerator_LDADD   = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
download_accelerator_SOURCES = main.cpp
download_accelerator_LDFLAGS = $(PRG_LDFLAGS)

noinst_PROGRAMS = download_accelerator

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
