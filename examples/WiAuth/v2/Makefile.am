## Makefile.am for examples/WiAuth/v2

.NOTPARALLEL:

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

EXTRA_DIST = wi_auth2.cpp wi_auth2.usp wi_auth_declaration2.h

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

if !MINGW
if !STATIC_SERVLET_WI_AUTH
moduledir = @ULIB_MODULEDIR@/usp
module_LTLIBRARIES = wi_auth2.la
wi_auth2_la_SOURCES = wi_auth2.cpp
wi_auth2_la_LIBADD  = $(ulib_la)
wi_auth2_la_LDFLAGS = -module @MODULE_LIBTOOL_OPTIONS@
endif
endif

if !CROSS_COMPILING
SUFFIXES = .cpp .usp .lo .la

../../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT):
	cd ../../../src/ulib/net/server/plugin/usp; $(MAKE) $(AM_MAKEFLAGS) ./usp_translator$(EXEEXT)

.usp.cpp: ../../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT)
	@$(WINELOADER) ../../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT) $<

wi_auth2.cpp: wi_auth2.usp ../../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT)

.lo.la:
	$(CXXLINK) -rpath $(moduledir) -module @MODULE_LIBTOOL_OPTIONS@ $< $(ulib_la)

if !STATIC_ONLY
if !STATIC_SERVLET_WI_AUTH
##FOR_ALL = .libs/wi_auth2.so

##all: $(FOR_ALL)
endif
endif
endif
## ----------------------------------------------------------------------------------------------------

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out
