## Makefile.am for libpq

EXTRA_DIST = *.c Makefile.*

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

DEFS = -D_GNU_SOURCE -D__USE_GNU -DFRONTEND

libpq_la_CPPFLAGS = @POSTGRESQL_CPPFLAGS@ -I./config
libpq_la_LDFLAGS  = @POSTGRESQL_LDFLAGS@
libpq_la_LIBADD   = @POSTGRESQL_LIBS@
libpq_la_SOURCES  = base64.c chklocale.c encnames.c fe-auth-scram.c fe-auth.c fe-connect.c fe-exec.c fe-lobj.c fe-misc.c fe-print.c \
						  fe-protocol2.c fe-protocol3.c fe-secure.c getpeereid.c inet_net_ntop.c ip.c libpq-events.c md5.c noblock.c pg_strong_random.c \
						  pgstrcasecmp.c pqexpbuffer.c pqsignal.c saslprep.c scram-common.c strlcpy.c thread.c unicode_norm.c wchar.c

if POSTGRESQL_USE_OPENSSL
libpq_la_SOURCES += sha2_openssl.c fe-secure-openssl.c
else
libpq_la_SOURCES += sha2.c
endif

## for convenience library
noinst_LTLIBRARIES = libpq.la

clean-local:
	-rm -rf core .libs *.bb* *.da *.la *.exe gmon.out
