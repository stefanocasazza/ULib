## Makefile.am for examples/IR

.NOTPARALLEL:

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

EXTRA_DIST = IR.h posting.h cquery.h ir_session.h ir_web.cpp ir_web.usp *.spec*

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

gendoc_LDADD   	= $(ulib_la)
gendoc_SOURCES 	= gendoc.cpp
gendoc_LDFLAGS 	= $(PRG_LDFLAGS)

index1_LDADD      = libIR.la libposting.la $(ulib_la)
index1_SOURCES    = index1.cpp
index1_LDFLAGS    = $(PRG_LDFLAGS)

index2_LDADD      = libIR.la libposting.la $(ulib_la)
index2_SOURCES    = index2.cpp
index2_LDFLAGS    = $(PRG_LDFLAGS)

db_dump_LDADD     = libIR.la libposting.la $(ulib_la)
db_dump_SOURCES   = db_dump.cpp
db_dump_LDFLAGS   = $(PRG_LDFLAGS)

db_check_LDADD    = libIR.la libposting.la $(ulib_la)
db_check_SOURCES  = db_check.cpp
db_check_LDFLAGS  = $(PRG_LDFLAGS)

update_LDADD   	= libIR.la libposting.la $(ulib_la)
update_SOURCES 	= update.cpp
update_LDFLAGS 	= $(PRG_LDFLAGS)

query_LDADD    	= libcquery.la libIR.la libposting.la $(ulib_la)
query_SOURCES  	= query.cpp
query_LDFLAGS  	= $(PRG_LDFLAGS)

libIR_la_SOURCES 		 = IR.cpp
libcquery_la_SOURCES  = cquery.cpp
libposting_la_SOURCES = posting.cpp

## for convenience library
noinst_LTLIBRARIES = libcquery.la libIR.la libposting.la

noinst_PROGRAMS = gendoc index1 index2 db_dump db_check query update

## usp web section
## --------------------------------------------------------------------------------------------------------------------------
moduledir = $(libexecdir)/ulib/usp
MODULE_LIBTOOL_OPTIONS = -export-dynamic -avoid-version

if !MINGW
module_LTLIBRARIES = ir_web.la
ir_web_la_SOURCES  = ir_web.cpp
ir_web_la_LIBADD   = libcquery.la libIR.la libposting.la $(ulib_la)
ir_web_la_LDFLAGS  = -module $(MODULE_LIBTOOL_OPTIONS)
endif

if !CROSS_COMPILING
SUFFIXES = .cpp .usp .lo .la

../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT):
	cd ../../src/ulib/net/server/plugin/usp; $(MAKE) $(AM_MAKEFLAGS) ./usp_translator$(EXEEXT)

.usp.cpp: ../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT)
	@$(WINELOADER) ../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT) $<

ir_web.cpp: ir_web.usp ../../src/ulib/net/server/plugin/usp/usp_translator$(EXEEXT)

.lo.la:
	$(CXXLINK) -rpath $(moduledir) -module $(MODULE_LIBTOOL_OPTIONS) $< libcquery.la libIR.la libposting.la $(ulib_la)

if !STATIC_ONLY
##FOR_ALL = .libs/ir_web.so

##all: $(FOR_ALL)
endif
endif
## ----------------------------------------------------------------------------------------------------

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out
