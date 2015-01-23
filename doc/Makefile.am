## Makefile.am -- Process this file with automake to produce Makefile.in

MAINTAINERCLEANFILES = Makefile.in Makefile
EXTRA_DIST = Doxyfile

if DOXY
noinst_DATA = doxy

doxy: Doxyfile
	$(DOXYGEN) Doxyfile
	touch Doxyfile

endif
