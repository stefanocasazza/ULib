## Makefile.am for contrib/RSIGN

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

EXTRA_DIST = RSIGN.err

RSIGN_la_SOURCES  = RSIGN.c
RSIGN_la_LIBADD   = -lcrypto -lz
RSIGN_la_LDFLAGS  = -module -rpath /nowhere $(MODULE_LIBTOOL_OPTIONS)

if MINGW
RSIGN_la_LIBADD  += -lwsock32 -lgdi32
endif

noinst_moduledir = $(libexecdir)/ulib

## for convenience module
noinst_LTLIBRARIES = RSIGN.la

##if SSL
##all: RSIGN.so
##if MINGW
##RSIGN.o: RSIGN.c
##	$(COMPILE.c) -c $< -o $@
##
##RSIGN.so: RSIGN.o
##	$(DLLTOOL) -e exports.o -l RSIGN.lib $<
##	$(CC) -shared exports.o $< -o $@ -lcrypto -lwsock32 -lgdi32 -lz
##else
##RSIGN.o: RSIGN.c
##	$(COMPILE.c) -fPIC $< -o $@
##
##RSIGN.so: RSIGN.o
##	$(CC) -shared $< -o $@
##endif
##endif

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out *.o *.so
