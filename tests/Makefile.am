## Makefile.am -- Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

EXTRA_DIST = .colors .function reset.color make_test.sh

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS1 = base
if DEBUG
SUBDIRS1 += debug
endif
SUBDIRS = $(SUBDIRS1) ulib
if ENABLE_EXAMPLES
SUBDIRS += examples contrib
endif

DIST_SUBDIRS = base debug ulib examples contrib

test:
	@for i in $(SUBDIRS); do cd $(abs_top_builddir)/tests/$$i; $(MAKE) $(AM_MAKEFLAGS) test; done 

dist-hook:
	find $(distdir) -type f -name \*.c   -exec /utility/expand_tab '{}' ';'
	find $(distdir) -type f -name \*.cpp -exec /utility/expand_tab '{}' ';'
	find $(distdir) -type f -name \*.cfg -exec /utility/expand_tab '{}' ';'
