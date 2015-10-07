## Makefile.am for examples/uclient

dist_sysconf_DATA = uclient.cfg.default

DEFAULT_INCLUDES    = -I. -I$(top_srcdir)/include
DEFAULT_CONFIG_FILE = @ULIB_SYSCONFDIR@/uclient.cfg

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

if SSL
uclient_LDADD   = $(ulib_la)
uclient_SOURCES = uclient.cpp
uclient_LDFLAGS = $(PRG_LDFLAGS)
bin_PROGRAMS 	 = uclient
endif

install-exec-hook:
	@if test -f "$(DESTDIR)$(DEFAULT_CONFIG_FILE)" ; then \
      echo "$@ will not overwrite existing $(DESTDIR)$(DEFAULT_CONFIG_FILE)" ; \
   else \
      echo "$(INSTALL_DATA) uclient.cfg.default $(DESTDIR)$(DEFAULT_CONFIG_FILE)"; \
      $(INSTALL_DATA) "$(srcdir)/uclient.cfg.default" "$(DESTDIR)$(DEFAULT_CONFIG_FILE)"; \
   fi

uninstall-hook:
	@if test -f "$(DESTDIR)$(DEFAULT_CONFIG_FILE)" ; then \
      rm "$(DESTDIR)$(DEFAULT_CONFIG_FILE)"; \
   fi

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out
