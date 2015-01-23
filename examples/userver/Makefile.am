## Makefile.am for userver

EXTRA_DIST = *.spec* *.start

dist_sysconf_DATA = userver.cfg.default

DEFAULT_INCLUDES    = -I. -I$(top_srcdir)/include
DEFAULT_CONFIG_FILE = $(sysconfdir)/userver.cfg

userver_tcp_LDADD    = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
userver_tcp_SOURCES  = userver.cpp
userver_tcp_LDFLAGS  = $(PRG_LDFLAGS)
userver_tcp_CPPFLAGS = -DU_TCP_SOCKET $(CPPFLAGS)
bin_PROGRAMS  			= userver_tcp

if SSL
userver_ssl_LDADD    = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
userver_ssl_SOURCES  = userver.cpp
userver_ssl_LDFLAGS  = $(PRG_LDFLAGS)
userver_ssl_CPPFLAGS = -DU_SSL_SOCKET $(CPPFLAGS)
bin_PROGRAMS  	  	  += userver_ssl
endif

## MINGW don't have unix socket...
if !MINGW
userver_ipc_LDADD    = $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
userver_ipc_SOURCES  = userver.cpp
userver_ipc_LDFLAGS  = $(PRG_LDFLAGS)
userver_ipc_CPPFLAGS = -DU_UNIX_SOCKET $(CPPFLAGS)
bin_PROGRAMS  	  	  += userver_ipc
endif

install-exec-hook:
	@if test -f "$(DESTDIR)$(DEFAULT_CONFIG_FILE)" ; then \
   	echo "$@ will not overwrite existing $(DESTDIR)$(DEFAULT_CONFIG_FILE)" ; \
   else \
   	echo "$(INSTALL_DATA) userver.cfg.default $(DESTDIR)$(DEFAULT_CONFIG_FILE)"; \
   	$(INSTALL_DATA) "$(srcdir)/userver.cfg.default" "$(DESTDIR)$(DEFAULT_CONFIG_FILE)"; \
   fi

uninstall-hook:
	@if test -f "$(DESTDIR)$(DEFAULT_CONFIG_FILE)" ; then \
   	rm "$(DESTDIR)$(DEFAULT_CONFIG_FILE)"; \
	fi

clean-local:
	-rm -rf core .libs *.bb* *.da *.gcov *.la *.exe gmon.out
