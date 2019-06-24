## Makefile.am for examples/loginCookie

.NOTPARALLEL:

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

EXTRA_DIST = index.cpp index.usp example_declaration.h userver.cfg

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

if !MINGW
moduledir = @ULIB_MODULEDIR@/usp
module_LTLIBRARIES = index.la
index_la_SOURCES = index.cpp
index_la_LIBADD  = $(ulib_la)
index_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif

if !CROSS_COMPILING
SUFFIXES = .cpp .usp .lo .la

../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT):
	cd ../../src/ulib/net/server/plugin/usp; $(MAKE) $(AM_MAKEFLAGS) ./usp_translator$(EXEEXT)

.usp.cpp: ../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT)
	@$(WINELOADER) ../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT) $<

index.cpp: index.usp ../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT)

.lo.la:
	$(CXXLINK) -rpath $(moduledir) -module @MODULE_LIBTOOL_OPTIONS@ $< $(ulib_la)

if !STATIC_ONLY
##FOR_ALL = .libs/index.so

##all: $(FOR_ALL)
endif
endif
## ----------------------------------------------------------------------------------------------------

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out
