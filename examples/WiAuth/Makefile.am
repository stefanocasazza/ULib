## Makefile.am for examples/IR

.NOTPARALLEL:

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

EXTRA_DIST = wi_auth.cpp wi_auth.usp cdbmake.cpp rdbgen.cpp login_adjust.cpp count_traffic.cpp count_traffic1.cpp

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

cdbmake_LDADD   = $(ulib_la)
cdbmake_SOURCES = cdbmake.cpp
cdbmake_LDFLAGS = $(PRG_LDFLAGS)

rdbgen_LDADD   = $(ulib_la)
rdbgen_SOURCES = rdbgen.cpp
rdbgen_LDFLAGS = $(PRG_LDFLAGS)

check_binary_LDADD   = $(ulib_la)
check_binary_SOURCES = check_binary.cpp
check_binary_LDFLAGS = $(PRG_LDFLAGS)

login_adjust_LDADD   = $(ulib_la)
login_adjust_SOURCES = login_adjust.cpp
login_adjust_LDFLAGS = $(PRG_LDFLAGS)

count_traffic_LDADD   = $(ulib_la)
count_traffic_SOURCES = count_traffic.cpp
count_traffic_LDFLAGS = $(PRG_LDFLAGS)

count_traffic1_LDADD   = $(ulib_la)
count_traffic1_SOURCES = count_traffic1.cpp
count_traffic1_LDFLAGS = $(PRG_LDFLAGS)

bin_PROGRAMS = cdbmake rdbgen
noinst_PROGRAMS = login_adjust count_traffic count_traffic1 check_binary

moduledir = $(libexecdir)/ulib/usp
MODULE_LIBTOOL_OPTIONS = -export-dynamic -avoid-version

if !MINGW
if !STATIC_SERVLET_WI_AUTH
module_LTLIBRARIES = wi_auth.la
wi_auth_la_SOURCES = wi_auth.cpp
wi_auth_la_LIBADD  = $(ulib_la)
wi_auth_la_LDFLAGS = -module $(MODULE_LIBTOOL_OPTIONS)
endif
endif

if !CROSS_COMPILING
SUFFIXES = .cpp .usp .lo .la

../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT):
	cd ../../src/ulib/net/server/plugin/usp; $(MAKE) $(AM_MAKEFLAGS) ./usp_translator$(EXEEXT)

.usp.cpp: ../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT)
	@$(WINELOADER) ../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT) $<

wi_auth.cpp: wi_auth.usp ../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT)

.lo.la:
	$(CXXLINK) -rpath $(moduledir) -module $(MODULE_LIBTOOL_OPTIONS) $< $(ulib_la)

if !STATIC_ONLY
if !STATIC_SERVLET_WI_AUTH
##FOR_ALL = .libs/wi_auth.so

##all: $(FOR_ALL)
endif
endif
endif
## ----------------------------------------------------------------------------------------------------

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out
