## Makefile.am for libpq

EXTRA_DIST = *.c Makefile.* common config gssapi libpq mb nodes utils

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I./config -I$(top_srcdir)/include

DEFS = -D_GNU_SOURCE -D__USE_GNU -DFRONTEND

libpq_la_SOURCES  = base64.c chklocale.c encnames.c fe-auth-scram.c fe-auth.c fe-connect.c fe-exec.c fe-lobj.c fe-misc.c fe-print.c \
						  fe-protocol2.c fe-protocol3.c fe-secure.c getpeereid.c inet_net_ntop.c ip.c libpq-events.c md5.c noblock.c pg_strong_random.c \
						  pgstrcasecmp.c pqexpbuffer.c pqsignal.c saslprep.c scram-common.c strlcpy.c thread.c unicode_norm.c wchar.c sha2.c

## for convenience library
noinst_LTLIBRARIES = libpq.la

clean-local:
	-rm -rf core .libs *.bb* *.da *.la *.exe gmon.out
