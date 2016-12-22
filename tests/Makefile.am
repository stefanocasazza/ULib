## Makefile.am -- Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = no-dependencies

EXTRA_DIST = .colors .function reset.color make_test.sh

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = base ulib
if DEBUG
SUBDIRS += debug
if HTTP2
SUBDIRS += ulib/http2
endif
endif
if ENABLE_EXAMPLES
SUBDIRS += examples
endif

DIST_SUBDIRS = base debug ulib ulib/http2 examples contrib

test:
	@for i in $(SUBDIRS); do cd $(abs_top_builddir)/tests/$$i; $(MAKE) $(AM_MAKEFLAGS) test; done 

dist-hook:
	find $(distdir) -type f -name \*.c   -exec /utility/expand_tab '{}' ';'
	find $(distdir) -type f -name \*.cpp -exec /utility/expand_tab '{}' ';'
	find $(distdir) -type f -name \*.cfg -exec /utility/expand_tab '{}' ';'

clean-local:
	-rm -rf core .libs *.bb* *.da *.la gmon.out *.gc* */*.gc* */*/*.gc* */*/*/*.gc*
