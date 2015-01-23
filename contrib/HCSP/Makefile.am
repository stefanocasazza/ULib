## Makefile.am for contrib/HCSP

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

EXTRA_DIST = CSP.fld CSP.dbg CSP.fcfg CSP.func CSP.h HCSP.err WinCrypt.h doc

moduledir = $(libexecdir)/ulib

module_LTLIBRARIES = HCSP.la

HCSP_la_SOURCES  = HCSP.c
HCSP_la_LIBADD   = -lcrypto
HCSP_la_LDFLAGS  = -module $(MODULE_LIBTOOL_OPTIONS)

if MINGW
if SSL
HCSP_la_LIBADD  += -lcrypt32
endif
endif

##if SSL
##all: HCSP.so
##if MINGW
##HCSP.o: HCSP.c
##	$(COMPILE.c) -c $< -o $@
##
##HCSP.so: HCSP.o
##	$(DLLTOOL) -e exports.o -l HCSP.lib $<
##	$(CC) -shared exports.o $< -o $@ -lcrypto -lcrypt32
##else
##HCSP.o: HCSP.c
##	$(COMPILE.c) -fPIC $< -o $@
##
##HCSP.so: HCSP.o
##	$(CC) -shared $< -o $@
##endif
##endif

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
