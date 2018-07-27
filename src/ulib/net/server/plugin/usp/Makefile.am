## Makefile.am for plugin dynamic page userver

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = *.usp usp2so.sh *.h *.cpp *.cxx README.alldemos usp_compile.sh.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la

usp_translator_LDADD   = $(ulib_la) @ULIB_LIBS@
usp_translator_SOURCES = usp_translator.cpp
usp_translator_LDFLAGS = $(PRG_LDFLAGS)

bin_PROGRAMS = usp_translator

LDADD = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

libWorld_la_SOURCES = libWorld.cxx
libWorld_la_LIBADD  = $(LDADD)
libWorld_la_LDFLAGS = -module -rpath /nowhere @MODULE_LIBTOOL_OPTIONS@

libWorldNoSql_la_SOURCES = libWorldNoSql.cxx
libWorldNoSql_la_LIBADD  = $(LDADD)
libWorldNoSql_la_LDFLAGS = -module -rpath /nowhere @MODULE_LIBTOOL_OPTIONS@

libFortune_la_SOURCES = libFortune.cxx
libFortune_la_LIBADD  = $(LDADD)
libFortune_la_LDFLAGS = -module -rpath /nowhere @MODULE_LIBTOOL_OPTIONS@

libFortuneNoSql_la_SOURCES = libFortuneNoSql.cxx
libFortuneNoSql_la_LIBADD  = $(LDADD)
libFortuneNoSql_la_LDFLAGS = -module -rpath /nowhere @MODULE_LIBTOOL_OPTIONS@

## for convenience library
noinst_LTLIBRARIES = libWorld.la libWorldNoSql.la libFortune.la libFortuneNoSql.la

install-data-local:
	$(MAKE) $(AM_MAKEFLAGS) usp_compile.sh; chmod 777 usp_compile.sh; ${INSTALL} -m 777 usp_compile.sh $(DESTDIR)${bindir}

clean-local:
	-rm -rf core .libs *.bb* *.da *.la *.exe trace.usp_translator* gmon.out usp_compile.sh

## usp section
## ----------------------------------------------------------------------------------------------------
MODULE_LIBTOOL_OPTIONS = -export-dynamic -avoid-version

if !CROSS_COMPILING
moduledir = @ULIB_MODULEDIR@/usp
AM_CPPFLAGS = @USP_FLAGS@

#SUFFIXES = .cpp .usp .lo .la

#db.cpp: ./usp_translator$(EXEEXT)
#json.cpp: ./usp_translator$(EXEEXT)
#query.cpp: ./usp_translator$(EXEEXT)
#update.cpp: ./usp_translator$(EXEEXT)
#fortune.cpp: ./usp_translator$(EXEEXT)
#plaintext.cpp: ./usp_translator$(EXEEXT)

.usp.cpp: ./usp_translator$(EXEEXT)
	@$(WINELOADER) ./usp_translator$(EXEEXT) $<

.lo.la:
	$(CXXLINK) @USP_LDFLAGS@ -rpath $(moduledir) -module @MODULE_LIBTOOL_OPTIONS@ $< $(ulib_la) @USP_LIBS@ @ULIB_LIBS@

#all: usp_translator db.la fortune.la json.la plaintext.la query.la update.la

all: usp_translator
endif
## ----------------------------------------------------------------------------------------------------
