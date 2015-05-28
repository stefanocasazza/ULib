## Makefile.am -- Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = ## dist-shar dist-zip

MAINTAINERCLEANFILES	= Makefile.in

DEFAULT_INCLUDES =  -I. -I$(top_srcdir)/include

PRG   = test_sprintf test_misc test_random test_utility crypto_url crypto_xml crypto_base64 crypto_base64url crypto_qp
TESTS = sprintf.test misc.test random.test url.test xml.test utility.test base64.test base64url.test qp.test

test_misc_SOURCES 	    = test_misc.c
test_random_SOURCES      = test_random.c
test_sprintf_SOURCES     = test_sprintf.c
test_utility_SOURCES     = test_utility.c
crypto_url_SOURCES 	    = crypto_url.c
crypto_xml_SOURCES 	    = crypto_xml.c
crypto_base64_SOURCES    = crypto_base64.c
crypto_base64url_SOURCES = crypto_base64url.c
crypto_qp_SOURCES 	    = crypto_qp.c

if SSL
PRG 	+= crypto_des3 crypto_dgst
TESTS += engine.test dgst.test
crypto_des3_SOURCES = crypto_des3.c
crypto_dgst_SOURCES = crypto_dgst.c
endif

if LIBZ
PRG 	+= test_gzio
TESTS += gzio.test
test_gzio_SOURCES = test_gzio.c
endif

if ZIP
PRG   += test_zip
TESTS += zip.test
test_zip_LDADD = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@
endif

check_PROGRAMS  = $(PRG)
TESTS 			+= ../reset.color

EXTRA_DIST = inp ok $(TESTS)

LDADD = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

test:
	-make check

clean-local:
	-rm -rf out err core tmp/* .libs *.bb* *.da *.gcov trace.*.[0-9]* object.*.[0-9]* stack.*.[0-9]* *.log
