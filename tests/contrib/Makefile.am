## Makefile.am -- Process this file with automake to produce Makefile.in

MAINTAINERCLEANFILES	= Makefile.in

DEFAULT_INCLUDES =  -I. -I$(top_srcdir) -I$(top_srcdir)/include

EXTRA_DIST = inp ok HCSP.test RSIGN.test

## DEFS = -DU_TEST @DEFS@

TST =

if SSL
TST += RSIGN.test
endif

if MINGW
if SSL
TST += HCSP.test
endif
endif

TESTS = $(TST) ../reset.color

clean-local:
	-rm -rf out err core .libs *.bb* *.da *.gcov trace.*.[0-9]* object.*.[0-9]* stack.*.[0-9]* *.log
