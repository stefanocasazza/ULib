## Makefile.am for examples/PEC_report_virus

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

EXTRA_DIST = PEC_report.h PEC_report_options.h PEC_report_anomalie.h *.spec*

PRG = PEC_date_generator

PEC_date_generator_LDADD   = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
PEC_date_generator_SOURCES = PEC_date_generator.cpp
PEC_date_generator_LDFLAGS = $(PRG_LDFLAGS)

if SSL
PEC_report_messaggi_LDADD   = libPEC_report.la $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
PEC_report_messaggi_SOURCES = PEC_report_messaggi.cpp
PEC_report_messaggi_LDFLAGS = $(PRG_LDFLAGS)

PEC_report_virus_LDADD   = libPEC_report.la $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
PEC_report_virus_SOURCES = PEC_report_virus.cpp
PEC_report_virus_LDFLAGS = $(PRG_LDFLAGS)

PEC_check_namefile_LDADD   = libPEC_report.la $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
PEC_check_namefile_SOURCES = PEC_check_namefile.cpp
PEC_check_namefile_LDFLAGS = $(PRG_LDFLAGS)

PEC_report_anomalie1_LDADD   = libPEC_report_anomalie.la libPEC_report.la $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
PEC_report_anomalie1_SOURCES = PEC_report_anomalie1.cpp
PEC_report_anomalie1_LDFLAGS = $(PRG_LDFLAGS)

PEC_report_anomalie2_LDADD   = libPEC_report_anomalie.la libPEC_report.la $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
PEC_report_anomalie2_SOURCES = PEC_report_anomalie2.cpp
PEC_report_anomalie2_LDFLAGS = $(PRG_LDFLAGS)

PRG += PEC_report_messaggi PEC_report_virus PEC_report_anomalie1 PEC_report_anomalie2 PEC_check_namefile

libPEC_report_anomalie_la_SOURCES = PEC_report_anomalie.cpp
endif

if LIBZ
PEC_report_rejected_LDADD   = libPEC_report.la $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
PEC_report_rejected_SOURCES = PEC_report_rejected.cpp
PEC_report_rejected_LDFLAGS = $(PRG_LDFLAGS)

PRG += PEC_report_rejected
endif

libPEC_report_la_SOURCES = PEC_report.cpp

## for convenience library
noinst_LTLIBRARIES = libPEC_report.la libPEC_report_anomalie.la

noinst_PROGRAMS = ${PRG}

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
