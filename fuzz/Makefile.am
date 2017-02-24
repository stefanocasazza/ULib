## Makefile.am for uclient_fuzzer

ulib_la = @ULIBS@ $(top_builddir)/src/ulib/lib@ULIB@.la @ULIB_LIBS@

DEFAULT_INCLUDES = -I. -I$(top_srcdir)/include

uclient_fuzzer_CC 		= clang
uclient_fuzzer_CXX 		= clang++
uclient_fuzzer_CPPFLAGS = -D_GLIBCXX_GCC_GTHR_H
uclient_fuzzer_SOURCES  = uclient_fuzzer.cpp
uclient_fuzzer_LDADD    = $(ulib_la) libFuzzer.a
uclient_fuzzer_CFLAGS 	= -fno-omit-frame-pointer -fsanitize=address -fsanitize-coverage=edge,indirect-calls,8bit-counters
uclient_fuzzer_CXXFLAGS = -fno-omit-frame-pointer -fsanitize=address -fsanitize-coverage=edge,indirect-calls,8bit-counters
uclient_fuzzer_LDFLAGS  = -fsanitize=address

if CLANG
noinst_PROGRAMS = uclient_fuzzer
endif

clean-local:
	-rm -rf core .libs *.bb* *.da *.gc* *.la *.exe gmon.out
