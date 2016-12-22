## Makefile.am for ulib/http2

RFC_TESTS = \
	rfc7230_3_2 \
	rfc7540_4_3 \
   rfc7540_8_1_2 \
   rfc7541_2_3_2 \
   rfc7541_2_3_3 \
   rfc7541_4_1 \
   rfc7541_4_2 \
   rfc7541_4_3 \
   rfc7541_4_4 \
   rfc7541_5_1 \
   rfc7541_5_2 \
   rfc7541_6_1 \
   rfc7541_6_3 \
   rfc7541_c_2_1 \
   rfc7541_c_2_2 \
   rfc7541_c_2_3 \
   rfc7541_c_2_4 \
   rfc7541_c_3_1 \
   rfc7541_c_3_2 \
   rfc7541_c_3_3 \
   rfc7541_c_4_1 \
   rfc7541_c_4_2 \
   rfc7541_c_4_3 \
   rfc7541_c_5_1 \
   rfc7541_c_5_2 \
   rfc7541_c_5_3 \
   rfc7541_c_6_1 \
   rfc7541_c_6_2 \
   rfc7541_c_6_3

EXTRA_DIST = $(RFC_TESTS) hpack_dec hpack_enc hpack_huf hpack_tbl afl_fuzz hexcheck bincheck hex_decode hex_encode common.sh

MAINTAINERCLEANFILES = Makefile.in

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

if HTTP2
hencode_LDADD   = $(ulib_la)
hencode_SOURCES = hencode.cpp
hdecode_LDADD   = $(ulib_la)
hdecode_SOURCES = hdecode.cpp

noinst_PROGRAMS = hencode hdecode

check: $(noinst_PROGRAMS)

TESTS = $(RFC_TESTS) hpack_dec hpack_enc hpack_huf hpack_tbl afl_fuzz
endif

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out *.log *.trs
