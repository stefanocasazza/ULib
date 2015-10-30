## Makefile.am for plugin dynamic page userver

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = *.usp usp2so.sh *.h *.cpp README.alldemos usp_compile.sh.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la

usp_translator_LDADD   = $(ulib_la) @ULIB_LIBS@
usp_translator_SOURCES = usp_translator.cpp
usp_translator_LDFLAGS = $(PRG_LDFLAGS)

bin_PROGRAMS = usp_translator

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe trace.usp_translator* gmon.out usp_compile.sh

## usp section
## ----------------------------------------------------------------------------------------------------
MODULE_LIBTOOL_OPTIONS = -export-dynamic -avoid-version

if !CROSS_COMPILING
AM_CPPFLAGS = @USP_FLAGS@
moduledir = @ULIB_MODULEDIR@/usp

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
