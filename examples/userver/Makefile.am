## Makefile.am for userver

EXTRA_DIST = *.spec* *.start

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la

dist_sysconf_DATA = userver.cfg.default

DEFAULT_INCLUDES 	  = -I. -I$(top_srcdir)/include -I..
DEFAULT_CONFIG_FILE = @ULIB_SYSCONFDIR@/userver.cfg

userver_tcp_LDADD    = $(ulib_la) @FSTACKLIBS@
userver_tcp_SOURCES  = userver.cpp
userver_tcp_LDFLAGS  = $(PRG_LDFLAGS) @FSTACKLDFLAGS@
userver_tcp_CPPFLAGS = -DU_TCP_SOCKET $(CPPFLAGS)
bin_PROGRAMS  			= userver_tcp

if LIBURING
userver_rng_LDADD    = $(ulib_la)
userver_rng_SOURCES  = userver.cpp
userver_rng_LDFLAGS  = $(PRG_LDFLAGS)
userver_rng_CPPFLAGS = -DU_TCP_SOCKET -DUSERVER_RNG $(CPPFLAGS)
bin_PROGRAMS  	  	  += userver_rng
endif

if SSL
userver_ssl_LDADD    = $(ulib_la)
userver_ssl_SOURCES  = userver.cpp
userver_ssl_LDFLAGS  = $(PRG_LDFLAGS)
userver_ssl_CPPFLAGS = -DU_SSL_SOCKET $(CPPFLAGS)
bin_PROGRAMS  	  	  += userver_ssl
endif

if USERVER_UDP
userver_udp_LDADD    = $(ulib_la)
userver_udp_SOURCES  = userver.cpp
userver_udp_LDFLAGS  = $(PRG_LDFLAGS)
userver_udp_CPPFLAGS = -DU_UDP_SOCKET $(CPPFLAGS)
bin_PROGRAMS  	  	  += userver_udp
endif

if USERVER_IPC
## MINGW don't have unix socket...
if !MINGW
userver_ipc_LDADD    = $(ulib_la)
userver_ipc_SOURCES  = userver.cpp
userver_ipc_LDFLAGS  = $(PRG_LDFLAGS)
userver_ipc_CPPFLAGS = -DU_UNIX_SOCKET $(CPPFLAGS)
bin_PROGRAMS  	  	  += userver_ipc
endif
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
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out
